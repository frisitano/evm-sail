/*md
# State store axioms

The host-side mutable state stores, as bodyless axioms: transient storage
(EIP-1153), the two-layer account and storage overlays (transaction over
block), the EIP-7928 block-access-list recorder, the EIP-2929 warm sets,
the log store, and the journal. Sail owns all semantics — checkpointing,
revert order, and merge discipline; the implementation behind the host
interface (`ffi/state_db.c`, `ffi/kernel_state.c`) owns only the
containers.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Clears all transient storage (EIP-1153); part of the per-transaction reset. */
val transient_reset = impure { c: "transient_storage_reset" } : unit -> unit
/*! Writes a transient storage slot — the raw store behind `TSTORE`
    (journaling for frame revert happens in the kernel). */
val transient_store = impure { c: "transient_storage_write" } : (address, word, word) -> unit
/*! Reads a transient storage slot; zero when never written (`TLOAD`). */
val transient_load = impure { c: "transient_storage_read" } : (address, word) -> word

/*! Inserts or overwrites a row in the transaction-layer storage overlay
    (the raw store behind `SSTORE`). */
val storage_tx_update = impure { c: "storage_tx_update" } : StorageEntry -> unit
/*! The transaction-layer row for a storage key, or `None` when this
    transaction has not written it. */
val storage_tx_get = impure { c: "storage_tx_get" } : StorageKey -> option(StorageValue)
/*! Removes and returns one row of the transaction-layer overlay, or
    `None` when it is empty; the drain loop of the transaction-end merge. */
val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> option(StorageEntry)
/*! A token capturing the current extent of the transaction-layer
    storage overlay, taken at every frame boundary. */
val storage_tx_checkpoint = impure { c: "storage_tx_checkpoint" } : unit -> StorageCheckpoint
/*! Truncates the transaction-layer storage overlay back to a
    checkpoint, discarding rows written since it was taken. */
val storage_tx_revert = impure { c: "storage_tx_revert" } : StorageCheckpoint -> unit
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
val storage_block_count = impure { c: "storage_block_count" } : address -> bits(64)
/*! Whether storage row `i` of the address differs from its
    block-original value (and so needs a trie update). */
val storage_block_changed = impure { c: "storage_block_changed" } : (address, bits(64)) -> bool
/*! The slot key of storage row `i` of the address. */
val storage_block_slot = impure { c: "storage_block_slot" } : (address, bits(64)) -> word
/*! The current value of storage row `i` of the address. */
val storage_block_current = impure { c: "storage_block_current" } : (address, bits(64)) -> word

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
val acct_tx_set_nonce = impure { c: "acct_tx_set_nonce" } : (address, u64) -> unit
/*! Code-hash-only fast path into the transaction-layer row. */
val acct_tx_set_code_hash = impure { c: "acct_tx_set_code_hash" } : (address, hash) -> unit
/*! Removes and returns the transaction-layer account row with the
    lowest address, or `None` when the layer is empty; gives the
    transaction-end merge its deterministic order. */
val acct_tx_pop_ascending = impure { c: "acct_tx_pop_ascending" } : unit -> option(AcctEntry)
/*! A token capturing the current extent of the transaction-layer
    account overlay, taken at every frame boundary. */
val acct_tx_checkpoint = impure { c: "acct_tx_checkpoint" } : unit -> AccountCheckpoint
/*! Truncates the transaction-layer account overlay back to a
    checkpoint. */
val acct_tx_revert = impure { c: "acct_tx_revert" } : AccountCheckpoint -> unit
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
/*! The number of block-layer account rows; with [acct_block_changed][]
    and [acct_block_address][], the state-root computation's iteration
    surface. */
val acct_block_count = impure { c: "acct_block_count" } : unit -> bits(64)
/*! Whether account row `i` differs from its block-original value in a
    trie-observable way. */
val acct_block_changed = impure { c: "acct_block_changed" } : bits(64) -> bool
/*! The address of account row `i`. */
val acct_block_address = impure { c: "acct_block_address" } : bits(64) -> address

/* EIP-7928 block-access-list recorder */
/*! Clears the block-access-list recorder (start of block execution). */
val bal_reset = impure { c: "bal_reset" } : unit -> unit
/*! Sets the transaction index stamped on subsequently recorded changes:
    EIP-7928 keys change entries by transaction index, with 0 for the
    pre-execution system calls and `count + 1` for the post-execution
    ones. */
val bal_set_index = impure { c: "bal_set_index" } : bits(64) -> unit
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
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (address, bits(64)) -> unit
/*! Records a post-transaction code hash for the address at the current
    transaction index. */
val bal_code_change = impure { c: "bal_note_code_change" } : (address, hash) -> unit
/*! Canonicalizes the recorded data — sorted and ready for indexed
    enumeration — before the block-access-list encoder reads it back. */
val bal_prepare = impure { c: "bal_prepare" } : unit -> unit
/*! The number of recorded accounts (valid after [bal_prepare][]). */
val bal_account_count = impure { c: "bal_account_count" } : unit -> bits(64)
/*! The address of recorded account `i`, in canonical ascending order. */
val bal_account_address = impure { c: "bal_account_address" } : bits(64) -> address
/*! The number of storage changes recorded for account `i`. */
val bal_storage_change_count = impure { c: "bal_storage_change_count" } : bits(64) -> bits(64)
/*! The slot of storage change `j` of account `i`. */
val bal_storage_change_slot = impure { c: "bal_storage_change_slot" } : (bits(64), bits(64)) -> word
/*! The transaction index of storage change `j` of account `i`. */
val bal_storage_change_index = impure { c: "bal_storage_change_index" } : (bits(64), bits(64)) -> bits(64)
/*! The post-transaction value of storage change `j` of account `i`. */
val bal_storage_change_value = impure { c: "bal_storage_change_value" } : (bits(64), bits(64)) -> word
/*! The number of change-free storage reads recorded for account `i`. */
val bal_storage_read_count = impure { c: "bal_storage_read_count" } : bits(64) -> bits(64)
/*! The slot of storage read `j` of account `i`. */
val bal_storage_read_slot = impure { c: "bal_storage_read_slot" } : (bits(64), bits(64)) -> word
/*! The number of balance changes recorded for account `i`. */
val bal_balance_change_count = impure { c: "bal_balance_change_count" } : bits(64) -> bits(64)
/*! The transaction index of balance change `j` of account `i`. */
val bal_balance_change_index = impure { c: "bal_balance_change_index" } : (bits(64), bits(64)) -> bits(64)
/*! The post-transaction balance of balance change `j` of account `i`. */
val bal_balance_change_value = impure { c: "bal_balance_change_value" } : (bits(64), bits(64)) -> word
/*! The number of nonce changes recorded for account `i`. */
val bal_nonce_change_count = impure { c: "bal_nonce_change_count" } : bits(64) -> bits(64)
/*! The transaction index of nonce change `j` of account `i`. */
val bal_nonce_change_index = impure { c: "bal_nonce_change_index" } : (bits(64), bits(64)) -> bits(64)
/*! The post-transaction nonce of nonce change `j` of account `i`. */
val bal_nonce_change_value = impure { c: "bal_nonce_change_value" } : (bits(64), bits(64)) -> bits(64)
/*! The number of code changes recorded for account `i`. */
val bal_code_change_count = impure { c: "bal_code_change_count" } : bits(64) -> bits(64)
/*! The transaction index of code change `j` of account `i`. */
val bal_code_change_index = impure { c: "bal_code_change_index" } : (bits(64), bits(64)) -> bits(64)
/*! The post-transaction code hash of code change `j` of account `i`. */
val bal_code_change_hash = impure { c: "bal_code_change_hash" } : (bits(64), bits(64)) -> hash

/* EIP-2929 warm address/slot sets */
/*! Clears both warm sets (per-transaction reset; EIP-2929 warmth is
    transaction-scoped). */
val warm_reset = impure { c: "warm_reset" } : unit -> unit
/*! Adds the address to the warm set and returns its prior membership —
    the EIP-2929 cold/warm discriminator. */
val warm_addr_touch = impure { c: "warm_addr_touch" } : address -> bool
/*! Removes the address from the warm set; the journal's undo for a
    reverted first touch. */
val warm_addr_remove = impure { c: "warm_addr_remove" } : address -> unit
/*! Adds `(address, slot)` to the warm slot set and returns its prior
    membership. */
val warm_slot_touch = impure { c: "warm_slot_touch" } : (address, word) -> bool
/*! Removes `(address, slot)` from the warm slot set (journal undo). */
val warm_slot_remove = impure { c: "warm_slot_remove" } : (address, word) -> unit

/* the per-transaction log store */
/*! Clears the transaction's log series (per-transaction reset). */
val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit
/*! Appends a log record — emitter address, topics, data — to the
    transaction's series (YP §4.4.1). */
val log_append = impure { c: "log_append_record" } : (address, list(word), Bytes) -> unit
/*! A token capturing the current length of the log series, taken at
    every frame boundary. */
val logs_checkpoint = impure { c: "logs_checkpoint" } : unit -> LogCheckpoint
/*! Truncates the log series back to a checkpoint, so logs of reverted
    frames never surface. */
val logs_revert = impure { c: "logs_revert" } : LogCheckpoint -> unit
/*! The transaction's complete log series in emission order (receipt
    construction). */
val read_logs = impure { c: "logs_read_all" } : unit -> list(LogEntry)

/* the undo journal */
/*! Empties the undo journal (per-transaction reset). */
val journal_reset = impure { c: "journal_reset" } : unit -> unit
/*! The journal length, used as the frame checkpoint token. */
val journal_len = impure { c: "journal_len" } : unit -> JournalCheckpoint
/*! Pushes an undo entry recording a prior value before an in-place
    mutation. */
val journal_push = impure { c: "journal_push" } : JEntry -> unit
/*! Pops the most recent undo entry; [k_revert][] replays entries
    backward to the checkpointed length. */
val journal_pop = impure { c: "journal_pop" } : unit -> JEntry
