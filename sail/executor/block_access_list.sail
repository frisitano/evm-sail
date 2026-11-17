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

/*! Historical transport slots are writable schema entries, not EIP-7928
    changes. Their sole `[0, 0]` marker carries no execution claim. */
function validate_historical_schema_slots forall 'off 'len, source_valid_range('off, 'len). (
    cursor : RlpCursor('off, 'len),
    previous : word,
    have_previous : bool,
    storage_root : hash,
) -> (
    unit
) = {
    if cursor.len == 0 then {
        return ()
    };
    let entry = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, entry.source.len);
    let fields = bal_ref_cursor(entry);
    let slot_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, slot_field.source.len);
    let markers = rlp_decode_item(fields);
    bal_expect_end(rlp_cursor_advance(fields, markers.source.len));
    let slot = bal_ref_word(slot_field);
    if have_previous & not_bool(word_ult(previous, slot)) then {
        fatal_error(InvalidBlockAccessList)
    };
    let values = bal_ref_cursor(markers);
    let marker = rlp_decode_item(values);
    bal_expect_end(rlp_cursor_advance(values, marker.source.len));
    bal_compare_index_word(marker, 0, ZERO_WORD);
    let _ = stateless_storage_by_key(storage_root, keccak256_word(slot));
    validate_historical_schema_slots(next, slot, true, storage_root)
}
termination_measure validate_historical_schema_slots(cursor, previous, have_previous, storage_root) = cursor.len

/*! Validate the uncommitted historical access schema identically on both
    backends before optimized C uses it to provision its dense state store. */
function validate_historical_schema_accounts forall 'off 'len, source_valid_range('off, 'len). (
    cursor : RlpCursor('off, 'len),
    previous : word,
    have_previous : bool,
) -> (
    unit
) = {
    if cursor.len == 0 then {
        return ()
    };
    let entry = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, entry.source.len);
    let fields = bal_ref_cursor(entry);
    let address_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, address_field.source.len);
    let slots_field = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, slots_field.source.len);
    let reads = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, reads.source.len);
    let balances = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, balances.source.len);
    let nonces = rlp_decode_item(fields);
    let fields = rlp_cursor_advance(fields, nonces.source.len);
    let codes = rlp_decode_item(fields);
    bal_expect_end(rlp_cursor_advance(fields, codes.source.len));
    let address_bytes = bal_ref_bytes(address_field);
    let address = rlp_decode_word(address_field);
    if address_bytes.len != 20 then {
        fatal_error(InvalidBlockAccessList)
    };
    if have_previous & not_bool(word_ult(previous, address)) then {
        fatal_error(InvalidBlockAccessList)
    };
    let account = stateless_account_by_key(k_parent_state_root, keccak256_address(word_to_address(address)));
    let info = account.info;
    validate_historical_schema_slots(bal_ref_cursor(slots_field), ZERO_WORD, false, info.storage_root);
    bal_expect_end(bal_ref_cursor(reads));
    bal_expect_end(bal_ref_cursor(balances));
    bal_expect_end(bal_ref_cursor(nonces));
    bal_expect_end(bal_ref_cursor(codes));
    validate_historical_schema_accounts(next, address, true)
}
termination_measure validate_historical_schema_accounts(cursor, previous, have_previous) = cursor.len

/*! Checks the complete historical access-schema framing and parent proofs. */
function validate_historical_access_schema(bytes : StatelessInputSliceAtMost(block_access_list_length_bound)) -> unit = {
    let root = rlp_single_ref(bytes);
    validate_historical_schema_accounts(bal_ref_cursor(root), ZERO_WORD, false)
}

/*! Searches sorted historical slot reservations without materializing them. */
function historical_schema_has_slot forall 'off 'len, source_valid_range('off, 'len). (
    cursor : RlpCursor('off, 'len),
    slot : word,
) -> (
    bool
) = {
    if cursor.len == 0 then {
        return false
    };
    let entry = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, entry.source.len);
    let fields = bal_ref_cursor(entry);
    let field = rlp_decode_item(fields);
    let candidate = bal_ref_word(field);
    if candidate == slot then {
        true
    } else if word_ult(slot, candidate) then {
        false
    } else {
        historical_schema_has_slot(next, slot)
    }
}
termination_measure historical_schema_has_slot(cursor, slot) = cursor.len

/*! Searches sorted historical account reservations and an optional slot. */
function historical_schema_has_access forall 'off 'len, source_valid_range('off, 'len). (
    cursor : RlpCursor('off, 'len),
    address : word,
    slot : option(word),
) -> (
    bool
) = {
    if cursor.len == 0 then {
        return false
    };
    let entry = rlp_decode_item(cursor);
    let next = rlp_cursor_advance(cursor, entry.source.len);
    let fields = bal_ref_cursor(entry);
    let field = rlp_decode_item(fields);
    let candidate = rlp_decode_word(field);
    if candidate == address then {
        let fields = rlp_cursor_advance(fields, field.source.len);
        let slots = rlp_decode_item(fields);
        match slot {
            None() => true,
            Some(key) => historical_schema_has_slot(bal_ref_cursor(slots), key),
        }
    } else if word_ult(address, candidate) then {
        false
    } else {
        historical_schema_has_access(next, address, slot)
    }
}
termination_measure historical_schema_has_access(cursor, address, slot) = cursor.len

/*! Enforces historical allocation-schema membership at kernel accesses. */
function require_historical_state_access(address, slot) = {
    let execution_profile = k_execution_profile;
    let protocol = execution_profile.protocol;
    if protocol.fork < Amsterdam then {
        /* Raw host warm bits certify a previous schema check (or a storage
           ID from the validated dense schema). Account warmth never proves
           slot membership. Do not use the kernel's precompile shortcut. */
        let already_checked : bool = match slot {
            None() => account_is_warm(address),
            Some(key) => storage_is_warm(address, key),
        };
        if not_bool(already_checked) then {
            let root = rlp_single_ref(k_historical_access_schema);
            let found = historical_schema_has_access(bal_ref_cursor(root), address_to_word(address), slot);
            if not_bool(found) then {
                fatal_error(InvalidBlockAccessList)
            }
        }
    }
}

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
