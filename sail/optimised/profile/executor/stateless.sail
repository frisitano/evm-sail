/* Optimized C overrides for `sail/executor/stateless.sail`. */

function validate_executed_block(block : Block, result : BlockExecutionResult) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let header = block.header;
    validation_debug_capture_block_gas(
        result.header_gas_used,
        header.gas_used,
        result.execution_gas_used,
        result.state_gas_used,
    );
    if result.header_gas_used != header.gas_used then {
        fatal_error(InvalidGasUsed)
    };
    if (profile.fork >= Cancun) & (result.blob_gas_used != header.blob_gas_used) then {
        fatal_error(InvalidBlobGasUsed)
    };
    cycle_scope_start(19);
    let poststate = compute_state_root();
    cycle_scope_end(19);
    if poststate != header.state_root then {
        fatal_error(InvalidStateRoot)
    };
    /* Receipts/commitments phase: the receipts-root comparison, the logs
       bloom check, and block-access-list validation accumulate into the same
       `receipts_root` tag as the receipts-trie reduction performed in
       `verify_stateless_payload`, mirroring the aligned reth/ethrex guests.
       The finer `block_access_list` tag stays nested inside. */
    cycle_scope_start(16);
    if result.receipts_root != header.receipts_root then {
        fatal_error(InvalidReceiptsRoot)
    };
    if not_bool(block_logs_bloom_matches(result.logs, header.logs_bloom)) then {
        fatal_error(InvalidLogsBloom)
    };
    if profile.fork >= Amsterdam then {
        cycle_scope_start(20);
        validate_block_access_list(block.body.block_access_list, execution_profile.gas.block_limit);
        cycle_scope_end(20)
    };
    cycle_scope_end(16)
}

function verify_stateless_payload(input_ref : StatelessInputRef) -> unit = {
    var active_stage : validation_stage = VALIDATION_STAGE_DECODE_INPUT;
    cycle_scope_start(active_stage);
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
    let execution_profile = k_execution_profile;
    if execution_profile.protocol.fork >= Amsterdam then {
        initialize_block_access_list_state(
            block.body.block_access_list,
            k_parent_state_root,
            block.body.transactions.count,
        )
    };
    let executed = execute_block_body(block.body, input_ref);

    cycle_scope_end(active_stage);
    /* Receipts-trie reduction under the `receipts_root` phase tag, as a
       sibling of the `execute_block` phase.  The optimized receipt store
       reads its own C arena and ignores the start pointer, and block-end
       processing never touches retained receipts, so the root is identical
       to the one the non-profile build computes inside the transaction
       loop. */
    cycle_scope_start(16);
    let receipts_root = receipt_store_root(0, block.body.transactions.count);
    cycle_scope_end(16);
    let result = { executed with receipts_root = receipts_root };

    active_stage = VALIDATION_STAGE_VALIDATE_RESULT;
    cycle_scope_start(active_stage);
    validate_executed_block(block, result);
    cycle_scope_end(active_stage)
}
