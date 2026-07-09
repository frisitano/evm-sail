/* ======================================================================== */
/* BUILD BACKEND: pointer-backed C bindings for the host interface          */
/*                                                                          */
/* The EVM_BACKEND=build definitions of the abstract vals declared in       */
/* host/io.sail (crypto, oracle, precompiles) and lib/mpt.sail (trie        */
/* node child refs): thin _ffi externs plus wrappers that refine the        */
/* canonical spec.sail models with C that never materializes a Sail byte    */
/* list (hashes and node assembly stream through host byte channels /       */
/* node_asm_*). This is the file compiled into every executable (native     */
/* runner, zkVM guest). Selected when EVM_BACKEND=build; the pure           */
/* reference counterpart is spec.sail.                                      */
/* ======================================================================== */

/* ---- private-input oracle + guest public-output trace (ffi). ---- */

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

/* ---- crypto core (source-free): the C host-bytes hash channel, fixed-width hash
 * helpers, header/tx/auth signing hashes, and the signature precompiles. ---- */

val host_keccak_word_ffi = impure { _: "host_keccak_word" } : word -> hash
val host_keccak_address_ffi = impure { _: "host_keccak_address" } : address -> hash
val host_keccak_create2_ffi = impure { _: "host_keccak_create2" } : (address, word, hash) -> hash
val host_create_address_ffi = impure { _: "host_create_address" } : (address, bits(64)) -> hash
val host_sha256_pair_ffi = impure { _: "host_sha256_pair" } : (hash, hash) -> hash

/* Block-header RLP is framed and hashed C-side. Args are in RLP field order;
 * logs_bloom (256B) and extraData are passed as witness offsets/length, the
 * remaining fields as fixed words / u64 scalars. */
val host_block_header_hash_ffi = impure { _: "host_block_header_hash" } : (
        hash, /* parent_hash */
        address, /* fee_recipient */
        hash, /* state_root */
        hash, /* transactions_root */
        hash, /* receipts_root */
        bits(64), /* logs_bloom witness offset (length 256) */
        bits(64), /* number */
        bits(64), /* gas_limit */
        bits(64), /* gas_used */
        bits(64), /* timestamp */
        bits(64), /* extraData witness offset */
        bits(64), /* extraData length */
        hash, /* prev_randao */
        word, /* base_fee */
        hash, /* withdrawals_root */
        bits(64), /* blob_gas_used */
        bits(64), /* excess_blob_gas */
        hash, /* parent_beacon_block_root */
        hash, /* requests_hash */
        hash, /* block_access_list_hash */
        bits(64), /* slot_number */
    ) -> hash

/* Tx signing hash: frame the pre-signature RLP content (a witness span) with
 * the type-byte domain / list prefix / EIP-155 suffix and hash it C-side.
 * apply_155 selects the (chain_id, 0, 0) legacy suffix. */
val host_tx_signing_hash_ffi = impure { _: "host_tx_signing_hash" } : (
        bits(8), /* tx type (0 = legacy, else typed domain byte) */
        bits(8), /* apply EIP-155 (chain_id, 0, 0) suffix */
        word, /* EIP-155 chain id */
        bits(64), /* content byte-source kind */
        bits(64), /* content offset */
        bits(64), /* content length */
    ) -> hash

/* EIP-7702 auth signing hash: frame 0x05 || rlp([chain_id, address, nonce]). */
val host_auth_signing_hash_ffi = impure { _: "host_auth_signing_hash" } : (word, address, bits(64)) -> hash
val host_sha256_digests3_ffi = impure { _: "host_sha256_digests3" } : (bits(8), hash, hash, hash) -> hash

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

function sha256_digests3(mask : bits(8), d0 : bits(256), d1 : bits(256), d2 : bits(256)) -> bits(256) =
    host_sha256_digests3_ffi(mask, d0, d1, d2)

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    word_to_address(host_keccak_create2_ffi(sender, salt, init_hash))

/* CREATE address: the RLP two-item list [sender, nonce] is framed and hashed in
 * C (nonce is a u64 account nonce). Refines keccak256(rlp([sender, nonce])). */
function create_address(sender : address, nonce : int) -> address =
    word_to_address(host_create_address_ffi(sender, get_slice_int(64, nonce, 0)))

/* Block hash: build re-reads logs_bloom / extraData from the witness (they are
 * decoded from there) and passes the remaining header fields as scalars, so the
 * whole RLP header is framed and hashed C-side with no Sail list. Refines the
 * spec.sail block_header_hash body. */
function block_header_hash(
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) = {
    let extra_off = pl_extra_start();
    let extra_len = pl_txs_start() - extra_off;
    host_block_header_hash_ffi(
        k_header.parent_hash,
        k_header.fee_recipient,
        k_header.state_root,
        transactions_root,
        k_header.receipts_root,
        get_slice_int(64, payload_start() + PL_LOGS_BLOOM, 0),
        get_slice_int(64, k_header.number, 0),
        get_slice_int(64, k_header.gas_limit, 0),
        get_slice_int(64, k_header.gas_used, 0),
        get_slice_int(64, k_header.timestamp, 0),
        get_slice_int(64, extra_off, 0),
        get_slice_int(64, extra_len, 0),
        k_header.prev_randao,
        k_header.base_fee,
        withdrawals_root,
        get_slice_int(64, k_header.blob_gas_used, 0),
        get_slice_int(64, k_header.excess_blob_gas, 0),
        k_header.parent_beacon_block_root,
        requests_hash,
        block_access_list_hash,
        get_slice_int(64, k_header.slot_number, 0),
    )
}

function tx_signing_hash(ttype : int, content_src : ByteSlice, v : int) -> hash = {
    let apply_155 : bool = (ttype == 0) & (v >= 35);
    let chain_id : word = if apply_155 then {
            get_slice_int(256, legacy_sig_chain_id(v), 0)
        } else {
            sail_zeros(256)
        };
    host_tx_signing_hash_ffi(
        get_slice_int(8, ttype, 0),
        if apply_155 then 0x01 else 0x00,
        chain_id,
        byte_source_kind(content_src.source),
        content_src.off,
        content_src.len,
    )
}

function auth_signing_hash(chain_id : int, addr : address, nonce : int) -> hash =
    host_auth_signing_hash_ffi(get_slice_int(256, chain_id, 0), addr, get_slice_int(64, nonce, 0))

function ecrecover_precompile(h : bits(256), yparity : int, r : word, s : word) -> (bool, address) = {
    let recovered = precompile_ecrecover_hash_sig_ffi(h, get_slice_int(64, yparity, 0), r, s);
    (recovered[160] == bitone, recovered[159 .. 0])
}

function secp256k1_verify_precompile(h : bits(256), r : word, s : word, x : bits(256), y : bits(256)) -> bool =
    precompile_secp256k1_verify_hash_sig_pub_ffi(h, r, s, x, y)

/* ---- source-tagged hashing: hash a byte source (Witness/Memory/TxInput/ActiveCode)
 * directly via C. bytes_keccak(s) = keccak256(read_byte_slice(s)); the prefixed
 * digest = sha256(type_byte :: read_byte_slice(s)). ---- */

val host_source_keccak = impure { _: "host_source_keccak" } : (bits(64), bits(64), bits(64)) -> bits(256)
val host_source_sha256_prefixed = impure { _: "host_source_sha256_prefixed" } : (bits(8), bits(64), bits(64), bits(64)) -> bits(
        256,
    )

function bytes_keccak(s : ByteSlice) -> bits(256) =
    host_source_keccak(byte_source_kind(s.source), s.off, s.len)

function bytes_sha256_prefixed(prefix : byte, s : ByteSlice) -> bits(256) =
    host_source_sha256_prefixed(prefix, byte_source_kind(s.source), s.off, s.len)

/* ---- precompiles: run over a pointer-backed input source without a Sail byte list
 * (the C runner snapshots the span and writes output into pending returndata). ---- */

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

/* ---- MPT node encode + hash via the C node-assembly channel (node_asm_*): branch /
 * extension / leaf refs are framed and hashed C-side from fixed-width child
 * refs, so the composed node byte list never crosses the FFI hash channel. ---- */

val node_asm_reset_ffi = impure { _: "node_asm_reset" } : unit -> unit
val node_asm_push_ref_ffi = impure { _: "node_asm_push_ref" } : (bits(64), word, bits(64)) -> unit
val node_asm_push_path_ffi = impure { _: "node_asm_push_path" } : (word, bits(64), bits(64)) -> unit
val node_asm_finish_branch_ffi = impure { _: "node_asm_finish_branch" } : unit -> bits(64)
val node_asm_finish_ffi = impure { _: "node_asm_finish" } : unit -> bits(64)
val node_asm_result_data_ffi = impure { _: "node_asm_result_data" } : unit -> word
val node_asm_result_hash_ffi = impure { _: "node_asm_result_hash" } : unit -> hash

/* push one list(byte) child ref as a fixed-width (kind, data, len) triple:
 * empty (0x80) -> kind 0; a 0xa0 ++ 32-byte hash -> kind 2 (data = the hash);
 * an inline node (< 32 bytes) -> kind 1 (data = its bytes, big-endian). */
function build_push_ref(r : list(byte)) -> unit = {
    let n = list_len(r);
    if (n == 1) & (list_nth(r, 0, 0x00) == 0x80) then {
        node_asm_push_ref_ffi(0x0000000000000000, sail_zeros(256), 0x0000000000000000)
    } else {
        match r {
            0xa0 :: h => if list_len(h) == 32 then {
                node_asm_push_ref_ffi(0x0000000000000002, bytes_be256(h), 0x0000000000000020)
            } else {
                node_asm_push_ref_ffi(0x0000000000000001, bytes_be256(r), get_slice_int(64, n, 0))
            },
            _ => node_asm_push_ref_ffi(0x0000000000000001, bytes_be256(r), get_slice_int(64, n, 0)),
        }
    }
}

/* reconstruct the low `len` bytes (big-endian) of a 256-bit word as a ref */
function build_word_to_ref(data : word, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = out ++ [|sail_shiftright(data, 8 * (len - 1 - k))[7 .. 0]|]
    };
    out
}

/* the finished node's child ref (Eq. 207): inline node bytes when < 32, else
 * 0xa0 ++ keccak(node). `nlen` is the encoded node length from finish. */
function nasm_result_ref(nlen : int) -> list(byte) =
    if nlen < 32 then {
        build_word_to_ref(node_asm_result_data_ffi(), nlen)
    } else {
        hash_child_ref(word_to_bytes32(node_asm_result_hash_ffi()))
    }

/* pack a nibble list big-endian, top nibble first, into a 256-bit word (64
 * nibbles max), returning the word and the nibble count. */
function pack_nibbles(nibs : list(nibble)) -> (word, int) = {
    var acc : word = sail_zeros(256);
    var cnt : int = 0;
    var rest : list(nibble) = nibs;
    var more : bool = true;
    while more do {
        match rest {
            n :: t => {
                acc = or_vec(sail_shiftleft(acc, 4), sail_zero_extend(n, 256));
                cnt = cnt + 1;
                rest = t
            },
            [||] => more = false,
        }
    };
    (acc, cnt)
}

function branch_child_ref(mask : bits(16), children : list(list(byte))) -> list(byte) = {
    node_asm_reset_ffi();
    var ch : list(list(byte)) = children;
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            match ch {
                c :: t => {
                    build_push_ref(c);
                    ch = t
                },
                [||] => (),
            }
        } else {
            node_asm_push_ref_ffi(0x0000000000000000, sail_zeros(256), 0x0000000000000000)
        }
    };
    nasm_result_ref(unsigned(node_asm_finish_branch_ffi()))
}

function extension_child_ref(key : list(nibble), childref : list(byte)) -> list(byte) = {
    node_asm_reset_ffi();
    let (packed, cnt) = pack_nibbles(key);
    node_asm_push_path_ffi(packed, get_slice_int(64, cnt, 0), 0x0000000000000000);
    build_push_ref(childref);
    nasm_result_ref(unsigned(node_asm_finish_ffi()))
}

/* ---- leaf values + the trie value arena ----------------------------------- */
val host_trie_arena_reset_ffi = impure { _: "host_trie_arena_reset" } : unit -> unit
val host_account_rlp_to_arena_ffi = impure { _: "host_account_rlp_to_arena" } : (bits(64), word, hash, hash) -> bits(64)
val host_storage_rlp_to_arena_ffi = impure { _: "host_storage_rlp_to_arena" } : word -> bits(64)
val host_arena_intern_begin_ffi = impure { _: "host_arena_intern_begin" } : unit -> unit
val host_arena_intern_push_ffi = impure { _: "host_arena_intern_push" } : bits(64) -> unit
val host_arena_intern_finish_ffi = impure { _: "host_arena_intern_finish" } : unit -> bits(64)
val node_asm_push_value_ffi = impure { _: "node_asm_push_value_source" } : (bits(64), bits(64), bits(64)) -> unit

/* unpack pack(off,len) = (off << 32) | len into a TrieArenaSource slice */
function arena_span_to_slice(packed : bits(64)) -> ByteSlice =
    byte_slice(TrieArenaSource, unsigned(packed[63 .. 32]), unsigned(packed[31 .. 0]))

/* update leaves: build the RLP in C from the structured fields, no list */
function account_leaf_value(nonce : int, balance : word, sroot : hash, chash : hash) -> ByteSlice =
    arena_span_to_slice(host_account_rlp_to_arena_ffi(get_slice_int(64, nonce, 0), balance, sroot, chash))

function storage_leaf_value(value : word) -> ByteSlice =
    arena_span_to_slice(host_storage_rlp_to_arena_ffi(value))

function trie_arena_reset() -> unit = host_trie_arena_reset_ffi()

/* streamed intern of a computed value list (top-level tx/withdrawals/receipts
 * tries only -- off the hot state-trie path) */
function arena_intern(bs : list(byte)) -> ByteSlice = {
    host_arena_intern_begin_ffi();
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b :: t => {
                host_arena_intern_push_ffi(sail_zero_extend(b, 64));
                rest = t
            },
            [||] => more = false,
        }
    };
    arena_span_to_slice(host_arena_intern_finish_ffi())
}

/* leaf child ref: frame the compact leaf path and the value (read from its byte
 * source: the value arena for updates, the witness for base leaves) in C, then
 * cap as inline node / keccak -- the value byte list never crosses. */
function leaf_child_ref(key : list(nibble), value : ByteSlice) -> list(byte) = {
    node_asm_reset_ffi();
    let (packed, cnt) = pack_nibbles(key);
    node_asm_push_path_ffi(packed, get_slice_int(64, cnt, 0), 0x0000000000000001);
    node_asm_push_value_ffi(byte_source_kind(value.source), value.off, value.len);
    nasm_result_ref(unsigned(node_asm_finish_ffi()))
}
