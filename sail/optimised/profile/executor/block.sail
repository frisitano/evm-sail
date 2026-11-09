/* Optimized C overrides for `sail/executor/block.sail`. */

function execute_block_transactions(
    transactions : TransactionListRef,
    expected_deposits : StatelessInputSlice,
    expected_versioned_hashes : StatelessInputSlice,
) -> (
    BlockExecutionResult
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let gas_limits = execution_profile.gas;
    let (gas_limit as 'gas_limit) = gas_limits.block_limit;
    var gas_usage : BlockGasUsageFor('gas_limit) = block_gas_usage_empty(gas_limit);
    var blob_gas_acc : blob_gas_used = 0;
    var tx0_to : address = ZERO_ADDRESS;

    /* The optimized receipt store resets its own C arena here and ignores the
       returned start pointer at root time.  The profile overlay computes the
       receipts root in `verify_stateless_payload`, outside the `execute_block`
       phase, so only the reset side effect is needed. */
    let _ = receipt_store_begin();
    let logs_start = log_store_index_add(logs_tx_start(), logs_tx_count());
    var remaining_deposits : StatelessInputSlice = expected_deposits;
    var remaining_versioned_hashes : StatelessInputSlice = expected_versioned_hashes;
    var cursor = ssz_list_cursor(transactions);
    while not_bool(ssz_list_cursor_empty(cursor)) termination_measure(cursor.items.count - cursor.index) do {
        let i = cursor.index;
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        cycle_scope_start(10);
        let tx = decode_transaction(transaction);
        cycle_scope_end(10);
        remaining_versioned_hashes = consume_versioned_hashes(tx.blob_hashes, 0, remaining_versioned_hashes);
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
                fatal_error(GasUsedExceedsLimit)
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
                receipt_store_append(receipt, next_usage.receipts, i);
                if profile.fork >= Prague then {
                    remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits)
                };
                blob_gas_acc = next_blob_gas
            }
        } else if available_execution_gas < allowance.total then {
            fatal_error(GasUsedExceedsLimit)
        } else {
            let tx_blob_gas : transaction_blob_gas = sizeof(gas_per_blob_value) * tx.blob_hashes.count;
            let next_blob_gas : blob_gas_used =
                if profile.fork < Cancun
                then blob_gas_acc
                else block_blob_gas_add(profile.blob_schedule.max, blob_gas_acc, tx_blob_gas);
            let receipt = process_transaction(tx, allowance);
            let next_usage = block_gas_usage_add(usage, receipt.gas_used, 0, receipt.gas_used);
            gas_usage = next_usage;
            receipt_store_append(receipt, next_usage.receipts, i);
            if profile.fork >= Prague then {
                remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits)
            };
            blob_gas_acc = next_blob_gas
        }
    };
    let remaining_hashes = remaining_versioned_hashes;
    if remaining_hashes.len != 0 then {
        fatal_error(InvalidVersionedHashes)
    };
    if (profile.fork >= Prague) & (region_slice_length(remaining_deposits) != 0) then {
        fatal_error(InvalidExecutionRequests)
    };
    let final_usage = gas_usage;
    let header_gas_used =
        if (profile.fork >= Amsterdam) & (final_usage.execution < final_usage.state)
        then final_usage.state
        else final_usage.execution;
    let retained = log_store_index_add(logs_tx_start(), logs_tx_count());
    let logs_count : log_store_index =
        if logs_start <= retained then retained - logs_start else 0;

    /* Placeholder: the profile overlay defers the receipts-trie reduction to
       `verify_stateless_payload`, where it runs under the `receipts_root`
       phase scope as a sibling of `execute_block` (matching the aligned
       reth/ethrex guest instrumentation). */
    struct {
        header_gas_used = header_gas_used,
        execution_gas_used = final_usage.execution,
        state_gas_used = final_usage.state,
        blob_gas_used = blob_gas_acc,
        first_tx_recipient = tx0_to,
        receipts_root = ZERO_HASH,
        logs = struct { start = logs_start, count = logs_count },
    }
}

function execute_block_body(body : BlockBody, input_ref : StatelessInputRef) -> BlockExecutionResult = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    bal_reset();
    k_current_transaction_epoch = 0;
    warm_reset(k_current_transaction_epoch);
    cycle_scope_start(8);
    run_block_start_system_calls();
    cycle_scope_end(8);
    cycle_scope_start(9);
    let result = execute_block_transactions(body.transactions, input_ref.deposits, input_ref.versioned_hashes);
    cycle_scope_end(9);
    let post_tx_index = body.transactions.count + 1;
    k_current_transaction_epoch = post_tx_index;
    warm_reset(k_current_transaction_epoch);
    cycle_scope_start(17);
    apply_block_end_state(body);
    cycle_scope_end(17);
    cycle_scope_start(18);
    if profile.fork >= Prague then {
        validate_execution_requests(input_ref)
    };
    cycle_scope_end(18);
    result
}
