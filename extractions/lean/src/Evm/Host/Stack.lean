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

/-! # The operand stack

The per-frame EVM operand stack (YP §9.1, μ_s): up to 1024 words, LIFO.
The words live behind the host interface (the corresponding `stack.c` in
`extractions/c/spec/contract/` or `extractions/c/optimised/contract/`) as per-frame flat arrays for O(1) access;
proof targets see the `val`s as bodyless axioms over an abstract per-frame
list-of-words model.

The active frame's position is the abstract cursor token
[StackPointer][type-StackPointer], threaded by value through the interpreter and
every opcode handler in the state-passing convention and held in the
`stack_top` frame register at frame boundaries. This module keeps
ownership of the word storage and its workspace binding — only the cursor
changes custody. Frames form a stack of stacks:
[operand_stack_push_empty_frame][] on a sub-call returns the child's
empty cursor, [operand_stack_pop_frame][] releases it on return (the
parent's cursor is restored from its frame checkpoint), and
[stack_reset][] returns the base frame's empty cursor per transaction.

The EVM layer ([push_word][] / [pop][] in the machine module) enforces the
depth bounds through [validate_stack][] before a handler body runs and
charges nothing here — this is pure mechanism, so the slot and cursor
axioms perform no re-checks.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

def stack_reset (_ : Unit) : SailM StackPointer := do
  (pure { storage := ← (stack_reset_host ()),
          height := 0 })

def operand_stack_push_empty_frame (_ : Unit) : SailM StackPointer := do
  (pure { storage := ← (operand_stack_push_empty_frame_host ()),
          height := 0 })

def stack_top_height (top : StackPointer) : Nat :=
  top.height

/- Type quantifiers: k_ex549774_ : Nat, 0 ≤ k_ex549774_ ∧ k_ex549774_ ≤ 1023 -/
def stack_slot_read (top : StackPointer) (index : Nat) : SailM Nat := do
  (stack_slot_read_host top.storage index)

/- Type quantifiers: k_ex549776_ : Nat, k_ex549775_ : Nat, 0 ≤ k_ex549775_ ∧
  k_ex549775_ ≤ 1023, 0 ≤ k_ex549776_ ∧ k_ex549776_ ≤ (2 ^ 256 - 1) -/
def stack_slot_write (top : StackPointer) (index : Nat) (value : Nat) : SailM Unit := do
  (stack_slot_write_host top.storage index value)

/- Type quantifiers: k_ex549777_ : Nat, 0 ≤ k_ex549777_ ∧ k_ex549777_ ≤ (2 ^ 256 - 1) -/
def stack_slot_write_next (top : StackPointer) (value : Nat) : SailM Unit := do
  (stack_slot_write_next_host top.storage value)

/-- Advances a stack cursor by `count` slots and refreshes its semantic height. -/
/- Type quantifiers: k_ex549778_ : Nat, 0 ≤ k_ex549778_ ∧ k_ex549778_ ≤ 1024 -/
def stack_top_advance (top : StackPointer) (count : Nat) : SailM StackPointer := do
  let storage ← do (stack_top_advance_host top.storage count)
  (pure { storage := storage,
          height := ← (stack_top_height_host storage) })

/-- Retreats a stack cursor by `count` slots and refreshes its semantic height. -/
/- Type quantifiers: k_ex549779_ : Nat, 0 ≤ k_ex549779_ ∧ k_ex549779_ ≤ 1024 -/
def stack_top_retreat (top : StackPointer) (count : Nat) : SailM StackPointer := do
  let storage ← do (stack_top_retreat_host top.storage count)
  (pure { storage := storage,
          height := ← (stack_top_height_host storage) })
