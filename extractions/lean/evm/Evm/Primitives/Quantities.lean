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

def word_to_account_nonce (value : word) : (Option account_nonce) :=
  (Option.map (fun semanticValue => ⟨semanticValue⟩) (let amount := (BitVec.toNatInt value)
  if ((amount ≤b ((2 ^i 64) -i 1)) : Bool)
  then (some amount)
  else none))

def word_to_precompile_id (value : word) : (Option precompile_id) :=
  (Option.map (fun semanticValue => ⟨semanticValue⟩) (let candidate :=
    (BitVec.toNatInt (Sail.BitVec.extractLsb value 8 0))
  if ((((Sail.BitVec.extractLsb value 255 9) == (BitVec.zero 247)) && ((1 ≤b candidate) && (candidate ≤b 256))) : Bool)
  then (some candidate)
  else none))

/-- Type quantifiers: value : Nat, 1 ≤ value ∧ value ≤ 256 -/
def word_of_precompile_id (value : precompile_id) : SailM word := do
  let value := (value).value
  (word_of_nat value)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def account_nonce_increment (value : account_nonce) : SailM account_nonce := do
  let value := (value).value
  let semanticResult ← do
    assert (value <b ((2 ^i 64) -i 1)) "sail/primitives/quantities.sail:125.29-125.30"
    (pure (value + 1))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def protocol_quantity_increment (value : protocol_quantity) : SailM protocol_quantity := do
  let value := (value).value
  let semanticResult ← do
    assert (value <b ((2 ^i 64) -i 1)) "sail/primitives/quantities.sail:131.29-131.30"
    (pure (value + 1))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def protocol_quantity_decrement (value : protocol_quantity) : SailM protocol_quantity := do
  let value := (value).value
  let semanticResult ← do
    assert (0 <b value) "sail/primitives/quantities.sail:137.20-137.21"
    (pure (value -i 1))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 1024 -/
def frame_depth_increment (value : frame_depth) : SailM frame_depth := do
  let value := (value).value
  let semanticResult ← do
    if ((value <b 1024) : Bool)
    then (pure (value + 1))
    else
      (do
        assert false "sail/primitives/quantities.sail:146.20-146.21"
        throw Error.Exit)
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def item_count_increment (value : item_count) : SailM item_count := do
  let value := (value).value
  let semanticResult ← do
    (do
        let semanticResult ← (protocol_quantity_increment ⟨value⟩)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def item_index_increment (value : item_index) : SailM item_index := do
  let value := (value).value
  let semanticResult ← do
    (do
        let semanticResult ← (protocol_quantity_increment ⟨value⟩)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_increment (value : merkle_depth) : SailM merkle_depth := do
  let value := (value).value
  let semanticResult ← do
    assert (value <b 64) "sail/primitives/quantities.sail:162.21-162.22"
    (pure (value + 1))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_decrement (value : merkle_depth) : SailM merkle_depth := do
  let value := (value).value
  let semanticResult ← do
    assert (0 <b value) "sail/primitives/quantities.sail:168.20-168.21"
    (pure (value -i 1))
  pure (⟨semanticResult⟩)

def BYTE_ONE : byte_quantity := (ByteQuantity 1)

def MAX_BYTE_QUANTITY : byte_quantity := (ByteQuantity BYTE_QUANTITY_MAX)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_fits_limb (value : Nat) : Bool :=
  (value ≤b BYTE_QUANTITY_MAX)

def byte_quantity_fits_limb (app_0 : byte_quantity) : Bool :=
  let .ByteQuantity value := app_0
  (nat_fits_limb value)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_to_limb (value : Nat) : SailM limb := do
  assert (nat_fits_limb value) "sail/primitives/quantities.sail:220.31-220.32"
  (pure (get_slice_int 64 value 0))

def byte_quantity_to_limb (app_0 : byte_quantity) : SailM limb := do
  let .ByteQuantity value := app_0
  (nat_to_limb value)

def word_of_byte_quantity (app_0 : byte_quantity) : SailM word := do
  let .ByteQuantity value := app_0
  (word_of_nat value)

/-- Type quantifiers: k_ex160757_ : Nat, k_ex160756_ : Nat, 0 ≤ k_ex160756_ ∧
  k_ex160756_ ≤ (2 ^ 64 - 1), 1 ≤ k_ex160757_ ∧ k_ex160757_ ≤ 256 -/
def protocol_quantity_quotient (value : protocol_quantity) (divisor : protocol_divisor) : SailM protocol_quantity := do
  let value := (value).value
  let divisor := (divisor).value
  let semanticResult ← do
    let quotient ← do (exact_quotient value divisor)
    assert (quotient ≤b value) "sail/primitives/quantities.sail:253.28-253.29"
    (pure quotient)
  pure (⟨semanticResult⟩)

def byte_quantity_quotient (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity dividend : byte_quantity := typ_0
  let .ByteQuantity divisor : byte_quantity := typ_1
  assert (dividend ≤b BYTE_QUANTITY_MAX) "sail/primitives/quantities.sail:310.40-310.41"
  (pure (ByteQuantity (← (exact_quotient dividend divisor))))

/-- Type quantifiers: k_ex160758_ : Nat, 1 ≤ k_ex160758_ ∧ k_ex160758_ ≤ 1000 -/
def gas_cost_quotient (typ_0 : gas_cost) (divisor : gas_divisor) : SailM gas_cost := do
  let divisor := (divisor).value
  let .GasCost value : gas_cost := typ_0
  (pure (GasCost (← (exact_quotient value divisor))))

/-- Type quantifiers: k_ex160759_ : Nat, 1 ≤ k_ex160759_ ∧ k_ex160759_ ≤ 1000 -/
def gas_quotient (typ_0 : gas) (divisor : gas_divisor) : SailM gas := do
  let divisor := (divisor).value
  let .Gas value : gas := typ_0
  let quotient ← do (exact_quotient value divisor)
  assert (quotient ≤b value) "sail/primitives/gas.sail:147.28-147.29"
  (pure (Gas quotient))

