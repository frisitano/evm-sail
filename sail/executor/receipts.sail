/*md
# Receipt retention and the receipts trie

Execution-ordered retention of encoded receipts and the post-execution
block-wide receipt reductions. Canonical wire encoding is owned by the
receipt RLP codec.
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

/*! Opens the execution-ordered receipt record region for one block. */
function receipt_store_begin() -> source_pointer =
    scratch_begin()

/*! Splits the first retained record from an execution-ordered record span. */
function receipt_record_pop(records : ScratchSlice) -> (ScratchSlice, ScratchSlice) = {
    let records : ScratchSliceAtLeast(8) =
        if EIGHT_BYTE_LENGTH <= records.len then records else fatal_error(WitnessDeficient);
    let value_length = decode_scratch_uint(records, 0);
    let payload = slice_suffix(records, EIGHT_BYTE_LENGTH);
    if value_length <= payload.len then {
        (sub_slice(payload, 0, value_length), slice_suffix(payload, value_length))
    } else {
        fatal_error(WitnessDeficient)
    }
}

/*! Retains the next numeric receipt as one persistent scratch record. The
    zero-based `index` restates the receipt's execution order for backends
    that store records outside the scratch stream. */
val receipt_store_append : forall 'cumulative, 0 <= 'cumulative & 'cumulative <= 2 * block_gas_limit_bound. (
        Receipt,
        int('cumulative),
        transaction_count,
    ) -> unit
function receipt_store_append(receipt, cumulative_gas_used, _index) =
    receipt_record_append(receipt, cumulative_gas_used)

/*! Computes the canonical receipts root over the retained records after the
    last transaction has executed, then releases the retained region. */
function receipt_store_root(records_start : source_pointer, count : transaction_count) -> hash = {
    let records = scratch_finish(records_start);
    let root = indexed_receipt_trie_root(struct { bytes = records, count = count });
    scratch_rewind(records_start);
    root
}

/*! Compares the block logs bloom of one consecutive retained log range with
    the payload-header commitment (YP §4.4.1). The range covers exactly the
    logs retained by the block's transaction receipts. */
function block_logs_bloom_matches(logs : LogSeriesRef, reference : LogsBloomRef) -> bool = {
    let logs_bloom = bloom_add_logs(EMPTY_LOGS_BLOOM, logs);
    logs_bloom_matches_ref(logs_bloom, reference)
}
