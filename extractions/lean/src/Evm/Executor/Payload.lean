import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Kernel.Scratch
import Evm.Primitives.Crypto
import Evm.Primitives.Fork
import Evm.Lib.Rlp.Encoding
import Evm.Kernel.Environment
import Evm.Evm.Gas
import Evm.Lib.Ssz.StatelessInput
import Evm.Lib.Rlp.Codecs.Withdrawals
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
open HtrProgressiveSource
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
def withdrawals_trie_root (wds : (BoundedSszListRef (2 ^ 20))) : SailM (Vector (BitVec 8) 32) := do
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

/-- EIP-1559 base-fee adjustment from the authenticated parent's gas usage.
Mathematical intermediates preserve the full product before division. -/
def expected_payload_base_fee (witness : WitnessContext) : SailM Nat := do
  if ((! witness.parent_has_base_fee) : Bool)
  then (pure 1000000000)
  else
    (do
      let raw_target := (witness.parent_gas_limit / 2)
      let target ← (( do
        if ((0 <b raw_target) : Bool)
        then (pure raw_target)
        else (fatal_error InvalidGasLimit) ) : SailM Nat )
      let used := witness.parent_gas_used
      let fee := witness.parent_base_fee_per_gas
      if (((target == 0) || ((witness.parent_gas_limit <b used) : Bool)) : Bool)
      then (fatal_error InvalidGasLimit)
      else (pure ())
      if ((used == target) : Bool)
      then (pure fee)
      else
        (if ((target <b used) : Bool)
        then
          (let change := (((fee *i (used - target)) / target) / 8)
          let candidate :=
            (fee + (if ((change <b 1) : Bool)
              then 1
              else change))
          (pure candidate))
        else
          (let change := (((fee *i (target - used)) / target) / 8)
          (pure (fee - change)))))

/-- EIP-7934 bounds the canonical full block RLP, not the SSZ input or the
transactions alone. Check additions before materializing a large sum. -/
def validate_block_rlp_size (header : BlockHeader) (transactions : (BoundedSszListRef (2 ^ 20))) (withdrawals : (BoundedSszListRef (2 ^ 20))) : SailM Unit := do
  let maximum : Nat := 8388608
  let tx_bytes : Nat := 0
  let cursor ← do (ssz_list_cursor transactions)
  let (cursor, tx_bytes) ← (( do
    let loop_vars ← whileFuelM (fuel :=(cursor.items.count -i cursor.index)) (fun (cursor, tx_bytes) => (pure (! (ssz_list_cursor_empty
          cursor)))) (cursor, tx_bytes)
      fun (cursor, tx_bytes) => do
        assert true "loop dummy assert"
        let (tx, next) ← do (ssz_list_pop cursor)
        let size ← (( do
          if (((((tx).2).2.len != 0) && (((tx).2).2.len ≤b (2 ^i 30))) : Bool)
          then
            (do
              if (((BitVec.toNatInt (← (stateless_input_slice_byte tx 0))) ≥b 192) : Bool)
              then (pure ((tx).2).2.len)
              else (rlp_input_slice_size ((tx).2).2))
          else (fatal_error RlpDecode) ) : SailM Nat )
        let total := (tx_bytes + size)
        let tx_bytes ←
          if ((total ≤b maximum) : Bool)
          then (pure total)
          else (fatal_error ExecutionInvalid)
        let cursor : (BoundedSszListCursor (2 ^ 20)) := next
        (pure (cursor, tx_bytes))
    (pure loop_vars) ) : SailM ((BoundedSszListCursor (2 ^ 20)) × Nat) )
  let withdrawal_bytes : Nat := 0
  let rest := withdrawals
  let (rest, withdrawal_bytes) ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun (rest, withdrawal_bytes) => (pure (rest.count != 0))) (rest, withdrawal_bytes)
      fun (rest, withdrawal_bytes) => do
        assert true "loop dummy assert"
        let (withdrawal, next) ← do (ssz_fixed_list_pop rest WD_SIZE)
        let mark ← do (scratch_begin ())
        let ⟨_, ⟨_, encoded⟩⟩ ← do (withdrawal_rlp withdrawal)
        let size := encoded.len
        let total := (withdrawal_bytes + size)
        let withdrawal_bytes ←
          if ((total ≤b maximum) : Bool)
          then (pure total)
          else (fatal_error ExecutionInvalid)
        (scratch_rewind mark)
        let rest : (BoundedSszListRef (2 ^ 20)) := next
        (pure (rest, withdrawal_bytes))
    (pure loop_vars) ) : SailM ((BoundedSszListRef (2 ^ 20)) × Nat) )
  let (_, header_size) ← do (block_header_rlp_lengths header)
  let transactions_size ← do (rlp_list_size tx_bytes)
  let withdrawals_size ← do (rlp_list_size withdrawal_bytes)
  let body_size := (((header_size + transactions_size) + 1) + withdrawals_size)
  let block_size ← do (rlp_list_size body_size)
  if ((maximum <b block_size) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())

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
  let parent_limit :=
    if (((profile.fork == London) && (! witness.parent_has_base_fee)) : Bool)
    then (2 *i witness.parent_gas_limit)
    else witness.parent_gas_limit
  let limit_delta :=
    if ((header.gas_limit <b parent_limit) : Bool)
    then (parent_limit -i header.gas_limit)
    else (header.gas_limit -i parent_limit)
  if (((header.gas_limit <b 5000) || ((((2 ^i 63) ≤b header.gas_limit) || ((parent_limit <b 5000) || ((parent_limit / 1024) ≤b limit_delta))) : Bool)) : Bool)
  then (fatal_error InvalidGasLimit)
  else (pure ())
  if ((profile.fork ≥b Osaka) : Bool)
  then (validate_block_rlp_size header body.transactions body.withdrawals)
  else (pure ())
  if ((header.gas_limit <b header.gas_used) : Bool)
  then (fatal_error InvalidGasUsed)
  else (pure ())
  if ((bne witness.parent_hash header.parent_hash) : Bool)
  then (fatal_error InvalidParentHash)
  else (pure ())
  if ((← if ((profile.fork ≥b London) : Bool)
       then
         (do
           (pure (header.base_fee != (← (expected_payload_base_fee witness)))))
       else (pure false)) : Bool)
  then (fatal_error ExecutionInvalid)
  else (pure ())
  if ((profile.fork ≥b Cancun) : Bool)
  then
    (do
      let expected_excess_blob_gas ← do (expected_payload_excess_blob_gas witness)
      if ((header.excess_blob_gas != expected_excess_blob_gas) : Bool)
      then (fatal_error InvalidExcessBlobGas)
      else (pure ()))
  else (pure ())
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
  else (pure ())
