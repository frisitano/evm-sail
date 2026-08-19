import Evm.Flow
import Evm.Prelude
import Evm.Host.RegionAccess

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

/-! # Block types

Block-level data structures. [BlockHeader][type-BlockHeader] is pure data;
its `k_header` register lives in the kernel environment alongside the
EIP-4895 withdrawal record. -/

def EMPTY_LOGS_BLOOM : LogsBloom := (vectorInit 0x00#8)

/-- Byte-wise bloom equality (the header `logs_bloom` check). -/
def logs_bloom_equal (a : (Vector (BitVec 8) 256)) (b : (Vector (BitVec 8) 256)) : Bool := Id.run do
  let equal : Bool := true
  let loop_i_lower := 0
  let loop_i_upper := 255
  let mut loop_vars := equal
  for i in [loop_i_lower:loop_i_upper:1]i do
    let equal := loop_vars
    loop_vars := (equal && ((GetElem?.getElem! a i) == (GetElem?.getElem! b i)))
  (pure loop_vars)

/-- Materializes a referenced wire-order bloom into the decreasing-index
semantic vector used by the Yellow Paper equations. -/
/- Type quantifiers: reference_dependentWitness1 : Nat, reference_dependentWitness0 : Nat, 0 ≤
  reference_dependentWitness0 ∧
  0 ≤ reference_dependentWitness1 ∧
  (reference_dependentWitness0 + reference_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  reference_dependentWitness1 = 256 -/
def logs_bloom_from_ref (reference : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 256) := do
  let reference_dependentWitness0 := (reference).1
  let reference_dependentWitness1 := ((reference).2).1
  let reference := ((reference).2).2
  let out : (Vector (BitVec 8) 256) := EMPTY_LOGS_BLOOM
  let loop_i_lower := 0
  let loop_i_upper := 255
  let mut loop_vars := out
  for i in [loop_i_lower:loop_i_upper:1]i do
    let out := loop_vars
    loop_vars ← do
      (pure (vectorUpdate out (255 - i)
          (← (stateless_input_slice_byte ⟨_, ⟨_, reference⟩⟩ i))))
  (pure loop_vars)

/-- Compares the computed block bloom with the payload-header commitment. -/
/- Type quantifiers: reference_dependentWitness1 : Nat, reference_dependentWitness0 : Nat, 0 ≤
  reference_dependentWitness0 ∧
  0 ≤ reference_dependentWitness1 ∧
  (reference_dependentWitness0 + reference_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  reference_dependentWitness1 = 256 -/
def logs_bloom_matches_ref (computed : (Vector (BitVec 8) 256)) (reference : (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Bool := do
  let reference_dependentWitness0 := (reference).1
  let reference_dependentWitness1 := ((reference).2).1
  let reference := ((reference).2).2
  let expected ← do (logs_bloom_from_ref ⟨_, ⟨_, reference⟩⟩)
  (pure (logs_bloom_equal computed expected))

/-- `keccak256(rlp([]))` — the ommers hash of every post-merge block
(EIP-3675 requires an empty ommers list). -/
def EMPTY_OMMER_HASH : hash :=
  (hash_from_bits 0x1DCC4DE8DEC75D7AAB85B567B6CCD41AD312451B948A7413F0A142FD40D49347#256)

def undefined_Withdrawal (_ : Unit) : SailM Withdrawal := do
  (pure { index := ← (undefined_range 0 ((2 ^i 64) - 1)),
          validator_index := ← (undefined_range 0 ((2 ^i 64) - 1)),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          amount := ← (undefined_range 0 ((2 ^i 64) - 1)) })

