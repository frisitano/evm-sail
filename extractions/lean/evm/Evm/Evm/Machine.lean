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

def DEPTH_LIMIT : frame_depth := ⟨1024⟩

def record_refund (delta : gas_refund) : SailM Unit := do
  writeReg frame_refund (gas_refund_add (← readReg frame_refund) delta)

def frame_code_len (_ : Unit) : SailM code_length := do
  (pure (← readReg frame_code).bytes.len)

def frame_jumpdest_valid (dest : code_pointer) : SailM Bool := do
  (jumpdest_ref_contains (← readReg frame_code).jumpdests (← readReg frame_code).bytes.len dest)

def STACK_LIMIT : operand_stack_height := ⟨1024⟩

def exc_halt (k : ExceptionKind) : SailM Unit := do
  writeReg gas_remaining GAS_ZERO
  writeReg frame_status (Exceptional k)

def stack_height (_ : Unit) : SailM operand_stack_height := do
  let semanticResult ← do
    let height ← do
      (do
          let semanticResult ← (stack_depth ())
          pure ((semanticResult).value))
    assert (height ≤b (STACK_LIMIT).value) "sail/evm/machine.sail:55.32-55.33"
    (pure height)
  pure (⟨semanticResult⟩)

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 16 -/
def peek (n : stack_index) : SailM word := do
  let n := (n).value
  (stack_peek_word ⟨n⟩)

def push (w : word) : SailM Unit := do
  if (((STACK_LIMIT).value ≤b ((← (stack_height ()))).value) : Bool)
  then (exc_halt StackOverflow)
  else (stack_push_word w)

def pop (_ : Unit) : SailM word := do
  if ((((← (stack_height ()))).value == 0) : Bool)
  then
    (do
      (exc_halt StackUnderflow)
      (pure WORD_ZERO))
  else (stack_pop_word ())

/-- Type quantifiers: k_ex161160_ : Nat, 0 ≤ k_ex161160_ ∧ k_ex161160_ ≤ 16 -/
def stack_set (n : stack_index) (w : word) : SailM Unit := do
  let n := (n).value
  (stack_set_word ⟨n⟩ w)

def is_running (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Running () => (pure true)
  | _ => (pure false)

def calldata_install (data : EvmByteSlice) : SailM Unit := do
  writeReg calldata data

def returndata_clear (_ : Unit) : SailM Unit := do
  writeReg returndata EMPTY_SLICE

def returndata_size (_ : Unit) : SailM byte_length := do
  (pure (← readReg returndata).len)

def returndata_byte (index : source_pointer) : SailM byte := do
  (slice_byte (← readReg returndata) index)

def returndata_copy (dst : memory_pointer) (off : source_pointer) (len : memory_length) : SailM Unit := do
  (slice_copy (← readReg returndata) dst off len)

def returndata_copy_prefix (dst : memory_pointer) (want : memory_length) : SailM Unit := do
  let have' ← do (returndata_size ())
  (slice_copy (← readReg returndata) dst BYTE_ZERO
    (if ((byte_quantity_lt want have') : Bool)
    then want
    else have'))

def returndata_copy_words (dst : memory_pointer) (source_offset : word) (length : word) : SailM Unit := do
  let available ← do (word_of_byte_quantity (← (returndata_size ())))
  if (((word_ule source_offset available) && (word_ule length (available - source_offset))) : Bool)
  then
    (returndata_copy dst (ByteQuantity (BitVec.toNatInt source_offset))
      (ByteQuantity (BitVec.toNatInt length)))
  else (exc_halt InvalidOpcode)

def evm_memory_size (_ : Unit) : SailM memory_length := do
  (pure (← readReg evm_memory).len)

def memory_byte_slice (off : memory_pointer) (len : memory_length) : SailM EvmByteSlice := do
  if ((byte_quantity_equal len BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else (sub_slice (← readReg evm_memory) off len)

def memory_reset (_ : Unit) : SailM Unit := do
  (mem_clear ())
  writeReg evm_memory (byte_slice EvmMemorySource BYTE_ZERO BYTE_ZERO)

def memory_expand_to (new_size : memory_length) : SailM Unit := do
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
  (pure { state := state,
          pc := saved_pc,
          gas_remaining := saved_gas,
          refund := saved_refund,
          status := saved_status,
          message := saved_message,
          call_depth := ⟨saved_depth⟩,
          code := saved_code,
          calldata := saved_calldata,
          memory := saved_memory })

def restore_frame (checkpoint : FrameCheckpoint) : SailM Unit := do
  (stack_leave_frame ())
  (memory_frame_leave checkpoint.memory)
  writeReg pc checkpoint.pc
  writeReg gas_remaining checkpoint.gas_remaining
  writeReg frame_refund checkpoint.refund
  writeReg frame_status checkpoint.status
  writeReg message checkpoint.message
  writeReg call_depth (checkpoint.call_depth).value
  writeReg frame_code checkpoint.code
  writeReg calldata checkpoint.calldata

def mem_get_byte (off : memory_pointer) : SailM byte := do
  if ((← (is_running ())) : Bool)
  then (mem_read_byte off)
  else (pure 0x00#8)

def mem_set_byte (off : memory_pointer) (v : byte) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_write_byte off v)
  else (pure ())

def mem_load (off : memory_pointer) : SailM word := do
  if ((← (is_running ())) : Bool)
  then (mem_load_word off)
  else (pure ZERO_WORD)

def mem_store (off : memory_pointer) (w : word) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_store_word off w)
  else (pure ())

def mem_store_byte (off : memory_pointer) (w : word) : SailM Unit := do
  (mem_set_byte off (Sail.BitVec.extractLsb w 7 0))

def mem_mcopy (dst : memory_pointer) (src : memory_pointer) (len : memory_length) : SailM Unit := do
  if ((byte_quantity_not_equal len BYTE_ZERO) : Bool)
  then (mem_move dst src len)
  else (pure ())

def mem_codecopy (dst : memory_pointer) (off : source_pointer) (len : memory_length) : SailM Unit := do
  (slice_copy (← readReg frame_code).bytes dst off len)

def mem_keccak (off : memory_pointer) (len : memory_length) : SailM word := do
  (keccak256_slice (← (memory_byte_slice off len)))

