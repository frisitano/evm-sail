/* Optimized C overrides for `sail/evm/transaction.sail`. */

function process_transaction(tx, allowance) = {
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
