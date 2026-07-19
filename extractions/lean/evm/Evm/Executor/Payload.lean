import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.ChainConfig
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

/-! # Payload commitments

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself. -/

/-- The RLP of one withdrawal (EIP-4895), assembled in the scratch
arena. -/
def withdrawal_rlp (withdrawal : EvmByteSlice) : SailM EvmByteSlice := do
  let index ← do
    (do
        let semanticResult ← (ssz_uint withdrawal WD_INDEX)
        pure ((semanticResult).value))
  let validator_index ← do
    (do
        let semanticResult ← (ssz_uint withdrawal WD_VALIDATOR_INDEX)
        pure ((semanticResult).value))
  let address ← do (sub_slice withdrawal WD_ADDRESS ADDRESS_BYTE_LENGTH)
  let amount ← do
    (do
        let semanticResult ← (ssz_uint withdrawal WD_AMOUNT)
        pure ((semanticResult).value))
  let content_len ← do (rlp_protocol_quantity_size ⟨index⟩)
  let content_len ←
    (byte_quantity_add content_len (← (rlp_protocol_quantity_size ⟨validator_index⟩)))
  let content_len ← (byte_quantity_add content_len (← (rlp_slice_size address)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_protocol_quantity_size ⟨amount⟩)))
  let encoded_len ← do (rlp_list_size content_len)
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_protocol_quantity ⟨validator_index⟩)
  (rlp_write_slice address)
  (rlp_write_protocol_quantity ⟨amount⟩)
  (rlp_finish start encoded_len)

/-- The block header hash: `keccak256(rlp(header))` with the recomputed
body roots spliced in (YP §4.4; post-merge constants for ommers,
difficulty, and nonce). -/
def block_header_hash (header : BlockHeader) (transactions_root : hash) (withdrawals_root : hash) (requests_hash : hash) (block_access_list_hash : hash) : SailM hash := do
  let bloom := (logs_bloom_bytes header.logs_bloom)
  let nonce : (List (BitVec 8)) := [0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8]
  let content_len := (rlp_word_size ())
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ← (byte_quantity_add content_len (rlp_addr_size ()))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_bytes_size bloom LOGS_BLOOM_BYTE_LENGTH)))
  let content_len ← (byte_quantity_add content_len (← (rlp_protocol_quantity_size ⟨0⟩)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_protocol_quantity_size ⟨(header.number).value⟩)))
  let content_len ← (byte_quantity_add content_len (← (rlp_gas_size header.gas_limit)))
  let content_len ← (byte_quantity_add content_len (← (rlp_gas_size header.gas_used)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_protocol_quantity_size ⟨(header.timestamp).value⟩)))
  let content_len ← (byte_quantity_add content_len (← (rlp_slice_size header.extra_data)))
  let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
  let content_len ← (byte_quantity_add content_len (← (rlp_bytes_size nonce EIGHT_BYTE_LENGTH)))
  let content_len ← (( do
    if ((fork_gteq (← readReg k_fork) London) : Bool)
    then
      (do
        (byte_quantity_add content_len (← (rlp_uint_word_size header.base_fee))))
    else (pure content_len) ) : SailM byte_quantity )
  let content_len ← (( do
    if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
    then
      (do
        (byte_quantity_add content_len (rlp_word_size ())))
    else (pure content_len) ) : SailM byte_quantity )
  let content_len ← (( do
    if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
    then
      (do
        let content_len ←
          (byte_quantity_add content_len
            (← (rlp_protocol_quantity_size ⟨(header.blob_gas_used).value⟩)))
        let content_len ←
          (byte_quantity_add content_len
            (← (rlp_protocol_quantity_size ⟨(header.excess_blob_gas).value⟩)))
        (byte_quantity_add content_len (rlp_word_size ())))
    else (pure content_len) ) : SailM byte_quantity )
  let content_len ← (( do
    if ((fork_gteq (← readReg k_fork) Prague) : Bool)
    then
      (do
        (byte_quantity_add content_len (rlp_word_size ())))
    else (pure content_len) ) : SailM byte_quantity )
  let content_len ← (( do
    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
    then
      (do
        let content_len ← (byte_quantity_add content_len (rlp_word_size ()))
        (byte_quantity_add content_len
          (← (rlp_protocol_quantity_size ⟨(header.slot_number).value⟩))))
    else (pure content_len) ) : SailM byte_quantity )
  let encoded_len ← do (rlp_list_size content_len)
  let mark ← do (scratch_begin ())
  (rlp_write_list_prefix content_len)
  (rlp_write_word (hash_to_word header.parent_hash))
  (rlp_write_word (hash_to_word EMPTY_OMMER_HASH))
  (rlp_write_addr header.fee_recipient)
  (rlp_write_word (hash_to_word header.state_root))
  (rlp_write_word (hash_to_word transactions_root))
  (rlp_write_word (hash_to_word header.receipts_root))
  (rlp_write_bytes bloom LOGS_BLOOM_BYTE_LENGTH)
  (rlp_write_protocol_quantity ⟨0⟩)
  (rlp_write_protocol_quantity ⟨(header.number).value⟩)
  (rlp_write_gas header.gas_limit)
  (rlp_write_gas header.gas_used)
  (rlp_write_protocol_quantity ⟨(header.timestamp).value⟩)
  (rlp_write_slice header.extra_data)
  (rlp_write_word header.prev_randao)
  (rlp_write_bytes nonce EIGHT_BYTE_LENGTH)
  if ((fork_gteq (← readReg k_fork) London) : Bool)
  then (rlp_write_uint_word header.base_fee)
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then (rlp_write_word (hash_to_word withdrawals_root))
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
  then
    (do
      (rlp_write_protocol_quantity ⟨(header.blob_gas_used).value⟩)
      (rlp_write_protocol_quantity ⟨(header.excess_blob_gas).value⟩)
      (rlp_write_word (hash_to_word header.parent_beacon_block_root)))
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Prague) : Bool)
  then (rlp_write_word (hash_to_word requests_hash))
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
  then
    (do
      (rlp_write_word (hash_to_word block_access_list_hash))
      (rlp_write_protocol_quantity ⟨(header.slot_number).value⟩))
  else (pure ())
  let encoded ← do (rlp_finish mark encoded_len)
  let block_hash ← do (keccak256_slice encoded)
  (scratch_rewind mark)
  (pure block_hash)

/-- The transactions-trie root (YP §4.4.2): leaf `i` holds the raw
EIP-2718 envelope of transaction `i`, keyed by `rlp(i)`. -/
def transaction_trie_root (txs : SszListRef) : SailM hash := do
  let builder := (trie_builder_empty ())
  let cursor := (rlp_index_cursor ⟨(txs.count).value⟩)
  let (builder, cursor) ← (( do
    let loop_vars ← whileFuelM (fuel :=((cursor.count).value -i (cursor.position).value)) (fun (builder, cursor) => (pure (! (rlp_index_cursor_empty
          cursor)))) (builder, cursor)
      fun (builder, cursor) => do
        assert true "loop dummy assert"
        let (item, next) ← do (rlp_index_cursor_pop cursor)
        let cursor : RlpIndexCursor := next
        let tx ← do (ssz_list_at txs ⟨(item.index).value⟩)
        let builder ← (trie_insert_item builder (item_leaf item.key tx) item.next_key)
        (pure (builder, cursor))
    (pure loop_vars) ) : SailM (TrieBuilder × RlpIndexCursor) )
  (trie_builder_root builder)

/-- The withdrawals-trie root (EIP-4895): leaf `i` holds
`rlp(withdrawal_i)`, keyed by `rlp(i)`. -/
def withdrawals_trie_root (wds : SszListRef) : SailM hash := do
  let builder := (trie_builder_empty ())
  let cursor := (rlp_index_cursor ⟨(wds.count).value⟩)
  let (builder, cursor) ← (( do
    let loop_vars ← whileFuelM (fuel :=((cursor.count).value -i (cursor.position).value)) (fun (builder, cursor) => (pure (! (rlp_index_cursor_empty
          cursor)))) (builder, cursor)
      fun (builder, cursor) => do
        assert true "loop dummy assert"
        let (item, next) ← do (rlp_index_cursor_pop cursor)
        let cursor : RlpIndexCursor := next
        let scratch_mark ← do (scratch_begin ())
        let withdrawal ← do (ssz_fixed_list_at wds ⟨(item.index).value⟩ WD_SIZE)
        let value ← do (withdrawal_rlp withdrawal)
        let builder ← (trie_insert_item builder (item_leaf item.key value) item.next_key)
        (scratch_rewind scratch_mark)
        (pure (builder, cursor))
    (pure loop_vars) ) : SailM (TrieBuilder × RlpIndexCursor) )
  (trie_builder_root builder)

/-- The `excess_blob_gas` the header must carry, derived from the
authenticated parent (EIP-4844). -/
def expected_payload_excess_blob_gas (witness : WitnessContext) : SailM blob_gas := do
  let semanticResult ← do
    (do
        let semanticResult ← (next_excess_blob_gas ⟨(witness.parent_excess_blob_gas).value⟩
        ⟨(witness.parent_blob_gas_used).value⟩ witness.parent_base_fee_per_gas)
        pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- The EIP-7685 requests hash: `sha256` over the present request-type
digests in request-type order; the request bodies remain
region-backed through the hash calls. -/
def execution_requests_hash (input_ref : StatelessInputRef) : SailM hash := do
  let l0 := input_ref.deposits.len
  let l1 := input_ref.withdrawal_requests.len
  let l2 := input_ref.consolidation_requests.len
  let d0 ← (( do
    if ((bne l0 BYTE_ZERO) : Bool)
    then (sha256_request_digest 0x00#8 input_ref.deposits)
    else (pure ZERO_HASH) ) : SailM b256 )
  let d1 ← (( do
    if ((bne l1 BYTE_ZERO) : Bool)
    then (sha256_request_digest 0x01#8 input_ref.withdrawal_requests)
    else (pure ZERO_HASH) ) : SailM b256 )
  let d2 ← (( do
    if ((bne l2 BYTE_ZERO) : Bool)
    then (sha256_request_digest 0x02#8 input_ref.consolidation_requests)
    else (pure ZERO_HASH) ) : SailM b256 )
  let segs : (List Bytes) := []
  let segs : (List Bytes) :=
    if ((bne l2 BYTE_ZERO) : Bool)
    then ((bytes_list (hash_to_bytes32 d2) WORD_BYTE_LENGTH) :: segs)
    else segs
  let segs : (List Bytes) :=
    if ((bne l1 BYTE_ZERO) : Bool)
    then ((bytes_list (hash_to_bytes32 d1) WORD_BYTE_LENGTH) :: segs)
    else segs
  let segs : (List Bytes) :=
    if ((bne l0 BYTE_ZERO) : Bool)
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
  if ((! ((9 ≤b (input.chain_config.fork_index).value) && ((((input.chain_config.fork_index).value ≤b 20) && (input.chain_config.activation_active && (chain_config_blob_schedule_valid
               input.chain_config))) : Bool))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((gas_lt header.gas_limit header.gas_used) : Bool)
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

