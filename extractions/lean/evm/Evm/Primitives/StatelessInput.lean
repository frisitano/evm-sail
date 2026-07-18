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

def undefined_WitnessContext (_ : Unit) : SailM WitnessContext := do
  (pure { parent_hash := ← (undefined_bitvector 256),
          parent_state_root := ← (undefined_bitvector 256),
          parent_base_fee_per_gas := ← (undefined_bitvector 256),
          parent_blob_gas_used := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          parent_excess_blob_gas := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩) })

