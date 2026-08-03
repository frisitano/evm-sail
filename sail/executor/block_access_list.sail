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
    let index_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, index_field.source.len);
    let value_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, value_field.source.len);
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
    let index_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, index_field.source.len);
    let code_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, code_field.source.len);
    bal_expect_end(fields);
    let code = code_db_resolve(code_hash);
    if (bal_ref_uint64(index_field) != index) | not_bool(region_slices_equal(bal_ref_bytes(code_field), code.bytes)) then {
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
        let pair = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, pair.source.len);
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
    cursor : RlpCursor('source_off, 'source_len)
) -> (
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
            throw InvalidBlock(InvalidBlockAccessList)
        };
        let slot = bal_ref_word(slot_value);
        bal_validate_storage_change_values(changes, slot);
        1 + bal_validate_storage_changes(next)
    }
termination_measure bal_validate_storage_changes(cursor) =
    cursor.len

/*! Consumes the read-only storage slots in their encoded order. */
function bal_validate_storage_reads forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len)
) -> (
    range(0, 'source_len)
) =
    if cursor.len == 0 then {
        0
    } else {
        let slot_field = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, slot_field.source.len);
        let slot = bal_ref_word(slot_field);
        match bal_iter_next() {
            BalStorageRead(recorded) => {
                if recorded != slot then {
                    throw InvalidBlock(InvalidBlockAccessList)
                }
            },
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        1 + bal_validate_storage_reads(next)
    }
termination_measure bal_validate_storage_reads(cursor) =
    cursor.len

/*! Consumes balance changes in their encoded order. */
function bal_validate_balance_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (cursor :
    RlpCursor('source_off, 'source_len)) -> (
    unit
) =
    if cursor.len == 0 then {
        ()
    } else {
        let pair = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, pair.source.len);
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
        let pair = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, pair.source.len);
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
        let pair = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, pair.source.len);
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
    cursor : RlpCursor('source_off, 'source_len)
) -> (
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
        let account = word_to_address(rlp_decode_word(address_field));
        if address_bytes.len != 20 then {
            throw InvalidBlock(InvalidBlockAccessList)
        };
        match bal_iter_next() {
            BalAccount(recorded) => if recorded != account then {
                throw InvalidBlock(InvalidBlockAccessList)
            },
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };

        let storage_changes = bal_validate_storage_changes(bal_ref_cursor(storage_changes_field));
        let storage_reads = bal_validate_storage_reads(bal_ref_cursor(storage_reads_field));
        bal_validate_balance_changes(bal_ref_cursor(balance_changes_field));
        bal_validate_nonce_changes(bal_ref_cursor(nonce_changes_field));
        bal_validate_code_changes(bal_ref_cursor(code_changes_field));
        match bal_iter_next() {
            BalAccountEnd(_) => (),
            _ => throw InvalidBlock(InvalidBlockAccessList),
        };
        1 + storage_changes + storage_reads + bal_validate_accounts(next)
    }
termination_measure bal_validate_accounts(cursor) =
    cursor.len

/*! Validates the canonical EIP-7928 BAL directly against the host recorder. */
function validate_block_access_list(
    bytes : StatelessInputSliceAtMost(block_access_list_length_bound),
    block_gas_limit : block_gas_limit,
) -> unit = {
    bal_prepare_iter();
    let root = rlp_single_ref(bytes);
    let bal_items = bal_validate_accounts(bal_ref_cursor(root));
    match bal_iter_next() {
        BalEmpty(_) => (),
        _ => throw InvalidBlock(InvalidBlockAccessList),
    };
    if BLOCK_ACCESS_LIST_ITEM_GAS * bal_items > block_gas_limit then {
        throw InvalidBlock(BlockAccessListTooLarge)
    }
}
