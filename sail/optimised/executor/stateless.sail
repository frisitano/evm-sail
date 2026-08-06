/* Optimized lifecycle for `sail/executor/stateless.sail`.
 *
 * Amsterdam's BAL defines the dense block-local account and storage schema.
 * Build that schema before executing the first transaction so subsequent
 * state access and final BAL authentication share the same stable IDs. */
function verify_stateless_payload(input_ref : StatelessInputRef) -> unit = {
    scratch_reset();
    let input = decode_stateless_input(input_ref);
    let witness = index_execution_witness(input_ref);
    validate_execution_payload(input, input_ref, witness);
    let block = input.payload.block;
    let execution_profile = k_execution_profile;
    if execution_profile.protocol.fork >= Amsterdam then {
        initialize_block_access_list_state(
            block.body.block_access_list,
            k_parent_state_root,
            block.body.transactions.count,
        )
    };
    let result = execute_block_body(block.body, input_ref);
    validate_executed_block(block, result)
}
