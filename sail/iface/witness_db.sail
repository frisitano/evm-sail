/* ======================================================================== */
/* HOST INTERFACE CONTRACT: WITNESS NODE DATABASE                           */
/*                                                                          */
/* Abstract `val` declarations for the C-backed witness node store (the     */
/* "Witness and MPT DB" contract group of proof/extern-boundary.md). The    */
/* node table refines authenticated MPT witness spans over SSZ input bytes; */
/* insert/select operations preserve the same abstract rows: build          */
/* implementations must refine those materialized list-model semantics.     */
/* ======================================================================== */

/* C-backed node-db (ffi/trie_node_db.c): keccak(node) -> (offset, len) span into
 * the SSZ witness buffer. Replaces a Sail assoc list whose linear node_db_lookup
 * (plus the whole list deep-copied through every recursive walk) made trie
 * traversal O(N^2); this is an O(1)-amortized hash map, so a walk is O(depth).
 * Because the key is keccak(node), every walk from an authenticated root is
 * itself a hash-chain proof -- this table plus the anchored parent-root
 * register (k_parent_state_root) is the whole lazy-witness model. The 256-bit
 * key crosses as a whole hash value; nodedb_sel caches the matched span for
 * the off/len accessors. */
val nodedb_reset : unit -> unit
val nodedb_insert : (hash, bits(64), bits(64)) -> unit
val nodedb_sel : hash -> bool
val nodedb_sel_off : unit -> bits(64)
val nodedb_sel_len : unit -> bits(64)
