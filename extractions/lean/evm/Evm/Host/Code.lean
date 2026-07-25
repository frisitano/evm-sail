import Evm.Flow
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice

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

/-! # Code storage

Content-addressed code storage and the Sail-side `JUMPDEST` analysis.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

def undefined_CodeAnalysis (_ : Unit) : SailM CodeAnalysis := do
  (pure { chunk := ← (undefined_bitvector 256),
          chunk_index := ← (undefined_nat ()),
          chunk_offset := ← (undefined_range 0 255) })

/-- Commits a completed nonempty bitmap chunk to its allocated table. -/
/- Type quantifiers: k_ex415477_ : Nat, 0 ≤ k_ex415477_ -/
def store_jumpdest_chunk (table : (BitVec 64)) (code_len : Nat) (analysis : CodeAnalysis) : SailM Unit := do
  if ((analysis.chunk != EMPTY_JUMPDEST_CHUNK) : Bool)
  then
    (do
      let stored ← do
        (jumpdest_table_store_chunk table code_len analysis.chunk_index analysis.chunk)
      assert stored "JUMPDEST chunk store")
  else (pure ())

/-- Returns the one-hot bitmap value for an offset within a 256-byte chunk. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 255 -/
def jumpdest_bit (index : Nat) : (BitVec 256) :=
  let chunk : (BitVec 256) := EMPTY_JUMPDEST_CHUNK
  (BitVec.update chunk index 1#1)

/- Type quantifiers: _reclimit : Nat, code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, pc
  : Nat, (code_valid_length pc), 0 ≤ code_dependentWitness0 ∧ 0 ≤ code_dependentWitness1 ∧
  0 ≤ code_dependentWitness1, 0 ≤ _reclimit -/
def _rec_analyze_code_from (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (fork : Fork) (table : (BitVec 64)) (pc : Nat) (analysis : CodeAnalysis) (_reclimit : Nat) : SailM Unit := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let position := pc
      let code_len := code.len
      if ((position <b code_len) : Bool)
      then
        (do
          let chunk := analysis.chunk
          let chunk_index := analysis.chunk_index
          let chunk_offset : Nat := analysis.chunk_offset
          let opcode ← do (slice_byte ⟨_, ⟨_, code⟩⟩ position)
          let chunk : (BitVec 256) :=
            if ((opcode == 0x5B#8) : Bool)
            then (chunk ||| (jumpdest_bit chunk_offset))
            else chunk
          let opcode_value := (BitVec.toNatInt opcode)
          let step ← (( do
            if (((96 ≤b opcode_value) && (opcode_value ≤b 127)) : Bool)
            then (pure (opcode_value - 94))
            else
              (do
                if (((fork_gteq fork Amsterdam) && ((opcode_value == 230) || (opcode_value == 231))) : Bool)
                then
                  (do
                    if ((deep_stack_immediate_valid
                         (← (slice_byte ⟨_, ⟨_, code⟩⟩ (position + 1)))) : Bool)
                    then (pure 2)
                    else (pure 1))
                else
                  (do
                    if (((fork_gteq fork Amsterdam) && (opcode_value == 232)) : Bool)
                    then
                      (do
                        if ((exchange_immediate_valid
                             (← (slice_byte ⟨_, ⟨_, code⟩⟩ (position + 1)))) : Bool)
                        then (pure 2)
                        else (pure 1))
                    else (pure 1))) ) : SailM Nat )
          if ((step <b (code_len - position)) : Bool)
          then
            (do
              let added := (position + step)
              let progressed : Nat := (chunk_offset + step)
              if ((progressed <b 256) : Bool)
              then
                (_rec_analyze_code_from ⟨_, ⟨_, code⟩⟩ fork table added
                  { chunk := chunk,
                    chunk_index := chunk_index,
                    chunk_offset := progressed } _reclimit_pred)
              else
                (do
                  (store_jumpdest_chunk table code.len
                    { chunk := chunk,
                      chunk_index := chunk_index,
                      chunk_offset := chunk_offset })
                  (_rec_analyze_code_from ⟨_, ⟨_, code⟩⟩ fork table added
                    { chunk := EMPTY_JUMPDEST_CHUNK,
                      chunk_index := (added / 256),
                      chunk_offset := (progressed - 256) } _reclimit_pred)))
          else
            (store_jumpdest_chunk table code.len
              { chunk := chunk,
                chunk_index := chunk_index,
                chunk_offset := chunk_offset }))
      else (store_jumpdest_chunk table code.len analysis))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, pc : Nat, (code_valid_length pc), 0
  ≤ code_dependentWitness0 ∧ 0 ≤ code_dependentWitness1 ∧ 0 ≤ code_dependentWitness1 -/
def analyze_code_from (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (fork : Fork) (table : (BitVec 64)) (pc : Nat) (analysis : CodeAnalysis) : SailM Unit := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  let _measure :=
    (let code_len := code.len
    let position := pc
    (code_len -i position) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_analyze_code_from ⟨_, ⟨_, code⟩⟩ fork table pc analysis (_measure + 1))

/-- The PUSH-aware `JUMPDEST` analysis (YP §9.4.3): PUSH immediate bytes
are data even when they contain `0x5b`. The completed bitmap remains a
first-class Sail value; the host never scans opcodes. -/
/- Type quantifiers: code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0 ≤
  code_dependentWitness0 ∧ 0 ≤ code_dependentWitness1 ∧ 0 ≤ code_dependentWitness1 -/
def analyze_code (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (fork : Fork) : SailM (BitVec 64) := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  if ((code.len == 0) : Bool)
  then (pure EMPTY_JUMPDEST_REF)
  else
    (do
      let length := code.len
      let table ← do (jumpdest_table_alloc length)
      assert (table != EMPTY_JUMPDEST_REF) "JUMPDEST table allocation"
      (analyze_code_from ⟨_, ⟨_, code⟩⟩ fork table 0
        { chunk := EMPTY_JUMPDEST_CHUNK,
          chunk_index := 0,
          chunk_offset := 0 })
      (pure table))

/-- Analyzes and stores code, returning its content hash. -/
/- Type quantifiers: code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0 ≤
  code_dependentWitness0 ∧ 0 ≤ code_dependentWitness1 ∧ 0 ≤ code_dependentWitness1 -/
def code_db_insert (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (fork : Fork) : SailM (Vector (BitVec 8) 32) := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  (code_db_store ⟨_, ⟨_, code⟩⟩ (← (analyze_code ⟨_, ⟨_, code⟩⟩ fork)))

/-- The code for a code hash; `KECCAK_EMPTY` resolves to empty code, and
an unwitnessed hash is a deficient witness. -/
def code_db_resolve (code_hash : (Vector (BitVec 8) 32)) : SailM Code := do
  if ((code_hash == KECCAK_EMPTY) : Bool)
  then (pure EMPTY_CODE)
  else
    (do
      match (← (code_db_lookup code_hash)) with
      | .some code => (pure code)
      | none => sailThrow ((InvalidBlock WitnessDeficient)))

