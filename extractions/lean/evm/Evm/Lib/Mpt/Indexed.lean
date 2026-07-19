import Evm.Flow
import Evm.Primitives.Quantities
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

/-! # Ordered trie indices

Transaction, withdrawal, and receipt tries use RLP-encoded list indices as
keys. Their bytewise trie order differs from numeric order, so this cursor
emits indices directly in canonical key order. -/

def undefined_RlpIndexCursor (_ : Unit) : SailM RlpIndexCursor := do
  (pure { count := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          position := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩) })

/-- Decrements a positive RLP index byte width. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 8 -/
def rlp_index_byte_width_decrement (value : Nat) : SailM Nat := do
  assert (0 <b value) "sail/lib/mpt/indexed.sail:27.20-27.21"
  (pure (value -i 1))

/-- Returns the minimal byte width of an indexed-trie position. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def rlp_index_encoded_width (value : item_index) : rlp_index_byte_width :=
  let value := (value).value
  ⟨if ((value <b 256) : Bool)
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
              else 8))))))⟩

/-- The transactions/withdrawals-trie key for list index `i`:
`rlp(i)` as a nibble path (YP §4.4.2). -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def trie_index_key (index : item_index) : SailM TriePath := do
  let index := (index).value
  if ((index == 0) : Bool)
  then (path_append_byte (path_empty ()) 0x80#8)
  else
    (do
      if ((index ≤b 127) : Bool)
      then (path_append_byte (path_empty ()) (get_slice_int 8 index 0))
      else
        (do
          let width := ((rlp_index_encoded_width ⟨index⟩)).value
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

/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def rlp_index_cursor (count : item_count) : RlpIndexCursor :=
  let count := (count).value
  { count := ⟨count⟩,
    position := ⟨0⟩ }

def rlp_index_cursor_empty (cursor : RlpIndexCursor) : Bool :=
  ((cursor.count).value ≤b (cursor.position).value)

/-- Counts the one-byte keys that sort before RLP index zero. -/
/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def rlp_index_single_count (count : item_count) : SailM item_count := do
  let count := (count).value
  let semanticResult ← do
    if ((count == 0) : Bool)
    then (pure 0)
    else
      (do
        let rest ← do
          (do
              let semanticResult ← (protocol_quantity_decrement ⟨count⟩)
              pure ((semanticResult).value))
        if ((rest <b 127) : Bool)
        then (pure rest)
        else (pure 127))
  pure (⟨semanticResult⟩)

/-- Maps a canonical-key cursor position back to its numeric list index. -/
def rlp_index_at_position (cursor : RlpIndexCursor) : SailM item_index := do
  let semanticResult ← do
    if ((rlp_index_cursor_empty cursor) : Bool)
    then sailThrow ((InvalidBlock WitnessDeficient))
    else (pure ())
    let single_count ← do
      (do
          let semanticResult ← (rlp_index_single_count ⟨(cursor.count).value⟩)
          pure ((semanticResult).value))
    if (((cursor.position).value <b single_count) : Bool)
    then
      (do
          let semanticResult ← (protocol_quantity_increment ⟨(cursor.position).value⟩)
          pure ((semanticResult).value))
    else
      (if (((cursor.position).value == single_count) : Bool)
      then (pure 0)
      else (pure (cursor.position).value))
  pure (⟨semanticResult⟩)

/-- Removes the next canonical-key item and advances the cursor. -/
def rlp_index_cursor_pop (cursor : RlpIndexCursor) : SailM (RlpIndexItem × RlpIndexCursor) := do
  let index ← do
    (do
        let semanticResult ← (rlp_index_at_position cursor)
        pure ((semanticResult).value))
  let next_cursor ← (( do
    (pure { count := ⟨(cursor.count).value⟩,
            position := ← do
                let semanticField ← (do
                    let semanticResult ← (protocol_quantity_increment
                    ⟨(cursor.position).value⟩)
                    pure ((semanticResult).value))
                pure (⟨semanticField⟩) }) ) : SailM RlpIndexCursor )
  let next_key ← do
    if ((rlp_index_cursor_empty next_cursor) : Bool)
    then (pure none)
    else
      (pure (some (← (trie_index_key ⟨((← (rlp_index_at_position next_cursor))).value⟩))))
  (pure ({ index := ⟨index⟩,
           key := ← (trie_index_key ⟨index⟩),
           next_key := next_key }, next_cursor))

