import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Primitives.Evm
import Evm.Host.Code
import Evm.Kernel.Environment
import Evm.Kernel.Lifecycle

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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # Machine state

The user-space half of the machine: per-frame registers, operand stack,
gas counter, and the saved-frame stack for nested calls. World state lives
in the host kernel and is reached only via `k_*` calls; world rollback on
revert is a kernel snapshot, not saved here.

## The state-passing convention

Hot machine state flows through handler signatures instead of registers,
mirroring the Yellow Paper's state-passing transition μ′ = Ξ(μ): the
remaining gas (μ_g), the program counter (μ_pc), the operand-stack cursor
(μ_s), and the frame-memory extent (μ_i) are carried by value from the
interpreter loop into each opcode handler and returned updated. No
handler reads or writes these registers: the canonical loop supplies each
step's arguments from the registers and assigns the returned state back,
while frame-boundary code — frame save/suspend ([suspend_frame][]),
resume ([restore_frame][]), and the transaction wrapper — synchronizes
them explicitly. The optimized interpreter carries the same values in
locals and touches the registers only at those frame boundaries.

## The frame registers

One [Message][type-Message] per active frame (YP "message call"
`I = (I_a, I_o, I_s, …)`); sub-calls save and restore these registers in
the interpreter. -/

/- Type quantifiers: left : Int, right : Int, ((- gas_refund_bound)) ≤ left ∧
  left ≤ gas_refund_bound ∧ ((- gas_refund_bound)) ≤ right ∧ right ≤ gas_refund_bound -/
def validated_refund_add (left : Int) (right : Int) : SailM Int := do
  let total := (left +i right)
  if ((((Neg.neg (199 *i ((2 ^i 64) - 1))) ≤b total) && (total ≤b (199 *i ((2 ^i 64) - 1)))) : Bool)
  then (pure total)
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: delta : Int, ((- gas_refund_bound)) ≤ delta ∧ delta ≤ gas_refund_bound -/
def record_refund (delta : Int) : SailM Unit := do
  writeReg frame_refund (← (validated_refund_add (← readReg frame_refund) delta))

/-- The frame code length in bytes (`CODESIZE`). -/
def frame_code_len (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, code⟩⟩ ← do readReg frame_code
  let length := code.len
  (pure length)

/- Type quantifiers: dest : Nat, (source_valid_length dest) -/
def frame_jumpdest_valid (dest : Nat) : SailM Bool := do
  let ⟨_, ⟨_, code⟩⟩ ← do readReg frame_code
  let length := code.len
  (jumpdest_ref_contains code.jumpdests length dest)

/-- The 1024-element operand-stack limit (YP §9.1). -/
def STACK_LIMIT : operand_stack_height := 1024

/- Type quantifiers: left : Nat, right : Nat, (live_gas_valid left) ∧ (live_gas_valid right) -/
def conserved_gas_add (left : Nat) (right : Nat) : Nat :=
  (left + right)

/-- Returns any Amsterdam state-gas spill to the carried execution gas and
restores the frame's state-gas reservoir. -/
/- Type quantifiers: g : Nat, 0 ≤ g -/
def refill_frame_state_gas (g : Nat) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (do
      let refilled ← do (pure (conserved_gas_add g (← readReg state_gas_spilled)))
      writeReg state_gas_remaining (← readReg message).state_gas_reservoir
      writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
      (pure refilled))
  else (pure g)

/-- Computes the signed state gas consumed by the current frame. -/
def frame_state_gas_used (_ : Unit) : SailM Int := do
  let entry ← do (pure (← readReg message).state_gas_reservoir)
  let remaining ← do readReg state_gas_remaining
  let spilled ← do readReg state_gas_spilled
  (pure ((entry -i remaining) +i spilled))

/-- Enters the exceptional halting state (YP §9.4.2), consuming all
remaining execution gas after consolidating the frame's state gas.
Takes the carried gas, performs the state-gas refill against it in the
canonical read-then-zero order, and returns the frame's zeroed gas. -/
/- Type quantifiers: k_ex551909_ : Nat, 0 ≤ k_ex551909_ -/
def exc_halt (g : Nat) (k : ExceptionKind) : SailM Nat := do
  let _ ← do (refill_frame_state_gas g)
  writeReg frame_status (Exceptional k)
  (pure GAS_ZERO)

/-- The stack height below a carried cursor. -/
def stack_height (top : (BitVec 64)) : SailM Nat := do
  (stack_top_height top)

/-- Checks the Yellow Paper stack precondition for one instruction before it
charges gas or performs side effects. `inputs` is the instruction's
required stack height (delta) and `outputs` is the height it contributes
after consuming those inputs (alpha). This is the single stack-bounds
guard: handler bodies consume and produce operands unchecked behind it. -/
/- Type quantifiers: k_ex551912_ : Nat, k_ex551911_ : Nat, k_ex551910_ : Nat, 0 ≤ k_ex551910_, 0
  ≤ k_ex551911_ ∧ k_ex551911_ ≤ 1024, 0 ≤ k_ex551912_ ∧ k_ex551912_ ≤ 1024 -/
def validate_stack (g : Nat) (top : (BitVec 64)) (inputs : Nat) (outputs : Nat) : SailM (Bool × Nat) := do
  let height ← do (stack_height top)
  if ((height <b inputs) : Bool)
  then (pure (false, (← (exc_halt g StackUnderflow))))
  else
    (do
      if ((STACK_LIMIT <b ((height - inputs) + outputs)) : Bool)
      then (pure (false, (← (exc_halt g StackOverflow))))
      else (pure (true, g)))

/-- The `n`-th-from-top operand (`n = 0` is the top), below a validated
cursor. -/
/- Type quantifiers: k_ex551913_ : Nat, 0 ≤ k_ex551913_ ∧ k_ex551913_ ≤ 1023 -/
def peek (top : (BitVec 64)) (n : Nat) : SailM Nat := do
  (stack_slot_read top n)

/-- Pushes a word below a validated cursor and returns the new cursor. -/
/- Type quantifiers: k_ex551914_ : Nat, 0 ≤ k_ex551914_ ∧ k_ex551914_ ≤ (2 ^ 256 - 1) -/
def push_word (top : (BitVec 64)) (w : Nat) : SailM (BitVec 64) := do
  let pushed ← do (stack_top_advance top 1)
  (stack_slot_write pushed 0 w)
  (pure pushed)

/-- Pushes the mathematical live-gas quantity as an EVM word. Transaction
validation establishes that every reachable value is below the word
modulus; the total canonical definition retains modular behavior outside
that invariant. -/
/- Type quantifiers: k_ex551915_ : Nat, 0 ≤ k_ex551915_ -/
def push_gas (top : (BitVec 64)) (value : Nat) : SailM (BitVec 64) := do
  let reduced := (Nat.mod value (2 ^i 256))
  let gas_word := (u256 reduced)
  (push_word top gas_word)

/-- Pops the top word below a validated cursor, returning the word and the
new cursor. -/
def pop (top : (BitVec 64)) : SailM (Nat × (BitVec 64)) := do
  let value ← do (stack_slot_read top 0)
  (pure (value, (← (stack_top_retreat top 1))))

/-- Overwrites the `n`-th-from-top operand (`SWAP`); the cursor is
unchanged. -/
/- Type quantifiers: k_ex551917_ : Nat, k_ex551916_ : Nat, 0 ≤ k_ex551916_ ∧
  k_ex551916_ ≤ 1023, 0 ≤ k_ex551917_ ∧ k_ex551917_ ≤ (2 ^ 256 - 1) -/
def stack_set (top : (BitVec 64)) (n : Nat) (w : Nat) : SailM Unit := do
  (stack_slot_write top n w)

/-- Whether the frame is still running. -/
def is_running (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Running () => (pure true)
  | _ => (pure false)

/-- Installs the frame's calldata reference. -/
def calldata_install (data : CalldataSlice) : SailM Unit := do
  writeReg calldata data

/-- Clears the returndata buffer (a new sub-call begins). -/
def returndata_clear (_ : Unit) : SailM Unit := do
  writeReg returndata ⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩

/-- `RETURNDATASIZE`. -/
def returndata_size (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, data⟩⟩ ← do readReg returndata
  (pure data.len)

/- Type quantifiers: dst : Nat, off : Nat, len : Nat, (host_valid_access dst) ∧
  (source_valid_length off) ∧ (host_valid_access len) -/
def returndata_copy (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  (do
      let dependentArg0 := (← readReg returndata)
      (output_slice_copy dependentArg0 dst off len))

/-- Copies `min(want, size)` returndata bytes — the `CALL`-family output
write-back. -/
/- Type quantifiers: k_ex551934_ : Nat, k_ex551933_ : Nat, 0 ≤ k_ex551933_ ∧
  k_ex551933_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551934_ ∧ k_ex551934_ ≤ (2 ^ 32 - 1) -/
def returndata_copy_prefix (dst : Nat) (want : Nat) : SailM Unit := do
  let wanted := want
  let available ← do (returndata_size ())
  let copy_length : Nat :=
    if ((wanted <b available) : Bool)
    then wanted
    else available
  (do
      let dependentArg0 := (← readReg returndata)
      (output_slice_copy dependentArg0 dst 0 copy_length))

/- Type quantifiers: available : Nat, offset : Nat, (source_valid_length available) ∧
  0 ≤ offset ∧ offset ≤ available -/
def returndata_remaining (available : Nat) (offset : Nat) : Nat :=
  (available - offset)

/-- Copies a returndata range after validating its word-sized offset and
length, entering an exceptional halt on the carried gas when the source
range is out of bounds. -/
/- Type quantifiers: dst : Nat, g : Nat, source_offset : Nat, length : Nat, 0 ≤ source_offset ∧
  source_offset < (2 ^ 256) ∧ 0 ≤ length ∧ length < (2 ^ 256), 0 ≤ g, 0 ≤ dst ∧
  dst ≤ (2 ^ 32 - 1) -/
def validated_returndata_copy (g : Nat) (dst : Nat) (source_offset : Nat) (length : Nat) : SailM Nat := do
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
          (returndata_copy dst bounded_source_offset bounded_length)
          (pure g))
      else (exc_halt g InvalidOpcode))
  else (exc_halt g InvalidOpcode)

/- Type quantifiers: k_ex551945_ : Nat, k_ex551944_ : Nat, k_ex551943_ : Nat, k_ex551942_ : Nat, 0
  ≤ k_ex551942_, 0 ≤ k_ex551943_ ∧ k_ex551943_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551944_ ∧
  k_ex551944_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex551945_ ∧ k_ex551945_ ≤ (2 ^ 256 - 1) -/
def returndata_copy_words (g : Nat) (dst : Nat) (source_offset : Nat) (length : Nat) : SailM Nat := do
  (validated_returndata_copy g dst source_offset length)

/-- Returns a carried cursor's exact byte high-water mark. -/
/- Type quantifiers: mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0 ≤
  mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_high_water (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : Nat :=
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let memory := mem
  let length := memory.len
  length

/-- Clears all frame-memory storage and resets the active range. -/
def memory_reset (_ : Unit) : SailM Unit := do
  (mem_clear ())
  writeReg evm_memory ⟨_, ⟨_, EMPTY_EVM_MEMORY_SLICE⟩⟩

/- Type quantifiers: mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, new_size : Nat, (source_valid_range 0 new_size), 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_expand_to (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (new_size : Nat) : SailM ((Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × (Sigma fun
  (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let memory := mem
  if _sailIf0 : ((memory.len <b new_size) : Bool) = true
  then
    (do
      let ⟨_, ⟨_, expanded⟩⟩ ← do (mem_expand new_size)
      (pure (((⟨_, ⟨_, expanded⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), ((⟨_, ⟨_, expanded⟩⟩ : (Sigma
        fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))))))
  else
    (pure (((⟨_, ⟨_, (memory_sub_slice memory 0 new_size)⟩⟩ : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), ((⟨_, ⟨_, mem⟩⟩ : (Sigma
      fun (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))))))

/- Type quantifiers: mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, off : Nat, len : Nat, (memory_valid_range off len), 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def active_memory_slice (mem : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len)))) (off : Nat) (len : Nat) : SailM ((Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × (Sigma fun
  (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure (((⟨_, ⟨_, EMPTY_EVM_MEMORY_SLICE⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), ((⟨_, ⟨_, mem⟩⟩ : (Sigma
      fun (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))))))
  else
    (do
      let (window, expanded) ← do (memory_expand_to ⟨_, ⟨_, mem⟩⟩ (off + len))
      (pure (((⟨_, ⟨_, (memory_sub_slice ((window).2).2 off len)⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (expanded : (Sigma
        fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))))))

/- Type quantifiers: mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, off : Nat, len : Nat, (memory_valid_range off len), 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_code_slice (mem : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len)))) (off : Nat) (len : Nat) : SailM ((Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (CodeRegionSliceFields mem_dependentWitness0 mem_dependentWitness1))) × (Sigma fun
  (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure (((⟨_, ⟨_, ((EMPTY_CODE_SLICE).2).2⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (CodeRegionSliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (CodeRegionSliceFields mem_dependentWitness0 mem_dependentWitness1)))), ((⟨_, ⟨_, mem⟩⟩ : (Sigma
      fun (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))))))
  else
    (do
      let (window, expanded) ← do (memory_expand_to ⟨_, ⟨_, mem⟩⟩ (off + len))
      let initcode := (memory_sub_slice ((window).2).2 off len)
      (pure ((← do
          (code_db_intern_memory ⟨_, ⟨_, initcode⟩⟩)), (expanded : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))))))

/-- Saves the parent range and enters an empty child-memory frame. -/
def memory_frame_enter (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len))) := do
  let ⟨_, ⟨_, parent⟩⟩ ← do readReg evm_memory
  let base ← (( do (mem_frame_enter ()) ) : SailM Nat )
  writeReg evm_memory ⟨_, ⟨_, (evm_memory_slice base 0)⟩⟩
  (pure ((⟨_, ⟨_, parent⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))))

/-- Releases child memory and restores the saved parent range. -/
/- Type quantifiers: parent_dependentWitness1 : Nat, parent_dependentWitness0 : Nat, 0 ≤
  parent_dependentWitness0 ∧
  0 ≤ parent_dependentWitness1 ∧
  (parent_dependentWitness0 + parent_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_frame_leave (parent : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM Unit := do
  let parent_dependentWitness0 := (parent).1
  let parent_dependentWitness1 := ((parent).2).1
  let parent := ((parent).2).2
  (mem_frame_leave ())
  writeReg evm_memory ⟨_, ⟨_, parent⟩⟩

/-- Captures parent execution state and enters child stack and memory frames.
The caller has published its carried machine state to the frame
registers: this checkpoint reads them at the one authoritative
boundary. The host operand-frame cursor mirrors `call_depth`: push the
empty child operand stack before installing the child's semantic depth,
and pop it before restoring the parent's semantic depth. -/
def suspend_frame (_ : Unit) : SailM FrameCheckpoint := do
  (k_journal_checkpoint ())
  let saved_pc ← do readReg pc
  let saved_gas ← do readReg gas_remaining
  let saved_stack ← do readReg stack_top
  let saved_state_gas ← do readReg state_gas_remaining
  let saved_state_spill ← do readReg state_gas_spilled
  let saved_refund ← do readReg frame_refund
  let saved_status ← do readReg frame_status
  let saved_message ← do readReg message
  let saved_depth ← do readReg call_depth
  let ⟨_, ⟨_, saved_code⟩⟩ ← do readReg frame_code
  let saved_calldata ← do readReg calldata
  writeReg stack_top (← (operand_stack_push_empty_frame ()))
  let ⟨_, ⟨_, saved_memory⟩⟩ ← do (memory_frame_enter ())
  (pure { pc := saved_pc,
          gas_remaining := saved_gas,
          stack_top := saved_stack,
          state_gas_remaining := saved_state_gas,
          state_gas_spilled := saved_state_spill,
          refund := saved_refund,
          status := saved_status,
          message := saved_message,
          call_depth := saved_depth,
          code := ⟨_, ⟨_, saved_code⟩⟩,
          calldata := saved_calldata,
          memory := ⟨_, ⟨_, saved_memory⟩⟩ })

/-- Restores a suspended parent after nested execution completes. -/
def restore_frame (checkpoint : FrameCheckpoint) : SailM Unit := do
  (operand_stack_pop_frame ())
  (memory_frame_leave checkpoint.memory)
  writeReg pc checkpoint.pc
  writeReg gas_remaining checkpoint.gas_remaining
  writeReg stack_top checkpoint.stack_top
  writeReg state_gas_remaining checkpoint.state_gas_remaining
  writeReg state_gas_spilled checkpoint.state_gas_spilled
  writeReg frame_refund checkpoint.refund
  writeReg frame_status checkpoint.status
  writeReg message checkpoint.message
  writeReg call_depth checkpoint.call_depth
  writeReg frame_code checkpoint.code
  writeReg calldata checkpoint.calldata

/-- Writes one memory byte and raises the high-water mark. -/
/- Type quantifiers: k_ex552020_ : Nat, 0 ≤ k_ex552020_ ∧ k_ex552020_ ≤ (2 ^ 32 - 1) -/
def mem_set_byte (off : Nat) (v : (BitVec 8)) : SailM Unit := do
  (mem_write_byte off v)

/-- `MLOAD`: the big-endian word at `off`. -/
/- Type quantifiers: off : Nat, 0 ≤ off ∧ off ≤ (2 ^ 32 - 1) -/
def mem_load (off : Nat) : SailM Nat := do
  (mem_load_word off)

/-- `MSTORE`: writes the big-endian word at `off` and raises the
high-water mark. -/
/- Type quantifiers: k_ex552023_ : Nat, k_ex552022_ : Nat, 0 ≤ k_ex552022_ ∧
  k_ex552022_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552023_ ∧ k_ex552023_ ≤ (2 ^ 256 - 1) -/
def mem_store (off : Nat) (w : Nat) : SailM Unit := do
  (mem_store_word off w)

/-- `MSTORE8`: writes the low byte of `w`. -/
/- Type quantifiers: k_ex552025_ : Nat, k_ex552024_ : Nat, 0 ≤ k_ex552024_ ∧
  k_ex552024_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552025_ ∧ k_ex552025_ ≤ (2 ^ 256 - 1) -/
def mem_store_byte (off : Nat) (w : Nat) : SailM Unit := do
  let value := (word_low_byte w)
  (mem_set_byte off value)

/-- `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. -/
/- Type quantifiers: k_ex552028_ : Nat, k_ex552027_ : Nat, k_ex552026_ : Nat, 0 ≤ k_ex552026_ ∧
  k_ex552026_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552027_ ∧ k_ex552027_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex552028_ ∧ k_ex552028_ ≤ (2 ^ 32 - 1) -/
def mem_mcopy (dst : Nat) (src : Nat) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then (mem_move dst src len)
  else (pure ())

/-- `KECCAK256` over the memory range `[off, off+len)`: semantically, the
caller reads the addressed bytes and hashes that byte list. Returns
the digest word and the updated memory cursor. -/
/- Type quantifiers: range_dependentWitness1 : Nat, range_dependentWitness0 : Nat, mem_dependentWitness1
  : Nat, mem_dependentWitness0 : Nat, 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ range_dependentWitness0 ∧
  0 ≤ range_dependentWitness1 ∧
  (range_dependentWitness0 + range_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def mem_keccak (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (range : (Sigma fun
  (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (MemoryRangeFields mem_dependentWitness0 mem_dependentWitness1)))) : SailM (Nat × (Sigma fun
  (range_dependentWitness0 : Nat) =>
  (Sigma fun (range_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields range_dependentWitness0 range_dependentWitness1)))) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let range_dependentWitness0 := (range).1
  let range_dependentWitness1 := ((range).2).1
  let range := ((range).2).2
  let (bytes, expanded) ← do (active_memory_slice ⟨_, ⟨_, mem⟩⟩ range.off range.len)
  let digest ← do (memory_keccak256 bytes)
  (pure (((hash_to_word digest) : Nat), (expanded : (Sigma fun (range_dependentWitness0 : Nat) =>
    (Sigma fun (range_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields range_dependentWitness0 range_dependentWitness1))))))

