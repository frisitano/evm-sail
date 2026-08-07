/* Optimized C overrides for `sail/kernel/accounts.sail`. */

/*
 * Complete optimized state accesses. The standard account/storage modules
 * remain the readable specification of overlay ordering, BAL effects,
 * authenticated witness fallback, caching, and transaction-original values.
 * Optimized C performs each complete operation against the same host-owned
 * tables in one crossing.
 */

val account_load_from_root = impure { c: "k_aload" } : (hash, address) -> Account

val account_store_record = impure { c: "store_account" } : (address, Account) -> unit
val account_store_record_info = impure { c: "store_account_info" } : (address, Account, AccountInfo) -> unit

function k_aload(a : address) -> Account =
    account_load_from_root(k_parent_state_root, a)

function store_account(a : address, account : Account) -> unit =
    account_store_record(a, account)

function store_account_info(a : address, account : Account, info : AccountInfo) -> unit =
    account_store_record_info(a, account, info)
