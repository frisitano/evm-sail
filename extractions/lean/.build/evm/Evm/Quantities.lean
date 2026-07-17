import Evm.Arith
import Evm.Prelude

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

def word_to_account_nonce (value : (BitVec 256)) : (Option Nat) :=
  let amount := (BitVec.toNatInt value)
  if ((amount ≤b ((2 ^i 64) -i 1)) : Bool)
  then (some amount)
  else none

def word_to_precompile_id (value : (BitVec 256)) : (Option Nat) :=
  let candidate := (BitVec.toNatInt (Sail.BitVec.extractLsb value 8 0))
  if ((((Sail.BitVec.extractLsb value 255 9) == (BitVec.zero 247)) && ((1 ≤b candidate) && (candidate ≤b 256))) : Bool)
  then (some candidate)
  else none

/-- Type quantifiers: value : Nat, 1 ≤ value ∧ value ≤ 256 -/
def word_of_precompile_id (value : Nat) : SailM (BitVec 256) := do
  (word_of_nat value)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def account_nonce_increment (value : Nat) : SailM Nat := do
  assert (value <b ((2 ^i 64) -i 1)) "sail/primitives/quantities.sail:70.29-70.30"
  (pure (value +i 1))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def protocol_quantity_increment (value : Nat) : SailM Nat := do
  assert (value <b ((2 ^i 64) -i 1)) "sail/primitives/quantities.sail:75.29-75.30"
  (pure (value +i 1))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def protocol_quantity_decrement (value : Nat) : SailM Nat := do
  assert (0 <b value) "sail/primitives/quantities.sail:80.20-80.21"
  (pure (value -i 1))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 1024 -/
def frame_depth_increment (value : Nat) : SailM Nat := do
  if ((value <b 1024) : Bool)
  then (pure (value +i 1))
  else
    (do
      assert false "sail/primitives/quantities.sail:88.20-88.21"
      throw Error.Exit)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def item_count_increment (value : Nat) : SailM Nat := do
  (protocol_quantity_increment value)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def item_index_increment (value : Nat) : SailM Nat := do
  (protocol_quantity_increment value)

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_increment (value : Nat) : SailM Nat := do
  assert (value <b 64) "sail/primitives/quantities.sail:101.21-101.22"
  (pure (value +i 1))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_decrement (value : Nat) : SailM Nat := do
  assert (0 <b value) "sail/primitives/quantities.sail:106.20-106.21"
  (pure (value -i 1))

def BYTE_ONE : byte_quantity := (ByteQuantity 1)

def MAX_BYTE_QUANTITY : byte_quantity := (ByteQuantity BYTE_QUANTITY_MAX)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_fits_limb (value : Nat) : Bool :=
  (value ≤b BYTE_QUANTITY_MAX)

def byte_quantity_fits_limb (app_0 : byte_quantity) : Bool :=
  let .ByteQuantity value := app_0
  (nat_fits_limb value)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_to_limb (value : Nat) : SailM (BitVec 64) := do
  assert (nat_fits_limb value) "sail/primitives/quantities.sail:148.31-148.32"
  (pure (get_slice_int 64 value 0))

def byte_quantity_to_limb (app_0 : byte_quantity) : SailM (BitVec 64) := do
  let .ByteQuantity value := app_0
  (nat_to_limb value)

def word_of_byte_quantity (app_0 : byte_quantity) : SailM (BitVec 256) := do
  let .ByteQuantity value := app_0
  (word_of_nat value)

/-- Type quantifiers: divisor : Nat, value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1), 1 ≤
  divisor ∧ divisor ≤ 256 -/
def protocol_quantity_quotient (value : Nat) (divisor : Nat) : SailM Nat := do
  let quotient ← do (exact_quotient value divisor)
  assert (quotient ≤b value) "sail/primitives/quantities.sail:177.28-177.29"
  (pure quotient)

def byte_quantity_quotient (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity dividend : byte_quantity := typ_0
  let .ByteQuantity divisor : byte_quantity := typ_1
  assert (dividend ≤b BYTE_QUANTITY_MAX) "sail/primitives/quantities.sail:227.40-227.41"
  (pure (ByteQuantity (← (exact_quotient dividend divisor))))

/-- Type quantifiers: divisor : Nat, 1 ≤ divisor ∧ divisor ≤ 1000 -/
def gas_cost_quotient (typ_0 : gas_cost) (divisor : Nat) : SailM gas_cost := do
  let .GasCost value : gas_cost := typ_0
  (pure (GasCost (← (exact_quotient value divisor))))

/-- Type quantifiers: divisor : Nat, 1 ≤ divisor ∧ divisor ≤ 1000 -/
def gas_quotient (typ_0 : gas) (divisor : Nat) : SailM gas := do
  let .Gas value : gas := typ_0
  let quotient ← do (exact_quotient value divisor)
  assert (quotient ≤b value) "sail/primitives/gas.sail:119.28-119.29"
  (pure (Gas quotient))

