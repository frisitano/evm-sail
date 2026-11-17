/* Optimized C overrides for `sail/executor/result.sail`. */

function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    cycle_scope_start(6);
    let root = htr_new_payload_request(input_ref);
    cycle_scope_end(6);
    cycle_scope_start(7);
    commit_validation_result(root, success, input_ref.chain_id, STATELESS_SCHEMA_ID);
    cycle_scope_end(7)
}

function write_invalid_result() -> unit = {
    cycle_scope_start(7);
    commit_validation_result(ZERO_HASH, false, 0, 0x0000);
    cycle_scope_end(7)
}
