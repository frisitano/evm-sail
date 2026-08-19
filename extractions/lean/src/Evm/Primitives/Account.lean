import Evm.Prelude
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

/-! # Accounts and storage

The account tuple and its sentinels, and the per-layer account/storage
entry types that cross the host interface (overlay rows, merge
enumeration). Pure
data — no registers, no externs. -/

def undefined_AccountInfo (_ : Unit) : SailM AccountInfo := do
  (pure { nonce := ← (undefined_range 0 ((2 ^i 64) - 1)),
          balance := ← (undefined_range 0 ((2 ^i 256) - 1)),
          code_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          storage_root := ← (undefined_vector 32 (← (undefined_bitvector 8))) })

def undefined_Account (_ : Unit) : SailM Account := do
  (pure { info := ← (undefined_AccountInfo ()),
          present := ← (undefined_bool ()),
          storage_cleared := ← (undefined_bool ()),
          created := ← (undefined_bool ()),
          selfdestructed := ← (undefined_bool ()) })

def undefined_StorageValue (_ : Unit) : SailM StorageValue := do
  (pure { curr := ← (undefined_range 0 ((2 ^i 256) - 1)),
          orig := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_StorageKey (_ : Unit) : SailM StorageKey := do
  (pure { addr := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          slot := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_StorageEntry (_ : Unit) : SailM StorageEntry := do
  (pure { key := ← (undefined_StorageKey ()),
          value := ← (undefined_StorageValue ()) })

def undefined_StorageBlockRow (_ : Unit) : SailM StorageBlockRow := do
  (pure { found := ← (undefined_bool ()),
          value := ← (undefined_StorageValue ()) })

def undefined_AcctValue (_ : Unit) : SailM AcctValue := do
  (pure { curr := ← (undefined_Account ()),
          orig := ← (undefined_Account ()) })

def undefined_AcctEntry (_ : Unit) : SailM AcctEntry := do
  (pure { addr := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          value := ← (undefined_AcctValue ()) })

def undefined_AccountRow (_ : Unit) : SailM AccountRow := do
  (pure { found := ← (undefined_bool ()),
          account := ← (undefined_Account ()) })

def undefined_StorageTrieEntry (_ : Unit) : SailM StorageTrieEntry := do
  (pure { entry := ← (undefined_StorageEntry ()),
          address_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          slot_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))) })

def undefined_AcctTrieEntry (_ : Unit) : SailM AcctTrieEntry := do
  (pure { entry := ← (undefined_AcctEntry ()),
          address_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))) })

/-- The empty account tuple: zero nonce and balance, `KECCAK_EMPTY` code
hash, empty-trie storage root (YP §4.1). -/
def EMPTY_ACCOUNT_INFO : AccountInfo :=
  { nonce := 0,
    balance := ZERO_WORD,
    code_hash := KECCAK_EMPTY,
    storage_root := EMPTY_TRIE_ROOT }

/-- The non-existent account sentinel (EIP-161 "empty"). -/
def EMPTY_ACCOUNT : Account :=
  { info := EMPTY_ACCOUNT_INFO,
    present := false,
    storage_cleared := true,
    created := false,
    selfdestructed := false }

/-- Wraps a witnessed account tuple as an existing, unmodified account. -/
def account_from_info (info : AccountInfo) : Account :=
  { info := info,
    present := true,
    storage_cleared := false,
    created := false,
    selfdestructed := false }
