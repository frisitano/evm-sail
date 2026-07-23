import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Primitives.Evm
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Environment
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
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
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
open ByteRegionResult
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
  writeReg frame_stack_top ⟨0⟩

/-- Whether the active frame is the top-level frame. -/
def frame_stack_is_empty (_ : Unit) : SailM Bool := do
  (pure (((← readReg frame_stack_top)).value == 0))

/-- Saves the action to perform when the newly-entered child finishes. -/
def frame_stack_push (continuation : FrameContinuation) : SailM Unit := do
  let top ← do pure (((← readReg frame_stack_top)).value)
  if ((top <b (DEPTH_LIMIT).value) : Bool)
  then
    (do
      writeReg frame_stack (vectorUpdate (← readReg frame_stack) top continuation)
      writeReg frame_stack_top ⟨(top + 1)⟩)
  else assert false "sail/evm/machine.sail:59.20-59.21"

/-- Removes and returns the current child frame's parent continuation. -/
def frame_stack_pop (_ : Unit) : SailM FrameContinuation := do
  let top ← do pure (((← readReg frame_stack_top)).value)
  if ((0 <b top) : Bool)
  then
    (do
      let parent_top := (top - 1)
      writeReg frame_stack_top ⟨parent_top⟩
      (pure (GetElem?.getElem! (← readReg frame_stack) parent_top)))
  else
    (do
      assert false "sail/evm/machine.sail:71.20-71.21"
      throw Error.Exit)

/- Type quantifiers: left : Int, right : Int, ((- gas_refund_bound)) ≤ left ∧
  left ≤ gas_refund_bound ∧ ((- gas_refund_bound)) ≤ right ∧ right ≤ gas_refund_bound -/
def validated_refund_add (left : Int) (right : Int) : SailM gas_refund := do
  let publicResult ← do
    let total := (left +i right)
    if ((((Neg.neg (199 *i ((2 ^i 64) - 1))) ≤b total) && (total ≤b (199 *i ((2 ^i 64) - 1)))) : Bool)
    then (pure total)
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/- Type quantifiers: delta : Int, ((- gas_refund_bound)) ≤ delta ∧ delta ≤ gas_refund_bound -/
def record_refund (delta : Int) : SailM Unit := do
  writeReg frame_refund ⟨((← (validated_refund_add ((← readReg frame_refund)).value delta))).value⟩

/-- The frame code length in bytes (`CODESIZE`). -/
def frame_code_len (_ : Unit) : SailM code_length := do
  let ⟨_, ⟨_, code⟩⟩ ← do (pure (← readReg frame_code).bytes)
  let length := code.len
  (pure length)

/- Type quantifiers: dest : Nat, source_valid_length(dest) -/
def frame_jumpdest_valid (dest : Nat) : SailM Bool := do
  let ⟨_, ⟨_, code⟩⟩ ← do (pure (← readReg frame_code).bytes)
  let length := code.len
  (jumpdest_ref_contains (← readReg frame_code).jumpdests length dest)

/-- The 1024-element operand-stack limit (YP §9.1). -/
def STACK_LIMIT : operand_stack_height := ⟨1024⟩

/- Type quantifiers: left : Nat, right : Nat, live_gas_valid(left) ∧ live_gas_valid(right) -/
def conserved_gas_add (left : Nat) (right : Nat) : gas :=
  (left + right)

/-- Returns any Amsterdam state-gas spill to the execution-gas pool and
restores the frame's state-gas reservoir. -/
def refill_frame_state_gas (_ : Unit) : SailM Unit := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      writeReg gas_remaining (conserved_gas_add (← readReg gas_remaining)
        ((← readReg state_gas_spilled)).value)
      writeReg state_gas_remaining (← readReg message).state_gas_reservoir
      writeReg state_gas_spilled ⟨(STATE_GAS_SPILL_ZERO).value⟩)
  else (pure ())

/-- Computes the signed state gas consumed by the current frame. -/
def frame_state_gas_used (_ : Unit) : SailM frame_state_gas_delta := do
  let entry ← do (pure (← readReg message).state_gas_reservoir)
  let remaining ← do pure ((← readReg state_gas_remaining))
  let spilled ← do pure (((← readReg state_gas_spilled)).value)
  (pure ((entry -i remaining) +i spilled))

/-- Enters the exceptional halting state (YP §9.4.2), consuming all
remaining execution gas after consolidating the frame's state gas. -/
def exc_halt (k : ExceptionKind) : SailM Unit := do
  (refill_frame_state_gas ())
  writeReg gas_remaining GAS_ZERO
  writeReg frame_status (Exceptional k)

/-- The stack height as a Sail integer. -/
def stack_height (_ : Unit) : SailM operand_stack_height := do
  let publicResult ← do
    let height ← do
      (do
          let publicResult ← (stack_depth ())
          pure ((publicResult).value))
    assert (height ≤b (STACK_LIMIT).value) "sail/evm/machine.sail:168.32-168.33"
    (pure height)
  pure (⟨publicResult⟩)

/-- The `n`-th-from-top operand (`n = 0` is the top); zero out of
range. -/
/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 1023 -/
def peek (n : stack_index) : SailM word := do
  let n := (n).value
  let publicResult ← do
    (do
        let publicResult ← (stack_peek_word ⟨n⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Pushes a word, entering `StackOverflow` at the 1024 limit. -/
/- Type quantifiers: w : Nat, 0 ≤ w ∧ w ≤ (2 ^ 256 - 1) -/
def push_word (w : word) : SailM Unit := do
  let w := (w).value
  if (((STACK_LIMIT).value ≤b ((← (stack_height ()))).value) : Bool)
  then (exc_halt StackOverflow)
  else (stack_push_word ⟨w⟩)

/-- Pushes the mathematical live-gas quantity as an EVM word. Transaction
validation establishes that every reachable value is below the word
modulus; the total canonical definition retains modular behavior outside
that invariant. -/
/- Type quantifiers: value : Nat, 0 ≤ value -/
def push_gas (value : gas) : SailM Unit := do
  (push_word ⟨((U256 (Nat.mod value (2 ^i 256)))).value⟩)

/-- Pops the top word, entering `StackUnderflow` (and yielding zero) on
an empty stack. -/
def pop (_ : Unit) : SailM word := do
  let publicResult ← do
    if ((((← (stack_height ()))).value == 0) : Bool)
    then
      (do
        (exc_halt StackUnderflow)
        (pure (WORD_ZERO).value))
    else
      (do
          let publicResult ← (stack_pop_word ())
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Overwrites the `n`-th-from-top operand (`SWAP`). -/
/- Type quantifiers: k_ex410532_ : Nat, k_ex410531_ : Nat, 0 ≤ k_ex410531_ ∧
  k_ex410531_ ≤ 1023, 0 ≤ k_ex410532_ ∧ k_ex410532_ ≤ (2 ^ 256 - 1) -/
def stack_set (n : stack_index) (w : word) : SailM Unit := do
  let n := (n).value
  let w := (w).value
  (stack_set_word ⟨n⟩ ⟨w⟩)

/-- Whether the frame is still running. -/
def is_running (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Running () => (pure true)
  | _ => (pure false)

/-- Installs the frame's calldata reference. -/
/- Type quantifiers: k_ex410536_ : Nat, k_ex410535_ : Nat, 0 ≤ k_ex410535_ ∧ 0 ≤ k_ex410536_ -/
def calldata_install (data : EvmByteSlice) : SailM Unit := do
  let data := ((data).2).2
  writeReg calldata ⟨_, ⟨_, data⟩⟩

/-- Clears the returndata buffer (a new sub-call begins). -/
def returndata_clear (_ : Unit) : SailM Unit := do
  writeReg returndata ⟨_, ⟨_, EMPTY_SLICE⟩⟩

/-- `RETURNDATASIZE`. -/
def returndata_size (_ : Unit) : SailM source_pointer := do
  let ⟨_, ⟨_, data⟩⟩ ← do pure ((← readReg returndata))
  (pure data.len)

/-- One returndata byte. -/
/- Type quantifiers: index : Nat, 0 ≤ index -/
def returndata_byte (index : source_pointer) : SailM byte := do
  (slice_byte (← readReg returndata) index)

/- Type quantifiers: dst : Nat, off : Nat, len : Nat, host_valid_access(dst) ∧
  source_valid_length(off) ∧ host_valid_access(len) -/
def returndata_copy (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  (slice_copy (← readReg returndata) dst off len)

/-- Copies `min(want, size)` returndata bytes — the `CALL`-family output
write-back. -/
/- Type quantifiers: k_ex410554_ : Nat, k_ex410553_ : Nat, 0 ≤ k_ex410553_, 0 ≤ k_ex410554_ -/
def returndata_copy_prefix (dst : memory_pointer) (want : memory_length) : SailM Unit := do
  let wanted := want
  let available ← do (returndata_size ())
  let copy_length : Nat :=
    if ((wanted <b available) : Bool)
    then wanted
    else available
  (slice_copy (← readReg returndata) dst 0 copy_length)

/- Type quantifiers: available : Nat, offset : Nat, source_valid_length(available) ∧
  0 ≤ offset ∧ offset ≤ available -/
def returndata_remaining (available : Nat) (offset : Nat) : Nat :=
  (available - offset)

/-- Copies a returndata range after validating its word-sized offset and length. -/
/- Type quantifiers: dst : Nat, source_offset : Nat, length : Nat, 0 ≤ source_offset ∧
  source_offset < (2 ^ 256) ∧ 0 ≤ length ∧ length < (2 ^ 256), 0 ≤ dst -/
def validated_returndata_copy (dst : memory_pointer) (source_offset : Nat) (length : Nat) : SailM Unit := do
  let available ← do (returndata_size ())
  if ((source_offset ≤b available) : Bool)
  then
    (do
      let remaining := (returndata_remaining available source_offset)
      let bounded_source_offset : Nat := source_offset
      if ((length ≤b remaining) : Bool)
      then
        (do
          let bounded_length : Nat := length
          (returndata_copy dst bounded_source_offset bounded_length))
      else (exc_halt InvalidOpcode))
  else (exc_halt InvalidOpcode)

/- Type quantifiers: k_ex410563_ : Nat, k_ex410562_ : Nat, k_ex410561_ : Nat, 0 ≤ k_ex410561_, 0
  ≤ k_ex410562_ ∧ k_ex410562_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410563_ ∧
  k_ex410563_ ≤ (2 ^ 256 - 1) -/
def returndata_copy_words (dst : memory_pointer) (source_offset : word) (length : word) : SailM Unit := do
  let source_offset := (source_offset).value
  let length := (length).value
  (validated_returndata_copy dst source_offset length)

/-- Returns the active frame's exact byte high-water mark. -/
def evm_memory_high_water (_ : Unit) : SailM memory_length := do
  let ⟨_, ⟨_, memory⟩⟩ ← do pure ((← readReg evm_memory))
  let length := memory.len
  (pure length)

/-- Clears all frame-memory storage and resets the active range. -/
def memory_reset (_ : Unit) : SailM Unit := do
  (mem_clear ())
  writeReg evm_memory ⟨_, ⟨_, (byte_slice EvmMemorySource 0 0)⟩⟩

/- Type quantifiers: new_size : Nat, source_valid_range(0, new_size) -/
def memory_expand_to (new_size : Nat) : SailM (EvmByteSliceLength new_size) := do
  let ⟨_, ⟨_, memory⟩⟩ ← do pure ((← readReg evm_memory))
  if ((memory.len <b new_size) : Bool)
  then
    (do
      let ⟨_, ⟨_, expanded⟩⟩ ← do (mem_expand new_size)
      writeReg evm_memory ⟨_, ⟨_, expanded⟩⟩
      (pure ((⟨_, ⟨_, expanded⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (new_size : Nat) => (EvmByteSliceFields k_off new_size)))) : (Sigma fun
        (k_off : Nat) => (Sigma fun (new_size : Nat) => (EvmByteSliceFields k_off new_size))))))
  else
    (pure ((⟨_, ⟨_, (sub_slice memory 0 new_size)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (new_size : Nat) => (EvmByteSliceFields k_off new_size)))) : (Sigma fun (k_off : Nat)
      => (Sigma fun (new_size : Nat) => (EvmByteSliceFields k_off new_size)))))

/- Type quantifiers: off : Nat, len : Nat, memory_valid_range(off, len) -/
def memory_byte_slice (off : Nat) (len : Nat) : SailM EvmByteSlice := do
  if ((len == 0) : Bool)
  then
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))) : (Sigma fun
      (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))
  else
    (do
      (do
        let dependentResult ← (do
            let ⟨_, ⟨_, dependentArg0⟩⟩ ← (memory_expand_to (off + len))
            pure ((⟨_, (sub_slice dependentArg0 off len)⟩ : (Sigma fun (k_ex421797_ : Nat) =>
            (EvmByteSliceFields (k_ex421797_ + off) len)))))
        pure ((⟨_, ⟨_, (dependentResult).2⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
        (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))))

/- Type quantifiers: off : Nat, len : Nat, memory_valid_range(off, len) -/
def memory_code_slice (off : Nat) (len : Nat) : SailM CodeSlice := do
  if ((len == 0) : Bool)
  then
    (pure ((⟨_, ⟨_, EMPTY_CODE_SLICE⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))) : (Sigma fun
      (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))
  else
    (do
      (do
        let dependentResult ← (do
            let ⟨_, dependentArg0⟩ ← (do
                let ⟨_, ⟨_, dependentArg0⟩⟩ ← (memory_expand_to (off + len))
                pure ((⟨_, (sub_slice dependentArg0 off len)⟩ : (Sigma fun (k_ex421844_ : Nat)
                => (EvmByteSliceFields (k_ex421844_ + off) len)))))
            pure ((⟨_, (code_slice dependentArg0)⟩ : (Sigma fun (k_ex421852_ : Nat) =>
            (EvmByteSliceFields (k_ex421852_ + off) len)))))
        pure ((⟨_, ⟨_, (dependentResult).2⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
        (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))))

/-- Saves the parent range and enters an empty child-memory frame. -/
def memory_frame_enter (_ : Unit) : SailM EvmByteSlice := do
  let ⟨_, ⟨_, parent⟩⟩ ← do pure ((← readReg evm_memory))
  let base ← (( do (mem_frame_enter ()) ) : SailM Nat )
  writeReg evm_memory ⟨_, ⟨_, (byte_slice EvmMemorySource base 0)⟩⟩
  (pure ((⟨_, ⟨_, parent⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Releases child memory and restores the saved parent range. -/
/- Type quantifiers: k_ex410598_ : Nat, k_ex410597_ : Nat, 0 ≤ k_ex410597_ ∧ 0 ≤ k_ex410598_ -/
def memory_frame_leave (parent : EvmByteSlice) : SailM Unit := do
  let parent := ((parent).2).2
  (mem_frame_leave ())
  writeReg evm_memory ⟨_, ⟨_, parent⟩⟩

/-- Captures parent execution state and enters child stack and memory frames. -/
def suspend_frame (_ : Unit) : SailM FrameCheckpoint := do
  let state ← do (k_state_checkpoint ())
  let saved_pc ← do pure ((← readReg pc))
  let saved_gas ← do pure ((← readReg gas_remaining))
  let saved_state_gas ← do pure ((← readReg state_gas_remaining))
  let saved_state_spill ← do pure (((← readReg state_gas_spilled)).value)
  let saved_refund ← do pure (((← readReg frame_refund)).value)
  let saved_status ← do pure ((← readReg frame_status))
  let saved_message ← do pure ((← readReg message))
  let saved_depth ← do pure (((← readReg call_depth)).value)
  let saved_code ← do pure ((← readReg frame_code))
  let ⟨_, ⟨_, saved_calldata⟩⟩ ← do pure ((← readReg calldata))
  (stack_enter_frame ())
  let ⟨_, ⟨_, saved_memory⟩⟩ ← do (memory_frame_enter ())
  (pure { state := state,
          pc := saved_pc,
          gas_remaining := saved_gas,
          state_gas_remaining := saved_state_gas,
          state_gas_spilled := ⟨saved_state_spill⟩,
          refund := ⟨saved_refund⟩,
          status := saved_status,
          message := saved_message,
          call_depth := ⟨saved_depth⟩,
          code := saved_code,
          calldata := ⟨_, ⟨_, saved_calldata⟩⟩,
          memory := ⟨_, ⟨_, saved_memory⟩⟩ })

/-- Restores a suspended parent after nested execution completes. -/
def restore_frame (checkpoint : FrameCheckpoint) : SailM Unit := do
  (stack_leave_frame ())
  (memory_frame_leave checkpoint.memory)
  writeReg pc checkpoint.pc
  writeReg gas_remaining checkpoint.gas_remaining
  writeReg state_gas_remaining checkpoint.state_gas_remaining
  writeReg state_gas_spilled ⟨(checkpoint.state_gas_spilled).value⟩
  writeReg frame_refund ⟨(checkpoint.refund).value⟩
  writeReg frame_status checkpoint.status
  writeReg message checkpoint.message
  writeReg call_depth ⟨(checkpoint.call_depth).value⟩
  writeReg frame_code checkpoint.code
  writeReg calldata checkpoint.calldata

/-- One memory byte; zero when the frame has halted. -/
/- Type quantifiers: off : Nat, 0 ≤ off -/
def mem_get_byte (off : memory_pointer) : SailM byte := do
  if ((← (is_running ())) : Bool)
  then (mem_read_byte off)
  else (pure 0x00#8)

/-- Writes one memory byte and raises the high-water mark; a no-op when
halted. -/
/- Type quantifiers: k_ex410600_ : Nat, 0 ≤ k_ex410600_ -/
def mem_set_byte (off : memory_pointer) (v : byte) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_write_byte off v)
  else (pure ())

/-- `MLOAD`: the big-endian word at `off`; zero when halted. -/
/- Type quantifiers: off : Nat, 0 ≤ off -/
def mem_load (off : memory_pointer) : SailM word := do
  let publicResult ← do
    if ((← (is_running ())) : Bool)
    then
      (do
          let publicResult ← (mem_load_word off)
          pure ((publicResult).value))
    else (pure (ZERO_WORD).value)
  pure (⟨publicResult⟩)

/-- `MSTORE`: writes the big-endian word at `off` and raises the
high-water mark. -/
/- Type quantifiers: k_ex410603_ : Nat, k_ex410602_ : Nat, 0 ≤ k_ex410602_, 0 ≤ k_ex410603_ ∧
  k_ex410603_ ≤ (2 ^ 256 - 1) -/
def mem_store (off : memory_pointer) (w : word) : SailM Unit := do
  let w := (w).value
  if ((← (is_running ())) : Bool)
  then (mem_store_word off ⟨w⟩)
  else (pure ())

/-- `MSTORE8`: writes the low byte of `w`. -/
/- Type quantifiers: k_ex410605_ : Nat, k_ex410604_ : Nat, 0 ≤ k_ex410604_, 0 ≤ k_ex410605_ ∧
  k_ex410605_ ≤ (2 ^ 256 - 1) -/
def mem_store_byte (off : memory_pointer) (w : word) : SailM Unit := do
  let w := (w).value
  (mem_set_byte off (word_low_byte w))

/-- `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. -/
/- Type quantifiers: k_ex410608_ : Nat, k_ex410607_ : Nat, k_ex410606_ : Nat, 0 ≤ k_ex410606_, 0
  ≤ k_ex410607_, 0 ≤ k_ex410608_ -/
def mem_mcopy (dst : memory_pointer) (src : memory_pointer) (len : memory_length) : SailM Unit := do
  if ((len != 0) : Bool)
  then (mem_move dst src len)
  else (pure ())

/-- `CODECOPY`: copies the frame's own code into memory, zero-padded past
the end. -/
/- Type quantifiers: k_ex410611_ : Nat, k_ex410610_ : Nat, k_ex410609_ : Nat, 0 ≤ k_ex410609_, 0
  ≤ k_ex410610_, 0 ≤ k_ex410611_ -/
def mem_codecopy (dst : memory_pointer) (off : source_pointer) (len : memory_length) : SailM Unit := do
  (slice_copy
    (⟨_, ⟨_, (((← readReg frame_code).bytes).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) dst off len)

/-- `KECCAK256` over the memory range `[off, off+len)`: semantically, the
caller reads the addressed bytes and hashes that byte list. -/
/- Type quantifiers: k_ex410615_ : Nat, k_ex410614_ : Nat, 0 ≤ k_ex410614_ ∧ 0 ≤ k_ex410615_ -/
def mem_keccak (range : MemoryRange) : SailM word := do
  let range := ((range).2).2
  let publicResult ← do
    (pure ((hash_to_word
        (← (keccak256_slice
            (⟨_, ⟨_, (((← (memory_byte_slice range.off range.len))).2).2⟩⟩ : (Sigma fun
            (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))).value)
  pure (⟨publicResult⟩)

