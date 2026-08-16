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
    let missing = not_bool(acc.present);
    if missing then {
        ZERO_HASH
    } else {
        acc.info.code_hash
    }
}

/*! Deploys code to an account: analyzes, stores, and binds its hash. */
function k_deploy_code(a : address, code : CodeSlice) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let cur = k_aload(a);
    let h : hash = code_db_insert(code, profile.fork);
    store_account_info(a, cur, { cur.info with code_hash = h })
}

/*! Installs an EIP-7702 delegation designator
    (`0xef0100 ‖ target`) as the account's code. */
function k_set_delegation(a : address, target : address) -> unit = {
    let cur = k_aload(a);
    let execution_profile = k_execution_profile;
    let code_region = code_region_from_delegation(target);
    let code = validated_code_slice(code_region);
    let h : hash = code_db_insert(code, execution_profile.protocol.fork);
    store_account_info(a, cur, { cur.info with code_hash = h })
}

/*! Resets an account's code to empty (EIP-7702 clearing). */
function k_clear_code(a : address) -> unit = {
    let cur = k_aload(a);
    store_account_info(a, cur, { cur.info with code_hash = KECCAK_EMPTY })
}

/*! The delegation target of an account's code, with a validity flag —
    false when the code is not a designator. */
function k_deleg_target(a : address) -> (bool, address) = {
    let h : hash = k_code_key(a);
    let r : AddressResult = code_db_read_delegation(h);
    (r.success, r.address)
}

/*! `EXTCODESIZE`: the account's code length in bytes. */
function k_get_code_size(a : address) -> code_length = {
    let code_key = k_code_key(a);
    let code = code_db_resolve(code_key);
    code.len
}

/*! `EXTCODECOPY`: copies account code into frame memory, zero-padded
    past the end. */
function k_code_copy(a : address, dst : memory_base, off : word, len : memory_length) -> unit = {
    let code_key = k_code_key(a);
    let code = code_db_resolve(code_key);
    let bytes = code_bytes(code);
    slice_copy_word_offset(bytes, dst, off, len)
}
