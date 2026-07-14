/*md
# SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules.
*/

/*! The width of one entry in an SSZ variable-field offset table
    (`uint32`, little-endian). */
let SSZ_OFF_BYTES : int = 4

/*! The private-input oracle's length. C provides one bounded byte vector
    through native and guest implementations; every reader in this module
    is a view over that source. */
val ssz_src_len = impure { c: "ssz_src_len" } : unit -> bits(64)
/*! One byte of the private input, at a 64-bit offset (the axiomatic
    read). */
val ssz_src_byte_at = impure { c: "ssz_src_byte" } : u64 -> bits(8)

/*! Integer-offset adapter over [ssz_src_byte_at][]: decode arithmetic
    stays in the `int` domain and converts here. */
function ssz_src_byte(i : int) -> byte = ssz_src_byte_at(get_slice_int(64, i, 0))

/*! Little-endian `n`-byte read at offset `b`; a pure derivation of
    [ssz_src_byte][], as are all bulk readers in this module. */
function ssz_src_le(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(ssz_src_byte(b + k), 64), 8 * k))
    };
    acc
}

/*! Big-endian `n`-byte read at offset `b`. */
function ssz_src_be(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(ssz_src_byte(b + k), 64))
    };
    acc
}

/*! A little-endian `uint32` at absolute input offset `b`. */
function ssz_u32(b : int) -> int = unsigned(ssz_src_le(b, 4))
/*! A little-endian `uint64` at absolute input offset `b`. */
function ssz_u64(b : int) -> int = unsigned(ssz_src_le(b, 8))

/*! A big-endian 20-byte address at offset `b`. */
function ssz_addr(b : int) -> address = {
    let w0 = ssz_src_be(b + 0, 8);
    let w1 = ssz_src_be(b + 8, 8);
    let w2 = ssz_src_be(b + 16, 4);
    append(w0, append(w1, w2[31 .. 0]))
}

/*! Reads `len` bytes at absolute offset `b` into a `list(byte)`
    (O(len)). */
function ssz_bytes(b : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = ssz_src_byte(b + (len - 1 - k)) :: out
    };
    out
}

/*! A big-endian 32-byte field (`Bytes32`, e.g. `prev_randao`). */
function ssz_bytes32(b : int) -> bits(256) = {
    let w0 = ssz_src_be(b + 0, 8);
    let w1 = ssz_src_be(b + 8, 8);
    let w2 = ssz_src_be(b + 16, 8);
    let w3 = ssz_src_be(b + 24, 8);
    append(w0, append(w1, append(w2, w3)))
}

/*! The 256-byte logs bloom at offset `b`. */
function ssz_logs_bloom(b : int) -> LogsBloom = {
    var out : LogsBloom = vector_init(256, 0x00);
    foreach (k from 0 to 31) {
        let word = ssz_src_be(b + 8 * k, 8);
        foreach (j from 0 to 7) {
            let i = unsigned(get_slice_int(8, 8 * k + j, 0));
            out[i] = sail_shiftright(word, 8 * (7 - j))[7 .. 0]
        }
    };
    out
}

/*! A little-endian 32-byte `uint256` (SSZ). */
function ssz_u256(b : int) -> word = {
    let c0 = ssz_src_le(b + 0, 8);
    let c1 = ssz_src_le(b + 8, 8);
    let c2 = ssz_src_le(b + 16, 8);
    let c3 = ssz_src_le(b + 24, 8);
    append(c3, append(c2, append(c1, c0)))
}
