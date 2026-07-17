val transient_reset = impure { c: "transient_storage_reset" } : unit -> unit
val transient_store = impure { c: "transient_storage_write" } : (address, word, word) -> unit
val transient_load = impure { c: "transient_storage_read" } : (address, word) -> word

val storage_tx_update = impure { c: "storage_tx_update" } : StorageEntry -> unit
val storage_tx_get = impure { c: "storage_tx_get" } : StorageKey -> option(StorageValue)
val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> option(StorageEntry)
val storage_tx_checkpoint = impure { c: "storage_tx_checkpoint" } : unit -> StorageCheckpoint
val storage_tx_revert = impure { c: "storage_tx_revert" } : StorageCheckpoint -> unit
val storage_tx_clear = impure { c: "storage_tx_clear" } : address -> unit
val storage_tx_reset = impure { c: "storage_tx_reset" } : unit -> unit
val storage_has_writes = impure { c: "storage_has_writes" } : address -> bool

val storage_block_get = impure { c: "storage_block_get" } : StorageKey -> option(StorageValue)
val storage_block_put = impure { c: "storage_block_put" } : StorageEntry -> unit
val storage_block_cache = impure { c: "storage_block_cache" } : (StorageKey, word) -> unit
val storage_block_clear = impure { c: "storage_block_clear" } : address -> unit
val storage_block_count = impure { c: "storage_block_count" } : address -> item_count
val storage_block_row = impure { c: "storage_block_row" } : (address, item_index) -> option(StorageEntry)

val acct_tx_get = impure { c: "acct_tx_get" } : address -> option(Account)
val acct_tx_update = impure { c: "acct_tx_update" } : (address, Account) -> unit
val acct_tx_set_balance = impure { c: "acct_tx_set_balance" } : (address, word) -> unit
val acct_tx_set_nonce = impure { c: "acct_tx_set_nonce" } : (address, account_nonce) -> unit
val acct_tx_set_code_hash = impure { c: "acct_tx_set_code_hash" } : (address, hash) -> unit
val acct_tx_pop_ascending = impure { c: "acct_tx_pop_ascending" } : unit -> option(AcctEntry)
val acct_tx_checkpoint = impure { c: "acct_tx_checkpoint" } : unit -> AccountCheckpoint
val acct_tx_revert = impure { c: "acct_tx_revert" } : AccountCheckpoint -> unit
val acct_tx_reset = impure { c: "acct_tx_reset" } : unit -> unit

val acct_block_get = impure { c: "acct_block_get" } : address -> option(Account)
val acct_block_write = impure { c: "acct_block_write" } : AcctEntry -> unit
val acct_block_cache = impure { c: "acct_block_cache" } : (address, Account) -> unit
val acct_block_count = impure { c: "acct_block_count" } : unit -> item_count
val acct_block_row = impure { c: "acct_block_row" } : item_index -> option(AcctEntry)

val bal_reset = impure { c: "bal_reset" } : unit -> unit
val bal_set_index = impure { c: "bal_set_index" } : item_index -> unit
val bal_account_touch = impure { c: "bal_note_account_touch" } : address -> unit
val bal_storage_change = impure { c: "bal_note_storage_change" } : (address, word, word) -> unit
val bal_storage_read = impure { c: "bal_note_storage_read" } : (address, word) -> unit
val bal_balance_change = impure { c: "bal_note_balance_change" } : (address, word) -> unit
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (address, account_nonce) -> unit
val bal_code_change = impure { c: "bal_note_code_change" } : (address, hash) -> unit
val bal_prepare = impure { c: "bal_prepare" } : unit -> unit
val bal_account_count = impure { c: "bal_account_count" } : unit -> item_count
val bal_account_address = impure { c: "bal_account_address" } : item_index -> address
val bal_storage_change_count = impure { c: "bal_storage_change_count" } : item_index -> item_count
val bal_storage_change_slot = impure { c: "bal_storage_change_slot" } : (item_index, item_index) -> word
val bal_storage_change_index = impure { c: "bal_storage_change_index" } : (item_index, item_index) -> item_index
val bal_storage_change_value = impure { c: "bal_storage_change_value" } : (item_index, item_index) -> word
val bal_storage_read_count = impure { c: "bal_storage_read_count" } : item_index -> item_count
val bal_storage_read_slot = impure { c: "bal_storage_read_slot" } : (item_index, item_index) -> word
val bal_balance_change_count = impure { c: "bal_balance_change_count" } : item_index -> item_count
val bal_balance_change_index = impure { c: "bal_balance_change_index" } : (item_index, item_index) -> item_index
val bal_balance_change_value = impure { c: "bal_balance_change_value" } : (item_index, item_index) -> word
val bal_nonce_change_count = impure { c: "bal_nonce_change_count" } : item_index -> item_count
val bal_nonce_change_index = impure { c: "bal_nonce_change_index" } : (item_index, item_index) -> item_index
val bal_nonce_change_value = impure { c: "bal_nonce_change_value" } : (item_index, item_index) -> account_nonce
val bal_code_change_count = impure { c: "bal_code_change_count" } : item_index -> item_count
val bal_code_change_index = impure { c: "bal_code_change_index" } : (item_index, item_index) -> item_index
val bal_code_change_hash = impure { c: "bal_code_change_hash" } : (item_index, item_index) -> hash

val warm_reset = impure { c: "warm_reset" } : unit -> unit
val warm_addr_touch = impure { c: "warm_addr_touch" } : address -> bool
val warm_addr_remove = impure { c: "warm_addr_remove" } : address -> unit
val warm_slot_touch = impure { c: "warm_slot_touch" } : (address, word) -> bool
val warm_slot_remove = impure { c: "warm_slot_remove" } : (address, word) -> unit

val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit
val log_append = impure { c: "log_append_record" } : (address, list(word), Bytes) -> unit
val logs_checkpoint = impure { c: "logs_checkpoint" } : unit -> LogCheckpoint
val logs_revert = impure { c: "logs_revert" } : LogCheckpoint -> unit
val read_logs = impure { c: "logs_read_all" } : unit -> list(LogEntry)

val journal_reset = impure { c: "journal_reset" } : unit -> unit
val journal_len = impure { c: "journal_len" } : unit -> JournalCheckpoint
val journal_push = impure { c: "journal_push" } : JEntry -> unit
val journal_pop = impure { c: "journal_pop" } : unit -> JEntry
