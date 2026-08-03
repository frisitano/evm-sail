
/*md
# Stateless block validation

Validation of the commitments produced by executing a block body: gas and
blob-gas accounting, the post-state root, the receipts root and logs
bloom, the EIP-7685 execution requests, and the EIP-7928 block access
list.
*/
/*! Checks every executed-block commitment against the header and payload,
    throwing the specific `InvalidBlock` reason on the first failure:
    gas/blob-gas totals, post-state root, receipts root, logs bloom,
    execution-request bytes (Prague+), and block-access-list bytes and
    size (Amsterdam+). */
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
    let poststate = compute_state_root();
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
        validate_block_access_list(block.body.block_access_list, execution_profile.gas.block_limit)
    }
}

/* Stable stage identifiers for optional native validation diagnostics. These
   are validation metadata, not performance instrumentation. */
let VALIDATION_STAGE_DECODE_INPUT : validation_stage = 1
let VALIDATION_STAGE_INDEX_WITNESS : validation_stage = 2
let VALIDATION_STAGE_VALIDATE_PAYLOAD : validation_stage = 3
let VALIDATION_STAGE_EXECUTE_BLOCK : validation_stage = 4
let VALIDATION_STAGE_VALIDATE_RESULT : validation_stage = 5

/*! A validation failure: the pipeline stage it occurred in and the
    violated rule. */
struct StatelessValidationFailure = {
    stage : validation_stage,
    reason : BlockError,
}

/*! The verdict of stateless payload verification. */
union StatelessValidationResult = {
    /* every validation stage passed */
    StatelessPayloadValid : unit,
    /* the first failing stage and its reason */
    StatelessPayloadInvalid : StatelessValidationFailure,
}

/*! The stateless verification pipeline: decode the semantic envelope,
    index the witness, validate the payload commitments, execute the
    block body one transaction at a time, and validate the execution
    results; the first violated rule becomes the failure verdict. */
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
    }
}
