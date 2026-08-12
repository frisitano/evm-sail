/*md
# Gas quantities

Semantic domains for available gas, transaction and block gas, schedule
constants, computed costs, and the signed refund counter. The canonical model
keeps EVM gas arithmetic mathematical; optimized builds may choose bounded
representations after preserving the validation and out-of-gas behavior.

## Types

`gas` is the live EVM quantity, `transaction_gas` is supplied by a transaction,
`block_gas_limit` bounds block execution, and `block_gas` records gas consumed
by the block. `gas_constant` contains fixed schedule values and `gas_cost`
marks transient, non-negative charges after their affordability or structural
bound has been established. It remains an exact natural in the canonical
model. `gas_refund` records signed changes before the transaction refund cap.

## Constants

The zero values initialize each gas domain. Protocol limits are checked at the
transaction or block boundary rather than encoded as a shared implementation
ceiling.
*/

/*! The maximum block gas limit admitted by the execution-payload SSZ
    `uint64` field. Provenance: consensus `ExecutionPayload.gas_limit` and
    Amsterdam `SszExecutionPayload.gas_limit`. */
type block_gas_limit_bound : Int = ssz_uint_bound

/*! EIP-7928 charges one BAL item against each 2,000 units of block gas. */
type block_access_list_item_gas : Int = 2000
let BLOCK_ACCESS_LIST_ITEM_GAS : int(block_access_list_item_gas) = sizeof(block_access_list_item_gas)

/*! Available gas in a running EVM frame. Every admitted transaction gas
    limit originates in the execution payload's SSZ `uint64` gas-limit
    domain, and child frames can only receive gas from their parent. */
type gas = range(0, 2 ^ 64 - 1)

/*! Representation invariant for a value copied from the live frame gas
    counter. */
type live_gas_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 ^ 64 - 1

/*! Gas that can be restored to a particular live counter without exceeding
    the admitted `uint64` gas domain. The index is type-level only; optimized
    C represents the value as the same native gas scalar. */
type gas_credit('available : Int) = {
    'credit,
       0 <= 'credit
    &  'available + 'credit <= 2 ^ 64 - 1.
    int('credit)
}

/*! State-gas spill that can be restored to the indicated execution gas. */
type state_gas_spill_credit('available : Int) = {
    'credit,
       0 <= 'credit
    &  'credit <= 2 ^ 24
    &  'available + 'credit <= 2 ^ 64 - 1.
    int('credit)
}

/*! Amsterdam's per-frame state-gas reservoir. The transaction's total gas
    allowance remains in the execution payload's `uint64` domain; only the
    regular-gas portion and state-gas spill into that portion are capped by
    EIP-7825. */
type state_gas = range(0, 2 ^ 64 - 1)

/*! Child state gas that can be restored to the indicated parent reservoir. */
type state_gas_credit('available : Int) = {
    'credit,
       0 <= 'credit
    &  'available + 'credit <= 2 ^ 64 - 1.
    int('credit)
}

/*! Gas supplied by a transaction before fork-specific validation. Any value
    above the execution payload's SSZ `uint64` block-gas-limit domain cannot be
    admitted, so the RLP boundary rejects it before constructing a
    transaction. */
type transaction_gas = range(0, block_gas_limit_bound)

/*! A transient computed charge after its affordability or structural bound
    has been established. Unaffordable larger computations are represented by
    `GasCharge.affordable = false` rather than materialized as a cost. */
type gas_cost = range(0, 2 ^ 64 - 1)

let GAS_COST_ZERO : gas_cost = 0

/*! Exact exclusive byte endpoint used by the canonical memory-expansion
    equation. The optimized C splice may replace endpoints beyond its
    materializable arena with one proven-unaffordable sentinel. */
type memory_required_endpoint = nat

/*! Exact incremental memory charge. Optimized C retains one additional
    sentinel value above every representable live-gas value so `charge`
    remains the sole out-of-gas decision. */
type memory_expansion_charge = nat

/*! One affordability decision together with its bounded cost. The payload is
    meaningful only when `affordable` is true. */
struct GasCharge = {
    affordable : bool,
    cost : gas_cost,
}

let GAS_CHARGE_UNAFFORDABLE : GasCharge = struct { affordable = false, cost = GAS_COST_ZERO }

function gas_charge(cost : gas_cost) -> GasCharge =
    struct { affordable = true, cost = cost }

/*! Intermediate MODEXP affordability factors, bounded by live gas and the
    at-most-255-bit exponent-head contribution. */
type modexp_factor = range(0, 3 * (2 ^ 64 - 1) + 257)

/*! Products of two bounded MODEXP affordability factors. */
type modexp_product = range(
    0,
    (3 * (2 ^ 64 - 1) + 257) * (3 * (2 ^ 64 - 1) + 257),
)

/*! Long-exponent byte counts after the fork-specific affordability check. */
type modexp_osaka_extra = range(0, div(2 ^ 64 - 1, 16))
/*! Long-exponent byte counts used by the pre-Osaka MODEXP gas formula. */
type modexp_pre_osaka_extra = range(0, div(3 * (2 ^ 64 - 1) + 2, 8))

/*! EIP-2537's base-cost/discount product. */
type bls_msm_coefficient = range(0, 45000 * (2 ^ 16 - 1))

/*! Exact EIP-2537 MSM products before their affordability check. */
type bls_msm_product = range(
    0,
    45000 * (2 ^ 16 - 1) * (2 ^ 64 - 1),
)

/*! EIP-2537's bounded remainder product before division by 1000. */
type bls_msm_tail_product = range(0, 45000 * (2 ^ 16 - 1) * 999)

/*! Linear precompile multiplier before adding the fixed base charge. */
type linear_gas_variable_product = range(0, 45000 * (2 ^ 64 - 1))

/*! Exact linear precompile cost before its live-gas affordability check. */
type linear_gas_product = range(0, 45000 * 2 ^ 64)

/*! The EIP-7825 per-transaction execution-gas limit reused by EIP-8037's
    regular-gas pool. */
type transaction_execution_gas_limit_value : Int = eip7825_transaction_gas_limit

/*! Gas admitted to Amsterdam's regular execution pool after applying the
    EIP-7825 per-transaction cap. */
type transaction_execution_gas = range(0, transaction_execution_gas_limit_value)

let TRANSACTION_EXECUTION_GAS_LIMIT : int(transaction_execution_gas_limit_value) = sizeof(
    transaction_execution_gas_limit_value
)

/*! Execution gas temporarily consumed by Amsterdam state charges. EIP-8037
    draws spill only from the regular-gas pool, which is capped by EIP-7825. */
type state_gas_spill = range(0, transaction_execution_gas_limit_value)

let STATE_GAS_SPILL_ZERO : int(0) = 0
let STATE_GAS_SPILL_LIMIT : state_gas_spill = sizeof(transaction_execution_gas_limit_value)

/*! A block header's gas limit. Its SSZ execution-payload field is `uint64`;
    `GASLIMIT` widens this bounded natural into an EVM word when it pushes the
    value onto the stack (Yellow Paper equation 147). */
type block_gas_limit = range(0, block_gas_limit_bound)

/*! Gas consumed by a block header. Provenance: the execution-payload SSZ
    schema declares `gas_used: uint64`. The separate payload-validity rule
    `gas_used <= gas_limit` is checked when the header is admitted. */
type block_gas = range(0, block_gas_limit_bound)

/*! Gas accumulated in execution-order receipts. Before Amsterdam this is
    bounded by the block gas limit. Amsterdam receipts account for regular and
    state gas together, so the execution profile supplies the tighter
    header-dependent bound. */
type receipt_cumulative_gas = range(0, 2 * block_gas_limit_bound)
/*! Whether a value lies in the receipt cumulative-gas domain. */
type receipt_cumulative_gas_value_valid('value : Int) -> Bool =
    0 <= 'value & 'value <= 2 * block_gas_limit_bound

let GAS_ZERO : int(0) = 0
let STATE_GAS_ZERO : int(0) = 0

/*! The execution-gas allowance of each protocol system call (EIP-4788,
    EIP-2935, EIP-7002, EIP-7251, and EIP-8282). */
let SYSTEM_CALL_GAS_LIMIT : int(30000000) = 30000000

/* Schedule constants are bounded independently from live gas. Arithmetic with
 * transaction-controlled multipliers produces exact natural intermediates. */
/*! A fixed gas-schedule value used as an opcode or transaction base cost. */
type gas_constant = range(0, 45000)

let GAS_CONSTANT_ZERO : gas_constant = 0

/* A frame's refund is a signed intermediate quantity. Dirty SSTORE
 * transitions can remove credit established by an earlier write, so this is
 * deliberately distinct from the non-negative gas type. */
/*! Absolute lifecycle bound for the signed refund accumulator. */
type gas_refund_bound : Int = 199 * (2 ^ 64 - 1)

/*! The signed transaction refund accumulator before capping. */
type gas_refund = range(
    -gas_refund_bound,
    gas_refund_bound,
)

let GAS_REFUND_ZERO : gas_refund = 0

/*! EIP-8037's `SSTORE` storage-clear refund. */
type amsterdam_storage_clear_refund : Int = 12480

/*! One EIP-8037 `SSTORE` transition's signed refund delta. A transition either
    establishes the storage-clear refund or withdraws the one an earlier write
    in the same transaction established, so it moves the accumulator by at most
    that refund in either direction. The accumulator itself is [gas_refund][],
    which this sits well inside. The pre-Amsterdam schedule prices its
    transitions from its own constants and is bounded separately. */
type gas_refund_delta = range(
    -amsterdam_storage_clear_refund,
    amsterdam_storage_clear_refund,
)

/*! Net state gas consumed by one execution frame. A credit can make this
    negative until transaction settlement clamps the block-level value at
    zero. The bounds follow from subtracting two live `uint64` counters and
    adding at most one EIP-7825 regular-pool spill. */
type frame_state_gas_delta = range(
    -(2 ^ 64 - 1),
    (2 ^ 64 - 1) + transaction_execution_gas_limit_value,
)

let FRAME_STATE_GAS_DELTA_ZERO : frame_state_gas_delta = 0

/*! Combined state-gas delta for the Amsterdam authorization and execution
    frames. */
type state_gas_delta = range(
    -(2 * (2 ^ 64 - 1)),
    2 * ((2 ^ 64 - 1) + transaction_execution_gas_limit_value),
)

let STATE_GAS_DELTA_ZERO : state_gas_delta = 0

/*! State gas after adding the intrinsic transaction charge and before the
    block-level non-negative/u64 checks. */
type transaction_state_gas_delta = range(
    -(2 * (2 ^ 64 - 1)),
    3 * (2 ^ 64 - 1) + 2 * transaction_execution_gas_limit_value,
)

/*! A non-negative transaction state-gas total after settlement has rejected
    a negative signed delta. */
type transaction_state_gas_used = range(
    0,
    3 * (2 ^ 64 - 1) + 2 * transaction_execution_gas_limit_value,
)

/* The transaction refund cap is `gas_used / 2` before London and
 * `gas_used / 5` from London onward.  Keep the exact protocol alternatives
 * in the type instead of a generic implementation-sized divisor range. */
/*! The pre-London refund divisor: refunds are capped at `gas_used / 2`. */
type pre_london_refund_divisor : Int = 2
/*! The London refund divisor: EIP-3529 caps refunds at `gas_used / 5`. */
type post_london_refund_divisor : Int = 5
/*! Whether a value is one of the two protocol refund divisors. */
type transaction_refund_divisor_value('value : Int) -> Bool =
       'value == pre_london_refund_divisor
    |  'value == post_london_refund_divisor
/*! The fork-selected transaction-refund divisor: 2 before London, 5 after. */
type transaction_refund_divisor = {
    'value,
    transaction_refund_divisor_value('value).
    int('value)
}
