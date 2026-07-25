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

/*! Consumes an equal-index run within the selected storage slot and returns
    its final row plus the first row of the following run. */
function bal_storage_change_run(first : BalStorageChangeEntry) -> (
    (BalStorageChangeEntry, option(BalStorageChangeEntry))
) = {
    var last = first;
    var next = bal_storage_change_next();
    var scanning : bool = true;
    while scanning termination_measure(2 ^ 64) do {
        match next {
            Some(entry) => if entry.index == first.index then {
                last = entry;
                next = bal_storage_change_next()
            } else {
                scanning = false
            },
            None() => scanning = false,
        }
    };
    (last, next)
}

/*! Compares every change-index run belonging to the selected storage slot. */
function bal_compare_storage_slot_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    fields : RlpCursor('source_off, 'source_len),
    current : option(BalStorageChangeEntry),
) -> (
    unit
) =
    match current {
        Some(first) => {
            let (last, next) = bal_storage_change_run(first);
            let (pair, remaining) = rlp_cursor_pop(fields);
            bal_compare_index_word(pair, first.index, last.value);
            bal_compare_storage_slot_changes(remaining, next)
        },
        None() => bal_expect_end(fields),
    }
termination_measure bal_compare_storage_slot_changes(fields, current) =
    fields.len

/*! Consumes the selected account's sorted storage-slot union. The host cursor
    drives one loop iteration per logical slot. A slot with one or more
    changes advances `storage_changes`; a change-free slot advances
    `storage_reads`. */
function bal_compare_storage_slots forall 'changes_source_off 'changes_source_len 'reads_source_off 'reads_source_len,
                                     source_valid_range('changes_source_off, 'changes_source_len) &
                                         source_valid_range('reads_source_off, 'reads_source_len). (
    initial_changes : RlpCursor('changes_source_off, 'changes_source_len),
    initial_reads : RlpCursor('reads_source_off, 'reads_source_len),
    item_count : item_count,
    maximum_items : item_count,
) -> (
    item_count
) = {
    var changes : ByteSlice = initial_changes;
    var reads : ByteSlice = initial_reads;
    var count : item_count = item_count;
    var current = bal_storage_slot_next();
    var scanning : bool = true;
    while scanning termination_measure(byte_slice_length(changes) + byte_slice_length(reads)) do {
        match current {
            Some(slot_entry) => {
                match slot_entry.change {
                    Some(first_change) => {
                        let (slot_field, remaining_changes) = rlp_cursor_pop(changes);
                        changes = remaining_changes;
                        let slot_fields = bal_ref_cursor(slot_field);
                        let (slot_value, slot_fields) = rlp_cursor_pop(slot_fields);
                        let (slot_changes, slot_fields) = rlp_cursor_pop(slot_fields);
                        bal_expect_end(slot_fields);
                        if bal_ref_word(slot_value) != slot_entry.slot then {
                            throw InvalidBlock(InvalidBlockAccessList)
                        };
                        bal_compare_storage_slot_changes(bal_ref_cursor(slot_changes), Some(first_change))
                    },
                    None() => {
                        let (slot_field, remaining_reads) = rlp_cursor_pop(reads);
                        reads = remaining_reads;
                        if bal_ref_word(slot_field) != slot_entry.slot then {
                            throw InvalidBlock(InvalidBlockAccessList)
                        }
                    },
                };
                count = bal_count_item(count, maximum_items);
                current = bal_storage_slot_next()
            },
            None() => scanning = false,
        }
    };
    rlp_cursor_expect_end(changes);
    rlp_cursor_expect_end(reads);
    count
}

/*! Consumes an equal-index balance run and returns its final row plus the
    first row of the following run. */
function bal_balance_change_run(first : BalBalanceChangeEntry) -> (
    (BalBalanceChangeEntry, option(BalBalanceChangeEntry))
) = {
    var last = first;
    var next = bal_balance_change_next();
    var scanning : bool = true;
    while scanning termination_measure(2 ^ 64) do {
        match next {
            Some(entry) => if entry.index == first.index then {
                last = entry;
                next = bal_balance_change_next()
            } else {
                scanning = false
            },
            None() => scanning = false,
        }
    };
    (last, next)
}

/*! Compares the final balance value for each change index. */
function bal_compare_balance_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    fields : RlpCursor('source_off, 'source_len),
    current : option(BalBalanceChangeEntry),
) -> (
    unit
) =
    match current {
        Some(first) => {
            let (last, next) = bal_balance_change_run(first);
            let (pair, remaining) = rlp_cursor_pop(fields);
            bal_compare_index_word(pair, first.index, last.value);
            bal_compare_balance_changes(remaining, next)
        },
        None() => bal_expect_end(fields),
    }
termination_measure bal_compare_balance_changes(fields, current) =
    fields.len

/*! Consumes an equal-index nonce run and returns its maximum value plus the
    first row of the following run. */
function bal_nonce_change_run(first : BalNonceChangeEntry) -> (account_nonce, option(BalNonceChangeEntry)) = {
    var maximum = first.value;
    var next = bal_nonce_change_next();
    var scanning : bool = true;
    while scanning termination_measure(2 ^ 64) do {
        match next {
            Some(entry) => if entry.index == first.index then {
                if maximum < entry.value then {
                    maximum = entry.value
                };
                next = bal_nonce_change_next()
            } else {
                scanning = false
            },
            None() => scanning = false,
        }
    };
    (maximum, next)
}

/*! Compares the maximum nonce value for each change index. */
function bal_compare_nonce_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    fields : RlpCursor('source_off, 'source_len),
    current : option(BalNonceChangeEntry),
) -> (
    unit
) =
    match current {
        Some(first) => {
            let (maximum, next) = bal_nonce_change_run(first);
            let (pair, remaining) = rlp_cursor_pop(fields);
            bal_compare_index_nonce(pair, first.index, maximum);
            bal_compare_nonce_changes(remaining, next)
        },
        None() => bal_expect_end(fields),
    }
termination_measure bal_compare_nonce_changes(fields, current) =
    fields.len

/*! Consumes an equal-index code run and returns its final row plus the first
    row of the following run. */
function bal_code_change_run(first : BalCodeChangeEntry) -> (BalCodeChangeEntry, option(BalCodeChangeEntry)) = {
    var last = first;
    var next = bal_code_change_next();
    var scanning : bool = true;
    while scanning termination_measure(2 ^ 64) do {
        match next {
            Some(entry) => if entry.index == first.index then {
                last = entry;
                next = bal_code_change_next()
            } else {
                scanning = false
            },
            None() => scanning = false,
        }
    };
    (last, next)
}

/*! Compares the final code value for each change index. */
function bal_compare_code_changes forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    fields : RlpCursor('source_off, 'source_len),
    current : option(BalCodeChangeEntry),
) -> (
    unit
) =
    match current {
        Some(first) => {
            let (last, next) = bal_code_change_run(first);
            let (pair, remaining) = rlp_cursor_pop(fields);
            bal_compare_index_code(pair, first.index, last.code_hash);
            bal_compare_code_changes(remaining, next)
        },
        None() => bal_expect_end(fields),
    }
termination_measure bal_compare_code_changes(fields, current) =
    fields.len

/*! Compares one account and all five ordered BAL child collections. */
function bal_compare_account forall 'source_off 'source_len 'content_len,
                               rlp_field_ref_valid('source_off, 'source_len, 'content_len). (
    field : RlpFieldRef('source_off, 'source_len, 'content_len),
    account : address,
    item_count : item_count,
    maximum_items : item_count,
) -> (
    item_count
) = {
    let fields = bal_ref_cursor(field);
    let (address_field, fields) = rlp_cursor_pop(fields);
    let (storage_changes_field, fields) = rlp_cursor_pop(fields);
    let (storage_reads_field, fields) = rlp_cursor_pop(fields);
    let (balance_changes_field, fields) = rlp_cursor_pop(fields);
    let (nonce_changes_field, fields) = rlp_cursor_pop(fields);
    let (code_changes_field, fields) = rlp_cursor_pop(fields);
    bal_expect_end(fields);

    let address_bytes = bal_ref_bytes(address_field);
    if (address_bytes.len != 20) | (word_to_address(rlp_ref_word(address_field)) != account) then {
        throw InvalidBlock(InvalidBlockAccessList)
    };

    let counted_account = bal_count_item(item_count, maximum_items);
    let counted_storage = bal_compare_storage_slots(
        bal_ref_cursor(storage_changes_field),
        bal_ref_cursor(storage_reads_field),
        counted_account,
        maximum_items,
    );
    bal_compare_balance_changes(bal_ref_cursor(balance_changes_field), bal_balance_change_next());
    bal_compare_nonce_changes(bal_ref_cursor(nonce_changes_field), bal_nonce_change_next());
    bal_compare_code_changes(bal_ref_cursor(code_changes_field), bal_code_change_next());
    counted_storage
}

/*! Compares all BAL accounts in canonical address order. */
function bal_compare_accounts forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    fields : RlpCursor('source_off, 'source_len),
    current : option(address),
    item_count : item_count,
    maximum_items : item_count,
) -> (
    item_count
) =
    match current {
        Some(account) => {
            let (field, remaining) = rlp_cursor_pop(fields);
            let next_count = bal_compare_account(field, account, item_count, maximum_items);
            bal_compare_accounts(remaining, bal_account_next(), next_count, maximum_items)
        },
        None() => {
            bal_expect_end(fields);
            item_count
        },
    }
termination_measure bal_compare_accounts(fields, current, item_count, maximum_items) =
    fields.len

/*! Validates the supplied canonical EIP-7928 block access list directly
    against the recorder's ordered account/change streams. */
function validate_block_access_list(bytes : ByteSlice, maximum_items : item_count) -> unit = {
    bal_prepare();
    let root = rlp_single_ref(bytes);
    let _ = bal_compare_accounts(bal_ref_cursor(root), bal_account_next(), 0, maximum_items);
    ()
}
