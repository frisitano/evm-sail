/* ======================================================================== */
/* SPEC BOUNDARY DEFINITIONS                                                */
/*                                                                          */
/* Canonical definitions for abstract host byte operations. These           */
/* definitions materialize mathematical byte lists before hashing or        */
/* storing bytes. Build roots may replace the same abstract functions with  */
/* pointer-backed implementations in build.sail.                            */
/* ======================================================================== */

val keccak256 : list(byte) -> bits(256)
val sha256 : list(byte) -> bits(256)
val run_precompile : (int, list(byte)) -> (bool, list(byte))

function ssz_src_le(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(ssz_src_byte(b + k), 64), 8 * k))
    };
    acc
}

function ssz_src_be(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(ssz_src_byte(b + k), 64))
    };
    acc
}

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

function keccak256_word(w : word) -> bits(256) =
    keccak256(word_to_bytes32(w))

function keccak256_address(a : address) -> bits(256) =
    keccak256(address_to_bytes(a))

function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    sha256(list_append(word_to_bytes32(a), word_to_bytes32(b)))

function sha256_selfcheck() -> unit = {
    assert(sha256([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855, "sha256('') on guest");
    assert(
        sha256([|0x61, 0x62, 0x63|]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
        "sha256('abc') on guest"
    )
}

function keccak_selfcheck() -> unit = {
    assert(keccak256([||]) == 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470, "keccak('') on guest");
    assert(
        keccak256([|0x61, 0x62, 0x63|]) == 0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45,
        "keccak('abc') on guest"
    )
}

function create_address(sender : address, nonce : int) -> address = {
    var payload : list(byte) = rlp_addr(sender);
    payload = list_append(payload, rlp_int(nonce));
    word_to_address(keccak256(rlp_list(payload)))
}

function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    var preimage : list(byte) = [|0xff|];
    preimage = list_append(preimage, address_to_bytes(sender));
    preimage = list_append(preimage, word_to_bytes32(salt));
    preimage = list_append(preimage, word_to_bytes32(init_hash));
    word_to_address(keccak256(preimage))
}

function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(keccak256(enc)))
    }

function trie_ref_to_root(r : list(byte)) -> bits(256) =
    match r {
        0xa0 :: h => if list_len(h) == 32 then {
            bytes_to_word_be256(h)
        } else {
            keccak256(r)
        },
        _ => keccak256(r),
    }

function mpt_node_hash(node : list(byte)) -> hash =
    keccak256(node)

function code_hash_and_store(c : list(byte)) -> hash = {
    let h : hash = keccak256(c);
    code_db_store_code(h, c);
    h
}

function precompile_gas_memory_input(num : int, off : int, len : int) -> gas =
    precompile_gas(num, mem_read_range(off, len))

function precompile_gas_tx_input(num : int, len : int) -> gas =
    precompile_gas(num, txdata_read_range(0, len))

function code_db_store_witness_code(s : witness_slice) -> unit =
    code_db_store_slice(witness_byte_slice(s))

function tx_signing_hash(ttype : int, fields : list(RlpField), i_v : int, v : int) -> bits(256) = {
    var content : list(byte) = rlp_fields_encoded_prefix(fields, i_v);
    if (ttype == 0) & (v >= 35) then {
        content = list_append(content, rlp_int(legacy_sig_chain_id(v)));
        content = list_append(content, [|0x80, 0x80|])
    };
    var pre : list(byte) = if ttype != 0 then {
            [|get_slice_int(8, ttype, 0)|]
        } else {
            [||]
        };
    pre = list_append(pre, rlp_list_prefix(list_len(content)));
    pre = list_append(pre, content);
    keccak256(pre)
}

function auth_signing_hash(fields : list(RlpField)) -> bits(256) = {
    let content : list(byte) = rlp_fields_encoded_prefix(fields, 3);
    var pre : list(byte) = [|0x05|];
    pre = list_append(pre, rlp_list_prefix(list_len(content)));
    pre = list_append(pre, content);
    keccak256(pre)
}

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool = {
    var inp : list(byte) = word_to_bytes32(h);
    inp = list_append(inp, word_to_bytes32(r));
    inp = list_append(inp, word_to_bytes32(s));
    inp = list_append(inp, word_to_bytes32(x));
    inp = list_append(inp, word_to_bytes32(y));
    let (_, out) = run_precompile(257, inp);
    list_len(out) == 32
}

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    var inp : list(byte) = word_to_bytes32(h);
    inp = list_append(inp, word_to_bytes32(word_of_int(27 + yparity)));
    inp = list_append(inp, word_to_bytes32(r));
    inp = list_append(inp, word_to_bytes32(s));
    let (_, out) = run_precompile(1, inp);
    if list_len(out) == 32 then {
        (true, word_to_address(bytes_be256(out)))
    } else {
        (false, ZERO_ADDR)
    }
}

function bloom_entry_bit(entry : list(byte), i : int) -> int = {
    let h = word_to_bytes32(keccak256(entry));
    let j = i * 2;
    emod_int(unsigned(bytes_get(h, j)) * 256 + unsigned(bytes_get(h, j + 1)), 2048)
}

function request_digest_bytes(request_type : int, body_start : int, body_len : int) -> list(byte) = {
    if body_len == 0 then {
        [||]
    } else {
        word_to_bytes32(sha256_request_digest(request_type, witness_slice(body_start, body_len)))
    }
}

function execution_requests_hash() -> hash = {
    var payload : list(byte) = [||];
    payload = list_append(payload, request_digest_bytes(0, dep_start(), wreq_start() - dep_start()));
    payload = list_append(payload, request_digest_bytes(1, wreq_start(), creq_start() - wreq_start()));
    payload = list_append(payload, request_digest_bytes(2, creq_start(), req_end() - creq_start()));
    sha256(payload)
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
    keccak256(rlp_list(payload))
}
