import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Primitives.Fork
import Evm.Primitives.System
import Evm.Primitives.Tx
import Evm.Primitives.Block
import Evm.Kernel.Environment

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

/-! # State: logs and refunds

Log emission (YP §4.4.1) — including the EIP-7708 transfer and burn
logs — and the gas-refund counter. -/

/-- Appends bounded topic operands without constructing a Sail list. -/
def k_log_topics (topics : LogTopics) : SailM Unit := do
  match topics with
  | .LogTopics0 () => (pure ())
  | .LogTopics1 t0 => (log_add_topic t0)
  | .LogTopics2 (t0, t1) =>
    (do
      (log_add_topic t0)
      (log_add_topic t1))
  | .LogTopics3 (t0, t1, t2) =>
    (do
      (log_add_topic t0)
      (log_add_topic t1)
      (log_add_topic t2))
  | .LogTopics4 (t0, t1, t2, t3) =>
    (do
      (log_add_topic t0)
      (log_add_topic t1)
      (log_add_topic t2)
      (log_add_topic t3))

/-- Appends a log payload without crossing an aggregate/list glue boundary. -/
def k_log_data (data : LogData) : SailM Unit := do
  match data with
  | .LogDataMemory ⟨_, ⟨_, bytes⟩⟩ => (log_add_data_memory ⟨_, ⟨_, bytes⟩⟩)
  | .LogDataWord value => (log_add_data_word value)

/-- Appends a log record (YP §4.4.1) to the transaction's log series. -/
def k_log (a : (Vector (BitVec 8) 20)) (topics : LogTopics) (data : LogData) : SailM Unit := do
  (log_begin a)
  (k_log_topics topics)
  (k_log_data data)

/-- Captures the current transaction's consecutive retained log range. -/
def read_logs (_ : Unit) : SailM LogSeriesRef := do
  let start ← do (logs_tx_start ())
  let count ← do (logs_tx_count ())
  (pure { start := start,
          count := count })

/-- Returns the source-backed payload of one retained log record. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def read_log_data (index : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len))) := do
  let off ← do (log_data_offset index)
  let len ← do (log_data_length index)
  if _sailIf0 : ((len ≤b (((2 ^i 32) - 1) - off)) : Bool) = true
  then
    (pure ((⟨_, ⟨_, (log_data_slice off len)⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))))
  else
    (do
      assert false "log data slice overflow"
      throw Error.Exit)

abbrev LOGS_BLOOM_BYTE_LENGTH : Nat := 256

/-- Constructs a one-hot mask for a bit within a bloom byte. -/
/- Type quantifiers: bit_to_set : Nat, 0 ≤ bit_to_set ∧ bit_to_set ≤ 7 -/
def bloom_bit_mask (bit_to_set : Nat) : (BitVec 8) :=
  (0x01#8 <<< bit_to_set)

/-- Sets one bit (0–2047) in the bloom, most-significant-byte first. -/
/- Type quantifiers: k_ex549996_ : Nat, 0 ≤ k_ex549996_ ∧ k_ex549996_ ≤ 2047 -/
def bloom_set_bit (bloom : (Vector (BitVec 8) 256)) (bit_to_set : Nat) : (Vector (BitVec 8) 256) :=
  let out := bloom
  let quotient := (bit_to_set / 8)
  let natural_byte : Nat := quotient
  let remainder := (Nat.mod bit_to_set 8)
  let bit_in_byte : Nat := remainder
  let mask := (bloom_bit_mask bit_in_byte)
  (vectorUpdate out natural_byte ((GetElem?.getElem! out natural_byte) ||| mask))

/-- Adds a hashed bloom entry: three bits from its KECCAK-256. -/
def bloom_add_entry_hash (bloom : (Vector (BitVec 8) 256)) (h : (Vector (BitVec 8) 32)) : (Vector (BitVec 8) 256) :=
  let bytes := h
  let first_bits :=
    ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes 0) 2 0) +++ (GetElem?.getElem! bytes 1))
  let first_index := (BitVec.toNatInt first_bits)
  let out := (bloom_set_bit bloom first_index)
  let second_bits :=
    ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes 2) 2 0) +++ (GetElem?.getElem! bytes 3))
  let second_index := (BitVec.toNatInt second_bits)
  let out : (Vector (BitVec 8) 256) := (bloom_set_bit out second_index)
  let third_bits :=
    ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes 4) 2 0) +++ (GetElem?.getElem! bytes 5))
  let third_index := (BitVec.toNatInt third_bits)
  (bloom_set_bit out third_index)

/-- Adds one retained log record to the bloom (YP §4.4.1, the M function). -/
/- Type quantifiers: k_ex549997_ : Nat, 0 ≤ k_ex549997_ ∧ k_ex549997_ ≤ (2 ^ 64 - 1) -/
def bloom_add_log_at (bloom : (Vector (BitVec 8) 256)) (index : Nat) : SailM (Vector (BitVec 8) 256) := do
  let address ← do (log_address index)
  let address_hash ← do (keccak256_address address)
  let out := (bloom_add_entry_hash bloom address_hash)
  let topic : Nat := 0
  let topic_count ← do (log_topics_count index)
  let (out, topic) ← (( do
    let loop_vars ← whileFuelM (fuel :=(topic_count -i topic)) (fun (out, topic) => (pure (topic <b topic_count))) (out, topic)
      fun (out, topic) => do
        assert true "loop dummy assert"
        let topic_value ← do (log_topic index topic)
        let topic_hash ← do (keccak256_word topic_value)
        let out : (Vector (BitVec 8) 256) := (bloom_add_entry_hash out topic_hash)
        let topic ← (log_store_index_increment topic)
        (pure (out, topic))
    (pure loop_vars) ) : SailM ((Vector (BitVec 8) 256) × Nat) )
  (pure out)

/-- Adds a consecutive retained log range to a bloom. -/
def bloom_add_logs (bloom : (Vector (BitVec 8) 256)) (logs : LogSeriesRef) : SailM (Vector (BitVec 8) 256) := do
  let out := bloom
  let offset : Nat := 0
  let (offset, out) ← (( do
    let loop_vars ← whileFuelM (fuel :=(logs.count -i offset)) (fun (offset, out) => (pure (offset <b logs.count))) (offset, out)
      fun (offset, out) => do
        assert true "loop dummy assert"
        let index ← do (log_store_index_add logs.start offset)
        let out ← (bloom_add_log_at out index)
        let offset ← (log_store_index_increment offset)
        (pure (offset, out))
    (pure loop_vars) ) : SailM (Nat × (Vector (BitVec 8) 256)) )
  (pure out)

/-- The bloom of one receipt's retained log range. -/
def logs_bloom_for_logs (logs : LogSeriesRef) : SailM (Vector (BitVec 8) 256) := do
  (bloom_add_logs EMPTY_LOGS_BLOOM logs)

/-- Combines two receipt blooms for the block header. -/
def logs_bloom_or (left : (Vector (BitVec 8) 256)) (right : (Vector (BitVec 8) 256)) : (Vector (BitVec 8) 256) := Id.run do
  let out := left
  let loop_k_lower := 0
  let loop_k_upper := 255
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := (vectorUpdate out k ((GetElem?.getElem! out k) ||| (GetElem?.getElem! right k)))
  (pure loop_vars)

/-- Emits the EIP-7708 transfer log for a nonzero, non-self value
transfer (Amsterdam onward). -/
/- Type quantifiers: k_ex549998_ : Nat, 0 ≤ k_ex549998_ ∧ k_ex549998_ ≤ (2 ^ 256 - 1) -/
def k_emit_transfer_log (src : (Vector (BitVec 8) 20)) (dst : (Vector (BitVec 8) 20)) (v : Nat) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let value_is_zero := (word_is_zero v)
  if (((profile.fork <b Amsterdam) || (value_is_zero || (src == dst))) : Bool)
  then (pure ())
  else
    (do
      let source := (address_to_word src)
      let destination := (address_to_word dst)
      let topics := (LogTopics3 (EIP7708_TRANSFER_TOPIC, source, destination))
      let data := (LogDataWord v)
      (k_log EIP7708_SYSTEM_ADDRESS topics data))

/-- Emits the EIP-7708 burn log when a selfdestruct deletion burns a
nonzero balance (Amsterdam onward). -/
/- Type quantifiers: k_ex549999_ : Nat, 0 ≤ k_ex549999_ ∧ k_ex549999_ ≤ (2 ^ 256 - 1) -/
def k_emit_burn_log (a : (Vector (BitVec 8) 20)) (v : Nat) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let value_is_zero := (word_is_zero v)
  if (((profile.fork <b Amsterdam) || value_is_zero) : Bool)
  then (pure ())
  else
    (do
      let address := (address_to_word a)
      let topics := (LogTopics2 (EIP7708_BURN_TOPIC, address))
      let data := (LogDataWord v)
      (k_log EIP7708_SYSTEM_ADDRESS topics data))

