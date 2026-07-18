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
open StateCheckpoint
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def undefined_BalNonceRun (_ : Unit) : SailM BalNonceRun := do
  (pure { cursor := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          maximum := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩) })

/-- Type quantifiers: k_ex161411_ : Nat, k_ex161410_ : Nat, 0 ≤ k_ex161410_ ∧
  k_ex161410_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161411_ ∧ k_ex161411_ ≤ (2 ^ 64 - 1) -/
def bal_count_add (a : item_count) (b : item_count) : SailM item_count := do
  let a := (a).value
  let b := (b).value
  let semanticResult ← do
    if ((b ≤b (((2 ^i 64) -i 1) -i a)) : Bool)
    then (pure (a + b))
    else sailThrow ((InvalidBlock ExecutionInvalid))
  pure (⟨semanticResult⟩)

/-- Type quantifiers: k_ex161412_ : Nat, 0 ≤ k_ex161412_ ∧ k_ex161412_ ≤ (2 ^ 64 - 1) -/
def bal_index_word_content_size (index : item_index) (value : word) : SailM byte_length := do
  let index := (index).value
  (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩)) (← (rlp_uint_word_size value)))

/-- Type quantifiers: k_ex161413_ : Nat, 0 ≤ k_ex161413_ ∧ k_ex161413_ ≤ (2 ^ 64 - 1) -/
def bal_index_word_size (index : item_index) (value : word) : SailM byte_length := do
  let index := (index).value
  (rlp_list_size (← (bal_index_word_content_size ⟨index⟩ value)))

/-- Type quantifiers: k_ex161414_ : Nat, 0 ≤ k_ex161414_ ∧ k_ex161414_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_word (index : item_index) (value : word) : SailM Unit := do
  let index := (index).value
  (rlp_write_list_prefix (← (bal_index_word_content_size ⟨index⟩ value)))
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_uint_word value)

/-- Type quantifiers: k_ex161416_ : Nat, k_ex161415_ : Nat, 0 ≤ k_ex161415_ ∧
  k_ex161415_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161416_ ∧ k_ex161416_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_content_size (index : item_index) (value : account_nonce) : SailM byte_length := do
  let index := (index).value
  let value := (value).value
  (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩))
    (← (rlp_protocol_quantity_size ⟨value⟩)))

/-- Type quantifiers: k_ex161418_ : Nat, k_ex161417_ : Nat, 0 ≤ k_ex161417_ ∧
  k_ex161417_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161418_ ∧ k_ex161418_ ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_size (index : item_index) (value : account_nonce) : SailM byte_length := do
  let index := (index).value
  let value := (value).value
  (rlp_list_size (← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩)))

/-- Type quantifiers: k_ex161420_ : Nat, k_ex161419_ : Nat, 0 ≤ k_ex161419_ ∧
  k_ex161419_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161420_ ∧ k_ex161420_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_nonce (index : item_index) (value : account_nonce) : SailM Unit := do
  let index := (index).value
  let value := (value).value
  (rlp_write_list_prefix (← (bal_index_nonce_content_size ⟨index⟩ ⟨value⟩)))
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_protocol_quantity ⟨value⟩)

/-- Type quantifiers: k_ex161421_ : Nat, 0 ≤ k_ex161421_ ∧ k_ex161421_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_content_size (index : item_index) (code_hash : hash) : SailM byte_length := do
  let index := (index).value
  let code ← do (code_db_resolve code_hash)
  (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩))
    (← (rlp_slice_size code.bytes)))

/-- Type quantifiers: k_ex161422_ : Nat, 0 ≤ k_ex161422_ ∧ k_ex161422_ ≤ (2 ^ 64 - 1) -/
def bal_index_code_size (index : item_index) (code_hash : hash) : SailM byte_length := do
  let index := (index).value
  (rlp_list_size (← (bal_index_code_content_size ⟨index⟩ code_hash)))

/-- Type quantifiers: k_ex161423_ : Nat, 0 ≤ k_ex161423_ ∧ k_ex161423_ ≤ (2 ^ 64 - 1) -/
def bal_write_index_code (index : item_index) (code_hash : hash) : SailM Unit := do
  let index := (index).value
  let code ← do (code_db_resolve code_hash)
  (rlp_write_list_prefix
    (← (byte_quantity_add (← (rlp_protocol_quantity_size ⟨index⟩))
        (← (rlp_slice_size code.bytes)))))
  (rlp_write_protocol_quantity ⟨index⟩)
  (rlp_write_slice code.bytes)

/-- Type quantifiers: _reclimit : Nat, k_ex161427_ : Nat, k_ex161426_ : Nat, k_ex161425_ : Nat, k_ex161424_
  : Nat, 0 ≤ k_ex161424_ ∧ k_ex161424_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161425_ ∧
  k_ex161425_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161426_ ∧ k_ex161426_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161427_ ∧ k_ex161427_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161436_ : Nat, k_ex161435_ : Nat, k_ex161434_ : Nat, 0
  ≤ k_ex161434_ ∧ k_ex161434_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161435_ ∧
  k_ex161435_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161436_ ∧ k_ex161436_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161444_ : Nat, k_ex161443_ : Nat, k_ex161442_ : Nat, 0
  ≤ k_ex161442_ ∧ k_ex161442_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161443_ ∧
  k_ex161443_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161444_ ∧ k_ex161444_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161452_ : Nat, k_ex161451_ : Nat, k_ex161450_ : Nat, 0
  ≤ k_ex161450_ ∧ k_ex161450_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161451_ ∧
  k_ex161451_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161452_ ∧ k_ex161452_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161460_ : Nat, k_ex161459_ : Nat, k_ex161458_ : Nat, 0
  ≤ k_ex161458_ ∧ k_ex161458_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161459_ ∧
  k_ex161459_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161460_ ∧ k_ex161460_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_storage_changes_size (account : item_index) : SailM BalContentCount := do
  let account := (account).value
  (bal_storage_change_groups_size ⟨account⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩
    { content_len := BYTE_ZERO,
      count := ⟨0⟩ })

/-- Type quantifiers: k_ex161467_ : Nat, 0 ≤ k_ex161467_ ∧ k_ex161467_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_changes (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix size.content_len)
  (bal_write_storage_change_groups ⟨account⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Type quantifiers: _reclimit : Nat, k_ex161470_ : Nat, k_ex161469_ : Nat, k_ex161468_ : Nat, 0
  ≤ k_ex161468_ ∧ k_ex161468_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161469_ ∧
  k_ex161469_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161470_ ∧ k_ex161470_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161478_ : Nat, k_ex161477_ : Nat, k_ex161476_ : Nat, 0
  ≤ k_ex161476_ ∧ k_ex161476_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161477_ ∧
  k_ex161477_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161478_ ∧ k_ex161478_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161488_ : Nat, k_ex161487_ : Nat, k_ex161486_ : Nat, k_ex161485_
  : Nat, k_ex161484_ : Nat, 0 ≤ k_ex161484_ ∧ k_ex161484_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161485_
  ∧ k_ex161485_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161486_ ∧ k_ex161486_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161487_ ∧ k_ex161487_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161488_ ∧ k_ex161488_ ≤ (2 ^ 64 - 1), 0
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

/-- Type quantifiers: change : Nat, read : Nat, change_count : Nat, read_count : Nat, account : Nat, 0
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

/-- Type quantifiers: _reclimit : Nat, k_ex161500_ : Nat, k_ex161499_ : Nat, k_ex161498_ : Nat, k_ex161497_
  : Nat, k_ex161496_ : Nat, 0 ≤ k_ex161496_ ∧ k_ex161496_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161497_
  ∧ k_ex161497_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161498_ ∧ k_ex161498_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161499_ ∧ k_ex161499_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161500_ ∧ k_ex161500_ ≤ (2 ^ 64 - 1), 0
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

/-- Type quantifiers: change : Nat, read : Nat, change_count : Nat, read_count : Nat, account : Nat, 0
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

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_storage_reads_size (account : item_index) : SailM BalContentCount := do
  let account := (account).value
  (bal_storage_read_groups_size ⟨account⟩
    ⟨((← (bal_storage_read_count ⟨account⟩))).value⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨0⟩
    { content_len := BYTE_ZERO,
      count := ⟨0⟩ })

/-- Type quantifiers: k_ex161509_ : Nat, 0 ≤ k_ex161509_ ∧ k_ex161509_ ≤ (2 ^ 64 - 1) -/
def bal_write_storage_reads (account : item_index) (size : BalContentCount) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix size.content_len)
  (bal_write_storage_read_groups ⟨account⟩
    ⟨((← (bal_storage_read_count ⟨account⟩))).value⟩
    ⟨((← (bal_storage_change_count ⟨account⟩))).value⟩ ⟨0⟩ ⟨0⟩)

/-- Type quantifiers: _reclimit : Nat, k_ex161513_ : Nat, k_ex161512_ : Nat, k_ex161511_ : Nat, k_ex161510_
  : Nat, 0 ≤ k_ex161510_ ∧ k_ex161510_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161511_ ∧
  k_ex161511_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161512_ ∧ k_ex161512_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161513_ ∧ k_ex161513_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161522_ : Nat, k_ex161521_ : Nat, k_ex161520_ : Nat, 0
  ≤ k_ex161520_ ∧ k_ex161520_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161521_ ∧
  k_ex161521_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161522_ ∧ k_ex161522_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161530_ : Nat, k_ex161529_ : Nat, k_ex161528_ : Nat, 0
  ≤ k_ex161528_ ∧ k_ex161528_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161529_ ∧
  k_ex161529_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161530_ ∧ k_ex161530_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_balance_changes_size (account : item_index) : SailM byte_length := do
  let account := (account).value
  (bal_balance_groups_size ⟨account⟩
    ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩ BYTE_ZERO)

/-- Type quantifiers: k_ex161537_ : Nat, 0 ≤ k_ex161537_ ∧ k_ex161537_ ≤ (2 ^ 64 - 1) -/
def bal_write_balance_changes (account : item_index) (content_len : byte_length) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix content_len)
  (bal_write_balance_groups ⟨account⟩
    ⟨((← (bal_balance_change_count ⟨account⟩))).value⟩ ⟨0⟩)

/-- Type quantifiers: _reclimit : Nat, k_ex161542_ : Nat, k_ex161541_ : Nat, k_ex161540_ : Nat, k_ex161539_
  : Nat, k_ex161538_ : Nat, 0 ≤ k_ex161538_ ∧ k_ex161538_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161539_
  ∧ k_ex161539_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161540_ ∧ k_ex161540_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161541_ ∧ k_ex161541_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161542_ ∧ k_ex161542_ ≤ (2 ^ 64 - 1), 0
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

/-- Type quantifiers: maximum : Nat, cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤
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

/-- Type quantifiers: _reclimit : Nat, k_ex161552_ : Nat, k_ex161551_ : Nat, k_ex161550_ : Nat, 0
  ≤ k_ex161550_ ∧ k_ex161550_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161551_ ∧
  k_ex161551_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161552_ ∧ k_ex161552_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161560_ : Nat, k_ex161559_ : Nat, k_ex161558_ : Nat, 0
  ≤ k_ex161558_ ∧ k_ex161558_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161559_ ∧
  k_ex161559_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161560_ ∧ k_ex161560_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_nonce_changes_size (account : item_index) : SailM byte_length := do
  let account := (account).value
  (bal_nonce_groups_size ⟨account⟩ ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩
    ⟨0⟩ BYTE_ZERO)

/-- Type quantifiers: k_ex161567_ : Nat, 0 ≤ k_ex161567_ ∧ k_ex161567_ ≤ (2 ^ 64 - 1) -/
def bal_write_nonce_changes (account : item_index) (content_len : byte_length) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix content_len)
  (bal_write_nonce_groups ⟨account⟩ ⟨((← (bal_nonce_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Type quantifiers: _reclimit : Nat, k_ex161571_ : Nat, k_ex161570_ : Nat, k_ex161569_ : Nat, k_ex161568_
  : Nat, 0 ≤ k_ex161568_ ∧ k_ex161568_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161569_ ∧
  k_ex161569_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161570_ ∧ k_ex161570_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex161571_ ∧ k_ex161571_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161580_ : Nat, k_ex161579_ : Nat, k_ex161578_ : Nat, 0
  ≤ k_ex161578_ ∧ k_ex161578_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161579_ ∧
  k_ex161579_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161580_ ∧ k_ex161580_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: _reclimit : Nat, k_ex161588_ : Nat, k_ex161587_ : Nat, k_ex161586_ : Nat, 0
  ≤ k_ex161586_ ∧ k_ex161586_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161587_ ∧
  k_ex161587_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161588_ ∧ k_ex161588_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
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

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_code_changes_size (account : item_index) : SailM byte_length := do
  let account := (account).value
  (bal_code_groups_size ⟨account⟩ ⟨((← (bal_code_change_count ⟨account⟩))).value⟩
    ⟨0⟩ BYTE_ZERO)

/-- Type quantifiers: k_ex161595_ : Nat, 0 ≤ k_ex161595_ ∧ k_ex161595_ ≤ (2 ^ 64 - 1) -/
def bal_write_code_changes (account : item_index) (content_len : byte_length) : SailM Unit := do
  let account := (account).value
  (rlp_write_list_prefix content_len)
  (bal_write_code_groups ⟨account⟩ ⟨((← (bal_code_change_count ⟨account⟩))).value⟩
    ⟨0⟩)

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
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

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
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

/-- Type quantifiers: _reclimit : Nat, k_ex161599_ : Nat, k_ex161598_ : Nat, 0 ≤ k_ex161598_ ∧
  k_ex161598_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161599_ ∧ k_ex161599_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: account : Nat, account_count : Nat, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1), 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_accounts_size (account_count : item_count) (account : item_index) (result : BalContentCount) : SailM BalContentCount := do
  let account_count := (account_count).value
  let account := (account).value
  let _measure := ((account_count -i account) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_accounts_size ⟨account_count⟩ ⟨account⟩ result (_measure + 1))

/-- Type quantifiers: _reclimit : Nat, k_ex161605_ : Nat, k_ex161604_ : Nat, 0 ≤ k_ex161604_ ∧
  k_ex161604_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161605_ ∧ k_ex161605_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
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

/-- Type quantifiers: account : Nat, account_count : Nat, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1), 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_accounts (account_count : item_count) (account : item_index) : SailM Unit := do
  let account_count := (account_count).value
  let account := (account).value
  let _measure := ((account_count -i account) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_bal_write_accounts ⟨account_count⟩ ⟨account⟩ (_measure + 1))

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

