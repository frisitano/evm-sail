/*md
# The block driver

Block-start system calls, the transaction loop with block-gas
accounting, withdrawals, and block-end request collection.
*/

/*! Everything block validation needs from a successfully executed body: gas
    and blob-gas totals, receipts, and the collected EIP-7685 requests. */
struct BlockExecutionResult = {
    header_gas_used : gas,
    execution_gas_used : gas,
    state_gas_used : gas,
    blob_gas_used : blob_gas_used,
    first_tx_recipient : address,
    receipts_root : hash,
    logs_bloom : LogsBloom,
    requests : ExecutionRequests,
}

let PRE_MERGE_BLOCK_REWARD = unsigned(0x1bc16d674ec80000)

/*! Returns a block's remaining gas, rejecting an accumulated value above the
    header limit with the block-validation error required by EIP-7778. */
val remaining_block_gas : forall 'limit 'used,
    0 <= 'limit
    & 'limit <= block_gas_limit_bound
    & 0 <= 'used.
    (int('limit), int('used)) -> range(0, 'limit)
function remaining_block_gas(limit, used) =
    if used <= limit then {
        limit - used
    } else {
        throw InvalidBlock(GasUsedExceedsLimit)
    }

/*! The block-start writes: beacon root (Cancun+, EIP-4788) and parent
    hash history (Prague+, EIP-2935). */
function run_block_start_system_calls() -> unit = {
    if protocol_profile_fork(k_protocol_profile) >= Cancun then {
        system_call(BEACON_ROOTS_ADDR, k_header.parent_beacon_block_root)
    };
    if protocol_profile_fork(k_protocol_profile) >= Prague then {
        system_call(HISTORY_STORAGE_ADDR, k_header.parent_hash)
    }
}

/*! Executes the block's transactions in order, enforcing per-tx
    applicability and block gas/blob-gas availability (EIP-7778 block-gas
    accounting), accumulating receipts. */
function execute_block_transactions(
    transactions : TransactionListRef,
    public_keys : StatelessInputSlice,
    header_gas_limit : block_gas_limit,
    expected_deposits : StatelessInputSlice,
) -> (
    BlockExecutionResult
) = {
    let public_keys_length = public_keys.len;
    let public_key_length = PUBLIC_KEY_LENGTH;
    let public_key_count_value = public_keys_length / public_key_length;
    if (public_key_count_value != transactions.count) |
        (public_keys_length != public_key_count_value * public_key_length) then {
        throw InvalidBlock(WitnessDeficient)
    };
    let gas_limit = header_gas_limit;
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
        let tx = decode_transaction(transaction, public_key);
        k_block_access_index = i + 1;
        if i == 0 then {
            tx0_to = tx.recipient
        };
        let available_execution_gas = remaining_block_gas(gas_limit, execution_gas_acc);
        let available_state_gas = remaining_block_gas(gas_limit, state_gas_acc);
        let execution_limit = transaction_execution_limit(tx.gas_limit);
        let transaction_fits : bool = if protocol_profile_fork(k_protocol_profile) >= Amsterdam then {
                (execution_limit <= available_execution_gas) & (tx.gas_limit <= available_state_gas)
            } else {
                tx.gas_limit <= available_execution_gas
            };
        if not_bool(transaction_fits) then {
            throw InvalidBlock(GasUsedExceedsLimit)
        } else {
            let tx_blob_gas = transaction_blob_gas_for_count(tx.blob_hashes.count);
            let next_blob_gas : blob_gas_used = if protocol_profile_fork(k_protocol_profile) < Cancun then {
                    blob_gas_acc
                } else {
                    checked_block_blob_gas_add(blob_gas_acc, tx_blob_gas)
                };
            let receipt = process_transaction(tx);
            execution_gas_acc = conserved_gas_add(execution_gas_acc, receipt.execution_gas);
            state_gas_acc = conserved_gas_add(state_gas_acc, receipt.state_gas);
            if (gas_limit < execution_gas_acc) | (gas_limit < state_gas_acc) then {
                throw InvalidBlock(GasUsedExceedsLimit)
            };
            receipts = receipt_accumulator_push(receipts, receipt, next.index);
            if protocol_profile_fork(k_protocol_profile) >= Prague then {
                remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits)
            };
            blob_gas_acc = next_blob_gas
        }
    };
    if (protocol_profile_fork(k_protocol_profile) >= Prague) & (region_slice_length(remaining_deposits) != 0) then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    let header_gas_used = if (protocol_profile_fork(k_protocol_profile) >= Amsterdam) & (execution_gas_acc < state_gas_acc) then {
            state_gas_acc
        } else {
            execution_gas_acc
        };
    let receipts_root = receipt_accumulator_root(receipts);
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

/*! Credits every withdrawal's recipient with its amount in gwei
    (EIP-4895); withdrawals cannot fail and charge no gas. */
function apply_withdrawals(withdrawals : WithdrawalListRef) -> unit = {
    var rest = withdrawals;
    while rest.count != 0 termination_measure(rest.count) do {
        let (withdrawal_ref, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        let withdrawal = decode_withdrawal(withdrawal_ref);
        k_add_balance(withdrawal.address, alu_mul(word_of_withdrawal_amount(withdrawal.amount), 1000000000))
    }
}

/*! The block-end state effects: withdrawals (Shanghai+, EIP-4895), the
    pre-merge static block reward before Paris, and the final merge into
    the block layer. */
function apply_block_end_state(body : BlockBody) -> unit = {
    if protocol_profile_fork(k_protocol_profile) >= Shanghai then {
        apply_withdrawals(body.withdrawals)
    };
    if protocol_profile_fork(k_protocol_profile) < Paris then {
        k_add_balance(k_coinbase(), PRE_MERGE_BLOCK_REWARD)
    };
    k_tx_merge()
}

/*! Executes a block body end to end: block-start system calls, the
    transaction loop, block-end state effects, and request collection;
    invalid execution throws immediately, while successful execution returns
    the accumulated [BlockExecutionResult][type-BlockExecutionResult]. */
function execute_block_body(
    body : BlockBody,
    public_keys : StatelessInputSlice,
    header_gas_limit : block_gas_limit,
    expected_deposits : StatelessInputSlice,
) -> (
    BlockExecutionResult
) = {
    bal_reset();
    k_block_access_index = 0;
    run_block_start_system_calls();
    let result = execute_block_transactions(body.transactions, public_keys, header_gas_limit, expected_deposits);
    let post_tx_index = body.transactions.count + 1;
    k_block_access_index = post_tx_index;
    apply_block_end_state(body);
    let requests = if protocol_profile_fork(k_protocol_profile) >= Prague then {
            collect_execution_requests(expected_deposits)
        } else {
            EMPTY_EXECUTION_REQUESTS
        };
    { result with requests = requests }
}
