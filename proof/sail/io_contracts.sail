/* Pure proof adapter for decode_stateless_input_ref's inline header checks.
 *
 * The executable and RISC-V guest read these values directly from input
 * memory. This relation starts after those loads, so it does not model the
 * random-access input buffer as a linked Sail list.
 */

$include "schema_prefix.sail"

let SSZ_BODY : int = 2
let STATELESS_INPUT_FIXED_HEADER_LEN : int = 18
let STATELESS_INPUT_FIELD_OFFSET0 : int = 16
let PUBLIC_KEY_BYTES : int = 65

function stateless_offsets_well_formed(o0 : int, o1 : int, o2 : int, o3 : int, len : int) -> bool =
      (o0 == STATELESS_INPUT_FIELD_OFFSET0)
    & (o0 <= o1)
    & (o1 <= o2)
    & (o2 <= o3)
    & (SSZ_BODY + o3 <= len)
    & (emod_int(len - (SSZ_BODY + o3), PUBLIC_KEY_BYTES) == 0)

function input_header_well_formed(
    input0 : byte,
    input1 : byte,
    o0 : int,
    o1 : int,
    o2 : int,
    o3 : int,
    len : int,
) -> (
    bool
) = {
    if len < STATELESS_INPUT_FIXED_HEADER_LEN then {
        false
    } else {
        if not_bool(schema_prefix_ok(input0, input1, len)) then {
            false
        } else {
            stateless_offsets_well_formed(o0, o1, o2, o3, len)
        }
    }
}

function input_header_matches_offsets(
    input0 : byte,
    input1 : byte,
    o0 : int,
    o1 : int,
    o2 : int,
    o3 : int,
    len : int,
) -> (
    bool
) = {
    input_header_well_formed(input0, input1, o0, o1, o2, o3, len) ==
        (  (len >= STATELESS_INPUT_FIXED_HEADER_LEN)
        & schema_prefix_ok(input0, input1, len)
        & stateless_offsets_well_formed(o0, o1, o2, o3, len))
}
