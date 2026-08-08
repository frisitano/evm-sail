import Evm.Flow
import Evm.Exceptions
import Evm.Kernel.Storage
import Evm.Lib.Mpt.Primitives

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

/-! # Ordered trie indices

Transaction, withdrawal, and receipt tries use RLP-encoded list indices as
keys. Their bytewise trie order differs from numeric order, so this cursor
emits indices directly in canonical key order. -/

/- Type quantifiers: atom_maximum : Nat, (rlp_index_valid_maximum atom_maximum) -/
def undefined_RlpIndexItem (atom_maximum : Nat) : SailM (RlpIndexItem atom_maximum) := do
  (pure { index := ← (undefined_range 0 (atom_maximum - 1)),
          key := ← (undefined_TriePath ()) })

/- Type quantifiers: atom_maximum : Nat, (rlp_index_valid_maximum atom_maximum) -/
def undefined_RlpIndexCursor (atom_maximum : Nat) : SailM (RlpIndexCursor atom_maximum) := do
  (pure { count := ← (undefined_range 0 atom_maximum),
          position := ← (undefined_range 0 atom_maximum),
          current := ← (undefined_RlpIndexItem atom_maximum) })

/-- Returns the minimal byte width of an indexed-trie position. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 20 - 1) -/
def rlp_index_encoded_width (value : Nat) : Nat :=
  if ((value <b 256) : Bool)
  then 1
  else
    (if ((value <b 65536) : Bool)
    then 2
    else 3)

/-- The transactions/withdrawals-trie key for list index `i`:
`rlp(i)` as a nibble path (YP §4.4.2). -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 20 - 1) -/
def trie_index_key (index : Nat) : SailM TriePath := do
  if ((index == 0) : Bool)
  then
    (do
      let empty_path := (path_empty ())
      (path_append_byte empty_path 0x80#8))
  else
    (do
      if ((index ≤b 127) : Bool)
      then
        (do
          let empty_path := (path_empty ())
          let encoded_index := (get_slice_int 8 index 0)
          (path_append_byte empty_path encoded_index))
      else
        (do
          let width := (rlp_index_encoded_width index)
          let empty_path := (path_empty ())
          let encoded_prefix := (get_slice_int 8 (128 + width) 0)
          let path ← do (path_append_byte empty_path encoded_prefix)
          let remaining : Nat := width
          let (path, remaining) ← (( do
            let loop_vars ← whileFuelM (fuel :=remaining) (fun (path, remaining) => (pure (remaining != 0))) (path, remaining)
              fun (path, remaining) => do
                assert true "loop dummy assert"
                let current_remaining := remaining
                let byte_offset ← (( do
                  if ((0 <b current_remaining) : Bool)
                  then (pure (current_remaining - 1))
                  else (fatal_error WitnessDeficient) ) : SailM Nat )
                let shift : Nat := (byte_offset *i 8)
                let encoded_index := (get_slice_int 8 index shift)
                let path ← (path_append_byte path encoded_index)
                let remaining : Nat := byte_offset
                (pure (path, remaining))
            (pure loop_vars) ) : SailM (TriePath × Nat) )
          (pure path)))

/-- Maps a canonical-key cursor position back to its numeric list index. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_at_position (cursor : (RlpIndexCursor k_maximum)) : SailM Nat := do
  let count := cursor.count
  let position := cursor.position
  if ((position <b count) : Bool)
  then
    (let rest : Nat := (count - 1)
    let single_count : Nat :=
      if ((rest <b 127) : Bool)
      then rest
      else 127
    if ((position <b single_count) : Bool)
    then (pure (position + 1))
    else
      (if ((position == single_count) : Bool)
      then (pure 0)
      else (pure position)))
  else (fatal_error WitnessDeficient)

/-- Starts canonical RLP-index traversal for a bounded collection. -/
/- Type quantifiers: count : Nat, k_maximum : Nat, (rlp_index_valid_maximum k_maximum), 0 ≤ count
  ∧ count ≤ k_maximum -/
def rlp_index_cursor (count : Nat) : SailM (RlpIndexCursor k_maximum) := do
  let cursor : (RlpIndexCursor k_maximum) :=
    { count := count,
      position := 0,
      current := { index := 0,
                   key := (path_empty ()) } }
  if ((count != 0) : Bool)
  then
    (do
      let index ← do (rlp_index_at_position cursor)
      (pure { cursor with current := ← (pure { index := index,
                                                 key := ← (trie_index_key index) }) }))
  else (pure cursor)

/-- Whether canonical RLP-index traversal has consumed every index. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_cursor_empty (cursor : (RlpIndexCursor k_maximum)) : Bool :=
  (cursor.position == cursor.count)

/-- Returns the cached numeric index and trie key without advancing. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_cursor_peek (cursor : (RlpIndexCursor k_maximum)) : SailM (RlpIndexItem k_maximum) := do
  if ((cursor.position <b cursor.count) : Bool)
  then (pure cursor.current)
  else (fatal_error WitnessDeficient)

/-- Consumes the current position and returns its fully populated successor. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_cursor_advance (cursor : (RlpIndexCursor k_maximum)) : SailM (RlpIndexCursor k_maximum) := do
  let count := cursor.count
  let position := cursor.position
  if ((position <b count) : Bool)
  then
    (do
      let next : (RlpIndexCursor k_maximum) :=
        { count := count,
          position := (position + 1),
          current := { index := 0,
                       key := (path_empty ()) } }
      if ((next.position <b count) : Bool)
      then
        (do
          let index ← do (rlp_index_at_position next)
          (pure { next with current := ← (pure { index := index,
                                                   key := ← (trie_index_key index) }) }))
      else (pure next))
  else (fatal_error WitnessDeficient)

/-- Removes the cached indexed item and advances the cursor. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_cursor_pop (cursor : (RlpIndexCursor k_maximum)) : SailM ((RlpIndexItem k_maximum) × (RlpIndexCursor k_maximum)) := do
  let item ← do (rlp_index_cursor_peek cursor)
  (pure (item, (← (rlp_index_cursor_advance cursor))))

