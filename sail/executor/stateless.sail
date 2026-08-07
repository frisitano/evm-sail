
/*md
# Stateless block validation

Validation of the commitments produced by executing a block body: gas and
blob-gas accounting, the post-state root, the receipts root and logs
bloom, the EIP-7685 execution requests, and the EIP-7928 block access
list.
*/
/*! Checks every executed-block commitment against the header and payload,
    throwing the specific `InvalidBlock` reason on the first failure:
    gas/blob-gas totals, post-state root, receipts root, logs bloom, and
    block-access-list bytes and size (Amsterdam+). Execution-request bytes
    (Prague+) are validated where they are collected. */
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
    let poststate = compute_state_root();
    if poststate != header.state_root then {
        fatal_error(InvalidStateRoot)
    };
    if result.receipts_root != header.receipts_root then {
        fatal_error(InvalidReceiptsRoot)
    };
    let logs_bloom_matches = block_logs_bloom_matches(result.logs, header.logs_bloom);
    let logs_bloom_mismatch = not_bool(logs_bloom_matches);
    if logs_bloom_mismatch then {
        fatal_error(InvalidLogsBloom)
    };
    if profile.fork >= Amsterdam then {
        validate_block_access_list(block.body.block_access_list, execution_profile.gas.block_limit)
    }
}

/*md
## Constants

Stable stage identifiers for optional native validation diagnostics. These
are validation metadata, not performance instrumentation.
*/
let VALIDATION_STAGE_DECODE_INPUT : validation_stage = 1
let VALIDATION_STAGE_INDEX_WITNESS : validation_stage = 2
let VALIDATION_STAGE_VALIDATE_PAYLOAD : validation_stage = 3
let VALIDATION_STAGE_EXECUTE_BLOCK : validation_stage = 4
let VALIDATION_STAGE_VALIDATE_RESULT : validation_stage = 5

/*md
## The verification pipeline

The single entry called for every stateless input.
*/
/*! The stateless verification pipeline: decode the semantic envelope,
    index the witness, validate the payload commitments, execute the
    block body one transaction at a time, and validate the execution results.
    Any failure terminates through `fatal_error`; normal return means valid. */
function verify_stateless_payload(input_ref : StatelessInputRef) -> unit = {
    scratch_reset();
    let input = decode_stateless_input(input_ref);
    let witness = index_execution_witness(input_ref);
    validate_execution_payload(input, input_ref, witness);
    let block = input.payload.block;
    let result = execute_block_body(block.body, input_ref);
    validate_executed_block(block, result)
}
