import Evm.Primitives.Account

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

/-! # State journal

The frame-revertible state journal is a closed operation algebra. Every
mutation records exactly the semantic field needed to restore its predecessor;
checkpoint and commit markers encode the nesting structure in the same ordered
stream. Backends may refine addresses and storage keys to dense identifiers,
but may not introduce rollback behavior outside these variants.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

def undefined_JournalTransientChange (_ : Unit) : SailM JournalTransientChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          slot := ← (undefined_range 0 ((2 ^i 256) - 1)),
          prior := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_JournalWarmAccountChange (_ : Unit) : SailM JournalWarmAccountChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior_epoch := ← (undefined_range 0 ((2 ^i 20) + 1)) })

def undefined_JournalWarmStorageChange (_ : Unit) : SailM JournalWarmStorageChange := do
  (pure { key := ← (undefined_StorageKey ()),
          prior_epoch := ← (undefined_range 0 ((2 ^i 20) + 1)) })

def undefined_JournalAccountBalanceChange (_ : Unit) : SailM JournalAccountBalanceChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_JournalAccountNonceChange (_ : Unit) : SailM JournalAccountNonceChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_range 0 ((2 ^i 64) - 1)) })

def undefined_JournalAccountCodeHashChange (_ : Unit) : SailM JournalAccountCodeHashChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_vector 32 (← (undefined_bitvector 8))) })

def undefined_JournalAccountExistsChange (_ : Unit) : SailM JournalAccountExistsChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_bool ()) })

def undefined_JournalAccountCreatedChange (_ : Unit) : SailM JournalAccountCreatedChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_bool ()) })

def undefined_JournalAccountSelfdestructedChange (_ : Unit) : SailM JournalAccountSelfdestructedChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_bool ()) })

def undefined_JournalStorageValueChange (_ : Unit) : SailM JournalStorageValueChange := do
  (pure { key := ← (undefined_StorageKey ()),
          prior := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_JournalStorageRowGenerationChange (_ : Unit) : SailM JournalStorageRowGenerationChange := do
  (pure { key := ← (undefined_StorageKey ()),
          prior := ← (undefined_range 0 ((2 ^i 32) - 1)) })

def undefined_JournalAccountStorageGenerationChange (_ : Unit) : SailM JournalAccountStorageGenerationChange := do
  (pure { address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          prior := ← (undefined_range 0 ((2 ^i 32) - 1)) })
