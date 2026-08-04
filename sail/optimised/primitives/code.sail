/* Optimized C representation override for `sail/primitives/code.sail`. */

/*
 * The optimized code store allocates JUMPDEST analysis eagerly and returns
 * its native byte-table address.  Keep the specification's bounded integer
 * reference for ordinary C and proof extraction, but carry the address
 * directly throughout optimized C without an offset-to-pointer adapter.
 */
$[c_repr byte_pointer]
type jump_table_index = range(0, 2 ^ 64 - 1)
