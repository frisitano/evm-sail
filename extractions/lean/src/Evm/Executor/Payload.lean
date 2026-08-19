import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Kernel.Scratch
import Evm.Primitives.Crypto
import Evm.Primitives.Fork
import Evm.Kernel.Environment
import Evm.Evm.Gas
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Receipts
import Evm.Lib.Rlp.Codecs.BlockHeader

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

/-! # Payload commitments

Recursive transaction, withdrawal, and receipt tries plus the header-level
commitment checks over their recomputed roots. -/

/-- The transactions-trie root (YP §4.4.2): leaf `i` holds the raw
EIP-2718 envelope of transaction `i`, keyed by `rlp(i)`. -/
def transaction_trie_root (txs : (BoundedSszListRef (2 ^ 20))) : SailM (Vector (BitVec 8) 32) := do
  let source := (IndexedTransactions txs)
  (indexed_trie_root source)

/-- The withdrawals-trie root (EIP-4895): leaf `i` holds
`rlp(withdrawal_i)`, keyed by `rlp(i)`. -/
def withdrawals_trie_root (wds : (BoundedSszListRef (2 ^ 4))) : SailM (Vector (BitVec 8) 32) := do
  let source := (IndexedWithdrawals wds)
  (indexed_trie_root source)

/-- The `excess_blob_gas` the header must carry, derived from the
authenticated parent (EIP-4844). -/
def expected_payload_excess_blob_gas (witness : WitnessContext) : SailM Nat := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  (next_excess_blob_gas
    ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile.protocol⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
    witness.parent_excess_blob_gas witness.parent_blob_gas_used witness.parent_base_fee_per_gas)

/-- The EIP-7685 requests hash: `sha256` over the present request-type
digests in request-type order; the request bodies remain
region-backed through the hash calls. -/
def execution_requests_hash (input_ref : StatelessInputRef) : SailM (Vector (BitVec 8) 32) := do
  let ⟨_, ⟨_, deposits⟩⟩ := input_ref.deposits
  let ⟨_, ⟨_, withdrawal_requests⟩⟩ := input_ref.withdrawal_requests
  let ⟨_, ⟨_, consolidation_requests⟩⟩ := input_ref.consolidation_requests
  let ⟨_, ⟨_, builder_deposit_requests⟩⟩ := input_ref.builder_deposit_requests
  let ⟨_, ⟨_, builder_exit_requests⟩⟩ := input_ref.builder_exit_requests
  let d0 ← (( do
    if ((deposits.len != 0) : Bool)
    then (sha256_request_digest 0x00#8 ⟨_, ⟨_, deposits⟩⟩)
    else (pure ZERO_HASH) ) : SailM (Vector (BitVec 8) 32) )
  let d1 ← (( do
    if ((withdrawal_requests.len != 0) : Bool)
    then (sha256_request_digest 0x01#8 ⟨_, ⟨_, withdrawal_requests⟩⟩)
    else (pure ZERO_HASH) ) : SailM (Vector (BitVec 8) 32) )
  let d2 ← (( do
    if ((consolidation_requests.len != 0) : Bool)
    then (sha256_request_digest 0x02#8 ⟨_, ⟨_, consolidation_requests⟩⟩)
    else (pure ZERO_HASH) ) : SailM (Vector (BitVec 8) 32) )
  let d3 ← (( do
    if ((builder_deposit_requests.len != 0) : Bool)
    then (sha256_request_digest 0x03#8 ⟨_, ⟨_, builder_deposit_requests⟩⟩)
    else (pure ZERO_HASH) ) : SailM (Vector (BitVec 8) 32) )
  let d4 ← (( do
    if ((builder_exit_requests.len != 0) : Bool)
    then (sha256_request_digest 0x04#8 ⟨_, ⟨_, builder_exit_requests⟩⟩)
    else (pure ZERO_HASH) ) : SailM (Vector (BitVec 8) 32) )
  let mark ← do (scratch_reserve (5 *i WORD_BYTE_LENGTH))
  if ((deposits.len != 0) : Bool)
  then (scratch_push_b256 d0 WORD_BYTE_LENGTH)
  else (pure ())
  if ((withdrawal_requests.len != 0) : Bool)
  then (scratch_push_b256 d1 WORD_BYTE_LENGTH)
  else (pure ())
  if ((consolidation_requests.len != 0) : Bool)
  then (scratch_push_b256 d2 WORD_BYTE_LENGTH)
  else (pure ())
  if ((builder_deposit_requests.len != 0) : Bool)
  then (scratch_push_b256 d3 WORD_BYTE_LENGTH)
  else (pure ())
  if ((builder_exit_requests.len != 0) : Bool)
  then (scratch_push_b256 d4 WORD_BYTE_LENGTH)
  else (pure ())
  let ⟨_, ⟨_, request_bytes⟩⟩ ← do (scratch_finish mark)
  let digest ← do (scratch_sha256 ⟨_, ⟨_, request_bytes⟩⟩)
  (scratch_rewind mark)
  (pure digest)

/-- Validates every commitment checkable before transaction decoding:
parent linkage, gas and blob-gas header rules, the transactions and
withdrawals roots, the requests hash, and the block hash. The
supplied block access list is hashed once for the header;
post-execution validation compares its bytes against the canonical
reconstruction. -/
def validate_execution_payload (input : StatelessInput) (input_ref : StatelessInputRef) (witness : WitnessContext) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let payload := input.payload
  let block' := payload.block'
  let header := block'.header
  let body := block'.body
  if ((header.gas_limit <b header.gas_used) : Bool)
  then (fatal_error InvalidGasUsed)
  else (pure ())
  if ((bne witness.parent_hash header.parent_hash) : Bool)
  then (fatal_error InvalidParentHash)
  else (pure ())
  let expected_excess_blob_gas ← do (expected_payload_excess_blob_gas witness)
  if (((profile.fork ≥b Cancun) && ((header.excess_blob_gas != expected_excess_blob_gas) : Bool)) : Bool)
  then (fatal_error InvalidExcessBlobGas)
  else (pure ())
  if ((profile.fork ≥b Paris) : Bool)
  then
    (do
      let transactions_root ← do (transaction_trie_root body.transactions)
      let withdrawals_root ← do
        if ((profile.fork ≥b Shanghai) : Bool)
        then (withdrawals_trie_root body.withdrawals)
        else (pure EMPTY_TRIE_ROOT)
      let requests_hash ← do
        if ((profile.fork ≥b Prague) : Bool)
        then (execution_requests_hash input_ref)
        else (pure ZERO_HASH)
      let block_access_list_hash ← do
        if ((profile.fork ≥b Amsterdam) : Bool)
        then (stateless_input_keccak256 body.block_access_list)
        else (pure ZERO_HASH)
      let computed_block_hash ← do
        (block_header_hash header transactions_root withdrawals_root requests_hash
          block_access_list_hash)
      if ((bne computed_block_hash payload.expected_block_hash) : Bool)
      then (fatal_error InvalidBlockHash)
      else (pure ()))
  else (pure ())

