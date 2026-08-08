import Evm.Exceptions
import Evm.Lib.Rlp.Decoding

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

/-! # Block access list RLP decoding

Canonical list, byte-string, and integer field decoding shared by EIP-7928
validation. -/

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_cursor (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (StatelessInputSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len) := do
  let framing_canonical ← do (rlp_ref_framing_canonical f)
  if ((f.is_list && framing_canonical) : Bool)
  then (rlp_decode_list f)
  else (fatal_error InvalidBlockAccessList)

/-- Requires a canonical RLP byte string and returns its content slice. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_bytes (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  let canonical ← do (rlp_ref_bytes_canonical f)
  if _sailIf0 : (canonical : Bool) = true
  then
    (pure ((rlp_item_content f) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
  else
    (do
      (fatal_error InvalidBlockAccessList))

/-- Requires a canonical RLP integer in the EVM-word domain. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_word (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let canonical ← do (rlp_item_uint_canonical f)
  if ((canonical && (k_content_len ≤b 32)) : Bool)
  then (rlp_decode_word f)
  else (fatal_error InvalidBlockAccessList)

/-- Requires a canonical RLP integer in the host-index/nonce domain. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_uint64 (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  let canonical ← do (rlp_item_uint_canonical f)
  if ((canonical && (k_content_len ≤b 8)) : Bool)
  then (rlp_decode_uint64 f)
  else (fatal_error InvalidBlockAccessList)

/-- Requires that a decoded list has no unconsumed children. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_expect_end (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Unit := do
  if ((k_source_len == 0) : Bool)
  then (pure ())
  else (fatal_error InvalidBlockAccessList)

