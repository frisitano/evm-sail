/* Optimized C overrides for `sail/kernel/lifecycle.sail`. */

/*
 * The standard lifecycle module keeps the explicit account-then-storage
 * transaction merge. Optimized C consumes the host-owned rows directly in
 * insertion order, avoiding generated aggregate construction and sorting.
 */
val optimized_tx_merge = impure { c: "tx_merge" } : (TransactionMergeSemantics, block_access_index) -> unit

function k_tx_merge() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let semantics = transaction_merge_semantics(profile.fork);
    optimized_tx_merge(semantics, k_current_transaction_epoch)
}
