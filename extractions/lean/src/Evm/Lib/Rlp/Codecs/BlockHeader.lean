import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Kernel.Scratch
import Evm.Primitives.Fork
import Evm.Primitives.Block
import Evm.Lib.Rlp.Encoding
import Evm.Kernel.Environment
import Evm.Kernel.Logs
import Evm.Lib.Rlp.Codecs.Receipts

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

/-! # Block header RLP codec

Canonical execution-header encoding and hashing over already recomputed payload
commitments. -/

/-- The block header hash: `keccak256(rlp(header))` with the recomputed
body roots spliced in (YP §4.4; post-merge constants for ommers,
difficulty, and nonce). -/
def block_header_hash (header : BlockHeader) (transactions_root : (Vector (BitVec 8) 32)) (withdrawals_root : (Vector (BitVec 8) 32)) (requests_hash : (Vector (BitVec 8) 32)) (block_access_list_hash : (Vector (BitVec 8) 32)) : SailM (Vector (BitVec 8) 32) := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let word_length := (rlp_word_size ())
  let address_length := (rlp_addr_size ())
  let bloom_length := (3 + LOGS_BLOOM_BYTE_LENGTH)
  let difficulty_length := (rlp_uint_word_size 0)
  let number_length := (rlp_uint_word_size header.number)
  let gas_limit_length := (rlp_uint_word_size header.gas_limit)
  let gas_used_length := (rlp_uint_word_size header.gas_used)
  let timestamp_length := (rlp_uint_word_size header.timestamp)
  let extra_data_length ← do (rlp_input_scratch_slice_size header.extra_data)
  let nonce_length := (1 + EIGHT_BYTE_LENGTH)
  let content_length ← (( do (rlp_scratch_length_add (6 *i word_length) address_length) ) : SailM
    Nat )
  let content_length ← (rlp_scratch_length_add content_length bloom_length)
  let content_length ← (rlp_scratch_length_add content_length difficulty_length)
  let content_length ← (rlp_scratch_length_add content_length number_length)
  let content_length ← (rlp_scratch_length_add content_length gas_limit_length)
  let content_length ← (rlp_scratch_length_add content_length gas_used_length)
  let content_length ← (rlp_scratch_length_add content_length timestamp_length)
  let content_length ← (rlp_scratch_length_add content_length extra_data_length)
  let content_length ← (rlp_scratch_length_add content_length nonce_length)
  let content_length ← (( do
    if ((profile.fork ≥b London) : Bool)
    then
      (do
        let field_length := (rlp_uint_word_size header.base_fee)
        (rlp_scratch_length_add content_length field_length))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((profile.fork ≥b Shanghai) : Bool)
    then
      (do
        (rlp_scratch_length_add content_length word_length))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((profile.fork ≥b Cancun) : Bool)
    then
      (do
        let blob_gas_used_length := (rlp_uint_word_size header.blob_gas_used)
        let excess_blob_gas_length := (rlp_uint_word_size header.excess_blob_gas)
        let content_length ← (rlp_scratch_length_add content_length blob_gas_used_length)
        let content_length ← (rlp_scratch_length_add content_length excess_blob_gas_length)
        (rlp_scratch_length_add content_length word_length))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((profile.fork ≥b Prague) : Bool)
    then
      (do
        (rlp_scratch_length_add content_length word_length))
    else (pure content_length) ) : SailM Nat )
  let content_length ← (( do
    if ((profile.fork ≥b Amsterdam) : Bool)
    then
      (do
        let slot_number_length := (rlp_uint_word_size header.slot_number)
        let content_length ← (rlp_scratch_length_add content_length word_length)
        (rlp_scratch_length_add content_length slot_number_length))
    else (pure content_length) ) : SailM Nat )
  if ((749 <b content_length) : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  let bounded_content_length : Nat := (Nat.mod content_length 750)
  let content_len := bounded_content_length
  let encoded_length ← do (rlp_list_size content_len)
  let encoder ← do (rlp_encoder_begin encoded_length)
  (rlp_write_list_prefix content_len)
  let parent_hash := (hash_to_word header.parent_hash)
  (rlp_write_word parent_hash)
  let ommer_hash := (hash_to_word EMPTY_OMMER_HASH)
  (rlp_write_word ommer_hash)
  (rlp_write_addr header.fee_recipient)
  let state_root := (hash_to_word header.state_root)
  (rlp_write_word state_root)
  let transactions_root_word := (hash_to_word transactions_root)
  (rlp_write_word transactions_root_word)
  let receipts_root := (hash_to_word header.receipts_root)
  (rlp_write_word receipts_root)
  let logs_bloom ← do (logs_bloom_from_ref header.logs_bloom)
  (rlp_write_logs_bloom logs_bloom)
  (rlp_write_uint_word 0)
  (rlp_write_uint_word header.number)
  (rlp_write_uint_word header.gas_limit)
  (rlp_write_uint_word header.gas_used)
  (rlp_write_uint_word header.timestamp)
  (rlp_write_input_slice header.extra_data)
  (rlp_write_word header.prev_randao)
  (rlp_write_string_prefix EIGHT_BYTE_LENGTH 0x00#8)
  (scratch_push_word_be ZERO_WORD EIGHT_BYTE_LENGTH)
  if ((profile.fork ≥b London) : Bool)
  then (rlp_write_uint_word header.base_fee)
  else (pure ())
  if ((profile.fork ≥b Shanghai) : Bool)
  then
    (do
      let withdrawals_root_word := (hash_to_word withdrawals_root)
      (rlp_write_word withdrawals_root_word))
  else (pure ())
  if ((profile.fork ≥b Cancun) : Bool)
  then
    (do
      (rlp_write_uint_word header.blob_gas_used)
      (rlp_write_uint_word header.excess_blob_gas)
      let parent_beacon_block_root := (hash_to_word header.parent_beacon_block_root)
      (rlp_write_word parent_beacon_block_root))
  else (pure ())
  if ((profile.fork ≥b Prague) : Bool)
  then
    (do
      let requests_hash_word := (hash_to_word requests_hash)
      (rlp_write_word requests_hash_word))
  else (pure ())
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (do
      let block_access_list_hash_word := (hash_to_word block_access_list_hash)
      (rlp_write_word block_access_list_hash_word)
      (rlp_write_uint_word header.slot_number))
  else (pure ())
  let ⟨_, ⟨_, encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  let block_hash ← do (scratch_keccak256 ⟨_, ⟨_, encoded⟩⟩)
  (rlp_encoder_rewind encoder)
  (pure block_hash)

