import Evm.Prelude
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Kernel.Scratch
import Evm.Kernel.Storage

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

/-! # Trie node types

Merkle-Patricia trie node forms, references, and decoding
(YP Appendix D). -/

def undefined_InlineNode (_ : Unit) : SailM InlineNode := do
  (pure { data := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          len := ← (undefined_range 0 31) })

/-- Copies a sub-32-byte scratch node encoding into an inline node value. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def inline_node_from_scratch_slice (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM InlineNode := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let length := bytes.len
  if ((length <b MPT_HASH_LENGTH) : Bool)
  then
    (do
      let encoded ← do (scratch_slice_load ⟨_, ⟨_, bytes⟩⟩ 0)
      (pure { data := (word_to_hash encoded),
              len := length }))
  else (fatal_error WitnessDeficient)

/-- Copies a sub-32-byte input node encoding into an inline node value. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0 ≤
  bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def inline_node_from_input_slice (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM InlineNode := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let length := bytes.len
  if ((length <b MPT_HASH_LENGTH) : Bool)
  then
    (do
      let encoded ← do (stateless_input_slice_load ⟨_, ⟨_, bytes⟩⟩ 0)
      (pure { data := (word_to_hash encoded),
              len := length }))
  else (fatal_error WitnessDeficient)

/-- Materializes an inline node in scratch memory as a byte slice. -/
def inline_node_slice (node : InlineNode) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let start ← do (scratch_reserve node.len)
  (scratch_push_b256 node.data node.len)
  (scratch_finish start)

/-- Hashes an inline node from its existing scratch representation. -/
def inline_node_hash (node : InlineNode) : SailM (Vector (BitVec 8) 32) := do
  let mark ← do (scratch_begin ())
  let ⟨_, ⟨_, encoded⟩⟩ ← do (inline_node_slice node)
  let digest ← do (scratch_keccak256 ⟨_, ⟨_, encoded⟩⟩)
  (scratch_rewind mark)
  (pure digest)

/-- Advances the branch payload length while preserving its structural bound. -/
/- Type quantifiers: k_ex610910_ : Nat, k_ex610909_ : Nat, 0 ≤ k_ex610909_ ∧ k_ex610909_ ≤ 529, 0
  ≤ k_ex610910_ ∧ k_ex610910_ ≤ 33 -/
def branch_content_length_add (current : Nat) (addition : Nat) : SailM Nat := do
  if ((addition ≤b (529 - current)) : Bool)
  then (pure (current + addition))
  else (fatal_error RlpDecode)

