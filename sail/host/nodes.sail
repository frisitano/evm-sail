/* keccak(node) -> span in the SSZ witness buffer. */

val nodedb_reset = impure { c: "nodedb_reset" } : unit -> unit
val nodedb_insert = impure { c: "nodedb_insert" } : (hash, source_pointer, byte_length) -> unit
val nodedb_off = impure { c: "nodedb_off" } : hash -> source_pointer
val nodedb_len = impure { c: "nodedb_len" } : hash -> byte_length
