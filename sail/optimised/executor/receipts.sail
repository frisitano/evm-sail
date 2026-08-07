/* Optimized C overrides for `sail/executor/receipts.sail`. */

/*
 * The standard model retains execution-ordered, length-prefixed receipt bytes
 * in scratch and feeds them to the shared recursive indexed-trie equations.
 * The optimized build instead copies each temporary encoding into one
 * reusable C arena and stores its offset/length in execution order. Its
 * high-level root operation owns RLP(index) key generation, canonical trie
 * ordering, and MPT construction entirely in C, and the block logs bloom is
 * accumulated and compared against the header commitment entirely in C, so
 * no Sail bloom value is ever materialized.
 */
val receipt_table_reset = impure { c: "receipt_table_reset" } : unit -> unit
val receipt_table_push = impure { c: "receipt_table_push" } : (transaction_count, ScratchSlice) -> unit
val receipt_table_root = impure { c: "receipt_table_root" } : transaction_count -> hash
val block_logs_bloom_matches_ref = impure { c: "block_logs_bloom_matches" } : LogsBloomRef -> bool

function receipt_store_begin() -> source_pointer = {
    receipt_table_reset();
    0
}

function receipt_store_append(receipt, cumulative_gas_used, index) = {
    let mark = scratch_begin();
    let value = receipt_encoded(receipt, cumulative_gas_used);
    receipt_table_push(index, value);
    scratch_rewind(mark)
}

function receipt_store_root(_records_start : source_pointer, count : transaction_count) -> hash =
    receipt_table_root(count)

function block_logs_bloom_matches(_logs : LogSeriesRef, reference : LogsBloomRef) -> bool =
    block_logs_bloom_matches_ref(reference)
