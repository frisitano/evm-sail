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
function validate_executed_block(
    block : Block,
    input_ref : StatelessInputRef,
    exec_ok : bool,
    result : BlockExecutionResult,
) -> (
    unit
) = {
    let header = block.header;
    let gas_used_ok = result.gas_acc == header.gas_used;
    let blob_gas_used_ok = (k_fork < Cancun) | (result.blob_gas_acc == unsigned(header.blob_gas_used));
    let poststate_ok = compute_state_root() == header.state_root;
    let receipts_root_ok = receipts_trie_root(result.receipts) == header.receipts_root;
    let logs_bloom_ok = logs_bloom_equal(receipts_bloom(result.receipts), header.logs_bloom);
    var block_access_list_size_ok : bool = true;
    var block_access_list_ok : bool = true;
    var execution_requests_ok : bool = true;
    if k_fork >= Prague then {
        execution_requests_ok =
              bytes_segments_equal_slice([|BytesSlice(result.requests.deposits)|], input_ref.deposits)
            & bytes_segments_equal_slice([|BytesSlice(result.requests.withdrawals)|], input_ref.withdrawal_requests)
            & bytes_segments_equal_slice(
                  [|BytesSlice(result.requests.consolidations)|],
                  input_ref.consolidation_requests,
              )
    };
    if k_fork >= Amsterdam then {
        let block_access_list = encode_block_access_list();
        block_access_list_size_ok = block_access_list.item_count <= ediv_int(header.gas_limit, 2000);
        block_access_list_ok = rlp_builder_equals_slice(block_access_list.rlp, block.body.block_access_list)
    };

    if result.block_gas_overflow then {
        throw InvalidBlock(GasUsedExceedsLimit)
    };
    if result.blob_gas_overflow then {
        throw InvalidBlock(BlobGasLimitExceeded)
    };
    if not_bool(exec_ok) then {
        throw InvalidBlock(ExecutionInvalid)
    };
    if not_bool(execution_requests_ok) then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    if not_bool(block_access_list_size_ok) then {
        throw InvalidBlock(BlockAccessListTooLarge)
    };
    if not_bool(gas_used_ok) then {
        throw InvalidBlock(InvalidGasUsed)
    };
    if not_bool(blob_gas_used_ok) then {
        throw InvalidBlock(InvalidBlobGasUsed)
    };
    if not_bool(poststate_ok) then {
        throw InvalidBlock(InvalidStateRoot)
    };
    if not_bool(receipts_root_ok) then {
        throw InvalidBlock(InvalidReceiptsRoot)
    };
    if not_bool(logs_bloom_ok) then {
        throw InvalidBlock(InvalidLogsBloom)
    };
    if not_bool(block_access_list_ok) then {
        throw InvalidBlock(InvalidBlockAccessList)
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
    StatelessPayloadValid : unit,
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
        k_fork = input.chain_config.fork;

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
        let (exec_ok, result) = execute_block_body(block.body, input_ref.public_keys, block.header.gas_limit);

        cycle_scope_end(active_scope);
        active_scope = SCOPE_VALIDATE_RESULT;
        cycle_scope_start(active_scope);
        validate_executed_block(block, input_ref, exec_ok, result);
        cycle_scope_end(active_scope);
        StatelessPayloadValid()
    } catch {
        InvalidBlock(reason) => {
            cycle_scope_end(active_scope);
            StatelessPayloadInvalid(struct { scope = active_scope, reason = reason })
        },
    }
}
