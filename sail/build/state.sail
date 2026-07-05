/* Build backend: C FFI bindings for transient storage, persistent storage,
 * and the account map (ffi/transient_storage.c, ffi/state_db.c). */

val transient_storage_reset_ffi = impure { _: "transient_storage_reset" } : bits(64) -> unit
function transient_storage_reset(x0 : bits(64)) -> unit = transient_storage_reset_ffi(x0)

val transient_storage_key_ffi = impure { _: "transient_storage_key" } : (bits(64), address, word) -> unit
function transient_storage_key(x0 : bits(64), a : address, s : word) -> unit = transient_storage_key_ffi(x0, a, s)

val transient_storage_write_ffi = impure { _: "transient_storage_write" } : word -> unit
function transient_storage_write(v : word) -> unit = transient_storage_write_ffi(v)

val transient_storage_read_ffi = impure { _: "transient_storage_read" } : unit -> word
function transient_storage_read() -> word = transient_storage_read_ffi()

val storage_map_reset_ffi = impure { _: "storage_map_reset" } : unit -> unit
function storage_map_reset() -> unit = storage_map_reset_ffi()

val storage_map_push_ffi = impure { _: "storage_map_push" } : unit -> unit
function storage_map_push() -> unit = storage_map_push_ffi()

val storage_map_pop_commit_ffi = impure { _: "storage_map_pop_commit" } : unit -> unit
function storage_map_pop_commit() -> unit = storage_map_pop_commit_ffi()

val storage_map_pop_discard_ffi = impure { _: "storage_map_pop_discard" } : unit -> unit
function storage_map_pop_discard() -> unit = storage_map_pop_discard_ffi()

val storage_map_key_ffi = impure { _: "storage_map_key" } : (word, hash, hash) -> unit
function storage_map_key(s : word, ah : hash, sh : hash) -> unit = storage_map_key_ffi(s, ah, sh)

val storage_map_seed_ffi = impure { _: "storage_map_seed" } : word -> unit
function storage_map_seed(v : word) -> unit = storage_map_seed_ffi(v)

val storage_map_store_ffi = impure { _: "storage_map_store" } : word -> unit
function storage_map_store(v : word) -> unit = storage_map_store_ffi(v)

val storage_map_value_ffi = impure { _: "storage_map_value" } : unit -> word
function storage_map_value() -> word = storage_map_value_ffi()

val storage_map_base_value_ffi = impure { _: "storage_map_base_value" } : unit -> word
function storage_map_base_value() -> word = storage_map_base_value_ffi()

val storage_map_present_ffi = impure { _: "storage_map_present" } : unit -> bool
function storage_map_present() -> bool = storage_map_present_ffi()

val storage_map_base_present_ffi = impure { _: "storage_map_base_present" } : unit -> bool
function storage_map_base_present() -> bool = storage_map_base_present_ffi()

val storage_map_wipe_acct_hash_ffi = impure { _: "storage_map_wipe_acct_hash" } : hash -> unit
function storage_map_wipe_acct_hash(ah : hash) -> unit = storage_map_wipe_acct_hash_ffi(ah)

val storage_map_count_ffi = impure { _: "storage_map_count" } : unit -> bits(64)
function storage_map_count() -> bits(64) = storage_map_count_ffi()

val storage_map_update_count_ffi = impure { _: "storage_map_update_count" } : unit -> bits(64)
function storage_map_update_count() -> bits(64) = storage_map_update_count_ffi()

val storage_map_at_ffi = impure { _: "storage_map_at" } : bits(64) -> unit
function storage_map_at(x0 : bits(64)) -> unit = storage_map_at_ffi(x0)

val storage_map_update_at_ffi = impure { _: "storage_map_update_at" } : bits(64) -> unit
function storage_map_update_at(x0 : bits(64)) -> unit = storage_map_update_at_ffi(x0)

val storage_map_it_acct_hash_ffi = impure { _: "storage_map_it_acct_hash" } : unit -> hash
function storage_map_it_acct_hash() -> hash = storage_map_it_acct_hash_ffi()

val storage_map_it_slot_ffi = impure { _: "storage_map_it_slot" } : unit -> word
function storage_map_it_slot() -> word = storage_map_it_slot_ffi()

val storage_map_it_val_ffi = impure { _: "storage_map_it_val" } : unit -> word
function storage_map_it_val() -> word = storage_map_it_val_ffi()

val acctmap_reset_ffi = impure { _: "acctmap_reset" } : unit -> unit
function acctmap_reset() -> unit = acctmap_reset_ffi()

val acctmap_key_ffi = impure { _: "acctmap_key" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function acctmap_key(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = acctmap_key_ffi(
    x0,
    x1,
    x2,
    x3,
)

val acctmap_present_ffi = impure { _: "acctmap_present" } : unit -> bool
function acctmap_present() -> bool = acctmap_present_ffi()

val acctmap_store_ffi = impure { _: "acctmap_store" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function acctmap_store(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
    x12 : bits(64),
    x13 : bits(64),
    x14 : bits(64),
    x15 : bits(64),
    x16 : bits(64),
) -> (
    unit
) =
    acctmap_store_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16)

val acctmap_mark_base_exists_ffi = impure { _: "acctmap_mark_base_exists" } : unit -> unit
function acctmap_mark_base_exists() -> unit = acctmap_mark_base_exists_ffi()

val acctmap_seed_ffi = impure { _: "acctmap_seed" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function acctmap_seed(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
    x12 : bits(64),
    x13 : bits(64),
    x14 : bits(64),
    x15 : bits(64),
    x16 : bits(64),
) -> (
    unit
) =
    acctmap_seed_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16)

val acctmap_nonce_ffi = impure { _: "acctmap_nonce" } : unit -> bits(64)
function acctmap_nonce() -> bits(64) = acctmap_nonce_ffi()

val acctmap_bal_ffi = impure { _: "acctmap_bal" } : bits(64) -> bits(64)
function acctmap_bal(x0 : bits(64)) -> bits(64) = acctmap_bal_ffi(x0)

val acctmap_sroot_ffi = impure { _: "acctmap_sroot" } : bits(64) -> bits(64)
function acctmap_sroot(x0 : bits(64)) -> bits(64) = acctmap_sroot_ffi(x0)

val acctmap_chash_ffi = impure { _: "acctmap_chash" } : bits(64) -> bits(64)
function acctmap_chash(x0 : bits(64)) -> bits(64) = acctmap_chash_ffi(x0)

val acctmap_remove_ffi = impure { _: "acctmap_remove" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function acctmap_remove(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = acctmap_remove_ffi(
    x0,
    x1,
    x2,
    x3,
)

val acctmap_count_ffi = impure { _: "acctmap_count" } : unit -> bits(64)
function acctmap_count() -> bits(64) = acctmap_count_ffi()

val acctmap_update_count_ffi = impure { _: "acctmap_update_count" } : unit -> bits(64)
function acctmap_update_count() -> bits(64) = acctmap_update_count_ffi()

val acctmap_at_ffi = impure { _: "acctmap_at" } : bits(64) -> unit
function acctmap_at(x0 : bits(64)) -> unit = acctmap_at_ffi(x0)

val acctmap_update_at_ffi = impure { _: "acctmap_update_at" } : bits(64) -> unit
function acctmap_update_at(x0 : bits(64)) -> unit = acctmap_update_at_ffi(x0)

val acctmap_at_hkey_ffi = impure { _: "acctmap_at_hkey" } : bits(64) -> bits(64)
function acctmap_at_hkey(x0 : bits(64)) -> bits(64) = acctmap_at_hkey_ffi(x0)

val acctmap_at_nonce_ffi = impure { _: "acctmap_at_nonce" } : unit -> bits(64)
function acctmap_at_nonce() -> bits(64) = acctmap_at_nonce_ffi()

val acctmap_at_bal_ffi = impure { _: "acctmap_at_bal" } : bits(64) -> bits(64)
function acctmap_at_bal(x0 : bits(64)) -> bits(64) = acctmap_at_bal_ffi(x0)

val acctmap_at_sroot_ffi = impure { _: "acctmap_at_sroot" } : bits(64) -> bits(64)
function acctmap_at_sroot(x0 : bits(64)) -> bits(64) = acctmap_at_sroot_ffi(x0)

val acctmap_at_chash_ffi = impure { _: "acctmap_at_chash" } : bits(64) -> bits(64)
function acctmap_at_chash(x0 : bits(64)) -> bits(64) = acctmap_at_chash_ffi(x0)

val acctmap_at_base_exists_ffi = impure { _: "acctmap_at_base_exists" } : unit -> bool
function acctmap_at_base_exists() -> bool = acctmap_at_base_exists_ffi()
