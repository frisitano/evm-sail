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

open option
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
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
open ByteRegionResult
open BlockError
open BalIterEntry

/-! # Stateless input

The decoded semantic input. Its block body remains source-backed:
transactions and withdrawals are decoded only when a consumer reaches the
corresponding SSZ list element. -/

def undefined_WitnessContext (_ : Unit) : SailM WitnessContext := do
  (pure { parent_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          parent_state_root := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          parent_base_fee_per_gas := ← (undefined_range 0 ((2 ^i 256) - 1)),
          parent_blob_gas_used := ← (undefined_range 0 (21 *i (2 ^i 17))),
          parent_excess_blob_gas := ← (undefined_range 0 ((2 ^i 64) - 1)) })

