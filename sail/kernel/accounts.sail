/*md
# State: accounts

Account reads and mutations over the two-layer overlay (transaction over
block) with the witness as base, plus the EIP-161/EIP-684 predicates and
value transfer.
*/

val stateless_account_by_key : (hash, hash) -> Account

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
function account_set_info(acc : Account, info : AccountInfo) -> Account = {
    let empty = account_info_empty(info);
    if empty then {
        {
            acc with
            info = { EMPTY_ACCOUNT_INFO with storage_root = acc.info.storage_root },
            present = false,
            storage_cleared = true,
        }
    } else {
        { acc with info = info,  present = true }
    }
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

/*! Clears nonce, code, and storage while preserving a nonzero balance.
    Amsterdam applies this form to an account created and selfdestructed in
    the same transaction (EIP-8246). */
function account_clear_preserving_balance(acc : Account) -> Account = {
    let cleared_storage = account_clear_storage(acc);
    account_set_info(cleared_storage, { acc.info with nonce = 0,  code_hash = KECCAK_EMPTY })
}

/*! Writes a whole-account row to the transaction overlay. */
function store_account(a : address, v : Account) -> unit =
    acct_tx_update(a, v)

/*! Applies a tuple change with minimal overlay traffic: scalar-field
    fast paths when existence/storage state is unchanged, and a storage
    clear when the tuple collapses to empty. */
function store_account_info(a : address, acc : Account, info : AccountInfo) -> unit = {
    let empty = account_info_empty(info);
    if empty then {
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

/*! Resolves an account through the transaction and block overlays before an
    authenticated witness read. A transaction-overlay hit was already touched
    when that row was established; misses record the EIP-7928 account touch
    before consulting block-scoped state. */
function k_aload(a : address) -> Account = {
    let tx_account = acct_tx_get(a);
    if tx_account.found then {
        return tx_account.account
    };

    bal_account_touch(a);

    let block_account = acct_block_get(a);
    if block_account.found then {
        return block_account.account
    };

    let address_hash = keccak256_address(a);
    let account = stateless_account_by_key(k_parent_state_root, address_hash);
    acct_block_cache(a, address_hash, account);
    account
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
    let account = k_aload(a);
    account_info_empty(account.info)
}

/*! The `CREATE`/`CREATE2`/create-transaction address-collision test
    (EIP-684/EIP-7610): the target is occupied if it has code, a nonzero
    nonce, or any storage. */
function k_account_occupied(a : address) -> bool = {
    let acc = k_aload(a);
    let info = acc.info;
    var anchored_storage : bool = false;
    if not_bool(acc.storage_cleared) then {
        anchored_storage = info.storage_root != EMPTY_TRIE_ROOT
    };
    let has_code = info.code_hash != KECCAK_EMPTY;
    let has_nonce = info.nonce != 0;
    if has_code | has_nonce | anchored_storage then {
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
    let value_is_zero = word_is_zero(v);
    if value_is_zero | (src == dst) then {
        return ()
    };
    let source_balance = alu_sub(src_acc.info.balance, v);
    store_account_info(src, src_acc, { src_acc.info with balance = source_balance });
    let destination_balance = alu_add(dst_acc.info.balance, v);
    store_account_info(dst, dst_acc, { dst_acc.info with balance = destination_balance });
    k_emit_transfer_log(src, dst, v)
}

/* Increment an account's nonce. EIP-2681 guards every path that reaches a
 * bump, so the semantic account_nonce bound cannot be crossed. */
/*! Increments the account nonce. The `u64` increment cannot wrap:
    EIP-2681 guards every path that reaches a bump. */
function k_bump_nonce(a : address) -> unit = {
    let cur = k_aload(a);
    let nonce = cur.info.nonce;
    if nonce < sizeof(account_nonce_bound) then {
        store_account_info(a, cur, { cur.info with nonce = nonce + 1 })
    } else {
        fatal_error(ExecutionInvalid)
    }
}

/*! Credits `v` wei (no-op when zero). */
val k_add_balance : forall 'value, 0 <= 'value & 'value < 2 ^ 256. (address, int('value)) -> unit
function k_add_balance(a, v) = {
    let cur = k_aload(a);
    let value_is_zero = word_is_zero(v);
    let value_is_nonzero = not_bool(value_is_zero);
    if value_is_nonzero then {
        let balance = alu_add(cur.info.balance, v);
        store_account_info(a, cur, { cur.info with balance = balance })
    }
}

/*! Debits `v` wei (no-op when zero; caller guarantees sufficiency). */
function k_sub_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    let value_is_zero = word_is_zero(v);
    let value_is_nonzero = not_bool(value_is_zero);
    if value_is_nonzero then {
        let balance = alu_sub(cur.info.balance, v);
        store_account_info(a, cur, { cur.info with balance = balance })
    }
}

/*! Clears the account's storage (create-time collision cleanup). */
function k_clear_storage(a : address) -> unit = {
    let cur = k_aload(a);
    storage_tx_clear(a);
    let cleared = account_clear_storage(cur);
    store_account(a, cleared)
}
