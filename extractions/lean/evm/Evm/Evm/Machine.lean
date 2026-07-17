import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Evm
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Lifecycle

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

noncomputable section
namespace Evm

open ConcurrencyInterfaceV1

open Defs
namespace Functions

open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def record_refund (delta : gas_refund) : SailM Unit := do
  writeReg frame_refund (gas_refund_add (← readReg frame_refund) delta)

def frame_code_len (_ : Unit) : SailM byte_quantity := do
  (pure (← readReg frame_code).bytes.len)

def frame_jumpdest_valid (dest : byte_quantity) : SailM Bool := do
  (jumpdest_ref_contains (← readReg frame_code).jumpdests (← readReg frame_code).bytes.len dest)

def STACK_LIMIT : operand_stack_height := 1024

def exc_halt (k : ExceptionKind) : SailM Unit := do
  writeReg gas_remaining GAS_ZERO
  writeReg frame_status (Exceptional k)

def stack_height (_ : Unit) : SailM Nat := do
  let height ← do (stack_depth ())
  assert (height ≤b STACK_LIMIT) "sail/evm/machine.sail:52.32-52.33"
  (pure height)

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 16 -/
def peek (n : Nat) : SailM (BitVec 256) := do
  (stack_peek_word n)

def push (w : (BitVec 256)) : SailM Unit := do
  if ((STACK_LIMIT ≤b (← (stack_height ()))) : Bool)
  then (exc_halt StackOverflow)
  else (stack_push_word w)

def pop (_ : Unit) : SailM (BitVec 256) := do
  if (((← (stack_height ())) == 0) : Bool)
  then
    (do
      (exc_halt StackUnderflow)
      (pure WORD_ZERO))
  else (stack_pop_word ())

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 16 -/
def stack_set (n : Nat) (w : (BitVec 256)) : SailM Unit := do
  (stack_set_word n w)

def is_running (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Running () => (pure true)
  | _ => (pure false)

def calldata_install (data : EvmByteSlice) : SailM Unit := do
  writeReg calldata data

def returndata_clear (_ : Unit) : SailM Unit := do
  writeReg returndata EMPTY_SLICE

def returndata_size (_ : Unit) : SailM byte_quantity := do
  (pure (← readReg returndata).len)

def returndata_byte (index : byte_quantity) : SailM (BitVec 8) := do
  (slice_byte (← readReg returndata) index)

def returndata_copy (dst : byte_quantity) (off : byte_quantity) (len : byte_quantity) : SailM Unit := do
  (slice_copy (← readReg returndata) dst off len)

def returndata_copy_prefix (dst : byte_quantity) (want : byte_quantity) : SailM Unit := do
  let have' ← do (returndata_size ())
  (slice_copy (← readReg returndata) dst BYTE_ZERO
    (if ((byte_quantity_lt want have') : Bool)
    then want
    else have'))

def returndata_copy_words (dst : byte_quantity) (source_offset : (BitVec 256)) (length : (BitVec 256)) : SailM Unit := do
  let available ← do (word_of_byte_quantity (← (returndata_size ())))
  if (((word_ule source_offset available) && (word_ule length (available - source_offset))) : Bool)
  then
    (returndata_copy dst (ByteQuantity (BitVec.toNatInt source_offset))
      (ByteQuantity (BitVec.toNatInt length)))
  else (exc_halt InvalidOpcode)

def evm_memory_size (_ : Unit) : SailM byte_quantity := do
  (pure (← readReg evm_memory).len)

def memory_byte_slice (off : byte_quantity) (len : byte_quantity) : SailM EvmByteSlice := do
  if ((byte_quantity_equal len BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else (sub_slice (← readReg evm_memory) off len)

def memory_reset (_ : Unit) : SailM Unit := do
  (mem_clear ())
  writeReg evm_memory (byte_slice EvmMemorySource BYTE_ZERO BYTE_ZERO)

def memory_expand_to (new_size : byte_quantity) : SailM Unit := do
  if ((byte_quantity_lt (← (evm_memory_size ())) new_size) : Bool)
  then
    (do
      if ((← (mem_expand new_size)) : Bool)
      then writeReg evm_memory (byte_slice EvmMemorySource (← readReg evm_memory).off new_size)
      else (exc_halt OutOfGas))
  else (pure ())

def memory_frame_enter (_ : Unit) : SailM EvmByteSlice := do
  let parent ← do readReg evm_memory
  let base ← do (mem_frame_enter ())
  if ((byte_quantity_equal base MAX_BYTE_QUANTITY) : Bool)
  then
    (do
      (exc_halt OutOfGas)
      writeReg evm_memory (byte_slice EvmMemorySource BYTE_ZERO BYTE_ZERO))
  else writeReg evm_memory (byte_slice EvmMemorySource base BYTE_ZERO)
  (pure parent)

def memory_frame_leave (parent : EvmByteSlice) : SailM Unit := do
  (mem_frame_leave ())
  writeReg evm_memory parent

def suspend_frame (_ : Unit) : SailM FrameCheckpoint := do
  let state ← do (k_state_checkpoint ())
  let saved_pc ← do readReg pc
  let saved_gas ← do readReg gas_remaining
  let saved_refund ← do readReg frame_refund
  let saved_status ← do readReg frame_status
  let saved_message ← do readReg message
  let saved_depth ← do readReg call_depth
  let saved_code ← do readReg frame_code
  let saved_calldata ← do readReg calldata
  (stack_enter_frame ())
  let saved_memory ← do (memory_frame_enter ())
  (pure { state := state
          pc := saved_pc
          gas_remaining := saved_gas
          refund := saved_refund
          status := saved_status
          message := saved_message
          call_depth := saved_depth
          code := saved_code
          calldata := saved_calldata
          memory := saved_memory })

def restore_frame (checkpoint : FrameCheckpoint) : SailM Unit := do
  (stack_leave_frame ())
  (memory_frame_leave checkpoint.memory)
  writeReg pc checkpoint.pc
  writeReg gas_remaining checkpoint.gas_remaining
  writeReg frame_refund checkpoint.refund
  writeReg frame_status checkpoint.status
  writeReg message checkpoint.message
  writeReg call_depth checkpoint.call_depth
  writeReg frame_code checkpoint.code
  writeReg calldata checkpoint.calldata

def mem_get_byte (off : byte_quantity) : SailM (BitVec 8) := do
  if ((← (is_running ())) : Bool)
  then (mem_read_byte off)
  else (pure 0x00#8)

def mem_set_byte (off : byte_quantity) (v : (BitVec 8)) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_write_byte off v)
  else (pure ())

def mem_load (off : byte_quantity) : SailM (BitVec 256) := do
  if ((← (is_running ())) : Bool)
  then (mem_load_word off)
  else (pure ZERO_WORD)

def mem_store (off : byte_quantity) (w : (BitVec 256)) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_store_word off w)
  else (pure ())

def mem_store_byte (off : byte_quantity) (w : (BitVec 256)) : SailM Unit := do
  (mem_set_byte off (Sail.BitVec.extractLsb w 7 0))

def mem_mcopy (dst : byte_quantity) (src : byte_quantity) (len : byte_quantity) : SailM Unit := do
  if ((byte_quantity_not_equal len BYTE_ZERO) : Bool)
  then (mem_move dst src len)
  else (pure ())

def mem_codecopy (dst : byte_quantity) (off : byte_quantity) (len : byte_quantity) : SailM Unit := do
  (slice_copy (← readReg frame_code).bytes dst off len)

def mem_keccak (off : byte_quantity) (len : byte_quantity) : SailM (BitVec 256) := do
  (keccak256_slice (← (memory_byte_slice off len)))

