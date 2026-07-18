/*md
# State store axioms

The host-side mutable state stores, as bodyless axioms: transient storage
(EIP-1153), the two-layer account and storage overlays (transaction over
block), the EIP-7928 block-access-list recorder, the EIP-2929 warm sets,
and the log store. A single semantic checkpoint atomically captures every
frame-revertible component. Its implementation may use cursors, snapshots,
or an undo journal, but those representation choices are unobservable to the
model.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Clears all transient storage (EIP-1153); part of the per-transaction reset. */
val transient_reset = impure { c: "transient_storage_reset" } : unit -> unit
/*! Writes a transient storage slot and includes the mutation in the current
    semantic checkpoint history. */
val transient_store = impure { c: "transient_storage_write" } : (address, word, word) -> unit
/*! Reads a transient storage slot; zero when never written (`TLOAD`). */
val transient_load = impure { c: "transient_storage_read" } : (address, word) -> word

/*! Clears the host's checkpoint handles and private rollback history at a
    transaction boundary. */
val state_checkpoint_reset = impure { c: "host_state_checkpoint_reset" } : unit -> unit
/*! Captures the complete frame-revertible transaction state: account and
    storage updates, transient storage, warm sets, and logs. */
val state_checkpoint = impure { c: "host_state_checkpoint" } : unit -> StateCheckpoint
/*! Restores exactly the semantic state denoted by `checkpoint`. */
val state_revert = impure { c: "host_state_revert" } : StateCheckpoint -> unit

/*! Inserts or overwrites a row in the transaction-layer storage overlay
    (the raw store behind `SSTORE`). */
val storage_tx_update = impure { c: "storage_tx_update" } : StorageEntry -> unit
/*! The transaction-layer row for a storage key, or `None` when this
    transaction has not written it. */
val storage_tx_get = impure { c: "storage_tx_get" } : StorageKey -> option(StorageValue)
/*! Removes and returns one row of the transaction-layer overlay, or
    `None` when it is empty; the drain loop of the transaction-end merge. */
val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> option(StorageEntry)
/*! Drops every transaction-layer storage row of an address — the
    storage side of an account collapsing to empty or clearing its storage. */
val storage_tx_clear = impure { c: "storage_tx_clear" } : address -> unit
/*! Empties the transaction-layer storage overlay (per-transaction
    reset). */
val storage_tx_reset = impure { c: "storage_tx_reset" } : unit -> unit
/*! Whether the transaction layer holds any storage write for the
    address — the overlay side of the EIP-684/EIP-7610 occupancy test. */
val storage_has_writes = impure { c: "storage_has_writes" } : address -> bool

/* block-layer storage overlay (drained by the state-root computation) */
/*! The block-layer row for a storage key, or `None` when no earlier
    transaction in the block has read or written it. */
val storage_block_get = impure { c: "storage_block_get" } : StorageKey -> option(StorageValue)
/*! Writes a merged row into the block-layer overlay (transaction-end
    merge of a changed slot). */
val storage_block_put = impure { c: "storage_block_put" } : StorageEntry -> unit
/*! Caches a witness-read value in the block layer so later
    transactions skip the trie walk. */
val storage_block_cache = impure { c: "storage_block_cache" } : (StorageKey, word) -> unit
/*! Drops every block-layer storage row of an address (account deletion
    or a fresh storage generation). */
val storage_block_clear = impure { c: "storage_block_clear" } : address -> unit
/*! The number of block-layer storage rows recorded for an address;
    with the indexed readers below, the state-root computation's iteration
    surface. */
val storage_block_count = impure { c: "storage_block_count" } : address -> item_count
/*! Returns the indexed materialized storage row for an account. */
val storage_block_row = impure { c: "storage_block_row" } : (address, item_index) -> option(StorageEntry)

/* transaction-layer account overlay */
/*! The transaction-layer account row for an address, or `None` when
    this transaction has not touched it. */
val acct_tx_get = impure { c: "acct_tx_get" } : address -> option(Account)
/*! Inserts or overwrites the transaction-layer account row for an
    address. */
val acct_tx_update = impure { c: "acct_tx_update" } : (address, Account) -> unit
/*! Balance-only fast path into the transaction-layer row, avoiding a
    whole-account rewrite. */
val acct_tx_set_balance = impure { c: "acct_tx_set_balance" } : (address, word) -> unit
/*! Nonce-only fast path into the transaction-layer row. */
val acct_tx_set_nonce = impure { c: "acct_tx_set_nonce" } : (address, account_nonce) -> unit
/*! Code-hash-only fast path into the transaction-layer row. */
val acct_tx_set_code_hash = impure { c: "acct_tx_set_code_hash" } : (address, hash) -> unit
/*! Removes and returns the transaction-layer account row with the
    lowest address, or `None` when the layer is empty; gives the
    transaction-end merge its deterministic order. */
val acct_tx_pop_ascending = impure { c: "acct_tx_pop_ascending" } : unit -> option(AcctEntry)
/*! Empties the transaction-layer account overlay (per-transaction
    reset). */
val acct_tx_reset = impure { c: "acct_tx_reset" } : unit -> unit

/* block-layer account overlay */
/*! The block-layer account row for an address, or `None` when no
    earlier transaction in the block has loaded it. */
val acct_block_get = impure { c: "acct_block_get" } : address -> option(Account)
/*! Writes a merged current/original account pair into the block layer
    (transaction-end merge). */
val acct_block_write = impure { c: "acct_block_write" } : AcctEntry -> unit
/*! Caches a witness-read account in the block layer so later
    transactions skip the trie walk. */
val acct_block_cache = impure { c: "acct_block_cache" } : (address, Account) -> unit
/*! The number of block-layer account rows; together with
    [acct_block_row][], this is the state-root computation's iteration
    surface. */
val acct_block_count = impure { c: "acct_block_count" } : unit -> item_count
/*! Returns an indexed materialized account row. */
val acct_block_row = impure { c: "acct_block_row" } : item_index -> option(AcctEntry)

/* EIP-7928 block-access-list recorder */
/*! Clears the block-access-list recorder (start of block execution). */
val bal_reset = impure { c: "bal_reset" } : unit -> unit
/*! Sets the transaction index stamped on subsequently recorded changes:
    EIP-7928 keys change entries by transaction index, with 0 for the
    pre-execution system calls and `count + 1` for the post-execution
    ones. */
val bal_set_index = impure { c: "bal_set_index" } : item_index -> unit
/*! Records that the address was accessed, so it appears in the block
    access list even without a change (EIP-7928). */
val bal_account_touch = impure { c: "bal_note_account_touch" } : address -> unit
/*! Records a post-transaction storage value for `(address, slot)` at
    the current transaction index. */
val bal_storage_change = impure { c: "bal_note_storage_change" } : (address, word, word) -> unit
/*! Records a storage read of `(address, slot)`; reads with no
    subsequent change are listed slot-only in EIP-7928. */
val bal_storage_read = impure { c: "bal_note_storage_read" } : (address, word) -> unit
/*! Records a post-transaction balance for the address at the current
    transaction index. */
val bal_balance_change = impure { c: "bal_note_balance_change" } : (address, word) -> unit
/*! Records a post-transaction nonce for the address at the current
    transaction index. */
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (address, account_nonce) -> unit
/*! Records a post-transaction code hash for the address at the current
    transaction index. */
val bal_code_change = impure { c: "bal_note_code_change" } : (address, hash) -> unit
/*! Canonicalizes the recorded data — sorted and ready for indexed
    enumeration — before the block-access-list encoder reads it back. */
val bal_prepare = impure { c: "bal_prepare" } : unit -> unit
/*! The number of recorded accounts (valid after [bal_prepare][]). */
val bal_account_count = impure { c: "bal_account_count" } : unit -> item_count
/*! The address of recorded account `i`, in canonical ascending order. */
val bal_account_address = impure { c: "bal_account_address" } : item_index -> address
/*! The number of storage changes recorded for account `i`. */
val bal_storage_change_count = impure { c: "bal_storage_change_count" } : item_index -> item_count
/*! The slot of storage change `j` of account `i`. */
val bal_storage_change_slot = impure { c: "bal_storage_change_slot" } : (item_index, item_index) -> word
/*! The transaction index of storage change `j` of account `i`. */
val bal_storage_change_index = impure { c: "bal_storage_change_index" } : (item_index, item_index) -> item_index
/*! The post-transaction value of storage change `j` of account `i`. */
val bal_storage_change_value = impure { c: "bal_storage_change_value" } : (item_index, item_index) -> word
/*! The number of change-free storage reads recorded for account `i`. */
val bal_storage_read_count = impure { c: "bal_storage_read_count" } : item_index -> item_count
/*! The slot of storage read `j` of account `i`. */
val bal_storage_read_slot = impure { c: "bal_storage_read_slot" } : (item_index, item_index) -> word
/*! The number of balance changes recorded for account `i`. */
val bal_balance_change_count = impure { c: "bal_balance_change_count" } : item_index -> item_count
/*! The transaction index of balance change `j` of account `i`. */
val bal_balance_change_index = impure { c: "bal_balance_change_index" } : (item_index, item_index) -> item_index
/*! The post-transaction balance of balance change `j` of account `i`. */
val bal_balance_change_value = impure { c: "bal_balance_change_value" } : (item_index, item_index) -> word
/*! The number of nonce changes recorded for account `i`. */
val bal_nonce_change_count = impure { c: "bal_nonce_change_count" } : item_index -> item_count
/*! The transaction index of nonce change `j` of account `i`. */
val bal_nonce_change_index = impure { c: "bal_nonce_change_index" } : (item_index, item_index) -> item_index
/*! The post-transaction nonce of nonce change `j` of account `i`. */
val bal_nonce_change_value = impure { c: "bal_nonce_change_value" } : (item_index, item_index) -> account_nonce
/*! The number of code changes recorded for account `i`. */
val bal_code_change_count = impure { c: "bal_code_change_count" } : item_index -> item_count
/*! The transaction index of code change `j` of account `i`. */
val bal_code_change_index = impure { c: "bal_code_change_index" } : (item_index, item_index) -> item_index
/*! The post-transaction code hash of code change `j` of account `i`. */
val bal_code_change_hash = impure { c: "bal_code_change_hash" } : (item_index, item_index) -> hash

/* EIP-2929 warm address/slot sets */
/*! Clears both warm sets (per-transaction reset; EIP-2929 warmth is
    transaction-scoped). */
val warm_reset = impure { c: "warm_reset" } : unit -> unit
/*! Adds the address to the warm set and returns its prior membership —
    the EIP-2929 cold/warm discriminator. */
val warm_addr_touch = impure { c: "warm_addr_touch" } : address -> bool
/*! Adds `(address, slot)` to the warm slot set and returns its prior
    membership. */
val warm_slot_touch = impure { c: "warm_slot_touch" } : (address, word) -> bool

/* the per-transaction log store */
/*! Clears the transaction's log series (per-transaction reset). */
val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit
/*! Appends a log record — emitter address, topics, data — to the
    transaction's series (YP §4.4.1). */
val log_append = impure { c: "log_append_record" } : (address, list(word), Bytes) -> unit
/*! The transaction's complete log series in emission order (receipt
    construction). */
val read_logs = impure { c: "logs_read_all" } : unit -> list(LogEntry)
