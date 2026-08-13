/*md
# Cryptographic primitives

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
| `SECP_N_FULL` / `SECP_N_HALF` | `n`, `n/2` | The secp256k1 group order bounds (EIP-2) |
*/

/*! keccak256 of the empty string: the `codeHash` of every codeless
    account. */
let KECCAK_EMPTY : hash = hash_from_bits(0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470)

/*! `keccak256(rlp(""))` — the root of an empty Merkle-Patricia trie: the
    storage root of every account with no storage (`EMPTY_ACCOUNT`, freshly
    created). */
let EMPTY_TRIE_ROOT : hash = hash_from_bits(0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421)

/* ---- Crypto core. The `c:`-bound vals below are the axiomatic boundary:
 * hash/signature primitives with NO pure Sail body (each C backend's hash.c and
 * precompiles.c directly use the eth-act zkvm-standards surface); proof
 * targets see them as bodyless axioms (the c: key binds only C). keccak256 and
 * sha256 are the axiomatic hash core. All shared consensus hashing is
 * written against these two. The
 * Each nominal slice denotes its abstract byte sequence directly;
 * the prefixed SHA-256 digest = sha256(type_byte :: bytes). ---- */

/*md
## The hash axioms

Pure functions of a contiguous source-backed slice. Single-slice inputs hash
in place. Composite preimages are written in order to the reusable scratch
arena and then cross the accelerator boundary as one slice; no Sail byte or
segment list is allocated.

The slice's nominal type selects its backing region without a runtime source
tag. RLP hashing writes the domain, list prefix, and payload into one
scratch-backed preimage.
*/
/*! KECCAK-256 of a range of the immutable stateless input. */
val stateless_input_keccak256 = impure { c: "host_keccak_stateless_input" } : StatelessInputSlice -> hash
/*! KECCAK-256 of a scratch-arena range — the composite-preimage path. */
val scratch_keccak256 = impure { c: "host_keccak_scratch" } : ScratchSlice -> hash
/*! KECCAK-256 of an EVM-memory range (the `KECCAK256` opcode's input). */
val memory_keccak256 = impure { c: "host_keccak_memory" } : EvmMemorySlice -> hash
/*! KECCAK-256 of a code-region range. */
val code_keccak256 = impure { c: "host_keccak_code" } : CodeRegionSlice -> hash
/*! KECCAK-256 of a frame-output range. */
val output_keccak256 = impure { c: "host_keccak_output" } : OutputSlice -> hash
/*! KECCAK-256 of a retained log-data range. */
val log_data_keccak256 = impure { c: "host_keccak_log_data" } : LogDataSlice -> hash

/*! SHA-256 of a range of the immutable stateless input. */
val stateless_input_sha256 = impure { c: "host_sha256_stateless_input" } : StatelessInputSlice -> hash
/*! SHA-256 of a scratch-arena range. */
val scratch_sha256 = impure { c: "host_sha256_scratch" } : ScratchSlice -> hash
/*! SHA-256 of an EVM-memory range. */
val memory_sha256 = impure { c: "host_sha256_memory" } : EvmMemorySlice -> hash

/*! KECCAK-256 of a calldata slice, dispatching on its provenance. */
function calldata_keccak256(input : CalldataSlice) -> hash =
    match input {
        InputCalldata(bytes) => stateless_input_keccak256(bytes),
        MemoryCalldata(bytes) => memory_keccak256(bytes),
    }

/*! SHA-256 of a calldata slice, dispatching on its provenance. */
function calldata_sha256(input : CalldataSlice) -> hash =
    match input {
        InputCalldata(bytes) => stateless_input_sha256(bytes),
        MemoryCalldata(bytes) => memory_sha256(bytes),
    }

overload keccak256 = {
    stateless_input_keccak256,
    scratch_keccak256,
    memory_keccak256,
    code_keccak256,
    output_keccak256,
    log_data_keccak256,
    calldata_keccak256
}
overload sha256 = {stateless_input_sha256, scratch_sha256, memory_sha256, calldata_sha256}

/*! KECCAK-256 of one EVM word in canonical big-endian byte order. */
val keccak256_word = impure { c: "host_keccak_word" } : word -> hash

/*! KECCAK-256 of a 20-byte address (secure-trie account keys). */
val keccak256_address = impure { c: "host_keccak_address" } : address -> hash

/*! The SSZ Merkle parent: `SHA-256(left ++ right)` over two 32-byte
    chunks. */
val sha256_pair = impure { c: "host_sha256_pair" } : (hash, hash) -> hash

/*md
## The signature core

The `host/accelerators.sail` k256 axioms, plus the curve-order constants
that signature validity rules bound `r`/`s` against.
*/
/*! `n` of the secp256k1 group order. */
let SECP_N_FULL : word = word_from_bits(0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141)
/*! `n/2` of the secp256k1 group order — the EIP-2 low-`s` malleability
    bound. */
let SECP_N_HALF : word = word_from_bits(0x7fffffffffffffffffffffffffffffff5d576e7357a4501ddfe92f46681b20a0)

/*! Recovers the signer address from `(h, y_parity, r, s)`, returning
    recovery success and the recovered address (used by EIP-7702). */
function ecrecover_addr(h : hash, yparity : y_parity, r : word, s : word) -> AddressResult = {
    host_ecrecover(h, yparity, r, s)
}
