import Evm.Flow
import Evm.Host.Kernel.Storage
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

/-! # Ordered trie indices

Transaction, withdrawal, and receipt tries use RLP-encoded list indices as
keys. Their bytewise trie order differs from numeric order, so this cursor
emits indices directly in canonical key order. -/

/- Type quantifiers: atom_maximum : Nat, (rlp_index_valid_maximum atom_maximum) -/
def undefined_RlpIndexCursor (atom_maximum : Nat) : SailM (RlpIndexCursor atom_maximum) := do
  (pure { count := ← (undefined_range 0 atom_maximum),
          position := ← (undefined_range 0 atom_maximum) })

/-- Decrements a positive RLP index byte width. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 8 -/
def rlp_index_byte_width_decrement (value : Nat) : SailM Nat := do
  assert (0 <b value) "sail/lib/mpt/indexed.sail:33.20-33.21"
  (pure (value - 1))

/-- Returns the minimal byte width of an indexed-trie position. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def rlp_index_encoded_width (value : Nat) : Nat :=
  if ((value <b 256) : Bool)
  then 1
  else
    (if ((value <b 65536) : Bool)
    then 2
    else
      (if ((value <b 16777216) : Bool)
      then 3
      else
        (if ((value <b 4294967296) : Bool)
        then 4
        else
          (if ((value <b 1099511627776) : Bool)
          then 5
          else
            (if ((value <b 281474976710656) : Bool)
            then 6
            else
              (if ((value <b 72057594037927936) : Bool)
              then 7
              else 8))))))

/-- The transactions/withdrawals-trie key for list index `i`:
`rlp(i)` as a nibble path (YP §4.4.2). -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def trie_index_key (index : Nat) : SailM TriePath := do
  if ((index == 0) : Bool)
  then (path_append_byte (path_empty ()) 0x80#8)
  else
    (do
      if ((index ≤b 127) : Bool)
      then (path_append_byte (path_empty ()) (get_slice_int 8 index 0))
      else
        (do
          let width := (rlp_index_encoded_width index)
          let path ← do (path_append_byte (path_empty ()) (get_slice_int 8 (128 + width) 0))
          let remaining : Nat := width
          let (path, remaining) ← (( do
            let loop__offset_lower := 0
            let loop__offset_upper := 7
            let mut loop_vars := (path, remaining)
            for _offset in [loop__offset_lower:loop__offset_upper:1]i do
              let (path, remaining) := loop_vars
              loop_vars ← do
                let (path, remaining) ← (( do
                  if ((remaining != 0) : Bool)
                  then
                    (do
                      let byte_offset ← do (rlp_index_byte_width_decrement remaining)
                      let shift : Nat := (byte_offset *i 8)
                      let path ← (path_append_byte path (get_slice_int 8 index shift))
                      let remaining : Nat := byte_offset
                      (pure (path, remaining)))
                  else (pure (path, remaining)) ) : SailM (TriePath × Nat) )
                (pure (path, remaining))
            (pure loop_vars) ) : SailM (TriePath × Nat) )
          (pure path)))

/-- Starts canonical RLP-index traversal for a bounded collection. -/
/- Type quantifiers: count : Nat, k_maximum : Nat, (rlp_index_valid_maximum k_maximum), 0 ≤ count
  ∧ count ≤ k_maximum -/
def rlp_index_cursor (count : Nat) : (RlpIndexCursor k_maximum) :=
  { count := count,
    position := 0 }

/-- Whether canonical RLP-index traversal has consumed every index. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_cursor_empty (cursor : (RlpIndexCursor k_maximum)) : Bool :=
  (cursor.count ≤b cursor.position)

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
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- Removes the next canonical-key item and advances the cursor. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
def rlp_index_cursor_pop (cursor : (RlpIndexCursor k_maximum)) : SailM ((RlpIndexItem k_maximum) × (RlpIndexCursor k_maximum)) := do
  let count := cursor.count
  let position := cursor.position
  if ((position <b count) : Bool)
  then
    (do
      let index ← do (rlp_index_at_position cursor)
      let next_cursor : (RlpIndexCursor k_maximum) :=
        { count := count,
          position := (position + 1) }
      let next_key ← do
        if ((rlp_index_cursor_empty next_cursor) : Bool)
        then (pure none)
        else (pure (some (← (trie_index_key (← (rlp_index_at_position next_cursor))))))
      (pure ({ index := index,
               key := ← (trie_index_key index),
               next_key := next_key }, next_cursor)))
  else sailThrow ((InvalidBlock WitnessDeficient))

