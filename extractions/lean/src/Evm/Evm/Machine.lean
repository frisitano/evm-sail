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
(μ_s), and the frame-memory height (μ_i) are carried by value from the
interpreter loop into each opcode handler and returned updated. No
handler reads or writes these registers: the canonical loop supplies each
step's arguments from the registers and assigns the returned state back,
while frame-boundary code — frame save/suspend ([suspend_frame][]),
resume ([resume_frame][]), and the transaction wrapper — synchronizes
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

/-- Computes the signed state gas consumed by the current frame. -/
/- Type quantifiers: k_ex550041_ : Nat, k_ex550040_ : Nat, k_ex550039_ : Nat, 0 ≤ k_ex550039_ ∧
  k_ex550039_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex550040_ ∧ k_ex550040_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex550041_ ∧ k_ex550041_ ≤ (2 ^ 24) -/
def frame_state_gas_used (state_gas_reservoir : Nat) (state_gas_remaining : Nat) (state_gas_spilled : Nat) : Int :=
  let entry := state_gas_reservoir
  let remaining := state_gas_remaining
  let spilled := state_gas_spilled
  ((entry -i remaining) +i spilled)

/- Type quantifiers: state_gas_reservoir : Nat, state_gas_spilled : Nat, state_gas_remaining : Nat, 0
  ≤ state_gas_remaining ∧ state_gas_remaining ≤ (2 ^ 64 - 1), 0 ≤ state_gas_spilled ∧
  state_gas_spilled ≤ (2 ^ 24), 0 ≤ state_gas_reservoir ∧ state_gas_reservoir ≤ (2 ^ 64 - 1) -/
def exceptional_state (state_gas_remaining : Nat) (state_gas_spilled : Nat) (state_gas_reservoir : Nat) (k : ExceptionKind) : SailM ExceptionalStateTransition := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (pure { state_gas_remaining := state_gas_reservoir,
            state_gas_spilled := STATE_GAS_SPILL_ZERO,
            status := (Exceptional k) })
  else
    (pure { state_gas_remaining := state_gas_remaining,
            state_gas_spilled := state_gas_spilled,
            status := (Exceptional k) })

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

/-- Classifies the carried cursor against one instruction's input and output
stack requirements. -/
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
/- Type quantifiers: k_ex550047_ : Nat, 0 ≤ k_ex550047_ ∧ k_ex550047_ ≤ (2 ^ 256 - 1) -/
def write_stack_word (sp : StackPointer) (value : Nat) : SailM Unit := do
  (stack_slot_write sp 0 value)

/-- Overwrites the `n`-th-from-top operand (`SWAP`); the cursor is
unchanged. -/
/- Type quantifiers: k_ex550049_ : Nat, k_ex550048_ : Nat, 0 ≤ k_ex550048_ ∧
  k_ex550048_ ≤ 1023, 0 ≤ k_ex550049_ ∧ k_ex550049_ ≤ (2 ^ 256 - 1) -/
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
/- Type quantifiers: k_ex550086_ : Nat, k_ex550085_ : Nat, returndata_dependentWitness1 : Nat, returndata_dependentWitness0
  : Nat, 0 ≤ returndata_dependentWitness0 ∧
  0 ≤ returndata_dependentWitness1 ∧
  (returndata_dependentWitness0 + returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex550085_
  ∧ k_ex550085_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550086_ ∧ k_ex550086_ ≤ (2 ^ 32 - 1) -/
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

/-- Returns the carried frame's exact byte high-water mark. -/
/- Type quantifiers: height : Nat, 0 ≤ height ∧ height ≤ (2 ^ 32 - 1) -/
def memory_high_water (height : Nat) : Nat :=
  height

/-- The empty EVM-memory high-water mark. -/
def MEMORY_HEIGHT_ZERO : memory_height := 0

/-- The top-level frame begins at the shared arena's semantic offset zero. -/
def MEMORY_BASE_ZERO : memory_base := 0

/-- Converts a frame-relative coordinate to an absolute arena coordinate. -/
/- Type quantifiers: k_ex550098_ : Nat, k_ex550097_ : Nat, 0 ≤ k_ex550097_ ∧
  k_ex550097_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550098_ ∧ k_ex550098_ ≤ (2 ^ 32 - 1) -/
def memory_absolute (base : Nat) (relative : Nat) : SailM Nat := do
  if ((relative ≤b (((2 ^i 32) - 1) - base)) : Bool)
  then (pure (base + relative))
  else (fatal_error ExecutionInvalid)

/-- Restores the parent arena cursor from the current child cursor and the
parent's frame-scoped memory height. -/
/- Type quantifiers: k_ex550100_ : Nat, k_ex550099_ : Nat, 0 ≤ k_ex550099_ ∧
  k_ex550099_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550100_ ∧ k_ex550100_ ≤ (2 ^ 32 - 1) -/
def memory_parent_base (child_base : Nat) (parent_height : Nat) : SailM Nat := do
  if ((parent_height ≤b child_base) : Bool)
  then (pure (child_base - parent_height))
  else (fatal_error ExecutionInvalid)

/-- Materializes an already-charged memory high-water mark and returns its
updated scalar height. -/
/- Type quantifiers: k_ex550103_ : Nat, k_ex550102_ : Nat, k_ex550101_ : Nat, 0 ≤ k_ex550101_ ∧
  k_ex550101_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550102_ ∧ k_ex550102_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550103_ ∧ k_ex550103_ ≤ (2 ^ 32 - 1) -/
def expand_memory (base : Nat) (height : Nat) (requested_height : Nat) : SailM Nat := do
  if ((requested_height ≤b (((2 ^i 32) - 1) - base)) : Bool)
  then
    (do
      if ((height <b requested_height) : Bool)
      then
        (do
          (mem_expand base height requested_height)
          (pure requested_height))
      else (pure height))
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: mem : Nat, base : Nat, off : Nat, len : Nat, (memory_valid_range off len), 0
  ≤ base ∧ base ≤ (2 ^ 32 - 1), 0 ≤ mem ∧ mem ≤ (2 ^ 32 - 1) -/
def active_memory_slice (base : Nat) (mem : Nat) (off : Nat) (len : Nat) : SailM (Sigma fun
  (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len))) := do
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure ((⟨_, ⟨_, EMPTY_EVM_MEMORY_SLICE⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len)))) : (Sigma fun
      (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len)))))
  else
    (do
      if _sailIf1 : (((mem ≤b (((2 ^i 32) - 1) - base)) && ((off + len) ≤b mem)) : Bool) = true
      then
        (do
          let ⟨_, ⟨_, window⟩⟩ ← do (mem_view base mem (off + len))
          (pure ((⟨_, ⟨_, (memory_sub_slice window off len)⟩⟩ : (Sigma fun (k_syn_off : Nat)
            => (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len)))) : (Sigma
            fun (k_syn_off : Nat) =>
            (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len))))))
      else
        (do
          (fatal_error ExecutionInvalid)))

/- Type quantifiers: mem : Nat, base : Nat, off : Nat, len : Nat, (memory_valid_range off len), 0
  ≤ base ∧ base ≤ (2 ^ 32 - 1), 0 ≤ mem ∧ mem ≤ (2 ^ 32 - 1) -/
def memory_code_slice (base : Nat) (mem : Nat) (off : Nat) (len : Nat) : SailM (Sigma fun
  (k_syn_off : Nat) => (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len))) := do
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure ((⟨_, ⟨_, ((EMPTY_CODE_SLICE).2).2⟩⟩ : (Sigma fun (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len)))) : (Sigma fun
      (k_syn_off : Nat) =>
      (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len)))))
  else
    (do
      if _sailIf1 : (((mem ≤b (((2 ^i 32) - 1) - base)) && ((off + len) ≤b mem)) : Bool) = true
      then
        (do
          let ⟨_, ⟨_, window⟩⟩ ← do (mem_view base mem (off + len))
          let initcode := (memory_sub_slice window off len)
          (code_db_intern_memory ⟨_, ⟨_, initcode⟩⟩))
      else
        (do
          (fatal_error ExecutionInvalid)))

/-- Captures parent execution state and enters child stack and memory frames.
The caller has published its carried machine state to the frame
registers: this checkpoint reads them at the one authoritative
boundary. The host operand-frame cursor mirrors `call_depth`: push the
empty child operand stack before installing the child's semantic depth,
and pop it before restoring the parent's semantic depth. -/
/- Type quantifiers: frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0 : Nat, k_ex550141_
  : Int, k_ex550140_ : Nat, k_ex550139_ : Nat, k_ex550138_ : Nat, k_ex550137_ : Nat, k_ex550136_ :
  Nat, k_ex550135_ : Nat, 0 ≤ k_ex550135_ ∧ k_ex550135_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550136_ ∧
  k_ex550136_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex550137_ ∧ k_ex550137_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550138_ ∧ k_ex550138_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550139_ ∧ k_ex550139_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex550140_ ∧ k_ex550140_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex550141_ ∧
  k_ex550141_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def suspend_frame (pc : Nat) (gas_remaining : Nat) (stack_top : StackPointer) (memory_base : Nat) (memory_height : Nat) (state_gas_remaining : Nat) (state_gas_spilled : Nat) (frame_refund : Int) (frame_status : FrameStatus) (message : Message) (frame_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (calldata : CalldataSlice) : SailM (FrameCheckpoint × StackPointer × Nat × Nat) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  (k_journal_checkpoint ())
  let child_stack ← do (operand_stack_push_empty_frame ())
  let child_memory_base ← do (memory_absolute memory_base memory_height)
  let child_memory_height := MEMORY_HEIGHT_ZERO
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
      memory_height := memory_height }
  (pure (checkpoint, child_stack, child_memory_base, child_memory_height))

/-- Writes one memory byte and raises the high-water mark. -/
/- Type quantifiers: k_ex550147_ : Nat, k_ex550146_ : Nat, 0 ≤ k_ex550146_ ∧
  k_ex550146_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550147_ ∧ k_ex550147_ ≤ (2 ^ 32 - 1) -/
def mem_set_byte (base : Nat) (off : Nat) (v : (BitVec 8)) : SailM Unit := do
  let absolute_offset ← do (memory_absolute base off)
  (mem_write_byte absolute_offset v)

/-- `MLOAD`: the big-endian word at `off`. -/
/- Type quantifiers: k_ex550149_ : Nat, k_ex550148_ : Nat, 0 ≤ k_ex550148_ ∧
  k_ex550148_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550149_ ∧ k_ex550149_ ≤ (2 ^ 32 - 1) -/
def mem_load (base : Nat) (off : Nat) : SailM Nat := do
  let absolute_offset ← do (memory_absolute base off)
  (mem_load_word absolute_offset)

/-- `MSTORE`: writes the big-endian word at `off` and raises the
high-water mark. -/
/- Type quantifiers: k_ex550152_ : Nat, k_ex550151_ : Nat, k_ex550150_ : Nat, 0 ≤ k_ex550150_ ∧
  k_ex550150_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550151_ ∧ k_ex550151_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550152_ ∧ k_ex550152_ ≤ (2 ^ 256 - 1) -/
def mem_store (base : Nat) (off : Nat) (w : Nat) : SailM Unit := do
  let absolute_offset ← do (memory_absolute base off)
  (mem_store_word absolute_offset w)

/-- `MSTORE8`: writes the low byte of `w`. -/
/- Type quantifiers: k_ex550155_ : Nat, k_ex550154_ : Nat, k_ex550153_ : Nat, 0 ≤ k_ex550153_ ∧
  k_ex550153_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550154_ ∧ k_ex550154_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550155_ ∧ k_ex550155_ ≤ (2 ^ 256 - 1) -/
def mem_store_byte (base : Nat) (off : Nat) (w : Nat) : SailM Unit := do
  let value := (word_low_byte w)
  (mem_set_byte base off value)

/-- `MCOPY` (EIP-5656): overlapping-safe memory-to-memory copy. -/
/- Type quantifiers: k_ex550159_ : Nat, k_ex550158_ : Nat, k_ex550157_ : Nat, k_ex550156_ : Nat, 0
  ≤ k_ex550156_ ∧ k_ex550156_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550157_ ∧
  k_ex550157_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550158_ ∧ k_ex550158_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550159_ ∧ k_ex550159_ ≤ (2 ^ 32 - 1) -/
def mem_mcopy (base : Nat) (dst : Nat) (src : Nat) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let absolute_dst ← do (memory_absolute base dst)
      let absolute_src ← do (memory_absolute base src)
      (mem_move absolute_dst absolute_src len))
  else (pure ())

/-- `KECCAK256` over the already-expanded memory range `[off, off+len)`. -/
/- Type quantifiers: range_dependentWitness1 : Nat, range_dependentWitness0 : Nat, k_ex550163_ : Nat, k_ex550162_
  : Nat, 0 ≤ k_ex550162_ ∧ k_ex550162_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550163_ ∧
  k_ex550163_ ≤ (2 ^ 32 - 1), 0 ≤ range_dependentWitness0 ∧
  0 ≤ range_dependentWitness1 ∧
  (range_dependentWitness0 + range_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def mem_keccak (base : Nat) (mem : Nat) (range : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : SailM Nat := do
  let range_dependentWitness0 := (range).1
  let range_dependentWitness1 := ((range).2).1
  let range := ((range).2).2
  let ⟨_, ⟨_, bytes⟩⟩ ← do (active_memory_slice base mem range.off range.len)
  let digest ← do (memory_keccak256 ⟨_, ⟨_, bytes⟩⟩)
  (pure (hash_to_word digest))

