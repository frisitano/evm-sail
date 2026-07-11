/* ======================================================================== */
/* PRIMITIVES: CRYPTO                                                       */
/*                                                                          */
/* The axiomatic hash core -- keccak256 (KECCAK256, trie/RLP node           */
/* references, CREATE2 salt) and sha256 (precompile 0x02 + SSZ              */
/* merkleization) over the C streaming hash channel -- plus the well-known  */
/* digests of fixed inputs. Loaded before the account types: EMPTY_ACCOUNT  */
/* and the kernel's emptiness checks read the digests long before           */
/* lib/mpt.sail loads.                                                      */
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
 * sha256 are the axiomatic hash core; keccak_rlp_payload refines
 * keccak256(domain ++ rlp_list_prefix(len) ++ payload). All shared consensus
 * hashing (lib/consensus_hash.sail) is written against these three. The
 * Witness/Memory/TxInput/ActiveCode source of a ByteSlice denotes an abstract
 * byte sequence: keccak256(s : ByteSlice) = keccak256(read_byte_slice(s)); the
 * prefixed SHA-256 digest = sha256(type_byte :: bytes). ---- */

/* streaming hash channel: reset, push bytes, finish = digest */
val host_bytes_reset = impure { c: "host_bytes_reset" } : unit -> unit
val host_bytes_push = impure { c: "host_bytes_push" } : bits(8) -> unit
val host_bytes_push8 = impure { c: "host_bytes_push8" } : bits(64) -> unit
val host_bytes_keccak_finish = impure { c: "host_bytes_keccak_finish" } : unit -> bits(256)
val host_bytes_sha256_finish = impure { c: "host_bytes_sha256_finish" } : unit -> bits(256)

/* keccak256 / sha256: the byte lists stream through the C hash channel
 * (host_bytes_*) 8 bytes per crossing; the channel primitives are the
 * axiomatic crypto core. */
function hash_push_bytes(bs : list(byte)) -> unit = {
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b0 :: (b1 :: (b2 :: (b3 :: (b4 :: (b5 :: (b6 :: (b7 :: t))))))) => {
                host_bytes_push8(append(b0, append(b1, append(b2, append(b3, append(b4, append(b5, append(b6, b7))))))));
                rest = t
            },
            b :: t => {
                host_bytes_push(b);
                rest = t
            },
            [||] => more = false,
        }
    }
}

function keccak256_bytes(bs : list(byte)) -> bits(256) = {
    host_bytes_reset();
    hash_push_bytes(bs);
    host_bytes_keccak_finish()
}

function sha256(bs : list(byte)) -> bits(256) = {
    host_bytes_reset();
    hash_push_bytes(bs);
    host_bytes_sha256_finish()
}

/* keccak256(domain ++ rlp_list_prefix(len(payload)) ++ payload); body in
 * lib/rlp.sail (it needs rlp_list_prefix), streamed part-by-part. */
val keccak_rlp_payload : (list(byte), list(byte)) -> bits(256)

/* Source-tagged hashing: keccak256_slice(s) = keccak256(read_byte_slice(s));
 * the prefixed digest = sha256(type_byte :: bytes). C hashes the source
 * without materializing it. */
val host_source_keccak = impure { c: "host_source_keccak" } : (bits(64), bits(64), bits(64)) -> bits(256)
val host_source_sha256_prefixed = impure { c: "host_source_sha256_prefixed" } : (bits(8), bits(64), bits(64), bits(64)) -> bits(
        256,
    )

function keccak256_slice(s : ByteSlice) -> bits(256) =
    host_source_keccak(byte_source_kind(s.source), s.off, s.len)

/* keccak256 is overloaded: materialized bytes or a source-tagged ByteSlice
 * both hash through the same axiomatic core. */
overload keccak256 = { keccak256_bytes, keccak256_slice }

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    host_source_sha256_prefixed(prefix, byte_source_kind(s.source), s.off, s.len)

/* Fixed-width helpers: pure meaning over keccak256/sha256. The dormant
 * one-call C fast paths in ffi/host_crypto.c (host_keccak_word & co) are
 * future link-time overrides for the hot path (secure-trie keys hash on
 * every state access). */
function keccak256_word(w : word) -> bits(256) = keccak256(word_to_bytes32(w))

function keccak256_address(a : address) -> bits(256) = keccak256(address_to_bytes(a))

/* SSZ merkle parent: SHA-256(left ++ right) over two 32-byte chunks. */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    sha256(list_append(word_to_bytes32(a), word_to_bytes32(b)))

/* ---- Signature core: secp256k1 verify / recover (k256 accelerator axioms,
 * ffi/precompiles.c) and the curve-order constants signature validity rules
 * bound r/s against. ---- */

/* n and n/2 of the secp256k1 group order (EIP-2 low-s malleability bound) */
let SECP_N_FULL : bits(256) = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
let SECP_N_HALF : bits(256) = 0x7fffffffffffffffffffffffffffffff5d576e7357a4501ddfe92f46681b20a0

/* verify signature (r, s) over hash h against the uncompressed public key
 * (x, y) -- cheaper than recovery, which is why stateless inputs witness the
 * key (EELS recover_sender_from_public_key allows verifying implementations) */
val secp256k1_verify = impure { c: "precompile_secp256k1_verify_hash_sig_pub" } : (
        bits(256),
        word,
        word,
        bits(256),
        bits(256),
    ) -> bool

val precompile_ecrecover_hash_sig = impure { c: "precompile_ecrecover_hash_sig" } : (bits(256), bits(64), word, word) -> bits(
        168,
    )

/* recover the signer address from (h, y_parity, r, s); the leading bit flags
 * recovery success (EIP-7702 authority recovery) */
function ecrecover_addr(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    let recovered = precompile_ecrecover_hash_sig(h, get_slice_int(64, yparity, 0), r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}
