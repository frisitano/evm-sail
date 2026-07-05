/* ======================================================================== */
/* HOST INTERFACE CONTRACT: WITNESS AND MPT DATABASES                       */
/*                                                                          */
/* Abstract `val` declarations for the C-backed witness node, account-leaf, */
/* and storage-slot stores (the "Witness and MPT DB" contract group of      */
/* proof/extern-boundary.md). Witness node/account/storage tables refine    */
/* authenticated MPT witness spans over SSZ input bytes; insert/select and  */
/* iteration operations preserve the same abstract rows: build              */
/* implementations must refine those materialized list-model semantics.     */
/* ======================================================================== */

/* C-backed witness storage-slot store (ffi/trie_node_db.c):
 * (keccak(addr), keccak(slot)) -> 256-bit value. The harvested, authenticated
 * pre-state storage. Point lookup (slotdb_sel + slotdb_selval) serves stateless
 * reads; iteration (slotdb_count + slotdb_at + slotdb_at_*) feeds the post-state
 * root. Keys and values cross as whole 256-bit hash/word values. */
val slotdb_reset : unit -> unit
val slotdb_insert : (hash, hash, word) -> unit
val slotdb_sel : (hash, hash) -> bool
val slotdb_selval : unit -> word
val slotdb_sel_existed : unit -> bool
val slotdb_count : unit -> bits(64)
val slotdb_at : bits(64) -> unit
val slotdb_at_acct : unit -> hash
val slotdb_at_slot : unit -> hash
val slotdb_at_val : unit -> word
val slotdb_at_existed : unit -> bool
val storage_mark_incomplete : hash -> unit
val storage_harvest_complete_raw : hash -> bool

/* node db: keccak(node) -> node bytes (linear scan; witnesses are small) */
/* C-backed node-db (ffi/trie_node_db.c): keccak(node) -> (offset, len) span into
 * the SSZ witness buffer. Replaces a Sail assoc list whose linear node_db_lookup
 * (plus the whole list deep-copied through every recursive walk) made re-rooting
 * O(N^2); this is an O(1)-amortized hash map, so the walk is O(N). The 256-bit
 * key crosses as a whole hash value; nodedb_sel caches
 * the matched span for the off/len accessors. */
val nodedb_reset : unit -> unit
val nodedb_insert : (hash, bits(64), bits(64)) -> unit
val nodedb_sel : hash -> bool
val nodedb_sel_off : unit -> bits(64)
val nodedb_sel_len : unit -> bits(64)

/* C-backed witness account-leaf store (ffi/trie_node_db.c): keccak(addr) ->
 * (offset, len) span of the account's leaf node in the SSZ witness buffer.
 * Recorded during the state re-root walk; a stateless account read looks the span
 * up here (O(1)) and decodes the leaf lazily. Same span model as the node-db. */
val acctdb_reset : unit -> unit
val acctdb_insert : (hash, bits(64), bits(64)) -> unit
val acctdb_sel : hash -> bool
val acctdb_sel_off : unit -> bits(64)
val acctdb_sel_len : unit -> bits(64)
val acctdb_count : unit -> bits(64)
val acctdb_at : bits(64) -> unit
val acctdb_at_key : unit -> hash
val acctdb_at_off : unit -> bits(64)
val acctdb_at_len : unit -> bits(64)
