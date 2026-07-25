import Evm.Flow
import Evm.Host.Kernel.Environment
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

/-! # State: the transaction lifecycle

Snapshot, revert, per-transaction reset, and the transaction-end merge
into the block layer. -/

/-- Captures all frame-revertible transaction state. The frame checkpoint
stores its refund counter separately. -/
def k_state_checkpoint (_ : Unit) : SailM Nat := do
  (state_checkpoint ())

/-- Installs the block header. -/
def k_set_header (h : BlockHeader) : SailM Unit := do
  writeReg k_header h

/-- Installs the per-transaction environment. -/
def k_set_tx (env : TxEnv) : SailM Unit := do
  writeReg k_tx env

/-- Resets every per-transaction store and the private checkpoint history. -/
def k_tx_reset (_ : Unit) : SailM Unit := do
  (acct_tx_reset ())
  (storage_tx_reset ())
  (warm_reset ())
  (transient_reset ())
  (logs_tx_reset ())
  (state_checkpoint_reset ())

/-- Whether a selfdestructed account is cleared at transaction end: always
before Cancun; only if created in the same transaction from Cancun on
(EIP-6780). Amsterdam preserves any balance left by a self-beneficiary
`SELFDESTRUCT` (EIP-8246). -/
def account_deleted_at_tx_end (acc : Account) : SailM Bool := do
  if (acc.selfdestructed : Bool)
  then
    (do
      (pure ((fork_lt (← readReg k_fork) Cancun) || acc.created)))
  else (pure false)

/-- The transaction-end merge: drains the transaction overlays into the
block layer, applying the fork-specific selfdestruct clearing rule,
storage-clear generations, and recording nonce/balance/code/storage
changes for the EIP-7928 block access list. Lifecycle flags reset as
rows merge. -/
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
                  let curr ← (( do
                    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
                    then (pure (account_clear_preserving_balance curr))
                    else (pure (account_delete curr)) ) : SailM Account )
                  (storage_tx_clear e.addr)
                  (pure curr))
              else (pure curr) ) : SailM Account )
            if ((deleted || (curr.storage_cleared && (! e.value.orig.storage_cleared))) : Bool)
            then (storage_block_clear e.addr)
            else (pure ())
            if ((curr.info.nonce != e.value.orig.info.nonce) : Bool)
            then (bal_nonce_change (← readReg k_block_access_index) e.addr curr.info.nonce)
            else (pure ())
            if ((curr.info.balance != e.value.orig.info.balance) : Bool)
            then (bal_balance_change (← readReg k_block_access_index) e.addr curr.info.balance)
            else (pure ())
            if ((bne curr.info.code_hash e.value.orig.info.code_hash) : Bool)
            then (bal_code_change (← readReg k_block_access_index) e.addr curr.info.code_hash)
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
        | none =>
          (let more : Bool := false
          (pure more))
    (pure loop_vars) ) : SailM Bool )
  let more : Bool := true
  let more ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun more => (pure more)) more
      fun more => do
        assert true "loop dummy assert"
        match (← (storage_tx_pop ())) with
        | .some e =>
          (do
            match (← (acct_block_get e.key.addr)) with
            | .some acc =>
              (do
                if ((acc.present && ((e.value.curr != e.value.orig) : Bool)) : Bool)
                then
                  (do
                    (bal_storage_change (← readReg k_block_access_index) e.key.addr e.key.slot
                      e.value.curr)
                    (storage_block_put e))
                else (pure ()))
            | none => (pure ())
            (pure more))
        | none =>
          (let more : Bool := false
          (pure more))
    (pure loop_vars) ) : SailM Bool )
  (acct_tx_reset ())
  (storage_tx_reset ())

/-- Atomically restores the transaction state captured at a frame boundary. -/
/- Type quantifiers: checkpoint : Nat, 0 ≤ checkpoint -/
def k_revert (checkpoint : Nat) : SailM Unit := do
  (state_revert checkpoint)

