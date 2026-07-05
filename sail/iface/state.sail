/* ======================================================================== */
/* HOST INTERFACE CONTRACT: WORLD STATE                                     */
/*                                                                          */
/* Abstract `val` declarations for the C-backed account map, persistent     */
/* storage, and transient storage (the "World state and code DB" contract   */
/* group of proof/extern-boundary.md). Account, storage, and transient      */
/* storage lookup/update operations refine an abstract world-state map      */
/* with explicit cache/update and commit/revert laws: build                 */
/* implementations must refine those materialized list-model semantics.     */
/* ======================================================================== */

/* ---- C-backed transient storage (ffi/transient_storage.c) ----
 * id 0 = EIP-1153 transient storage. Warm access sets are Sail values.
 * Persistent storage is split below into a separate secure-keyed backend. */
val transient_storage_reset : bits(64) -> unit
val transient_storage_write : (bits(64), address, word, word) -> unit
val transient_storage_read : (bits(64), address, word) -> word

/* ---- C-backed persistent storage (ffi/state_db.c): cache/update split ----
 * Cache rows are authenticated pre-state/fixture material; update rows are
 * execution writes. Both are sorted by (keccak(address), keccak(slot)), while
 * update rows remain layered for frame rollback. The hooks are keyed by raw
 * (address, slot); the backend computes and memoizes the secure trie hashes. */
val storage_map_reset : unit -> unit
val storage_map_push : unit -> unit
val storage_map_pop_commit : unit -> unit
val storage_map_pop_discard : unit -> unit
val storage_map_load : (address, word) -> word
val storage_map_store : (address, word, word) -> unit
val storage_map_seed : (address, word, word) -> unit
val storage_map_base_load : (address, word) -> word
val storage_map_present : (address, word) -> bool
val storage_map_base_present : (address, word) -> bool
val storage_map_cache_nonzero : (hash, hash) -> bool
val storage_map_wipe_addr : address -> unit
val storage_map_count : unit -> bits(64)
val storage_map_update_count : unit -> bits(64)
val storage_map_at : bits(64) -> unit
val storage_map_update_at : bits(64) -> unit
val storage_map_it_acct_hash : unit -> hash
val storage_map_it_slot : unit -> word
val storage_map_it_val : unit -> word

/* ---- C-backed account map (ffi/state_db.c): the kernel account working
 * set (k_accounts), split into materialized base/cache rows and execution
 * update rows. Both tables are ordered by keccak256(address). The point-access
 * hooks are keyed by raw address; the backend computes and memoizes the secure
 * trie key, and accessors resolve update first, cache second. ---- */
val acctmap_reset : unit -> unit
val acctmap_present : address -> bool
val acctmap_store : (address, bits(64), word, hash, hash) -> unit
val acctmap_mark_base_exists : address -> unit
val acctmap_seed : (address, bits(64), word, hash, hash) -> unit
val acctmap_nonce : address -> bits(64)
val acctmap_bal : address -> word
val acctmap_sroot : address -> hash
val acctmap_chash : address -> hash
val acctmap_remove : address -> unit
val acctmap_count : unit -> bits(64)
val acctmap_update_count : unit -> bits(64)
val acctmap_at : bits(64) -> unit
val acctmap_update_at : bits(64) -> unit
val acctmap_at_hkey : unit -> hash
val acctmap_at_nonce : unit -> bits(64)
val acctmap_at_bal : unit -> word
val acctmap_at_sroot : unit -> hash
val acctmap_at_chash : unit -> hash
val acctmap_at_base_exists : unit -> bool
