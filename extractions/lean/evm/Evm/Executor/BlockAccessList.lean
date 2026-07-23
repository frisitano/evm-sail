import Evm.Flow
import Evm.Prelude
import Evm.Host.Kernel.Scratch
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

Reconstruction of the EIP-7928 block access list from the recorder's
per-account change streams, as source-backed RLP that is compared against
the payload's claimed bytes.

## Types

The cursor records pair an encoded-content length with the ordered-table
position reached by a sizing pass, so the subsequent writer follows exactly
the same grouping boundaries. -/

def BAL_RLP_ZERO : bal_rlp_length := ⟨0⟩

/- Type quantifiers: value : Nat, 0 ≤ value -/
def bal_rlp_length_from_byte_length (value : Nat) : SailM bal_rlp_length := do
  let publicResult ← do
    if ((value ≤b (2 ^i 30)) : Bool)
    then (pure value)
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/-- Exposes a schema-bounded BAL RLP length as a byte length. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 30) -/
def bal_rlp_length_to_byte_length (value : bal_rlp_length) : Nat :=
  let value := (value).value
  value

/- Type quantifiers: current : Nat, addition : Nat, 0 ≤ addition, 0 ≤ current ∧
  current ≤ (2 ^ 30) -/
def bal_rlp_length_add (current : bal_rlp_length) (addition : Nat) : SailM bal_rlp_length := do
  let current := (current).value
  let publicResult ← do
    let bounded_addition ← do
      (do
          let publicResult ← (bal_rlp_length_from_byte_length addition)
          pure ((publicResult).value))
    if ((bounded_addition ≤b ((2 ^i 30) - current)) : Bool)
    then (pure (current + bounded_addition))
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧ 0 ≤ right -/
def bal_bounded_byte_length_add (left : Nat) (right : Nat) : SailM Nat := do
  (pure (bal_rlp_length_to_byte_length
      ⟨((← (bal_rlp_length_add ⟨((← (bal_rlp_length_from_byte_length left))).value⟩ right))).value⟩))

/-- Adds the canonical RLP list evm_prefix to a bounded BAL content length. -/
/- Type quantifiers: content_len : Nat, 0 ≤ content_len ∧ content_len ≤ (2 ^ 30) -/
def bal_rlp_list_size (content_len : bal_rlp_length) : SailM bal_rlp_length := do
  let content_len := (content_len).value
  let publicResult ← do
    let content_length := (bal_rlp_length_to_byte_length ⟨content_len⟩)
    let encoded_length ← do (rlp_list_size content_length)
    (do
        let publicResult ← (bal_rlp_length_from_byte_length encoded_length)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/- Type quantifiers: content_len : Nat, 0 ≤ content_len -/
def bal_bounded_list_size (content_len : Nat) : SailM Nat := do
  (pure (bal_rlp_length_to_byte_length
      ⟨((← (bal_rlp_list_size ⟨((← (bal_rlp_length_from_byte_length content_len))).value⟩))).value⟩))

def undefined_BalContentCursor (_ : Unit) : SailM BalContentCursor := do
  (pure { content_len := ← do
              let publicField ← (undefined_range 0 (2 ^i 30))
              pure (⟨publicField⟩),
          cursor := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩) })

def undefined_BalContentCount (_ : Unit) : SailM BalContentCount := do
  (pure { content_len := ← do
              let publicField ← (undefined_range 0 (2 ^i 30))
              pure (⟨publicField⟩),
          count := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩) })

def undefined_BalAccountSize (_ : Unit) : SailM BalAccountSize := do
  (pure { encoded_len := ← do
              let publicField ← (undefined_range 0 (2 ^i 30))
              pure (⟨publicField⟩),
          item_count := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩) })

def undefined_BalNonceRun (_ : Unit) : SailM BalNonceRun := do
  (pure { cursor := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩),
          maximum := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩) })

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ 0 ≤ b -/
def bal_count_add (a : Nat) (b : Nat) : SailM Nat := do
  if (((a ≤b (2 ^i 30)) && (b ≤b ((2 ^i 30) -i a))) : Bool)
  then (pure (a + b))
  else sailThrow ((InvalidBlock BlockAccessListTooLarge))

/-- Returns the final row of a non-empty BAL run. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def bal_previous_index (value : item_index) : SailM item_index := do
  let value := (value).value
  let publicResult ← do
    if ((value != 0) : Bool)
    then (pure (value - 1))
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex411431_ : Nat, k_ex411430_ : Nat, 0 ≤ k_ex411430_ ∧
  k_ex411430_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411431_ ∧ k_ex411431_ ≤ (2 ^ 256 - 1) -/
def bal_index_word_content_size (index : item_index) (value : word) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_byte_length_add (rlp_uint_word_size index) (rlp_uint_word_size value))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex411433_ : Nat, k_ex411432_ : Nat, 0 ≤ k_ex411432_ ∧
  k_ex411432_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411433_ ∧ k_ex411433_ ≤ (2 ^ 256 - 1) -/
def bal_index_word_size (index : item_index) (value : word) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_list_size ((← (bal_index_word_content_size ⟨index⟩ ⟨value⟩))).value)
  pure (⟨publicResult⟩)

/-- Writes one `[index, word]` BAL pair. -/
/- Type quantifiers: k_ex411435_ : Nat, k_ex411434_ : Nat, 0 ≤ k_ex411434_ ∧
  k_ex411434_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411435_ ∧ k_ex411435_ ≤ (2 ^ 256 - 1) -/
def bal_write_index_word (index : item_index) (value : word) : SailM Unit := do
  let index := (index).value
  let value := (value).value
  (rlp_write_list_prefix ((← (bal_index_word_content_size ⟨index⟩ ⟨value⟩))).value)
  (rlp_write_uint_word index)
  (rlp_write_uint_word value)

/- Type quantifiers: k_ex411437_ : Nat, k_ex411436_ : Nat, 0 ≤ k_ex411436_ ∧
  k_ex411436_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411437_ ∧ k_ex411437_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_content_size (index : item_index) (value : account_nonce) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_byte_length_add (rlp_uint_word_size index) (rlp_uint_word_size value))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex411439_ : Nat, k_ex411438_ : Nat, 0 ≤ k_ex411438_ ∧
  k_ex411438_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411439_ ∧ k_ex411439_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_size (index : item_index) (value : account_nonce) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_list_size ((← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩))).value)
  pure (⟨publicResult⟩)

/-- Writes one `[index, nonce]` BAL pair. -/
/- Type quantifiers: k_ex411441_ : Nat, k_ex411440_ : Nat, 0 ≤ k_ex411440_ ∧
  k_ex411440_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411441_ ∧ k_ex411441_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_nonce (index : item_index) (value : account_nonce) : SailM Unit := do
  let index := (index).value
  let value := (value).value
  (rlp_write_list_prefix ((← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩))).value)
  (rlp_write_uint_word index)
  (rlp_write_uint_word value)

/-- Sizes the content of an index-and-code pair. -/
/- Type quantifiers: k_ex411442_ : Nat, 0 ≤ k_ex411442_ ∧ k_ex411442_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_content_size (index : item_index) (code_hash : hash) : SailM bal_rlp_length := do
  let index := (index).value
  let publicResult ← do
    let code ← do (code_db_resolve code_hash)
    let index_length := (rlp_uint_word_size index)
    let code_length ← do (rlp_slice_size ((code.bytes).2).2)
    (bal_bounded_byte_length_add index_length code_length)
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex411443_ : Nat, 0 ≤ k_ex411443_ ∧ k_ex411443_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_size (index : item_index) (code_hash : hash) : SailM bal_rlp_length := do
  let index := (index).value
  let publicResult ← do
    (bal_bounded_list_size ((← (bal_index_code_content_size ⟨index⟩ code_hash))).value)
  pure (⟨publicResult⟩)

/-- Writes one `[index, code]` BAL pair after resolving the code hash. -/
/- Type quantifiers: k_ex411444_ : Nat, 0 ≤ k_ex411444_ ∧ k_ex411444_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_code (index : item_index) (code_hash : hash) : SailM Unit := do
  let index := (index).value
  let code ← do (code_db_resolve code_hash)
  (rlp_write_list_prefix ((← (bal_index_code_content_size ⟨index⟩ code_hash))).value)
  (rlp_write_uint_word index)
  (rlp_write_slice
    (⟨_, ⟨_, ((code.bytes).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Finds the end of an equal-slot, equal-index storage-change run. -/
/- Type quantifiers: _reclimit : Nat, k_ex411449_ : Nat, k_ex411448_ : Nat, k_ex411447_ : Nat, k_ex411446_
  : Nat, k_ex411445_ : Nat, 0 ≤ k_ex411445_ ∧ k_ex411445_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411446_
  ∧ k_ex411446_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411447_ ∧ k_ex411447_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex411448_ ∧ k_ex411448_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411449_ ∧ k_ex411449_ ≤ (2 ^ 64 - 1), 0
  ≤ _reclimit -/
def _rec_bal_storage_change_run_end (account : item_index) (count : item_count) (slot : word) (index : item_index) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let index := (index).value
  let cursor := (cursor).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            if (((((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩))).value == slot) && (← do
                   (pure (((← (bal_storage_change_index ⟨account⟩ ⟨cursor⟩))).value == index)))) : Bool)
            then
              (do
                  let publicResult ← (_rec_bal_storage_change_run_end ⟨account⟩ ⟨count⟩
                  ⟨slot⟩ ⟨index⟩ ⟨(cursor + 1)⟩ _reclimit_pred)
                  pure ((publicResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds the end of an equal-slot, equal-index storage-change run. -/
/- Type quantifiers: cursor : Nat, index : Nat, slot : Nat, count : Nat, account : Nat, 0 ≤
  account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ slot ∧
  slot ≤ (2 ^ 256 - 1), 0 ≤ index ∧ index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_change_run_end (account : item_index) (count : item_count) (slot : word) (index : item_index) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let index := (index).value
  let cursor := (cursor).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_storage_change_run_end ⟨account⟩ ⟨count⟩ ⟨slot⟩
          ⟨index⟩ ⟨cursor⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Sizes the final values for all change indices of one storage slot. -/
/- Type quantifiers: _reclimit : Nat, k_ex411461_ : Nat, k_ex411460_ : Nat, k_ex411459_ : Nat, k_ex411458_
  : Nat, k_ex411457_ : Nat, 0 ≤ k_ex411457_ ∧ k_ex411457_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411458_
  ∧ k_ex411458_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411459_ ∧ k_ex411459_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex411460_ ∧ k_ex411460_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411461_ ∧ k_ex411461_ ≤ (2 ^ 30), 0
  ≤ _reclimit -/
def _rec_bal_storage_slot_changes_size (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (content_len : bal_rlp_length) (_reclimit : Nat) : SailM BalContentCursor := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          if ((((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩))).value == slot) : Bool)
          then
            (do
              let index ← do
                (do
                    let publicResult ← (bal_storage_change_index ⟨account⟩ ⟨cursor⟩)
                    pure ((publicResult).value))
              let next ← do
                (do
                    let publicResult ← (bal_storage_change_run_end ⟨account⟩ ⟨count⟩
                    ⟨slot⟩ ⟨index⟩ ⟨cursor⟩)
                    pure ((publicResult).value))
              let last ← do
                (do
                    let publicResult ← (bal_previous_index ⟨next⟩)
                    pure ((publicResult).value))
              let next_len ← do
                (do
                    let publicResult ← (bal_rlp_length_add ⟨content_len⟩
                    ((← (bal_index_word_size ⟨index⟩
                      ⟨((← (bal_storage_change_value ⟨account⟩ ⟨last⟩))).value⟩))).value)
                    pure ((publicResult).value))
              (_rec_bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ ⟨slot⟩ ⟨next⟩
                ⟨next_len⟩ _reclimit_pred))
          else
            (pure { content_len := ⟨content_len⟩,
                    cursor := ⟨cursor⟩ }))
      else
        (pure { content_len := ⟨content_len⟩,
                cursor := ⟨cursor⟩ }))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the final values for all change indices of one storage slot. -/
/- Type quantifiers: content_len : Nat, cursor : Nat, slot : Nat, count : Nat, account : Nat, 0 ≤
  account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ slot ∧
  slot ≤ (2 ^ 256 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1), 0 ≤ content_len ∧
  content_len ≤ (2 ^ 30) -/
def bal_storage_slot_changes_size (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (content_len : bal_rlp_length) : SailM BalContentCursor := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ ⟨slot⟩ ⟨cursor⟩
      ⟨content_len⟩ (_measure + 1))

/-- Writes the final value for each change index of one storage slot. -/
/- Type quantifiers: _reclimit : Nat, k_ex411472_ : Nat, k_ex411471_ : Nat, k_ex411470_ : Nat, k_ex411469_
  : Nat, 0 ≤ k_ex411469_ ∧ k_ex411469_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411470_ ∧
  k_ex411470_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411471_ ∧ k_ex411471_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex411472_ ∧ k_ex411472_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_storage_slot_changes (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            if ((((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩))).value == slot) : Bool)
            then
              (do
                let index ← do
                  (do
                      let publicResult ← (bal_storage_change_index ⟨account⟩ ⟨cursor⟩)
                      pure ((publicResult).value))
                let next ← do
                  (do
                      let publicResult ← (bal_storage_change_run_end ⟨account⟩ ⟨count⟩
                      ⟨slot⟩ ⟨index⟩ ⟨cursor⟩)
                      pure ((publicResult).value))
                let last ← do
                  (do
                      let publicResult ← (bal_previous_index ⟨next⟩)
                      pure ((publicResult).value))
                (bal_write_index_word ⟨index⟩
                  ⟨((← (bal_storage_change_value ⟨account⟩ ⟨last⟩))).value⟩)
                (do
                    let publicResult ← (_rec_bal_write_storage_slot_changes ⟨account⟩
                    ⟨count⟩ ⟨slot⟩ ⟨next⟩ _reclimit_pred)
                    pure ((publicResult).value)))
            else (pure cursor))
        else (pure cursor))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes the final value for each change index of one storage slot. -/
/- Type quantifiers: cursor : Nat, slot : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ slot ∧
  slot ≤ (2 ^ 256 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
def bal_write_storage_slot_changes (account : item_index) (count : item_count) (slot : word) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_write_storage_slot_changes ⟨account⟩ ⟨count⟩
          ⟨slot⟩ ⟨cursor⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Sizes storage changes grouped by slot and counts the groups. -/
/- Type quantifiers: _reclimit : Nat, k_ex411481_ : Nat, k_ex411480_ : Nat, k_ex411479_ : Nat, 0 ≤
  k_ex411479_ ∧ k_ex411479_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411480_ ∧ k_ex411480_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex411481_ ∧ k_ex411481_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_change_groups_size (account : item_index) (count : item_count) (cursor : item_index) (result : BalContentCount) (_reclimit : Nat) : SailM BalContentCount := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          let slot ← do
            (do
                let publicResult ← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          let changes ← do
            (bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ ⟨slot⟩ ⟨cursor⟩
              ⟨(BAL_RLP_ZERO).value⟩)
          let slot_content_len ← do
            (do
                let publicResult ← (bal_rlp_length_add
                ⟨((← (bal_rlp_length_from_byte_length (rlp_uint_word_size slot)))).value⟩
                (bal_rlp_length_to_byte_length
                  ⟨((← (bal_rlp_list_size ⟨(changes.content_len).value⟩))).value⟩))
                pure ((publicResult).value))
          let next_content_len ← do
            (do
                let publicResult ← (bal_rlp_length_add ⟨(result.content_len).value⟩
                (bal_rlp_length_to_byte_length
                  ⟨((← (bal_rlp_list_size ⟨slot_content_len⟩))).value⟩))
                pure ((publicResult).value))
          (_rec_bal_storage_change_groups_size ⟨account⟩ ⟨count⟩
            ⟨(changes.cursor).value⟩
            { content_len := ⟨next_content_len⟩,
              count := ← do
                  let publicField ← (bal_count_add (result.count).value 1)
                  pure (⟨publicField⟩) } _reclimit_pred))
      else (pure result))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes storage changes grouped by slot and counts the groups. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_change_groups_size (account : item_index) (count : item_count) (cursor : item_index) (result : BalContentCount) : SailM BalContentCount := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_storage_change_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩ result
      (_measure + 1))

/-- Writes storage changes grouped by slot. -/
/- Type quantifiers: _reclimit : Nat, k_ex411489_ : Nat, k_ex411488_ : Nat, k_ex411487_ : Nat, 0 ≤
  k_ex411487_ ∧ k_ex411487_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411488_ ∧ k_ex411488_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex411489_ ∧ k_ex411489_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_storage_change_groups (account : item_index) (count : item_count) (cursor : item_index) (_reclimit : Nat) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          let slot ← do
            (do
                let publicResult ← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          let changes ← do
            (bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ ⟨slot⟩ ⟨cursor⟩
              ⟨(BAL_RLP_ZERO).value⟩)
          let slot_content_len ← do
            (do
                let publicResult ← (bal_rlp_length_add
                ⟨((← (bal_rlp_length_from_byte_length (rlp_uint_word_size slot)))).value⟩
                (bal_rlp_length_to_byte_length
                  ⟨((← (bal_rlp_list_size ⟨(changes.content_len).value⟩))).value⟩))
                pure ((publicResult).value))
          (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨slot_content_len⟩))
          (rlp_write_uint_word slot)
          (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨(changes.content_len).value⟩))
          let next ← do
            (do
                let publicResult ← (bal_write_storage_slot_changes ⟨account⟩ ⟨count⟩
                ⟨slot⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          assert (next == (changes.cursor).value) "BAL storage-change sizing cursor"
          (_rec_bal_write_storage_change_groups ⟨account⟩ ⟨count⟩ ⟨next⟩ _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes storage changes grouped by slot. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_write_storage_change_groups (account : item_index) (count : item_count) (cursor : item_index) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_storage_change_groups ⟨account⟩ ⟨count⟩ ⟨cursor⟩ (_measure + 1))

/-- Sizes the storage-change list for one account. -/
/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_storage_changes_size (account : item_index) : SailM BalContentCount := do
  let account := (account).value
  (bal_storage_change_groups_size ⟨account⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩
    { content_len := ⟨(BAL_RLP_ZERO).value⟩,
      count := ⟨0⟩ })

/-- Writes the storage-change list for one account. -/
/- Type quantifiers: k_ex411496_ : Nat, 0 ≤ k_ex411496_ ∧ k_ex411496_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_changes (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨(size.content_len).value⟩))
  (bal_write_storage_change_groups ⟨account⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Finds the end of a run of equal storage-read slots. -/
/- Type quantifiers: _reclimit : Nat, k_ex411500_ : Nat, k_ex411499_ : Nat, k_ex411498_ : Nat, k_ex411497_
  : Nat, 0 ≤ k_ex411497_ ∧ k_ex411497_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411498_ ∧
  k_ex411498_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411499_ ∧ k_ex411499_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex411500_ ∧ k_ex411500_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_read_run_end (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            if ((((← (bal_storage_read_slot ⟨account⟩ ⟨cursor⟩))).value == slot) : Bool)
            then
              (do
                  let publicResult ← (_rec_bal_storage_read_run_end ⟨account⟩ ⟨count⟩
                  ⟨slot⟩ ⟨(cursor + 1)⟩ _reclimit_pred)
                  pure ((publicResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds the end of a run of equal storage-read slots. -/
/- Type quantifiers: cursor : Nat, slot : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ slot ∧
  slot ≤ (2 ^ 256 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_read_run_end (account : item_index) (count : item_count) (slot : word) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_storage_read_run_end ⟨account⟩ ⟨count⟩ ⟨slot⟩
          ⟨cursor⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Advances the ordered change cursor to the first slot not below a read. -/
/- Type quantifiers: _reclimit : Nat, k_ex411510_ : Nat, k_ex411509_ : Nat, k_ex411508_ : Nat, k_ex411507_
  : Nat, 0 ≤ k_ex411507_ ∧ k_ex411507_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411508_ ∧
  k_ex411508_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411509_ ∧ k_ex411509_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex411510_ ∧ k_ex411510_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_change_seek (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            if ((word_ult ((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩))).value slot) : Bool)
            then
              (do
                  let publicResult ← (_rec_bal_storage_change_seek ⟨account⟩ ⟨count⟩
                  ⟨slot⟩ ⟨(cursor + 1)⟩ _reclimit_pred)
                  pure ((publicResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Advances the ordered change cursor to the first slot not below a read. -/
/- Type quantifiers: cursor : Nat, slot : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ slot ∧
  slot ≤ (2 ^ 256 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_change_seek (account : item_index) (count : item_count) (slot : word) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let slot := (slot).value
  let cursor := (cursor).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_storage_change_seek ⟨account⟩ ⟨count⟩ ⟨slot⟩
          ⟨cursor⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Sizes read slots that are not also represented by a storage change. -/
/- Type quantifiers: _reclimit : Nat, k_ex411521_ : Nat, k_ex411520_ : Nat, k_ex411519_ : Nat, k_ex411518_
  : Nat, k_ex411517_ : Nat, 0 ≤ k_ex411517_ ∧ k_ex411517_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411518_
  ∧ k_ex411518_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411519_ ∧ k_ex411519_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411520_ ∧ k_ex411520_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411521_ ∧ k_ex411521_ ≤ (2 ^ 64 - 1), 0
  ≤ _reclimit -/
def _rec_bal_storage_read_groups_size (account : item_index) (read_count : item_count) (change_count : item_count) (read : item_index) (change : item_index) (result : BalContentCount) (_reclimit : Nat) : SailM BalContentCount := do
  let account := (account).value
  let read_count := (read_count).value
  let change_count := (change_count).value
  let read := (read).value
  let change := (change).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((read <b read_count) : Bool)
      then
        (do
          let slot ← do
            (do
                let publicResult ← (bal_storage_read_slot ⟨account⟩ ⟨read⟩)
                pure ((publicResult).value))
          let next_read ← do
            (do
                let publicResult ← (bal_storage_read_run_end ⟨account⟩ ⟨read_count⟩
                ⟨slot⟩ ⟨read⟩)
                pure ((publicResult).value))
          let next_change ← do
            (do
                let publicResult ← (bal_storage_change_seek ⟨account⟩ ⟨change_count⟩
                ⟨slot⟩ ⟨change⟩)
                pure ((publicResult).value))
          let changed ← (( do
            if ((next_change <b change_count) : Bool)
            then
              (pure (((← (bal_storage_change_slot ⟨account⟩ ⟨next_change⟩))).value == slot))
            else (pure false) ) : SailM Bool )
          let updated ← do
            if (changed : Bool)
            then (pure result)
            else
              (pure { content_len := ← do
                          let publicField ← (do
                              let publicResult ← (bal_rlp_length_add
                              ⟨(result.content_len).value⟩ (rlp_uint_word_size slot))
                              pure ((publicResult).value))
                          pure (⟨publicField⟩),
                      count := ← do
                          let publicField ← (bal_count_add (result.count).value 1)
                          pure (⟨publicField⟩) })
          (_rec_bal_storage_read_groups_size ⟨account⟩ ⟨read_count⟩ ⟨change_count⟩
            ⟨next_read⟩ ⟨next_change⟩ updated _reclimit_pred))
      else (pure result))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes read slots that are not also represented by a storage change. -/
/- Type quantifiers: change : Nat, read : Nat, change_count : Nat, read_count : Nat, account : Nat, 0
  ≤ account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ read_count ∧ read_count ≤ (2 ^ 64 - 1), 0 ≤
  change_count ∧ change_count ≤ (2 ^ 64 - 1), 0 ≤ read ∧ read ≤ (2 ^ 64 - 1), 0 ≤ change
  ∧ change ≤ (2 ^ 64 - 1) -/
def bal_storage_read_groups_size (account : item_index) (read_count : item_count) (change_count : item_count) (read : item_index) (change : item_index) (result : BalContentCount) : SailM BalContentCount := do
  let account := (account).value
  let read_count := (read_count).value
  let change_count := (change_count).value
  let read := (read).value
  let change := (change).value
  let _measure := ((read_count -i read) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_storage_read_groups_size ⟨account⟩ ⟨read_count⟩ ⟨change_count⟩ ⟨read⟩
      ⟨change⟩ result (_measure + 1))

/-- Writes read slots that are not also represented by a storage change. -/
/- Type quantifiers: _reclimit : Nat, k_ex411533_ : Nat, k_ex411532_ : Nat, k_ex411531_ : Nat, k_ex411530_
  : Nat, k_ex411529_ : Nat, 0 ≤ k_ex411529_ ∧ k_ex411529_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411530_
  ∧ k_ex411530_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411531_ ∧ k_ex411531_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411532_ ∧ k_ex411532_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411533_ ∧ k_ex411533_ ≤ (2 ^ 64 - 1), 0
  ≤ _reclimit -/
def _rec_bal_write_storage_read_groups (account : item_index) (read_count : item_count) (change_count : item_count) (read : item_index) (change : item_index) (_reclimit : Nat) : SailM Unit := do
  let account := (account).value
  let read_count := (read_count).value
  let change_count := (change_count).value
  let read := (read).value
  let change := (change).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((read <b read_count) : Bool)
      then
        (do
          let slot ← do
            (do
                let publicResult ← (bal_storage_read_slot ⟨account⟩ ⟨read⟩)
                pure ((publicResult).value))
          let next_read ← do
            (do
                let publicResult ← (bal_storage_read_run_end ⟨account⟩ ⟨read_count⟩
                ⟨slot⟩ ⟨read⟩)
                pure ((publicResult).value))
          let next_change ← do
            (do
                let publicResult ← (bal_storage_change_seek ⟨account⟩ ⟨change_count⟩
                ⟨slot⟩ ⟨change⟩)
                pure ((publicResult).value))
          let changed ← (( do
            if ((next_change <b change_count) : Bool)
            then
              (pure (((← (bal_storage_change_slot ⟨account⟩ ⟨next_change⟩))).value == slot))
            else (pure false) ) : SailM Bool )
          if ((! changed) : Bool)
          then (rlp_write_uint_word slot)
          else (pure ())
          (_rec_bal_write_storage_read_groups ⟨account⟩ ⟨read_count⟩ ⟨change_count⟩
            ⟨next_read⟩ ⟨next_change⟩ _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes read slots that are not also represented by a storage change. -/
/- Type quantifiers: change : Nat, read : Nat, change_count : Nat, read_count : Nat, account : Nat, 0
  ≤ account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ read_count ∧ read_count ≤ (2 ^ 64 - 1), 0 ≤
  change_count ∧ change_count ≤ (2 ^ 64 - 1), 0 ≤ read ∧ read ≤ (2 ^ 64 - 1), 0 ≤ change
  ∧ change ≤ (2 ^ 64 - 1) -/
def bal_write_storage_read_groups (account : item_index) (read_count : item_count) (change_count : item_count) (read : item_index) (change : item_index) : SailM Unit := do
  let account := (account).value
  let read_count := (read_count).value
  let change_count := (change_count).value
  let read := (read).value
  let change := (change).value
  let _measure := ((read_count -i read) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_write_storage_read_groups ⟨account⟩ ⟨read_count⟩ ⟨change_count⟩ ⟨read⟩
      ⟨change⟩ (_measure + 1))

/-- Sizes the storage-read list for one account. -/
/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_storage_reads_size (account : item_index) : SailM BalContentCount := do
  let account := (account).value
  (bal_storage_read_groups_size ⟨account⟩
    ⟨((← (bal_storage_read_count ⟨account⟩))).value⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨0⟩
    { content_len := ⟨(BAL_RLP_ZERO).value⟩,
      count := ⟨0⟩ })

/-- Writes the storage-read list for one account. -/
/- Type quantifiers: k_ex411542_ : Nat, 0 ≤ k_ex411542_ ∧ k_ex411542_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_reads (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨(size.content_len).value⟩))
  (bal_write_storage_read_groups ⟨account⟩
    ⟨((← (bal_storage_read_count ⟨account⟩))).value⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨0⟩)

/-- Finds the end of a run of equal balance-change indices. -/
/- Type quantifiers: _reclimit : Nat, k_ex411546_ : Nat, k_ex411545_ : Nat, k_ex411544_ : Nat, k_ex411543_
  : Nat, 0 ≤ k_ex411543_ ∧ k_ex411543_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411544_ ∧
  k_ex411544_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411545_ ∧ k_ex411545_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411546_ ∧ k_ex411546_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_balance_run_end (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            if ((((← (bal_balance_change_index ⟨account⟩ ⟨cursor⟩))).value == index) : Bool)
            then
              (do
                  let publicResult ← (_rec_bal_balance_run_end ⟨account⟩ ⟨count⟩
                  ⟨index⟩ ⟨(cursor + 1)⟩ _reclimit_pred)
                  pure ((publicResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds the end of a run of equal balance-change indices. -/
/- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
def bal_balance_run_end (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_balance_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
          ⟨cursor⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Sizes the final balance value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex411556_ : Nat, k_ex411555_ : Nat, k_ex411554_ : Nat, k_ex411553_
  : Nat, 0 ≤ k_ex411553_ ∧ k_ex411553_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411554_ ∧
  k_ex411554_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411555_ ∧ k_ex411555_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411556_ ∧ k_ex411556_ ≤ (2 ^ 30), 0 ≤ _reclimit -/
def _rec_bal_balance_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : bal_rlp_length) (_reclimit : Nat) : SailM bal_rlp_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            let index ← do
              (do
                  let publicResult ← (bal_balance_change_index ⟨account⟩ ⟨cursor⟩)
                  pure ((publicResult).value))
            let next ← do
              (do
                  let publicResult ← (bal_balance_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                  ⟨cursor⟩)
                  pure ((publicResult).value))
            let last ← do
              (do
                  let publicResult ← (bal_previous_index ⟨next⟩)
                  pure ((publicResult).value))
            let next_length ← do
              (do
                  let publicResult ← (bal_rlp_length_add ⟨content_len⟩
                  ((← (bal_index_word_size ⟨index⟩
                    ⟨((← (bal_balance_change_value ⟨account⟩ ⟨last⟩))).value⟩))).value)
                  pure ((publicResult).value))
            (do
                let publicResult ← (_rec_bal_balance_groups_size ⟨account⟩ ⟨count⟩
                ⟨next⟩ ⟨next_length⟩ _reclimit_pred)
                pure ((publicResult).value)))
        else (pure content_len))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the final balance value for each change index. -/
/- Type quantifiers: content_len : Nat, cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1), 0 ≤ content_len ∧ content_len ≤ (2 ^ 30) -/
def bal_balance_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : bal_rlp_length) : SailM bal_rlp_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_balance_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩
          ⟨content_len⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Writes the final balance value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex411565_ : Nat, k_ex411564_ : Nat, k_ex411563_ : Nat, 0 ≤
  k_ex411563_ ∧ k_ex411563_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411564_ ∧ k_ex411564_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex411565_ ∧ k_ex411565_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_balance_groups (account : item_index) (count : item_count) (cursor : item_index) (_reclimit : Nat) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          let index ← do
            (do
                let publicResult ← (bal_balance_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          let next ← do
            (do
                let publicResult ← (bal_balance_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨cursor⟩)
                pure ((publicResult).value))
          let last ← do
            (do
                let publicResult ← (bal_previous_index ⟨next⟩)
                pure ((publicResult).value))
          (bal_write_index_word ⟨index⟩
            ⟨((← (bal_balance_change_value ⟨account⟩ ⟨last⟩))).value⟩)
          (_rec_bal_write_balance_groups ⟨account⟩ ⟨count⟩ ⟨next⟩ _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes the final balance value for each change index. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_write_balance_groups (account : item_index) (count : item_count) (cursor : item_index) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_balance_groups ⟨account⟩ ⟨count⟩ ⟨cursor⟩ (_measure + 1))

/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_balance_changes_size (account : item_index) : SailM bal_rlp_length := do
  let account := (account).value
  let publicResult ← do
    (do
        let publicResult ← (bal_balance_groups_size ⟨account⟩
        ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩
        ⟨(BAL_RLP_ZERO).value⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Writes the balance-change list for one account. -/
/- Type quantifiers: k_ex411573_ : Nat, k_ex411572_ : Nat, 0 ≤ k_ex411572_ ∧
  k_ex411572_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411573_ ∧ k_ex411573_ ≤ (2 ^ 30) -/
def bal_write_balance_changes (account : item_index) (content_len : bal_rlp_length) : SailM Unit := do
  let account := (account).value
  let content_len := (content_len).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨content_len⟩))
  (bal_write_balance_groups ⟨account⟩
    ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Finds one nonce-change run and its greatest observed nonce. -/
/- Type quantifiers: _reclimit : Nat, k_ex411578_ : Nat, k_ex411577_ : Nat, k_ex411576_ : Nat, k_ex411575_
  : Nat, k_ex411574_ : Nat, 0 ≤ k_ex411574_ ∧ k_ex411574_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411575_
  ∧ k_ex411575_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411576_ ∧ k_ex411576_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411577_ ∧ k_ex411577_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411578_ ∧ k_ex411578_ ≤ (2 ^ 64 - 1), 0
  ≤ _reclimit -/
def _rec_bal_nonce_run (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) (maximum : account_nonce) (_reclimit : Nat) : SailM BalNonceRun := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let maximum := (maximum).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          if ((((← (bal_nonce_change_index ⟨account⟩ ⟨cursor⟩))).value == index) : Bool)
          then
            (do
              let value ← do
                (do
                    let publicResult ← (bal_nonce_change_value ⟨account⟩ ⟨cursor⟩)
                    pure ((publicResult).value))
              let next_maximum :=
                if ((maximum <b value) : Bool)
                then value
                else maximum
              (_rec_bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩ ⟨(cursor + 1)⟩
                ⟨next_maximum⟩ _reclimit_pred))
          else
            (pure { cursor := ⟨cursor⟩,
                    maximum := ⟨maximum⟩ }))
      else
        (pure { cursor := ⟨cursor⟩,
                maximum := ⟨maximum⟩ }))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds one nonce-change run and its greatest observed nonce. -/
/- Type quantifiers: maximum : Nat, cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤
  account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1), 0 ≤ maximum ∧
  maximum ≤ (2 ^ 64 - 1) -/
def bal_nonce_run (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) (maximum : account_nonce) : SailM BalNonceRun := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let maximum := (maximum).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩ ⟨cursor⟩ ⟨maximum⟩
      (_measure + 1))

/-- Sizes the maximum nonce value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex411589_ : Nat, k_ex411588_ : Nat, k_ex411587_ : Nat, k_ex411586_
  : Nat, 0 ≤ k_ex411586_ ∧ k_ex411586_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411587_ ∧
  k_ex411587_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411588_ ∧ k_ex411588_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411589_ ∧ k_ex411589_ ≤ (2 ^ 30), 0 ≤ _reclimit -/
def _rec_bal_nonce_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : bal_rlp_length) (_reclimit : Nat) : SailM bal_rlp_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            let index ← do
              (do
                  let publicResult ← (bal_nonce_change_index ⟨account⟩ ⟨cursor⟩)
                  pure ((publicResult).value))
            let first ← do
              (do
                  let publicResult ← (bal_nonce_change_value ⟨account⟩ ⟨cursor⟩)
                  pure ((publicResult).value))
            let run ← do
              (bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩ ⟨(cursor + 1)⟩ ⟨first⟩)
            let next_length ← do
              (do
                  let publicResult ← (bal_rlp_length_add ⟨content_len⟩
                  ((← (bal_index_nonce_size ⟨index⟩ ⟨(run.maximum).value⟩))).value)
                  pure ((publicResult).value))
            (do
                let publicResult ← (_rec_bal_nonce_groups_size ⟨account⟩ ⟨count⟩
                ⟨(run.cursor).value⟩ ⟨next_length⟩ _reclimit_pred)
                pure ((publicResult).value)))
        else (pure content_len))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the maximum nonce value for each change index. -/
/- Type quantifiers: content_len : Nat, cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1), 0 ≤ content_len ∧ content_len ≤ (2 ^ 30) -/
def bal_nonce_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : bal_rlp_length) : SailM bal_rlp_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_nonce_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩
          ⟨content_len⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Writes the maximum nonce value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex411598_ : Nat, k_ex411597_ : Nat, k_ex411596_ : Nat, 0 ≤
  k_ex411596_ ∧ k_ex411596_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411597_ ∧ k_ex411597_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex411598_ ∧ k_ex411598_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_nonce_groups (account : item_index) (count : item_count) (cursor : item_index) (_reclimit : Nat) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          let index ← do
            (do
                let publicResult ← (bal_nonce_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          let first ← do
            (do
                let publicResult ← (bal_nonce_change_value ⟨account⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          let run ← do
            (bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩ ⟨(cursor + 1)⟩ ⟨first⟩)
          (bal_write_index_nonce ⟨index⟩ ⟨(run.maximum).value⟩)
          (_rec_bal_write_nonce_groups ⟨account⟩ ⟨count⟩ ⟨(run.cursor).value⟩
            _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes the maximum nonce value for each change index. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_write_nonce_groups (account : item_index) (count : item_count) (cursor : item_index) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_nonce_groups ⟨account⟩ ⟨count⟩ ⟨cursor⟩ (_measure + 1))

/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_nonce_changes_size (account : item_index) : SailM bal_rlp_length := do
  let account := (account).value
  let publicResult ← do
    (do
        let publicResult ← (bal_nonce_groups_size ⟨account⟩
        ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩ ⟨0⟩
        ⟨(BAL_RLP_ZERO).value⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Writes the nonce-change list for one account. -/
/- Type quantifiers: k_ex411606_ : Nat, k_ex411605_ : Nat, 0 ≤ k_ex411605_ ∧
  k_ex411605_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411606_ ∧ k_ex411606_ ≤ (2 ^ 30) -/
def bal_write_nonce_changes (account : item_index) (content_len : bal_rlp_length) : SailM Unit := do
  let account := (account).value
  let content_len := (content_len).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨content_len⟩))
  (bal_write_nonce_groups ⟨account⟩ ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Finds the end of a run of equal code-change indices. -/
/- Type quantifiers: _reclimit : Nat, k_ex411610_ : Nat, k_ex411609_ : Nat, k_ex411608_ : Nat, k_ex411607_
  : Nat, 0 ≤ k_ex411607_ ∧ k_ex411607_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411608_ ∧
  k_ex411608_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411609_ ∧ k_ex411609_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411610_ ∧ k_ex411610_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_code_run_end (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            if ((((← (bal_code_change_index ⟨account⟩ ⟨cursor⟩))).value == index) : Bool)
            then
              (do
                  let publicResult ← (_rec_bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                  ⟨(cursor + 1)⟩ _reclimit_pred)
                  pure ((publicResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds the end of a run of equal code-change indices. -/
/- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
def bal_code_run_end (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
          ⟨cursor⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Sizes the final code value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex411620_ : Nat, k_ex411619_ : Nat, k_ex411618_ : Nat, k_ex411617_
  : Nat, 0 ≤ k_ex411617_ ∧ k_ex411617_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411618_ ∧
  k_ex411618_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411619_ ∧ k_ex411619_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex411620_ ∧ k_ex411620_ ≤ (2 ^ 30), 0 ≤ _reclimit -/
def _rec_bal_code_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : bal_rlp_length) (_reclimit : Nat) : SailM bal_rlp_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let publicResult ← do
    match _reclimit with
    | 0 =>
      (do
        assert false "recursion limit reached"
        throw Error.Exit)
    | _reclimit_pred + 1 =>
      (do
        if ((cursor <b count) : Bool)
        then
          (do
            let index ← do
              (do
                  let publicResult ← (bal_code_change_index ⟨account⟩ ⟨cursor⟩)
                  pure ((publicResult).value))
            let next ← do
              (do
                  let publicResult ← (bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                  ⟨cursor⟩)
                  pure ((publicResult).value))
            let last ← do
              (do
                  let publicResult ← (bal_previous_index ⟨next⟩)
                  pure ((publicResult).value))
            let next_length ← do
              (do
                  let publicResult ← (bal_rlp_length_add ⟨content_len⟩
                  ((← (bal_index_code_size ⟨index⟩
                    (← (bal_code_change_hash ⟨account⟩ ⟨last⟩))))).value)
                  pure ((publicResult).value))
            (do
                let publicResult ← (_rec_bal_code_groups_size ⟨account⟩ ⟨count⟩ ⟨next⟩
                ⟨next_length⟩ _reclimit_pred)
                pure ((publicResult).value)))
        else (pure content_len))
  pure (⟨publicResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the final code value for each change index. -/
/- Type quantifiers: content_len : Nat, cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1), 0 ≤ content_len ∧ content_len ≤ (2 ^ 30) -/
def bal_code_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : bal_rlp_length) : SailM bal_rlp_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let content_len := (content_len).value
  let publicResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let publicResult ← (_rec_bal_code_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩
          ⟨content_len⟩ (_measure + 1))
          pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Writes the final code value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex411629_ : Nat, k_ex411628_ : Nat, k_ex411627_ : Nat, 0 ≤
  k_ex411627_ ∧ k_ex411627_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411628_ ∧ k_ex411628_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex411629_ ∧ k_ex411629_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_code_groups (account : item_index) (count : item_count) (cursor : item_index) (_reclimit : Nat) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((cursor <b count) : Bool)
      then
        (do
          let index ← do
            (do
                let publicResult ← (bal_code_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((publicResult).value))
          let next ← do
            (do
                let publicResult ← (bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨cursor⟩)
                pure ((publicResult).value))
          let last ← do
            (do
                let publicResult ← (bal_previous_index ⟨next⟩)
                pure ((publicResult).value))
          (bal_write_index_code ⟨index⟩ (← (bal_code_change_hash ⟨account⟩ ⟨last⟩)))
          (_rec_bal_write_code_groups ⟨account⟩ ⟨count⟩ ⟨next⟩ _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes the final code value for each change index. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_write_code_groups (account : item_index) (count : item_count) (cursor : item_index) : SailM Unit := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_code_groups ⟨account⟩ ⟨count⟩ ⟨cursor⟩ (_measure + 1))

/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_code_changes_size (account : item_index) : SailM bal_rlp_length := do
  let account := (account).value
  let publicResult ← do
    (do
        let publicResult ← (bal_code_groups_size ⟨account⟩
        ⟨((← (bal_code_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨(BAL_RLP_ZERO).value⟩)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Writes the code-change list for one account. -/
/- Type quantifiers: k_ex411637_ : Nat, k_ex411636_ : Nat, 0 ≤ k_ex411636_ ∧
  k_ex411636_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex411637_ ∧ k_ex411637_ ≤ (2 ^ 30) -/
def bal_write_code_changes (account : item_index) (content_len : bal_rlp_length) : SailM Unit := do
  let account := (account).value
  let content_len := (content_len).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨content_len⟩))
  (bal_write_code_groups ⟨account⟩ ⟨((← (bal_code_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Sizes all BAL fields contributed by one account. -/
/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_account_size (account : item_index) : SailM BalAccountSize := do
  let account := (account).value
  let storage_changes ← do (bal_storage_changes_size ⟨account⟩)
  let storage_reads ← do (bal_storage_reads_size ⟨account⟩)
  let balance_changes_len ← do
    (do
        let publicResult ← (bal_balance_changes_size ⟨account⟩)
        pure ((publicResult).value))
  let nonce_changes_len ← do
    (do
        let publicResult ← (bal_nonce_changes_size ⟨account⟩)
        pure ((publicResult).value))
  let code_changes_len ← do
    (do
        let publicResult ← (bal_code_changes_size ⟨account⟩)
        pure ((publicResult).value))
  let content_len ← do
    (do
        let publicResult ← (bal_rlp_length_from_byte_length (rlp_addr_size ()))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨(storage_changes.content_len).value⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨(storage_reads.content_len).value⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨balance_changes_len⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨nonce_changes_len⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨code_changes_len⟩))).value⟩))
        pure ((publicResult).value))
  (pure { encoded_len := ← do
              let publicField ← (do
                  let publicResult ← (bal_rlp_list_size ⟨content_len⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          item_count := ← do
              let publicField ← (bal_count_add
                (← (bal_count_add 1 (storage_changes.count).value)) (storage_reads.count).value)
              pure (⟨publicField⟩) })

/-- Writes one account and all of its ordered BAL change lists. -/
/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_account (account : item_index) : SailM Unit := do
  let account := (account).value
  let storage_changes ← do (bal_storage_changes_size ⟨account⟩)
  let storage_reads ← do (bal_storage_reads_size ⟨account⟩)
  let balance_changes_len ← do
    (do
        let publicResult ← (bal_balance_changes_size ⟨account⟩)
        pure ((publicResult).value))
  let nonce_changes_len ← do
    (do
        let publicResult ← (bal_nonce_changes_size ⟨account⟩)
        pure ((publicResult).value))
  let code_changes_len ← do
    (do
        let publicResult ← (bal_code_changes_size ⟨account⟩)
        pure ((publicResult).value))
  let content_len ← do
    (do
        let publicResult ← (bal_rlp_length_from_byte_length (rlp_addr_size ()))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨(storage_changes.content_len).value⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨(storage_reads.content_len).value⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨balance_changes_len⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨nonce_changes_len⟩))).value⟩))
        pure ((publicResult).value))
  let content_len ←
    (do
        let publicResult ← (bal_rlp_length_add ⟨content_len⟩
        (bal_rlp_length_to_byte_length
          ⟨((← (bal_rlp_list_size ⟨code_changes_len⟩))).value⟩))
        pure ((publicResult).value))
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨content_len⟩))
  (rlp_write_addr (← (bal_account_address ⟨account⟩)))
  (bal_write_storage_changes ⟨account⟩ storage_changes)
  (bal_write_storage_reads ⟨account⟩ storage_reads)
  (bal_write_balance_changes ⟨account⟩ ⟨balance_changes_len⟩)
  (bal_write_nonce_changes ⟨account⟩ ⟨nonce_changes_len⟩)
  (bal_write_code_changes ⟨account⟩ ⟨code_changes_len⟩)

/- Type quantifiers: _reclimit : Nat, account_count : Nat, account : Nat, 0 ≤ account, 0 ≤
  account_count ∧ account_count ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_accounts_size (account_count : item_count) (account : Nat) (result : BalContentCount) (_reclimit : Nat) : SailM BalContentCount := do
  let account_count := (account_count).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((account <b account_count) : Bool)
      then
        (do
          let account_size ← do (bal_account_size ⟨account⟩)
          let next_length ← do
            (do
                let publicResult ← (bal_rlp_length_add ⟨(result.content_len).value⟩
                (bal_rlp_length_to_byte_length ⟨(account_size.encoded_len).value⟩))
                pure ((publicResult).value))
          (_rec_bal_accounts_size ⟨account_count⟩ (account + 1)
            { content_len := ⟨next_length⟩,
              count := ← do
                  let publicField ← (bal_count_add (result.count).value
                    (account_size.item_count).value)
                  pure (⟨publicField⟩) } _reclimit_pred))
      else (pure result))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: account_count : Nat, account : Nat, 0 ≤ account, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1) -/
def bal_accounts_size (account_count : item_count) (account : Nat) (result : BalContentCount) : SailM BalContentCount := do
  let account_count := (account_count).value
  let _measure := ((account_count -i account) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_accounts_size ⟨account_count⟩ account result (_measure + 1))

/- Type quantifiers: _reclimit : Nat, account_count : Nat, account : Nat, 0 ≤ account, 0 ≤
  account_count ∧ account_count ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_accounts (account_count : item_count) (account : Nat) (_reclimit : Nat) : SailM Unit := do
  let account_count := (account_count).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((account <b account_count) : Bool)
      then
        (do
          (bal_write_account ⟨account⟩)
          (_rec_bal_write_accounts ⟨account_count⟩ (account + 1) _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/- Type quantifiers: account_count : Nat, account : Nat, 0 ≤ account, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1) -/
def bal_write_accounts (account_count : item_count) (account : Nat) : SailM Unit := do
  let account_count := (account_count).value
  let _measure := ((account_count -i account) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_accounts ⟨account_count⟩ account (_measure + 1))

/-- Reconstructs the canonical EIP-7928 block access list from the
recorder: per account (address order) — storage changes, storage
reads, balance, nonce, and code changes — counting items against the
`gas_limit / 2000` bound. -/
def encode_block_access_list (_ : Unit) : SailM EncodedBlockAccessList := do
  (bal_prepare ())
  let account_count ← do
    (do
        let publicResult ← (bal_account_count ())
        pure ((publicResult).value))
  let size ← do
    (bal_accounts_size ⟨account_count⟩ 0
      { content_len := ⟨(BAL_RLP_ZERO).value⟩,
        count := ⟨0⟩ })
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨(size.content_len).value⟩))
  (bal_write_accounts ⟨account_count⟩ 0)
  (pure { bytes := ← do
              let publicField ← (rlp_finish start)
              pure (publicField),
          item_count := ⟨(size.count).value⟩ })

