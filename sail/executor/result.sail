let RESULT_METADATA_LENGTH : byte_length = ByteQuantity(5)

function result_prefix(root : hash, success : bool) -> unit = {
    scratch_push_bytes(word_to_bytes32(root), WORD_BYTE_LENGTH);
    scratch_push_bytes([|if success then { 0x01 } else { 0x00 }, 0x25, 0x00, 0x00, 0x00|], RESULT_METADATA_LENGTH)
}

function commit_validation_result(root : hash, success : bool, chain_config : ByteSlice) -> unit = {
    let start = scratch_begin();
    result_prefix(root, success);
    scratch_push_slice(chain_config);
    assert(public_output_write(scratch_finish(start)), "public output write")
}

function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    cycle_scope_start(SCOPE_COMPUTE_OUTPUT_ROOT);
    let root = htr_new_payload_request(input_ref);
    cycle_scope_end(SCOPE_COMPUTE_OUTPUT_ROOT);

    cycle_scope_start(SCOPE_SERIALIZE_OUTPUT);
    commit_validation_result(root, success, input_ref.chain_config);
    cycle_scope_end(SCOPE_SERIALIZE_OUTPUT)
}

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
