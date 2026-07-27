import Evm.Flow
import Evm.Prelude
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
open BalIterEntry

/-! # The block access list

Validation of the EIP-7928 block access list supplied by the stateless host.
The supplied bytes are decoded as canonical RLP and consumed in lockstep with
the recorder's ordered account/change tables.  Validation never reconstructs
or re-encodes the list: the original source-backed slice is also the value
hashed into the execution-payload header. -/

/-- Counts one logical BAL item and enforces `gas_limit / 2000` without a
separate sizing pass. -/
/- Type quantifiers: k_ex416285_ : Nat, k_ex416284_ : Nat, 0 ≤ k_ex416284_ ∧
  k_ex416284_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex416285_ ∧ k_ex416285_ ≤ (2 ^ 64 - 1) -/
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
/- Type quantifiers: k_ex416387_ : Nat, k_ex416386_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex416386_ ∧
  k_ex416386_ ≤ (2 ^ 20 + 1), 0 ≤ k_ex416387_ ∧ k_ex416387_ ≤ (2 ^ 256 - 1) -/
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
/- Type quantifiers: k_ex416407_ : Nat, k_ex416406_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex416406_ ∧
  k_ex416406_ ≤ (2 ^ 20 + 1), 0 ≤ k_ex416407_ ∧ k_ex416407_ ≤ (2 ^ 64 - 1) -/
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
/- Type quantifiers: k_ex416426_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0
  ≤ k_ex416426_ ∧ k_ex416426_ ≤ (2 ^ 20 + 1) -/
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

/-- Consumes the encoded changes for one storage slot.  Each RLP pop strictly
reduces the remaining byte length and must have one matching host event. -/
/- Type quantifiers: _reclimit : Nat, k_ex416441_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ k_ex416441_ ∧ k_ex416441_ ≤ (2 ^ 256 - 1), 0 ≤ _reclimit -/
def _rec_bal_validate_storage_change_values (cursor : (EvmByteSliceFields k_source_off k_source_len)) (slot : Nat) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure ())
      else
        (do
          let ⟨pair_content_len, ⟨pair_full_len, (pair, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          match (← (bal_iter_next ())) with
          | .BalStorageChange change =>
            (do
              if ((change.slot != slot) : Bool)
              then sailThrow ((InvalidBlock InvalidBlockAccessList))
              else (bal_compare_index_word pair change.index change.value))
          | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
          (_rec_bal_validate_storage_change_values next slot _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes the encoded changes for one storage slot.  Each RLP pop strictly
reduces the remaining byte length and must have one matching host event. -/
/- Type quantifiers: slot : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ slot ∧ slot ≤ (2 ^ 256 - 1) -/
def bal_validate_storage_change_values (cursor : (EvmByteSliceFields k_source_off k_source_len)) (slot : Nat) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_storage_change_values cursor slot (_measure + 1))

/-- Consumes canonical `[slot, changes]` entries in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_ex416470_ : Nat, k_ex416469_ : Nat, k_source_off : Nat, k_source_len
  : Nat, (source_valid_range k_source_off k_source_len), 0 ≤ k_ex416469_ ∧
  k_ex416469_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex416470_ ∧ k_ex416470_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_validate_storage_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (maximum : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure count)
      else
        (do
          let ⟨slot_field_content_len, ⟨slot_field_full_len, (slot_field, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let fields ← do (bal_ref_cursor slot_field)
          let ⟨slot_value_content_len, ⟨slot_value_full_len, (slot_value, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨changes_value_content_len, ⟨changes_value_full_len, (changes_value, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (bal_expect_end fields)
          let changes ← do (bal_ref_cursor changes_value)
          if ((changes.len == 0) : Bool)
          then sailThrow ((InvalidBlock InvalidBlockAccessList))
          else
            (do
              let slot ← do (bal_ref_word slot_value)
              (bal_validate_storage_change_values changes slot)
              let count ← do (bal_count_item count maximum)
              (_rec_bal_validate_storage_changes next count maximum _reclimit_pred))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes canonical `[slot, changes]` entries in their encoded order. -/
/- Type quantifiers: maximum : Nat, count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ maximum ∧ maximum ≤ (2 ^ 64 - 1) -/
def bal_validate_storage_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (maximum : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_storage_changes cursor count maximum (_measure + 1))

/-- Consumes the read-only storage slots in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_ex416500_ : Nat, k_ex416499_ : Nat, k_source_off : Nat, k_source_len
  : Nat, (source_valid_range k_source_off k_source_len), 0 ≤ k_ex416499_ ∧
  k_ex416499_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex416500_ ∧ k_ex416500_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_validate_storage_reads (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (maximum : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure count)
      else
        (do
          let ⟨slot_field_content_len, ⟨slot_field_full_len, (slot_field, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let slot ← do (bal_ref_word slot_field)
          match (← (bal_iter_next ())) with
          | .BalStorageRead recorded =>
            (do
              if ((recorded != slot) : Bool)
              then sailThrow ((InvalidBlock InvalidBlockAccessList))
              else (pure ()))
          | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
          let count ← do (bal_count_item count maximum)
          (_rec_bal_validate_storage_reads next count maximum _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes the read-only storage slots in their encoded order. -/
/- Type quantifiers: maximum : Nat, count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ maximum ∧ maximum ≤ (2 ^ 64 - 1) -/
def bal_validate_storage_reads (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (maximum : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_storage_reads cursor count maximum (_measure + 1))

/-- Consumes balance changes in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_balance_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure ())
      else
        (do
          let ⟨pair_content_len, ⟨pair_full_len, (pair, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          match (← (bal_iter_next ())) with
          | .BalBalanceChange change => (bal_compare_index_word pair change.index change.value)
          | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
          (_rec_bal_validate_balance_changes next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes balance changes in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_balance_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_balance_changes cursor (_measure + 1))

/-- Consumes nonce changes in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_nonce_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure ())
      else
        (do
          let ⟨pair_content_len, ⟨pair_full_len, (pair, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          match (← (bal_iter_next ())) with
          | .BalNonceChange change => (bal_compare_index_nonce pair change.index change.value)
          | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
          (_rec_bal_validate_nonce_changes next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes nonce changes in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_nonce_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_nonce_changes cursor (_measure + 1))

/-- Consumes code changes in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_code_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure ())
      else
        (do
          let ⟨pair_content_len, ⟨pair_full_len, (pair, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          match (← (bal_iter_next ())) with
          | .BalCodeChange change => (bal_compare_index_code pair change.index change.code_hash)
          | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
          (_rec_bal_validate_code_changes next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes code changes in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_code_changes (cursor : (EvmByteSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_code_changes cursor (_measure + 1))

/-- Consumes account entries from the canonical RLP list.  The encoded cursor
is the traversal driver; the host iterator supplies exactly one comparison
event for every decoded semantic value. -/
/- Type quantifiers: _reclimit : Nat, k_ex416608_ : Nat, k_ex416607_ : Nat, k_source_off : Nat, k_source_len
  : Nat, (source_valid_range k_source_off k_source_len), 0 ≤ k_ex416607_ ∧
  k_ex416607_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex416608_ ∧ k_ex416608_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_validate_accounts (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (maximum : Nat) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure count)
      else
        (do
          let ⟨account_field_content_len, ⟨account_field_full_len, (account_field, next)⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let fields ← do (bal_ref_cursor account_field)
          let ⟨address_field_content_len, ⟨address_field_full_len, (address_field, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨storage_changes_field_content_len, ⟨storage_changes_field_full_len, (storage_changes_field, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨storage_reads_field_content_len, ⟨storage_reads_field_full_len, (storage_reads_field, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨balance_changes_field_content_len, ⟨balance_changes_field_full_len, (balance_changes_field, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨nonce_changes_field_content_len, ⟨nonce_changes_field_full_len, (nonce_changes_field, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          let ⟨code_changes_field_content_len, ⟨code_changes_field_full_len, (code_changes_field, fields)⟩⟩ ← do
            (rlp_cursor_pop fields)
          (bal_expect_end fields)
          let ⟨_, ⟨_, address_bytes⟩⟩ ← do (bal_ref_bytes address_field)
          let account ← do (pure (word_to_address (← (rlp_ref_word address_field))))
          if ((address_bytes.len != 20) : Bool)
          then sailThrow ((InvalidBlock InvalidBlockAccessList))
          else
            (do
              match (← (bal_iter_next ())) with
              | .BalAccount recorded =>
                (do
                  if ((bne recorded account) : Bool)
                  then sailThrow ((InvalidBlock InvalidBlockAccessList))
                  else (pure ()))
              | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
              let count ← do (bal_count_item count maximum)
              let count ← do
                (bal_validate_storage_changes (← (bal_ref_cursor storage_changes_field)) count
                  maximum)
              let count ← do
                (bal_validate_storage_reads (← (bal_ref_cursor storage_reads_field)) count maximum)
              (bal_validate_balance_changes (← (bal_ref_cursor balance_changes_field)))
              (bal_validate_nonce_changes (← (bal_ref_cursor nonce_changes_field)))
              (bal_validate_code_changes (← (bal_ref_cursor code_changes_field)))
              match (← (bal_iter_next ())) with
              | .BalAccountEnd _ => (pure ())
              | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))
              (_rec_bal_validate_accounts next count maximum _reclimit_pred))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes account entries from the canonical RLP list.  The encoded cursor
is the traversal driver; the host iterator supplies exactly one comparison
event for every decoded semantic value. -/
/- Type quantifiers: maximum : Nat, count : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ maximum ∧ maximum ≤ (2 ^ 64 - 1) -/
def bal_validate_accounts (cursor : (EvmByteSliceFields k_source_off k_source_len)) (count : Nat) (maximum : Nat) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_accounts cursor count maximum (_measure + 1))

/-- Validates the canonical EIP-7928 BAL directly against the host recorder. -/
/- Type quantifiers: k_ex416635_ : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0
  ≤ bytes_dependentWitness0 ∧ 0 ≤ bytes_dependentWitness1, 0 ≤ k_ex416635_ ∧
  k_ex416635_ ≤ (2 ^ 64 - 1) -/
def validate_block_access_list (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (maximum_items : Nat) : SailM Unit := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  (bal_prepare_iter ())
  let ⟨_, root⟩ ← do (rlp_single_ref bytes)
  let _ ← do (bal_validate_accounts (← (bal_ref_cursor root)) 0 maximum_items)
  match (← (bal_iter_next ())) with
  | .BalEmpty _ => (pure ())
  | _ => sailThrow ((InvalidBlock InvalidBlockAccessList))

