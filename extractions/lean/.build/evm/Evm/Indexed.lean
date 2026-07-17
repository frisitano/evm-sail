import Evm.Prelude
import Evm.Quantities
import Evm.Storage

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

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
open Register
open NodeRef
open MerkleSlot
open JEntry
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
  (pure { count := ← (undefined_range 0 ((2 ^i 64) -i 1))
          position := ← (undefined_range 0 ((2 ^i 64) -i 1)) })

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
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

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def trie_index_key (index : Nat) : SailM TriePath := do
  if ((index == 0) : Bool)
  then (pure (path_new ((Sail.BitVec.zeroExtend 0x80#8 256) <<< 248) 2))
  else
    (do
      if ((index ≤b 127) : Bool)
      then (pure (path_new ((← (word_of_nat index)) <<< 248) 2))
      else
        (do
          let width := (rlp_index_encoded_width index)
          let byte_len := (width +i 1)
          let encoded ← do
            (pure (((← (word_of_nat (128 +i width))) <<< (width *i 8)) ||| (← (word_of_nat index))))
          let align_shift := (256 -i (byte_len *i 8))
          (pure (path_new (encoded <<< align_shift) (byte_len *i 2)))))

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def rlp_index_cursor (count : Nat) : RlpIndexCursor :=
  { count := count
    position := 0 }

def rlp_index_cursor_empty (cursor : RlpIndexCursor) : Bool :=
  (cursor.count ≤b cursor.position)

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ (2 ^ 64 - 1) -/
def rlp_index_single_count (count : Nat) : SailM Nat := do
  if ((count == 0) : Bool)
  then (pure 0)
  else
    (do
      let rest ← do (protocol_quantity_decrement count)
      if ((rest <b 127) : Bool)
      then (pure rest)
      else (pure 127))

def rlp_index_at_position (cursor : RlpIndexCursor) : SailM Nat := do
  if ((rlp_index_cursor_empty cursor) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let single_count ← do (rlp_index_single_count cursor.count)
  if ((cursor.position <b single_count) : Bool)
  then (protocol_quantity_increment cursor.position)
  else
    (if ((cursor.position == single_count) : Bool)
    then (pure 0)
    else (pure cursor.position))

def rlp_index_cursor_pop (cursor : RlpIndexCursor) : SailM (RlpIndexItem × RlpIndexCursor) := do
  let index ← do (rlp_index_at_position cursor)
  let next_cursor ← (( do
    (pure { count := cursor.count
            position := ← (protocol_quantity_increment cursor.position) }) ) : SailM
    RlpIndexCursor )
  let next_key ← do
    if ((rlp_index_cursor_empty next_cursor) : Bool)
    then (pure none)
    else (pure (some (← (trie_index_key (← (rlp_index_at_position next_cursor))))))
  (pure ({ index := index
           key := ← (trie_index_key index)
           next_key := next_key }, next_cursor))

