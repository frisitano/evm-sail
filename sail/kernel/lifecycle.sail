/*md
# State: the transaction lifecycle

Journal checkpoints, rollback, per-transaction reset, and transaction-end merge
into the block layer.
*/

/*! Appends a frame marker to the state journal. The suspended frame stores its
    refund counter separately. */
function k_journal_checkpoint() -> unit = state_journal_checkpoint()

/*! Installs the block header. */
function k_set_header(h : BlockHeader) -> unit = k_header = h

/*! Installs the per-transaction environment. */
function k_set_tx(env : TxEnv) -> unit = k_tx = env

/*! Resets every per-transaction store and the state journal. */
function k_tx_reset() -> unit = {
    /* Storage reset consumes the per-account transaction worklists owned by
       the account table, so it must precede the account reset. */
    storage_tx_reset();
    acct_tx_reset();
    warm_reset(k_current_transaction_epoch);
    transient_reset();
    logs_tx_reset();
    state_journal_reset()
}

/*! The two correlated lifecycle choices selected once from the active fork.
    Passing this descriptor into the merge keeps both the Sail implementation
    and optimized host implementation from independently re-dispatching on the
    fork or observing impossible feature combinations. */
struct TransactionMergeSemantics = {
    delete_only_created : bool,
    preserve_selfdestruct_balance : bool,
}

/*! Selects the complete transaction-end lifecycle semantics for one fork. */
function transaction_merge_semantics(fork : Fork) -> TransactionMergeSemantics =
    if fork >= Amsterdam then {
        struct {
            delete_only_created = true,
            preserve_selfdestruct_balance = true,
        }
    } else if fork >= Cancun then {
        struct {
            delete_only_created = true,
            preserve_selfdestruct_balance = false,
        }
    } else {
        struct {
            delete_only_created = false,
            preserve_selfdestruct_balance = false,
        }
    }

/*! Whether a selfdestructed account is cleared at transaction end: always
    before Cancun; only if created in the same transaction from Cancun on
    (EIP-6780). Amsterdam preserves any balance left by a self-beneficiary
    `SELFDESTRUCT` (EIP-8246). */
function account_deleted_at_tx_end(semantics : TransactionMergeSemantics, acc : Account) -> bool =
    acc.selfdestructed & (not_bool(semantics.delete_only_created) | acc.created)

/*! The transaction-end merge: drains the transaction overlays into the
    block layer, applying the fork-specific selfdestruct clearing rule,
    storage-clear generations, and recording nonce/balance/code/storage
    changes for the EIP-7928 block access list. Lifecycle flags reset as
    rows merge. */
function k_tx_merge() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let semantics = transaction_merge_semantics(profile.fork);
    var more : bool = true;

    /* Each pop advances a cursor over a protocol-sized host table. */
    while more termination_measure(2 ^ 64) do {
        match acct_tx_pop() {
            AcctTxPopRow(e) => {
                var curr : Account = e.value.curr;
                let deleted = account_deleted_at_tx_end(semantics, curr);
                if deleted then {
                    if semantics.preserve_selfdestruct_balance then {
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
                    bal_nonce_change(k_current_transaction_epoch, e.addr, curr.info.nonce)
                };
                if curr.info.balance != e.value.orig.info.balance then {
                    bal_balance_change(k_current_transaction_epoch, e.addr, curr.info.balance)
                };
                if curr.info.code_hash != e.value.orig.info.code_hash then {
                    bal_code_change(k_current_transaction_epoch, e.addr, curr.info.code_hash)
                };
                curr = { curr with created = false,  selfdestructed = false };
                if account_changed(curr, e.value.orig) then {
                    acct_block_write(struct { addr = e.addr, value = struct { curr = curr, orig = e.value.orig } })
                }
            },
            AcctTxPopExhausted(_) => more = false,
        }
    };
    more = true;

    /* Each pop advances a cursor over a protocol-sized host table. */
    while more termination_measure(2 ^ 64) do {
        match storage_tx_pop() {
            StorageTxPopRow(e) => {
                let account = acct_block_get(e.key.addr);
                if account.found then {
                    let acc = account.account;
                    if acc.present & e.value.curr != e.value.orig then {
                        bal_storage_change(k_current_transaction_epoch, e.key.addr, e.key.slot, e.value.curr);
                        storage_block_put(e)
                    }
                }
            },
            StorageTxPopExhausted(_) => more = false,
        }
    };
    storage_tx_reset();
    acct_tx_reset()
}

/*! Replays the state journal backwards to its innermost open frame boundary. */
function k_journal_revert() -> unit = state_journal_revert()

/*! Records a successful child frame without discarding its reversible entries. */
function k_journal_commit() -> unit = state_journal_commit()
