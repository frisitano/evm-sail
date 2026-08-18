/*md
# The block driver

Block-start system calls, the transaction loop with block-gas
accounting, withdrawals, and block-end request collection.
*/

/*! Everything block validation needs from a successfully executed body: gas
    and blob-gas totals, the post-execution receipts root, and the block's
    retained receipt-log range. EIP-7685 requests are validated where they
    are collected rather than carried in the result. */
struct BlockExecutionResult = {
    header_gas_used : block_gas,
    execution_gas_used : block_gas,
    state_gas_used : block_gas,
    blob_gas_used : blob_gas_used,
    first_tx_recipient : address,
    receipts_root : hash,
    logs : LogSeriesRef,
}

/*! The two independent block gas dimensions introduced by Amsterdam. Earlier
    forks use only `execution`; `state` remains zero. Indexing the accumulator
    by the concrete header limit makes an over-limit intermediate
    unrepresentable after transaction admission. */
type block_gas_usage_relation(
    'limit : Int,
    'execution : Int,
    'state : Int,
    'receipts : Int,
) -> Bool =
       0 <= 'limit
    &  'limit <= block_gas_limit_bound
    &  0 <= 'execution
    &  'execution <= 'limit
    &  0 <= 'state
    &  'state <= 'limit
    &  0 <= 'receipts
    &  'receipts <= 'execution + 'state

/*! The exact execution, state, and receipt gas accumulated under one block
    header gas limit. */
struct BlockGasUsageFields(
    'limit : Int,
    'execution : Int,
    'state : Int,
    'receipts : Int,
), block_gas_usage_relation('limit, 'execution, 'state, 'receipts) = {
    execution : int('execution),
    state : int('state),
    receipts : int('receipts),
}

/*! A block gas accumulator existentially hiding its current totals while
    retaining their relationship to the concrete header limit. */
type BlockGasUsageFor('limit : Int) = {
    'execution 'state 'receipts,
    block_gas_usage_relation('limit, 'execution, 'state, 'receipts).
    BlockGasUsageFields('limit, 'execution, 'state, 'receipts)
}

val block_gas_usage_empty : forall 'limit, 0 <= 'limit & 'limit <= block_gas_limit_bound. int('limit) -> BlockGasUsageFields(
        'limit,
        0,
        0,
        0,
    )
function block_gas_usage_empty(_limit) =
    struct { execution = 0, state = 0, receipts = 0 }

/*! Adds gas contributions whose transaction allowance was already checked
    against the exact remaining block budgets. No overflow/limit test is
    repeated here: the dependent preconditions are the admission proof. */
val block_gas_usage_add : forall 'limit 'execution 'state 'receipts 'add_execution 'add_state 'add_receipt,
                               block_gas_usage_relation('limit, 'execution, 'state, 'receipts)
                            &  0
                            <= 'add_execution
                            &  'add_execution
                            <= 'limit
                            -  'execution
                            &  0
                            <= 'add_state
                            &  'add_state
                            <= 'limit
                            -  'state
                            &  0
                            <= 'add_receipt
                            &  'add_receipt
                            <= 'add_execution
                            +  'add_state. (
        BlockGasUsageFields('limit, 'execution, 'state, 'receipts),
        int('add_execution),
        int('add_state),
        int('add_receipt),
    ) -> BlockGasUsageFields('limit, 'execution + 'add_execution, 'state + 'add_state, 'receipts + 'add_receipt)
function block_gas_usage_add(usage, add_execution, add_state, add_receipt) =
    struct {
        execution = usage.execution + add_execution,
        state = usage.state + add_state,
        receipts = usage.receipts + add_receipt,
    }

let PRE_MERGE_BLOCK_REWARD = unsigned(0x1bc16d674ec80000)

/*! The block-start writes: beacon root (Cancun+, EIP-4788) and parent
    hash history (Prague+, EIP-2935). */
function run_block_start_system_calls() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Cancun then {
        system_call(BEACON_ROOTS_ADDR, k_header.parent_beacon_block_root)
    };
    if profile.fork >= Prague then {
        system_call(HISTORY_STORAGE_ADDR, k_header.parent_hash)
    }
}

/*! Executes the block's transactions in order, enforcing per-tx
    applicability and block gas/blob-gas availability (EIP-7778 block-gas
    accounting), accumulating receipts. */
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
        fatal_error(WitnessDeficient)
    };
    let (gas_limit as 'gas_limit) = gas_limits.block_limit;
    var gas_usage : BlockGasUsageFor('gas_limit) = block_gas_usage_empty(gas_limit);
    var blob_gas_acc : blob_gas_used = 0;
    var tx0_to : address = ZERO_ADDRESS;
    let records_start = receipt_store_begin();
    let transaction_logs_start = logs_tx_start();
    let transaction_logs_count = logs_tx_count();
    let logs_start = log_store_index_add(transaction_logs_start, transaction_logs_count);
    var remaining_deposits : StatelessInputSlice = expected_deposits;
    var cursor = ssz_list_cursor(transactions);
    var keys : StatelessInputSlice = public_keys;
    let initial_cursor_empty = ssz_list_cursor_empty(cursor);
    var cursor_has_item : bool = not_bool(initial_cursor_empty);
    while cursor_has_item termination_measure(cursor.items.count - cursor.index) do {
        let i = cursor.index;
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        let keys_fields = keys;
        let keys_fields : StatelessInputSliceAtLeast(65) =
            if public_key_length <= keys_fields.len then keys_fields else fatal_error(WitnessDeficient);
        let public_key = sub_slice(keys_fields, 0, PUBLIC_KEY_LENGTH);
        keys = slice_suffix(keys_fields, public_key_length);
        let tx = decode_transaction(transaction, public_key);
        k_current_transaction_epoch = i + 1;
        if i == 0 then {
            tx0_to = tx.recipient
        };

        /* A local binding opens the existential accumulator, retaining its
           concrete execution/state indices for this admission step. */
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
        };

        let cursor_empty = ssz_list_cursor_empty(cursor);
        cursor_has_item = not_bool(cursor_empty)
    };
    let remaining_deposits_length = region_slice_length(remaining_deposits);
    if (profile.fork >= Prague) & (remaining_deposits_length != 0) then {
        fatal_error(InvalidExecutionRequests)
    };
    let final_usage = gas_usage;
    let header_gas_used =
        if (profile.fork >= Amsterdam) & (final_usage.execution < final_usage.state)
        then final_usage.state
        else final_usage.execution;
    let receipts_root = receipt_store_root(records_start, transactions.count);
    let retained_logs_start = logs_tx_start();
    let retained_logs_count = logs_tx_count();
    let retained = log_store_index_add(retained_logs_start, retained_logs_count);
    let logs_count : log_store_index =
        if logs_start <= retained then retained - logs_start else 0;
    struct {
        header_gas_used = header_gas_used,
        execution_gas_used = final_usage.execution,
        state_gas_used = final_usage.state,
        blob_gas_used = blob_gas_acc,
        first_tx_recipient = tx0_to,
        receipts_root = receipts_root,
        logs = struct { start = logs_start, count = logs_count },
    }
}

/*! Credits every withdrawal's recipient with its amount in gwei
    (EIP-4895); withdrawals cannot fail and charge no gas. */
function apply_withdrawals(withdrawals : WithdrawalListRef) -> unit = {
    var rest = withdrawals;
    while rest.count != 0 termination_measure(rest.count) do {
        let (withdrawal_ref, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        let withdrawal = decode_withdrawal(withdrawal_ref);
        let amount_in_wei = withdrawal.amount * 1000000000;
        k_add_balance(withdrawal.address, amount_in_wei)
    }
}

/*! The block-end state effects: withdrawals (Shanghai+, EIP-4895), the
    pre-merge static block reward before Paris, and the final merge into
    the block layer. */
function apply_block_end_state(body : BlockBody) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if profile.fork >= Shanghai then {
        apply_withdrawals(body.withdrawals)
    };
    if profile.fork < Paris then {
        let coinbase = k_coinbase();
        k_add_balance(coinbase, PRE_MERGE_BLOCK_REWARD)
    };
    k_tx_merge()
}

/*! Executes a block body end to end: block-start system calls, the
    transaction loop, block-end state effects, and request validation;
    invalid execution throws immediately, while successful execution returns
    the accumulated [BlockExecutionResult][type-BlockExecutionResult]. */
function execute_block_body(body : BlockBody, input_ref : StatelessInputRef) -> BlockExecutionResult = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    bal_reset();
    k_current_transaction_epoch = 0;
    warm_reset(k_current_transaction_epoch);
    run_block_start_system_calls();
    let result = execute_block_transactions(body.transactions, input_ref.public_keys, input_ref.deposits);
    let post_tx_index = body.transactions.count + 1;
    k_current_transaction_epoch = post_tx_index;
    warm_reset(k_current_transaction_epoch);
    apply_block_end_state(body);
    if profile.fork >= Prague then {
        validate_execution_requests(input_ref)
    };
    result
}
