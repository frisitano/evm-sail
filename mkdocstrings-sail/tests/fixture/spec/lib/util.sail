default Order dec

$include <prelude.sail>

/*! The machine word type. */
type word = bits(8)

/*! Increment a `word` by one. */
val increment : word -> word

function increment(w) = add_bits(w, 0x01)

/*! Converts a flag bit to a boolean. */
mapping flag_bit : bool <-> bits(1) = {
    true <-> 0b1,
    false <-> 0b0
}
