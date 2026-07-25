
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
    let header = block.header;
    if k_fork >= Prague then {
        if   not_bool(bytes_segments_equal_slice([|BytesSlice(result.requests.deposits)|], input_ref.deposits))
           | not_bool(
                 bytes_segments_equal_slice([|BytesSlice(result.requests.withdrawals)|], input_ref.withdrawal_requests),
             )
           | not_bool(
                 bytes_segments_equal_slice(
                     [|BytesSlice(result.requests.consolidations)|],
                     input_ref.consolidation_requests,
                 ),
             )
           | not_bool(
                 bytes_segments_equal_slice(
                     [|BytesSlice(result.requests.builder_deposits)|],
                     input_ref.builder_deposit_requests,
                 ),
             )
           | not_bool(
                 bytes_segments_equal_slice(
                     [|BytesSlice(result.requests.builder_exits)|],
                     input_ref.builder_exit_requests,
                 ),
             ) then {
            throw InvalidBlock(InvalidExecutionRequests)
        }
    };
    if result.header_gas_used != header.gas_used then {
        throw InvalidBlock(InvalidGasUsed)
    };
    if (k_fork >= Cancun) & (result.blob_gas_used != header.blob_gas_used) then {
        throw InvalidBlock(InvalidBlobGasUsed)
    };
    cycle_scope_start(SCOPE_STATE_ROOT);
    let poststate = compute_state_root();
    cycle_scope_end(SCOPE_STATE_ROOT);
    if poststate != header.state_root then {
        throw InvalidBlock(InvalidStateRoot)
    };
    if result.receipts_root != header.receipts_root then {
        throw InvalidBlock(InvalidReceiptsRoot)
    };
    if not_bool(logs_bloom_equal(result.logs_bloom, header.logs_bloom)) then {
        throw InvalidBlock(InvalidLogsBloom)
    };
    if k_fork >= Amsterdam then {
        cycle_scope_start(SCOPE_BLOCK_ACCESS_LIST);
        let maximum_items : item_count = header.gas_limit / 2000;
        validate_block_access_list(block.body.block_access_list, maximum_items);
        cycle_scope_end(SCOPE_BLOCK_ACCESS_LIST)
    }
}

/*! A validation failure: the pipeline stage it occurred in and the
    violated rule. */
struct StatelessValidationFailure = {
    scope : bits(8),
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
    var active_scope = SCOPE_DECODE_INPUT;
    try {
        cycle_scope_start(active_scope);
        scratch_reset();
        let input = decode_stateless_input(input_ref);
        k_fork = input_ref.protocol.fork;

        cycle_scope_end(active_scope);
        active_scope = SCOPE_INDEX_WITNESS;
        cycle_scope_start(active_scope);
        let witness = index_execution_witness(input_ref);

        cycle_scope_end(active_scope);
        active_scope = SCOPE_VALIDATE_PAYLOAD;
        cycle_scope_start(active_scope);
        validate_execution_payload(input, input_ref, witness);

        cycle_scope_end(active_scope);
        active_scope = SCOPE_EXECUTE_BLOCK;
        cycle_scope_start(active_scope);
        let block = input.payload.block;
        let result = execute_block_body(block.body, input_ref.public_keys, block.header.gas_limit);

        cycle_scope_end(active_scope);
        active_scope = SCOPE_VALIDATE_RESULT;
        cycle_scope_start(active_scope);
        validate_executed_block(block, input_ref, result);
        cycle_scope_end(active_scope);
        StatelessPayloadValid()
    } catch {
        InvalidBlock(reason) => {
            cycle_scope_end(active_scope);
            StatelessPayloadInvalid(struct { scope = active_scope, reason = reason })
        },
    }
}
