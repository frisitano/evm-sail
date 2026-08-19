import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Host.Code
import Evm.Host.Stack
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
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
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

/- Type quantifiers: refund : Int, delta : Int, ((- gas_refund_bound)) ≤ delta ∧
  delta ≤ gas_refund_bound, ((- (199 * (2 ^ 64 - 1)))) ≤ refund ∧
  refund ≤ (199 * (2 ^ 64 - 1)) -/
def record_refund (refund : Int) (delta : Int) : SailM Int := do
  (validated_refund_add refund delta)

/-- The frame code length in bytes (`CODESIZE`). -/
/- Type quantifiers: frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0 : Nat, 0 ≤
  frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def frame_code_len (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : Nat :=
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let code := frame_code
  let length := code.len
  length

/- Type quantifiers: frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0 : Nat, dest :
  Nat, (source_valid_length dest), 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def frame_jumpdest_valid (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (dest : Nat) : SailM Bool := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let code := frame_code
  let length := code.len
  (jumpdest_ref_contains code.jumpdests length dest)

/-- The 1024-element operand-stack limit (YP §9.1). -/
def STACK_LIMIT : operand_stack_height := 1024

/- Type quantifiers: credit : Nat, available : Nat, 0 ≤ available ∧ available ≤ (2 ^ 64 - 1), 0
  ≤ credit ∧ credit ≤ (2 ^ 64 - 1) -/
def conserved_gas_add (available : Nat) (credit : Nat) : SailM Nat := do
  if ((credit ≤b (((2 ^i 64) - 1) - available)) : Bool)
  then (pure (available + credit))
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: state_gas_reservoir : Nat, state_gas_spilled : Nat, state_gas_remaining : Nat, g
  : Nat, 0 ≤ g ∧ g ≤ (2 ^ 64 - 1), 0 ≤ state_gas_remaining ∧
  state_gas_remaining ≤ (2 ^ 64 - 1), 0 ≤ state_gas_spilled ∧ state_gas_spilled ≤ (2 ^ 24), 0
  ≤ state_gas_reservoir ∧ state_gas_reservoir ≤ (2 ^ 64 - 1) -/
def refill_frame_state_gas (g : Nat) (state_gas_remaining : Nat) (state_gas_spilled : Nat) (state_gas_reservoir : Nat) : SailM (Nat × Nat × Nat) := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (do
      let refilled ← do (conserved_gas_add g state_gas_spilled)
      (pure (refilled, state_gas_reservoir, STATE_GAS_SPILL_ZERO)))
  else (pure (g, state_gas_remaining, state_gas_spilled))

/-- Computes the signed state gas consumed by the current frame. -/
/- Type quantifiers: k_ex609130_ : Nat, k_ex609129_ : Nat, k_ex609128_ : Nat, 0 ≤ k_ex609128_ ∧
  k_ex609128_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex609129_ ∧ k_ex609129_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex609130_ ∧ k_ex609130_ ≤ (2 ^ 24) -/
def frame_state_gas_used (state_gas_reservoir : Nat) (state_gas_remaining : Nat) (state_gas_spilled : Nat) : Int :=
  let entry := state_gas_reservoir
  let remaining := state_gas_remaining
  let spilled := state_gas_spilled
  ((entry -i remaining) +i spilled)

/- Type quantifiers: state_gas_reservoir : Nat, state_gas_spilled : Nat, state_gas_remaining : Nat, 0
  ≤ state_gas_remaining ∧ state_gas_remaining ≤ (2 ^ 64 - 1), 0 ≤ state_gas_spilled ∧
  state_gas_spilled ≤ (2 ^ 24), 0 ≤ state_gas_reservoir ∧ state_gas_reservoir ≤ (2 ^ 64 - 1) -/
def exceptional_state (state_gas_remaining : Nat) (state_gas_spilled : Nat) (state_gas_reservoir : Nat) (k : ExceptionKind) : SailM (Nat × Nat × FrameStatus) := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then (pure (state_gas_reservoir, STATE_GAS_SPILL_ZERO, (Exceptional k)))
  else (pure (state_gas_remaining, state_gas_spilled, (Exceptional k)))

/-- The stack height below a carried cursor. -/
def stack_height (top : StackPointer) : Nat :=
  (stack_top_height top)

def undefined_StackValidation (_ : Unit) : SailM StackValidation := do
  (internal_pick [StackValid, StackUnderflowFailure, StackOverflowFailure])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 2 -/
def StackValidation_of_num (arg_ : Nat) : StackValidation :=
  match arg_ with
  | 0 => StackValid
  | 1 => StackUnderflowFailure
  | _ => StackOverflowFailure

def num_of_StackValidation (arg_ : StackValidation) : Nat :=
  match arg_ with
  | .StackValid => 0
  | .StackUnderflowFailure => 1
  | .StackOverflowFailure => 2

/- Type quantifiers: outputs : Nat, inputs : Nat, 0 ≤ inputs ∧ inputs ≤ 1024, 0 ≤ outputs
  ∧ outputs ≤ 1024 -/
def validate_stack (top : StackPointer) (inputs : Nat) (outputs : Nat) : StackValidation :=
  let height := (stack_height top)
  if ((height <b inputs) : Bool)
  then StackUnderflowFailure
  else
    (if ((STACK_LIMIT <b ((height - inputs) + outputs)) : Bool)
    then StackOverflowFailure
    else StackValid)

/-- Reads the `n=0` slot under a validated cursor. -/
def read_stack_word (sp : StackPointer) : SailM Nat := do
  (stack_slot_read sp 0)

/-- Writes the `n=0` slot under a validated cursor. -/
/- Type quantifiers: k_ex609136_ : Nat, 0 ≤ k_ex609136_ ∧ k_ex609136_ ≤ (2 ^ 256 - 1) -/
def write_stack_word (sp : StackPointer) (value : Nat) : SailM Unit := do
  (stack_slot_write sp 0 value)

/-- Overwrites the `n`-th-from-top operand (`SWAP`); the cursor is
unchanged. -/
/- Type quantifiers: k_ex609138_ : Nat, k_ex609137_ : Nat, 0 ≤ k_ex609137_ ∧
  k_ex609137_ ≤ 1023, 0 ≤ k_ex609138_ ∧ k_ex609138_ ≤ (2 ^ 256 - 1) -/
def stack_set (top : StackPointer) (n : Nat) (w : Nat) : SailM Unit := do
  (stack_slot_write top n w)

/-- Whether the frame is still running. -/
def is_running (frame_status : FrameStatus) : Bool :=
  match frame_status with
  | .Running () => true
  | _ => false

/-- Installs the frame's calldata reference. -/
def calldata_install (data : CalldataSlice) : CalldataSlice :=
  data

/-- Clears the returndata buffer (a new sub-call begins). -/
def returndata_clear (_ : Unit) : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
  ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))

/-- `RETURNDATASIZE`. -/
/- Type quantifiers: returndata_dependentWitness1 : Nat, returndata_dependentWitness0 : Nat, 0 ≤
  returndata_dependentWitness0 ∧
  0 ≤ returndata_dependentWitness1 ∧
  (returndata_dependentWitness0 + returndata_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def returndata_size (returndata : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : Nat :=
  let returndata_dependentWitness0 := (returndata).1
  let returndata_dependentWitness1 := ((returndata).2).1
  let returndata := ((returndata).2).2
  let data := returndata
  data.len

/- Type quantifiers: returndata_dependentWitness1 : Nat, returndata_dependentWitness0 : Nat, dst :
  Nat, off : Nat, len : Nat, (host_valid_access dst) ∧
  (source_valid_length off) ∧ (host_valid_access len), 0 ≤ returndata_dependentWitness0 ∧
  0 ≤ returndata_dependentWitness1 ∧
  (returndata_dependentWitness0 + returndata_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def returndata_copy (returndata : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (OutputSliceFields k_syn_off k_syn_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let returndata_dependentWitness0 := (returndata).1
  let returndata_dependentWitness1 := ((returndata).2).1
  let returndata := ((returndata).2).2
  (output_slice_copy ⟨_, ⟨_, returndata⟩⟩ dst off len)

/-- Copies `min(want, size)` returndata bytes — the `CALL`-family output
write-back. -/
/- Type quantifiers: k_ex609175_ : Nat, k_ex609174_ : Nat, returndata_dependentWitness1 : Nat, returndata_dependentWitness0
  : Nat, 0 ≤ returndata_dependentWitness0 ∧
  0 ≤ returndata_dependentWitness1 ∧
  (returndata_dependentWitness0 + returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex609174_
  ∧ k_ex609174_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex609175_ ∧ k_ex609175_ ≤ (2 ^ 32 - 1) -/
def returndata_copy_prefix (returndata : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (dst : Nat) (want : Nat) : SailM Unit := do
  let returndata_dependentWitness0 := (returndata).1
  let returndata_dependentWitness1 := ((returndata).2).1
  let returndata := ((returndata).2).2
  let wanted := want
  let available := (returndata_size ⟨_, ⟨_, returndata⟩⟩)
  let copy_length : Nat :=
    if ((wanted <b available) : Bool)
    then wanted
    else available
  (output_slice_copy ⟨_, ⟨_, returndata⟩⟩ dst 0 copy_length)

/- Type quantifiers: available : Nat, offset : Nat, (source_valid_length available) ∧
  0 ≤ offset ∧ offset ≤ available -/
def returndata_remaining (available : Nat) (offset : Nat) : Nat :=
  (available - offset)

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
def memory_reset (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len))) := do
  (mem_clear ())
  (pure ((⟨_, ⟨_, EMPTY_EVM_MEMORY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))))

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
  let base ← (( do (mem_frame_enter ()) ) : SailM Nat )
  (pure ((⟨_, ⟨_, (evm_memory_slice base 0)⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))))

/-- Releases child memory and restores the saved parent range. -/
/- Type quantifiers: parent_dependentWitness1 : Nat, parent_dependentWitness0 : Nat, 0 ≤
  parent_dependentWitness0 ∧
  0 ≤ parent_dependentWitness1 ∧
  (parent_dependentWitness0 + parent_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_frame_leave (parent : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Sigma fun
  (parent_dependentWitness0 : Nat) =>
  (Sigma fun (parent_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields parent_dependentWitness0 parent_dependentWitness1))) := do
  let parent_dependentWitness0 := (parent).1
  let parent_dependentWitness1 := ((parent).2).1
  let parent := ((parent).2).2
  (mem_frame_leave ())
  (pure ((⟨_, ⟨_, parent⟩⟩ : (Sigma fun (parent_dependentWitness0 : Nat) =>
    (Sigma fun (parent_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields parent_dependentWitness0 parent_dependentWitness1)))) : (Sigma fun
    (parent_dependentWitness0 : Nat) =>
    (Sigma fun (parent_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields parent_dependentWitness0 parent_dependentWitness1)))))

/-- Captures parent execution state and enters child stack and memory frames.
The caller has published its carried machine state to the frame
registers: this checkpoint reads them at the one authoritative
boundary. The host operand-frame cursor mirrors `call_depth`: push the
empty child operand stack before installing the child's semantic depth,
and pop it before restoring the parent's semantic depth. -/
/- Type quantifiers: frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0 : Nat, k_ex609274_
  : Int, k_ex609273_ : Nat, k_ex609272_ : Nat, evm_memory_dependentWitness1 : Nat, evm_memory_dependentWitness0
  : Nat, k_ex609267_ : Nat, k_ex609266_ : Nat, 0 ≤ k_ex609266_ ∧ k_ex609266_ ≤ (2 ^ 32 - 1), 0
  ≤ k_ex609267_ ∧ k_ex609267_ ≤ (2 ^ 64 - 1), 0 ≤ evm_memory_dependentWitness0 ∧
  0 ≤ evm_memory_dependentWitness1 ∧
  (evm_memory_dependentWitness0 + evm_memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex609272_
  ∧ k_ex609272_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex609273_ ∧ k_ex609273_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex609274_ ∧ k_ex609274_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def suspend_frame (pc : Nat) (gas_remaining : Nat) (stack_top : StackPointer) (evm_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (state_gas_remaining : Nat) (state_gas_spilled : Nat) (frame_refund : Int) (frame_status : FrameStatus) (message : Message) (frame_code : (Sigma
  fun (evm_memory_dependentWitness0 : Nat) =>
  (Sigma fun (evm_memory_dependentWitness1 : Nat) =>
  (CodeFields evm_memory_dependentWitness0 evm_memory_dependentWitness1)))) (calldata : CalldataSlice) : SailM (FrameCheckpoint × StackPointer × (Sigma
  fun (frame_code_dependentWitness0 : Nat) =>
  (Sigma fun (frame_code_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields frame_code_dependentWitness0 frame_code_dependentWitness1)))) := do
  let evm_memory_dependentWitness0 := (evm_memory).1
  let evm_memory_dependentWitness1 := ((evm_memory).2).1
  let evm_memory := ((evm_memory).2).2
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  (k_journal_checkpoint ())
  let child_stack ← do (operand_stack_push_empty_frame ())
  let ⟨_, ⟨_, child_memory⟩⟩ ← do (memory_frame_enter ())
  let checkpoint : FrameCheckpoint :=
    { pc := pc,
      gas_remaining := gas_remaining,
      stack_top := stack_top,
      state_gas_remaining := state_gas_remaining,
      state_gas_spilled := state_gas_spilled,
      refund := frame_refund,
      status := frame_status,
      message := message,
      code := ⟨_, ⟨_, frame_code⟩⟩,
      calldata := calldata,
      memory := ⟨_, ⟨_, evm_memory⟩⟩ }
  (pure ((checkpoint : FrameCheckpoint), (child_stack : StackPointer), ((⟨_, ⟨_, child_memory⟩⟩ : (Sigma
    fun (frame_code_dependentWitness0 : Nat) =>
    (Sigma fun (frame_code_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields frame_code_dependentWitness0 frame_code_dependentWitness1)))) : (Sigma fun
    (frame_code_dependentWitness0 : Nat) =>
    (Sigma fun (frame_code_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields frame_code_dependentWitness0 frame_code_dependentWitness1))))))

/-- Restores a suspended parent after nested execution completes. -/
def restore_frame (checkpoint : FrameCheckpoint) : SailM (Nat × Nat × StackPointer × (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len))) × Nat × Nat × Int × FrameStatus × Message × (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) × CalldataSlice) := do
  (operand_stack_pop_frame ())
  let ⟨_, ⟨_, memory⟩⟩ ← do (memory_frame_leave checkpoint.memory)
  (pure ((checkpoint.pc : Nat), (checkpoint.gas_remaining : Nat), (checkpoint.stack_top : StackPointer), ((⟨_, ⟨_, memory⟩⟩ : (Sigma
    fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : (Sigma
    fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))), (checkpoint.state_gas_remaining : Nat), (checkpoint.state_gas_spilled : Nat), (checkpoint.refund : Int), (checkpoint.status : FrameStatus), (checkpoint.message : Message), (checkpoint.code : (Sigma
    fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))), (checkpoint.calldata : CalldataSlice)))

/-- Writes one memory byte and raises the high-water mark. -/
/- Type quantifiers: k_ex609284_ : Nat, 0 ≤ k_ex609284_ ∧ k_ex609284_ ≤ (2 ^ 32 - 1) -/
def mem_set_byte (off : Nat) (v : (BitVec 8)) : SailM Unit := do
  (mem_write_byte off v)

/-- `MLOAD`: the big-endian word at `off`. -/
/- Type quantifiers: off : Nat, 0 ≤ off ∧ off ≤ (2 ^ 32 - 1) -/
def mem_load (off : Nat) : SailM Nat := do
  (mem_load_word off)

/-- `MSTORE`: writes the big-endian word at `off` and raises the
high-water mark. -/
/- Type quantifiers: k_ex609287_ : Nat, k_ex609286_ : Nat, 0 ≤ k_ex609286_ ∧
  k_ex609286_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex609287_ ∧ k_ex609287_ ≤ (2 ^ 256 - 1) -/
def mem_store (off : Nat) (w : Nat) : SailM Unit := do
  (mem_store_word off w)

/-- `MSTORE8`: writes the low byte of `w`. -/
/- Type quantifiers: k_ex609289_ : Nat, k_ex609288_ : Nat, 0 ≤ k_ex609288_ ∧
  k_ex609288_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex609289_ ∧ k_ex609289_ ≤ (2 ^ 256 - 1) -/
def mem_store_byte (off : Nat) (w : Nat) : SailM Unit := do
  let value := (word_low_byte w)
  (mem_set_byte off value)

/-- `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. -/
/- Type quantifiers: k_ex609292_ : Nat, k_ex609291_ : Nat, k_ex609290_ : Nat, 0 ≤ k_ex609290_ ∧
  k_ex609290_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex609291_ ∧ k_ex609291_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex609292_ ∧ k_ex609292_ ≤ (2 ^ 32 - 1) -/
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

