import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

/-! # The block access list

Reconstruction of the EIP-7928 block access list from the recorder's
per-account change streams, as source-backed RLP that is compared against
the payload's claimed bytes.

## Types

The cursor records pair an encoded-content length with the ordered-table
position reached by a sizing pass, so the subsequent writer follows exactly
the same grouping boundaries. -/

def undefined_BalNonceRun (_ : Unit) : SailM BalNonceRun := do
  (pure { cursor := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          maximum := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩) })

/-- Adds logical BAL item counts and rejects protocol-quantity overflow. -/
/- Type quantifiers: k_ex161459_ : Nat, k_ex161458_ : Nat, 0 ≤ k_ex161458_ ∧
  k_ex161458_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161459_ ∧ k_ex161459_ ≤ (2 ^ 64 - 1) -/
def bal_count_add (a : item_count) (b : item_count) : SailM item_count := do
  let a := (a).value
  let b := (b).value
  let semanticResult ← do
    if ((b ≤b ((BYTE_QUANTITY_MAX).value -i a)) : Bool)
    then (pure (a + b))
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨semanticResult⟩)

/- Type quantifiers: k_ex161460_ : Nat, 0 ≤ k_ex161460_ ∧ k_ex161460_ ≤ (2 ^ 64 - 1) -/
def bal_index_word_content_size (index : item_index) (value : word) : SailM byte_length := do
  let index := (index).value
  (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩)) (← (rlp_uint_word_size value)))

/- Type quantifiers: k_ex161461_ : Nat, 0 ≤ k_ex161461_ ∧ k_ex161461_ ≤ (2 ^ 64 - 1) -/
def bal_index_word_size (index : item_index) (value : word) : SailM byte_length := do
  let index := (index).value
  (rlp_list_size (← (bal_index_word_content_size ⟨index⟩ value)))

/-- Writes one `[index, word]` BAL pair. -/
/- Type quantifiers: k_ex161462_ : Nat, 0 ≤ k_ex161462_ ∧ k_ex161462_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_word (index : item_index) (value : word) : SailM Unit := do
  let index := (index).value
  (rlp_write_list_prefix (← (bal_index_word_content_size ⟨index⟩ value)))
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_uint_word value)

/- Type quantifiers: k_ex161464_ : Nat, k_ex161463_ : Nat, 0 ≤ k_ex161463_ ∧
  k_ex161463_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161464_ ∧ k_ex161464_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_content_size (index : item_index) (value : account_nonce) : SailM byte_length := do
  let index := (index).value
  let value := (value).value
  (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩))
    (← (rlp_protocol_quantity_size ⟨value⟩)))

/- Type quantifiers: k_ex161466_ : Nat, k_ex161465_ : Nat, 0 ≤ k_ex161465_ ∧
  k_ex161465_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161466_ ∧ k_ex161466_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_size (index : item_index) (value : account_nonce) : SailM byte_length := do
  let index := (index).value
  let value := (value).value
  (rlp_list_size (← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩)))

/-- Writes one `[index, nonce]` BAL pair. -/
/- Type quantifiers: k_ex161468_ : Nat, k_ex161467_ : Nat, 0 ≤ k_ex161467_ ∧
  k_ex161467_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161468_ ∧ k_ex161468_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_nonce (index : item_index) (value : account_nonce) : SailM Unit := do
  let index := (index).value
  let value := (value).value
  (rlp_write_list_prefix (← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩)))
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_protocol_quantity ⟨value⟩)

/-- Sizes the content of an index-and-code pair. -/
/- Type quantifiers: k_ex161469_ : Nat, 0 ≤ k_ex161469_ ∧ k_ex161469_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_content_size (index : item_index) (code_hash : hash) : SailM byte_length := do
  let index := (index).value
  let code ← do (code_db_resolve code_hash)
  (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩))
    (← (rlp_slice_size code.bytes)))

/- Type quantifiers: k_ex161470_ : Nat, 0 ≤ k_ex161470_ ∧ k_ex161470_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_size (index : item_index) (code_hash : hash) : SailM byte_length := do
  let index := (index).value
  (rlp_list_size (← (bal_index_code_content_size ⟨index⟩ code_hash)))

/-- Writes one `[index, code]` BAL pair after resolving the code hash. -/
/- Type quantifiers: k_ex161471_ : Nat, 0 ≤ k_ex161471_ ∧ k_ex161471_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_code (index : item_index) (code_hash : hash) : SailM Unit := do
  let index := (index).value
  let code ← do (code_db_resolve code_hash)
  (rlp_write_list_prefix
    (← (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩))
        (← (rlp_slice_size code.bytes)))))
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_slice code.bytes)

/-- Finds the end of an equal-slot, equal-index storage-change run. -/
/- Type quantifiers: _reclimit : Nat, k_ex161475_ : Nat, k_ex161474_ : Nat, k_ex161473_ : Nat, k_ex161472_
  : Nat, 0 ≤ k_ex161472_ ∧ k_ex161472_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161473_ ∧
  k_ex161473_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161474_ ∧ k_ex161474_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161475_ ∧ k_ex161475_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_change_run_end (account : item_index) (count : item_count) (slot : word) (index : item_index) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let semanticResult ← do
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
            if ((((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)) == slot) && (← do
                   (pure (((← (bal_storage_change_index ⟨account⟩ ⟨cursor⟩))).value == index)))) : Bool)
            then
              (do
                  let semanticResult ← (_rec_bal_storage_change_run_end ⟨account⟩ ⟨count⟩
                  slot ⟨index⟩ ⟨((← (item_index_increment ⟨cursor⟩))).value⟩
                  _reclimit_pred)
                  pure ((semanticResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨semanticResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds the end of an equal-slot, equal-index storage-change run. -/
/- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_change_run_end (account : item_index) (count : item_count) (slot : word) (index : item_index) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let semanticResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let semanticResult ← (_rec_bal_storage_change_run_end ⟨account⟩ ⟨count⟩ slot
          ⟨index⟩ ⟨cursor⟩ (_measure + 1))
          pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Sizes the final values for all change indices of one storage slot. -/
/- Type quantifiers: _reclimit : Nat, k_ex161484_ : Nat, k_ex161483_ : Nat, k_ex161482_ : Nat, 0 ≤
  k_ex161482_ ∧ k_ex161482_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161483_ ∧ k_ex161483_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161484_ ∧ k_ex161484_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_slot_changes_size (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (content_len : byte_length) (_reclimit : Nat) : SailM BalContentCursor := do
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
          if (((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)) == slot) : Bool)
          then
            (do
              let index ← do
                (do
                    let semanticResult ← (bal_storage_change_index ⟨account⟩ ⟨cursor⟩)
                    pure ((semanticResult).value))
              let next ← do
                (do
                    let semanticResult ← (bal_storage_change_run_end ⟨account⟩ ⟨count⟩
                    slot ⟨index⟩ ⟨cursor⟩)
                    pure ((semanticResult).value))
              let last ← do
                (do
                    let semanticResult ← (protocol_quantity_decrement ⟨next⟩)
                    pure ((semanticResult).value))
              let next_len ← do
                (byte_quantity_add content_len
                  (← (bal_index_word_size ⟨index⟩
                      (← (bal_storage_change_value ⟨account⟩ ⟨last⟩)))))
              (_rec_bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ slot ⟨next⟩ next_len
                _reclimit_pred))
          else
            (pure { content_len := content_len,
                    cursor := ⟨cursor⟩ }))
      else
        (pure { content_len := content_len,
                cursor := ⟨cursor⟩ }))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the final values for all change indices of one storage slot. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_slot_changes_size (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (content_len : byte_length) : SailM BalContentCursor := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ slot ⟨cursor⟩ content_len
      (_measure + 1))

/-- Writes the final value for each change index of one storage slot. -/
/- Type quantifiers: _reclimit : Nat, k_ex161492_ : Nat, k_ex161491_ : Nat, k_ex161490_ : Nat, 0 ≤
  k_ex161490_ ∧ k_ex161490_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161491_ ∧ k_ex161491_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161492_ ∧ k_ex161492_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_storage_slot_changes (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let semanticResult ← do
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
            if (((← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)) == slot) : Bool)
            then
              (do
                let index ← do
                  (do
                      let semanticResult ← (bal_storage_change_index ⟨account⟩ ⟨cursor⟩)
                      pure ((semanticResult).value))
                let next ← do
                  (do
                      let semanticResult ← (bal_storage_change_run_end ⟨account⟩ ⟨count⟩
                      slot ⟨index⟩ ⟨cursor⟩)
                      pure ((semanticResult).value))
                let last ← do
                  (do
                      let semanticResult ← (protocol_quantity_decrement ⟨next⟩)
                      pure ((semanticResult).value))
                (bal_write_index_word ⟨index⟩
                  (← (bal_storage_change_value ⟨account⟩ ⟨last⟩)))
                (do
                    let semanticResult ← (_rec_bal_write_storage_slot_changes ⟨account⟩
                    ⟨count⟩ slot ⟨next⟩ _reclimit_pred)
                    pure ((semanticResult).value)))
            else (pure cursor))
        else (pure cursor))
  pure (⟨semanticResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes the final value for each change index of one storage slot. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_write_storage_slot_changes (account : item_index) (count : item_count) (slot : word) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let semanticResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let semanticResult ← (_rec_bal_write_storage_slot_changes ⟨account⟩ ⟨count⟩ slot
          ⟨cursor⟩ (_measure + 1))
          pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Sizes storage changes grouped by slot and counts the groups. -/
/- Type quantifiers: _reclimit : Nat, k_ex161500_ : Nat, k_ex161499_ : Nat, k_ex161498_ : Nat, 0 ≤
  k_ex161498_ ∧ k_ex161498_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161499_ ∧ k_ex161499_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161500_ ∧ k_ex161500_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
          let slot ← do (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)
          let changes ← do
            (bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ slot ⟨cursor⟩ BYTE_ZERO)
          let slot_content_len ← do
            (byte_quantity_add (← (rlp_uint_word_size slot))
              (← (rlp_list_size changes.content_len)))
          (_rec_bal_storage_change_groups_size ⟨account⟩ ⟨count⟩
            ⟨(changes.cursor).value⟩
            { content_len := ← (byte_quantity_add result.content_len
                  (← (rlp_list_size slot_content_len))),
              count := ← do
                  let semanticField ← (do
                      let semanticResult ← (bal_count_add ⟨(result.count).value⟩ ⟨1⟩)
                      pure ((semanticResult).value))
                  pure (⟨semanticField⟩) } _reclimit_pred))
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
/- Type quantifiers: _reclimit : Nat, k_ex161508_ : Nat, k_ex161507_ : Nat, k_ex161506_ : Nat, 0 ≤
  k_ex161506_ ∧ k_ex161506_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161507_ ∧ k_ex161507_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161508_ ∧ k_ex161508_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
          let slot ← do (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)
          let changes ← do
            (bal_storage_slot_changes_size ⟨account⟩ ⟨count⟩ slot ⟨cursor⟩ BYTE_ZERO)
          let slot_content_len ← do
            (byte_quantity_add (← (rlp_uint_word_size slot))
              (← (rlp_list_size changes.content_len)))
          (rlp_write_list_prefix slot_content_len)
          (rlp_write_uint_word slot)
          (rlp_write_list_prefix changes.content_len)
          let next ← do
            (do
                let semanticResult ← (bal_write_storage_slot_changes ⟨account⟩ ⟨count⟩
                slot ⟨cursor⟩)
                pure ((semanticResult).value))
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
    { content_len := BYTE_ZERO,
      count := ⟨0⟩ })

/-- Writes the storage-change list for one account. -/
/- Type quantifiers: k_ex161515_ : Nat, 0 ≤ k_ex161515_ ∧ k_ex161515_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_changes (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix size.content_len)
  (bal_write_storage_change_groups ⟨account⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Finds the end of a run of equal storage-read slots. -/
/- Type quantifiers: _reclimit : Nat, k_ex161518_ : Nat, k_ex161517_ : Nat, k_ex161516_ : Nat, 0 ≤
  k_ex161516_ ∧ k_ex161516_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161517_ ∧ k_ex161517_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161518_ ∧ k_ex161518_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_read_run_end (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let semanticResult ← do
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
            if (((← (bal_storage_read_slot ⟨account⟩ ⟨cursor⟩)) == slot) : Bool)
            then
              (do
                  let semanticResult ← (_rec_bal_storage_read_run_end ⟨account⟩ ⟨count⟩
                  slot ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ _reclimit_pred)
                  pure ((semanticResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨semanticResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Finds the end of a run of equal storage-read slots. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_read_run_end (account : item_index) (count : item_count) (slot : word) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let semanticResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let semanticResult ← (_rec_bal_storage_read_run_end ⟨account⟩ ⟨count⟩ slot
          ⟨cursor⟩ (_measure + 1))
          pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Advances the ordered change cursor to the first slot not below a read. -/
/- Type quantifiers: _reclimit : Nat, k_ex161526_ : Nat, k_ex161525_ : Nat, k_ex161524_ : Nat, 0 ≤
  k_ex161524_ ∧ k_ex161524_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161525_ ∧ k_ex161525_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161526_ ∧ k_ex161526_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_storage_change_seek (account : item_index) (count : item_count) (slot : word) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let semanticResult ← do
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
            if ((word_ult (← (bal_storage_change_slot ⟨account⟩ ⟨cursor⟩)) slot) : Bool)
            then
              (do
                  let semanticResult ← (_rec_bal_storage_change_seek ⟨account⟩ ⟨count⟩
                  slot ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ _reclimit_pred)
                  pure ((semanticResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨semanticResult⟩)
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Advances the ordered change cursor to the first slot not below a read. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_storage_change_seek (account : item_index) (count : item_count) (slot : word) (cursor : item_index) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let semanticResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let semanticResult ← (_rec_bal_storage_change_seek ⟨account⟩ ⟨count⟩ slot
          ⟨cursor⟩ (_measure + 1))
          pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Sizes read slots that are not also represented by a storage change. -/
/- Type quantifiers: _reclimit : Nat, k_ex161536_ : Nat, k_ex161535_ : Nat, k_ex161534_ : Nat, k_ex161533_
  : Nat, k_ex161532_ : Nat, 0 ≤ k_ex161532_ ∧ k_ex161532_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161533_
  ∧ k_ex161533_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161534_ ∧ k_ex161534_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161535_ ∧ k_ex161535_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161536_ ∧ k_ex161536_ ≤ (2 ^ 64 - 1), 0
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
          let slot ← do (bal_storage_read_slot ⟨account⟩ ⟨read⟩)
          let next_read ← do
            (do
                let semanticResult ← (bal_storage_read_run_end ⟨account⟩ ⟨read_count⟩ slot
                ⟨read⟩)
                pure ((semanticResult).value))
          let next_change ← do
            (do
                let semanticResult ← (bal_storage_change_seek ⟨account⟩ ⟨change_count⟩
                slot ⟨change⟩)
                pure ((semanticResult).value))
          let changed ← do
            if ((next_change <b change_count) : Bool)
            then (pure ((← (bal_storage_change_slot ⟨account⟩ ⟨next_change⟩)) == slot))
            else (pure false)
          let updated ← do
            if (changed : Bool)
            then (pure result)
            else
              (pure { content_len := ← (byte_quantity_add result.content_len
                          (← (rlp_uint_word_size slot))),
                      count := ← do
                          let semanticField ← (do
                              let semanticResult ← (bal_count_add ⟨(result.count).value⟩
                              ⟨1⟩)
                              pure ((semanticResult).value))
                          pure (⟨semanticField⟩) })
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
/- Type quantifiers: _reclimit : Nat, k_ex161548_ : Nat, k_ex161547_ : Nat, k_ex161546_ : Nat, k_ex161545_
  : Nat, k_ex161544_ : Nat, 0 ≤ k_ex161544_ ∧ k_ex161544_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161545_
  ∧ k_ex161545_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161546_ ∧ k_ex161546_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161547_ ∧ k_ex161547_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161548_ ∧ k_ex161548_ ≤ (2 ^ 64 - 1), 0
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
          let slot ← do (bal_storage_read_slot ⟨account⟩ ⟨read⟩)
          let next_read ← do
            (do
                let semanticResult ← (bal_storage_read_run_end ⟨account⟩ ⟨read_count⟩ slot
                ⟨read⟩)
                pure ((semanticResult).value))
          let next_change ← do
            (do
                let semanticResult ← (bal_storage_change_seek ⟨account⟩ ⟨change_count⟩
                slot ⟨change⟩)
                pure ((semanticResult).value))
          let changed ← do
            if ((next_change <b change_count) : Bool)
            then (pure ((← (bal_storage_change_slot ⟨account⟩ ⟨next_change⟩)) == slot))
            else (pure false)
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
    { content_len := BYTE_ZERO,
      count := ⟨0⟩ })

/-- Writes the storage-read list for one account. -/
/- Type quantifiers: k_ex161557_ : Nat, 0 ≤ k_ex161557_ ∧ k_ex161557_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_reads (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix size.content_len)
  (bal_write_storage_read_groups ⟨account⟩
    ⟨((← (bal_storage_read_count ⟨account⟩))).value⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨0⟩)

/-- Finds the end of a run of equal balance-change indices. -/
/- Type quantifiers: _reclimit : Nat, k_ex161561_ : Nat, k_ex161560_ : Nat, k_ex161559_ : Nat, k_ex161558_
  : Nat, 0 ≤ k_ex161558_ ∧ k_ex161558_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161559_ ∧
  k_ex161559_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161560_ ∧ k_ex161560_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161561_ ∧ k_ex161561_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_balance_run_end (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let semanticResult ← do
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
                  let semanticResult ← (_rec_bal_balance_run_end ⟨account⟩ ⟨count⟩
                  ⟨index⟩ ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ _reclimit_pred)
                  pure ((semanticResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨semanticResult⟩)
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
  let semanticResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let semanticResult ← (_rec_bal_balance_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
          ⟨cursor⟩ (_measure + 1))
          pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Sizes the final balance value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex161570_ : Nat, k_ex161569_ : Nat, k_ex161568_ : Nat, 0 ≤
  k_ex161568_ ∧ k_ex161568_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161569_ ∧ k_ex161569_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161570_ ∧ k_ex161570_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_balance_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : byte_length) (_reclimit : Nat) : SailM byte_length := do
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
                let semanticResult ← (bal_balance_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let next ← do
            (do
                let semanticResult ← (bal_balance_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨cursor⟩)
                pure ((semanticResult).value))
          let last ← do
            (do
                let semanticResult ← (protocol_quantity_decrement ⟨next⟩)
                pure ((semanticResult).value))
          (_rec_bal_balance_groups_size ⟨account⟩ ⟨count⟩ ⟨next⟩
            (← (byte_quantity_add content_len
                (← (bal_index_word_size ⟨index⟩
                    (← (bal_balance_change_value ⟨account⟩ ⟨last⟩)))))) _reclimit_pred))
      else (pure content_len))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the final balance value for each change index. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_balance_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : byte_length) : SailM byte_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_balance_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩ content_len (_measure + 1))

/-- Writes the final balance value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex161578_ : Nat, k_ex161577_ : Nat, k_ex161576_ : Nat, 0 ≤
  k_ex161576_ ∧ k_ex161576_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161577_ ∧ k_ex161577_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161578_ ∧ k_ex161578_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
                let semanticResult ← (bal_balance_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let next ← do
            (do
                let semanticResult ← (bal_balance_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨cursor⟩)
                pure ((semanticResult).value))
          let last ← do
            (do
                let semanticResult ← (protocol_quantity_decrement ⟨next⟩)
                pure ((semanticResult).value))
          (bal_write_index_word ⟨index⟩
            (← (bal_balance_change_value ⟨account⟩ ⟨last⟩)))
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
def bal_balance_changes_size (account : item_index) : SailM byte_length := do
  let account := (account).value
  (bal_balance_groups_size ⟨account⟩
    ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩ BYTE_ZERO)

/-- Writes the balance-change list for one account. -/
/- Type quantifiers: k_ex161585_ : Nat, 0 ≤ k_ex161585_ ∧ k_ex161585_ ≤ (2 ^ 64 - 1) -/
def bal_write_balance_changes (account : item_index) (content_len : byte_length) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix content_len)
  (bal_write_balance_groups ⟨account⟩
    ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Finds one nonce-change run and its greatest observed nonce. -/
/- Type quantifiers: _reclimit : Nat, k_ex161590_ : Nat, k_ex161589_ : Nat, k_ex161588_ : Nat, k_ex161587_
  : Nat, k_ex161586_ : Nat, 0 ≤ k_ex161586_ ∧ k_ex161586_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161587_
  ∧ k_ex161587_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161588_ ∧ k_ex161588_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161589_ ∧ k_ex161589_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161590_ ∧ k_ex161590_ ≤ (2 ^ 64 - 1), 0
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
                    let semanticResult ← (bal_nonce_change_value ⟨account⟩ ⟨cursor⟩)
                    pure ((semanticResult).value))
              let next_maximum :=
                if ((maximum <b value) : Bool)
                then value
                else maximum
              (_rec_bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ ⟨next_maximum⟩
                _reclimit_pred))
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
/- Type quantifiers: _reclimit : Nat, k_ex161600_ : Nat, k_ex161599_ : Nat, k_ex161598_ : Nat, 0 ≤
  k_ex161598_ ∧ k_ex161598_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161599_ ∧ k_ex161599_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161600_ ∧ k_ex161600_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_nonce_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : byte_length) (_reclimit : Nat) : SailM byte_length := do
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
                let semanticResult ← (bal_nonce_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let first ← do
            (do
                let semanticResult ← (bal_nonce_change_value ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let run ← do
            (bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩
              ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ ⟨first⟩)
          (_rec_bal_nonce_groups_size ⟨account⟩ ⟨count⟩ ⟨(run.cursor).value⟩
            (← (byte_quantity_add content_len
                (← (bal_index_nonce_size ⟨index⟩ ⟨(run.maximum).value⟩)))) _reclimit_pred))
      else (pure content_len))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the maximum nonce value for each change index. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_nonce_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : byte_length) : SailM byte_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_bal_nonce_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩ content_len (_measure + 1))

/-- Writes the maximum nonce value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex161608_ : Nat, k_ex161607_ : Nat, k_ex161606_ : Nat, 0 ≤
  k_ex161606_ ∧ k_ex161606_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161607_ ∧ k_ex161607_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161608_ ∧ k_ex161608_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
                let semanticResult ← (bal_nonce_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let first ← do
            (do
                let semanticResult ← (bal_nonce_change_value ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let run ← do
            (bal_nonce_run ⟨account⟩ ⟨count⟩ ⟨index⟩
              ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ ⟨first⟩)
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
def bal_nonce_changes_size (account : item_index) : SailM byte_length := do
  let account := (account).value
  (bal_nonce_groups_size ⟨account⟩ ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩
    ⟨0⟩ BYTE_ZERO)

/-- Writes the nonce-change list for one account. -/
/- Type quantifiers: k_ex161615_ : Nat, 0 ≤ k_ex161615_ ∧ k_ex161615_ ≤ (2 ^ 64 - 1) -/
def bal_write_nonce_changes (account : item_index) (content_len : byte_length) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix content_len)
  (bal_write_nonce_groups ⟨account⟩ ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Finds the end of a run of equal code-change indices. -/
/- Type quantifiers: _reclimit : Nat, k_ex161619_ : Nat, k_ex161618_ : Nat, k_ex161617_ : Nat, k_ex161616_
  : Nat, 0 ≤ k_ex161616_ ∧ k_ex161616_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161617_ ∧
  k_ex161617_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161618_ ∧ k_ex161618_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161619_ ∧ k_ex161619_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_code_run_end (account : item_index) (count : item_count) (index : item_index) (cursor : item_index) (_reclimit : Nat) : SailM item_index := do
  let account := (account).value
  let count := (count).value
  let index := (index).value
  let cursor := (cursor).value
  let semanticResult ← do
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
                  let semanticResult ← (_rec_bal_code_run_end ⟨account⟩ ⟨count⟩
                  ⟨index⟩ ⟨((← (item_index_increment ⟨cursor⟩))).value⟩ _reclimit_pred)
                  pure ((semanticResult).value))
            else (pure cursor))
        else (pure cursor))
  pure (⟨semanticResult⟩)
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
  let semanticResult ← do
    let _measure := ((count -i cursor) : Int)
    if ((_measure <b 0) : Bool)
    then throw Error.Exit
    else
      (do
          let semanticResult ← (_rec_bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
          ⟨cursor⟩ (_measure + 1))
          pure ((semanticResult).value))
  pure (⟨semanticResult⟩)

/-- Sizes the final code value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex161628_ : Nat, k_ex161627_ : Nat, k_ex161626_ : Nat, 0 ≤
  k_ex161626_ ∧ k_ex161626_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161627_ ∧ k_ex161627_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161628_ ∧ k_ex161628_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_code_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : byte_length) (_reclimit : Nat) : SailM byte_length := do
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
                let semanticResult ← (bal_code_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let next ← do
            (do
                let semanticResult ← (bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨cursor⟩)
                pure ((semanticResult).value))
          let last ← do
            (do
                let semanticResult ← (protocol_quantity_decrement ⟨next⟩)
                pure ((semanticResult).value))
          (_rec_bal_code_groups_size ⟨account⟩ ⟨count⟩ ⟨next⟩
            (← (byte_quantity_add content_len
                (← (bal_index_code_size ⟨index⟩
                    (← (bal_code_change_hash ⟨account⟩ ⟨last⟩)))))) _reclimit_pred))
      else (pure content_len))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes the final code value for each change index. -/
/- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
def bal_code_groups_size (account : item_index) (count : item_count) (cursor : item_index) (content_len : byte_length) : SailM byte_length := do
  let account := (account).value
  let count := (count).value
  let cursor := (cursor).value
  let _measure := ((count -i cursor) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_code_groups_size ⟨account⟩ ⟨count⟩ ⟨cursor⟩ content_len (_measure + 1))

/-- Writes the final code value for each change index. -/
/- Type quantifiers: _reclimit : Nat, k_ex161636_ : Nat, k_ex161635_ : Nat, k_ex161634_ : Nat, 0 ≤
  k_ex161634_ ∧ k_ex161634_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161635_ ∧ k_ex161635_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex161636_ ∧ k_ex161636_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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
                let semanticResult ← (bal_code_change_index ⟨account⟩ ⟨cursor⟩)
                pure ((semanticResult).value))
          let next ← do
            (do
                let semanticResult ← (bal_code_run_end ⟨account⟩ ⟨count⟩ ⟨index⟩
                ⟨cursor⟩)
                pure ((semanticResult).value))
          let last ← do
            (do
                let semanticResult ← (protocol_quantity_decrement ⟨next⟩)
                pure ((semanticResult).value))
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
def bal_code_changes_size (account : item_index) : SailM byte_length := do
  let account := (account).value
  (bal_code_groups_size ⟨account⟩ ⟨((← (bal_code_change_count ⟨account⟩))).value⟩
    ⟨0⟩ BYTE_ZERO)

/-- Writes the code-change list for one account. -/
/- Type quantifiers: k_ex161643_ : Nat, 0 ≤ k_ex161643_ ∧ k_ex161643_ ≤ (2 ^ 64 - 1) -/
def bal_write_code_changes (account : item_index) (content_len : byte_length) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix content_len)
  (bal_write_code_groups ⟨account⟩ ⟨((← (bal_code_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Sizes all BAL fields contributed by one account. -/
/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_account_size (account : item_index) : SailM BalAccountSize := do
  let account := (account).value
  let storage_changes ← do (bal_storage_changes_size ⟨account⟩)
  let storage_reads ← do (bal_storage_reads_size ⟨account⟩)
  let balance_changes_len ← do (bal_balance_changes_size ⟨account⟩)
  let nonce_changes_len ← do (bal_nonce_changes_size ⟨account⟩)
  let code_changes_len ← do (bal_code_changes_size ⟨account⟩)
  let content_len := (rlp_addr_size ())
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_changes.content_len)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_reads.content_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size balance_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size nonce_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size code_changes_len)))
  (pure { encoded_len := ← (rlp_list_size content_len),
          item_count := ← do
              let semanticField ← (do
                  let semanticResult ← (bal_count_add
                  ⟨((← (bal_count_add ⟨1⟩ ⟨(storage_changes.count).value⟩))).value⟩
                  ⟨(storage_reads.count).value⟩)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩) })

/-- Writes one account and all of its ordered BAL change lists. -/
/- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_account (account : item_index) : SailM Unit := do
  let account := (account).value
  let storage_changes ← do (bal_storage_changes_size ⟨account⟩)
  let storage_reads ← do (bal_storage_reads_size ⟨account⟩)
  let balance_changes_len ← do (bal_balance_changes_size ⟨account⟩)
  let nonce_changes_len ← do (bal_nonce_changes_size ⟨account⟩)
  let code_changes_len ← do (bal_code_changes_size ⟨account⟩)
  let content_len := (rlp_addr_size ())
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_changes.content_len)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_reads.content_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size balance_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size nonce_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size code_changes_len)))
  (rlp_write_list_prefix content_len)
  (rlp_write_addr (← (bal_account_address ⟨account⟩)))
  (bal_write_storage_changes ⟨account⟩ storage_changes)
  (bal_write_storage_reads ⟨account⟩ storage_reads)
  (bal_write_balance_changes ⟨account⟩ balance_changes_len)
  (bal_write_nonce_changes ⟨account⟩ nonce_changes_len)
  (bal_write_code_changes ⟨account⟩ code_changes_len)

/-- Sizes all ordered BAL accounts and counts their logical items. -/
/- Type quantifiers: _reclimit : Nat, k_ex161647_ : Nat, k_ex161646_ : Nat, 0 ≤ k_ex161646_ ∧
  k_ex161646_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161647_ ∧ k_ex161647_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_accounts_size (account_count : item_count) (account : item_index) (result : BalContentCount) (_reclimit : Nat) : SailM BalContentCount := do
  let account_count := (account_count).value
  let account := (account).value
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
          (_rec_bal_accounts_size ⟨account_count⟩
            ⟨((← (item_index_increment ⟨account⟩))).value⟩
            { content_len := ← (byte_quantity_add result.content_len account_size.encoded_len),
              count := ← do
                  let semanticField ← (do
                      let semanticResult ← (bal_count_add ⟨(result.count).value⟩
                      ⟨(account_size.item_count).value⟩)
                      pure ((semanticResult).value))
                  pure (⟨semanticField⟩) } _reclimit_pred))
      else (pure result))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Sizes all ordered BAL accounts and counts their logical items. -/
/- Type quantifiers: account : Nat, account_count : Nat, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1), 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_accounts_size (account_count : item_count) (account : item_index) (result : BalContentCount) : SailM BalContentCount := do
  let account_count := (account_count).value
  let account := (account).value
  let _measure := ((account_count -i account) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_accounts_size ⟨account_count⟩ ⟨account⟩ result (_measure + 1))

/-- Writes all BAL accounts in recorder order. -/
/- Type quantifiers: _reclimit : Nat, k_ex161653_ : Nat, k_ex161652_ : Nat, 0 ≤ k_ex161652_ ∧
  k_ex161652_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161653_ ∧ k_ex161653_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_bal_write_accounts (account_count : item_count) (account : item_index) (_reclimit : Nat) : SailM Unit := do
  let account_count := (account_count).value
  let account := (account).value
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
          (_rec_bal_write_accounts ⟨account_count⟩
            ⟨((← (item_index_increment ⟨account⟩))).value⟩ _reclimit_pred))
      else (pure ()))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Writes all BAL accounts in recorder order. -/
/- Type quantifiers: account : Nat, account_count : Nat, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1), 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_accounts (account_count : item_count) (account : item_index) : SailM Unit := do
  let account_count := (account_count).value
  let account := (account).value
  let _measure := ((account_count -i account) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_accounts ⟨account_count⟩ ⟨account⟩ (_measure + 1))

/-- Reconstructs the canonical EIP-7928 block access list from the
recorder: per account (address order) — storage changes, storage
reads, balance, nonce, and code changes — counting items against the
`gas_limit / 2000` bound. -/
def encode_block_access_list (_ : Unit) : SailM EncodedBlockAccessList := do
  (bal_prepare ())
  let account_count ← do
    (do
        let semanticResult ← (bal_account_count ())
        pure ((semanticResult).value))
  let size ← do
    (bal_accounts_size ⟨account_count⟩ ⟨0⟩
      { content_len := BYTE_ZERO,
        count := ⟨0⟩ })
  let encoded_len ← do (rlp_list_size size.content_len)
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix size.content_len)
  (bal_write_accounts ⟨account_count⟩ ⟨0⟩)
  (pure { bytes := ← (rlp_finish start encoded_len),
          item_count := ⟨(size.count).value⟩ })

