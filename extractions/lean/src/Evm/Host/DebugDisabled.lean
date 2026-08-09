import Evm.Primitives.Crypto

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

/-! # Validation debug (optional)

Default build variant discarding the optional validation debug
records.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- Clears the recorded failure at the start of a validation attempt. -/
def validation_debug_reset (_ : Unit) : Unit :=
  ()

/- Type quantifiers: _actual : Nat, _expected : Nat, _execution : Nat, _state : Nat, 0 ≤ _actual
  ∧
  _actual ≤ block_gas_limit_bound ∧
  0 ≤ _expected ∧
  _expected ≤ block_gas_limit_bound ∧
  0 ≤ _execution ∧
  _execution ≤ block_gas_limit_bound ∧ 0 ≤ _state ∧ _state ≤ block_gas_limit_bound -/
def validation_debug_capture_block_gas (_actual : Nat) (_expected : Nat) (_execution : Nat) (_state : Nat) : Unit :=
  ()

/-- Records the failing validation stage and reason for host
inspection; a later record overwrites an earlier one. -/
/- Type quantifiers: k_ex553912_ : Nat, 0 ≤ k_ex553912_ ∧ k_ex553912_ ≤ 255 -/
def validation_debug_record (_stage : Nat) (_reason : FatalError) : Unit :=
  ()

def debug_account_storage_root (_a : (Vector (BitVec 8) 20)) : (Vector (BitVec 8) 32) :=
  EMPTY_TRIE_ROOT

