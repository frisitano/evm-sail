/* ======================================================================== */
/* BUILD BOUNDARY DEFINITIONS: BYTE SOURCES, CRYPTO, AND PRECOMPILES        */
/*                                                                          */
/* Executable definitions for abstract host byte operations. These use C    */
/* FFI hooks that operate over pointer-backed byte sources without first    */
/* materializing Sail byte lists. The corresponding canonical definitions   */
/* live in sail/spec/crypto.sail.                                           */
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
