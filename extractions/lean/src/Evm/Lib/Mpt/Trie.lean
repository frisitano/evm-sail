import Evm.Lib.Mpt.Updates

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

/-! # The witness-native trie

The witness-native Ethereum Merkle-Patricia trie. [trie_root][] merges
ordered updates into an authenticated base trie and fails closed when a
touched hash is absent from the witness. With an empty base the same
builder computes the Yellow Paper `TRIE(I)` directly.

## The witness walker

`witness_subtree(node, evm_prefix, updates, cursor)` returns the post-state
subtree rooted at `node` together with the first update after that subtree.
`updates` is a cursor into the one globally sorted update stream; each call
consumes its contiguous evm_prefix range and returns the cursor to its parent.

The walker descends only along touched paths; untouched children pass
through as single reference items with zero node-db work. Deletes are
consumed here and only here: a delete suppresses its base leaf, and a delete
with no base leaf (the walk proves absence) contributes no subtree. The
recursive child combiner collapses branches as it returns. RLP fields retain
their source and spans, so embedded nodes and leaf values remain witness
slices. -/

def undefined_TrieRootResult (_ : Unit) : SailM TrieRootResult := do
  (pure { root := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          changed := ← (undefined_bool ()) })

/-- The root of the trie after pulling and applying the source's ordered
updates. The source's host iterator must be opened by its owner first. -/
def trie_root (base_root : (Vector (BitVec 8) 32)) (source : TrieUpdateSource) : SailM TrieRootResult := do
  let updates ← do (trie_updates_begin source)
  (trie_root_cursor base_root updates)

