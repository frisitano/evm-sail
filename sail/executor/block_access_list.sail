/*md
# The block access list

Reconstruction of the EIP-7928 block access list from the recorder's
per-account change streams, as source-backed RLP that is compared against
the payload's claimed bytes.

## Types

The cursor records pair an encoded-content length with the ordered-table
position reached by a sizing pass, so the subsequent writer follows exactly
the same grouping boundaries.
*/

/*! The reconstructed BAL: its RLP and its item count (bounded by
    `gas_limit / 2000`). */
struct EncodedBlockAccessList = {
    bytes : ByteSlice,
    item_count : item_count,
}

/*! A BAL RLP span. The enclosing SSZ field is `ByteList[2^30]`, so every
    sizing cursor can retain that schema bound instead of repeatedly falling
    back to the backend-wide byte-address maximum. */
type bal_rlp_length = range(0, block_access_list_length_bound)

let BAL_RLP_ZERO : bal_rlp_length = 0

/*! Narrows a byte length to the BAL schema limit. */
val bal_rlp_length_from_byte_length : forall 'value, 0 <= 'value. int('value) -> bal_rlp_length
function bal_rlp_length_from_byte_length(value) =
    if value <= sizeof(block_access_list_length_bound) then {
        value
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

/*! Exposes a schema-bounded BAL RLP length as a byte length. */
function bal_rlp_length_to_byte_length(value : bal_rlp_length) -> range(0, block_access_list_length_bound) =
    value

/*! Advances a BAL sizing cursor, checking its one protocol/schema limit. */
val bal_rlp_length_add : forall 'addition, 0 <= 'addition. (bal_rlp_length, int('addition)) -> bal_rlp_length
function bal_rlp_length_add(current, addition) = {
    /* Narrow only after the schema check.  A caller may use a wider native
     * representation for its source length, but every subsequent BAL cursor
     * operation stays in the schema-bounded representation. */
    let bounded_addition = bal_rlp_length_from_byte_length(addition);
    if bounded_addition <= sizeof(block_access_list_length_bound) - current then {
        current + bounded_addition
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }
}

val bal_bounded_byte_length_add : forall 'left 'right, 0 <= 'left & 0 <= 'right. (int('left), int('right)) -> range(
        0,
        block_access_list_length_bound,
    )
function bal_bounded_byte_length_add(left, right) =
    bal_rlp_length_to_byte_length(bal_rlp_length_add(bal_rlp_length_from_byte_length(left), right))

/*! Adds the canonical RLP list prefix to a bounded BAL content length. */
function bal_rlp_list_size(content_len : bal_rlp_length) -> bal_rlp_length = {
    let content_length = bal_rlp_length_to_byte_length(content_len);
    let encoded_length = rlp_list_size(content_length);
    bal_rlp_length_from_byte_length(encoded_length)
}

val bal_bounded_list_size : forall 'content_len, 0 <= 'content_len. int('content_len) -> range(
        0,
        block_access_list_length_bound,
    )
function bal_bounded_list_size(content_len) =
    bal_rlp_length_to_byte_length(bal_rlp_list_size(bal_rlp_length_from_byte_length(content_len)))

/*! An encoded-content length paired with the next table cursor. */
struct BalContentCursor = {
    content_len : bal_rlp_length,
    cursor : item_index,
}

/*! An encoded-content length paired with its logical BAL item count. */
struct BalContentCount = {
    content_len : bal_rlp_length,
    count : item_count,
}

/*! The encoded length and logical item count contributed by one account. */
struct BalAccountSize = {
    encoded_len : bal_rlp_length,
    item_count : item_count,
}

/*! The end and maximum value of one equal-index nonce run. */
struct BalNonceRun = {
    cursor : item_index,
    maximum : account_nonce,
}

/*! Adds logical BAL item counts. The enclosing BAL byte-list bound limits
    every realizable item sequence. */
val bal_count_add : forall 'a 'b, 0 <= 'a & 0 <= 'b. (int('a), int('b)) -> range(0, block_access_list_length_bound)
function bal_count_add(a, b) =
    if (a <= sizeof(block_access_list_length_bound)) & (b <= sizeof(block_access_list_length_bound) - a) then {
        a + b
    } else {
        throw InvalidBlock(BlockAccessListTooLarge)
    }

/*! Returns the final row of a non-empty BAL run. */
function bal_previous_index(value : item_index) -> item_index =
    if value != 0 then {
        value - 1
    } else {
        throw InvalidBlock(ExecutionInvalid)
    }

function bal_index_word_content_size(index : item_index, value : word) -> bal_rlp_length = {
    bal_bounded_byte_length_add(rlp_uint_size(index), rlp_uint_word_size(value))
}

function bal_index_word_size(index : item_index, value : word) -> bal_rlp_length =
    bal_bounded_list_size(bal_index_word_content_size(index, value))

/*! Writes one `[index, word]` BAL pair. */
function bal_write_index_word(index : item_index, value : word) -> unit = {
    rlp_write_list_prefix(bal_index_word_content_size(index, value));
    rlp_write_uint(index);
    rlp_write_uint_word(value)
}

function bal_index_nonce_content_size(index : item_index, value : account_nonce) -> bal_rlp_length = {
    bal_bounded_byte_length_add(rlp_uint_size(index), rlp_uint_size(value))
}

function bal_index_nonce_size(index : item_index, value : account_nonce) -> bal_rlp_length =
    bal_bounded_list_size(bal_index_nonce_content_size(index, value))

/*! Writes one `[index, nonce]` BAL pair. */
function bal_write_index_nonce(index : item_index, value : account_nonce) -> unit = {
    rlp_write_list_prefix(bal_index_nonce_content_size(index, value));
    rlp_write_uint(index);
    rlp_write_uint(value)
}

/*! Sizes the content of an index-and-code pair. */
function bal_index_code_content_size(index : item_index, code_hash : hash) -> bal_rlp_length = {
    let code = code_db_resolve(code_hash);
    let index_length = rlp_uint_size(index);
    let code_length = rlp_slice_size(code.bytes);
    bal_bounded_byte_length_add(index_length, code_length)
}

function bal_index_code_size(index : item_index, code_hash : hash) -> bal_rlp_length =
    bal_bounded_list_size(bal_index_code_content_size(index, code_hash))

/*! Writes one `[index, code]` BAL pair after resolving the code hash. */
function bal_write_index_code(index : item_index, code_hash : hash) -> unit = {
    let code = code_db_resolve(code_hash);
    rlp_write_list_prefix(bal_index_code_content_size(index, code_hash));
    rlp_write_uint(index);
    rlp_write_slice(code.bytes)
}

/* The unconsumed suffix of one (slot, index) run strictly decreases. */
/*! Finds the end of an equal-slot, equal-index storage-change run. */
function bal_storage_change_run_end(
    account : item_index,
    count : item_count,
    slot : word,
    index : item_index,
    cursor : item_index,
) -> (
    item_index
) =
    if cursor < count then {
        if bal_storage_change_slot(account, cursor) == slot & bal_storage_change_index(account, cursor) == index then {
            bal_storage_change_run_end(account, count, slot, index, cursor + 1)
        } else {
            cursor
        }
    } else {
        cursor
    }
termination_measure bal_storage_change_run_end(account, count, slot, index, cursor) =
    count - cursor

/* The unconsumed suffix of one slot's ordered change rows strictly decreases. */
/*! Sizes the final values for all change indices of one storage slot. */
function bal_storage_slot_changes_size(
    account : item_index,
    count : item_count,
    slot : word,
    cursor : item_index,
    content_len : bal_rlp_length,
) -> (
    BalContentCursor
) =
    if cursor < count then {
        if bal_storage_change_slot(account, cursor) == slot then {
            let index = bal_storage_change_index(account, cursor);
            let next = bal_storage_change_run_end(account, count, slot, index, cursor);
            let last = bal_previous_index(next);
            let next_len = bal_rlp_length_add(
                content_len,
                bal_index_word_size(index, bal_storage_change_value(account, last)),
            );
            bal_storage_slot_changes_size(account, count, slot, next, next_len)
        } else {
            struct { content_len = content_len, cursor = cursor }
        }
    } else {
        struct { content_len = content_len, cursor = cursor }
    }
termination_measure bal_storage_slot_changes_size(account, count, slot, cursor, content_len) =
    count - cursor

/* The unconsumed suffix of one slot's ordered change rows strictly decreases. */
/*! Writes the final value for each change index of one storage slot. */
function bal_write_storage_slot_changes(
    account : item_index,
    count : item_count,
    slot : word,
    cursor : item_index,
) -> (
    item_index
) =
    if cursor < count then {
        if bal_storage_change_slot(account, cursor) == slot then {
            let index = bal_storage_change_index(account, cursor);
            let next = bal_storage_change_run_end(account, count, slot, index, cursor);
            let last = bal_previous_index(next);
            bal_write_index_word(index, bal_storage_change_value(account, last));
            bal_write_storage_slot_changes(account, count, slot, next)
        } else {
            cursor
        }
    } else {
        cursor
    }
termination_measure bal_write_storage_slot_changes(account, count, slot, cursor) =
    count - cursor

/* The unconsumed ordered storage-change table strictly decreases. */
/*! Sizes storage changes grouped by slot and counts the groups. */
function bal_storage_change_groups_size(
    account : item_index,
    count : item_count,
    cursor : item_index,
    result : BalContentCount,
) -> (
    BalContentCount
) =
    if cursor < count then {
        let slot = bal_storage_change_slot(account, cursor);
        let changes = bal_storage_slot_changes_size(account, count, slot, cursor, BAL_RLP_ZERO);
        let slot_content_len = bal_rlp_length_add(
            bal_rlp_length_from_byte_length(rlp_uint_word_size(slot)),
            bal_rlp_length_to_byte_length(bal_rlp_list_size(changes.content_len)),
        );
        let next_content_len = bal_rlp_length_add(
            result.content_len,
            bal_rlp_length_to_byte_length(bal_rlp_list_size(slot_content_len)),
        );
        bal_storage_change_groups_size(
            account,
            count,
            changes.cursor,
            struct { content_len = next_content_len, count = bal_count_add(result.count, 1) },
        )
    } else {
        result
    }
termination_measure bal_storage_change_groups_size(account, count, cursor, result) =
    count - cursor

/* The unconsumed ordered storage-change table strictly decreases. */
/*! Writes storage changes grouped by slot. */
function bal_write_storage_change_groups(account : item_index, count : item_count, cursor : item_index) -> unit =
    if cursor < count then {
        let slot = bal_storage_change_slot(account, cursor);
        let changes = bal_storage_slot_changes_size(account, count, slot, cursor, BAL_RLP_ZERO);
        let slot_content_len = bal_rlp_length_add(
            bal_rlp_length_from_byte_length(rlp_uint_word_size(slot)),
            bal_rlp_length_to_byte_length(bal_rlp_list_size(changes.content_len)),
        );
        rlp_write_list_prefix(bal_rlp_length_to_byte_length(slot_content_len));
        rlp_write_uint_word(slot);
        rlp_write_list_prefix(bal_rlp_length_to_byte_length(changes.content_len));
        let next = bal_write_storage_slot_changes(account, count, slot, cursor);
        assert(next == changes.cursor, "BAL storage-change sizing cursor");
        bal_write_storage_change_groups(account, count, next)
    }
termination_measure bal_write_storage_change_groups(account, count, cursor) =
    count - cursor

/*! Sizes the storage-change list for one account. */
function bal_storage_changes_size(account : item_index) -> BalContentCount =
    bal_storage_change_groups_size(
        account,
        bal_storage_change_count(account),
        0,
        struct { content_len = BAL_RLP_ZERO, count = 0 },
    )

/*! Writes the storage-change list for one account. */
function bal_write_storage_changes(account : item_index, size : BalContentCount) -> unit = {
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(size.content_len));
    bal_write_storage_change_groups(account, bal_storage_change_count(account), 0)
}

/* The unconsumed run of equal read slots strictly decreases. */
/*! Finds the end of a run of equal storage-read slots. */
function bal_storage_read_run_end(
    account : item_index,
    count : item_count,
    slot : word,
    cursor : item_index,
) -> (
    item_index
) =
    if cursor < count then {
        if bal_storage_read_slot(account, cursor) == slot then {
            bal_storage_read_run_end(account, count, slot, cursor + 1)
        } else {
            cursor
        }
    } else {
        cursor
    }
termination_measure bal_storage_read_run_end(account, count, slot, cursor) =
    count - cursor

/* The unconsumed ordered change table strictly decreases while seeking slot. */
/*! Advances the ordered change cursor to the first slot not below a read. */
function bal_storage_change_seek(
    account : item_index,
    count : item_count,
    slot : word,
    cursor : item_index,
) -> (
    item_index
) =
    if cursor < count then {
        if word_ult(bal_storage_change_slot(account, cursor), slot) then {
            bal_storage_change_seek(account, count, slot, cursor + 1)
        } else {
            cursor
        }
    } else {
        cursor
    }
termination_measure bal_storage_change_seek(account, count, slot, cursor) =
    count - cursor

/* The unconsumed ordered storage-read table strictly decreases. */
/*! Sizes read slots that are not also represented by a storage change. */
function bal_storage_read_groups_size(
    account : item_index,
    read_count : item_count,
    change_count : item_count,
    read : item_index,
    change : item_index,
    result : BalContentCount,
) -> (
    BalContentCount
) =
    if read < read_count then {
        let slot = bal_storage_read_slot(account, read);
        let next_read = bal_storage_read_run_end(account, read_count, slot, read);
        let next_change = bal_storage_change_seek(account, change_count, slot, change);
        let changed : bool = if next_change < change_count then {
                bal_storage_change_slot(account, next_change) == slot
            } else {
                false
            };
        let updated = if changed then {
                result
            } else {
                struct {
                    content_len = bal_rlp_length_add(result.content_len, rlp_uint_word_size(slot)),
                    count = bal_count_add(result.count, 1),
                }
            };
        bal_storage_read_groups_size(account, read_count, change_count, next_read, next_change, updated)
    } else {
        result
    }
termination_measure bal_storage_read_groups_size(account, read_count, change_count, read, change, result) =
    read_count - read

/* The unconsumed ordered storage-read table strictly decreases. */
/*! Writes read slots that are not also represented by a storage change. */
function bal_write_storage_read_groups(
    account : item_index,
    read_count : item_count,
    change_count : item_count,
    read : item_index,
    change : item_index,
) -> (
    unit
) =
    if read < read_count then {
        let slot = bal_storage_read_slot(account, read);
        let next_read = bal_storage_read_run_end(account, read_count, slot, read);
        let next_change = bal_storage_change_seek(account, change_count, slot, change);
        let changed : bool = if next_change < change_count then {
                bal_storage_change_slot(account, next_change) == slot
            } else {
                false
            };
        if not_bool(changed) then {
            rlp_write_uint_word(slot)
        };
        bal_write_storage_read_groups(account, read_count, change_count, next_read, next_change)
    }
termination_measure bal_write_storage_read_groups(account, read_count, change_count, read, change) =
    read_count - read

/*! Sizes the storage-read list for one account. */
function bal_storage_reads_size(account : item_index) -> BalContentCount =
    bal_storage_read_groups_size(
        account,
        bal_storage_read_count(account),
        bal_storage_change_count(account),
        0,
        0,
        struct { content_len = BAL_RLP_ZERO, count = 0 },
    )

/*! Writes the storage-read list for one account. */
function bal_write_storage_reads(account : item_index, size : BalContentCount) -> unit = {
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(size.content_len));
    bal_write_storage_read_groups(account, bal_storage_read_count(account), bal_storage_change_count(account), 0, 0)
}

/* The unconsumed run of equal balance-change indices strictly decreases. */
/*! Finds the end of a run of equal balance-change indices. */
function bal_balance_run_end(
    account : item_index,
    count : item_count,
    index : item_index,
    cursor : item_index,
) -> (
    item_index
) =
    if cursor < count then {
        if bal_balance_change_index(account, cursor) == index then {
            bal_balance_run_end(account, count, index, cursor + 1)
        } else {
            cursor
        }
    } else {
        cursor
    }
termination_measure bal_balance_run_end(account, count, index, cursor) = count - cursor

/* The unconsumed ordered balance-change table strictly decreases. */
/*! Sizes the final balance value for each change index. */
function bal_balance_groups_size(
    account : item_index,
    count : item_count,
    cursor : item_index,
    content_len : bal_rlp_length,
) -> (
    bal_rlp_length
) =
    if cursor < count then {
        let index = bal_balance_change_index(account, cursor);
        let next = bal_balance_run_end(account, count, index, cursor);
        let last = bal_previous_index(next);
        let next_length = bal_rlp_length_add(
            content_len,
            bal_index_word_size(index, bal_balance_change_value(account, last)),
        );
        bal_balance_groups_size(account, count, next, next_length)
    } else {
        content_len
    }
termination_measure bal_balance_groups_size(account, count, cursor, content_len) = count - cursor

/* The unconsumed ordered balance-change table strictly decreases. */
/*! Writes the final balance value for each change index. */
function bal_write_balance_groups(account : item_index, count : item_count, cursor : item_index) -> unit =
    if cursor < count then {
        let index = bal_balance_change_index(account, cursor);
        let next = bal_balance_run_end(account, count, index, cursor);
        let last = bal_previous_index(next);
        bal_write_index_word(index, bal_balance_change_value(account, last));
        bal_write_balance_groups(account, count, next)
    }
termination_measure bal_write_balance_groups(account, count, cursor) = count - cursor

function bal_balance_changes_size(account : item_index) -> bal_rlp_length =
    bal_balance_groups_size(account, bal_balance_change_count(account), 0, BAL_RLP_ZERO)

/*! Writes the balance-change list for one account. */
function bal_write_balance_changes(account : item_index, content_len : bal_rlp_length) -> unit = {
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(content_len));
    bal_write_balance_groups(account, bal_balance_change_count(account), 0)
}

/* The unconsumed run of equal nonce-change indices strictly decreases. */
/*! Finds one nonce-change run and its greatest observed nonce. */
function bal_nonce_run(
    account : item_index,
    count : item_count,
    index : item_index,
    cursor : item_index,
    maximum : account_nonce,
) -> (
    BalNonceRun
) =
    if cursor < count then {
        if bal_nonce_change_index(account, cursor) == index then {
            let value = bal_nonce_change_value(account, cursor);
            let next_maximum = if maximum < value then { value } else { maximum };
            bal_nonce_run(account, count, index, cursor + 1, next_maximum)
        } else {
            struct { cursor = cursor, maximum = maximum }
        }
    } else {
        struct { cursor = cursor, maximum = maximum }
    }
termination_measure bal_nonce_run(account, count, index, cursor, maximum) = count - cursor

/* The unconsumed ordered nonce-change table strictly decreases. */
/*! Sizes the maximum nonce value for each change index. */
function bal_nonce_groups_size(
    account : item_index,
    count : item_count,
    cursor : item_index,
    content_len : bal_rlp_length,
) -> (
    bal_rlp_length
) =
    if cursor < count then {
        let index = bal_nonce_change_index(account, cursor);
        let first = bal_nonce_change_value(account, cursor);
        let run = bal_nonce_run(account, count, index, cursor + 1, first);
        let next_length = bal_rlp_length_add(content_len, bal_index_nonce_size(index, run.maximum));
        bal_nonce_groups_size(account, count, run.cursor, next_length)
    } else {
        content_len
    }
termination_measure bal_nonce_groups_size(account, count, cursor, content_len) = count - cursor

/* The unconsumed ordered nonce-change table strictly decreases. */
/*! Writes the maximum nonce value for each change index. */
function bal_write_nonce_groups(account : item_index, count : item_count, cursor : item_index) -> unit =
    if cursor < count then {
        let index = bal_nonce_change_index(account, cursor);
        let first = bal_nonce_change_value(account, cursor);
        let run = bal_nonce_run(account, count, index, cursor + 1, first);
        bal_write_index_nonce(index, run.maximum);
        bal_write_nonce_groups(account, count, run.cursor)
    }
termination_measure bal_write_nonce_groups(account, count, cursor) = count - cursor

function bal_nonce_changes_size(account : item_index) -> bal_rlp_length =
    bal_nonce_groups_size(account, bal_nonce_change_count(account), 0, BAL_RLP_ZERO)

/*! Writes the nonce-change list for one account. */
function bal_write_nonce_changes(account : item_index, content_len : bal_rlp_length) -> unit = {
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(content_len));
    bal_write_nonce_groups(account, bal_nonce_change_count(account), 0)
}

/* The unconsumed run of equal code-change indices strictly decreases. */
/*! Finds the end of a run of equal code-change indices. */
function bal_code_run_end(
    account : item_index,
    count : item_count,
    index : item_index,
    cursor : item_index,
) -> (
    item_index
) =
    if cursor < count then {
        if bal_code_change_index(account, cursor) == index then {
            bal_code_run_end(account, count, index, cursor + 1)
        } else {
            cursor
        }
    } else {
        cursor
    }
termination_measure bal_code_run_end(account, count, index, cursor) = count - cursor

/* The unconsumed ordered code-change table strictly decreases. */
/*! Sizes the final code value for each change index. */
function bal_code_groups_size(
    account : item_index,
    count : item_count,
    cursor : item_index,
    content_len : bal_rlp_length,
) -> (
    bal_rlp_length
) =
    if cursor < count then {
        let index = bal_code_change_index(account, cursor);
        let next = bal_code_run_end(account, count, index, cursor);
        let last = bal_previous_index(next);
        let next_length = bal_rlp_length_add(
            content_len,
            bal_index_code_size(index, bal_code_change_hash(account, last)),
        );
        bal_code_groups_size(account, count, next, next_length)
    } else {
        content_len
    }
termination_measure bal_code_groups_size(account, count, cursor, content_len) = count - cursor

/* The unconsumed ordered code-change table strictly decreases. */
/*! Writes the final code value for each change index. */
function bal_write_code_groups(account : item_index, count : item_count, cursor : item_index) -> unit =
    if cursor < count then {
        let index = bal_code_change_index(account, cursor);
        let next = bal_code_run_end(account, count, index, cursor);
        let last = bal_previous_index(next);
        bal_write_index_code(index, bal_code_change_hash(account, last));
        bal_write_code_groups(account, count, next)
    }
termination_measure bal_write_code_groups(account, count, cursor) = count - cursor

function bal_code_changes_size(account : item_index) -> bal_rlp_length =
    bal_code_groups_size(account, bal_code_change_count(account), 0, BAL_RLP_ZERO)

/*! Writes the code-change list for one account. */
function bal_write_code_changes(account : item_index, content_len : bal_rlp_length) -> unit = {
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(content_len));
    bal_write_code_groups(account, bal_code_change_count(account), 0)
}

/*! Sizes all BAL fields contributed by one account. */
function bal_account_size(account : item_index) -> BalAccountSize = {
    let storage_changes = bal_storage_changes_size(account);
    let storage_reads = bal_storage_reads_size(account);
    let balance_changes_len = bal_balance_changes_size(account);
    let nonce_changes_len = bal_nonce_changes_size(account);
    let code_changes_len = bal_code_changes_size(account);
    var content_len = bal_rlp_length_from_byte_length(rlp_addr_size());
    content_len = bal_rlp_length_add(
        content_len,
        bal_rlp_length_to_byte_length(bal_rlp_list_size(storage_changes.content_len)),
    );
    content_len = bal_rlp_length_add(
        content_len,
        bal_rlp_length_to_byte_length(bal_rlp_list_size(storage_reads.content_len)),
    );
    content_len = bal_rlp_length_add(content_len, bal_rlp_length_to_byte_length(bal_rlp_list_size(balance_changes_len)));
    content_len = bal_rlp_length_add(content_len, bal_rlp_length_to_byte_length(bal_rlp_list_size(nonce_changes_len)));
    content_len = bal_rlp_length_add(content_len, bal_rlp_length_to_byte_length(bal_rlp_list_size(code_changes_len)));
    struct {
        encoded_len = bal_rlp_list_size(content_len),
        item_count = bal_count_add(bal_count_add(1, storage_changes.count), storage_reads.count),
    }
}

/*! Writes one account and all of its ordered BAL change lists. */
function bal_write_account(account : item_index) -> unit = {
    let storage_changes = bal_storage_changes_size(account);
    let storage_reads = bal_storage_reads_size(account);
    let balance_changes_len = bal_balance_changes_size(account);
    let nonce_changes_len = bal_nonce_changes_size(account);
    let code_changes_len = bal_code_changes_size(account);
    var content_len = bal_rlp_length_from_byte_length(rlp_addr_size());
    content_len = bal_rlp_length_add(
        content_len,
        bal_rlp_length_to_byte_length(bal_rlp_list_size(storage_changes.content_len)),
    );
    content_len = bal_rlp_length_add(
        content_len,
        bal_rlp_length_to_byte_length(bal_rlp_list_size(storage_reads.content_len)),
    );
    content_len = bal_rlp_length_add(content_len, bal_rlp_length_to_byte_length(bal_rlp_list_size(balance_changes_len)));
    content_len = bal_rlp_length_add(content_len, bal_rlp_length_to_byte_length(bal_rlp_list_size(nonce_changes_len)));
    content_len = bal_rlp_length_add(content_len, bal_rlp_length_to_byte_length(bal_rlp_list_size(code_changes_len)));
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(content_len));
    rlp_write_addr(bal_account_address(account));
    bal_write_storage_changes(account, storage_changes);
    bal_write_storage_reads(account, storage_reads);
    bal_write_balance_changes(account, balance_changes_len);
    bal_write_nonce_changes(account, nonce_changes_len);
    bal_write_code_changes(account, code_changes_len)
}

/* The unconsumed ordered account table strictly decreases. */
/*! Sizes all ordered BAL accounts and counts their logical items. */
val bal_accounts_size : forall 'account, 0 <= 'account. (item_count, int('account), BalContentCount) -> BalContentCount
function bal_accounts_size(account_count, account, result) -> BalContentCount =
    if account < account_count then {
        let account_size = bal_account_size(account);
        let next_length = bal_rlp_length_add(
            result.content_len,
            bal_rlp_length_to_byte_length(account_size.encoded_len),
        );
        bal_accounts_size(
            account_count,
            account + 1,
            struct { content_len = next_length, count = bal_count_add(result.count, account_size.item_count) },
        )
    } else {
        result
    }
termination_measure bal_accounts_size(account_count, account, result) = account_count - account

/* The unconsumed ordered account table strictly decreases. */
/*! Writes all BAL accounts in recorder order. */
val bal_write_accounts : forall 'account, 0 <= 'account. (item_count, int('account)) -> unit
function bal_write_accounts(account_count, account) =
    if account < account_count then {
        bal_write_account(account);
        bal_write_accounts(account_count, account + 1)
    }
termination_measure bal_write_accounts(account_count, account) = account_count - account

/*! Reconstructs the canonical EIP-7928 block access list from the
    recorder: per account (address order) — storage changes, storage
    reads, balance, nonce, and code changes — counting items against the
    `gas_limit / 2000` bound. */
function encode_block_access_list() -> EncodedBlockAccessList = {
    bal_prepare();
    let account_count = bal_account_count();
    let size = bal_accounts_size(account_count, 0, struct { content_len = BAL_RLP_ZERO, count = 0 });
    let start = scratch_begin();
    rlp_write_list_prefix(bal_rlp_length_to_byte_length(size.content_len));
    bal_write_accounts(account_count, 0);
    struct { bytes = rlp_finish(start), item_count = size.count }
}
