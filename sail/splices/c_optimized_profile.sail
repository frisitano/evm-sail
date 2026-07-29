/*
 * Optional cycle-scope overlay for optimized C guests.
 *
 * The executable specification contains no performance instrumentation.
 * Profile builds apply this splice after c_optimized.sail, replacing only
 * orchestration functions at useful measurement boundaries. Scope numbers
 * remain stable because benchmark profiles persist them outside the model.
 */

type cycle_scope_id = range(0, 255)

val cycle_scope_start = impure { c: "evmsail_cycle_scope_start" } : cycle_scope_id -> unit
val cycle_scope_end = impure { c: "evmsail_cycle_scope_end" } : cycle_scope_id -> unit

function main() -> unit = {
    cycle_scope_start(0);
    scratch_reset();
    validation_debug_reset();
    let result : option(GuestValidation) = try {
        let input_ref = decode_stateless_input_ref(stateless_input());
        let validation : StatelessValidationResult = verify_stateless_payload(input_ref);
        let valid : bool = match validation {
            StatelessPayloadValid() => true,
            StatelessPayloadInvalid(failure) => {
                validation_debug_record(failure.stage, failure.reason);
                false
            },
        };
        Some(struct { input_ref = input_ref, valid = valid })
    } catch {
        InvalidBlock(reason) => {
            validation_debug_record(VALIDATION_STAGE_DECODE_INPUT, reason);
            None()
        },
        Overflow() => None(),
    };
    match result {
        Some(result) => write_validation_result(result.input_ref, result.valid),
        None() => write_invalid_result(),
    };
    cycle_scope_end(0)
}

function index_execution_witness(input_ref : StatelessInputRef) -> WitnessContext = {
    nodedb_reset();
    cycle_scope_start(38);
    index_witness_nodes(input_ref.witness_state);
    cycle_scope_end(38);
    cycle_scope_start(39);
    index_witness_codes(input_ref.witness_codes);
    cycle_scope_end(39);
    cycle_scope_start(40);
    let witness = index_witness_headers(input_ref.witness_headers);
    cycle_scope_end(40);
    k_parent_state_root = witness.parent_state_root;
    witness
}

function system_call(tgt : address, input : hash) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        ()
    } else {
        memory_reset();
        let input_range : MemoryRange = memory_range(0, SYSTEM_CALL_INPUT_LENGTH);
        let _ = memory_expand_to(input_range.len);
        mem_store_word(input_range.off, hash_to_word(input));
        let input_slice = active_memory_slice(input_range.off, input_range.len);
        let parent_memory = memory_frame_enter();
        let checkpoint = enter_system_call_frame(tgt, MemoryCalldata(memory_slice(input_slice.off, input_slice.len)));
        cycle_scope_start(50);
        let _ = interpret();
        cycle_scope_end(50);
        memory_frame_leave(parent_memory);
        if not_bool(frame_succeeded()) then {
            k_revert(checkpoint)
        };
        cycle_scope_start(51);
        k_tx_merge();
        cycle_scope_end(51)
    }
}

function system_call_checked(tgt : address) -> ScratchSlice = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        throw InvalidBlock(ExecutionInvalid)
    } else {
        memory_reset();
        let parent_memory = memory_frame_enter();
        let checkpoint = enter_system_call_frame(tgt, EMPTY_CALLDATA);
        cycle_scope_start(50);
        let output = interpret();
        cycle_scope_end(50);
        if frame_succeeded() then {
            let start = scratch_reserve(output.len);
            scratch_push_slice(output);
            let result = scratch_finish(start);
            memory_frame_leave(parent_memory);
            cycle_scope_start(51);
            k_tx_merge();
            cycle_scope_end(51);
            result
        } else {
            memory_frame_leave(parent_memory);
            k_revert(checkpoint);
            cycle_scope_start(51);
            k_tx_merge();
            cycle_scope_end(51);
            throw InvalidBlock(ExecutionInvalid)
        }
    }
}

function collect_execution_requests(deposits : StatelessInputSlice) -> ExecutionRequests = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    cycle_scope_start(41);
    let withdrawals = system_call_checked(WITHDRAWAL_REQUEST_ADDR);
    cycle_scope_end(41);
    cycle_scope_start(42);
    let consolidations = system_call_checked(CONSOLIDATION_REQUEST_ADDR);
    cycle_scope_end(42);
    cycle_scope_start(43);
    let builder_deposits : ScratchSlice = if profile.fork >= Amsterdam then {
            system_call_checked(BUILDER_DEPOSIT_REQUEST_ADDR)
        } else {
            EMPTY_SCRATCH_SLICE
        };
    cycle_scope_end(43);
    cycle_scope_start(44);
    let builder_exits : ScratchSlice = if profile.fork >= Amsterdam then {
            system_call_checked(BUILDER_EXIT_REQUEST_ADDR)
        } else {
            EMPTY_SCRATCH_SLICE
        };
    cycle_scope_end(44);
    struct {
        deposits = deposits,
        withdrawals = withdrawals,
        consolidations = consolidations,
        builder_deposits = builder_deposits,
        builder_exits = builder_exits,
    }
}

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
        let maximum_items : item_count = header.gas_limit / 2000;
        cycle_scope_start(20);
        validate_block_access_list(block.body.block_access_list, maximum_items);
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
        let result = execute_block_body(
            block.body,
            input_ref.public_keys,
            input_ref.deposits,
        );

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

function write_validation_result(input_ref : StatelessInputRef, success : bool) -> unit = {
    cycle_scope_start(6);
    let root = htr_new_payload_request(input_ref);
    cycle_scope_end(6);
    cycle_scope_start(7);
    commit_validation_result(root, success, input_ref.chain_config);
    cycle_scope_end(7)
}

function write_invalid_result() -> unit = {
    cycle_scope_start(7);
    let start = scratch_reserve(24);
    var default_chain_config = ZERO_HASH;
    default_chain_config[8] = 0x0c;
    default_chain_config[12] = 0x04;
    default_chain_config[16] = 0x08;
    default_chain_config[20] = 0x08;
    scratch_push_b256(default_chain_config, 24);
    let chain_config = scratch_finish(start);
    let fixed_length = WORD_BYTE_LENGTH + RESULT_METADATA_LENGTH;
    let output_start = scratch_reserve(scratch_length_add(fixed_length, chain_config.len));
    result_prefix(ZERO_HASH, false);
    scratch_push_slice(chain_config);
    assert(public_output_write(scratch_finish(output_start)), "public output write");
    cycle_scope_end(7)
}

function execute_block_transactions(
    transactions : TransactionListRef,
    public_keys : StatelessInputSlice,
    expected_deposits : StatelessInputSlice,
) -> (
    BlockExecutionResult
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let gas_limits = execution_profile.gas;
    let public_keys_length = public_keys.len;
    let public_key_length = PUBLIC_KEY_LENGTH;
    let public_key_count_value = public_keys_length / public_key_length;
    if (public_key_count_value != transactions.count) |
        (public_keys_length != public_key_count_value * public_key_length) then {
        throw InvalidBlock(WitnessDeficient)
    };
    let gas_limit = gas_limits.block_limit;
    var execution_gas_acc : gas = GAS_ZERO;
    var state_gas_acc : gas = GAS_ZERO;
    var blob_gas_acc : blob_gas_used = 0;
    var tx0_to : address = ZERO_ADDRESS;
    var receipts = receipt_accumulator_empty();
    var remaining_deposits : StatelessInputSlice = expected_deposits;
    var cursor = ssz_list_cursor(transactions);
    var keys : StatelessInputSlice = public_keys;
    while not_bool(ssz_list_cursor_empty(cursor)) termination_measure(cursor.items.count - cursor.index) do {
        let i = cursor.index;
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        let keys_fields = keys;
        let keys_length = keys_fields.len;
        if keys_length < public_key_length then {
            throw InvalidBlock(WitnessDeficient)
        };
        let public_key = sub_slice(keys_fields, 0, PUBLIC_KEY_LENGTH);
        keys = slice_suffix(keys_fields, public_key_length);
        cycle_scope_start(10);
        let tx = decode_transaction(transaction, public_key);
        cycle_scope_end(10);
        k_block_access_index = i + 1;
        if i == 0 then {
            tx0_to = tx.recipient
        };
        let available_execution_gas = remaining_block_gas(gas_limit, execution_gas_acc);
        let available_state_gas = remaining_block_gas(gas_limit, state_gas_acc);
        let allowance = transaction_gas_allowance(
            tx.gas_limit,
            gas_limits.transaction_total_limit,
            gas_limits.transaction_regular_limit,
        );
        let transaction_fits : bool = if profile.fork >= Amsterdam then {
                (allowance.regular <= available_execution_gas) &
                (allowance.total <= available_state_gas)
            } else {
                allowance.total <= available_execution_gas
            };
        if not_bool(transaction_fits) then {
            throw InvalidBlock(GasUsedExceedsLimit)
        } else {
            let tx_blob_gas : transaction_blob_gas =
                sizeof(gas_per_blob_value) * tx.blob_hashes.count;
            let next_blob_gas : blob_gas_used = if profile.fork < Cancun then {
                    blob_gas_acc
                } else {
                    block_blob_gas_add(
                        profile.blob_schedule.max,
                        blob_gas_acc,
                        tx_blob_gas,
                    )
                };
            let receipt = process_transaction(tx, allowance);
            execution_gas_acc = conserved_gas_add(execution_gas_acc, receipt.execution_gas);
            state_gas_acc = conserved_gas_add(state_gas_acc, receipt.state_gas);
            if (gas_limit < execution_gas_acc) | (gas_limit < state_gas_acc) then {
                throw InvalidBlock(GasUsedExceedsLimit)
            };
            receipts = receipt_accumulator_push(receipts, receipt, next.index);
            if profile.fork >= Prague then {
                remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits)
            };
            blob_gas_acc = next_blob_gas
        }
    };
    if (profile.fork >= Prague) & (region_slice_length(remaining_deposits) != 0) then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    let header_gas_used = if (profile.fork >= Amsterdam) & (execution_gas_acc < state_gas_acc) then {
            state_gas_acc
        } else {
            execution_gas_acc
        };
    cycle_scope_start(16);
    let receipts_root = receipt_accumulator_root(receipts);
    cycle_scope_end(16);
    struct {
        header_gas_used = header_gas_used,
        execution_gas_used = execution_gas_acc,
        state_gas_used = state_gas_acc,
        blob_gas_used = blob_gas_acc,
        first_tx_recipient = tx0_to,
        receipts_root = receipts_root,
        logs_bloom = receipt_accumulator_bloom(receipts),
        requests = EMPTY_EXECUTION_REQUESTS,
    }
}

function execute_block_body(
    body : BlockBody,
    public_keys : StatelessInputSlice,
    expected_deposits : StatelessInputSlice,
) -> (
    BlockExecutionResult
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    bal_reset();
    k_block_access_index = 0;
    cycle_scope_start(8);
    run_block_start_system_calls();
    cycle_scope_end(8);
    cycle_scope_start(9);
    let result = execute_block_transactions(
        body.transactions,
        public_keys,
        expected_deposits,
    );
    cycle_scope_end(9);
    let post_tx_index = body.transactions.count + 1;
    k_block_access_index = post_tx_index;
    cycle_scope_start(17);
    apply_block_end_state(body);
    cycle_scope_end(17);
    cycle_scope_start(18);
    let requests = if profile.fork >= Prague then {
            collect_execution_requests(expected_deposits)
        } else {
            EMPTY_EXECUTION_REQUESTS
        };
    cycle_scope_end(18);
    { result with requests = requests }
}

function process_transaction(
    tx : Transaction,
    allowance : TransactionGasAllowance,
) -> Receipt = {
    cycle_scope_start(11);
    k_tx_reset();
    cycle_scope_end(11);
    cycle_scope_start(12);
    let validity = check_transaction_validity(tx, allowance);
    cycle_scope_end(12);
    k_set_tx(tx_env(tx.sender, validity.gas_price, tx.blob_hashes));
    cycle_scope_start(13);
    let upfront = apply_transaction_upfront_effects(tx, validity);
    cycle_scope_end(13);
    cycle_scope_start(14);
    let frame_result = run_transaction_frame(tx, validity, upfront);
    cycle_scope_end(14);
    cycle_scope_start(15);
    let receipt = settle_transaction(tx, validity, upfront.authorization_refund, frame_result);
    cycle_scope_end(15);
    receipt
}

function k_tx_merge() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    cycle_scope_start(47);
    optimized_tx_merge(profile.fork >= Cancun, profile.fork >= Amsterdam, k_block_access_index);
    cycle_scope_end(47)
}
