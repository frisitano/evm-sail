import Evm.Flow
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
import Evm.Exceptions
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Host.Kernel.Environment
import Evm.Lib.StateTrie
import Evm.Lib.Ssz.StatelessInput
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

/-! # Stateless block validation

Validation of the commitments produced by executing a block body: gas and
blob-gas accounting, the post-state root, the receipts root and logs
bloom, the EIP-7685 execution requests, and the EIP-7928 block access
list. -/

/-- Checks every executed-block commitment against the header and payload,
throwing the specific `InvalidBlock` reason on the first failure:
gas/blob-gas totals, post-state root, receipts root, logs bloom,
execution-request bytes (Prague+), and block-access-list bytes and
size (Amsterdam+). -/
def validate_executed_block (block' : Block) (input_ref : StatelessInputRef) (result : BlockExecutionResult) : SailM Unit := do
  let header := block'.header
  if ((fork_gteq (← readReg k_fork) Prague) : Bool)
  then
    (do
      if (((! (← (bytes_segments_equal_slice [(BytesSlice result.requests.deposits)]
                 input_ref.deposits))) || ((! (← (bytes_segments_equal_slice
                   [(BytesSlice result.requests.withdrawals)] input_ref.withdrawal_requests))) || ((! (← (bytes_segments_equal_slice
                     [(BytesSlice result.requests.consolidations)] input_ref.consolidation_requests))) || ((! (← (bytes_segments_equal_slice
                       [(BytesSlice result.requests.builder_deposits)]
                       input_ref.builder_deposit_requests))) || (! (← (bytes_segments_equal_slice
                       [(BytesSlice result.requests.builder_exits)] input_ref.builder_exit_requests))))))) : Bool)
      then sailThrow ((InvalidBlock InvalidExecutionRequests))
      else (pure ()))
  else (pure ())
  if ((result.header_gas_used != header.gas_used) : Bool)
  then sailThrow ((InvalidBlock InvalidGasUsed))
  else (pure ())
  if (((fork_gteq (← readReg k_fork) Cancun) && (((result.blob_gas_used).value != (header.blob_gas_used).value) : Bool)) : Bool)
  then sailThrow ((InvalidBlock InvalidBlobGasUsed))
  else (pure ())
  let _ : Unit := (cycle_scope_start SCOPE_STATE_ROOT)
  let poststate ← do (compute_state_root ())
  let _ : Unit := (cycle_scope_end SCOPE_STATE_ROOT)
  if ((bne poststate header.state_root) : Bool)
  then sailThrow ((InvalidBlock InvalidStateRoot))
  else (pure ())
  if ((bne result.receipts_root header.receipts_root) : Bool)
  then sailThrow ((InvalidBlock InvalidReceiptsRoot))
  else (pure ())
  if ((! (logs_bloom_equal result.logs_bloom header.logs_bloom)) : Bool)
  then sailThrow ((InvalidBlock InvalidLogsBloom))
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      let _ : Unit := (cycle_scope_start SCOPE_BLOCK_ACCESS_LIST)
      let block_access_list ← do (encode_block_access_list ())
      let _ : Unit := (cycle_scope_end SCOPE_BLOCK_ACCESS_LIST)
      let maximum_items := (Int.ediv (header.gas_limit).value 2000)
      if (((block_access_list.item_count).value >b maximum_items) : Bool)
      then sailThrow ((InvalidBlock BlockAccessListTooLarge))
      else (pure ())
      if ((! (← (bytes_segments_equal_slice [(BytesSlice block_access_list.bytes)]
               block'.body.block_access_list))) : Bool)
      then sailThrow ((InvalidBlock InvalidBlockAccessList))
      else (pure ()))
  else (pure ())

def undefined_StatelessValidationFailure (_ : Unit) : SailM StatelessValidationFailure := do
  (pure { scope := ← (undefined_bitvector 8),
          reason := ← (undefined_BlockError ()) })

/-- The stateless verification pipeline: decode the semantic envelope,
index the witness, validate the payload commitments, execute the
block body one transaction at a time, and validate the execution
results; the first violated rule becomes the failure verdict. -/
def verify_stateless_payload (input_ref : StatelessInputRef) : SailM StatelessValidationResult := do
  let active_scope := SCOPE_DECODE_INPUT
  sailTryCatch ((do
      let _ : Unit := (cycle_scope_start active_scope)
      (scratch_reset ())
      let input ← do (decode_stateless_input input_ref)
      writeReg k_fork input_ref.protocol.fork
      let _ : Unit := (cycle_scope_end active_scope)
      let active_scope : (BitVec 8) := SCOPE_INDEX_WITNESS
      let _ : Unit := (cycle_scope_start active_scope)
      let witness ← do (index_execution_witness input_ref)
      let _ : Unit := (cycle_scope_end active_scope)
      let active_scope : (BitVec 8) := SCOPE_VALIDATE_PAYLOAD
      let _ : Unit := (cycle_scope_start active_scope)
      (validate_execution_payload input input_ref witness)
      let _ : Unit := (cycle_scope_end active_scope)
      let active_scope : (BitVec 8) := SCOPE_EXECUTE_BLOCK
      let _ : Unit := (cycle_scope_start active_scope)
      let block' := input.payload.block'
      let result ← do
        (execute_block_body block'.body input_ref.public_keys ⟨(block'.header.gas_limit).value⟩)
      let _ : Unit := (cycle_scope_end active_scope)
      let active_scope : (BitVec 8) := SCOPE_VALIDATE_RESULT
      let _ : Unit := (cycle_scope_start active_scope)
      (validate_executed_block block' input_ref result)
      let _ : Unit := (cycle_scope_end active_scope)
      (pure (StatelessPayloadValid ())))) (fun the_exception => 
    match the_exception with
      | .InvalidBlock reason =>
        (let _ : Unit := (cycle_scope_end active_scope)
        (pure (StatelessPayloadInvalid
            { scope := active_scope,
              reason := reason }))))

