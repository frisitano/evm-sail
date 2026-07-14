/* keccak(node) -> span in the SSZ witness buffer. */

val nodedb_reset = impure { c: "nodedb_reset" } : unit -> unit
val nodedb_insert = impure { c: "nodedb_insert" } : (hash, bits(64), bits(64)) -> unit
val nodedb_off = impure { c: "nodedb_off" } : hash -> bits(64)
val nodedb_len = impure { c: "nodedb_len" } : hash -> bits(64)
