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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
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
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

/-! # Accounts and storage

The account tuple and its sentinels, and the per-layer account/storage
entry types that cross the host interface (overlay rows, merge
enumeration). Pure
data — no registers, no externs. -/

/-- The empty account tuple: zero nonce and balance, `KECCAK_EMPTY` code
hash, empty-trie storage root (YP §4.1). -/
def EMPTY_ACCOUNT_INFO : AccountInfo :=
  { nonce := ⟨0⟩,
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

