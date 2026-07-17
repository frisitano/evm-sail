import Evm.Prelude
import Evm.Primitives.Crypto

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

noncomputable section
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
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def undefined_AccountInfo (_ : Unit) : SailM AccountInfo := do
  (pure { nonce := ← (undefined_range 0 ((2 ^i 64) -i 1))
          balance := ← (undefined_bitvector 256)
          code_hash := ← (undefined_bitvector 256)
          storage_root := ← (undefined_bitvector 256) })

def undefined_Account (_ : Unit) : SailM Account := do
  (pure { info := ← (undefined_AccountInfo ())
          present := ← (undefined_bool ())
          storage_cleared := ← (undefined_bool ())
          created := ← (undefined_bool ())
          selfdestructed := ← (undefined_bool ()) })

def undefined_StorageValue (_ : Unit) : SailM StorageValue := do
  (pure { curr := ← (undefined_bitvector 256)
          orig := ← (undefined_bitvector 256) })

def undefined_StorageKey (_ : Unit) : SailM StorageKey := do
  (pure { addr := ← (undefined_bitvector 160)
          slot := ← (undefined_bitvector 256) })

def undefined_StorageEntry (_ : Unit) : SailM StorageEntry := do
  (pure { key := ← (undefined_StorageKey ())
          value := ← (undefined_StorageValue ()) })

def undefined_AcctValue (_ : Unit) : SailM AcctValue := do
  (pure { curr := ← (undefined_Account ())
          orig := ← (undefined_Account ()) })

def undefined_AcctEntry (_ : Unit) : SailM AcctEntry := do
  (pure { addr := ← (undefined_bitvector 160)
          value := ← (undefined_AcctValue ()) })

def EMPTY_ACCOUNT_INFO : AccountInfo :=
  { nonce := 0
    balance := ZERO_WORD
    code_hash := KECCAK_EMPTY
    storage_root := EMPTY_TRIE_ROOT }

def EMPTY_ACCOUNT : Account :=
  { info := EMPTY_ACCOUNT_INFO
    present := false
    storage_cleared := true
    created := false
    selfdestructed := false }

def account_from_info (info : AccountInfo) : Account :=
  { info := info
    present := true
    storage_cleared := false
    created := false
    selfdestructed := false }

