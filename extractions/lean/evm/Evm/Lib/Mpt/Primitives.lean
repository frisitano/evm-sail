import Evm.Flow
import Evm.Arith
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

def undefined_TriePath (_ : Unit) : SailM TriePath := do
  (pure { data := ← (undefined_bitvector 256),
          len := ← do
              let semanticField ← (undefined_range 0 64)
              pure (⟨semanticField⟩) })

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def to_trie_depth (value : trie_path_len) : SailM trie_depth := do
  let value := (value).value
  let semanticResult ← do
    if ((64 ≤b value) : Bool)
    then sailThrow ((InvalidBlock WitnessDeficient))
    else (pure value)
  pure (⟨semanticResult⟩)

/-- Type quantifiers: k_ex161562_ : Nat, 0 ≤ k_ex161562_ ∧ k_ex161562_ ≤ 64 -/
def path_take (path : TriePath) (n : trie_path_len) : TriePath :=
  let n := (n).value
  if ((n == 0) : Bool)
  then (path_empty ())
  else
    (if ((((path_len path)).value ≤b n) : Bool)
    then path
    else
      (let shift := (256 -i (n *i 4))
      (path_new ((path.data >>> shift) <<< shift) ⟨n⟩)))

/-- Type quantifiers: k_ex161563_ : Nat, 0 ≤ k_ex161563_ ∧ k_ex161563_ ≤ 64 -/
def path_drop (path : TriePath) (n : trie_path_len) : TriePath :=
  let n := (n).value
  let length := ((path_len path)).value
  if ((length ≤b n) : Bool)
  then (path_empty ())
  else
    (if ((n == 0) : Bool)
    then path
    else
      (let remain : Nat := (length -i n)
      (path_new (path.data <<< (n *i 4)) ⟨remain⟩)))

def path_eq (a : TriePath) (b : TriePath) : Bool :=
  (((a.len).value == (b.len).value) && (a.data == b.data))

def path_lt (a : TriePath) (b : TriePath) : Bool :=
  if ((a.data == b.data) : Bool)
  then (((path_len a)).value <b ((path_len b)).value)
  else (word_ult a.data b.data)

def path_prefix_of (evm_prefix' : TriePath) (path : TriePath) : Bool :=
  if ((((path_len path)).value <b ((path_len evm_prefix')).value) : Bool)
  then false
  else (path_eq evm_prefix' (path_take path ⟨((path_len evm_prefix')).value⟩))

/-- Type quantifiers: k_ex161564_ : Nat, 0 ≤ k_ex161564_ ∧ k_ex161564_ ≤ 64 -/
def common_prefix_from (a : TriePath) (b : TriePath) (start : trie_path_len) : SailM trie_path_len := do
  let start := (start).value
  let semanticResult ← do
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
                  if (((path_nibble a ⟨index⟩) == (path_nibble b ⟨index⟩)) : Bool)
                  then
                    (do
                      let count ←
                        (do
                            let semanticResult ← (trie_path_len_increment ⟨count⟩)
                            pure ((semanticResult).value))
                      let index ←
                        (do
                            let semanticResult ← (trie_path_len_increment ⟨index⟩)
                            pure ((semanticResult).value))
                      (pure (count, index, matching)))
                  else
                    (let matching : Bool := false
                    (pure (count, index, matching))) ) : SailM (Nat × Nat × Bool) )
                (pure (count, index, matching)))
            else (pure (count, index, matching)) ) : SailM (Nat × Nat × Bool) )
          (pure (count, index, matching))
      (pure loop_vars) ) : SailM (Nat × Nat × Bool) )
    (pure count)
  pure (⟨semanticResult⟩)

/-- Type quantifiers: _reclimit : Nat, k_ex161565_ : Nat, 0 ≤ k_ex161565_ ∧ k_ex161565_ ≤ 65, 0
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
          (pure (((path_nibble path ⟨index⟩) +++ (path_nibble path ⟨next⟩)) :: (← (_rec_hex_prefix_pairs
                path ⟨(next + 1)⟩ _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 65 -/
def hex_prefix_pairs (path : TriePath) (index : hex_prefix_cursor) : SailM (List byte) := do
  let index := (index).value
  let _measure := ((((path_len path)).value -i index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_hex_prefix_pairs path ⟨index⟩ (_measure + 1))

/-- Type quantifiers: k_ex161569_ : Bool -/
def hex_prefix_compact (path : TriePath) (is_leaf : Bool) : SailM ((List byte) × byte_length) := do
  let odd := ((Int.tmod ((path_len path)).value 2) == 1)
  let flag : (BitVec 4) :=
    if (is_leaf : Bool)
    then 0x2#4
    else 0x0#4
  let first :=
    if (odd : Bool)
    then ((flag ||| 0x1#4) +++ (path_nibble path ⟨0⟩))
    else (flag +++ 0x0#4)
  let first_path_index : Nat :=
    if (odd : Bool)
    then 1
    else 0
  let encoded_len ← (( do
    (pure (ByteQuantity (1 + (← (exact_quotient ((path_len path)).value 2))))) ) : SailM
    byte_quantity )
  (pure ((first :: (← (hex_prefix_pairs path ⟨first_path_index⟩))), encoded_len))

