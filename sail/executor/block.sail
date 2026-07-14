/*md
# Block execution

The block driver: block-start system calls, the transaction loop with
block-gas accounting, withdrawals, and block-end request collection.
*/

/*! Everything block validation needs from an executed body: overall
    success, gas and blob-gas totals, receipts, and the collected
    EIP-7685 requests. */
struct BlockExecutionResult = {
    all_ok : bool,
    gas_acc : int,
    blob_gas_acc : int,
    first_tx_recipient : address,
    block_gas_overflow : bool,
    blob_gas_overflow : bool,
    receipts : list(Receipt),
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
    accounting), accumulating receipts.

    *EELS*: `apply_body`. */
function execute_block_transactions(
    transactions : SszListRef,
    public_keys : ByteSlice,
    block_gas_limit : int,
) -> (
    BlockExecutionResult
) = {
    if unsigned(public_keys.len) != transactions.count * 65 then {
        throw InvalidBlock(WitnessDeficient)
    };
    var all_ok : bool = true;
    var gas_acc : int = 0;
    var blob_gas_acc : int = 0;
    var tx0_to : address = ZERO_ADDR;
    var block_gas_overflow : bool = false;
    var blob_gas_overflow : bool = false;
    var receipts_rev : list(Receipt) = [||];
    var cursor = ssz_list_cursor(transactions);
    var keys = public_keys;
    while not_bool(ssz_list_cursor_empty(cursor)) do {
        let i = cursor.index;
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        let public_key = sub_slice(keys, 0, 65);
        keys = sub_slice(keys, 65, unsigned(keys.len) - 65);
        let tx = decode_transaction(transaction, public_key);
        bal_set_index(get_slice_int(64, i + 1, 0));
        if not_bool(block_gas_overflow | blob_gas_overflow) then {
            let tx_blob_gas = tx.blob_hashes.count * GAS_PER_BLOB;
            if i == 0 then {
                tx0_to = tx.recipient
            };
            if tx.gas_limit > block_gas_limit - gas_acc then {
                block_gas_overflow = true
            } else {
                if (k_fork >= Cancun) & (tx_blob_gas > blob_max_gas_per_block() - blob_gas_acc) then {
                    blob_gas_overflow = true
                } else {
                    let receipt = process_transaction(tx);
                    all_ok = all_ok & receipt.valid;
                    gas_acc = gas_acc + receipt.block_gas;
                    if receipt.valid then {
                        receipts_rev = receipt :: receipts_rev;
                        blob_gas_acc = blob_gas_acc + tx_blob_gas
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
        receipts = list_reverse(receipts_rev),
        requests = EMPTY_EXECUTION_REQUESTS,
    }
}

function apply_withdrawals(withdrawals : SszListRef) -> unit = {
    var rest = withdrawals;
    while rest.count > 0 do {
        let (withdrawal_ref, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        let withdrawal = decode_withdrawal(withdrawal_ref);
        k_add_balance(withdrawal.address, word_of_int(withdrawal.amount * 1000000000))
    }
}

function run_checked_block_end_system_calls(all_ok : bool, receipts : list(Receipt)) -> (bool, ExecutionRequests) = {
    if not_bool(all_ok) then {
        (false, EMPTY_EXECUTION_REQUESTS)
    } else {
        if k_fork >= Prague then {
            collect_execution_requests(receipts)
        } else {
            (true, EMPTY_EXECUTION_REQUESTS)
        }
    }
}

function apply_block_end_state(body : BlockBody) -> unit = {
    if k_fork >= Shanghai then {
        apply_withdrawals(body.withdrawals)
    };
    if k_fork < Paris then {
        k_add_balance(k_coinbase(), word_of_int(2000000000000000000))
    };
    k_tx_merge()
}

function execute_block_body(
    body : BlockBody,
    public_keys : ByteSlice,
    block_gas_limit : int,
) -> (
    (bool, BlockExecutionResult)
) = {
    bal_reset();
    bal_set_index(0x0000000000000000);
    run_block_start_system_calls();
    let result = execute_block_transactions(body.transactions, public_keys, block_gas_limit);
    bal_set_index(get_slice_int(64, body.transactions.count + 1, 0));
    apply_block_end_state(body);
    let (exec_ok, requests) = run_checked_block_end_system_calls(result.all_ok, result.receipts);
    (exec_ok, { result with requests = requests })
}
