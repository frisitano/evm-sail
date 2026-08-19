import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Host.Stack
import Evm.Host.Output
import Evm.Kernel.Environment
import Evm.Kernel.Storage
import Evm.Kernel.Logs
import Evm.Kernel.Accounts
import Evm.Kernel.Code
import Evm.Kernel.Selfdestruct
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Instructions

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

/-! # Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Handlers follow the state-passing convention (YP μ′ = Ξ(μ)): each takes
only the carried values it uses and returns the same values in the same order. Decoded
instruction data remains explicit where required. The registers behind the
state are read and written only at frame boundaries.

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [memory_expansion_cost][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][]. -/

/-- Lightweight result returned by non-terminal opcode handlers. Return and
revert data stay on their specialized paths rather than inflating this
result. -/
def opcode_failed (result : OpcodeOutcome) : Bool :=
  match result with
  | .Continue () => false
  | .Failed _ => true

def opcode_frame_status (result : OpcodeOutcome) : FrameStatus :=
  match result with
  | .Continue () => (Running ())
  | .Failed kind => (Exceptional kind)

/-- The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
`SELFDESTRUCT` act on this account. -/
def self_addr (message : Message) : (Vector (BitVec 8) 20) :=
  message.address

/-- EIP-214 write protection: any state-modifying opcode in a
`STATICCALL` frame halts exceptionally on the carried gas.
State-changing opcodes call this first; `true` means already
halted. -/
/- Type quantifiers: k_ex609903_ : Bool, k_ex609902_ : Nat, 0 ≤ k_ex609902_ ∧
  k_ex609902_ ≤ (2 ^ 64 - 1) -/
def guard_static (g : Nat) (is_static : Bool) : (Nat × OpcodeOutcome) :=
  if (is_static : Bool)
  then (GAS_ZERO, (Failed WriteProtection))
  else (g, (Continue ()))

/-- `JUMP`/`JUMPI` target validity: the destination must be in code range
and land on a `JUMPDEST` (the precomputed valid-destination set,
YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
bytes that happen to equal `0x5b` are not valid. Returns the next
program counter and the carried gas. -/
/- Type quantifiers: k_ex609913_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, k_ex609908_ : Nat, k_ex609907_ : Nat, 0 ≤ k_ex609907_ ∧ k_ex609907_ ≤ (2 ^ 32 - 1), 0
  ≤ k_ex609908_ ∧ k_ex609908_ ≤ (2 ^ 64 - 1), 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex609913_ ∧ k_ex609913_ ≤ (2 ^ 256 - 1) -/
def do_jump (pc_in : Nat) (g : Nat) (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (destination_value : Nat) : SailM (Nat × Nat × OpcodeOutcome) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let code_length := (frame_code_len ⟨_, ⟨_, frame_code⟩⟩)
  if ((destination_value <b code_length) : Bool)
  then
    (do
      let destination : Nat := destination_value
      let valid_destination ← do (frame_jumpdest_valid ⟨_, ⟨_, frame_code⟩⟩ destination)
      if (valid_destination : Bool)
      then (pure (destination, g, (Continue ())))
      else (pure (pc_in, GAS_ZERO, (Failed InvalidJump))))
  else (pure (pc_in, GAS_ZERO, (Failed InvalidJump)))

/-- Establishes an opcode's local stack precondition before gas charging or
any instruction side effect. The caller supplies the opcode's constant
Yellow Paper `(delta, alpha)` values. -/
/- Type quantifiers: k_ex609915_ : Nat, k_ex609914_ : Nat, 0 ≤ k_ex609914_ ∧
  k_ex609914_ ≤ 1024, 0 ≤ k_ex609915_ ∧ k_ex609915_ ≤ 1024 -/
def guard_stack (carried_sp : StackPointer) (inputs : Nat) (outputs : Nat) : OpcodeOutcome :=
  let stack_validation := (validate_stack carried_sp inputs outputs)
  match stack_validation with
  | .StackValid => (Continue ())
  | .StackUnderflowFailure => (Failed StackUnderflow)
  | .StackOverflowFailure => (Failed StackOverflow)

/-- Pops `count` log topics from the stack into its bounded representation. -/
/- Type quantifiers: k_ex609916_ : Nat, 0 ≤ k_ex609916_ ∧ k_ex609916_ ≤ 4 -/
def pop_log_topics (count : Nat) (sp_in : StackPointer) : SailM (LogTopics × StackPointer) := do
  let sp := sp_in
  match count with
  | 0 => (pure ((LogTopics0 ()), sp))
  | 1 =>
    (do
      let t0 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      (pure ((LogTopics1 t0), sp)))
  | 2 =>
    (do
      let t0 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let t1 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      (pure ((LogTopics2 (t0, t1)), sp)))
  | 3 =>
    (do
      let t0 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let t1 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let t2 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      (pure ((LogTopics3 (t0, t1, t2)), sp)))
  | 4 =>
    (do
      let t0 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let t1 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let t2 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let t3 ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      (pure ((LogTopics4 (t0, t1, t2, t3)), sp)))
  | _ => (pure ((LogTopics0 ()), sp))

/-- Implements `ADD`. -/
/- Type quantifiers: k_ex609917_ : Nat, 0 ≤ k_ex609917_ ∧ k_ex609917_ ≤ (2 ^ 64 - 1) -/
def execute_add (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_add a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `MUL`. -/
/- Type quantifiers: k_ex609918_ : Nat, 0 ≤ k_ex609918_ ∧ k_ex609918_ ≤ (2 ^ 64 - 1) -/
def execute_mul (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_mul a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `SUB`. -/
/- Type quantifiers: k_ex609919_ : Nat, 0 ≤ k_ex609919_ ∧ k_ex609919_ ≤ (2 ^ 64 - 1) -/
def execute_sub (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_sub a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements unsigned `DIV`. -/
/- Type quantifiers: k_ex609920_ : Nat, 0 ≤ k_ex609920_ ∧ k_ex609920_ ≤ (2 ^ 64 - 1) -/
def execute_div (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_div a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements signed `SDIV`. -/
/- Type quantifiers: k_ex609921_ : Nat, 0 ≤ k_ex609921_ ∧ k_ex609921_ ≤ (2 ^ 64 - 1) -/
def execute_sdiv (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_sdiv a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements unsigned `MOD`. -/
/- Type quantifiers: k_ex609922_ : Nat, 0 ≤ k_ex609922_ ∧ k_ex609922_ ≤ (2 ^ 64 - 1) -/
def execute_mod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_mod a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements signed `SMOD`. -/
/- Type quantifiers: k_ex609923_ : Nat, 0 ≤ k_ex609923_ ∧ k_ex609923_ ≤ (2 ^ 64 - 1) -/
def execute_smod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_smod a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `ADDMOD`. -/
/- Type quantifiers: k_ex609924_ : Nat, 0 ≤ k_ex609924_ ∧ k_ex609924_ ≤ (2 ^ 64 - 1) -/
def execute_addmod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 3 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_mid)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let n ← do (read_stack_word sp_after)
          let result := (alu_addmod a b n)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `MULMOD`. -/
/- Type quantifiers: k_ex609925_ : Nat, 0 ≤ k_ex609925_ ∧ k_ex609925_ ≤ (2 ^ 64 - 1) -/
def execute_mulmod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 3 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_mid)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let n ← do (read_stack_word sp_after)
          let result := (alu_mulmod a b n)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `EXP`, including exponent-dependent gas. -/
/- Type quantifiers: k_ex609926_ : Nat, 0 ≤ k_ex609926_ ∧ k_ex609926_ ≤ (2 ^ 64 - 1) -/
def execute_exp (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let a ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let e ← do (read_stack_word sp_after)
      let gas_cost := (exp_gas e)
      let (halt, gas_after_charge) := (charge gas_after gas_cost)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let result ← do (alu_exp a e)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `SIGNEXTEND`. -/
/- Type quantifiers: k_ex609927_ : Nat, 0 ≤ k_ex609927_ ∧ k_ex609927_ ≤ (2 ^ 64 - 1) -/
def execute_signextend (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let bi ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let v ← do (read_stack_word sp_after)
          let result := (alu_signextend bi v)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements unsigned `LT`. -/
/- Type quantifiers: k_ex609928_ : Nat, 0 ≤ k_ex609928_ ∧ k_ex609928_ ≤ (2 ^ 64 - 1) -/
def execute_lt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_lt a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements unsigned `GT`. -/
/- Type quantifiers: k_ex609929_ : Nat, 0 ≤ k_ex609929_ ∧ k_ex609929_ ≤ (2 ^ 64 - 1) -/
def execute_gt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_gt a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements signed `SLT`. -/
/- Type quantifiers: k_ex609930_ : Nat, 0 ≤ k_ex609930_ ∧ k_ex609930_ ≤ (2 ^ 64 - 1) -/
def execute_slt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_slt a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements signed `SGT`. -/
/- Type quantifiers: k_ex609931_ : Nat, 0 ≤ k_ex609931_ ∧ k_ex609931_ ≤ (2 ^ 64 - 1) -/
def execute_sgt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_sgt a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `EQ`. -/
/- Type quantifiers: k_ex609932_ : Nat, 0 ≤ k_ex609932_ ∧ k_ex609932_ ≤ (2 ^ 64 - 1) -/
def execute_eq (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_eq a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `ISZERO`. -/
/- Type quantifiers: k_ex609933_ : Nat, 0 ≤ k_ex609933_ ∧ k_ex609933_ ≤ (2 ^ 64 - 1) -/
def execute_iszero (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, carried_sp, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word carried_sp)
          let result := (alu_iszero a)
          (write_stack_word carried_sp result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, carried_sp, (Continue ())))))

/-- Implements bitwise `AND`. -/
/- Type quantifiers: k_ex609934_ : Nat, 0 ≤ k_ex609934_ ∧ k_ex609934_ ≤ (2 ^ 64 - 1) -/
def execute_and (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_and a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements bitwise `OR`. -/
/- Type quantifiers: k_ex609935_ : Nat, 0 ≤ k_ex609935_ ∧ k_ex609935_ ≤ (2 ^ 64 - 1) -/
def execute_or (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_or a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements bitwise `XOR`. -/
/- Type quantifiers: k_ex609936_ : Nat, 0 ≤ k_ex609936_ ∧ k_ex609936_ ≤ (2 ^ 64 - 1) -/
def execute_xor (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let b ← do (read_stack_word sp_after)
          let result := (alu_xor a b)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements bitwise `NOT`. -/
/- Type quantifiers: k_ex609937_ : Nat, 0 ≤ k_ex609937_ ∧ k_ex609937_ ≤ (2 ^ 64 - 1) -/
def execute_not (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, carried_sp, (Failed OutOfGas))))
      else
        (do
          let a ← do (read_stack_word carried_sp)
          let result := (alu_not a)
          (write_stack_word carried_sp result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, carried_sp, (Continue ())))))

/-- Implements `BYTE`. -/
/- Type quantifiers: k_ex609938_ : Nat, 0 ≤ k_ex609938_ ∧ k_ex609938_ ≤ (2 ^ 64 - 1) -/
def execute_byte (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let i ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let x ← do (read_stack_word sp_after)
          let result := (alu_byte i x)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements logical left shift `SHL` (EIP-145). -/
/- Type quantifiers: k_ex609939_ : Nat, 0 ≤ k_ex609939_ ∧ k_ex609939_ ≤ (2 ^ 64 - 1) -/
def execute_shl (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let s ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let v ← do (read_stack_word sp_after)
          let result := (alu_shl s v)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements logical right shift `SHR` (EIP-145). -/
/- Type quantifiers: k_ex609940_ : Nat, 0 ≤ k_ex609940_ ∧ k_ex609940_ ≤ (2 ^ 64 - 1) -/
def execute_shr (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let s ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let v ← do (read_stack_word sp_after)
          let result := (alu_shr s v)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements arithmetic right shift `SAR` (EIP-145). -/
/- Type quantifiers: k_ex609941_ : Nat, 0 ≤ k_ex609941_ ∧ k_ex609941_ ≤ (2 ^ 64 - 1) -/
def execute_sar (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let s ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let v ← do (read_stack_word sp_after)
          let result := (alu_sar s v)
          (write_stack_word sp_after result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements count-leading-zeroes `CLZ` (EIP-7939). -/
/- Type quantifiers: k_ex609942_ : Nat, 0 ≤ k_ex609942_ ∧ k_ex609942_ ≤ (2 ^ 64 - 1) -/
def execute_clz (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, carried_sp, (Failed OutOfGas))))
      else
        (do
          let x ← do (read_stack_word carried_sp)
          let result := (alu_clz x)
          (write_stack_word carried_sp result)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, carried_sp, (Continue ())))))

/-- Implements `KECCAK256` over an expanded memory range. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex609947_
  : Nat, 0 ≤ k_ex609947_ ∧ k_ex609947_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_keccak256 (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 2 1)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let offset_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let length_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let (hashing_halt, gas_after_hashing) := (charge_keccak_gas gas_after length_word)
      if _sailIf1 : (hashing_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_hashing
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let required_size := (memory_required_size offset_word length_word)
          let (expansion_halt, gas_after_expansion) :=
            (charge_memory_expansion gas_after_hashing memory_after required_size)
          if _sailIf2 : (expansion_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_expansion
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory memory_after access.required_size)
              let (digest, mem2) ← do
                (mem_keccak ⟨_, ⟨_, mem1⟩⟩ ⟨_, ⟨_, access.range⟩⟩)
              let sp_after ← (stack_top_advance sp_after 1)
              (write_stack_word sp_after digest)
              let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                (mem2 : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
              let gas_after : Nat := gas_after_expansion
              (pure ((gas_after : Nat), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                  ()) : OpcodeOutcome))))))

/-- Implements `ADDRESS`. -/
/- Type quantifiers: k_ex609952_ : Nat, 0 ≤ k_ex609952_ ∧ k_ex609952_ ≤ (2 ^ 64 - 1) -/
def execute_address (carried_address : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let address_word := (address_to_word carried_address)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after address_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `ORIGIN`. -/
/- Type quantifiers: k_ex609953_ : Nat, 0 ≤ k_ex609953_ ∧ k_ex609953_ ≤ (2 ^ 64 - 1) -/
def execute_origin (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let origin ← do (k_env F_Origin)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after origin)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CALLER`. -/
/- Type quantifiers: k_ex609954_ : Nat, 0 ≤ k_ex609954_ ∧ k_ex609954_ ≤ (2 ^ 64 - 1) -/
def execute_caller (carried_caller : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let caller := (address_to_word carried_caller)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after caller)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CALLVALUE`. -/
/- Type quantifiers: k_ex609956_ : Nat, k_ex609955_ : Nat, 0 ≤ k_ex609955_ ∧
  k_ex609955_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex609956_ ∧ k_ex609956_ ≤ (2 ^ 64 - 1) -/
def execute_callvalue (carried_value : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after carried_value)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `GASPRICE`. -/
/- Type quantifiers: k_ex609957_ : Nat, 0 ≤ k_ex609957_ ∧ k_ex609957_ ≤ (2 ^ 64 - 1) -/
def execute_gasprice (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let gas_price ← do (k_env F_GasPrice)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after gas_price)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CALLDATASIZE`. -/
/- Type quantifiers: k_ex609958_ : Nat, 0 ≤ k_ex609958_ ∧ k_ex609958_ ≤ (2 ^ 64 - 1) -/
def execute_calldatasize (carried_calldata : CalldataSlice) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let input := carried_calldata
          let input_length := (calldata_slice_length input)
          let length_word ← do (word_of_source_byte_count input_length)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after length_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CALLDATALOAD`. -/
/- Type quantifiers: k_ex609959_ : Nat, 0 ≤ k_ex609959_ ∧ k_ex609959_ ≤ (2 ^ 64 - 1) -/
def execute_calldataload (carried_calldata : CalldataSlice) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let offset_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let value ← do (calldata_slice_load_word_offset carried_calldata offset_word)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after value)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CALLDATACOPY`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex609964_
  : Nat, 0 ≤ k_ex609964_ ∧ k_ex609964_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_calldatacopy (carried_calldata : CalldataSlice) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 3 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let destination_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let source_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let length_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let (copy_halt, gas_after_copy) := (charge_copy_gas gas_after_base length_word)
          if _sailIf2 : (copy_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_copy
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let required_size := (memory_required_size destination_word length_word)
              let (expansion_halt, gas_after_expansion) :=
                (charge_memory_expansion gas_after_copy memory_after required_size)
              if _sailIf3 : (expansion_halt : Bool) = true
              then
                (let gas_after : Nat := gas_after_expansion
                (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                  (carried_memory_dependentWitness0 : Nat) =>
                  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                    OutOfGas) : OpcodeOutcome))))
              else
                (do
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let ⟨_, ⟨_, mem1⟩⟩ ← do
                    (expand_memory memory_after access.required_size)
                  let range := access.range
                  (calldata_slice_copy_word_offset carried_calldata range.off source_word range.len)
                  let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                    ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat)
                    =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                    fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
                  let gas_after : Nat := gas_after_expansion
                  (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                    (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                      ()) : OpcodeOutcome)))))))

/-- Implements `CODESIZE`. -/
/- Type quantifiers: k_ex609976_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex609976_ ∧ k_ex609976_ ≤ (2 ^ 64 - 1) -/
def execute_codesize (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let code_length := (frame_code_len ⟨_, ⟨_, carried_code⟩⟩)
          let length_word ← do (word_of_source_byte_count code_length)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after length_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CODECOPY`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex609988_
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, 0 ≤
  carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex609988_ ∧ k_ex609988_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0 ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_codecopy (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 3 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let destination_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let source_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let length_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let (copy_halt, gas_after_copy) := (charge_copy_gas gas_after_base length_word)
          if _sailIf2 : (copy_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_copy
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let required_size := (memory_required_size destination_word length_word)
              let (expansion_halt, gas_after_expansion) :=
                (charge_memory_expansion gas_after_copy memory_after required_size)
              if _sailIf3 : (expansion_halt : Bool) = true
              then
                (let gas_after : Nat := gas_after_expansion
                (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                  (carried_memory_dependentWitness0 : Nat) =>
                  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                    OutOfGas) : OpcodeOutcome))))
              else
                (do
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let ⟨_, ⟨_, mem1⟩⟩ ← do
                    (expand_memory memory_after access.required_size)
                  let range := access.range
                  let code := carried_code
                  let bytes := (code_bytes code)
                  (code_slice_copy_word_offset ⟨_, ⟨_, bytes⟩⟩ range.off source_word
                    range.len)
                  let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                    ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat)
                    =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                    fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
                  let gas_after : Nat := gas_after_expansion
                  (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                    (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                      ()) : OpcodeOutcome)))))))

/-- Implements `BALANCE`, including warm/cold account access. -/
/- Type quantifiers: k_ex609993_ : Nat, 0 ≤ k_ex609993_ ∧ k_ex609993_ ≤ (2 ^ 64 - 1) -/
def execute_balance (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let address_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let a := (word_to_address address_word)
      let warm ← do (k_account_is_warm a)
      let gas_cost ← do (account_cost warm)
      let (halt, gas_after_charge) := (charge gas_after gas_cost)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          (k_account_mark_warm a)
          let balance ← do (k_get_balance a)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after balance)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `SELFBALANCE`. -/
/- Type quantifiers: k_ex609994_ : Nat, 0 ≤ k_ex609994_ ∧ k_ex609994_ ≤ (2 ^ 64 - 1) -/
def execute_selfbalance (carried_address : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_low)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let balance ← do (k_get_balance carried_address)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after balance)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `EXTCODESIZE`, including warm/cold account access. -/
/- Type quantifiers: k_ex609995_ : Nat, 0 ≤ k_ex609995_ ∧ k_ex609995_ ≤ (2 ^ 64 - 1) -/
def execute_extcodesize (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let address_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let a := (word_to_address address_word)
      let warm ← do (k_account_is_warm a)
      let access_cost ← do (account_cost warm)
      let read_cost ← do (external_code_read_cost ())
      let (halt, gas_after_charge) := (charge gas_after (access_cost + read_cost))
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          (k_account_mark_warm a)
          let code_size ← do (k_get_code_size a)
          let size_word ← do (word_of_source_byte_count code_size)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after size_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `EXTCODECOPY`, including access and copy gas. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610000_
  : Nat, 0 ≤ k_ex610000_ ∧ k_ex610000_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_extcodecopy (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 4 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let address_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let a := (word_to_address address_word)
      let destination_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let source_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let length_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let warm ← do (k_account_is_warm a)
      let access_cost ← do (account_cost warm)
      let read_cost ← do (external_code_read_cost ())
      let (access_halt, gas_after_access) := (charge gas_after (access_cost + read_cost))
      if _sailIf1 : (access_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_access
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let (copy_halt, gas_after_copy) := (charge_copy_gas gas_after_access length_word)
          if _sailIf2 : (copy_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_copy
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let required_size := (memory_required_size destination_word length_word)
              let (expansion_halt, gas_after_expansion) :=
                (charge_memory_expansion gas_after_copy memory_after required_size)
              if _sailIf3 : (expansion_halt : Bool) = true
              then
                (let gas_after : Nat := gas_after_expansion
                (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                  (carried_memory_dependentWitness0 : Nat) =>
                  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                    OutOfGas) : OpcodeOutcome))))
              else
                (do
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let ⟨_, ⟨_, mem1⟩⟩ ← do
                    (expand_memory memory_after access.required_size)
                  let range := access.range
                  (k_account_mark_warm a)
                  (k_code_copy a range.off source_word range.len)
                  let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                    ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat)
                    =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                    fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
                  let gas_after : Nat := gas_after_expansion
                  (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                    (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                      ()) : OpcodeOutcome)))))))

/-- Implements `EXTCODEHASH`, including warm/cold account access. -/
/- Type quantifiers: k_ex610005_ : Nat, 0 ≤ k_ex610005_ ∧ k_ex610005_ ≤ (2 ^ 64 - 1) -/
def execute_extcodehash (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let address_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let a := (word_to_address address_word)
      let warm ← do (k_account_is_warm a)
      let gas_cost ← do (account_cost warm)
      let (halt, gas_after_charge) := (charge gas_after gas_cost)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          (k_account_mark_warm a)
          let code_hash ← do (k_get_codehash a)
          let hash_word := (hash_to_word code_hash)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after hash_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `RETURNDATASIZE`. -/
/- Type quantifiers: k_ex610012_ : Nat, carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, 0 ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610012_ ∧ k_ex610012_ ≤ (2 ^ 64 - 1) -/
def execute_returndatasize (carried_returndata : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let return_data_size := (returndata_size ⟨_, ⟨_, carried_returndata⟩⟩)
          let size_word ← do (word_of_source_byte_count return_data_size)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after size_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements bounds-checked `RETURNDATACOPY`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610023_
  : Nat, carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0 : Nat, 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610023_ ∧ k_ex610023_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0 ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_returndatacopy (carried_returndata : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 3 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let destination_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let source_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let length_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let available := (returndata_size ⟨_, ⟨_, carried_returndata⟩⟩)
          if _sailIf2 : ((source_word ≤b available) : Bool) = true
          then
            (do
              let remaining := (returndata_remaining available source_word)
              let bounded_source_offset : Nat := source_word
              if _sailIf3 : ((length_word ≤b remaining) : Bool) = true
              then
                (do
                  let bounded_length : Nat := length_word
                  let (copy_halt, gas_after_copy) := (charge_copy_gas gas_after_base length_word)
                  if _sailIf4 : (copy_halt : Bool) = true
                  then
                    (let gas_after : Nat := gas_after_copy
                    (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                      (carried_memory_dependentWitness0 : Nat) =>
                      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                        OutOfGas) : OpcodeOutcome))))
                  else
                    (do
                      let required_size := (memory_required_size destination_word length_word)
                      let (expansion_halt, gas_after_expansion) :=
                        (charge_memory_expansion gas_after_copy memory_after required_size)
                      if _sailIf5 : (expansion_halt : Bool) = true
                      then
                        (let gas_after : Nat := gas_after_expansion
                        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma
                          fun (carried_memory_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                            OutOfGas) : OpcodeOutcome))))
                      else
                        (do
                          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                            (memory_access destination_word length_word)
                          let ⟨_, ⟨_, mem1⟩⟩ ← do
                            (expand_memory memory_after access.required_size)
                          (returndata_copy ⟨_, ⟨_, carried_returndata⟩⟩ access.range.off
                            bounded_source_offset bounded_length)
                          let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                            ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                            (carried_memory_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                            fun (carried_memory_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
                          let gas_after : Nat := gas_after_expansion
                          (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (carried_memory_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                              ()) : OpcodeOutcome))))))
              else
                (pure ((GAS_ZERO : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                  (carried_memory_dependentWitness0 : Nat) =>
                  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                    InvalidOpcode) : OpcodeOutcome))))
          else
            (pure ((GAS_ZERO : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                InvalidOpcode) : OpcodeOutcome)))))

/-- Implements `BLOCKHASH`. -/
/- Type quantifiers: k_ex610028_ : Nat, 0 ≤ k_ex610028_ ∧ k_ex610028_ ≤ (2 ^ 64 - 1) -/
def execute_blockhash (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after 20)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let block_number ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let block_hash ← do (k_blockhash block_number)
          let hash_word := (hash_to_word block_hash)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after hash_word)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `COINBASE`. -/
/- Type quantifiers: k_ex610029_ : Nat, 0 ≤ k_ex610029_ ∧ k_ex610029_ ≤ (2 ^ 64 - 1) -/
def execute_coinbase (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let coinbase ← do (k_env F_Coinbase)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after coinbase)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `TIMESTAMP`. -/
/- Type quantifiers: k_ex610030_ : Nat, 0 ≤ k_ex610030_ ∧ k_ex610030_ ≤ (2 ^ 64 - 1) -/
def execute_timestamp (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let timestamp ← do (k_env F_Timestamp)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after timestamp)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `NUMBER`. -/
/- Type quantifiers: k_ex610031_ : Nat, 0 ≤ k_ex610031_ ∧ k_ex610031_ ≤ (2 ^ 64 - 1) -/
def execute_number (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let block_number ← do (k_env F_Number)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after block_number)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `SLOTNUM`. -/
/- Type quantifiers: k_ex610032_ : Nat, 0 ≤ k_ex610032_ ∧ k_ex610032_ ≤ (2 ^ 64 - 1) -/
def execute_slotnum (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let slot_number ← do (k_env F_SlotNumber)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after slot_number)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `PREVRANDAO`. -/
/- Type quantifiers: k_ex610033_ : Nat, 0 ≤ k_ex610033_ ∧ k_ex610033_ ≤ (2 ^ 64 - 1) -/
def execute_prevrandao (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let prev_randao ← do (k_env F_PrevRandao)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after prev_randao)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `GASLIMIT`. -/
/- Type quantifiers: k_ex610034_ : Nat, 0 ≤ k_ex610034_ ∧ k_ex610034_ ≤ (2 ^ 64 - 1) -/
def execute_gaslimit (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let gas_limit ← do (k_env F_GasLimit)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after gas_limit)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `CHAINID`. -/
/- Type quantifiers: k_ex610035_ : Nat, 0 ≤ k_ex610035_ ∧ k_ex610035_ ≤ (2 ^ 64 - 1) -/
def execute_chainid (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let chain_id ← do (k_env F_ChainId)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after chain_id)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `BASEFEE`. -/
/- Type quantifiers: k_ex610036_ : Nat, 0 ≤ k_ex610036_ ∧ k_ex610036_ ≤ (2 ^ 64 - 1) -/
def execute_basefee (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let base_fee ← do (k_env F_BaseFee)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after base_fee)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `BLOBBASEFEE`. -/
/- Type quantifiers: k_ex610038_ : Nat, k_ex610037_ : Nat, 0 ≤ k_ex610037_ ∧
  k_ex610037_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex610038_ ∧ k_ex610038_ ≤ (2 ^ 64 - 1) -/
def execute_blobbasefee (blob_fee : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after blob_fee)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `BLOBHASH`. -/
/- Type quantifiers: k_ex610039_ : Nat, 0 ≤ k_ex610039_ ∧ k_ex610039_ ≤ (2 ^ 64 - 1) -/
def execute_blobhash (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let index ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let blob_hash ← do (k_blobhash index)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after blob_hash)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `POP`. -/
/- Type quantifiers: k_ex610040_ : Nat, 0 ≤ k_ex610040_ ∧ k_ex610040_ ≤ (2 ^ 64 - 1) -/
def execute_pop (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 0)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let _ ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `MLOAD`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610045_
  : Nat, 0 ≤ k_ex610045_ ∧ k_ex610045_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_mload (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let offset_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let word_size := (u256 32)
          let required_size := (memory_required_size offset_word word_size)
          let (expansion_halt, gas_after_expansion) :=
            (charge_memory_expansion gas_after_base memory_after required_size)
          if _sailIf2 : (expansion_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_expansion
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word word_size)
              let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory memory_after access.required_size)
              let value ← do (mem_load access.range.off)
              let sp_after ← (stack_top_advance sp_after 1)
              (write_stack_word sp_after value)
              let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
              let gas_after : Nat := gas_after_expansion
              (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                  ()) : OpcodeOutcome))))))

/-- Implements `MSTORE`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610054_
  : Nat, 0 ≤ k_ex610054_ ∧ k_ex610054_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_mstore (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let offset_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let v ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let word_size := (u256 32)
          let required_size := (memory_required_size offset_word word_size)
          let (expansion_halt, gas_after_expansion) :=
            (charge_memory_expansion gas_after_base memory_after required_size)
          if _sailIf2 : (expansion_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_expansion
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word word_size)
              let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory memory_after access.required_size)
              (mem_store access.range.off v)
              let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
              let gas_after : Nat := gas_after_expansion
              (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                  ()) : OpcodeOutcome))))))

/-- Implements `MSTORE8`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610063_
  : Nat, 0 ≤ k_ex610063_ ∧ k_ex610063_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_mstore8 (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let offset_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let v ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let required_size := (memory_required_size offset_word WORD_ONE)
          let (expansion_halt, gas_after_expansion) :=
            (charge_memory_expansion gas_after_base memory_after required_size)
          if _sailIf2 : (expansion_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_expansion
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word WORD_ONE)
              let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory memory_after access.required_size)
              (mem_store_byte access.range.off v)
              let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                fun (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
              let gas_after : Nat := gas_after_expansion
              (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                (carried_memory_dependentWitness0 : Nat) =>
                (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                  ()) : OpcodeOutcome))))))

/-- Implements `MSIZE`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610072_
  : Nat, 0 ≤ k_ex610072_ ∧ k_ex610072_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_msize (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if _sailIf1 : (halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_charge
        (pure ((gas_after : Nat), (sp_after : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
          fun (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
          fun (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let high_water := (memory_high_water ⟨_, ⟨_, carried_memory⟩⟩)
          let words := (memory_word_count high_water)
          let size ← do (word_of_nat_byte_count (words *i 32))
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after size)
          let gas_after : Nat := gas_after_charge
          (pure ((gas_after : Nat), (sp_after : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
            fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
            fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
              ()) : OpcodeOutcome)))))

/-- Implements overlapping memory copy `MCOPY` (EIP-5656). -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610081_
  : Nat, 0 ≤ k_ex610081_ ∧ k_ex610081_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_mcopy (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 3 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (base_halt, gas_after_base) := (charge gas_after G_verylow)
      if _sailIf1 : (base_halt : Bool) = true
      then
        (let gas_after : Nat := gas_after_base
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
            OutOfGas) : OpcodeOutcome))))
      else
        (do
          let destination_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let source_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let length_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let (copy_halt, gas_after_copy) := (charge_copy_gas gas_after_base length_word)
          if _sailIf2 : (copy_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_copy
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let destination_required := (memory_required_size destination_word length_word)
              let source_required := (memory_required_size source_word length_word)
              let required_size :=
                if ((destination_required <b source_required) : Bool)
                then source_required
                else destination_required
              let (expansion_halt, gas_after_expansion) :=
                (charge_memory_expansion gas_after_copy memory_after required_size)
              if _sailIf3 : (expansion_halt : Bool) = true
              then
                (let gas_after : Nat := gas_after_expansion
                (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                  (carried_memory_dependentWitness0 : Nat) =>
                  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                    OutOfGas) : OpcodeOutcome))))
              else
                (do
                  let ⟨_, ⟨_, ⟨_, destination⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let ⟨_, ⟨_, ⟨_, source⟩⟩⟩ ← do
                    (memory_access source_word length_word)
                  let materialized_required_size :=
                    if ((destination.required_size <b source.required_size) : Bool)
                    then source.required_size
                    else destination.required_size
                  let ⟨_, ⟨_, mem1⟩⟩ ← do
                    (expand_memory memory_after materialized_required_size)
                  (mem_mcopy destination.range.off source.range.off destination.range.len)
                  let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                    ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat)
                    =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                    fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
                  let gas_after : Nat := gas_after_expansion
                  (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                    (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                      ()) : OpcodeOutcome)))))))

def undefined_AccountExecutionContext (_ : Unit) : SailM AccountExecutionContext := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))) })

def account_execution_context (address : (Vector (BitVec 8) 20)) : AccountExecutionContext :=
  { address := address }

def refresh_account_execution_context (context : AccountExecutionContext) (previous_address : (Vector (BitVec 8) 20)) (next_address : (Vector (BitVec 8) 20)) : AccountExecutionContext :=
  if ((previous_address == next_address) : Bool)
  then context
  else (account_execution_context next_address)

/-- Implements `SLOAD`, including warm/cold access gas. -/
/- Type quantifiers: k_ex610086_ : Nat, 0 ≤ k_ex610086_ ∧ k_ex610086_ ≤ (2 ^ 64 - 1) -/
def execute_sload (context : AccountExecutionContext) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let s ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let warm ← do (k_slot_is_warm context.address s)
      let gas_cost ← do (sload_cost warm)
      let (halt, gas_after_charge) := (charge gas_after gas_cost)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          (k_slot_mark_warm context.address s)
          let entry ← do (k_sload context.address s)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after entry.curr)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/- Type quantifiers: carried_refund : Int, carried_state_spill : Nat, carried_state_gas : Nat, carried_gas
  : Nat, k_ex610088_ : Bool, fork : Nat, 0 ≤ fork ∧ fork ≤ 16, 0 ≤ carried_gas ∧
  carried_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_gas ∧ carried_state_gas ≤ (2 ^ 64 - 1), 0
  ≤ carried_state_spill ∧ carried_state_spill ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤
  carried_refund ∧ carried_refund ≤ (199 * (2 ^ 64 - 1)) -/
def execute_sstore (context : AccountExecutionContext) (fork : Nat) (carried_is_static : Bool) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) : SailM (Nat × Nat × Nat × Int × StackPointer × OpcodeOutcome) := SailME.run do
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then
    (pure (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, stack_status))
  else
    (do
      let (gas_after_guard, guard_result) := (guard_static carried_gas carried_is_static)
      let guard_failed := (opcode_failed guard_result)
      if (guard_failed : Bool)
      then
        (pure (gas_after_guard, carried_state_gas, carried_state_spill, carried_refund, carried_sp, guard_result))
      else
        (do
          if (((fork <b Amsterdam) && (carried_gas ≤b G_callstipend)) : Bool)
          then
            (pure (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, (Failed
                OutOfGas)))
          else
            (do
              let s ← do (read_stack_word carried_sp)
              let sp_after_slot ← do (stack_top_retreat carried_sp 1)
              let v ← do (read_stack_word sp_after_slot)
              let sp_after ← do (stack_top_retreat sp_after_slot 1)
              let warm ← do (k_slot_is_warm context.address s)
              let cold := (! warm)
              if ((fork ≥b Amsterdam) : Bool)
              then
                (do
                  let sentry_cost := (sstore_sentry_cost cold)
                  let (sentry_halt, _) := (check_execution_gas carried_gas sentry_cost)
                  if (sentry_halt : Bool)
                  then
                    SailME.throw ((carried_gas, carried_state_gas, carried_state_spill, carried_refund, sp_after, (Failed
                        OutOfGas)) : (Nat × Nat × Nat × Int × StackPointer × OpcodeOutcome))
                  else (pure ()))
              else (pure ())
              (k_slot_mark_warm context.address s)
              let entry ← do (k_sload context.address s)
              let costs ← do (sstore_costs entry.orig entry.curr v cold)
              let (gas_after_refund_credit, state_gas_after_credit, state_spill_after_credit) ← do
                if ((costs.state_credit != 0) : Bool)
                then
                  (credit_state_gas_refund carried_gas carried_state_gas carried_state_spill
                    costs.state_credit)
                else (pure (carried_gas, carried_state_gas, carried_state_spill))
              let (execution_halt, gas_after_execution) :=
                (charge gas_after_refund_credit costs.execution)
              if (execution_halt : Bool)
              then
                (pure (gas_after_execution, state_gas_after_credit, state_spill_after_credit, carried_refund, sp_after, (Failed
                    OutOfGas)))
              else
                (do
                  let (state_halt, gas_after_state_charge, state_gas_after_charge, state_spill_after_charge) ← do
                    (charge_state_gas gas_after_execution state_gas_after_credit
                      state_spill_after_credit costs.state_charge)
                  if (state_halt : Bool)
                  then
                    (pure (gas_after_state_charge, state_gas_after_charge, state_spill_after_charge, carried_refund, sp_after, (Failed
                        OutOfGas)))
                  else
                    (do
                      let refund_after ← do
                        if ((! (costs.refund == GAS_REFUND_ZERO)) : Bool)
                        then (record_refund carried_refund costs.refund)
                        else (pure carried_refund)
                      if ((entry.curr != v) : Bool)
                      then
                        (k_sstore context.address s
                          { curr := v,
                            orig := entry.orig })
                      else (pure ())
                      (pure (gas_after_state_charge, state_gas_after_charge, state_spill_after_charge, refund_after, sp_after, (Continue
                          ()))))))))

/-- Implements transient-storage load `TLOAD` (EIP-1153). -/
/- Type quantifiers: k_ex610093_ : Nat, 0 ≤ k_ex610093_ ∧ k_ex610093_ ≤ (2 ^ 64 - 1) -/
def execute_tload (carried_address : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_warm_access)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let s ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let value ← do (k_tload carried_address s)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after value)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements transient-storage write `TSTORE` (EIP-1153). -/
/- Type quantifiers: k_ex610095_ : Nat, k_ex610094_ : Bool, 0 ≤ k_ex610095_ ∧
  k_ex610095_ ≤ (2 ^ 64 - 1) -/
def execute_tstore (carried_address : (Vector (BitVec 8) 20)) (carried_is_static : Bool) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (gas_after_static_guard, status_after_guard) := (guard_static gas_after carried_is_static)
      let guard_failed := (opcode_failed status_after_guard)
      if (guard_failed : Bool)
      then
        (let gas_after : Nat := gas_after_static_guard
        (pure (gas_after, sp_after, status_after_guard)))
      else
        (do
          let (halt, gas_after_charge) := (charge gas_after_static_guard G_warm_access)
          if (halt : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (gas_after, sp_after, (Failed OutOfGas))))
          else
            (do
              let s ← do (read_stack_word sp_after)
              let sp_after ← (stack_top_retreat sp_after 1)
              let v ← do (read_stack_word sp_after)
              let sp_after ← (stack_top_retreat sp_after 1)
              (k_tstore carried_address s v)
              let gas_after : Nat := gas_after_charge
              (pure (gas_after, sp_after, (Continue ()))))))

/-- Implements unconditional `JUMP`. -/
/- Type quantifiers: k_ex610104_ : Nat, k_ex610103_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610103_ ∧ k_ex610103_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex610104_ ∧
  k_ex610104_ ≤ (2 ^ 64 - 1) -/
def execute_jump (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_pc : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 1 0)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (carried_pc, GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let pc_after : Nat := carried_pc
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_mid)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (pc_after, gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let dest ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let (next_pc, g2, jump_status) ← do
            (do_jump pc_after gas_after_charge ⟨_, ⟨_, carried_code⟩⟩ dest)
          let pc_after : Nat := next_pc
          let gas_after : Nat := g2
          (pure (pc_after, gas_after, sp_after, jump_status))))

/-- Implements conditional `JUMPI`. -/
/- Type quantifiers: k_ex610113_ : Nat, k_ex610112_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610112_ ∧ k_ex610112_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex610113_ ∧
  k_ex610113_ ≤ (2 ^ 64 - 1) -/
def execute_jumpi (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_pc : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (carried_pc, GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let pc_after : Nat := carried_pc
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_high)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (pc_after, gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let dest ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let cond ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let condition_is_zero := (word_is_zero cond)
          if (condition_is_zero : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (pc_after, gas_after, sp_after, (Continue ()))))
          else
            (do
              let (next_pc, g2, jump_status) ← do
                (do_jump pc_after gas_after_charge ⟨_, ⟨_, carried_code⟩⟩ dest)
              let pc_after : Nat := next_pc
              let gas_after : Nat := g2
              (pure (pc_after, gas_after, sp_after, jump_status)))))

/-- Implements `PC`, returning the current opcode position from the
carried, already-advanced program counter. -/
/- Type quantifiers: k_ex610115_ : Nat, k_ex610114_ : Nat, 0 ≤ k_ex610114_ ∧
  k_ex610114_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex610115_ ∧ k_ex610115_ ≤ (2 ^ 64 - 1) -/
def execute_pc (carried_pc : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (carried_pc, GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (carried_pc, gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let next_pc ← do (word_of_source_byte_count carried_pc)
          let opcode_pc := (alu_sub next_pc WORD_ONE)
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after opcode_pc)
          let gas_after : Nat := gas_after_charge
          (pure (carried_pc, gas_after, sp_after, (Continue ())))))

/-- Implements `GAS`, returning the carried gas remaining after its own
charge. -/
/- Type quantifiers: k_ex610116_ : Nat, 0 ≤ k_ex610116_ ∧ k_ex610116_ ≤ (2 ^ 64 - 1) -/
def execute_gas (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_base)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after (← (word_of_nat_byte_count gas_after_charge)))
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements `JUMPDEST`. -/
/- Type quantifiers: carried_gas : Nat, 0 ≤ carried_gas ∧ carried_gas ≤ (2 ^ 64 - 1) -/
def execute_jumpdest (carried_gas : Nat) : (Nat × OpcodeOutcome) :=
  let gas_after : Nat := carried_gas
  let (halt, next_gas) := (charge gas_after G_jumpdest)
  if (halt : Bool)
  then
    (let gas_after : Nat := next_gas
    (gas_after, (Failed OutOfGas)))
  else
    (let gas_after : Nat := next_gas
    (gas_after, (Continue ())))

/-- Implements the `PUSH0` through `PUSH32` family. -/
/- Type quantifiers: k_ex610120_ : Nat, k_ex610119_ : Nat, k_ex610118_ : Nat, 0 ≤ k_ex610118_ ∧
  k_ex610118_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610119_ ∧ k_ex610119_ ≤ 32, 0 ≤ k_ex610120_ ∧
  k_ex610120_ ≤ (2 ^ 256 - 1) -/
def execute_push (carried_gas : Nat) (carried_sp : StackPointer) (n : Nat) (v : Nat) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      if ((n == 0) : Bool)
      then
        (do
          let (halt, gas_after_charge) := (charge gas_after G_base)
          if (halt : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (gas_after, sp_after, (Failed OutOfGas))))
          else
            (do
              let sp_after ← (stack_top_advance sp_after 1)
              (write_stack_word sp_after v)
              let gas_after : Nat := gas_after_charge
              (pure (gas_after, sp_after, (Continue ())))))
      else
        (do
          let (halt, gas_after_charge) := (charge gas_after G_verylow)
          if (halt : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (gas_after, sp_after, (Failed OutOfGas))))
          else
            (do
              let sp_after ← (stack_top_advance sp_after 1)
              (write_stack_word sp_after v)
              let gas_after : Nat := gas_after_charge
              (pure (gas_after, sp_after, (Continue ()))))))

/-- Implements the `DUP1` through `DUP16` family. -/
/- Type quantifiers: k_ex610122_ : Nat, k_ex610121_ : Nat, 0 ≤ k_ex610121_ ∧
  k_ex610121_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex610122_ ∧ k_ex610122_ ≤ 16 -/
def execute_dup (carried_gas : Nat) (carried_sp : StackPointer) (n : Nat) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp n (n + 1))
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, sp_after, (Failed OutOfGas))))
      else
        (do
          let value ← do (stack_slot_read sp_after (n - 1))
          let sp_after ← (stack_top_advance sp_after 1)
          (write_stack_word sp_after value)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, sp_after, (Continue ())))))

/-- Implements the `SWAP1` through `SWAP16` family. -/
/- Type quantifiers: k_ex610124_ : Nat, k_ex610123_ : Nat, 0 ≤ k_ex610123_ ∧
  k_ex610123_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex610124_ ∧ k_ex610124_ ≤ 16 -/
def execute_swap (carried_gas : Nat) (carried_sp : StackPointer) (n : Nat) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp (n + 1) (n + 1))
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas_after : Nat := carried_gas
      let (halt, gas_after_charge) := (charge gas_after G_verylow)
      if (halt : Bool)
      then
        (let gas_after : Nat := gas_after_charge
        (pure (gas_after, carried_sp, (Failed OutOfGas))))
      else
        (do
          let top_value ← do (read_stack_word carried_sp)
          let other ← do (stack_slot_read carried_sp n)
          (stack_set carried_sp 0 other)
          (stack_set carried_sp n top_value)
          let gas_after : Nat := gas_after_charge
          (pure (gas_after, carried_sp, (Continue ())))))

/-- Implements immediate deep-stack duplication `DUPN`. -/
/- Type quantifiers: k_ex610125_ : Nat, 0 ≤ k_ex610125_ ∧ k_ex610125_ ≤ (2 ^ 64 - 1) -/
def execute_dupn (carried_gas : Nat) (carried_sp : StackPointer) (immediate : (BitVec 8)) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let gas_after : Nat := carried_gas
  let sp_after : StackPointer := carried_sp
  let valid_immediate := (deep_stack_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (gas_after, sp_after, (Failed InvalidOpcode)))
  else
    (do
      let n ← do (decode_single_stack_index immediate)
      let stack_status := (guard_stack carried_sp n (n + 1))
      let stack_failed := (opcode_failed stack_status)
      if (stack_failed : Bool)
      then (pure (GAS_ZERO, carried_sp, stack_status))
      else
        (do
          let (halt, gas_after_charge) := (charge gas_after G_verylow)
          if (halt : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (gas_after, sp_after, (Failed OutOfGas))))
          else
            (do
              let value ← do (stack_slot_read sp_after (n - 1))
              let sp_after ← (stack_top_advance sp_after 1)
              (write_stack_word sp_after value)
              let gas_after : Nat := gas_after_charge
              (pure (gas_after, sp_after, (Continue ()))))))

/-- Implements immediate deep-stack exchange `SWAPN`. -/
/- Type quantifiers: k_ex610126_ : Nat, 0 ≤ k_ex610126_ ∧ k_ex610126_ ≤ (2 ^ 64 - 1) -/
def execute_swapn (carried_gas : Nat) (carried_sp : StackPointer) (immediate : (BitVec 8)) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let gas_after : Nat := carried_gas
  let valid_immediate := (deep_stack_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (gas_after, carried_sp, (Failed InvalidOpcode)))
  else
    (do
      let n ← do (decode_single_stack_index immediate)
      let stack_status := (guard_stack carried_sp (n + 1) (n + 1))
      let stack_failed := (opcode_failed stack_status)
      if (stack_failed : Bool)
      then (pure (GAS_ZERO, carried_sp, stack_status))
      else
        (do
          let (halt, gas_after_charge) := (charge gas_after G_verylow)
          if (halt : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (gas_after, carried_sp, (Failed OutOfGas))))
          else
            (do
              let top_value ← do (read_stack_word carried_sp)
              let other ← do (stack_slot_read carried_sp n)
              (stack_set carried_sp 0 other)
              (stack_set carried_sp n top_value)
              let gas_after : Nat := gas_after_charge
              (pure (gas_after, carried_sp, (Continue ()))))))

/-- Implements immediate pairwise deep-stack `EXCHANGE`. -/
/- Type quantifiers: k_ex610127_ : Nat, 0 ≤ k_ex610127_ ∧ k_ex610127_ ≤ (2 ^ 64 - 1) -/
def execute_exchange (carried_gas : Nat) (carried_sp : StackPointer) (immediate : (BitVec 8)) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let gas_after : Nat := carried_gas
  let valid_immediate := (exchange_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (gas_after, carried_sp, (Failed InvalidOpcode)))
  else
    (do
      let (n, m) ← do (decode_exchange_stack_indices immediate)
      let stack_status := (guard_stack carried_sp (m + 1) (m + 1))
      let stack_failed := (opcode_failed stack_status)
      if (stack_failed : Bool)
      then (pure (GAS_ZERO, carried_sp, stack_status))
      else
        (do
          let (halt, gas_after_charge) := (charge gas_after G_verylow)
          if (halt : Bool)
          then
            (let gas_after : Nat := gas_after_charge
            (pure (gas_after, carried_sp, (Failed OutOfGas))))
          else
            (do
              let first ← do (stack_slot_read carried_sp n)
              let second ← do (stack_slot_read carried_sp m)
              (stack_set carried_sp n second)
              (stack_set carried_sp m first)
              let gas_after : Nat := gas_after_charge
              (pure (gas_after, carried_sp, (Continue ()))))))

/-- Implements the `LOG0` through `LOG4` family. -/
/- Type quantifiers: k_ex610138_ : Nat, carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0
  : Nat, k_ex610133_ : Nat, k_ex610132_ : Bool, 0 ≤ k_ex610133_ ∧ k_ex610133_ ≤ (2 ^ 64 - 1), 0
  ≤ carried_memory_dependentWitness0 ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610138_ ∧ k_ex610138_ ≤ 4 -/
def execute_log (carried_address : (Vector (BitVec 8) 20)) (carried_is_static : Bool) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (n : Nat) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × OpcodeOutcome) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp (n + 2) 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (stack_status : OpcodeOutcome)))
  else
    (do
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let (gas_after_static_guard, status_after_guard) := (guard_static gas_after carried_is_static)
      let guard_failed := (opcode_failed status_after_guard)
      if _sailIf1 : (guard_failed : Bool) = true
      then
        (let gas_after : Nat := gas_after_static_guard
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (status_after_guard : OpcodeOutcome))))
      else
        (do
          let offset_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let length_word ← do (read_stack_word sp_after)
          let sp_after ← (stack_top_retreat sp_after 1)
          let (topics, next_sp_1) ← do (pop_log_topics n sp_after)
          let sp_after : StackPointer := next_sp_1
          let (log_halt, gas_after_log) := (charge_log_gas gas_after_static_guard n length_word)
          if _sailIf2 : (log_halt : Bool) = true
          then
            (let gas_after : Nat := gas_after_log
            (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
              (carried_memory_dependentWitness0 : Nat) =>
              (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                OutOfGas) : OpcodeOutcome))))
          else
            (do
              let required_size := (memory_required_size offset_word length_word)
              let (expansion_halt, gas_after_expansion) :=
                (charge_memory_expansion gas_after_log memory_after required_size)
              if _sailIf3 : (expansion_halt : Bool) = true
              then
                (let gas_after : Nat := gas_after_expansion
                (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
                  (carried_memory_dependentWitness0 : Nat) =>
                  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Failed
                    OutOfGas) : OpcodeOutcome))))
              else
                (do
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access offset_word length_word)
                  let ⟨_, ⟨_, mem1⟩⟩ ← do
                    (expand_memory memory_after access.required_size)
                  let range := access.range
                  let (data, mem2) ← do
                    (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ range.off range.len)
                  let memory_slice := (evm_memory_slice ((data).2).2.bytes ((data).2).2.len)
                  let log_data := (LogDataMemory ⟨_, ⟨_, memory_slice⟩⟩)
                  (k_log carried_address topics log_data)
                  let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                    (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
                    (mem2 : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
                  let gas_after : Nat := gas_after_expansion
                  (pure ((gas_after : Nat), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                    fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
                    fun (carried_memory_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((Continue
                      ()) : OpcodeOutcome)))))))

/-- Implements normal `STOP`. -/
def execute_stop (_ : Unit) : FrameStatus :=
  let status_after : FrameStatus := (Running ())
  let reason := (HaltStop ())
  (Halted reason)

/-- Implements successful `RETURN`. -/
/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, k_ex610143_
  : Nat, 0 ≤ k_ex610143_ ∧ k_ex610143_ ≤ (2 ^ 64 - 1), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_return (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × FrameStatus) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((opcode_frame_status
        stack_status) : FrameStatus)))
  else
    (do
      let status_after : FrameStatus := (Running ())
      let gas_after : Nat := carried_gas
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let offset_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let length_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let required_size := (memory_required_size offset_word length_word)
      let (expansion_halt, gas_after_expansion) :=
        (charge_memory_expansion gas_after memory_after required_size)
      if _sailIf1 : (expansion_halt : Bool) = true
      then
        (let status_after : FrameStatus := (Exceptional OutOfGas)
        let gas_after : Nat := gas_after_expansion
        (pure ((gas_after : Nat), (sp_after : StackPointer), (memory_after : (Sigma fun
          (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (status_after : FrameStatus))))
      else
        (do
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
          let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory memory_after access.required_size)
          let range := access.range
          let (data, mem2) ← do (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ range.off range.len)
          let ⟨_, ⟨_, output⟩⟩ ← do (freeze_memory_output data)
          let reason := (HaltReturn ⟨_, ⟨_, output⟩⟩)
          let status_after : FrameStatus := (Halted reason)
          let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat)
            =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
            (mem2 : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
          let gas_after : Nat := gas_after_expansion
          (pure ((gas_after : Nat), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
            fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
            fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (status_after : FrameStatus)))))

/- Type quantifiers: carried_memory_dependentWitness1 : Nat, carried_memory_dependentWitness0 : Nat, carried_state_spill
  : Nat, carried_state_gas : Nat, carried_gas : Nat, carried_state_gas_reservoir : Nat, 0 ≤
  carried_state_gas_reservoir ∧ carried_state_gas_reservoir ≤ (2 ^ 64 - 1), 0 ≤ carried_gas
  ∧ carried_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_gas ∧ carried_state_gas ≤ (2 ^ 64 - 1), 0
  ≤ carried_state_spill ∧ carried_state_spill ≤ (2 ^ 24), 0 ≤
  carried_memory_dependentWitness0 ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_revert (carried_state_gas_reservoir : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Nat × Nat × Nat × StackPointer × (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) × FrameStatus) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let stack_status := (guard_stack carried_sp 2 0)
  let stack_failed := (opcode_failed stack_status)
  if _sailIf0 : (stack_failed : Bool) = true
  then
    (pure ((GAS_ZERO : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
      fun (carried_memory_dependentWitness0 : Nat) =>
      (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), ((opcode_frame_status
        stack_status) : FrameStatus)))
  else
    (do
      let status_after : FrameStatus := (Running ())
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
        fun (carried_memory_dependentWitness0 : Nat) =>
        (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
      let offset_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let length_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let required_size := (memory_required_size offset_word length_word)
      let (expansion_halt, gas_after_expansion) :=
        (charge_memory_expansion carried_gas memory_after required_size)
      if _sailIf1 : (expansion_halt : Bool) = true
      then
        (let status_after : FrameStatus := (Exceptional OutOfGas)
        (pure ((gas_after_expansion : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (sp_after : StackPointer), (memory_after : (Sigma
          fun (carried_memory_dependentWitness0 : Nat) =>
          (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (status_after : FrameStatus))))
      else
        (do
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
          let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory memory_after access.required_size)
          let range := access.range
          let (g2, state_gas_after, state_spill_after) ← do
            (refill_frame_state_gas gas_after_expansion carried_state_gas carried_state_spill
              carried_state_gas_reservoir)
          let (data, mem2) ← do (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ range.off range.len)
          let ⟨_, ⟨_, output⟩⟩ ← do (freeze_memory_output data)
          let reason := (HaltRevert ⟨_, ⟨_, output⟩⟩)
          let status_after : FrameStatus := (Halted reason)
          let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun (carried_memory_dependentWitness0 : Nat)
            =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))) :=
            (mem2 : (Sigma fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1))))
          (pure ((g2 : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
            fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) : (Sigma
            fun (carried_memory_dependentWitness0 : Nat) =>
            (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))), (status_after : FrameStatus)))))

/-- Reports invalid-opcode termination to the interpreter's exceptional-halt
boundary. -/
/- Type quantifiers: carried_gas : Nat, 0 ≤ carried_gas ∧ carried_gas ≤ (2 ^ 64 - 1) -/
def execute_invalid (carried_gas : Nat) : (Nat × OpcodeOutcome) :=
  (carried_gas, (Failed InvalidOpcode))

/- Type quantifiers: carried_refund : Int, carried_state_spill : Nat, carried_state_gas : Nat, carried_gas
  : Nat, k_ex610162_ : Bool, fork : Nat, 0 ≤ fork ∧ fork ≤ 16, 0 ≤ carried_gas ∧
  carried_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_gas ∧ carried_state_gas ≤ (2 ^ 64 - 1), 0
  ≤ carried_state_spill ∧ carried_state_spill ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤
  carried_refund ∧ carried_refund ≤ (199 * (2 ^ 64 - 1)) -/
def execute_selfdestruct (carried_address : (Vector (BitVec 8) 20)) (fork : Nat) (carried_is_static : Bool) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) : SailM (Nat × Nat × Nat × Int × StackPointer × FrameStatus) := do
  let stack_status := (guard_stack carried_sp 1 0)
  let stack_failed := (opcode_failed stack_status)
  if (stack_failed : Bool)
  then
    (pure (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, (opcode_frame_status
        stack_status)))
  else
    (do
      let (gas_after_guard, guard_result) := (guard_static carried_gas carried_is_static)
      let guard_failed := (opcode_failed guard_result)
      if (guard_failed : Bool)
      then
        (pure (gas_after_guard, carried_state_gas, carried_state_spill, carried_refund, carried_sp, (opcode_frame_status
            guard_result)))
      else
        (do
          let beneficiary_word ← do (read_stack_word carried_sp)
          let sp_after ← do (stack_top_retreat carried_sp 1)
          let beneficiary := (word_to_address beneficiary_word)
          if ((fork ≥b Amsterdam) : Bool)
          then
            (do
              let warm ← do (k_account_is_warm beneficiary)
              let access_cost :=
                ((0 + G_selfdestruct) + (if (warm : Bool)
                  then G_zero
                  else G_amsterdam_cold_account_access))
              let (sentry_halt, sentry_gas) := (check_execution_gas carried_gas access_cost)
              if (sentry_halt : Bool)
              then
                (pure (sentry_gas, carried_state_gas, carried_state_spill, carried_refund, sp_after, (Exceptional
                    OutOfGas)))
              else
                (do
                  (k_account_mark_warm beneficiary)
                  let bal ← do (k_get_balance carried_address)
                  let nonzero_balance := (word_nonzero bal)
                  let beneficiary_empty ← do (k_account_is_empty beneficiary)
                  let creates_account := (nonzero_balance && beneficiary_empty)
                  let execution_cost :=
                    if (creates_account : Bool)
                    then (access_cost + G_amsterdam_account_write)
                    else access_cost
                  let (execution_halt, gas_after_execution) := (charge carried_gas execution_cost)
                  if (execution_halt : Bool)
                  then
                    (pure (gas_after_execution, carried_state_gas, carried_state_spill, carried_refund, sp_after, (Exceptional
                        OutOfGas)))
                  else
                    (do
                      let (state_halt, gas_after_all_charges, state_gas_after, state_spill_after) ← do
                        if (creates_account : Bool)
                        then
                          (charge_state_gas gas_after_execution carried_state_gas
                            carried_state_spill G_amsterdam_state_new_account)
                        else
                          (pure (false, gas_after_execution, carried_state_gas, carried_state_spill))
                      if (state_halt : Bool)
                      then
                        (pure (gas_after_all_charges, state_gas_after, state_spill_after, carried_refund, sp_after, (Exceptional
                            OutOfGas)))
                      else
                        (do
                          (k_transfer carried_address beneficiary bal)
                          let created ← do (k_was_created carried_address)
                          if (created : Bool)
                          then (k_selfdestruct carried_address)
                          else (pure ())
                          (pure (gas_after_all_charges, state_gas_after, state_spill_after, carried_refund, sp_after, (Halted
                              (HaltSelfDestruct ()))))))))
          else
            (do
              let bal ← do (k_get_balance carried_address)
              let warm ← do (k_account_is_warm beneficiary)
              let (selfdestruct_halt, gas_after_execution) := (charge carried_gas G_selfdestruct)
              if (selfdestruct_halt : Bool)
              then
                (pure (gas_after_execution, carried_state_gas, carried_state_spill, carried_refund, sp_after, (Exceptional
                    OutOfGas)))
              else
                (do
                  let (cold_account_halt, gas_after_access) :=
                    if ((! warm) : Bool)
                    then (charge gas_after_execution G_cold_account)
                    else (false, gas_after_execution)
                  if (cold_account_halt : Bool)
                  then
                    (pure (gas_after_access, carried_state_gas, carried_state_spill, carried_refund, sp_after, (Exceptional
                        OutOfGas)))
                  else
                    (do
                      (k_account_mark_warm beneficiary)
                      let nonzero_balance := (word_nonzero bal)
                      let beneficiary_empty ← do (k_account_is_empty beneficiary)
                      let (new_account_halt, gas_after_all_charges) :=
                        if ((nonzero_balance && beneficiary_empty) : Bool)
                        then (charge gas_after_access G_newaccount)
                        else (false, gas_after_access)
                      if (new_account_halt : Bool)
                      then
                        (pure (gas_after_all_charges, carried_state_gas, carried_state_spill, carried_refund, sp_after, (Exceptional
                            OutOfGas)))
                      else
                        (do
                          let is_selfdestructed ← do (k_is_selfdestructed carried_address)
                          let first_selfdestruct := (! is_selfdestructed)
                          let refund_after ← do
                            if (((fork <b London) && first_selfdestruct) : Bool)
                            then (record_refund carried_refund R_selfdestruct_pre_london)
                            else (pure carried_refund)
                          (k_transfer carried_address beneficiary bal)
                          if ((fork <b Cancun) : Bool)
                          then
                            (do
                              (k_zero_balance carried_address)
                              (k_selfdestruct carried_address))
                          else
                            (do
                              let created ← do (k_was_created carried_address)
                              if (created : Bool)
                              then
                                (do
                                  (k_zero_balance carried_address)
                                  (k_selfdestruct carried_address))
                              else (pure ()))
                          (pure (gas_after_all_charges, carried_state_gas, carried_state_spill, refund_after, sp_after, (Halted
                              (HaltSelfDestruct ()))))))))))

