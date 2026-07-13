val stateless_account : (hash, address) -> option(AccountInfo)

function account_info_changed(c : AccountInfo, o : AccountInfo) -> bool =
    c.nonce != o.nonce | c.balance != o.balance | c.code_hash != o.code_hash | c.storage_root != o.storage_root

function account_info_empty(info : AccountInfo) -> bool =
    (info.code_hash == KECCAK_EMPTY) & info.nonce == sail_zeros(64) & word_is_zero(info.balance)

function account_changed(c : Account, o : Account) -> bool =
    account_info_changed(c.info, o.info) | c.exists != o.exists | c.storage_cleared != o.storage_cleared

function account_set_info(acc : Account, info : AccountInfo) -> Account =
    if account_info_empty(info) then {
        {
            acc with
            info = { EMPTY_ACCOUNT_INFO with storage_root = acc.info.storage_root },
            exists = false,
            storage_cleared = true,
        }
    } else {
        { acc with info = info,  exists = true }
    }

function account_clear_storage(acc : Account) -> Account =
    {
        acc with
        storage_cleared = true,
    }

function account_delete(acc : Account) -> Account =
    {
        acc with
        info = { EMPTY_ACCOUNT_INFO with storage_root = acc.info.storage_root },
        exists = false,
        storage_cleared = true,
    }

function store_account(a : address, v : Account) -> unit =
    acct_tx_update(a, v)

function store_account_info(a : address, acc : Account, info : AccountInfo) -> unit = {
    if account_info_empty(info) then {
        storage_tx_clear(a)
    };
    let next = account_set_info(acc, info);
    if (next.info.storage_root != acc.info.storage_root)
        | (next.exists != acc.exists)
        | (next.storage_cleared != acc.storage_cleared)
    then {
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

function k_aload(a : address) -> Account = {
    match acct_tx_get(a) {
        Some(acc) => return acc,
        None() => (),
    };
    let acc : Account = match acct_block_get(a) {
        Some(acc) => acc,
        None() => match stateless_account(k_parent_state_root, a) {
            Some(info) => account_from_info(info),
            None() => EMPTY_ACCOUNT,
        },
    };
    acct_tx_cache(a, acc);
    acc
}

function k_get_balance(a : address) -> word = {
    k_aload(a).info.balance
}

function k_get_nonce(a : address) -> u64 = {
    k_aload(a).info.nonce
}

function k_account_exists(a : address) -> bool = k_aload(a).exists

/* EIP-161 "empty" account: nonce 0, balance 0, no code. */
function k_account_is_empty(a : address) -> bool = {
    account_info_empty(k_aload(a).info)
}

/* CREATE/CREATE2/create-tx address-collision test: the target is occupied if it
 * has code OR a nonzero nonce OR any storage. */
function k_account_occupied(a : address) -> bool = {
    let info = k_aload(a).info;
    if (info.code_hash != KECCAK_EMPTY)
        | (info.nonce != sail_zeros(64))
        | (info.storage_root != EMPTY_TRIE_ROOT)
    then {
        true
    } else {
        storage_has_writes(a)
    }
}

/* Move `v` wei from src to dst. The account store records both updates for
 * frame rollback; the EVM checks sufficiency before calling. */
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

/* Increment an account's nonce. The u64 increment cannot wrap: EIP-2681
 * guards every path that reaches a bump. */
function k_bump_nonce(a : address) -> unit = {
    let cur = k_aload(a);
    store_account_info(a, cur, { cur.info with nonce = get_slice_int(64, unsigned(cur.info.nonce) + 1, 0) })
}

function k_add_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        store_account_info(a, cur, { cur.info with balance = alu_add(cur.info.balance, v) })
    }
}

function k_sub_balance(a : address, v : word) -> unit = {
    let cur = k_aload(a);
    if not_bool(word_is_zero(v)) then {
        store_account_info(a, cur, { cur.info with balance = alu_sub(cur.info.balance, v) })
    }
}

function k_clear_storage(a : address) -> unit = {
    let cur = k_aload(a);
    storage_tx_clear(a);
    store_account(a, account_clear_storage(cur))
}
