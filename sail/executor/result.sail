/*md
# The public output

The guest's public output: the `hash_tree_root` of the new-payload
request, the validation verdict, and the echoed chain configuration.
*/

let RESULT_METADATA_LENGTH : byte_length = ByteQuantity(5)

/*! Writes the request root and validation metadata prefix. */
function result_prefix(root : hash, success : bool) -> unit = {
    scratch_push_bytes(word_to_bytes32(root), WORD_BYTE_LENGTH);
    scratch_push_bytes([|if success then { 0x01 } else { 0x00 }, 0x25, 0x00, 0x00, 0x00|], RESULT_METADATA_LENGTH)
}

/*! Serializes and commits the public validation result exactly once. */
function commit_validation_result(root : hash, success : bool, chain_config : ByteSlice) -> unit = {
    let start = scratch_begin();
    result_prefix(root, success);
    scratch_push_slice(chain_config);
    assert(public_output_write(scratch_finish(start)), "public output write")
}

/*! Emits the full public output for a decoded input: the request root
    computed from the input itself, the verdict, and the input's chain
    configuration echoed byte for byte. */
function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    cycle_scope_start(SCOPE_COMPUTE_OUTPUT_ROOT);
    let root = htr_new_payload_request(input_ref);
    cycle_scope_end(SCOPE_COMPUTE_OUTPUT_ROOT);

    cycle_scope_start(SCOPE_SERIALIZE_OUTPUT);
    commit_validation_result(root, success, input_ref.chain_config);
    cycle_scope_end(SCOPE_SERIALIZE_OUTPUT)
}

/*! Emits the failure output for an undecodable input: a zero root, a
    false verdict, and a default configuration frame. */
function write_invalid_result() -> unit = {
    cycle_scope_start(SCOPE_SERIALIZE_OUTPUT);
    let start = scratch_begin();
    foreach (i from 0 to 35) {
        let b : byte = match i {
            8 => 0x0c,
            20 => 0x10,
            24 => 0x18,
            28 => 0x08,
            32 => 0x08,
            _ => 0x00,
        };
        scratch_push_bytes([|b|], BYTE_ONE)
    };
    let chain_config = scratch_finish(start);
    commit_validation_result(0x0000000000000000000000000000000000000000000000000000000000000000, false, chain_config);
    cycle_scope_end(SCOPE_SERIALIZE_OUTPUT)
}
