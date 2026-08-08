import Evm.Prelude

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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # Cryptographic primitives

The axiomatic hash core — `keccak256` (the `KECCAK256` opcode, trie/RLP
node references, `CREATE2` salting) and `sha256` (precompile `0x02` and SSZ
merkleization) as pure functions of contiguous byte slices — plus the
well-known digests of fixed inputs. Loaded before the account types:
`EMPTY_ACCOUNT` and the kernel's emptiness checks read the digests long
before `lib/mpt/primitives.sail` loads.

The `c:`-bound `val`s in this module form the accelerator interface:
hash/signature primitives with no pure Sail body (the `hash.c` and
`precompiles.c` implementations in `extractions/c/spec/contract/` and `extractions/c/optimised/contract/` call the
eth-act zkvm-standards surface directly). Proof targets see them as bodyless
axioms.

| Name | Value | Description |
| ---- | ----- | ----------- |
| `KECCAK_EMPTY` | `keccak256("")` | The codeless account's code hash |
| `EMPTY_TRIE_ROOT` | `keccak256(rlp(""))` | The empty trie's root |
| `SECP_N_FULL` / `SECP_N_HALF` | `n`, `n/2` | The secp256k1 group order bounds (EIP-2) | -/

/-- keccak256 of the empty string: the `codeHash` of every codeless
account. -/
def KECCAK_EMPTY : hash :=
  (hash_from_bits 0xC5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470#256)

/-- `keccak256(rlp(""))` — the root of an empty Merkle-Patricia trie: the
storage root of every account with no storage (`EMPTY_ACCOUNT`, freshly
created). -/
def EMPTY_TRIE_ROOT : hash :=
  (hash_from_bits 0x56E81F171BCC55A6FF8345E692C0F86E5B48E01B996CADC001622FB5E363B421#256)

/-- KECCAK-256 of a calldata slice, dispatching on its provenance. -/
def calldata_keccak256 (input : CalldataSlice) : SailM (Vector (BitVec 8) 32) := do
  match input with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ => (stateless_input_keccak256 ⟨_, ⟨_, bytes⟩⟩)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ => (memory_keccak256 ⟨_, ⟨_, bytes⟩⟩)

/-- SHA-256 of a calldata slice, dispatching on its provenance. -/
def calldata_sha256 (input : CalldataSlice) : SailM (Vector (BitVec 8) 32) := do
  match input with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ => (stateless_input_sha256 ⟨_, ⟨_, bytes⟩⟩)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ => (memory_sha256 ⟨_, ⟨_, bytes⟩⟩)

/-- `n` of the secp256k1 group order. -/
def SECP_N_FULL : word :=
  (word_from_bits 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141#256)

/-- `n/2` of the secp256k1 group order — the EIP-2 low-`s` malleability
bound. -/
def SECP_N_HALF : word :=
  (word_from_bits 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0#256)

/-- Recovers the signer address from `(h, y_parity, r, s)`, returning
recovery success and the recovered address (used by EIP-7702). -/
/- Type quantifiers: k_ex548766_ : Nat, k_ex548765_ : Nat, k_ex548764_ : Nat, 0 ≤ k_ex548764_ ∧
  k_ex548764_ ≤ 1, 0 ≤ k_ex548765_ ∧ k_ex548765_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex548766_ ∧
  k_ex548766_ ≤ (2 ^ 256 - 1) -/
def ecrecover_addr (h : (Vector (BitVec 8) 32)) (yparity : Nat) (r : Nat) (s : Nat) : SailM (Bool × (Vector (BitVec 8) 20)) := do
  let recovered ← do (host_ecrecover h yparity r s)
  (pure (recovered.success, recovered.address))

