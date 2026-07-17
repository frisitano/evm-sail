import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Crypto
import Evm.Primitives.Account
import Evm.Host.Kernel.Storage
import Evm.Host.Kernel.Logs

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

def account_info_changed (c : AccountInfo) (o : AccountInfo) : Bool :=
  (((c.nonce).value != (o.nonce).value) || ((c.balance != o.balance) || ((c.code_hash != o.code_hash) || (c.storage_root != o.storage_root))))

def account_info_empty (info : AccountInfo) : Bool :=
  ((info.code_hash == KECCAK_EMPTY) && ((((info.nonce).value == 0) && (word_is_zero info.balance)) : Bool))

def account_changed (c : Account) (o : Account) : Bool :=
  ((account_info_changed c.info o.info) || ((neq_bool c.present o.present) || (neq_bool
        c.storage_cleared o.storage_cleared)))

def account_set_info (acc : Account) (info : AccountInfo) : Account :=
  if ((account_info_empty info) : Bool)
  then
    { acc with info := { EMPTY_ACCOUNT_INFO with storage_root := acc.info.storage_root }, present := false, storage_cleared := true }
  else { acc with info := info, present := true }

def account_clear_storage (acc : Account) : Account :=
  { acc with storage_cleared := true }

def account_delete (acc : Account) : Account :=
  { acc with info := { EMPTY_ACCOUNT_INFO with storage_root := acc.info.storage_root }, present := false, storage_cleared := true }

def store_account (a : address) (v : Account) : SailM Unit := do
  (acct_tx_update a v)

def store_account_info (a : address) (acc : Account) (info : AccountInfo) : SailM Unit := do
  if ((account_info_empty info) : Bool)
  then (storage_tx_clear a)
  else (pure ())
  let next := (account_set_info acc info)
  if (((next.info.storage_root != acc.info.storage_root) || ((neq_bool next.present acc.present) || (neq_bool
           next.storage_cleared acc.storage_cleared))) : Bool)
  then (store_account a next)
  else
    (do
      if ((next.info.balance != acc.info.balance) : Bool)
      then (acct_tx_set_balance a next.info.balance)
      else (pure ())
      if (((next.info.nonce).value != (acc.info.nonce).value) : Bool)
      then (acct_tx_set_nonce a ⟨(next.info.nonce).value⟩)
      else (pure ())
      if ((next.info.code_hash != acc.info.code_hash) : Bool)
      then (acct_tx_set_code_hash a next.info.code_hash)
      else (pure ()))

def k_get_balance (a : address) : SailM word := do
  (pure (← (k_aload a)).info.balance)

def k_get_nonce (a : address) : SailM account_nonce := do
  let semanticResult ← do (pure ((← (k_aload a)).info.nonce).value)
  pure (⟨semanticResult⟩)

def k_account_exists (a : address) : SailM Bool := do
  (pure (← (k_aload a)).present)

def k_account_is_empty (a : address) : SailM Bool := do
  (pure (account_info_empty (← (k_aload a)).info))

def k_account_occupied (a : address) : SailM Bool := do
  let acc ← do (k_aload a)
  let info := acc.info
  let anchored_storage := ((! acc.storage_cleared) && (info.storage_root != EMPTY_TRIE_ROOT))
  if (((info.code_hash != KECCAK_EMPTY) || ((((info.nonce).value != 0) || anchored_storage) : Bool)) : Bool)
  then (pure true)
  else (storage_has_writes a)

def k_transfer (src : address) (dst : address) (v : word) : SailM Unit := do
  let src_acc ← do (k_aload src)
  let dst_acc ← do (k_aload dst)
  if (((word_is_zero v) || (src == dst)) : Bool)
  then (pure ())
  else
    (do
      (store_account_info src src_acc
        { src_acc.info with balance := (alu_sub src_acc.info.balance v) })
      (store_account_info dst dst_acc
        { dst_acc.info with balance := (alu_add dst_acc.info.balance v) })
      (k_emit_transfer_log src dst v))

def k_bump_nonce (a : address) : SailM Unit := do
  let cur ← do (k_aload a)
  (store_account_info a cur
    { cur.info with nonce := ← do
        let semanticField ← (do
            let semanticResult ← (account_nonce_increment ⟨(cur.info.nonce).value⟩)
            pure ((semanticResult).value))
        pure (⟨semanticField⟩) })

def k_add_balance (a : address) (v : word) : SailM Unit := do
  let cur ← do (k_aload a)
  if ((! (word_is_zero v)) : Bool)
  then (store_account_info a cur { cur.info with balance := (alu_add cur.info.balance v) })
  else (pure ())

def k_sub_balance (a : address) (v : word) : SailM Unit := do
  let cur ← do (k_aload a)
  if ((! (word_is_zero v)) : Bool)
  then (store_account_info a cur { cur.info with balance := (alu_sub cur.info.balance v) })
  else (pure ())

def k_clear_storage (a : address) : SailM Unit := do
  let cur ← do (k_aload a)
  (storage_tx_clear a)
  (store_account a (account_clear_storage cur))

