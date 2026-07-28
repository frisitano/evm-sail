
/*md
# The state trie

Ethereum account and storage tries over the generic MPT core: secure-trie
reads for stateless execution, and the post-state root computation
(YP §4.1).
*/
/*! Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
    code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
    root/hash fields decode to their empty-sentinel digests. */
function decode_state_account(value : StatelessInputSlice) -> AccountInfo = {
    let (nonce, fields) = rlp_cursor_pop(rlp_node_cursor(value));
    let (balance, fields) = rlp_cursor_pop(fields);
    let (storage, fields) = rlp_cursor_pop(fields);
    let (code, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    let storage_root = if storage.content_len == 0 then {
            EMPTY_TRIE_ROOT
        } else {
            word_to_hash(rlp_ref_word(storage))
        };
    let code_hash = if code.content_len == 0 then {
            KECCAK_EMPTY
        } else {
            word_to_hash(rlp_ref_word(code))
        };
    struct {
        nonce = rlp_ref_uint64(nonce),
        balance = rlp_ref_uint_word(balance),
        storage_root = storage_root,
        code_hash = code_hash,
    }
}

/*! The witnessed account at address `a` under state root `root`,
    reading the secure trie at `keccak256(a)`; `None` when the walk
    proves absence. */
function stateless_account_by_key(root : hash, address_hash : hash) -> option(AccountInfo) = {
    let value = trie_lookup(root, path_from_hash(address_hash));
    if value.len == 0 then {
        None()
    } else {
        Some(decode_state_account(value))
    }
}

/*! The witnessed storage value of `slot` under a storage root, reading
    the secure trie at `keccak256(slot)`; absent slots are zero. */
function stateless_storage_by_key(root : hash, slot_hash : hash) -> word = {
    let value = trie_lookup(root, path_from_hash(slot_hash));
    if value.len == 0 then {
        ZERO_WORD
    } else {
        rlp_ref_uint_word(rlp_single_ref(value))
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

/*! Encodes a nonzero storage value as its minimal RLP integer leaf payload. */
function encode_storage_value(value : word) -> ScratchSlice = {
    let encoded_len = rlp_uint_word_size(value);
    let start = scratch_reserve(encoded_len);
    rlp_write_uint_word(value);
    rlp_finish(start)
}

/*! Encodes an account trie leaf with its recomputed storage root. */
function encode_state_account(info : AccountInfo, storage_root : hash) -> ScratchSlice = {
    let nonce_length = rlp_uint_size(info.nonce);
    let balance_length = rlp_uint_word_size(info.balance);
    let storage_root_length = rlp_word_size();
    let code_hash_length = rlp_word_size();
    let content_len = nonce_length + balance_length + storage_root_length + code_hash_length;
    let start = scratch_reserve(rlp_list_size(content_len));
    rlp_write_list_prefix(content_len);
    rlp_write_uint(info.nonce);
    rlp_write_uint_word(info.balance);
    rlp_write_word(hash_to_word(storage_root));
    rlp_write_word(hash_to_word(info.code_hash));
    rlp_finish(start)
}

/*! Converts one changed storage row into a secure-trie update. */
function storage_update(trie_entry : StorageTrieEntry) -> TrieUpdate = {
    let entry = trie_entry.entry;
    let key = path_from_hash(trie_entry.slot_hash);
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
    let key = path_from_hash(trie_entry.address_hash);
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

/*! Derives and caches one account's post-state storage root. Keeping this
    preparation separate from the account-update pull source makes the nested
    storage walk statically acyclic while retaining lazy ordered update streams. */
function prepare_account_post_storage_root(trie_entry : AcctTrieEntry) -> unit = {
    let entry = trie_entry.entry;
    let current = entry.value.curr;
    storage_block_iter_begin(entry.addr);
    let storage_updates = trie_updates_begin(StorageTrieUpdates(entry.addr));
    let base_storage_root = if current.storage_cleared then {
            EMPTY_TRIE_ROOT
        } else {
            current.info.storage_root
        };
    let storage_root = if not_bool(current.present) | account_info_empty(current.info) then {
            base_storage_root
        } else {
            trie_root_cursor(base_storage_root, storage_updates).root
        };
    acct_post_storage_root_store(entry.addr, storage_root)
}

/*! Prepares post-state storage roots for every account update candidate. */
function prepare_changed_account_post_storage_roots() -> unit = {
    var preparing : bool = true;
    while preparing termination_measure(2 ^ 64) do {
        match acct_block_iter_next() {
            Some(entry) => prepare_account_post_storage_root(entry),
            None() => preparing = false,
        }
    }
}

/*! Computes one already-prepared account update and whether the account or
    any storage row has a net change. The equality decision remains in Sail. */
function account_trie_update(trie_entry : AcctTrieEntry) -> (TrieUpdate, bool) = {
    let entry = trie_entry.entry;
    storage_block_iter_begin(entry.addr);
    let storage_changed : bool = match next_changed_storage_entry(entry.addr) {
        Some(_storage_entry) => true,
        None() => false,
    };
    let storage_root = acct_post_storage_root_read(entry.addr);
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
        CachedAccountTrieUpdates() => cached_account_trie_update_next(),
    }

/*! The post-state root: traverses every changed account in the kernel's
    block-level overlay, recomputes each touched account's storage root
    from its changed slots (zero-valued slots delete), re-encodes the
    account leaf (empty accounts delete, per EIP-161), and streams the
    ordered updates into the parent state root via [trie_root][]. */
function compute_state_root() -> hash = {
    acct_block_iter_begin();
    prepare_changed_account_post_storage_roots();
    acct_block_iter_begin();
    trie_root(k_parent_state_root, ChangedAccountTrieUpdates()).root
}
