/* Optimized lifecycle for `sail/executor/stateless.sail`.
 *
 * Amsterdam's BAL defines the dense block-local account and storage schema.
 * Build that schema before executing the first transaction so subsequent
 * state access and final BAL authentication share the same stable IDs. */
function verify_stateless_payload(input_ref : StatelessInputRef) -> StatelessValidationResult = {
    var active_stage : validation_stage = VALIDATION_STAGE_DECODE_INPUT;
    try {
        scratch_reset();
        let input = decode_stateless_input(input_ref);

        active_stage = VALIDATION_STAGE_INDEX_WITNESS;
        let witness = index_execution_witness(input_ref);

        active_stage = VALIDATION_STAGE_VALIDATE_PAYLOAD;
        validate_execution_payload(input, input_ref, witness);

        active_stage = VALIDATION_STAGE_EXECUTE_BLOCK;
        let block = input.payload.block;
        let execution_profile = k_execution_profile;
        if execution_profile.protocol.fork >= Amsterdam then {
            initialize_block_access_list_state(
                block.body.block_access_list,
                k_parent_state_root,
            )
        };
        let result = execute_block_body(
            block.body,
            input_ref.public_keys,
            input_ref.deposits,
        );

        active_stage = VALIDATION_STAGE_VALIDATE_RESULT;
        validate_executed_block(block, input_ref, result);
        StatelessPayloadValid()
    } catch {
        InvalidBlock(reason) => {
            StatelessPayloadInvalid(struct { stage = active_stage, reason = reason })
        },
        Overflow() => throw Overflow(),
    }
}
