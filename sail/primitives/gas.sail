/*md
# Gas quantities

Semantic domains for available gas, schedule constants, computed costs, and
the signed refund counter. The separation keeps unaffordable intermediate
costs exact while preventing them from being mistaken for gas that a valid
frame can hold.

## Types

`gas` is bounded by the protocol's signed-64-bit validity limit;
`gas_constant` contains fixed schedule values; `gas_cost` is an exact natural;
and `gas_refund` records signed changes before the transaction refund cap.

## Constants

The zero values initialize each gas domain, while `GAS_MAX` is the largest gas
quantity admitted by transaction and block validation.
*/

/*! Available frame, transaction, or block gas below the protocol limit. */
/* Valid block, transaction, frame, and cumulative gas values are strictly
 * below 2^63. Wire formats remain uint64; callers validate that boundary
 * before constructing this semantic quantity. */

newtype gas = Gas : range(0, 2 ^ 63 - 1)

let GAS_MAX_VALUE : nat = 2 ^ 63 - 1
let GAS_ZERO : gas = Gas(0)
let GAS_MAX : gas = Gas(2 ^ 63 - 1)

/* Schedule constants are bounded independently from live gas. Arithmetic on
 * them promotes to gas_cost, so a transaction-controlled multiplier never
 * narrows an exact intermediate result. */
/*! A fixed gas-schedule value used as an opcode or transaction base cost. */
newtype gas_constant = GasConstant : range(0, 45000)

let GAS_CONSTANT_ZERO : gas_constant = GasConstant(0)

/* Computed costs are mathematical naturals. They remain exact even when an
 * attacker-controlled operand makes the result larger than any frame can pay. */
/*! An exact non-negative cost that may exceed all available gas. */
newtype gas_cost = GasCost : nat

let GAS_COST_ZERO : gas_cost = GasCost(0)

/* A frame's refund is a signed intermediate quantity. Dirty SSTORE
 * transitions can remove credit established by an earlier write, so this is
 * deliberately distinct from the non-negative gas type. */
/*! The signed transaction refund accumulator before capping. */
newtype gas_refund = GasRefund : int

let GAS_REFUND_ZERO : gas_refund = GasRefund(0)

/* Every protocol gas quotient uses a small positive schedule divisor. */
/*! A small positive divisor used by the gas schedule. */
type gas_divisor = range(1, 1000)

/*! Constructs bounded gas from a natural number. */
function nat_to_gas(value : nat) -> gas = {
    if value <= 2 ^ 63 - 1 then {
        Gas(value)
    } else {
        assert(false);
        GAS_ZERO
    }
}

/*! Narrows a word to gas when it satisfies the protocol bound. */
function word_to_gas(value : word) -> option(gas) =
    if value[255 .. 63] == sail_zeros(193) then {
        Some(Gas(unsigned(value[62 .. 0])))
    } else {
        None()
    }

function word_of_gas(Gas(value) : gas) -> word = word_of_nat(value)

function gas_to_cost(Gas(amount) : gas) -> gas_cost = GasCost(amount)

function gas_to_refund(Gas(amount) : gas) -> gas_refund = GasRefund(amount)

function gas_constant_to_refund(GasConstant(amount) : gas_constant) -> gas_refund =
    GasRefund(amount)

function gas_cost_to_word(GasCost(amount) : gas_cost) -> word =
    word_of_quantity(amount)

function gas_cost_to_refund(GasCost(amount) : gas_cost) -> gas_refund =
    GasRefund(amount)

function gas_refund_equal(GasRefund(left) : gas_refund, GasRefund(right) : gas_refund) -> bool =
    left == right

function gas_refund_add(GasRefund(left) : gas_refund, GasRefund(right) : gas_refund) -> gas_refund =
    GasRefund(left + right)

function gas_refund_sub(GasRefund(left) : gas_refund, GasRefund(right) : gas_refund) -> gas_refund =
    GasRefund(left - right)

/*! Clamps a signed refund to the interval from zero through the supplied cap. */
function capped_gas_refund(GasRefund(refund) : gas_refund, Gas(limit) : gas) -> gas =
    if refund <= 0 then {
        GAS_ZERO
    } else {
        if refund <= limit then {
            Gas(refund)
        } else {
            Gas(limit)
        }
    }

function gas_equal(Gas(left) : gas, Gas(right) : gas) -> bool = left == right

function gas_lt(Gas(left) : gas, Gas(right) : gas) -> bool = left < right

function gas_le(Gas(left) : gas, Gas(right) : gas) -> bool = left <= right

function gas_constant_equal(GasConstant(left) : gas_constant, GasConstant(right) : gas_constant) -> bool =
    left == right

function gas_cost_equal(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> bool =
    left == right

function gas_cost_lt(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> bool =
    left < right

function gas_cost_le(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> bool =
    left <= right

function gas_cost_le_gas(GasCost(cost) : gas_cost, Gas(limit) : gas) -> bool =
    cost <= limit

function gas_lt_cost(Gas(limit) : gas, GasCost(cost) : gas_cost) -> bool =
    limit < cost

/*! Subtracts bounded gas after the caller proves the right operand does
    not exceed the left; the assertion preserves the [gas][] range. */
function gas_sub(Gas(left) : gas, Gas(right) : gas) -> gas = {
    assert(right <= left);
    Gas(left - right)
}

/*! Divides available gas by a positive schedule divisor. */
function gas_quotient(Gas(value) : gas, divisor : gas_divisor) -> gas = {
    let quotient = exact_quotient(value, divisor);
    assert(quotient <= value);
    Gas(quotient)
}

function gas_cost_add(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> gas_cost =
    GasCost(left + right)

function gas_cost_add_constant(GasCost(left) : gas_cost, GasConstant(right) : gas_constant) -> gas_cost =
    GasCost(left + right)

function gas_constant_add(GasConstant(left) : gas_constant, GasConstant(right) : gas_constant) -> gas_cost =
    GasCost(left + right)

/*! Subtracts one exact gas cost from another without underflow. */
function gas_cost_sub(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> gas_cost = {
    assert(right <= left);
    GasCost(left - right)
}

/*! Subtracts schedule constants and promotes the result to an exact cost. */
function gas_constant_sub(GasConstant(left) : gas_constant, GasConstant(right) : gas_constant) -> gas_cost = {
    assert(right <= left);
    GasCost(left - right)
}

function gas_cost_scale(GasCost(value) : gas_cost, factor : nat) -> gas_cost =
    GasCost(value * factor)

function gas_constant_scale(GasConstant(value) : gas_constant, factor : nat) -> gas_cost =
    GasCost(value * factor)

/*! Scales a schedule constant by an exact byte quantity. */
function gas_constant_scale_byte_quantity(
    GasConstant(value) : gas_constant,
    ByteQuantity(factor) : byte_quantity,
) -> (
    gas_cost
) =
    GasCost(value * factor)

function gas_cost_quotient(GasCost(value) : gas_cost, divisor : gas_divisor) -> gas_cost =
    GasCost(exact_quotient(value, divisor))

/*! Multiplies a word by a protocol quantity and reports 256-bit overflow. */
function word_checked_mul_protocol_quantity(value : word, factor : protocol_quantity) -> option(word) = {
    var result : word = ZERO_WORD;
    var addend : word = value;
    var remaining : protocol_quantity = factor;
    var valid : bool = true;
    foreach (i from 0 to 63) {
        if valid & (tmod_int(remaining, 2) == 1) then {
            let added = word_checked_add(result, addend);
            match added {
                Some(value) => result = value,
                None() => valid = false,
            }
        };
        remaining = protocol_quantity_quotient(remaining, 2);
        if valid & (i < 63) & (remaining != 0) then {
            let doubled = word_checked_add(addend, addend);
            match doubled {
                Some(value) => addend = value,
                None() => valid = false,
            }
        }
    };
    if valid then {
        Some(result)
    } else {
        None()
    }
}

/*! Multiplies a word by bounded gas and reports 256-bit overflow. */
function word_checked_mul_gas(value : word, Gas(factor) : gas) -> option(word) =
    word_checked_mul_protocol_quantity(value, factor)

overload word_checked_mul = {word_checked_mul_protocol_quantity, word_checked_mul_gas}

overload operator == = {gas_equal, gas_constant_equal, gas_cost_equal}
overload operator < = {gas_lt, gas_cost_lt, gas_lt_cost}
overload operator <= = {gas_le, gas_cost_le, gas_cost_le_gas}
overload operator + = {
    gas_cost_add,
    gas_cost_add_constant,
    gas_constant_add
}
overload operator - = {gas_sub, gas_cost_sub, gas_constant_sub}
overload operator * = {
    gas_cost_scale,
    gas_constant_scale,
    gas_constant_scale_byte_quantity
}
overload operator / = {gas_quotient, gas_cost_quotient}

overload operator == = {gas_refund_equal}
overload operator + = {gas_refund_add}
overload operator - = {gas_refund_sub}
