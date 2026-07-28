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
    if len <= 2 ^ 64 - 1 - off then {
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
    let natural_byte : range(0, 255) = if quotient <= 255 then {
            quotient
        } else {
            assert(false);
            0
        };
    let remainder = tmod_int(bit_to_set, 8);
    let bit_in_byte : range(0, 7) = if remainder <= 7 then {
            remainder
        } else {
            assert(false);
            0
        };
    out[natural_byte] = or_vec(out[natural_byte], bloom_bit_mask(bit_in_byte));
    out
}

/*! Adds a hashed bloom entry: three bits from its KECCAK-256. */
function bloom_add_entry_hash(bloom : LogsBloom, h : hash) -> LogsBloom = {
    let bytes = h;
    var out = bloom_set_bit(bloom, unsigned(append(bytes[0][2 .. 0], bytes[1])));
    out = bloom_set_bit(out, unsigned(append(bytes[2][2 .. 0], bytes[3])));
    out = bloom_set_bit(out, unsigned(append(bytes[4][2 .. 0], bytes[5])));
    out
}

/*! Adds one retained log record to the bloom (YP §4.4.1, the M function). */
function bloom_add_log_at(bloom : LogsBloom, index : log_store_index) -> LogsBloom = {
    var out = bloom_add_entry_hash(bloom, keccak256_address(log_address(index)));
    var topic : log_store_index = 0;
    let topic_count = log_topics_count(index);
    while topic < topic_count termination_measure(topic_count - topic) do {
        out = bloom_add_entry_hash(out, keccak256_word(log_topic(index, topic)));
        topic = log_store_index_increment(topic)
    };
    out
}

/*! Adds a consecutive retained log range to a bloom. */
function bloom_add_logs(bloom : LogsBloom, logs : LogSeriesRef) -> LogsBloom = {
    var out = bloom;
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        out = bloom_add_log_at(out, log_store_index_add(logs.start, offset));
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
    if (protocol_profile_fork(k_protocol_profile) < Amsterdam) | word_is_zero(v) | (src == dst) then {
        ()
    } else {
        k_log(
            EIP7708_SYSTEM_ADDRESS,
            LogTopics3((EIP7708_TRANSFER_TOPIC, address_to_word(src), address_to_word(dst))),
            LogDataWord(v),
        )
    }
}

/*! Emits the EIP-7708 burn log when a selfdestruct deletion burns a
    nonzero balance (Amsterdam onward). */
function k_emit_burn_log(a : address, v : word) -> unit = {
    if (protocol_profile_fork(k_protocol_profile) < Amsterdam) | word_is_zero(v) then {
        ()
    } else {
        k_log(EIP7708_SYSTEM_ADDRESS, LogTopics2((EIP7708_BURN_TOPIC, address_to_word(a))), LogDataWord(v))
    }
}
