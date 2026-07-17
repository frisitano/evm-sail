import Evm.Flow
import Evm.Prelude
import Evm.Quantities

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

def GAS_MAX_VALUE : Nat := ((2 ^i 63) -i 1)

def GAS_ZERO : gas := (Gas 0)

def GAS_MAX : gas := (Gas ((2 ^i 63) -i 1))

def GAS_CONSTANT_ZERO : gas_constant := (GasConstant 0)

def GAS_COST_ZERO : gas_cost := (GasCost 0)

def GAS_REFUND_ZERO : gas_refund := (GasRefund 0)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def nat_to_gas (value : Nat) : SailM gas := do
  if ((value ≤b ((2 ^i 63) -i 1)) : Bool)
  then (pure (Gas value))
  else
    (do
      assert false "sail/primitives/gas.sail:38.20-38.21"
      throw Error.Exit)

def word_to_gas (value : (BitVec 256)) : (Option gas) :=
  if (((Sail.BitVec.extractLsb value 255 63) == (BitVec.zero 193)) : Bool)
  then (some (Gas (BitVec.toNatInt (Sail.BitVec.extractLsb value 62 0))))
  else none

def word_of_gas (app_0 : gas) : SailM (BitVec 256) := do
  let .Gas value := app_0
  (word_of_nat value)

def gas_to_cost (app_0 : gas) : gas_cost :=
  let .Gas amount := app_0
  (GasCost amount)

def gas_to_refund (app_0 : gas) : gas_refund :=
  let .Gas amount := app_0
  (GasRefund amount)

def gas_constant_to_cost (app_0 : gas_constant) : gas_cost :=
  let .GasConstant amount := app_0
  (GasCost amount)

def gas_constant_to_refund (app_0 : gas_constant) : gas_refund :=
  let .GasConstant amount := app_0
  (GasRefund amount)

def gas_cost_to_word (app_0 : gas_cost) : SailM (BitVec 256) := do
  let .GasCost amount := app_0
  (word_of_nat amount)

def gas_cost_to_refund (app_0 : gas_cost) : gas_refund :=
  let .GasCost amount := app_0
  (GasRefund amount)

def capped_gas_refund (typ_0 : gas_refund) (typ_1 : gas) : gas :=
  let .GasRefund refund : gas_refund := typ_0
  let .Gas limit : gas := typ_1
  if ((refund ≤b 0) : Bool)
  then GAS_ZERO
  else
    (if ((refund ≤b limit) : Bool)
    then (Gas refund)
    else (Gas limit))

/-- Type quantifiers: factor : Nat, 0 ≤ factor ∧ factor ≤ (2 ^ 64 - 1) -/
def word_checked_mul_protocol_quantity (value : (BitVec 256)) (factor : Nat) : SailM (Option (BitVec 256)) := do
  let result : word := ZERO_WORD
  let addend : word := value
  let remaining : protocol_quantity := factor
  let valid : Bool := true
  let (addend, remaining, result, valid) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 63
    let mut loop_vars := (addend, remaining, result, valid)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (addend, remaining, result, valid) := loop_vars
      loop_vars ← do
        let (result, valid) : ((BitVec 256) × Bool) :=
          if ((valid && (((Int.tmod remaining 2) == 1) : Bool)) : Bool)
          then
            (let added := (word_checked_add result addend)
            let (result, valid) : ((BitVec 256) × Bool) :=
              match added with
              | .some value =>
                (let result : word := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining ← (protocol_quantity_quotient remaining 2)
        let (addend, valid) : ((BitVec 256) × Bool) :=
          if ((valid && ((i <b 63) && ((remaining != 0) : Bool))) : Bool)
          then
            (let doubled := (word_checked_add addend addend)
            let (addend, valid) : ((BitVec 256) × Bool) :=
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
    (pure loop_vars) ) : SailM ((BitVec 256) × Nat × (BitVec 256) × Bool) )
  if (valid : Bool)
  then (pure (some result))
  else (pure none)

def word_checked_mul_gas (value : (BitVec 256)) (typ_1 : gas) : SailM (Option (BitVec 256)) := do
  let .Gas factor : gas := typ_1
  (word_checked_mul_protocol_quantity value factor)

