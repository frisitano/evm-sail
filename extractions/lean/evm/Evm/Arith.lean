import Evm.Flow

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

def BYTE_QUANTITY_MAX : protocol_quantity := ⟨18446744073709551615⟩

/-- Adds two byte quantities after checking the addressable bound. -/
def byte_quantity_add (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  assert (left ≤b (BYTE_QUANTITY_MAX).value) "sail/primitives/quantities.sail:312.36-312.37"
  assert (right ≤b ((BYTE_QUANTITY_MAX).value -i left)) "sail/primitives/quantities.sail:313.44-313.45"
  (pure (ByteQuantity (left + right)))

/-- Adds live gas after establishing that the sum remains in range. -/
def gas_add (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  assert (right ≤b (9223372036854775807 -i left)) "sail/primitives/gas.sail:148.46-148.47"
  (pure (Gas (left + right)))

def gas_constant_add (typ_0 : gas_constant) (typ_1 : gas_constant) : gas_cost :=
  let .GasConstant left : gas_constant := typ_0
  let .GasConstant right : gas_constant := typ_1
  (GasCost (left + right))

def gas_cost_add (typ_0 : gas_cost) (typ_1 : gas_cost) : gas_cost :=
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  (GasCost (left + right))

def gas_cost_add_constant (typ_0 : gas_cost) (typ_1 : gas_constant) : gas_cost :=
  let .GasCost left : gas_cost := typ_0
  let .GasConstant right : gas_constant := typ_1
  (GasCost (left + right))

def gas_refund_add (typ_0 : gas_refund) (typ_1 : gas_refund) : gas_refund :=
  let .GasRefund left : gas_refund := typ_0
  let .GasRefund right : gas_refund := typ_1
  (GasRefund (left +i right))

/-- Subtracts a byte quantity without permitting underflow. -/
def byte_quantity_sub (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  assert (left ≤b (BYTE_QUANTITY_MAX).value) "sail/primitives/quantities.sail:319.36-319.37"
  assert (right ≤b left) "sail/primitives/quantities.sail:320.24-320.25"
  (pure (ByteQuantity (left -i right)))

/-- Subtracts schedule constants and promotes the result to an exact cost. -/
def gas_constant_sub (typ_0 : gas_constant) (typ_1 : gas_constant) : SailM gas_cost := do
  let .GasConstant left : gas_constant := typ_0
  let .GasConstant right : gas_constant := typ_1
  assert (right ≤b left) "sail/primitives/gas.sail:183.24-183.25"
  (pure (GasCost (left -i right)))

/-- Subtracts one exact gas cost from another without underflow. -/
def gas_cost_sub (typ_0 : gas_cost) (typ_1 : gas_cost) : SailM gas_cost := do
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  assert (right ≤b left) "sail/primitives/gas.sail:177.24-177.25"
  (pure (GasCost (left -i right)))

def gas_refund_sub (typ_0 : gas_refund) (typ_1 : gas_refund) : gas_refund :=
  let .GasRefund left : gas_refund := typ_0
  let .GasRefund right : gas_refund := typ_1
  (GasRefund (left -i right))

/-- Subtracts bounded gas after the caller proves the right operand does
not exceed the left; the assertion preserves the [gas][] range. -/
def gas_sub (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  assert (right ≤b left) "sail/primitives/gas.sail:155.24-155.25"
  (pure (Gas (left -i right)))

/-- Computes the non-negative integer quotient for a nonzero divisor. -/
/- Type quantifiers: k_ex161063_ : Nat, k_ex161062_ : Nat, 0 ≤ k_ex161062_, 0 ≤ k_ex161063_ -/
def exact_quotient (dividend : Nat) (divisor : Nat) : SailM Nat := do
  assert (divisor != 0) "sail/primitives/quantities.sail:279.23-279.24"
  let quotient := (Int.tdiv dividend divisor)
  assert (quotient ≥b 0) "sail/primitives/quantities.sail:281.24-281.25"
  (pure quotient)

def BYTE_ZERO : byte_quantity := (ByteQuantity 0)

/-- Multiplies byte quantities after checking the addressable bound. -/
def byte_quantity_mul (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  assert (left ≤b (BYTE_QUANTITY_MAX).value) "sail/primitives/quantities.sail:326.36-326.37"
  if ((left == 0) : Bool)
  then (pure BYTE_ZERO)
  else
    (do
      assert (right ≤b (← (exact_quotient (BYTE_QUANTITY_MAX).value left))) "sail/primitives/quantities.sail:330.63-330.64"
      (pure (ByteQuantity (left *i right))))

/- Type quantifiers: k_ex161064_ : Nat, 0 ≤ k_ex161064_ -/
def gas_constant_scale (typ_0 : gas_constant) (factor : Nat) : gas_cost :=
  let .GasConstant value : gas_constant := typ_0
  (GasCost (value *i factor))

/-- Scales a schedule constant by an exact byte quantity. -/
def gas_constant_scale_byte_quantity (typ_0 : gas_constant) (typ_1 : byte_quantity) : gas_cost :=
  let .GasConstant value : gas_constant := typ_0
  let .ByteQuantity factor : byte_quantity := typ_1
  (GasCost (value *i factor))

/-- Scales a schedule constant by a decoded protocol quantity. -/
/- Type quantifiers: k_ex161065_ : Nat, 0 ≤ k_ex161065_ ∧ k_ex161065_ ≤ (2 ^ 64 - 1) -/
def gas_constant_scale_protocol_quantity (typ_0 : gas_constant) (factor : protocol_quantity) : gas_cost :=
  let factor := (factor).value
  let .GasConstant value : gas_constant := typ_0
  (GasCost (value *i factor))

/- Type quantifiers: k_ex161066_ : Nat, 0 ≤ k_ex161066_ -/
def gas_cost_scale (typ_0 : gas_cost) (factor : Nat) : gas_cost :=
  let .GasCost value : gas_cost := typ_0
  (GasCost (value *i factor))

/-- Scales an exact gas cost by an exact byte quantity. -/
def gas_cost_scale_byte_quantity (typ_0 : gas_cost) (typ_1 : byte_quantity) : gas_cost :=
  let .GasCost value : gas_cost := typ_0
  let .ByteQuantity factor : byte_quantity := typ_1
  (GasCost (value *i factor))

/-- Scales an exact gas cost by a decoded protocol quantity. -/
/- Type quantifiers: k_ex161067_ : Nat, 0 ≤ k_ex161067_ ∧ k_ex161067_ ≤ (2 ^ 64 - 1) -/
def gas_cost_scale_protocol_quantity (typ_0 : gas_cost) (factor : protocol_quantity) : gas_cost :=
  let factor := (factor).value
  let .GasCost value : gas_cost := typ_0
  (GasCost (value *i factor))

/- Type quantifiers: k_ex161068_ : Nat, 0 ≤ k_ex161068_ -/
def nat_scale_byte_quantity (left : Nat) (typ_1 : byte_quantity) : Nat :=
  let .ByteQuantity right : byte_quantity := typ_1
  (left *i right)

/- Type quantifiers: k_ex161074_ : Int, k_ex161073_ : Int -/
def _shl_int_general (m : Int) (n : Int) : Int :=
  if ((n ≥b 0) : Bool)
  then (Int.shiftl m n)
  else (Int.shiftr m (Neg.neg n))

/- Type quantifiers: k_ex161076_ : Int, k_ex161075_ : Int -/
def _shr_int_general (m : Int) (n : Int) : Int :=
  if ((n ≥b 0) : Bool)
  then (Int.shiftr m n)
  else (Int.shiftl m (Neg.neg n))

/- Type quantifiers: k_ex161081_ : Int, k_ex161080_ : Int -/
def fdiv_int (n : Int) (m : Int) : Int :=
  if (((n <b 0) && (m >b 0)) : Bool)
  then ((Int.tdiv (n +i 1) m) -i 1)
  else
    (if (((n >b 0) && (m <b 0)) : Bool)
    then ((Int.tdiv (n -i 1) m) -i 1)
    else (Int.tdiv n m))

/- Type quantifiers: k_ex161083_ : Int, k_ex161082_ : Int -/
def fmod_int (n : Int) (m : Int) : Int :=
  (n -i (m *i (fdiv_int n m)))

