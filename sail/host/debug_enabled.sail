/*md
# Validation debug (enabled)

Build variant recording the failing validation stage and reason.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Whether a validation failure has been recorded for the current
    block. */
register validation_failure_present : bool = false
/*! The instrumentation scope that was active when the failure was
    recorded. */
register validation_failure_scope : bits(8) = SCOPE_STATELESS_VALIDATION
/*! The block-validation error recorded by the last failure. */
register validation_failure_reason : BlockError = InvalidConfig

/*! Clears the recorded failure at the start of a validation attempt. */
function validation_debug_reset() -> unit = {
    validation_failure_present = false
}

/*! Records the failing validation stage and reason for host
    inspection; a later record overwrites an earlier one. */
function validation_debug_record(scope : bits(8), reason : BlockError) -> unit = {
    validation_failure_present = true;
    validation_failure_scope = scope;
    validation_failure_reason = reason
}

/*! Rebuilds the storage root represented by the cached storage of `a`. */
function debug_account_storage_root(a : address) -> hash =
    match acct_block_get(a) {
        None() => EMPTY_TRIE_ROOT,
        Some(account) => trie_root(
            if account.storage_cleared then {
                EMPTY_TRIE_ROOT
            } else {
                account.info.storage_root
            },
            storage_updates(a),
        ),
    }

/*! Rebuilds the state root represented by all materialized block-layer rows. */
function debug_rebuild_state_root() -> hash = {
    var remaining : item_count = acct_block_count();
    var accounts : list(TrieUpdate) = [||];
    while remaining != 0 termination_measure(remaining) do {
        remaining = protocol_quantity_decrement(remaining);
        match acct_block_row(remaining) {
            Some(entry) => accounts = account_update(entry, storage_updates(entry.addr)) :: accounts,
            None() => throw InvalidBlock(WitnessDeficient),
        }
    };
    trie_root(EMPTY_TRIE_ROOT, accounts)
}
