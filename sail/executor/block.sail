/*md
# The block driver

Block-start system calls, the transaction loop with block-gas
accounting, withdrawals, and block-end request collection.
*/

/*! Everything block validation needs from an executed body: overall
    success, gas and blob-gas totals, receipts, and the collected
    EIP-7685 requests. */
struct BlockExecutionResult = {
    all_ok : bool,
    gas_acc : gas,
    blob_gas_acc : blob_gas,
    first_tx_recipient : address,
    block_gas_overflow : bool,
    blob_gas_overflow : bool,
    receipts_root : hash,
    logs_bloom : LogsBloom,
    deposits : ByteSlice,
    requests : ExecutionRequests,
}

/*! The block-start writes: beacon root (Cancun+, EIP-4788) and parent
    hash history (Prague+, EIP-2935). */
function run_block_start_system_calls() -> unit = {
    if k_fork >= Cancun then {
        system_call(BEACON_ROOTS_ADDR, k_header.parent_beacon_block_root)
    };
    if k_fork >= Prague then {
        system_call(HISTORY_STORAGE_ADDR, k_header.parent_hash)
    }
}

/*! Executes the block's transactions in order, enforcing per-tx
    applicability and block gas/blob-gas availability (EIP-7778 block-gas
    accounting), accumulating receipts. */
function execute_block_transactions(
    transactions : SszListRef,
    public_keys : ByteSlice,
    block_gas_limit : gas,
) -> (
    BlockExecutionResult
) = {
    let public_key_count = public_keys.len / PUBLIC_KEY_LENGTH;
    let ByteQuantity(public_key_count_value) = public_key_count;
    if (public_key_count_value != transactions.count) | (public_keys.len != public_key_count * PUBLIC_KEY_LENGTH) then {
        throw InvalidBlock(WitnessDeficient)
    };
    var all_ok : bool = true;
    let gas_limit = block_gas_limit;
    var gas_acc : gas = GAS_ZERO;
    var blob_gas_acc : blob_gas = 0;
    var tx0_to : address = ZERO_ADDR;
    var block_gas_overflow : bool = false;
    var blob_gas_overflow : bool = false;
    var receipts = receipt_accumulator_empty();
    let deposits_start = scratch_begin();
    var cursor = ssz_list_cursor(transactions);
    var keys = public_keys;
    while not_bool(ssz_list_cursor_empty(cursor)) termination_measure(cursor.items.count - cursor.index) do {
        let i = cursor.index;
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        let public_key = sub_slice(keys, BYTE_ZERO, PUBLIC_KEY_LENGTH);
        keys = sub_slice(keys, PUBLIC_KEY_LENGTH, keys.len - PUBLIC_KEY_LENGTH);
        let tx = decode_transaction(transaction, public_key);
        bal_set_index(protocol_quantity_increment(i));
        if not_bool(block_gas_overflow | blob_gas_overflow) then {
            if i == 0 then {
                tx0_to = tx.recipient
            };
            if gas_limit < gas_acc then {
                block_gas_overflow = true
            } else {
                let available_gas = gas_sub(gas_limit, gas_acc);
                if available_gas < tx.gas_limit then {
                    block_gas_overflow = true
                } else {
                    let tx_blob_gas_value = tx.blob_hashes.count * GAS_PER_BLOB;
                    if tx_blob_gas_value > 2 ^ 64 - 1 then {
                        blob_gas_overflow = true
                    } else {
                        let tx_blob_gas : blob_gas = tx_blob_gas_value;
                        let blob_capacity_ok : bool = if k_fork < Cancun then {
                                true
                            } else {
                                let maximum = blob_max_gas_per_block();
                                if blob_gas_acc <= maximum then {
                                    tx_blob_gas <= maximum - blob_gas_acc
                                } else {
                                    false
                                }
                            };
                        if not_bool(blob_capacity_ok) then {
                            blob_gas_overflow = true
                        } else {
                            let receipt = process_transaction(tx);
                            all_ok = all_ok & receipt.valid;
                            let Gas(accumulated) = gas_acc;
                            let Gas(used) = receipt.block_gas;
                            if used <= (2 ^ 63 - 1) - accumulated then {
                                gas_acc = Gas(accumulated + used);
                                block_gas_overflow = gas_limit < gas_acc
                            } else {
                                block_gas_overflow = true
                            };
                            if receipt.valid then {
                                receipts = receipt_accumulator_push(receipts, receipt);
                                append_deposit_logs(receipt.logs);
                                let next_blob_gas = blob_gas_acc + tx_blob_gas;
                                if next_blob_gas <= 2 ^ 64 - 1 then {
                                    blob_gas_acc = next_blob_gas
                                } else {
                                    blob_gas_overflow = true
                                }
                            }
                        }
                    }
                }
            }
        }
    };
    struct {
        all_ok = all_ok & not_bool(block_gas_overflow) & not_bool(blob_gas_overflow),
        gas_acc = gas_acc,
        blob_gas_acc = blob_gas_acc,
        first_tx_recipient = tx0_to,
        block_gas_overflow = block_gas_overflow,
        blob_gas_overflow = blob_gas_overflow,
        receipts_root = receipt_accumulator_root(receipts),
        logs_bloom = receipts.bloom,
        deposits = scratch_finish(deposits_start),
        requests = EMPTY_EXECUTION_REQUESTS,
    }
}

/*! Credits every withdrawal's recipient with its amount in gwei
    (EIP-4895); withdrawals cannot fail and charge no gas. */
function apply_withdrawals(withdrawals : SszListRef) -> unit = {
    var rest = withdrawals;
    while rest.count != 0 termination_measure(rest.count) do {
        let (withdrawal_ref, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        let withdrawal = decode_withdrawal(withdrawal_ref);
        k_add_balance(withdrawal.address, alu_mul(word_of_nat(withdrawal.amount), word_of_nat(1000000000)))
    }
}

/*! Runs the checked block-end request collection (Prague+, EIP-7685);
    skipped entirely when the body already failed. */
function run_checked_block_end_system_calls(all_ok : bool, deposits : ByteSlice) -> (bool, ExecutionRequests) = {
    if not_bool(all_ok) then {
        (false, EMPTY_EXECUTION_REQUESTS)
    } else {
        if k_fork >= Prague then {
            collect_execution_requests(deposits)
        } else {
            (true, EMPTY_EXECUTION_REQUESTS)
        }
    }
}

/*! The block-end state effects: withdrawals (Shanghai+, EIP-4895), the
    pre-merge static block reward before Paris, and the final merge into
    the block layer. */
function apply_block_end_state(body : BlockBody) -> unit = {
    if k_fork >= Shanghai then {
        apply_withdrawals(body.withdrawals)
    };
    if k_fork < Paris then {
        k_add_balance(k_coinbase(), word_of_nat(2000000000000000000))
    };
    k_tx_merge()
}

/*! Executes a block body end to end: block-start system calls, the
    transaction loop, block-end state effects, and request collection;
    returns overall success with the accumulated
    [BlockExecutionResult][type-BlockExecutionResult]. */
function execute_block_body(
    body : BlockBody,
    public_keys : ByteSlice,
    block_gas_limit : gas,
) -> (
    (bool, BlockExecutionResult)
) = {
    bal_reset();
    bal_set_index(0);
    run_block_start_system_calls();
    let result = execute_block_transactions(body.transactions, public_keys, block_gas_limit);
    let post_tx_index : item_index = if body.transactions.count < 2 ^ 64 - 1 then {
            protocol_quantity_increment(body.transactions.count)
        } else {
            throw InvalidBlock(WitnessDeficient)
        };
    bal_set_index(post_tx_index);
    apply_block_end_state(body);
    let (exec_ok, requests) = run_checked_block_end_system_calls(result.all_ok, result.deposits);
    (exec_ok, { result with requests = requests })
}
