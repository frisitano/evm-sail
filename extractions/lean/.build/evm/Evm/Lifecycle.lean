import Evm.Flow
import Evm.Quantities
import Evm.Environment
import Evm.Logs
import Evm.Accounts

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

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

def apply_undo (e : JEntry) : SailM Unit := do
  match e with
  | .JTran (a, s, v) => (transient_store a s v)
  | .JWarmA a => (warm_addr_remove a)
  | .JWarmS (a, s) => (warm_slot_remove a s)

def k_state_checkpoint (_ : Unit) : SailM StateCheckpoint := do
  (pure { journal := ← (journal_len ())
          accounts := ← (acct_tx_checkpoint ())
          storage := ← (storage_tx_checkpoint ())
          logs := ← (logs_checkpoint ()) })

def k_set_header (h : BlockHeader) : SailM Unit := do
  writeReg k_header h

def k_set_tx (env : TxEnv) : SailM Unit := do
  writeReg k_tx env

def k_tx_reset (_ : Unit) : SailM Unit := do
  (acct_tx_reset ())
  (storage_tx_reset ())
  (warm_reset ())
  (transient_reset ())
  (logs_tx_reset ())
  (journal_reset ())

def account_deleted_at_tx_end (acc : Account) : SailM Bool := do
  (pure (acc.selfdestructed && ((fork_lt (← readReg k_fork) Cancun) || acc.created)))

def k_tx_merge (_ : Unit) : SailM Unit := do
  let more : Bool := true
  let more ← (( do
    let mut loop_vars := more
    while (λ more => more) loop_vars do
      let more := loop_vars
      loop_vars ← do
        match (← (acct_tx_pop_ascending ())) with
        | .some e =>
          (do
            let curr : Account := e.value.curr
            let deleted ← do (account_deleted_at_tx_end curr)
            let curr ← (( do
              if (deleted : Bool)
              then
                (do
                  (k_emit_burn_log e.addr curr.info.balance)
                  (pure (account_delete curr)))
              else (pure curr) ) : SailM Account )
            if ((deleted || (curr.storage_cleared && (! e.value.orig.storage_cleared))) : Bool)
            then (storage_block_clear e.addr)
            else (pure ())
            if ((curr.info.nonce != e.value.orig.info.nonce) : Bool)
            then (bal_nonce_change e.addr curr.info.nonce)
            else (pure ())
            if ((curr.info.balance != e.value.orig.info.balance) : Bool)
            then (bal_balance_change e.addr curr.info.balance)
            else (pure ())
            if ((curr.info.code_hash != e.value.orig.info.code_hash) : Bool)
            then (bal_code_change e.addr curr.info.code_hash)
            else (pure ())
            let curr : Account := { curr with created := false, selfdestructed := false }
            if ((account_changed curr e.value.orig) : Bool)
            then
              (acct_block_write
                { addr := e.addr
                  value := { curr := curr
                             orig := e.value.orig } })
            else (pure ())
            (pure more))
        | none => (pure false)
    (pure loop_vars) ) : SailM Bool )
  let more : Bool := true
  let more ← (( do
    let mut loop_vars_1 := more
    while (λ more => more) loop_vars_1 do
      let more := loop_vars_1
      loop_vars_1 ← do
        match (← (storage_tx_pop ())) with
        | .some e =>
          (do
            match (← (acct_block_get e.key.addr)) with
            | .some acc =>
              (do
                if ((acc.exists && (e.value.curr != e.value.orig)) : Bool)
                then
                  (do
                    (bal_storage_change e.key.addr e.key.slot e.value.curr)
                    (storage_block_put e))
                else (pure ()))
            | none => (pure ())
            (pure more))
        | none => (pure false)
    (pure loop_vars_1) ) : SailM Bool )
  (acct_tx_reset ())
  (storage_tx_reset ())

def k_revert (checkpoint : StateCheckpoint) : SailM Unit := do
  (acct_tx_revert checkpoint.accounts)
  (storage_tx_revert checkpoint.storage)
  (logs_revert checkpoint.logs)
  let current ← do (journal_len ())
  let saved := checkpoint.journal
  let remaining ← (( do
    if ((saved ≤b current) : Bool)
    then (pure (current -i saved))
    else
      (do
        assert false "sail/host/kernel/lifecycle.sail:89.24-89.25"
        throw Error.Exit) ) : SailM JournalCheckpoint )
  let remaining ← (( do
    let mut loop_vars := remaining
    while (λ remaining => (remaining != 0)) loop_vars do
      let remaining := loop_vars
      loop_vars ← do
        (apply_undo (← (journal_pop ())))
        (protocol_quantity_decrement remaining)
    (pure loop_vars) ) : SailM Nat )
  (pure ())

