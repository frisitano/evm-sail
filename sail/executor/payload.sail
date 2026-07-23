/*md
# Payload commitments

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself.
*/

/*! The RLP of one withdrawal (EIP-4895), assembled in the scratch
    arena. */
function withdrawal_rlp(withdrawal : ByteSliceLength(44)) -> ByteSlice = {
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
    let start = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_uint(index);
    rlp_write_uint(validator_index);
    rlp_write_slice(address);
    rlp_write_uint(amount);
    rlp_finish(start)
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
    let bloom = logs_bloom_bytes(header.logs_bloom);
    let nonce : list(byte) = [|0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00|];
    let word_length = rlp_word_size();
    let address_length = rlp_addr_size();
    let bloom_length = rlp_scratch_bytes_size(bloom, LOGS_BLOOM_BYTE_LENGTH);
    let difficulty_length = rlp_uint_size(0);
    let number_length = rlp_uint_size(header.number);
    let gas_limit_length = rlp_uint_size(header.gas_limit);
    let gas_used_length = rlp_uint_size(header.gas_used);
    let timestamp_length = rlp_uint_size(header.timestamp);
    let extra_data_length = rlp_scratch_slice_size(header.extra_data);
    let nonce_length = rlp_scratch_bytes_size(nonce, EIGHT_BYTE_LENGTH);
    var content_length : rlp_scratch_length = rlp_scratch_length_add(6 * word_length, address_length);
    content_length = rlp_scratch_length_add(content_length, bloom_length);
    content_length = rlp_scratch_length_add(content_length, difficulty_length);
    content_length = rlp_scratch_length_add(content_length, number_length);
    content_length = rlp_scratch_length_add(content_length, gas_limit_length);
    content_length = rlp_scratch_length_add(content_length, gas_used_length);
    content_length = rlp_scratch_length_add(content_length, timestamp_length);
    content_length = rlp_scratch_length_add(content_length, extra_data_length);
    content_length = rlp_scratch_length_add(content_length, nonce_length);
    if k_fork >= London then {
        let field_length = rlp_uint_word_size(header.base_fee);
        content_length = rlp_scratch_length_add(content_length, field_length)
    };
    if k_fork >= Shanghai then {
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if k_fork >= Cancun then {
        let blob_gas_used_length = rlp_uint_size(header.blob_gas_used);
        let excess_blob_gas_length = rlp_uint_size(header.excess_blob_gas);
        content_length = rlp_scratch_length_add(content_length, blob_gas_used_length);
        content_length = rlp_scratch_length_add(content_length, excess_blob_gas_length);
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if k_fork >= Prague then {
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if k_fork >= Amsterdam then {
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
    let mark = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_word(hash_to_word(header.parent_hash));
    rlp_write_word(hash_to_word(EMPTY_OMMER_HASH));
    rlp_write_addr(header.fee_recipient);
    rlp_write_word(hash_to_word(header.state_root));
    rlp_write_word(hash_to_word(transactions_root));
    rlp_write_word(hash_to_word(header.receipts_root));
    rlp_write_bytes(bloom, LOGS_BLOOM_BYTE_LENGTH);
    rlp_write_uint(0);
    rlp_write_uint(header.number);
    rlp_write_uint(header.gas_limit);
    rlp_write_uint(header.gas_used);
    rlp_write_uint(header.timestamp);
    rlp_write_slice(header.extra_data);
    rlp_write_word(header.prev_randao);
    rlp_write_bytes(nonce, EIGHT_BYTE_LENGTH);
    if k_fork >= London then {
        rlp_write_uint_word(header.base_fee)
    };
    if k_fork >= Shanghai then {
        rlp_write_word(hash_to_word(withdrawals_root))
    };

    if k_fork >= Cancun then {
        rlp_write_uint(header.blob_gas_used);
        rlp_write_uint(header.excess_blob_gas);
        rlp_write_word(hash_to_word(header.parent_beacon_block_root))
    };
    if k_fork >= Prague then {
        rlp_write_word(hash_to_word(requests_hash))
    };
    if k_fork >= Amsterdam then {
        rlp_write_word(hash_to_word(block_access_list_hash));
        rlp_write_uint(header.slot_number)
    };
    let encoded = rlp_finish(mark);
    let block_hash = keccak256_slice(encoded);
    scratch_rewind(mark);
    block_hash
}

/*! The transactions-trie root (YP §4.4.2): leaf `i` holds the raw
    EIP-2718 envelope of transaction `i`, keyed by `rlp(i)`. */
function transaction_trie_root(txs : TransactionListRef) -> hash = {
    var builder = trie_builder_empty();
    var cursor : RlpIndexCursor(transaction_count_bound) = rlp_index_cursor(txs.count);
    while not_bool(rlp_index_cursor_empty(cursor)) termination_measure(cursor.count - cursor.position) do {
        let (item, next) = rlp_index_cursor_pop(cursor);
        cursor = next;
        let tx = ssz_list_at(txs, item.index);
        builder = trie_insert_item(builder, item_leaf(item.key, tx), item.next_key)
    };
    trie_builder_root(builder)
}

/*! The withdrawals-trie root (EIP-4895): leaf `i` holds
    `rlp(withdrawal_i)`, keyed by `rlp(i)`. */
function withdrawals_trie_root(wds : WithdrawalListRef) -> hash = {
    var builder = trie_builder_empty();
    var cursor : RlpIndexCursor(withdrawal_count_bound) = rlp_index_cursor(wds.count);
    while not_bool(rlp_index_cursor_empty(cursor)) termination_measure(cursor.count - cursor.position) do {
        let (item, next) = rlp_index_cursor_pop(cursor);
        cursor = next;
        let scratch_mark = scratch_begin();
        let withdrawal = ssz_fixed_list_at(wds, item.index, WD_SIZE);
        let value = withdrawal_rlp(withdrawal);
        builder = trie_insert_item(builder, item_leaf(item.key, value), item.next_key);
        scratch_rewind(scratch_mark)
    };
    trie_builder_root(builder)
}

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
    var segs : list(Bytes) = [||];
    if builder_exit_requests.len != 0 then {
        segs = bytes_list(hash_to_bytes32(d4), WORD_BYTE_LENGTH) :: segs
    };
    if builder_deposit_requests.len != 0 then {
        segs = bytes_list(hash_to_bytes32(d3), WORD_BYTE_LENGTH) :: segs
    };
    if consolidation_requests.len != 0 then {
        segs = bytes_list(hash_to_bytes32(d2), WORD_BYTE_LENGTH) :: segs
    };
    if withdrawal_requests.len != 0 then {
        segs = bytes_list(hash_to_bytes32(d1), WORD_BYTE_LENGTH) :: segs
    };
    if deposits.len != 0 then {
        segs = bytes_list(hash_to_bytes32(d0), WORD_BYTE_LENGTH) :: segs
    };
    sha256(segs)
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
    if (k_fork >= Cancun) & (header.excess_blob_gas != expected_payload_excess_blob_gas(witness)) then {
        throw InvalidBlock(InvalidExcessBlobGas)
    };
    if k_fork >= Paris then {
        let transactions_root = transaction_trie_root(body.transactions);
        let withdrawals_root = if k_fork >= Shanghai then {
                withdrawals_trie_root(body.withdrawals)
            } else {
                EMPTY_TRIE_ROOT
            };
        let requests_hash = if k_fork >= Prague then {
                execution_requests_hash(input_ref)
            } else {
                ZERO_HASH
            };
        let block_access_list_hash = if k_fork >= Amsterdam then {
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
