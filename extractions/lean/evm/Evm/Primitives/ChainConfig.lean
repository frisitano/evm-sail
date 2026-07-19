import Sail
import Evm.Defs
import Evm.Specialization
import Evm.FakeReal
import Evm.HostAxioms

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

/-! # Chain configuration

The decoded `SszChainConfig`: the chain id and the active fork — both as
the raw SSZ `ProtocolFork` index and as the [Fork][type-Fork] whose
execution rules apply (blob-parameter-only forks collapse to their base) —
plus whether the fork's activation point has been reached. Pure data — no
registers, no externs. -/

def undefined_BlobSchedule (_ : Unit) : SailM BlobSchedule := do
  (pure { target := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          max := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          base_fee_update_fraction := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩) })

/-- Structural equality of blob schedules. -/
def blob_schedule_equal (a : BlobSchedule) (b : BlobSchedule) : Bool :=
  (((a.target).value == (b.target).value) && ((((a.max).value == (b.max).value) && (((a.base_fee_update_fraction).value == (b.base_fee_update_fraction).value) : Bool)) : Bool))

/-- The protocol-defined blob schedule for an SSZ `ProtocolFork` index:
Cancun 3/6, Prague/Osaka 6/9, BPO1 10/15, BPO2+ 14/21; forks before
Cancun have none. -/
/- Type quantifiers: idx : Nat, 0 ≤ idx ∧ idx ≤ (2 ^ 64 - 1) -/
def expected_blob_schedule (idx : protocol_fork_index) : (Option BlobSchedule) :=
  let idx := (idx).value
  if ((idx <b 15) : Bool)
  then none
  else
    (if ((idx == 15) : Bool)
    then
      (some
        { target := ⟨3⟩,
          max := ⟨6⟩,
          base_fee_update_fraction := ⟨3338477⟩ })
    else
      (if ((idx ≤b 17) : Bool)
      then
        (some
          { target := ⟨6⟩,
            max := ⟨9⟩,
            base_fee_update_fraction := ⟨5007716⟩ })
      else
        (if ((idx == 18) : Bool)
        then
          (some
            { target := ⟨10⟩,
              max := ⟨15⟩,
              base_fee_update_fraction := ⟨8346193⟩ })
        else
          (some
            { target := ⟨14⟩,
              max := ⟨21⟩,
              base_fee_update_fraction := ⟨11684671⟩ }))))

/-- Whether the config's blob schedule is well-shaped and equals the
protocol-defined schedule for its fork index. -/
def chain_config_blob_schedule_valid (cc : ChainConfig) : Bool :=
  if ((! cc.blob_schedule_shape_valid) : Bool)
  then false
  else
    (match ((expected_blob_schedule ⟨(cc.fork_index).value⟩), cc.blob_schedule) with
    | (none, none) => true
    | (.some expected, .some actual) => (blob_schedule_equal expected actual)
    | _ => false)

