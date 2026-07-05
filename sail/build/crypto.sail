/* ======================================================================== */
/* BUILD BACKEND: BYTE SOURCES, CRYPTO, AND PRECOMPILES                     */
/*                                                                          */
/* Executable definitions for abstract host byte operations. These use C    */
/* FFI hooks that operate over pointer-backed byte sources without first    */
/* materializing Sail byte lists. The corresponding canonical definitions   */
/* live in sail/spec/crypto.sail; shared consumers in                       */
/* lib/consensus_hash.sail call through the primitives defined here.        */
/* ======================================================================== */

val precompile_run_source_to_returndata_ffi = impure { _: "precompile_run_source_to_returndata" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool

function run_precompile_slice(num : int, s : ByteSlice) -> bool =
    precompile_run_source_to_returndata_ffi(get_slice_int(64, num, 0), byte_source_kind(s.source), s.off, s.len)

function run_precompile_memory_input(num : int, off : int, len : int) -> bool =
    run_precompile_slice(num, memory_byte_slice(off, len))

function run_precompile_tx_input(num : int) -> bool =
    run_precompile_slice(num, tx_input_byte_slice(0, txdata_size()))

/* Optimized read+keccak hook. The reference semantics for each source is
 * keccak256(read_source_bytes(source, off, len)); C backends may avoid
 * materializing that byte list. */
val host_source_keccak = impure { _: "host_source_keccak" } : (bits(64), bits(64), bits(64)) -> bits(256)
val host_source_sha256_prefixed = impure { _: "host_source_sha256_prefixed" } : (bits(8), bits(64), bits(64), bits(64)) -> bits(
        256,
    )

val host_keccak_word_ffi = impure { _: "host_keccak_word" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(256)
val host_keccak_address_ffi = impure { _: "host_keccak_address" } : (bits(64), bits(64), bits(64)) -> bits(256)
val host_keccak_create2_ffi = impure { _: "host_keccak_create2" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(256)
val host_sha256_pair_ffi = impure { _: "host_sha256_pair" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(256)

val code_db_store_source_ffi = impure { _: "code_db_store_source" } : (bits(64), bits(64), bits(64)) -> unit

val host_bytes_reset_ffi = impure { _: "host_bytes_reset" } : unit -> unit
val host_bytes_push_ffi = impure { _: "host_bytes_push" } : bits(8) -> unit
val host_bytes_push8_ffi = impure { _: "host_bytes_push8" } : bits(64) -> unit
val host_bytes_keccak_finish_ffi = impure { _: "host_bytes_keccak_finish" } : unit -> bits(256)
val host_bytes_sha256_finish_ffi = impure { _: "host_bytes_sha256_finish" } : unit -> bits(256)

val precompile_secp256k1_verify_hash_sig_pub_ffi = impure { _: "precompile_secp256k1_verify_hash_sig_pub" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
val precompile_ecrecover_hash_sig_ffi = impure { _: "precompile_ecrecover_hash_sig" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(168)

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

function word_limb3(w : bits(256)) -> bits(64) = w[255 .. 192]
function word_limb2(w : bits(256)) -> bits(64) = w[191 .. 128]
function word_limb1(w : bits(256)) -> bits(64) = w[127 .. 64]
function word_limb0(w : bits(256)) -> bits(64) = w[63 .. 0]

function address_limb2(a : address) -> bits(64) = a[159 .. 96]
function address_limb1(a : address) -> bits(64) = a[95 .. 32]
function address_limb0(a : address) -> bits(64) = sail_zero_extend(a[31 .. 0], 64)

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

function bytes_keccak(s : ByteSlice) -> bits(256) =
    host_source_keccak(byte_source_kind(s.source), s.off, s.len)

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    host_source_sha256_prefixed(prefix, byte_source_kind(s.source), s.off, s.len)

function code_db_store_slice(s : ByteSlice) -> unit =
    code_db_store_source_ffi(byte_source_kind(s.source), s.off, s.len)

function keccak256_word(w : word) -> bits(256) =
    host_keccak_word_ffi(word_limb3(w), word_limb2(w), word_limb1(w), word_limb0(w))

function keccak256_address(a : address) -> bits(256) =
    host_keccak_address_ffi(address_limb2(a), address_limb1(a), address_limb0(a))

function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    host_sha256_pair_ffi(
        word_limb3(a),
        word_limb2(a),
        word_limb1(a),
        word_limb0(a),
        word_limb3(b),
        word_limb2(b),
        word_limb1(b),
        word_limb0(b),
    )

function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    word_to_address(
        host_keccak_create2_ffi(
            address_limb2(sender),
            address_limb1(sender),
            address_limb0(sender),
            word_limb3(salt),
            word_limb2(salt),
            word_limb1(salt),
            word_limb0(salt),
            word_limb3(init_hash),
            word_limb2(init_hash),
            word_limb1(init_hash),
            word_limb0(init_hash),
        ),
    )
}

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool = {
    precompile_secp256k1_verify_hash_sig_pub_ffi(
        word_limb3(h),
        word_limb2(h),
        word_limb1(h),
        word_limb0(h),
        word_limb3(r),
        word_limb2(r),
        word_limb1(r),
        word_limb0(r),
        word_limb3(s),
        word_limb2(s),
        word_limb1(s),
        word_limb0(s),
        word_limb3(x),
        word_limb2(x),
        word_limb1(x),
        word_limb0(x),
        word_limb3(y),
        word_limb2(y),
        word_limb1(y),
        word_limb0(y),
    ) ==
        0x0000000000000001
}

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    let recovered = precompile_ecrecover_hash_sig_ffi(
        word_limb3(h),
        word_limb2(h),
        word_limb1(h),
        word_limb0(h),
        get_slice_int(64, yparity, 0),
        word_limb3(r),
        word_limb2(r),
        word_limb1(r),
        word_limb0(r),
        word_limb3(s),
        word_limb2(s),
        word_limb1(s),
        word_limb0(s),
    );
    (recovered[160] == bitone, recovered[159 .. 0])
}
