
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

/*! The 32-byte big-endian encoding of a hash. */
function hash_to_bytes32(bytes : hash) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = bytes[k] :: out
    };
    out
}

/*! The 20-byte big-endian encoding of an address. */
function address_to_bytes(bytes : address) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = bytes[k] :: out
    };
    out
}
