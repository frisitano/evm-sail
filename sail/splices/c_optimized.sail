/* Generated-C representation overrides. This is a build input, not a second
 * specification: Sail replaces these definitions after loading the canonical
 * model. Proof backends continue to see the exact mathematical definitions. */

/* Live gas is bounded by the canonical model. The optimized cost domain adds
 * exactly one value: 2^63 means "at least as large as every valid gas limit".
 * Calculators clamp only after their final monotone expression; a saturated
 * value must never be fed through a later subtraction or division. */
newtype gas_cost = GasCost : range(0, 2 ^ 63)

$[c_repr int64]
newtype gas_refund = GasRefund : int

function protocol_quantity_quotient(value : protocol_quantity, divisor : protocol_divisor) -> protocol_quantity = {
    let quotient = tdiv_int(value, divisor);
    assert(0 <= quotient);
    assert(quotient <= value);
    quotient
}

function optimized_gas_cost_sentinel() -> limb =
    0x8000000000000000

function optimized_unaffordable_gas_cost() -> gas_cost =
    GasCost(9223372036854775808)

function optimized_gas_cost_from_limb(value : limb) -> gas_cost =
    if value[63] == bitzero then {
        GasCost(unsigned(value[62 .. 0]))
    } else {
        optimized_unaffordable_gas_cost()
    }

function optimized_gas_cost_multiply(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> gas_cost = {
    if (left == 0) | (right == 0) then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if tdiv_int(limit, left) < right then {
            optimized_unaffordable_gas_cost()
        } else {
            let product = left * right;
            assert(product <= limit);
            GasCost(product)
        }
    }
}

function gas_cost_add(GasCost(left) : gas_cost, GasCost(right) : gas_cost) -> gas_cost = {
    let GasCost(limit) = optimized_unaffordable_gas_cost();
    if right <= limit - left then {
        GasCost(left + right)
    } else {
        optimized_unaffordable_gas_cost()
    }
}

function gas_cost_add_constant(left : gas_cost, GasConstant(right) : gas_constant) -> gas_cost =
    gas_cost_add(left, GasCost(right))

function gas_constant_add(GasConstant(left) : gas_constant, GasConstant(right) : gas_constant) -> gas_cost =
    gas_cost_add(GasCost(left), GasCost(right))

function gas_cost_scale(cost : gas_cost, factor : nat) -> gas_cost =
    if (cost == GAS_COST_ZERO) | (factor == 0) then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if factor <= limit then {
            optimized_gas_cost_multiply(cost, GasCost(factor))
        } else {
            optimized_unaffordable_gas_cost()
        }
    }

function gas_constant_scale(GasConstant(schedule_cost) : gas_constant, factor : nat) -> gas_cost =
    gas_cost_scale(GasCost(schedule_cost), factor)

function gas_constant_scale_byte_quantity(
    GasConstant(schedule_cost) : gas_constant,
    ByteQuantity(factor) : byte_quantity,
) -> (
    gas_cost
) =
    gas_cost_scale(GasCost(schedule_cost), factor)

function gas_quotient(Gas(value) : gas, divisor : gas_divisor) -> gas = {
    let quotient = tdiv_int(value, divisor);
    assert(0 <= quotient);
    assert(quotient <= value);
    Gas(quotient)
}

function gas_cost_quotient(GasCost(cost) : gas_cost, divisor : gas_divisor) -> gas_cost = {
    let quotient = tdiv_int(cost, divisor);
    assert(0 <= quotient);
    assert(quotient <= cost);
    GasCost(quotient)
}

function capped_gas_refund(GasRefund(refund) : gas_refund, limit : gas) -> gas = {
    let GasRefund(signed_limit) = gas_to_refund(limit);
    if refund <= 0 then {
        GAS_ZERO
    } else {
        if refund <= signed_limit then {
            assert(not_bool(refund <= 0));
            assert(refund <= 9223372036854775807);
            Gas(refund)
        } else {
            limit
        }
    }
}

function bloom_bit_mask(bit_to_set : bloom_limb_bit) -> limb =
    sail_shiftleft(LIMB_ONE, bit_to_set)

/*! Sets one bit (0–2047) in the bloom, most-significant-byte first. */
function bloom_set_bit(bloom : LogsBloom, bit_to_set : bloom_bit_index) -> LogsBloom = {
    var out = bloom;
    let quotient = tdiv_int(bit_to_set, 64);
    assert(0 <= quotient);
    assert(quotient <= 31);
    let natural_limb : bloom_limb_index = quotient;
    let limb_index : bloom_limb_index = 31 - natural_limb;
    let remainder = tmod_int(bit_to_set, 64);
    assert(0 <= remainder);
    assert(remainder <= 63);
    let bit_in_limb : bloom_limb_bit = remainder;
    let mask = bloom_bit_mask(bit_in_limb);
    out[limb_index] = or_vec(out[limb_index], mask);
    out
}

/* This generic helper is retained for the canonical API. Hot calculators
 * whose expression decreases after a wide intermediate are replaced below. */
function gas_product_quotient(left : nat, right : nat, divisor : gas_divisor) -> gas_cost = {
    let cost = exact_quotient(left * right, divisor);
    if (2 ^ 63) <= cost then {
        optimized_unaffordable_gas_cost()
    } else {
        GasCost(cost)
    }
}

function optimized_memory_words(size : limb) -> limb = {
    let quotient = sail_shiftright(size, 5);
    if and_vec(size, 0x000000000000001F) == LIMB_ZERO then {
        quotient
    } else {
        add_bits(quotient, LIMB_ONE)
    }
}

/* C_mem(new) - C_mem(old) is calculated as one fixed-width expression.
 * Saturating the two absolute C_mem values first would make sentinel -
 * sentinel incorrectly look free. A one-limb byte length has at most 2^59 words,
 * so both squares fit exactly in the existing 256-bit word algebra. */
function memory_expansion_gas(required_size : nat) -> gas_cost = {
    if not_bool(fits_limb(required_size)) then {
        return optimized_unaffordable_gas_cost()
    };
    let required = to_limb(required_size);
    let ByteQuantity(old_size) = evm_memory_size();
    let new_words = optimized_memory_words(required);
    let old_words = optimized_memory_words(to_limb(old_size));
    if limb_ule(new_words, old_words) then {
        GAS_COST_ZERO
    } else {
        let new_word = limb_to_word(new_words);
        let old_word = limb_to_word(old_words);
        let linear = word_mul(limb_to_word(0x0000000000000003), sub_bits(new_word, old_word));
        let new_quadratic = sail_shiftright(word_mul(new_word, new_word), 9);
        let old_quadratic = sail_shiftright(word_mul(old_word, old_word), 9);
        let cost = add_bits(linear, sub_bits(new_quadratic, old_quadratic));
        if word_ule(limb_to_word(optimized_gas_cost_sentinel()), cost) then {
            optimized_unaffordable_gas_cost()
        } else {
            match word_to_limb(cost) {
                Some(value) => optimized_gas_cost_from_limb(value),
                None() => optimized_unaffordable_gas_cost(),
            }
        }
    }
}

/* MODEXP deliberately widens through a 256-bit integer. Once its three
 * lengths fit one limb, the complete EIP-2565/7883 expression is below 2^190, so
 * the EVM word algebra is exact rather than wrapping. */
/*! `MODEXP` (precompile `0x05`) gas: EIP-2565 as amended by EIP-7883
    (Osaka). Input layout: `bsize[32] esize[32] msize[32] base exp mod`.
    Gas grows with the multiplication complexity of the larger of base
    and modulus, and with an iteration count derived from the exponent's
    bit length; EIP-7823 (Osaka) caps each field at 1024 bytes, modeled
    as an unforwardable cost. */
function modexp_gas(input : ByteSlice) -> option(gas_cost) = {
    let base_length_word = pc_word(input, 0, 32);
    let exponent_length_word = pc_word(input, 32, 32);
    let modulus_length_word = pc_word(input, 64, 32);
    let osaka = k_fork >= Osaka;
    let osaka_limit = limb_to_word(0x0000000000000400);

    if osaka &
        (  word_ult(osaka_limit, base_length_word)
        | word_ult(osaka_limit, exponent_length_word)
        | word_ult(osaka_limit, modulus_length_word)) then {
        return None()
    };

    if not_bool(osaka) & word_is_zero(base_length_word) & word_is_zero(modulus_length_word) then {
        return Some(GasCost(200))
    };

    let base_length : limb = match word_to_limb(base_length_word) {
        Some(value) => value,
        None() => return Some(optimized_unaffordable_gas_cost()),
    };
    let exponent_length : limb = match word_to_limb(exponent_length_word) {
        Some(value) => value,
        None() => return Some(optimized_unaffordable_gas_cost()),
    };
    let modulus_length : limb = match word_to_limb(modulus_length_word) {
        Some(value) => value,
        None() => return Some(optimized_unaffordable_gas_cost()),
    };

    let maximum_length = if limb_ult(modulus_length, base_length) then { base_length } else { modulus_length };
    let length_words_parts = limb_divmod(maximum_length, 0x0000000000000008);
    let length_words = if length_words_parts.remainder == LIMB_ZERO then {
            length_words_parts.quotient
        } else {
            add_bits(length_words_parts.quotient, LIMB_ONE)
        };
    let length_words_word = limb_to_word(length_words);
    let words_squared = word_mul(length_words_word, length_words_word);
    let multiplication_complexity = if osaka & limb_ule(maximum_length, 0x0000000000000020) then {
            limb_to_word(0x0000000000000010)
        } else {
            if osaka then {
                sail_shiftleft(words_squared, 1)
            } else {
                words_squared
            }
        };

    let exponent_offset = 96 + unsigned(base_length);
    let iteration_count : word = if limb_ule(exponent_length, 0x0000000000000020) then {
            let exponent_bits = word_bit_length(pc_word(input, exponent_offset, unsigned(exponent_length)));
            if exponent_bits <= 1 then {
                WORD_ONE
            } else {
                word_of_nat(exponent_bits - 1)
            }
        } else {
            let head_bits = word_bit_length(pc_word(input, exponent_offset, 32));
            let high_bits = if head_bits == 0 then {
                    0
                } else {
                    head_bits - 1
                };
            let multiplier = if osaka then { 0x0000000000000010 } else { 0x0000000000000008 };
            add_bits(
                word_mul(limb_to_word(multiplier), limb_to_word(sub_bits(exponent_length, 0x0000000000000020))),
                word_of_nat(high_bits),
            )
        };

    let product = word_mul(multiplication_complexity, iteration_count);
    let calculated = if osaka then {
            product
        } else {
            word_divmod(product, limb_to_word(0x0000000000000003)).quotient
        };
    let minimum = if osaka then {
            limb_to_word(0x00000000000001F4)
        } else {
            limb_to_word(0x00000000000000C8)
        };
    let cost = if word_ult(calculated, minimum) then { minimum } else { calculated };
    if word_ule(limb_to_word(0x8000000000000000), cost) then {
        Some(optimized_unaffordable_gas_cost())
    } else {
        match word_to_limb(cost) {
            Some(value) => Some(optimized_gas_cost_from_limb(value)),
            None() => Some(optimized_unaffordable_gas_cost()),
        }
    }
}

/* Divide the pair count first so the EIP-2537 product never needs a wide
 * temporary: a*k/1000 = a*(k/1000) + floor(a*(k%1000)/1000). */
/*! EIP-2537 MSM gas: `(k · base · discount(k)) / 1000`, with the
    discount clamped to the `k = 128` entry beyond the table. */
function bls_msm_gas(
    table : vector(128, inc, bits(16)),
    GasConstant(base) : gas_constant,
    maxd : bls_discount,
    k : nat,
) -> (
    gas_cost
) = {
    if k == 0 then {
        return GAS_COST_ZERO
    };
    var discount : gas_cost = GasCost(maxd);
    if k < 128 then {
        let target = 128 - k;
        var cursor : nat = 0;
        foreach (index from 0 to 127) {
            if cursor == target then {
                discount = GasCost(unsigned(table[index]))
            };
            cursor = cursor + 1
        }
    };
    let scaled_base = optimized_gas_cost_multiply(GasCost(base), discount);
    if scaled_base == GAS_COST_ZERO then {
        GAS_COST_ZERO
    } else {
        let GasCost(limit) = optimized_unaffordable_gas_cost();
        if limit < k then {
            optimized_unaffordable_gas_cost()
        } else {
            let bounded_k : gas_cost = GasCost(k);
            let GasCost(k_value) = bounded_k;
            let quotient = tdiv_int(k_value, 1000);
            let remainder = tmod_int(k_value, 1000);
            assert(0 <= quotient);
            assert(quotient <= limit);
            assert(0 <= remainder);
            assert(remainder <= limit);
            let whole = optimized_gas_cost_multiply(scaled_base, GasCost(quotient));
            let residual_product = optimized_gas_cost_multiply(scaled_base, GasCost(remainder));
            let residual = gas_cost_quotient(residual_product, 1000);
            gas_cost_add(whole, residual)
        }
    }
}

$[c_repr uint64]
newtype byte_quantity = ByteQuantity : nat

type source_pointer = byte_quantity
type byte_length = byte_quantity

type memory_pointer = byte_quantity
type memory_length = byte_quantity

type code_pointer = byte_quantity
type code_length = byte_quantity

function optimized_max_byte_quantity() -> byte_quantity =
    ByteQuantity(18446744073709551615)

function byte_quantity_fits_limb(_ : byte_quantity) -> bool =
    true

function byte_quantity_not_equal(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> bool =
    not_bool(left == right)

function byte_quantity_add(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    let ByteQuantity(maximum) = optimized_max_byte_quantity();
    assert(right <= maximum - left);
    ByteQuantity(left + right)
}

function byte_quantity_sub(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    assert(right <= left);
    ByteQuantity(left - right)
}

function byte_quantity_mul(ByteQuantity(left) : byte_quantity, ByteQuantity(right) : byte_quantity) -> byte_quantity = {
    let ByteQuantity(maximum) = optimized_max_byte_quantity();
    if left == 0 then {
        BYTE_ZERO
    } else {
        assert(right <= tdiv_int(maximum, left));
        ByteQuantity(left * right)
    }
}

function byte_quantity_quotient(
    ByteQuantity(dividend) : byte_quantity,
    ByteQuantity(divisor) : byte_quantity,
) -> (
    byte_quantity
) = {
    assert(not_bool(divisor == 0));
    let quotient = tdiv_int(dividend, divisor);
    assert(0 <= quotient);
    assert(quotient <= dividend);
    ByteQuantity(quotient)
}
