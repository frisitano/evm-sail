/*md
# The block access list

Validation of the EIP-7928 block access list supplied by the stateless host.
The supplied bytes are decoded as canonical RLP and consumed in lockstep with
the recorder's ordered account/change tables.  Validation never reconstructs
or re-encodes the list: the original source-backed slice is also the value
hashed into the execution-payload header.
*/

/*! Counts one logical BAL item and enforces `gas_limit / 2000` without a
    separate sizing pass. */
function bal_count_item(count : item_count, maximum : item_count) -> item_count =
    if count < maximum then {
        count + 1
    } else {
        throw InvalidBlock(BlockAccessListTooLarge)
    }

/*! Requires a canonical RLP list and returns its child cursor. */
val bal_ref_cursor : forall 'source_off 'source_len 'content_len,
                       rlp_field_ref_valid('source_off, 'source_len, 'content_len). RlpFieldRef(
        'source_off,
        'source_len,
        'content_len,
    ) -> RlpCursor('source_off + ('source_len - 'content_len), 'content_len)
function bal_ref_cursor(f) =
    if f.is_list & rlp_ref_framing_canonical(f) then {
        rlp_ref_cursor(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires a canonical RLP byte string and returns its content slice. */
function bal_ref_bytes forall 'source_off 'source_len 'content_len,
                         rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ByteSlice
) =
    if rlp_ref_bytes_canonical(f) then {
        rlp_ref_content(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires a canonical RLP integer in the EVM-word domain. */
function bal_ref_word forall 'source_off 'source_len 'content_len,
                        rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    word
) =
    if rlp_ref_uint_canonical(f) & (f.content_len <= 32) then {
        rlp_ref_word(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires a canonical RLP integer in the host-index/nonce domain. */
function bal_ref_uint64 forall 'source_off 'source_len 'content_len,
                          rlp_field_ref_valid('source_off, 'source_len, 'content_len). (f :
    RlpFieldRef('source_off, 'source_len, 'content_len)) -> (
    ssz_uint
) =
    if rlp_ref_uint_canonical(f) & (f.content_len <= 8) then {
        rlp_ref_uint64(f)
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Requires that a decoded list has no unconsumed children. */
function bal_expect_end forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        throw InvalidBlock(InvalidBlockAccessList)
    }

/*! Compares one canonical `[index, word]` pair. */
function bal_compare_index_word forall 'source_off 'source_len 'content_len,
                                  rlp_field_ref_valid('source_off, 'source_len, 'content_len). (
    pair : RlpFieldRef('source_off, 'source_len, 'content_len),
    index : block_access_index,
    value : word,
) -> (
    unit
) = {
    let fields = bal_ref_cursor(pair);
    let (index_field, fields) = rlp_cursor_pop(fields);
    let (value_field, fields) = rlp_cursor_pop(fields);
    bal_expect_end(fields);
    if (bal_ref_uint64(index_field) != index) | (bal_ref_word(value_field) != value) then {
        throw InvalidBlock(InvalidBlockAccessList)
    }
}

/*! Compares one canonical `[index, nonce]` pair. */
function bal_compare_index_nonce forall 'source_off 'source_len 'content_len,
                                   rlp_field_ref_valid('source_off, 'source_len, 'content_len). (
    pair : RlpFieldRef('source_off, 'source_len, 'content_len),
    index : block_access_index,
    value : account_nonce,
) -> (
    unit
) = {
    let fields = bal_ref_cursor(pair);
    let (index_field, fields) = rlp_cursor_pop(fields);
    let (value_field, fields) = rlp_cursor_pop(fields);
    bal_expect_end(fields);
    if (bal_ref_uint64(index_field) != index) | (bal_ref_uint64(value_field) != value) then {
        throw InvalidBlock(InvalidBlockAccessList)
    }
}

/*! Compares one canonical `[index, code]` pair without materializing either
    source-backed code sequence. */
function bal_compare_index_code forall 'source_off 'source_len 'content_len,
                                  rlp_field_ref_valid('source_off, 'source_len, 'content_len). (
    pair : RlpFieldRef('source_off, 'source_len, 'content_len),
    index : block_access_index,
    code_hash : hash,
) -> (
    unit
) = {
    let fields = bal_ref_cursor(pair);
    let (index_field, fields) = rlp_cursor_pop(fields);
    let (code_field, fields) = rlp_cursor_pop(fields);
    bal_expect_end(fields);
    let code = code_db_resolve(code_hash);
    if (bal_ref_uint64(index_field) != index) | not_bool(byte_slices_equal(bal_ref_bytes(code_field), code.bytes)) then {
        throw InvalidBlock(InvalidBlockAccessList)
    }
}

/*! Consumes the encoded changes for one storage slot.  Each RLP pop strictly
    reduces the remaining byte length and must have one matching host event. */
function bal_validate_storage_change_values forall 'source_off 'source_len,
                                              source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    slot : word,
) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        let (pair, next) = rlp_cursor_pop(cursor);
        match bal_iter_next() {
            BalStorageChange(change) => {
                if change.slot != slot then {
                    throw InvalidBlock(InvalidBlockAccessList)
                };
                bal_compare_index_word(pair, change.index, change.value)
            },
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        bal_validate_storage_change_values(next, slot)
    }
termination_measure bal_validate_storage_change_values(cursor, slot) =
    cursor.len

/*! Consumes canonical `[slot, changes]` entries in their encoded order. */
function bal_validate_storage_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    count : item_count,
    maximum : item_count,
) -> (
    item_count
) =
    if cursor.len == 0 then {
        count
    } else {
        let (slot_field, next) = rlp_cursor_pop(cursor);
        let fields = bal_ref_cursor(slot_field);
        let (slot_value, fields) = rlp_cursor_pop(fields);
        let (changes_value, fields) = rlp_cursor_pop(fields);
        bal_expect_end(fields);
        let changes = bal_ref_cursor(changes_value);
        if changes.len == 0 then {
            throw InvalidBlock(InvalidBlockAccessList)
        };
        let slot = bal_ref_word(slot_value);
        bal_validate_storage_change_values(changes, slot);
        let count = bal_count_item(count, maximum);
        bal_validate_storage_changes(next, count, maximum)
    }
termination_measure bal_validate_storage_changes(cursor, count, maximum) =
    cursor.len

/*! Consumes the read-only storage slots in their encoded order. */
function bal_validate_storage_reads forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    count : item_count,
    maximum : item_count,
) -> (
    item_count
) =
    if cursor.len == 0 then {
        count
    } else {
        let (slot_field, next) = rlp_cursor_pop(cursor);
        let slot = bal_ref_word(slot_field);
        match bal_iter_next() {
            BalStorageRead(recorded) => {
                if recorded != slot then {
                    throw InvalidBlock(InvalidBlockAccessList)
                }
            },
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        let count = bal_count_item(count, maximum);
        bal_validate_storage_reads(next, count, maximum)
    }
termination_measure bal_validate_storage_reads(cursor, count, maximum) =
    cursor.len

/*! Consumes balance changes in their encoded order. */
function bal_validate_balance_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        let (pair, next) = rlp_cursor_pop(cursor);
        match bal_iter_next() {
            BalBalanceChange(change) => bal_compare_index_word(pair, change.index, change.value),
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        bal_validate_balance_changes(next)
    }
termination_measure bal_validate_balance_changes(cursor) =
    cursor.len

/*! Consumes nonce changes in their encoded order. */
function bal_validate_nonce_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        let (pair, next) = rlp_cursor_pop(cursor);
        match bal_iter_next() {
            BalNonceChange(change) => bal_compare_index_nonce(pair, change.index, change.value),
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        bal_validate_nonce_changes(next)
    }
termination_measure bal_validate_nonce_changes(cursor) =
    cursor.len

/*! Consumes code changes in their encoded order. */
function bal_validate_code_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        let (pair, next) = rlp_cursor_pop(cursor);
        match bal_iter_next() {
            BalCodeChange(change) => bal_compare_index_code(pair, change.index, change.code_hash),
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        bal_validate_code_changes(next)
    }
termination_measure bal_validate_code_changes(cursor) =
    cursor.len

/*! Consumes account entries from the canonical RLP list.  The encoded cursor
    is the traversal driver; the host iterator supplies exactly one comparison
    event for every decoded semantic value. */
function bal_validate_accounts forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    count : item_count,
    maximum : item_count,
) -> (
    item_count
) =
    if cursor.len == 0 then {
        count
    } else {
        let (account_field, next) = rlp_cursor_pop(cursor);
        let fields = bal_ref_cursor(account_field);
        let (address_field, fields) = rlp_cursor_pop(fields);
        let (storage_changes_field, fields) = rlp_cursor_pop(fields);
        let (storage_reads_field, fields) = rlp_cursor_pop(fields);
        let (balance_changes_field, fields) = rlp_cursor_pop(fields);
        let (nonce_changes_field, fields) = rlp_cursor_pop(fields);
        let (code_changes_field, fields) = rlp_cursor_pop(fields);
        bal_expect_end(fields);

        let address_bytes = bal_ref_bytes(address_field);
        let account = word_to_address(rlp_ref_word(address_field));
        if address_bytes.len != 20 then {
            throw InvalidBlock(InvalidBlockAccessList)
        };
        match bal_iter_next() {
            BalAccount(recorded) => if recorded != account then {
                throw InvalidBlock(InvalidBlockAccessList)
            },
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };

        let count = bal_count_item(count, maximum);
        let count = bal_validate_storage_changes(bal_ref_cursor(storage_changes_field), count, maximum);
        let count = bal_validate_storage_reads(bal_ref_cursor(storage_reads_field), count, maximum);
        bal_validate_balance_changes(bal_ref_cursor(balance_changes_field));
        bal_validate_nonce_changes(bal_ref_cursor(nonce_changes_field));
        bal_validate_code_changes(bal_ref_cursor(code_changes_field));
        match bal_iter_next() {
            BalAccountEnd(_) => (),
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        bal_validate_accounts(next, count, maximum)
    }
termination_measure bal_validate_accounts(cursor, count, maximum) =
    cursor.len

/*! Validates the canonical EIP-7928 BAL directly against the host recorder. */
function validate_block_access_list(bytes : ByteSlice, maximum_items : item_count) -> unit = {
    bal_prepare_iter();
    let root = rlp_single_ref(bytes);
    let _ = bal_validate_accounts(bal_ref_cursor(root), 0, maximum_items);
    match bal_iter_next() {
        BalEmpty(_) => (),
        _ => throw InvalidBlock(InvalidBlockAccessList),
    }
}
