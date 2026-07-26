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
def DEPTH_LIMIT : frame_depth := 1024

/- Type quantifiers: left : Int, right : Int, ((- gas_refund_bound)) ≤ left ∧
  left ≤ gas_refund_bound ∧ ((- gas_refund_bound)) ≤ right ∧ right ≤ gas_refund_bound -/
def validated_refund_add (left : Int) (right : Int) : SailM Int := do
  let total := (left +i right)
  if ((((Neg.neg (199 *i ((2 ^i 64) - 1))) ≤b total) && (total ≤b (199 *i ((2 ^i 64) - 1)))) : Bool)
  then (pure total)
  else sailThrow ((InvalidBlock ExecutionInvalid))

/- Type quantifiers: delta : Int, ((- gas_refund_bound)) ≤ delta ∧ delta ≤ gas_refund_bound -/
def record_refund (delta : Int) : SailM Unit := do
  writeReg frame_refund (← (validated_refund_add (← readReg frame_refund) delta))

/-- The frame code length in bytes (`CODESIZE`). -/
def frame_code_len (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, code⟩⟩ ← do (pure (← readReg frame_code).bytes)
  let length := code.len
  (pure length)

/- Type quantifiers: dest : Nat, (source_valid_length dest) -/
def frame_jumpdest_valid (dest : Nat) : SailM Bool := do
  let ⟨_, ⟨_, code⟩⟩ ← do (pure (← readReg frame_code).bytes)
  let length := code.len
  (jumpdest_ref_contains (← readReg frame_code).jumpdests length dest)

/-- The 1024-element operand-stack limit (YP §9.1). -/
def STACK_LIMIT : operand_stack_height := 1024

/- Type quantifiers: left : Nat, right : Nat, (live_gas_valid left) ∧ (live_gas_valid right) -/
def conserved_gas_add (left : Nat) (right : Nat) : Nat :=
  (left + right)

/-- Returns any Amsterdam state-gas spill to the execution-gas pool and
restores the frame's state-gas reservoir. -/
def refill_frame_state_gas (_ : Unit) : SailM Unit := do
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      writeReg gas_remaining (conserved_gas_add (← readReg gas_remaining)
        (← readReg state_gas_spilled))
      writeReg state_gas_remaining (← readReg message).state_gas_reservoir
      writeReg state_gas_spilled STATE_GAS_SPILL_ZERO)
  else (pure ())

/-- Computes the signed state gas consumed by the current frame. -/
def frame_state_gas_used (_ : Unit) : SailM Int := do
  let entry ← do (pure (← readReg message).state_gas_reservoir)
  let remaining ← do readReg state_gas_remaining
  let spilled ← do readReg state_gas_spilled
  (pure ((entry -i remaining) +i spilled))

/-- Enters the exceptional halting state (YP §9.4.2), consuming all
remaining execution gas after consolidating the frame's state gas. -/
def exc_halt (k : ExceptionKind) : SailM Unit := do
  (refill_frame_state_gas ())
  writeReg gas_remaining GAS_ZERO
  writeReg frame_status (Exceptional k)

/-- The stack height as a Sail integer. -/
def stack_height (_ : Unit) : SailM Nat := do
  let height ← do (stack_depth ())
  assert (height ≤b STACK_LIMIT) "sail/evm/machine.sail:131.32-131.33"
  (pure height)

/-- The `n`-th-from-top operand (`n = 0` is the top); zero out of
range. -/
/- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 1023 -/
def peek (n : Nat) : SailM Nat := do
  (stack_peek_word n)

/-- Pushes a word, entering `StackOverflow` at the 1024 limit. -/
/- Type quantifiers: w : Nat, 0 ≤ w ∧ w ≤ (2 ^ 256 - 1) -/
def push_word (w : Nat) : SailM Unit := do
  if ((STACK_LIMIT ≤b (← (stack_height ()))) : Bool)
  then (exc_halt StackOverflow)
  else (stack_push_word w)

/-- Pushes the mathematical live-gas quantity as an EVM word. Transaction
validation establishes that every reachable value is below the word
modulus; the total canonical definition retains modular behavior outside
that invariant. -/
/- Type quantifiers: value : Nat, 0 ≤ value -/
def push_gas (value : Nat) : SailM Unit := do
  (push_word (U256 (Nat.mod value (2 ^i 256))))

/-- Pops the top word, entering `StackUnderflow` (and yielding zero) on
an empty stack. -/
def pop (_ : Unit) : SailM Nat := do
  if (((← (stack_height ())) == 0) : Bool)
  then
    (do
      (exc_halt StackUnderflow)
      (pure WORD_ZERO))
  else (stack_pop_word ())

/-- Overwrites the `n`-th-from-top operand (`SWAP`). -/
/- Type quantifiers: k_ex416369_ : Nat, k_ex416368_ : Nat, 0 ≤ k_ex416368_ ∧
  k_ex416368_ ≤ 1023, 0 ≤ k_ex416369_ ∧ k_ex416369_ ≤ (2 ^ 256 - 1) -/
def stack_set (n : Nat) (w : Nat) : SailM Unit := do
  (stack_set_word n w)

/-- Whether the frame is still running. -/
def is_running (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Running () => (pure true)
  | _ => (pure false)

/-- Installs the frame's calldata reference. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧ 0 ≤ data_dependentWitness1 -/
def calldata_install (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  writeReg calldata ⟨_, ⟨_, data⟩⟩

/-- Clears the returndata buffer (a new sub-call begins). -/
def returndata_clear (_ : Unit) : SailM Unit := do
  writeReg returndata ⟨_, ⟨_, EMPTY_SLICE⟩⟩

/-- `RETURNDATASIZE`. -/
def returndata_size (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, data⟩⟩ ← do readReg returndata
  (pure data.len)

/-- One returndata byte. -/
/- Type quantifiers: index : Nat, 0 ≤ index -/
def returndata_byte (index : Nat) : SailM (BitVec 8) := do
  (do
      let dependentArg0 := (← readReg returndata)
      (slice_byte dependentArg0 index))

/- Type quantifiers: dst : Nat, off : Nat, len : Nat, (host_valid_access dst) ∧
  (source_valid_length off) ∧ (host_valid_access len) -/
def returndata_copy (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  (do
      let dependentArg0 := (← readReg returndata)
      (slice_copy dependentArg0 dst off len))

/-- Copies `min(want, size)` returndata bytes — the `CALL`-family output
write-back. -/
/- Type quantifiers: k_ex416393_ : Nat, k_ex416392_ : Nat, 0 ≤ k_ex416392_, 0 ≤ k_ex416393_ -/
def returndata_copy_prefix (dst : Nat) (want : Nat) : SailM Unit := do
  let wanted := want
  let available ← do (returndata_size ())
  let copy_length : Nat :=
    if ((wanted <b available) : Bool)
    then wanted
    else available
  (do
      let dependentArg0 := (← readReg returndata)
      (slice_copy dependentArg0 dst 0 copy_length))

/- Type quantifiers: available : Nat, offset : Nat, (source_valid_length available) ∧
  0 ≤ offset ∧ offset ≤ available -/
def returndata_remaining (available : Nat) (offset : Nat) : Nat :=
  (available - offset)

/-- Copies a returndata range after validating its word-sized offset and length. -/
/- Type quantifiers: dst : Nat, source_offset : Nat, length : Nat, 0 ≤ source_offset ∧
  source_offset < (2 ^ 256) ∧ 0 ≤ length ∧ length < (2 ^ 256), 0 ≤ dst -/
def validated_returndata_copy (dst : Nat) (source_offset : Nat) (length : Nat) : SailM Unit := do
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

/- Type quantifiers: k_ex416402_ : Nat, k_ex416401_ : Nat, k_ex416400_ : Nat, 0 ≤ k_ex416400_, 0
  ≤ k_ex416401_ ∧ k_ex416401_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex416402_ ∧
  k_ex416402_ ≤ (2 ^ 256 - 1) -/
def returndata_copy_words (dst : Nat) (source_offset : Nat) (length : Nat) : SailM Unit := do
  (validated_returndata_copy dst source_offset length)

/-- Returns the active frame's exact byte high-water mark. -/
def evm_memory_high_water (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, memory⟩⟩ ← do readReg evm_memory
  let length := memory.len
  (pure length)

/-- Clears all frame-memory storage and resets the active range. -/
def memory_reset (_ : Unit) : SailM Unit := do
  (mem_clear ())
  writeReg evm_memory ⟨_, ⟨_, (byte_slice EvmMemorySource 0 0)⟩⟩

/- Type quantifiers: new_size : Nat, (source_valid_range 0 new_size) -/
def memory_expand_to (new_size : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, memory⟩⟩ ← do readReg evm_memory
  if _sailIf0 : ((memory.len <b new_size) : Bool) = true
  then
    (do
      let ⟨_, ⟨_, expanded⟩⟩ ← do (mem_expand new_size)
      writeReg evm_memory ⟨_, ⟨_, expanded⟩⟩
      (pure ((⟨_, ⟨_, expanded⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))
  else
    (pure ((⟨_, ⟨_, (sub_slice memory 0 new_size)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/- Type quantifiers: off : Nat, len : Nat, (memory_valid_range off len) -/
def memory_byte_slice (off : Nat) (len : Nat) : SailM (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len))) := do
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))) : (Sigma fun
      (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))
  else
    (do
      (do
        let dependentResult ← (do
            let dependentArg0 := (← (memory_expand_to (off + len)))
            pure ((⟨_, (sub_slice ((dependentArg0).2).2 off len)⟩ : (Sigma fun
            (k_ex427456_ : Nat) => (EvmByteSliceFields (k_ex427456_ + off) len)))))
        pure ((⟨_, ⟨_, (dependentResult).2⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
        (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))))

/- Type quantifiers: off : Nat, len : Nat, (memory_valid_range off len) -/
def memory_code_slice (off : Nat) (len : Nat) : SailM (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len))) := do
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure (EMPTY_CODE_SLICE : (Sigma fun (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))
  else
    (do
      (do
          let dependentArg0 := (← do
              let dependentArg0 := (← (memory_expand_to (off + len)))
              pure ((⟨_, (sub_slice ((dependentArg0).2).2 off len)⟩ : (Sigma fun
              (k_ex427503_ : Nat) => (EvmByteSliceFields (k_ex427503_ + off) len)))))
          pure ((⟨_, ⟨_, (((code_slice (dependentArg0).2)).2).2⟩⟩ : (Sigma fun
          (k_syn_off : Nat) =>
          (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))))))

/-- Saves the parent range and enters an empty child-memory frame. -/
def memory_frame_enter (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, parent⟩⟩ ← do readReg evm_memory
  let base ← (( do (mem_frame_enter ()) ) : SailM Nat )
  writeReg evm_memory ⟨_, ⟨_, (byte_slice EvmMemorySource base 0)⟩⟩
  (pure ((⟨_, ⟨_, parent⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Releases child memory and restores the saved parent range. -/
/- Type quantifiers: parent_dependentWitness1 : Nat, parent_dependentWitness0 : Nat, 0 ≤
  parent_dependentWitness0 ∧ 0 ≤ parent_dependentWitness1 -/
def memory_frame_leave (parent : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let parent_dependentWitness0 := (parent).1
  let parent_dependentWitness1 := ((parent).2).1
  let parent := ((parent).2).2
  (mem_frame_leave ())
  writeReg evm_memory ⟨_, ⟨_, parent⟩⟩

/-- Captures parent execution state and enters child stack and memory frames. -/
def suspend_frame (_ : Unit) : SailM FrameCheckpoint := do
  let state ← do (k_state_checkpoint ())
  let saved_pc ← do readReg pc
  let saved_gas ← do readReg gas_remaining
  let saved_state_gas ← do readReg state_gas_remaining
  let saved_state_spill ← do readReg state_gas_spilled
  let saved_refund ← do readReg frame_refund
  let saved_status ← do readReg frame_status
  let saved_message ← do readReg message
  let saved_depth ← do readReg call_depth
  let saved_code ← do readReg frame_code
  let ⟨_, ⟨_, saved_calldata⟩⟩ ← do readReg calldata
  (stack_enter_frame ())
  let ⟨_, ⟨_, saved_memory⟩⟩ ← do (memory_frame_enter ())
  (pure { state := state,
          pc := saved_pc,
          gas_remaining := saved_gas,
          state_gas_remaining := saved_state_gas,
          state_gas_spilled := saved_state_spill,
          refund := saved_refund,
          status := saved_status,
          message := saved_message,
          call_depth := saved_depth,
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
  writeReg state_gas_spilled checkpoint.state_gas_spilled
  writeReg frame_refund checkpoint.refund
  writeReg frame_status checkpoint.status
  writeReg message checkpoint.message
  writeReg call_depth checkpoint.call_depth
  writeReg frame_code checkpoint.code
  writeReg calldata checkpoint.calldata

/-- One memory byte; zero when the frame has halted. -/
/- Type quantifiers: off : Nat, 0 ≤ off -/
def mem_get_byte (off : Nat) : SailM (BitVec 8) := do
  if ((← (is_running ())) : Bool)
  then (mem_read_byte off)
  else (pure 0x00#8)

/-- Writes one memory byte and raises the high-water mark; a no-op when
halted. -/
/- Type quantifiers: k_ex416448_ : Nat, 0 ≤ k_ex416448_ -/
def mem_set_byte (off : Nat) (v : (BitVec 8)) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_write_byte off v)
  else (pure ())

/-- `MLOAD`: the big-endian word at `off`; zero when halted. -/
/- Type quantifiers: off : Nat, 0 ≤ off -/
def mem_load (off : Nat) : SailM Nat := do
  if ((← (is_running ())) : Bool)
  then (mem_load_word off)
  else (pure ZERO_WORD)

/-- `MSTORE`: writes the big-endian word at `off` and raises the
high-water mark. -/
/- Type quantifiers: k_ex416451_ : Nat, k_ex416450_ : Nat, 0 ≤ k_ex416450_, 0 ≤ k_ex416451_ ∧
  k_ex416451_ ≤ (2 ^ 256 - 1) -/
def mem_store (off : Nat) (w : Nat) : SailM Unit := do
  if ((← (is_running ())) : Bool)
  then (mem_store_word off w)
  else (pure ())

/-- `MSTORE8`: writes the low byte of `w`. -/
/- Type quantifiers: k_ex416453_ : Nat, k_ex416452_ : Nat, 0 ≤ k_ex416452_, 0 ≤ k_ex416453_ ∧
  k_ex416453_ ≤ (2 ^ 256 - 1) -/
def mem_store_byte (off : Nat) (w : Nat) : SailM Unit := do
  (mem_set_byte off (word_low_byte w))

/-- `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. -/
/- Type quantifiers: k_ex416456_ : Nat, k_ex416455_ : Nat, k_ex416454_ : Nat, 0 ≤ k_ex416454_, 0
  ≤ k_ex416455_, 0 ≤ k_ex416456_ -/
def mem_mcopy (dst : Nat) (src : Nat) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then (mem_move dst src len)
  else (pure ())

/-- `CODECOPY`: copies the frame's own code into memory, zero-padded past
the end. -/
/- Type quantifiers: k_ex416459_ : Nat, k_ex416458_ : Nat, k_ex416457_ : Nat, 0 ≤ k_ex416457_, 0
  ≤ k_ex416458_, 0 ≤ k_ex416459_ -/
def mem_codecopy (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  (do
      let dependentArg0 := (← readReg frame_code).bytes
      (slice_copy dependentArg0 dst off len))

/-- `KECCAK256` over the memory range `[off, off+len)`: semantically, the
caller reads the addressed bytes and hashes that byte list. -/
/- Type quantifiers: range_dependentWitness1 : Nat, range_dependentWitness0 : Nat, 0 ≤
  range_dependentWitness0 ∧ 0 ≤ range_dependentWitness1 -/
def mem_keccak (range : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : SailM Nat := do
  let range_dependentWitness0 := (range).1
  let range_dependentWitness1 := ((range).2).1
  let range := ((range).2).2
  (pure (hash_to_word
      (← do
          let dependentArg0 := (← (memory_byte_slice range.off range.len))
          (keccak256_slice dependentArg0))))

