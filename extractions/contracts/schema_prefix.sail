/* Pure proof adapter for the SszStatelessInput schema prefix.
 *
 * Mirrors sail/lib/ssz/stateless_input.sail:
 *
 *   slice_byte(input, 0) @ slice_byte(input, 1) == 0x0001
 *
 * The executable model therefore treats the two schema bytes as big-endian.
 * Schema id 1 is byte prefix 00 01.
 */

$include "../../sail/prelude.sail"

let SCHEMA_PREFIX_LEN : limb = 0x0000000000000002
let STATELESS_INPUT_SCHEMA_ID : bits(16) = 0x0001

function schema_prefix_id(input0 : byte, input1 : byte) -> bits(16) =
    input0 @ input1

function schema_prefix_ok(input0 : byte, input1 : byte, len : limb) -> bool =
    limb_ule(SCHEMA_PREFIX_LEN, len) & (schema_prefix_id(input0, input1) == STATELESS_INPUT_SCHEMA_ID)

function schema_prefix_expected_bytes_equiv(input0 : byte, input1 : byte, len : limb) -> bool = {
    let expected = limb_ule(SCHEMA_PREFIX_LEN, len) & (input0 == 0x00) & (input1 == 0x01);
    schema_prefix_ok(input0, input1, len) == expected
}

function schema_prefix_rejects_little_endian_one() -> bool =
    not_bool(schema_prefix_ok(0x01, 0x00, SCHEMA_PREFIX_LEN))
