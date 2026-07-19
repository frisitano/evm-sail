import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities

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

/-! # Gas quantities

Semantic domains for available gas, schedule constants, computed costs, and
the signed refund counter. The separation keeps unaffordable intermediate
costs exact while preventing them from being mistaken for gas that a valid
frame can hold.

## Types

`gas` is bounded by the protocol's signed-64-bit validity limit;
`gas_constant` contains fixed schedule values; `gas_cost` is an exact natural;
and `gas_refund` records signed changes before the transaction refund cap.

## Constants

The zero values initialize each gas domain, while `GAS_MAX` is the largest gas
quantity admitted by transaction and block validation. -/

def GAS_MAX_VALUE : protocol_quantity := ⟨9223372036854775807⟩

def GAS_ZERO : gas := (Gas 0)

def GAS_MAX : gas := (Gas 9223372036854775807)

def GAS_CONSTANT_ZERO : gas_constant := (GasConstant 0)

def GAS_COST_ZERO : gas_cost := (GasCost 0)

def GAS_REFUND_ZERO : gas_refund := (GasRefund 0)

/-- Tests whether a decoded protocol quantity fits the live-gas domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def gas_value_supported (value : protocol_quantity) : Bool :=
  let value := (value).value
  (value ≤b (GAS_MAX_VALUE).value)

/-- Constructs bounded gas from a decoded protocol quantity. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def nat_to_gas (value : protocol_quantity) : SailM gas := do
  let value := (value).value
  if ((value ≤b 9223372036854775807) : Bool)
  then (pure (Gas value))
  else
    (do
      assert false "sail/primitives/gas.sail:68.20-68.21"
      throw Error.Exit)

/-- Narrows a word to gas when it satisfies the protocol bound. -/
def word_to_gas (value : word) : (Option gas) :=
  let bits := (word_to_bits value)
  if (((Sail.BitVec.extractLsb bits 255 63) == (BitVec.zero 193)) : Bool)
  then (some (Gas (BitVec.toNatInt (Sail.BitVec.extractLsb bits 62 0))))
  else none

def word_of_gas (app_0 : gas) : SailM word := do
  let .Gas value := app_0
  (word_of_nat value)

def gas_to_cost (app_0 : gas) : gas_cost :=
  let .Gas amount := app_0
  (GasCost amount)

def gas_to_refund (app_0 : gas) : gas_refund :=
  let .Gas amount := app_0
  (GasRefund amount)

def gas_constant_to_refund (app_0 : gas_constant) : gas_refund :=
  let .GasConstant amount := app_0
  (GasRefund amount)

def gas_cost_to_word (app_0 : gas_cost) : SailM word := do
  let .GasCost amount := app_0
  (word_of_nat amount)

def gas_cost_to_refund (app_0 : gas_cost) : gas_refund :=
  let .GasCost amount := app_0
  (GasRefund amount)

/-- Clamps a signed refund to the interval from zero through the supplied cap. -/
def capped_gas_refund (typ_0 : gas_refund) (typ_1 : gas) : gas :=
  let .GasRefund refund : gas_refund := typ_0
  let .Gas limit : gas := typ_1
  if ((refund ≤b 0) : Bool)
  then GAS_ZERO
  else
    (if ((refund ≤b limit) : Bool)
    then (Gas refund)
    else (Gas limit))

/-- Tests whether adding two live-gas quantities remains in range. -/
def gas_sum_supported (typ_0 : gas) (typ_1 : gas) : Bool :=
  let .Gas left : gas := typ_0
  let .Gas right : gas := typ_1
  (right ≤b ((GAS_MAX_VALUE).value -i left))

/-- Multiplies a word by a protocol quantity and reports 256-bit overflow. -/
/- Type quantifiers: k_ex161192_ : Nat, 0 ≤ k_ex161192_ ∧ k_ex161192_ ≤ (2 ^ 64 - 1) -/
def word_checked_mul_protocol_quantity (value : word) (factor : protocol_quantity) : SailM (Option word) := do
  let factor := (factor).value
  let result : word := ZERO_WORD
  let addend : word := value
  let remaining : Nat := factor
  let valid : Bool := true
  let (addend, remaining, result, valid) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 63
    let mut loop_vars := (addend, remaining, result, valid)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (addend, remaining, result, valid) := loop_vars
      loop_vars ← do
        let (result, valid) : (word × Bool) :=
          if ((valid && (((Int.tmod remaining 2) == 1) : Bool)) : Bool)
          then
            (let added := (word_checked_add result addend)
            let (result, valid) : (word × Bool) :=
              match added with
              | .some value =>
                (let result : word := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining ←
          (do
              let semanticResult ← (protocol_quantity_quotient ⟨remaining⟩ ⟨2⟩)
              pure ((semanticResult).value))
        let (addend, valid) : (word × Bool) :=
          if ((valid && ((i <b 63) && ((remaining != 0) : Bool))) : Bool)
          then
            (let doubled := (word_checked_add addend addend)
            let (addend, valid) : (word × Bool) :=
              match doubled with
              | .some value =>
                (let addend : word := value
                (addend, valid))
              | none =>
                (let valid : Bool := false
                (addend, valid))
            (addend, valid))
          else (addend, valid)
        (pure (addend, remaining, result, valid))
    (pure loop_vars) ) : SailM (word × Nat × word × Bool) )
  if (valid : Bool)
  then (pure (some result))
  else (pure none)

/-- Multiplies a word by bounded gas and reports 256-bit overflow. -/
def word_checked_mul_gas (value : word) (typ_1 : gas) : SailM (Option word) := do
  let .Gas factor : gas := typ_1
  (word_checked_mul_protocol_quantity value ⟨factor⟩)

