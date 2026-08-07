/* Optimized C overrides for `sail/prelude.sail`. */

/*
 * Preserve the specification's canonical byte-index semantics while storing
 * addresses and digests in packed fixed-width 64-bit lanes. Transparent aliases are
 * erased before C lowering, so these annotations select the representation
 * structurally for 20-byte and 32-byte vectors in the optimized compilation.
 */
$[c_repr { representation = fixed_bytes_u64_lanes, name = bytes20 }]
type address = vector(20, inc, byte)

$[c_repr { representation = fixed_bytes_u64_lanes, name = bytes32 }]
type b256 = vector(32, inc, byte)

/*
 * Optimized refinements of the base prelude's direct endian equations.
 * Canonical fixed bytes and native least-significant-first word limbs cross
 * this boundary directly; the standard model and proof extractions retain the
 * readable vector conversions.
 */
val hash_to_word = pure { c: "hash_to_word" } : hash -> word

$[c_static_eval word_to_fixed_bytes]
val word_to_hash = pure { c: "word_to_hash" } : word -> hash

$[c_static_eval word_to_fixed_bytes]
val word_to_address = pure { c: "word_to_address" } : word -> address

val address_to_word = pure { c: "address_to_word" } : address -> word

/*
 * Sail's semantic count-leading-zeros primitive returns a dependent integer.
 * The generic C ABI therefore routes it through `sail_int`; the optimized
 * runtime selects the equivalent native instruction at this boundary.
 */
val u64_bit_length_native = pure { c: "u64_bit_length" } : range(0, 2 ^ 64 - 1) -> range(0, 64)

function u64_bit_length(value : range(0, 2 ^ 64 - 1)) -> range(0, 64) =
    u64_bit_length_native(value)
