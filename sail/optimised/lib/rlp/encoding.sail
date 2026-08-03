/* Optimized C overrides for `sail/lib/rlp/encoding.sail`. */

/* The widest protocol natural encoded through this path is Amsterdam's
 * cumulative receipt gas: two independently header-bounded gas dimensions.
 * It needs 65 bits in the maximal SSZ-header case, while still fitting
 * directly in the native U256 conversion used below. */
type rlp_natural_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 * (2 ^ 64 - 1)

type rlp_natural_increment_valid('value : Int) -> Bool =
    0 <= 'value & 'value < 2 ^ 64 - 1

type rlp_natural_size = range(1, 33)
