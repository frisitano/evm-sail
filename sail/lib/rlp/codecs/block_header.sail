/*md
# Block header RLP codec

Canonical execution-header encoding and hashing over already recomputed payload
commitments.
*/

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
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
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
    if profile.fork >= London then {
        let field_length = rlp_uint_word_size(header.base_fee);
        content_length = rlp_scratch_length_add(content_length, field_length)
    };
    if profile.fork >= Shanghai then {
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if profile.fork >= Cancun then {
        let blob_gas_used_length = rlp_uint_size(header.blob_gas_used);
        let excess_blob_gas_length = rlp_uint_size(header.excess_blob_gas);
        content_length = rlp_scratch_length_add(content_length, blob_gas_used_length);
        content_length = rlp_scratch_length_add(content_length, excess_blob_gas_length);
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if profile.fork >= Prague then {
        content_length = rlp_scratch_length_add(content_length, word_length)
    };
    if profile.fork >= Amsterdam then {
        let slot_number_length = rlp_uint_size(header.slot_number);
        content_length = rlp_scratch_length_add(content_length, word_length);
        content_length = rlp_scratch_length_add(content_length, slot_number_length)
    };

    /* Six fixed words plus the protocol-bounded variable fields give an
     * Amsterdam header-content maximum of 749 bytes. Keep that semantic
     * bound instead of reconstructing a generic backend byte length. */
    if 749 < content_length then {
        fatal_error(RlpDecode);
        exit(())
    };
    let bounded_content_length : range(0, 749) = tmod_nat(content_length, 750);
    let content_len = bounded_content_length;
    let encoder = rlp_encoder_begin(rlp_list_size(content_len));
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
    if profile.fork >= London then {
        rlp_write_uint_word(header.base_fee)
    };
    if profile.fork >= Shanghai then {
        rlp_write_word(hash_to_word(withdrawals_root))
    };

    if profile.fork >= Cancun then {
        rlp_write_uint(header.blob_gas_used);
        rlp_write_uint(header.excess_blob_gas);
        rlp_write_word(hash_to_word(header.parent_beacon_block_root))
    };
    if profile.fork >= Prague then {
        rlp_write_word(hash_to_word(requests_hash))
    };
    if profile.fork >= Amsterdam then {
        rlp_write_word(hash_to_word(block_access_list_hash));
        rlp_write_uint(header.slot_number)
    };
    let encoded = rlp_encoder_finish(encoder);
    let block_hash = keccak256(encoded);
    rlp_encoder_rewind(encoder);
    block_hash
}
