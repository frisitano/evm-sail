/*md
# Ancestor hashes

The distance-indexed ancestor block-hash table (the `BLOCKHASH` source),
populated while indexing the witness header chain.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

val ancestor_hash_write = impure { c: "ancestor_hash_write" } : (bits(8), hash) -> unit
val ancestor_hash_read = impure { c: "ancestor_hash_read" } : bits(8) -> hash
