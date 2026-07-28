/*md
# The public output

The guest's public output: the `hash_tree_root` of the new-payload
request, the validation verdict, and the echoed chain configuration.
*/

let RESULT_METADATA_LENGTH : int(5) = 5

/*! Writes the request root and validation metadata prefix. */
function result_prefix(root : hash, success : bool) -> unit = {
    scratch_push_b256(root, WORD_BYTE_LENGTH);
    scratch_push_byte(if success then { 0x01 } else { 0x00 });
    scratch_push_byte(0x25);
    scratch_push_byte(0x00);
    scratch_push_byte(0x00);
    scratch_push_byte(0x00)
}

/*! Serializes and commits the public validation result exactly once. */
function commit_validation_result(root : hash, success : bool, chain_config : StatelessInputSlice) -> unit = {
    let fixed_length = WORD_BYTE_LENGTH + RESULT_METADATA_LENGTH;
    let start = scratch_reserve(scratch_length_add(fixed_length, chain_config.len));
    result_prefix(root, success);
    scratch_push_slice(chain_config);
    assert(public_output_write(scratch_finish(start)), "public output write")
}

/*! Emits the full public output for a decoded input: the request root
    computed from the input itself, the verdict, and the input's chain
    configuration echoed byte for byte. */
function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    let root = htr_new_payload_request(input_ref);
    commit_validation_result(root, success, input_ref.chain_config)
}

/*! Emits the failure output for an undecodable input: a zero root, a
    false verdict, and a default configuration frame. */
function write_invalid_result() -> unit = {
    let start = scratch_reserve(24);
    var default_chain_config = ZERO_HASH;
    default_chain_config[8] = 0x0c;
    default_chain_config[12] = 0x04;
    default_chain_config[16] = 0x08;
    default_chain_config[20] = 0x08;
    scratch_push_b256(default_chain_config, 24);
    let chain_config = scratch_finish(start);
    let fixed_length = WORD_BYTE_LENGTH + RESULT_METADATA_LENGTH;
    let output_start = scratch_reserve(scratch_length_add(fixed_length, chain_config.len));
    result_prefix(ZERO_HASH, false);
    scratch_push_slice(chain_config);
    assert(public_output_write(scratch_finish(output_start)), "public output write")
}
