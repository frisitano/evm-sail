import Evm.Primitives.Crypto

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

/-! # Validation debug (optional)

Default build variant discarding the optional validation debug
records.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- Clears the recorded failure at the start of a validation attempt. -/
def validation_debug_reset (_ : Unit) : Unit :=
  ()

/-- Records the failing validation stage and reason for host
inspection; a later record overwrites an earlier one. -/
def validation_debug_record (_scope : (BitVec 8)) (_reason : BlockError) : Unit :=
  ()

def debug_account_storage_root (_a : address) : hash :=
  EMPTY_TRIE_ROOT

def debug_rebuild_state_root (_ : Unit) : hash :=
  EMPTY_TRIE_ROOT

