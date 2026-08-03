/* Optimized C overrides for `sail/executor/stateless.sail`. */

function validate_executed_block(block : Block, input_ref : StatelessInputRef, result : BlockExecutionResult) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let header = block.header;
    if profile.fork >= Prague then {
        if   not_bool(region_slices_equal(result.requests.withdrawals, input_ref.withdrawal_requests))
           | not_bool(region_slices_equal(result.requests.consolidations, input_ref.consolidation_requests))
           | not_bool(region_slices_equal(result.requests.builder_deposits, input_ref.builder_deposit_requests))
           | not_bool(region_slices_equal(result.requests.builder_exits, input_ref.builder_exit_requests)) then {
            throw InvalidBlock(InvalidExecutionRequests)
        }
    };
    validation_debug_capture_block_gas(
        result.header_gas_used,
        header.gas_used,
        result.execution_gas_used,
        result.state_gas_used,
    );
    if result.header_gas_used != header.gas_used then {
        throw InvalidBlock(InvalidGasUsed)
    };
    if (profile.fork >= Cancun) & (result.blob_gas_used != header.blob_gas_used) then {
        throw InvalidBlock(InvalidBlobGasUsed)
    };
    cycle_scope_start(19);
    let poststate = compute_state_root();
    cycle_scope_end(19);
    if poststate != header.state_root then {
        throw InvalidBlock(InvalidStateRoot)
    };
    if result.receipts_root != header.receipts_root then {
        throw InvalidBlock(InvalidReceiptsRoot)
    };
    if not_bool(logs_bloom_matches_ref(result.logs_bloom, header.logs_bloom)) then {
        throw InvalidBlock(InvalidLogsBloom)
    };
    if profile.fork >= Amsterdam then {
        cycle_scope_start(20);
        validate_block_access_list(block.body.block_access_list, header.gas_limit);
        cycle_scope_end(20)
    }
}

function verify_stateless_payload(input_ref : StatelessInputRef) -> StatelessValidationResult = {
    var active_stage : validation_stage = VALIDATION_STAGE_DECODE_INPUT;
    cycle_scope_start(active_stage);
    try {
        scratch_reset();
        let input = decode_stateless_input(input_ref);

        cycle_scope_end(active_stage);
        active_stage = VALIDATION_STAGE_INDEX_WITNESS;
        cycle_scope_start(active_stage);
        let witness = index_execution_witness(input_ref);

        cycle_scope_end(active_stage);
        active_stage = VALIDATION_STAGE_VALIDATE_PAYLOAD;
        cycle_scope_start(active_stage);
        validate_execution_payload(input, input_ref, witness);

        cycle_scope_end(active_stage);
        active_stage = VALIDATION_STAGE_EXECUTE_BLOCK;
        cycle_scope_start(active_stage);
        let block = input.payload.block;
        if k_protocol_profile.fork >= Amsterdam then {
            initialize_block_access_list_state(
                block.body.block_access_list,
                k_parent_state_root,
            )
        };
        let result = execute_block_body(block.body, input_ref.public_keys, input_ref.deposits);

        cycle_scope_end(active_stage);
        active_stage = VALIDATION_STAGE_VALIDATE_RESULT;
        cycle_scope_start(active_stage);
        validate_executed_block(block, input_ref, result);
        cycle_scope_end(active_stage);
        StatelessPayloadValid()
    } catch {
        InvalidBlock(reason) => {
            cycle_scope_end(active_stage);
            StatelessPayloadInvalid(struct { stage = active_stage, reason = reason })
        },
        Overflow() => {
            cycle_scope_end(active_stage);
            throw Overflow()
        },
    }
}
