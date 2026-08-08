import Evm.Flow
import Evm.Prelude
import Evm.Exceptions
import Evm.Primitives.Crypto
import Evm.Primitives.Account
import Evm.Kernel.Storage
import Evm.Kernel.Logs

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

/-! # State: accounts

Account reads and mutations over the two-layer overlay (transaction over
block) with the witness as base, plus the EIP-161/EIP-684 predicates and
value transfer. -/

/-- Field-wise inequality of account tuples. -/
def account_info_changed (c : AccountInfo) (o : AccountInfo) : Bool :=
  ((c.nonce != o.nonce) || (((c.balance != o.balance) || ((bne c.code_hash o.code_hash) || (bne
          c.storage_root o.storage_root))) : Bool))

/-- The EIP-161 emptiness test: no code, zero nonce, zero balance. -/
def account_info_empty (info : AccountInfo) : Bool :=
  ((info.code_hash == KECCAK_EMPTY) && (((info.nonce == 0) && (word_is_zero info.balance)) : Bool))

/-- Whether an account differs from its original in any
trie-observable way. -/
def account_changed (c : Account) (o : Account) : Bool :=
  ((account_info_changed c.info o.info) || ((neq_bool c.present o.present) || (neq_bool
        c.storage_cleared o.storage_cleared)))

/-- Installs a new tuple, collapsing to the non-existent form when it is
EIP-161-empty. -/
def account_set_info (acc : Account) (info : AccountInfo) : Account :=
  let empty := (account_info_empty info)
  if (empty : Bool)
  then
    { acc with info := { EMPTY_ACCOUNT_INFO with storage_root := acc.info.storage_root }, present := false, storage_cleared := true }
  else { acc with info := info, present := true }

/-- Marks the account's storage cleared (fresh storage generation). -/
def account_clear_storage (acc : Account) : Account :=
  { acc with storage_cleared := true }

/-- The deleted form of an account: empty tuple, non-existent, storage
cleared. -/
def account_delete (acc : Account) : Account :=
  { acc with info := { EMPTY_ACCOUNT_INFO with storage_root := acc.info.storage_root }, present := false, storage_cleared := true }

/-- Clears nonce, code, and storage while preserving a nonzero balance.
Amsterdam applies this form to an account created and selfdestructed in
the same transaction (EIP-8246). -/
def account_clear_preserving_balance (acc : Account) : Account :=
  let cleared_storage := (account_clear_storage acc)
  (account_set_info cleared_storage { acc.info with nonce := 0, code_hash := KECCAK_EMPTY })

/-- Writes a whole-account row to the transaction overlay. -/
def store_account (a : (Vector (BitVec 8) 20)) (v : Account) : SailM Unit := do
  (acct_tx_update a v)

/-- Applies a tuple change with minimal overlay traffic: scalar-field
fast paths when existence/storage state is unchanged, and a storage
clear when the tuple collapses to empty. -/
def store_account_info (a : (Vector (BitVec 8) 20)) (acc : Account) (info : AccountInfo) : SailM Unit := do
  let empty := (account_info_empty info)
  if (empty : Bool)
  then (storage_tx_clear a)
  else (pure ())
  let next := (account_set_info acc info)
  if (((bne next.info.storage_root acc.info.storage_root) || ((neq_bool next.present acc.present) || (neq_bool
           next.storage_cleared acc.storage_cleared))) : Bool)
  then (store_account a next)
  else
    (do
      if ((next.info.balance != acc.info.balance) : Bool)
      then (acct_tx_set_balance a next.info.balance)
      else (pure ())
      if ((next.info.nonce != acc.info.nonce) : Bool)
      then (acct_tx_set_nonce a next.info.nonce)
      else (pure ())
      if ((bne next.info.code_hash acc.info.code_hash) : Bool)
      then (acct_tx_set_code_hash a next.info.code_hash)
      else (pure ()))

/-- The account balance (`BALANCE`, `SELFBALANCE`). -/
def k_get_balance (a : (Vector (BitVec 8) 20)) : SailM Nat := do
  (pure (← (k_aload a)).info.balance)

/-- The account nonce. -/
def k_get_nonce (a : (Vector (BitVec 8) 20)) : SailM Nat := do
  (pure (← (k_aload a)).info.nonce)

/-- Whether the account exists (post-EIP-161 sense). -/
def k_account_exists (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  (pure (← (k_aload a)).present)

/-- The EIP-161 "empty" test on the live account: zero nonce, zero
balance, no code. -/
def k_account_is_empty (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  let account ← do (k_aload a)
  (pure (account_info_empty account.info))

/-- The `CREATE`/`CREATE2`/create-transaction address-collision test
(EIP-684/EIP-7610): the target is occupied if it has code, a nonzero
nonce, or any storage. -/
def k_account_occupied (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  let acc ← do (k_aload a)
  let info := acc.info
  let anchored_storage := ((! acc.storage_cleared) && (bne info.storage_root EMPTY_TRIE_ROOT))
  if (((bne info.code_hash KECCAK_EMPTY) || (((info.nonce != 0) || anchored_storage) : Bool)) : Bool)
  then (pure true)
  else (storage_has_writes a)

/-- Moves `v` wei from `src` to `dst` (both updates recorded for frame
rollback; the EVM checks sufficiency before calling) and emits the
EIP-7708 transfer log. -/
/- Type quantifiers: k_ex551876_ : Nat, 0 ≤ k_ex551876_ ∧ k_ex551876_ ≤ (2 ^ 256 - 1) -/
def k_transfer (src : (Vector (BitVec 8) 20)) (dst : (Vector (BitVec 8) 20)) (v : Nat) : SailM Unit := do
  let src_acc ← do (k_aload src)
  let dst_acc ← do (k_aload dst)
  let value_is_zero := (word_is_zero v)
  if ((value_is_zero || (src == dst)) : Bool)
  then (pure ())
  else
    (do
      let source_balance := (alu_sub src_acc.info.balance v)
      (store_account_info src src_acc { src_acc.info with balance := source_balance })
      let destination_balance := (alu_add dst_acc.info.balance v)
      (store_account_info dst dst_acc { dst_acc.info with balance := destination_balance })
      (k_emit_transfer_log src dst v))

/-- Increments the account nonce. The `u64` increment cannot wrap:
EIP-2681 guards every path that reaches a bump. -/
def k_bump_nonce (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  let nonce := cur.info.nonce
  if ((nonce <b ((2 ^i 64) - 1)) : Bool)
  then (store_account_info a cur { cur.info with nonce := (nonce + 1) })
  else (fatal_error ExecutionInvalid)

/- Type quantifiers: v : Nat, 0 ≤ v ∧ v < (2 ^ 256) -/
def k_add_balance (a : (Vector (BitVec 8) 20)) (v : Nat) : SailM Unit := do
  let cur ← do (k_aload a)
  let value_is_zero := (word_is_zero v)
  let value_is_nonzero := (! value_is_zero)
  if (value_is_nonzero : Bool)
  then
    (do
      let balance := (alu_add cur.info.balance v)
      (store_account_info a cur { cur.info with balance := balance }))
  else (pure ())

/-- Debits `v` wei (no-op when zero; caller guarantees sufficiency). -/
/- Type quantifiers: k_ex551877_ : Nat, 0 ≤ k_ex551877_ ∧ k_ex551877_ ≤ (2 ^ 256 - 1) -/
def k_sub_balance (a : (Vector (BitVec 8) 20)) (v : Nat) : SailM Unit := do
  let cur ← do (k_aload a)
  let value_is_zero := (word_is_zero v)
  let value_is_nonzero := (! value_is_zero)
  if (value_is_nonzero : Bool)
  then
    (do
      let balance := (alu_sub cur.info.balance v)
      (store_account_info a cur { cur.info with balance := balance }))
  else (pure ())

/-- Clears the account's storage (create-time collision cleanup). -/
def k_clear_storage (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  (storage_tx_clear a)
  let cleared := (account_clear_storage cur)
  (store_account a cleared)

