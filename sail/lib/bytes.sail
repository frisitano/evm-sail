/* Big-endian materialized-byte conversions. */

function word_to_bytes32(w : word) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}

function address_to_bytes(a : address) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(a, 8 * k)[7 .. 0] :: out
    };
    out
}

function bytes_be256(bytes : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    var rest : list(byte) = bytes;
    var more : bool = true;
    while more do {
        match rest {
            byte :: tail => {
                acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(byte, 256));
                rest = tail
            },
            [||] => more = false,
        }
    };
    acc
}
