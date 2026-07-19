import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

/-! # Machine state

The user-space half of the machine: per-frame registers, operand stack,
gas counter, and the saved-frame stack for nested calls. It holds the gas
counter and all per-frame compute state (`pc`, stack, memory, code,
message, returndata). World state lives in the host kernel and is reached
only via `k_*` calls; world rollback on revert is a kernel snapshot, not
saved here.

## The frame registers

One [Message][type-Message] per active frame (YP "message call"
`I = (I_a, I_o, I_s, …)`); sub-calls save and restore these registers in
the interpreter. -/

/-- A message call may nest at most 1024 frames deep (EIP-150 / YP). -/
def DEPTH_LIMIT : frame_depth := ⟨1024⟩

/-- Starts a top-level interpretation with no suspended parent. -/
def frame_stack_reset (_ : Unit) : SailM Unit := do
  writeReg frame_stack_top 0

/-- Whether the active frame is the top-level frame. -/
def frame_stack_is_empty (_ : Unit) : SailM Bool := do
  (pure ((← readReg frame_stack_top) == 0))

/-- Saves the action to perform when the newly-entered child finishes. -/
def frame_stack_push (continuation : FrameContinuation) : SailM Unit := do
  let top ← do readReg frame_stack_top
  if ((top <b (DEPTH_LIMIT).value) : Bool)
  then
    (do
      writeReg frame_stack (vectorUpdate (← readReg frame_stack) top continuation)
      writeReg frame_stack_top (top + 1))
  else assert false "sail/evm/machine.sail:55.20-55.21"

/-- Removes and returns the current child frame's parent continuation. -/
def frame_stack_pop (_ : Unit) : SailM FrameContinuation := do
  let top ← do readReg frame_stack_top
  if ((0 <b top) : Bool)
  then
    (do
      let parent_top := (top -i 1)
      writeReg frame_stack_top parent_top
      (pure (GetElem?.getElem! (← readReg frame_stack) parent_top)))
  else
    (do
      assert false "sail/evm/machine.sail:67.20-67.21"
      throw Error.Exit)

def record_refund (delta : gas_refund) : SailM Unit := do
  writeReg frame_refund (gas_refund_add (← readReg frame_refund) delta)

/-- The frame code length in bytes (`CODESIZE`). -/
def frame_code_len (_ : Unit) : SailM code_length := do
  (pure (← readReg frame_code).bytes.len)

/-- Whether `dest` is a valid `JUMPDEST` in the frame's code
(YP §9.4.3). -/
def frame_jumpdest_valid (dest : code_pointer) : SailM Bool := do
  (jumpdest_ref_contains (← readReg frame_code).jumpdests (← readReg frame_code).bytes.len dest)

/-- The 1024-element operand-stack limit (YP §9.1). -/
def STACK_LIMIT : operand_stack_height := ⟨1024⟩

/-- Enters the exceptional halting state (YP §9.4.2), consuming all
remaining gas — unlike `REVERT`, which refunds it. Every exceptional
halt routes through here so the gas is always zeroed. -/
def exc_halt (k : ExceptionKind) : SailM Unit := do
  writeReg gas_remaining GAS_ZERO
  writeReg frame_status (Exceptional k)

def word_to_byte_quantity_or_oog (value : word) : byte_quantity :=
  (word_to_byte_quantity value)

/-- Converts an EVM memory range to byte quantities, preserving the
zero-length rule and halting when a live endpoint cannot be represented. -/
def word_byte_range_or_oog (offset : word) (length : word) : (byte_quantity × byte_quantity) :=
  if ((word_is_zero length) : Bool)
  then (BYTE_ZERO, BYTE_ZERO)
  else ((word_to_byte_quantity_or_oog offset), (word_to_byte_quantity_or_oog length))

/-- The stack height as a Sail integer. -/
def stack_height (_ : Unit) : SailM operand_stack_height := do
  let semanticResult ← do
    let height ← do
      (do
          let semanticResult ← (stack_depth ())
          pure ((semanticResult).value))
    assert (height ≤b (STACK_LIMIT).value) "sail/evm/machine.sail:127.32-127.33"
    (pure height)
  pure (⟨semanticResult⟩)

/-- The `n`-th-from-top operand (`n = 0` is the top); zero out of
range. -/
/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 16 -/
def peek (n : stack_index) : SailM word := do
  let n := (n).value
  (stack_peek_word ⟨n⟩)

/-- Pushes a word, entering `StackOverflow` at the 1024 limit. -/
def push (w : word) : SailM Unit := do
  if (((STACK_LIMIT).value ≤b ((← (stack_height ()))).value) : Bool)
  then (exc_halt StackOverflow)
  else (stack_push_word w)

/-- Pops the top word, entering `StackUnderflow` (and yielding zero) on
an empty stack. -/
def pop (_ : Unit) : SailM word := do
  if ((((← (stack_height ()))).value == 0) : Bool)
  then
    (do
      (exc_halt StackUnderflow)
      (pure WORD_ZERO))
  else (stack_pop_word ())

/-- Overwrites the `n`-th-from-top operand (`SWAP`). -/
/- Type quantifiers: k_ex161295_ : Nat, 0 ≤ k_ex161295_ ∧ k_ex161295_ ≤ 16 -/
def stack_set (n : stack_index) (w : word) : SailM Unit := do
  let n := (n).value
  (stack_set_word ⟨n⟩ w)

/-- Whether the frame is still running. -/
def is_running (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Running () => (pure true)
  | _ => (pure false)

/-- Installs the frame's calldata reference. -/
def calldata_install (data : EvmByteSlice) : SailM Unit := do
  writeReg calldata data

/-- Clears the returndata buffer (a new sub-call begins). -/
def returndata_clear (_ : Unit) : SailM Unit := do
  writeReg returndata EMPTY_SLICE

/-- `RETURNDATASIZE`. -/
def returndata_size (_ : Unit) : SailM byte_length := do
  (pure (← readReg returndata).len)

/-- One returndata byte. -/
def returndata_byte (index : source_pointer) : SailM byte := do
  (slice_byte (← readReg returndata) index)

/-- `RETURNDATACOPY` into frame memory (bounds pre-checked by the
opcode). -/
def returndata_copy (dst : memory_pointer) (off : source_pointer) (len : memory_length) : SailM Unit := do
  (slice_copy (← readReg returndata) dst off len)

/-- Copies `min(want, size)` returndata bytes — the `CALL`-family output
write-back. -/
def returndata_copy_prefix (dst : memory_pointer) (want : memory_length) : SailM Unit := do
  let have' ← do (returndata_size ())
  (slice_copy (← readReg returndata) dst BYTE_ZERO
    (if ((byte_quantity_lt want have') : Bool)
    then want
    else have'))

/-- Validates word-sized returndata bounds and copies the requested range. -/
def returndata_copy_words (dst : memory_pointer) (source_offset : word) (length : word) : SailM Unit := do
  let available ← do (word_of_byte_quantity (← (returndata_size ())))
  match (word_to_limb source_offset) with
  | .some source_bits =>
    (do
      match (word_to_limb length) with
      | .some length_bits =>
        (do
          if (((word_ule source_offset available) && (word_ule length
                 (word_sub available source_offset))) : Bool)
          then
            (returndata_copy dst (ByteQuantity (BitVec.toNatInt source_bits))
              (ByteQuantity (BitVec.toNatInt length_bits)))
          else (exc_halt InvalidOpcode))
      | none => (exc_halt InvalidOpcode))
  | none => (exc_halt InvalidOpcode)

def evm_memory_size (_ : Unit) : SailM memory_length := do
  (pure (← readReg evm_memory).len)

/-- A slice into frame memory. -/
def memory_byte_slice (off : memory_pointer) (len : memory_length) : SailM EvmByteSlice := do
  if ((len == BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else (sub_slice (← readReg evm_memory) off len)

/-- Clears all frame-memory storage and resets the active range. -/
def memory_reset (_ : Unit) : SailM Unit := do
  (mem_clear ())
  writeReg evm_memory (byte_slice EvmMemorySource BYTE_ZERO BYTE_ZERO)

/-- Expands the active frame memory to an already-charged size. -/
def memory_expand_to (new_size : memory_length) : SailM Unit := do
  if ((byte_quantity_lt (← (evm_memory_size ())) new_size) : Bool)
  then
    (do
      if ((← (mem_expand new_size)) : Bool)
      then writeReg evm_memory (byte_slice EvmMemorySource (← readReg evm_memory).off new_size)
      else (exc_halt OutOfGas))
  else (pure ())

/-- Saves the parent range and enters an empty child-memory frame. -/
def memory_frame_enter (_ : Unit) : SailM EvmByteSlice := do
  let parent ← do readReg evm_memory
  let base ← do (mem_frame_enter ())
  if ((base == MAX_BYTE_QUANTITY) : Bool)
  then
    (do
      (exc_halt OutOfGas)
      writeReg evm_memory (byte_slice EvmMemorySource BYTE_ZERO BYTE_ZERO))
  else writeReg evm_memory (byte_slice EvmMemorySource base BYTE_ZERO)
  (pure parent)

/-- Releases child memory and restores the saved parent range. -/
def memory_frame_leave (parent : EvmByteSlice) : SailM Unit := do
  (mem_frame_leave ())
  writeReg evm_memory parent

/-- Captures parent execution state and enters child stack and memory frames. -/
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

/-- Restores a suspended parent after nested execution completes. -/
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

/-- One memory byte; zero when the frame has halted. -/
def mem_get_byte (off : memory_pointer) : SailM byte := do
  if ((← (is_running ())) : Bool)
  then (mem_read_byte off)
  else (pure 0x00#8)

/-- Writes one memory byte and raises the high-water mark; a no-op when
halted. -/
def mem_set_byte (off : memory_pointer) (v : byte) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_write_byte off v)
  else (pure ())

/-- `MLOAD`: the big-endian word at `off`; zero when halted. -/
def mem_load (off : memory_pointer) : SailM word := do
  if ((← (is_running ())) : Bool)
  then (mem_load_word off)
  else (pure ZERO_WORD)

/-- `MSTORE`: writes the big-endian word at `off` and raises the
high-water mark. -/
def mem_store (off : memory_pointer) (w : word) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_store_word off w)
  else (pure ())

/-- `MSTORE8`: writes the low byte of `w`. -/
def mem_store_byte (off : memory_pointer) (w : word) : SailM Unit := do
  (mem_set_byte off (word_low_byte w))

/-- `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. -/
def mem_mcopy (dst : memory_pointer) (src : memory_pointer) (len : memory_length) : SailM Unit := do
  if ((bne len BYTE_ZERO) : Bool)
  then (mem_move dst src len)
  else (pure ())

/-- `CODECOPY`: copies the frame's own code into memory, zero-padded past
the end. -/
def mem_codecopy (dst : memory_pointer) (off : source_pointer) (len : memory_length) : SailM Unit := do
  (slice_copy (← readReg frame_code).bytes dst off len)

/-- `KECCAK256` over the memory range `[off, off+len)`: semantically, the
caller reads the addressed bytes and hashes that byte list. -/
def mem_keccak (off : memory_pointer) (len : memory_length) : SailM word := do
  (pure (hash_to_word (← (keccak256_slice (← (memory_byte_slice off len))))))

