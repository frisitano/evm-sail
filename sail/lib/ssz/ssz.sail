/* SSZ: GENERIC SLICE DECODING                                              */
/* Scalar readers over a source-backed ByteSlice. Concrete container        */
/* layouts belong in their decoder modules.                                 */

/*md
# SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules.
*/
/*! The width of one entry in an SSZ variable-field offset table
    (`uint32`, little-endian). */
let SSZ_OFF_BYTES : byte_length = ByteQuantity(4)
let SSZ_UINT_BYTES : byte_length = EIGHT_BYTE_LENGTH

/* Container decoders establish that the complete fixed-width field lies in
 * the input before calling these scalar readers. */
function ssz_field_offset(base : source_pointer, delta : nat) -> source_pointer =
    base + ByteQuantity(delta)

/*! Reads a little-endian SSZ `uint32` used by a variable-field offset table. */
function ssz_u32_at(input : ByteSlice, offset : source_pointer) -> protocol_quantity = {
    let b0 = sail_zero_extend(slice_byte(input, offset), 64);

    let b1 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 1)), 64);
    let b2 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 2)), 64);
    let b3 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 3)), 64);
    unsigned(or_vec(b0, or_vec(sail_shiftleft(b1, 8), or_vec(sail_shiftleft(b2, 16), sail_shiftleft(b3, 24)))))
}

/*! A little-endian `uint32` at absolute input offset `b`. */
function ssz_u32(input : ByteSlice, offset : source_pointer) -> protocol_quantity = ssz_u32_at(input, offset)

/*! Reads an eight-byte little-endian limb from a validated SSZ field. */
function ssz_limb(input : ByteSlice, offset : source_pointer) -> limb = {
    let b0 = sail_zero_extend(slice_byte(input, offset), 64);
    let b1 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 1)), 64);
    let b2 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 2)), 64);
    let b3 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 3)), 64);
    let b4 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 4)), 64);
    let b5 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 5)), 64);
    let b6 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 6)), 64);
    let b7 = sail_zero_extend(slice_byte(input, ssz_field_offset(offset, 7)), 64);
    or_vec(
        b0,
        or_vec(
            sail_shiftleft(b1, 8),
            or_vec(
                sail_shiftleft(b2, 16),
                or_vec(
                    sail_shiftleft(b3, 24),
                    or_vec(
                        sail_shiftleft(b4, 32),
                        or_vec(sail_shiftleft(b5, 40), or_vec(sail_shiftleft(b6, 48), sail_shiftleft(b7, 56))),
                    ),
                ),
            ),
        ),
    )
}

function ssz_uint(input : ByteSlice, offset : source_pointer) -> protocol_quantity =
    unsigned(ssz_limb(input, offset))

/*! A big-endian 20-byte address at offset `b`. */
function ssz_addr(input : ByteSlice, offset : source_pointer) -> address =
    slice_load_n(input, offset, ADDRESS_BYTE_LENGTH)[159 .. 0]

/*! A big-endian 32-byte field (`Bytes32`, e.g. `prev_randao`). */
function ssz_bytes32(input : ByteSlice, offset : source_pointer) -> bits(256) =
    slice_load(input, offset)

/*! The 256-byte logs bloom at offset `b`. */
function ssz_logs_bloom(input : ByteSlice, offset : source_pointer) -> LogsBloom = {
    var out : LogsBloom = vector_init(32, LIMB_ZERO);
    var cursor : source_pointer = offset;
    foreach (k from 0 to 31) {
        out[k] = slice_load_n(input, cursor, EIGHT_BYTE_LENGTH)[63 .. 0];
        cursor = ssz_field_offset(cursor, 8)
    };
    out
}

/* little-endian 32-byte uint256 (SSZ), low word first */
/*! A little-endian 32-byte `uint256` (SSZ). */
function ssz_u256(input : ByteSlice, offset : source_pointer) -> word = {
    let c0 = ssz_limb(input, offset);
    let c1 = ssz_limb(input, ssz_field_offset(offset, 8));
    let c2 = ssz_limb(input, ssz_field_offset(offset, 16));
    let c3 = ssz_limb(input, ssz_field_offset(offset, 24));
    append(c3, append(c2, append(c1, c0)))
}
