/*md
# Ancestor hashes

The distance-indexed ancestor block-hash table (the `BLOCKHASH` source),
populated while indexing the witness header chain.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Records an ancestor block hash while the witness header chain is
    indexed; index `d` holds the ancestor at distance `d + 1`. */
val ancestor_hash_write = impure { c: "ancestor_hash_write" } : (ancestor_index, hash) -> unit
/*! The recorded ancestor hash at a distance index (the `BLOCKHASH`
    source). */
val ancestor_hash_read = impure { c: "ancestor_hash_read" } : ancestor_index -> hash
