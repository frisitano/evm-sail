/* Build backend: C FFI bindings for transient storage, persistent storage,
 * and the account map (ffi/transient_storage.c, ffi/state_db.c). */

val transient_storage_reset_ffi = impure { _: "transient_storage_reset" } : bits(64) -> unit
function transient_storage_reset(x0 : bits(64)) -> unit = transient_storage_reset_ffi(x0)

val transient_storage_write_ffi = impure { _: "transient_storage_write" } : (bits(64), address, word, word) -> unit
function transient_storage_write(x0 : bits(64), a : address, s : word, v : word) -> unit =
    transient_storage_write_ffi(x0, a, s, v)

val transient_storage_read_ffi = impure { _: "transient_storage_read" } : (bits(64), address, word) -> word
function transient_storage_read(x0 : bits(64), a : address, s : word) -> word = transient_storage_read_ffi(x0, a, s)

val storage_map_reset_ffi = impure { _: "storage_map_reset" } : unit -> unit
function storage_map_reset() -> unit = storage_map_reset_ffi()

val storage_map_seed_ffi = impure { _: "storage_map_seed" } : (address, word, word) -> unit
function storage_map_seed(a : address, s : word, v : word) -> unit = storage_map_seed_ffi(a, s, v)

val storage_cache_present_ffi = impure { _: "storage_cache_present" } : (address, word) -> bool
function storage_cache_present(a : address, s : word) -> bool = storage_cache_present_ffi(a, s)

val storage_cache_load_ffi = impure { _: "storage_cache_load" } : (address, word) -> word
function storage_cache_load(a : address, s : word) -> word = storage_cache_load_ffi(a, s)

val storage_map_wipe_addr_ffi = impure { _: "storage_map_wipe_addr" } : address -> unit
function storage_map_wipe_addr(a : address) -> unit = storage_map_wipe_addr_ffi(a)

/* ---- write-set storage overlay (ffi/state_db.c storage_wset_*) ---- */
val storage_wset_reset_ffi = impure { _: "storage_wset_reset" } : unit -> unit
function storage_wset_reset() -> unit = storage_wset_reset_ffi()

val storage_wset_tx_clear_ffi = impure { _: "storage_wset_tx_clear" } : unit -> unit
function storage_wset_tx_clear() -> unit = storage_wset_tx_clear_ffi()

val storage_wset_merge_ffi = impure { _: "storage_wset_merge" } : unit -> unit
function storage_wset_merge() -> unit = storage_wset_merge_ffi()

val storage_wset_present_ffi = impure { _: "storage_wset_present" } : (address, word) -> bool
function storage_wset_present(a : address, s : word) -> bool = storage_wset_present_ffi(a, s)

val storage_wset_load_ffi = impure { _: "storage_wset_load" } : (address, word) -> word
function storage_wset_load(a : address, s : word) -> word = storage_wset_load_ffi(a, s)

val storage_wset_base_present_ffi = impure { _: "storage_wset_base_present" } : (address, word) -> bool
function storage_wset_base_present(a : address, s : word) -> bool = storage_wset_base_present_ffi(a, s)

val storage_wset_base_load_ffi = impure { _: "storage_wset_base_load" } : (address, word) -> word
function storage_wset_base_load(a : address, s : word) -> word = storage_wset_base_load_ffi(a, s)

val storage_wset_touch_ffi = impure { _: "storage_wset_touch" } : (address, word) -> unit
function storage_wset_touch(a : address, s : word) -> unit = storage_wset_touch_ffi(a, s)

val storage_wset_prior_ffi = impure { _: "storage_wset_prior" } : (address, word, word) -> word
function storage_wset_prior(a : address, s : word, orig : word) -> word = storage_wset_prior_ffi(a, s, orig)

val storage_wset_write_ffi = impure { _: "storage_wset_write" } : (address, word, word, word) -> unit
function storage_wset_write(a : address, s : word, v : word, orig : word) -> unit = storage_wset_write_ffi(a, s, v, orig)

val storage_wset_restore_ffi = impure { _: "storage_wset_restore" } : (address, word, word) -> unit
function storage_wset_restore(a : address, s : word, prior : word) -> unit = storage_wset_restore_ffi(a, s, prior)

val storage_wset_warm_ffi = impure { _: "storage_wset_warm" } : (address, word) -> bool
function storage_wset_warm(a : address, s : word) -> bool = storage_wset_warm_ffi(a, s)

val storage_wset_is_warm_ffi = impure { _: "storage_wset_is_warm" } : (address, word) -> bool
function storage_wset_is_warm(a : address, s : word) -> bool = storage_wset_is_warm_ffi(a, s)

val storage_wset_unwarm_ffi = impure { _: "storage_wset_unwarm" } : (address, word) -> unit
function storage_wset_unwarm(a : address, s : word) -> unit = storage_wset_unwarm_ffi(a, s)

val storage_wset_wipe_addr_ffi = impure { _: "storage_wset_wipe_addr" } : address -> unit
function storage_wset_wipe_addr(a : address) -> unit = storage_wset_wipe_addr_ffi(a)

val storage_wset_block_dirty_count_ffi = impure { _: "storage_wset_block_dirty_count" } : hash -> bits(64)
function storage_wset_block_dirty_count(ak : hash) -> bits(64) = storage_wset_block_dirty_count_ffi(ak)

val storage_wset_block_dirty_slot_ffi = impure { _: "storage_wset_block_dirty_slot" } : (hash, bits(64)) -> word
function storage_wset_block_dirty_slot(ak : hash, j : bits(64)) -> word = storage_wset_block_dirty_slot_ffi(ak, j)

val storage_wset_block_dirty_val_ffi = impure { _: "storage_wset_block_dirty_val" } : (hash, bits(64)) -> word
function storage_wset_block_dirty_val(ak : hash, j : bits(64)) -> word = storage_wset_block_dirty_val_ffi(ak, j)

val storage_wset_union_count_ffi = impure { _: "storage_wset_union_count" } : hash -> bits(64)
function storage_wset_union_count(ak : hash) -> bits(64) = storage_wset_union_count_ffi(ak)

val storage_wset_union_slot_ffi = impure { _: "storage_wset_union_slot" } : (hash, bits(64)) -> word
function storage_wset_union_slot(ak : hash, j : bits(64)) -> word = storage_wset_union_slot_ffi(ak, j)

val storage_wset_union_val_ffi = impure { _: "storage_wset_union_val" } : (hash, bits(64)) -> word
function storage_wset_union_val(ak : hash, j : bits(64)) -> word = storage_wset_union_val_ffi(ak, j)

val acctmap_reset_ffi = impure { _: "acctmap_reset" } : unit -> unit
function acctmap_reset() -> unit = acctmap_reset_ffi()

val acctmap_present_ffi = impure { _: "acctmap_present" } : address -> bool
function acctmap_present(a : address) -> bool = acctmap_present_ffi(a)

val acctmap_store_ffi = impure { _: "acctmap_store" } : (address, bits(64), word, hash, hash) -> unit
function acctmap_store(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    acctmap_store_ffi(a, nonce, bal, sroot, chash)

val acctmap_mark_base_exists_ffi = impure { _: "acctmap_mark_base_exists" } : address -> unit
function acctmap_mark_base_exists(a : address) -> unit = acctmap_mark_base_exists_ffi(a)

val acctmap_seed_ffi = impure { _: "acctmap_seed" } : (address, bits(64), word, hash, hash) -> unit
function acctmap_seed(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    acctmap_seed_ffi(a, nonce, bal, sroot, chash)

val acctmap_nonce_ffi = impure { _: "acctmap_nonce" } : address -> bits(64)
function acctmap_nonce(a : address) -> bits(64) = acctmap_nonce_ffi(a)

val acctmap_bal_ffi = impure { _: "acctmap_bal" } : address -> word
function acctmap_bal(a : address) -> word = acctmap_bal_ffi(a)

val acctmap_sroot_ffi = impure { _: "acctmap_sroot" } : address -> hash
function acctmap_sroot(a : address) -> hash = acctmap_sroot_ffi(a)

val acctmap_chash_ffi = impure { _: "acctmap_chash" } : address -> hash
function acctmap_chash(a : address) -> hash = acctmap_chash_ffi(a)

val acctmap_remove_ffi = impure { _: "acctmap_remove" } : address -> unit
function acctmap_remove(a : address) -> unit = acctmap_remove_ffi(a)

val acctmap_count_ffi = impure { _: "acctmap_count" } : unit -> bits(64)
function acctmap_count() -> bits(64) = acctmap_count_ffi()

val acctmap_update_count_ffi = impure { _: "acctmap_update_count" } : unit -> bits(64)
function acctmap_update_count() -> bits(64) = acctmap_update_count_ffi()

val acctmap_row_hkey_ffi = impure { _: "acctmap_row_hkey" } : bits(64) -> hash
function acctmap_row_hkey(idx : bits(64)) -> hash = acctmap_row_hkey_ffi(idx)

val acctmap_row_nonce_ffi = impure { _: "acctmap_row_nonce" } : bits(64) -> bits(64)
function acctmap_row_nonce(idx : bits(64)) -> bits(64) = acctmap_row_nonce_ffi(idx)

val acctmap_row_bal_ffi = impure { _: "acctmap_row_bal" } : bits(64) -> word
function acctmap_row_bal(idx : bits(64)) -> word = acctmap_row_bal_ffi(idx)

val acctmap_row_sroot_ffi = impure { _: "acctmap_row_sroot" } : bits(64) -> hash
function acctmap_row_sroot(idx : bits(64)) -> hash = acctmap_row_sroot_ffi(idx)

val acctmap_row_chash_ffi = impure { _: "acctmap_row_chash" } : bits(64) -> hash
function acctmap_row_chash(idx : bits(64)) -> hash = acctmap_row_chash_ffi(idx)

val acctmap_row_base_exists_ffi = impure { _: "acctmap_row_base_exists" } : bits(64) -> bool
function acctmap_row_base_exists(idx : bits(64)) -> bool = acctmap_row_base_exists_ffi(idx)

val acctmap_update_row_hkey_ffi = impure { _: "acctmap_update_row_hkey" } : bits(64) -> hash
function acctmap_update_row_hkey(idx : bits(64)) -> hash = acctmap_update_row_hkey_ffi(idx)

val acctmap_update_row_nonce_ffi = impure { _: "acctmap_update_row_nonce" } : bits(64) -> bits(64)
function acctmap_update_row_nonce(idx : bits(64)) -> bits(64) = acctmap_update_row_nonce_ffi(idx)

val acctmap_update_row_bal_ffi = impure { _: "acctmap_update_row_bal" } : bits(64) -> word
function acctmap_update_row_bal(idx : bits(64)) -> word = acctmap_update_row_bal_ffi(idx)

val acctmap_update_row_sroot_ffi = impure { _: "acctmap_update_row_sroot" } : bits(64) -> hash
function acctmap_update_row_sroot(idx : bits(64)) -> hash = acctmap_update_row_sroot_ffi(idx)

val acctmap_update_row_chash_ffi = impure { _: "acctmap_update_row_chash" } : bits(64) -> hash
function acctmap_update_row_chash(idx : bits(64)) -> hash = acctmap_update_row_chash_ffi(idx)

val acctmap_update_row_base_exists_ffi = impure { _: "acctmap_update_row_base_exists" } : bits(64) -> bool
function acctmap_update_row_base_exists(idx : bits(64)) -> bool = acctmap_update_row_base_exists_ffi(idx)

/* ---- write-set account overlay (ffi/state_db.c acct_wset_*) ---- */
val acct_wset_reset_ffi = impure { _: "acct_wset_reset" } : unit -> unit
function acct_wset_reset() -> unit = acct_wset_reset_ffi()

val acct_wset_tx_clear_ffi = impure { _: "acct_wset_tx_clear" } : unit -> unit
function acct_wset_tx_clear() -> unit = acct_wset_tx_clear_ffi()

val acct_wset_merge_ffi = impure { _: "acct_wset_merge" } : unit -> unit
function acct_wset_merge() -> unit = acct_wset_merge_ffi()

val acct_wset_wipe_addr_ffi = impure { _: "acct_wset_wipe_addr" } : address -> unit
function acct_wset_wipe_addr(a : address) -> unit = acct_wset_wipe_addr_ffi(a)

val acct_wset_present_ffi = impure { _: "acct_wset_present" } : address -> bool
function acct_wset_present(a : address) -> bool = acct_wset_present_ffi(a)

val acct_wset_nonce_ffi = impure { _: "acct_wset_nonce" } : address -> bits(64)
function acct_wset_nonce(a : address) -> bits(64) = acct_wset_nonce_ffi(a)

val acct_wset_bal_ffi = impure { _: "acct_wset_bal" } : address -> word
function acct_wset_bal(a : address) -> word = acct_wset_bal_ffi(a)

val acct_wset_sroot_ffi = impure { _: "acct_wset_sroot" } : address -> hash
function acct_wset_sroot(a : address) -> hash = acct_wset_sroot_ffi(a)

val acct_wset_chash_ffi = impure { _: "acct_wset_chash" } : address -> hash
function acct_wset_chash(a : address) -> hash = acct_wset_chash_ffi(a)

val acct_wset_write_ffi = impure { _: "acct_wset_write" } : (address, bits(64), word, hash, hash) -> unit
function acct_wset_write(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    acct_wset_write_ffi(a, nonce, bal, sroot, chash)

val acct_wset_restore_ffi = impure { _: "acct_wset_restore" } : (address, bits(64), word, hash, hash) -> unit
function acct_wset_restore(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    acct_wset_restore_ffi(a, nonce, bal, sroot, chash)
