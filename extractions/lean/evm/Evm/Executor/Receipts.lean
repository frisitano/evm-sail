import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Crypto
import Evm.Primitives.Tx
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Lib.Rlp.Rlp
import Evm.Lib.Mpt.Updates
import Evm.Lib.Mpt.Indexed

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

def LOGS_BLOOM_BYTE_LENGTH : byte_length := (ByteQuantity 256)

/-- Type quantifiers: bit_to_set : Nat, 0 ≤ bit_to_set ∧ bit_to_set ≤ 63 -/
def bloom_bit_mask (bit_to_set : Nat) : (BitVec 64) :=
  let mask : limb := LIMB_ZERO
  (BitVec.update mask bit_to_set 1#1)

/-- Type quantifiers: bit_to_set : Nat, 0 ≤ bit_to_set ∧ bit_to_set ≤ 2047 -/
def bloom_set_bit (bloom : (Vector (BitVec 64) 32)) (bit_to_set : Nat) : SailM (Vector (BitVec 64) 32) := do
  let out := bloom
  let quotient ← do (exact_quotient bit_to_set 64)
  let natural_limb ← (( do
    if ((quotient ≤b 31) : Bool)
    then (pure quotient)
    else
      (do
        assert false "sail/executor/receipts.sail:15.24-15.25"
        throw Error.Exit) ) : SailM bloom_limb_index )
  let limb_index : bloom_limb_index := (31 -i natural_limb)
  let remainder := (Int.tmod bit_to_set 64)
  let bit_in_limb ← (( do
    if ((remainder ≤b 63) : Bool)
    then (pure remainder)
    else
      (do
        assert false "sail/executor/receipts.sail:23.24-23.25"
        throw Error.Exit) ) : SailM bloom_limb_bit )
  (pure (vectorUpdate out limb_index
      ((GetElem?.getElem! out limb_index) ||| (bloom_bit_mask bit_in_limb))))

def bloom_add_entry_hash (bloom : (Vector (BitVec 64) 32)) (h : (BitVec 256)) : SailM (Vector (BitVec 64) 32) := do
  let out ← do (bloom_set_bit bloom (BitVec.toNatInt (Sail.BitVec.extractLsb h 250 240)))
  let out ← (bloom_set_bit out (BitVec.toNatInt (Sail.BitVec.extractLsb h 234 224)))
  (bloom_set_bit out (BitVec.toNatInt (Sail.BitVec.extractLsb h 218 208)))

def bloom_add_topics (bloom : (Vector (BitVec 64) 32)) (topics : (List (BitVec 256))) : SailM (Vector (BitVec 64) 32) := do
  match topics with
  | [] => (pure bloom)
  | (topic :: rest) =>
    (bloom_add_topics (← (bloom_add_entry_hash bloom (← (keccak256_word topic)))) rest)

def bloom_add_log (bloom : (Vector (BitVec 64) 32)) (log : LogEntry) : SailM (Vector (BitVec 64) 32) := do
  let with_address ← do (bloom_add_entry_hash bloom (← (keccak256_address log.address)))
  (bloom_add_topics with_address log.topics)

def bloom_add_logs (bloom : (Vector (BitVec 64) 32)) (logs : (List LogEntry)) : SailM (Vector (BitVec 64) 32) := do
  match logs with
  | [] => (pure bloom)
  | (log :: rest) => (bloom_add_logs (← (bloom_add_log bloom log)) rest)

def logs_bloom_for_logs (logs : (List LogEntry)) : SailM (Vector (BitVec 64) 32) := do
  (bloom_add_logs EMPTY_LOGS_BLOOM logs)

def topics_rlp_content_size (topics : (List (BitVec 256))) : SailM byte_quantity := do
  match topics with
  | [] => (pure BYTE_ZERO)
  | (_ :: rest) => (byte_quantity_add (rlp_word_size ()) (← (topics_rlp_content_size rest)))

def topics_rlp_size (topics : (List (BitVec 256))) : SailM byte_quantity := do
  (rlp_list_size (← (topics_rlp_content_size topics)))

def log_entry_rlp_content_size (log : LogEntry) : SailM byte_quantity := do
  let content_len := (rlp_addr_size ())
  let content_len ← (byte_quantity_add content_len (← (topics_rlp_size log.topics)))
  (byte_quantity_add content_len (← (rlp_slice_size log.data)))

def log_entry_rlp_size (log : LogEntry) : SailM byte_quantity := do
  (rlp_list_size (← (log_entry_rlp_content_size log)))

def logs_rlp_content_size (logs : (List LogEntry)) : SailM byte_quantity := do
  match logs with
  | [] => (pure BYTE_ZERO)
  | (log :: rest) =>
    (byte_quantity_add (← (log_entry_rlp_size log)) (← (logs_rlp_content_size rest)))

def logs_rlp_size (logs : (List LogEntry)) : SailM byte_quantity := do
  (rlp_list_size (← (logs_rlp_content_size logs)))

def rlp_write_topics_content (topics : (List (BitVec 256))) : SailM Unit := do
  match topics with
  | [] => (pure ())
  | (topic :: rest) =>
    (do
      (rlp_write_word topic)
      (rlp_write_topics_content rest))

def rlp_write_topics (topics : (List (BitVec 256))) : SailM Unit := do
  (rlp_write_list_prefix (← (topics_rlp_content_size topics)))
  (rlp_write_topics_content topics)

def rlp_write_log_entry (log : LogEntry) : SailM Unit := do
  (rlp_write_list_prefix (← (log_entry_rlp_content_size log)))
  (rlp_write_addr log.address)
  (rlp_write_topics log.topics)
  (rlp_write_slice log.data)

def rlp_write_logs_content (logs : (List LogEntry)) : SailM Unit := do
  match logs with
  | [] => (pure ())
  | (log :: rest) =>
    (do
      (rlp_write_log_entry log)
      (rlp_write_logs_content rest))

def rlp_write_logs (logs : (List LogEntry)) : SailM Unit := do
  (rlp_write_list_prefix (← (logs_rlp_content_size logs)))
  (rlp_write_logs_content logs)

def receipt_payload_content_size (r : Receipt) (cumulative_gas_used : gas) : SailM byte_quantity := do
  let status : protocol_quantity :=
    if (r.success : Bool)
    then 1
    else 0
  let content_len ← do (rlp_protocol_quantity_size status)
  let content_len ← (byte_quantity_add content_len (← (rlp_gas_size cumulative_gas_used)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_string_size LOGS_BLOOM_BYTE_LENGTH 0x00#8)))
  (byte_quantity_add content_len (← (logs_rlp_size r.logs)))

def receipt_encoded (r : Receipt) (cumulative_gas_used : gas) : SailM EvmByteSlice := do
  let status : protocol_quantity :=
    if (r.success : Bool)
    then 1
    else 0
  let bloom ← do (pure (logs_bloom_bytes (← (logs_bloom_for_logs r.logs))))
  let content_len ← do (receipt_payload_content_size r cumulative_gas_used)
  let payload_len ← do (rlp_list_size content_len)
  let typed := ((tx_type_byte r.tx_type) != 0x00#8)
  let encoded_len ← do
    if (typed : Bool)
    then (byte_quantity_add BYTE_ONE payload_len)
    else (pure payload_len)
  let start ← do (scratch_begin ())
  if (typed : Bool)
  then (scratch_push_bytes [(tx_type_byte r.tx_type)] BYTE_ONE)
  else (pure ())
  (rlp_write_list_prefix content_len)
  (rlp_write_protocol_quantity status)
  (rlp_write_gas cumulative_gas_used)
  (rlp_write_bytes bloom LOGS_BLOOM_BYTE_LENGTH)
  (rlp_write_logs r.logs)
  (rlp_finish start encoded_len)

def receipt_accumulator_empty (_ : Unit) : ReceiptAccumulator :=
  { builder := (trie_builder_empty ())
    first := none
    pending := none
    count := 0
    cumulative_gas_used := GAS_ZERO
    bloom := EMPTY_LOGS_BLOOM }

def receipt_insert (builder : TrieBuilder) (pending : PendingReceipt) (next_key : (Option TriePath)) : SailM TrieBuilder := do
  let mark ← do (scratch_begin ())
  let value ← do (receipt_encoded pending.receipt pending.cumulative_gas_used)
  let inserted ← do
    (trie_insert_item builder (item_leaf (← (trie_index_key pending.index)) value) next_key)
  (scratch_rewind mark)
  (pure inserted)

def receipt_accumulator_push (acc : ReceiptAccumulator) (receipt : Receipt) : SailM ReceiptAccumulator := do
  if ((! receipt.valid) : Bool)
  then sailThrow ((InvalidBlock ExecutionInvalid))
  else (pure ())
  let next_count ← (( do
    if ((acc.count <b ((2 ^i 64) -i 1)) : Bool)
    then (protocol_quantity_increment acc.count)
    else sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM item_count )
  let .Gas cumulative_before := acc.cumulative_gas_used
  let .Gas receipt_gas := receipt.gas_used
  let cumulative ← (( do
    if ((receipt_gas ≤b (((2 ^i 63) -i 1) -i cumulative_before)) : Bool)
    then (pure (Gas (cumulative_before +i receipt_gas)))
    else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM gas )
  let current : PendingReceipt :=
    { index := acc.count
      cumulative_gas_used := cumulative
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
  (pure { builder := builder
          first := first
          pending := pending
          count := next_count
          cumulative_gas_used := cumulative
          bloom := ← (bloom_add_logs acc.bloom receipt.logs) })

def receipt_accumulator_root (acc : ReceiptAccumulator) : SailM (BitVec 256) := do
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

