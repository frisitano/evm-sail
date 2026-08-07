/*md
# The block access list

Validation of the EIP-7928 block access list supplied by the stateless host.
The supplied bytes are decoded as canonical RLP and consumed in lockstep with
the recorder's ordered account/change tables.  Validation never reconstructs
or re-encodes the list: the original source-backed slice is also the value
hashed into the execution-payload header.
*/

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
    let index_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, index_field.source.len);
    let value_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_field.source.len);
    bal_expect_end(fields);
    let decoded_index = bal_ref_uint64(index_field);
    let decoded_value = bal_ref_word(value_field);
    if (decoded_index != index) | (decoded_value != value) then {
        fatal_error(InvalidBlockAccessList)
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
    let index_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, index_field.source.len);
    let value_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_field.source.len);
    bal_expect_end(fields);
    let decoded_index = bal_ref_uint64(index_field);
    let decoded_value = bal_ref_uint64(value_field);
    if (decoded_index != index) | (decoded_value != value) then {
        fatal_error(InvalidBlockAccessList)
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
    let index_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, index_field.source.len);
    let code_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, code_field.source.len);
    bal_expect_end(fields);
    let code = code_db_resolve(code_hash);
    let decoded_index = bal_ref_uint64(index_field);
    let encoded_code = bal_ref_bytes(code_field);
    let expected_code = code_bytes(code);
    let code_matches = region_slices_equal(encoded_code, expected_code);
    let code_mismatch = not_bool(code_matches);
    if (decoded_index != index) | code_mismatch then {
        fatal_error(InvalidBlockAccessList)
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
) = {
    if cursor.len == 0 then {
        return ()
    };
    let pair = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, pair.source.len);
    let event = bal_iter_next();
    match event {
        BalStorageChange(change) => {
            if change.slot != slot then {
                fatal_error(InvalidBlockAccessList)
            };
            bal_compare_index_word(pair, change.index, change.value)
        },
        _ => fatal_error(InvalidBlockAccessList),
    };
    bal_validate_storage_change_values(next, slot)
}
termination_measure bal_validate_storage_change_values(cursor, slot) =
    cursor.len

/*! Consumes canonical `[slot, changes]` entries in their encoded order. */
function bal_validate_storage_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    range(0, 'source_len)
) =
    if cursor.len == 0 then {
        0
    } else {
        let slot_field = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, slot_field.source.len);
        let fields = bal_ref_cursor(slot_field);
        let slot_value = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, slot_value.source.len);
        let changes_value = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, changes_value.source.len);
        bal_expect_end(fields);
        let changes = bal_ref_cursor(changes_value);
        if changes.len == 0 then {
            fatal_error(InvalidBlockAccessList)
        };
        let slot = bal_ref_word(slot_value);
        bal_validate_storage_change_values(changes, slot);
        1 + bal_validate_storage_changes(next)
    }
termination_measure bal_validate_storage_changes(cursor) =
    cursor.len

/*! Consumes the read-only storage slots in their encoded order. */
function bal_validate_storage_reads forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    range(0, 'source_len)
) =
    if cursor.len == 0 then {
        0
    } else {
        let slot_field = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, slot_field.source.len);
        let slot = bal_ref_word(slot_field);
        let event = bal_iter_next();
        match event {
            BalStorageRead(recorded) => {
                if recorded != slot then {
                    fatal_error(InvalidBlockAccessList)
                }
            },
            _ => fatal_error(InvalidBlockAccessList),
        };
        1 + bal_validate_storage_reads(next)
    }
termination_measure bal_validate_storage_reads(cursor) =
    cursor.len

/*! Consumes balance changes in their encoded order. */
function bal_validate_balance_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) = {
    if cursor.len == 0 then {
        return ()
    };
    let pair = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, pair.source.len);
    let event = bal_iter_next();
    match event {
        BalBalanceChange(change) => bal_compare_index_word(pair, change.index, change.value),
        _ => fatal_error(InvalidBlockAccessList),
    };
    bal_validate_balance_changes(next)
}
termination_measure bal_validate_balance_changes(cursor) =
    cursor.len

/*! Consumes nonce changes in their encoded order. */
function bal_validate_nonce_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) = {
    if cursor.len == 0 then {
        return ()
    };
    let pair = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, pair.source.len);
    let event = bal_iter_next();
    match event {
        BalNonceChange(change) => bal_compare_index_nonce(pair, change.index, change.value),
        _ => fatal_error(InvalidBlockAccessList),
    };
    bal_validate_nonce_changes(next)
}
termination_measure bal_validate_nonce_changes(cursor) =
    cursor.len

/*! Consumes code changes in their encoded order. */
function bal_validate_code_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) = {
    if cursor.len == 0 then {
        return ()
    };
    let pair = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, pair.source.len);
    let event = bal_iter_next();
    match event {
        BalCodeChange(change) => bal_compare_index_code(pair, change.index, change.code_hash),
        _ => fatal_error(InvalidBlockAccessList),
    };
    bal_validate_code_changes(next)
}
termination_measure bal_validate_code_changes(cursor) =
    cursor.len

/*! Consumes account entries from the canonical RLP list.  The encoded cursor
    is the traversal driver; the host iterator supplies exactly one comparison
    event for every decoded semantic value. */
function bal_validate_accounts forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    range(0, 'source_len)
) =
    if cursor.len == 0 then {
        0
    } else {
        let account_field = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, account_field.source.len);
        let fields = bal_ref_cursor(account_field);
        let address_field = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, address_field.source.len);
        let storage_changes_field = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, storage_changes_field.source.len);
        let storage_reads_field = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, storage_reads_field.source.len);
        let balance_changes_field = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, balance_changes_field.source.len);
        let nonce_changes_field = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, nonce_changes_field.source.len);
        let code_changes_field = rlp_decode_item(fields);
        let fields = rlp_cursor_advance(fields, code_changes_field.source.len);
        bal_expect_end(fields);

        let address_bytes = bal_ref_bytes(address_field);
        let address_word = rlp_decode_word(address_field);
        let account = word_to_address(address_word);
        if address_bytes.len != 20 then {
            fatal_error(InvalidBlockAccessList)
        };
        let account_event = bal_iter_next();
        match account_event {
            BalAccount(recorded) => if recorded != account then {
                fatal_error(InvalidBlockAccessList)
            },
            _ => fatal_error(InvalidBlockAccessList),
        };

        let storage_changes_cursor = bal_ref_cursor(storage_changes_field);
        let storage_changes = bal_validate_storage_changes(storage_changes_cursor);
        let storage_reads_cursor = bal_ref_cursor(storage_reads_field);
        let storage_reads = bal_validate_storage_reads(storage_reads_cursor);
        let balance_changes_cursor = bal_ref_cursor(balance_changes_field);
        bal_validate_balance_changes(balance_changes_cursor);
        let nonce_changes_cursor = bal_ref_cursor(nonce_changes_field);
        bal_validate_nonce_changes(nonce_changes_cursor);
        let code_changes_cursor = bal_ref_cursor(code_changes_field);
        bal_validate_code_changes(code_changes_cursor);
        let account_end_event = bal_iter_next();
        match account_end_event {
            BalAccountEnd(_) => (),
            _ => fatal_error(InvalidBlockAccessList),
        };
        1 + storage_changes + storage_reads + bal_validate_accounts(next)
    }
termination_measure bal_validate_accounts(cursor) =
    cursor.len

/*! Validates the canonical EIP-7928 BAL directly against the host recorder. */
function validate_block_access_list(
    bytes : StatelessInputSliceAtMost(block_access_list_length_bound),
    block_gas_limit : block_gas_limit,
) -> (
    unit
) = {
    bal_prepare_iter();
    let root = rlp_single_ref(bytes);
    let accounts_cursor = bal_ref_cursor(root);
    let bal_items = bal_validate_accounts(accounts_cursor);
    let remaining_event = bal_iter_next();
    match remaining_event {
        BalEmpty(_) => (),
        _ => fatal_error(InvalidBlockAccessList),
    };
    if BLOCK_ACCESS_LIST_ITEM_GAS * bal_items > block_gas_limit then {
        fatal_error(BlockAccessListTooLarge)
    }
}
