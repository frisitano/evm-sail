val transient_reset = impure { c: "transient_storage_reset" } : unit -> unit
val transient_store = impure { c: "transient_storage_write" } : (address, word, word) -> unit
val transient_load = impure { c: "transient_storage_read" } : (address, word) -> word

val storage_tx_cache = impure { c: "storage_tx_cache" } : (StorageKey, word) -> unit
val storage_tx_update = impure { c: "storage_tx_update" } : (StorageKey, word) -> unit
val storage_tx_get = impure { c: "storage_tx_get" } : StorageKey -> option(StorageValue)
val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> option(StorageMergeEntry)
val storage_tx_checkpoint = impure { c: "storage_tx_checkpoint" } : unit -> StorageCheckpoint
val storage_tx_revert = impure { c: "storage_tx_revert" } : StorageCheckpoint -> unit
val storage_tx_clear = impure { c: "storage_tx_clear" } : address -> unit
val storage_tx_reset = impure { c: "storage_tx_reset" } : unit -> unit
val storage_has_writes = impure { c: "storage_has_writes" } : address -> bool

val storage_block_get = impure { c: "storage_block_get" } : StorageKey -> option(StorageValue)
val storage_block_put = impure { c: "storage_block_put" } : StorageEntry -> unit
val storage_block_cache = impure { c: "storage_block_cache" } : (StorageKey, word) -> unit
val storage_block_clear = impure { c: "storage_block_clear" } : address -> unit
val storage_block_count = impure { c: "storage_block_count" } : address -> bits(64)
val storage_block_changed = impure { c: "storage_block_changed" } : (address, bits(64)) -> bool
val storage_block_slot = impure { c: "storage_block_slot" } : (address, bits(64)) -> word
val storage_block_current = impure { c: "storage_block_current" } : (address, bits(64)) -> word

val acct_tx_get = impure { c: "acct_tx_get" } : address -> option(Account)
val acct_tx_update = impure { c: "acct_tx_update" } : (address, Account) -> unit
val acct_tx_set_balance = impure { c: "acct_tx_set_balance" } : (address, word) -> unit
val acct_tx_set_nonce = impure { c: "acct_tx_set_nonce" } : (address, u64) -> unit
val acct_tx_set_code_hash = impure { c: "acct_tx_set_code_hash" } : (address, hash) -> unit
val acct_tx_cache = impure { c: "acct_tx_cache" } : (address, Account) -> unit
val acct_tx_pop = impure { c: "acct_tx_pop" } : unit -> option(AcctEntry)
val acct_tx_checkpoint = impure { c: "acct_tx_checkpoint" } : unit -> AccountCheckpoint
val acct_tx_revert = impure { c: "acct_tx_revert" } : AccountCheckpoint -> unit
val acct_tx_reset = impure { c: "acct_tx_reset" } : unit -> unit

val acct_block_get = impure { c: "acct_block_get" } : address -> option(Account)
val acct_block_write = impure { c: "acct_block_write" } : AcctEntry -> unit
val acct_block_cache = impure { c: "acct_block_cache" } : (address, Account) -> unit
val acct_block_count = impure { c: "acct_block_count" } : unit -> bits(64)
val acct_block_changed = impure { c: "acct_block_changed" } : bits(64) -> bool
val acct_block_address = impure { c: "acct_block_address" } : bits(64) -> address

val bal_storage_change = impure { c: "bal_note_storage_change" } : (address, word, word) -> unit
val bal_storage_read = impure { c: "bal_note_storage_read" } : (address, word) -> unit
val bal_balance_change = impure { c: "bal_note_balance_change" } : (address, word) -> unit
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (address, bits(64)) -> unit
val bal_code_change = impure { c: "bal_note_code_change" } : (address, hash) -> unit

val warm_reset = impure { c: "warm_reset" } : unit -> unit
val warm_addr_touch = impure { c: "warm_addr_touch" } : address -> bool
val warm_addr_remove = impure { c: "warm_addr_remove" } : address -> unit
val warm_slot_touch = impure { c: "warm_slot_touch" } : (address, word) -> bool
val warm_slot_remove = impure { c: "warm_slot_remove" } : (address, word) -> unit

val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit
val log_append = impure { c: "log_append_record" } : (address, list(word), Bytes) -> unit
val log_drop_last = impure { c: "log_drop_last" } : unit -> unit
val read_logs = impure { c: "logs_read_all" } : unit -> list(LogEntry)

val journal_reset = impure { c: "journal_reset" } : unit -> unit
val journal_len = impure { c: "journal_len" } : unit -> JournalCheckpoint
val journal_push = impure { c: "journal_push" } : JEntry -> unit
val journal_pop = impure { c: "journal_pop" } : unit -> JEntry
