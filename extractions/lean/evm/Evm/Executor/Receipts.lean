import Evm.Flow
import Evm.Primitives.Quantities
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp
import Evm.Evm.Machine
import Evm.Lib.Mpt.Updates
import Evm.Lib.Mpt.Indexed

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

/-! # Receipts, blooms, and the receipts trie

The logs-bloom construction (YP §4.4.1) and the receipts trie
(EIP-2718 typed receipt encoding). -/

def LOGS_BLOOM_BYTE_LENGTH : Nat := 256

/-- Constructs a one-hot mask for a bit within a bloom byte. -/
/- Type quantifiers: bit_to_set : Nat, 0 ≤ bit_to_set ∧ bit_to_set ≤ 7 -/
def bloom_bit_mask (bit_to_set : Nat) : (BitVec 8) :=
  (0x01#8 <<< bit_to_set)

/-- Sets one bit (0–2047) in the bloom, most-significant-byte first. -/
/- Type quantifiers: k_ex417387_ : Nat, 0 ≤ k_ex417387_ ∧ k_ex417387_ ≤ 2047 -/
def bloom_set_bit (bloom : (Vector (BitVec 8) 256)) (bit_to_set : Nat) : (Vector (BitVec 8) 256) :=
  let out := bloom
  let quotient := (bit_to_set / 8)
  let natural_byte : Nat := quotient
  let remainder := (Nat.mod bit_to_set 8)
  let bit_in_byte : Nat := remainder
  (vectorUpdate out natural_byte
    ((GetElem?.getElem! out natural_byte) ||| (bloom_bit_mask bit_in_byte)))

/-- Adds a hashed bloom entry: three bits from its KECCAK-256. -/
def bloom_add_entry_hash (bloom : (Vector (BitVec 8) 256)) (h : (Vector (BitVec 8) 32)) : (Vector (BitVec 8) 256) :=
  let bytes := h
  let out :=
    (bloom_set_bit bloom
      (BitVec.toNatInt
        ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes 31) 2 0) +++ (GetElem?.getElem! bytes 30))))
  let out : (Vector (BitVec 8) 256) :=
    (bloom_set_bit out
      (BitVec.toNatInt
        ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes 29) 2 0) +++ (GetElem?.getElem! bytes 28))))
  (bloom_set_bit out
    (BitVec.toNatInt
      ((Sail.BitVec.extractLsb (GetElem?.getElem! bytes 27) 2 0) +++ (GetElem?.getElem! bytes 26))))

/-- Adds every topic of a log record to the bloom. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_bloom_add_topics (bloom : (Vector (BitVec 8) 256)) (topics : (List Nat)) (_reclimit : Nat) : SailM (Vector (BitVec 8) 256) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match topics with
      | [] => (pure bloom)
      | (topic :: rest) =>
        (_rec_bloom_add_topics (bloom_add_entry_hash bloom (← (keccak256_word topic))) rest
          _reclimit_pred))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Adds every topic of a log record to the bloom. -/
def bloom_add_topics (bloom : (Vector (BitVec 8) 256)) (topics : (List Nat)) : SailM (Vector (BitVec 8) 256) := do
  let _measure := ((2 ^i 64) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bloom_add_topics bloom topics (_measure + 1))

/-- Adds a log record to the bloom: its address and every topic
(YP §4.4.1, the M function). -/
def bloom_add_log (bloom : (Vector (BitVec 8) 256)) (log : LogEntry) : SailM (Vector (BitVec 8) 256) := do
  let with_address ← do (pure (bloom_add_entry_hash bloom (← (keccak256_address log.address))))
  (bloom_add_topics with_address log.topics)

/-- Adds a sequence of log records to a bloom. -/
def bloom_add_logs (bloom : (Vector (BitVec 8) 256)) (logs : (List LogEntry)) : SailM (Vector (BitVec 8) 256) := do
  match logs with
  | [] => (pure bloom)
  | (log :: rest) => (bloom_add_logs (← (bloom_add_log bloom log)) rest)

/-- The bloom of one receipt's log series. -/
def logs_bloom_for_logs (logs : (List LogEntry)) : SailM (Vector (BitVec 8) 256) := do
  (bloom_add_logs EMPTY_LOGS_BLOOM logs)

/-- Sizes the RLP content of a topic list. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_topics_rlp_content_size (topics : (List Nat)) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match topics with
      | [] => (pure 0)
      | (_ :: rest) =>
        (do
          let topic_length := (rlp_scratch_small_length (rlp_word_size ()))
          let rest_length ← do (_rec_topics_rlp_content_size rest _reclimit_pred)
          (pure (rlp_scratch_length_add topic_length rest_length))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the RLP content of a topic list. -/
def topics_rlp_content_size (topics : (List Nat)) : SailM Nat := do
  let _measure := ((2 ^i 64) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_topics_rlp_content_size topics (_measure + 1))

def topics_rlp_size (topics : (List Nat)) : SailM Nat := do
  (rlp_scratch_list_size (← (topics_rlp_content_size topics)))

/-- Sizes the RLP content of one log entry. -/
def log_entry_rlp_content_size (log : LogEntry) : SailM Nat := do
  let address_length := (rlp_scratch_small_length (rlp_addr_size ()))
  let topics_length ← do (topics_rlp_size log.topics)
  let data_length ← do (rlp_scratch_slice_size log.data)
  (pure (rlp_scratch_length_add (rlp_scratch_length_add address_length topics_length) data_length))

def log_entry_rlp_size (log : LogEntry) : SailM Nat := do
  (rlp_scratch_list_size (← (log_entry_rlp_content_size log)))

/-- Sizes the RLP content of a log-entry list. -/
def logs_rlp_content_size (logs : (List LogEntry)) : SailM Nat := do
  match logs with
  | [] => (pure 0)
  | (log :: rest) =>
    (do
      let log_length ← do (log_entry_rlp_size log)
      let rest_length ← do (logs_rlp_content_size rest)
      (pure (rlp_scratch_length_add log_length rest_length)))

def logs_rlp_size (logs : (List LogEntry)) : SailM Nat := do
  (rlp_scratch_list_size (← (logs_rlp_content_size logs)))

/-- Writes topic words as the content of an RLP list. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_rlp_write_topics_content (topics : (List Nat)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match topics with
      | [] => (pure ())
      | (topic :: rest) =>
        (do
          (rlp_write_word topic)
          (_rec_rlp_write_topics_content rest _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes topic words as the content of an RLP list. -/
def rlp_write_topics_content (topics : (List Nat)) : SailM Unit := do
  let _measure := ((2 ^i 64) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_rlp_write_topics_content topics (_measure + 1))

/-- Writes an RLP list of topic words. -/
def rlp_write_topics (topics : (List Nat)) : SailM Unit := do
  (rlp_write_list_prefix (← (topics_rlp_content_size topics)))
  (rlp_write_topics_content topics)

/-- Writes one canonical receipt log entry. -/
def rlp_write_log_entry (log : LogEntry) : SailM Unit := do
  (rlp_write_list_prefix (← (log_entry_rlp_content_size log)))
  (rlp_write_addr log.address)
  (rlp_write_topics log.topics)
  (rlp_write_slice log.data)

/-- Writes log entries as the content of an RLP list. -/
def rlp_write_logs_content (logs : (List LogEntry)) : SailM Unit := do
  match logs with
  | [] => (pure ())
  | (log :: rest) =>
    (do
      (rlp_write_log_entry log)
      (rlp_write_logs_content rest))

/-- Writes the RLP list of receipt logs. -/
def rlp_write_logs (logs : (List LogEntry)) : SailM Unit := do
  (rlp_write_list_prefix (← (logs_rlp_content_size logs)))
  (rlp_write_logs_content logs)

/-- Sizes a receipt payload from status, gas, bloom, and logs. -/
/- Type quantifiers: k_ex417394_ : Nat, 0 ≤ k_ex417394_ -/
def receipt_payload_content_size (r : Receipt) (cumulative_gas_used : Nat) : SailM Nat := do
  let status : Nat :=
    if (r.success : Bool)
    then 1
    else 0
  let status_length := (rlp_scratch_small_length (rlp_uint_word_size status))
  let gas_word ← do (word_of_nat_byte_count cumulative_gas_used)
  let gas_length := (rlp_scratch_small_length (rlp_uint_word_size gas_word))
  let bloom_length ← do
    (pure (rlp_scratch_length_add LOGS_BLOOM_BYTE_LENGTH
        (rlp_scratch_small_length (← (rlp_length_prefix_len LOGS_BLOOM_BYTE_LENGTH)))))
  let logs_length ← do (logs_rlp_size r.logs)
  let fixed_length := (rlp_scratch_length_add status_length gas_length)
  (pure (rlp_scratch_length_add (rlp_scratch_length_add fixed_length bloom_length) logs_length))

/-- The receipt as stored in the receipts trie: the RLP payload,
prefixed by the envelope type byte for typed transactions
(EIP-2718). -/
/- Type quantifiers: k_ex417397_ : Nat, 0 ≤ k_ex417397_ -/
def receipt_encoded (r : Receipt) (cumulative_gas_used : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let status : Nat :=
    if (r.success : Bool)
    then 1
    else 0
  let bloom ← do (pure (logs_bloom_bytes (← (logs_bloom_for_logs r.logs))))
  let content_len ← do (receipt_payload_content_size r cumulative_gas_used)
  let typed := ((tx_type_byte r.tx_type) != 0x00#8)
  let start ← do (scratch_begin ())
  if (typed : Bool)
  then (scratch_push_bytes [(tx_type_byte r.tx_type)] 1)
  else (pure ())
  (rlp_write_list_prefix content_len)
  (rlp_write_uint_word status)
  (rlp_write_uint_nat cumulative_gas_used)
  (rlp_write_bytes bloom LOGS_BLOOM_BYTE_LENGTH)
  (rlp_write_logs r.logs)
  (rlp_finish start)

/-- Constructs an empty receipts-trie accumulator. -/
def receipt_accumulator_empty (_ : Unit) : ReceiptAccumulator :=
  { builder := (trie_builder_empty ()),
    first := none,
    pending := none,
    count := 0,
    cumulative_gas_used := 0,
    bloom := EMPTY_LOGS_BLOOM }

/-- Encodes and inserts one pending receipt with its next lexical key. -/
def receipt_insert (builder : TrieBuilder) (pending : PendingReceipt) (next_key : (Option TriePath)) : SailM TrieBuilder := do
  let mark ← do (scratch_begin ())
  let ⟨_, ⟨_, value⟩⟩ ← do (receipt_encoded pending.receipt pending.cumulative_gas_used)
  let inserted ← do
    (trie_insert_item builder
      (item_leaf (← (trie_index_key pending.index)) ⟨_, ⟨_, value⟩⟩) next_key)
  (scratch_rewind mark)
  (pure inserted)

/-- Adds the next numeric receipt while respecting trie-key lexical order. -/
/- Type quantifiers: k_ex417398_ : Nat, 0 ≤ k_ex417398_ ∧ k_ex417398_ ≤ (2 ^ 20) -/
def receipt_accumulator_push (acc : ReceiptAccumulator) (receipt : Receipt) (next_count : Nat) : SailM ReceiptAccumulator := do
  let cumulative : Nat := (conserved_gas_add acc.cumulative_gas_used receipt.gas_used)
  let current : PendingReceipt :=
    { index := acc.count,
      cumulative_gas_used := cumulative,
      receipt := receipt }
  let builder := acc.builder
  let first := acc.first
  let pending := acc.pending
  let (builder, first, pending) ← (( do
    if ((acc.count == 0) : Bool)
    then
      (let first : (Option PendingReceipt) := (some current)
      (pure (builder, first, pending)))
    else
      (do
        let (builder, first, pending) ← (( do
          if ((acc.count == 128) : Bool)
          then
            (do
              let builder ← (( do
                match pending with
                | .some previous =>
                  (do
                    (receipt_insert builder previous (some (← (trie_index_key 0)))))
                | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM TrieBuilder )
              let builder ← (( do
                match first with
                | .some zero =>
                  (do
                    (receipt_insert builder zero (some (← (trie_index_key acc.count)))))
                | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM TrieBuilder )
              let first : (Option PendingReceipt) := none
              let pending : (Option PendingReceipt) := (some current)
              (pure (builder, first, pending)))
          else
            (do
              let (builder, pending) ← (( do
                match pending with
                | .some previous =>
                  (do
                    let builder ←
                      (receipt_insert builder previous (some (← (trie_index_key acc.count))))
                    let pending : (Option PendingReceipt) := (some current)
                    (pure (builder, pending)))
                | none =>
                  (do
                    let pending ← (( do
                      if ((acc.count == 1) : Bool)
                      then (pure (some current))
                      else sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM
                      (Option PendingReceipt) )
                    (pure (builder, pending))) ) : SailM (TrieBuilder × (Option PendingReceipt)) )
              (pure (builder, first, pending))) ) : SailM
          (TrieBuilder × (Option PendingReceipt) × (Option PendingReceipt)) )
        (pure (builder, first, pending))) ) : SailM
    (TrieBuilder × (Option PendingReceipt) × (Option PendingReceipt)) )
  (pure { builder := builder,
          first := first,
          pending := pending,
          count := next_count,
          cumulative_gas_used := cumulative,
          bloom := ← (bloom_add_logs acc.bloom receipt.logs) })

/-- Flushes retained receipts and returns the canonical receipts root. -/
def receipt_accumulator_root (acc : ReceiptAccumulator) : SailM (Vector (BitVec 8) 32) := do
  let builder := acc.builder
  let builder ← (( do
    if ((acc.count == 0) : Bool)
    then (pure builder)
    else
      (do
        if ((acc.count == 1) : Bool)
        then
          (do
            match acc.first with
            | .some zero =>
              (do
                (receipt_insert builder zero none))
            | none => sailThrow ((InvalidBlock WitnessDeficient)))
        else
          (do
            if ((acc.count ≤b 128) : Bool)
            then
              (do
                let builder ← (( do
                  match acc.pending with
                  | .some previous =>
                    (do
                      (receipt_insert builder previous (some (← (trie_index_key 0)))))
                  | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM TrieBuilder )
                match acc.first with
                | .some zero =>
                  (do
                    (receipt_insert builder zero none))
                | none => sailThrow ((InvalidBlock WitnessDeficient)))
            else
              (do
                let builder ← (( do
                  match acc.pending with
                  | .some previous =>
                    (do
                      (receipt_insert builder previous none))
                  | none => sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM TrieBuilder )
                match acc.first with
                | .some _ => sailThrow ((InvalidBlock WitnessDeficient))
                | none => (pure ())
                (pure builder)))) ) : SailM TrieBuilder )
  (trie_builder_root builder)

