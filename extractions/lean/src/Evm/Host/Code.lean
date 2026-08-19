import Evm.Flow
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Crypto
import Evm.Primitives.Fork

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

/-! # Code storage

Content-addressed code storage and the Sail-side `JUMPDEST` analysis.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/- Type quantifiers: table : Nat, fork : Nat, code_dependentWitness1 : Nat, code_dependentWitness0 :
  Nat, pc : Nat, (code_valid_length pc), 0 ≤ code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤ fork ∧
  fork ≤ 16, 0 ≤ table ∧ table ≤ (2 ^ 64 - 1) -/
def analyze_code_from (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (fork : Nat) (table : Nat) (pc : Nat) : SailM Unit := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  let code_len := code.len
  let scanning : Bool := true
  let position : Nat := pc
  let (position, scanning) ← (( do
    let loop_vars ← whileFuelM (fuel :=(code_len -i position)) (fun (position, scanning) => (pure (scanning && ((position <b code_len) : Bool)))) (position, scanning)
      fun (position, scanning) => do
        assert true "loop dummy assert"
        let current := position
        let opcode ← do (code_slice_byte ⟨_, ⟨_, code⟩⟩ current)
        if ((opcode == 0x5B#8) : Bool)
        then
          (do
            let marked ← do (jumpdest_table_mark table code_len current)
            assert marked "JUMPDEST mark")
        else (pure ())
        let opcode_value := (BitVec.toNatInt opcode)
        let step ← (( do
          if (((96 ≤b opcode_value) && (opcode_value ≤b 127)) : Bool)
          then (pure (opcode_value - 94))
          else
            (do
              if ((fork ≥b Amsterdam) : Bool)
              then
                (do
                  let operation := (deep_stack_operation opcode_value)
                  let immediate ← do (code_slice_byte ⟨_, ⟨_, code⟩⟩ (current + 1))
                  let immediate_valid := (deep_stack_operation_immediate_valid operation immediate)
                  if (immediate_valid : Bool)
                  then (pure 2)
                  else (pure 1))
              else (pure 1)) ) : SailM Nat )
        let (position, scanning) : (Nat × Bool) :=
          if ((step <b (code_len -i current)) : Bool)
          then
            (let position : Nat := (current + step)
            (position, scanning))
          else
            (let scanning : Bool := false
            (position, scanning))
        (pure (position, scanning))
    (pure loop_vars) ) : SailM (Nat × Bool) )
  (pure ())

/-- The PUSH-aware `JUMPDEST` analysis (YP §9.4.3): PUSH immediate bytes
are data even when they contain `0x5b`. The completed bitmap remains a
first-class Sail value; the host never scans opcodes. -/
/- Type quantifiers: k_ex607502_ : Nat, code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0
  ≤ code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤ k_ex607502_
  ∧ k_ex607502_ ≤ 16 -/
def analyze_code (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (fork : Nat) : SailM Nat := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  if ((code.len == 0) : Bool)
  then (pure EMPTY_JUMP_TABLE)
  else
    (do
      let table ← do (jumpdest_table_alloc ⟨_, ⟨_, code⟩⟩)
      assert (table != EMPTY_JUMP_TABLE) "JUMPDEST table allocation"
      (analyze_code_from ⟨_, ⟨_, code⟩⟩ fork table 0)
      (pure table))

/-- Analyzes and stores code, returning its content hash. -/
/- Type quantifiers: k_ex607509_ : Nat, code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0
  ≤ code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤ k_ex607509_
  ∧ k_ex607509_ ≤ 16 -/
def code_db_insert (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (fork : Nat) : SailM (Vector (BitVec 8) 32) := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  let jumpdest_table ← do (analyze_code ⟨_, ⟨_, code⟩⟩ fork)
  let analyzed := (analyzed_code code jumpdest_table)
  (code_db_store ⟨_, ⟨_, analyzed⟩⟩)

/-- Normalizes stateless-input code into the code arena before analysis. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_db_intern_input (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Sigma fun
  (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (CodeRegionSliceFields bytes_dependentWitness0 bytes_dependentWitness1))) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let ⟨_, ⟨_, region⟩⟩ ← do (code_region_from_input ⟨_, ⟨_, bytes⟩⟩)
  (validated_code_slice ⟨_, ⟨_, region⟩⟩)

/-- Normalizes memory-backed initcode into the code arena before analysis. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_db_intern_memory (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) : SailM (Sigma fun
  (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (CodeRegionSliceFields bytes_dependentWitness0 bytes_dependentWitness1))) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let ⟨_, ⟨_, region⟩⟩ ← do (code_region_from_memory ⟨_, ⟨_, bytes⟩⟩)
  (validated_code_slice ⟨_, ⟨_, region⟩⟩)

/-- Normalizes frozen creation output into the code arena before deployment. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_db_intern_output (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : SailM (Sigma fun
  (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (CodeRegionSliceFields bytes_dependentWitness0 bytes_dependentWitness1))) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let ⟨_, ⟨_, region⟩⟩ ← do (code_region_from_output ⟨_, ⟨_, bytes⟩⟩)
  (validated_code_slice ⟨_, ⟨_, region⟩⟩)

/-- The code for a code hash; `KECCAK_EMPTY` resolves to empty code, and
an unwitnessed hash is a deficient witness. -/
def code_db_resolve (code_hash : (Vector (BitVec 8) 32)) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) := do
  if _sailIf0 : ((code_hash == KECCAK_EMPTY) : Bool) = true
  then
    (pure ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))))
  else
    (do
      let ⟨_, ⟨_, code⟩⟩ ← do (code_db_lookup code_hash)
      if _sailIf1 : ((code.len == 0) : Bool) = true
      then
        (do
          (fatal_error WitnessDeficient))
      else
        (pure ((⟨_, ⟨_, code⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))))))

