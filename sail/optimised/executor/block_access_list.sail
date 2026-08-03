/* Optimized EIP-7928 validation.
 *
 * The specification keeps the canonical recursive RLP walk in Sail.  The
 * optimized guest crosses into C once for the complete operation so encoded
 * fields can be compared directly with the C-private BAL recorder without
 * constructing generated iterator unions for every row.
 */

$[c_throws]
val optimized_initialize_block_access_list_state =
    impure { c: "initialize_block_access_list_state" } :
    (StatelessInputSliceAtMost(block_access_list_length_bound), hash) -> unit

function initialize_block_access_list_state(
    bytes : StatelessInputSliceAtMost(block_access_list_length_bound),
    parent_state_root : hash,
) -> unit =
    optimized_initialize_block_access_list_state(bytes, parent_state_root)

$[c_throws]
val optimized_validate_block_access_list =
    impure { c: "validate_block_access_list" } :
    (StatelessInputSliceAtMost(block_access_list_length_bound), block_gas_limit) -> unit

function validate_block_access_list(bytes, block_gas_limit) =
    optimized_validate_block_access_list(bytes, block_gas_limit)
