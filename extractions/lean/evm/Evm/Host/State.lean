import Sail
import Evm.Defs
import Evm.Specialization
import Evm.FakeReal
import Evm.HostAxioms

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

/-! # State store axioms

The host-side mutable state stores, as bodyless axioms: transient storage
(EIP-1153), the two-layer account and storage overlays (transaction over
block), the EIP-7928 block-access-list recorder, the EIP-2929 warm sets,
and the log store. A single semantic checkpoint atomically captures every
frame-revertible component. Its implementation may use cursors, snapshots,
or an undo journal, but those representation choices are unobservable to the
model.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

def undefined_BalStorageChangeEntry (_ : Unit) : SailM BalStorageChangeEntry := do
  (pure { slot := ← (undefined_range 0 ((2 ^i 256) - 1)),
          index := ← (undefined_range 0 ((2 ^i 20) + 1)),
          value := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_BalBalanceChangeEntry (_ : Unit) : SailM BalBalanceChangeEntry := do
  (pure { index := ← (undefined_range 0 ((2 ^i 20) + 1)),
          value := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_BalNonceChangeEntry (_ : Unit) : SailM BalNonceChangeEntry := do
  (pure { index := ← (undefined_range 0 ((2 ^i 20) + 1)),
          value := ← (undefined_range 0 ((2 ^i 64) - 1)) })

def undefined_BalCodeChangeEntry (_ : Unit) : SailM BalCodeChangeEntry := do
  (pure { index := ← (undefined_range 0 ((2 ^i 20) + 1)),
          code_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))) })

