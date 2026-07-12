function apply_undo(e : JEntry) -> unit = match e {
    JCheck()       => (),
    JAcct(a, old)  => store_account(a, old),
    JTran(a, s, v) => transient_store(a, s, v),
    JStor(a, s, p) => storage_tx_update(a, s, p),
    JWarmA(a)      => warm_addr_remove(a),
    JWarmS(a, s)   => warm_slot_remove(a, s),
    JLog()         => log_drop_last(),
    JRefund(old)   => k_refund = old,
    JSelfD()       => selfdestr_drop_last(),
}

function k_snapshot() -> unit = {
    journal_push(JCheck())
}

function k_set_header(h : BlockHeader) -> unit = k_header = h

function k_set_tx(env : TxEnv) -> unit = k_tx = env

function k_tx_reset() -> unit = {
    warm_reset();
    transient_reset();
    logs_tx_reset();
    k_refund = 0;
    selfdestr_reset();
    created_reset();
    journal_reset()
}

function k_commit() -> unit = {
    journal_commit()
}

function k_tx_merge() -> unit = {
    var more : bool = true;
    while more do {
        match storage_tx_pop() {
            Some(e) => if e.value.curr != e.value.orig then {
                bal_storage_change(e.key.addr, e.key.slot, e.value.curr);
                storage_block_put(e.key.addr, e.key.slot, e.value.curr, e.value.orig)
            } else {
                bal_storage_read(e.key.addr, e.key.slot);
                storage_block_cache(e.key.addr, e.key.slot, e.value.curr)
            },
            None() => more = false,
        }
    };
    more = true;
    while more do {
        match acct_tx_pop() {
            Some(e) => {
                if e.value.curr.nonce != e.value.orig.nonce then {
                    bal_nonce_change(e.addr, e.value.curr.nonce)
                };
                if e.value.curr.balance != e.value.orig.balance then {
                    bal_balance_change(e.addr, e.value.curr.balance)
                };
                if e.value.curr.code_hash != e.value.orig.code_hash then {
                    bal_code_change(e.addr, e.value.curr.code_hash)
                };
                if account_changed(e.value.curr, e.value.orig) then {
                    acct_block_write(
                        e.addr,
                        e.value.curr.nonce,
                        e.value.curr.balance,
                        e.value.curr.storage_root,
                        e.value.curr.code_hash,
                        e.value.orig.nonce,
                        e.value.orig.balance,
                        e.value.orig.storage_root,
                        e.value.orig.code_hash,
                    )
                } else {
                    acct_block_cache(
                        e.addr,
                        e.value.curr.nonce,
                        e.value.curr.balance,
                        e.value.curr.storage_root,
                        e.value.curr.code_hash,
                    )
                }
            },
            None() => more = false,
        }
    }
}

function k_revert() -> unit = {
    var done : bool = false;
    while not_bool(done) do {
        let e = journal_pop();
        match e {
            JCheck() => done = true,
            _        => apply_undo(e),
        }
    }
}
