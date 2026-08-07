/*md
# Payload commitments

Recursive transaction, withdrawal, and receipt tries plus the header-level
commitment checks over their recomputed roots.
*/

/*md
## The indexed tries

Transactions, withdrawals, and receipts all use `rlp(index)` keys. Their
values come from different source-backed regions, but their trie shape is the
same: recursively partition the canonical key-order cursor by nibble and use
the shared MPT child combiner. No Sail list, explicit frame stack, or
collection-sized vector is involved.
*/
/*! The closed source algebra for Ethereum's three index-keyed payload tries.
    [indexed_trie_begin][] and [indexed_trie_pop][] interpret it as count and
    value operations, allowing one recursive trie builder without storing
    function values or duplicating the traversal. */
union IndexedTrieSource = {
    /* the block body's transaction list */
    IndexedTransactions : TransactionListRef,
    /* the block body's withdrawal list */
    IndexedWithdrawals : WithdrawalListRef,
    /* the executed block's encoded receipt records */
    IndexedReceipts : ReceiptRecordsRef,
}

/*! Canonical key traversal plus the receipt-only record state. `receipt_zero`
    bridges the sole difference between numeric receipt storage order and
    lexical `rlp(index)` order; `receipt_remaining` otherwise advances once. */
struct IndexedTrieCursor = {
    keys : RlpIndexCursor(transaction_count_bound),
    receipt_zero : ScratchSlice,
    receipt_remaining : ScratchSlice,
}

/*! Opens the receipt record span while giving non-receipt sources empty
    placeholders. The explicit result type keeps both existential slices
    independent. */
function indexed_receipt_parts(source : IndexedTrieSource) -> (ScratchSlice, ScratchSlice) =
    match source {
        IndexedReceipts(receipts) => if receipts.count == 0 then {
            (EMPTY_SCRATCH_SLICE, receipts.bytes)
        } else {
            receipt_record_pop(receipts.bytes)
        },
        _ => (EMPTY_SCRATCH_SLICE, EMPTY_SCRATCH_SLICE),
    }

/*! Opens one source at its first canonical trie key. */
function indexed_trie_begin(source : IndexedTrieSource) -> IndexedTrieCursor = {
    let count : transaction_count = match source {
        IndexedTransactions(txs) => txs.count,
        IndexedWithdrawals(wds) => wds.count,
        IndexedReceipts(receipts) => receipts.count,
    };
    let (zero, remaining) = indexed_receipt_parts(source);
    struct { keys = rlp_index_cursor(count), receipt_zero = zero, receipt_remaining = remaining }
}

/*! Whether the source has no canonical keys left. */
function indexed_trie_cursor_empty(cursor : IndexedTrieCursor) -> bool =
    rlp_index_cursor_empty(cursor.keys)

/*! The next canonical key without consuming its value. */
function indexed_trie_cursor_key(cursor : IndexedTrieCursor) -> TriePath =
    rlp_index_cursor_peek(cursor.keys).key

/*! Whether the next indexed key lies beneath `prefix`. */
function indexed_trie_next_under(cursor : IndexedTrieCursor, prefix : TriePath) -> bool =
    let cursor_empty = indexed_trie_cursor_empty(cursor) in
    if cursor_empty then {
        false
    } else {
        let key = indexed_trie_cursor_key(cursor);
        path_prefix_of(prefix, key)
    }

/*! Consumes one canonical indexed key and resolves its source-backed value. */
function indexed_trie_pop(source : IndexedTrieSource, cursor : IndexedTrieCursor) -> (TrieItem, IndexedTrieCursor) = {
    let (index_item, next_keys) = rlp_index_cursor_pop(cursor.keys);
    var remaining = cursor.receipt_remaining;
    let value : TrieLeafValue = match source {
        IndexedTransactions(txs) => {
            let transaction = ssz_list_at(txs, index_item.index);
            InputTrieLeaf(transaction)
        },
        IndexedWithdrawals(wds) => {
            let withdrawal = ssz_fixed_list_at(wds, index_item.index, WD_SIZE);
            let encoded_withdrawal = withdrawal_rlp(withdrawal);
            ScratchTrieLeaf(encoded_withdrawal)
        },
        IndexedReceipts(_) => if index_item.index == 0 then {
            ScratchTrieLeaf(cursor.receipt_zero)
        } else {
            let (receipt, rest) = receipt_record_pop(remaining);
            remaining = rest;
            ScratchTrieLeaf(receipt)
        },
    };
    (
        trie_leaf(index_item.key, value),
        struct { keys = next_keys, receipt_zero = cursor.receipt_zero, receipt_remaining = remaining },
    )
}

/*! Recursively assembles the indexed leaves beneath `prefix`. */
function indexed_trie_subtree(
    source : IndexedTrieSource,
    cursor : IndexedTrieCursor,
    prefix : TriePath,
) -> (
    (TrieItem, IndexedTrieCursor)
) = {
    let cursor_under_prefix = indexed_trie_next_under(cursor, prefix);
    let outside_prefix = not_bool(cursor_under_prefix);
    if outside_prefix then {
        (trie_empty_subtree(), cursor)
    } else {
        let key = indexed_trie_cursor_key(cursor);
        let key_at_prefix = path_eq(key, prefix);
        if key_at_prefix then {
            indexed_trie_pop(source, cursor)
        } else {
            let depth = path_len(prefix);
            if 64 <= depth then {
                fatal_error(WitnessDeficient)
            };
            var children = trie_children_empty();
            var remaining = cursor;
            var remaining_under_prefix : bool = indexed_trie_next_under(remaining, prefix);
            while remaining_under_prefix termination_measure(remaining.keys.count - remaining.keys.position) do {
                let next_key = indexed_trie_cursor_key(remaining);
                let nib = path_nibble(next_key, depth);
                let nibble_path = path_single(nib);
                let child_prefix = path_concat(prefix, nibble_path);
                let (child, next) = indexed_trie_subtree(source, remaining, child_prefix);
                children = trie_children_add(children, prefix, nib, child);
                remaining = next;
                remaining_under_prefix = indexed_trie_next_under(remaining, prefix)
            };
            (trie_children_finish(prefix, children), remaining)
        }
    }
}
termination_measure indexed_trie_subtree(source, cursor, prefix) = 64 - path_len(prefix)

/*! Computes one complete index-keyed trie. Temporary withdrawal encodings
    and node encodings share one scratch suffix, released after the root has
    absorbed every leaf. Receipt records beneath the mark remain live until
    their accumulator releases them. */
function indexed_trie_root(source : IndexedTrieSource) -> hash = {
    let mark = scratch_begin();
    let initial = indexed_trie_begin(source);
    let root_path = path_empty();
    let (subtree, remaining) = indexed_trie_subtree(source, initial, root_path);
    let retained = remaining.receipt_remaining;
    let cursor_empty = indexed_trie_cursor_empty(remaining);
    let cursor_not_empty = not_bool(cursor_empty);
    if cursor_not_empty | retained.len != 0 then {
        fatal_error(WitnessDeficient)
    };
    let root = trie_subtree_root(subtree);
    scratch_rewind(mark);
    root
}

/*! The transactions-trie root (YP §4.4.2): leaf `i` holds the raw
    EIP-2718 envelope of transaction `i`, keyed by `rlp(i)`. */
function transaction_trie_root(txs : TransactionListRef) -> hash = {
    let source = IndexedTransactions(txs);
    indexed_trie_root(source)
}

/*! The withdrawals-trie root (EIP-4895): leaf `i` holds
    `rlp(withdrawal_i)`, keyed by `rlp(i)`. */
function withdrawals_trie_root(wds : WithdrawalListRef) -> hash = {
    let source = IndexedWithdrawals(wds);
    indexed_trie_root(source)
}

/*! The receipts-trie root over execution-ordered retained records. */
function indexed_receipt_trie_root(receipts : ReceiptRecordsRef) -> hash = {
    let source = IndexedReceipts(receipts);
    indexed_trie_root(source)
}

/*! The `excess_blob_gas` the header must carry, derived from the
    authenticated parent (EIP-4844). */
function expected_payload_excess_blob_gas(witness : WitnessContext) -> excess_blob_gas = {
    let execution_profile = k_execution_profile;
    next_excess_blob_gas(
        execution_profile.protocol,
        witness.parent_excess_blob_gas,
        witness.parent_blob_gas_used,
        witness.parent_base_fee_per_gas,
    )
}

/*! The EIP-7685 requests hash: `sha256` over the present request-type
    digests in request-type order; the request bodies remain
    region-backed through the hash calls. */
function execution_requests_hash(input_ref : StatelessInputRef) -> hash = {
    let deposits = input_ref.deposits;
    let withdrawal_requests = input_ref.withdrawal_requests;
    let consolidation_requests = input_ref.consolidation_requests;
    let builder_deposit_requests = input_ref.builder_deposit_requests;
    let builder_exit_requests = input_ref.builder_exit_requests;
    let d0 : hash =
        if deposits.len != 0 then sha256_request_digest(0x00, deposits) else ZERO_HASH;
    let d1 : hash =
        if withdrawal_requests.len != 0 then sha256_request_digest(0x01, withdrawal_requests) else ZERO_HASH;
    let d2 : hash =
        if consolidation_requests.len != 0 then sha256_request_digest(0x02, consolidation_requests) else ZERO_HASH;
    let d3 : hash =
        if builder_deposit_requests.len != 0 then sha256_request_digest(0x03, builder_deposit_requests) else ZERO_HASH;
    let d4 : hash =
        if builder_exit_requests.len != 0 then sha256_request_digest(0x04, builder_exit_requests) else ZERO_HASH;
    let mark = scratch_reserve(5 * WORD_BYTE_LENGTH);
    if deposits.len != 0 then {
        scratch_push_b256(d0, WORD_BYTE_LENGTH)
    };
    if withdrawal_requests.len != 0 then {
        scratch_push_b256(d1, WORD_BYTE_LENGTH)
    };
    if consolidation_requests.len != 0 then {
        scratch_push_b256(d2, WORD_BYTE_LENGTH)
    };
    if builder_deposit_requests.len != 0 then {
        scratch_push_b256(d3, WORD_BYTE_LENGTH)
    };
    if builder_exit_requests.len != 0 then {
        scratch_push_b256(d4, WORD_BYTE_LENGTH)
    };
    let request_bytes = scratch_finish(mark);
    let digest = sha256(request_bytes);
    scratch_rewind(mark);
    digest
}

/*! Validates every commitment checkable before transaction decoding:
    parent linkage, gas and blob-gas header rules, the transactions and
    withdrawals roots, the requests hash, and the block hash. The
    supplied block access list is hashed once for the header;
    post-execution validation compares its bytes against the canonical
    reconstruction. */
function validate_execution_payload(
    input : StatelessInput,
    input_ref : StatelessInputRef,
    witness : WitnessContext,
) -> (
    unit
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let payload = input.payload;
    let block = payload.block;
    let header = block.header;
    let body = block.body;
    if header.gas_limit < header.gas_used then {
        fatal_error(InvalidGasUsed)
    };
    if witness.parent_hash != header.parent_hash then {
        fatal_error(InvalidParentHash)
    };
    let expected_excess_blob_gas = expected_payload_excess_blob_gas(witness);
    if (profile.fork >= Cancun) & (header.excess_blob_gas != expected_excess_blob_gas) then {
        fatal_error(InvalidExcessBlobGas)
    };
    if profile.fork >= Paris then {
        let transactions_root = transaction_trie_root(body.transactions);
        let withdrawals_root =
            if profile.fork >= Shanghai then withdrawals_trie_root(body.withdrawals) else EMPTY_TRIE_ROOT;
        let requests_hash =
            if profile.fork >= Prague then execution_requests_hash(input_ref) else ZERO_HASH;
        let block_access_list_hash =
            if profile.fork >= Amsterdam then keccak256(body.block_access_list) else ZERO_HASH;
        let computed_block_hash = block_header_hash(
            header,
            transactions_root,
            withdrawals_root,
            requests_hash,
            block_access_list_hash,
        );
        if computed_block_hash != payload.expected_block_hash then {
            fatal_error(InvalidBlockHash)
        }
    }
}
