/*md
# Selfdestruct and creation flags
*/

/*! Marks an account selfdestructed (`SELFDESTRUCT`; deletion is decided
    at transaction end per EIP-6780). */
function k_selfdestruct(a : address) -> unit = {
    let cur = k_aload(a);
    if not_bool(cur.selfdestructed) then {
        store_account(a, { cur with selfdestructed = true })
    }
}

/*! Whether the account is marked selfdestructed this transaction. */
function k_is_selfdestructed(a : address) -> bool = k_aload(a).selfdestructed

/*! Marks an account as created in this transaction (EIP-6780's
    same-transaction test). */
function k_mark_created(a : address) -> unit = {
    let cur = k_aload(a);
    store_account(a, { cur with created = true })
}

/*! Whether the account was created in this transaction. */
function k_was_created(a : address) -> bool = k_aload(a).created

/*! Zeroes an account's balance (the `SELFDESTRUCT` sweep of a
    self-beneficiary). */
function k_zero_balance(a : address) -> unit = {
    let cur = k_aload(a);
    if word_is_zero(cur.info.balance) then {
        ()
    } else {
        store_account_info(a, cur, { cur.info with balance = ZERO_WORD })
    }
}
