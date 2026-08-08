import Evm.Kernel.Scratch
import Evm.Host.DebugDisabled
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Stateless
import Evm.Executor.Result

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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # The guest entry point

The zkVM guest's `main`: decode the stateless input, verify the payload,
and emit the public validation result. Fatal failures publish the invalid
result and terminate inside `fatal_error`; only success returns here. -/

def fatal_error_set_input (_input_ref : StatelessInputRef) : SailM Unit := do
  (pure ())

/-- Decodes, verifies, and reports the successful validation result. -/
def sail_main (_ : Unit) : SailM Unit := do
  (scratch_reset ())
  let _ : Unit := (validation_debug_reset ())
  let ⟨_, ⟨_, input_bytes⟩⟩ ← do (stateless_input ())
  let input_ref ← do (decode_stateless_input_ref ⟨_, ⟨_, input_bytes⟩⟩)
  (fatal_error_set_input input_ref)
  (verify_stateless_payload input_ref)
  (write_validation_result input_ref true)

