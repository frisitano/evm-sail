/* Optimized C overrides for `sail/primitives/gas.sail`. */

/* Valid execution gas is bounded by the greatest value reachable in the live
 * frame counter. Costs outside the same domain are rejected as unaffordable
 * by the optimized cost planners before they are materialized. */
type gas = range(0, 2 ^ 64 - 1)
type gas_cost = range(0, 2 ^ 64 - 1)
type modexp_factor = range(0, 3 * (2 ^ 64 - 1) + 257)
type modexp_product = range(
    0,
    (3 * (2 ^ 64 - 1) + 257) * (3 * (2 ^ 64 - 1) + 257),
)
type modexp_osaka_extra = range(0, div(2 ^ 64 - 1, 16))
type modexp_pre_osaka_extra = range(0, div(3 * (2 ^ 64 - 1) + 2, 8))
type bls_msm_coefficient = range(0, 45000 * (2 ^ 16 - 1))
type bls_msm_product = range(
    0,
    45000 * (2 ^ 16 - 1) * (2 ^ 64 - 1),
)
type bls_msm_tail_product = range(0, 45000 * (2 ^ 16 - 1) * 999)
type linear_gas_variable_product = range(0, 45000 * (2 ^ 64 - 1))
type linear_gas_product = range(0, 45000 * 2 ^ 64)
type live_gas_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 ^ 64 - 1

/* Exact signed state-gas bounds follow the value's lifecycle.  A single
 * frame subtracts two u64 counters and adds at most the EIP-7825 regular-pool
 * spill.  Amsterdam combines at most the authorization and execution frames,
 * then settlement adds one u64 intrinsic charge.  These are representation
 * invariants derived from the source domains, not protocol gas caps. */
type frame_state_gas_delta = range(
    -(2 ^ 64 - 1),
    (2 ^ 64 - 1) + transaction_execution_gas_limit_value,
)
type state_gas_delta = range(
    -(2 * (2 ^ 64 - 1)),
    2 * ((2 ^ 64 - 1) + transaction_execution_gas_limit_value),
)
type transaction_state_gas_delta = range(
    -(2 * (2 ^ 64 - 1)),
    3 * (2 ^ 64 - 1) + 2 * transaction_execution_gas_limit_value,
)
type transaction_state_gas_used = range(
    0,
    3 * (2 ^ 64 - 1) + 2 * transaction_execution_gas_limit_value,
)
