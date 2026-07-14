/*md
# Witness node database

The `keccak256(node) → span` index into the SSZ witness buffer, backing
[node_db_lookup][].

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

val nodedb_reset = impure { c: "nodedb_reset" } : unit -> unit
val nodedb_insert = impure { c: "nodedb_insert" } : (hash, bits(64), bits(64)) -> unit
val nodedb_off = impure { c: "nodedb_off" } : hash -> bits(64)
val nodedb_len = impure { c: "nodedb_len" } : hash -> bits(64)
