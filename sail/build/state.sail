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

val acctmap_key_ffi = impure { _: "acctmap_key" } : hash -> unit
function acctmap_key(h : hash) -> unit = acctmap_key_ffi(h)

val acctmap_present_ffi = impure { _: "acctmap_present" } : unit -> bool
function acctmap_present() -> bool = acctmap_present_ffi()

val acctmap_store_ffi = impure { _: "acctmap_store" } : (hash, bits(64), word, hash, hash) -> unit
function acctmap_store(h : hash, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    acctmap_store_ffi(h, nonce, bal, sroot, chash)

val acctmap_mark_base_exists_ffi = impure { _: "acctmap_mark_base_exists" } : unit -> unit
function acctmap_mark_base_exists() -> unit = acctmap_mark_base_exists_ffi()

val acctmap_seed_ffi = impure { _: "acctmap_seed" } : (hash, bits(64), word, hash, hash) -> unit
function acctmap_seed(h : hash, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    acctmap_seed_ffi(h, nonce, bal, sroot, chash)

val acctmap_nonce_ffi = impure { _: "acctmap_nonce" } : unit -> bits(64)
function acctmap_nonce() -> bits(64) = acctmap_nonce_ffi()

val acctmap_bal_ffi = impure { _: "acctmap_bal" } : unit -> word
function acctmap_bal() -> word = acctmap_bal_ffi()

val acctmap_sroot_ffi = impure { _: "acctmap_sroot" } : unit -> hash
function acctmap_sroot() -> hash = acctmap_sroot_ffi()

val acctmap_chash_ffi = impure { _: "acctmap_chash" } : unit -> hash
function acctmap_chash() -> hash = acctmap_chash_ffi()

val acctmap_remove_ffi = impure { _: "acctmap_remove" } : hash -> unit
function acctmap_remove(h : hash) -> unit = acctmap_remove_ffi(h)

val acctmap_count_ffi = impure { _: "acctmap_count" } : unit -> bits(64)
function acctmap_count() -> bits(64) = acctmap_count_ffi()

val acctmap_update_count_ffi = impure { _: "acctmap_update_count" } : unit -> bits(64)
function acctmap_update_count() -> bits(64) = acctmap_update_count_ffi()

val acctmap_at_ffi = impure { _: "acctmap_at" } : bits(64) -> unit
function acctmap_at(x0 : bits(64)) -> unit = acctmap_at_ffi(x0)

val acctmap_update_at_ffi = impure { _: "acctmap_update_at" } : bits(64) -> unit
function acctmap_update_at(x0 : bits(64)) -> unit = acctmap_update_at_ffi(x0)

val acctmap_at_hkey_ffi = impure { _: "acctmap_at_hkey" } : unit -> hash
function acctmap_at_hkey() -> hash = acctmap_at_hkey_ffi()

val acctmap_at_nonce_ffi = impure { _: "acctmap_at_nonce" } : unit -> bits(64)
function acctmap_at_nonce() -> bits(64) = acctmap_at_nonce_ffi()

val acctmap_at_bal_ffi = impure { _: "acctmap_at_bal" } : unit -> word
function acctmap_at_bal() -> word = acctmap_at_bal_ffi()

val acctmap_at_sroot_ffi = impure { _: "acctmap_at_sroot" } : unit -> hash
function acctmap_at_sroot() -> hash = acctmap_at_sroot_ffi()

val acctmap_at_chash_ffi = impure { _: "acctmap_at_chash" } : unit -> hash
function acctmap_at_chash() -> hash = acctmap_at_chash_ffi()

val acctmap_at_base_exists_ffi = impure { _: "acctmap_at_base_exists" } : unit -> bool
function acctmap_at_base_exists() -> bool = acctmap_at_base_exists_ffi()
