import Evm.Prelude
import Evm.Kernel.Storage
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

/-! # State: selfdestruct and creation flags

The per-transaction account lifecycle flags behind `SELFDESTRUCT`
(EIP-6780) and same-transaction creation tracking. -/

/-- Marks an account selfdestructed (`SELFDESTRUCT`; deletion is decided
at transaction end per EIP-6780). -/
def k_selfdestruct (a : (Vector (BitVec 8) 20)) : SailM Unit := do
  let cur ← do (k_aload a)
  let active := (! cur.selfdestructed)
  if (active : Bool)
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
  let balance_is_zero := (word_is_zero cur.info.balance)
  if (balance_is_zero : Bool)
  then (pure ())
  else (store_account_info a cur { cur.info with balance := ZERO_WORD })

