import Evm.Flow
import Evm.Prelude
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

/-! # Trie paths and hex-evm_prefix encoding

Nibble paths through the Merkle-Patricia trie and the compact (hex-evm_prefix)
encoding of YP Appendix C. -/

def undefined_TriePath (_ : Unit) : SailM TriePath := do
  (pure { data := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          len := ← do
              let publicField ← (undefined_range 0 64)
              pure (⟨publicField⟩) })

/-- Narrows a nonterminal path position to a branch depth. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def to_trie_depth (value : trie_path_len) : SailM trie_depth := do
  let value := (value).value
  let publicResult ← do
    if ((64 ≤b value) : Bool)
    then sailThrow ((InvalidBlock WitnessDeficient))
    else (pure value)
  pure (⟨publicResult⟩)

/-- Appends one nibble to a path, rejecting paths already at the key bound. -/
def path_append_nibble (path : TriePath) (value : nibble) : SailM TriePath := do
  let length := ((path_len path)).value
  if ((64 ≤b length) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let original := path.data
  let bytes := original
  let byte_index ← do
    (do
        let publicResult ← (path_byte_index ⟨length⟩)
        pure ((publicResult).value))
  let bytes : (Vector (BitVec 8) 32) :=
    if (((Nat.mod length 2) == 0) : Bool)
    then (vectorUpdate bytes byte_index (value +++ 0x0#4))
    else
      (vectorUpdate bytes byte_index
        ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes byte_index) 7 4) +++ value))
  (pure (path_new (B256 bytes) ⟨((← (trie_path_len_increment ⟨length⟩))).value⟩))

def path_append_byte (path : TriePath) (value : byte) : SailM TriePath := do
  (path_append_nibble (← (path_append_nibble path (Sail.BitVec.extractLsb value 7 4)))
    (Sail.BitVec.extractLsb value 3 0))

/-- A one-nibble path. -/
def path_single (n : nibble) : SailM TriePath := do
  (path_append_nibble (path_empty ()) n)

/-- Path concatenation; over 64 nibbles is a witness fault. -/
def path_concat (a : TriePath) (b : TriePath) : SailM TriePath := do
  let alen := ((path_len a)).value
  let blen := ((path_len b)).value
  let combined := (alen + blen)
  if ((combined ≤b 64) : Bool)
  then
    (do
      let result := a
      let index : Nat := 0
      let (index, result) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 63
        let mut loop_vars := (index, result)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (index, result) := loop_vars
          loop_vars ← do
            let (index, result) ← (( do
              if ((index <b blen) : Bool)
              then
                (do
                  let result ← (path_append_nibble result (← (path_nibble b ⟨index⟩)))
                  let index ←
                    (do
                        let publicResult ← (trie_path_len_increment ⟨index⟩)
                        pure ((publicResult).value))
                  (pure (index, result)))
              else (pure (index, result)) ) : SailM (Nat × TriePath) )
            (pure (index, result))
        (pure loop_vars) ) : SailM (Nat × TriePath) )
      (pure result))
  else sailThrow ((InvalidBlock WitnessDeficient))

/-- The first `n` nibbles. -/
/- Type quantifiers: k_ex408801_ : Nat, 0 ≤ k_ex408801_ ∧ k_ex408801_ ≤ 64 -/
def path_take (path : TriePath) (n : trie_path_len) : SailM TriePath := do
  let n := (n).value
  if ((n == 0) : Bool)
  then (pure (path_empty ()))
  else
    (do
      if ((((path_len path)).value ≤b n) : Bool)
      then (pure path)
      else
        (do
          let result := (path_empty ())
          let index : Nat := 0
          let (index, result) ← (( do
            let loop__step_lower := 0
            let loop__step_upper := 63
            let mut loop_vars := (index, result)
            for _step in [loop__step_lower:loop__step_upper:1]i do
              let (index, result) := loop_vars
              loop_vars ← do
                let (index, result) ← (( do
                  if ((index <b n) : Bool)
                  then
                    (do
                      let result ←
                        (path_append_nibble result (← (path_nibble path ⟨index⟩)))
                      let index ←
                        (do
                            let publicResult ← (trie_path_len_increment ⟨index⟩)
                            pure ((publicResult).value))
                      (pure (index, result)))
                  else (pure (index, result)) ) : SailM (Nat × TriePath) )
                (pure (index, result))
            (pure loop_vars) ) : SailM (Nat × TriePath) )
          (pure result)))

/-- The path with its first `n` nibbles removed. -/
/- Type quantifiers: k_ex408802_ : Nat, 0 ≤ k_ex408802_ ∧ k_ex408802_ ≤ 64 -/
def path_drop (path : TriePath) (n : trie_path_len) : SailM TriePath := do
  let n := (n).value
  let length := ((path_len path)).value
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
            let loop__step_lower := 0
            let loop__step_upper := 63
            let mut loop_vars := (offset, result)
            for _step in [loop__step_lower:loop__step_upper:1]i do
              let (offset, result) := loop_vars
              loop_vars ← do
                let (offset, result) ← (( do
                  if ((offset <b remain) : Bool)
                  then
                    (do
                      let candidate := (n + offset)
                      let source_index ← (( do
                        if (((0 ≤b candidate) && (candidate ≤b 64)) : Bool)
                        then (pure candidate)
                        else
                          (do
                            assert false "sail/lib/mpt/primitives.sail:166.40-166.41"
                            throw Error.Exit) ) : SailM Nat )
                      let result ←
                        (path_append_nibble result (← (path_nibble path ⟨source_index⟩)))
                      let offset ←
                        (do
                            let publicResult ← (trie_path_len_increment ⟨offset⟩)
                            pure ((publicResult).value))
                      (pure (offset, result)))
                  else (pure (offset, result)) ) : SailM (Nat × TriePath) )
                (pure (offset, result))
            (pure loop_vars) ) : SailM (Nat × TriePath) )
          (pure result)))

/-- Path equality. -/
def path_eq (a : TriePath) (b : TriePath) : Bool :=
  (((a.len).value == (b.len).value) && (a.data == b.data))

/-- Lexicographic path order (data, then length). -/
def path_lt (a : TriePath) (b : TriePath) : Bool :=
  if ((a.data == b.data) : Bool)
  then (((path_len a)).value <b ((path_len b)).value)
  else (hash_lt a.data b.data)

/-- Whether `evm_prefix` is a evm_prefix of `path`. -/
def path_prefix_of (evm_prefix' : TriePath) (path : TriePath) : SailM Bool := do
  if ((((path_len path)).value <b ((path_len evm_prefix')).value) : Bool)
  then (pure false)
  else (pure (path_eq evm_prefix' (← (path_take path ⟨((path_len evm_prefix')).value⟩))))

/-- The length of the common evm_prefix of `a` and `b` starting at nibble
`start`. -/
/- Type quantifiers: k_ex408803_ : Nat, 0 ≤ k_ex408803_ ∧ k_ex408803_ ≤ 64 -/
def common_prefix_from (a : TriePath) (b : TriePath) (start : trie_path_len) : SailM trie_path_len := do
  let start := (start).value
  let publicResult ← do
    let alen := ((path_len a)).value
    let blen := ((path_len b)).value
    let stop :=
      if ((alen <b blen) : Bool)
      then alen
      else blen
    let index : Nat := start
    let count : Nat := 0
    let matching : Bool := true
    let (count, index, matching) ← (( do
      let loop__step_lower := 0
      let loop__step_upper := 63
      let mut loop_vars := (count, index, matching)
      for _step in [loop__step_lower:loop__step_upper:1]i do
        let (count, index, matching) := loop_vars
        loop_vars ← do
          let (count, index, matching) ← (( do
            if ((matching && ((index <b stop) : Bool)) : Bool)
            then
              (do
                let (count, index, matching) ← (( do
                  if (((← (path_nibble a ⟨index⟩)) == (← (path_nibble b ⟨index⟩))) : Bool)
                  then
                    (do
                      let count ←
                        (do
                            let publicResult ← (trie_path_len_increment ⟨count⟩)
                            pure ((publicResult).value))
                      let index ←
                        (do
                            let publicResult ← (trie_path_len_increment ⟨index⟩)
                            pure ((publicResult).value))
                      (pure (count, index, matching)))
                  else
                    (let matching : Bool := false
                    (pure (count, index, matching))) ) : SailM (Nat × Nat × Bool) )
                (pure (count, index, matching)))
            else (pure (count, index, matching)) ) : SailM (Nat × Nat × Bool) )
          (pure (count, index, matching))
      (pure loop_vars) ) : SailM (Nat × Nat × Bool) )
    (pure count)
  pure (⟨publicResult⟩)

/-- Encodes the remaining nibble pairs of a compact trie path in wire order. -/
/- Type quantifiers: _reclimit : Nat, k_ex408804_ : Nat, 0 ≤ k_ex408804_ ∧ k_ex408804_ ≤ 65, 0
  ≤ _reclimit -/
def _rec_hex_prefix_pairs (path : TriePath) (index : hex_prefix_cursor) (_reclimit : Nat) : SailM (List byte) := do
  let index := (index).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((((path_len path)).value ≤b index) : Bool)
      then (pure [])
      else
        (do
          let next := (index + 1)
          (pure (((← (path_nibble path ⟨index⟩)) +++ (← (path_nibble path ⟨next⟩))) :: (← (_rec_hex_prefix_pairs
                path ⟨(next + 1)⟩ _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Encodes the remaining nibble pairs of a compact trie path in wire order. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 65 -/
def hex_prefix_pairs (path : TriePath) (index : hex_prefix_cursor) : SailM (List byte) := do
  let index := (index).value
  let _measure := ((((path_len path)).value -i index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_hex_prefix_pairs path ⟨index⟩ (_measure + 1))

/-- The hex-evm_prefix (compact) encoding of a nibble path with its
leaf/extension flag (YP Appendix C.1, the HP function). -/
/- Type quantifiers: k_ex408808_ : Bool -/
def hex_prefix_compact (path : TriePath) (is_leaf : Bool) : SailM ((List byte) × Nat) := do
  let length : Nat := ((path_len path)).value
  let packed_pair_count : Nat := (Nat.div length 2)
  let odd := ((Nat.mod length 2) != 0)
  let flag : (BitVec 4) :=
    if (is_leaf : Bool)
    then 0x2#4
    else 0x0#4
  let first ← do
    if (odd : Bool)
    then (pure ((flag ||| 0x1#4) +++ (← (path_nibble path ⟨0⟩))))
    else (pure (flag +++ 0x0#4))
  let first_path_index : Nat :=
    if (odd : Bool)
    then 1
    else 0
  let encoded_len : Nat := (1 + packed_pair_count)
  (pure ((first :: (← (hex_prefix_pairs path ⟨first_path_index⟩))), encoded_len))

