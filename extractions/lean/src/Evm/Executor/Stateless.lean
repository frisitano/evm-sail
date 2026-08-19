import Evm.Flow
import Evm.Exceptions
import Evm.Kernel.Scratch
import Evm.Primitives.Fork
import Evm.Kernel.Environment
import Evm.Lib.StateTrie
import Evm.Host.DebugDisabled
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Receipts
import Evm.Executor.BlockAccessList
import Evm.Executor.Block
import Evm.Executor.Payload

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
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
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

/-! # Stateless block validation

Validation of the commitments produced by executing a block body: gas and
blob-gas accounting, the post-state root, the receipts root and logs
bloom, the EIP-7685 execution requests, and the EIP-7928 block access
list. -/

/-- Checks every executed-block commitment against the header and payload,
throwing the specific `InvalidBlock` reason on the first failure:
gas/blob-gas totals, post-state root, receipts root, logs bloom, and
block-access-list bytes and size (Amsterdam+). Execution-request bytes
(Prague+) are validated where they are collected. -/
def validate_executed_block (block' : Block) (result : BlockExecutionResult) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let header := block'.header
  let _ : Unit :=
    (validation_debug_capture_block_gas result.header_gas_used header.gas_used
      result.execution_gas_used result.state_gas_used)
  if ((result.header_gas_used != header.gas_used) : Bool)
  then (fatal_error InvalidGasUsed)
  else (pure ())
  if (((profile.fork ≥b Cancun) && ((result.blob_gas_used != header.blob_gas_used) : Bool)) : Bool)
  then (fatal_error InvalidBlobGasUsed)
  else (pure ())
  let poststate ← do (compute_state_root ())
  if ((bne poststate header.state_root) : Bool)
  then (fatal_error InvalidStateRoot)
  else (pure ())
  if ((bne result.receipts_root header.receipts_root) : Bool)
  then (fatal_error InvalidReceiptsRoot)
  else (pure ())
  let logs_bloom_matches ← do (block_logs_bloom_matches result.logs header.logs_bloom)
  let logs_bloom_mismatch := (! logs_bloom_matches)
  if (logs_bloom_mismatch : Bool)
  then (fatal_error InvalidLogsBloom)
  else (pure ())
  if ((profile.fork ≥b Amsterdam) : Bool)
  then (validate_block_access_list block'.body.block_access_list execution_profile.gas.block_limit)
  else (pure ())

def VALIDATION_STAGE_DECODE_INPUT : validation_stage := 1

def VALIDATION_STAGE_INDEX_WITNESS : validation_stage := 2

def VALIDATION_STAGE_VALIDATE_PAYLOAD : validation_stage := 3

def VALIDATION_STAGE_EXECUTE_BLOCK : validation_stage := 4

def VALIDATION_STAGE_VALIDATE_RESULT : validation_stage := 5

/-- The stateless verification pipeline: decode the semantic envelope,
index the witness, validate the payload commitments, execute the
block body one transaction at a time, and validate the execution results.
Any failure terminates through `fatal_error`; normal return means valid. -/
def verify_stateless_payload (input_ref : StatelessInputRef) : SailM Unit := do
  (scratch_reset ())
  let input ← do (decode_stateless_input input_ref)
  let witness ← do (index_execution_witness input_ref)
  (validate_execution_payload input input_ref witness)
  let block' := input.payload.block'
  let result ← do (execute_block_body block'.body input_ref)
  (validate_executed_block block' result)

