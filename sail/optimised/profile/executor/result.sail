/* Optimized C overrides for `sail/executor/result.sail`. */

function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    cycle_scope_start(6);
    let root = htr_new_payload_request(input_ref);
    cycle_scope_end(6);
    cycle_scope_start(7);
    commit_validation_result(root, success, input_ref.chain_config);
    cycle_scope_end(7)
}

function write_invalid_result() -> unit = {
    cycle_scope_start(7);
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
    write_prefix(ZERO_HASH, false);
    scratch_push_slice(chain_config);
    assert(public_output_write(scratch_finish(output_start)), "public output write");
    cycle_scope_end(7)
}
