function withdrawal_rlp(withdrawal : ByteSlice) -> ByteSlice = {
    let index = ssz_uint(withdrawal, WD_INDEX);
    let validator_index = ssz_uint(withdrawal, WD_VALIDATOR_INDEX);
    let address = sub_slice(withdrawal, WD_ADDRESS, ADDRESS_BYTE_LENGTH);
    let amount = ssz_uint(withdrawal, WD_AMOUNT);
    var content_len = rlp_uint_size(index);
    content_len = content_len + rlp_uint_size(validator_index);
    content_len = content_len + rlp_slice_size(address);
    content_len = content_len + rlp_uint_size(amount);
    let encoded_len = rlp_list_size(content_len);
    let start = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_uint(index);
    rlp_write_uint(validator_index);
    rlp_write_slice(address);
    rlp_write_uint(amount);
    rlp_finish(start, encoded_len)
}

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
    var content_len = rlp_word_size();
    content_len = content_len + rlp_word_size();
    content_len = content_len + rlp_addr_size();
    content_len = content_len + rlp_word_size();
    content_len = content_len + rlp_word_size();
    content_len = content_len + rlp_word_size();
    content_len = content_len + rlp_bytes_size(bloom, LOGS_BLOOM_BYTE_LENGTH);
    content_len = content_len + rlp_uint_size(0);
    content_len = content_len + rlp_uint_size(header.number);
    content_len = content_len + rlp_uint_size(header.gas_limit);
    content_len = content_len + rlp_uint_size(header.gas_used);
    content_len = content_len + rlp_uint_size(header.timestamp);
    content_len = content_len + rlp_slice_size(header.extra_data);
    content_len = content_len + rlp_word_size();
    content_len = content_len + rlp_bytes_size(nonce, EIGHT_BYTE_LENGTH);
    if k_fork >= London then {
        content_len = content_len + rlp_uint_word_size(header.base_fee)
    };
    if k_fork >= Shanghai then {
        content_len = content_len + rlp_word_size()
    };
    if k_fork >= Cancun then {
        content_len = content_len + rlp_uint_size(header.blob_gas_used);
        content_len = content_len + rlp_uint_size(header.excess_blob_gas);
        content_len = content_len + rlp_word_size()
    };
    if k_fork >= Prague then {
        content_len = content_len + rlp_word_size()
    };
    if k_fork >= Amsterdam then {
        content_len = content_len + rlp_word_size();
        content_len = content_len + rlp_uint_size(header.slot_number)
    };
    let encoded_len = rlp_list_size(content_len);
    let mark = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_word(header.parent_hash);
    rlp_write_word(EMPTY_OMMER_HASH);
    rlp_write_addr(header.fee_recipient);
    rlp_write_word(header.state_root);
    rlp_write_word(transactions_root);
    rlp_write_word(header.receipts_root);
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
        rlp_write_word(withdrawals_root)
    };
    if k_fork >= Cancun then {
        rlp_write_uint(header.blob_gas_used);
        rlp_write_uint(header.excess_blob_gas);
        rlp_write_word(header.parent_beacon_block_root)
    };
    if k_fork >= Prague then {
        rlp_write_word(requests_hash)
    };
    if k_fork >= Amsterdam then {
        rlp_write_word(block_access_list_hash);
        rlp_write_uint(header.slot_number)
    };
    let encoded = rlp_finish(mark, encoded_len);
    let block_hash = keccak256_slice(encoded);
    scratch_rewind(mark);
    block_hash
}

function transaction_trie_root(txs : SszListRef) -> hash = {
    var builder = trie_builder_empty();
    var cursor = rlp_index_cursor(txs.count);
    while not_bool(rlp_index_cursor_empty(cursor)) termination_measure(cursor.count - cursor.position) do {
        let (item, next) = rlp_index_cursor_pop(cursor);
        cursor = next;
        let tx = ssz_list_at(txs, item.index);
        builder = trie_insert_item(builder, item_leaf(item.key, tx), item.next_key)
    };
    trie_builder_root(builder)
}

function withdrawals_trie_root(wds : SszListRef) -> hash = {
    var builder = trie_builder_empty();
    var cursor = rlp_index_cursor(wds.count);
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

function expected_payload_excess_blob_gas(witness : WitnessContext) -> blob_gas =
    next_excess_blob_gas(witness.parent_excess_blob_gas, witness.parent_blob_gas_used, witness.parent_base_fee_per_gas)

/* EIP-7685 commitment over the present request-type digests, in request-type
 * order. The request bodies remain source-backed through the hash calls. */
function execution_requests_hash(input_ref : StatelessInputRef) -> hash = {
    let l0 = input_ref.deposits.len;
    let l1 = input_ref.withdrawal_requests.len;
    let l2 = input_ref.consolidation_requests.len;
    let d0 : hash = if l0 != BYTE_ZERO then {
            sha256_request_digest(0x00, input_ref.deposits)
        } else {
            sail_zeros(256)
        };
    let d1 : hash = if l1 != BYTE_ZERO then {
            sha256_request_digest(0x01, input_ref.withdrawal_requests)
        } else {
            sail_zeros(256)
        };
    let d2 : hash = if l2 != BYTE_ZERO then {
            sha256_request_digest(0x02, input_ref.consolidation_requests)
        } else {
            sail_zeros(256)
        };
    var segs : list(Bytes) = [||];
    if l2 != BYTE_ZERO then {
        segs = bytes_list(word_to_bytes32(d2), WORD_BYTE_LENGTH) :: segs
    };
    if l1 != BYTE_ZERO then {
        segs = bytes_list(word_to_bytes32(d1), WORD_BYTE_LENGTH) :: segs
    };
    if l0 != BYTE_ZERO then {
        segs = bytes_list(word_to_bytes32(d0), WORD_BYTE_LENGTH) :: segs
    };
    sha256(segs)
}

/* Validate commitments that can be checked before transaction decoding. The
 * supplied BAL is hashed once for the header; post-execution validation
 * compares its bytes directly with the canonical model encoding. */
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
    if not_bool(
          (9 <= input.chain_config.fork_index)
        & (input.chain_config.fork_index <= 20)
        & input.chain_config.activation_active
        & chain_config_blob_schedule_valid(input.chain_config),
    ) then {
        throw InvalidBlock(InvalidConfig)
    };
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
                ZERO_WORD
            };
        let block_access_list_hash = if k_fork >= Amsterdam then {
                keccak256(body.block_access_list)
            } else {
                ZERO_WORD
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
