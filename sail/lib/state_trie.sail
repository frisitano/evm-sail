/*md
# State trie

Authenticated state/storage traversal and post-state update assembly over the
shared MPT and state-leaf codec.
*/

/*! The witnessed account at address `a` under state root `root`,
    reading the secure trie at `keccak256(a)`; a walk that proves absence
    yields [EMPTY_ACCOUNT], whose `present` field is the absence witness. */
function stateless_account_by_key(root : hash, address_hash : hash) -> Account = {
    let path = path_new(address_hash, 64);
    let value = trie_lookup(root, path);
    if value.len == 0 then {
        EMPTY_ACCOUNT
    } else {
        let account_info = decode_state_account(value);
        account_from_info(account_info)
    }
}

/*! The witnessed storage value of `slot` under a storage root, reading
    the secure trie at `keccak256(slot)`; absent slots are zero. */
function stateless_storage_by_key(root : hash, slot_hash : hash) -> word = {
    let path = path_new(slot_hash, 64);
    let value = trie_lookup(root, path);
    if value.len == 0 then {
        ZERO_WORD
    } else {
        let encoded_value = rlp_single_ref(value);
        rlp_decode_u256(encoded_value)
    }
}

function storage_value_changed(value : StorageValue) -> bool =
    not_bool(value.curr == value.orig)

/*! Whether any persisted account field changed across the block. */
function account_value_changed(value : AcctValue) -> bool =
      not_bool(value.curr.info.nonce == value.orig.info.nonce)
    | not_bool(value.curr.info.balance == value.orig.info.balance)
    | not_bool(value.curr.info.storage_root == value.orig.info.storage_root)
    | not_bool(value.curr.info.code_hash == value.orig.info.code_hash)
    | not_bool(value.curr.present == value.orig.present)
    | not_bool(value.curr.storage_cleared == value.orig.storage_cleared)

/*! Converts one changed storage row into a secure-trie update. */
function storage_update(trie_entry : StorageTrieEntry) -> TrieUpdate = {
    let entry = trie_entry.entry;
    let key = path_new(trie_entry.slot_hash, 64);
    let value_is_zero = word_is_zero(entry.value.curr);
    let change =
        if value_is_zero then {
            TrieDelete()
        } else {
            let encoded_value = encode_storage_value(entry.value.curr);
            TriePut(encoded_value)
        };
    struct { key = key, change = change }
}

/*! Converts one account row and its recomputed storage root into a
    state-trie insertion or deletion. */
function account_update(trie_entry : AcctTrieEntry, storage_root : hash) -> TrieUpdate = {
    let entry = trie_entry.entry;
    let current = entry.value.curr;
    let key = path_new(trie_entry.address_hash, 64);
    let account_absent = not_bool(current.present);
    let account_empty = account_info_empty(current.info);
    if account_absent | account_empty then {
        struct { key = key, change = TrieDelete() }
    } else {
        let encoded_account = encode_state_account(current.info, storage_root);
        struct { key = key, change = TriePut(encoded_account) }
    }
}

/*! Pulls the next changed storage row and converts it to a trie update,
    skipping materialized read-only rows. */
function next_storage_trie_update(addr : address) -> TrieUpdateFetch = {
    var searching : bool = true;
    var result : TrieUpdateFetch = struct { available = false, update = EMPTY_TRIE_UPDATE };
    while searching termination_measure(2 ^ 64) do {
        let iterator_item = storage_block_iter_next(addr);
        match iterator_item {
            StorageBlockIterRow(trie_entry) => {
                let changed = storage_value_changed(trie_entry.entry.value);
                if changed then {
                    result = struct { available = true, update = storage_update(trie_entry) };
                    searching = false
                }
            },
            StorageBlockIterExhausted(_) => searching = false,
        }
    };
    result
}

/*! Computes one account update directly from its ordered storage-update
    source. Opening the cursor loads at most one changed storage row; that
    loaded item both proves whether storage changed and remains owned by the
    cursor when the generic trie reducer consumes it. Consequently post-storage
    roots need no auxiliary host cache or second account traversal. */
function account_trie_update(trie_entry : AcctTrieEntry) -> (TrieUpdate, bool) = {
    let entry = trie_entry.entry;
    let current = entry.value.curr;
    storage_block_iter_begin(entry.addr);
    let storage_source = StorageTrieUpdates(entry.addr);
    let storage_updates = trie_updates_begin(storage_source);
    let no_storage_changes = updates_empty(storage_updates);
    let storage_changed = not_bool(no_storage_changes);
    let base_storage_root =
        if current.storage_cleared then EMPTY_TRIE_ROOT else current.info.storage_root;
    let account_empty = account_info_empty(current.info);
    let account_nonempty = not_bool(account_empty);
    let storage_root =
        if current.present & account_nonempty & storage_changed
        then trie_root_cursor(base_storage_root, storage_updates).root
        else base_storage_root;
    let update = account_update(trie_entry, storage_root);
    let persisted_account_changed = account_value_changed(entry.value);
    (update, persisted_account_changed | storage_changed)
}
termination_measure account_trie_update(trie_entry) = 1024

/*! Pulls the next net-changed account update for the protocol state-root path.
    The host excludes read-only candidates; Sail skips reverted/no-op writes. */
function next_changed_account_trie_update() -> TrieUpdateFetch = {
    var searching : bool = true;
    var result : TrieUpdateFetch = struct { available = false, update = EMPTY_TRIE_UPDATE };
    while searching termination_measure(2 ^ 64) do {
        let iterator_item = acct_block_iter_next();
        match iterator_item {
            AcctBlockIterRow(entry) => {
                let (update, changed) = account_trie_update(entry);
                if changed then {
                    result = struct { available = true, update = update };
                    searching = false
                }
            },
            AcctBlockIterExhausted(_) => searching = false,
        }
    };
    result
}
termination_measure next_changed_account_trie_update() = 1024

/*! State-backed implementation of the generic trie's pull-source contract. */
function trie_update_source_next(source : TrieUpdateSource) -> TrieUpdateFetch =
    match source {
        StorageTrieUpdates(addr) => next_storage_trie_update(addr),
        ChangedAccountTrieUpdates() => next_changed_account_trie_update(),
    }
termination_measure trie_update_source_next(source) = 1024

/*! The post-state root: traverses every changed account in the kernel's
    block-level overlay, recomputes each touched account's storage root
    from its changed slots (zero-valued slots delete), re-encodes the
    account leaf (empty accounts delete, per EIP-161), and streams the
    ordered updates into the parent state root via [trie_root][]. */
function compute_state_root() -> hash = {
    acct_block_iter_begin();
    let updates = ChangedAccountTrieUpdates();
    trie_root(k_parent_state_root, updates).root
}
