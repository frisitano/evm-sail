/* Build backend: C FFI bindings for the witness trie-node store
 * (ffi/trie_node_db.c). */

val nodedb_reset_ffi = impure { _: "nodedb_reset" } : unit -> unit
function nodedb_reset() -> unit = nodedb_reset_ffi()

val nodedb_insert_ffi = impure { _: "nodedb_insert" } : (hash, bits(64), bits(64)) -> unit
function nodedb_insert(h : hash, off : bits(64), len : bits(64)) -> unit = nodedb_insert_ffi(h, off, len)

val nodedb_off_ffi = impure { _: "nodedb_off" } : hash -> bits(64)
function nodedb_off(h : hash) -> bits(64) = nodedb_off_ffi(h)

val nodedb_len_ffi = impure { _: "nodedb_len" } : hash -> bits(64)
function nodedb_len(h : hash) -> bits(64) = nodedb_len_ffi(h)
