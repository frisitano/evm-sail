/* ======================================================================== */
/* C BINDINGS: crypto accelerators + precompiles (ffi/host_crypto.c,        */
/* ffi/precompiles.c -- the eth-act zkvm-standards accelerator surface)     */
/*                                                                          */
/* The hash/signature primitives have NO pure Sail body (the axiomatic      */
/* crypto core): the host_bytes_* streaming channel, the source-tagged      */
/* hashers, and the precompile runners. The fixed-width helpers             */
/* (keccak256_word/address, sha256_pair, sha256_digests3,                   */
/* secp256k1_verify_precompile) DO have shared pure bodies over keccak256/  */
/* sha256 (host/io.sail); these externs override them with one-call C fast  */
/* paths. Excluding this file leaves the pure bodies in force (over the     */
/* then-axiomatic keccak256/sha256).                                        */
/* ======================================================================== */

/* streaming hash channel: reset, push bytes, finish = digest */
val host_bytes_reset = impure { c: "host_bytes_reset" } : unit -> unit
val host_bytes_push = impure { c: "host_bytes_push" } : bits(8) -> unit
val host_bytes_push8 = impure { c: "host_bytes_push8" } : bits(64) -> unit
val host_bytes_keccak_finish = impure { c: "host_bytes_keccak_finish" } : unit -> bits(256)
val host_bytes_sha256_finish = impure { c: "host_bytes_sha256_finish" } : unit -> bits(256)

/* fixed-width fast paths (override the pure bodies in host/io.sail) */
val keccak256_word = impure { c: "host_keccak_word" } : word -> bits(256)
val keccak256_address = impure { c: "host_keccak_address" } : address -> bits(256)
val sha256_pair = impure { c: "host_sha256_pair" } : (bits(256), bits(256)) -> bits(256)
val sha256_digests3 = impure { c: "host_sha256_digests3" } : (bits(8), bits(256), bits(256), bits(256)) -> bits(
        256,
    )

/* source-tagged hashing: hash a byte source (witness/memory/tx-input/code)
 * directly in C without materializing it */
val host_source_keccak = impure { c: "host_source_keccak" } : (bits(64), bits(64), bits(64)) -> bits(256)
val host_source_sha256_prefixed = impure { c: "host_source_sha256_prefixed" } : (bits(8), bits(64), bits(64), bits(64)) -> bits(
        256,
    )

/* signature precompiles (k256 accelerator) */
val secp256k1_verify_precompile = impure { c: "precompile_secp256k1_verify_hash_sig_pub" } : (
        bits(256),
        word,
        word,
        bits(256),
        bits(256),
    ) -> bool
val precompile_ecrecover_hash_sig = impure { c: "precompile_ecrecover_hash_sig" } : (bits(256), bits(64), word, word) -> bits(
        168,
    )

/* run a precompile over a byte source; successful output lands in pending
 * returndata C-side */
val precompile_run_source_to_returndata = impure { c: "precompile_run_source_to_returndata" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool
