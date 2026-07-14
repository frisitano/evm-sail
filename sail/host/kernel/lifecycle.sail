/*md
# State: the transaction lifecycle

Snapshot, revert, per-transaction reset, and the transaction-end merge
into the block layer.
*/

/*! Replays one journal entry backward. */
function apply_undo(e : JEntry) -> unit = match e {
    JTran(a, s, v) => transient_store(a, s, v),
    JWarmA(a)      => warm_addr_remove(a),
    JWarmS(a, s)   => warm_slot_remove(a, s),
    JRefund(old)   => k_refund = old,
}

/*! A checkpoint of all four mutable stores — taken at every frame
    boundary. */
function k_snapshot() -> StateCheckpoint = struct {
    journal = journal_len(),
    accounts = acct_tx_checkpoint(),
    storage = storage_tx_checkpoint(),
    logs = logs_checkpoint(),
}

/*! Installs the block header. */
function k_set_header(h : BlockHeader) -> unit = k_header = h

/*! Installs the per-transaction environment. */
function k_set_tx(env : TxEnv) -> unit = k_tx = env

/*! Resets every per-transaction store: overlays, warm sets, transient
    storage, logs, refund counter, journal. */
function k_tx_reset() -> unit = {
    acct_tx_reset();
    storage_tx_reset();
    warm_reset();
    transient_reset();
    logs_tx_reset();
    k_refund = 0;
    journal_reset()
}

/*! Whether a selfdestructed account is actually deleted at transaction
    end: always before Cancun; only if created in the same transaction
    from Cancun on (EIP-6780). */
function account_deleted_at_tx_end(acc : Account) -> bool =
    acc.selfdestructed & ((k_fork < Cancun) | acc.created)

/*! The transaction-end merge: drains the transaction overlays into the
    block layer, applying EIP-6780 deletion (with the EIP-7708 burn log),
    storage-clear generations, and recording nonce/balance/code/storage
    changes for the EIP-7928 block access list. Lifecycle flags reset as
    rows merge. */
function k_tx_merge() -> unit = {
    var more : bool = true;
    while more do {
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
    while more do {
        match storage_tx_pop() {
            Some(e) => {
                match acct_block_get(e.key.addr) {
                    Some(acc) => if acc.exists & e.value.curr != e.value.orig then {
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

/*! Reverts a frame to its checkpoint: truncates the overlays and log
    store, then replays the journal backward to the checkpointed
    length. */
function k_revert(checkpoint : StateCheckpoint) -> unit = {
    acct_tx_revert(checkpoint.accounts);
    storage_tx_revert(checkpoint.storage);
    logs_revert(checkpoint.logs);
    var remaining : u64 = sub_bits(journal_len(), checkpoint.journal);
    while remaining != 0x0000000000000000 do {
        apply_undo(journal_pop());
        remaining = sub_bits(remaining, 0x0000000000000001)
    }
}
