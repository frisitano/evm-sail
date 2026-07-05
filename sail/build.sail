/* ======================================================================== */
/* BUILD BOUNDARY DEFINITIONS                                               */
/*                                                                          */
/* Executable definitions for abstract host byte operations. These use C    */
/* FFI hooks that operate over pointer-backed byte sources without first    */
/* materializing Sail byte lists. The corresponding canonical definitions   */
/* live in spec.sail.                                                       */
/* ======================================================================== */

val ssz_src_len_ffi = impure { _: "ssz_src_len" } : unit -> bits(64)
val ssz_src_byte_ffi = impure { _: "ssz_src_byte" } : int -> bits(8)
val ssz_src_le_ffi = impure { _: "ssz_src_le" } : (int, int) -> bits(64)
val ssz_src_be_ffi = impure { _: "ssz_src_be" } : (int, int) -> bits(64)
val emit_out_ffi = impure { _: "el_emit_out" } : bits(8) -> unit

function ssz_src_len() -> bits(64) = ssz_src_len_ffi()
function ssz_src_byte(i : int) -> bits(8) = ssz_src_byte_ffi(i)
function ssz_src_le(b : int, n : int) -> bits(64) = ssz_src_le_ffi(b, n)
function ssz_src_be(b : int, n : int) -> bits(64) = ssz_src_be_ffi(b, n)
function emit_out(b : bits(8)) -> unit = emit_out_ffi(b)

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

enum BuildPrecompileInputSource = { BuildPcInputMemory, BuildPcInputTxData }

function build_pc_input_byte(src : BuildPrecompileInputSource, base : int, off : int) -> byte =
    match src {
        BuildPcInputMemory => mem_get_byte(base + off),
        BuildPcInputTxData => txdata_byte(off),
    }

function build_pc_be(src : BuildPrecompileInputSource, base : int, off : int, n : int) -> int = {
    var v : int = 0;
    foreach (i from 0 to (n - 1)) {
        v = v * 256 + unsigned(build_pc_input_byte(src, base, off + i))
    };
    v
}

function build_modexp_gas(src : BuildPrecompileInputSource, base : int) -> gas = {
    let bl = build_pc_be(src, base, 0, 32);
    let el = build_pc_be(src, base, 32, 32);
    let ml = build_pc_be(src, base, 64, 32);

    if (k_fork >= FORK_OSAKA) & ((bl > 1024) | (el > 1024) | (ml > 1024)) then {
        return 1000000000000
    };

    let maxlen = if bl > ml then { bl } else { ml };
    let words = ediv_int(maxlen + 7, 8);
    let osaka = k_fork >= FORK_OSAKA;
    let mult = if osaka then {
            if maxlen <= 32 then {
                16
            } else {
                2 * words * words
            }
        } else {
            words * words
        };
    let big_mul = if osaka then { 16 } else { 8 };
    let exp_off = 96 + bl;

    let iter : int = if el <= 32 then {
            let ev = build_pc_be(src, base, exp_off, el);
            if ev == 0 then {
                0
            } else {
                bit_length(ev) - 1
            }
        } else {
            let head = build_pc_be(src, base, exp_off, 32);
            let hb = if bit_length(head) > 0 then {
                    bit_length(head) - 1
                } else {
                    0
                };
            big_mul * (el - 32) + hb
        };
    let it = if iter < 1 then { 1 } else { iter };

    let g = if osaka then {
            mult * it
        } else {
            ediv_int(mult * it, 3)
        };
    to_gas(if osaka then if g < 500 then 500 else g else if g < 200 then 200 else g)
}

function build_precompile_gas_source(num : int, input_len : int, src : BuildPrecompileInputSource, base : int) -> gas = {
    let words = ediv_int(input_len + 31, 32);
    let g : int = match num {
        1 => 3000,
        2 => 60 + 12 * words,
        3 => 600 + 120 * words,
        4 => 15 + 3 * words,
        5 => build_modexp_gas(src, base),
        6 => 150,
        7 => 6000,
        8 => 45000 + 34000 * ediv_int(input_len, 192),
        9 => build_pc_be(src, base, 0, 4),
        10 => 50000,
        11 => 375,
        12 => bls_msm_gas(BLS_G1_DISCOUNT, 12000, 519, ediv_int(input_len, 160)),
        13 => 600,
        14 => bls_msm_gas(BLS_G2_DISCOUNT, 22500, 524, ediv_int(input_len, 288)),
        15 => 37700 + 32600 * ediv_int(input_len, 384),
        16 => 5500,
        17 => 23800,
        256 => 6900,
        _ => 0,
    };
    to_gas(g)
}

function precompile_gas_memory_input(num : int, off : int, len : int) -> gas =
    build_precompile_gas_source(num, len, BuildPcInputMemory, off)

function precompile_gas_tx_input(num : int, len : int) -> gas =
    build_precompile_gas_source(num, len, BuildPcInputTxData, 0)

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
val host_sha256_execution_requests_ffi = impure { _: "host_sha256_execution_requests" } : (
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

function build_bool_word64(b : bool) -> bits(64) =
    if b then { 0x0000000000000001 } else { 0x0000000000000000 }

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

function build_keccak_bytes(bs : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(bs);
    host_bytes_keccak_finish_ffi()
}

function build_sha256_bytes(bs : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(bs);
    host_bytes_sha256_finish_ffi()
}

function build_keccak_rlp_payload(domain : list(byte), payload : list(byte)) -> bits(256) = {
    host_bytes_reset_ffi();
    build_hash_push_bytes(domain);
    build_hash_push_bytes(rlp_list_prefix(list_len(payload)));
    build_hash_push_bytes(payload);
    host_bytes_keccak_finish_ffi()
}

function build_keccak_rlp_list(payload : list(byte)) -> bits(256) =
    build_keccak_rlp_payload([||], payload)

function bytes_keccak(s : ByteSlice) -> bits(256) =
    host_source_keccak(byte_source_kind(s.source), s.off, s.len)

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    host_source_sha256_prefixed(prefix, byte_source_kind(s.source), s.off, s.len)

function code_db_store_slice(s : ByteSlice) -> unit =
    code_db_store_source_ffi(byte_source_kind(s.source), s.off, s.len)

/* Concrete C-backed host mechanisms moved out of common spec files. */
val code_db_frame_jumpdest_valid_ffi = impure { _: "code_db_frame_jumpdest_valid" } : bits(64) -> bool
function code_db_frame_jumpdest_valid(x0 : bits(64)) -> bool = code_db_frame_jumpdest_valid_ffi(x0)

val code_db_frame_code_byte_ffi = impure { _: "code_db_frame_code_byte" } : bits(64) -> bits(8)
function code_db_frame_code_byte(x0 : bits(64)) -> bits(8) = code_db_frame_code_byte_ffi(x0)

val code_db_frame_push_immediate_word_ffi = impure { _: "code_db_frame_push_immediate_word" } : (bits(64), bits(64)) -> word
function code_db_frame_push_immediate_word(x0 : bits(64), x1 : bits(64)) -> word = code_db_frame_push_immediate_word_ffi(
    x0,
    x1,
)

val code_db_frame_set_stored_code_ffi = impure { _: "code_db_frame_set_stored_code" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
function code_db_frame_set_stored_code(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(64) = code_db_frame_set_stored_code_ffi(
    x0,
    x1,
    x2,
    x3,
)

val code_db_prepare_child_frame_memory_code_ffi = impure { _: "code_db_prepare_child_frame_memory_code" } : (
        bits(64),
        bits(64),
    ) -> bits(64)
function code_db_prepare_child_frame_memory_code(x0 : bits(64), x1 : bits(64)) -> bits(64) = code_db_prepare_child_frame_memory_code_ffi(
    x0,
    x1,
)

val code_db_frame_set_tx_input_code_ffi = impure { _: "code_db_frame_set_tx_input_code" } : unit -> bits(64)
function code_db_frame_set_tx_input_code() -> bits(64) = code_db_frame_set_tx_input_code_ffi()

val code_db_frame_clear_code_ffi = impure { _: "code_db_frame_clear_code" } : unit -> unit
function code_db_frame_clear_code() -> unit = code_db_frame_clear_code_ffi()

val code_db_frame_code_length_ffi = impure { _: "code_db_frame_code_length" } : unit -> bits(64)
function code_db_frame_code_length() -> bits(64) = code_db_frame_code_length_ffi()

val hs_reset_ffi = impure { _: "hs_reset" } : unit -> unit
function hs_reset() -> unit = hs_reset_ffi()

val hs_enter_ffi = impure { _: "hs_enter" } : unit -> unit
function hs_enter() -> unit = hs_enter_ffi()

val hs_leave_ffi = impure { _: "hs_leave" } : unit -> unit
function hs_leave() -> unit = hs_leave_ffi()

val hs_depth_ffi = impure { _: "hs_depth" } : unit -> bits(64)
function hs_depth() -> bits(64) = hs_depth_ffi()

val hs_push_w_ffi = impure { _: "hs_push_w" } : word -> unit
function hs_push_w(x0 : word) -> unit = hs_push_w_ffi(x0)

val hs_pop_ffi = impure { _: "hs_pop" } : unit -> unit
function hs_pop() -> unit = hs_pop_ffi()

val hs_peek_w_ffi = impure { _: "hs_peek_w" } : bits(64) -> word
function hs_peek_w(x0 : bits(64)) -> word = hs_peek_w_ffi(x0)

val hs_set_w_ffi = impure { _: "hs_set_w" } : (bits(64), word) -> unit
function hs_set_w(x0 : bits(64), x1 : word) -> unit = hs_set_w_ffi(x0, x1)

val code_db_read_delegation_ffi = impure { _: "code_db_read_delegation" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(
        168,
    )
function code_db_read_delegation(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(168) = code_db_read_delegation_ffi(
    x0,
    x1,
    x2,
    x3,
)

val host_mem_read_ffi = impure { _: "host_mem_read" } : bits(64) -> bits(8)
function host_mem_read(x0 : bits(64)) -> bits(8) = host_mem_read_ffi(x0)

val host_mem_write_ffi = impure { _: "host_mem_write" } : (bits(64), bits(8)) -> unit
function host_mem_write(x0 : bits(64), x1 : bits(8)) -> unit = host_mem_write_ffi(x0, x1)

val host_mem_reset_ffi = impure { _: "host_mem_reset" } : unit -> unit
function host_mem_reset() -> unit = host_mem_reset_ffi()

val host_mem_push_ffi = impure { _: "host_mem_push" } : unit -> unit
function host_mem_push() -> unit = host_mem_push_ffi()

val host_mem_pop_ffi = impure { _: "host_mem_pop" } : unit -> unit
function host_mem_pop() -> unit = host_mem_pop_ffi()

val hm_move_ffi = impure { _: "hm_move" } : (bits(64), bits(64), bits(64)) -> unit
function hm_move(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = hm_move_ffi(x0, x1, x2)

val code_db_copy_frame_code_to_memory_ffi = impure { _: "code_db_copy_frame_code_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function code_db_copy_frame_code_to_memory(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = code_db_copy_frame_code_to_memory_ffi(
    x0,
    x1,
    x2,
)

val returndata_reset_all_ffi = impure { _: "returndata_reset_all" } : unit -> unit
function returndata_reset_all() -> unit = returndata_reset_all_ffi()

val returndata_clear_current_ffi = impure { _: "returndata_clear_current" } : unit -> unit
function returndata_clear_current() -> unit = returndata_clear_current_ffi()

val returndata_discard_pending_ffi = impure { _: "returndata_discard_pending" } : unit -> unit
function returndata_discard_pending() -> unit = returndata_discard_pending_ffi()

val returndata_capture_memory_ffi = impure { _: "returndata_capture_memory" } : (bits(64), bits(64)) -> unit
function returndata_capture_memory(x0 : bits(64), x1 : bits(64)) -> unit = returndata_capture_memory_ffi(x0, x1)

val returndata_adopt_pending_ffi = impure { _: "returndata_adopt_pending" } : unit -> unit
function returndata_adopt_pending() -> unit = returndata_adopt_pending_ffi()

val returndata_current_len_ffi = impure { _: "returndata_current_len" } : unit -> bits(64)
function returndata_current_len() -> bits(64) = returndata_current_len_ffi()

val returndata_pending_len_ffi = impure { _: "returndata_pending_len" } : unit -> bits(64)
function returndata_pending_len() -> bits(64) = returndata_pending_len_ffi()

val returndata_pending_byte_at_ffi = impure { _: "returndata_pending_byte" } : bits(64) -> bits(8)
function returndata_pending_byte_at(x0 : bits(64)) -> bits(8) = returndata_pending_byte_at_ffi(x0)

val returndata_copy_current_to_memory_ffi = impure { _: "returndata_copy_current_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function returndata_copy_current_to_memory(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = returndata_copy_current_to_memory_ffi(
    x0,
    x1,
    x2,
)

val returndata_copy_current_prefix_to_memory_ffi = impure { _: "returndata_copy_current_prefix_to_memory" } : (
        bits(64),
        bits(64),
    ) -> unit
function returndata_copy_current_prefix_to_memory(x0 : bits(64), x1 : bits(64)) -> unit = returndata_copy_current_prefix_to_memory_ffi(
    x0,
    x1,
)

val cd_set_ffi = impure { _: "cd_set" } : (bits(64), bits(64)) -> unit
function cd_set(x0 : bits(64), x1 : bits(64)) -> unit = cd_set_ffi(x0, x1)

val cd_set_empty_ffi = impure { _: "cd_set_empty" } : unit -> unit
function cd_set_empty() -> unit = cd_set_empty_ffi()

val cd_set_tx_ffi = impure { _: "cd_set_tx" } : unit -> unit
function cd_set_tx() -> unit = cd_set_tx_ffi()

val txin_begin_ffi = impure { _: "txin_begin" } : unit -> unit
function txin_begin() -> unit = txin_begin_ffi()

val txin_byte_ffi = impure { _: "txin_byte" } : bits(8) -> unit
function txin_byte(x0 : bits(8)) -> unit = txin_byte_ffi(x0)

val code_db_calldata_load_word_ffi = impure { _: "code_db_calldata_load_word" } : bits(64) -> word
function code_db_calldata_load_word(x0 : bits(64)) -> word = code_db_calldata_load_word_ffi(x0)

val cd_to_mem_ffi = impure { _: "cd_to_mem" } : (bits(64), bits(64), bits(64)) -> unit
function cd_to_mem(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = cd_to_mem_ffi(x0, x1, x2)

val txd_at_ffi = impure { _: "txd_at" } : bits(64) -> bits(8)
function txd_at(x0 : bits(64)) -> bits(8) = txd_at_ffi(x0)

val txd_length_ffi = impure { _: "txd_length" } : unit -> bits(64)
function txd_length() -> bits(64) = txd_length_ffi()

val transient_storage_reset_ffi = impure { _: "transient_storage_reset" } : bits(64) -> unit
function transient_storage_reset(x0 : bits(64)) -> unit = transient_storage_reset_ffi(x0)

val transient_storage_key_ffi = impure { _: "transient_storage_key" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function transient_storage_key(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
) -> (
    unit
) =
    transient_storage_key_ffi(x0, x1, x2, x3, x4, x5, x6, x7)

val transient_storage_write_ffi = impure { _: "transient_storage_write" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function transient_storage_write(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = transient_storage_write_ffi(
    x0,
    x1,
    x2,
    x3,
)

val transient_storage_read_word_ffi = impure { _: "transient_storage_word" } : bits(64) -> bits(64)
function transient_storage_read_word(x0 : bits(64)) -> bits(64) = transient_storage_read_word_ffi(x0)

val storage_map_reset_ffi = impure { _: "storage_map_reset" } : unit -> unit
function storage_map_reset() -> unit = storage_map_reset_ffi()

val storage_map_push_ffi = impure { _: "storage_map_push" } : unit -> unit
function storage_map_push() -> unit = storage_map_push_ffi()

val storage_map_pop_commit_ffi = impure { _: "storage_map_pop_commit" } : unit -> unit
function storage_map_pop_commit() -> unit = storage_map_pop_commit_ffi()

val storage_map_pop_discard_ffi = impure { _: "storage_map_pop_discard" } : unit -> unit
function storage_map_pop_discard() -> unit = storage_map_pop_discard_ffi()

val storage_map_key_ffi = impure { _: "storage_map_key" } : (
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
    ) -> unit
function storage_map_key(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
) -> (
    unit
) =
    storage_map_key_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)

val storage_map_seed_ffi = impure { _: "storage_map_seed" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function storage_map_seed(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = storage_map_seed_ffi(
    x0,
    x1,
    x2,
    x3,
)

val storage_map_store_ffi = impure { _: "storage_map_store" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function storage_map_store(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = storage_map_store_ffi(
    x0,
    x1,
    x2,
    x3,
)

val storage_map_word_ffi = impure { _: "storage_map_word" } : bits(64) -> bits(64)
function storage_map_word(x0 : bits(64)) -> bits(64) = storage_map_word_ffi(x0)

val storage_map_base_word_ffi = impure { _: "storage_map_base_word" } : bits(64) -> bits(64)
function storage_map_base_word(x0 : bits(64)) -> bits(64) = storage_map_base_word_ffi(x0)

val storage_map_present_ffi = impure { _: "storage_map_present" } : unit -> bool
function storage_map_present() -> bool = storage_map_present_ffi()

val storage_map_base_present_ffi = impure { _: "storage_map_base_present" } : unit -> bool
function storage_map_base_present() -> bool = storage_map_base_present_ffi()

val storage_map_wipe_acct_hash_ffi = impure { _: "storage_map_wipe_acct_hash" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function storage_map_wipe_acct_hash(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = storage_map_wipe_acct_hash_ffi(
    x0,
    x1,
    x2,
    x3,
)

val storage_map_count_ffi = impure { _: "storage_map_count" } : unit -> bits(64)
function storage_map_count() -> bits(64) = storage_map_count_ffi()

val storage_map_update_count_ffi = impure { _: "storage_map_update_count" } : unit -> bits(64)
function storage_map_update_count() -> bits(64) = storage_map_update_count_ffi()

val storage_map_at_ffi = impure { _: "storage_map_at" } : bits(64) -> unit
function storage_map_at(x0 : bits(64)) -> unit = storage_map_at_ffi(x0)

val storage_map_update_at_ffi = impure { _: "storage_map_update_at" } : bits(64) -> unit
function storage_map_update_at(x0 : bits(64)) -> unit = storage_map_update_at_ffi(x0)

val storage_map_it_acct_hash_ffi = impure { _: "storage_map_it_acct_hash" } : bits(64) -> bits(64)
function storage_map_it_acct_hash(x0 : bits(64)) -> bits(64) = storage_map_it_acct_hash_ffi(x0)

val storage_map_it_slot_ffi = impure { _: "storage_map_it_slot" } : bits(64) -> bits(64)
function storage_map_it_slot(x0 : bits(64)) -> bits(64) = storage_map_it_slot_ffi(x0)

val storage_map_it_val_ffi = impure { _: "storage_map_it_val" } : bits(64) -> bits(64)
function storage_map_it_val(x0 : bits(64)) -> bits(64) = storage_map_it_val_ffi(x0)

val acctmap_reset_ffi = impure { _: "acctmap_reset" } : unit -> unit
function acctmap_reset() -> unit = acctmap_reset_ffi()

val acctmap_key_ffi = impure { _: "acctmap_key" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function acctmap_key(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = acctmap_key_ffi(
    x0,
    x1,
    x2,
    x3,
)

val acctmap_present_ffi = impure { _: "acctmap_present" } : unit -> bool
function acctmap_present() -> bool = acctmap_present_ffi()

val acctmap_store_ffi = impure { _: "acctmap_store" } : (
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
    ) -> unit
function acctmap_store(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
    x12 : bits(64),
    x13 : bits(64),
    x14 : bits(64),
    x15 : bits(64),
    x16 : bits(64),
) -> (
    unit
) =
    acctmap_store_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16)

val acctmap_mark_base_exists_ffi = impure { _: "acctmap_mark_base_exists" } : unit -> unit
function acctmap_mark_base_exists() -> unit = acctmap_mark_base_exists_ffi()

val acctmap_seed_ffi = impure { _: "acctmap_seed" } : (
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
    ) -> unit
function acctmap_seed(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
    x12 : bits(64),
    x13 : bits(64),
    x14 : bits(64),
    x15 : bits(64),
    x16 : bits(64),
) -> (
    unit
) =
    acctmap_seed_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16)

val acctmap_nonce_ffi = impure { _: "acctmap_nonce" } : unit -> bits(64)
function acctmap_nonce() -> bits(64) = acctmap_nonce_ffi()

val acctmap_bal_ffi = impure { _: "acctmap_bal" } : bits(64) -> bits(64)
function acctmap_bal(x0 : bits(64)) -> bits(64) = acctmap_bal_ffi(x0)

val acctmap_sroot_ffi = impure { _: "acctmap_sroot" } : bits(64) -> bits(64)
function acctmap_sroot(x0 : bits(64)) -> bits(64) = acctmap_sroot_ffi(x0)

val acctmap_chash_ffi = impure { _: "acctmap_chash" } : bits(64) -> bits(64)
function acctmap_chash(x0 : bits(64)) -> bits(64) = acctmap_chash_ffi(x0)

val acctmap_remove_ffi = impure { _: "acctmap_remove" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function acctmap_remove(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = acctmap_remove_ffi(
    x0,
    x1,
    x2,
    x3,
)

val acctmap_count_ffi = impure { _: "acctmap_count" } : unit -> bits(64)
function acctmap_count() -> bits(64) = acctmap_count_ffi()

val acctmap_update_count_ffi = impure { _: "acctmap_update_count" } : unit -> bits(64)
function acctmap_update_count() -> bits(64) = acctmap_update_count_ffi()

val acctmap_at_ffi = impure { _: "acctmap_at" } : bits(64) -> unit
function acctmap_at(x0 : bits(64)) -> unit = acctmap_at_ffi(x0)

val acctmap_update_at_ffi = impure { _: "acctmap_update_at" } : bits(64) -> unit
function acctmap_update_at(x0 : bits(64)) -> unit = acctmap_update_at_ffi(x0)

val acctmap_at_hkey_ffi = impure { _: "acctmap_at_hkey" } : bits(64) -> bits(64)
function acctmap_at_hkey(x0 : bits(64)) -> bits(64) = acctmap_at_hkey_ffi(x0)

val acctmap_at_nonce_ffi = impure { _: "acctmap_at_nonce" } : unit -> bits(64)
function acctmap_at_nonce() -> bits(64) = acctmap_at_nonce_ffi()

val acctmap_at_bal_ffi = impure { _: "acctmap_at_bal" } : bits(64) -> bits(64)
function acctmap_at_bal(x0 : bits(64)) -> bits(64) = acctmap_at_bal_ffi(x0)

val acctmap_at_sroot_ffi = impure { _: "acctmap_at_sroot" } : bits(64) -> bits(64)
function acctmap_at_sroot(x0 : bits(64)) -> bits(64) = acctmap_at_sroot_ffi(x0)

val acctmap_at_chash_ffi = impure { _: "acctmap_at_chash" } : bits(64) -> bits(64)
function acctmap_at_chash(x0 : bits(64)) -> bits(64) = acctmap_at_chash_ffi(x0)

val acctmap_at_base_exists_ffi = impure { _: "acctmap_at_base_exists" } : unit -> bool
function acctmap_at_base_exists() -> bool = acctmap_at_base_exists_ffi()

val code_db_stream_code_begin_ffi = impure { _: "code_db_stream_code_begin" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(
        64,
    )
function code_db_stream_code_begin(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(64) = code_db_stream_code_begin_ffi(
    x0,
    x1,
    x2,
    x3,
)

val code_db_stored_code_length_ffi = impure { _: "code_db_stored_code_length" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
function code_db_stored_code_length(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(64) = code_db_stored_code_length_ffi(
    x0,
    x1,
    x2,
    x3,
)

val code_db_copy_stored_code_to_memory_ffi = impure { _: "code_db_copy_stored_code_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function code_db_copy_stored_code_to_memory(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
) -> (
    unit
) =
    code_db_copy_stored_code_to_memory_ffi(x0, x1, x2, x3, x4, x5, x6)

val code_db_stream_code_byte_ffi = impure { _: "code_db_stream_code_byte" } : bits(8) -> unit
function code_db_stream_code_byte(x0 : bits(8)) -> unit = code_db_stream_code_byte_ffi(x0)

val slotdb_reset_ffi = impure { _: "slotdb_reset" } : unit -> unit
function slotdb_reset() -> unit = slotdb_reset_ffi()

val slotdb_insert_ffi = impure { _: "slotdb_insert" } : (
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
    ) -> unit
function slotdb_insert(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
) -> (
    unit
) =
    slotdb_insert_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)

val slotdb_sel_ffi = impure { _: "slotdb_sel" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool
function slotdb_sel(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
) -> (
    bool
) =
    slotdb_sel_ffi(x0, x1, x2, x3, x4, x5, x6, x7)

val slotdb_selval_ffi = impure { _: "slotdb_selval" } : bits(64) -> bits(64)
function slotdb_selval(x0 : bits(64)) -> bits(64) = slotdb_selval_ffi(x0)

val slotdb_sel_existed_ffi = impure { _: "slotdb_sel_existed" } : unit -> bool
function slotdb_sel_existed() -> bool = slotdb_sel_existed_ffi()

val slotdb_count_ffi = impure { _: "slotdb_count" } : unit -> bits(64)
function slotdb_count() -> bits(64) = slotdb_count_ffi()

val slotdb_at_ffi = impure { _: "slotdb_at" } : bits(64) -> unit
function slotdb_at(x0 : bits(64)) -> unit = slotdb_at_ffi(x0)

val slotdb_at_acct_ffi = impure { _: "slotdb_at_acct" } : bits(64) -> bits(64)
function slotdb_at_acct(x0 : bits(64)) -> bits(64) = slotdb_at_acct_ffi(x0)

val slotdb_at_slot_ffi = impure { _: "slotdb_at_slot" } : bits(64) -> bits(64)
function slotdb_at_slot(x0 : bits(64)) -> bits(64) = slotdb_at_slot_ffi(x0)

val slotdb_at_val_ffi = impure { _: "slotdb_at_val" } : bits(64) -> bits(64)
function slotdb_at_val(x0 : bits(64)) -> bits(64) = slotdb_at_val_ffi(x0)

val slotdb_at_existed_ffi = impure { _: "slotdb_at_existed" } : unit -> bool
function slotdb_at_existed() -> bool = slotdb_at_existed_ffi()

val storage_mark_incomplete_ffi = impure { _: "storage_mark_incomplete" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function storage_mark_incomplete(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = storage_mark_incomplete_ffi(
    x0,
    x1,
    x2,
    x3,
)

val storage_harvest_complete_raw_ffi = impure { _: "storage_harvest_complete" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool
function storage_harvest_complete_raw(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bool = storage_harvest_complete_raw_ffi(
    x0,
    x1,
    x2,
    x3,
)

val nodedb_reset_ffi = impure { _: "nodedb_reset" } : unit -> unit
function nodedb_reset() -> unit = nodedb_reset_ffi()

val nodedb_insert_ffi = impure { _: "nodedb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
function nodedb_insert(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
) -> (
    unit
) =
    nodedb_insert_ffi(x0, x1, x2, x3, x4, x5)

val nodedb_sel_ffi = impure { _: "nodedb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bool
function nodedb_sel(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bool = nodedb_sel_ffi(x0, x1, x2, x3)

val nodedb_sel_off_ffi = impure { _: "nodedb_sel_off" } : unit -> bits(64)
function nodedb_sel_off() -> bits(64) = nodedb_sel_off_ffi()

val nodedb_sel_len_ffi = impure { _: "nodedb_sel_len" } : unit -> bits(64)
function nodedb_sel_len() -> bits(64) = nodedb_sel_len_ffi()

val acctdb_reset_ffi = impure { _: "acctdb_reset" } : unit -> unit
function acctdb_reset() -> unit = acctdb_reset_ffi()

val acctdb_insert_ffi = impure { _: "acctdb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
function acctdb_insert(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
) -> (
    unit
) =
    acctdb_insert_ffi(x0, x1, x2, x3, x4, x5)

val acctdb_sel_ffi = impure { _: "acctdb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bool
function acctdb_sel(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bool = acctdb_sel_ffi(x0, x1, x2, x3)

val acctdb_sel_off_ffi = impure { _: "acctdb_sel_off" } : unit -> bits(64)
function acctdb_sel_off() -> bits(64) = acctdb_sel_off_ffi()

val acctdb_sel_len_ffi = impure { _: "acctdb_sel_len" } : unit -> bits(64)
function acctdb_sel_len() -> bits(64) = acctdb_sel_len_ffi()

val acctdb_count_ffi = impure { _: "acctdb_count" } : unit -> bits(64)
function acctdb_count() -> bits(64) = acctdb_count_ffi()

val acctdb_at_ffi = impure { _: "acctdb_at" } : bits(64) -> unit
function acctdb_at(x0 : bits(64)) -> unit = acctdb_at_ffi(x0)

val acctdb_at_key_ffi = impure { _: "acctdb_at_key" } : bits(64) -> bits(64)
function acctdb_at_key(x0 : bits(64)) -> bits(64) = acctdb_at_key_ffi(x0)

val acctdb_at_off_ffi = impure { _: "acctdb_at_off" } : unit -> bits(64)
function acctdb_at_off() -> bits(64) = acctdb_at_off_ffi()

val acctdb_at_len_ffi = impure { _: "acctdb_at_len" } : unit -> bits(64)
function acctdb_at_len() -> bits(64) = acctdb_at_len_ffi()

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

function sha256_selfcheck() -> unit = {
    assert(
        build_sha256_bytes([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855,
        "sha256('') on guest"
    );
    assert(
        build_sha256_bytes([|0x61, 0x62, 0x63|]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
        "sha256('abc') on guest"
    )
}

function keccak_selfcheck() -> unit = {
    assert(
        build_keccak_bytes([||]) == 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470,
        "keccak('') on guest"
    );
    assert(
        build_keccak_bytes([|0x61, 0x62, 0x63|]) == 0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45,
        "keccak('abc') on guest"
    )
}

function create_address(sender : address, nonce : int) -> address = {
    var payload : list(byte) = rlp_addr(sender);
    payload = list_append(payload, rlp_int(nonce));
    word_to_address(build_keccak_rlp_list(payload))
}

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

function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(build_keccak_bytes(enc)))
    }

function trie_ref_to_root(r : list(byte)) -> bits(256) =
    match r {
        0xa0 :: h => if list_len(h) == 32 then {
            bytes_to_word_be256(h)
        } else {
            build_keccak_bytes(r)
        },
        _ => build_keccak_bytes(r),
    }

function mpt_node_hash(node : list(byte)) -> hash =
    build_keccak_bytes(node)

function code_hash_and_store(c : list(byte)) -> hash = {
    let h : hash = build_keccak_bytes(c);
    code_db_store_code(h, c);
    h
}

function code_db_store_witness_code(s : witness_slice) -> unit =
    code_db_store_slice(witness_byte_slice(s))

function tx_signing_hash(ttype : int, fields : list(RlpField), i_v : int, v : int) -> bits(256) = {
    var content : list(byte) = rlp_fields_encoded_prefix(fields, i_v);
    if (ttype == 0) & (v >= 35) then {
        content = list_append(content, rlp_int(legacy_sig_chain_id(v)));
        content = list_append(content, [|0x80, 0x80|])
    };
    let domain : list(byte) = if ttype != 0 then {
            [|get_slice_int(8, ttype, 0)|]
        } else {
            [||]
        };
    build_keccak_rlp_payload(domain, content)
}

function auth_signing_hash(fields : list(RlpField)) -> bits(256) = {
    let content : list(byte) = rlp_fields_encoded_prefix(fields, 3);
    build_keccak_rlp_payload([|0x05|], content)
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

function bloom_entry_bit(entry : list(byte), i : int) -> int = {
    let h = word_to_bytes32(build_keccak_bytes(entry));
    let j = i * 2;
    emod_int(unsigned(bytes_get(h, j)) * 256 + unsigned(bytes_get(h, j + 1)), 2048)
}

function build_request_digest_or_zero(request_type : int, body_start : int, body_len : int) -> hash = {
    if body_len == 0 then {
        sail_zeros(256)
    } else {
        sha256_request_digest(request_type, witness_slice(body_start, body_len))
    }
}

function execution_requests_hash() -> hash = {
    let len0 = wreq_start() - dep_start();
    let len1 = creq_start() - wreq_start();
    let len2 = req_end() - creq_start();
    let d0 = build_request_digest_or_zero(0, dep_start(), len0);
    let d1 = build_request_digest_or_zero(1, wreq_start(), len1);
    let d2 = build_request_digest_or_zero(2, creq_start(), len2);
    host_sha256_execution_requests_ffi(
        build_bool_word64(len0 != 0),
        word_limb3(d0),
        word_limb2(d0),
        word_limb1(d0),
        word_limb0(d0),
        build_bool_word64(len1 != 0),
        word_limb3(d1),
        word_limb2(d1),
        word_limb1(d1),
        word_limb0(d1),
        build_bool_word64(len2 != 0),
        word_limb3(d2),
        word_limb2(d2),
        word_limb1(d2),
        word_limb0(d2),
    )
}

function block_header_hash(
    header : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) = {
    var payload : list(byte) = rlp_word(header.parent_hash);
    payload = list_append(payload, rlp_word(EMPTY_OMMER_HASH));
    payload = list_append(payload, rlp_addr(header.fee_recipient));
    payload = list_append(payload, rlp_word(header.state_root));
    payload = list_append(payload, rlp_word(transactions_root));
    payload = list_append(payload, rlp_word(header.receipts_root));
    payload = list_append(payload, rlp_bytes(header.logs_bloom));
    payload = list_append(payload, rlp_int(0));
    payload = list_append(payload, rlp_int(header.number));
    payload = list_append(payload, rlp_int(header.gas_limit));
    payload = list_append(payload, rlp_int(header.gas_used));
    payload = list_append(payload, rlp_int(header.timestamp));
    payload = list_append(payload, rlp_bytes(ssz_bytes(pl_extra_start(), pl_txs_start() - pl_extra_start())));
    payload = list_append(payload, rlp_word(header.prev_randao));
    payload = list_append(payload, rlp_bytes(zero_bytes(8)));
    payload = list_append(payload, rlp_int_word(header.base_fee));
    payload = list_append(payload, rlp_word(withdrawals_root));
    payload = list_append(payload, rlp_int(header.blob_gas_used));
    payload = list_append(payload, rlp_int(header.excess_blob_gas));
    payload = list_append(payload, rlp_word(header.parent_beacon_block_root));
    payload = list_append(payload, rlp_word(requests_hash));
    payload = list_append(payload, rlp_word(block_access_list_hash));
    payload = list_append(payload, rlp_int(header.slot_number));
    build_keccak_rlp_list(payload)
}
