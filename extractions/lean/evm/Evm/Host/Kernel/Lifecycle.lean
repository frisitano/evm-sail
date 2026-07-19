import Evm.Flow
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

/-! # State: the transaction lifecycle

Snapshot, revert, per-transaction reset, and the transaction-end merge
into the block layer. -/

/-- Captures all frame-revertible transaction state. The frame checkpoint
stores its refund counter separately. -/
def k_state_checkpoint (_ : Unit) : SailM StateCheckpoint := do
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

/-- Whether a selfdestructed account is actually deleted at transaction
end: always before Cancun; only if created in the same transaction
from Cancun on (EIP-6780). -/
def account_deleted_at_tx_end (acc : Account) : SailM Bool := do
  (pure (acc.selfdestructed && ((fork_lt (← readReg k_fork) Cancun) || acc.created)))

/-- The transaction-end merge: drains the transaction overlays into the
block layer, applying EIP-6780 deletion (with the EIP-7708 burn log),
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
                  (k_emit_burn_log e.addr curr.info.balance)
                  (pure (account_delete curr)))
              else (pure curr) ) : SailM Account )
            if ((deleted || (curr.storage_cleared && (! e.value.orig.storage_cleared))) : Bool)
            then (storage_block_clear e.addr)
            else (pure ())
            if (((curr.info.nonce).value != (e.value.orig.info.nonce).value) : Bool)
            then (bal_nonce_change e.addr ⟨(curr.info.nonce).value⟩)
            else (pure ())
            if ((bne curr.info.balance e.value.orig.info.balance) : Bool)
            then (bal_balance_change e.addr curr.info.balance)
            else (pure ())
            if ((bne curr.info.code_hash e.value.orig.info.code_hash) : Bool)
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
                if ((acc.present && (bne e.value.curr e.value.orig)) : Bool)
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

/-- Atomically restores the transaction state captured at a frame boundary. -/
def k_revert (checkpoint : StateCheckpoint) : SailM Unit := do
  (state_revert checkpoint)

