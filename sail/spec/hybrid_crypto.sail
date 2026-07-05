/* ======================================================================== */
/* HYBRID BACKEND: BYTE SOURCES, CRYPTO, AND PRECOMPILES                    */
/*                                                                          */
/* The crypto layer for EVM_BACKEND=hybrid, which composes the pure Sail    */
/* reference models (spec/model_state.sail, spec/model_buffers.sail) with   */
/* the build backend's C oracle and crypto primitives to run the models     */
/* executably against real fixtures. The build backend's source-tagged      */
/* hooks (host_source_keccak, code_db_store_source,                         */
/* precompile_run_source_to_returndata) resolve Memory/TxInput/ActiveCode   */
/* sources against the C buffers -- which are EMPTY under hybrid, because   */
/* those buffers are modeled in Sail. So this file keeps only the           */
/* source-free C externs (streamed keccak256/sha256, fixed-width hash and   */
/* signature helpers) and defines every source-tagged operation the spec    */
/* way: materialize the bytes from the MODELS, then hash/store the list.    */
/*                                                                          */
/* Precompile execution has no source-free C entry point (the C runner      */
/* snapshots source bytes and writes pending returndata internally), so     */
/* run_precompile_memory_input / run_precompile_tx_input are stubbed to     */
/* plain failure here: a precompile CALL under hybrid behaves like a failed */
/* call. Fixtures exercised under hybrid must not execute precompiles by    */
/* address (sender authentication is fine: it uses the direct               */
/* secp256k1/ecrecover externs below, which carry no byte source).          */
/* ======================================================================== */

function run_precompile_memory_input(_ : int, _ : int, _ : int) -> bool = false

function run_precompile_tx_input(_ : int) -> bool = false

/* ---- source-free C crypto externs (as in build/crypto.sail) ---- */

val host_keccak_word_ffi = impure { _: "host_keccak_word" } : word -> hash
val host_keccak_address_ffi = impure { _: "host_keccak_address" } : address -> hash
val host_keccak_create2_ffi = impure { _: "host_keccak_create2" } : (address, word, hash) -> hash
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

function keccak_rlp_payload(domain : list(byte), payload : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(domain);
    build_hash_push_bytes(rlp_list_prefix(list_len(payload)));
    build_hash_push_bytes(payload);
    host_bytes_keccak_finish_ffi()
}

/* ---- source-tagged operations, materialized from the MODELS ----
 * (the spec/crypto.sail definitions; mem_read_range / txdata_read_range /
 * code_db_frame_code_byte resolve to the pure models under hybrid) */

function read_active_code_slice(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = code_db_frame_code_byte(get_slice_int(64, off + (len - 1 - k), 0)) :: out
    };
    out
}

function read_byte_slice(s : ByteSlice) -> list(byte) =
    match s.source {
        WitnessSource => ssz_bytes(unsigned(s.off), unsigned(s.len)),
        MemorySource => mem_read_range(unsigned(s.off), unsigned(s.len)),
        TxInputSource => txdata_read_range(unsigned(s.off), unsigned(s.len)),
        ActiveCodeSource => read_active_code_slice(unsigned(s.off), unsigned(s.len)),
    }

function bytes_keccak(s : ByteSlice) -> bits(256) =
    keccak256(read_byte_slice(s))

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    sha256(prefix :: read_byte_slice(s))

function code_db_store_slice(s : ByteSlice) -> unit =
    code_db_store_code(bytes_keccak(s), read_byte_slice(s))

/* ---- fixed-width helpers via the direct (source-free) externs ---- */

function keccak256_word(w : word) -> bits(256) = host_keccak_word_ffi(w)

function keccak256_address(a : address) -> bits(256) = host_keccak_address_ffi(a)

function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) = host_sha256_pair_ffi(a, b)

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    word_to_address(host_keccak_create2_ffi(sender, salt, init_hash))

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool =
    precompile_secp256k1_verify_hash_sig_pub_ffi(h, r, s, x, y)

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    let recovered = precompile_ecrecover_hash_sig_ffi(h, get_slice_int(64, yparity, 0), r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}
