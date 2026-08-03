/* Optimized C overrides for `sail/lib/address.sail`. */

/*
 * Complete preimage refinements. The standard model retains the explicit
 * scratch/RLP equations; optimized C computes the same exact widths and
 * writes each complete preimage once into borrowed shared scratch.
 */
val optimized_create2_address = impure { c: "create2_address" } : (address, word, hash) -> address

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    optimized_create2_address(sender, salt, init_hash)
