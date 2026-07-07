/* Build backend: MPT node encode + hash via the C node-assembly channel
 * (ffi/host_crypto.c node_asm_*). Refines lib/mpt.sail's branch_child_ref: the
 * 17-item branch node is framed and hashed C-side from fixed-width child refs,
 * so the composed node byte list never crosses the FFI hash channel. */

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

function trie_arena_reset() -> unit = host_trie_arena_reset_ffi()

/* unpack pack(off,len) = (off << 32) | len into a TrieArenaSource slice */
function arena_span_to_slice(packed : bits(64)) -> ByteSlice =
    byte_slice(TrieArenaSource, unsigned(packed[63 .. 32]), unsigned(packed[31 .. 0]))

/* update leaves: build the RLP in C from the structured fields, no list */
function account_leaf_value(nonce : int, balance : word, sroot : hash, chash : hash) -> ByteSlice =
    arena_span_to_slice(host_account_rlp_to_arena_ffi(get_slice_int(64, nonce, 0), balance, sroot, chash))

function storage_leaf_value(value : word) -> ByteSlice =
    arena_span_to_slice(host_storage_rlp_to_arena_ffi(value))

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
