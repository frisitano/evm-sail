/*md
# The public output

The guest's public output, the 43-byte `StatelessValidationResult`: the
`hash_tree_root` of the new-payload request, the validation verdict, the
chain id, and the schema id of the decoded input. An undecodable input
publishes the all-zero sentinel.
*/

/*! The width of the `StatelessValidationResult`: the request root, the
    verdict byte, the `uint64` chain id, and the `uint16` schema id. */
let RESULT_LENGTH : int(43) = 43

/*! Serializes and commits the public validation result exactly once. */
function commit_validation_result(
    root : hash,
    success : bool,
    chain_id : chain_identifier,
    schema_id : bits(16),
) -> (
    unit
) = {
    let start = scratch_reserve(RESULT_LENGTH);
    scratch_push_b256(root, WORD_BYTE_LENGTH);
    let success_byte : byte =
        if success then 0x01 else 0x00;
    scratch_push_byte(success_byte);
    var identity = htr_uint(chain_id);
    identity[8] = schema_id[7 .. 0];
    identity[9] = schema_id[15 .. 8];
    scratch_push_b256(identity, 10);
    let output = scratch_finish(start);
    let written = public_output_write(output);
    assert(written, "public output write")
}

/*! Emits the full public output for a decoded input: the request root
    computed from the input itself, the verdict, and the input's chain id and
    schema id. */
function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    let root = htr_new_payload_request(input_ref);
    commit_validation_result(root, success, input_ref.chain_id, STATELESS_SCHEMA_ID)
}

/*! Emits the failure output for an undecodable input: a zero root, a
    false verdict, and a zero chain id and schema id. */
function write_invalid_result() -> unit =
    commit_validation_result(ZERO_HASH, false, 0, 0x0000)
