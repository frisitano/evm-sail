/* Optimized C overrides for `sail/lib/address.sail`. */

/*
 * Complete preimage refinements. The standard model retains the explicit
 * scratch/RLP equations; optimized C computes the same exact widths and
 * writes each complete preimage once into borrowed shared scratch.
 */
val create2_address_from_hash = impure { c: "create2_address" } : (address, word, hash) -> address

function create2_address(sender : address, salt : word, init_hash : hash) -> address =
    create2_address_from_hash(sender, salt, init_hash)
