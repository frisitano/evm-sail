/* Optimized C overrides for `sail/lib/ssz/stateless_input.sail`. */

/* Decode the complete fixed-width SSZ uint256 at its semantic boundary.  The
 * specification retains the byte-wise Horner equation; optimized C loads the
 * same little-endian value directly into the numeric word carrier. */
val ssz_u256_at = impure { c: "ssz_u256" } : forall 'offset, stateless_input_valid_range('offset, 32). (
        StatelessInputSlice,
        int('offset),
    ) -> word

function ssz_u256(input, offset) =
    ssz_u256_at(input, offset)

/*
 * Resolve the complete nested SSZ envelope once in C. The specification keeps
 * the explicit container-cursor equations; this override returns the same
 * source-backed spans and bounded list references without materializing any
 * variable-length field.
 */
val stateless_input_ref_decode = impure { c: "decode_stateless_input_ref" } : StatelessInputSlice -> StatelessInputRef

function decode_stateless_input_ref(input : StatelessInputSlice) -> StatelessInputRef =
    stateless_input_ref_decode(input)

/*! Decode one fixed-width withdrawal without four separate source accesses. */
val withdrawal_decode_from_ssz = impure { c: "decode_withdrawal" } : StatelessInputSlice -> Withdrawal

function decode_withdrawal(withdrawal : StatelessInputSlice) -> Withdrawal =
    withdrawal_decode_from_ssz(withdrawal)

val request_digest_sha256 = impure { c: "sha256_request_digest" } : (byte, StatelessInputSlice) -> hash

function sha256_request_digest(request_type : byte, request : StatelessInputSlice) -> hash =
    request_digest_sha256(request_type, request)

/*
 * Index source-backed witness nodes in one operation. The standard model
 * retains the explicit SSZ cursor and per-node hash equation.
 */
val witness_nodes_index = impure { c: "index_witness_nodes" } : WitnessNodeListRef -> unit

function index_witness_nodes(nodes : WitnessNodeListRef) -> unit =
    witness_nodes_index(nodes)

/*
 * Likewise, keep the standard PUSH-aware opcode scan in Sail while allowing
 * the optimized build to consume and index the complete source-backed code
 * list in one call.
 */
val index_witness_codes_for_fork = impure { c: "index_witness_codes" } : (WitnessCodeListRef, bool) -> unit

function index_witness_codes(codes : WitnessCodeListRef) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    index_witness_codes_for_fork(codes, profile.fork >= Amsterdam)
}
