/* Optimized C overrides for `sail/kernel/storage.sail`. */

/*
 * Complete optimized state accesses. The standard account/storage modules
 * remain the readable specification of overlay ordering, BAL effects,
 * authenticated witness fallback, caching, and transaction-original values.
 * Optimized C performs each complete operation against the same host-owned
 * tables in one crossing.
 */

$[c_throws]
val optimized_k_sload = impure { c: "k_sload" } : (hash, address, word) -> StorageValue
val optimized_k_sstore = impure { c: "k_sstore" } : (address, word, word, word) -> unit

function k_sload(a : address, s : word) -> StorageValue =
    optimized_k_sload(k_parent_state_root, a, s)

function k_sstore(a : address, s : word, value : StorageValue) -> unit =
    optimized_k_sstore(a, s, value.curr, value.orig)
