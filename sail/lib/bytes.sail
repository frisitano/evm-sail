
/*md
# Byte conversions

Big-endian conversions between words, addresses, and materialized byte
lists.
*/
/*! The 32-byte big-endian encoding of a word. */
function word_to_bytes32(value : word) -> list(byte) = {
    var remaining : word = value;
    var out : list(byte) = [||];
    foreach (byte_index from 0 to 31) {
        out = word_low_byte(remaining) :: out;
        remaining = word_shift_right(remaining, 8)
    };
    out
}
