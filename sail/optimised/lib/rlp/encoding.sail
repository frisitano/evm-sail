/* Optimized C overrides for `sail/lib/rlp/encoding.sail`. */

/* The widest protocol natural encoded through this path is Amsterdam's
 * cumulative receipt gas: two independently header-bounded gas dimensions.
 * It needs 65 bits in the maximal SSZ-header case, while still fitting
 * directly in the native u256 conversion used below. */
type rlp_natural_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 * (2 ^ 64 - 1)

type rlp_natural_increment_valid('value : Int) -> Bool =
    0 <= 'value & 'value < 2 ^ 64 - 1

type rlp_natural_size = range(1, 33)

/* The specialized backend currently cannot lower get_slice_int from a native
 * integer carrier to an 8-bit vector. Keep the canonical conversion in the
 * base model and cross that representation boundary through one pure native
 * primitive in optimized C. */
val rlp_length_byte_native = pure { c: "rlp_length_byte_native" } : range(0, 255) -> byte

/*! Appends the canonical RLP prefix of a byte-string payload. */
val rlp_write_string_prefix : forall 'len, 0 <= 'len. (int('len), byte) -> unit
function rlp_write_string_prefix(len, first) = {
    if (len == 1) & (first[7] == bitzero) then {
        ()
    } else if len <= RLP_SHORT_LENGTH_LIMIT then {
        scratch_push_byte(add_bits(0x80, rlp_length_byte_native(len)))
    } else {
        let length_word = rlp_length_word(len);
        let length_len = rlp_minimal_word_len(length_word);
        scratch_push_byte(add_bits(0xb7, rlp_length_byte_native(length_len)));
        scratch_push_word_be(length_word, length_len)
    }
}

/*! Appends the canonical RLP prefix of a list payload. */
val rlp_write_list_prefix : forall 'len, 0 <= 'len. int('len) -> unit
function rlp_write_list_prefix(content_len) = {
    if content_len <= RLP_SHORT_LENGTH_LIMIT then {
        scratch_push_byte(add_bits(0xc0, rlp_length_byte_native(content_len)))
    } else {
        let length_word = rlp_length_word(content_len);
        let length_len = rlp_minimal_word_len(length_word);
        scratch_push_byte(add_bits(0xf7, rlp_length_byte_native(length_len)));
        scratch_push_word_be(length_word, length_len)
    }
}
