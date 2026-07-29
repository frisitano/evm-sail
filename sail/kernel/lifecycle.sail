/*md
# State: the transaction lifecycle

Snapshot, revert, per-transaction reset, and the transaction-end merge
into the block layer.
*/

/*! Captures all frame-revertible transaction state. The frame checkpoint
    stores its refund counter separately. */
function k_state_checkpoint() -> journal_checkpoint = state_checkpoint()

/*! Installs the block header. */
function k_set_header(h : BlockHeader) -> unit = k_header = h

/*! Installs the per-transaction environment. */
function k_set_tx(env : TxEnv) -> unit = k_tx = env

/*! Resets every per-transaction store and the private checkpoint history. */
function k_tx_reset() -> unit = {
    acct_tx_reset();
    storage_tx_reset();
    warm_reset();
    transient_reset();
    logs_tx_reset();
    state_checkpoint_reset()
}

/*! Whether a selfdestructed account is cleared at transaction end: always
    before Cancun; only if created in the same transaction from Cancun on
    (EIP-6780). Amsterdam preserves any balance left by a self-beneficiary
    `SELFDESTRUCT` (EIP-8246). */
function account_deleted_at_tx_end(acc : Account) -> bool = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    acc.selfdestructed & ((profile.fork < Cancun) | acc.created)
}

/*! The transaction-end merge: drains the transaction overlays into the
    block layer, applying the fork-specific selfdestruct clearing rule,
    storage-clear generations, and recording nonce/balance/code/storage
    changes for the EIP-7928 block access list. Lifecycle flags reset as
    rows merge. */
function k_tx_merge() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    var more : bool = true;

    /* Each pop advances a cursor over a protocol-sized host table. */
    while more termination_measure(2 ^ 64) do {
        match acct_tx_pop() {
            Some(e) => {
                var curr : Account = e.value.curr;
                let deleted = account_deleted_at_tx_end(curr);
                if deleted then {
                    if profile.fork >= Amsterdam then {
                        curr = account_clear_preserving_balance(curr)
                    } else {
                        curr = account_delete(curr)
                    };

                    /* The account's active transaction generation belongs to
                       the deleted incarnation. Retire it before the storage
                       drain so its writes cannot be reinserted after the
                       block-layer clear. */
                    storage_tx_clear(e.addr)
                };
                if deleted | (curr.storage_cleared & not_bool(e.value.orig.storage_cleared)) then {
                    storage_block_clear(e.addr)
                };
                if curr.info.nonce != e.value.orig.info.nonce then {
                    bal_nonce_change(k_block_access_index, e.addr, curr.info.nonce)
                };
                if curr.info.balance != e.value.orig.info.balance then {
                    bal_balance_change(k_block_access_index, e.addr, curr.info.balance)
                };
                if curr.info.code_hash != e.value.orig.info.code_hash then {
                    bal_code_change(k_block_access_index, e.addr, curr.info.code_hash)
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
                        bal_storage_change(k_block_access_index, e.key.addr, e.key.slot, e.value.curr);
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

/*! Atomically restores the transaction state captured at a frame boundary. */
function k_revert(checkpoint : journal_checkpoint) -> unit = state_revert(checkpoint)
