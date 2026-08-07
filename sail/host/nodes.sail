
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
val nodedb_insert = impure { c: "nodedb_insert" } : (hash, stateless_input_pointer, stateless_input_length) -> unit
/*! The complete stateless-input span recorded for a node hash; the empty
    slice when the node is unwitnessed. Returning the span as one value keeps
    its source and host-range constraint intact. */
val nodedb_lookup = impure { c: "nodedb_lookup" } : hash -> StatelessInputSlice
