/* Pure proof adapter for decode_stateless_input_ref's inline header checks.
 *
 * The executable and RISC-V guest read these values directly from input
 * memory. This relation starts after those loads, so it does not model the
 * random-access input buffer as a linked Sail list.
 */

$include "schema_prefix.sail"

let SSZ_BODY : limb = 0x0000000000000002
let STATELESS_INPUT_FIXED_HEADER_LEN : limb = 0x0000000000000012
let STATELESS_INPUT_FIELD_OFFSET0 : limb = 0x0000000000000010
let PUBLIC_KEY_BYTES : limb = 0x0000000000000041

function stateless_offsets_well_formed(o0 : limb, o1 : limb, o2 : limb, o3 : limb, len : limb) -> bool = {
    match limb_checked_add(SSZ_BODY, o3) {
        None() => false,
        Some(body_end) => if not_bool(limb_ule(body_end, len)) then {
            false
        } else {
            let public_keys_len = sub_bits(len, body_end);
            let divided = limb_divmod(public_keys_len, PUBLIC_KEY_BYTES);
              (o0 == STATELESS_INPUT_FIELD_OFFSET0)
            & limb_ule(o0, o1)
            & limb_ule(o1, o2)
            & limb_ule(o2, o3)
            & (divided.remainder == LIMB_ZERO)
        },
    }
}

function input_header_well_formed(
    input0 : byte,
    input1 : byte,
    o0 : limb,
    o1 : limb,
    o2 : limb,
    o3 : limb,
    len : limb,
) -> (
    bool
) = {
    if limb_ult(len, STATELESS_INPUT_FIXED_HEADER_LEN) then {
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
    o0 : limb,
    o1 : limb,
    o2 : limb,
    o3 : limb,
    len : limb,
) -> (
    bool
) = {
    input_header_well_formed(input0, input1, o0, o1, o2, o3, len) ==
        (  limb_ule(STATELESS_INPUT_FIXED_HEADER_LEN, len)
        & schema_prefix_ok(input0, input1, len)
        & stateless_offsets_well_formed(o0, o1, o2, o3, len))
}
