let LOGS_BLOOM_BYTE_LENGTH : byte_length = ByteQuantity(256)

function bloom_bit_mask(bit_to_set : bloom_limb_bit) -> limb = {
    var mask : limb = LIMB_ZERO;
    mask[bit_to_set] = bitone;
    mask
}

function bloom_set_bit(bloom : LogsBloom, bit_to_set : bloom_bit_index) -> LogsBloom = {
    var out = bloom;
    let quotient = exact_quotient(bit_to_set, 64);
    let natural_limb : bloom_limb_index = if quotient <= 31 then {
            quotient
        } else {
            assert(false);
            0
        };
    let limb_index : bloom_limb_index = 31 - natural_limb;
    let remainder = tmod_int(bit_to_set, 64);
    let bit_in_limb : bloom_limb_bit = if remainder <= 63 then {
            remainder
        } else {
            assert(false);
            0
        };
    out[limb_index] = or_vec(out[limb_index], bloom_bit_mask(bit_in_limb));
    out
}

function bloom_add_entry_hash(bloom : LogsBloom, h : bits(256)) -> LogsBloom = {
    var out = bloom_set_bit(bloom, unsigned(h[250 .. 240]));
    out = bloom_set_bit(out, unsigned(h[234 .. 224]));
    out = bloom_set_bit(out, unsigned(h[218 .. 208]));
    out
}

function bloom_add_topics(bloom : LogsBloom, topics : list(word)) -> LogsBloom =
    match topics {
        [||] => bloom,
        topic :: rest => bloom_add_topics(bloom_add_entry_hash(bloom, keccak256_word(topic)), rest),
    }

function bloom_add_log(bloom : LogsBloom, log : LogEntry) -> LogsBloom = {
    let with_address = bloom_add_entry_hash(bloom, keccak256_address(log.address));
    bloom_add_topics(with_address, log.topics)
}

function bloom_add_logs(bloom : LogsBloom, logs : list(LogEntry)) -> LogsBloom =
    match logs {
        [||] => bloom,
        log :: rest => bloom_add_logs(bloom_add_log(bloom, log), rest),
    }

function logs_bloom_for_logs(logs : list(LogEntry)) -> LogsBloom = bloom_add_logs(EMPTY_LOGS_BLOOM, logs)

function topics_rlp_content_size(topics : list(word)) -> byte_length =
    match topics {
        [||] => BYTE_ZERO,
        _ :: rest => rlp_word_size() + topics_rlp_content_size(rest),
    }

function topics_rlp_size(topics : list(word)) -> byte_length =
    rlp_list_size(topics_rlp_content_size(topics))

function log_entry_rlp_content_size(log : LogEntry) -> byte_length = {
    var content_len = rlp_addr_size();
    content_len = content_len + topics_rlp_size(log.topics);
    content_len = content_len + rlp_slice_size(log.data);
    content_len
}

function log_entry_rlp_size(log : LogEntry) -> byte_length =
    rlp_list_size(log_entry_rlp_content_size(log))

function logs_rlp_content_size(logs : list(LogEntry)) -> byte_length =
    match logs {
        [||] => BYTE_ZERO,
        log :: rest => log_entry_rlp_size(log) + logs_rlp_content_size(rest),
    }

function logs_rlp_size(logs : list(LogEntry)) -> byte_length =
    rlp_list_size(logs_rlp_content_size(logs))

function rlp_write_topics_content(topics : list(word)) -> unit =
    match topics {
        [||] => (),
        topic :: rest => {
            rlp_write_word(topic);
            rlp_write_topics_content(rest)
        },
    }

function rlp_write_topics(topics : list(word)) -> unit = {
    rlp_write_list_prefix(topics_rlp_content_size(topics));
    rlp_write_topics_content(topics)
}

function rlp_write_log_entry(log : LogEntry) -> unit = {
    rlp_write_list_prefix(log_entry_rlp_content_size(log));
    rlp_write_addr(log.address);
    rlp_write_topics(log.topics);
    rlp_write_slice(log.data)
}

function rlp_write_logs_content(logs : list(LogEntry)) -> unit =
    match logs {
        [||] => (),
        log :: rest => {
            rlp_write_log_entry(log);
            rlp_write_logs_content(rest)
        },
    }

function rlp_write_logs(logs : list(LogEntry)) -> unit = {
    rlp_write_list_prefix(logs_rlp_content_size(logs));
    rlp_write_logs_content(logs)
}

function receipt_payload_content_size(r : Receipt, cumulative_gas_used : gas) -> byte_length = {
    let status : protocol_quantity = if r.success then { 1 } else { 0 };
    var content_len = rlp_uint_size(status);
    content_len = content_len + rlp_uint_size(cumulative_gas_used);
    content_len = content_len + rlp_string_size(LOGS_BLOOM_BYTE_LENGTH, 0x00);
    content_len = content_len + logs_rlp_size(r.logs);
    content_len
}

function receipt_encoded(r : Receipt, cumulative_gas_used : gas) -> ByteSlice = {
    let status : protocol_quantity = if r.success then { 1 } else { 0 };
    let bloom = logs_bloom_bytes(logs_bloom_for_logs(r.logs));
    let content_len = receipt_payload_content_size(r, cumulative_gas_used);
    let payload_len = rlp_list_size(content_len);
    let typed = tx_type_byte(r.tx_type) != 0x00;
    let encoded_len = if typed then {
            BYTE_ONE + payload_len
        } else {
            payload_len
        };
    let start = scratch_begin();
    if typed then {
        scratch_push_bytes([|tx_type_byte(r.tx_type)|], BYTE_ONE)
    };
    rlp_write_list_prefix(content_len);
    rlp_write_uint(status);
    rlp_write_uint(cumulative_gas_used);
    rlp_write_bytes(bloom, LOGS_BLOOM_BYTE_LENGTH);
    rlp_write_logs(r.logs);
    rlp_finish(start, encoded_len)
}

/* Receipt trie keys are RLP(transaction_index), whose lexical order is
 * 1..127, 0, 128.... Execution already produces receipts in numeric order.
 * Retaining receipt zero plus one pending receipt is therefore sufficient to
 * bridge the only discontinuity; every other receipt is encoded and consumed
 * when its successor arrives. */
struct PendingReceipt = {
    index : item_index,
    cumulative_gas_used : gas,
    receipt : Receipt,
}

struct ReceiptAccumulator = {
    builder : TrieBuilder,
    first : option(PendingReceipt),
    pending : option(PendingReceipt),
    count : item_count,
    cumulative_gas_used : gas,
    bloom : LogsBloom,
}

function receipt_accumulator_empty() -> ReceiptAccumulator =
    struct {
        builder = trie_builder_empty(),
        first = None(),
        pending = None(),
        count = 0,
        cumulative_gas_used = GAS_ZERO,
        bloom = EMPTY_LOGS_BLOOM,
    }

function receipt_insert(builder : TrieBuilder, pending : PendingReceipt, next_key : option(TriePath)) -> TrieBuilder = {
    let mark = scratch_begin();
    let value = receipt_encoded(pending.receipt, pending.cumulative_gas_used);
    let inserted = trie_insert_item(builder, item_leaf(trie_index_key(pending.index), value), next_key);
    scratch_rewind(mark);
    inserted
}

function receipt_accumulator_push(acc : ReceiptAccumulator, receipt : Receipt) -> ReceiptAccumulator = {
    if not_bool(receipt.valid) then {
        throw InvalidBlock(ExecutionInvalid)
    };
    let next_count : item_count = if acc.count < 2 ^ 64 - 1 then {
            protocol_quantity_increment(acc.count)
        } else {
            throw InvalidBlock(WitnessDeficient)
        };
    let Gas(cumulative_before) = acc.cumulative_gas_used;
    let Gas(receipt_gas) = receipt.gas_used;
    let cumulative : gas = if receipt_gas <= (2 ^ 63 - 1) - cumulative_before then {
            Gas(cumulative_before + receipt_gas)
        } else {
            throw InvalidBlock(ExecutionInvalid)
        };
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
