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
    let word_size = rlp_word_size();
    let encoded_word_length = rlp_scratch_small_length(word_size);
    while topic < topic_count termination_measure(topic_count - topic) do {
        size = rlp_scratch_length_add(size, encoded_word_length);
        topic = log_store_index_increment(topic)
    };
    size
}

function topics_rlp_size(index : log_store_index) -> rlp_scratch_length = {
    let content_size = topics_rlp_content_size(index);
    rlp_scratch_list_size(content_size)
}

/*! Sizes one retained log entry. */
function log_entry_rlp_content_size(index : log_store_index) -> rlp_scratch_length = {
    let address_size = rlp_addr_size();
    let address_length = rlp_scratch_small_length(address_size);
    let topics_length = topics_rlp_size(index);
    let data = read_log_data(index);
    let data_length = rlp_scratch_slice_size(data);
    let address_and_topics_length = rlp_scratch_length_add(address_length, topics_length);
    rlp_scratch_length_add(address_and_topics_length, data_length)
}

function log_entry_rlp_size(index : log_store_index) -> rlp_scratch_length = {
    let content_size = log_entry_rlp_content_size(index);
    rlp_scratch_list_size(content_size)
}

/*! Sizes the RLP content of a retained log range. */
function logs_rlp_content_size(logs : LogSeriesRef) -> rlp_scratch_length = {
    var size : rlp_scratch_length = 0;
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        let index = log_store_index_add(logs.start, offset);
        let entry_size = log_entry_rlp_size(index);
        size = rlp_scratch_length_add(size, entry_size);
        offset = log_store_index_increment(offset)
    };
    size
}

function logs_rlp_size(logs : LogSeriesRef) -> rlp_scratch_length = {
    let content_size = logs_rlp_content_size(logs);
    rlp_scratch_list_size(content_size)
}

/*! Writes one retained record's topic list. */
function rlp_write_topics(index : log_store_index) -> unit = {
    let content_size = topics_rlp_content_size(index);
    rlp_write_list_prefix(content_size);
    var topic : log_store_index = 0;
    let topic_count = log_topics_count(index);
    while topic < topic_count termination_measure(topic_count - topic) do {
        let value = log_topic(index, topic);
        rlp_write_word(value);
        topic = log_store_index_increment(topic)
    }
}

/*! Writes one canonical retained receipt log entry. */
function rlp_write_log_entry(index : log_store_index) -> unit = {
    let content_size = log_entry_rlp_content_size(index);
    rlp_write_list_prefix(content_size);
    let address = log_address(index);
    rlp_write_addr(address);
    rlp_write_topics(index);
    let data = read_log_data(index);
    rlp_write_slice(data)
}

/*! Writes the RLP list of a retained log range. */
function rlp_write_logs(logs : LogSeriesRef) -> unit = {
    let content_size = logs_rlp_content_size(logs);
    rlp_write_list_prefix(content_size);
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        let index = log_store_index_add(logs.start, offset);
        rlp_write_log_entry(index);
        offset = log_store_index_increment(offset)
    }
}

/*! Writes a bloom as its fixed-width RLP byte string without constructing a
    temporary Sail list. */
function rlp_write_logs_bloom(bloom : LogsBloom) -> unit = {
    rlp_write_string_prefix(LOGS_BLOOM_BYTE_LENGTH, 0x00);
    scratch_push_fixed_bytes_256(bloom)
}

/*! Writes the bloom belonging to one receipt, constructed from its retained
    log range (YP §4.4.1). Keeping this small semantic hook separate lets an
    optimized backend populate the already-reserved RLP span directly from its
    log-store accumulator without materializing a Sail bloom value. */
function receipt_write_logs_bloom(receipt : Receipt) -> unit = {
    let bloom = logs_bloom_for_logs(receipt.logs);
    rlp_write_logs_bloom(bloom)
}

/*! Sizes a receipt payload from status, gas, bloom, and logs. */
val receipt_payload_content_size : forall 'cumulative, receipt_cumulative_gas_value_valid('cumulative). (
        Receipt,
        int('cumulative),
    ) -> rlp_scratch_length
function receipt_payload_content_size(r, cumulative_gas_used) = {
    let status : range(0, 1) =
        if r.success then 1 else 0;
    let status_size = rlp_uint_size(status);
    let status_length = rlp_scratch_small_length(status_size);
    let gas_word = word_of_nat_byte_count(cumulative_gas_used);
    let gas_size = rlp_uint_word_size(gas_word);
    let gas_length = rlp_scratch_small_length(gas_size);
    let bloom_prefix_size = rlp_length_prefix_len(LOGS_BLOOM_BYTE_LENGTH);
    let bloom_prefix_length = rlp_scratch_small_length(bloom_prefix_size);
    let bloom_length = rlp_scratch_length_add(LOGS_BLOOM_BYTE_LENGTH, bloom_prefix_length);
    let logs_length = logs_rlp_size(r.logs);
    let fixed_length = rlp_scratch_length_add(status_length, gas_length);
    let fixed_and_bloom_length = rlp_scratch_length_add(fixed_length, bloom_length);
    rlp_scratch_length_add(fixed_and_bloom_length, logs_length)
}

/*! The receipt as stored in the receipts trie: the RLP payload,
    prefixed by the envelope type byte for typed transactions
    (EIP-2718). */
val receipt_encoded_length : forall 'cumulative, receipt_cumulative_gas_value_valid('cumulative). (
        Receipt,
        int('cumulative),
    ) -> rlp_scratch_length
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

/*! Writes one already-sized receipt at the current scratch cursor. */
val receipt_write_encoded : forall 'cumulative, receipt_cumulative_gas_value_valid('cumulative). (
        Receipt,
        int('cumulative),
    ) -> unit
function receipt_write_encoded(r, cumulative_gas_used) = {
    let status : range(0, 1) =
        if r.success then 1 else 0;
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let envelope_type : byte = tx_envelope_type(r.tx_type);
    let typed = envelope_type != 0x00;
    if typed then {
        scratch_push_byte(envelope_type)
    };
    rlp_write_list_prefix(content_len);
    rlp_write_uint(status);
    rlp_write_uint(cumulative_gas_used);
    receipt_write_logs_bloom(r);
    rlp_write_logs(r.logs)
}

val receipt_encoded : forall 'cumulative, receipt_cumulative_gas_value_valid('cumulative). (Receipt, int('cumulative)) -> ScratchSlice
function receipt_encoded(r, cumulative_gas_used) = {
    let encoded_len = receipt_encoded_length(r, cumulative_gas_used);
    let encoder = rlp_encoder_begin(encoded_len);
    receipt_write_encoded(r, cumulative_gas_used);
    rlp_encoder_finish(encoder)
}

/*! Appends an SSZ-style little-endian uint64 record length. */
val receipt_record_write_length : forall 'value, 0 <= 'value & 'value <= ssz_uint_bound. int('value) -> unit
function receipt_record_write_length(value) = {
    let byte_0 = get_slice_int(8, value, 0);
    scratch_push_byte(byte_0);
    let byte_1 = get_slice_int(8, value, 8);
    scratch_push_byte(byte_1);
    let byte_2 = get_slice_int(8, value, 16);
    scratch_push_byte(byte_2);
    let byte_3 = get_slice_int(8, value, 24);
    scratch_push_byte(byte_3);
    let byte_4 = get_slice_int(8, value, 32);
    scratch_push_byte(byte_4);
    let byte_5 = get_slice_int(8, value, 40);
    scratch_push_byte(byte_5);
    let byte_6 = get_slice_int(8, value, 48);
    scratch_push_byte(byte_6);
    let byte_7 = get_slice_int(8, value, 56);
    scratch_push_byte(byte_7)
}

/*! Appends one length-prefixed receipt record to the persistent scratch
    stream. The codec owns both the outer record reservation and the exact
    RLP-body reservation, so callers never manipulate encoding storage. */
val receipt_record_append : forall 'cumulative, receipt_cumulative_gas_value_valid('cumulative). (
        Receipt,
        int('cumulative),
    ) -> unit
function receipt_record_append(r, cumulative_gas_used) = {
    let encoded_len = receipt_encoded_length(r, cumulative_gas_used);
    if 2 ^ 64 - 1 < encoded_len then {
        fatal_error(RlpDecode)
    };
    let record_len = scratch_length_add(EIGHT_BYTE_LENGTH, encoded_len);
    let _record_start = scratch_reserve(record_len);
    receipt_record_write_length(encoded_len);
    let encoder = rlp_encoder_begin(encoded_len);
    receipt_write_encoded(r, cumulative_gas_used);
    let _encoded = rlp_encoder_finish(encoder);
    ()
}
