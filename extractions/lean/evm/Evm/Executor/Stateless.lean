import Evm.Flow
import Evm.Arith
import Evm.Exceptions
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Host.Kernel.Environment
import Evm.Lib.StateTrie
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
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

/-- Type quantifiers: k_ex160365_ : Bool -/
def validate_executed_block (block' : Block) (input_ref : StatelessInputRef) (exec_ok : Bool) (result : BlockExecutionResult) : SailM Unit := do
  let header := block'.header
  let gas_used_ok := (gas_equal result.gas_acc header.gas_used)
  let blob_gas_used_ok ← do
    (pure ((fork_lt (← readReg k_fork) Cancun) || ((result.blob_gas_acc == header.blob_gas_used) : Bool)))
  let poststate_ok ← do (pure ((← (compute_state_root ())) == header.state_root))
  let receipts_root_ok := (result.receipts_root == header.receipts_root)
  let logs_bloom_ok := (logs_bloom_equal result.logs_bloom header.logs_bloom)
  let block_access_list_size_ok : Bool := true
  let block_access_list_ok : Bool := true
  let execution_requests_ok : Bool := true
  let execution_requests_ok ← (( do
    if ((fork_gteq (← readReg k_fork) Prague) : Bool)
    then
      (do
        (pure ((← (bytes_segments_equal_slice [(BytesSlice result.requests.deposits)]
                input_ref.deposits)) && ((← (bytes_segments_equal_slice
                  [(BytesSlice result.requests.withdrawals)] input_ref.withdrawal_requests)) && (← (bytes_segments_equal_slice
                  [(BytesSlice result.requests.consolidations)] input_ref.consolidation_requests))))))
    else (pure execution_requests_ok) ) : SailM Bool )
  let (block_access_list_ok, block_access_list_size_ok) ← (( do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then
      (do
        let block_access_list ← do (encode_block_access_list ())
        let .Gas gas_limit := header.gas_limit
        let maximum_items ← do (exact_quotient gas_limit 2000)
        let block_access_list_size_ok : Bool := (block_access_list.item_count ≤b maximum_items)
        let block_access_list_ok ←
          (bytes_segments_equal_slice [(BytesSlice block_access_list.bytes)]
            block'.body.block_access_list)
        (pure (block_access_list_ok, block_access_list_size_ok)))
    else (pure (block_access_list_ok, block_access_list_size_ok)) ) : SailM (Bool × Bool) )
  if (result.block_gas_overflow : Bool)
  then sailThrow ((InvalidBlock GasUsedExceedsLimit))
  else (pure ())
  if (result.blob_gas_overflow : Bool)
  then sailThrow ((InvalidBlock BlobGasLimitExceeded))
  else (pure ())
  if ((! exec_ok) : Bool)
  then sailThrow ((InvalidBlock ExecutionInvalid))
  else (pure ())
  if ((! execution_requests_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidExecutionRequests))
  else (pure ())
  if ((! block_access_list_size_ok) : Bool)
  then sailThrow ((InvalidBlock BlockAccessListTooLarge))
  else (pure ())
  if ((! gas_used_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidGasUsed))
  else (pure ())
  if ((! blob_gas_used_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidBlobGasUsed))
  else (pure ())
  if ((! poststate_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidStateRoot))
  else (pure ())
  if ((! receipts_root_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidReceiptsRoot))
  else (pure ())
  if ((! logs_bloom_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidLogsBloom))
  else (pure ())
  if ((! block_access_list_ok) : Bool)
  then sailThrow ((InvalidBlock InvalidBlockAccessList))
  else (pure ())

def undefined_StatelessValidationFailure (_ : Unit) : SailM StatelessValidationFailure := do
  (pure { scope := ← (undefined_bitvector 8)
          reason := ← (undefined_BlockError ()) })

def verify_stateless_payload (input_ref : StatelessInputRef) : SailM StatelessValidationResult := do
  let active_scope := SCOPE_DECODE_INPUT
  sailTryCatch ((do
      let _ : Unit := (cycle_scope_start active_scope)
      (scratch_reset ())
      let input ← do (decode_stateless_input input_ref)
      writeReg k_fork input.chain_config.fork
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
      let (exec_ok, result) ← do
        (execute_block_body block'.body input_ref.public_keys block'.header.gas_limit)
      let _ : Unit := (cycle_scope_end active_scope)
      let active_scope : (BitVec 8) := SCOPE_VALIDATE_RESULT
      let _ : Unit := (cycle_scope_start active_scope)
      (validate_executed_block block' input_ref exec_ok result)
      let _ : Unit := (cycle_scope_end active_scope)
      (pure (StatelessPayloadValid ())))) (fun the_exception => 
    match the_exception with
      | .InvalidBlock reason =>
        (let _ : Unit := (cycle_scope_end active_scope)
        (pure (StatelessPayloadInvalid
            { scope := active_scope
              reason := reason }))))

