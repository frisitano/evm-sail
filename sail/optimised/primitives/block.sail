/* Optimized C overrides for `sail/primitives/block.sail`. */

/*
 * The computed bloom uses the fixed decreasing-index representation while
 * the header retains canonical SSZ wire order. Compare those representations
 * directly at the source boundary without materializing the header bloom.
 */
val optimized_logs_bloom_matches_ref = pure { c: "logs_bloom_matches_ref" } : (LogsBloom, LogsBloomRef) -> bool

function logs_bloom_matches_ref(computed : LogsBloom, reference : LogsBloomRef) -> bool =
    optimized_logs_bloom_matches_ref(computed, reference)
