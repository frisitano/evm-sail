function account_changed(c : Account, o : Account) -> bool =
    c.nonce != o.nonce | c.balance != o.balance | c.code_hash != o.code_hash | c.storage_root != o.storage_root

function store_account(a : address, v : Account) -> unit =
    acct_tx_update(a, v.nonce, v.balance, v.storage_root, v.code_hash)

function k_aload(a : address) -> Account = {
    match acct_tx_get(a) {
        Some(acc) => return acc,
        None() => (),
    };
    let acc : Account = match acct_block_get(a) {
        Some(acc) => acc,
        None() => match stateless_account(a) {
            Some(acc) => acc,
            None() => EMPTY_ACCOUNT,
        },
    };
    acct_tx_cache(a, acc.nonce, acc.balance, acc.storage_root, acc.code_hash);
    acc
}

function k_get_balance(a : address) -> word = {
    k_aload(a).balance
}

function k_get_nonce(a : address) -> u64 = {
    k_aload(a).nonce
}

/* EIP-161 "empty" account: nonce 0, balance 0, no code. */
function k_account_is_empty(a : address) -> bool = {
    let acc = k_aload(a);
    (acc.code_hash == KECCAK_EMPTY) & acc.nonce == sail_zeros(64) & word_is_zero(acc.balance)
}

/* CREATE/CREATE2/create-tx address-collision test: the target is occupied if it
 * has code OR a nonzero nonce OR any storage. */
function k_account_occupied(a : address) -> bool = {
    let acc = k_aload(a);
    let has_code : bool = acc.code_hash != KECCAK_EMPTY;
    let has_storage : bool = (acc.storage_root != EMPTY_TRIE_ROOT) | storage_has_live_addr(a);
    has_code | (acc.nonce != sail_zeros(64)) | has_storage
}

/* Move `v` wei from src to dst. Both balances are journaled; the EVM checks
 * sufficiency before calling. */
function k_transfer(src : address, dst : address, v : word) -> unit = {
    let _ = k_aload(src);
    let _ = k_aload(dst);
    if word_is_zero(v) | (src == dst) then {
        ()
    } else {
        journal_push(JAcct(src, k_aload(src)));
        journal_push(JAcct(dst, k_aload(dst)));
        store_account(src, { k_aload(src) with balance = alu_sub(k_aload(src).balance, v) });
        store_account(dst, { k_aload(dst) with balance = alu_add(k_aload(dst).balance, v) });
        k_emit_transfer_log(src, dst, v)
    }
}

/* Increment an account's nonce. The u64 increment cannot wrap: EIP-2681
 * guards every path that reaches a bump. */
function k_bump_nonce(a : address) -> unit = {
    journal_push(JAcct(a, k_aload(a)));
    store_account(a, { k_aload(a) with nonce = get_slice_int(64, unsigned(k_aload(a).nonce) + 1, 0) })
}

function k_add_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        journal_push(JAcct(a, cur));
        store_account(a, { cur with balance = alu_add(cur.balance, v) })
    }
}

function k_sub_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        journal_push(JAcct(a, cur));
        store_account(a, { cur with balance = alu_sub(cur.balance, v) })
    }
}
