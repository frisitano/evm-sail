/*md
# Receipts, blooms, and the receipts trie

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
function receipt_payload_content_size(r : Receipt, cumulative_gas_used : block_gas) -> rlp_scratch_length = {
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
function receipt_encoded_length(r : Receipt, cumulative_gas_used : block_gas) -> rlp_scratch_length = {
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let typed = tx_type_byte(r.tx_type) != 0x00;
    let payload_len = rlp_scratch_list_size(content_len);
    if typed then {
        rlp_scratch_length_add(payload_len, 1)
    } else {
        payload_len
    }
}

/*! Writes one already-sized receipt and its precomputed bloom at the current
    scratch cursor. */
function receipt_write_encoded(r : Receipt, cumulative_gas_used : block_gas, bloom : LogsBloom) -> unit = {
    let status : range(0, 1) = if r.success then { 1 } else { 0 };
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let typed = tx_type_byte(r.tx_type) != 0x00;
    if typed then {
        scratch_push_byte(tx_type_byte(r.tx_type))
    };
    rlp_write_list_prefix(content_len);
    rlp_write_uint(status);
    rlp_write_uint(cumulative_gas_used);
    receipt_write_logs_bloom(r, bloom);
    rlp_write_logs(r.logs)
}

function receipt_encoded(r : Receipt, cumulative_gas_used : block_gas, bloom : LogsBloom) -> ScratchSlice = {
    let encoded_len = receipt_encoded_length(r, cumulative_gas_used);
    let start = scratch_reserve(encoded_len);
    receipt_write_encoded(r, cumulative_gas_used, bloom);
    rlp_finish(start)
}

/*! One execution-ordered sequence of length-prefixed encoded receipts in the
    scratch arena. The records contain no Sail list or aggregate receipt
    values: each push writes an eight-byte little-endian length followed by
    the canonical trie value. */
struct ReceiptRecordsRef = {
    bytes : ScratchSlice,
    count : transaction_count,
}

/*! Computes the recursive ordered-key trie over retained receipt records.
    Its definition lives with the transaction and withdrawal trie roots so
    all three indexed tries share one recursive construction. */
val indexed_receipt_trie_root : ReceiptRecordsRef -> hash

/*! The retained receipt-record region and block-wide receipt aggregates. */
struct ReceiptAccumulator = {
    records_start : source_pointer,
    count : transaction_count,
    cumulative_gas_used : block_gas,
    bloom : LogsBloom,
}

/*! Constructs an empty receipts-trie accumulator. */
function receipt_accumulator_empty() -> ReceiptAccumulator =
    struct { records_start = scratch_begin(), count = 0, cumulative_gas_used = 0, bloom = EMPTY_LOGS_BLOOM }

/*! Appends an SSZ-style little-endian uint64 record length. */
function receipt_record_write_length(value : ssz_uint) -> unit = {
    scratch_push_byte(get_slice_int(8, value, 0));
    scratch_push_byte(get_slice_int(8, value, 8));
    scratch_push_byte(get_slice_int(8, value, 16));
    scratch_push_byte(get_slice_int(8, value, 24));
    scratch_push_byte(get_slice_int(8, value, 32));
    scratch_push_byte(get_slice_int(8, value, 40));
    scratch_push_byte(get_slice_int(8, value, 48));
    scratch_push_byte(get_slice_int(8, value, 56))
}

/*! Splits the first retained record from an execution-ordered record span. */
function receipt_record_pop(records : ScratchSlice) -> (ScratchSlice, ScratchSlice) = {
    if records.len < EIGHT_BYTE_LENGTH then {
        throw InvalidBlock(WitnessDeficient)
    };
    let value_length = decode_scratch_uint(records, 0);
    let payload = slice_suffix(records, EIGHT_BYTE_LENGTH);
    if payload.len < value_length then {
        throw InvalidBlock(WitnessDeficient)
    };
    (sub_slice(payload, 0, value_length), slice_suffix(payload, value_length))
}

/*! Adds the next numeric receipt as one persistent scratch record. */
function receipt_accumulator_push(
    acc : ReceiptAccumulator,
    receipt : Receipt,
    next_count : transaction_count,
) -> (
    ReceiptAccumulator
) = {
    if next_count != acc.count + 1 then {
        throw InvalidBlock(WitnessDeficient)
    };
    let cumulative : block_gas = conserved_gas_add(acc.cumulative_gas_used, receipt.gas_used);
    let receipt_bloom = logs_bloom_for_logs(receipt.logs);
    let encoded_length = receipt_encoded_length(receipt, cumulative);
    if 2 ^ 64 - 1 < encoded_length then {
        throw InvalidBlock(RlpDecode)
    } else {
        let length : ssz_uint = tmod_nat(encoded_length, 2 ^ 64);
        let _record_start = scratch_reserve(scratch_length_add(EIGHT_BYTE_LENGTH, encoded_length));
        receipt_record_write_length(length);
        receipt_write_encoded(receipt, cumulative, receipt_bloom)
    };
    struct {
        records_start = acc.records_start,
        count = next_count,
        cumulative_gas_used = cumulative,
        bloom = logs_bloom_or(acc.bloom, receipt_bloom),
    }
}

/*! Computes the canonical receipts root, then releases the retained records. */
function receipt_accumulator_root(acc : ReceiptAccumulator) -> hash = {
    let records = scratch_finish(acc.records_start);
    let root = indexed_receipt_trie_root(struct { bytes = records, count = acc.count });
    scratch_rewind(acc.records_start);
    root
}

/*! Returns the block-wide logs bloom accumulated from all receipts. */
function receipt_accumulator_bloom(acc : ReceiptAccumulator) -> LogsBloom =
    acc.bloom
