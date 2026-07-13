val ancestor_hash_write = impure { c: "ancestor_hash_write" } : (bits(8), hash) -> unit
val ancestor_hash_read = impure { c: "ancestor_hash_read" } : bits(8) -> hash
