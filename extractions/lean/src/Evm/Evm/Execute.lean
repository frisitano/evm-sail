import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Host.Code
import Evm.Host.Output
import Evm.Kernel.Environment
import Evm.Kernel.Storage
import Evm.Kernel.Logs
import Evm.Kernel.Accounts
import Evm.Kernel.Code
import Evm.Kernel.Selfdestruct
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Precompiles
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

/-! # Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Handlers follow the state-passing convention (YP μ′ = Ξ(μ)): each takes
the machine state it uses — the carried gas, the operand-stack cursor,
the memory cursor for the memory family, and the program counter for the
flow family — by value and returns it updated. The registers behind these
values are read and written only at frame boundaries.

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [memory_expansion_cost][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][]. -/

/-- The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
`SELFDESTRUCT` act on this account. -/
def self_addr (_ : Unit) : SailM (Vector (BitVec 8) 20) := do
  (pure (← readReg message).address)

/-- EIP-214 write protection: any state-modifying opcode in a
`STATICCALL` frame halts exceptionally on the carried gas.
State-changing opcodes call this first; `true` means already
halted. -/
/- Type quantifiers: g : Nat, 0 ≤ g -/
def guard_static (g : Nat) : SailM (Bool × Nat) := do
  if ((← readReg message).is_static : Bool)
  then (pure (true, (← (exc_halt g WriteProtection))))
  else (pure (false, g))

/-- `JUMP`/`JUMPI` target validity: the destination must be in code range
and land on a `JUMPDEST` (the precomputed valid-destination set,
YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
bytes that happen to equal `0x5b` are not valid. Returns the next
program counter and the carried gas. -/
/- Type quantifiers: k_ex552632_ : Nat, k_ex552631_ : Nat, k_ex552630_ : Nat, 0 ≤ k_ex552630_ ∧
  k_ex552630_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552631_, 0 ≤ k_ex552632_ ∧
  k_ex552632_ ≤ (2 ^ 256 - 1) -/
def do_jump (pc_in : Nat) (g : Nat) (destination_value : Nat) : SailM (Nat × Nat) := do
  let code_length ← do (frame_code_len ())
  if ((destination_value <b code_length) : Bool)
  then
    (do
      let destination : Nat := destination_value
      let valid_destination ← do (frame_jumpdest_valid destination)
      if (valid_destination : Bool)
      then (pure (destination, g))
      else (pure (pc_in, (← (exc_halt g InvalidJump)))))
  else (pure (pc_in, (← (exc_halt g InvalidJump))))

/-- Pops `count` log topics from the stack into its bounded representation. -/
/- Type quantifiers: k_ex552633_ : Nat, 0 ≤ k_ex552633_ ∧ k_ex552633_ ≤ 4 -/
def pop_log_topics (count : Nat) (top : (BitVec 64)) : SailM (LogTopics × (BitVec 64)) := do
  match count with
  | 0 => (pure ((LogTopics0 ()), top))
  | 1 =>
    (do
      let (t0, top1) ← do (pop top)
      (pure ((LogTopics1 t0), top1)))
  | 2 =>
    (do
      let (t0, top1) ← do (pop top)
      let (t1, top2) ← do (pop top1)
      (pure ((LogTopics2 (t0, t1)), top2)))
  | 3 =>
    (do
      let (t0, top1) ← do (pop top)
      let (t1, top2) ← do (pop top1)
      let (t2, top3) ← do (pop top2)
      (pure ((LogTopics3 (t0, t1, t2)), top3)))
  | 4 =>
    (do
      let (t0, top1) ← do (pop top)
      let (t1, top2) ← do (pop top1)
      let (t2, top3) ← do (pop top2)
      let (t3, top4) ← do (pop top3)
      (pure ((LogTopics4 (t0, t1, t2, t3)), top4)))
  | _ => (pure ((LogTopics0 ()), top))

/-- Implements `ADD`. -/
/- Type quantifiers: k_ex552634_ : Nat, 0 ≤ k_ex552634_ -/
def execute_add (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_add a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements `MUL`. -/
/- Type quantifiers: k_ex552635_ : Nat, 0 ≤ k_ex552635_ -/
def execute_mul (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_mul a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements `SUB`. -/
/- Type quantifiers: k_ex552636_ : Nat, 0 ≤ k_ex552636_ -/
def execute_sub (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_sub a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements unsigned `DIV`. -/
/- Type quantifiers: k_ex552637_ : Nat, 0 ≤ k_ex552637_ -/
def execute_div (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_div a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements signed `SDIV`. -/
/- Type quantifiers: k_ex552638_ : Nat, 0 ≤ k_ex552638_ -/
def execute_sdiv (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_sdiv a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements unsigned `MOD`. -/
/- Type quantifiers: k_ex552639_ : Nat, 0 ≤ k_ex552639_ -/
def execute_mod (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_mod a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements signed `SMOD`. -/
/- Type quantifiers: k_ex552640_ : Nat, 0 ≤ k_ex552640_ -/
def execute_smod (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_smod a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements `ADDMOD`. -/
/- Type quantifiers: k_ex552641_ : Nat, 0 ≤ k_ex552641_ -/
def execute_addmod (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_mid)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let (n, top3) ← do (pop top2)
      let result := (alu_addmod a b n)
      (pure ((← (push_word top3 result)), g1)))

/-- Implements `MULMOD`. -/
/- Type quantifiers: k_ex552642_ : Nat, 0 ≤ k_ex552642_ -/
def execute_mulmod (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_mid)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let (n, top3) ← do (pop top2)
      let result := (alu_mulmod a b n)
      (pure ((← (push_word top3 result)), g1)))

/-- Implements `EXP`, including exponent-dependent gas. -/
/- Type quantifiers: k_ex552643_ : Nat, 0 ≤ k_ex552643_ -/
def execute_exp (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (a, top1) ← do (pop top)
  let (e, top2) ← do (pop top1)
  let gas_cost := (exp_gas e)
  let (gas_charged, g1) ← do (charge g gas_cost)
  if ((! gas_charged) : Bool)
  then (pure (top2, g1))
  else
    (do
      let result ← do (alu_exp a e)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements `SIGNEXTEND`. -/
/- Type quantifiers: k_ex552644_ : Nat, 0 ≤ k_ex552644_ -/
def execute_signextend (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (bi, top1) ← do (pop top)
      let (v, top2) ← do (pop top1)
      let result := (alu_signextend bi v)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements unsigned `LT`. -/
/- Type quantifiers: k_ex552645_ : Nat, 0 ≤ k_ex552645_ -/
def execute_lt (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_lt a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements unsigned `GT`. -/
/- Type quantifiers: k_ex552646_ : Nat, 0 ≤ k_ex552646_ -/
def execute_gt (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_gt a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements signed `SLT`. -/
/- Type quantifiers: k_ex552647_ : Nat, 0 ≤ k_ex552647_ -/
def execute_slt (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_slt a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements signed `SGT`. -/
/- Type quantifiers: k_ex552648_ : Nat, 0 ≤ k_ex552648_ -/
def execute_sgt (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_sgt a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements `EQ`. -/
/- Type quantifiers: k_ex552649_ : Nat, 0 ≤ k_ex552649_ -/
def execute_eq (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_eq a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements `ISZERO`. -/
/- Type quantifiers: k_ex552650_ : Nat, 0 ≤ k_ex552650_ -/
def execute_iszero (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let result := (alu_iszero a)
      (pure ((← (push_word top1 result)), g1)))

/-- Implements bitwise `AND`. -/
/- Type quantifiers: k_ex552651_ : Nat, 0 ≤ k_ex552651_ -/
def execute_and (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_and a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements bitwise `OR`. -/
/- Type quantifiers: k_ex552652_ : Nat, 0 ≤ k_ex552652_ -/
def execute_or (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_or a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements bitwise `XOR`. -/
/- Type quantifiers: k_ex552653_ : Nat, 0 ≤ k_ex552653_ -/
def execute_xor (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let (b, top2) ← do (pop top1)
      let result := (alu_xor a b)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements bitwise `NOT`. -/
/- Type quantifiers: k_ex552654_ : Nat, 0 ≤ k_ex552654_ -/
def execute_not (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (a, top1) ← do (pop top)
      let result := (alu_not a)
      (pure ((← (push_word top1 result)), g1)))

/-- Implements `BYTE`. -/
/- Type quantifiers: k_ex552655_ : Nat, 0 ≤ k_ex552655_ -/
def execute_byte (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (i, top1) ← do (pop top)
      let (x, top2) ← do (pop top1)
      let result := (alu_byte i x)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements logical left shift `SHL` (EIP-145). -/
/- Type quantifiers: k_ex552656_ : Nat, 0 ≤ k_ex552656_ -/
def execute_shl (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (s, top1) ← do (pop top)
      let (v, top2) ← do (pop top1)
      let result := (alu_shl s v)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements logical right shift `SHR` (EIP-145). -/
/- Type quantifiers: k_ex552657_ : Nat, 0 ≤ k_ex552657_ -/
def execute_shr (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (s, top1) ← do (pop top)
      let (v, top2) ← do (pop top1)
      let result := (alu_shr s v)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements arithmetic right shift `SAR` (EIP-145). -/
/- Type quantifiers: k_ex552658_ : Nat, 0 ≤ k_ex552658_ -/
def execute_sar (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (s, top1) ← do (pop top)
      let (v, top2) ← do (pop top1)
      let result := (alu_sar s v)
      (pure ((← (push_word top2 result)), g1)))

/-- Implements count-leading-zeroes `CLZ` (EIP-7939). -/
/- Type quantifiers: k_ex552659_ : Nat, 0 ≤ k_ex552659_ -/
def execute_clz (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (x, top1) ← do (pop top)
      let result := (alu_clz x)
      (pure ((← (push_word top1 result)), g1)))

/-- Implements `KECCAK256` over an expanded memory range. -/
/- Type quantifiers: k_ex552668_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552668_ -/
def execute_keccak256 (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (offset_word, top1) ← do (pop top)
  let (length_word, top2) ← do (pop top1)
  let (hashing_gas_charged, g1) ← do (charge_keccak_gas g length_word)
  if _sailIf0 : ((! hashing_gas_charged) : Bool) = true
  then
    (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let required_size := (memory_required_size offset_word length_word)
      let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
      let (expansion_gas_charged, g2) ← do (charge g1 expansion_cost)
      if _sailIf1 : ((! expansion_gas_charged) : Bool) = true
      then
        (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
          let ⟨_, ⟨_, mem1⟩⟩ ← do
            (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
          let (digest, mem2) ← do
            (mem_keccak ⟨_, ⟨_, mem1⟩⟩ ⟨_, ⟨_, access.range⟩⟩)
          (pure ((← do
              (push_word top2 digest)), (mem2 : (Sigma fun (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))))

/-- Implements `ADDRESS`. -/
/- Type quantifiers: k_ex552669_ : Nat, 0 ≤ k_ex552669_ -/
def execute_address (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let address ← do (self_addr ())
      let address_word := (address_to_word address)
      (pure ((← (push_word top address_word)), g1)))

/-- Implements `ORIGIN`. -/
/- Type quantifiers: k_ex552670_ : Nat, 0 ≤ k_ex552670_ -/
def execute_origin (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let origin ← do (k_env F_Origin)
      (pure ((← (push_word top origin)), g1)))

/-- Implements `CALLER`. -/
/- Type quantifiers: k_ex552671_ : Nat, 0 ≤ k_ex552671_ -/
def execute_caller (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let caller ← do (pure (address_to_word (← readReg message).caller))
      (pure ((← (push_word top caller)), g1)))

/-- Implements `CALLVALUE`. -/
/- Type quantifiers: k_ex552672_ : Nat, 0 ≤ k_ex552672_ -/
def execute_callvalue (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else (pure ((← (push_word top (← readReg message).value)), g1))

/-- Implements `GASPRICE`. -/
/- Type quantifiers: k_ex552673_ : Nat, 0 ≤ k_ex552673_ -/
def execute_gasprice (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let gas_price ← do (k_env F_GasPrice)
      (pure ((← (push_word top gas_price)), g1)))

/-- Implements `CALLDATASIZE`. -/
/- Type quantifiers: k_ex552674_ : Nat, 0 ≤ k_ex552674_ -/
def execute_calldatasize (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let input ← do readReg calldata
      let input_length := (calldata_slice_length input)
      let length_word ← do (word_of_source_byte_count input_length)
      (pure ((← (push_word top length_word)), g1)))

/-- Implements `CALLDATALOAD`. -/
/- Type quantifiers: k_ex552675_ : Nat, 0 ≤ k_ex552675_ -/
def execute_calldataload (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (offset_word, top1) ← do (pop top)
      let value ← do (calldata_slice_load_word_offset (← readReg calldata) offset_word)
      (pure ((← (push_word top1 value)), g1)))

/-- Implements `CALLDATACOPY`. -/
/- Type quantifiers: k_ex552684_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552684_ -/
def execute_calldatacopy (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (destination_word, top1) ← do (pop top)
      let (source_word, top2) ← do (pop top1)
      let (length_word, top3) ← do (pop top2)
      let (copy_gas_charged, g2) ← do (charge_copy_gas g1 length_word)
      if _sailIf1 : ((! copy_gas_charged) : Bool) = true
      then
        (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let required_size := (memory_required_size destination_word length_word)
          let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
          let (expansion_gas_charged, g3) ← do (charge g2 expansion_cost)
          if _sailIf2 : ((! expansion_gas_charged) : Bool) = true
          then
            (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat)))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                (memory_access destination_word length_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
              let range := access.range
              (calldata_slice_copy_word_offset (← readReg calldata) range.off source_word
                range.len)
              (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat))))))

/-- Implements `CODESIZE`. -/
/- Type quantifiers: k_ex552685_ : Nat, 0 ≤ k_ex552685_ -/
def execute_codesize (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let code_length ← do (frame_code_len ())
      let length_word ← do (word_of_source_byte_count code_length)
      (pure ((← (push_word top length_word)), g1)))

/-- Implements `CODECOPY`. -/
/- Type quantifiers: k_ex552694_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552694_ -/
def execute_codecopy (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (destination_word, top1) ← do (pop top)
      let (source_word, top2) ← do (pop top1)
      let (length_word, top3) ← do (pop top2)
      let (copy_gas_charged, g2) ← do (charge_copy_gas g1 length_word)
      if _sailIf1 : ((! copy_gas_charged) : Bool) = true
      then
        (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let required_size := (memory_required_size destination_word length_word)
          let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
          let (expansion_gas_charged, g3) ← do (charge g2 expansion_cost)
          if _sailIf2 : ((! expansion_gas_charged) : Bool) = true
          then
            (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat)))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                (memory_access destination_word length_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
              let range := access.range
              let ⟨_, ⟨_, code⟩⟩ ← do readReg frame_code
              let bytes := (code_bytes code)
              (code_slice_copy_word_offset ⟨_, ⟨_, bytes⟩⟩ range.off source_word range.len)
              (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat))))))

/-- Implements `BALANCE`, including warm/cold account access. -/
/- Type quantifiers: k_ex552695_ : Nat, 0 ≤ k_ex552695_ -/
def execute_balance (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (address_word, top1) ← do (pop top)
  let a := (word_to_address address_word)
  let warm ← do (k_account_is_warm a)
  let gas_cost ← do (account_cost warm)
  let (gas_charged, g1) ← do (charge g gas_cost)
  if ((! gas_charged) : Bool)
  then (pure (top1, g1))
  else
    (do
      (k_account_mark_warm a)
      let balance ← do (k_get_balance a)
      (pure ((← (push_word top1 balance)), g1)))

/-- Implements `SELFBALANCE`. -/
/- Type quantifiers: k_ex552696_ : Nat, 0 ≤ k_ex552696_ -/
def execute_selfbalance (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_low)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let address ← do (self_addr ())
      let balance ← do (k_get_balance address)
      (pure ((← (push_word top balance)), g1)))

/-- Implements `EXTCODESIZE`, including warm/cold account access. -/
/- Type quantifiers: k_ex552697_ : Nat, 0 ≤ k_ex552697_ -/
def execute_extcodesize (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (address_word, top1) ← do (pop top)
  let a := (word_to_address address_word)
  let warm ← do (k_account_is_warm a)
  let access_cost ← do (account_cost warm)
  let read_cost ← do (external_code_read_cost ())
  let (gas_charged, g1) ← do (charge g (access_cost + read_cost))
  if ((! gas_charged) : Bool)
  then (pure (top1, g1))
  else
    (do
      (k_account_mark_warm a)
      let code_size ← do (k_get_code_size a)
      let size_word ← do (word_of_source_byte_count code_size)
      (pure ((← (push_word top1 size_word)), g1)))

/-- Implements `EXTCODECOPY`, including access and copy gas. -/
/- Type quantifiers: k_ex552706_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552706_ -/
def execute_extcodecopy (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (address_word, top1) ← do (pop top)
  let a := (word_to_address address_word)
  let (destination_word, top2) ← do (pop top1)
  let (source_word, top3) ← do (pop top2)
  let (length_word, top4) ← do (pop top3)
  let warm ← do (k_account_is_warm a)
  let access_cost ← do (account_cost warm)
  let read_cost ← do (external_code_read_cost ())
  let (access_gas_charged, g1) ← do (charge g (access_cost + read_cost))
  if _sailIf0 : ((! access_gas_charged) : Bool) = true
  then
    (pure ((top4 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (copy_gas_charged, g2) ← do (charge_copy_gas g1 length_word)
      if _sailIf1 : ((! copy_gas_charged) : Bool) = true
      then
        (pure ((top4 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let required_size := (memory_required_size destination_word length_word)
          let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
          let (expansion_gas_charged, g3) ← do (charge g2 expansion_cost)
          if _sailIf2 : ((! expansion_gas_charged) : Bool) = true
          then
            (pure ((top4 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat)))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                (memory_access destination_word length_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
              let range := access.range
              (k_account_mark_warm a)
              (k_code_copy a range.off source_word range.len)
              (pure ((top4 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat))))))

/-- Implements `EXTCODEHASH`, including warm/cold account access. -/
/- Type quantifiers: k_ex552707_ : Nat, 0 ≤ k_ex552707_ -/
def execute_extcodehash (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (address_word, top1) ← do (pop top)
  let a := (word_to_address address_word)
  let warm ← do (k_account_is_warm a)
  let gas_cost ← do (account_cost warm)
  let (gas_charged, g1) ← do (charge g gas_cost)
  if ((! gas_charged) : Bool)
  then (pure (top1, g1))
  else
    (do
      (k_account_mark_warm a)
      let code_hash ← do (k_get_codehash a)
      let hash_word := (hash_to_word code_hash)
      (pure ((← (push_word top1 hash_word)), g1)))

/-- Implements `RETURNDATASIZE`. -/
/- Type quantifiers: k_ex552708_ : Nat, 0 ≤ k_ex552708_ -/
def execute_returndatasize (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let return_data_size ← do (returndata_size ())
      let size_word ← do (word_of_source_byte_count return_data_size)
      (pure ((← (push_word top size_word)), g1)))

/-- Implements bounds-checked `RETURNDATACOPY`. -/
/- Type quantifiers: k_ex552717_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552717_ -/
def execute_returndatacopy (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (destination_word, top1) ← do (pop top)
      let (source_word, top2) ← do (pop top1)
      let (length_word, top3) ← do (pop top2)
      let (copy_gas_charged, g2) ← do (charge_copy_gas g1 length_word)
      if _sailIf1 : ((! copy_gas_charged) : Bool) = true
      then
        (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let required_size := (memory_required_size destination_word length_word)
          let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
          let (expansion_gas_charged, g3) ← do (charge g2 expansion_cost)
          if _sailIf2 : ((! expansion_gas_charged) : Bool) = true
          then
            (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat)))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do
                (memory_access destination_word length_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
              let range := access.range
              let g4 ← do (returndata_copy_words g3 range.off source_word length_word)
              (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g4 : Nat))))))

/-- Implements `BLOCKHASH`. -/
/- Type quantifiers: k_ex552718_ : Nat, 0 ≤ k_ex552718_ -/
def execute_blockhash (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g 20)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (block_number, top1) ← do (pop top)
      let block_hash ← do (k_blockhash block_number)
      let hash_word := (hash_to_word block_hash)
      (pure ((← (push_word top1 hash_word)), g1)))

/-- Implements `COINBASE`. -/
/- Type quantifiers: k_ex552719_ : Nat, 0 ≤ k_ex552719_ -/
def execute_coinbase (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let coinbase ← do (k_env F_Coinbase)
      (pure ((← (push_word top coinbase)), g1)))

/-- Implements `TIMESTAMP`. -/
/- Type quantifiers: k_ex552720_ : Nat, 0 ≤ k_ex552720_ -/
def execute_timestamp (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let timestamp ← do (k_env F_Timestamp)
      (pure ((← (push_word top timestamp)), g1)))

/-- Implements `NUMBER`. -/
/- Type quantifiers: k_ex552721_ : Nat, 0 ≤ k_ex552721_ -/
def execute_number (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let block_number ← do (k_env F_Number)
      (pure ((← (push_word top block_number)), g1)))

/-- Implements `SLOTNUM`. -/
/- Type quantifiers: k_ex552722_ : Nat, 0 ≤ k_ex552722_ -/
def execute_slotnum (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let slot_number ← do (k_env F_SlotNumber)
      (pure ((← (push_word top slot_number)), g1)))

/-- Implements `PREVRANDAO`. -/
/- Type quantifiers: k_ex552723_ : Nat, 0 ≤ k_ex552723_ -/
def execute_prevrandao (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let prev_randao ← do (k_env F_PrevRandao)
      (pure ((← (push_word top prev_randao)), g1)))

/-- Implements `GASLIMIT`. -/
/- Type quantifiers: k_ex552724_ : Nat, 0 ≤ k_ex552724_ -/
def execute_gaslimit (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let gas_limit ← do (k_env F_GasLimit)
      (pure ((← (push_word top gas_limit)), g1)))

/-- Implements `CHAINID`. -/
/- Type quantifiers: k_ex552725_ : Nat, 0 ≤ k_ex552725_ -/
def execute_chainid (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let chain_id ← do (k_env F_ChainId)
      (pure ((← (push_word top chain_id)), g1)))

/-- Implements `BASEFEE`. -/
/- Type quantifiers: k_ex552726_ : Nat, 0 ≤ k_ex552726_ -/
def execute_basefee (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let base_fee ← do (k_env F_BaseFee)
      (pure ((← (push_word top base_fee)), g1)))

/-- Implements `BLOBBASEFEE`. -/
/- Type quantifiers: k_ex552727_ : Nat, 0 ≤ k_ex552727_ -/
def execute_blobbasefee (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
        readReg k_execution_profile
      let base_fee ← do
        (blob_base_fee
          ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile.protocol⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
          (← readReg k_header).excess_blob_gas)
      (pure ((← (push_word top base_fee)), g1)))

/-- Implements `BLOBHASH`. -/
/- Type quantifiers: k_ex552728_ : Nat, 0 ≤ k_ex552728_ -/
def execute_blobhash (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (index, top1) ← do (pop top)
      let blob_hash ← do (k_blobhash index)
      (pure ((← (push_word top1 blob_hash)), g1)))

/-- Implements `POP`. -/
/- Type quantifiers: k_ex552729_ : Nat, 0 ≤ k_ex552729_ -/
def execute_pop (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (_, top1) ← do (pop top)
      (pure (top1, g1)))

/-- Implements `MLOAD`. -/
/- Type quantifiers: k_ex552738_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552738_ -/
def execute_mload (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (offset_word, top1) ← do (pop top)
      let word_size := (u256 32)
      let required_size := (memory_required_size offset_word word_size)
      let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
      let (expansion_gas_charged, g2) ← do (charge g1 expansion_cost)
      if _sailIf1 : ((! expansion_gas_charged) : Bool) = true
      then
        (pure ((top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word word_size)
          let ⟨_, ⟨_, mem1⟩⟩ ← do
            (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
          let value ← do (mem_load access.range.off)
          (pure ((← do
              (push_word top1 value)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
            (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
            (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))))

/-- Implements `MSTORE`. -/
/- Type quantifiers: k_ex552747_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552747_ -/
def execute_mstore (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (offset_word, top1) ← do (pop top)
      let (v, top2) ← do (pop top1)
      let word_size := (u256 32)
      let required_size := (memory_required_size offset_word word_size)
      let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
      let (expansion_gas_charged, g2) ← do (charge g1 expansion_cost)
      if _sailIf1 : ((! expansion_gas_charged) : Bool) = true
      then
        (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word word_size)
          let ⟨_, ⟨_, mem1⟩⟩ ← do
            (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
          (mem_store access.range.off v)
          (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
            (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
            (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))))

/-- Implements `MSTORE8`. -/
/- Type quantifiers: k_ex552756_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552756_ -/
def execute_mstore8 (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (offset_word, top1) ← do (pop top)
      let (v, top2) ← do (pop top1)
      let required_size := (memory_required_size offset_word WORD_ONE)
      let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
      let (expansion_gas_charged, g2) ← do (charge g1 expansion_cost)
      if _sailIf1 : ((! expansion_gas_charged) : Bool) = true
      then
        (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word WORD_ONE)
          let ⟨_, ⟨_, mem1⟩⟩ ← do
            (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
          (mem_store_byte access.range.off v)
          (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
            (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
            (mem_dependentWitness0 : Nat) =>
            (Sigma fun (mem_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))))

/-- Implements `MSIZE`. -/
/- Type quantifiers: k_ex552765_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552765_ -/
def execute_msize (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (gas_charged, g1) ← do (charge g G_base)
  if _sailIf0 : ((! gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let high_water := (memory_high_water ⟨_, ⟨_, mem⟩⟩)
      let words := (memory_word_count high_water)
      let size ← do (word_of_nat_byte_count (words *i 32))
      (pure ((← do
          (push_word top size)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))

/-- Implements overlapping memory copy `MCOPY` (EIP-5656). -/
/- Type quantifiers: k_ex552774_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552774_ -/
def execute_mcopy (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (base_gas_charged, g1) ← do (charge g G_verylow)
  if _sailIf0 : ((! base_gas_charged) : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let (destination_word, top1) ← do (pop top)
      let (source_word, top2) ← do (pop top1)
      let (length_word, top3) ← do (pop top2)
      let (copy_gas_charged, g2) ← do (charge_copy_gas g1 length_word)
      if _sailIf1 : ((! copy_gas_charged) : Bool) = true
      then
        (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
      else
        (do
          let destination_required := (memory_required_size destination_word length_word)
          let source_required := (memory_required_size source_word length_word)
          let required_size :=
            if ((destination_required <b source_required) : Bool)
            then source_required
            else destination_required
          let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
          let (expansion_gas_charged, g3) ← do (charge g2 expansion_cost)
          if _sailIf2 : ((! expansion_gas_charged) : Bool) = true
          then
            (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat)))
          else
            (do
              let ⟨_, ⟨_, ⟨_, destination⟩⟩⟩ ← do
                (memory_access destination_word length_word)
              let ⟨_, ⟨_, ⟨_, source⟩⟩⟩ ← do (memory_access source_word length_word)
              let materialized_required_size :=
                if ((destination.required_size <b source.required_size) : Bool)
                then source.required_size
                else destination.required_size
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory ⟨_, ⟨_, mem⟩⟩ materialized_required_size)
              (mem_mcopy destination.range.off source.range.off destination.range.len)
              (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
                (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat))))))

/-- Implements `SLOAD`, including warm/cold access gas. -/
/- Type quantifiers: k_ex552775_ : Nat, 0 ≤ k_ex552775_ -/
def execute_sload (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (s, top1) ← do (pop top)
  let address ← do (self_addr ())
  let warm ← do (k_slot_is_warm address s)
  let gas_cost ← do (sload_cost warm)
  let (gas_charged, g1) ← do (charge g gas_cost)
  if ((! gas_charged) : Bool)
  then (pure (top1, g1))
  else
    (do
      (k_slot_mark_warm address s)
      let entry ← do (k_sload address s)
      (pure ((← (push_word top1 entry.curr)), g1)))

/-- Implements `SSTORE`, including fork-specific gas and refunds. -/
/- Type quantifiers: k_ex552776_ : Nat, 0 ≤ k_ex552776_ -/
def execute_sstore (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := SailME.run do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let (write_protected, g0) ← do (guard_static g)
  if (write_protected : Bool)
  then (pure (top, g0))
  else
    (do
      if (((profile.fork <b Amsterdam) && (g0 ≤b G_callstipend)) : Bool)
      then (pure (top, (← (exc_halt g0 OutOfGas))))
      else
        (do
          let (s, top1) ← do (pop top)
          let (v, top2) ← do (pop top1)
          let address ← do (self_addr ())
          let warm ← do (k_slot_is_warm address s)
          let cold := (! warm)
          if ((profile.fork ≥b Amsterdam) : Bool)
          then
            (do
              let sentry_cost := (sstore_sentry_cost cold)
              let (sentry_affordable, sentry_gas) ← do (check_execution_gas g0 sentry_cost)
              if ((! sentry_affordable) : Bool)
              then SailME.throw ((top2, sentry_gas) : ((BitVec 64) × Nat))
              else (pure ()))
          else (pure ())
          (k_slot_mark_warm address s)
          let entry ← do (k_sload address s)
          let costs ← do (sstore_costs entry.orig entry.curr v cold)
          let g1 ← (( do
            if ((costs.state_credit != 0) : Bool)
            then (credit_state_gas_refund g0 costs.state_credit)
            else (pure g0) ) : SailME ((BitVec 64) × Nat) Nat )
          let (execution_gas_charged, g2) ← do (charge g1 costs.execution)
          if ((! execution_gas_charged) : Bool)
          then (pure (top2, g2))
          else
            (do
              let (state_gas_charged, g3) ← do (charge_state_gas g2 costs.state_charge)
              if ((! state_gas_charged) : Bool)
              then (pure (top2, g3))
              else
                (do
                  if ((! (costs.refund == GAS_REFUND_ZERO)) : Bool)
                  then (record_refund costs.refund)
                  else (pure ())
                  if ((entry.curr != v) : Bool)
                  then
                    (k_sstore address s
                      { curr := v,
                        orig := entry.orig })
                  else (pure ())
                  (pure (top2, g3))))))

/-- Implements transient-storage load `TLOAD` (EIP-1153). -/
/- Type quantifiers: k_ex552777_ : Nat, 0 ≤ k_ex552777_ -/
def execute_tload (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_warm_access)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let (s, top1) ← do (pop top)
      let address ← do (self_addr ())
      let value ← do (k_tload address s)
      (pure ((← (push_word top1 value)), g1)))

/-- Implements transient-storage write `TSTORE` (EIP-1153). -/
/- Type quantifiers: k_ex552778_ : Nat, 0 ≤ k_ex552778_ -/
def execute_tstore (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (write_protected, g0) ← do (guard_static g)
  if (write_protected : Bool)
  then (pure (top, g0))
  else
    (do
      let (gas_charged, g1) ← do (charge g0 G_warm_access)
      if ((! gas_charged) : Bool)
      then (pure (top, g1))
      else
        (do
          let (s, top1) ← do (pop top)
          let (v, top2) ← do (pop top1)
          let address ← do (self_addr ())
          (k_tstore address s v)
          (pure (top2, g1))))

/-- Implements unconditional `JUMP`. -/
/- Type quantifiers: k_ex552780_ : Nat, k_ex552779_ : Nat, 0 ≤ k_ex552779_ ∧
  k_ex552779_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552780_ -/
def execute_jump (pc_in : Nat) (top : (BitVec 64)) (g : Nat) : SailM (Nat × (BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_mid)
  if ((! gas_charged) : Bool)
  then (pure (pc_in, top, g1))
  else
    (do
      let (dest, top1) ← do (pop top)
      let (next_pc, g2) ← do (do_jump pc_in g1 dest)
      (pure (next_pc, top1, g2)))

/-- Implements conditional `JUMPI`. -/
/- Type quantifiers: k_ex552782_ : Nat, k_ex552781_ : Nat, 0 ≤ k_ex552781_ ∧
  k_ex552781_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552782_ -/
def execute_jumpi (pc_in : Nat) (top : (BitVec 64)) (g : Nat) : SailM (Nat × (BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_high)
  if ((! gas_charged) : Bool)
  then (pure (pc_in, top, g1))
  else
    (do
      let (dest, top1) ← do (pop top)
      let (cond, top2) ← do (pop top1)
      let condition_is_zero := (word_is_zero cond)
      if (condition_is_zero : Bool)
      then (pure (pc_in, top2, g1))
      else
        (do
          let (next_pc, g2) ← do (do_jump pc_in g1 dest)
          (pure (next_pc, top2, g2))))

/-- Implements `PC`, returning the current opcode position from the
carried, already-advanced program counter. -/
/- Type quantifiers: k_ex552784_ : Nat, k_ex552783_ : Nat, 0 ≤ k_ex552783_ ∧
  k_ex552783_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552784_ -/
def execute_pc (pc_in : Nat) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let next_pc ← do (word_of_source_byte_count pc_in)
      let opcode_pc := (alu_sub next_pc WORD_ONE)
      (pure ((← (push_word top opcode_pc)), g1)))

/-- Implements `GAS`, returning the carried gas remaining after its own
charge. -/
/- Type quantifiers: k_ex552785_ : Nat, 0 ≤ k_ex552785_ -/
def execute_gas (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_base)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else (pure ((← (push_gas top g1)), g1))

/-- Implements `JUMPDEST`. -/
/- Type quantifiers: g : Nat, 0 ≤ g -/
def execute_jumpdest (g : Nat) : SailM Nat := do
  let (_, g1) ← do (charge g G_jumpdest)
  (pure g1)

/-- Implements the `PUSH0` through `PUSH32` family. -/
/- Type quantifiers: k_ex552789_ : Nat, k_ex552788_ : Nat, k_ex552787_ : Nat, 0 ≤ k_ex552787_ ∧
  k_ex552787_ ≤ 32, 0 ≤ k_ex552788_ ∧ k_ex552788_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex552789_ -/
def execute_push (n : Nat) (v : Nat) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  if ((n == 0) : Bool)
  then
    (do
      let (gas_charged, g1) ← do (charge g G_base)
      if ((! gas_charged) : Bool)
      then (pure (top, g1))
      else (pure ((← (push_word top v)), g1)))
  else
    (do
      let (gas_charged, g1) ← do (charge g G_verylow)
      if ((! gas_charged) : Bool)
      then (pure (top, g1))
      else (pure ((← (push_word top v)), g1)))

/-- Implements the `DUP1` through `DUP16` family. -/
/- Type quantifiers: k_ex552791_ : Nat, k_ex552790_ : Nat, 1 ≤ k_ex552790_ ∧ k_ex552790_ ≤ 16, 0
  ≤ k_ex552791_ -/
def execute_dup (n : Nat) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let value ← do (peek top (n - 1))
      (pure ((← (push_word top value)), g1)))

/-- Implements the `SWAP1` through `SWAP16` family. -/
/- Type quantifiers: k_ex552793_ : Nat, k_ex552792_ : Nat, 1 ≤ k_ex552792_ ∧ k_ex552792_ ≤ 16, 0
  ≤ k_ex552793_ -/
def execute_swap (n : Nat) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let (gas_charged, g1) ← do (charge g G_verylow)
  if ((! gas_charged) : Bool)
  then (pure (top, g1))
  else
    (do
      let top_value ← do (peek top 0)
      let other ← do (peek top n)
      (stack_set top 0 other)
      (stack_set top n top_value)
      (pure (top, g1)))

/-- Implements immediate deep-stack duplication `DUPN`. -/
/- Type quantifiers: k_ex552794_ : Nat, 0 ≤ k_ex552794_ -/
def execute_dupn (immediate : (BitVec 8)) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let valid_immediate := (deep_stack_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (top, (← (exc_halt g InvalidOpcode))))
  else
    (do
      let (gas_charged, g1) ← do (charge g G_verylow)
      if ((! gas_charged) : Bool)
      then (pure (top, g1))
      else
        (do
          let n ← do (decode_single_stack_index immediate)
          let value ← do (peek top (n - 1))
          (pure ((← (push_word top value)), g1))))

/-- Implements immediate deep-stack exchange `SWAPN`. -/
/- Type quantifiers: k_ex552795_ : Nat, 0 ≤ k_ex552795_ -/
def execute_swapn (immediate : (BitVec 8)) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let valid_immediate := (deep_stack_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (top, (← (exc_halt g InvalidOpcode))))
  else
    (do
      let (gas_charged, g1) ← do (charge g G_verylow)
      if ((! gas_charged) : Bool)
      then (pure (top, g1))
      else
        (do
          let n ← do (decode_single_stack_index immediate)
          let top_value ← do (peek top 0)
          let other ← do (peek top n)
          (stack_set top 0 other)
          (stack_set top n top_value)
          (pure (top, g1))))

/-- Implements immediate pairwise deep-stack `EXCHANGE`. -/
/- Type quantifiers: k_ex552796_ : Nat, 0 ≤ k_ex552796_ -/
def execute_exchange (immediate : (BitVec 8)) (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := do
  let valid_immediate := (exchange_immediate_valid immediate)
  if ((! valid_immediate) : Bool)
  then (pure (top, (← (exc_halt g InvalidOpcode))))
  else
    (do
      let (gas_charged, g1) ← do (charge g G_verylow)
      if ((! gas_charged) : Bool)
      then (pure (top, g1))
      else
        (do
          let (n, m) ← do (decode_exchange_stack_indices immediate)
          let first ← do (peek top n)
          let second ← do (peek top m)
          (stack_set top n second)
          (stack_set top m first)
          (pure (top, g1))))

/-- Implements the `LOG0` through `LOG4` family. -/
/- Type quantifiers: k_ex552806_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552801_
  : Nat, 0 ≤ k_ex552801_ ∧ k_ex552801_ ≤ 4, 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552806_ -/
def execute_log (n : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (write_protected, g0) ← do (guard_static g)
  if _sailIf0 : (write_protected : Bool) = true
  then
    (pure ((top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g0 : Nat)))
  else
    (do
      let (offset_word, top1) ← do (pop top)
      let (length_word, top2) ← do (pop top1)
      let (topics, top3) ← do (pop_log_topics n top2)
      let (log_gas_charged, g1) ← do (charge_log_gas g0 n length_word)
      if _sailIf1 : ((! log_gas_charged) : Bool) = true
      then
        (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
      else
        (do
          let required_size := (memory_required_size offset_word length_word)
          let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
          let (expansion_gas_charged, g2) ← do (charge g1 expansion_cost)
          if _sailIf2 : ((! expansion_gas_charged) : Bool) = true
          then
            (pure ((top3 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
          else
            (do
              let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
              let range := access.range
              let (data, mem2) ← do
                (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ range.off range.len)
              let address ← do (self_addr ())
              let memory_slice := (evm_memory_slice ((data).2).2.bytes ((data).2).2.len)
              let log_data := (LogDataMemory ⟨_, ⟨_, memory_slice⟩⟩)
              (k_log address topics log_data)
              (pure ((top3 : (BitVec 64)), (mem2 : (Sigma fun (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat))))))

/-- Implements normal `STOP`. -/
def execute_stop (_ : Unit) : SailM Unit := do
  let reason := (HaltStop ())
  writeReg frame_status (Halted reason)

/-- Implements successful `RETURN`. -/
/- Type quantifiers: k_ex552815_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552815_ -/
def execute_return (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (offset_word, top1) ← do (pop top)
  let (length_word, top2) ← do (pop top1)
  let required_size := (memory_required_size offset_word length_word)
  let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
  let (expansion_gas_charged, g1) ← do (charge g expansion_cost)
  if _sailIf0 : ((! expansion_gas_charged) : Bool) = true
  then
    (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
      let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
      let range := access.range
      let (data, mem2) ← do (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ range.off range.len)
      let ⟨_, ⟨_, output⟩⟩ ← do (freeze_memory_output data)
      let reason := (HaltReturn ⟨_, ⟨_, output⟩⟩)
      writeReg frame_status (Halted reason)
      (pure ((top2 : (BitVec 64)), (mem2 : (Sigma fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))

/-- Implements rollback-carrying `REVERT` (EIP-140). -/
/- Type quantifiers: k_ex552824_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, 0
  ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552824_ -/
def execute_revert (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM ((BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (offset_word, top1) ← do (pop top)
  let (length_word, top2) ← do (pop top1)
  let required_size := (memory_required_size offset_word length_word)
  let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
  let (expansion_gas_charged, g1) ← do (charge g expansion_cost)
  if _sailIf0 : ((! expansion_gas_charged) : Bool) = true
  then
    (pure ((top2 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat)
      =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
  else
    (do
      let ⟨_, ⟨_, ⟨_, access⟩⟩⟩ ← do (memory_access offset_word length_word)
      let ⟨_, ⟨_, mem1⟩⟩ ← do (expand_memory ⟨_, ⟨_, mem⟩⟩ access.required_size)
      let range := access.range
      let g2 ← do (refill_frame_state_gas g1)
      let (data, mem2) ← do (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ range.off range.len)
      let ⟨_, ⟨_, output⟩⟩ ← do (freeze_memory_output data)
      let reason := (HaltRevert ⟨_, ⟨_, output⟩⟩)
      writeReg frame_status (Halted reason)
      (pure ((top2 : (BitVec 64)), (mem2 : (Sigma fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat))))

/-- Implements exceptional invalid-opcode termination: all carried gas is
consumed before the exceptional halt. -/
/- Type quantifiers: g : Nat, 0 ≤ g -/
def execute_invalid (g : Nat) : SailM Nat := do
  let consumed := (gas_sub g g)
  (exc_halt consumed InvalidOpcode)

/-- Implements `SELFDESTRUCT` with fork-specific semantics. -/
/- Type quantifiers: k_ex552826_ : Nat, 0 ≤ k_ex552826_ -/
def execute_selfdestruct (top : (BitVec 64)) (g : Nat) : SailM ((BitVec 64) × Nat) := SailME.run do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let (write_protected, g0) ← do (guard_static g)
  if (write_protected : Bool)
  then (pure (top, g0))
  else
    (do
      let (beneficiary_word, top1) ← do (pop top)
      let beneficiary := (word_to_address beneficiary_word)
      let address ← do (self_addr ())
      if ((profile.fork ≥b Amsterdam) : Bool)
      then
        (do
          let warm ← do (k_account_is_warm beneficiary)
          let access_cost :=
            ((0 + G_selfdestruct) + (if (warm : Bool)
              then G_zero
              else G_amsterdam_cold_account_access))
          let (execution_affordable, sentry_gas) ← do (check_execution_gas g0 access_cost)
          if ((! execution_affordable) : Bool)
          then (pure (top1, sentry_gas))
          else
            (do
              (k_account_mark_warm beneficiary)
              let bal ← do (k_get_balance address)
              let nonzero_balance := (word_nonzero bal)
              let beneficiary_empty ← do (k_account_is_empty beneficiary)
              let creates_account := (nonzero_balance && beneficiary_empty)
              let execution_cost :=
                if (creates_account : Bool)
                then (access_cost + G_amsterdam_account_write)
                else access_cost
              let (execution_gas_charged, g1) ← do (charge g0 execution_cost)
              if ((! execution_gas_charged) : Bool)
              then (pure (top1, g1))
              else
                (do
                  let g2 : Nat := g1
                  let g2 ← (( do
                    if (creates_account : Bool)
                    then
                      (do
                        let (state_gas_charged, state_gas) ← do
                          (charge_state_gas g1 G_amsterdam_state_new_account)
                        let g2 : Nat := state_gas
                        if ((! state_gas_charged) : Bool)
                        then SailME.throw ((top1, state_gas) : ((BitVec 64) × Nat))
                        else (pure ())
                        (pure g2))
                    else (pure g2) ) : SailME ((BitVec 64) × Nat) Nat )
                  (k_transfer address beneficiary bal)
                  let created ← do (k_was_created address)
                  if (created : Bool)
                  then (k_selfdestruct address)
                  else (pure ())
                  let reason := (HaltSelfDestruct ())
                  writeReg frame_status (Halted reason)
                  (pure (top1, g2)))))
      else
        (do
          let bal ← do (k_get_balance address)
          let warm ← do (k_account_is_warm beneficiary)
          let (selfdestruct_gas_charged, g1) ← do (charge g0 G_selfdestruct)
          if ((! selfdestruct_gas_charged) : Bool)
          then (pure (top1, g1))
          else
            (do
              let g2 : Nat := g1
              let g2 ← (( do
                if ((! warm) : Bool)
                then
                  (do
                    let (cold_account_gas_charged, cold_gas) ← do (charge g1 G_cold_account)
                    let g2 : Nat := cold_gas
                    if ((! cold_account_gas_charged) : Bool)
                    then SailME.throw ((top1, cold_gas) : ((BitVec 64) × Nat))
                    else (pure ())
                    (pure g2))
                else (pure g2) ) : SailME ((BitVec 64) × Nat) Nat )
              (k_account_mark_warm beneficiary)
              let nonzero_balance := (word_nonzero bal)
              let beneficiary_empty ← do (k_account_is_empty beneficiary)
              let g3 : Nat := g2
              let g3 ← (( do
                if ((nonzero_balance && beneficiary_empty) : Bool)
                then
                  (do
                    let (new_account_gas_charged, new_account_gas) ← do (charge g2 G_newaccount)
                    let g3 : Nat := new_account_gas
                    if ((! new_account_gas_charged) : Bool)
                    then SailME.throw ((top1, new_account_gas) : ((BitVec 64) × Nat))
                    else (pure ())
                    (pure g3))
                else (pure g3) ) : SailME ((BitVec 64) × Nat) Nat )
              let is_selfdestructed ← do (k_is_selfdestructed address)
              let first_selfdestruct := (! is_selfdestructed)
              if (((profile.fork <b London) && first_selfdestruct) : Bool)
              then (record_refund R_selfdestruct_pre_london)
              else (pure ())
              (k_transfer address beneficiary bal)
              if ((profile.fork <b Cancun) : Bool)
              then
                (do
                  (k_zero_balance address)
                  (k_selfdestruct address))
              else
                (do
                  let created ← do (k_was_created address)
                  if (created : Bool)
                  then
                    (do
                      (k_zero_balance address)
                      (k_selfdestruct address))
                  else (pure ()))
              let reason := (HaltSelfDestruct ())
              writeReg frame_status (Halted reason)
              (pure (top1, g3)))))

/-- Defunctionalizes each CREATE-family opcode into the data consumed by the
shared contract-creation interpreter. -/
def create_semantics (kind : CreateKind) : CreateSemantics :=
  match kind with
  | .CreateByNonce => { uses_salt := false }
  | .CreateBySalt => { uses_salt := true }

/-- `CREATE` (`CreateByNonce`) and `CREATE2` (`CreateBySalt`, EIP-1014).
Both deploy a new contract by running the initcode supplied in
memory; the new address derives from `(creator, nonce)` for `CREATE`
or `(creator, salt, keccak256(initcode))` for `CREATE2`. Operand
layout: `value`, `offset`, `length`, `salt` (for `CREATE2`). Pushes
the new address on success, 0 on failure. Takes the parent's carried
machine state; returns the parent's updated state on the non-entering
paths and the freshly installed child's state after a frame entry. -/
/- Type quantifiers: k_ex552836_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552831_
  : Nat, 0 ≤ k_ex552831_ ∧ k_ex552831_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552836_ -/
def run_create (kind : CreateKind) (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat)
  => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := SailME.run do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let semantics := (create_semantics kind)
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let current_depth ← do readReg call_depth
  let creator ← do (self_addr ())
  let (value, top1) ← do (pop top)
  let (off_word, top2) ← do (pop top1)
  let (len_word, top3) ← do (pop top2)
  let (salt, top4) ← do
    if (semantics.uses_salt : Bool)
    then (pop top3)
    else (pure (WORD_ZERO, top3))
  let (static_violation, g0) ← do (guard_static g)
  if _sailIf0 : (static_violation : Bool) = true
  then
    (pure ((pc_in : Nat), (top4 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g0 : Nat)))
  else
    (do
      let required_size := (memory_required_size off_word len_word)
      let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
      let (expansion_affordable, g1) ← do (charge g0 expansion_cost)
      let expansion_out_of_gas := (! expansion_affordable)
      if _sailIf1 : (expansion_out_of_gas : Bool) = true
      then
        (pure ((pc_in : Nat), (top4 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
      else
        (do
          let ⟨_, ⟨_, ⟨_, initcode_access⟩⟩⟩ ← do (memory_access off_word len_word)
          let ⟨_, ⟨_, mem1⟩⟩ ← do
            (expand_memory ⟨_, ⟨_, mem⟩⟩ initcode_access.required_size)
          let initcode := initcode_access.range
          let access_cost ← do (create_access_cost ())
          let (access_affordable, g2) ← do (charge g1 access_cost)
          let access_out_of_gas := (! access_affordable)
          if _sailIf2 : (access_out_of_gas : Bool) = true
          then
            (pure ((pc_in : Nat), (top4 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
          else
            (do
              let g3 : Nat := g2
              let initcode_word_count := (memory_word_count_word len_word)
              let g3 ← (( do
                if ((profile.fork ≥b Shanghai) : Bool)
                then
                  (do
                    let (initcode_charge_affordable, initcode_gas) ← do
                      (charge_word_scaled_gas g3 G_initcode_word initcode_word_count)
                    let g3 : Nat := initcode_gas
                    let initcode_charge_out_of_gas := (! initcode_charge_affordable)
                    if (initcode_charge_out_of_gas : Bool)
                    then
                      SailME.throw ((pc_in : Nat), (top4 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma
                        fun (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                        fun (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (initcode_gas : Nat))
                    else (pure ())
                    (pure g3))
                else (pure g3) ) : SailME
                (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) Nat )
              let g3 ← (( do
                if (semantics.uses_salt : Bool)
                then
                  (do
                    let (hashing_affordable, hashing_gas) ← do
                      (charge_word_scaled_gas g3 G_keccak_word initcode_word_count)
                    let g3 : Nat := hashing_gas
                    let hashing_out_of_gas := (! hashing_affordable)
                    if (hashing_out_of_gas : Bool)
                    then
                      SailME.throw ((pc_in : Nat), (top4 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma
                        fun (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                        fun (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (hashing_gas : Nat))
                    else (pure ())
                    (pure g3))
                else (pure g3) ) : SailME
                (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) Nat )
              let valid_initcode_size ← do (initcode_size_allowed initcode.len)
              let invalid_initcode_size := (! valid_initcode_size)
              if _sailIf3 : (invalid_initcode_size : Bool) = true
              then
                (do
                  (pure ((pc_in : Nat), (top4 : (BitVec 64)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                    (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                    fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (← do
                      (exc_halt g3 InitCodeTooLarge)))))
              else
                (do
                  let nonce ← do (k_get_nonce creator)
                  let mem2 : (Sigma fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) :=
                    ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                    fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))))
                  let (new_addr, mem2) ← (( do
                    if _sailIf4 : (semantics.uses_salt : Bool) = true
                    then
                      (do
                        let (initcode_digest_word, hashed_mem) ← do
                          (mem_keccak ⟨_, ⟨_, mem1⟩⟩ ⟨_, ⟨_, initcode⟩⟩)
                        let ⟨_, ⟨_, mem2⟩⟩ : (Sigma fun (mem_dependentWitness0 : Nat) =>
                          (Sigma fun (mem_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) :=
                          (hashed_mem : (Sigma fun (mem_dependentWitness0 : Nat) =>
                          (Sigma fun (mem_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))))
                        let initcode_digest := (word_to_hash initcode_digest_word)
                        (pure ((← do
                            (k_create2_addr creator salt initcode_digest)), ((⟨_, ⟨_, mem2⟩⟩ : (Sigma
                          fun (mem_dependentWitness0 : Nat) =>
                          (Sigma fun (mem_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                          fun (mem_dependentWitness0 : Nat) =>
                          (Sigma fun (mem_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))))))
                    else
                      (do
                        (pure ((← do
                            (k_create_addr creator nonce)), (mem2 : (Sigma fun
                          (mem_dependentWitness0 : Nat) =>
                          (Sigma fun (mem_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))))))) )
                    : SailME
                    (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                    ((Vector (BitVec 8) 20) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) )
                  let g4 : Nat := g3
                  let child_gas : Nat := GAS_ZERO
                  let (child_gas, g4) : (Nat × Nat) :=
                    if ((profile.fork <b Amsterdam) : Bool)
                    then
                      (let avail := g4
                      let retained_gas : Nat := (avail / 64)
                      let child_gas : Nat := (gas_sub avail retained_gas)
                      let g4 : Nat := retained_gas
                      (child_gas, g4))
                    else (child_gas, g4)
                  if _sailIf4 : ((← readReg message).is_static : Bool) = true
                  then
                    (do
                      (pure ((pc_in : Nat), (top4 : (BitVec 64)), (mem2 : (Sigma fun
                        (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (← do
                          (exc_halt g4 WriteProtection)))))
                  else
                    (do
                      let creator_balance ← do (k_get_balance creator)
                      let endowment_affordable := (word_ule value creator_balance)
                      let insufficient_balance := (! endowment_affordable)
                      let nonce_limit := ((2 ^i 64) - 1)
                      let depth_limit := 1024
                      if _sailIf5 : ((insufficient_balance || ((nonce == nonce_limit) || (current_depth == depth_limit))) : Bool) = true
                      then
                        (do
                          (returndata_clear ())
                          let g5 : Nat :=
                            if ((profile.fork <b Amsterdam) : Bool)
                            then (refund_gas g4 child_gas)
                            else g4
                          (pure ((pc_in : Nat), (← do
                              (push_word top4 WORD_ZERO)), (mem2 : (Sigma fun
                            (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g5 : Nat))))
                      else
                        (do
                          let child_depth : Nat := (current_depth + 1)
                          (k_account_mark_warm new_addr)
                          let new_account_charged ← do
                            if ((profile.fork ≥b Amsterdam) : Bool)
                            then
                              (do
                                (k_account_is_empty new_addr))
                            else (pure false)
                          let g4 ← (( do
                            if (new_account_charged : Bool)
                            then
                              (do
                                let (state_gas_affordable, state_gas) ← do
                                  (charge_state_gas g4 G_amsterdam_state_new_account)
                                let g4 : Nat := state_gas
                                let state_gas_out_of_gas := (! state_gas_affordable)
                                if (state_gas_out_of_gas : Bool)
                                then
                                  SailME.throw ((pc_in : Nat), (top4 : (BitVec 64)), (mem2 : (Sigma
                                    fun (mem_dependentWitness0 : Nat) =>
                                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (state_gas : Nat))
                                else (pure ())
                                (pure g4))
                            else (pure g4) ) : SailME
                            (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                            Nat )
                          let (child_gas, g4) : (Nat × Nat) :=
                            if ((profile.fork ≥b Amsterdam) : Bool)
                            then
                              (let avail := g4
                              let retained_gas : Nat := (avail / 64)
                              let child_gas : Nat := (gas_sub avail retained_gas)
                              let g4 : Nat := retained_gas
                              (child_gas, g4))
                            else (child_gas, g4)
                          let occupied ← (( do (k_account_occupied new_addr) ) : SailME
                            (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                            Bool )
                          (returndata_clear ())
                          (k_bump_nonce creator)
                          if _sailIf6 : (occupied : Bool) = true
                          then
                            (do
                              let g5 ← (( do
                                if (new_account_charged : Bool)
                                then (credit_state_gas_refund g4 G_amsterdam_state_new_account)
                                else (pure g4) ) : SailME
                                (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                                (Sigma fun (mem_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                                Nat )
                              (pure ((pc_in : Nat), (← do
                                  (push_word top4 WORD_ZERO)), (mem2 : (Sigma fun
                                (mem_dependentWitness0 : Nat) =>
                                (Sigma fun (mem_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g5 : Nat))))
                          else
                            (do
                              let (initcode_bytes, mem3) ← do
                                (memory_code_slice mem2 initcode.off initcode.len)
                              let child_code_id ← do (code_db_insert initcode_bytes profile.fork)
                              let ⟨_, ⟨_, child_code⟩⟩ ← do
                                (code_db_resolve child_code_id)
                              let child_state_gas ← do readReg state_gas_remaining
                              writeReg pc pc_in
                              writeReg gas_remaining g4
                              writeReg stack_top top4
                              writeReg evm_memory mem3
                              let checkpoint ← do
                                (pure { (← (suspend_frame ())) with state_gas_remaining := GAS_ZERO })
                              let create_continuation : CreateContinuation :=
                                { checkpoint := checkpoint,
                                  address := new_addr,
                                  new_account_charged := new_account_charged }
                              let continuation := (ResumeCreate create_continuation)
                              (frame_stack_push continuation)
                              (k_mark_created new_addr)
                              (k_clear_storage new_addr)
                              (k_bump_nonce new_addr)
                              (k_transfer creator new_addr value)
                              writeReg message { caller := creator,
                                                 address := new_addr,
                                                 code_address := new_addr,
                                                 value := value,
                                                 state_gas_reservoir := child_state_gas,
                                                 is_static := checkpoint.message.is_static,
                                                 depth := child_depth }
                              (calldata_install EMPTY_CALLDATA)
                              writeReg pc 0
                              writeReg gas_remaining child_gas
                              writeReg state_gas_remaining child_state_gas
                              writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
                              writeReg frame_status (Running ())
                              (returndata_clear ())
                              writeReg frame_code ⟨_, ⟨_, child_code⟩⟩
                              writeReg call_depth child_depth
                              writeReg frame_refund GAS_REFUND_ZERO
                              (pure ((0 : Nat), (← do
                                  readReg stack_top), (← do
                                  readReg evm_memory), (child_gas : Nat))))))))))

/-- Implements `CREATE` through the common create runner. -/
/- Type quantifiers: k_ex552846_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552841_
  : Nat, 0 ≤ k_ex552841_ ∧ k_ex552841_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552846_ -/
def execute_create (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  (run_create CreateByNonce pc_in top ⟨_, ⟨_, mem⟩⟩ g)

/-- Implements `CREATE2` through the common create runner. -/
/- Type quantifiers: k_ex552856_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552851_
  : Nat, 0 ≤ k_ex552851_ ∧ k_ex552851_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552856_ -/
def execute_create2 (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  (run_create CreateBySalt pc_in top ⟨_, ⟨_, mem⟩⟩ g)

/-- Defunctionalizes each CALL-family opcode into the data consumed by the
shared message-call interpreter. -/
def call_semantics (kind : CallKind) : CallSemantics :=
  match kind with
  | .Call =>
    { takes_value := true,
      transfers_value := true,
      uses_target_address := true,
      inherits_caller_and_value := false,
      enters_static_context := false }
  | .CallCode =>
    { takes_value := true,
      transfers_value := false,
      uses_target_address := false,
      inherits_caller_and_value := false,
      enters_static_context := false }
  | .DelegateCall =>
    { takes_value := false,
      transfers_value := false,
      uses_target_address := false,
      inherits_caller_and_value := true,
      enters_static_context := false }
  | .StaticCall =>
    { takes_value := false,
      transfers_value := false,
      uses_target_address := true,
      inherits_caller_and_value := false,
      enters_static_context := true }

/-- Selects the code a frame actually executes (EIP-7702). A delegated
account runs the code at its delegation target, following exactly one
hop; a delegation whose target is a precompile (or has no code)
executes as empty code. An undelegated account runs its own code. -/
/- Type quantifiers: k_ex552860_ : Bool -/
def executable_code (target : (Vector (BitVec 8) 20)) (dele : Bool) (dtgt : (Vector (BitVec 8) 20)) : SailM (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) := do
  if _sailIf0 : (dele : Bool) = true
  then
    (do
      let delegate_key ← do (k_code_key dtgt)
      let ⟨_, ⟨_, delegate_code⟩⟩ ← do (code_db_resolve delegate_key)
      let delegate_precompile ← do (precompile_id_for_address dtgt)
      if _sailIf1 : ((bne delegate_precompile NotPrecompile) : Bool) = true
      then
        (pure ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))))
      else
        (pure ((⟨_, ⟨_, delegate_code⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))))))
  else
    (do
      let target_key ← do (k_code_key target)
      (code_db_resolve target_key))

/-- The four call opcodes, multiplexed on `mode`.

- `0` — `CALL`: a new frame at `target`, may transfer value.
- `1` — `CALLCODE`: runs the target's code in the caller's account,
  may transfer.
- `2` — `DELEGATECALL` (EIP-7): runs the target's code in the
  caller's account, inheriting the parent's caller/value/static
  context.
- `3` — `STATICCALL` (EIP-214): `CALL` with value 0 and a forced
  static context.

Operand layout (top of stack first): `gas`, `target`, `value` (for
`CALL`/`CALLCODE`), `argsOffset`, `argsLen`, `retOffset`, `retLen`.
Pushes 1 on success, 0 on failure. Takes the parent's carried machine
state; returns the parent's updated state on the non-entering paths
and the freshly installed child's state after a frame entry. -/
/- Type quantifiers: k_ex552870_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552865_
  : Nat, 0 ≤ k_ex552865_ ∧ k_ex552865_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552870_ -/
def run_call (kind : CallKind) (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := SailME.run do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let semantics := (call_semantics kind)
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let current_depth ← do readReg call_depth
  let caller ← do (self_addr ())
  let (gas_request, top1) ← do (pop top)
  let (target_word, top2) ← do (pop top1)
  let target := (word_to_address target_word)
  let (value, top3) ← (( do
    if (semantics.takes_value : Bool)
    then (pop top2)
    else (pure (WORD_ZERO, top2)) ) : SailME
    (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
    (Sigma fun (mem_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
    (Nat × (BitVec 64)) )
  let value_nonzero := (word_nonzero value)
  let (args_off_word, top4) ← do (pop top3)
  let (args_len_word, top5) ← do (pop top4)
  let (ret_off_word, top6) ← do (pop top5)
  let (ret_len_word, top7) ← do (pop top6)
  if _sailIf0 : ((← if (semantics.transfers_value : Bool)
       then
         (do
           if (value_nonzero : Bool)
           then
             (do
               (pure (← readReg message).is_static))
           else (pure false))
       else (pure false)) : Bool) = true
  then
    (do
      (pure ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (← do
          (exc_halt g WriteProtection)))))
  else
    (do
      let warm ← do (k_account_is_warm target)
      let target_cost ← (( do (account_cost warm) ) : SailME
        (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) Nat )
      let transfer_cost ← (( do
        if (value_nonzero : Bool)
        then (call_value_cost ())
        else (pure GAS_CONSTANT_ZERO) ) : SailME
        (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) Nat )
      let args_required := (memory_required_size args_off_word args_len_word)
      let ret_required := (memory_required_size ret_off_word ret_len_word)
      let required_size :=
        if ((args_required <b ret_required) : Bool)
        then ret_required
        else args_required
      let expansion_cost := (memory_expansion_cost ⟨_, ⟨_, mem⟩⟩ required_size)
      let (expansion_affordable, g1) ← do (charge g expansion_cost)
      let expansion_out_of_gas := (! expansion_affordable)
      if _sailIf1 : (expansion_out_of_gas : Bool) = true
      then
        (pure ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
          (mem_dependentWitness0 : Nat) =>
          (Sigma fun (mem_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))
      else
        (do
          let static_base : Nat := (target_cost + transfer_cost)
          let (static_base_affordable, g2) ← do (charge g1 static_base)
          let static_base_out_of_gas := (! static_base_affordable)
          if _sailIf2 : (static_base_out_of_gas : Bool) = true
          then
            (pure ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
              (mem_dependentWitness0 : Nat) =>
              (Sigma fun (mem_dependentWitness1 : Nat) =>
              (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g2 : Nat)))
          else
            (do
              (k_account_mark_warm target)
              let (tg_deleg, tg_target) ← do (k_deleg_target target)
              let delegation_cost ← (( do
                if (tg_deleg : Bool)
                then
                  (do
                    let dw ← do (k_account_is_warm tg_target)
                    (account_cost dw))
                else (pure GAS_CONSTANT_ZERO) ) : SailME
                (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                (Sigma fun (mem_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) Nat )
              let target_empty ← do (k_account_is_empty target)
              let new_account_charged :=
                ((profile.fork ≥b Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)))
              let create_cost : Nat :=
                if (((profile.fork <b Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty))) : Bool)
                then G_newaccount
                else GAS_CONSTANT_ZERO
              let additional_cost : Nat := (delegation_cost + create_cost)
              let (additional_cost_affordable, g3) ← do (charge g2 additional_cost)
              let additional_cost_out_of_gas := (! additional_cost_affordable)
              if _sailIf3 : (additional_cost_out_of_gas : Bool) = true
              then
                (pure ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
                  (mem_dependentWitness0 : Nat) =>
                  (Sigma fun (mem_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
                  (mem_dependentWitness0 : Nat) =>
                  (Sigma fun (mem_dependentWitness1 : Nat) =>
                  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g3 : Nat)))
              else
                (do
                  let stipend : Nat :=
                    if (value_nonzero : Bool)
                    then G_callstipend
                    else GAS_ZERO
                  let g4 : Nat := g3
                  let base_child : Nat := GAS_ZERO
                  let (base_child, g4) ← (( do
                    if ((profile.fork ≥b Amsterdam) : Bool)
                    then
                      (do
                        let g4 ← (( do
                          if (new_account_charged : Bool)
                          then
                            (do
                              let (state_gas_affordable, state_gas) ← do
                                (charge_state_gas g3 G_amsterdam_state_new_account)
                              let g4 : Nat := state_gas
                              let state_gas_out_of_gas := (! state_gas_affordable)
                              if (state_gas_out_of_gas : Bool)
                              then
                                SailME.throw ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma
                                  fun (mem_dependentWitness0 : Nat) =>
                                  (Sigma fun (mem_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                                  fun (mem_dependentWitness0 : Nat) =>
                                  (Sigma fun (mem_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (state_gas : Nat))
                              else (pure ())
                              (pure g4))
                          else (pure g4) ) : SailME
                          (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                          (Sigma fun (mem_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                          Nat )
                        let base_child : Nat := (call_gas_cap_word g4 gas_request)
                        let (child_gas_affordable, child_charged_gas) ← do (charge g4 base_child)
                        let g4 : Nat := child_charged_gas
                        let child_out_of_gas := (! child_gas_affordable)
                        if (child_out_of_gas : Bool)
                        then
                          SailME.throw ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma
                            fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                            fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (child_charged_gas : Nat))
                        else (pure ())
                        (pure (base_child, g4)))
                    else
                      (do
                        let base_child : Nat := (call_gas_cap_word g4 gas_request)
                        let (child_gas_affordable, child_charged_gas) ← do (charge g4 base_child)
                        let g4 : Nat := child_charged_gas
                        let child_out_of_gas := (! child_gas_affordable)
                        if (child_out_of_gas : Bool)
                        then
                          SailME.throw ((pc_in : Nat), (top7 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma
                            fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                            fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (child_charged_gas : Nat))
                        else (pure ())
                        (pure (base_child, g4))) ) : SailME
                    (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                    (Nat × Nat) )
                  if (tg_deleg : Bool)
                  then (k_account_mark_warm tg_target)
                  else (pure ())
                  if (tg_deleg : Bool)
                  then
                    (do
                      let delegate_key ← do (k_code_key tg_target)
                      let ⟨_, ⟨_, _⟩⟩ ← do (code_db_resolve delegate_key)
                      let _ ← do (k_aload tg_target)
                      (pure ()))
                  else (pure ())
                  let ⟨_, ⟨_, ⟨_, args_access⟩⟩⟩ ← do
                    (memory_access args_off_word args_len_word)
                  let ⟨_, ⟨_, ⟨_, ret_access⟩⟩⟩ ← do
                    (memory_access ret_off_word ret_len_word)
                  let materialized_required_size :=
                    if ((args_access.required_size <b ret_access.required_size) : Bool)
                    then ret_access.required_size
                    else args_access.required_size
                  let ⟨_, ⟨_, mem1⟩⟩ ← do
                    (expand_memory ⟨_, ⟨_, mem⟩⟩ materialized_required_size)
                  let args := args_access.range
                  let ret := ret_access.range
                  let child_gas : Nat := (conserved_gas_add base_child stipend)
                  let _ ← do (k_aload target)
                  let insufficient_balance ← (( do
                    if ((semantics.takes_value && value_nonzero) : Bool)
                    then
                      (do
                        let caller_balance ← do (k_get_balance caller)
                        let transfer_affordable := (word_ule value caller_balance)
                        (pure (! transfer_affordable)))
                    else (pure false) ) : SailME
                    (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                    Bool )
                  let depth_limit := 1024
                  if _sailIf4 : ((insufficient_balance || (current_depth == depth_limit)) : Bool) = true
                  then
                    (do
                      (returndata_clear ())
                      let g5 := (refund_gas g4 child_gas)
                      let g6 ← (( do
                        if (new_account_charged : Bool)
                        then (credit_state_gas_refund g5 G_amsterdam_state_new_account)
                        else (pure g5) ) : SailME
                        (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                        Nat )
                      (pure ((pc_in : Nat), (← do
                          (push_word top7 WORD_ZERO)), ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                        (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma
                        fun (mem_dependentWitness0 : Nat) =>
                        (Sigma fun (mem_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g6 : Nat))))
                  else
                    (do
                      let selected_precompile ← do (precompile_id_for_address target)
                      if _sailIf5 : ((bne selected_precompile NotPrecompile) : Bool) = true
                      then
                        (do
                          let (input_memory, mem2) ← do
                            (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ args.off args.len)
                          let input := (MemoryCalldata input_memory)
                          let precompile_charge ← do
                            (precompile_gas selected_precompile input child_gas)
                          if _sailIf6 : (precompile_charge.affordable : Bool) = true
                          then
                            (do
                              let used := precompile_charge.cost
                              let result ← do (run_precompile_slice selected_precompile input)
                              if _sailIf7 : (result.success : Bool) = true
                              then
                                (do
                                  writeReg returndata result.output
                                  if ((semantics.transfers_value && value_nonzero) : Bool)
                                  then (k_transfer caller target value)
                                  else (pure ())
                                  (returndata_copy_prefix ret.off ret.len)
                                  let unused : Nat := (gas_sub child_gas used)
                                  let g5 := (refund_gas g4 unused)
                                  (pure ((pc_in : Nat), (← do
                                      (push_word top7 WORD_ONE)), (mem2 : (Sigma fun
                                    (mem_dependentWitness0 : Nat) =>
                                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g5 : Nat))))
                              else
                                (do
                                  (returndata_clear ())
                                  let g5 ← (( do
                                    if (new_account_charged : Bool)
                                    then (credit_state_gas_refund g4 G_amsterdam_state_new_account)
                                    else (pure g4) ) : SailME
                                    (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat)
                                    =>
                                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                                    Nat )
                                  (pure ((pc_in : Nat), (← do
                                      (push_word top7 WORD_ZERO)), (mem2 : (Sigma fun
                                    (mem_dependentWitness0 : Nat) =>
                                    (Sigma fun (mem_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g5 : Nat)))))
                          else
                            (do
                              (returndata_clear ())
                              let g5 ← (( do
                                if (new_account_charged : Bool)
                                then (credit_state_gas_refund g4 G_amsterdam_state_new_account)
                                else (pure g4) ) : SailME
                                (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                                (Sigma fun (mem_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                                Nat )
                              (pure ((pc_in : Nat), (← do
                                  (push_word top7 WORD_ZERO)), (mem2 : (Sigma fun
                                (mem_dependentWitness0 : Nat) =>
                                (Sigma fun (mem_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g5 : Nat)))))
                      else
                        (do
                          let child_depth : Nat := (current_depth + 1)
                          let ⟨_, ⟨_, child_code⟩⟩ ← do
                            (executable_code target tg_deleg tg_target)
                          let child_addr : (Vector (BitVec 8) 20) :=
                            if (semantics.uses_target_address : Bool)
                            then target
                            else caller
                          let child_caller ← (( do
                            if (semantics.inherits_caller_and_value : Bool)
                            then (pure (← readReg message).caller)
                            else (pure caller) ) : SailME
                            (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                            (Vector (BitVec 8) 20) )
                          let child_value ← (( do
                            if (semantics.inherits_caller_and_value : Bool)
                            then (pure (← readReg message).value)
                            else (pure value) ) : SailME
                            (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                            Nat )
                          let child_static ← (( do
                            if (semantics.enters_static_context : Bool)
                            then (pure true)
                            else (pure (← readReg message).is_static) ) : SailME
                            (Nat × (BitVec 64) × (Sigma fun (mem_dependentWitness0 : Nat) =>
                            (Sigma fun (mem_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat)
                            Bool )
                          let (bytes, mem2) ← do
                            (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ args.off args.len)
                          let child_memory :=
                            (evm_memory_slice ((bytes).2).2.bytes ((bytes).2).2.len)
                          let child_calldata := (MemoryCalldata ⟨_, ⟨_, child_memory⟩⟩)
                          let child_state_gas ← do readReg state_gas_remaining
                          writeReg pc pc_in
                          writeReg gas_remaining g4
                          writeReg stack_top top7
                          writeReg evm_memory mem2
                          let checkpoint ← do
                            (pure { (← (suspend_frame ())) with state_gas_remaining := GAS_ZERO })
                          let call_continuation : CallContinuation :=
                            { checkpoint := checkpoint,
                              return_offset := ret.off,
                              return_length := ret.len,
                              new_account_charged := new_account_charged }
                          let continuation := (ResumeCall call_continuation)
                          (frame_stack_push continuation)
                          if ((semantics.transfers_value && value_nonzero) : Bool)
                          then (k_transfer caller target value)
                          else (pure ())
                          writeReg message { caller := child_caller,
                                             address := child_addr,
                                             code_address := target,
                                             value := child_value,
                                             state_gas_reservoir := child_state_gas,
                                             is_static := child_static,
                                             depth := child_depth }
                          (calldata_install child_calldata)
                          writeReg pc 0
                          writeReg gas_remaining child_gas
                          writeReg state_gas_remaining child_state_gas
                          writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
                          writeReg frame_status (Running ())
                          (returndata_clear ())
                          writeReg frame_code ⟨_, ⟨_, child_code⟩⟩
                          writeReg call_depth child_depth
                          writeReg frame_refund GAS_REFUND_ZERO
                          (pure ((0 : Nat), (← do
                              readReg stack_top), (← do
                              readReg evm_memory), (child_gas : Nat)))))))))

/-- Implements `CALL` through the common call runner. -/
/- Type quantifiers: k_ex552880_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552875_
  : Nat, 0 ≤ k_ex552875_ ∧ k_ex552875_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552880_ -/
def execute_call (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  (run_call Call pc_in top ⟨_, ⟨_, mem⟩⟩ g)

/-- Implements `CALLCODE` through the common call runner. -/
/- Type quantifiers: k_ex552890_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552885_
  : Nat, 0 ≤ k_ex552885_ ∧ k_ex552885_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552890_ -/
def execute_callcode (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  (run_call CallCode pc_in top ⟨_, ⟨_, mem⟩⟩ g)

/-- Implements `DELEGATECALL` through the common call runner. -/
/- Type quantifiers: k_ex552900_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552895_
  : Nat, 0 ≤ k_ex552895_ ∧ k_ex552895_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552900_ -/
def execute_delegatecall (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  (run_call DelegateCall pc_in top ⟨_, ⟨_, mem⟩⟩ g)

/-- Implements `STATICCALL` through the common call runner. -/
/- Type quantifiers: k_ex552910_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552905_
  : Nat, 0 ≤ k_ex552905_ ∧ k_ex552905_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552910_ -/
def execute_staticcall (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  (run_call StaticCall pc_in top ⟨_, ⟨_, mem⟩⟩ g)

/-- The Yellow Paper stack effect `(delta, alpha)` for every decoded
instruction. Invalid deep-stack immediates have no stack effect because
their opcode body raises `InvalidOpcode` before accessing the stack. -/
def opcode_stack_effect (op : ast) : SailM (Nat × Nat) := do
  match op with
  | .STOP () => (pure (0, 0))
  | .ADD () => (pure (2, 1))
  | .MUL () => (pure (2, 1))
  | .SUB () => (pure (2, 1))
  | .DIV () => (pure (2, 1))
  | .SDIV () => (pure (2, 1))
  | .MOD () => (pure (2, 1))
  | .SMOD () => (pure (2, 1))
  | .ADDMOD () => (pure (3, 1))
  | .MULMOD () => (pure (3, 1))
  | .EXP () => (pure (2, 1))
  | .SIGNEXTEND () => (pure (2, 1))
  | .LT () => (pure (2, 1))
  | .GT () => (pure (2, 1))
  | .SLT () => (pure (2, 1))
  | .SGT () => (pure (2, 1))
  | .EQ () => (pure (2, 1))
  | .ISZERO () => (pure (1, 1))
  | .AND () => (pure (2, 1))
  | .OR () => (pure (2, 1))
  | .XOR () => (pure (2, 1))
  | .NOT () => (pure (1, 1))
  | .BYTE () => (pure (2, 1))
  | .SHL () => (pure (2, 1))
  | .SHR () => (pure (2, 1))
  | .SAR () => (pure (2, 1))
  | .CLZ () => (pure (1, 1))
  | .KECCAK256 () => (pure (2, 1))
  | .ADDRESS () => (pure (0, 1))
  | .BALANCE () => (pure (1, 1))
  | .ORIGIN () => (pure (0, 1))
  | .CALLER () => (pure (0, 1))
  | .CALLVALUE () => (pure (0, 1))
  | .CALLDATALOAD () => (pure (1, 1))
  | .CALLDATASIZE () => (pure (0, 1))
  | .CALLDATACOPY () => (pure (3, 0))
  | .CODESIZE () => (pure (0, 1))
  | .CODECOPY () => (pure (3, 0))
  | .GASPRICE () => (pure (0, 1))
  | .EXTCODESIZE () => (pure (1, 1))
  | .EXTCODECOPY () => (pure (4, 0))
  | .RETURNDATASIZE () => (pure (0, 1))
  | .RETURNDATACOPY () => (pure (3, 0))
  | .EXTCODEHASH () => (pure (1, 1))
  | .BLOCKHASH () => (pure (1, 1))
  | .COINBASE () => (pure (0, 1))
  | .TIMESTAMP () => (pure (0, 1))
  | .NUMBER () => (pure (0, 1))
  | .PREVRANDAO () => (pure (0, 1))
  | .GASLIMIT () => (pure (0, 1))
  | .CHAINID () => (pure (0, 1))
  | .SELFBALANCE () => (pure (0, 1))
  | .BASEFEE () => (pure (0, 1))
  | .BLOBHASH () => (pure (1, 1))
  | .BLOBBASEFEE () => (pure (0, 1))
  | .SLOTNUM () => (pure (0, 1))
  | .POP () => (pure (1, 0))
  | .MLOAD () => (pure (1, 1))
  | .MSTORE () => (pure (2, 0))
  | .MSTORE8 () => (pure (2, 0))
  | .SLOAD () => (pure (1, 1))
  | .SSTORE () => (pure (2, 0))
  | .JUMP () => (pure (1, 0))
  | .JUMPI () => (pure (2, 0))
  | .PC () => (pure (0, 1))
  | .MSIZE () => (pure (0, 1))
  | .GAS () => (pure (0, 1))
  | .JUMPDEST () => (pure (0, 0))
  | .TLOAD () => (pure (1, 1))
  | .TSTORE () => (pure (2, 0))
  | .MCOPY () => (pure (3, 0))
  | .PUSH (_, _) => (pure (0, 1))
  | .DUP n => (pure (n, (n + 1)))
  | .SWAP n => (pure ((n + 1), (n + 1)))
  | .DUPN immediate =>
    (do
      let valid_immediate := (deep_stack_immediate_valid immediate)
      if (valid_immediate : Bool)
      then
        (do
          let n ← do (decode_single_stack_index immediate)
          (pure (n, (n + 1))))
      else (pure (0, 0)))
  | .SWAPN immediate =>
    (do
      let valid_immediate := (deep_stack_immediate_valid immediate)
      if (valid_immediate : Bool)
      then
        (do
          let n ← do (decode_single_stack_index immediate)
          (pure ((n + 1), (n + 1))))
      else (pure (0, 0)))
  | .EXCHANGE immediate =>
    (do
      let valid_immediate := (exchange_immediate_valid immediate)
      if (valid_immediate : Bool)
      then
        (do
          let (_, higher) ← do (decode_exchange_stack_indices immediate)
          (pure ((higher + 1), (higher + 1))))
      else (pure (0, 0)))
  | .LOG n => (pure ((n + 2), 0))
  | .opcode_CREATE () => (pure (3, 1))
  | .CALL () => (pure (7, 1))
  | .CALLCODE () => (pure (7, 1))
  | .RETURN () => (pure (2, 0))
  | .DELEGATECALL () => (pure (6, 1))
  | .CREATE2 () => (pure (4, 1))
  | .STATICCALL () => (pure (6, 1))
  | .REVERT () => (pure (2, 0))
  | .INVALID () => (pure (0, 0))
  | .SELFDESTRUCT () => (pure (1, 0))

/-- Standard decoded-opcode dispatcher. Each arm invokes the same narrow
semantic function used directly by the optimized raw-byte interpreter,
threading exactly the machine state that family carries. -/
/- Type quantifiers: k_ex552920_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552915_
  : Nat, 0 ≤ k_ex552915_ ∧ k_ex552915_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552920_ -/
def execute_opcode (op : ast) (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  match op with
  | .STOP () =>
    (do
      (execute_stop ())
      (pure ((pc_in : Nat), (top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g : Nat))))
  | .ADD () =>
    (do
      let (top1, g1) ← do (execute_add top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MUL () =>
    (do
      let (top1, g1) ← do (execute_mul top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SUB () =>
    (do
      let (top1, g1) ← do (execute_sub top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .DIV () =>
    (do
      let (top1, g1) ← do (execute_div top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SDIV () =>
    (do
      let (top1, g1) ← do (execute_sdiv top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MOD () =>
    (do
      let (top1, g1) ← do (execute_mod top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SMOD () =>
    (do
      let (top1, g1) ← do (execute_smod top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .ADDMOD () =>
    (do
      let (top1, g1) ← do (execute_addmod top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MULMOD () =>
    (do
      let (top1, g1) ← do (execute_mulmod top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .EXP () =>
    (do
      let (top1, g1) ← do (execute_exp top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SIGNEXTEND () =>
    (do
      let (top1, g1) ← do (execute_signextend top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .LT () =>
    (do
      let (top1, g1) ← do (execute_lt top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .GT () =>
    (do
      let (top1, g1) ← do (execute_gt top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SLT () =>
    (do
      let (top1, g1) ← do (execute_slt top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SGT () =>
    (do
      let (top1, g1) ← do (execute_sgt top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .EQ () =>
    (do
      let (top1, g1) ← do (execute_eq top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .ISZERO () =>
    (do
      let (top1, g1) ← do (execute_iszero top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .AND () =>
    (do
      let (top1, g1) ← do (execute_and top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .OR () =>
    (do
      let (top1, g1) ← do (execute_or top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .XOR () =>
    (do
      let (top1, g1) ← do (execute_xor top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .NOT () =>
    (do
      let (top1, g1) ← do (execute_not top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .BYTE () =>
    (do
      let (top1, g1) ← do (execute_byte top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SHL () =>
    (do
      let (top1, g1) ← do (execute_shl top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SHR () =>
    (do
      let (top1, g1) ← do (execute_shr top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SAR () =>
    (do
      let (top1, g1) ← do (execute_sar top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CLZ () =>
    (do
      let (top1, g1) ← do (execute_clz top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .KECCAK256 () =>
    (do
      let (top1, mem1, g1) ← do (execute_keccak256 top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .ADDRESS () =>
    (do
      let (top1, g1) ← do (execute_address top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .ORIGIN () =>
    (do
      let (top1, g1) ← do (execute_origin top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CALLER () =>
    (do
      let (top1, g1) ← do (execute_caller top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CALLVALUE () =>
    (do
      let (top1, g1) ← do (execute_callvalue top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .GASPRICE () =>
    (do
      let (top1, g1) ← do (execute_gasprice top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CALLDATASIZE () =>
    (do
      let (top1, g1) ← do (execute_calldatasize top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CALLDATALOAD () =>
    (do
      let (top1, g1) ← do (execute_calldataload top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CALLDATACOPY () =>
    (do
      let (top1, mem1, g1) ← do (execute_calldatacopy top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CODESIZE () =>
    (do
      let (top1, g1) ← do (execute_codesize top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CODECOPY () =>
    (do
      let (top1, mem1, g1) ← do (execute_codecopy top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .BALANCE () =>
    (do
      let (top1, g1) ← do (execute_balance top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SELFBALANCE () =>
    (do
      let (top1, g1) ← do (execute_selfbalance top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .EXTCODESIZE () =>
    (do
      let (top1, g1) ← do (execute_extcodesize top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .EXTCODECOPY () =>
    (do
      let (top1, mem1, g1) ← do (execute_extcodecopy top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .EXTCODEHASH () =>
    (do
      let (top1, g1) ← do (execute_extcodehash top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .RETURNDATASIZE () =>
    (do
      let (top1, g1) ← do (execute_returndatasize top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .RETURNDATACOPY () =>
    (do
      let (top1, mem1, g1) ← do (execute_returndatacopy top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .BLOCKHASH () =>
    (do
      let (top1, g1) ← do (execute_blockhash top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .COINBASE () =>
    (do
      let (top1, g1) ← do (execute_coinbase top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .TIMESTAMP () =>
    (do
      let (top1, g1) ← do (execute_timestamp top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .NUMBER () =>
    (do
      let (top1, g1) ← do (execute_number top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SLOTNUM () =>
    (do
      let (top1, g1) ← do (execute_slotnum top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .PREVRANDAO () =>
    (do
      let (top1, g1) ← do (execute_prevrandao top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .GASLIMIT () =>
    (do
      let (top1, g1) ← do (execute_gaslimit top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .CHAINID () =>
    (do
      let (top1, g1) ← do (execute_chainid top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .BASEFEE () =>
    (do
      let (top1, g1) ← do (execute_basefee top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .BLOBBASEFEE () =>
    (do
      let (top1, g1) ← do (execute_blobbasefee top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .BLOBHASH () =>
    (do
      let (top1, g1) ← do (execute_blobhash top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .POP () =>
    (do
      let (top1, g1) ← do (execute_pop top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MLOAD () =>
    (do
      let (top1, mem1, g1) ← do (execute_mload top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MSTORE () =>
    (do
      let (top1, mem1, g1) ← do (execute_mstore top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MSTORE8 () =>
    (do
      let (top1, mem1, g1) ← do (execute_mstore8 top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MSIZE () =>
    (do
      let (top1, mem1, g1) ← do (execute_msize top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .MCOPY () =>
    (do
      let (top1, mem1, g1) ← do (execute_mcopy top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SLOAD () =>
    (do
      let (top1, g1) ← do (execute_sload top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SSTORE () =>
    (do
      let (top1, g1) ← do (execute_sstore top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .TLOAD () =>
    (do
      let (top1, g1) ← do (execute_tload top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .TSTORE () =>
    (do
      let (top1, g1) ← do (execute_tstore top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .JUMP () =>
    (do
      let (pc1, top1, g1) ← do (execute_jump pc_in top g)
      (pure ((pc1 : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .JUMPI () =>
    (do
      let (pc1, top1, g1) ← do (execute_jumpi pc_in top g)
      (pure ((pc1 : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .PC () =>
    (do
      let (top1, g1) ← do (execute_pc pc_in top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .GAS () =>
    (do
      let (top1, g1) ← do (execute_gas top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .JUMPDEST () =>
    (do
      (pure ((pc_in : Nat), (top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (← do
          (execute_jumpdest g)))))
  | .PUSH (n, v) =>
    (do
      let (top1, g1) ← do (execute_push n v top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .DUP n =>
    (do
      let (top1, g1) ← do (execute_dup n top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SWAP n =>
    (do
      let (top1, g1) ← do (execute_swap n top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .DUPN immediate =>
    (do
      let (top1, g1) ← do (execute_dupn immediate top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .SWAPN immediate =>
    (do
      let (top1, g1) ← do (execute_swapn immediate top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .EXCHANGE immediate =>
    (do
      let (top1, g1) ← do (execute_exchange immediate top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .LOG n =>
    (do
      let (top1, mem1, g1) ← do (execute_log n top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .opcode_CREATE () =>
    (do
      (execute_create pc_in top ⟨_, ⟨_, mem⟩⟩ g))
  | .CREATE2 () =>
    (do
      (execute_create2 pc_in top ⟨_, ⟨_, mem⟩⟩ g))
  | .CALL () =>
    (do
      (execute_call pc_in top ⟨_, ⟨_, mem⟩⟩ g))
  | .CALLCODE () =>
    (do
      (execute_callcode pc_in top ⟨_, ⟨_, mem⟩⟩ g))
  | .DELEGATECALL () =>
    (do
      (execute_delegatecall pc_in top ⟨_, ⟨_, mem⟩⟩ g))
  | .STATICCALL () =>
    (do
      (execute_staticcall pc_in top ⟨_, ⟨_, mem⟩⟩ g))
  | .RETURN () =>
    (do
      let (top1, mem1, g1) ← do (execute_return top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .REVERT () =>
    (do
      let (top1, mem1, g1) ← do (execute_revert top ⟨_, ⟨_, mem⟩⟩ g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), (mem1 : (Sigma fun (mem_dependentWitness0 : Nat)
        =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))
  | .INVALID () =>
    (do
      (pure ((pc_in : Nat), (top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (← do
          (execute_invalid g)))))
  | .SELFDESTRUCT () =>
    (do
      let (top1, g1) ← do (execute_selfdestruct top g)
      (pure ((pc_in : Nat), (top1 : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
        (mem_dependentWitness0 : Nat) =>
        (Sigma fun (mem_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat))))

/-- The single-step transition function over the standard decoded AST. Stack
validity is established before gas charging or any instruction side
effect, as part of the Yellow Paper exceptional-halt predicate. -/
/- Type quantifiers: k_ex552930_ : Nat, mem_dependentWitness1 : Nat, mem_dependentWitness0 : Nat, k_ex552925_
  : Nat, 0 ≤ k_ex552925_ ∧ k_ex552925_ ≤ (2 ^ 32 - 1), 0 ≤ mem_dependentWitness0 ∧
  0 ≤ mem_dependentWitness1 ∧ (mem_dependentWitness0 + mem_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex552930_ -/
def execute (op : ast) (pc_in : Nat) (top : (BitVec 64)) (mem : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (g : Nat) : SailM (Nat × (BitVec 64) × (Sigma
  fun (mem_dependentWitness0 : Nat) =>
  (Sigma fun (mem_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1))) × Nat) := do
  let mem_dependentWitness0 := (mem).1
  let mem_dependentWitness1 := ((mem).2).1
  let mem := ((mem).2).2
  let (inputs, outputs) ← do (opcode_stack_effect op)
  let (stack_valid, g1) ← do (validate_stack g top inputs outputs)
  if _sailIf0 : (stack_valid : Bool) = true
  then
    (do
      (execute_opcode op pc_in top ⟨_, ⟨_, mem⟩⟩ g1))
  else
    (pure ((pc_in : Nat), (top : (BitVec 64)), ((⟨_, ⟨_, mem⟩⟩ : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))) : (Sigma fun
      (mem_dependentWitness0 : Nat) =>
      (Sigma fun (mem_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields mem_dependentWitness0 mem_dependentWitness1)))), (g1 : Nat)))

