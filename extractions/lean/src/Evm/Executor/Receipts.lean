import Evm.Flow
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Kernel.Scratch
import Evm.Primitives.Block
import Evm.Lib.Ssz.Ssz
import Evm.Kernel.Storage
import Evm.Kernel.Logs
import Evm.Lib.Mpt.Primitives
import Evm.Lib.Mpt.Updates
import Evm.Lib.Mpt.Indexed
import Evm.Lib.Ssz.StatelessInput
import Evm.Lib.Rlp.Codecs.Withdrawals
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

/-! # Receipt retention and the receipts trie

Execution-ordered retention of encoded receipts and the post-execution
block-wide receipt reductions. Canonical wire encoding is owned by the
receipt RLP codec. -/

/-- Opens the execution-ordered receipt record region for one block. -/
def receipt_store_begin (_ : Unit) : SailM Nat := do
  (scratch_begin ())

/-- Splits the first retained record from an execution-ordered record span. -/
/- Type quantifiers: records_dependentWitness1 : Nat, records_dependentWitness0 : Nat, 0 ≤
  records_dependentWitness0 ∧
  0 ≤ records_dependentWitness1 ∧
  (records_dependentWitness0 + records_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def receipt_record_pop (records : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM ((Sigma fun
  (records_dependentWitness0 : Nat) =>
  (Sigma fun (records_dependentWitness1 : Nat) =>
  (ScratchSliceFields records_dependentWitness0 records_dependentWitness1))) × (Sigma fun
  (records_dependentWitness0 : Nat) =>
  (Sigma fun (records_dependentWitness1 : Nat) =>
  (ScratchSliceFields records_dependentWitness0 records_dependentWitness1)))) := do
  let records_dependentWitness0 := (records).1
  let records_dependentWitness1 := ((records).2).1
  let records := ((records).2).2
  let ⟨_, ⟨_, records⟩⟩ ← (( do
    if _sailIf0 : ((EIGHT_BYTE_LENGTH ≤b records.len) : Bool) = true
    then
      (pure ((⟨_, ⟨_, records⟩⟩ : (Sigma fun (records_dependentWitness0 : Nat) =>
        (Sigma fun (records_dependentWitness1 : Nat) =>
        (ScratchSliceFields records_dependentWitness0 records_dependentWitness1)))) : (Sigma fun
        (records_dependentWitness0 : Nat) =>
        (Sigma fun (records_dependentWitness1 : Nat) =>
        (ScratchSliceFields records_dependentWitness0 records_dependentWitness1)))))
    else
      (do
        (fatal_error WitnessDeficient)) ) : SailM
    (Sigma fun (records_dependentWitness0 : Nat) =>
    (Sigma fun (records_dependentWitness1 : Nat) =>
    (ScratchSliceFields records_dependentWitness0 records_dependentWitness1))) )
  let value_length ← do (decode_scratch_uint ⟨_, ⟨_, records⟩⟩ 0)
  let payload := (scratch_slice_suffix records EIGHT_BYTE_LENGTH)
  if _sailIf0 : ((value_length ≤b payload.len) : Bool) = true
  then
    (pure (((⟨_, ⟨_, (scratch_sub_slice payload 0 value_length)⟩⟩ : (Sigma fun
      (records_dependentWitness0 : Nat) =>
      (Sigma fun (records_dependentWitness1 : Nat) =>
      (ScratchSliceFields records_dependentWitness0 records_dependentWitness1)))) : (Sigma fun
      (records_dependentWitness0 : Nat) =>
      (Sigma fun (records_dependentWitness1 : Nat) =>
      (ScratchSliceFields records_dependentWitness0 records_dependentWitness1)))), ((⟨_, ⟨_, (scratch_slice_suffix
        payload value_length)⟩⟩ : (Sigma fun (records_dependentWitness0 : Nat) =>
      (Sigma fun (records_dependentWitness1 : Nat) =>
      (ScratchSliceFields records_dependentWitness0 records_dependentWitness1)))) : (Sigma fun
      (records_dependentWitness0 : Nat) =>
      (Sigma fun (records_dependentWitness1 : Nat) =>
      (ScratchSliceFields records_dependentWitness0 records_dependentWitness1))))))
  else
    (do
      (fatal_error WitnessDeficient))

/- Type quantifiers: _index : Nat, receipt_dependentWitness4 : Nat, receipt_dependentWitness3 : Nat, receipt_dependentWitness2
  : Nat, receipt_dependentWitness1 : Nat, receipt_dependentWitness0 : Nat, cumulative_gas_used : Nat, 0
  ≤ cumulative_gas_used ∧ cumulative_gas_used ≤ (2 * block_gas_limit_bound), 0 ≤
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
  receipt_dependentWitness2 ≤ (receipt_dependentWitness1 + receipt_dependentWitness0), 0 ≤
  _index ∧ _index ≤ (2 ^ 20) -/
def receipt_store_append (receipt : (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))) (cumulative_gas_used : Nat) (_index : Nat) : SailM Unit := do
  let receipt_dependentWitness0 := (receipt).1
  let receipt_dependentWitness1 := ((receipt).2).1
  let receipt_dependentWitness2 := (((receipt).2).2).1
  let receipt_dependentWitness3 := ((((receipt).2).2).2).1
  let receipt_dependentWitness4 := (((((receipt).2).2).2).2).1
  let receipt := (((((receipt).2).2).2).2).2
  (receipt_record_append ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, receipt⟩⟩⟩⟩⟩ cumulative_gas_used)

/-- Opens the receipt record span while giving non-receipt sources empty
placeholders. The explicit result type keeps both existential slices
independent. -/
def indexed_receipt_parts (source : IndexedTrieSource) : SailM ((Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) × (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) := do
  match source with
  | .IndexedReceipts receipts =>
    (do
      if _sailIf0 : ((receipts.count == 0) : Bool) = true
      then
        (pure (((⟨_, ⟨_, EMPTY_SCRATCH_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat)
          => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))), (receipts.bytes : (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))))
      else
        (do
          (receipt_record_pop receipts.bytes)))
  | _ =>
    (pure (((⟨_, ⟨_, EMPTY_SCRATCH_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))), ((⟨_, ⟨_, EMPTY_SCRATCH_SLICE⟩⟩ : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))))

/-- Opens one source at its first canonical trie key. -/
def indexed_trie_begin (source : IndexedTrieSource) : SailM IndexedTrieCursor := do
  let count : Nat :=
    match source with
    | .IndexedTransactions txs => txs.count
    | .IndexedWithdrawals wds => wds.count
    | .IndexedReceipts receipts => receipts.count
  let (zero, remaining) ← do (indexed_receipt_parts source)
  (pure { keys := ← (rlp_index_cursor (k_maximum := (2 ^ 20)) count),
          receipt_zero := zero,
          receipt_remaining := remaining })

/-- Whether the source has no canonical keys left. -/
def indexed_trie_cursor_empty (cursor : IndexedTrieCursor) : Bool :=
  (rlp_index_cursor_empty cursor.keys)

/-- The next canonical key without consuming its value. -/
def indexed_trie_cursor_key (cursor : IndexedTrieCursor) : SailM TriePath := do
  (pure (← (rlp_index_cursor_peek cursor.keys)).key)

/-- Whether the next indexed key lies beneath `evm_prefix`. -/
def indexed_trie_next_under (cursor : IndexedTrieCursor) (evm_prefix' : TriePath) : SailM Bool := do
  let cursor_empty := (indexed_trie_cursor_empty cursor)
  if (cursor_empty : Bool)
  then (pure false)
  else
    (do
      let key ← do (indexed_trie_cursor_key cursor)
      (path_prefix_of evm_prefix' key))

/-- Consumes one canonical indexed key and resolves its source-backed value. -/
def indexed_trie_pop (source : IndexedTrieSource) (cursor : IndexedTrieCursor) : SailM (TrieItem × IndexedTrieCursor) := do
  let (index_item, next_keys) ← do (rlp_index_cursor_pop cursor.keys)
  let ⟨_, ⟨_, remaining⟩⟩ := cursor.receipt_remaining
  let (value, remaining) ← (( do
    match source with
    | .IndexedTransactions txs =>
      (do
        let ⟨_, ⟨_, transaction⟩⟩ ← do (ssz_list_at txs index_item.index)
        (pure (((InputTrieLeaf ⟨_, ⟨_, transaction⟩⟩) : TrieLeafValue), ((⟨_, ⟨_, remaining⟩⟩ : (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))))))
    | .IndexedWithdrawals wds =>
      (do
        let ⟨_, ⟨_, withdrawal⟩⟩ ← do (ssz_fixed_list_at wds index_item.index WD_SIZE)
        let ⟨_, ⟨_, encoded_withdrawal⟩⟩ ← do
          (withdrawal_rlp ⟨_, ⟨_, withdrawal⟩⟩)
        (pure (((ScratchTrieLeaf ⟨_, ⟨_, encoded_withdrawal⟩⟩) : TrieLeafValue), ((⟨_, ⟨_, remaining⟩⟩ : (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))))))
    | .IndexedReceipts _ =>
      (do
        let (t__95, remaining) ← (( do
          if _sailIf0 : ((index_item.index == 0) : Bool) = true
          then
            (pure (((ScratchTrieLeaf cursor.receipt_zero) : TrieLeafValue), ((⟨_, ⟨_, remaining⟩⟩ : (Sigma
              fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma
              fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))))
          else
            (do
              let (receipt, rest) ← do (receipt_record_pop ⟨_, ⟨_, remaining⟩⟩)
              let ⟨_, ⟨_, remaining⟩⟩ : (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) :=
                (rest : (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))
              (pure (((ScratchTrieLeaf receipt) : TrieLeafValue), ((⟨_, ⟨_, remaining⟩⟩ : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))))))
          ) : SailM
          (TrieLeafValue × (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) )
        (pure ((t__95 : TrieLeafValue), (remaining : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))))) ) : SailM
    (TrieLeafValue × (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) )
  (pure ((trie_leaf index_item.key value), { keys := next_keys,
                                             receipt_zero := cursor.receipt_zero,
                                             receipt_remaining := remaining }))

/-- Recursively assembles the indexed leaves beneath `evm_prefix`. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_indexed_trie_subtree (source : IndexedTrieSource) (cursor : IndexedTrieCursor) (evm_prefix' : TriePath) (_reclimit : Nat) : SailM (TrieItem × IndexedTrieCursor) := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let cursor_under_prefix ← do (indexed_trie_next_under cursor evm_prefix')
      let outside_prefix := (! cursor_under_prefix)
      if (outside_prefix : Bool)
      then (pure ((trie_empty_subtree ()), cursor))
      else
        (do
          let key ← do (indexed_trie_cursor_key cursor)
          let key_at_prefix := (path_eq key evm_prefix')
          if (key_at_prefix : Bool)
          then (indexed_trie_pop source cursor)
          else
            (do
              let depth := (path_len evm_prefix')
              if ((64 ≤b depth) : Bool)
              then (fatal_error WitnessDeficient)
              else (pure ())
              let children := (trie_children_empty ())
              let remaining := cursor
              let remaining_under_prefix ← (( do (indexed_trie_next_under remaining evm_prefix') ) :
                SailM Bool )
              let (children, remaining, remaining_under_prefix) ← (( do
                let loop_vars ← whileFuelM (fuel :=(remaining.keys.count -i remaining.keys.position)) (fun (children, remaining, remaining_under_prefix) => (pure remaining_under_prefix)) (children, remaining, remaining_under_prefix)
                  fun (children, remaining, remaining_under_prefix) => do
                    assert true "loop dummy assert"
                    let next_key ← do (indexed_trie_cursor_key remaining)
                    let nib ← do (path_nibble next_key depth)
                    let nibble_path ← do (path_single nib)
                    let child_prefix ← do (path_concat evm_prefix' nibble_path)
                    let (child, next) ← do
                      (_rec_indexed_trie_subtree source remaining child_prefix _reclimit_pred)
                    let children ← (trie_children_add children evm_prefix' nib child)
                    let remaining : IndexedTrieCursor := next
                    let remaining_under_prefix ← (indexed_trie_next_under remaining evm_prefix')
                    (pure (children, remaining, remaining_under_prefix))
                (pure loop_vars) ) : SailM (TrieChildren × IndexedTrieCursor × Bool) )
              (pure ((← (trie_children_finish evm_prefix' children)), remaining)))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Recursively assembles the indexed leaves beneath `evm_prefix`. -/
def indexed_trie_subtree (source : IndexedTrieSource) (cursor : IndexedTrieCursor) (evm_prefix' : TriePath) : SailM (TrieItem × IndexedTrieCursor) := do
  let _measure := ((64 - (path_len evm_prefix')) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_indexed_trie_subtree source cursor evm_prefix' (_measure + 1))

/-- Computes one complete index-keyed trie. Temporary withdrawal encodings
and node encodings share one scratch suffix, released after the root has
absorbed every leaf. Receipt records beneath the mark remain live until
their accumulator releases them. -/
def indexed_trie_root (source : IndexedTrieSource) : SailM (Vector (BitVec 8) 32) := do
  let mark ← do (scratch_begin ())
  let initial ← do (indexed_trie_begin source)
  let root_path := (path_empty ())
  let (subtree, remaining) ← do (indexed_trie_subtree source initial root_path)
  let ⟨_, ⟨_, retained⟩⟩ := remaining.receipt_remaining
  let cursor_empty := (indexed_trie_cursor_empty remaining)
  let cursor_not_empty := (! cursor_empty)
  if ((cursor_not_empty || (retained.len != 0)) : Bool)
  then (fatal_error WitnessDeficient)
  else (pure ())
  let root ← do (trie_subtree_root subtree)
  (scratch_rewind mark)
  (pure root)

/-- The receipts-trie root over execution-ordered retained records. -/
def indexed_receipt_trie_root (receipts : ReceiptRecordsRef) : SailM (Vector (BitVec 8) 32) := do
  let source := (IndexedReceipts receipts)
  (indexed_trie_root source)

/-- Computes the canonical receipts root over the retained records after the
last transaction has executed, then releases the retained region. -/
/- Type quantifiers: k_ex554416_ : Nat, k_ex554415_ : Nat, 0 ≤ k_ex554415_ ∧
  k_ex554415_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex554416_ ∧ k_ex554416_ ≤ (2 ^ 20) -/
def receipt_store_root (records_start : Nat) (count : Nat) : SailM (Vector (BitVec 8) 32) := do
  let ⟨_, ⟨_, records⟩⟩ ← do (scratch_finish records_start)
  let root ← do
    (indexed_receipt_trie_root
      { bytes := ⟨_, ⟨_, records⟩⟩,
        count := count })
  (scratch_rewind records_start)
  (pure root)

/-- Compares the block logs bloom of one consecutive retained log range with
the payload-header commitment (YP §4.4.1). The range covers exactly the
logs retained by the block's transaction receipts. -/
/- Type quantifiers: reference_dependentWitness1 : Nat, reference_dependentWitness0 : Nat, 0 ≤
  reference_dependentWitness0 ∧
  0 ≤ reference_dependentWitness1 ∧
  (reference_dependentWitness0 + reference_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  reference_dependentWitness1 = 256 -/
def block_logs_bloom_matches (logs : LogSeriesRef) (reference : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Bool := do
  let reference_dependentWitness0 := (reference).1
  let reference_dependentWitness1 := ((reference).2).1
  let reference := ((reference).2).2
  let logs_bloom ← do (bloom_add_logs EMPTY_LOGS_BLOOM logs)
  (logs_bloom_matches_ref logs_bloom ⟨_, ⟨_, reference⟩⟩)

