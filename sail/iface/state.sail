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
/* native base resolver backing: read the seeded pre-state cache directly
 * (write-set model keeps the cache as the pure base k/v store; miss = absent) */
val storage_cache_present : (address, word) -> bool
val storage_cache_load : (address, word) -> word
val storage_map_cache_nonzero : (hash, hash) -> bool
val storage_map_wipe_addr : address -> unit

/* per-account enumeration, keyed by keccak(address): row count and
 * (slot, value) getters over the flattened cache/update union and over the
 * write set only -- no selection state */
val storage_map_acct_count : hash -> bits(64)
val storage_map_acct_slot : (hash, bits(64)) -> word
val storage_map_acct_val : (hash, bits(64)) -> word
val storage_map_acct_update_count : hash -> bits(64)
val storage_map_acct_update_slot : (hash, bits(64)) -> word
val storage_map_acct_update_val : (hash, bits(64)) -> word

/* ---- write-set storage overlay (ffi/state_db.c storage_wset_*): the tx overlay +
 * block base that replace the layered storage_map. A working-set miss falls
 * through to the backend base resolver (native seeded map / witness MPT). */
val storage_wset_reset : unit -> unit
val storage_wset_tx_clear : unit -> unit
val storage_wset_merge : unit -> unit
val storage_wset_present : (address, word) -> bool          /* tx-or-block has a write */
val storage_wset_load : (address, word) -> word             /* live value (0 on miss) */
val storage_wset_base_present : (address, word) -> bool      /* block base has a write */
val storage_wset_base_load : (address, word) -> word         /* tx-start value (k_orig) */
val storage_wset_touch : (address, word) -> unit             /* mark touched (BAL reads) */
val storage_wset_prior : (address, word, word) -> word       /* JStor payload: value a revert restores */
val storage_wset_write : (address, word, word, word) -> unit /* (a,s,v,orig): orig adopted on first write */
val storage_wset_restore : (address, word, word) -> unit     /* JStor undo: current := prior */
val storage_wset_warm : (address, word) -> bool              /* set warm; true iff was cold */
val storage_wset_is_warm : (address, word) -> bool
val storage_wset_unwarm : (address, word) -> unit            /* JWarmS undo */
val storage_wset_wipe_addr : address -> unit                 /* EIP-6780 tx-end delete */

/* compute_root enumeration over the block base, per account keccak(addr):
 * witness = dirty rows (net change vs MPT); native = cache-union-block rows */
val storage_wset_block_dirty_count : hash -> bits(64)
val storage_wset_block_dirty_slot : (hash, bits(64)) -> word
val storage_wset_block_dirty_val : (hash, bits(64)) -> word
val storage_wset_union_count : hash -> bits(64)
val storage_wset_union_slot : (hash, bits(64)) -> word
val storage_wset_union_val : (hash, bits(64)) -> word

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

/* enumeration by row index, no cursor: `row` reads the secure-key sorted
 * cache/update union, `update_row` reads the update table alone;
 * out-of-range indices read as zeros */
val acctmap_row_hkey : bits(64) -> hash
val acctmap_row_nonce : bits(64) -> bits(64)
val acctmap_row_bal : bits(64) -> word
val acctmap_row_sroot : bits(64) -> hash
val acctmap_row_chash : bits(64) -> hash
val acctmap_row_base_exists : bits(64) -> bool
val acctmap_update_row_hkey : bits(64) -> hash
val acctmap_update_row_nonce : bits(64) -> bits(64)
val acctmap_update_row_bal : bits(64) -> word
val acctmap_update_row_sroot : bits(64) -> hash
val acctmap_update_row_chash : bits(64) -> hash
val acctmap_update_row_base_exists : bits(64) -> bool
