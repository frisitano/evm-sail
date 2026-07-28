/*md
# Validation debug (enabled)

Build variant recording the failing validation stage and reason.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Debug-only traversal of every cached block-layer account row. */
val acct_debug_iter_begin = impure { c: "acct_debug_iter_begin" } : unit -> unit
/*! Returns the next cached account row from the debug-only traversal. */
val acct_debug_iter_next = impure { c: "acct_debug_iter_next" } : unit -> option(AcctTrieEntry)

/*! Pulls one all-cached account update for the debug empty-base rebuild. */
function cached_account_trie_update_next() -> option(TrieUpdate) =
    match acct_debug_iter_next() {
        Some(entry) => {
            Some(account_update(entry, acct_post_storage_root_read(entry.entry.addr)))
        },
        None() => None(),
    }

/*! Prepares post-state storage roots for the debug-only all-cached traversal. */
function debug_prepare_account_post_storage_roots() -> unit = {
    var preparing : bool = true;
    while preparing termination_measure(2 ^ 64) do {
        match acct_debug_iter_next() {
            Some(entry) => prepare_account_post_storage_root(entry),
            None() => preparing = false,
        }
    }
}

/*! Whether a validation failure has been recorded for the current
    block. */
register validation_failure_present : bool = false
/*! The validation stage that was active when the failure was recorded. */
register validation_failure_scope : validation_stage = 0
/*! The block-validation error recorded by the last failure. */
register validation_failure_reason : BlockError = InvalidConfig

/*! Captures Sail's current throw site for the native failure dump. */
val validation_debug_capture_location = impure { c: "validation_debug_capture_location" } : unit -> unit

/*! Clears the recorded failure at the start of a validation attempt. */
function validation_debug_reset() -> unit = {
    validation_failure_present = false
}

/*! Records the failing validation stage and reason for host
    inspection; a later record overwrites an earlier one. */
function validation_debug_record(stage : validation_stage, reason : BlockError) -> unit = {
    validation_failure_present = true;
    validation_failure_scope = stage;
    validation_failure_reason = reason;
    validation_debug_capture_location()
}

/*! Rebuilds the storage root represented by the cached storage of `a`. */
function debug_account_storage_root(a : address) -> hash = {
    storage_block_iter_begin(a);
    match acct_block_get(a) {
        None() => EMPTY_TRIE_ROOT,
        Some(account) => trie_root(
            if account.storage_cleared then {
                EMPTY_TRIE_ROOT
            } else {
                account.info.storage_root
            },
            StorageTrieUpdates(a),
        ).root,
    }
}

/*! Rebuilds the state root represented by all materialized block-layer rows. */
function debug_rebuild_state_root() -> hash = {
    acct_debug_iter_begin();
    debug_prepare_account_post_storage_roots();
    acct_debug_iter_begin();
    trie_root(EMPTY_TRIE_ROOT, CachedAccountTrieUpdates()).root
}
