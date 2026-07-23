import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Primitives.System
import Evm.Host.Kernel.Environment

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

/-! # State: logs and refunds

Log emission (YP §4.4.1) — including the EIP-7708 transfer and burn
logs — and the gas-refund counter. -/

/-- Appends a log record (YP §4.4.1) to the transaction's log series. -/
def k_log (a : address) (topics : (List word)) (data : Bytes) : SailM Unit := do
  let topics := (List.map (fun semanticValue => (semanticValue).value) (topics))
  (log_append a (List.map (fun semanticValue => ⟨semanticValue⟩) (topics)) data)

/-- Emits the EIP-7708 transfer log for a nonzero, non-self value
transfer (Amsterdam onward). -/
/- Type quantifiers: k_ex410497_ : Nat, 0 ≤ k_ex410497_ ∧ k_ex410497_ ≤ (2 ^ 256 - 1) -/
def k_emit_transfer_log (src : address) (dst : address) (v : word) : SailM Unit := do
  let v := (v).value
  if (((fork_lt (← readReg k_fork) Amsterdam) || ((word_is_zero v) || (src == dst))) : Bool)
  then (pure ())
  else
    (log_append EIP7708_SYSTEM_ADDRESS
      (List.map (fun semanticValue => ⟨semanticValue⟩) ([(EIP7708_TRANSFER_TOPIC).value, ((address_to_word
        src)).value, ((address_to_word dst)).value]))
      (bytes_list (word_to_bytes32 ⟨v⟩) WORD_BYTE_LENGTH))

/-- Emits the EIP-7708 burn log when a selfdestruct deletion burns a
nonzero balance (Amsterdam onward). -/
/- Type quantifiers: k_ex410498_ : Nat, 0 ≤ k_ex410498_ ∧ k_ex410498_ ≤ (2 ^ 256 - 1) -/
def k_emit_burn_log (a : address) (v : word) : SailM Unit := do
  let v := (v).value
  if (((fork_lt (← readReg k_fork) Amsterdam) || (word_is_zero v)) : Bool)
  then (pure ())
  else
    (log_append EIP7708_SYSTEM_ADDRESS
      (List.map (fun semanticValue => ⟨semanticValue⟩) ([(EIP7708_BURN_TOPIC).value, ((address_to_word
        a)).value])) (bytes_list (word_to_bytes32 ⟨v⟩) WORD_BYTE_LENGTH))

