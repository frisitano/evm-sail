import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Host.Code
import Evm.Lib.Rlp.Rlp

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

/-! # The block access list

Validation of the EIP-7928 block access list supplied by the stateless host.
The supplied bytes are decoded as canonical RLP and consumed in lockstep with
the recorder's ordered account/change tables.  Validation never reconstructs
or re-encodes the list: the original source-backed slice is also the value
hashed into the execution-payload header. -/

/-- Counts one logical BAL item and enforces `gas_limit / 2000` without a
separate sizing pass. -/
/- Type quantifiers: k_ex417741_ : Nat, k_ex417740_ : Nat, 0 ≤ k_ex417740_ ∧
  k_ex417740_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex417741_ ∧ k_ex417741_ ≤ (2 ^ 64 - 1) -/
def bal_count_item (count : Nat) (maximum : Nat) : SailM Nat := do
  if ((count <b maximum) : Bool)
  then (pure (count + 1))
  else sailThrow ((InvalidBlock BlockAccessListTooLarge))

/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_cursor (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (EvmByteSliceFields (k_source_off + (k_source_len - k_content_len)) k_content_len) := do
  if ((← if (f.is_list : Bool)
       then
         (do
           (rlp_ref_framing_canonical f))
       else (pure false)) : Bool)
  then (rlp_ref_cursor f)
  else sailThrow ((InvalidBlock InvalidBlockAccessList))

/-- Requires a canonical RLP byte string and returns its content slice. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_bytes (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  if _sailIf0 : ((← (rlp_ref_bytes_canonical f)) : Bool) = true
  then
    (pure ((rlp_ref_content f) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      sailThrow ((InvalidBlock InvalidBlockAccessList)))

/-- Requires a canonical RLP integer in the EVM-word domain. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_word (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  if (((← (rlp_ref_uint_canonical f)) && (k_content_len ≤b 32)) : Bool)
  then (rlp_ref_word f)
  else sailThrow ((InvalidBlock InvalidBlockAccessList))

/-- Requires a canonical RLP integer in the host-index/nonce domain. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
def bal_ref_uint64 (f : (RlpFieldRef k_source_off k_source_len k_content_len)) : SailM Nat := do
  if (((← (rlp_ref_uint_canonical f)) && (k_content_len ≤b 8)) : Bool)
  then (rlp_ref_uint64 f)
  else sailThrow ((InvalidBlock InvalidBlockAccessList))

/-- Requires that a decoded list has no unconsumed children. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_expect_end (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM Unit := do
  if ((k_source_len == 0) : Bool)
  then (pure ())
  else sailThrow ((InvalidBlock InvalidBlockAccessList))

/-- Compares one canonical `[index, word]` pair. -/
/- Type quantifiers: k_ex417843_ : Nat, k_ex417842_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex417842_ ∧
  k_ex417842_ ≤ (2 ^ 20 + 1), 0 ≤ k_ex417843_ ∧ k_ex417843_ ≤ (2 ^ 256 - 1) -/
def bal_compare_index_word (pair : (RlpFieldRef k_source_off k_source_len k_content_len)) (index : Nat) (value : Nat) : SailM Unit := do
  let fields ← do (bal_ref_cursor pair)
  let ⟨index_field_syn_content_len, ⟨index_field_full_len, (index_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨value_field_syn_content_len, ⟨value_field_full_len, (value_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  (bal_expect_end fields)
  if ((← if (((← (bal_ref_uint64 index_field)) != index) : Bool)
       then (pure true)
       else
         (do
           (pure ((← (bal_ref_word value_field)) != value)))) : Bool)
  then sailThrow ((InvalidBlock InvalidBlockAccessList))
  else (pure ())

/-- Compares one canonical `[index, nonce]` pair. -/
/- Type quantifiers: k_ex417863_ : Nat, k_ex417862_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex417862_ ∧
  k_ex417862_ ≤ (2 ^ 20 + 1), 0 ≤ k_ex417863_ ∧ k_ex417863_ ≤ (2 ^ 64 - 1) -/
def bal_compare_index_nonce (pair : (RlpFieldRef k_source_off k_source_len k_content_len)) (index : Nat) (value : Nat) : SailM Unit := do
  let fields ← do (bal_ref_cursor pair)
  let ⟨index_field_syn_content_len, ⟨index_field_full_len, (index_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨value_field_syn_content_len, ⟨value_field_full_len, (value_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  (bal_expect_end fields)
  if ((← if (((← (bal_ref_uint64 index_field)) != index) : Bool)
       then (pure true)
       else
         (do
           (pure ((← (bal_ref_uint64 value_field)) != value)))) : Bool)
  then sailThrow ((InvalidBlock InvalidBlockAccessList))
  else (pure ())

/-- Compares one canonical `[index, code]` pair without materializing either
source-backed code sequence. -/
/- Type quantifiers: k_ex417882_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0
  ≤ k_ex417882_ ∧ k_ex417882_ ≤ (2 ^ 20 + 1) -/
def bal_compare_index_code (pair : (RlpFieldRef k_source_off k_source_len k_content_len)) (index : Nat) (code_hash : (Vector (BitVec 8) 32)) : SailM Unit := do
  let fields ← do (bal_ref_cursor pair)
  let ⟨index_field_syn_content_len, ⟨index_field_full_len, (index_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨code_field_syn_content_len, ⟨code_field_full_len, (code_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  (bal_expect_end fields)
  let code ← do (code_db_resolve code_hash)
  if ((← if (((← (bal_ref_uint64 index_field)) != index) : Bool)
       then (pure true)
       else
         (do
           (pure (! (← do
                   let dependentArg0 := (← (bal_ref_bytes code_field))
                   (byte_slices_equal dependentArg0 code.bytes)))))) : Bool)
  then sailThrow ((InvalidBlock InvalidBlockAccessList))
  else (pure ())

/-- Consumes an equal-index run within the selected storage slot and returns
its final row plus the first row of the following run. -/
def bal_storage_change_run (first : BalStorageChangeEntry) : SailM (BalStorageChangeEntry × (Option BalStorageChangeEntry)) := do
  let last := first
  let next ← do (bal_storage_change_next ())
  let scanning : Bool := true
  let (last, next, scanning) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (last, next, scanning) => (pure scanning)) (last, next, scanning)
      fun (last, next, scanning) => do
        assert true "loop dummy assert"
        let (last, next, scanning) ← (( do
          match next with
          | .some entry =>
            (do
              let (last, next, scanning) ← (( do
                if ((entry.index == first.index) : Bool)
                then
                  (do
                    let last : BalStorageChangeEntry := entry
                    let next ← (bal_storage_change_next ())
                    (pure (last, next, scanning)))
                else
                  (let scanning : Bool := false
                  (pure (last, next, scanning))) ) : SailM
                (BalStorageChangeEntry × (Option BalStorageChangeEntry) × Bool) )
              (pure (last, next, scanning)))
          | none =>
            (let scanning : Bool := false
            (pure (last, next, scanning))) ) : SailM
          (BalStorageChangeEntry × (Option BalStorageChangeEntry) × Bool) )
        (pure (last, next, scanning))
    (pure loop_vars) ) : SailM (BalStorageChangeEntry × (Option BalStorageChangeEntry) × Bool) )
  (pure (last, next))

/-- Compares every change-index run belonging to the selected storage slot. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_compare_storage_slot_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalStorageChangeEntry)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match current with
      | .some first =>
        (do
          let (last, next) ← do (bal_storage_change_run first)
          let ⟨pair_content_len, ⟨pair_full_len, (pair, remaining)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (bal_compare_index_word pair first.index last.value)
          (_rec_bal_compare_storage_slot_changes remaining next _reclimit_pred))
      | none => (bal_expect_end fields))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Compares every change-index run belonging to the selected storage slot. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_compare_storage_slot_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalStorageChangeEntry)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_compare_storage_slot_changes fields current (_measure + 1))

/-- Consumes the selected account's sorted storage-slot union. The host cursor
drives one loop iteration per logical slot. A slot with one or more
changes advances `storage_changes`; a change-free slot advances
`storage_reads`. -/
/- Type quantifiers: k_ex417932_ : Nat, k_ex417931_ : Nat, k_changes_source_off : Nat, k_changes_source_len
  : Nat, k_reads_source_off : Nat, k_reads_source_len : Nat, (source_valid_range k_changes_source_off k_changes_source_len)
  ∧ (source_valid_range k_reads_source_off k_reads_source_len), 0 ≤ k_ex417931_ ∧
  k_ex417931_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex417932_ ∧ k_ex417932_ ≤ (2 ^ 64 - 1) -/
def bal_compare_storage_slots (initial_changes : (EvmByteSliceFields k_changes_source_off k_changes_source_len)) (initial_reads : (EvmByteSliceFields k_reads_source_off k_reads_source_len)) (item_count : Nat) (maximum_items : Nat) : SailM Nat := do
  let changes : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
    ((⟨_, ⟨_, initial_changes⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
  let reads : (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
    ((⟨_, ⟨_, initial_reads⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
  let count : Nat := item_count
  let current ← do (bal_storage_slot_next ())
  let scanning : Bool := true
  let (changes, count, current, reads, scanning) ← (( do
    let loop_vars ← whileFuelM (fuel :=((byte_slice_length changes) + (byte_slice_length reads))) (fun (changes, count, current, reads, scanning) => (pure scanning)) (changes, count, current, reads, scanning)
      fun (changes, count, current, reads, scanning) => do
        assert true "loop dummy assert"
        let (changes, count, current, reads, scanning) ← (( do
          match current with
          | .some slot_entry =>
            (do
              let (changes, reads) ← (( do
                match slot_entry.change with
                | .some first_change =>
                  (do
                    let ⟨slot_field_content_len, ⟨slot_field_full_len, (slot_field, remaining_changes)⟩⟩ ← do
                      (rlp_cursor_pop ((changes).2).2)
                    let changes : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
                      ((⟨_, ⟨_, remaining_changes⟩⟩ : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
                    let slot_fields ← do (bal_ref_cursor slot_field)
                    let ⟨slot_value_content_len, ⟨slot_value_full_len, (slot_value, slot_fields)⟩⟩ ← do
                      (rlp_cursor_pop slot_fields)
                    let ⟨slot_changes_content_len, ⟨slot_changes_full_len, (slot_changes, slot_fields)⟩⟩ ← do
                      (rlp_cursor_pop slot_fields)
                    (bal_expect_end slot_fields)
                    if _sailIf0 : (((← (bal_ref_word slot_value)) != slot_entry.slot) : Bool) = true
                    then
                      (do
                        sailThrow ((InvalidBlock InvalidBlockAccessList)))
                    else
                      (do
                        (bal_compare_storage_slot_changes (← (bal_ref_cursor slot_changes))
                          (some first_change))
                        (pure ((changes, reads) : ((Sigma fun (k_off : Nat) =>
                          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × (Sigma fun
                          (k_off : Nat) =>
                          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))))
                | none =>
                  (do
                    let ⟨slot_field_content_len, ⟨slot_field_full_len, (slot_field, remaining_reads)⟩⟩ ← do
                      (rlp_cursor_pop ((reads).2).2)
                    let reads : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
                      ((⟨_, ⟨_, remaining_reads⟩⟩ : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
                    if _sailIf0 : (((← (bal_ref_word slot_field)) != slot_entry.slot) : Bool) = true
                    then
                      (do
                        sailThrow ((InvalidBlock InvalidBlockAccessList)))
                    else
                      (pure ((changes, reads) : ((Sigma fun (k_off : Nat) =>
                        (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × (Sigma fun
                        (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))
                ) : SailM
                ((Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × (Sigma fun
                (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) )
              let count ← (bal_count_item count maximum_items)
              let current ← (bal_storage_slot_next ())
              (pure ((changes, count, current, reads, scanning) : ((Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Nat × (Option BalStorageSlotEntry) × (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Bool))))
          | none =>
            (let scanning : Bool := false
            (pure ((changes, count, current, reads, scanning) : ((Sigma fun (k_off : Nat) =>
              (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Nat × (Option BalStorageSlotEntry) × (Sigma
              fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Bool))))
          ) : SailM
          ((Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Nat × (Option BalStorageSlotEntry) × (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Bool)
          )
        (pure ((changes, count, current, reads, scanning) : ((Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Nat × (Option BalStorageSlotEntry) × (Sigma
          fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Bool)))
    (pure loop_vars) ) : SailM
    ((Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Nat × (Option BalStorageSlotEntry) × (Sigma
    fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × Bool) )
  (rlp_cursor_expect_end ((changes).2).2)
  (rlp_cursor_expect_end ((reads).2).2)
  (pure count)

/-- Consumes an equal-index balance run and returns its final row plus the
first row of the following run. -/
def bal_balance_change_run (first : BalBalanceChangeEntry) : SailM (BalBalanceChangeEntry × (Option BalBalanceChangeEntry)) := do
  let last := first
  let next ← do (bal_balance_change_next ())
  let scanning : Bool := true
  let (last, next, scanning) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (last, next, scanning) => (pure scanning)) (last, next, scanning)
      fun (last, next, scanning) => do
        assert true "loop dummy assert"
        let (last, next, scanning) ← (( do
          match next with
          | .some entry =>
            (do
              let (last, next, scanning) ← (( do
                if ((entry.index == first.index) : Bool)
                then
                  (do
                    let last : BalBalanceChangeEntry := entry
                    let next ← (bal_balance_change_next ())
                    (pure (last, next, scanning)))
                else
                  (let scanning : Bool := false
                  (pure (last, next, scanning))) ) : SailM
                (BalBalanceChangeEntry × (Option BalBalanceChangeEntry) × Bool) )
              (pure (last, next, scanning)))
          | none =>
            (let scanning : Bool := false
            (pure (last, next, scanning))) ) : SailM
          (BalBalanceChangeEntry × (Option BalBalanceChangeEntry) × Bool) )
        (pure (last, next, scanning))
    (pure loop_vars) ) : SailM (BalBalanceChangeEntry × (Option BalBalanceChangeEntry) × Bool) )
  (pure (last, next))

/-- Compares the final balance value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_compare_balance_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalBalanceChangeEntry)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match current with
      | .some first =>
        (do
          let (last, next) ← do (bal_balance_change_run first)
          let ⟨pair_content_len, ⟨pair_full_len, (pair, remaining)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (bal_compare_index_word pair first.index last.value)
          (_rec_bal_compare_balance_changes remaining next _reclimit_pred))
      | none => (bal_expect_end fields))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Compares the final balance value for each change index. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_compare_balance_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalBalanceChangeEntry)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_compare_balance_changes fields current (_measure + 1))

/-- Consumes an equal-index nonce run and returns its maximum value plus the
first row of the following run. -/
def bal_nonce_change_run (first : BalNonceChangeEntry) : SailM (Nat × (Option BalNonceChangeEntry)) := do
  let maximum := first.value
  let next ← do (bal_nonce_change_next ())
  let scanning : Bool := true
  let (maximum, next, scanning) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (maximum, next, scanning) => (pure scanning)) (maximum, next, scanning)
      fun (maximum, next, scanning) => do
        assert true "loop dummy assert"
        let (maximum, next, scanning) ← (( do
          match next with
          | .some entry =>
            (do
              let (maximum, next, scanning) ← (( do
                if ((entry.index == first.index) : Bool)
                then
                  (do
                    let maximum : Nat :=
                      if ((maximum <b entry.value) : Bool)
                      then entry.value
                      else maximum
                    let next ← (bal_nonce_change_next ())
                    (pure (maximum, next, scanning)))
                else
                  (let scanning : Bool := false
                  (pure (maximum, next, scanning))) ) : SailM
                (Nat × (Option BalNonceChangeEntry) × Bool) )
              (pure (maximum, next, scanning)))
          | none =>
            (let scanning : Bool := false
            (pure (maximum, next, scanning))) ) : SailM
          (Nat × (Option BalNonceChangeEntry) × Bool) )
        (pure (maximum, next, scanning))
    (pure loop_vars) ) : SailM (Nat × (Option BalNonceChangeEntry) × Bool) )
  (pure (maximum, next))

/-- Compares the maximum nonce value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_compare_nonce_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalNonceChangeEntry)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match current with
      | .some first =>
        (do
          let (maximum, next) ← do (bal_nonce_change_run first)
          let ⟨pair_content_len, ⟨pair_full_len, (pair, remaining)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (bal_compare_index_nonce pair first.index maximum)
          (_rec_bal_compare_nonce_changes remaining next _reclimit_pred))
      | none => (bal_expect_end fields))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Compares the maximum nonce value for each change index. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_compare_nonce_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalNonceChangeEntry)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_compare_nonce_changes fields current (_measure + 1))

/-- Consumes an equal-index code run and returns its final row plus the first
row of the following run. -/
def bal_code_change_run (first : BalCodeChangeEntry) : SailM (BalCodeChangeEntry × (Option BalCodeChangeEntry)) := do
  let last := first
  let next ← do (bal_code_change_next ())
  let scanning : Bool := true
  let (last, next, scanning) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (last, next, scanning) => (pure scanning)) (last, next, scanning)
      fun (last, next, scanning) => do
        assert true "loop dummy assert"
        let (last, next, scanning) ← (( do
          match next with
          | .some entry =>
            (do
              let (last, next, scanning) ← (( do
                if ((entry.index == first.index) : Bool)
                then
                  (do
                    let last : BalCodeChangeEntry := entry
                    let next ← (bal_code_change_next ())
                    (pure (last, next, scanning)))
                else
                  (let scanning : Bool := false
                  (pure (last, next, scanning))) ) : SailM
                (BalCodeChangeEntry × (Option BalCodeChangeEntry) × Bool) )
              (pure (last, next, scanning)))
          | none =>
            (let scanning : Bool := false
            (pure (last, next, scanning))) ) : SailM
          (BalCodeChangeEntry × (Option BalCodeChangeEntry) × Bool) )
        (pure (last, next, scanning))
    (pure loop_vars) ) : SailM (BalCodeChangeEntry × (Option BalCodeChangeEntry) × Bool) )
  (pure (last, next))

/-- Compares the final code value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_compare_code_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalCodeChangeEntry)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match current with
      | .some first =>
        (do
          let (last, next) ← do (bal_code_change_run first)
          let ⟨pair_content_len, ⟨pair_full_len, (pair, remaining)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (bal_compare_index_code pair first.index last.code_hash)
          (_rec_bal_compare_code_changes remaining next _reclimit_pred))
      | none => (bal_expect_end fields))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Compares the final code value for each change index. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_compare_code_changes (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option BalCodeChangeEntry)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_compare_code_changes fields current (_measure + 1))

/-- Compares one account and all five ordered BAL child collections. -/
/- Type quantifiers: k_ex418032_ : Nat, k_ex418031_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex418031_ ∧
  k_ex418031_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex418032_ ∧ k_ex418032_ ≤ (2 ^ 64 - 1) -/
def bal_compare_account (field : (RlpFieldRef k_source_off k_source_len k_content_len)) (account : (Vector (BitVec 8) 20)) (item_count : Nat) (maximum_items : Nat) : SailM Nat := do
  let fields ← do (bal_ref_cursor field)
  let ⟨address_field_syn_content_len, ⟨address_field_full_len, (address_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨storage_changes_field_syn_content_len, ⟨storage_changes_field_full_len, (storage_changes_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨storage_reads_field_syn_content_len, ⟨storage_reads_field_full_len, (storage_reads_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨balance_changes_field_syn_content_len, ⟨balance_changes_field_full_len, (balance_changes_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨nonce_changes_field_syn_content_len, ⟨nonce_changes_field_full_len, (nonce_changes_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  let ⟨code_changes_field_syn_content_len, ⟨code_changes_field_full_len, (code_changes_field, fields)⟩⟩ ← do
    (rlp_cursor_pop fields)
  (bal_expect_end fields)
  let ⟨_, ⟨_, address_bytes⟩⟩ ← do (bal_ref_bytes address_field)
  if ((← if ((address_bytes.len != 20) : Bool)
       then (pure true)
       else
         (do
           (pure (bne (word_to_address (← (rlp_ref_word address_field))) account)))) : Bool)
  then sailThrow ((InvalidBlock InvalidBlockAccessList))
  else
    (do
      let counted_account ← do (bal_count_item item_count maximum_items)
      let counted_storage ← do
        (bal_compare_storage_slots (← (bal_ref_cursor storage_changes_field))
          (← (bal_ref_cursor storage_reads_field)) counted_account maximum_items)
      (bal_compare_balance_changes (← (bal_ref_cursor balance_changes_field))
        (← (bal_balance_change_next ())))
      (bal_compare_nonce_changes (← (bal_ref_cursor nonce_changes_field))
        (← (bal_nonce_change_next ())))
      (bal_compare_code_changes (← (bal_ref_cursor code_changes_field))
        (← (bal_code_change_next ())))
      (pure counted_storage))

/-- Compares all BAL accounts in canonical address order. -/
/- Type quantifiers: _reclimit : Nat, k_ex418048_ : Nat, k_ex418047_ : Nat, k_source_off : Nat, k_source_len
  : Nat, (source_valid_range k_source_off k_source_len), 0 ≤ k_ex418047_ ∧
  k_ex418047_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex418048_ ∧ k_ex418048_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_compare_accounts (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option (Vector (BitVec 8) 20))) (item_count : Nat) (maximum_items : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match current with
      | .some account =>
        (do
          let ⟨field_content_len, ⟨field_full_len, (field, remaining)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let next_count ← do (bal_compare_account field account item_count maximum_items)
          (_rec_bal_compare_accounts remaining (← (bal_account_next ())) next_count maximum_items
            _reclimit_pred))
      | none =>
        (do
          (bal_expect_end fields)
          (pure item_count)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Compares all BAL accounts in canonical address order. -/
/- Type quantifiers: maximum_items : Nat, item_count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ item_count ∧ item_count ≤ (2 ^ 64 - 1), 0 ≤ maximum_items ∧
  maximum_items ≤ (2 ^ 64 - 1) -/
def bal_compare_accounts (fields : (EvmByteSliceFields k_source_off k_source_len)) (current : (Option (Vector (BitVec 8) 20))) (item_count : Nat) (maximum_items : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_compare_accounts fields current item_count maximum_items (_measure + 1))

/-- Validates the supplied canonical EIP-7928 block access list directly
against the recorder's ordered account/change streams. -/
/- Type quantifiers: k_ex418075_ : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0
  ≤ bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1, 0 ≤ k_ex418075_ ∧
  k_ex418075_ ≤ (2 ^ 64 - 1) -/
def validate_block_access_list (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (maximum_items : Nat) : SailM Unit := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  (bal_prepare ())
  let ⟨_, root⟩ ← do (rlp_single_ref bytes)
  let _ ← do
    (bal_compare_accounts (← (bal_ref_cursor root)) (← (bal_account_next ())) 0 maximum_items)
  (pure ())

