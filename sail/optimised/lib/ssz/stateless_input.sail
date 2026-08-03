/* Optimized C overrides for `sail/lib/ssz/stateless_input.sail`. */

/* Decode the complete fixed-width SSZ uint256 at its semantic boundary.  The
 * specification retains the byte-wise Horner equation; optimized C loads the
 * same little-endian value directly into the numeric word carrier. */
$[c_throws]
val optimized_ssz_u256 = impure { c: "ssz_u256" } :
    forall 'offset, stateless_input_valid_range('offset, 32).
    (StatelessInputSlice, int('offset)) -> word

function ssz_u256(input, offset) =
    optimized_ssz_u256(input, offset)

/*
 * Resolve the complete nested SSZ envelope once in C. The specification keeps
 * the explicit container-cursor equations; this override returns the same
 * source-backed spans and bounded list references without materializing any
 * variable-length field.
 */
$[c_throws]
val optimized_decode_stateless_input_ref =
    impure { c: "decode_stateless_input_ref" } :
    StatelessInputSlice -> StatelessInputRef

function decode_stateless_input_ref(input : StatelessInputSlice) -> StatelessInputRef =
    optimized_decode_stateless_input_ref(input)

/*! Decode one fixed-width withdrawal without four separate source accesses. */
$[c_throws]
val optimized_decode_withdrawal =
    impure { c: "decode_withdrawal" } :
    StatelessInputSlice -> Withdrawal

function decode_withdrawal(withdrawal : StatelessInputSlice) -> Withdrawal =
    optimized_decode_withdrawal(withdrawal)

val optimized_sha256_request_digest = impure { c: "sha256_request_digest" } : (
        byte,
        StatelessInputSlice,
    ) -> hash

function sha256_request_digest(request_type : byte, request : StatelessInputSlice) -> hash =
    optimized_sha256_request_digest(request_type, request)

/*
 * Index source-backed witness nodes in one operation. The standard model
 * retains the explicit SSZ cursor and per-node hash equation.
 */
$[c_throws]
val optimized_index_witness_nodes = impure { c: "index_witness_nodes" } : WitnessNodeListRef -> unit

function index_witness_nodes(nodes : WitnessNodeListRef) -> unit =
    optimized_index_witness_nodes(nodes)

/*
 * Likewise, keep the standard PUSH-aware opcode scan in Sail while allowing
 * the optimized build to consume and index the complete source-backed code
 * list in one call.
 */
$[c_throws]
val optimized_index_witness_codes = impure { c: "index_witness_codes" } : (WitnessCodeListRef, bool) -> unit

function index_witness_codes(codes : WitnessCodeListRef) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    optimized_index_witness_codes(codes, profile.fork >= Amsterdam)
}
