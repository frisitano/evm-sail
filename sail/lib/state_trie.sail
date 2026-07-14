/*md
# The state trie

Ethereum account and storage tries over the generic MPT core: secure-trie
reads for stateless execution, and the post-state root computation
(YP §4.1).
*/

/*! Decodes an account trie leaf — `rlp([nonce, balance, storage_root,
    code_hash])` — into an [AccountInfo][type-AccountInfo]; empty
    root/hash fields decode to their empty-sentinel digests. */
function decode_state_account(value : Bytes) -> AccountInfo = {
    let (nonce, fields) = rlp_cursor_pop(rlp_node_cursor(value));
    let (balance, fields) = rlp_cursor_pop(fields);
    let (storage, fields) = rlp_cursor_pop(fields);
    let (code, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    struct {
        nonce = get_slice_int(64, rlp_ref_int(nonce), 0),
        balance = rlp_ref_uint_word(balance),
        storage_root =
            if unsigned(storage.content_len) == 0 then {
                EMPTY_TRIE_ROOT
            } else {
                rlp_ref_word(storage)
            },
        code_hash =
            if unsigned(code.content_len) == 0 then {
                KECCAK_EMPTY
            } else {
                rlp_ref_word(code)
            },
    }
}

/*! The witnessed account at address `a` under state root `root`,
    reading the secure trie at `keccak256(a)`; `None` when the walk
    proves absence. */
function stateless_account(root : hash, a : address) -> option(AccountInfo) = {
    let value = trie_lookup(root, path_from_hash(keccak256_address(a)));
    if bytes_len(value) == 0 then {
        None()
    } else {
        Some(decode_state_account(value))
    }
}

/*! The witnessed storage value of `slot` under a storage root, reading
    the secure trie at `keccak256(slot)`; absent slots are zero. */
function stateless_storage(root : hash, slot : word) -> word = {
    let value = trie_lookup(root, path_from_hash(keccak256_word(slot)));
    if bytes_len(value) == 0 then {
        ZERO_WORD
    } else {
        rlp_ref_uint_word(rlp_single_ref(value))
    }
}

/*! The post-state root: drains every changed account from the kernel's
    block-level overlay, recomputes each touched account's storage root
    from its changed slots (zero-valued slots delete), re-encodes the
    account leaf (empty accounts delete, per EIP-161), and folds the
    sorted update list into the parent state root via [trie_root][]. */
function compute_state_root() -> hash = {
    var accounts : list(TrieUpdate) = [||];
    foreach (i from 0 to (unsigned(acct_block_count()) - 1)) {
        let index = get_slice_int(64, i, 0);
        let addr = acct_block_address(index);
        let current : Account = match acct_block_get(addr) {
            Some(account) => account,
            None() => EMPTY_ACCOUNT,
        };
        var storage : list(TrieUpdate) = [||];

        foreach (j from 0 to (unsigned(storage_block_count(addr)) - 1)) {
            let storage_index = get_slice_int(64, j, 0);
            if storage_block_changed(addr, storage_index) then {
                let slot = storage_block_slot(addr, storage_index);
                let value = storage_block_current(addr, storage_index);
                let key = path_from_hash(keccak256_word(slot));
                storage =
                    if word_is_zero(value) then {
                        struct { key = key, change = TrieDelete() } :: storage
                    } else {
                        struct { key = key, change = TriePut(BytesList(rlp_int_word(value))) } :: storage
                    }
            }
        };

        if acct_block_changed(index) | not_bool(updates_empty(storage)) then {
            let key = path_from_hash(keccak256_address(addr));
            let update : TrieUpdate = if not_bool(current.exists) | account_info_empty(current.info) then {
                    struct { key = key, change = TrieDelete() }
                } else {
                    let storage_root = trie_root(
                        if current.storage_cleared then {
                            EMPTY_TRIE_ROOT
                        } else {
                            current.info.storage_root
                        },
                        sort_updates(storage),
                    );
                    struct {
                        key = key,
                        change =
                            TriePut(
                                BytesList(
                                    rlp_list(
                                           rlp_int(unsigned(current.info.nonce))
                                        ++ rlp_int_word(current.info.balance)
                                        ++ rlp_word(storage_root)
                                        ++ rlp_word(current.info.code_hash),
                                    ),
                                ),
                            ),
                    }
                };
            accounts = update :: accounts
        }
    };
    trie_root(k_parent_state_root, sort_updates(accounts))
}
