function apply_undo(e : JEntry) -> unit = match e {
    JTran(a, s, v) => transient_store(a, s, v),
    JWarmA(a)      => warm_addr_remove(a),
    JWarmS(a, s)   => warm_slot_remove(a, s),
    JLog()         => log_drop_last(),
    JRefund(old)   => k_refund = old,
}

function k_snapshot() -> StateCheckpoint = struct {
    journal = journal_len(),
    accounts = acct_tx_checkpoint(),
    storage = storage_tx_checkpoint(),
}

function k_set_header(h : BlockHeader) -> unit = k_header = h

function k_set_tx(env : TxEnv) -> unit = k_tx = env

function k_tx_reset() -> unit = {
    acct_tx_reset();
    storage_tx_reset();
    warm_reset();
    transient_reset();
    logs_tx_reset();
    k_refund = 0;
    journal_reset()
}

function account_deleted_at_tx_end(acc : Account) -> bool =
    acc.selfdestructed & ((k_fork < Cancun) | acc.created)

function k_tx_merge() -> unit = {
    var more : bool = true;
    while more do {
        match acct_tx_pop() {
            Some(e) => {
                var curr : Account = e.value.curr;
                let deleted = account_deleted_at_tx_end(curr);
                if deleted then {
                    k_emit_burn_log(e.addr, curr.info.balance);
                    curr = account_delete(curr)
                };
                if deleted | (curr.storage_cleared & not_bool(e.value.orig.storage_cleared)) then {
                    storage_block_clear(e.addr)
                };
                if curr.info.nonce != e.value.orig.info.nonce then {
                    bal_nonce_change(e.addr, curr.info.nonce)
                };
                if curr.info.balance != e.value.orig.info.balance then {
                    bal_balance_change(e.addr, curr.info.balance)
                };
                if curr.info.code_hash != e.value.orig.info.code_hash then {
                    bal_code_change(e.addr, curr.info.code_hash)
                };
                if account_changed(curr, e.value.orig) then {
                    acct_block_write(struct { addr = e.addr, value = struct { curr = curr, orig = e.value.orig } })
                } else {
                    acct_block_cache(e.addr, curr)
                }
            },
            None() => more = false,
        }
    };
    more = true;
    while more do {
        match storage_tx_pop() {
            Some(e) => {
                bal_storage_read(e.key.addr, e.key.slot);
                match acct_tx_get(e.key.addr) {
                    Some(acc) => if acc.exists & not_bool(account_deleted_at_tx_end(acc)) then {
                        match e.write {
                            Some(v) => {
                                if v.curr != e.tx_original then {
                                    bal_storage_change(e.key.addr, e.key.slot, v.curr)
                                };
                                storage_block_put(struct { key = e.key, value = v })
                            },
                            None() => if not_bool(acc.storage_cleared) then {
                                storage_block_cache(e.key, e.tx_original)
                            },
                        }
                    },
                    None() => (),
                }
            },
            None() => more = false,
        }
    };
    acct_tx_reset();
    storage_tx_reset()
}

function k_revert(checkpoint : StateCheckpoint) -> unit = {
    acct_tx_revert(checkpoint.accounts);
    storage_tx_revert(checkpoint.storage);
    var remaining : u64 = sub_bits(journal_len(), checkpoint.journal);
    while remaining != 0x0000000000000000 do {
        apply_undo(journal_pop());
        remaining = sub_bits(remaining, 0x0000000000000001)
    }
}
