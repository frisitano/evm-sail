register validation_failure_present : bool = false
register validation_failure_scope : bits(8) = SCOPE_STATELESS_VALIDATION
register validation_failure_reason : BlockError = InvalidConfig

function validation_debug_reset() -> unit = {
    validation_failure_present = false
}

function validation_debug_record(scope : bits(8), reason : BlockError) -> unit = {
    validation_failure_present = true;
    validation_failure_scope = scope;
    validation_failure_reason = reason
}

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
