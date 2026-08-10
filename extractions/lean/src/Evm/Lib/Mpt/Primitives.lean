import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
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

/-! # Trie paths and hex-evm_prefix encoding

Nibble paths through the Merkle-Patricia trie and the compact (hex-evm_prefix)
encoding of YP Appendix C. -/

def undefined_TriePath (_ : Unit) : SailM TriePath := do
  (pure { data := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          len := ← (undefined_range 0 64) })

/-- Appends one nibble to a path, rejecting paths already at the key bound. -/
def path_append_nibble (path : TriePath) (value : (BitVec 4)) : SailM TriePath := do
  let length := (path_len path)
  if ((length <b 64) : Bool)
  then
    (do
      let original := path.data
      let bytes := original
      let byte_index ← do (path_byte_index length)
      let parity := (Nat.mod length 2)
      let bytes : (Vector (BitVec 8) 32) :=
        if ((parity == 0) : Bool)
        then (vectorUpdate bytes byte_index (value +++ 0x0#4))
        else
          (vectorUpdate bytes byte_index
            ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4) +++ value))
      let path_data := (B256 bytes)
      (pure (path_new path_data (length + 1))))
  else (fatal_error WitnessDeficient)

/-- Appends both nibbles of a byte to a path, high nibble first. -/
def path_append_byte (path : TriePath) (value : (BitVec 8)) : SailM TriePath := do
  let high_nibble ← do (path_append_nibble path (Sail.BitVec.extractLsb value 7 4))
  (path_append_nibble high_nibble (Sail.BitVec.extractLsb value 3 0))

/-- A one-nibble path. -/
def path_single (n : (BitVec 4)) : SailM TriePath := do
  let empty_path := (path_empty ())
  (path_append_nibble empty_path n)

/-- Path concatenation; over 64 nibbles is a witness fault. -/
def path_concat (a : TriePath) (b : TriePath) : SailM TriePath := do
  let alen := (path_len a)
  let blen := (path_len b)
  let combined := (alen + blen)
  if ((combined ≤b 64) : Bool)
  then
    (do
      let result := a
      let index : Nat := 0
      let (index, result) ← (( do
        let loop_vars ← whileFuelM (fuel :=(blen -i index)) (fun (index, result) => (pure (index <b blen))) (index, result)
          fun (index, result) => do
            assert true "loop dummy assert"
            let nibble ← do (path_nibble b index)
            let result ← (path_append_nibble result nibble)
            let current_index := index
            let index ←
              if ((current_index <b 64) : Bool)
              then (pure (current_index + 1))
              else (fatal_error WitnessDeficient)
            (pure (index, result))
        (pure loop_vars) ) : SailM (Nat × TriePath) )
      (pure result))
  else (fatal_error WitnessDeficient)

/-- The path with its first `n` nibbles removed. -/
/- Type quantifiers: k_ex553441_ : Nat, 0 ≤ k_ex553441_ ∧ k_ex553441_ ≤ 64 -/
def path_drop (path : TriePath) (n : Nat) : SailM TriePath := do
  let length := (path_len path)
  if ((length ≤b n) : Bool)
  then (pure (path_empty ()))
  else
    (do
      if ((n == 0) : Bool)
      then (pure path)
      else
        (do
          let remain : Nat := (length - n)
          let result := (path_empty ())
          let offset : Nat := 0
          let (offset, result) ← (( do
            let loop_vars ← whileFuelM (fuel :=(remain -i offset)) (fun (offset, result) => (pure (offset <b remain))) (offset, result)
              fun (offset, result) => do
                assert true "loop dummy assert"
                let candidate := (n + offset)
                let source_index ← (( do
                  if (((0 ≤b candidate) && (candidate ≤b 64)) : Bool)
                  then (pure candidate)
                  else
                    (do
                      assert false "sail/lib/mpt/primitives.sail:141.32-141.33"
                      throw Error.Exit) ) : SailM Nat )
                let nibble ← do (path_nibble path source_index)
                let result ← (path_append_nibble result nibble)
                let current_offset := offset
                let offset ←
                  if ((current_offset <b 64) : Bool)
                  then (pure (current_offset + 1))
                  else (fatal_error WitnessDeficient)
                (pure (offset, result))
            (pure loop_vars) ) : SailM (Nat × TriePath) )
          (pure result)))

/-- Path equality. -/
def path_eq (a : TriePath) (b : TriePath) : Bool :=
  ((a.len == b.len) && (a.data == b.data))

/-- Whether `evm_prefix` is a evm_prefix of `path`. -/
def path_prefix_of (evm_prefix' : TriePath) (path : TriePath) : SailM Bool := do
  (path_matches path 0 evm_prefix')

/-- The common-evm_prefix length of two canonical nibble paths. -/
def common_prefix_length (a : TriePath) (b : TriePath) : SailM Nat := do
  let alen := (path_len a)
  let blen := (path_len b)
  let stop :=
    if ((alen <b blen) : Bool)
    then alen
    else blen
  let length : Nat := 0
  let matching : Bool := true
  let (length, matching) ← (( do
    let loop_vars ← whileFuelM (fuel :=(stop -i length)) (fun (length, matching) => (pure (matching && ((length <b stop) : Bool)))) (length, matching)
      fun (length, matching) => do
        assert true "loop dummy assert"
        let a_nibble ← do (path_nibble a length)
        let b_nibble ← do (path_nibble b length)
        let (length, matching) ← (( do
          if ((a_nibble == b_nibble) : Bool)
          then
            (do
              let current_length := length
              let length ←
                if ((current_length <b 64) : Bool)
                then (pure (current_length + 1))
                else (fatal_error WitnessDeficient)
              (pure (length, matching)))
          else
            (let matching : Bool := false
            (pure (length, matching))) ) : SailM (Nat × Bool) )
        (pure (length, matching))
    (pure loop_vars) ) : SailM (Nat × Bool) )
  (pure length)

/-- The encoded byte length of the hex-evm_prefix form of a trie path. -/
def hex_prefix_encoded_length (path : TriePath) : Nat :=
  let length : Nat := (path_len path)
  let packed_pair_count : Nat := (Nat.div length 2)
  (1 + packed_pair_count)

/-- The flag byte beginning the hex-evm_prefix form of a trie path. -/
/- Type quantifiers: k_ex553442_ : Bool -/
def hex_prefix_first_byte (path : TriePath) (is_leaf : Bool) : SailM (BitVec 8) := do
  let length : Nat := (path_len path)
  let odd := ((Nat.mod length 2) != 0)
  let flag : (BitVec 4) :=
    if (is_leaf : Bool)
    then 0x2#4
    else 0x0#4
  if (odd : Bool)
  then
    (do
      let first_nibble ← do (path_nibble path 0)
      (pure ((flag ||| 0x1#4) +++ first_nibble)))
  else (pure (flag +++ 0x0#4))

/-- Scratch-backed counterpart used only when canonicalization reopens an
embedded node that it just encoded. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def scratch_hex_prefix_decode_ref (f : (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Bool × TriePath) := do
  if (f.is_list : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let n := k_content_len
  if ((n == 0) : Bool)
  then (pure (false, (path_empty ())))
  else
    (do
      let maximum_length := HEX_PREFIX_MAX_LENGTH
      if ((maximum_length <b n) : Bool)
      then (fatal_error RlpDecode)
      else
        (do
          let content := (scratch_sub_slice f.source (k_source_len - n) n)
          let fb ← do (scratch_byte ⟨_, ⟨_, content⟩⟩ 0)
          let flag : (BitVec 4) := (Sail.BitVec.extractLsb fb 7 4)
          let is_leaf : Bool := ((BitVec.access flag 1) == 1#1)
          let odd : Bool := ((BitVec.access flag 0) == 1#1)
          let tail_length : Nat := (n - 1)
          let tail := (scratch_slice_suffix content 1)
          let packed ← do (scratch_slice_load ⟨_, ⟨_, tail⟩⟩ 0)
          let paired_nibbles : Nat := (tail_length *i 2)
          if (odd : Bool)
          then
            (do
              if ((paired_nibbles <b 64) : Bool)
              then
                (let shifted := (word_shift_right packed 4)
                let bytes := (word_to_hash shifted)
                let bytes : (Vector (BitVec 8) 32) :=
                  (vectorUpdate bytes 0
                    ((Sail.BitVec.extractLsb fb 3 0) +++ (Sail.BitVec.extractLsb
                        (GetElem?.getElem! bytes 0) 3 0)))
                let path_data := (B256 bytes)
                let path := (path_new path_data (paired_nibbles + 1))
                (pure (is_leaf, path)))
              else (fatal_error WitnessDeficient))
          else
            (let path_data := (word_to_hash packed)
            let path := (path_new path_data paired_nibbles)
            (pure (is_leaf, path)))))

