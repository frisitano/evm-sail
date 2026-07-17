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

noncomputable section
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
  (pure { data := ← (undefined_bitvector 256)
          len := ← (undefined_range 0 64) })

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def to_trie_depth (value : Nat) : SailM Nat := do
  if ((64 ≤b value) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure value)

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 64 -/
def path_take (path : TriePath) (n : Nat) : TriePath :=
  if ((n == 0) : Bool)
  then (path_empty ())
  else
    (if (((path_len path) ≤b n) : Bool)
    then path
    else
      (let shift := (256 -i (n *i 4))
      (path_new ((path.data >>> shift) <<< shift) n)))

/-- Type quantifiers: n : Nat, 0 ≤ n ∧ n ≤ 64 -/
def path_drop (path : TriePath) (n : Nat) : TriePath :=
  let length := (path_len path)
  if ((length ≤b n) : Bool)
  then (path_empty ())
  else
    (if ((n == 0) : Bool)
    then path
    else
      (let remain : trie_path_len := (length -i n)
      (path_new (path.data <<< (n *i 4)) remain)))

def path_eq (a : TriePath) (b : TriePath) : Bool :=
  ((a.len == b.len) && (a.data == b.data))

def path_lt (a : TriePath) (b : TriePath) : Bool :=
  if ((a.data == b.data) : Bool)
  then ((path_len a) <b (path_len b))
  else (word_ult a.data b.data)

def path_prefix_of (evm_prefix' : TriePath) (path : TriePath) : Bool :=
  if (((path_len path) <b (path_len evm_prefix')) : Bool)
  then false
  else (path_eq evm_prefix' (path_take path (path_len evm_prefix')))

/-- Type quantifiers: start : Nat, 0 ≤ start ∧ start ≤ 64 -/
def common_prefix_from (a : TriePath) (b : TriePath) (start : Nat) : SailM Nat := do
  let alen := (path_len a)
  let blen := (path_len b)
  let stop :=
    if ((alen <b blen) : Bool)
    then alen
    else blen
  let index : trie_path_len := start
  let count : trie_path_len := 0
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
                if (((path_nibble a index) == (path_nibble b index)) : Bool)
                then
                  (do
                    let count ← (trie_path_len_increment count)
                    let index ← (trie_path_len_increment index)
                    (pure (count, index, matching)))
                else
                  (let matching : Bool := false
                  (pure (count, index, matching))) ) : SailM (Nat × Nat × Bool) )
              (pure (count, index, matching)))
          else (pure (count, index, matching)) ) : SailM (Nat × Nat × Bool) )
        (pure (count, index, matching))
    (pure loop_vars) ) : SailM (Nat × Nat × Bool) )
  (pure count)

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 65 -/
partial def hex_prefix_pairs (path : TriePath) (index : Nat) : (List (BitVec 8)) :=
  if (((path_len path) ≤b index) : Bool)
  then []
  else
    (let next := (index +i 1)
    (((path_nibble path index) +++ (path_nibble path next)) :: (hex_prefix_pairs path (next +i 1))))
termination_by (let (path, index) := (path, index)
(65 -i index)).toNat

/-- Type quantifiers: k_ex160145_ : Bool -/
def hex_prefix_compact (path : TriePath) (is_leaf : Bool) : SailM ((List (BitVec 8)) × byte_quantity) := do
  let odd := ((Int.tmod (path_len path) 2) == 1)
  let flag : nibble :=
    if (is_leaf : Bool)
    then 0x2#4
    else 0x0#4
  let first :=
    if (odd : Bool)
    then ((flag ||| 0x1#4) +++ (path_nibble path 0))
    else (flag +++ 0x0#4)
  let first_path_index : trie_path_cursor :=
    if (odd : Bool)
    then 1
    else 0
  let encoded_len ← (( do (pure (ByteQuantity (1 +i (← (exact_quotient (path_len path) 2))))) )
    : SailM byte_length )
  (pure ((first :: (hex_prefix_pairs path first_path_index)), encoded_len))

