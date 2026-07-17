import Evm.Flow

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

def BYTE_QUANTITY_MAX : Nat := ((2 ^i 64) -i 1)

def byte_quantity_add (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  assert (left ≤b BYTE_QUANTITY_MAX) "sail/primitives/quantities.sail:200.36-200.37"
  assert (right ≤b (BYTE_QUANTITY_MAX -i left)) "sail/primitives/quantities.sail:201.44-201.45"
  (pure (ByteQuantity (left +i right)))

def gas_constant_add (typ_0 : gas_constant) (typ_1 : gas_constant) : gas_cost :=
  let .GasConstant left : gas_constant := typ_0
  let .GasConstant right : gas_constant := typ_1
  (GasCost (left +i right))

def gas_cost_add (typ_0 : gas_cost) (typ_1 : gas_cost) : gas_cost :=
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  (GasCost (left +i right))

def gas_cost_add_constant (typ_0 : gas_cost) (typ_1 : gas_constant) : gas_cost :=
  let .GasCost left : gas_cost := typ_0
  let .GasConstant right : gas_constant := typ_1
  (GasCost (left +i right))

def gas_refund_add (typ_0 : gas_refund) (typ_1 : gas_refund) : gas_refund :=
  let .GasRefund left : gas_refund := typ_0
  let .GasRefund right : gas_refund := typ_1
  (GasRefund (left +i right))

def byte_quantity_sub (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  assert (left ≤b BYTE_QUANTITY_MAX) "sail/primitives/quantities.sail:206.36-206.37"
  assert (right ≤b left) "sail/primitives/quantities.sail:207.24-207.25"
  (pure (ByteQuantity (left -i right)))

def gas_constant_sub (typ_0 : gas_constant) (typ_1 : gas_constant) : SailM gas_cost := do
  let .GasConstant left : gas_constant := typ_0
  let .GasConstant right : gas_constant := typ_1
  assert (right ≤b left) "sail/primitives/gas.sail:135.24-135.25"
  (pure (GasCost (left -i right)))

def gas_cost_sub (typ_0 : gas_cost) (typ_1 : gas_cost) : SailM gas_cost := do
  let .GasCost left : gas_cost := typ_0
  let .GasCost right : gas_cost := typ_1
  assert (right ≤b left) "sail/primitives/gas.sail:130.24-130.25"
  (pure (GasCost (left -i right)))

def gas_refund_sub (typ_0 : gas_refund) (typ_1 : gas_refund) : gas_refund :=
  let .GasRefund left : gas_refund := typ_0
  let .GasRefund right : gas_refund := typ_1
  (GasRefund (left -i right))

def gas_sub (typ_0 : gas) (typ_1 : gas) : SailM gas := do
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  assert (right ≤b left) "sail/primitives/gas.sail:110.24-110.25"
  (pure (Gas (left -i right)))

/-- Type quantifiers: divisor : Nat, dividend : Nat, 0 ≤ dividend, 0 ≤ divisor -/
def exact_quotient (dividend : Nat) (divisor : Nat) : SailM Nat := do
  assert (divisor != 0) "sail/primitives/quantities.sail:169.23-169.24"
  let quotient := (Int.tdiv dividend divisor)
  assert (quotient ≥b 0) "sail/primitives/quantities.sail:171.24-171.25"
  (pure quotient)

def BYTE_ZERO : byte_quantity := (ByteQuantity 0)

def byte_quantity_mul (typ_0 : byte_quantity) (typ_1 : byte_quantity) : SailM byte_quantity := do
  let .ByteQuantity left : byte_quantity := typ_0
  let .ByteQuantity right : byte_quantity := typ_1
  assert (left ≤b BYTE_QUANTITY_MAX) "sail/primitives/quantities.sail:212.36-212.37"
  if ((left == 0) : Bool)
  then (pure BYTE_ZERO)
  else
    (do
      assert (right ≤b (← (exact_quotient BYTE_QUANTITY_MAX left))) "sail/primitives/quantities.sail:216.63-216.64"
      (pure (ByteQuantity (left *i right))))

/-- Type quantifiers: factor : Nat, 0 ≤ factor -/
def gas_constant_scale (typ_0 : gas_constant) (factor : Nat) : gas_cost :=
  let .GasConstant value : gas_constant := typ_0
  (GasCost (value *i factor))

def gas_constant_scale_byte_quantity (typ_0 : gas_constant) (typ_1 : byte_quantity) : gas_cost :=
  let .GasConstant value : gas_constant := typ_0
  let .ByteQuantity factor : byte_quantity := typ_1
  (GasCost (value *i factor))

/-- Type quantifiers: factor : Nat, 0 ≤ factor -/
def gas_cost_scale (typ_0 : gas_cost) (factor : Nat) : gas_cost :=
  let .GasCost value : gas_cost := typ_0
  (GasCost (value *i factor))

/-- Type quantifiers: left : Nat, 0 ≤ left -/
def nat_scale_byte_quantity (left : Nat) (typ_1 : byte_quantity) : Nat :=
  let .ByteQuantity right : byte_quantity := typ_1
  (left *i right)

/-- Type quantifiers: n : Int, m : Int -/
def _shl_int_general (m : Int) (n : Int) : Int :=
  if ((n ≥b 0) : Bool)
  then (Int.shiftl m n)
  else (Int.shiftr m (Neg.neg n))

/-- Type quantifiers: n : Int, m : Int -/
def _shr_int_general (m : Int) (n : Int) : Int :=
  if ((n ≥b 0) : Bool)
  then (Int.shiftr m n)
  else (Int.shiftl m (Neg.neg n))

/-- Type quantifiers: m : Int, n : Int -/
def fdiv_int (n : Int) (m : Int) : Int :=
  if (((n <b 0) && (m >b 0)) : Bool)
  then ((Int.tdiv (n +i 1) m) -i 1)
  else
    (if (((n >b 0) && (m <b 0)) : Bool)
    then ((Int.tdiv (n -i 1) m) -i 1)
    else (Int.tdiv n m))

/-- Type quantifiers: m : Int, n : Int -/
def fmod_int (n : Int) (m : Int) : Int :=
  (n -i (m *i (fdiv_int n m)))

