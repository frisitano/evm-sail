struct BlockExecutionResult = {
    all_ok : bool,
    gas_acc : int,
    blob_gas_acc : int,
    first_tx_recipient : address,
    block_gas_overflow : bool,
    blob_gas_overflow : bool,
    receipts : list(Receipt),
}

function run_block_start_system_calls() -> unit = {
    system_call(BEACON_ROOTS_ADDR, k_header.parent_beacon_block_root);
    system_call(HISTORY_STORAGE_ADDR, k_header.parent_hash)
}

function execute_block_transactions(txs : list(Transaction), block_gas_limit : int) -> BlockExecutionResult = {
    var all_ok : bool = true;
    var gas_acc : int = 0;
    var blob_gas_acc : int = 0;
    var tx0_to : address = ZERO_ADDR;
    var block_gas_overflow : bool = false;
    var blob_gas_overflow : bool = false;
    var receipts_rev : list(Receipt) = [||];
    var rest : list(Transaction) = txs;
    var i : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            tx :: txs_rest => {
                rest = txs_rest;
                if not_bool(block_gas_overflow | blob_gas_overflow) then {
                    let tx_blob_gas = tx.blob_count * GAS_PER_BLOB;
                    if i == 0 then {
                        tx0_to = tx.recipient
                    };
                    if tx.gas_limit > block_gas_limit - gas_acc then {
                        block_gas_overflow = true
                    } else if tx_blob_gas > blob_max_gas_per_block() - blob_gas_acc then {
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
                };
                i = i + 1
            },
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
    }
}

function apply_withdrawals(withdrawals : list(Withdrawal)) -> unit = {
    var rest : list(Withdrawal) = withdrawals;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            withdrawal :: tail => {
                k_add_balance(withdrawal.address, word_of_int(withdrawal.amount * 1000000000));
                rest = tail
            },
        }
    }
}

function run_checked_block_end_system_calls(all_ok : bool) -> bool = {
    if all_ok then {
        let withdrawals_ok = system_call_checked(WITHDRAWAL_REQUEST_ADDR);
        let consolidations_ok = system_call_checked(CONSOLIDATION_REQUEST_ADDR);
        withdrawals_ok & consolidations_ok
    } else {
        false
    }
}

function execute_block_body(
    transactions : list(Transaction),
    withdrawals : list(Withdrawal),
    block_gas_limit : int,
) -> (bool, BlockExecutionResult) = {
    run_block_start_system_calls();
    let result = execute_block_transactions(transactions, block_gas_limit);
    apply_withdrawals(withdrawals);
    let exec_ok = run_checked_block_end_system_calls(result.all_ok);
    (exec_ok, result)
}
