/* ======================================================================== */
/* SPEC BACKEND: PRIVATE INPUT ORACLE                                       */
/*                                                                          */
/* Canonical definitions for the bulk private-input readers: both are pure  */
/* derivations of ssz_src_byte, so little-endian and big-endian reads are   */
/* consistent views over the same bounded input byte vector.                */
/* ======================================================================== */

function ssz_src_le(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(ssz_src_byte(b + k), 64), 8 * k))
    };
    acc
}

function ssz_src_be(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(ssz_src_byte(b + k), 64))
    };
    acc
}
