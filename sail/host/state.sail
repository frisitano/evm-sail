/*md
# State store axioms

The host-side mutable state stores, as bodyless axioms: transient storage
(EIP-1153), the two-layer account and storage overlays (transaction over
block), the EIP-7928 block-access-list recorder, the EIP-2929 warm sets,
and the log store. Frame rollback is specified separately by the closed
[state-journal operation algebra](journal.md).

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

/*! Inserts or overwrites a row in the transaction-layer storage overlay
    (the raw store behind `SSTORE`). */
val storage_tx_update = impure { c: "storage_tx_update" } : StorageEntry -> unit
/*! Resolves a transaction-layer storage key, distinguishing an actual cached
    row from an uncached slot made known-zero by an account storage clear. */
val storage_tx_get = impure { c: "storage_tx_get" } : StorageKey -> StorageTxLookup
/*! Removes and returns one row of the transaction-layer overlay, or reports
    exhaustion; the drain loop of the transaction-end merge. */
val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> StorageTxPopResult
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
/*! The block-layer row for a storage key. `found` is false when no earlier
    transaction in the block has read or written it. */
val storage_block_get = impure { c: "storage_block_get" } : StorageKey -> StorageBlockRow
/*! Writes a merged row into the block-layer overlay (transaction-end
    merge of a changed slot). */
val storage_block_put = impure { c: "storage_block_put" } : StorageEntry -> unit
/*! Caches a witness-read value and its already-derived secure slot key in the
    block layer so later transactions skip both the trie walk and re-hashing. */
val storage_block_cache = impure { c: "storage_block_cache" } : (StorageKey, hash, word) -> unit
/*! Drops every block-layer storage row of an address (account deletion
    or a fresh storage generation). */
val storage_block_clear = impure { c: "storage_block_clear" } : address -> unit
/*! Starts a non-destructive ascending traversal of the block-layer storage rows
    for an address. The iterator is valid until the block layer is mutated. */
val storage_block_iter_begin = impure { c: "storage_block_iter_begin" } : address -> unit
/*! Returns the next materialized storage row and its cached secure keys in
    ascending slot-key order, or reports traversal exhaustion. */
val storage_block_iter_next = impure { c: "storage_block_iter_next" } : address -> StorageBlockIterResult

/* transaction-layer account overlay */
/*! The transaction-layer account row for an address. `found` is false when
    this transaction has not touched it. */
val acct_tx_get = impure { c: "acct_tx_get" } : address -> AccountRow
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
/*! Returns the next transaction-layer account row in insertion order, or
    reports exhaustion. Merge order is not protocol-visible. */
val acct_tx_pop = impure { c: "acct_tx_pop" } : unit -> AcctTxPopResult
/*! Empties the transaction-layer account overlay (per-transaction
    reset). */
val acct_tx_reset = impure { c: "acct_tx_reset" } : unit -> unit

/* block-layer account overlay */
/*! The block-layer account row for an address. `found` is false when no
    earlier transaction in the block has loaded it. */
val acct_block_get = impure { c: "acct_block_get" } : address -> AccountRow
/*! Writes a merged current/original account pair into the block layer
    (transaction-end merge). */
val acct_block_write = impure { c: "acct_block_write" } : AcctEntry -> unit
/*! Caches a witness-read account and its already-derived secure address key in
    the block layer so later transactions skip both the trie walk and
    re-hashing. */
val acct_block_cache = impure { c: "acct_block_cache" } : (address, hash, Account) -> unit
/*! Starts a non-destructive ascending traversal of block-layer account update
    candidates. Candidates were written directly or have written storage rows;
    Sail decides whether their final current/original values differ. The
    iterator is valid until the block layer is mutated. */
val acct_block_iter_begin = impure { c: "acct_block_iter_begin" } : unit -> unit
/*! Returns the next account update candidate and its cached secure key in
    ascending secure-key order, or reports traversal exhaustion. */
val acct_block_iter_next = impure { c: "acct_block_iter_next" } : unit -> AcctBlockIterResult

/* EIP-7928 block-access-list recorder */
/*! Clears the block-access-list recorder (start of block execution). */
val bal_reset = impure { c: "bal_reset" } : unit -> unit
/*! Records that the address was accessed, so it appears in the block
    access list even without a change (EIP-7928). */
val bal_account_touch = impure { c: "bal_note_account_touch" } : address -> unit
/*! Records a post-transaction storage value for `(address, slot)` at
    the supplied EIP-7928 position. */
val bal_storage_change = impure { c: "bal_note_storage_change" } : (block_access_index, address, word, word) -> unit
/*! Records a storage read of `(address, slot)`; reads with no
    subsequent change are listed slot-only in EIP-7928. */
val bal_storage_read = impure { c: "bal_note_storage_read" } : (address, word) -> unit
/*! Records a post-transaction balance at the supplied EIP-7928 position. */
val bal_balance_change = impure { c: "bal_note_balance_change" } : (block_access_index, address, word) -> unit
/*! Records a post-transaction nonce at the supplied EIP-7928 position. */
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (block_access_index, address, account_nonce) -> unit
/*! Records a post-transaction code hash at the supplied EIP-7928 position. */
val bal_code_change = impure { c: "bal_note_code_change" } : (block_access_index, address, hash) -> unit
/*! One recorded storage change in canonical host traversal order. */
struct BalStorageChangeEntry = {
    slot : word,
    index : block_access_index,
    value : word,
}
/*! One recorded balance change in canonical host traversal order. */
struct BalBalanceChangeEntry = {
    index : block_access_index,
    value : word,
}
/*! One recorded nonce change in canonical host traversal order. */
struct BalNonceChangeEntry = {
    index : block_access_index,
    value : account_nonce,
}
/*! One recorded code change in canonical host traversal order. */
struct BalCodeChangeEntry = {
    index : block_access_index,
    code_hash : hash,
}
/*! One event in the canonical account-delimited BAL stream. */
union BalIterEntry = {
    /* starts one account and carries its address */
    BalAccount : address,
    /* one indexed post-transaction value for a changed storage slot */
    BalStorageChange : BalStorageChangeEntry,
    /* one storage slot that was read but never changed */
    BalStorageRead : word,
    /* one indexed post-transaction balance */
    BalBalanceChange : BalBalanceChangeEntry,
    /* one indexed post-transaction nonce */
    BalNonceChange : BalNonceChangeEntry,
    /* one indexed post-transaction code hash */
    BalCodeChange : BalCodeChangeEntry,
    /* ends the current account */
    BalAccountEnd : unit,
    /* marks the end of the complete stream */
    BalEmpty : unit,
}
/*! Canonicalizes the recorded keyed tables and resets the flat iterator. */
val bal_prepare_iter = impure { c: "bal_prepare_iter" } : unit -> unit
/*! Pops the next event from the canonical account-delimited BAL stream. */
val bal_iter_next = impure { c: "bal_iter_next" } : unit -> BalIterEntry

/* EIP-2929 warm address/slot sets */
/*! Clears both warm sets (per-transaction reset; EIP-2929 warmth is
    transaction-scoped). */
val warm_reset = impure { c: "warm_reset" } : block_access_index -> unit
/*! Returns whether the address is already warm without mutating the warm set. */
val account_is_warm = impure { c: "account_is_warm" } : address -> bool
/*! Marks the address warm after its access charge has been paid. */
val account_mark_warm = impure { c: "account_mark_warm" } : address -> unit
/*! Returns whether `(address, slot)` is already warm without mutation. */
val storage_is_warm = impure { c: "storage_is_warm" } : (address, word) -> bool
/*! Marks `(address, slot)` warm after its access charge has been paid. */
val storage_mark_warm = impure { c: "storage_mark_warm" } : (address, word) -> unit

/* EIP-7702 transaction-local authority bookkeeping */
/*! Clears and pre-sizes the authorization tracker for one transaction. */
val authorization_tracker_reset = impure { c: "authorization_tracker_reset" } : prepared_authorization_count -> unit
/*! Whether a successfully applied authorization for this authority was seen. */
val authorization_tracker_seen = impure { c: "authorization_tracker_seen" } : address -> bool
/*! Whether the authority was delegated before its first successful tuple. */
val authorization_tracker_originally_delegated = impure { c: "authorization_tracker_originally_delegated" } : address -> bool
/*! Whether this transaction has successfully set a delegation for the authority. */
val authorization_tracker_delegation_set = impure { c: "authorization_tracker_delegation_set" } : address -> bool
/*! Records one successfully applied authorization. */
val authorization_tracker_commit = impure { c: "authorization_tracker_commit" } : (address, bool, bool) -> unit

/* the per-transaction log store */
/*! Starts a new transaction-local view in the block-lifetime log store. */
val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit
/*! Begins a log record for `address`. */
val log_begin = impure { c: "log_begin" } : address -> unit
/*! Appends one topic to the current record. */
val log_add_topic = impure { c: "log_add_topic" } : word -> unit
/*! Copies an EVM-memory payload into the current retained log record. */
val log_add_data_memory = impure { c: "log_add_data_memory" } : EvmMemorySlice -> unit
/*! Appends one canonical big-endian word payload to the current record. */
val log_add_data_word = impure { c: "log_add_data_word" } : word -> unit
/*! Start and length of the current transaction's retained log range. */
val logs_tx_start = impure { c: "logs_tx_start" } : unit -> log_store_index
/*! Number of retained logs emitted by the current transaction. */
val logs_tx_count = impure { c: "logs_tx_count" } : unit -> log_store_index
/*! Indexed access to a retained log record. */
val log_address = impure { c: "log_addr" } : log_store_index -> address
/*! Number of topics attached to an indexed retained log. */
val log_topics_count = impure { c: "log_topic_count" } : log_store_index -> log_store_index
/*! Reads one topic from an indexed retained log. */
val log_topic = impure { c: "log_topic" } : (log_store_index, log_store_index) -> word
/*! Offset of an indexed retained log's payload in the host log-data arena. */
val log_data_offset = impure { c: "log_data_off" } : log_store_index -> log_data_pointer
/*! Byte length of an indexed retained log's payload. */
val log_data_length = impure { c: "log_data_len" } : log_store_index -> log_data_length
