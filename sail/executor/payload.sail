/*md
# Payload commitments

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself.
*/

/*! The RLP of one withdrawal (EIP-4895), assembled in the scratch
    arena. */
function withdrawal_rlp(withdrawal : StatelessInputSliceLength(44)) -> ScratchSlice = {
    let index = decode_ssz_uint(withdrawal, WD_INDEX);
    let validator_index = decode_ssz_uint(withdrawal, WD_VALIDATOR_INDEX);
    let address = sub_slice(withdrawal, WD_ADDRESS, ADDRESS_BYTE_LENGTH);
    let amount = decode_ssz_uint(withdrawal, WD_AMOUNT);
    let index_length = rlp_uint_size(index);
    let validator_index_length = rlp_uint_size(validator_index);
    let address_length = rlp_slice_size(address);
    let amount_length = rlp_uint_size(amount);
    let content_length = index_length + validator_index_length + address_length + amount_length;
    if 48 < content_length then {
        throw InvalidBlock(RlpDecode)
    };
    let bounded_content_length : range(0, 48) = tmod_nat(content_length, 49);
    let content_len = bounded_content_length;
    let start = scratch_reserve(rlp_list_size(content_len));
    rlp_write_list_prefix(content_len);
    rlp_write_uint(index);
    rlp_write_uint(validator_index);
    rlp_write_slice(address);
    rlp_write_uint(amount);
    rlp_finish(start)
}

/*md
## Recursive indexed tries

Transactions, withdrawals, and receipts all use `rlp(index)` keys. Their
values come from different source-backed regions, but their trie shape is the
same: recursively partition the canonical key-order cursor by nibble and use
the shared MPT child combiner. No Sail list, explicit frame stack, or
collection-sized vector is involved.
*/
/*! The three value sources committed by Ethereum's index-keyed payload
    tries. */
union IndexedTrieSource = {
    IndexedTransactions : TransactionListRef,
    IndexedWithdrawals : WithdrawalListRef,
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
    if indexed_trie_cursor_empty(cursor) then {
        false
    } else {
        path_prefix_of(prefix, indexed_trie_cursor_key(cursor))
    }

/*! Consumes one canonical indexed key and resolves its source-backed value. */
function indexed_trie_pop(source : IndexedTrieSource, cursor : IndexedTrieCursor) -> (TrieItem, IndexedTrieCursor) = {
    let (index_item, next_keys) = rlp_index_cursor_pop(cursor.keys);
    var remaining = cursor.receipt_remaining;
    let value : TrieLeafValue = match source {
        IndexedTransactions(txs) => InputTrieLeaf(ssz_list_at(txs, index_item.index)),
        IndexedWithdrawals(wds) => {
            let withdrawal = ssz_fixed_list_at(wds, index_item.index, WD_SIZE);
            ScratchTrieLeaf(withdrawal_rlp(withdrawal))
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
        item_leaf(index_item.key, value),
        struct { keys = next_keys, receipt_zero = cursor.receipt_zero, receipt_remaining = remaining },
    )
}

/*! Recursively assembles the indexed leaves beneath `prefix`. */
function indexed_trie_subtree(
    source : IndexedTrieSource,
    cursor : IndexedTrieCursor,
    prefix : TriePath,
) -> (
    (option(TrieItem), IndexedTrieCursor)
) = {
    if not_bool(indexed_trie_next_under(cursor, prefix)) then {
        (None(), cursor)
    } else {
        let key = indexed_trie_cursor_key(cursor);
        if path_eq(key, prefix) then {
            let (leaf, next) = indexed_trie_pop(source, cursor);
            (Some(leaf), next)
        } else {
            let depth = path_len(prefix);
            if 64 <= depth then {
                throw InvalidBlock(WitnessDeficient)
            };
            var children = trie_children_empty();
            var remaining = cursor;
            while indexed_trie_next_under(remaining, prefix) termination_measure(
                remaining.keys.count - remaining.keys.position
            ) do {
                let next_key = indexed_trie_cursor_key(remaining);
                let nib = path_nibble(next_key, depth);
                let child_prefix = path_concat(prefix, path_single(nib));
                let (child, next) = indexed_trie_subtree(source, remaining, child_prefix);
                children = trie_children_add(children, prefix, nib, child);
                remaining = next
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
    let (subtree, remaining) = indexed_trie_subtree(source, initial, path_empty());
    let retained = remaining.receipt_remaining;
    if not_bool(indexed_trie_cursor_empty(remaining)) | retained.len != 0 then {
        throw InvalidBlock(WitnessDeficient)
    };
    let root = trie_subtree_root(subtree);
    scratch_rewind(mark);
    root
}

/*! The block header hash: `keccak256(rlp(header))` with the recomputed
    body roots spliced in (YP §4.4; post-merge constants for ommers,
    difficulty, and nonce). */
function block_header_hash(
    header : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    block_access_list_hash : hash,
) -> (
    hash
) = {
    let word_length = rlp_word_size();
    let address_length = rlp_addr_size();
    let bloom_length = 3 + LOGS_BLOOM_BYTE_LENGTH;
    let difficulty_length = rlp_uint_size(0);
    let number_length = rlp_uint_size(header.number);
    let gas_limit_length = rlp_uint_size(header.gas_limit);
    let gas_used_length = rlp_uint_size(header.gas_used);
    let timestamp_length = rlp_uint_size(header.timestamp);
    let extra_data_length = rlp_scratch_slice_size(header.extra_data);
    let nonce_length = 1 + EIGHT_BYTE_LENGTH;
    var content_length : rlp_scratch_length = rlp_scratch_length_add(6 * word_length, address_length);
    content_length = rlp_scratch_length_add(content_length, bloom_length);
    content_length = rlp_scratch_length_add(content_length, difficulty_length);
    content_length = rlp_scratch_length_add(content_length, number_length);
    content_length = rlp_scratch_length_add(content_length, gas_limit_length);
    content_length = rlp_scratch_length_add(content_length, gas_used_length);
    content_length = rlp_scratch_length_add(content_length, timestamp_length);
    content_length = rlp_scratch_length_add(content_length, extra_data_length);
    content_length = rlp_scratch_length_add(content_length, nonce_length);
    if protocol_profile_fork(k_protocol_profile) >= London then {
        let field_length = rlp_uint_word_size(header.base_fee);
        content_length = rlp_scratch_length_add(content_length, field_length)
    };
    if protocol_profile_fork(k_protocol_profile) >= Shanghai then {
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if protocol_profile_fork(k_protocol_profile) >= Cancun then {
        let blob_gas_used_length = rlp_uint_size(header.blob_gas_used);
        let excess_blob_gas_length = rlp_uint_size(header.excess_blob_gas);
        content_length = rlp_scratch_length_add(content_length, blob_gas_used_length);
        content_length = rlp_scratch_length_add(content_length, excess_blob_gas_length);
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if protocol_profile_fork(k_protocol_profile) >= Prague then {
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if protocol_profile_fork(k_protocol_profile) >= Amsterdam then {
        let slot_number_length = rlp_uint_size(header.slot_number);
        content_length = rlp_scratch_length_add(content_length, word_length);
        content_length = rlp_scratch_length_add(content_length, slot_number_length)
    };

    /* Six fixed words plus the protocol-bounded variable fields give an
     * Amsterdam header-content maximum of 749 bytes. Keep that semantic
     * bound instead of reconstructing a generic backend byte length. */
    if 749 < content_length then {
        throw InvalidBlock(RlpDecode)
    };
    let bounded_content_length : range(0, 749) = tmod_nat(content_length, 750);
    let content_len = bounded_content_length;
    let mark = scratch_reserve(rlp_list_size(content_len));
    rlp_write_list_prefix(content_len);
    rlp_write_word(hash_to_word(header.parent_hash));
    rlp_write_word(hash_to_word(EMPTY_OMMER_HASH));
    rlp_write_addr(header.fee_recipient);
    rlp_write_word(hash_to_word(header.state_root));
    rlp_write_word(hash_to_word(transactions_root));
    rlp_write_word(hash_to_word(header.receipts_root));
    rlp_write_logs_bloom(logs_bloom_from_ref(header.logs_bloom));
    rlp_write_uint(0);
    rlp_write_uint(header.number);
    rlp_write_uint(header.gas_limit);
    rlp_write_uint(header.gas_used);
    rlp_write_uint(header.timestamp);
    rlp_write_slice(header.extra_data);
    rlp_write_word(header.prev_randao);
    rlp_write_string_prefix(EIGHT_BYTE_LENGTH, 0x00);
    scratch_push_word_be(ZERO_WORD, EIGHT_BYTE_LENGTH);
    if protocol_profile_fork(k_protocol_profile) >= London then {
        rlp_write_uint_word(header.base_fee)
    };
    if protocol_profile_fork(k_protocol_profile) >= Shanghai then {
        rlp_write_word(hash_to_word(withdrawals_root))
    };

    if protocol_profile_fork(k_protocol_profile) >= Cancun then {
        rlp_write_uint(header.blob_gas_used);
        rlp_write_uint(header.excess_blob_gas);
        rlp_write_word(hash_to_word(header.parent_beacon_block_root))
    };
    if protocol_profile_fork(k_protocol_profile) >= Prague then {
        rlp_write_word(hash_to_word(requests_hash))
    };
    if protocol_profile_fork(k_protocol_profile) >= Amsterdam then {
        rlp_write_word(hash_to_word(block_access_list_hash));
        rlp_write_uint(header.slot_number)
    };
    let encoded = rlp_finish(mark);
    let block_hash = keccak256(encoded);
    scratch_rewind(mark);
    block_hash
}

/*! The transactions-trie root (YP §4.4.2): leaf `i` holds the raw
    EIP-2718 envelope of transaction `i`, keyed by `rlp(i)`. */
function transaction_trie_root(txs : TransactionListRef) -> hash = {
    indexed_trie_root(IndexedTransactions(txs))
}

/*! The withdrawals-trie root (EIP-4895): leaf `i` holds
    `rlp(withdrawal_i)`, keyed by `rlp(i)`. */
function withdrawals_trie_root(wds : WithdrawalListRef) -> hash = {
    indexed_trie_root(IndexedWithdrawals(wds))
}

/*! The receipts-trie root over execution-ordered retained records. */
function indexed_receipt_trie_root(receipts : ReceiptRecordsRef) -> hash =
    indexed_trie_root(IndexedReceipts(receipts))

/*! The `excess_blob_gas` the header must carry, derived from the
    authenticated parent (EIP-4844). */
function expected_payload_excess_blob_gas(witness : WitnessContext) -> excess_blob_gas =
    next_excess_blob_gas(witness.parent_excess_blob_gas, witness.parent_blob_gas_used, witness.parent_base_fee_per_gas)

/*! The EIP-7685 requests hash: `sha256` over the present request-type
    digests in request-type order; the request bodies remain
    region-backed through the hash calls. */
function execution_requests_hash(input_ref : StatelessInputRef) -> hash = {
    let deposits = input_ref.deposits;
    let withdrawal_requests = input_ref.withdrawal_requests;
    let consolidation_requests = input_ref.consolidation_requests;
    let builder_deposit_requests = input_ref.builder_deposit_requests;
    let builder_exit_requests = input_ref.builder_exit_requests;
    let d0 : hash = if deposits.len != 0 then {
            sha256_request_digest(0x00, deposits)
        } else {
            ZERO_HASH
        };
    let d1 : hash = if withdrawal_requests.len != 0 then {
            sha256_request_digest(0x01, withdrawal_requests)
        } else {
            ZERO_HASH
        };
    let d2 : hash = if consolidation_requests.len != 0 then {
            sha256_request_digest(0x02, consolidation_requests)
        } else {
            ZERO_HASH
        };
    let d3 : hash = if builder_deposit_requests.len != 0 then {
            sha256_request_digest(0x03, builder_deposit_requests)
        } else {
            ZERO_HASH
        };
    let d4 : hash = if builder_exit_requests.len != 0 then {
            sha256_request_digest(0x04, builder_exit_requests)
        } else {
            ZERO_HASH
        };
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
    let digest = sha256(scratch_finish(mark));
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
    let payload = input.payload;
    let block = payload.block;
    let header = block.header;
    let body = block.body;
    if header.gas_limit < header.gas_used then {
        throw InvalidBlock(InvalidGasUsed)
    };
    if witness.parent_hash != header.parent_hash then {
        throw InvalidBlock(InvalidParentHash)
    };
    if (protocol_profile_fork(k_protocol_profile) >= Cancun) & (header.excess_blob_gas != expected_payload_excess_blob_gas(witness)) then {
        throw InvalidBlock(InvalidExcessBlobGas)
    };
    if protocol_profile_fork(k_protocol_profile) >= Paris then {
        let transactions_root = transaction_trie_root(body.transactions);
        let withdrawals_root = if protocol_profile_fork(k_protocol_profile) >= Shanghai then {
                withdrawals_trie_root(body.withdrawals)
            } else {
                EMPTY_TRIE_ROOT
            };
        let requests_hash = if protocol_profile_fork(k_protocol_profile) >= Prague then {
                execution_requests_hash(input_ref)
            } else {
                ZERO_HASH
            };
        let block_access_list_hash = if protocol_profile_fork(k_protocol_profile) >= Amsterdam then {
                keccak256(body.block_access_list)
            } else {
                ZERO_HASH
            };
        let computed_block_hash = block_header_hash(
            header,
            transactions_root,
            withdrawals_root,
            requests_hash,
            block_access_list_hash,
        );
        if computed_block_hash != payload.expected_block_hash then {
            throw InvalidBlock(InvalidBlockHash)
        }
    }
}
