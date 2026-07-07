/* Build backend: MPT node encode + hash via the C node-assembly channel
 * (ffi/host_crypto.c node_asm_*). Refines lib/mpt.sail's branch_child_ref: the
 * 17-item branch node is framed and hashed C-side from fixed-width child refs,
 * so the composed node byte list never crosses the FFI hash channel. */

val node_asm_reset_ffi = impure { _: "node_asm_reset" } : unit -> unit
val node_asm_push_ref_ffi = impure { _: "node_asm_push_ref" } : (bits(64), word, bits(64)) -> unit
val node_asm_finish_branch_ffi = impure { _: "node_asm_finish_branch" } : unit -> bits(64)
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
    let nlen = unsigned(node_asm_finish_branch_ffi());
    if nlen < 32 then {
        build_word_to_ref(node_asm_result_data_ffi(), nlen)
    } else {
        hash_child_ref(word_to_bytes32(node_asm_result_hash_ffi()))
    }
}
