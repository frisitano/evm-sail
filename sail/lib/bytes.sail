
/*md
# Byte conversions

Big-endian conversions between words, addresses, and materialized byte
lists.
*/
/*! The 32-byte big-endian encoding of a word. */
function word_to_bytes32(w : word) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}

/*! The 20-byte big-endian encoding of an address. */
function address_to_bytes(a : address) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(a, 8 * k)[7 .. 0] :: out
    };
    out
}

/* The sole caller rejects inputs longer than one word before entering here. */
/*! Folds a big-endian byte list into a 256-bit value (later bytes are
    less significant). */
function bytes_be256(bytes : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    var rest : list(byte) = bytes;
    foreach (byte_index from 0 to 31) {
        match rest {
            b :: tail => {
                acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(b, 256));
                rest = tail
            },
            [||] => (),
        }
    };
    acc
}
