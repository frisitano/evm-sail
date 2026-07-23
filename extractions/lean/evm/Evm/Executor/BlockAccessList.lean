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

/- Type quantifiers: k_ex409151_ : Nat, k_ex409150_ : Nat, 0 ≤ k_ex409150_ ∧
  k_ex409150_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409151_ ∧ k_ex409151_ ≤ (2 ^ 256 - 1) -/
def bal_index_word_content_size (index : item_index) (value : word) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_byte_length_add (rlp_uint_word_size index) (rlp_uint_word_size value))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409153_ : Nat, k_ex409152_ : Nat, 0 ≤ k_ex409152_ ∧
  k_ex409152_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409153_ ∧ k_ex409153_ ≤ (2 ^ 256 - 1) -/
def bal_index_word_size (index : item_index) (value : word) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_list_size ((← (bal_index_word_content_size ⟨index⟩ ⟨value⟩))).value)
  pure (⟨publicResult⟩)

/-- Writes one `[index, word]` BAL pair. -/
/- Type quantifiers: k_ex409155_ : Nat, k_ex409154_ : Nat, 0 ≤ k_ex409154_ ∧
  k_ex409154_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409155_ ∧ k_ex409155_ ≤ (2 ^ 256 - 1) -/
def bal_write_index_word (index : item_index) (value : word) : SailM Unit := do
  let index := (index).value
  let value := (value).value
  (rlp_write_list_prefix ((← (bal_index_word_content_size ⟨index⟩ ⟨value⟩))).value)
  (rlp_write_uint_word index)
  (rlp_write_uint_word value)

/- Type quantifiers: k_ex409157_ : Nat, k_ex409156_ : Nat, 0 ≤ k_ex409156_ ∧
  k_ex409156_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409157_ ∧ k_ex409157_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_content_size (index : item_index) (value : account_nonce) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_byte_length_add (rlp_uint_word_size index) (rlp_uint_word_size value))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409159_ : Nat, k_ex409158_ : Nat, 0 ≤ k_ex409158_ ∧
  k_ex409158_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409159_ ∧ k_ex409159_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_size (index : item_index) (value : account_nonce) : SailM bal_rlp_length := do
  let index := (index).value
  let value := (value).value
  let publicResult ← do
    (bal_bounded_list_size ((← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩))).value)
  pure (⟨publicResult⟩)

/-- Writes one `[index, nonce]` BAL pair. -/
/- Type quantifiers: k_ex409161_ : Nat, k_ex409160_ : Nat, 0 ≤ k_ex409160_ ∧
  k_ex409160_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409161_ ∧ k_ex409161_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_nonce (index : item_index) (value : account_nonce) : SailM Unit := do
  let index := (index).value
  let value := (value).value
  (rlp_write_list_prefix ((← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩))).value)
  (rlp_write_uint_word index)
  (rlp_write_uint_word value)

/-- Sizes the content of an index-and-code pair. -/
/- Type quantifiers: k_ex409162_ : Nat, 0 ≤ k_ex409162_ ∧ k_ex409162_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_content_size (index : item_index) (code_hash : hash) : SailM bal_rlp_length := do
  let index := (index).value
  let publicResult ← do
    let code ← do (code_db_resolve code_hash)
    let index_length := (rlp_uint_word_size index)
    let code_length ← do (rlp_slice_size ((code.bytes).2).2)
    (bal_bounded_byte_length_add index_length code_length)
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409163_ : Nat, 0 ≤ k_ex409163_ ∧ k_ex409163_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_size (index : item_index) (code_hash : hash) : SailM bal_rlp_length := do
  let index := (index).value
  let publicResult ← do
    (bal_bounded_list_size ((← (bal_index_code_content_size ⟨index⟩ code_hash))).value)
  pure (⟨publicResult⟩)

/-- Writes one `[index, code]` BAL pair after resolving the code hash. -/
/- Type quantifiers: k_ex409164_ : Nat, 0 ≤ k_ex409164_ ∧ k_ex409164_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_code (index : item_index) (code_hash : hash) : SailM Unit := do
  let index := (index).value
  let code ← do (code_db_resolve code_hash)
  (rlp_write_list_prefix ((← (bal_index_code_content_size ⟨index⟩ code_hash))).value)
  (rlp_write_uint_word index)
  (rlp_write_slice
    (⟨_, ⟨_, ((code.bytes).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Finds the end of an equal-slot, equal-index storage-change run. -/
/- Type quantifiers: _reclimit : Nat, k_ex409169_ : Nat, k_ex409168_ : Nat, k_ex409167_ : Nat, k_ex409166_
  : Nat, k_ex409165_ : Nat, 0 ≤ k_ex409165_ ∧ k_ex409165_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409166_
  ∧ k_ex409166_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409167_ ∧ k_ex409167_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex409168_ ∧ k_ex409168_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409169_ ∧ k_ex409169_ ≤ (2 ^ 64 - 1), 0
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
/- Type quantifiers: _reclimit : Nat, k_ex409181_ : Nat, k_ex409180_ : Nat, k_ex409179_ : Nat, k_ex409178_
  : Nat, k_ex409177_ : Nat, 0 ≤ k_ex409177_ ∧ k_ex409177_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409178_
  ∧ k_ex409178_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409179_ ∧ k_ex409179_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex409180_ ∧ k_ex409180_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409181_ ∧ k_ex409181_ ≤ (2 ^ 30), 0
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
/- Type quantifiers: _reclimit : Nat, k_ex409192_ : Nat, k_ex409191_ : Nat, k_ex409190_ : Nat, k_ex409189_
  : Nat, 0 ≤ k_ex409189_ ∧ k_ex409189_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409190_ ∧
  k_ex409190_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409191_ ∧ k_ex409191_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex409192_ ∧ k_ex409192_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409201_ : Nat, k_ex409200_ : Nat, k_ex409199_ : Nat, 0 ≤
  k_ex409199_ ∧ k_ex409199_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409200_ ∧ k_ex409200_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex409201_ ∧ k_ex409201_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409209_ : Nat, k_ex409208_ : Nat, k_ex409207_ : Nat, 0 ≤
  k_ex409207_ ∧ k_ex409207_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409208_ ∧ k_ex409208_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex409209_ ∧ k_ex409209_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: k_ex409216_ : Nat, 0 ≤ k_ex409216_ ∧ k_ex409216_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_changes (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨(size.content_len).value⟩))
  (bal_write_storage_change_groups ⟨account⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Finds the end of a run of equal storage-read slots. -/
/- Type quantifiers: _reclimit : Nat, k_ex409220_ : Nat, k_ex409219_ : Nat, k_ex409218_ : Nat, k_ex409217_
  : Nat, 0 ≤ k_ex409217_ ∧ k_ex409217_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409218_ ∧
  k_ex409218_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409219_ ∧ k_ex409219_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex409220_ ∧ k_ex409220_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409230_ : Nat, k_ex409229_ : Nat, k_ex409228_ : Nat, k_ex409227_
  : Nat, 0 ≤ k_ex409227_ ∧ k_ex409227_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409228_ ∧
  k_ex409228_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409229_ ∧ k_ex409229_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex409230_ ∧ k_ex409230_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409241_ : Nat, k_ex409240_ : Nat, k_ex409239_ : Nat, k_ex409238_
  : Nat, k_ex409237_ : Nat, 0 ≤ k_ex409237_ ∧ k_ex409237_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409238_
  ∧ k_ex409238_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409239_ ∧ k_ex409239_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409240_ ∧ k_ex409240_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409241_ ∧ k_ex409241_ ≤ (2 ^ 64 - 1), 0
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
/- Type quantifiers: _reclimit : Nat, k_ex409253_ : Nat, k_ex409252_ : Nat, k_ex409251_ : Nat, k_ex409250_
  : Nat, k_ex409249_ : Nat, 0 ≤ k_ex409249_ ∧ k_ex409249_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409250_
  ∧ k_ex409250_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409251_ ∧ k_ex409251_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409252_ ∧ k_ex409252_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409253_ ∧ k_ex409253_ ≤ (2 ^ 64 - 1), 0
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
/- Type quantifiers: k_ex409262_ : Nat, 0 ≤ k_ex409262_ ∧ k_ex409262_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_reads (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨(size.content_len).value⟩))
  (bal_write_storage_read_groups ⟨account⟩
    ⟨((← (bal_storage_read_count ⟨account⟩))).value⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨0⟩)

/-- Finds the end of a run of equal balance-change indices. -/
/- Type quantifiers: _reclimit : Nat, k_ex409266_ : Nat, k_ex409265_ : Nat, k_ex409264_ : Nat, k_ex409263_
  : Nat, 0 ≤ k_ex409263_ ∧ k_ex409263_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409264_ ∧
  k_ex409264_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409265_ ∧ k_ex409265_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409266_ ∧ k_ex409266_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409276_ : Nat, k_ex409275_ : Nat, k_ex409274_ : Nat, k_ex409273_
  : Nat, 0 ≤ k_ex409273_ ∧ k_ex409273_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409274_ ∧
  k_ex409274_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409275_ ∧ k_ex409275_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409276_ ∧ k_ex409276_ ≤ (2 ^ 30), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409285_ : Nat, k_ex409284_ : Nat, k_ex409283_ : Nat, 0 ≤
  k_ex409283_ ∧ k_ex409283_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409284_ ∧ k_ex409284_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex409285_ ∧ k_ex409285_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: k_ex409293_ : Nat, k_ex409292_ : Nat, 0 ≤ k_ex409292_ ∧
  k_ex409292_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409293_ ∧ k_ex409293_ ≤ (2 ^ 30) -/
def bal_write_balance_changes (account : item_index) (content_len : bal_rlp_length) : SailM Unit := do
  let account := (account).value
  let content_len := (content_len).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨content_len⟩))
  (bal_write_balance_groups ⟨account⟩
    ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Finds one nonce-change run and its greatest observed nonce. -/
/- Type quantifiers: _reclimit : Nat, k_ex409298_ : Nat, k_ex409297_ : Nat, k_ex409296_ : Nat, k_ex409295_
  : Nat, k_ex409294_ : Nat, 0 ≤ k_ex409294_ ∧ k_ex409294_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409295_
  ∧ k_ex409295_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409296_ ∧ k_ex409296_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409297_ ∧ k_ex409297_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409298_ ∧ k_ex409298_ ≤ (2 ^ 64 - 1), 0
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
/- Type quantifiers: _reclimit : Nat, k_ex409309_ : Nat, k_ex409308_ : Nat, k_ex409307_ : Nat, k_ex409306_
  : Nat, 0 ≤ k_ex409306_ ∧ k_ex409306_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409307_ ∧
  k_ex409307_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409308_ ∧ k_ex409308_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409309_ ∧ k_ex409309_ ≤ (2 ^ 30), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409318_ : Nat, k_ex409317_ : Nat, k_ex409316_ : Nat, 0 ≤
  k_ex409316_ ∧ k_ex409316_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409317_ ∧ k_ex409317_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex409318_ ∧ k_ex409318_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: k_ex409326_ : Nat, k_ex409325_ : Nat, 0 ≤ k_ex409325_ ∧
  k_ex409325_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409326_ ∧ k_ex409326_ ≤ (2 ^ 30) -/
def bal_write_nonce_changes (account : item_index) (content_len : bal_rlp_length) : SailM Unit := do
  let account := (account).value
  let content_len := (content_len).value
  (rlp_write_list_prefix (bal_rlp_length_to_byte_length ⟨content_len⟩))
  (bal_write_nonce_groups ⟨account⟩ ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Finds the end of a run of equal code-change indices. -/
/- Type quantifiers: _reclimit : Nat, k_ex409330_ : Nat, k_ex409329_ : Nat, k_ex409328_ : Nat, k_ex409327_
  : Nat, 0 ≤ k_ex409327_ ∧ k_ex409327_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409328_ ∧
  k_ex409328_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409329_ ∧ k_ex409329_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409330_ ∧ k_ex409330_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409340_ : Nat, k_ex409339_ : Nat, k_ex409338_ : Nat, k_ex409337_
  : Nat, 0 ≤ k_ex409337_ ∧ k_ex409337_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409338_ ∧
  k_ex409338_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409339_ ∧ k_ex409339_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex409340_ ∧ k_ex409340_ ≤ (2 ^ 30), 0 ≤ _reclimit -/
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
/- Type quantifiers: _reclimit : Nat, k_ex409349_ : Nat, k_ex409348_ : Nat, k_ex409347_ : Nat, 0 ≤
  k_ex409347_ ∧ k_ex409347_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409348_ ∧ k_ex409348_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex409349_ ∧ k_ex409349_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
/- Type quantifiers: k_ex409357_ : Nat, k_ex409356_ : Nat, 0 ≤ k_ex409356_ ∧
  k_ex409356_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex409357_ ∧ k_ex409357_ ≤ (2 ^ 30) -/
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

