import Evm.Flow
import Evm.Primitives.Quantities
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Logs
import Evm.Host.Kernel.Accounts

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

def apply_undo (e : JEntry) : SailM Unit := do
  match e with
  | .JTran (a, s, v) => (transient_store a s v)
  | .JWarmA a => (warm_addr_remove a)
  | .JWarmS (a, s) => (warm_slot_remove a s)

def k_state_checkpoint (_ : Unit) : SailM StateCheckpoint := do
  (pure { journal := ← do
              let semanticField ← (do
                  let semanticResult ← (journal_len ())
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          accounts := ← do
              let semanticField ← (do
                  let semanticResult ← (acct_tx_checkpoint ())
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          storage := ← do
              let semanticField ← (do
                  let semanticResult ← (storage_tx_checkpoint ())
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          logs := ← do
              let semanticField ← (do
                  let semanticResult ← (logs_checkpoint ())
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩) })

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
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun more => (pure more)) more
      fun more => do
        assert true "loop dummy assert"
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
            if (((curr.info.nonce).value != (e.value.orig.info.nonce).value) : Bool)
            then (bal_nonce_change e.addr ⟨(curr.info.nonce).value⟩)
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
                { addr := e.addr,
                  value := { curr := curr,
                             orig := e.value.orig } })
            else (pure ())
            (pure more))
        | none => (pure false)
    (pure loop_vars) ) : SailM Bool )
  let more : Bool := true
  let more ← (( do
    let loop_vars_1 ← whileFuelM (fuel :=(2 ^i 64)) (fun more => (pure more)) more
      fun more => do
        assert true "loop dummy assert"
        match (← (storage_tx_pop ())) with
        | .some e =>
          (do
            match (← (acct_block_get e.key.addr)) with
            | .some acc =>
              (do
                if ((acc.present && (e.value.curr != e.value.orig)) : Bool)
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
  (acct_tx_revert ⟨(checkpoint.accounts).value⟩)
  (storage_tx_revert ⟨(checkpoint.storage).value⟩)
  (logs_revert ⟨(checkpoint.logs).value⟩)
  let current ← do
    (do
        let semanticResult ← (journal_len ())
        pure ((semanticResult).value))
  let saved := (checkpoint.journal).value
  let remaining ← (( do
    if ((saved ≤b current) : Bool)
    then (pure (current -i saved))
    else
      (do
        assert false "sail/host/kernel/lifecycle.sail:118.24-118.25"
        throw Error.Exit) ) : SailM Nat )
  let remaining ← (( do
    let loop_vars ← whileFuelM (fuel :=remaining) (fun remaining => (pure (remaining != 0))) remaining
      fun remaining => do
        assert true "loop dummy assert"
        (apply_undo (← (journal_pop ())))
        (do
            let semanticResult ← (protocol_quantity_decrement ⟨remaining⟩)
            pure ((semanticResult).value))
    (pure loop_vars) ) : SailM Nat )
  (pure ())

