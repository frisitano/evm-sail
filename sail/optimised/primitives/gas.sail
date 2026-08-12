/* Optimized C overrides for `sail/primitives/gas.sail`. */

/* Oversized memory endpoints and charges are saturated only in the optimized
 * representation. The remaining gas and arithmetic lifecycle bounds now live
 * in the canonical specification. */
type memory_required_endpoint = range(0, memory_region_bound + 1)
type memory_expansion_charge = range(0, 2 ^ 64)
