import Evm.Sail.Sail
import Evm.Sail.BitVec
import Evm.Sail.IntRange
import Evm.Defs
import Evm.Specialization
import Evm.FakeReal

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

def undefined_BlobSchedule (_ : Unit) : SailM BlobSchedule := do
  (pure { target := ← (undefined_range 0 ((2 ^i 64) -i 1))
          max := ← (undefined_range 0 ((2 ^i 64) -i 1))
          base_fee_update_fraction := ← (undefined_range 0 ((2 ^i 64) -i 1)) })

def blob_schedule_equal (a : BlobSchedule) (b : BlobSchedule) : Bool :=
  ((a.target == b.target) && (((a.max == b.max) && ((a.base_fee_update_fraction == b.base_fee_update_fraction) : Bool)) : Bool))

/-- Type quantifiers: idx : Nat, 0 ≤ idx ∧ idx ≤ (2 ^ 64 - 1) -/
def expected_blob_schedule (idx : Nat) : (Option BlobSchedule) :=
  if ((idx <b 15) : Bool)
  then none
  else
    (if ((idx == 15) : Bool)
    then
      (some
        { target := 3
          max := 6
          base_fee_update_fraction := 3338477 })
    else
      (if ((idx ≤b 17) : Bool)
      then
        (some
          { target := 6
            max := 9
            base_fee_update_fraction := 5007716 })
      else
        (if ((idx == 18) : Bool)
        then
          (some
            { target := 10
              max := 15
              base_fee_update_fraction := 8346193 })
        else
          (some
            { target := 14
              max := 21
              base_fee_update_fraction := 11684671 }))))

def chain_config_blob_schedule_valid (cc : ChainConfig) : Bool :=
  if ((! cc.blob_schedule_shape_valid) : Bool)
  then false
  else
    (match ((expected_blob_schedule cc.fork_index), cc.blob_schedule) with
    | (none, none) => true
    | (.some expected, .some actual) => (blob_schedule_equal expected actual)
    | _ => false)

