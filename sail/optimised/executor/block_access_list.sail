/* Optimized EIP-7928 validation.
 *
 * The specification keeps the canonical recursive RLP walk in Sail.  The
 * optimized guest crosses into C once for the complete operation so encoded
 * fields can be compared directly with the C-private BAL recorder without
 * constructing generated iterator unions for every row.
 */

val block_access_list_state_initialize = impure { c: "initialize_block_access_list_state" } : (
        StatelessInputSliceAtMost(block_access_list_length_bound),
        hash,
        transaction_count,
    ) -> unit

val block_access_list_validate_encoded = impure { c: "validate_block_access_list" } : (
        StatelessInputSliceAtMost(block_access_list_length_bound),
        block_gas_limit,
    ) -> unit

function initialize_block_access_list_state(
    bytes : StatelessInputSliceAtMost(block_access_list_length_bound),
    parent_state_root : hash,
    transaction_count : transaction_count,
) -> (
    unit
) =
    block_access_list_state_initialize(bytes, parent_state_root, transaction_count)

function validate_block_access_list(bytes, block_gas_limit) =
    block_access_list_validate_encoded(bytes, block_gas_limit)
