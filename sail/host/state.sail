val transient_reset = impure { c: "transient_storage_reset" } : unit -> unit
val transient_store = impure { c: "transient_storage_write" } : (address, word, word) -> unit
val transient_load = impure { c: "transient_storage_read" } : (address, word) -> word

val storage_tx_cache = impure { c: "storage_tx_cache" } : (address, word, word) -> unit
val storage_tx_update = impure { c: "storage_tx_update" } : (address, word, word) -> unit
val storage_tx_get = impure { c: "storage_tx_get" } : (address, word) -> option(StorageValue)
val storage_tx_pop = impure { c: "storage_tx_pop" } : unit -> option(StorageEntry)
val storage_tx_wipe = impure { c: "storage_tx_wipe" } : address -> option(word)

val storage_block_get = impure { c: "storage_block_get" } : (address, word) -> option(StorageValue)
val storage_block_put = impure { c: "storage_block_put" } : (address, word, word, word) -> unit
val storage_block_cache = impure { c: "storage_block_cache" } : (address, word, word) -> unit
val storage_block_wipe = impure { c: "storage_block_wipe" } : address -> unit
val storage_block_row_count = impure { c: "storage_block_row_count" } : address -> bits(64)
val storage_block_row = impure { c: "storage_block_row" } : (address, bits(64)) -> StorageEntry

val storage_wset_union_count = impure { c: "storage_wset_union_count" } : hash -> bits(64)
val storage_wset_union_slot = impure { c: "storage_wset_union_slot" } : (hash, bits(64)) -> word
val storage_wset_union_val = impure { c: "storage_wset_union_val" } : (hash, bits(64)) -> word

val acct_tx_get = impure { c: "acct_tx_get" } : address -> option(Account)
val acct_tx_update = impure { c: "acct_tx_update" } : (address, bits(64), word, hash, hash) -> unit
val acct_tx_cache = impure { c: "acct_tx_cache" } : (address, bits(64), word, hash, hash) -> unit
val acct_tx_pop = impure { c: "acct_tx_pop" } : unit -> option(AcctEntry)

val acct_block_get = impure { c: "acct_block_get" } : address -> option(Account)
val acct_block_write = impure { c: "acct_block_write" } : (
        address,
        bits(64),
        word,
        hash,
        hash,
        bits(64),
        word,
        hash,
        hash,
    ) -> unit
val acct_block_cache = impure { c: "acct_block_cache" } : (address, bits(64), word, hash, hash) -> unit
val acct_block_row_count = impure { c: "acct_block_row_count" } : unit -> bits(64)
val acct_block_row = impure { c: "acct_block_row" } : bits(64) -> AcctEntry

val bal_reset = impure { c: "bal_reset" } : unit -> unit
val bal_set_index = impure { c: "bal_set_index" } : bits(64) -> unit
val bal_storage_change = impure { c: "bal_note_storage_change" } : (address, word, word) -> unit
val bal_storage_read = impure { c: "bal_note_storage_read" } : (address, word) -> unit
val bal_balance_change = impure { c: "bal_note_balance_change" } : (address, word) -> unit
val bal_nonce_change = impure { c: "bal_note_nonce_change" } : (address, bits(64)) -> unit
val bal_code_change = impure { c: "bal_note_code_change" } : (address, hash) -> unit
val bal_recompute_hash = impure { c: "bal_recompute_hash" } : unit -> hash

val code_db_stored_code_length = impure { c: "code_db_stored_code_length" } : hash -> bits(64)
val code_db_stored_code_offset = impure { c: "code_db_stored_code_offset" } : hash -> bits(64)
val code_db_stored_jumpdest_ref = impure { c: "code_db_stored_jumpdest_ref" } : hash -> JumpdestRef
val jumpdest_ref_contains = impure { c: "jumpdest_ref_contains" } : (JumpdestRef, bits(64), bits(64)) -> bool
val code_db_byte_at = impure { c: "code_db_byte_at" } : bits(64) -> bits(8)
val code_db_copy_stored_code_to_memory = impure { c: "code_db_copy_stored_code_to_memory" } : (
        hash,
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val code_db_store_indexed_source = impure { c: "code_db_store_indexed_source" } : (
        bits(64),
        bits(64),
        bits(64),
        JumpdestBitmap,
    ) -> hash
val code_intern_indexed_delegation = impure { c: "code_intern_indexed_delegation" } : (address, JumpdestBitmap) -> hash
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> bits(168)

val warm_reset = impure { c: "warm_reset" } : unit -> unit
val warm_addr_contains = impure { c: "warm_addr_contains" } : address -> bool
val warm_addr_insert = impure { c: "warm_addr_insert" } : address -> unit
val warm_addr_remove = impure { c: "warm_addr_remove" } : address -> unit
val warm_slot_contains = impure { c: "warm_slot_contains" } : (address, word) -> bool
val warm_slot_insert = impure { c: "warm_slot_insert" } : (address, word) -> unit
val warm_slot_remove = impure { c: "warm_slot_remove" } : (address, word) -> unit

val logs_tx_reset = impure { c: "logs_tx_reset" } : unit -> unit
val log_append = impure { c: "log_append_record" } : (address, list(word), Bytes) -> unit
val log_drop_last = impure { c: "log_drop_last" } : unit -> unit
val read_logs = impure { c: "logs_read_all" } : unit -> list(LogEntry)
val log_arena_byte = impure { c: "log_arena_byte" } : bits(64) -> bits(8)
val mem_arena_byte_read = impure { c: "mem_arena_byte" } : bits(64) -> bits(8)

val selfdestr_reset = impure { c: "selfdestr_reset" } : unit -> unit
val selfdestr_push = impure { c: "selfdestr_push" } : address -> unit
val selfdestr_drop_last = impure { c: "selfdestr_drop_last" } : unit -> unit
val selfdestr_contains = impure { c: "selfdestr_contains" } : address -> bool
val selfdestr_count = impure { c: "selfdestr_count" } : unit -> bits(64)
val selfdestr_get = impure { c: "selfdestr_get" } : bits(64) -> word

val created_reset = impure { c: "created_reset" } : unit -> unit
val created_insert = impure { c: "created_insert" } : address -> unit
val created_contains = impure { c: "created_contains" } : address -> bool

val journal_reset = impure { c: "journal_reset" } : unit -> unit
val journal_commit = impure { c: "journal_commit" } : unit -> unit
val journal_push = impure { c: "journal_push" } : JEntry -> unit
val journal_pop = impure { c: "journal_pop" } : unit -> JEntry

val headerhash_set = impure { c: "headerhash_set" } : (bits(64), hash) -> unit
val headerhash_get = impure { c: "headerhash_get" } : bits(64) -> hash

val stateless_storage : (address, word) -> word
val stateless_account : address -> option(Account)
