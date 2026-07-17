/* Ethereum account and storage tries over the generic MPT core. */

function decode_state_account(value : ByteSlice) -> AccountInfo = {
    let (nonce, fields) = rlp_cursor_pop(rlp_node_cursor(value));
    let (balance, fields) = rlp_cursor_pop(fields);
    let (storage, fields) = rlp_cursor_pop(fields);
    let (code, fields) = rlp_cursor_pop(fields);
    rlp_cursor_expect_end(fields);
    struct {
        nonce = rlp_ref_uint(nonce),
        balance = rlp_ref_uint_word(balance),
        storage_root =
            if storage.content_len == BYTE_ZERO then {
                EMPTY_TRIE_ROOT
            } else {
                rlp_ref_word(storage)
            },
        code_hash =
            if code.content_len == BYTE_ZERO then {
                KECCAK_EMPTY
            } else {
                rlp_ref_word(code)
            },
    }
}

function stateless_account(root : hash, a : address) -> option(AccountInfo) = {
    let value = trie_lookup(root, path_from_hash(keccak256_address(a)));
    if value.len == BYTE_ZERO then {
        None()
    } else {
        Some(decode_state_account(value))
    }
}

function stateless_storage(root : hash, slot : word) -> word = {
    let value = trie_lookup(root, path_from_hash(keccak256_word(slot)));
    if value.len == BYTE_ZERO then {
        ZERO_WORD
    } else {
        rlp_ref_uint_word(rlp_single_ref(value))
    }
}

function storage_value_changed(value : StorageValue) -> bool =
    not_bool(value.curr == value.orig)

function account_value_changed(value : AcctValue) -> bool =
      not_bool(value.curr.info.nonce == value.orig.info.nonce)
    | not_bool(value.curr.info.balance == value.orig.info.balance)
    | not_bool(value.curr.info.storage_root == value.orig.info.storage_root)
    | not_bool(value.curr.info.code_hash == value.orig.info.code_hash)
    | not_bool(value.curr.present == value.orig.present)
    | not_bool(value.curr.storage_cleared == value.orig.storage_cleared)

function encode_storage_value(value : word) -> ByteSlice = {
    let encoded_len = rlp_uint_word_size(value);
    let start = scratch_begin();
    rlp_write_uint_word(value);
    rlp_finish(start, encoded_len)
}

function encode_state_account(info : AccountInfo, storage_root : hash) -> ByteSlice = {
    var content_len = rlp_uint_size(info.nonce);
    content_len = content_len + rlp_uint_word_size(info.balance);
    content_len = content_len + rlp_word_size();
    content_len = content_len + rlp_word_size();
    let encoded_len = rlp_list_size(content_len);
    let start = scratch_begin();
    rlp_write_list_prefix(content_len);
    rlp_write_uint(info.nonce);
    rlp_write_uint_word(info.balance);
    rlp_write_word(storage_root);
    rlp_write_word(info.code_hash);
    rlp_finish(start, encoded_len)
}

/* The host maps are already ordered by their secure trie keys. Iterate them
 * backwards while consing so the resulting cursor remains ascending without
 * sorting or reversing it. */
function storage_updates(addr : address) -> list(TrieUpdate) = {
    var remaining : item_count = storage_block_count(addr);
    var updates : list(TrieUpdate) = [||];
    while remaining != 0 do {
        remaining = protocol_quantity_decrement(remaining);
        let index = remaining;
        match storage_block_row(addr, index) {
            Some(entry) => if storage_value_changed(entry.value) then {
                let key = path_from_hash(keccak256_word(entry.key.slot));
                let change = if word_is_zero(entry.value.curr) then {
                        TrieDelete()
                    } else {
                        TriePut(encode_storage_value(entry.value.curr))
                    };
                updates = struct { key = key, change = change } :: updates
            },
            None() => throw InvalidBlock(WitnessDeficient),
        }
    };
    updates
}

function account_update(entry : AcctEntry, storage : list(TrieUpdate)) -> TrieUpdate = {
    let current = entry.value.curr;
    let key = path_from_hash(keccak256_address(entry.addr));
    if not_bool(current.present) | account_info_empty(current.info) then {
        struct { key = key, change = TrieDelete() }
    } else {
        let storage_root = trie_root(
            if current.storage_cleared then {
                EMPTY_TRIE_ROOT
            } else {
                current.info.storage_root
            },
            storage,
        );
        struct { key = key, change = TriePut(encode_state_account(current.info, storage_root)) }
    }
}

function compute_state_root() -> hash = {
    var remaining : item_count = acct_block_count();
    var accounts : list(TrieUpdate) = [||];
    while remaining != 0 do {
        remaining = protocol_quantity_decrement(remaining);
        let index = remaining;
        match acct_block_row(index) {
            Some(entry) => {
                let storage = storage_updates(entry.addr);
                if account_value_changed(entry.value) | not_bool(updates_empty(storage)) then {
                    accounts = account_update(entry, storage) :: accounts
                }
            },
            None() => throw InvalidBlock(WitnessDeficient),
        }
    };
    trie_root(k_parent_state_root, accounts)
}
