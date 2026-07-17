val ancestor_hash_write = impure { c: "ancestor_hash_write" } : (ancestor_index, hash) -> unit
val ancestor_hash_read = impure { c: "ancestor_hash_read" } : ancestor_index -> hash
