/*md
# State: account code

Account code operations over the content-addressed code store, including
EIP-7702 delegation designators.
*/

/*! The account's code hash — the code-store key. */
function k_code_key(a : address) -> hash = k_aload(a).info.code_hash

/*! `EXTCODEHASH` (EIP-1052): a truly non-existent account reads as `0`,
    not `KECCAK_EMPTY`. */
function k_get_codehash(a : address) -> hash = {
    let acc = k_aload(a);
    if not_bool(acc.present) then {
        WORD_ZERO
    } else {
        acc.info.code_hash
    }
}

/*! Deploys code to an account: analyzes, stores, and binds its hash. */
function k_deploy_code(a : address, code : ByteSlice) -> unit = {
    let cur = k_aload(a);
    let h : hash = code_db_insert(code);
    store_account_info(a, cur, { cur.info with code_hash = h })
}

/*! Computes the PUSH-aware JUMPDEST bitmap of an EIP-7702 delegation
    designator. */
function delegation_jumpdest_chunk(target : address) -> JumpdestChunk = {
    let w = address_to_word(target);
    var bits = EMPTY_JUMPDEST_CHUNK;
    foreach (k from 0 to 19) {
        let b = sail_shiftright(w, 8 * (19 - k))[7 .. 0];
        if b == 0x5b then {
            bits = or_vec(bits, sail_shiftleft(WORD_ONE, 3 + k))
        }
    };
    bits
}

/*! Installs an EIP-7702 delegation designator
    (`0xef0100 ‖ target`) as the account's code. */
function k_set_delegation(a : address, target : address) -> unit = {
    let cur = k_aload(a);
    let code_len : byte_length = ByteQuantity(23);
    let table = jumpdest_table_alloc(code_len);
    assert(table != EMPTY_JUMPDEST_REF, "delegation JUMPDEST table allocation");
    let chunk = delegation_jumpdest_chunk(target);
    if chunk != EMPTY_JUMPDEST_CHUNK then {
        let stored = jumpdest_table_store_chunk(table, code_len, BYTE_ZERO, chunk);
        assert(stored, "delegation JUMPDEST chunk store")
    };
    let h : hash = code_intern_delegation(target, table);
    store_account_info(a, cur, { cur.info with code_hash = h })
}

/*! Resets an account's code to empty (EIP-7702 clearing). */
function k_clear_code(a : address) -> unit = {
    let cur = k_aload(a);
    store_account_info(a, cur, { cur.info with code_hash = KECCAK_EMPTY })
}

/*! An address as 20 big-endian bytes. */
function addr_bytes(a : address) -> list(byte) = {
    let w = address_to_word(a);
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}

/*! The EIP-7702 delegation designator bytes: `0xef0100 ‖ address`. */
function delegation_code(a : address) -> list(byte) = 0xef :: 0x01 :: 0x00 :: addr_bytes(a)

/*! The delegation target of an account's code, with a validity flag —
    false when the code is not a designator. */
function k_deleg_target(a : address) -> (bool, address) = {
    let h : hash = k_code_key(a);
    let r : bits(168) = code_db_read_delegation(h);
    (r[160] == bitone, r[159 .. 0])
}

/*! `EXTCODESIZE`: the account's code length in bytes. */
function k_get_code_size(a : address) -> byte_length = {
    let code = code_db_resolve(k_code_key(a));
    code.bytes.len
}

/*! `EXTCODECOPY`: copies account code into frame memory, zero-padded
    past the end. */
function k_code_copy(a : address, dst : memory_pointer, off : word, len : memory_length) -> unit = {
    let code = code_db_resolve(k_code_key(a));
    slice_copy_word_offset(code.bytes, dst, off, len)
}
