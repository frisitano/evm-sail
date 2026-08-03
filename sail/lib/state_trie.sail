/*md
# State trie

Authenticated state/storage traversal and post-state update assembly over the
shared MPT and state-leaf codec.
*/

/*! The witnessed account at address `a` under state root `root`,
    reading the secure trie at `keccak256(a)`; `None` when the walk
    proves absence. */
function stateless_account_by_key(root : hash, address_hash : hash) -> option(AccountInfo) = {
    let value = trie_lookup(root, path_new(address_hash, 64));
    if value.len == 0 then {
        None()
    } else {
        Some(decode_state_account(value))
    }
}

/*! The witnessed storage value of `slot` under a storage root, reading
    the secure trie at `keccak256(slot)`; absent slots are zero. */
function stateless_storage_by_key(root : hash, slot_hash : hash) -> word = {
    let value = trie_lookup(root, path_new(slot_hash, 64));
    if value.len == 0 then {
        ZERO_WORD
    } else {
        rlp_decode_u256(rlp_single_ref(value))
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
    let change = if word_is_zero(entry.value.curr) then {
            TrieDelete()
        } else {
            TriePut(encode_storage_value(entry.value.curr))
        };
    struct { key = key, change = change }
}

/*! Converts one account row and its recomputed storage root into a
    state-trie insertion or deletion. */
function account_update(trie_entry : AcctTrieEntry, storage_root : hash) -> TrieUpdate = {
    let entry = trie_entry.entry;
    let current = entry.value.curr;
    let key = path_new(trie_entry.address_hash, 64);
    if not_bool(current.present) | account_info_empty(current.info) then {
        struct { key = key, change = TrieDelete() }
    } else {
        struct { key = key, change = TriePut(encode_state_account(current.info, storage_root)) }
    }
}

/*! Pulls the next changed storage row, skipping materialized read-only rows. */
function next_changed_storage_entry(addr : address) -> option(StorageTrieEntry) = {
    var searching : bool = true;
    var result : option(StorageTrieEntry) = None();
    while searching termination_measure(2 ^ 64) do {
        match storage_block_iter_next(addr) {
            Some(trie_entry) => if storage_value_changed(trie_entry.entry.value) then {
                result = Some(trie_entry);
                searching = false
            },
            None() => searching = false,
        }
    };
    result
}

/*! Converts the next changed storage row to a trie update. */
function next_storage_trie_update(addr : address) -> option(TrieUpdate) =
    match next_changed_storage_entry(addr) {
        Some(entry) => Some(storage_update(entry)),
        None() => None(),
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
    let storage_updates = trie_updates_begin(StorageTrieUpdates(entry.addr));
    let storage_changed = not_bool(updates_empty(storage_updates));
    let base_storage_root = if current.storage_cleared then {
            EMPTY_TRIE_ROOT
        } else {
            current.info.storage_root
        };
    let storage_root = if not_bool(current.present) | account_info_empty(current.info) then {
            base_storage_root
        } else if storage_changed then {
            trie_root_cursor(base_storage_root, storage_updates).root
        } else {
            base_storage_root
    };
    (account_update(trie_entry, storage_root), account_value_changed(entry.value) | storage_changed)
}

/*! Pulls the next net-changed account update for the protocol state-root path.
    The host excludes read-only candidates; Sail skips reverted/no-op writes. */
function next_changed_account_trie_update() -> option(TrieUpdate) = {
    var searching : bool = true;
    var result : option(TrieUpdate) = None();
    while searching termination_measure(2 ^ 64) do {
        match acct_block_iter_next() {
            Some(entry) => {
                let (update, changed) = account_trie_update(entry);
                if changed then {
                    result = Some(update);
                    searching = false
                }
            },
            None() => searching = false,
        }
    };
    result
}

/*! State-backed implementation of the generic trie's pull-source contract. */
function trie_update_source_next(source : TrieUpdateSource) -> option(TrieUpdate) =
    match source {
        StorageTrieUpdates(addr) => next_storage_trie_update(addr),
        ChangedAccountTrieUpdates() => next_changed_account_trie_update(),
    }

/*! The post-state root: traverses every changed account in the kernel's
    block-level overlay, recomputes each touched account's storage root
    from its changed slots (zero-valued slots delete), re-encodes the
    account leaf (empty accounts delete, per EIP-161), and streams the
    ordered updates into the parent state root via [trie_root][]. */
function compute_state_root() -> hash = {
    acct_block_iter_begin();
    trie_root(k_parent_state_root, ChangedAccountTrieUpdates()).root
}
