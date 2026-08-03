/*md
# Receipt RLP codec

The logs-bloom construction (YP §4.4.1) and the receipts trie
(EIP-2718 typed receipt encoding).
*/

/*! Sizes the RLP content of one retained record's topic list. */
function topics_rlp_content_size(index : log_store_index) -> rlp_scratch_length = {
    var size : rlp_scratch_length = 0;
    var topic : log_store_index = 0;
    let topic_count = log_topics_count(index);
    while topic < topic_count termination_measure(topic_count - topic) do {
        size = rlp_scratch_length_add(size, rlp_scratch_small_length(rlp_word_size()));
        topic = log_store_index_increment(topic)
    };
    size
}

function topics_rlp_size(index : log_store_index) -> rlp_scratch_length =
    rlp_scratch_list_size(topics_rlp_content_size(index))

/*! Sizes one retained log entry. */
function log_entry_rlp_content_size(index : log_store_index) -> rlp_scratch_length = {
    let address_length = rlp_scratch_small_length(rlp_addr_size());
    let topics_length = topics_rlp_size(index);
    let data_length = rlp_scratch_slice_size(read_log_data(index));
    rlp_scratch_length_add(rlp_scratch_length_add(address_length, topics_length), data_length)
}

function log_entry_rlp_size(index : log_store_index) -> rlp_scratch_length =
    rlp_scratch_list_size(log_entry_rlp_content_size(index))

/*! Sizes the RLP content of a retained log range. */
function logs_rlp_content_size(logs : LogSeriesRef) -> rlp_scratch_length = {
    var size : rlp_scratch_length = 0;
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        size = rlp_scratch_length_add(size, log_entry_rlp_size(log_store_index_add(logs.start, offset)));
        offset = log_store_index_increment(offset)
    };
    size
}

function logs_rlp_size(logs : LogSeriesRef) -> rlp_scratch_length =
    rlp_scratch_list_size(logs_rlp_content_size(logs))

/*! Writes one retained record's topic list. */
function rlp_write_topics(index : log_store_index) -> unit = {
    rlp_write_list_prefix(topics_rlp_content_size(index));
    var topic : log_store_index = 0;
    let topic_count = log_topics_count(index);
    while topic < topic_count termination_measure(topic_count - topic) do {
        rlp_write_word(log_topic(index, topic));
        topic = log_store_index_increment(topic)
    }
}

/*! Writes one canonical retained receipt log entry. */
function rlp_write_log_entry(index : log_store_index) -> unit = {
    rlp_write_list_prefix(log_entry_rlp_content_size(index));
    rlp_write_addr(log_address(index));
    rlp_write_topics(index);
    rlp_write_slice(read_log_data(index))
}

/*! Writes the RLP list of a retained log range. */
function rlp_write_logs(logs : LogSeriesRef) -> unit = {
    rlp_write_list_prefix(logs_rlp_content_size(logs));
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        rlp_write_log_entry(log_store_index_add(logs.start, offset));
        offset = log_store_index_increment(offset)
    }
}

/*! Writes a bloom as its fixed-width RLP byte string without constructing a
    temporary Sail list. */
function rlp_write_logs_bloom(bloom : LogsBloom) -> unit = {
    rlp_write_string_prefix(LOGS_BLOOM_BYTE_LENGTH, 0x00);
    scratch_push_fixed_bytes_256(bloom)
}

/*! Writes the bloom belonging to one receipt. Keeping this small semantic
    hook separate lets an optimized backend populate the already-reserved RLP
    span directly from its log-store accumulator. */
function receipt_write_logs_bloom(_receipt : Receipt, bloom : LogsBloom) -> unit =
    rlp_write_logs_bloom(bloom)

/*! Sizes a receipt payload from status, gas, bloom, and logs. */
val receipt_payload_content_size : forall 'cumulative,
    receipt_cumulative_gas_valid('cumulative).
    (Receipt, int('cumulative)) -> rlp_scratch_length
function receipt_payload_content_size(r, cumulative_gas_used) = {
    let status : range(0, 1) = if r.success then { 1 } else { 0 };
    let status_length = rlp_scratch_small_length(rlp_uint_size(status));
    let gas_word = word_of_nat_byte_count(cumulative_gas_used);
    let gas_length = rlp_scratch_small_length(rlp_uint_word_size(gas_word));
    let bloom_length = rlp_scratch_length_add(
        LOGS_BLOOM_BYTE_LENGTH,
        rlp_scratch_small_length(rlp_length_prefix_len(LOGS_BLOOM_BYTE_LENGTH)),
    );
    let logs_length = logs_rlp_size(r.logs);
    let fixed_length = rlp_scratch_length_add(status_length, gas_length);
    rlp_scratch_length_add(rlp_scratch_length_add(fixed_length, bloom_length), logs_length)
}

/*! The receipt as stored in the receipts trie: the RLP payload,
    prefixed by the envelope type byte for typed transactions
    (EIP-2718). */
val receipt_encoded_length : forall 'cumulative,
    receipt_cumulative_gas_valid('cumulative).
    (Receipt, int('cumulative)) -> rlp_scratch_length
function receipt_encoded_length(r, cumulative_gas_used) = {
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let envelope_type : byte = tx_envelope_type(r.tx_type);
    let typed = envelope_type != 0x00;
    let payload_len = rlp_scratch_list_size(content_len);
    if typed then {
        rlp_scratch_length_add(payload_len, 1)
    } else {
        payload_len
    }
}

/*! Writes one already-sized receipt and its precomputed bloom at the current
    scratch cursor. */
val receipt_write_encoded : forall 'cumulative,
    receipt_cumulative_gas_valid('cumulative).
    (Receipt, int('cumulative), LogsBloom) -> unit
function receipt_write_encoded(r, cumulative_gas_used, bloom) = {
    let status : range(0, 1) = if r.success then { 1 } else { 0 };
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let envelope_type : byte = tx_envelope_type(r.tx_type);
    let typed = envelope_type != 0x00;
    if typed then {
        scratch_push_byte(envelope_type)
    };
    rlp_write_list_prefix(content_len);
    rlp_write_uint(status);
    rlp_write_uint(cumulative_gas_used);
    receipt_write_logs_bloom(r, bloom);
    rlp_write_logs(r.logs)
}

val receipt_encoded : forall 'cumulative,
    receipt_cumulative_gas_valid('cumulative).
    (Receipt, int('cumulative), LogsBloom) -> ScratchSlice
function receipt_encoded(r, cumulative_gas_used, bloom) = {
    let encoded_len = receipt_encoded_length(r, cumulative_gas_used);
    let encoder = rlp_encoder_begin(encoded_len);
    receipt_write_encoded(r, cumulative_gas_used, bloom);
    rlp_encoder_finish(encoder)
}

/*! Appends an SSZ-style little-endian uint64 record length. */
val receipt_record_write_length : forall 'value,
    0 <= 'value & 'value <= ssz_uint_bound.
    int('value) -> unit
function receipt_record_write_length(value) = {
    scratch_push_byte(get_slice_int(8, value, 0));
    scratch_push_byte(get_slice_int(8, value, 8));
    scratch_push_byte(get_slice_int(8, value, 16));
    scratch_push_byte(get_slice_int(8, value, 24));
    scratch_push_byte(get_slice_int(8, value, 32));
    scratch_push_byte(get_slice_int(8, value, 40));
    scratch_push_byte(get_slice_int(8, value, 48));
    scratch_push_byte(get_slice_int(8, value, 56))
}

/*! Appends one length-prefixed receipt record to the persistent scratch
    stream. The codec owns both the outer record reservation and the exact
    RLP-body reservation, so callers never manipulate encoding storage. */
val receipt_record_append : forall 'cumulative,
    receipt_cumulative_gas_valid('cumulative).
    (Receipt, int('cumulative), LogsBloom) -> unit
function receipt_record_append(r, cumulative_gas_used, bloom) = {
    let encoded_len = receipt_encoded_length(r, cumulative_gas_used);
    if 2 ^ 64 - 1 < encoded_len then {
        throw InvalidBlock(RlpDecode)
    };
    let record_len = scratch_length_add(EIGHT_BYTE_LENGTH, encoded_len);
    let _record_start = scratch_reserve(record_len);
    receipt_record_write_length(encoded_len);
    let encoder = rlp_encoder_begin(encoded_len);
    receipt_write_encoded(r, cumulative_gas_used, bloom);
    let _encoded = rlp_encoder_finish(encoder);
    ()
}
