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
open StateCheckpoint
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def k_selfdestruct (a : address) : SailM Unit := do
  let cur ← do (k_aload a)
  if ((! cur.selfdestructed) : Bool)
  then (store_account a { cur with selfdestructed := true })
  else (pure ())

def k_is_selfdestructed (a : address) : SailM Bool := do
  (pure (← (k_aload a)).selfdestructed)

def k_mark_created (a : address) : SailM Unit := do
  let cur ← do (k_aload a)
  (store_account a { cur with created := true })

def k_was_created (a : address) : SailM Bool := do
  (pure (← (k_aload a)).created)

def k_zero_balance (a : address) : SailM Unit := do
  let cur ← do (k_aload a)
  if ((word_is_zero cur.info.balance) : Bool)
  then (pure ())
  else (store_account_info a cur { cur.info with balance := ZERO_WORD })

