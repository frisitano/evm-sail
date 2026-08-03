/* Optimized C overrides for `sail/executor/receipts.sail`. */

/*
 * The standard model retains execution-ordered, length-prefixed receipt bytes
 * in scratch and feeds them to the shared recursive indexed-trie equations.
 * The optimized build instead copies each temporary encoding into one
 * reusable C arena and stores its offset/length in execution order. Its
 * high-level root operation owns RLP(index) key generation, canonical trie
 * ordering, and MPT construction entirely in C.
 */
val optimized_receipt_table_reset = impure { c: "receipt_table_reset" } : unit -> unit
$[c_throws]
val optimized_receipt_table_push = impure { c: "receipt_table_push" } : (transaction_count, ScratchSlice) -> unit
$[c_throws]
val optimized_receipt_table_root = impure { c: "receipt_table_root" } : transaction_count -> hash
val optimized_block_logs_bloom_reset = impure { c: "block_logs_bloom_reset" } : unit -> unit
val optimized_block_logs_bloom = impure { c: "block_logs_bloom" } : unit -> LogsBloom

function receipt_accumulator_empty() -> ReceiptAccumulator = {
    optimized_receipt_table_reset();
    optimized_block_logs_bloom_reset();
    struct { records_start = 0, count = 0, bloom = EMPTY_LOGS_BLOOM }
}

function receipt_accumulator_push(acc, receipt, cumulative_gas_used, next_count) = {
    let mark = scratch_begin();
    let value = receipt_encoded(receipt, cumulative_gas_used, EMPTY_LOGS_BLOOM);
    optimized_receipt_table_push(acc.count, value);
    scratch_rewind(mark);
    struct { records_start = acc.records_start, count = next_count, bloom = acc.bloom }
}

function receipt_accumulator_root(acc : ReceiptAccumulator) -> hash =
    optimized_receipt_table_root(acc.count)

function receipt_accumulator_bloom(_acc : ReceiptAccumulator) -> LogsBloom =
    optimized_block_logs_bloom()
