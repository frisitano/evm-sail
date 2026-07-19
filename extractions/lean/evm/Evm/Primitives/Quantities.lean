import Evm.Arith
import Evm.Prelude

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

/-! # Protocol quantities

Semantic numeric domains used outside the EVM's 256-bit word algebra.
Protocol fields retain their wire bounds, structural counters carry the
limits imposed by their data structures, and byte positions remain exact
non-negative quantities after operand validation.

## Types

The aliases below prevent unrelated protocol and structural quantities from
being interchanged accidentally while preserving their mathematical values.

## Constants

`BYTE_ZERO`, `BYTE_ONE`, and `MAX_BYTE_QUANTITY` delimit the byte-addressable
domain used by source-backed slices and memory regions. -/

/-- Embeds a bounded protocol quantity in an EVM word. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_protocol_quantity (value : protocol_quantity) : SailM word := do
  let value := (value).value
  (word_of_nat value)

/-- Converts a word to an account nonce when it fits the protocol field. -/
def word_to_account_nonce (value : word) : (Option account_nonce) :=
  (Option.map (fun semanticValue => ⟨semanticValue⟩) (match (word_to_limb value) with
  | .some amount => (some (BitVec.toNatInt amount))
  | none => none))

/-- Narrows a word losslessly to the bounded protocol-quantity domain. -/
def word_to_protocol_quantity (value : word) : (Option protocol_quantity) :=
  (Option.map (fun semanticValue => ⟨semanticValue⟩) (match (word_to_limb value) with
  | .some amount => (some (BitVec.toNatInt amount))
  | none => none))

/-- Recognizes a word as a valid precompiled-contract identifier. -/
def word_to_precompile_id (value : word) : (Option precompile_id) :=
  (Option.map (fun semanticValue => ⟨semanticValue⟩) (match (word_to_limb value) with
  | .some limb_value =>
    (let candidate := (BitVec.toNatInt (Sail.BitVec.extractLsb limb_value 8 0))
    if ((((Sail.BitVec.extractLsb limb_value 63 9) == (BitVec.zero 55)) && ((1 ≤b candidate) && (candidate ≤b 256))) : Bool)
    then (some candidate)
    else none)
  | none => none))

/-- Embeds a precompiled-contract identifier in an EVM word. -/
/- Type quantifiers: value : Nat, 1 ≤ value ∧ value ≤ 256 -/
def word_of_precompile_id (value : precompile_id) : SailM word := do
  let value := (value).value
  (word_of_protocol_quantity ⟨value⟩)

/-- Increments an account nonce after establishing that it is not maximal. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def account_nonce_increment (value : account_nonce) : SailM account_nonce := do
  let value := (value).value
  let semanticResult ← do
    assert (value <b (BYTE_QUANTITY_MAX).value) "sail/primitives/quantities.sail:144.36-144.37"
    (pure (value + 1))
  pure (⟨semanticResult⟩)

/-- Increments a bounded protocol quantity. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def protocol_quantity_increment (value : protocol_quantity) : SailM protocol_quantity := do
  let value := (value).value
  let semanticResult ← do
    assert (value <b (BYTE_QUANTITY_MAX).value) "sail/primitives/quantities.sail:150.36-150.37"
    (pure (value + 1))
  pure (⟨semanticResult⟩)

/-- Decrements a positive protocol quantity. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def protocol_quantity_decrement (value : protocol_quantity) : SailM protocol_quantity := do
  let value := (value).value
  let semanticResult ← do
    assert (0 <b value) "sail/primitives/quantities.sail:156.20-156.21"
    (pure (value -i 1))
  pure (⟨semanticResult⟩)

/-- Subtracts bounded protocol quantities without underflow. -/
/- Type quantifiers: k_ex161174_ : Nat, k_ex161173_ : Nat, 0 ≤ k_ex161173_ ∧
  k_ex161173_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161174_ ∧ k_ex161174_ ≤ (2 ^ 64 - 1) -/
def protocol_quantity_sub (left : protocol_quantity) (right : protocol_quantity) : SailM protocol_quantity := do
  let left := (left).value
  let right := (right).value
  let semanticResult ← do
    assert (right ≤b left) "sail/primitives/quantities.sail:162.24-162.25"
    (pure (left -i right))
  pure (⟨semanticResult⟩)

/-- Adds bounded protocol quantities without overflow. -/
/- Type quantifiers: k_ex161176_ : Nat, k_ex161175_ : Nat, 0 ≤ k_ex161175_ ∧
  k_ex161175_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161176_ ∧ k_ex161176_ ≤ (2 ^ 64 - 1) -/
def protocol_quantity_add (left : protocol_quantity) (right : protocol_quantity) : SailM protocol_quantity := do
  let left := (left).value
  let right := (right).value
  let semanticResult ← do
    assert (right ≤b ((BYTE_QUANTITY_MAX).value -i left)) "sail/primitives/quantities.sail:168.44-168.45"
    (pure (left + right))
  pure (⟨semanticResult⟩)

/-- Advances the call depth and rejects an attempt to exceed its bound. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 1024 -/
def frame_depth_increment (value : frame_depth) : SailM frame_depth := do
  let value := (value).value
  let semanticResult ← do
    if ((value <b 1024) : Bool)
    then (pure (value + 1))
    else
      (do
        assert false "sail/primitives/quantities.sail:177.20-177.21"
        throw Error.Exit)
  pure (⟨semanticResult⟩)

/-- Advances a source-backed collection count. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def item_count_increment (value : item_count) : SailM item_count := do
  let value := (value).value
  let semanticResult ← do
    (do
        let semanticResult ← (protocol_quantity_increment ⟨value⟩)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Advances a source-backed collection index. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def item_index_increment (value : item_index) : SailM item_index := do
  let value := (value).value
  let semanticResult ← do
    (do
        let semanticResult ← (protocol_quantity_increment ⟨value⟩)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Descends by one level in a bounded Merkle tree. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_increment (value : merkle_depth) : SailM merkle_depth := do
  let value := (value).value
  let semanticResult ← do
    assert (value <b 64) "sail/primitives/quantities.sail:193.21-193.22"
    (pure (value + 1))
  pure (⟨semanticResult⟩)

/-- Ascends by one level in a bounded Merkle tree. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_decrement (value : merkle_depth) : SailM merkle_depth := do
  let value := (value).value
  let semanticResult ← do
    assert (0 <b value) "sail/primitives/quantities.sail:199.20-199.21"
    (pure (value -i 1))
  pure (⟨semanticResult⟩)

def BYTE_ONE : byte_quantity := (ByteQuantity 1)

def MAX_BYTE_QUANTITY : byte_quantity := (ByteQuantity (BYTE_QUANTITY_MAX).value)

/-- Tests whether a natural number fits the byte-addressable domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_fits_limb (value : Nat) : Bool :=
  (value ≤b (BYTE_QUANTITY_MAX).value)

/-- Tests whether a byte quantity fits the byte-addressable domain. -/
def byte_quantity_fits_limb (app_0 : byte_quantity) : Bool :=
  let .ByteQuantity value := app_0
  (nat_fits_limb value)

/-- Converts a representable natural number to a 64-bit limb. -/
/- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_to_limb (value : Nat) : SailM limb := do
  assert (nat_fits_limb value) "sail/primitives/quantities.sail:250.31-250.32"
  (pure (get_slice_int 64 value 0))

/-- Converts a representable byte quantity to a 64-bit limb. -/
def byte_quantity_to_limb (app_0 : byte_quantity) : SailM limb := do
  let .ByteQuantity value := app_0
  (nat_to_limb value)

/-- Embeds a byte quantity in an EVM word. -/
def word_of_byte_quantity (app_0 : byte_quantity) : SailM word := do
  let .ByteQuantity value := app_0
  (word_of_nat value)

/-- Interprets an EVM word as an exact mathematical byte quantity. -/
def word_to_byte_quantity (value : word) : byte_quantity :=
  (ByteQuantity (BitVec.toNatInt (word_to_bits value)))

/-- Divides a protocol quantity by a small positive divisor. -/
/- Type quantifiers: k_ex161185_ : Nat, k_ex161184_ : Nat, 0 ≤ k_ex161184_ ∧
  k_ex161184_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex161185_ ∧ k_ex161185_ ≤ 256 -/
def protocol_quantity_quotient (value : protocol_quantity) (divisor : protocol_divisor) : SailM protocol_quantity := do
  let value := (value).value
  let divisor := (divisor).value
  let semanticResult ← do
    let quotient ← do (exact_quotient value divisor)
    assert (quotient ≤b value) "sail/primitives/quantities.sail:288.28-288.29"
    (pure quotient)
  pure (⟨semanticResult⟩)

/-- Divides a byte quantity by a nonzero byte quantity. -/
def byte_quantity_quotient (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity dividend : byte_quantity := typ_0
  let .ByteQuantity divisor : byte_quantity := typ_1
  assert (dividend ≤b (BYTE_QUANTITY_MAX).value) "sail/primitives/quantities.sail:345.40-345.41"
  (pure (ByteQuantity (← (exact_quotient dividend divisor))))

/- Type quantifiers: k_ex161186_ : Nat, 1 ≤ k_ex161186_ ∧ k_ex161186_ ≤ 2000 -/
def gas_cost_quotient (typ_0 : gas_cost) (divisor : gas_divisor) : SailM gas_cost := do
  let divisor := (divisor).value
  let .GasCost value : gas_cost := typ_0
  (pure (GasCost (← (exact_quotient value divisor))))

/-- Divides available gas by a positive schedule divisor. -/
/- Type quantifiers: k_ex161187_ : Nat, 1 ≤ k_ex161187_ ∧ k_ex161187_ ≤ 2000 -/
def gas_quotient (typ_0 : gas) (divisor : gas_divisor) : SailM gas := do
  let divisor := (divisor).value
  let .Gas value : gas := typ_0
  let quotient ← do (exact_quotient value divisor)
  assert (quotient ≤b value) "sail/primitives/gas.sail:162.28-162.29"
  (pure (Gas quotient))

