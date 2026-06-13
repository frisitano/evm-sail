/* ======================================================================== */
/* Keccak-256 (Ethereum variant) via the zkvm-standards accelerator boundary  */
/*                                                                           */
/* keccak256 routes through the host crypto boundary (host/crypto.sail  */
/* -> ffi/zkvm_accelerators.h zkvm_keccak256, id 0). This IS the eth-act       */
/* c-interface-accelerators boundary: native links the reference impl, a real  */
/* zkVM swaps it for a native keccak precompile behind the same standard       */
/* header. The pure-Sail reference permutation is preserved in git history.    */
/* Validate against keccak256("") = c5d2460186f7233c927e7db2dcc703c0e500...   */
/* ======================================================================== */

/* Keccak-256 via the zkvm-standards accelerator boundary (host/crypto.sail ->
 * ffi/zkvm_accelerators.h zkvm_keccak256). id 0 is the standard's non-precompile
 * keccak accelerator. */
function keccak256(msg : list(byte)) -> bits(256) = acc_hash(0, msg)

/* ---- CREATE / CREATE2 address derivation (real, keccak + RLP) ---------- */

function word_to_bytes32(w : word) -> list(byte) = {
    var out : list(byte) = [||]; /* O(n): prepend big-endian */
    foreach (k from 0 to 31) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}
function address_to_bytes(a : address) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(a, 8 * k)[7 .. 0] :: out
    };
    out
}
function int_be_len(n : int) -> int = if n <= 0 then {
    0
} else {
    1 + int_be_len(ediv_int(n, 256))
}
termination_measure int_be_len(n) = n
function int_to_be_bytes(n : int) -> list(byte) = {
    let len = int_be_len(n);
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = list_append(out, [|get_slice_int(8, emod_int(ediv_int(n, pow2(8 * (len - 1 - k))), 256), 0)|])
    };
    out
}

/* RLP encoding of the nonce for CREATE address derivation. */
function rlp_nonce(nonce : int) -> list(byte) =
    if nonce == 0 then {
        [|0x80|]
    } else {
        if nonce < 128 then {
            [|get_slice_int(8, nonce, 0)|]
        } else {
            list_append([|get_slice_int(8, 128 + int_be_len(nonce), 0)|], int_to_be_bytes(nonce))
        }
    }

/* CREATE: address = keccak256(rlp([sender, nonce]))[12:] */
function create_address(sender : address, nonce : int) -> address = {
    let items = list_append(list_append([|0x94|], address_to_bytes(sender)), rlp_nonce(nonce));
    let payload = list_append([|get_slice_int(8, 192 + list_len(items), 0)|], items);
    word_to_address(keccak256(payload))
}

/* CREATE2: address = keccak256(0xff ++ sender ++ salt ++ keccak(init))[12:] */
function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    var buf : list(byte) = [|0xff|];
    buf = list_append(buf, address_to_bytes(sender));
    buf = list_append(buf, word_to_bytes32(salt));
    buf = list_append(buf, word_to_bytes32(init_hash));
    word_to_address(keccak256(buf))
}
