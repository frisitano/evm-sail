import Evm.Flow
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Kernel.Scratch
import Evm.Primitives.Tx
import Evm.Lib.Rlp.Encoding
import Evm.Kernel.Logs

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

/-! # Receipt RLP codec

The logs-bloom construction (YP §4.4.1) and the receipts trie
(EIP-2718 typed receipt encoding). -/

/-- Sizes the RLP content of one retained record's topic list. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def topics_rlp_content_size (index : Nat) : SailM Nat := do
  let size : Nat := 0
  let topic : Nat := 0
  let topic_count ← do (log_topics_count index)
  let word_size := (rlp_word_size ())
  let encoded_word_length := (rlp_scratch_small_length word_size)
  let (size, topic) ← (( do
    let loop_vars ← whileFuelM (fuel :=(topic_count -i topic)) (fun (size, topic) => (pure (topic <b topic_count))) (size, topic)
      fun (size, topic) => do
        assert true "loop dummy assert"
        let size ← (rlp_scratch_length_add size encoded_word_length)
        let topic ← (log_store_index_increment topic)
        (pure (size, topic))
    (pure loop_vars) ) : SailM (Nat × Nat) )
  (pure size)

/-- Sizes one retained record's topic list with its list evm_prefix. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def topics_rlp_size (index : Nat) : SailM Nat := do
  let content_size ← do (topics_rlp_content_size index)
  (rlp_scratch_list_size content_size)

/-- Sizes one retained log entry. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def log_entry_rlp_content_size (index : Nat) : SailM Nat := do
  let address_size := (rlp_addr_size ())
  let address_length := (rlp_scratch_small_length address_size)
  let topics_length ← do (topics_rlp_size index)
  let ⟨_, ⟨_, data⟩⟩ ← do (read_log_data index)
  let data_length ← do (rlp_log_scratch_slice_size ⟨_, ⟨_, data⟩⟩)
  let address_and_topics_length ← do (rlp_scratch_length_add address_length topics_length)
  (rlp_scratch_length_add address_and_topics_length data_length)

/-- Sizes one retained log entry with its list evm_prefix. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def log_entry_rlp_size (index : Nat) : SailM Nat := do
  let content_size ← do (log_entry_rlp_content_size index)
  (rlp_scratch_list_size content_size)

/-- Sizes the RLP content of a retained log range. -/
def logs_rlp_content_size (logs : LogSeriesRef) : SailM Nat := do
  let size : Nat := 0
  let offset : Nat := 0
  let (offset, size) ← (( do
    let loop_vars ← whileFuelM (fuel :=(logs.count -i offset)) (fun (offset, size) => (pure (offset <b logs.count))) (offset, size)
      fun (offset, size) => do
        assert true "loop dummy assert"
        let index ← do (log_store_index_add logs.start offset)
        let entry_size ← do (log_entry_rlp_size index)
        let size ← (rlp_scratch_length_add size entry_size)
        let offset ← (log_store_index_increment offset)
        (pure (offset, size))
    (pure loop_vars) ) : SailM (Nat × Nat) )
  (pure size)

/-- Sizes a retained log range with its list evm_prefix. -/
def logs_rlp_size (logs : LogSeriesRef) : SailM Nat := do
  let content_size ← do (logs_rlp_content_size logs)
  (rlp_scratch_list_size content_size)

/-- Writes one retained record's topic list. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def rlp_write_topics (index : Nat) : SailM Unit := do
  let content_size ← do (topics_rlp_content_size index)
  (rlp_write_list_prefix content_size)
  let topic : Nat := 0
  let topic_count ← do (log_topics_count index)
  let topic ← (( do
    let loop_vars ← whileFuelM (fuel :=(topic_count -i topic)) (fun topic => (pure (topic <b topic_count))) topic
      fun topic => do
        assert true "loop dummy assert"
        let value ← do (log_topic index topic)
        (rlp_write_word value)
        (log_store_index_increment topic)
    (pure loop_vars) ) : SailM Nat )
  (pure ())

/-- Writes one canonical retained receipt log entry. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def rlp_write_log_entry (index : Nat) : SailM Unit := do
  let content_size ← do (log_entry_rlp_content_size index)
  (rlp_write_list_prefix content_size)
  let address ← do (log_address index)
  (rlp_write_addr address)
  (rlp_write_topics index)
  let ⟨_, ⟨_, data⟩⟩ ← do (read_log_data index)
  (rlp_write_log_data_slice ⟨_, ⟨_, data⟩⟩)

/-- Writes the RLP list of a retained log range. -/
def rlp_write_logs (logs : LogSeriesRef) : SailM Unit := do
  let content_size ← do (logs_rlp_content_size logs)
  (rlp_write_list_prefix content_size)
  let offset : Nat := 0
  let offset ← (( do
    let loop_vars ← whileFuelM (fuel :=(logs.count -i offset)) (fun offset => (pure (offset <b logs.count))) offset
      fun offset => do
        assert true "loop dummy assert"
        let index ← do (log_store_index_add logs.start offset)
        (rlp_write_log_entry index)
        (log_store_index_increment offset)
    (pure loop_vars) ) : SailM Nat )
  (pure ())

/-- Writes a bloom as its fixed-width RLP byte string without constructing a
temporary Sail list. -/
def rlp_write_logs_bloom (bloom : (Vector (BitVec 8) 256)) : SailM Unit := do
  (rlp_write_string_prefix LOGS_BLOOM_BYTE_LENGTH 0x00#8)
  (scratch_push_fixed_bytes_256 bloom)

/-- Writes the bloom belonging to one receipt, constructed from its retained
log range (YP §4.4.1). Keeping this small semantic hook separate lets an
optimized backend populate the already-reserved RLP span directly from its
log-store accumulator without materializing a Sail bloom value. -/
/- Type quantifiers: receipt_dependentWitness4 : Nat, receipt_dependentWitness3 : Nat, receipt_dependentWitness2
  : Nat, receipt_dependentWitness1 : Nat, receipt_dependentWitness0 : Nat, 0 ≤
  receipt_dependentWitness4 ∧
  receipt_dependentWitness4 ≤ (2 ^ 64 - 1) ∧
  0 ≤ receipt_dependentWitness3 ∧
  receipt_dependentWitness3 ≤ receipt_dependentWitness4 ∧
  0 ≤ receipt_dependentWitness2 ∧
  receipt_dependentWitness2 ≤ receipt_dependentWitness4 ∧
  0 ≤ receipt_dependentWitness1 ∧
  receipt_dependentWitness1 ≤ receipt_dependentWitness3 ∧
  0 ≤ receipt_dependentWitness0 ∧
  receipt_dependentWitness0 ≤ receipt_dependentWitness4 ∧
  receipt_dependentWitness2 ≤ (receipt_dependentWitness1 + receipt_dependentWitness0) -/
def receipt_write_logs_bloom (receipt : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) : SailM Unit := do
  let receipt_dependentWitness0 := (receipt).1
  let receipt_dependentWitness1 := ((receipt).2).1
  let receipt_dependentWitness2 := (((receipt).2).2).1
  let receipt_dependentWitness3 := ((((receipt).2).2).2).1
  let receipt_dependentWitness4 := (((((receipt).2).2).2).2).1
  let receipt := (((((receipt).2).2).2).2).2
  let bloom ← do (logs_bloom_for_logs receipt.logs)
  (rlp_write_logs_bloom bloom)

/- Type quantifiers: r_dependentWitness4 : Nat, r_dependentWitness3 : Nat, r_dependentWitness2 : Nat, r_dependentWitness1
  : Nat, r_dependentWitness0 : Nat, cumulative_gas_used : Nat, (receipt_cumulative_gas_value_valid cumulative_gas_used), 0
  ≤ r_dependentWitness4 ∧
  r_dependentWitness4 ≤ (2 ^ 64 - 1) ∧
  0 ≤ r_dependentWitness3 ∧
  r_dependentWitness3 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness2 ∧
  r_dependentWitness2 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness1 ∧
  r_dependentWitness1 ≤ r_dependentWitness3 ∧
  0 ≤ r_dependentWitness0 ∧
  r_dependentWitness0 ≤ r_dependentWitness4 ∧
  r_dependentWitness2 ≤ (r_dependentWitness1 + r_dependentWitness0) -/
def receipt_payload_content_size (r : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) (cumulative_gas_used : Nat) : SailM Nat := do
  let r_dependentWitness0 := (r).1
  let r_dependentWitness1 := ((r).2).1
  let r_dependentWitness2 := (((r).2).2).1
  let r_dependentWitness3 := ((((r).2).2).2).1
  let r_dependentWitness4 := (((((r).2).2).2).2).1
  let r := (((((r).2).2).2).2).2
  let status : Nat :=
    if (r.success : Bool)
    then 1
    else 0
  let status_size := (rlp_uint_word_size status)
  let status_length := (rlp_scratch_small_length status_size)
  let gas_word ← do (word_of_nat_byte_count cumulative_gas_used)
  let gas_size := (rlp_uint_word_size gas_word)
  let gas_length := (rlp_scratch_small_length gas_size)
  let bloom_prefix_size ← do (rlp_length_prefix_len LOGS_BLOOM_BYTE_LENGTH)
  let bloom_prefix_length := (rlp_scratch_small_length bloom_prefix_size)
  let bloom_length ← do (rlp_scratch_length_add LOGS_BLOOM_BYTE_LENGTH bloom_prefix_length)
  let logs_length ← do (logs_rlp_size r.logs)
  let fixed_length ← do (rlp_scratch_length_add status_length gas_length)
  let fixed_and_bloom_length ← do (rlp_scratch_length_add fixed_length bloom_length)
  (rlp_scratch_length_add fixed_and_bloom_length logs_length)

/- Type quantifiers: r_dependentWitness4 : Nat, r_dependentWitness3 : Nat, r_dependentWitness2 : Nat, r_dependentWitness1
  : Nat, r_dependentWitness0 : Nat, cumulative_gas_used : Nat, (receipt_cumulative_gas_value_valid cumulative_gas_used), 0
  ≤ r_dependentWitness4 ∧
  r_dependentWitness4 ≤ (2 ^ 64 - 1) ∧
  0 ≤ r_dependentWitness3 ∧
  r_dependentWitness3 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness2 ∧
  r_dependentWitness2 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness1 ∧
  r_dependentWitness1 ≤ r_dependentWitness3 ∧
  0 ≤ r_dependentWitness0 ∧
  r_dependentWitness0 ≤ r_dependentWitness4 ∧
  r_dependentWitness2 ≤ (r_dependentWitness1 + r_dependentWitness0) -/
def receipt_encoded_length (r : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) (cumulative_gas_used : Nat) : SailM Nat := do
  let r_dependentWitness0 := (r).1
  let r_dependentWitness1 := ((r).2).1
  let r_dependentWitness2 := (((r).2).2).1
  let r_dependentWitness3 := ((((r).2).2).2).1
  let r_dependentWitness4 := (((((r).2).2).2).2).1
  let r := (((((r).2).2).2).2).2
  let content_len ← do
    (receipt_payload_content_size ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩ cumulative_gas_used)
  let envelope_type : (BitVec 8) := (tx_envelope_type r.tx_type)
  let typed := (envelope_type != 0x00#8)
  let payload_len ← do (rlp_scratch_list_size content_len)
  if (typed : Bool)
  then (rlp_scratch_length_add payload_len 1)
  else (pure payload_len)

/- Type quantifiers: r_dependentWitness4 : Nat, r_dependentWitness3 : Nat, r_dependentWitness2 : Nat, r_dependentWitness1
  : Nat, r_dependentWitness0 : Nat, cumulative_gas_used : Nat, (receipt_cumulative_gas_value_valid cumulative_gas_used), 0
  ≤ r_dependentWitness4 ∧
  r_dependentWitness4 ≤ (2 ^ 64 - 1) ∧
  0 ≤ r_dependentWitness3 ∧
  r_dependentWitness3 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness2 ∧
  r_dependentWitness2 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness1 ∧
  r_dependentWitness1 ≤ r_dependentWitness3 ∧
  0 ≤ r_dependentWitness0 ∧
  r_dependentWitness0 ≤ r_dependentWitness4 ∧
  r_dependentWitness2 ≤ (r_dependentWitness1 + r_dependentWitness0) -/
def receipt_write_encoded (r : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) (cumulative_gas_used : Nat) : SailM Unit := do
  let r_dependentWitness0 := (r).1
  let r_dependentWitness1 := ((r).2).1
  let r_dependentWitness2 := (((r).2).2).1
  let r_dependentWitness3 := ((((r).2).2).2).1
  let r_dependentWitness4 := (((((r).2).2).2).2).1
  let r := (((((r).2).2).2).2).2
  let status : Nat :=
    if (r.success : Bool)
    then 1
    else 0
  let content_len ← do
    (receipt_payload_content_size ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩ cumulative_gas_used)
  let envelope_type : (BitVec 8) := (tx_envelope_type r.tx_type)
  let typed := (envelope_type != 0x00#8)
  if (typed : Bool)
  then (scratch_push_byte envelope_type)
  else (pure ())
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word status)
  (rlp_write_uint_word cumulative_gas_used)
  (receipt_write_logs_bloom ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩)
  (rlp_write_logs r.logs)

/- Type quantifiers: r_dependentWitness4 : Nat, r_dependentWitness3 : Nat, r_dependentWitness2 : Nat, r_dependentWitness1
  : Nat, r_dependentWitness0 : Nat, cumulative_gas_used : Nat, (receipt_cumulative_gas_value_valid cumulative_gas_used), 0
  ≤ r_dependentWitness4 ∧
  r_dependentWitness4 ≤ (2 ^ 64 - 1) ∧
  0 ≤ r_dependentWitness3 ∧
  r_dependentWitness3 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness2 ∧
  r_dependentWitness2 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness1 ∧
  r_dependentWitness1 ≤ r_dependentWitness3 ∧
  0 ≤ r_dependentWitness0 ∧
  r_dependentWitness0 ≤ r_dependentWitness4 ∧
  r_dependentWitness2 ≤ (r_dependentWitness1 + r_dependentWitness0) -/
def receipt_encoded (r : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) (cumulative_gas_used : Nat) : SailM (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let r_dependentWitness0 := (r).1
  let r_dependentWitness1 := ((r).2).1
  let r_dependentWitness2 := (((r).2).2).1
  let r_dependentWitness3 := ((((r).2).2).2).1
  let r_dependentWitness4 := (((((r).2).2).2).2).1
  let r := (((((r).2).2).2).2).2
  let encoded_len ← do
    (receipt_encoded_length ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩ cumulative_gas_used)
  let encoder ← do (rlp_encoder_begin encoded_len)
  (receipt_write_encoded ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩ cumulative_gas_used)
  (rlp_encoder_finish encoder)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ ssz_uint_bound -/
def receipt_record_write_length (value : Nat) : SailM Unit := do
  let byte_0 := (get_slice_int 8 value 0)
  (scratch_push_byte byte_0)
  let byte_1 := (get_slice_int 8 value 8)
  (scratch_push_byte byte_1)
  let byte_2 := (get_slice_int 8 value 16)
  (scratch_push_byte byte_2)
  let byte_3 := (get_slice_int 8 value 24)
  (scratch_push_byte byte_3)
  let byte_4 := (get_slice_int 8 value 32)
  (scratch_push_byte byte_4)
  let byte_5 := (get_slice_int 8 value 40)
  (scratch_push_byte byte_5)
  let byte_6 := (get_slice_int 8 value 48)
  (scratch_push_byte byte_6)
  let byte_7 := (get_slice_int 8 value 56)
  (scratch_push_byte byte_7)

/- Type quantifiers: r_dependentWitness4 : Nat, r_dependentWitness3 : Nat, r_dependentWitness2 : Nat, r_dependentWitness1
  : Nat, r_dependentWitness0 : Nat, cumulative_gas_used : Nat, (receipt_cumulative_gas_value_valid cumulative_gas_used), 0
  ≤ r_dependentWitness4 ∧
  r_dependentWitness4 ≤ (2 ^ 64 - 1) ∧
  0 ≤ r_dependentWitness3 ∧
  r_dependentWitness3 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness2 ∧
  r_dependentWitness2 ≤ r_dependentWitness4 ∧
  0 ≤ r_dependentWitness1 ∧
  r_dependentWitness1 ≤ r_dependentWitness3 ∧
  0 ≤ r_dependentWitness0 ∧
  r_dependentWitness0 ≤ r_dependentWitness4 ∧
  r_dependentWitness2 ≤ (r_dependentWitness1 + r_dependentWitness0) -/
def receipt_record_append (r : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) (cumulative_gas_used : Nat) : SailM Unit := do
  let r_dependentWitness0 := (r).1
  let r_dependentWitness1 := ((r).2).1
  let r_dependentWitness2 := (((r).2).2).1
  let r_dependentWitness3 := ((((r).2).2).2).1
  let r_dependentWitness4 := (((((r).2).2).2).2).1
  let r := (((((r).2).2).2).2).2
  let encoded_len ← do
    (receipt_encoded_length ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩ cumulative_gas_used)
  let record_len ← do (scratch_length_add EIGHT_BYTE_LENGTH encoded_len)
  let _record_start ← do (scratch_reserve record_len)
  (receipt_record_write_length encoded_len)
  let encoder ← do (rlp_encoder_begin encoded_len)
  (receipt_write_encoded ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, r⟩⟩⟩⟩⟩ cumulative_gas_used)
  let ⟨_, ⟨_, _encoded⟩⟩ ← do (rlp_encoder_finish encoder)
  (pure ())

