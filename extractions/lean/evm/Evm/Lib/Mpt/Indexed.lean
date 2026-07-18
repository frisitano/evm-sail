import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Host.Kernel.Storage

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
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
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
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def undefined_RlpIndexCursor (_ : Unit) : SailM RlpIndexCursor := do
  (pure { count := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          position := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩) })

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
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

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def trie_index_key (index : item_index) : SailM TriePath := do
  let index := (index).value
  if ((index == 0) : Bool)
  then (pure (path_new ((Sail.BitVec.zeroExtend 0x80#8 256) <<< 248) ⟨2⟩))
  else
    (do
      if ((index ≤b 127) : Bool)
      then (pure (path_new ((← (word_of_nat index)) <<< 248) ⟨2⟩))
      else
        (do
          let width := ((rlp_index_encoded_width ⟨index⟩)).value
          let byte_len := (width + 1)
          let encoded ← do
            (pure (((← (word_of_nat (128 + width))) <<< (width *i 8)) ||| (← (word_of_nat index))))
          let align_shift := (256 -i (byte_len *i 8))
          (pure (path_new (encoded <<< align_shift) ⟨(byte_len *i 2)⟩))))

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def rlp_index_cursor (count : item_count) : RlpIndexCursor :=
  let count := (count).value
  { count := ⟨count⟩,
    position := ⟨0⟩ }

def rlp_index_cursor_empty (cursor : RlpIndexCursor) : Bool :=
  ((cursor.count).value ≤b (cursor.position).value)

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
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

