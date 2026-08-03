/* Optimized C overrides for `sail/executor/block.sail`. */

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
    let (gas_limit as 'gas_limit) = gas_limits.block_limit;
    var gas_usage : BlockGasUsageFor('gas_limit) = block_gas_usage_empty(gas_limit);
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
        k_current_transaction_epoch = i + 1;
        if i == 0 then {
            tx0_to = tx.recipient
        };
        let usage = gas_usage;
        let available_execution_gas = gas_limit - usage.execution;
        let available_state_gas = gas_limit - usage.state;
        let allowance = transaction_gas_allowance(
            tx.gas_limit,
            gas_limits.transaction_total_limit,
            gas_limits.transaction_regular_limit,
        );
        if profile.fork >= Amsterdam then {
            if (available_execution_gas < allowance.regular) | (available_state_gas < allowance.total) then {
                throw InvalidBlock(GasUsedExceedsLimit)
            } else {
                let tx_blob_gas : transaction_blob_gas = sizeof(gas_per_blob_value) * tx.blob_hashes.count;
                let next_blob_gas : blob_gas_used = block_blob_gas_add(
                    profile.blob_schedule.max,
                    blob_gas_acc,
                    tx_blob_gas,
                );
                let receipt = process_transaction(tx, allowance);
                let next_usage = block_gas_usage_add(usage, receipt.execution_gas, receipt.state_gas, receipt.gas_used);
                gas_usage = next_usage;
                receipts = receipt_accumulator_push(receipts, receipt, next_usage.receipts, next.index);
                if profile.fork >= Prague then {
                    remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits)
                };
                blob_gas_acc = next_blob_gas
            }
        } else {
            if available_execution_gas < allowance.total then {
                throw InvalidBlock(GasUsedExceedsLimit)
            } else {
                let tx_blob_gas : transaction_blob_gas = sizeof(gas_per_blob_value) * tx.blob_hashes.count;
                let next_blob_gas : blob_gas_used = if profile.fork < Cancun then {
                        blob_gas_acc
                    } else {
                        block_blob_gas_add(profile.blob_schedule.max, blob_gas_acc, tx_blob_gas)
                    };
                let receipt = process_transaction(tx, allowance);
                let next_usage = block_gas_usage_add(usage, receipt.gas_used, 0, receipt.gas_used);
                gas_usage = next_usage;
                receipts = receipt_accumulator_push(receipts, receipt, next_usage.receipts, next.index);
                if profile.fork >= Prague then {
                    remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits)
                };
                blob_gas_acc = next_blob_gas
            }
        }
    };
    if (profile.fork >= Prague) & (region_slice_length(remaining_deposits) != 0) then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    let final_usage = gas_usage;
    let header_gas_used = if (profile.fork >= Amsterdam) & (final_usage.execution < final_usage.state) then {
            final_usage.state
        } else {
            final_usage.execution
        };
    cycle_scope_start(16);
    let receipts_root = receipt_accumulator_root(receipts);
    cycle_scope_end(16);
    struct {
        header_gas_used = header_gas_used,
        execution_gas_used = final_usage.execution,
        state_gas_used = final_usage.state,
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
    k_current_transaction_epoch = 0;
    warm_reset(k_current_transaction_epoch);
    cycle_scope_start(8);
    run_block_start_system_calls();
    cycle_scope_end(8);
    cycle_scope_start(9);
    let result = execute_block_transactions(body.transactions, public_keys, expected_deposits);
    cycle_scope_end(9);
    let post_tx_index = body.transactions.count + 1;
    k_current_transaction_epoch = post_tx_index;
    warm_reset(k_current_transaction_epoch);
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
