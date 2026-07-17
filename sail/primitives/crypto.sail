
/*md
# Cryptographic primitives

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
| `SECP_N_FULL` / `SECP_N_HALF` | `n`, `n/2` | The secp256k1 group order bounds (EIP-2) |
*/
/*! keccak256 of the empty string: the `codeHash` of every codeless
    account. */
let KECCAK_EMPTY : hash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/*! `keccak256(rlp(""))` — the root of an empty Merkle-Patricia trie: the
    storage root of every account with no storage (`EMPTY_ACCOUNT`, freshly
    created). */
let EMPTY_TRIE_ROOT : hash = 0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421

/* ---- Crypto core. The `c:`-bound vals below are the axiomatic boundary:
 * hash/signature primitives with NO pure Sail body (ffi/host_crypto.c,
 * ffi/precompiles.c -- the eth-act zkvm-standards accelerator surface); proof
 * targets see them as bodyless axioms (the c: key binds only C). keccak256 and
 * sha256 are the axiomatic hash core. All shared consensus
 * hashing (lib/consensus_hash.sail) is written against these three. The
 * StatelessInput/Memory/Code source of a ByteSlice denotes its abstract byte
 * sequence directly: keccak256(s : ByteSlice) hashes exactly that sequence;
 * the prefixed SHA-256 digest = sha256(type_byte :: bytes). ---- */

/*md
## The hash axioms

Pure functions of a segment list. A preimage is accumulated Sail-side as
`list(Bytes)` — its denotation is the concatenation of the segments — and
crosses the host interface in one call; the implementation
(`ffi/hash_glue.c`) walks the segment list natively, resolves each
`BytesSlice` from its region, and hashes. No channel state, no
per-byte crossings.

The `StatelessInput`/`Memory`/`Code` source of a
[ByteSlice][type-ByteSlice] denotes an abstract byte sequence:
`keccak256(s : ByteSlice) = keccak256(read_byte_slice(s))`. All shared
consensus hashing is written against [keccak256_segments][] and
[sha256_segments][]. RLP hashing constructs the domain, list prefix, and
payload as separate segments of the same preimage.
*/
/*! The KECCAK-256 axiom: the digest of a segment list's concatenation
    (bodyless; bound to `host_keccak_segments` in C). */
val keccak256_segments = impure { c: "host_keccak_segments" } : list(Bytes) -> bits(256)
/*! The SHA-256 axiom: the digest of a segment list's concatenation
    (bodyless; bound to `host_sha256_segments` in C). */
val sha256_segments = impure { c: "host_sha256_segments" } : list(Bytes) -> bits(256)

/*! KECCAK-256 of a source-tagged slice, without materializing it. */
function keccak256_slice(s : ByteSlice) -> bits(256) = keccak256_segments([|BytesSlice(s)|])

/* Materialized fragments always enter through Bytes with an explicit length;
 * source-backed payloads can use the single-slice overload directly. */
overload keccak256 = { keccak256_slice, keccak256_segments }

overload sha256 = { sha256_segments }

/*! KECCAK-256 of a single word. Fixed-width helper with pure meaning over
    [keccak256_segments][]; implementations may override
    it with a one-call accelerator (`host_keccak_word`) — secure-trie keys
    hash on every state access. */
function keccak256_word(w : word) -> bits(256) =
    keccak256([|bytes_list(word_to_bytes32(w), WORD_BYTE_LENGTH)|])

/*! KECCAK-256 of a 20-byte address (secure-trie account keys). */
function keccak256_address(a : address) -> bits(256) =
    keccak256([|bytes_list(address_to_bytes(a), ADDRESS_BYTE_LENGTH)|])

/*! The SSZ Merkle parent: `SHA-256(left ++ right)` over two 32-byte
    chunks. */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    sha256([|bytes_list(word_to_bytes32(a), WORD_BYTE_LENGTH), bytes_list(word_to_bytes32(b), WORD_BYTE_LENGTH)|])

/*md
## The signature core

The `host/accelerators.sail` k256 axioms, plus the curve-order constants
that signature validity rules bound `r`/`s` against.
*/
/*! `n` of the secp256k1 group order. */
let SECP_N_FULL : bits(256) = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
/*! `n/2` of the secp256k1 group order — the EIP-2 low-`s` malleability
    bound. */
let SECP_N_HALF : bits(256) = 0x7fffffffffffffffffffffffffffffff5d576e7357a4501ddfe92f46681b20a0

/*! Recovers the signer address from `(h, y_parity, r, s)`; the leading
    bit of the recovery result flags success (used by EIP-7702 authority
    recovery). */
function ecrecover_addr(h : bits(256), yparity : y_parity, r : word, s : word) -> (bool, address) = {
    let recovered = host_ecrecover(h, yparity, r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}
