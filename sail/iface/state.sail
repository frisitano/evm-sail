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
val transient_storage_key : (bits(64), address, word) -> unit
val transient_storage_write : word -> unit
val transient_storage_read : unit -> word

/* ---- C-backed persistent storage (ffi/state_db.c): cache/update split ----
 * Cache rows are authenticated pre-state/fixture material; update rows are
 * execution writes. Both are sorted by (keccak(address), keccak(slot)), while
 * update rows remain layered for frame rollback. */
val storage_map_reset : unit -> unit
val storage_map_push : unit -> unit
val storage_map_pop_commit : unit -> unit
val storage_map_pop_discard : unit -> unit
val storage_map_key : (
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
val storage_map_seed : (bits(64), bits(64), bits(64), bits(64)) -> unit
val storage_map_store : (bits(64), bits(64), bits(64), bits(64)) -> unit
val storage_map_word : bits(64) -> bits(64)
val storage_map_base_word : bits(64) -> bits(64)
val storage_map_present : unit -> bool
val storage_map_base_present : unit -> bool
val storage_map_wipe_acct_hash : (bits(64), bits(64), bits(64), bits(64)) -> unit
val storage_map_count : unit -> bits(64)
val storage_map_update_count : unit -> bits(64)
val storage_map_at : bits(64) -> unit
val storage_map_update_at : bits(64) -> unit
val storage_map_it_acct_hash : bits(64) -> bits(64)
val storage_map_it_slot : bits(64) -> bits(64)
val storage_map_it_val : bits(64) -> bits(64)

/* ---- C-backed account map (ffi/state_db.c): the kernel account working
 * set (k_accounts), split into materialized base/cache rows and execution
 * update rows. Both tables are ordered by keccak256(address). Point access
 * selects the secure trie key once, then accessors resolve update first, cache
 * second. ---- */
val acctmap_reset : unit -> unit
val acctmap_key : (bits(64), bits(64), bits(64), bits(64)) -> unit
val acctmap_present : unit -> bool
val acctmap_store : (
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
val acctmap_mark_base_exists : unit -> unit
val acctmap_seed : (
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
val acctmap_nonce : unit -> bits(64)
val acctmap_bal : bits(64) -> bits(64)
val acctmap_sroot : bits(64) -> bits(64)
val acctmap_chash : bits(64) -> bits(64)
val acctmap_remove : (bits(64), bits(64), bits(64), bits(64)) -> unit
val acctmap_count : unit -> bits(64)
val acctmap_update_count : unit -> bits(64)
val acctmap_at : bits(64) -> unit
val acctmap_update_at : bits(64) -> unit
val acctmap_at_hkey : bits(64) -> bits(64)
val acctmap_at_nonce : unit -> bits(64)
val acctmap_at_bal : bits(64) -> bits(64)
val acctmap_at_sroot : bits(64) -> bits(64)
val acctmap_at_chash : bits(64) -> bits(64)
val acctmap_at_base_exists : unit -> bool
