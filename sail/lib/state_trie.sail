/* Ethereum account and storage tries over the generic MPT core. */

function decode_state_account(value : Bytes) -> AccountInfo = {
    let fields = rlp_node_refs(value);
    let storage = rlp_ref_item(fields, 2);
    let code = rlp_ref_item(fields, 3);
    struct {
        nonce = get_slice_int(64, rlp_ref_int(rlp_ref_item(fields, 0)), 0),
        balance = rlp_ref_uint_word(rlp_ref_item(fields, 1)),
        storage_root = if unsigned(storage.content_len) == 0 then { EMPTY_TRIE_ROOT } else { rlp_ref_word(storage) },
        code_hash = if unsigned(code.content_len) == 0 then { KECCAK_EMPTY } else { rlp_ref_word(code) },
    }
}

function stateless_account(root : hash, a : address) -> option(AccountInfo) = {
    let value = trie_lookup(root, path_from_hash(keccak256_address(a)));
    if bytes_len(value) == 0 then { None() } else { Some(decode_state_account(value)) }
}

function stateless_storage(root : hash, slot : word) -> word = {
    let value = trie_lookup(root, path_from_hash(keccak256_word(slot)));
    if bytes_len(value) == 0 then { ZERO_WORD } else { rlp_ref_uint_word(rlp_single_ref(value)) }
}

function compute_state_root() -> hash = {
    var accounts : list(TriePatch) = [||];
    foreach (i from 0 to (unsigned(acct_block_count()) - 1)) {
        let index = get_slice_int(64, i, 0);
        let addr = acct_block_address(index);
        let current : Account = match acct_block_get(addr) {
                Some(account) => account,
                None() => EMPTY_ACCOUNT,
            };
        var storage : list(TriePatch) = [||];

        foreach (j from 0 to (unsigned(storage_block_count(addr)) - 1)) {
            let storage_index = get_slice_int(64, j, 0);
            if storage_block_changed(addr, storage_index) then {
                let slot = storage_block_slot(addr, storage_index);
                let value = storage_block_current(addr, storage_index);
                let key = path_from_hash(keccak256_word(slot));
                storage = patch_insert_sorted(
                    if word_is_zero(value) then {
                        patch_delete(key)
                    } else {
                        patch_leaf(key, BytesList(rlp_int_word(value)))
                    },
                    storage,
                )
            }
        };

        if acct_block_changed(index) | not_bool(patches_empty(storage)) then {
            let key = path_from_hash(keccak256_address(addr));
            let change = if not_bool(current.exists) | account_info_empty(current.info) then {
                    patch_delete(key)
                } else {
                    let storage_root = trie_root(
                        if current.storage_cleared then { EMPTY_TRIE_ROOT } else { current.info.storage_root },
                        storage,
                    );
                    patch_leaf(
                        key,
                        BytesList(
                            rlp_list(
                                rlp_int(unsigned(current.info.nonce)) ++
                                rlp_int_word(current.info.balance) ++
                                rlp_word(storage_root) ++
                                rlp_word(current.info.code_hash),
                            ),
                        ),
                    )
                };
            accounts = patch_insert_sorted(change, accounts)
        }
    };
    trie_root(k_parent_state_root, accounts)
}
