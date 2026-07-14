/*md
# Receipts, blooms, and the receipts trie

The logs-bloom construction (YP §4.4.1) and the receipts trie
(EIP-2718 typed receipt encoding).
*/

/*! Sets one bit (0–2047) in the bloom, most-significant-byte first. */
function bloom_set_bit(bloom : LogsBloom, bit_to_set : int) -> LogsBloom = {
    var out = bloom;
    let bit = unsigned(get_slice_int(11, bit_to_set, 0));
    let byte_index = unsigned(get_slice_int(8, 255 - ediv_int(bit, 8), 0));
    let mask : byte = sail_shiftleft(0x01, emod_int(bit, 8));
    out[byte_index] = or_vec(out[byte_index], mask);
    out
}

/*! The `i`-th bloom bit index of a hash: bytes `2i, 2i+1` mod 2048
    (YP M3:2048). */
function bloom_bit(h : bits(256), i : int) -> int = {
    let j = i * 2;
    let hi : byte = sail_shiftright(h, 8 * (31 - j))[7 .. 0];
    let lo : byte = sail_shiftright(h, 8 * (30 - j))[7 .. 0];
    emod_int(unsigned(hi) * 256 + unsigned(lo), 2048)
}

/*! Adds a hashed bloom entry: three bits from its KECCAK-256. */
function bloom_add_entry_hash(bloom : LogsBloom, h : bits(256)) -> LogsBloom = {
    var out = bloom;
    foreach (i from 0 to 2) {
        out = bloom_set_bit(out, bloom_bit(h, i))
    };
    out
}

/*! Adds every topic of a log record to the bloom. */
function bloom_add_topics(bloom : LogsBloom, topics : list(word)) -> LogsBloom = {
    var out = bloom;
    var rest : list(word) = topics;
    var more : bool = true;
    while more do {
        match rest {
            topic :: tail => {
                out = bloom_add_entry_hash(out, keccak256_word(topic));
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function bloom_add_log(bloom : LogsBloom, log : LogEntry) -> LogsBloom = {
    let with_address = bloom_add_entry_hash(bloom, keccak256_address(log.address));
    bloom_add_topics(with_address, log.topics)
}

function logs_bloom_for_logs(logs : list(LogEntry)) -> LogsBloom = {
    var out = EMPTY_LOGS_BLOOM;
    var rest : list(LogEntry) = logs;
    var more : bool = true;
    while more do {
        match rest {
            log :: tail => {
                out = bloom_add_log(out, log);
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function receipts_bloom(receipts : list(Receipt)) -> LogsBloom = {
    var out = EMPTY_LOGS_BLOOM;
    var rest : list(Receipt) = receipts;
    var more : bool = true;
    while more do {
        match rest {
            receipt :: tail => {
                var logs : list(LogEntry) = receipt.logs;
                var more_logs : bool = true;
                while more_logs do {
                    match logs {
                        log :: log_tail => {
                            out = bloom_add_log(out, log);
                            logs = log_tail
                        },
                        [||] => more_logs = false,
                    }
                };
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function topics_rlp_payload(topics : list(word)) -> list(byte) =
    match topics {
        [||] => [||],
        topic :: rest => list_append(rlp_word(topic), topics_rlp_payload(rest)),
    }

function topics_rlp(topics : list(word)) -> list(byte) = rlp_list(topics_rlp_payload(topics))

function concat_rlp_segments_rev(segments : list(list(byte)), tail : list(byte)) -> list(byte) =
    match segments {
        [||] => tail,
        segment :: rest => concat_rlp_segments_rev(rest, list_append(segment, tail)),
    }

function log_entry_rlp(log : LogEntry) -> list(byte) =
    rlp_list(
        list_append(rlp_addr(log.address), list_append(topics_rlp(log.topics), rlp_bytes(read_byte_slice(log.data)))),
    )

function logs_rlp(logs : list(LogEntry)) -> list(byte) = {
    var encoded_rev : list(list(byte)) = [||];
    var rest : list(LogEntry) = logs;
    var more : bool = true;
    while more do {
        match rest {
            log :: tail => {
                encoded_rev = log_entry_rlp(log) :: encoded_rev;
                rest = tail
            },
            [||] => more = false,
        }
    };
    rlp_list(concat_rlp_segments_rev(encoded_rev, [||]))
}

function receipt_payload_rlp(r : Receipt, cumulative_gas_used : int) -> list(byte) =
    rlp_list(
        list_append(
            rlp_int(if r.success then { 1 } else { 0 }),
            list_append(
                rlp_int(cumulative_gas_used),
                list_append(rlp_bytes(logs_bloom_bytes(logs_bloom_for_logs(r.logs))), logs_rlp(r.logs)),
            ),
        ),
    )

function receipt_encoded(r : Receipt, cumulative_gas_used : int) -> list(byte) = {
    let payload = receipt_payload_rlp(r, cumulative_gas_used);
    if tx_type_byte(r.tx_type) == 0 then {
        payload
    } else {
        let type_byte : byte = get_slice_int(8, tx_type_byte(r.tx_type), 0);
        type_byte :: payload
    }
}

function receipts_trie_root(receipts : list(Receipt)) -> hash = {
    var updates_rev : list(TrieUpdate) = [||];
    var rest : list(Receipt) = receipts;
    var i : int = 0;
    var cumulative : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            r :: tail => {
                cumulative = cumulative + r.gas_used;
                updates_rev =
                    struct { key = trie_index_key(i), change = TriePut(BytesList(receipt_encoded(r, cumulative))) } ::
                        updates_rev;
                rest = tail;
                i = i + 1
            },
            [||] => more = false,
        }
    };
    trie_root(EMPTY_TRIE_ROOT, order_trie_index_updates(list_reverse(updates_rev)))
}
