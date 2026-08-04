/* Optimized C overrides for `sail/lib/state_trie.sail`. */

/*
 * Optimized state-root refinement. The standard model keeps the explicit
 * witness walker and canonical MPT builder; optimized C links an equivalent
 * fixed-stack implementation over the same host iterators and node database.
 * The optimized ABI raises the same Sail InvalidBlock exception on witness or
 * RLP failure as the standard implementation.
 */
$[c_throws]
val compute_state_root = impure { c: "compute_state_root" } : unit -> hash
