import Evm.Prelude
import Evm.Host.Kernel.Storage
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
open BalIterEntry

/-! # State: selfdestruct and creation flags

The per-transaction account lifecycle flags behind `SELFDESTRUCT`
(EIP-6780) and same-transaction creation tracking. -/

/-- Marks an account selfdestructed (`SELFDESTRUCT`; deletion is decided
at transaction end per EIP-6780). -/
def k_selfdestruct (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  if ((! cur.selfdestructed) : Bool)
  then (store_account a { cur with selfdestructed := true })
  else (pure ())

/-- Whether the account is marked selfdestructed this transaction. -/
def k_is_selfdestructed (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  (pure (← (k_aload a)).selfdestructed)

/-- Marks an account as created in this transaction (EIP-6780's
same-transaction test). -/
def k_mark_created (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  (store_account a { cur with created := true })

/-- Whether the account was created in this transaction. -/
def k_was_created (a : (Vector (BitVec 8) 20)) : SailM Bool := do
  (pure (← (k_aload a)).created)

/-- Zeroes an account's balance (the `SELFDESTRUCT` sweep of a
self-beneficiary). -/
def k_zero_balance (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  if ((word_is_zero cur.info.balance) : Bool)
  then (pure ())
  else (store_account_info a cur { cur.info with balance := ZERO_WORD })

