/* Pure proof adapter for the SszStatelessInput schema prefix.
 *
 * Mirrors sail/host/io.sail:
 *
 *   function ssz_schema_id() -> int = ib(0) * 256 + ib(1)
 *
 * The executable model therefore treats the two schema bytes as big-endian.
 * Schema id 1 is byte prefix 00 01.
 */

$include "../../sail/prelude.sail"

let SCHEMA_PREFIX_LEN : int = 2
let STATELESS_INPUT_SCHEMA_ID : int = 1

function schema_prefix_id(input0 : byte, input1 : byte) -> int =
    unsigned(input0) * 256 + unsigned(input1)

function schema_prefix_ok(input0 : byte, input1 : byte, len : int) -> bool =
    (len >= SCHEMA_PREFIX_LEN) & (schema_prefix_id(input0, input1) == STATELESS_INPUT_SCHEMA_ID)

function schema_prefix_ok_bytes(input : list(byte), len : int) -> bool =
    schema_prefix_ok(list_nth(input, 0, 0x00), list_nth(input, 1, 0x00), len)

function schema_prefix_expected_bytes_equiv(input0 : byte, input1 : byte, len : int) -> bool = {
    let expected = (len >= SCHEMA_PREFIX_LEN) & (input0 == 0x00) & (input1 == 0x01);
    schema_prefix_ok(input0, input1, len) == expected
}

function schema_prefix_rejects_little_endian_one() -> bool =
    not_bool(schema_prefix_ok(0x01, 0x00, SCHEMA_PREFIX_LEN))
