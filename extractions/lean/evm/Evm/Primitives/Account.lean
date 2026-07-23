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

/-! # Accounts and storage

The account tuple and its sentinels, and the per-layer account/storage
entry types that cross the host interface (overlay rows, merge
enumeration). Pure
data — no registers, no externs. -/

def undefined_AccountInfo (_ : Unit) : SailM AccountInfo := do
  (pure { nonce := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩),
          balance := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩),
          code_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          storage_root := ← (undefined_vector 32 (← (undefined_bitvector 8))) })

def undefined_Account (_ : Unit) : SailM Account := do
  (pure { info := ← (undefined_AccountInfo ()),
          present := ← (undefined_bool ()),
          storage_cleared := ← (undefined_bool ()),
          created := ← (undefined_bool ()),
          selfdestructed := ← (undefined_bool ()) })

def undefined_StorageValue (_ : Unit) : SailM StorageValue := do
  (pure { curr := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩),
          orig := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩) })

def undefined_StorageKey (_ : Unit) : SailM StorageKey := do
  (pure { addr := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          slot := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩) })

def undefined_StorageEntry (_ : Unit) : SailM StorageEntry := do
  (pure { key := ← (undefined_StorageKey ()),
          value := ← (undefined_StorageValue ()) })

def undefined_AcctValue (_ : Unit) : SailM AcctValue := do
  (pure { curr := ← (undefined_Account ()),
          orig := ← (undefined_Account ()) })

def undefined_AcctEntry (_ : Unit) : SailM AcctEntry := do
  (pure { addr := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          value := ← (undefined_AcctValue ()) })

/-- The empty account tuple: zero nonce and balance, `KECCAK_EMPTY` code
hash, empty-trie storage root (YP §4.1). -/
def EMPTY_ACCOUNT_INFO : AccountInfo :=
  { nonce := ⟨0⟩,
    balance := ⟨(ZERO_WORD).value⟩,
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

