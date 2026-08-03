/* Optimized C overrides for `sail/kernel/lifecycle.sail`. */

function k_tx_merge() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let semantics = transaction_merge_semantics(profile.fork);
    cycle_scope_start(47);
    optimized_tx_merge(semantics, k_current_transaction_epoch);
    cycle_scope_end(47)
}
