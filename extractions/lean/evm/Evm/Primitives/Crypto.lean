import Evm.Vector
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Lib.Bytes

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

/-! # Cryptographic primitives

The axiomatic hash core — `keccak256` (the `KECCAK256` opcode, trie/RLP
node references, `CREATE2` salting) and `sha256` (precompile `0x02` and SSZ
merkleization) as pure functions of `Bytes` segment lists — plus the
well-known digests of fixed inputs. Loaded before the account types:
`EMPTY_ACCOUNT` and the kernel's emptiness checks read the digests long
before `lib/mpt/primitives.sail` loads.

The `c:`-bound `val`s in this module form the accelerator interface:
hash/signature primitives with no pure Sail body (`ffi/host_crypto.c`,
`ffi/precompiles.c` — the eth-act zkvm-standards accelerator surface).
Proof targets see them as bodyless axioms.

| Name | Value | Description |
| ---- | ----- | ----------- |
| `KECCAK_EMPTY` | `keccak256("")` | The codeless account's code hash |
| `EMPTY_TRIE_ROOT` | `keccak256(rlp(""))` | The empty trie's root |
| `SECP_N_FULL` / `SECP_N_HALF` | `n`, `n/2` | The secp256k1 group order bounds (EIP-2) | -/

/-- keccak256 of the empty string: the `codeHash` of every codeless
account. -/
def KECCAK_EMPTY : hash :=
  (B256
    (to_bytes_le (n := 32) 0xC5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470#256))

/-- `keccak256(rlp(""))` — the root of an empty Merkle-Patricia trie: the
storage root of every account with no storage (`EMPTY_ACCOUNT`, freshly
created). -/
def EMPTY_TRIE_ROOT : hash :=
  (B256
    (to_bytes_le (n := 32) 0x56E81F171BCC55A6FF8345E692C0F86E5B48E01B996CADC001622FB5E363B421#256))

/-- KECCAK-256 of a source-tagged slice, without materializing it. -/
/- Type quantifiers: k_ex409105_ : Nat, k_ex409104_ : Nat, 0 ≤ k_ex409104_ ∧ 0 ≤ k_ex409105_ -/
def keccak256_slice (s : EvmByteSlice) : SailM hash := do
  let s := ((s).2).2
  (keccak256_segments [(BytesSlice ⟨_, ⟨_, s⟩⟩)])

/-- SHA-256 of a source-tagged slice, without materializing it. -/
/- Type quantifiers: k_ex409109_ : Nat, k_ex409108_ : Nat, 0 ≤ k_ex409108_ ∧ 0 ≤ k_ex409109_ -/
def sha256_slice (s : EvmByteSlice) : SailM hash := do
  let s := ((s).2).2
  (sha256_segments [(BytesSlice ⟨_, ⟨_, s⟩⟩)])

/-- KECCAK-256 of a single word. Fixed-width helper with pure meaning over
[keccak256_segments][]; implementations may override
it with a one-call accelerator (`host_keccak_word`) — secure-trie keys
hash on every state access. -/
/- Type quantifiers: w : Nat, 0 ≤ w ∧ w ≤ (2 ^ 256 - 1) -/
def keccak256_word (w : word) : SailM hash := do
  let w := (w).value
  (keccak256_segments [(bytes_list (word_to_bytes32 ⟨w⟩) WORD_BYTE_LENGTH)])

/-- KECCAK-256 of a 20-byte address (secure-trie account keys). -/
def keccak256_address (a : address) : SailM hash := do
  (keccak256_segments [(bytes_list (address_to_bytes a) ADDRESS_BYTE_LENGTH)])

/-- The SSZ Merkle parent: `SHA-256(left ++ right)` over two 32-byte
chunks. -/
def sha256_pair (a : hash) (b : hash) : SailM hash := do
  (sha256_segments
    [(bytes_list (hash_to_bytes32 a) WORD_BYTE_LENGTH), (bytes_list (hash_to_bytes32 b)
      WORD_BYTE_LENGTH)])

/-- `n` of the secp256k1 group order. -/
def SECP_N_FULL : word :=
  ⟨((U256 (BitVec.toNatInt 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141#256))).value⟩

/-- `n/2` of the secp256k1 group order — the EIP-2 low-`s` malleability
bound. -/
def SECP_N_HALF : word :=
  ⟨((U256 (BitVec.toNatInt 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0#256))).value⟩

/-- Recovers the signer address from `(h, y_parity, r, s)`, returning
recovery success and the recovered address (used by EIP-7702). -/
/- Type quantifiers: k_ex409121_ : Nat, k_ex409120_ : Nat, k_ex409119_ : Nat, 0 ≤ k_ex409119_ ∧
  k_ex409119_ ≤ 1, 0 ≤ k_ex409120_ ∧ k_ex409120_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex409121_ ∧
  k_ex409121_ ≤ (2 ^ 256 - 1) -/
def ecrecover_addr (h : hash) (yparity : y_parity) (r : word) (s : word) : SailM (Bool × address) := do
  let yparity := (yparity).value
  let r := (r).value
  let s := (s).value
  let recovered ← do (host_ecrecover h ⟨yparity⟩ ⟨r⟩ ⟨s⟩)
  (pure (recovered.success, recovered.address))

