
/*md
# Witness node database

The `keccak256(node) → span` index into the SSZ witness buffer, backing
[node_db_lookup][].

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Empties the node index (start of witness indexing). */
val nodedb_reset = impure { c: "nodedb_reset" } : unit -> unit
/*! Indexes a witness trie node: records its `(offset, length)` span in
    the SSZ buffer under its KECCAK-256 hash. */
val nodedb_insert = impure { c: "nodedb_insert" } : (hash, source_pointer, byte_length) -> unit
/*! The buffer offset recorded for a node hash. */
val nodedb_off = impure { c: "nodedb_off" } : hash -> source_pointer
/*! The span length recorded for a node hash; zero when the node is
    unwitnessed. */
val nodedb_len = impure { c: "nodedb_len" } : hash -> byte_length
