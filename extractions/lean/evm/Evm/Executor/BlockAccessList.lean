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

def undefined_BalNonceRun (_ : Unit) : SailM BalNonceRun := do
  (pure { cursor := ← (undefined_range 0 ((2 ^i 64) -i 1))
          maximum := ← (undefined_range 0 ((2 ^i 64) -i 1)) })

/-- Type quantifiers: b : Nat, a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 64 - 1), 0 ≤ b ∧
  b ≤ (2 ^ 64 - 1) -/
def bal_count_add (a : Nat) (b : Nat) : SailM Nat := do
  if ((b ≤b (((2 ^i 64) -i 1) -i a)) : Bool)
  then (pure (a +i b))
  else sailThrow ((InvalidBlock ExecutionInvalid))

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def bal_index_word_content_size (index : Nat) (value : (BitVec 256)) : SailM byte_quantity := do
  (byte_quantity_add (← (rlp_protocol_quantity_size index)) (← (rlp_uint_word_size value)))

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def bal_index_word_size (index : Nat) (value : (BitVec 256)) : SailM byte_quantity := do
  (rlp_list_size (← (bal_index_word_content_size index value)))

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def bal_write_index_word (index : Nat) (value : (BitVec 256)) : SailM Unit := do
  (rlp_write_list_prefix (← (bal_index_word_content_size index value)))
  (rlp_write_protocol_quantity index)
  (rlp_write_uint_word value)

/-- Type quantifiers: value : Nat, index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1), 0 ≤ value
  ∧ value ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_content_size (index : Nat) (value : Nat) : SailM byte_quantity := do
  (byte_quantity_add (← (rlp_protocol_quantity_size index))
    (← (rlp_protocol_quantity_size value)))

/-- Type quantifiers: value : Nat, index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1), 0 ≤ value
  ∧ value ≤ (2 ^ 64 - 1) -/
def bal_index_nonce_size (index : Nat) (value : Nat) : SailM byte_quantity := do
  (rlp_list_size (← (bal_index_nonce_content_size index value)))

/-- Type quantifiers: value : Nat, index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1), 0 ≤ value
  ∧ value ≤ (2 ^ 64 - 1) -/
def bal_write_index_nonce (index : Nat) (value : Nat) : SailM Unit := do
  (rlp_write_list_prefix (← (bal_index_nonce_content_size index value)))
  (rlp_write_protocol_quantity index)
  (rlp_write_protocol_quantity value)

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def bal_index_code_content_size (index : Nat) (code_hash : (BitVec 256)) : SailM byte_quantity := do
  let code ← do (code_db_resolve code_hash)
  (byte_quantity_add (← (rlp_protocol_quantity_size index)) (← (rlp_slice_size code.bytes)))

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def bal_index_code_size (index : Nat) (code_hash : (BitVec 256)) : SailM byte_quantity := do
  (rlp_list_size (← (bal_index_code_content_size index code_hash)))

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def bal_write_index_code (index : Nat) (code_hash : (BitVec 256)) : SailM Unit := do
  let code ← do (code_db_resolve code_hash)
  (rlp_write_list_prefix
    (← (byte_quantity_add (← (rlp_protocol_quantity_size index))
        (← (rlp_slice_size code.bytes)))))
  (rlp_write_protocol_quantity index)
  (rlp_write_slice code.bytes)

/-- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
partial def bal_storage_change_run_end (account : Nat) (count : Nat) (slot : (BitVec 256)) (index : Nat) (cursor : Nat) : SailM Nat := do
  if ((cursor <b count) : Bool)
  then
    (do
      if ((((← (bal_storage_change_slot account cursor)) == slot) && (← do
             (pure ((← (bal_storage_change_index account cursor)) == index)))) : Bool)
      then (bal_storage_change_run_end account count slot index (← (item_index_increment cursor)))
      else (pure cursor))
  else (pure cursor)
termination_by (let (account, count, slot, index, cursor) := (account, count, slot, index, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_storage_slot_changes_size (account : Nat) (count : Nat) (slot : (BitVec 256)) (cursor : Nat) (content_len : byte_quantity) : SailM BalContentCursor := do
  if ((cursor <b count) : Bool)
  then
    (do
      if (((← (bal_storage_change_slot account cursor)) == slot) : Bool)
      then
        (do
          let index ← do (bal_storage_change_index account cursor)
          let next ← do (bal_storage_change_run_end account count slot index cursor)
          let last ← do (protocol_quantity_decrement next)
          let next_len ← do
            (byte_quantity_add content_len
              (← (bal_index_word_size index (← (bal_storage_change_value account last)))))
          (bal_storage_slot_changes_size account count slot next next_len))
      else
        (pure { content_len := content_len
                cursor := cursor }))
  else
    (pure { content_len := content_len
            cursor := cursor })
termination_by (let (account, count, slot, cursor, content_len) :=
  (account, count, slot, cursor, content_len)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_write_storage_slot_changes (account : Nat) (count : Nat) (slot : (BitVec 256)) (cursor : Nat) : SailM Nat := do
  if ((cursor <b count) : Bool)
  then
    (do
      if (((← (bal_storage_change_slot account cursor)) == slot) : Bool)
      then
        (do
          let index ← do (bal_storage_change_index account cursor)
          let next ← do (bal_storage_change_run_end account count slot index cursor)
          let last ← do (protocol_quantity_decrement next)
          (bal_write_index_word index (← (bal_storage_change_value account last)))
          (bal_write_storage_slot_changes account count slot next))
      else (pure cursor))
  else (pure cursor)
termination_by (let (account, count, slot, cursor) := (account, count, slot, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_storage_change_groups_size (account : Nat) (count : Nat) (cursor : Nat) (result : BalContentCount) : SailM BalContentCount := do
  if ((cursor <b count) : Bool)
  then
    (do
      let slot ← do (bal_storage_change_slot account cursor)
      let changes ← do (bal_storage_slot_changes_size account count slot cursor BYTE_ZERO)
      let slot_content_len ← do
        (byte_quantity_add (← (rlp_uint_word_size slot)) (← (rlp_list_size changes.content_len)))
      (bal_storage_change_groups_size account count changes.cursor
        { content_len := ← (byte_quantity_add result.content_len
              (← (rlp_list_size slot_content_len)))
          count := ← (bal_count_add result.count 1) }))
  else (pure result)
termination_by (let (account, count, cursor, result) := (account, count, cursor, result)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_write_storage_change_groups (account : Nat) (count : Nat) (cursor : Nat) : SailM Unit := do
  if ((cursor <b count) : Bool)
  then
    (do
      let slot ← do (bal_storage_change_slot account cursor)
      let changes ← do (bal_storage_slot_changes_size account count slot cursor BYTE_ZERO)
      let slot_content_len ← do
        (byte_quantity_add (← (rlp_uint_word_size slot)) (← (rlp_list_size changes.content_len)))
      (rlp_write_list_prefix slot_content_len)
      (rlp_write_uint_word slot)
      (rlp_write_list_prefix changes.content_len)
      let next ← do (bal_write_storage_slot_changes account count slot cursor)
      assert (next == changes.cursor) "BAL storage-change sizing cursor"
      (bal_write_storage_change_groups account count next))
  else (pure ())
termination_by (let (account, count, cursor) := (account, count, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_storage_changes_size (account : Nat) : SailM BalContentCount := do
  (bal_storage_change_groups_size account (← (bal_storage_change_count account)) 0
    { content_len := BYTE_ZERO
      count := 0 })

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_storage_changes (account : Nat) (size : BalContentCount) : SailM Unit := do
  (rlp_write_list_prefix size.content_len)
  (bal_write_storage_change_groups account (← (bal_storage_change_count account)) 0)

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_storage_read_run_end (account : Nat) (count : Nat) (slot : (BitVec 256)) (cursor : Nat) : SailM Nat := do
  if ((cursor <b count) : Bool)
  then
    (do
      if (((← (bal_storage_read_slot account cursor)) == slot) : Bool)
      then (bal_storage_read_run_end account count slot (← (item_index_increment cursor)))
      else (pure cursor))
  else (pure cursor)
termination_by (let (account, count, slot, cursor) := (account, count, slot, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_storage_change_seek (account : Nat) (count : Nat) (slot : (BitVec 256)) (cursor : Nat) : SailM Nat := do
  if ((cursor <b count) : Bool)
  then
    (do
      if ((word_ult (← (bal_storage_change_slot account cursor)) slot) : Bool)
      then (bal_storage_change_seek account count slot (← (item_index_increment cursor)))
      else (pure cursor))
  else (pure cursor)
termination_by (let (account, count, slot, cursor) := (account, count, slot, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: change : Nat, read : Nat, change_count : Nat, read_count : Nat, account : Nat, 0
  ≤ account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ read_count ∧ read_count ≤ (2 ^ 64 - 1), 0 ≤
  change_count ∧ change_count ≤ (2 ^ 64 - 1), 0 ≤ read ∧ read ≤ (2 ^ 64 - 1), 0 ≤ change
  ∧ change ≤ (2 ^ 64 - 1) -/
partial def bal_storage_read_groups_size (account : Nat) (read_count : Nat) (change_count : Nat) (read : Nat) (change : Nat) (result : BalContentCount) : SailM BalContentCount := do
  if ((read <b read_count) : Bool)
  then
    (do
      let slot ← do (bal_storage_read_slot account read)
      let next_read ← do (bal_storage_read_run_end account read_count slot read)
      let next_change ← do (bal_storage_change_seek account change_count slot change)
      let changed ← do
        if ((next_change <b change_count) : Bool)
        then (pure ((← (bal_storage_change_slot account next_change)) == slot))
        else (pure false)
      let updated ← do
        if (changed : Bool)
        then (pure result)
        else
          (pure { content_len := ← (byte_quantity_add result.content_len
                      (← (rlp_uint_word_size slot)))
                  count := ← (bal_count_add result.count 1) })
      (bal_storage_read_groups_size account read_count change_count next_read next_change updated))
  else (pure result)
termination_by (let (account, read_count, change_count, read, change, result) :=
  (account, read_count, change_count, read, change, result)
(read_count -i read)).toNat

/-- Type quantifiers: change : Nat, read : Nat, change_count : Nat, read_count : Nat, account : Nat, 0
  ≤ account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ read_count ∧ read_count ≤ (2 ^ 64 - 1), 0 ≤
  change_count ∧ change_count ≤ (2 ^ 64 - 1), 0 ≤ read ∧ read ≤ (2 ^ 64 - 1), 0 ≤ change
  ∧ change ≤ (2 ^ 64 - 1) -/
partial def bal_write_storage_read_groups (account : Nat) (read_count : Nat) (change_count : Nat) (read : Nat) (change : Nat) : SailM Unit := do
  if ((read <b read_count) : Bool)
  then
    (do
      let slot ← do (bal_storage_read_slot account read)
      let next_read ← do (bal_storage_read_run_end account read_count slot read)
      let next_change ← do (bal_storage_change_seek account change_count slot change)
      let changed ← do
        if ((next_change <b change_count) : Bool)
        then (pure ((← (bal_storage_change_slot account next_change)) == slot))
        else (pure false)
      if ((! changed) : Bool)
      then (rlp_write_uint_word slot)
      else (pure ())
      (bal_write_storage_read_groups account read_count change_count next_read next_change))
  else (pure ())
termination_by (let (account, read_count, change_count, read, change) :=
  (account, read_count, change_count, read, change)
(read_count -i read)).toNat

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_storage_reads_size (account : Nat) : SailM BalContentCount := do
  (bal_storage_read_groups_size account (← (bal_storage_read_count account))
    (← (bal_storage_change_count account)) 0 0
    { content_len := BYTE_ZERO
      count := 0 })

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_storage_reads (account : Nat) (size : BalContentCount) : SailM Unit := do
  (rlp_write_list_prefix size.content_len)
  (bal_write_storage_read_groups account (← (bal_storage_read_count account))
    (← (bal_storage_change_count account)) 0 0)

/-- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
partial def bal_balance_run_end (account : Nat) (count : Nat) (index : Nat) (cursor : Nat) : SailM Nat := do
  if ((cursor <b count) : Bool)
  then
    (do
      if (((← (bal_balance_change_index account cursor)) == index) : Bool)
      then (bal_balance_run_end account count index (← (item_index_increment cursor)))
      else (pure cursor))
  else (pure cursor)
termination_by (let (account, count, index, cursor) := (account, count, index, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_balance_groups_size (account : Nat) (count : Nat) (cursor : Nat) (content_len : byte_quantity) : SailM byte_quantity := do
  if ((cursor <b count) : Bool)
  then
    (do
      let index ← do (bal_balance_change_index account cursor)
      let next ← do (bal_balance_run_end account count index cursor)
      let last ← do (protocol_quantity_decrement next)
      (bal_balance_groups_size account count next
        (← (byte_quantity_add content_len
            (← (bal_index_word_size index (← (bal_balance_change_value account last))))))))
  else (pure content_len)
termination_by (let (account, count, cursor, content_len) := (account, count, cursor, content_len)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_write_balance_groups (account : Nat) (count : Nat) (cursor : Nat) : SailM Unit := do
  if ((cursor <b count) : Bool)
  then
    (do
      let index ← do (bal_balance_change_index account cursor)
      let next ← do (bal_balance_run_end account count index cursor)
      let last ← do (protocol_quantity_decrement next)
      (bal_write_index_word index (← (bal_balance_change_value account last)))
      (bal_write_balance_groups account count next))
  else (pure ())
termination_by (let (account, count, cursor) := (account, count, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_balance_changes_size (account : Nat) : SailM byte_quantity := do
  (bal_balance_groups_size account (← (bal_balance_change_count account)) 0 BYTE_ZERO)

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_balance_changes (account : Nat) (content_len : byte_quantity) : SailM Unit := do
  (rlp_write_list_prefix content_len)
  (bal_write_balance_groups account (← (bal_balance_change_count account)) 0)

/-- Type quantifiers: maximum : Nat, cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤
  account ∧ account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1), 0 ≤ maximum ∧
  maximum ≤ (2 ^ 64 - 1) -/
partial def bal_nonce_run (account : Nat) (count : Nat) (index : Nat) (cursor : Nat) (maximum : Nat) : SailM BalNonceRun := do
  if ((cursor <b count) : Bool)
  then
    (do
      if (((← (bal_nonce_change_index account cursor)) == index) : Bool)
      then
        (do
          let value ← do (bal_nonce_change_value account cursor)
          let next_maximum :=
            if ((maximum <b value) : Bool)
            then value
            else maximum
          (bal_nonce_run account count index (← (item_index_increment cursor)) next_maximum))
      else
        (pure { cursor := cursor
                maximum := maximum }))
  else
    (pure { cursor := cursor
            maximum := maximum })
termination_by (let (account, count, index, cursor, maximum) :=
  (account, count, index, cursor, maximum)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_nonce_groups_size (account : Nat) (count : Nat) (cursor : Nat) (content_len : byte_quantity) : SailM byte_quantity := do
  if ((cursor <b count) : Bool)
  then
    (do
      let index ← do (bal_nonce_change_index account cursor)
      let first ← do (bal_nonce_change_value account cursor)
      let run ← do (bal_nonce_run account count index (← (item_index_increment cursor)) first)
      (bal_nonce_groups_size account count run.cursor
        (← (byte_quantity_add content_len (← (bal_index_nonce_size index run.maximum))))))
  else (pure content_len)
termination_by (let (account, count, cursor, content_len) := (account, count, cursor, content_len)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_write_nonce_groups (account : Nat) (count : Nat) (cursor : Nat) : SailM Unit := do
  if ((cursor <b count) : Bool)
  then
    (do
      let index ← do (bal_nonce_change_index account cursor)
      let first ← do (bal_nonce_change_value account cursor)
      let run ← do (bal_nonce_run account count index (← (item_index_increment cursor)) first)
      (bal_write_index_nonce index run.maximum)
      (bal_write_nonce_groups account count run.cursor))
  else (pure ())
termination_by (let (account, count, cursor) := (account, count, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_nonce_changes_size (account : Nat) : SailM byte_quantity := do
  (bal_nonce_groups_size account (← (bal_nonce_change_count account)) 0 BYTE_ZERO)

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_nonce_changes (account : Nat) (content_len : byte_quantity) : SailM Unit := do
  (rlp_write_list_prefix content_len)
  (bal_write_nonce_groups account (← (bal_nonce_change_count account)) 0)

/-- Type quantifiers: cursor : Nat, index : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ index ∧
  index ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧ cursor ≤ (2 ^ 64 - 1) -/
partial def bal_code_run_end (account : Nat) (count : Nat) (index : Nat) (cursor : Nat) : SailM Nat := do
  if ((cursor <b count) : Bool)
  then
    (do
      if (((← (bal_code_change_index account cursor)) == index) : Bool)
      then (bal_code_run_end account count index (← (item_index_increment cursor)))
      else (pure cursor))
  else (pure cursor)
termination_by (let (account, count, index, cursor) := (account, count, index, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_code_groups_size (account : Nat) (count : Nat) (cursor : Nat) (content_len : byte_quantity) : SailM byte_quantity := do
  if ((cursor <b count) : Bool)
  then
    (do
      let index ← do (bal_code_change_index account cursor)
      let next ← do (bal_code_run_end account count index cursor)
      let last ← do (protocol_quantity_decrement next)
      (bal_code_groups_size account count next
        (← (byte_quantity_add content_len
            (← (bal_index_code_size index (← (bal_code_change_hash account last))))))))
  else (pure content_len)
termination_by (let (account, count, cursor, content_len) := (account, count, cursor, content_len)
(count -i cursor)).toNat

/-- Type quantifiers: cursor : Nat, count : Nat, account : Nat, 0 ≤ account ∧
  account ≤ (2 ^ 64 - 1), 0 ≤ count ∧ count ≤ (2 ^ 64 - 1), 0 ≤ cursor ∧
  cursor ≤ (2 ^ 64 - 1) -/
partial def bal_write_code_groups (account : Nat) (count : Nat) (cursor : Nat) : SailM Unit := do
  if ((cursor <b count) : Bool)
  then
    (do
      let index ← do (bal_code_change_index account cursor)
      let next ← do (bal_code_run_end account count index cursor)
      let last ← do (protocol_quantity_decrement next)
      (bal_write_index_code index (← (bal_code_change_hash account last)))
      (bal_write_code_groups account count next))
  else (pure ())
termination_by (let (account, count, cursor) := (account, count, cursor)
(count -i cursor)).toNat

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_code_changes_size (account : Nat) : SailM byte_quantity := do
  (bal_code_groups_size account (← (bal_code_change_count account)) 0 BYTE_ZERO)

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_code_changes (account : Nat) (content_len : byte_quantity) : SailM Unit := do
  (rlp_write_list_prefix content_len)
  (bal_write_code_groups account (← (bal_code_change_count account)) 0)

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_account_size (account : Nat) : SailM BalAccountSize := do
  let storage_changes ← do (bal_storage_changes_size account)
  let storage_reads ← do (bal_storage_reads_size account)
  let balance_changes_len ← do (bal_balance_changes_size account)
  let nonce_changes_len ← do (bal_nonce_changes_size account)
  let code_changes_len ← do (bal_code_changes_size account)
  let content_len := (rlp_addr_size ())
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_changes.content_len)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_reads.content_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size balance_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size nonce_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size code_changes_len)))
  (pure { encoded_len := ← (rlp_list_size content_len)
          item_count := ← (bal_count_add (← (bal_count_add 1 storage_changes.count))
              storage_reads.count) })

/-- Type quantifiers: account : Nat, 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
def bal_write_account (account : Nat) : SailM Unit := do
  let storage_changes ← do (bal_storage_changes_size account)
  let storage_reads ← do (bal_storage_reads_size account)
  let balance_changes_len ← do (bal_balance_changes_size account)
  let nonce_changes_len ← do (bal_nonce_changes_size account)
  let code_changes_len ← do (bal_code_changes_size account)
  let content_len := (rlp_addr_size ())
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_changes.content_len)))
  let content_len ←
    (byte_quantity_add content_len (← (rlp_list_size storage_reads.content_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size balance_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size nonce_changes_len)))
  let content_len ← (byte_quantity_add content_len (← (rlp_list_size code_changes_len)))
  (rlp_write_list_prefix content_len)
  (rlp_write_addr (← (bal_account_address account)))
  (bal_write_storage_changes account storage_changes)
  (bal_write_storage_reads account storage_reads)
  (bal_write_balance_changes account balance_changes_len)
  (bal_write_nonce_changes account nonce_changes_len)
  (bal_write_code_changes account code_changes_len)

/-- Type quantifiers: account : Nat, account_count : Nat, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1), 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
partial def bal_accounts_size (account_count : Nat) (account : Nat) (result : BalContentCount) : SailM BalContentCount := do
  if ((account <b account_count) : Bool)
  then
    (do
      let account_size ← do (bal_account_size account)
      (bal_accounts_size account_count (← (item_index_increment account))
        { content_len := ← (byte_quantity_add result.content_len account_size.encoded_len)
          count := ← (bal_count_add result.count account_size.item_count) }))
  else (pure result)
termination_by (let (account_count, account, result) := (account_count, account, result)
(account_count -i account)).toNat

/-- Type quantifiers: account : Nat, account_count : Nat, 0 ≤ account_count ∧
  account_count ≤ (2 ^ 64 - 1), 0 ≤ account ∧ account ≤ (2 ^ 64 - 1) -/
partial def bal_write_accounts (account_count : Nat) (account : Nat) : SailM Unit := do
  if ((account <b account_count) : Bool)
  then
    (do
      (bal_write_account account)
      (bal_write_accounts account_count (← (item_index_increment account))))
  else (pure ())
termination_by (let (account_count, account) := (account_count, account)
(account_count -i account)).toNat

def encode_block_access_list (_ : Unit) : SailM EncodedBlockAccessList := do
  (bal_prepare ())
  let account_count ← do (bal_account_count ())
  let size ← do
    (bal_accounts_size account_count 0
      { content_len := BYTE_ZERO
        count := 0 })
  let encoded_len ← do (rlp_list_size size.content_len)
  let start ← do (scratch_begin ())
  (rlp_write_list_prefix size.content_len)
  (bal_write_accounts account_count 0)
  (pure { bytes := ← (rlp_finish start encoded_len)
          item_count := size.count })

