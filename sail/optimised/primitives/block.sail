/* Optimized C overrides for `sail/primitives/block.sail`. */

/* Keep the bloom's concrete optimized-C carrier name at the representation
 * boundary rather than deriving it from its Sail alias or vector width. */
$[c_repr {representation = fixed_bytes, name = bytes256}]
type LogsBloom = vector(256, dec, byte)

/*
 * The computed bloom uses the fixed decreasing-index representation while
 * the header retains canonical SSZ wire order. Compare those representations
 * directly at the source boundary without materializing the header bloom.
 */
val optimized_logs_bloom_matches_ref = pure { c: "logs_bloom_matches_ref" } : (LogsBloom, LogsBloomRef) -> bool

function logs_bloom_matches_ref(computed : LogsBloom, reference : LogsBloomRef) -> bool =
    optimized_logs_bloom_matches_ref(computed, reference)
