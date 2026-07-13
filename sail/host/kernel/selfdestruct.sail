function k_selfdestruct(a : address) -> unit = {
    let cur = k_aload(a);
    if not_bool(cur.selfdestructed) then {
        store_account(a, { cur with selfdestructed = true })
    }
}

function k_is_selfdestructed(a : address) -> bool = k_aload(a).selfdestructed

function k_mark_created(a : address) -> unit = {
    let cur = k_aload(a);
    store_account(a, { cur with created = true })
}

function k_was_created(a : address) -> bool = k_aload(a).created

function k_zero_balance(a : address) -> unit = {
    let cur = k_aload(a);
    if word_is_zero(cur.info.balance) then {
        ()
    } else {
        store_account_info(a, cur, { cur.info with balance = ZERO_WORD })
    }
}
