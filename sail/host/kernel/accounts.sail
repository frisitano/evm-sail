/*md
# State: accounts

Account reads and mutations over the two-layer overlay (transaction over
block) with the witness as base, plus the EIP-161/EIP-684 predicates and
value transfer.
*/

val stateless_account : (hash, address) -> option(AccountInfo)

/*! Field-wise inequality of account tuples. */
function account_info_changed(c : AccountInfo, o : AccountInfo) -> bool =
    c.nonce != o.nonce | c.balance != o.balance | c.code_hash != o.code_hash | c.storage_root != o.storage_root

/*! The EIP-161 emptiness test: no code, zero nonce, zero balance. */
function account_info_empty(info : AccountInfo) -> bool =
    (info.code_hash == KECCAK_EMPTY) & info.nonce == 0 & word_is_zero(info.balance)

/*! Whether an account differs from its original in any
    trie-observable way. */
function account_changed(c : Account, o : Account) -> bool =
    account_info_changed(c.info, o.info) | c.present != o.present | c.storage_cleared != o.storage_cleared

/*! Installs a new tuple, collapsing to the non-existent form when it is
    EIP-161-empty. */
function account_set_info(acc : Account, info : AccountInfo) -> Account =
    if account_info_empty(info) then {
        {
            acc with
            info = { EMPTY_ACCOUNT_INFO with storage_root = acc.info.storage_root },
            present = false,
            storage_cleared = true,
        }
    } else {
        { acc with info = info,  present = true }
    }

/*! Marks the account's storage cleared (fresh storage generation). */
function account_clear_storage(acc : Account) -> Account =
    {
        acc with
        storage_cleared = true,
    }

/*! The deleted form of an account: empty tuple, non-existent, storage
    cleared. */
function account_delete(acc : Account) -> Account =
    {
        acc with
        info = { EMPTY_ACCOUNT_INFO with storage_root = acc.info.storage_root },
        present = false,
        storage_cleared = true,
    }

/*! Writes a whole-account row to the transaction overlay. */
function store_account(a : address, v : Account) -> unit =
    acct_tx_update(a, v)

/*! Applies a tuple change with minimal overlay traffic: scalar-field
    fast paths when existence/storage state is unchanged, and a storage
    clear when the tuple collapses to empty. */
function store_account_info(a : address, acc : Account, info : AccountInfo) -> unit = {
    if account_info_empty(info) then {
        storage_tx_clear(a)
    };
    let next = account_set_info(acc, info);
    if   (next.info.storage_root != acc.info.storage_root)
       | (next.present != acc.present)
       | (next.storage_cleared != acc.storage_cleared) then {
        store_account(a, next)
    } else {
        if next.info.balance != acc.info.balance then {
            acct_tx_set_balance(a, next.info.balance)
        };
        if next.info.nonce != acc.info.nonce then {
            acct_tx_set_nonce(a, next.info.nonce)
        };
        if next.info.code_hash != acc.info.code_hash then {
            acct_tx_set_code_hash(a, next.info.code_hash)
        }
    }
}

/*! The account at `a`: transaction overlay, then block overlay, then the
    authenticated witness (cached block-level on first read). Every load
    is recorded for the EIP-7928 block access list. */
function k_aload(a : address) -> Account = {
    bal_account_touch(a);
    match acct_tx_get(a) {
        Some(acc) => return acc,
        None() => (),
    };
    match acct_block_get(a) {
        Some(acc) => return acc,
        None() => (),
    };
    let acc : Account = match stateless_account(k_parent_state_root, a) {
        Some(info) => account_from_info(info),
        None() => EMPTY_ACCOUNT,
    };
    acct_block_cache(a, acc);
    acc
}

/*! The account balance (`BALANCE`, `SELFBALANCE`). */
function k_get_balance(a : address) -> word = {
    k_aload(a).info.balance
}

/*! The account nonce. */
function k_get_nonce(a : address) -> account_nonce = {
    k_aload(a).info.nonce
}

/*! Whether the account exists (post-EIP-161 sense). */
function k_account_exists(a : address) -> bool = k_aload(a).present

/*! The EIP-161 "empty" test on the live account: zero nonce, zero
    balance, no code. */
function k_account_is_empty(a : address) -> bool = {
    account_info_empty(k_aload(a).info)
}

/*! The `CREATE`/`CREATE2`/create-transaction address-collision test
    (EIP-684/EIP-7610): the target is occupied if it has code, a nonzero
    nonce, or any storage. */
function k_account_occupied(a : address) -> bool = {
    let acc = k_aload(a);
    let info = acc.info;
    let anchored_storage = not_bool(acc.storage_cleared) & (info.storage_root != EMPTY_TRIE_ROOT);
    if (info.code_hash != KECCAK_EMPTY) | (info.nonce != 0) | anchored_storage then {
        true
    } else {
        storage_has_writes(a)
    }
}

/*! Moves `v` wei from `src` to `dst` (both updates recorded for frame
    rollback; the EVM checks sufficiency before calling) and emits the
    EIP-7708 transfer log. */
function k_transfer(src : address, dst : address, v : word) -> unit = {
    let src_acc = k_aload(src);
    let dst_acc = k_aload(dst);
    if word_is_zero(v) | (src == dst) then {
        ()
    } else {
        store_account_info(src, src_acc, { src_acc.info with balance = alu_sub(src_acc.info.balance, v) });
        store_account_info(dst, dst_acc, { dst_acc.info with balance = alu_add(dst_acc.info.balance, v) });
        k_emit_transfer_log(src, dst, v)
    }
}

/* Increment an account's nonce. EIP-2681 guards every path that reaches a
 * bump, so the semantic account_nonce bound cannot be crossed. */
/*! Increments the account nonce. The `u64` increment cannot wrap:
    EIP-2681 guards every path that reaches a bump. */
function k_bump_nonce(a : address) -> unit = {
    let cur = k_aload(a);
    store_account_info(a, cur, { cur.info with nonce = account_nonce_increment(cur.info.nonce) })
}

/*! Credits `v` wei (no-op when zero). */
function k_add_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        store_account_info(a, cur, { cur.info with balance = alu_add(cur.info.balance, v) })
    }
}

/*! Debits `v` wei (no-op when zero; caller guarantees sufficiency). */
function k_sub_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        store_account_info(a, cur, { cur.info with balance = alu_sub(cur.info.balance, v) })
    }
}

/*! Clears the account's storage (create-time collision cleanup). */
function k_clear_storage(a : address) -> unit = {
    let cur = k_aload(a);
    storage_tx_clear(a);
    store_account(a, account_clear_storage(cur))
}
