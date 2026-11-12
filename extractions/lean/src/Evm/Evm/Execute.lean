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

The interpreter [interpret][] dispatches each decoded opcode to a handler
that charges its gas, consumes its stack operands, and produces its result
or effect. The handlers are grouped by family (arithmetic, bitwise, keccak,
environment, block, stack/memory, storage, flow, push/dup/swap, log, system).

Handlers follow the state-passing convention (YP μ′ = Ξ(μ)): each takes
only the carried values it uses and returns the same values in the same order. Decoded
instruction data remains explicit where required. The registers behind the
state are read and written only at frame boundaries.

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [memory_expansion_gas_cost][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies are the `execute_*` functions below. -/

/-- Converts a non-terminal opcode result into the corresponding frame status. -/
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
/- Type quantifiers: k_ex426485_ : Bool, k_ex426484_ : Nat, 0 ≤ k_ex426484_ ∧
  k_ex426484_ ≤ (2 ^ 64 - 1) -/
def guard_static (g : Nat) (is_static : Bool) : (Nat × OpcodeOutcome) :=
  if (is_static : Bool)
  then (GAS_ZERO, (Failed WriteProtection))
  else (g, (Continue ()))

/-- `JUMP`/`JUMPI` target validity: the destination must be in code range
and land on a `JUMPDEST` (the precomputed valid-destination set,
YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
bytes that happen to equal `0x5b` are not valid. Returns the next
program counter and the carried gas. -/
/- Type quantifiers: k_ex426495_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, k_ex426490_ : Nat, k_ex426489_ : Nat, 0 ≤ k_ex426489_ ∧ k_ex426489_ ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426490_ ∧ k_ex426490_ ≤ (2 ^ 64 - 1), 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex426495_ ∧ k_ex426495_ ≤ (2 ^ 256 - 1) -/
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
/- Type quantifiers: k_ex426497_ : Nat, k_ex426496_ : Nat, 0 ≤ k_ex426496_ ∧
  k_ex426496_ ≤ 1024, 0 ≤ k_ex426497_ ∧ k_ex426497_ ≤ 1024 -/
def guard_stack (carried_sp : StackPointer) (inputs : Nat) (outputs : Nat) : OpcodeOutcome :=
  let stack_validation := (validate_stack carried_sp inputs outputs)
  match stack_validation with
  | .StackValid => (Continue ())
  | .StackUnderflowFailure => (Failed StackUnderflow)
  | .StackOverflowFailure => (Failed StackOverflow)

/-- Pops `count` log topics from the stack into its bounded representation. -/
/- Type quantifiers: k_ex426498_ : Nat, 0 ≤ k_ex426498_ ∧ k_ex426498_ ≤ 4 -/
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
/- Type quantifiers: k_ex426499_ : Nat, 0 ≤ k_ex426499_ ∧ k_ex426499_ ≤ (2 ^ 64 - 1) -/
def execute_add (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_add a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `MUL`. -/
/- Type quantifiers: k_ex426500_ : Nat, 0 ≤ k_ex426500_ ∧ k_ex426500_ ≤ (2 ^ 64 - 1) -/
def execute_mul (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_mul a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `SUB`. -/
/- Type quantifiers: k_ex426501_ : Nat, 0 ≤ k_ex426501_ ∧ k_ex426501_ ≤ (2 ^ 64 - 1) -/
def execute_sub (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_sub a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements unsigned `DIV`. -/
/- Type quantifiers: k_ex426502_ : Nat, 0 ≤ k_ex426502_ ∧ k_ex426502_ ≤ (2 ^ 64 - 1) -/
def execute_div (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_div a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements signed `SDIV`. -/
/- Type quantifiers: k_ex426503_ : Nat, 0 ≤ k_ex426503_ ∧ k_ex426503_ ≤ (2 ^ 64 - 1) -/
def execute_sdiv (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_sdiv a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements unsigned `MOD`. -/
/- Type quantifiers: k_ex426504_ : Nat, 0 ≤ k_ex426504_ ∧ k_ex426504_ ≤ (2 ^ 64 - 1) -/
def execute_mod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_mod a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements signed `SMOD`. -/
/- Type quantifiers: k_ex426505_ : Nat, 0 ≤ k_ex426505_ ∧ k_ex426505_ ≤ (2 ^ 64 - 1) -/
def execute_smod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_smod a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `ADDMOD`. -/
/- Type quantifiers: k_ex426506_ : Nat, 0 ≤ k_ex426506_ ∧ k_ex426506_ ≤ (2 ^ 64 - 1) -/
def execute_addmod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 3 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_mid) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_mid)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let n ← do (read_stack_word sp)
          let result := (alu_addmod a b n)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `MULMOD`. -/
/- Type quantifiers: k_ex426507_ : Nat, 0 ≤ k_ex426507_ ∧ k_ex426507_ ≤ (2 ^ 64 - 1) -/
def execute_mulmod (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 3 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_mid) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_mid)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let n ← do (read_stack_word sp)
          let result := (alu_mulmod a b n)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `EXP`, including exponent-dependent gas. -/
/- Type quantifiers: k_ex426508_ : Nat, 0 ≤ k_ex426508_ ∧ k_ex426508_ ≤ (2 ^ 64 - 1) -/
def execute_exp (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      let a ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let e ← do (read_stack_word sp)
      let gas_cost := (exp_gas e)
      if ((carried_gas <b gas_cost) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - gas_cost)
          let result ← do (alu_exp a e)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `SIGNEXTEND`. -/
/- Type quantifiers: k_ex426509_ : Nat, 0 ≤ k_ex426509_ ∧ k_ex426509_ ≤ (2 ^ 64 - 1) -/
def execute_signextend (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let bi ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let v ← do (read_stack_word sp)
          let result := (alu_signextend bi v)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements unsigned `LT`. -/
/- Type quantifiers: k_ex426510_ : Nat, 0 ≤ k_ex426510_ ∧ k_ex426510_ ≤ (2 ^ 64 - 1) -/
def execute_lt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_lt a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements unsigned `GT`. -/
/- Type quantifiers: k_ex426511_ : Nat, 0 ≤ k_ex426511_ ∧ k_ex426511_ ≤ (2 ^ 64 - 1) -/
def execute_gt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_gt a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements signed `SLT`. -/
/- Type quantifiers: k_ex426512_ : Nat, 0 ≤ k_ex426512_ ∧ k_ex426512_ ≤ (2 ^ 64 - 1) -/
def execute_slt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_slt a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements signed `SGT`. -/
/- Type quantifiers: k_ex426513_ : Nat, 0 ≤ k_ex426513_ ∧ k_ex426513_ ≤ (2 ^ 64 - 1) -/
def execute_sgt (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_sgt a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `EQ`. -/
/- Type quantifiers: k_ex426514_ : Nat, 0 ≤ k_ex426514_ ∧ k_ex426514_ ≤ (2 ^ 64 - 1) -/
def execute_eq (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_eq a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements `ISZERO`. -/
/- Type quantifiers: k_ex426515_ : Nat, 0 ≤ k_ex426515_ ∧ k_ex426515_ ≤ (2 ^ 64 - 1) -/
def execute_iszero (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word carried_sp)
          let result := (alu_iszero a)
          (write_stack_word carried_sp result)
          (pure (gas, carried_sp, (Continue ())))))

/-- Implements bitwise `AND`. -/
/- Type quantifiers: k_ex426516_ : Nat, 0 ≤ k_ex426516_ ∧ k_ex426516_ ≤ (2 ^ 64 - 1) -/
def execute_and (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_and a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements bitwise `OR`. -/
/- Type quantifiers: k_ex426517_ : Nat, 0 ≤ k_ex426517_ ∧ k_ex426517_ ≤ (2 ^ 64 - 1) -/
def execute_or (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_or a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements bitwise `XOR`. -/
/- Type quantifiers: k_ex426518_ : Nat, 0 ≤ k_ex426518_ ∧ k_ex426518_ ≤ (2 ^ 64 - 1) -/
def execute_xor (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let b ← do (read_stack_word sp)
          let result := (alu_xor a b)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements bitwise `NOT`. -/
/- Type quantifiers: k_ex426519_ : Nat, 0 ≤ k_ex426519_ ∧ k_ex426519_ ≤ (2 ^ 64 - 1) -/
def execute_not (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let a ← do (read_stack_word carried_sp)
          let result := (alu_not a)
          (write_stack_word carried_sp result)
          (pure (gas, carried_sp, (Continue ())))))

/-- Implements `BYTE`. -/
/- Type quantifiers: k_ex426520_ : Nat, 0 ≤ k_ex426520_ ∧ k_ex426520_ ≤ (2 ^ 64 - 1) -/
def execute_byte (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let i ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let x ← do (read_stack_word sp)
          let result := (alu_byte i x)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements logical left shift `SHL` (EIP-145). -/
/- Type quantifiers: k_ex426521_ : Nat, 0 ≤ k_ex426521_ ∧ k_ex426521_ ≤ (2 ^ 64 - 1) -/
def execute_shl (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let s ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let v ← do (read_stack_word sp)
          let result := (alu_shl s v)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements logical right shift `SHR` (EIP-145). -/
/- Type quantifiers: k_ex426522_ : Nat, 0 ≤ k_ex426522_ ∧ k_ex426522_ ≤ (2 ^ 64 - 1) -/
def execute_shr (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let s ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let v ← do (read_stack_word sp)
          let result := (alu_shr s v)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements arithmetic right shift `SAR` (EIP-145). -/
/- Type quantifiers: k_ex426523_ : Nat, 0 ≤ k_ex426523_ ∧ k_ex426523_ ≤ (2 ^ 64 - 1) -/
def execute_sar (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let s ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let v ← do (read_stack_word sp)
          let result := (alu_sar s v)
          (write_stack_word sp result)
          (pure (gas, sp, (Continue ())))))

/-- Implements count-leading-zeroes `CLZ` (EIP-7939). -/
/- Type quantifiers: k_ex426524_ : Nat, 0 ≤ k_ex426524_ ∧ k_ex426524_ ≤ (2 ^ 64 - 1) -/
def execute_clz (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let x ← do (read_stack_word carried_sp)
          let result := (alu_clz x)
          (write_stack_word carried_sp result)
          (pure (gas, carried_sp, (Continue ())))))

/-- Implements `KECCAK256` over an expanded memory range. -/
/- Type quantifiers: k_ex426527_ : Nat, k_ex426526_ : Nat, k_ex426525_ : Nat, 0 ≤ k_ex426525_ ∧
  k_ex426525_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426526_ ∧ k_ex426526_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426527_ ∧ k_ex426527_ ≤ (2 ^ 32 - 1) -/
def execute_keccak256 (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      let offset_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let length_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let keccak_cost := (keccak_gas_cost length_word gas)
      if ((! keccak_cost.affordable) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas keccak_cost.cost)
          let requested_height := (memory_requested_height offset_word length_word)
          let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
          if ((! expansion_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas expansion_cost.cost)
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
              let memory ← (expand_memory memory_base memory access.requested_height)
              let digest ← do (mem_keccak memory_base memory ⟨_, ⟨_, access.range⟩⟩)
              let sp ← (stack_top_advance sp 1)
              (write_stack_word sp digest)
              (pure (gas, sp, memory, (Continue ()))))))

/-- Implements `ADDRESS`. -/
/- Type quantifiers: k_ex426528_ : Nat, 0 ≤ k_ex426528_ ∧ k_ex426528_ ≤ (2 ^ 64 - 1) -/
def execute_address (carried_address : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let address_word := (address_to_word carried_address)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp address_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `ORIGIN`. -/
/- Type quantifiers: k_ex426529_ : Nat, 0 ≤ k_ex426529_ ∧ k_ex426529_ ≤ (2 ^ 64 - 1) -/
def execute_origin (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let origin ← do (k_env F_Origin)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp origin)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CALLER`. -/
/- Type quantifiers: k_ex426530_ : Nat, 0 ≤ k_ex426530_ ∧ k_ex426530_ ≤ (2 ^ 64 - 1) -/
def execute_caller (carried_caller : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let caller := (address_to_word carried_caller)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp caller)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CALLVALUE`. -/
/- Type quantifiers: k_ex426532_ : Nat, k_ex426531_ : Nat, 0 ≤ k_ex426531_ ∧
  k_ex426531_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex426532_ ∧ k_ex426532_ ≤ (2 ^ 64 - 1) -/
def execute_callvalue (carried_value : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp carried_value)
          (pure (gas, sp, (Continue ())))))

/-- Implements `GASPRICE`. -/
/- Type quantifiers: k_ex426533_ : Nat, 0 ≤ k_ex426533_ ∧ k_ex426533_ ≤ (2 ^ 64 - 1) -/
def execute_gasprice (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let gas_price ← do (k_env F_GasPrice)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp gas_price)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CALLDATASIZE`. -/
/- Type quantifiers: k_ex426534_ : Nat, 0 ≤ k_ex426534_ ∧ k_ex426534_ ≤ (2 ^ 64 - 1) -/
def execute_calldatasize (carried_calldata : CalldataSlice) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let input := carried_calldata
          let input_length := (calldata_slice_length input)
          let length_word ← do (word_of_source_byte_count input_length)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp length_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CALLDATALOAD`. -/
/- Type quantifiers: k_ex426535_ : Nat, 0 ≤ k_ex426535_ ∧ k_ex426535_ ≤ (2 ^ 64 - 1) -/
def execute_calldataload (carried_calldata : CalldataSlice) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let offset_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let value ← do (calldata_slice_load_word_offset carried_calldata offset_word)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp value)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CALLDATACOPY`. -/
/- Type quantifiers: k_ex426538_ : Nat, k_ex426537_ : Nat, k_ex426536_ : Nat, 0 ≤ k_ex426536_ ∧
  k_ex426536_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426537_ ∧ k_ex426537_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426538_ ∧ k_ex426538_ ≤ (2 ^ 32 - 1) -/
def execute_calldatacopy (carried_calldata : CalldataSlice) (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 3 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let destination_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let source_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let length_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let copy_cost := (copy_gas_cost length_word gas)
          if ((! copy_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas copy_cost.cost)
              let requested_height := (memory_requested_height destination_word length_word)
              let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
              if ((! expansion_cost.affordable) : Bool)
              then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas expansion_cost.cost)
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let memory ← (expand_memory memory_base memory access.requested_height)
                  let range := access.range
                  let destination ← do (memory_absolute memory_base range.off)
                  (calldata_slice_copy_word_offset carried_calldata destination source_word
                    range.len)
                  (pure (gas, sp, memory, (Continue ())))))))

/-- Implements `CODESIZE`. -/
/- Type quantifiers: k_ex426546_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426546_ ∧ k_ex426546_ ≤ (2 ^ 64 - 1) -/
def execute_codesize (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let code_length := (frame_code_len ⟨_, ⟨_, carried_code⟩⟩)
          let length_word ← do (word_of_source_byte_count code_length)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp length_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CODECOPY`. -/
/- Type quantifiers: k_ex426556_ : Nat, k_ex426555_ : Nat, k_ex426554_ : Nat, carried_code_dependentWitness1
  : Nat, carried_code_dependentWitness0 : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426554_ ∧ k_ex426554_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426555_ ∧
  k_ex426555_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex426556_ ∧ k_ex426556_ ≤ (2 ^ 32 - 1) -/
def execute_codecopy (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 3 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let destination_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let source_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let length_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let copy_cost := (copy_gas_cost length_word gas)
          if ((! copy_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas copy_cost.cost)
              let requested_height := (memory_requested_height destination_word length_word)
              let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
              if ((! expansion_cost.affordable) : Bool)
              then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas expansion_cost.cost)
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let memory ← (expand_memory memory_base memory access.requested_height)
                  let range := access.range
                  let code := carried_code
                  let bytes := (code_bytes code)
                  let destination ← do (memory_absolute memory_base range.off)
                  (code_slice_copy_word_offset ⟨_, ⟨_, bytes⟩⟩ destination source_word
                    range.len)
                  (pure (gas, sp, memory, (Continue ())))))))

/-- Implements `BALANCE`, including warm/cold account access. -/
/- Type quantifiers: k_ex426557_ : Nat, 0 ≤ k_ex426557_ ∧ k_ex426557_ ≤ (2 ^ 64 - 1) -/
def execute_balance (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      let address_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let a := (word_to_address address_word)
      let warm ← do (k_account_is_warm a)
      let gas_cost ← do (account_cost warm)
      if ((carried_gas <b gas_cost) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - gas_cost)
          (k_account_mark_warm a)
          let balance ← do (k_get_balance a)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp balance)
          (pure (gas, sp, (Continue ())))))

/-- Implements `SELFBALANCE`. -/
/- Type quantifiers: k_ex426558_ : Nat, 0 ≤ k_ex426558_ ∧ k_ex426558_ ≤ (2 ^ 64 - 1) -/
def execute_selfbalance (carried_address : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_low) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_low)
          let balance ← do (k_get_balance carried_address)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp balance)
          (pure (gas, sp, (Continue ())))))

/-- Implements `EXTCODESIZE`, including warm/cold account access. -/
/- Type quantifiers: k_ex426559_ : Nat, 0 ≤ k_ex426559_ ∧ k_ex426559_ ≤ (2 ^ 64 - 1) -/
def execute_extcodesize (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      let address_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let a := (word_to_address address_word)
      let warm ← do (k_account_is_warm a)
      let access_cost ← do (account_cost warm)
      let read_cost ← do (external_code_read_cost ())
      if ((carried_gas <b (access_cost + read_cost)) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub carried_gas (access_cost + read_cost))
          (k_account_mark_warm a)
          let code_size ← do (k_get_code_size a)
          let size_word ← do (word_of_source_byte_count code_size)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp size_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `EXTCODECOPY`, including access and copy gas. -/
/- Type quantifiers: k_ex426562_ : Nat, k_ex426561_ : Nat, k_ex426560_ : Nat, 0 ≤ k_ex426560_ ∧
  k_ex426560_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426561_ ∧ k_ex426561_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426562_ ∧ k_ex426562_ ≤ (2 ^ 32 - 1) -/
def execute_extcodecopy (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 4 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      let address_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let a := (word_to_address address_word)
      let destination_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let source_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let length_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let warm ← do (k_account_is_warm a)
      let access_cost ← do (account_cost warm)
      let read_cost ← do (external_code_read_cost ())
      if ((gas <b (access_cost + read_cost)) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas (access_cost + read_cost))
          let copy_cost := (copy_gas_cost length_word gas)
          if ((! copy_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas copy_cost.cost)
              let requested_height := (memory_requested_height destination_word length_word)
              let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
              if ((! expansion_cost.affordable) : Bool)
              then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas expansion_cost.cost)
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let memory ← (expand_memory memory_base memory access.requested_height)
                  let range := access.range
                  (k_account_mark_warm a)
                  let destination ← do (memory_absolute memory_base range.off)
                  (k_code_copy a destination source_word range.len)
                  (pure (gas, sp, memory, (Continue ())))))))

/-- Implements `EXTCODEHASH`, including warm/cold account access. -/
/- Type quantifiers: k_ex426563_ : Nat, 0 ≤ k_ex426563_ ∧ k_ex426563_ ≤ (2 ^ 64 - 1) -/
def execute_extcodehash (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      let address_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let a := (word_to_address address_word)
      let warm ← do (k_account_is_warm a)
      let gas_cost ← do (account_cost warm)
      if ((carried_gas <b gas_cost) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - gas_cost)
          (k_account_mark_warm a)
          let code_hash ← do (k_get_codehash a)
          let hash_word := (hash_to_word code_hash)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp hash_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `RETURNDATASIZE`. -/
/- Type quantifiers: k_ex426570_ : Nat, carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, 0 ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426570_ ∧ k_ex426570_ ≤ (2 ^ 64 - 1) -/
def execute_returndatasize (carried_returndata : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let return_data_size := (returndata_size ⟨_, ⟨_, carried_returndata⟩⟩)
          let size_word ← do (word_of_source_byte_count return_data_size)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp size_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements bounds-checked `RETURNDATACOPY`. -/
/- Type quantifiers: k_ex426579_ : Nat, k_ex426578_ : Nat, k_ex426577_ : Nat, carried_returndata_dependentWitness1
  : Nat, carried_returndata_dependentWitness0 : Nat, 0 ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426577_ ∧ k_ex426577_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426578_ ∧
  k_ex426578_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex426579_ ∧ k_ex426579_ ≤ (2 ^ 32 - 1) -/
def execute_returndatacopy (carried_returndata : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  let stack_status := (guard_stack carried_sp 3 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let destination_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let source_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let length_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let available := (returndata_size ⟨_, ⟨_, carried_returndata⟩⟩)
          if ((source_word ≤b available) : Bool)
          then
            (do
              let remaining := (returndata_remaining available source_word)
              if ((length_word ≤b remaining) : Bool)
              then
                (do
                  let bounded_length : Nat := length_word
                  let copy_cost := (copy_gas_cost length_word gas)
                  if ((! copy_cost.affordable) : Bool)
                  then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
                  else
                    (do
                      let gas : Nat := (gas_sub gas copy_cost.cost)
                      let requested_height := (memory_requested_height destination_word length_word)
                      let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
                      if ((! expansion_cost.affordable) : Bool)
                      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
                      else
                        (do
                          let gas : Nat := (gas_sub gas expansion_cost.cost)
                          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                            (memory_access destination_word length_word)
                          let memory ← (expand_memory memory_base memory access.requested_height)
                          let destination ← do (memory_absolute memory_base access.range.off)
                          let bounded_source_offset : Nat := source_word
                          (returndata_copy ⟨_, ⟨_, carried_returndata⟩⟩ destination
                            bounded_source_offset bounded_length)
                          (pure (gas, sp, memory, (Continue ()))))))
              else (pure (GAS_ZERO, sp, memory, (Failed InvalidOpcode))))
          else (pure (GAS_ZERO, sp, memory, (Failed InvalidOpcode)))))

/-- Implements `BLOCKHASH`. -/
/- Type quantifiers: k_ex426580_ : Nat, 0 ≤ k_ex426580_ ∧ k_ex426580_ ≤ (2 ^ 64 - 1) -/
def execute_blockhash (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b 20) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - 20)
          let block_number ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let block_hash ← do (k_blockhash block_number)
          let hash_word := (hash_to_word block_hash)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp hash_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `COINBASE`. -/
/- Type quantifiers: k_ex426581_ : Nat, 0 ≤ k_ex426581_ ∧ k_ex426581_ ≤ (2 ^ 64 - 1) -/
def execute_coinbase (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let coinbase ← do (k_env F_Coinbase)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp coinbase)
          (pure (gas, sp, (Continue ())))))

/-- Implements `TIMESTAMP`. -/
/- Type quantifiers: k_ex426582_ : Nat, 0 ≤ k_ex426582_ ∧ k_ex426582_ ≤ (2 ^ 64 - 1) -/
def execute_timestamp (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let timestamp ← do (k_env F_Timestamp)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp timestamp)
          (pure (gas, sp, (Continue ())))))

/-- Implements `NUMBER`. -/
/- Type quantifiers: k_ex426583_ : Nat, 0 ≤ k_ex426583_ ∧ k_ex426583_ ≤ (2 ^ 64 - 1) -/
def execute_number (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let block_number ← do (k_env F_Number)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp block_number)
          (pure (gas, sp, (Continue ())))))

/-- Implements `SLOTNUM`. -/
/- Type quantifiers: k_ex426584_ : Nat, 0 ≤ k_ex426584_ ∧ k_ex426584_ ≤ (2 ^ 64 - 1) -/
def execute_slotnum (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let slot_number ← do (k_env F_SlotNumber)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp slot_number)
          (pure (gas, sp, (Continue ())))))

/-- Implements `PREVRANDAO`. -/
/- Type quantifiers: k_ex426585_ : Nat, 0 ≤ k_ex426585_ ∧ k_ex426585_ ≤ (2 ^ 64 - 1) -/
def execute_prevrandao (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let prev_randao ← do (k_env F_PrevRandao)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp prev_randao)
          (pure (gas, sp, (Continue ())))))

/-- Implements `GASLIMIT`. -/
/- Type quantifiers: k_ex426586_ : Nat, 0 ≤ k_ex426586_ ∧ k_ex426586_ ≤ (2 ^ 64 - 1) -/
def execute_gaslimit (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let gas_limit ← do (k_env F_GasLimit)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp gas_limit)
          (pure (gas, sp, (Continue ())))))

/-- Implements `CHAINID`. -/
/- Type quantifiers: k_ex426587_ : Nat, 0 ≤ k_ex426587_ ∧ k_ex426587_ ≤ (2 ^ 64 - 1) -/
def execute_chainid (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let chain_id ← do (k_env F_ChainId)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp chain_id)
          (pure (gas, sp, (Continue ())))))

/-- Implements `BASEFEE`. -/
/- Type quantifiers: k_ex426588_ : Nat, 0 ≤ k_ex426588_ ∧ k_ex426588_ ≤ (2 ^ 64 - 1) -/
def execute_basefee (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let base_fee ← do (k_env F_BaseFee)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp base_fee)
          (pure (gas, sp, (Continue ())))))

/-- Implements `BLOBBASEFEE`. -/
/- Type quantifiers: k_ex426590_ : Nat, k_ex426589_ : Nat, 0 ≤ k_ex426589_ ∧
  k_ex426589_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex426590_ ∧ k_ex426590_ ≤ (2 ^ 64 - 1) -/
def execute_blobbasefee (blob_fee : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp blob_fee)
          (pure (gas, sp, (Continue ())))))

/-- Implements `BLOBHASH`. -/
/- Type quantifiers: k_ex426591_ : Nat, 0 ≤ k_ex426591_ ∧ k_ex426591_ ≤ (2 ^ 64 - 1) -/
def execute_blobhash (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let index ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let blob_hash ← do (k_blobhash index)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp blob_hash)
          (pure (gas, sp, (Continue ())))))

/-- Implements `POP`. -/
/- Type quantifiers: k_ex426592_ : Nat, 0 ≤ k_ex426592_ ∧ k_ex426592_ ≤ (2 ^ 64 - 1) -/
def execute_pop (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let _ ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          (pure (gas, sp, (Continue ())))))

/-- Implements `MLOAD`. -/
/- Type quantifiers: k_ex426595_ : Nat, k_ex426594_ : Nat, k_ex426593_ : Nat, 0 ≤ k_ex426593_ ∧
  k_ex426593_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426594_ ∧ k_ex426594_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426595_ ∧ k_ex426595_ ≤ (2 ^ 32 - 1) -/
def execute_mload (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let offset_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let word_size := (u256 32)
          let requested_height := (memory_requested_height offset_word word_size)
          let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
          if ((! expansion_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas expansion_cost.cost)
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word word_size)
              let memory ← (expand_memory memory_base memory access.requested_height)
              let value ← do (mem_load memory_base access.range.off)
              let sp ← (stack_top_advance sp 1)
              (write_stack_word sp value)
              (pure (gas, sp, memory, (Continue ()))))))

/-- Implements `MSTORE`. -/
/- Type quantifiers: k_ex426598_ : Nat, k_ex426597_ : Nat, k_ex426596_ : Nat, 0 ≤ k_ex426596_ ∧
  k_ex426596_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426597_ ∧ k_ex426597_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426598_ ∧ k_ex426598_ ≤ (2 ^ 32 - 1) -/
def execute_mstore (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let offset_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let v ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let word_size := (u256 32)
          let requested_height := (memory_requested_height offset_word word_size)
          let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
          if ((! expansion_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas expansion_cost.cost)
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word word_size)
              let memory ← (expand_memory memory_base memory access.requested_height)
              (mem_store memory_base access.range.off v)
              (pure (gas, sp, memory, (Continue ()))))))

/-- Implements `MSTORE8`. -/
/- Type quantifiers: k_ex426601_ : Nat, k_ex426600_ : Nat, k_ex426599_ : Nat, 0 ≤ k_ex426599_ ∧
  k_ex426599_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426600_ ∧ k_ex426600_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426601_ ∧ k_ex426601_ ≤ (2 ^ 32 - 1) -/
def execute_mstore8 (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let offset_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let v ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let requested_height := (memory_requested_height offset_word WORD_ONE)
          let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
          if ((! expansion_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas expansion_cost.cost)
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word WORD_ONE)
              let memory ← (expand_memory memory_base memory access.requested_height)
              (mem_store_byte memory_base access.range.off v)
              (pure (gas, sp, memory, (Continue ()))))))

/-- Implements `MSIZE`. -/
/- Type quantifiers: k_ex426603_ : Nat, k_ex426602_ : Nat, 0 ≤ k_ex426602_ ∧
  k_ex426602_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex426603_ ∧ k_ex426603_ ≤ (2 ^ 32 - 1) -/
def execute_msize (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, sp, carried_memory_height, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let high_water := (memory_high_water carried_memory_height)
          let words := (memory_word_count high_water)
          let size ← do (word_of_nat_byte_count (words *i 32))
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp size)
          (pure (gas, sp, carried_memory_height, (Continue ())))))

/-- Implements overlapping memory copy `MCOPY` (EIP-5656). -/
/- Type quantifiers: k_ex426606_ : Nat, k_ex426605_ : Nat, k_ex426604_ : Nat, 0 ≤ k_ex426604_ ∧
  k_ex426604_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426605_ ∧ k_ex426605_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426606_ ∧ k_ex426606_ ≤ (2 ^ 32 - 1) -/
def execute_mcopy (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 3 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      if ((gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_verylow)
          let destination_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let source_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let length_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let copy_cost := (copy_gas_cost length_word gas)
          if ((! copy_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas copy_cost.cost)
              let destination_requested_height :=
                (memory_requested_height destination_word length_word)
              let source_requested_height := (memory_requested_height source_word length_word)
              let requested_height :=
                if ((destination_requested_height <b source_requested_height) : Bool)
                then source_requested_height
                else destination_requested_height
              let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
              if ((! expansion_cost.affordable) : Bool)
              then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas expansion_cost.cost)
                  let ⟨_, ⟨_, ⟨_, destination⟩⟩⟩ ← do
                    (memory_access destination_word length_word)
                  let ⟨_, ⟨_, ⟨_, source⟩⟩⟩ ← do
                    (memory_access source_word length_word)
                  let materialized_required_size :=
                    if ((destination.requested_height <b source.requested_height) : Bool)
                    then source.requested_height
                    else destination.requested_height
                  let memory ← (expand_memory memory_base memory materialized_required_size)
                  (mem_mcopy memory_base destination.range.off source.range.off
                    destination.range.len)
                  (pure (gas, sp, memory, (Continue ())))))))

def undefined_AccountExecutionContext (_ : Unit) : SailM AccountExecutionContext := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))) })

def account_execution_context (address : (Vector (BitVec 8) 20)) : AccountExecutionContext :=
  { address := address }

/-- Reuses the carried account context when the frame address is unchanged and
rebuilds it when execution enters a different account. -/
def refresh_account_execution_context (context : AccountExecutionContext) (previous_address : (Vector (BitVec 8) 20)) (next_address : (Vector (BitVec 8) 20)) : AccountExecutionContext :=
  if ((previous_address == next_address) : Bool)
  then context
  else (account_execution_context next_address)

/-- Implements `SLOAD`, including warm/cold access gas. -/
/- Type quantifiers: k_ex426607_ : Nat, 0 ≤ k_ex426607_ ∧ k_ex426607_ ≤ (2 ^ 64 - 1) -/
def execute_sload (context : AccountExecutionContext) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      let s ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let warm ← do (k_slot_is_warm context.address s)
      let gas_cost ← do (sload_cost warm)
      if ((carried_gas <b gas_cost) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - gas_cost)
          (k_slot_mark_warm context.address s)
          let entry ← do (k_sload context.address s)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp entry.curr)
          (pure (gas, sp, (Continue ())))))

/- Type quantifiers: carried_refund : Int, carried_state_spill : Nat, carried_state_gas : Nat, carried_gas
  : Nat, k_ex426609_ : Bool, fork : Nat, 0 ≤ fork ∧ fork ≤ 16, 0 ≤ carried_gas ∧
  carried_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_gas ∧ carried_state_gas ≤ (2 ^ 64 - 1), 0
  ≤ carried_state_spill ∧ carried_state_spill ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤
  carried_refund ∧ carried_refund ≤ (199 * (2 ^ 64 - 1)) -/
def execute_sstore (context : AccountExecutionContext) (fork : Nat) (carried_is_static : Bool) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) : SailM (Nat × Nat × Nat × Int × StackPointer × OpcodeOutcome) := SailME.run do
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then
    (pure (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let state_gas : Nat := carried_state_gas
      let state_spill : Nat := carried_state_spill
      let refund : Int := carried_refund
      let sp : StackPointer := carried_sp
      let halt : Bool := false
      let status : OpcodeOutcome := (Continue ())
      let _ : Unit :=
        let (tup__0, tup__1) := ((guard_static gas carried_is_static) : (Nat × OpcodeOutcome))
        let gas : Nat := tup__0
        let status : OpcodeOutcome := tup__1
        ()
      if ((match status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (gas, state_gas, state_spill, refund, sp, status))
      else
        (do
          if (((fork <b Amsterdam) && ((gas ≤b G_callstipend) : Bool)) : Bool)
          then (pure (gas, state_gas, state_spill, refund, sp, (Failed OutOfGas)))
          else
            (do
              let s ← do (read_stack_word sp)
              let sp ← (stack_top_retreat sp 1)
              let v ← do (read_stack_word sp)
              let sp ← (stack_top_retreat sp 1)
              let warm ← do (k_slot_is_warm context.address s)
              let cold := (! warm)
              if ((fork ≥b Amsterdam) : Bool)
              then
                (do
                  let sentry_cost := (sstore_sentry_cost cold)
                  if ((gas <b sentry_cost) : Bool)
                  then
                    SailME.throw ((GAS_ZERO, state_gas, state_spill, refund, sp, (Failed OutOfGas)) : (Nat × Nat × Nat × Int × StackPointer × OpcodeOutcome))
                  else (pure ()))
              else (pure ())
              (k_slot_mark_warm context.address s)
              let entry ← do (k_sload context.address s)
              let costs ← do (sstore_costs entry.orig entry.curr v cold)
              let (gas, state_gas, state_spill) ← (( do
                if ((costs.state_credit != 0) : Bool)
                then
                  (do
                    let (tup__0, tup__1, tup__2) ← do
                      (credit_state_gas_refund gas state_gas state_spill costs.state_credit)
                    let gas : Nat := tup__0
                    let state_gas : Nat := tup__1
                    let state_spill : Nat := tup__2
                    (pure (gas, state_gas, state_spill)))
                else (pure (gas, state_gas, state_spill)) ) : SailME
                (Nat × Nat × Nat × Int × StackPointer × OpcodeOutcome) (Nat × Nat × Nat) )
              if ((gas <b costs.execution) : Bool)
              then (pure (GAS_ZERO, state_gas, state_spill, refund, sp, (Failed OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas costs.execution)
                  let (tup__0, tup__1, tup__2, tup__3) ← do
                    (charge_state_gas gas state_gas state_spill costs.state_charge)
                  let halt : Bool := tup__0
                  let gas : Nat := tup__1
                  let state_gas : Nat := tup__2
                  let state_spill : Nat := tup__3
                  (pure ())
                  if (halt : Bool)
                  then (pure (gas, state_gas, state_spill, refund, sp, (Failed OutOfGas)))
                  else
                    (do
                      let refund ← (( do
                        if ((! (costs.refund == GAS_REFUND_ZERO)) : Bool)
                        then
                          (do
                            (record_refund refund costs.refund))
                        else (pure refund) ) : SailME
                        (Nat × Nat × Nat × Int × StackPointer × OpcodeOutcome) Int )
                      if ((entry.curr != v) : Bool)
                      then
                        (k_sstore context.address s
                          { curr := v,
                            orig := entry.orig })
                      else (pure ())
                      (pure (gas, state_gas, state_spill, refund, sp, (Continue ()))))))))

/-- Implements transient-storage load `TLOAD` (EIP-1153). -/
/- Type quantifiers: k_ex426614_ : Nat, 0 ≤ k_ex426614_ ∧ k_ex426614_ ≤ (2 ^ 64 - 1) -/
def execute_tload (carried_address : (Vector (BitVec 8) 20)) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 1 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let sp : StackPointer := carried_sp
      if ((carried_gas <b G_warm_access) : Bool)
      then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_warm_access)
          let s ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let value ← do (k_tload carried_address s)
          let sp ← (stack_top_advance sp 1)
          (write_stack_word sp value)
          (pure (gas, sp, (Continue ())))))

/-- Implements transient-storage write `TSTORE` (EIP-1153). -/
/- Type quantifiers: k_ex426616_ : Nat, k_ex426615_ : Bool, 0 ≤ k_ex426616_ ∧
  k_ex426616_ ≤ (2 ^ 64 - 1) -/
def execute_tstore (carried_address : (Vector (BitVec 8) 20)) (carried_is_static : Bool) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let status : OpcodeOutcome := (Continue ())
      let sp : StackPointer := carried_sp
      let _ : Unit :=
        let (tup__0, tup__1) := ((guard_static gas carried_is_static) : (Nat × OpcodeOutcome))
        let gas : Nat := tup__0
        let status : OpcodeOutcome := tup__1
        ()
      if ((match status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (gas, sp, status))
      else
        (do
          if ((gas <b G_warm_access) : Bool)
          then (pure (GAS_ZERO, sp, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas G_warm_access)
              let s ← do (read_stack_word sp)
              let sp ← (stack_top_retreat sp 1)
              let v ← do (read_stack_word sp)
              let sp ← (stack_top_retreat sp 1)
              (k_tstore carried_address s v)
              (pure (gas, sp, (Continue ()))))))

/-- Implements unconditional `JUMP`. -/
/- Type quantifiers: k_ex426625_ : Nat, k_ex426624_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426624_ ∧ k_ex426624_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426625_ ∧
  k_ex426625_ ≤ (2 ^ 64 - 1) -/
def execute_jump (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_pc : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 1 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (carried_pc, GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let pc : Nat := carried_pc
      let status : OpcodeOutcome := (Continue ())
      if ((gas <b G_mid) : Bool)
      then (pure (pc, GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_mid)
          let dest ← do (read_stack_word carried_sp)
          let sp ← do (stack_top_retreat carried_sp 1)
          let (tup__0, tup__1, tup__2) ← do (do_jump pc gas ⟨_, ⟨_, carried_code⟩⟩ dest)
          let pc : Nat := tup__0
          let gas : Nat := tup__1
          let status : OpcodeOutcome := tup__2
          (pure ())
          (pure (pc, gas, sp, status))))

/-- Implements conditional `JUMPI`. -/
/- Type quantifiers: k_ex426634_ : Nat, k_ex426633_ : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0
  : Nat, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex426633_ ∧ k_ex426633_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426634_ ∧
  k_ex426634_ ≤ (2 ^ 64 - 1) -/
def execute_jumpi (carried_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_pc : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (carried_pc, GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let pc : Nat := carried_pc
      let status : OpcodeOutcome := (Continue ())
      let sp : StackPointer := carried_sp
      if ((gas <b G_high) : Bool)
      then (pure (pc, GAS_ZERO, sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas G_high)
          let dest ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let cond ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let condition_is_zero := (word_is_zero cond)
          if (condition_is_zero : Bool)
          then (pure (pc, gas, sp, status))
          else
            (do
              let (tup__0, tup__1, tup__2) ← do
                (do_jump pc gas ⟨_, ⟨_, carried_code⟩⟩ dest)
              let pc : Nat := tup__0
              let gas : Nat := tup__1
              let status : OpcodeOutcome := tup__2
              (pure ())
              (pure (pc, gas, sp, status)))))

/-- Implements `PC`, returning the current opcode position from the
carried, already-advanced program counter. -/
/- Type quantifiers: k_ex426636_ : Nat, k_ex426635_ : Nat, 0 ≤ k_ex426635_ ∧
  k_ex426635_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426636_ ∧ k_ex426636_ ≤ (2 ^ 64 - 1) -/
def execute_pc (carried_pc : Nat) (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (carried_pc, GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_base) : Bool)
      then (pure (carried_pc, GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let next_pc ← do (word_of_source_byte_count carried_pc)
          let opcode_pc := (alu_sub next_pc WORD_ONE)
          let sp ← do (stack_top_advance carried_sp 1)
          (write_stack_word sp opcode_pc)
          (pure (carried_pc, gas, sp, (Continue ())))))

/-- Implements `GAS`, returning the carried gas remaining after its own
charge. -/
/- Type quantifiers: k_ex426637_ : Nat, 0 ≤ k_ex426637_ ∧ k_ex426637_ ≤ (2 ^ 64 - 1) -/
def execute_gas (carried_gas : Nat) (carried_sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_base) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_base)
          let gas_word ← do (word_of_nat_byte_count gas)
          let sp ← do (stack_top_advance carried_sp 1)
          (write_stack_word sp gas_word)
          (pure (gas, sp, (Continue ())))))

/-- Implements `JUMPDEST`. -/
/- Type quantifiers: carried_gas : Nat, 0 ≤ carried_gas ∧ carried_gas ≤ (2 ^ 64 - 1) -/
def execute_jumpdest (carried_gas : Nat) : (Nat × OpcodeOutcome) :=
  if ((carried_gas <b G_jumpdest) : Bool)
  then (GAS_ZERO, (Failed OutOfGas))
  else
    (let gas : Nat := (carried_gas - G_jumpdest)
    (gas, (Continue ())))

/-- Implements the `PUSH0` through `PUSH32` family. -/
/- Type quantifiers: k_ex426641_ : Nat, k_ex426640_ : Nat, k_ex426639_ : Nat, 0 ≤ k_ex426639_ ∧
  k_ex426639_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex426640_ ∧ k_ex426640_ ≤ 32, 0 ≤ k_ex426641_ ∧
  k_ex426641_ ≤ (2 ^ 256 - 1) -/
def execute_push (carried_gas : Nat) (carried_sp : StackPointer) (n : Nat) (v : Nat) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp 0 1)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      let cost :=
        if ((n == 0) : Bool)
        then G_base
        else G_verylow
      if ((carried_gas <b cost) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - cost)
          let sp ← do (stack_top_advance carried_sp 1)
          (write_stack_word sp v)
          (pure (gas, sp, (Continue ())))))

/-- Implements the `DUP1` through `DUP16` family. -/
/- Type quantifiers: k_ex426643_ : Nat, k_ex426642_ : Nat, 0 ≤ k_ex426642_ ∧
  k_ex426642_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex426643_ ∧ k_ex426643_ ≤ 16 -/
def execute_dup (carried_gas : Nat) (carried_sp : StackPointer) (n : Nat) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp n (n + 1))
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let value ← do (stack_slot_read carried_sp (n - 1))
          let sp ← do (stack_top_advance carried_sp 1)
          (write_stack_word sp value)
          (pure (gas, sp, (Continue ())))))

/-- Implements the `SWAP1` through `SWAP16` family. -/
/- Type quantifiers: k_ex426645_ : Nat, k_ex426644_ : Nat, 0 ≤ k_ex426644_ ∧
  k_ex426644_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex426645_ ∧ k_ex426645_ ≤ 16 -/
def execute_swap (carried_gas : Nat) (carried_sp : StackPointer) (n : Nat) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp (n + 1) (n + 1))
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, stack_status))
  else
    (do
      if ((carried_gas <b G_verylow) : Bool)
      then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
      else
        (do
          let gas : Nat := (carried_gas - G_verylow)
          let top_value ← do (read_stack_word carried_sp)
          let other ← do (stack_slot_read carried_sp n)
          (stack_set carried_sp 0 other)
          (stack_set carried_sp n top_value)
          (pure (gas, carried_sp, (Continue ())))))

/-- Implements immediate deep-stack duplication `DUPN`. -/
/- Type quantifiers: k_ex426646_ : Nat, 0 ≤ k_ex426646_ ∧ k_ex426646_ ≤ (2 ^ 64 - 1) -/
def execute_dupn (carried_gas : Nat) (carried_sp : StackPointer) (immediate : (BitVec 8)) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let valid_immediate := (deep_stack_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (carried_gas, carried_sp, (Failed InvalidOpcode)))
  else
    (do
      let n ← do (decode_single_stack_index immediate)
      let stack_status := (guard_stack carried_sp n (n + 1))
      if ((match stack_status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (GAS_ZERO, carried_sp, stack_status))
      else
        (do
          if ((carried_gas <b G_verylow) : Bool)
          then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (carried_gas - G_verylow)
              let value ← do (stack_slot_read carried_sp (n - 1))
              let sp ← do (stack_top_advance carried_sp 1)
              (write_stack_word sp value)
              (pure (gas, sp, (Continue ()))))))

/-- Implements immediate deep-stack exchange `SWAPN`. -/
/- Type quantifiers: k_ex426647_ : Nat, 0 ≤ k_ex426647_ ∧ k_ex426647_ ≤ (2 ^ 64 - 1) -/
def execute_swapn (carried_gas : Nat) (carried_sp : StackPointer) (immediate : (BitVec 8)) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let valid_immediate := (deep_stack_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (carried_gas, carried_sp, (Failed InvalidOpcode)))
  else
    (do
      let n ← do (decode_single_stack_index immediate)
      let stack_status := (guard_stack carried_sp (n + 1) (n + 1))
      if ((match stack_status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (GAS_ZERO, carried_sp, stack_status))
      else
        (do
          if ((carried_gas <b G_verylow) : Bool)
          then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (carried_gas - G_verylow)
              let top_value ← do (read_stack_word carried_sp)
              let other ← do (stack_slot_read carried_sp n)
              (stack_set carried_sp 0 other)
              (stack_set carried_sp n top_value)
              (pure (gas, carried_sp, (Continue ()))))))

/-- Implements immediate pairwise deep-stack `EXCHANGE`. -/
/- Type quantifiers: k_ex426648_ : Nat, 0 ≤ k_ex426648_ ∧ k_ex426648_ ≤ (2 ^ 64 - 1) -/
def execute_exchange (carried_gas : Nat) (carried_sp : StackPointer) (immediate : (BitVec 8)) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  let valid_immediate := (exchange_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (carried_gas, carried_sp, (Failed InvalidOpcode)))
  else
    (do
      let (n, m) ← do (decode_exchange_stack_indices immediate)
      let stack_status := (guard_stack carried_sp (m + 1) (m + 1))
      if ((match stack_status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (GAS_ZERO, carried_sp, stack_status))
      else
        (do
          if ((carried_gas <b G_verylow) : Bool)
          then (pure (GAS_ZERO, carried_sp, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (carried_gas - G_verylow)
              let first ← do (stack_slot_read carried_sp n)
              let second ← do (stack_slot_read carried_sp m)
              (stack_set carried_sp n second)
              (stack_set carried_sp m first)
              (pure (gas, carried_sp, (Continue ()))))))

/-- Implements the `LOG0` through `LOG4` family. -/
/- Type quantifiers: k_ex426653_ : Nat, k_ex426652_ : Nat, k_ex426651_ : Nat, k_ex426650_ : Nat, k_ex426649_
  : Bool, 0 ≤ k_ex426650_ ∧ k_ex426650_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426651_ ∧
  k_ex426651_ ≤ 4, 0 ≤ k_ex426652_ ∧ k_ex426652_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex426653_ ∧
  k_ex426653_ ≤ (2 ^ 32 - 1) -/
def execute_log (carried_address : (Vector (BitVec 8) 20)) (carried_is_static : Bool) (memory_base : Nat) (n : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  let stack_status := (guard_stack carried_sp (n + 2) 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, stack_status))
  else
    (do
      let gas : Nat := carried_gas
      let status : OpcodeOutcome := (Continue ())
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      let topics : LogTopics := (LogTopics0 ())
      let _ : Unit :=
        let (tup__0, tup__1) := ((guard_static gas carried_is_static) : (Nat × OpcodeOutcome))
        let gas : Nat := tup__0
        let status : OpcodeOutcome := tup__1
        ()
      if ((match status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (gas, sp, memory, status))
      else
        (do
          let offset_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let length_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let (tup__0, tup__1) ← do (pop_log_topics n sp)
          let topics : LogTopics := tup__0
          let sp : StackPointer := tup__1
          (pure ())
          let log_cost := (log_gas_cost n length_word gas)
          if ((! log_cost.affordable) : Bool)
          then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
          else
            (do
              let gas : Nat := (gas_sub gas log_cost.cost)
              let requested_height := (memory_requested_height offset_word length_word)
              let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
              if ((! expansion_cost.affordable) : Bool)
              then (pure (GAS_ZERO, sp, memory, (Failed OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas expansion_cost.cost)
                  let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                    (memory_access offset_word length_word)
                  let memory ← (expand_memory memory_base memory access.requested_height)
                  let range := access.range
                  let ⟨_, ⟨_, data⟩⟩ ← do
                    (active_memory_slice memory_base memory range.off range.len)
                  let memory_slice := (evm_memory_slice data.bytes data.len)
                  let log_data := (LogDataMemory ⟨_, ⟨_, memory_slice⟩⟩)
                  (k_log carried_address topics log_data)
                  (pure (gas, sp, memory, (Continue ())))))))

/-- Implements normal `STOP`. -/
def execute_stop (_ : Unit) : FrameStatus :=
  let reason := (HaltStop ())
  (Halted reason)

/-- Implements successful `RETURN`. -/
/- Type quantifiers: k_ex426656_ : Nat, k_ex426655_ : Nat, k_ex426654_ : Nat, 0 ≤ k_ex426654_ ∧
  k_ex426654_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex426655_ ∧ k_ex426655_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex426656_ ∧ k_ex426656_ ≤ (2 ^ 32 - 1) -/
def execute_return (memory_base : Nat) (carried_gas : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × StackPointer × Nat × FrameStatus) := do
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then (pure (GAS_ZERO, carried_sp, carried_memory_height, (opcode_frame_status stack_status)))
  else
    (do
      let gas : Nat := carried_gas
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      let offset_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let length_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let requested_height := (memory_requested_height offset_word length_word)
      let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
      if ((! expansion_cost.affordable) : Bool)
      then (pure (GAS_ZERO, sp, memory, (Exceptional OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas expansion_cost.cost)
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
          let memory ← (expand_memory memory_base memory access.requested_height)
          let range := access.range
          let ⟨_, ⟨_, data⟩⟩ ← do
            (active_memory_slice memory_base memory range.off range.len)
          let ⟨_, ⟨_, output⟩⟩ ← do (freeze_memory_output ⟨_, ⟨_, data⟩⟩)
          let reason := (HaltReturn ⟨_, ⟨_, output⟩⟩)
          (pure (gas, sp, memory, (Halted reason)))))

/- Type quantifiers: carried_memory_height : Nat, carried_state_spill : Nat, carried_state_gas : Nat, carried_gas
  : Nat, memory_base : Nat, carried_state_gas_reservoir : Nat, 0 ≤ carried_state_gas_reservoir ∧
  carried_state_gas_reservoir ≤ (2 ^ 64 - 1), 0 ≤ memory_base ∧ memory_base ≤ (2 ^ 32 - 1), 0
  ≤ carried_gas ∧ carried_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_gas ∧
  carried_state_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_spill ∧ carried_state_spill ≤ (2 ^ 24), 0
  ≤ carried_memory_height ∧ carried_memory_height ≤ (2 ^ 32 - 1) -/
def execute_revert (carried_state_gas_reservoir : Nat) (memory_base : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_sp : StackPointer) (carried_memory_height : Nat) : SailM (Nat × Nat × Nat × StackPointer × Nat × FrameStatus) := do
  let stack_status := (guard_stack carried_sp 2 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then
    (pure (GAS_ZERO, carried_state_gas, carried_state_spill, carried_sp, carried_memory_height, (opcode_frame_status
        stack_status)))
  else
    (do
      let gas : Nat := carried_gas
      let state_gas : Nat := carried_state_gas
      let state_spill : Nat := carried_state_spill
      let sp : StackPointer := carried_sp
      let memory : Nat := carried_memory_height
      let offset_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let length_word ← do (read_stack_word sp)
      let sp ← (stack_top_retreat sp 1)
      let requested_height := (memory_requested_height offset_word length_word)
      let expansion_cost := (memory_expansion_gas_cost memory requested_height gas)
      if ((! expansion_cost.affordable) : Bool)
      then
        (pure (GAS_ZERO, carried_state_gas, carried_state_spill, sp, memory, (Exceptional OutOfGas)))
      else
        (do
          let gas : Nat := (gas_sub gas expansion_cost.cost)
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
          let memory ← (expand_memory memory_base memory access.requested_height)
          let range := access.range
          let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
            readReg k_execution_profile
          let profile := execution_profile.protocol
          let (gas, state_gas, state_spill) ← (( do
            if ((profile.fork ≥b Amsterdam) : Bool)
            then
              (do
                let gas ← (conserved_gas_add gas state_spill)
                let state_gas : Nat := carried_state_gas_reservoir
                let state_spill : Nat := STATE_GAS_SPILL_ZERO
                (pure (gas, state_gas, state_spill)))
            else (pure (gas, state_gas, state_spill)) ) : SailM (Nat × Nat × Nat) )
          let ⟨_, ⟨_, data⟩⟩ ← do
            (active_memory_slice memory_base memory range.off range.len)
          let ⟨_, ⟨_, output⟩⟩ ← do (freeze_memory_output ⟨_, ⟨_, data⟩⟩)
          let reason := (HaltRevert ⟨_, ⟨_, output⟩⟩)
          (pure (gas, state_gas, state_spill, sp, memory, (Halted reason)))))

/-- Reports invalid-opcode termination to the interpreter's exceptional-halt
boundary. -/
/- Type quantifiers: carried_gas : Nat, 0 ≤ carried_gas ∧ carried_gas ≤ (2 ^ 64 - 1) -/
def execute_invalid (carried_gas : Nat) : (Nat × OpcodeOutcome) :=
  (carried_gas, (Failed InvalidOpcode))

/- Type quantifiers: carried_refund : Int, carried_state_spill : Nat, carried_state_gas : Nat, carried_gas
  : Nat, k_ex426665_ : Bool, fork : Nat, 0 ≤ fork ∧ fork ≤ 16, 0 ≤ carried_gas ∧
  carried_gas ≤ (2 ^ 64 - 1), 0 ≤ carried_state_gas ∧ carried_state_gas ≤ (2 ^ 64 - 1), 0
  ≤ carried_state_spill ∧ carried_state_spill ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤
  carried_refund ∧ carried_refund ≤ (199 * (2 ^ 64 - 1)) -/
def execute_selfdestruct (carried_address : (Vector (BitVec 8) 20)) (fork : Nat) (carried_is_static : Bool) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) : SailM (Nat × Nat × Nat × Int × StackPointer × FrameStatus) := SailME.run do
  let stack_status := (guard_stack carried_sp 1 0)
  if ((match stack_status with
     | .Failed _ => true
     | _ => false) : Bool)
  then
    (pure (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, (opcode_frame_status
        stack_status)))
  else
    (do
      let gas : Nat := carried_gas
      let state_gas : Nat := carried_state_gas
      let state_spill : Nat := carried_state_spill
      let refund : Int := carried_refund
      let sp : StackPointer := carried_sp
      let halt : Bool := false
      let status : OpcodeOutcome := (Continue ())
      let _ : Unit :=
        let (tup__0, tup__1) := ((guard_static gas carried_is_static) : (Nat × OpcodeOutcome))
        let gas : Nat := tup__0
        let status : OpcodeOutcome := tup__1
        ()
      if ((match status with
         | .Failed _ => true
         | _ => false) : Bool)
      then (pure (gas, state_gas, state_spill, refund, sp, (opcode_frame_status status)))
      else
        (do
          let beneficiary_word ← do (read_stack_word sp)
          let sp ← (stack_top_retreat sp 1)
          let beneficiary := (word_to_address beneficiary_word)
          let halt_reason := (HaltSelfDestruct ())
          let halt_status := (Halted halt_reason)
          if ((fork ≥b Amsterdam) : Bool)
          then
            (do
              let warm ← do (k_account_is_warm beneficiary)
              let cold_access_cost :=
                if (warm : Bool)
                then G_zero
                else G_amsterdam_cold_account_access
              let access_cost := (G_selfdestruct + cold_access_cost)
              if ((gas <b access_cost) : Bool)
              then (pure (GAS_ZERO, state_gas, state_spill, refund, sp, (Exceptional OutOfGas)))
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
                  if ((gas <b execution_cost) : Bool)
                  then (pure (GAS_ZERO, state_gas, state_spill, refund, sp, (Exceptional OutOfGas)))
                  else
                    (do
                      let gas : Nat := (gas_sub gas execution_cost)
                      let (gas, halt, state_gas, state_spill) ← (( do
                        if (creates_account : Bool)
                        then
                          (do
                            let (tup__0, tup__1, tup__2, tup__3) ← do
                              (charge_state_gas gas state_gas state_spill
                                G_amsterdam_state_new_account)
                            let halt : Bool := tup__0
                            let gas : Nat := tup__1
                            let state_gas : Nat := tup__2
                            let state_spill : Nat := tup__3
                            (pure (gas, halt, state_gas, state_spill)))
                        else (pure (gas, halt, state_gas, state_spill)) ) : SailME
                        (Nat × Nat × Nat × Int × StackPointer × FrameStatus)
                        (Nat × Bool × Nat × Nat) )
                      if (halt : Bool)
                      then (pure (gas, state_gas, state_spill, refund, sp, (Exceptional OutOfGas)))
                      else
                        (do
                          (k_transfer carried_address beneficiary bal)
                          let created ← do (k_was_created carried_address)
                          if (created : Bool)
                          then (k_selfdestruct carried_address)
                          else (pure ())
                          (pure (gas, state_gas, state_spill, refund, sp, halt_status))))))
          else
            (do
              let bal ← do (k_get_balance carried_address)
              let warm ← do (k_account_is_warm beneficiary)
              if ((gas <b G_selfdestruct) : Bool)
              then (pure (GAS_ZERO, state_gas, state_spill, refund, sp, (Exceptional OutOfGas)))
              else
                (do
                  let gas : Nat := (gas_sub gas G_selfdestruct)
                  let gas ← (( do
                    if ((! warm) : Bool)
                    then
                      (do
                        let gas ← (( do
                          if ((gas <b G_cold_account) : Bool)
                          then
                            SailME.throw ((GAS_ZERO, state_gas, state_spill, refund, sp, (Exceptional
                                OutOfGas)) : (Nat × Nat × Nat × Int × StackPointer × FrameStatus))
                          else (pure (gas_sub gas G_cold_account)) ) : SailME
                          (Nat × Nat × Nat × Int × StackPointer × FrameStatus) Nat )
                        (pure gas))
                    else (pure gas) ) : SailME
                    (Nat × Nat × Nat × Int × StackPointer × FrameStatus) Nat )
                  (k_account_mark_warm beneficiary)
                  let nonzero_balance := (word_nonzero bal)
                  let beneficiary_empty ← do (k_account_is_empty beneficiary)
                  let gas ← (( do
                    if ((nonzero_balance && beneficiary_empty) : Bool)
                    then
                      (do
                        let gas ← (( do
                          if ((gas <b G_newaccount) : Bool)
                          then
                            SailME.throw ((GAS_ZERO, state_gas, state_spill, refund, sp, (Exceptional
                                OutOfGas)) : (Nat × Nat × Nat × Int × StackPointer × FrameStatus))
                          else (pure (gas_sub gas G_newaccount)) ) : SailME
                          (Nat × Nat × Nat × Int × StackPointer × FrameStatus) Nat )
                        (pure gas))
                    else (pure gas) ) : SailME
                    (Nat × Nat × Nat × Int × StackPointer × FrameStatus) Nat )
                  let is_selfdestructed ← do (k_is_selfdestructed carried_address)
                  let first_selfdestruct := (! is_selfdestructed)
                  let refund ← (( do
                    if (((fork <b London) && first_selfdestruct) : Bool)
                    then
                      (do
                        (record_refund refund R_selfdestruct_pre_london))
                    else (pure refund) ) : SailME
                    (Nat × Nat × Nat × Int × StackPointer × FrameStatus) Int )
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
                  (pure (gas, state_gas, state_spill, refund, sp, halt_status))))))
