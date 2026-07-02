/* Pure proof adapter for the byte-level input_well_formed header checks.
 *
 * Mirrors sail/main.sail::input_well_formed without reading from ssz_src_*.
 * The adapter takes an explicit byte list and length, so symbolic tools can
 * reason over the contract independently of host I/O and C runtime state.
 */

$include "schema_prefix.sail"

let SSZ_BODY : int = 2
let STATELESS_INPUT_FIXED_HEADER_LEN : int = 18
let STATELESS_INPUT_FIELD_OFFSET0 : int = 16
let PUBLIC_KEY_BYTES : int = 65

function byte_at(input : list(byte), i : int) -> byte =
    list_nth(input, i, 0x00)

function u32le_from_bytes(b0 : byte, b1 : byte, b2 : byte, b3 : byte) -> int =
    unsigned(b0) + unsigned(b1) * 256 + unsigned(b2) * 65536 + unsigned(b3) * 16777216

function u32le_at(input : list(byte), offset : int) -> int =
    u32le_from_bytes(
        byte_at(input, offset),
        byte_at(input, offset + 1),
        byte_at(input, offset + 2),
        byte_at(input, offset + 3),
    )

function stateless_offsets_well_formed(o0 : int, o1 : int, o2 : int, o3 : int, len : int) -> bool =
      (o0 == STATELESS_INPUT_FIELD_OFFSET0)
    & (o0 <= o1)
    & (o1 <= o2)
    & (o2 <= o3)
    & (SSZ_BODY + o3 <= len)
    & (emod_int(len - (SSZ_BODY + o3), PUBLIC_KEY_BYTES) == 0)

function input_header_well_formed(input : list(byte), len : int) -> bool = {
    if len < STATELESS_INPUT_FIXED_HEADER_LEN then {
        false
    } else {
        if not_bool(schema_prefix_ok_bytes(input, len)) then {
            false
        } else {
            let o0 = u32le_at(input, SSZ_BODY + 0);
            let o1 = u32le_at(input, SSZ_BODY + 4);
            let o2 = u32le_at(input, SSZ_BODY + 8);
            let o3 = u32le_at(input, SSZ_BODY + 12);
            stateless_offsets_well_formed(o0, o1, o2, o3, len)
        }
    }
}

function input_header_matches_offsets(input : list(byte), len : int) -> bool = {
    let o0 = u32le_at(input, SSZ_BODY + 0);
    let o1 = u32le_at(input, SSZ_BODY + 4);
    let o2 = u32le_at(input, SSZ_BODY + 8);
    let o3 = u32le_at(input, SSZ_BODY + 12);
    input_header_well_formed(input, len) ==
        (  (len >= STATELESS_INPUT_FIXED_HEADER_LEN)
        & schema_prefix_ok_bytes(input, len)
        & stateless_offsets_well_formed(o0, o1, o2, o3, len))
}
