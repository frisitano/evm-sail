import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.Code
import Evm.Lib.Rlp.Decoding
import Evm.Lib.Rlp.Codecs.BlockAccessList

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

/-! # The block access list

Validation of the EIP-7928 block access list supplied by the stateless host.
The supplied bytes are decoded as canonical RLP and consumed in lockstep with
the recorder's ordered account/change tables.  Validation never reconstructs
or re-encodes the list: the original source-backed slice is also the value
hashed into the execution-payload header. -/

/-- Compares one canonical `[index, word]` pair. -/
/- Type quantifiers: k_ex552672_ : Nat, k_ex552671_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex552671_ ∧
  k_ex552671_ ≤ (2 ^ 20 + 1), 0 ≤ k_ex552672_ ∧ k_ex552672_ ≤ (2 ^ 256 - 1) -/
def bal_compare_index_word (pair : (RlpFieldRef k_source_off k_source_len k_content_len)) (index : Nat) (value : Nat) : SailM Unit := do
  let fields ← do (bal_ref_cursor pair)
  let ⟨_, ⟨_, index_field⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields index_field.source.len)
  let ⟨_, ⟨_, value_field⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_field.source.len)
  (bal_expect_end fields)
  let decoded_index ← do (bal_ref_uint64 index_field)
  let decoded_value ← do (bal_ref_word value_field)
  if (((decoded_index != index) || (decoded_value != value)) : Bool)
  then (fatal_error InvalidBlockAccessList)
  else (pure ())

/-- Compares one canonical `[index, nonce]` pair. -/
/- Type quantifiers: k_ex552692_ : Nat, k_ex552691_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len
  : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0 ≤ k_ex552691_ ∧
  k_ex552691_ ≤ (2 ^ 20 + 1), 0 ≤ k_ex552692_ ∧ k_ex552692_ ≤ (2 ^ 64 - 1) -/
def bal_compare_index_nonce (pair : (RlpFieldRef k_source_off k_source_len k_content_len)) (index : Nat) (value : Nat) : SailM Unit := do
  let fields ← do (bal_ref_cursor pair)
  let ⟨_, ⟨_, index_field⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields index_field.source.len)
  let ⟨_, ⟨_, value_field⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields value_field.source.len)
  (bal_expect_end fields)
  let decoded_index ← do (bal_ref_uint64 index_field)
  let decoded_value ← do (bal_ref_uint64 value_field)
  if (((decoded_index != index) || (decoded_value != value)) : Bool)
  then (fatal_error InvalidBlockAccessList)
  else (pure ())

/-- Compares one canonical `[index, code]` pair without materializing either
source-backed code sequence. -/
/- Type quantifiers: k_ex552711_ : Nat, k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len), 0
  ≤ k_ex552711_ ∧ k_ex552711_ ≤ (2 ^ 20 + 1) -/
def bal_compare_index_code (pair : (RlpFieldRef k_source_off k_source_len k_content_len)) (index : Nat) (code_hash : (Vector (BitVec 8) 32)) : SailM Unit := do
  let fields ← do (bal_ref_cursor pair)
  let ⟨_, ⟨_, index_field⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields index_field.source.len)
  let ⟨_, ⟨_, code_field⟩⟩ ← do (rlp_decode_item fields)
  let fields := (rlp_cursor_advance fields code_field.source.len)
  (bal_expect_end fields)
  let ⟨_, ⟨_, code⟩⟩ ← do (code_db_resolve code_hash)
  let decoded_index ← do (bal_ref_uint64 index_field)
  let ⟨_, ⟨_, encoded_code⟩⟩ ← do (bal_ref_bytes code_field)
  let expected_code := (code_bytes code)
  let code_matches ← do
    (input_code_slices_equal ⟨_, ⟨_, encoded_code⟩⟩ ⟨_, ⟨_, expected_code⟩⟩)
  let code_mismatch := (! code_matches)
  if (((decoded_index != index) || code_mismatch) : Bool)
  then (fatal_error InvalidBlockAccessList)
  else (pure ())

/-- Consumes the encoded changes for one storage slot.  Each RLP pop strictly
reduces the remaining byte length and must have one matching host event. -/
/- Type quantifiers: _reclimit : Nat, k_ex552726_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ k_ex552726_ ∧ k_ex552726_ ≤ (2 ^ 256 - 1), 0 ≤ _reclimit -/
def _rec_bal_validate_storage_change_values (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (slot : Nat) (_reclimit : Nat) : SailM Unit := do
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
          let ⟨_, ⟨_, pair⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor pair.source.len)
          let event ← do (bal_iter_next ())
          match event with
          | .BalStorageChange change =>
            (do
              if ((change.slot != slot) : Bool)
              then (fatal_error InvalidBlockAccessList)
              else (pure ())
              (bal_compare_index_word pair change.index change.value))
          | _ => (fatal_error InvalidBlockAccessList)
          (_rec_bal_validate_storage_change_values next slot _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes the encoded changes for one storage slot.  Each RLP pop strictly
reduces the remaining byte length and must have one matching host event. -/
/- Type quantifiers: slot : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ slot ∧ slot ≤ (2 ^ 256 - 1) -/
def bal_validate_storage_change_values (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (slot : Nat) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_storage_change_values cursor slot (_measure + 1))

/-- Consumes canonical `[slot, changes]` entries in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_storage_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure 0)
      else
        (do
          let ⟨_, ⟨_, slot_field⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor slot_field.source.len)
          let fields ← do (bal_ref_cursor slot_field)
          let ⟨_, ⟨_, slot_value⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields slot_value.source.len)
          let ⟨_, ⟨_, changes_value⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields changes_value.source.len)
          (bal_expect_end fields)
          let changes ← do (bal_ref_cursor changes_value)
          if ((changes.len == 0) : Bool)
          then (fatal_error InvalidBlockAccessList)
          else (pure ())
          let slot ← do (bal_ref_word slot_value)
          (bal_validate_storage_change_values changes slot)
          (pure (1 + (← (_rec_bal_validate_storage_changes next _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes canonical `[slot, changes]` entries in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_storage_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_storage_changes cursor (_measure + 1))

/-- Consumes the read-only storage slots in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_storage_reads (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure 0)
      else
        (do
          let ⟨_, ⟨_, slot_field⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor slot_field.source.len)
          let slot ← do (bal_ref_word slot_field)
          let event ← do (bal_iter_next ())
          match event with
          | .BalStorageRead recorded =>
            (do
              if ((recorded != slot) : Bool)
              then (fatal_error InvalidBlockAccessList)
              else (pure ()))
          | _ => (fatal_error InvalidBlockAccessList)
          (pure (1 + (← (_rec_bal_validate_storage_reads next _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes the read-only storage slots in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_storage_reads (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_storage_reads cursor (_measure + 1))

/-- Consumes balance changes in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_balance_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
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
          let ⟨_, ⟨_, pair⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor pair.source.len)
          let event ← do (bal_iter_next ())
          match event with
          | .BalBalanceChange change => (bal_compare_index_word pair change.index change.value)
          | _ => (fatal_error InvalidBlockAccessList)
          (_rec_bal_validate_balance_changes next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes balance changes in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_balance_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_balance_changes cursor (_measure + 1))

/-- Consumes nonce changes in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_nonce_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
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
          let ⟨_, ⟨_, pair⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor pair.source.len)
          let event ← do (bal_iter_next ())
          match event with
          | .BalNonceChange change => (bal_compare_index_nonce pair change.index change.value)
          | _ => (fatal_error InvalidBlockAccessList)
          (_rec_bal_validate_nonce_changes next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes nonce changes in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_nonce_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_nonce_changes cursor (_measure + 1))

/-- Consumes code changes in their encoded order. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_code_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Unit := do
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
          let ⟨_, ⟨_, pair⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor pair.source.len)
          let event ← do (bal_iter_next ())
          match event with
          | .BalCodeChange change => (bal_compare_index_code pair change.index change.code_hash)
          | _ => (fatal_error InvalidBlockAccessList)
          (_rec_bal_validate_code_changes next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes code changes in their encoded order. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_code_changes (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Unit := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_code_changes cursor (_measure + 1))

/-- Consumes account entries from the canonical RLP list.  The encoded cursor
is the traversal driver; the host iterator supplies exactly one comparison
event for every decoded semantic value. -/
/- Type quantifiers: _reclimit : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ _reclimit -/
def _rec_bal_validate_accounts (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (_reclimit : Nat) : SailM Nat := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure 0)
      else
        (do
          let ⟨_, ⟨_, account_field⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor account_field.source.len)
          let fields ← do (bal_ref_cursor account_field)
          let ⟨_, ⟨_, address_field⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields address_field.source.len)
          let ⟨_, ⟨_, storage_changes_field⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields storage_changes_field.source.len)
          let ⟨_, ⟨_, storage_reads_field⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields storage_reads_field.source.len)
          let ⟨_, ⟨_, balance_changes_field⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields balance_changes_field.source.len)
          let ⟨_, ⟨_, nonce_changes_field⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields nonce_changes_field.source.len)
          let ⟨_, ⟨_, code_changes_field⟩⟩ ← do (rlp_decode_item fields)
          let fields := (rlp_cursor_advance fields code_changes_field.source.len)
          (bal_expect_end fields)
          let ⟨_, ⟨_, address_bytes⟩⟩ ← do (bal_ref_bytes address_field)
          let address_word ← do (rlp_decode_word address_field)
          let account := (word_to_address address_word)
          if ((address_bytes.len != 20) : Bool)
          then (fatal_error InvalidBlockAccessList)
          else (pure ())
          let account_event ← do (bal_iter_next ())
          match account_event with
          | .BalAccount recorded =>
            (do
              if ((bne recorded account) : Bool)
              then (fatal_error InvalidBlockAccessList)
              else (pure ()))
          | _ => (fatal_error InvalidBlockAccessList)
          let storage_changes_cursor ← do (bal_ref_cursor storage_changes_field)
          let storage_changes ← do (bal_validate_storage_changes storage_changes_cursor)
          let storage_reads_cursor ← do (bal_ref_cursor storage_reads_field)
          let storage_reads ← do (bal_validate_storage_reads storage_reads_cursor)
          let balance_changes_cursor ← do (bal_ref_cursor balance_changes_field)
          (bal_validate_balance_changes balance_changes_cursor)
          let nonce_changes_cursor ← do (bal_ref_cursor nonce_changes_field)
          (bal_validate_nonce_changes nonce_changes_cursor)
          let code_changes_cursor ← do (bal_ref_cursor code_changes_field)
          (bal_validate_code_changes code_changes_cursor)
          let account_end_event ← do (bal_iter_next ())
          match account_end_event with
          | .BalAccountEnd _ => (pure ())
          | _ => (fatal_error InvalidBlockAccessList)
          (pure (((1 + storage_changes) + storage_reads) + (← (_rec_bal_validate_accounts next
                  _reclimit_pred))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Consumes account entries from the canonical RLP list.  The encoded cursor
is the traversal driver; the host iterator supplies exactly one comparison
event for every decoded semantic value. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len) -/
def bal_validate_accounts (cursor : (StatelessInputSliceFields k_source_off k_source_len)) : SailM Nat := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_validate_accounts cursor (_measure + 1))

/-- Validates the canonical EIP-7928 BAL directly against the host recorder. -/
/- Type quantifiers: k_ex552908_ : Nat, bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ (2 ^ 30) ∧ bytes_dependentWitness1 ≤ (2 ^ 30), 0 ≤ k_ex552908_ ∧
  k_ex552908_ ≤ (2 ^ 64 - 1) -/
def validate_block_access_list (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (block_gas_limit : Nat) : SailM Unit := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  (bal_prepare_iter ())
  let ⟨_, root⟩ ← do (rlp_single_ref bytes)
  let accounts_cursor ← do (bal_ref_cursor root)
  let bal_items ← do (bal_validate_accounts accounts_cursor)
  let remaining_event ← do (bal_iter_next ())
  match remaining_event with
  | .BalEmpty _ => (pure ())
  | _ => (fatal_error InvalidBlockAccessList)
  if (((BLOCK_ACCESS_LIST_ITEM_GAS *i bal_items) >b block_gas_limit) : Bool)
  then (fatal_error BlockAccessListTooLarge)
  else (pure ())

