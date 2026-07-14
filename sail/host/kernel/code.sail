function k_code_key(a : address) -> hash = k_aload(a).info.code_hash

/* EXTCODEHASH (EIP-1052): a truly empty account reads as 0, not
 * KECCAK_EMPTY. */
function k_get_codehash(a : address) -> hash = {
    let acc = k_aload(a);
    if not_bool(acc.exists) then {
        word_of_int(0)
    } else {
        acc.info.code_hash
    }
}

function k_deploy_code(a : address, code : ByteSlice) -> unit = {
    let cur = k_aload(a);
    let h : hash = code_db_insert(code);
    store_account_info(a, cur, { cur.info with code_hash = h })
}

function delegation_jumpdest_bitmap(target : address) -> JumpdestBitmap = {
    let w = address_to_word(target);
    var bits : bits(64) = 0x0000000000000000;
    foreach (k from 0 to 19) {
        let b = sail_shiftright(w, 8 * (19 - k))[7 .. 0];
        if b == 0x5b then {
            bits = or_vec(bits, sail_shiftleft(0x0000000000000001, 3 + k))
        }
    };
    [|bits|]
}

function k_set_delegation(a : address, target : address) -> unit = {
    let cur = k_aload(a);
    let h : hash = code_intern_delegation(target, delegation_jumpdest_bitmap(target));
    store_account_info(a, cur, { cur.info with code_hash = h })
}

function k_clear_code(a : address) -> unit = {
    let cur = k_aload(a);
    store_account_info(a, cur, { cur.info with code_hash = KECCAK_EMPTY })
}

function addr_bytes(a : address) -> list(byte) = {
    let w = address_to_word(a);
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}

function delegation_code(a : address) -> list(byte) = 0xef :: 0x01 :: 0x00 :: addr_bytes(a)

function k_deleg_target(a : address) -> (bool, address) = {
    let h : hash = k_code_key(a);
    let r : bits(168) = code_db_read_delegation(h);
    (r[160] == bitone, r[159 .. 0])
}

function k_get_code_size(a : address) -> int = {
    let code = code_db_resolve(k_code_key(a));
    unsigned(code.bytes.len)
}

function k_code_copy(a : address, dst : int, off : int, len : int) -> unit = {
    let code = code_db_resolve(k_code_key(a));
    let o = if off > 16777216 then { 16777216 } else { off };
    slice_copy(code.bytes, dst, o, len)
}
