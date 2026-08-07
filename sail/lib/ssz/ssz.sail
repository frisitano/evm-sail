/* SSZ: GENERIC SLICE DECODING                                              */
/* Scalar readers over a source-backed StatelessInputSlice. Concrete container        */
/* layouts belong in their decoder modules.                                 */

/*md
# SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules.
*/
/*! The width of one entry in an SSZ variable-field offset table
    (`uint32`, little-endian). */
let SSZ_OFF_BYTES : int(4) = 4
let SSZ_UINT_BYTES : int(8) = 8

/* Container decoders establish that the complete fixed-width field lies in
 * the input before calling these scalar readers. */
/*! Resolves a validated container-relative field offset to a source pointer. */
val ssz_field_offset : forall 'base 'delta, stateless_input_valid_range('base, 'delta). (int('base), int('delta)) -> int(
        'base + 'delta,
    )

function ssz_field_offset(base, delta) = base + delta

/*! Resolves a validated scratch-relative field offset. */
val scratch_field_offset : forall 'base 'delta, scratch_valid_range('base, 'delta). (int('base), int('delta)) -> int(
        'base + 'delta,
    )

function scratch_field_offset(base, delta) = base + delta

/*! Reads a little-endian SSZ `uint32` used by a variable-field offset table. */
val ssz_u32_at : forall 'offset, stateless_input_valid_range('offset, 4). (StatelessInputSlice, int('offset)) -> ssz_offset

function ssz_u32_at(input, offset) = {
    let byte0 = slice_byte(input, offset);
    let offset1 = ssz_field_offset(offset, 1);
    let byte1 = slice_byte(input, offset1);
    let offset2 = ssz_field_offset(offset, 2);
    let byte2 = slice_byte(input, offset2);
    let offset3 = ssz_field_offset(offset, 3);
    let byte3 = slice_byte(input, offset3);
    let b0 = sail_zero_extend(byte0, 32);
    let b1 = sail_zero_extend(byte1, 32);
    let b2 = sail_zero_extend(byte2, 32);
    let b3 = sail_zero_extend(byte3, 32);
    let shifted1 = sail_shiftleft(b1, 8);
    let shifted2 = sail_shiftleft(b2, 16);
    let shifted3 = sail_shiftleft(b3, 24);
    let high = or_vec(shifted2, shifted3);
    let nonzero = or_vec(shifted1, high);
    let result = or_vec(b0, nonzero);
    unsigned(result)
}

/*! A little-endian `uint32` at absolute input offset `b`. */
val ssz_u32 : forall 'offset, stateless_input_valid_range('offset, 4). (StatelessInputSlice, int('offset)) -> ssz_offset

function ssz_u32(input, offset) = ssz_u32_at(input, offset)

/*! Reads an offset-table entry after establishing that the dynamic table
    position is contained by its enclosing input slice. */
function ssz_u32_in_slice(input : StatelessInputSlice, offset : ssz_offset) -> ssz_offset =
    if offset <= input.len & 4 <= input.len - offset then {
        ssz_u32_at(input, offset)
    } else {
        fatal_error(InvalidConfig)
    }

/*! Narrows a wire-bounded SSZ offset at the host byte-position boundary. */
function ssz_offset_to_source_pointer(value : ssz_offset) -> stateless_input_pointer =
    value

/*! Reads an eight-byte little-endian SSZ protocol quantity. */
val decode_ssz_uint : forall 'offset, stateless_input_valid_range('offset, 8). (StatelessInputSlice, int('offset)) -> ssz_uint

function decode_ssz_uint(input, offset) = {
    let byte0 = slice_byte(input, offset);
    let offset1 = ssz_field_offset(offset, 1);
    let byte1 = slice_byte(input, offset1);
    let offset2 = ssz_field_offset(offset, 2);
    let byte2 = slice_byte(input, offset2);
    let offset3 = ssz_field_offset(offset, 3);
    let byte3 = slice_byte(input, offset3);
    let offset4 = ssz_field_offset(offset, 4);
    let byte4 = slice_byte(input, offset4);
    let offset5 = ssz_field_offset(offset, 5);
    let byte5 = slice_byte(input, offset5);
    let offset6 = ssz_field_offset(offset, 6);
    let byte6 = slice_byte(input, offset6);
    let offset7 = ssz_field_offset(offset, 7);
    let byte7 = slice_byte(input, offset7);
      unsigned(byte0)
    + unsigned(byte1)
    * 2 ^ 8
    + unsigned(byte2)
    * 2 ^ 16
    + unsigned(byte3)
    * 2 ^ 24
    + unsigned(byte4)
    * 2 ^ 32
    + unsigned(byte5)
    * 2 ^ 40
    + unsigned(byte6)
    * 2 ^ 48
    + unsigned(byte7)
    * 2 ^ 56
}

/*! Reads an eight-byte little-endian quantity retained in executor scratch.
    This is deliberately separate from input SSZ decoding: the nominal slice
    type fixes the backing region without a runtime source discriminator. */
val decode_scratch_uint : forall 'offset, scratch_valid_range('offset, 8). (ScratchSlice, int('offset)) -> ssz_uint

function decode_scratch_uint(input, offset) = {
    let byte0 = slice_byte(input, offset);
    let offset1 = scratch_field_offset(offset, 1);
    let byte1 = slice_byte(input, offset1);
    let offset2 = scratch_field_offset(offset, 2);
    let byte2 = slice_byte(input, offset2);
    let offset3 = scratch_field_offset(offset, 3);
    let byte3 = slice_byte(input, offset3);
    let offset4 = scratch_field_offset(offset, 4);
    let byte4 = slice_byte(input, offset4);
    let offset5 = scratch_field_offset(offset, 5);
    let byte5 = slice_byte(input, offset5);
    let offset6 = scratch_field_offset(offset, 6);
    let byte6 = slice_byte(input, offset6);
    let offset7 = scratch_field_offset(offset, 7);
    let byte7 = slice_byte(input, offset7);
      unsigned(byte0)
    + unsigned(byte1)
    * 2 ^ 8
    + unsigned(byte2)
    * 2 ^ 16
    + unsigned(byte3)
    * 2 ^ 24
    + unsigned(byte4)
    * 2 ^ 32
    + unsigned(byte5)
    * 2 ^ 40
    + unsigned(byte6)
    * 2 ^ 48
    + unsigned(byte7)
    * 2 ^ 56
}

/*! A big-endian 20-byte address at offset `b`. */
val ssz_addr : forall 'offset, stateless_input_valid_range('offset, 20). (StatelessInputSlice, int('offset)) -> address

function ssz_addr(input, offset) = {
    let value = slice_load_n(input, offset, ADDRESS_BYTE_LENGTH);
    word_to_address(value)
}

/*! A big-endian 32-byte field (`Bytes32`, e.g. `prev_randao`). */
val ssz_bytes32 : forall 'offset, stateless_input_valid_range('offset, 32). (StatelessInputSlice, int('offset)) -> hash

function ssz_bytes32(input, offset) = {
    let value = slice_load(input, offset);
    word_to_hash(value)
}

/*! The 256-byte logs bloom at offset `b`. */
val ssz_logs_bloom : forall 'offset, stateless_input_valid_range('offset, 256). (StatelessInputSlice, int('offset)) -> LogsBloom

/*! Maps wire order to the decreasing byte-vector index. */
val ssz_logs_bloom_index : forall 'index, 0 <= 'index & 'index <= 255. int('index) -> int(255 - 'index)

function ssz_logs_bloom_index(index) = 255 - index

/*! Decodes a fixed 256-byte logs bloom from SSZ wire order. */
function ssz_logs_bloom(input, offset) = {
    var out : LogsBloom = vector_init(256, 0x00);
    foreach (k from 0 to 255) {
        let output_index = ssz_logs_bloom_index(k);
        let source_offset = ssz_field_offset(offset, k);
        out[output_index] = slice_byte(input, source_offset)
    };
    out
}

/* little-endian 32-byte uint256 (SSZ), low word first */
/*! A little-endian 32-byte `uint256` (SSZ). */
val ssz_u256 : forall 'offset, stateless_input_valid_range('offset, 32). (StatelessInputSlice, int('offset)) -> word

/*! Maps Horner iteration order to the little-endian source-byte index. */
val ssz_u256_index : forall 'index, 0 <= 'index & 'index <= 31. int('index) -> int(31 - 'index)

function ssz_u256_index(index) = 31 - index

/*! Decodes a 32-byte little-endian SSZ integer into an EVM word. */
function ssz_u256(input, offset) = {
    var result : word = WORD_ZERO;
    foreach (k from 0 to 31) {
        let source_index = ssz_u256_index(k);
        let source_offset = ssz_field_offset(offset, source_index);
        let source_byte = slice_byte(input, source_offset);
        let byte_value = unsigned(source_byte);
        let shifted_result = word_mul(result, 256);
        result = word_add(shifted_result, byte_value)
    };
    result
}
