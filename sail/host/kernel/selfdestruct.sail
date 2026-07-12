function k_selfdestruct(a : address) -> unit = {
    journal_push(JSelfD());
    selfdestr_push(a)
}

function k_mark_created(a : address) -> unit = created_insert(a)

function k_was_created(a : address) -> bool = created_contains(a)

function k_zero_balance(a : address) -> unit = {
    if word_is_zero(k_aload(a).balance) then {
        ()
    } else {
        journal_push(JAcct(a, k_aload(a)));
        store_account(a, { k_aload(a) with balance = ZERO_WORD })
    }
}

function k_account_delete(a : address) -> unit = {
    store_account(a, EMPTY_ACCOUNT);
    var more : bool = true;
    while more do {
        match storage_tx_wipe(a) {
            Some(slot) => bal_storage_read(a, slot),
            None() => more = false,
        }
    };
    storage_block_wipe(a)
}

function address_insert_sorted_unique(xs : list(address), a : address) -> list(address) =
    match xs {
        [||] => [|a|],
        x :: r => if x == a then {
            xs
        } else {
            if unsigned(address_to_word(a)) < unsigned(address_to_word(x)) then {
                a :: xs
            } else {
                x :: address_insert_sorted_unique(r, a)
            }
        },
    }

function address_sort_unique(xs : list(address)) -> list(address) =
    match xs {
        [||] => [||],
        x :: r => address_insert_sorted_unique(address_sort_unique(r), x),
    }

function read_selfdestr_rev(i : int, n : int, acc : list(address)) -> list(address) =
    if i >= n then {
        acc
    } else {
        read_selfdestr_rev(i + 1, n, selfdestr_get(get_slice_int(64, i, 0))[159 .. 0] :: acc)
    }

function read_selfdestr() -> list(address) = read_selfdestr_rev(0, unsigned(selfdestr_count()), [||])

function k_finalize_selfdestructs() -> unit = {
    var xs : list(address) = address_sort_unique(read_selfdestr());
    var more : bool = true;
    while more do {
        match xs {
            a :: r => {
                if (k_fork < Cancun) | k_was_created(a) then {
                    k_emit_burn_log(a, k_aload(a).balance);
                    k_account_delete(a)
                };
                xs = r
            },
            [||]   => more = false,
        }
    }
}
