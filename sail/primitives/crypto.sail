/* ======================================================================== */
/* PRIMITIVES: CRYPTO                                                       */
/*                                                                          */
/* The axiomatic hash core -- keccak256 (KECCAK256, trie/RLP node           */
/* references, CREATE2 salt) and sha256 (precompile 0x02 + SSZ              */
/* merkleization) as pure functions of Bytes segment lists -- plus the      */
/* well-known digests of fixed inputs. Loaded before the account types:     */
/* EMPTY_ACCOUNT and the kernel's emptiness checks read the digests long    */
/* before lib/mpt/primitives.sail loads.                                    */
/* ======================================================================== */

/* keccak256 of the empty string: the codeHash of every codeless account */
let KECCAK_EMPTY : hash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* keccak256(rlp("")) -- the root of an empty Merkle-Patricia trie: the storage
 * root of every account with no storage (EMPTY_ACCOUNT, freshly created). */
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

/* ---- THE hash axioms: pure functions of a segment list. A preimage is
 * accumulated Sail-side as list(Bytes) -- its denotation is the concatenation
 * of the segments -- and crosses the boundary in ONE call; the C side
 * (ffi/hash_glue.c, compiled per build against the generated model header)
 * walks the segment list natively, resolves each BytesSlice from its source
 * store, and hashes. No channel state, no per-byte crossings. ---- */
val keccak256_segments = impure { c: "host_keccak_segments" } : list(Bytes) -> bits(256)
val sha256_segments = impure { c: "host_sha256_segments" } : list(Bytes) -> bits(256)

function keccak256_slice(s : ByteSlice) -> bits(256) = keccak256_segments([|BytesSlice(s)|])

/* Materialized fragments always enter through Bytes with an explicit length;
 * source-backed payloads can use the single-slice overload directly. */
overload keccak256 = { keccak256_slice, keccak256_segments }

overload sha256 = { sha256_segments }

/* Fixed-width helpers: pure meaning over keccak256/sha256. The dormant
 * one-call C fast paths in ffi/host_crypto.c (host_keccak_word & co) are
 * future link-time overrides for the hot path (secure-trie keys hash on
 * every state access). */
function keccak256_word(w : word) -> bits(256) =
    keccak256([|bytes_list(word_to_bytes32(w), WORD_BYTE_LENGTH)|])

function keccak256_address(a : address) -> bits(256) =
    keccak256([|bytes_list(address_to_bytes(a), ADDRESS_BYTE_LENGTH)|])

/* SSZ merkle parent: SHA-256(left ++ right) over two 32-byte chunks. */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    sha256([|bytes_list(word_to_bytes32(a), WORD_BYTE_LENGTH), bytes_list(word_to_bytes32(b), WORD_BYTE_LENGTH)|])

/* ---- Signature core: the host/accelerators.sail k256 axioms plus the
 * curve-order constants signature validity rules bound r/s against. ---- */

/* n and n/2 of the secp256k1 group order (EIP-2 low-s malleability bound) */
let SECP_N_FULL : bits(256) = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
let SECP_N_HALF : bits(256) = 0x7fffffffffffffffffffffffffffffff5d576e7357a4501ddfe92f46681b20a0

/* recover the signer address from (h, y_parity, r, s); the leading bit flags
 * recovery success (EIP-7702 authority recovery) */
function ecrecover_addr(h : bits(256), yparity : y_parity, r : word, s : word) -> (bool, address) = {
    let recovered = host_ecrecover(h, yparity, r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}
