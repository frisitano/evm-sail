/*md
# Receipts, blooms, and the receipts trie

The logs-bloom construction (YP §4.4.1) and the receipts trie
(EIP-2718 typed receipt encoding).
*/

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

/*! Adds every topic of a log record to the bloom. */
function bloom_add_topics(bloom : LogsBloom, topics : list(word)) -> LogsBloom =
    match topics {
        [||] => bloom,
        topic :: rest => bloom_add_topics(bloom_add_entry_hash(bloom, keccak256_word(topic)), rest),
    }

termination_measure bloom_add_topics(bloom, topics) = 2 ^ 64

/*! Adds a log record to the bloom: its address and every topic
    (YP §4.4.1, the M function). */
function bloom_add_log(bloom : LogsBloom, log : LogEntry) -> LogsBloom = {
    let with_address = bloom_add_entry_hash(bloom, keccak256_address(log.address));
    bloom_add_topics(with_address, log.topics)
}

/*! Adds a sequence of log records to a bloom. */
function bloom_add_logs(bloom : LogsBloom, logs : list(LogEntry)) -> LogsBloom =
    match logs {
        [||] => bloom,
        log :: rest => bloom_add_logs(bloom_add_log(bloom, log), rest),
    }

/*! The bloom of one receipt's log series. */
function logs_bloom_for_logs(logs : list(LogEntry)) -> LogsBloom = bloom_add_logs(EMPTY_LOGS_BLOOM, logs)

/*! Sizes the RLP content of a topic list. */
function topics_rlp_content_size(topics : list(word)) -> rlp_scratch_length =
    match topics {
        [||] => 0,
        _ :: rest => {
            let topic_length = rlp_scratch_small_length(rlp_word_size());
            let rest_length = topics_rlp_content_size(rest);
            rlp_scratch_length_add(topic_length, rest_length)
        },
    }

termination_measure topics_rlp_content_size(topics) = 2 ^ 64

function topics_rlp_size(topics : list(word)) -> rlp_scratch_length =
    rlp_scratch_list_size(topics_rlp_content_size(topics))

/*! Sizes the RLP content of one log entry. */
function log_entry_rlp_content_size(log : LogEntry) -> rlp_scratch_length = {
    let address_length = rlp_scratch_small_length(rlp_addr_size());
    let topics_length = topics_rlp_size(log.topics);
    let data_length = rlp_scratch_slice_size(log.data);
    rlp_scratch_length_add(rlp_scratch_length_add(address_length, topics_length), data_length)
}

function log_entry_rlp_size(log : LogEntry) -> rlp_scratch_length =
    rlp_scratch_list_size(log_entry_rlp_content_size(log))

/*! Sizes the RLP content of a log-entry list. */
function logs_rlp_content_size(logs : list(LogEntry)) -> rlp_scratch_length =
    match logs {
        [||] => 0,
        log :: rest => {
            let log_length = log_entry_rlp_size(log);
            let rest_length = logs_rlp_content_size(rest);
            rlp_scratch_length_add(log_length, rest_length)
        },
    }

function logs_rlp_size(logs : list(LogEntry)) -> rlp_scratch_length =
    rlp_scratch_list_size(logs_rlp_content_size(logs))

/*! Writes topic words as the content of an RLP list. */
function rlp_write_topics_content(topics : list(word)) -> unit =
    match topics {
        [||] => (),
        topic :: rest => {
            rlp_write_word(topic);
            rlp_write_topics_content(rest)
        },
    }

termination_measure rlp_write_topics_content(topics) = 2 ^ 64

/*! Writes an RLP list of topic words. */
function rlp_write_topics(topics : list(word)) -> unit = {
    rlp_write_list_prefix(topics_rlp_content_size(topics));
    rlp_write_topics_content(topics)
}

/*! Writes one canonical receipt log entry. */
function rlp_write_log_entry(log : LogEntry) -> unit = {
    rlp_write_list_prefix(log_entry_rlp_content_size(log));
    rlp_write_addr(log.address);
    rlp_write_topics(log.topics);
    rlp_write_slice(log.data)
}

/*! Writes log entries as the content of an RLP list. */
function rlp_write_logs_content(logs : list(LogEntry)) -> unit =
    match logs {
        [||] => (),
        log :: rest => {
            rlp_write_log_entry(log);
            rlp_write_logs_content(rest)
        },
    }

/*! Writes the RLP list of receipt logs. */
function rlp_write_logs(logs : list(LogEntry)) -> unit = {
    rlp_write_list_prefix(logs_rlp_content_size(logs));
    rlp_write_logs_content(logs)
}

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
function receipt_encoded(r : Receipt, cumulative_gas_used : block_gas) -> ByteSlice = {
    let status : range(0, 1) = if r.success then { 1 } else { 0 };
    let bloom = logs_bloom_bytes(logs_bloom_for_logs(r.logs));
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let typed = tx_type_byte(r.tx_type) != 0x00;
    let start = scratch_begin();
    if typed then {
        scratch_push_bytes([|tx_type_byte(r.tx_type)|], 1)
    };
    rlp_write_list_prefix(content_len);
    rlp_write_uint(status);
    rlp_write_uint(cumulative_gas_used);
    rlp_write_bytes(bloom, LOGS_BLOOM_BYTE_LENGTH);
    rlp_write_logs(r.logs);
    rlp_finish(start)
}

/* Receipt trie keys are RLP(transaction_index), whose lexical order is
 * 1..127, 0, 128.... Execution already produces receipts in numeric order.
 * Retaining receipt zero plus one pending receipt is therefore sufficient to
 * bridge the only discontinuity; every other receipt is encoded and consumed
 * when its successor arrives. */
/*! A receipt retained until its lexicographic trie successor is known. */
struct PendingReceipt = {
    index : transaction_count,
    cumulative_gas_used : block_gas,
    receipt : Receipt,
}

/*! The streaming receipts-trie builder and block-wide receipt aggregates. */
struct ReceiptAccumulator = {
    builder : TrieBuilder,
    first : option(PendingReceipt),
    pending : option(PendingReceipt),
    count : transaction_count,
    cumulative_gas_used : block_gas,
    bloom : LogsBloom,
}

/*! Constructs an empty receipts-trie accumulator. */
function receipt_accumulator_empty() -> ReceiptAccumulator =
    struct {
        builder = trie_builder_empty(),
        first = None(),
        pending = None(),
        count = 0,
        cumulative_gas_used = 0,
        bloom = EMPTY_LOGS_BLOOM,
    }

/*! Encodes and inserts one pending receipt with its next lexical key. */
function receipt_insert(builder : TrieBuilder, pending : PendingReceipt, next_key : option(TriePath)) -> TrieBuilder = {
    let mark = scratch_begin();
    let value = receipt_encoded(pending.receipt, pending.cumulative_gas_used);
    let inserted = trie_insert_item(builder, item_leaf(trie_index_key(pending.index), value), next_key);
    scratch_rewind(mark);
    inserted
}

/*! Adds the next numeric receipt while respecting trie-key lexical order. */
function receipt_accumulator_push(
    acc : ReceiptAccumulator,
    receipt : Receipt,
    next_count : transaction_count,
) -> (
    ReceiptAccumulator
) = {
    let cumulative : block_gas = conserved_gas_add(acc.cumulative_gas_used, receipt.gas_used);
    let current : PendingReceipt = struct { index = acc.count, cumulative_gas_used = cumulative, receipt = receipt };
    var builder = acc.builder;
    var first = acc.first;
    var pending = acc.pending;

    if acc.count == 0 then {
        first = Some(current)
    } else {
        if acc.count == 128 then {
            match pending {
                Some(previous) => builder = receipt_insert(builder, previous, Some(trie_index_key(0))),
                None() => throw InvalidBlock(WitnessDeficient),
            };
            match first {
                Some(zero) => builder = receipt_insert(builder, zero, Some(trie_index_key(acc.count))),
                None() => throw InvalidBlock(WitnessDeficient),
            };
            first = None();
            pending = Some(current)
        } else {
            match pending {
                Some(previous) => {
                    builder = receipt_insert(builder, previous, Some(trie_index_key(acc.count)));
                    pending = Some(current)
                },
                None() => if acc.count == 1 then {
                    pending = Some(current)
                } else {
                    throw InvalidBlock(WitnessDeficient)
                },
            }
        }
    };
    struct {
        builder = builder,
        first = first,
        pending = pending,
        count = next_count,
        cumulative_gas_used = cumulative,
        bloom = bloom_add_logs(acc.bloom, receipt.logs),
    }
}

/*! Flushes retained receipts and returns the canonical receipts root. */
function receipt_accumulator_root(acc : ReceiptAccumulator) -> hash = {
    var builder = acc.builder;
    if acc.count == 0 then {
        ()
    } else {
        if acc.count == 1 then {
            match acc.first {
                Some(zero) => builder = receipt_insert(builder, zero, None()),
                None() => throw InvalidBlock(WitnessDeficient),
            }
        } else {
            if acc.count <= 128 then {
                match acc.pending {
                    Some(previous) => builder = receipt_insert(builder, previous, Some(trie_index_key(0))),
                    None() => throw InvalidBlock(WitnessDeficient),
                };
                match acc.first {
                    Some(zero) => builder = receipt_insert(builder, zero, None()),
                    None() => throw InvalidBlock(WitnessDeficient),
                }
            } else {
                match acc.pending {
                    Some(previous) => builder = receipt_insert(builder, previous, None()),
                    None() => throw InvalidBlock(WitnessDeficient),
                };
                match acc.first {
                    Some(_) => throw InvalidBlock(WitnessDeficient),
                    None() => (),
                }
            }
        }
    };
    trie_builder_root(builder)
}
