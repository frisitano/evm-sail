/*md
# Receipt accumulation and the receipts trie

Execution-ordered retention and block-wide receipt aggregates. Canonical wire
encoding is owned by the receipt RLP codec.
*/

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
    bloom : LogsBloom,
}

/*! Constructs an empty receipts-trie accumulator. */
function receipt_accumulator_empty() -> ReceiptAccumulator =
    struct { records_start = scratch_begin(), count = 0, bloom = EMPTY_LOGS_BLOOM }

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
val receipt_accumulator_push : forall 'cumulative, 0 <= 'cumulative & 'cumulative <= 2 * block_gas_limit_bound. (
        ReceiptAccumulator,
        Receipt,
        int('cumulative),
        transaction_count,
    ) -> ReceiptAccumulator
function receipt_accumulator_push(acc, receipt, cumulative_gas_used, next_count) = {
    if next_count != acc.count + 1 then {
        throw InvalidBlock(WitnessDeficient)
    };
    let receipt_bloom = logs_bloom_for_logs(receipt.logs);
    receipt_record_append(receipt, cumulative_gas_used, receipt_bloom);
    struct { records_start = acc.records_start, count = next_count, bloom = logs_bloom_or(acc.bloom, receipt_bloom) }
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
