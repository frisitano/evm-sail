/*md
# State: logs and refunds

Log emission (YP §4.4.1) — including the EIP-7708 transfer and burn
logs — and the gas-refund counter.
*/

/*! Appends bounded topic operands without constructing a Sail list. */
function k_log_topics(topics : LogTopics) -> unit = match topics {
    LogTopics0() => (),
    LogTopics1(t0) => log_add_topic(t0),
    LogTopics2((t0, t1)) => {
        log_add_topic(t0);
        log_add_topic(t1)
    },
    LogTopics3((t0, t1, t2)) => {
        log_add_topic(t0);
        log_add_topic(t1);
        log_add_topic(t2)
    },
    LogTopics4((t0, t1, t2, t3)) => {
        log_add_topic(t0);
        log_add_topic(t1);
        log_add_topic(t2);
        log_add_topic(t3)
    },
}

/*! Appends a log payload without crossing an aggregate/list glue boundary. */
function k_log_data(data : LogData) -> unit = match data {
    LogDataMemory(bytes) => log_add_data_memory(bytes),
    LogDataWord(value) => log_add_data_word(value),
}

/*! Appends a log record (YP §4.4.1) to the transaction's log series. */
function k_log(a : address, topics : LogTopics, data : LogData) -> unit = {
    log_begin(a);
    k_log_topics(topics);
    k_log_data(data)
}

/*! Captures the current transaction's consecutive retained log range. */
function read_logs() -> LogSeriesRef =
    struct { start = logs_tx_start(), count = logs_tx_count() }

/*! Returns the source-backed payload of one retained log record. */
function read_log_data(index : log_store_index) -> LogDataSlice = {
    let off = log_data_offset(index);
    let len = log_data_length(index);
    if len <= sizeof(log_data_region_bound) - off then {
        log_data_slice(off, len)
    } else {
        assert(false, "log data slice overflow");
        EMPTY_LOG_DATA_SLICE
    }
}

let LOGS_BLOOM_BYTE_LENGTH : int(256) = 256

/*! Constructs a one-hot mask for a bit within a bloom byte. */
function bloom_bit_mask(bit_to_set : range(0, 7)) -> byte =
    sail_shiftleft(0x01, bit_to_set)

/*! Sets one bit (0–2047) in the bloom, most-significant-byte first. */
function bloom_set_bit(bloom : LogsBloom, bit_to_set : bloom_bit_index) -> LogsBloom = {
    var out = bloom;
    let quotient = bit_to_set / 8;
    let natural_byte : range(0, 255) =
        if quotient <= 255 then {
            quotient
        } else {
            assert(false);
            0
        };
    let remainder = tmod_int(bit_to_set, 8);
    let bit_in_byte : range(0, 7) =
        if remainder <= 7 then {
            remainder
        } else {
            assert(false);
            0
        };
    let mask = bloom_bit_mask(bit_in_byte);
    out[natural_byte] = or_vec(out[natural_byte], mask);
    out
}

/*! Adds a hashed bloom entry: three bits from its KECCAK-256. */
function bloom_add_entry_hash(bloom : LogsBloom, h : hash) -> LogsBloom = {
    let bytes = h;
    let first_bits = append(bytes[0][2 .. 0], bytes[1]);
    let first_index = unsigned(first_bits);
    var out = bloom_set_bit(bloom, first_index);
    let second_bits = append(bytes[2][2 .. 0], bytes[3]);
    let second_index = unsigned(second_bits);
    out = bloom_set_bit(out, second_index);
    let third_bits = append(bytes[4][2 .. 0], bytes[5]);
    let third_index = unsigned(third_bits);
    out = bloom_set_bit(out, third_index);
    out
}

/*! Adds one retained log record to the bloom (YP §4.4.1, the M function). */
function bloom_add_log_at(bloom : LogsBloom, index : log_store_index) -> LogsBloom = {
    let address = log_address(index);
    let address_hash = keccak256_address(address);
    var out = bloom_add_entry_hash(bloom, address_hash);
    var topic : log_store_index = 0;
    let topic_count = log_topics_count(index);
    while topic < topic_count termination_measure(topic_count - topic) do {
        let topic_value = log_topic(index, topic);
        let topic_hash = keccak256_word(topic_value);
        out = bloom_add_entry_hash(out, topic_hash);
        topic = log_store_index_increment(topic)
    };
    out
}

/*! Adds a consecutive retained log range to a bloom. */
function bloom_add_logs(bloom : LogsBloom, logs : LogSeriesRef) -> LogsBloom = {
    var out = bloom;
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        let index = log_store_index_add(logs.start, offset);
        out = bloom_add_log_at(out, index);
        offset = log_store_index_increment(offset)
    };
    out
}

/*! The bloom of one receipt's retained log range. */
function logs_bloom_for_logs(logs : LogSeriesRef) -> LogsBloom =
    bloom_add_logs(EMPTY_LOGS_BLOOM, logs)

/*! Combines two receipt blooms for the block header. */
function logs_bloom_or(left : LogsBloom, right : LogsBloom) -> LogsBloom = {
    var out = left;
    foreach (k from 0 to 255) {
        out[k] = or_vec(out[k], right[k])
    };
    out
}

/*! Emits the EIP-7708 transfer log for a nonzero, non-self value
    transfer (Amsterdam onward). */
function k_emit_transfer_log(src : address, dst : address, v : word) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let value_is_zero = word_is_zero(v);
    if (profile.fork < Amsterdam) | value_is_zero | (src == dst) then {
        return ()
    };
    let source = address_to_word(src);
    let destination = address_to_word(dst);
    let topics = LogTopics3((EIP7708_TRANSFER_TOPIC, source, destination));
    let data = LogDataWord(v);
    k_log(EIP7708_SYSTEM_ADDRESS, topics, data)
}

/*! Emits the EIP-7708 burn log when a selfdestruct deletion burns a
    nonzero balance (Amsterdam onward). */
function k_emit_burn_log(a : address, v : word) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let value_is_zero = word_is_zero(v);
    if (profile.fork < Amsterdam) | value_is_zero then {
        return ()
    };
    let address = address_to_word(a);
    let topics = LogTopics2((EIP7708_BURN_TOPIC, address));
    let data = LogDataWord(v);
    k_log(EIP7708_SYSTEM_ADDRESS, topics, data)
}
