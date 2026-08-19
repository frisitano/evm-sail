import Evm.Flow
import Evm.Primitives.Fork
import Evm.Kernel.Environment
import Evm.Kernel.Accounts

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

/-! # State: the transaction lifecycle

Journal checkpoints, rollback, per-transaction reset, and transaction-end merge
into the block layer. -/

/-- Appends a frame marker to the state journal. The suspended frame stores its
refund counter separately. -/
def k_journal_checkpoint (_ : Unit) : SailM Unit := do
  (state_journal_checkpoint ())

/-- Installs the block header. -/
def k_set_header (h : BlockHeader) : SailM Unit := do
  writeReg k_header h

/-- Installs the per-transaction environment. -/
/- Type quantifiers: env_dependentWitness0 : Nat, env_dependentWitness0 = 0 ∨
  env_dependentWitness0 = 6 ∨ env_dependentWitness0 = 9 -/
def k_set_tx (env : (Sigma fun (k_blob_limit : Nat) => (TxEnvFields k_blob_limit))) : SailM Unit := do
  let env_dependentWitness0 := (env).1
  let env := (env).2
  writeReg k_tx ⟨_, env⟩

/-- Resets every per-transaction store and the state journal. -/
def k_tx_reset (_ : Unit) : SailM Unit := do
  (storage_tx_reset ())
  (acct_tx_reset ())
  (warm_reset (← readReg k_current_transaction_epoch))
  (transient_reset ())
  (logs_tx_reset ())
  (state_journal_reset ())

def undefined_TransactionMergeSemantics (_ : Unit) : SailM TransactionMergeSemantics := do
  (pure { delete_only_created := ← (undefined_bool ()),
          preserve_selfdestruct_balance := ← (undefined_bool ()) })

/-- Selects the complete transaction-end lifecycle semantics for one fork. -/
/- Type quantifiers: fork : Nat, 0 ≤ fork ∧ fork ≤ 16 -/
def transaction_merge_semantics (fork : Nat) : TransactionMergeSemantics :=
  if ((fork ≥b Amsterdam) : Bool)
  then
    { delete_only_created := true,
      preserve_selfdestruct_balance := true }
  else
    (if ((fork ≥b Cancun) : Bool)
    then
      { delete_only_created := true,
        preserve_selfdestruct_balance := false }
    else
      { delete_only_created := false,
        preserve_selfdestruct_balance := false })

/-- Whether a selfdestructed account is cleared at transaction end: always
before Cancun; only if created in the same transaction from Cancun on
(EIP-6780). Amsterdam preserves any balance left by a self-beneficiary
`SELFDESTRUCT` (EIP-8246). -/
def account_deleted_at_tx_end (semantics : TransactionMergeSemantics) (acc : Account) : Bool :=
  (acc.selfdestructed && ((! semantics.delete_only_created) || acc.created))

/-- The transaction-end merge: drains the transaction overlays into the
block layer, applying the fork-specific selfdestruct clearing rule,
storage-clear generations, and recording nonce/balance/code/storage
changes for the EIP-7928 block access list. Lifecycle flags reset as
rows merge. -/
def k_tx_merge (_ : Unit) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let semantics := (transaction_merge_semantics profile.fork)
  let more : Bool := true
  let more ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun more => (pure more)) more
      fun more => do
        assert true "loop dummy assert"
        let popped_account ← do (acct_tx_pop ())
        match popped_account with
        | .AcctTxPopRow e =>
          (do
            let curr : Account := e.value.curr
            let deleted := (account_deleted_at_tx_end semantics curr)
            let curr ← (( do
              if (deleted : Bool)
              then
                (do
                  let cleared_account :=
                    if (semantics.preserve_selfdestruct_balance : Bool)
                    then (account_clear_preserving_balance curr)
                    else (account_delete curr)
                  let curr : Account := cleared_account
                  (storage_tx_clear e.addr)
                  (pure curr))
              else (pure curr) ) : SailM Account )
            let original_storage_retained := (! e.value.orig.storage_cleared)
            if ((deleted || (curr.storage_cleared && original_storage_retained)) : Bool)
            then (storage_block_clear e.addr)
            else (pure ())
            if ((curr.info.nonce != e.value.orig.info.nonce) : Bool)
            then (bal_nonce_change (← readReg k_current_transaction_epoch) e.addr curr.info.nonce)
            else (pure ())
            if ((curr.info.balance != e.value.orig.info.balance) : Bool)
            then
              (bal_balance_change (← readReg k_current_transaction_epoch) e.addr curr.info.balance)
            else (pure ())
            if ((bne curr.info.code_hash e.value.orig.info.code_hash) : Bool)
            then
              (bal_code_change (← readReg k_current_transaction_epoch) e.addr curr.info.code_hash)
            else (pure ())
            let curr : Account := { curr with created := false, selfdestructed := false }
            let changed := (account_changed curr e.value.orig)
            if (changed : Bool)
            then
              (acct_block_write
                { addr := e.addr,
                  value := { curr := curr,
                             orig := e.value.orig } })
            else (pure ())
            (pure more))
        | .AcctTxPopExhausted _ =>
          (let more : Bool := false
          (pure more))
    (pure loop_vars) ) : SailM Bool )
  let more : Bool := true
  let more ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun more => (pure more)) more
      fun more => do
        assert true "loop dummy assert"
        let popped_storage ← do (storage_tx_pop ())
        match popped_storage with
        | .StorageTxPopRow e =>
          (do
            let account ← do (acct_block_get e.key.addr)
            if (account.found : Bool)
            then
              (do
                let acc := account.account
                if ((acc.present && ((e.value.curr != e.value.orig) : Bool)) : Bool)
                then
                  (do
                    (bal_storage_change (← readReg k_current_transaction_epoch) e.key.addr
                      e.key.slot e.value.curr)
                    (storage_block_put e))
                else (pure ()))
            else (pure ())
            (pure more))
        | .StorageTxPopExhausted _ =>
          (let more : Bool := false
          (pure more))
    (pure loop_vars) ) : SailM Bool )
  (storage_tx_reset ())
  (acct_tx_reset ())

/-- Replays the state journal backwards to its innermost open frame boundary. -/
def k_journal_revert (_ : Unit) : SailM Unit := do
  (state_journal_revert ())

/-- Records a successful child frame without discarding its reversible entries. -/
def k_journal_commit (_ : Unit) : SailM Unit := do
  (state_journal_commit ())

