/* ======================================================================== */
/* BUILD BACKEND: CRYPTO CORE (source-free)                                 */
/*                                                                          */
/* Executable definitions for hashing and the signing precompiles that      */
/* carry NO byte source: the C host-bytes hash channel, the fixed-width     */
/* hash helpers, and the signature precompiles. Shared by EVM_BACKEND=build */
/* and EVM_BACKEND=hybrid -- both hash via the C channel; only the          */
/* source-tagged bytes_keccak / bytes_sha256_prefixed differ, so those live */
/* in build/crypto_source.sail (build) and hybrid/crypto.sail (hybrid). The */
/* spec backend's canonical byte-list definitions live in spec/crypto.sail; */
/* shared consumers in lib/consensus_hash.sail call these primitives.       */
/* ======================================================================== */

val host_keccak_word_ffi = impure { _: "host_keccak_word" } : word -> hash
val host_keccak_address_ffi = impure { _: "host_keccak_address" } : address -> hash
val host_keccak_create2_ffi = impure { _: "host_keccak_create2" } : (address, word, hash) -> hash
val host_create_address_ffi = impure { _: "host_create_address" } : (address, bits(64)) -> hash
val host_sha256_pair_ffi = impure { _: "host_sha256_pair" } : (hash, hash) -> hash

val host_bytes_reset_ffi = impure { _: "host_bytes_reset" } : unit -> unit
val host_bytes_push_ffi = impure { _: "host_bytes_push" } : bits(8) -> unit
val host_bytes_push8_ffi = impure { _: "host_bytes_push8" } : bits(64) -> unit
val host_bytes_keccak_finish_ffi = impure { _: "host_bytes_keccak_finish" } : unit -> bits(256)
val host_bytes_sha256_finish_ffi = impure { _: "host_bytes_sha256_finish" } : unit -> bits(256)

val precompile_secp256k1_verify_hash_sig_pub_ffi = impure { _: "precompile_secp256k1_verify_hash_sig_pub" } : (
        hash,
        word,
        word,
        word,
        word,
    ) -> bool
val precompile_ecrecover_hash_sig_ffi = impure { _: "precompile_ecrecover_hash_sig" } : (hash, bits(64), word, word) -> bits(
        168,
    )

function build_pack8(
    b0 : byte,
    b1 : byte,
    b2 : byte,
    b3 : byte,
    b4 : byte,
    b5 : byte,
    b6 : byte,
    b7 : byte,
) -> (
    bits(64)
) =
    append(b0, append(b1, append(b2, append(b3, append(b4, append(b5, append(b6, b7)))))))

function build_hash_push_bytes(bs : list(byte)) -> unit = {
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b0 :: (b1 :: (b2 :: (b3 :: (b4 :: (b5 :: (b6 :: (b7 :: t))))))) => {
                host_bytes_push8_ffi(build_pack8(b0, b1, b2, b3, b4, b5, b6, b7));
                rest = t
            },
            b :: t => {
                host_bytes_push_ffi(b);
                rest = t
            },
            [||] => more = false,
        }
    }
}

/* The list-typed hash primitives, streamed through the C byte channel. */
function keccak256(bs : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(bs);
    host_bytes_keccak_finish_ffi()
}

function sha256(bs : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(bs);
    host_bytes_sha256_finish_ffi()
}

/* Refines keccak256(domain ++ rlp_list_prefix(len(payload)) ++ payload)
 * by streaming the three parts without concatenating them. */
function keccak_rlp_payload(domain : list(byte), payload : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(domain);
    build_hash_push_bytes(rlp_list_prefix(list_len(payload)));
    build_hash_push_bytes(payload);
    host_bytes_keccak_finish_ffi()
}

function keccak256_word(w : word) -> bits(256) = host_keccak_word_ffi(w)

function keccak256_address(a : address) -> bits(256) = host_keccak_address_ffi(a)

function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) = host_sha256_pair_ffi(a, b)

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    word_to_address(host_keccak_create2_ffi(sender, salt, init_hash))

/* CREATE address: the RLP two-item list [sender, nonce] is framed and hashed in
 * C (nonce is a u64 account nonce). Refines keccak256(rlp([sender, nonce])). */
function create_address(sender : address, nonce : int) -> address =
    word_to_address(host_create_address_ffi(sender, get_slice_int(64, nonce, 0)))

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool =
    precompile_secp256k1_verify_hash_sig_pub_ffi(h, r, s, x, y)

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    let recovered = precompile_ecrecover_hash_sig_ffi(h, get_slice_int(64, yparity, 0), r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}
