function apply_undo(e : JEntry) -> unit = match e {
    JTran(a, s, v) => transient_store(a, s, v),
    JWarmA(a) => warm_addr_remove(a),
    JWarmS(a, s) => warm_slot_remove(a, s),
}

function k_state_checkpoint() -> StateCheckpoint = struct {
    journal = journal_len(),
    accounts = acct_tx_checkpoint(),
    storage = storage_tx_checkpoint(),
    logs = logs_checkpoint(),
}

function k_set_header(h : BlockHeader) -> unit = k_header = h

function k_set_tx(env : TxEnv) -> unit = k_tx = env

function k_tx_reset() -> unit = {
    acct_tx_reset();
    storage_tx_reset();
    warm_reset();
    transient_reset();
    logs_tx_reset();
    journal_reset()
}

function account_deleted_at_tx_end(acc : Account) -> bool =
    acc.selfdestructed & ((k_fork < Cancun) | acc.created)

function k_tx_merge() -> unit = {
    var more : bool = true;

    /* Each pop advances a cursor over a protocol-sized host table. */
    while more termination_measure(2 ^ 64) do {
        match acct_tx_pop_ascending() {
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
                curr = { curr with created = false,  selfdestructed = false };
                if account_changed(curr, e.value.orig) then {
                    acct_block_write(struct { addr = e.addr, value = struct { curr = curr, orig = e.value.orig } })
                }
            },
            None() => more = false,
        }
    };
    more = true;

    /* Each pop advances a cursor over a protocol-sized host table. */
    while more termination_measure(2 ^ 64) do {
        match storage_tx_pop() {
            Some(e) => {
                match acct_block_get(e.key.addr) {
                    Some(acc) => if acc.present & e.value.curr != e.value.orig then {
                        bal_storage_change(e.key.addr, e.key.slot, e.value.curr);
                        storage_block_put(e)
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
    logs_revert(checkpoint.logs);
    let current = journal_len();
    let saved = checkpoint.journal;
    var remaining : JournalCheckpoint = if saved <= current then {
            current - saved
        } else {
            assert(false);
            0
        };
    while remaining != 0 termination_measure(remaining) do {
        apply_undo(journal_pop());
        remaining = protocol_quantity_decrement(remaining)
    }
}
