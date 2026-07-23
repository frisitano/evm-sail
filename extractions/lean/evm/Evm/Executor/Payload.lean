import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Lib.Ssz.Ssz
import Evm.Lib.Rlp.Rlp
import Evm.Host.Kernel.Environment
import Evm.Evm.Gas
import Evm.Lib.Mpt.Updates
import Evm.Lib.Mpt.Indexed
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Receipts

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

/-! # Payload commitments

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself. -/

/-- The RLP of one withdrawal (EIP-4895), assembled in the scratch
arena. -/
/- Type quantifiers: k_ex409384_ : Nat, k_ex409383_ : Nat, 0 ≤ k_ex409383_ ∧ 0 ≤ k_ex409384_
  ∧ k_ex409384_ = 44 -/
def withdrawal_rlp (withdrawal : (EvmByteSliceLength 44)) : SailM EvmByteSlice := do
  let withdrawal := ((withdrawal).2).2
  let index ← do
    (do
        let publicResult ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_INDEX)
        pure ((publicResult).value))
  let validator_index ← do
    (do
        let publicResult ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_VALIDATOR_INDEX)
        pure ((publicResult).value))
  let address := (sub_slice withdrawal WD_ADDRESS ADDRESS_BYTE_LENGTH)
  let amount ← do
    (do
        let publicResult ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_AMOUNT)
        pure ((publicResult).value))
  let index_length := (rlp_uint_word_size index)
  let validator_index_length := (rlp_uint_word_size validator_index)
  let address_length ← do (rlp_slice_size address)
  let amount_length := (rlp_uint_word_size amount)
  let content_length := (((index_length + validator_index_length) + address_length) + amount_length)
  if ((48 <b content_length) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let bounded_content_length : Nat := (Nat.mod content_length 49)
  let content_len := bounded_content_length
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word index)
  (rlp_write_uint_word validator_index)
  (rlp_write_slice ⟨_, ⟨_, address⟩⟩)
  (rlp_write_uint_word amount)
  (rlp_finish start)

/-- The block header hash: `keccak256(rlp(header))` with the recomputed
body roots spliced in (YP §4.4; post-merge constants for ommers,
difficulty, and nonce). -/
def block_header_hash (header : BlockHeader) (transactions_root : hash) (withdrawals_root : hash) (requests_hash : hash) (block_access_list_hash : hash) : SailM hash := do
  let bloom := (logs_bloom_bytes header.logs_bloom)
  let nonce : (List (BitVec 8)) := [0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]
  let word_length := (rlp_word_size ())
  let address_length := (rlp_addr_size ())
  let bloom_length ← do (rlp_scratch_bytes_size bloom LOGS_BLOOM_BYTE_LENGTH)
  let difficulty_length := (rlp_uint_word_size 0)
  let number_length ← do (rlp_uint_nat_size header.number)
  let gas_limit_length := (rlp_uint_word_size (header.gas_limit).value)
  let gas_used_length ← do (rlp_uint_nat_size header.gas_used)
  let timestamp_length ← do (rlp_uint_nat_size header.timestamp)
  let extra_data_length ← do (rlp_scratch_slice_size header.extra_data)
  let nonce_length ← do (rlp_scratch_bytes_size nonce EIGHT_BYTE_LENGTH)
  let content_length : Nat := (rlp_scratch_length_add (6 *i word_length) address_length)
  let content_length : Nat := (rlp_scratch_length_add content_length bloom_length)
  let content_length : Nat := (rlp_scratch_length_add content_length difficulty_length)
  let content_length : Nat := (rlp_scratch_length_add content_length number_length)
  let content_length : Nat := (rlp_scratch_length_add content_length gas_limit_length)
  let content_length : Nat := (rlp_scratch_length_add content_length gas_used_length)
  let content_length : Nat := (rlp_scratch_length_add content_length timestamp_length)
  let content_length : Nat := (rlp_scratch_length_add content_length extra_data_length)
  let content_length : Nat := (rlp_scratch_length_add content_length nonce_length)
  let content_length ← (( do
    if ((fork_gteq (← readReg k_fork) London) : Bool)
    then
      (let field_length := (rlp_uint_word_size (header.base_fee).value)
      (pure (rlp_scratch_length_add content_length field_length)))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
    then (pure (rlp_scratch_length_add content_length word_length))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
    then
      (let blob_gas_used_length := (rlp_uint_word_size (header.blob_gas_used).value)
      let excess_blob_gas_length := (rlp_uint_word_size (header.excess_blob_gas).value)
      let content_length : Nat := (rlp_scratch_length_add content_length blob_gas_used_length)
      let content_length : Nat := (rlp_scratch_length_add content_length excess_blob_gas_length)
      (pure (rlp_scratch_length_add content_length word_length)))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((fork_gteq (← readReg k_fork) Prague) : Bool)
    then (pure (rlp_scratch_length_add content_length word_length))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then
      (let slot_number_length := (rlp_uint_word_size (header.slot_number).value)
      let content_length : Nat := (rlp_scratch_length_add content_length word_length)
      (pure (rlp_scratch_length_add content_length slot_number_length)))
    else (pure content_length) ) : SailM Nat )
  if ((749 <b content_length) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  let bounded_content_length : Nat := (Nat.mod content_length 750)
  let content_len := bounded_content_length
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_word ⟨((hash_to_word header.parent_hash)).value⟩)
  (rlp_write_word ⟨((hash_to_word EMPTY_OMMER_HASH)).value⟩)
  (rlp_write_addr header.fee_recipient)
  (rlp_write_word ⟨((hash_to_word header.state_root)).value⟩)
  (rlp_write_word ⟨((hash_to_word transactions_root)).value⟩)
  (rlp_write_word ⟨((hash_to_word header.receipts_root)).value⟩)
  (rlp_write_bytes bloom LOGS_BLOOM_BYTE_LENGTH)
  (rlp_write_uint_word 0)
  (rlp_write_uint_nat header.number)
  (rlp_write_uint_word (header.gas_limit).value)
  (rlp_write_uint_nat header.gas_used)
  (rlp_write_uint_nat header.timestamp)
  (rlp_write_slice header.extra_data)
  (rlp_write_word ⟨(header.prev_randao).value⟩)
  (rlp_write_bytes nonce EIGHT_BYTE_LENGTH)
  if ((fork_gteq (← readReg k_fork) London) : Bool)
  then (rlp_write_uint_word (header.base_fee).value)
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then (rlp_write_word ⟨((hash_to_word withdrawals_root)).value⟩)
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
  then
    (do
      (rlp_write_uint_word (header.blob_gas_used).value)
      (rlp_write_uint_word (header.excess_blob_gas).value)
      (rlp_write_word ⟨((hash_to_word header.parent_beacon_block_root)).value⟩))
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Prague) : Bool)
  then (rlp_write_word ⟨((hash_to_word requests_hash)).value⟩)
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      (rlp_write_word ⟨((hash_to_word block_access_list_hash)).value⟩)
      (rlp_write_uint_word (header.slot_number).value))
  else (pure ())
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_finish mark)
  let block_hash ← do (keccak256_slice ⟨_, ⟨_, encoded⟩⟩)
  (scratch_rewind mark)
  (pure block_hash)

/-- The transactions-trie root (YP §4.4.2): leaf `i` holds the raw
EIP-2718 envelope of transaction `i`, keyed by `rlp(i)`. -/
def transaction_trie_root (txs : TransactionListRef) : SailM hash := do
  let builder := (trie_builder_empty ())
  let cursor : (RlpIndexCursor (2 ^ 20)) := (rlp_index_cursor (k_maximum := (2 ^ 20)) txs.count)
  let (builder, cursor) ← (( do
    let loop_vars ← whileFuelM (fuel :=(cursor.count -i cursor.position)) (fun (builder, cursor) => (pure (! (rlp_index_cursor_empty
          cursor)))) (builder, cursor)
      fun (builder, cursor) => do
        assert true "loop dummy assert"
        let (item, next) ← do (rlp_index_cursor_pop cursor)
        let cursor : (RlpIndexCursor (2 ^ 20)) := next
        let ⟨_, ⟨_, tx⟩⟩ ← do (ssz_list_at txs item.index)
        let builder ←
          (trie_insert_item builder (item_leaf item.key ⟨_, ⟨_, tx⟩⟩) item.next_key)
        (pure (builder, cursor))
    (pure loop_vars) ) : SailM (TrieBuilder × (RlpIndexCursor (2 ^ 20))) )
  (trie_builder_root builder)

/-- The withdrawals-trie root (EIP-4895): leaf `i` holds
`rlp(withdrawal_i)`, keyed by `rlp(i)`. -/
def withdrawals_trie_root (wds : WithdrawalListRef) : SailM hash := do
  let builder := (trie_builder_empty ())
  let cursor : (RlpIndexCursor (2 ^ 4)) := (rlp_index_cursor (k_maximum := (2 ^ 4)) wds.count)
  let (builder, cursor) ← (( do
    let loop_vars ← whileFuelM (fuel :=(cursor.count -i cursor.position)) (fun (builder, cursor) => (pure (! (rlp_index_cursor_empty
          cursor)))) (builder, cursor)
      fun (builder, cursor) => do
        assert true "loop dummy assert"
        let (item, next) ← do (rlp_index_cursor_pop cursor)
        let cursor : (RlpIndexCursor (2 ^ 4)) := next
        let scratch_mark ← do (scratch_begin ())
        let ⟨_, ⟨_, withdrawal⟩⟩ ← do (ssz_fixed_list_at wds item.index WD_SIZE)
        let ⟨_, ⟨_, value⟩⟩ ← do (withdrawal_rlp ⟨_, ⟨_, withdrawal⟩⟩)
        let builder ←
          (trie_insert_item builder (item_leaf item.key ⟨_, ⟨_, value⟩⟩) item.next_key)
        (scratch_rewind scratch_mark)
        (pure (builder, cursor))
    (pure loop_vars) ) : SailM (TrieBuilder × (RlpIndexCursor (2 ^ 4))) )
  (trie_builder_root builder)

/-- The `excess_blob_gas` the header must carry, derived from the
authenticated parent (EIP-4844). -/
def expected_payload_excess_blob_gas (witness : WitnessContext) : SailM excess_blob_gas := do
  let publicResult ← do
    (do
        let publicResult ← (next_excess_blob_gas ⟨(witness.parent_excess_blob_gas).value⟩
        ⟨(witness.parent_blob_gas_used).value⟩ ⟨(witness.parent_base_fee_per_gas).value⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- The EIP-7685 requests hash: `sha256` over the present request-type
digests in request-type order; the request bodies remain
region-backed through the hash calls. -/
def execution_requests_hash (input_ref : StatelessInputRef) : SailM hash := do
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
  let segs : (List Bytes) := []
  let segs : (List Bytes) :=
    if ((builder_exit_requests.len != 0) : Bool)
    then ((bytes_list (hash_to_bytes32 d4) WORD_BYTE_LENGTH) :: segs)
    else segs
  let segs : (List Bytes) :=
    if ((builder_deposit_requests.len != 0) : Bool)
    then ((bytes_list (hash_to_bytes32 d3) WORD_BYTE_LENGTH) :: segs)
    else segs
  let segs : (List Bytes) :=
    if ((consolidation_requests.len != 0) : Bool)
    then ((bytes_list (hash_to_bytes32 d2) WORD_BYTE_LENGTH) :: segs)
    else segs
  let segs : (List Bytes) :=
    if ((withdrawal_requests.len != 0) : Bool)
    then ((bytes_list (hash_to_bytes32 d1) WORD_BYTE_LENGTH) :: segs)
    else segs
  let segs : (List Bytes) :=
    if ((deposits.len != 0) : Bool)
    then ((bytes_list (hash_to_bytes32 d0) WORD_BYTE_LENGTH) :: segs)
    else segs
  (sha256_segments segs)

/-- Validates every commitment checkable before transaction decoding:
parent linkage, gas and blob-gas header rules, the transactions and
withdrawals roots, the requests hash, and the block hash. The
supplied block access list is hashed once for the header;
post-execution validation compares its bytes against the canonical
reconstruction. -/
def validate_execution_payload (input : StatelessInput) (input_ref : StatelessInputRef) (witness : WitnessContext) : SailM Unit := do
  let payload := input.payload
  let block' := payload.block'
  let header := block'.header
  let body := block'.body
  if (((header.gas_limit).value <b header.gas_used) : Bool)
  then sailThrow ((InvalidBlock InvalidGasUsed))
  else (pure ())
  if ((bne witness.parent_hash header.parent_hash) : Bool)
  then sailThrow ((InvalidBlock InvalidParentHash))
  else (pure ())
  if (((fork_gteq (← readReg k_fork) Cancun) && (← do
         (pure ((header.excess_blob_gas).value != ((← (expected_payload_excess_blob_gas witness))).value)))) : Bool)
  then sailThrow ((InvalidBlock InvalidExcessBlobGas))
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Paris) : Bool)
  then
    (do
      let transactions_root ← do (transaction_trie_root body.transactions)
      let withdrawals_root ← do
        if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
        then (withdrawals_trie_root body.withdrawals)
        else (pure EMPTY_TRIE_ROOT)
      let requests_hash ← do
        if ((fork_gteq (← readReg k_fork) Prague) : Bool)
        then (execution_requests_hash input_ref)
        else (pure ZERO_HASH)
      let block_access_list_hash ← do
        if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
        then (keccak256_slice body.block_access_list)
        else (pure ZERO_HASH)
      let computed_block_hash ← do
        (block_header_hash header transactions_root withdrawals_root requests_hash
          block_access_list_hash)
      if ((bne computed_block_hash payload.expected_block_hash) : Bool)
      then sailThrow ((InvalidBlock InvalidBlockHash))
      else (pure ()))
  else (pure ())

