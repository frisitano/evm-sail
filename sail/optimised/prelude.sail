/* Optimized C overrides for `sail/prelude.sail`. */

/*
 * Preserve the specification's canonical byte-index semantics while storing
 * addresses and digests in packed fixed-width 64-bit lanes. Transparent aliases are
 * erased before C lowering, so these annotations select the representation
 * structurally for 20-byte and 32-byte vectors in the optimized compilation.
 */
$[c_repr { representation = fixed_bytes_u64_lanes, name = bytes20 }]
type address = vector(20, inc, byte)

$[c_repr { representation = fixed_bytes_u64_lanes, name = bytes32 }]
type b256 = vector(32, inc, byte)

/*
 * Optimized refinements of the base prelude's direct endian equations.
 * Canonical fixed bytes and native least-significant-first word limbs cross
 * this boundary directly; the standard model and proof extractions retain the
 * readable vector conversions.
 */
val hash_to_word = pure { c: "hash_to_word" } : hash -> word

$[c_static_eval word_to_fixed_bytes]
val word_to_hash = pure { c: "word_to_hash" } : word -> hash

$[c_static_eval word_to_fixed_bytes]
val word_to_address = pure { c: "word_to_address" } : word -> address

val address_to_word = pure { c: "address_to_word" } : address -> word

/*
 * Sail's semantic count-leading-zeros primitive returns a dependent integer.
 * The generic C ABI therefore routes it through `sail_int`; the optimized
 * runtime selects the equivalent native instruction at this boundary.
 */
val u64_bit_length_native = pure { c: "u64_bit_length" } : range(0, 2 ^ 64 - 1) -> range(0, 64)

function u64_bit_length(value : range(0, 2 ^ 64 - 1)) -> range(0, 64) =
    u64_bit_length_native(value)

/*
 * The exponentiation ladder is one closed word-level operation; optimized C
 * executes it directly over 64-bit limbs so each round costs native
 * multiplications instead of generic word helper calls.
 */
val word_exp_ladder = pure { c: "word_exp_ladder" } : (word, word) -> word

function alu_exp(base : word, exponent : word) -> word =
    word_exp_ladder(base, exponent)

/*
 * Nonzero-divisor 256-bit division and modular arithmetic are closed
 * word-level operations executed by the platform bigint provider
 * (ffi/zkvm_bigint.h); on ZisK each costs an arith256 precompile call
 * instead of a limb-level software routine. The EVM's zero-divisor rules
 * (DIV/MOD/SDIV/SMOD/ADDMOD/MULMOD yield 0) stay here in Sail, so the
 * provider's nonzero precondition always holds at the boundary.
 */
val word_div_nonzero = pure { c: "word_div_nonzero" } : (word, word) -> word

val word_mod_nonzero = pure { c: "word_mod_nonzero" } : (word, word) -> word

val word_mulmod_nonzero = pure { c: "word_mulmod_nonzero" } : (word, word, word) -> word

val word_addmod_nonzero = pure { c: "word_addmod_nonzero" } : (word, word, word) -> word

function alu_div(a : word, b : word) -> word = {
    /* DIV */
    let divisor_is_zero = word_is_zero(b);
    if divisor_is_zero then {
        WORD_ZERO
    } else {
        word_div_nonzero(a, b)
    }
}

function alu_mod(a : word, b : word) -> word = {
    /* MOD */
    let modulus_is_zero = word_is_zero(b);
    if modulus_is_zero then {
        WORD_ZERO
    } else {
        word_mod_nonzero(a, b)
    }
}

/* Canonical SDIV sign logic (sail/prelude.sail) with the unsigned magnitude
   division routed through the nonzero provider: a nonzero divisor has a
   nonzero magnitude, and SDIV(min, -1) truncates through the unsigned
   magnitudes exactly as in the base equations. */
function alu_sdiv(a : word, b : word) -> word = {
    /* SDIV */
    let divisor_is_zero = word_is_zero(b);
    if divisor_is_zero then {
        WORD_ZERO
    } else {
        let dividend_magnitude = word_abs(a);
        let divisor_magnitude = word_abs(b);
        let quotient = word_div_nonzero(dividend_magnitude, divisor_magnitude);
        let dividend_sign = word_bit(a, 255);
        let divisor_sign = word_bit(b, 255);
        if (dividend_sign == bitone) != (divisor_sign == bitone) then {
            word_negate(quotient)
        } else {
            quotient
        }
    }
}

/* Canonical SMOD sign logic with the magnitude remainder routed through the
   nonzero provider. */
function alu_smod(a : word, b : word) -> word = {
    /* SMOD */
    let modulus_is_zero = word_is_zero(b);
    if modulus_is_zero then {
        WORD_ZERO
    } else {
        let dividend_magnitude = word_abs(a);
        let modulus_magnitude = word_abs(b);
        let remainder = word_mod_nonzero(dividend_magnitude, modulus_magnitude);
        let dividend_sign = word_bit(a, 255);
        if dividend_sign == bitone then {
            word_negate(remainder)
        } else {
            remainder
        }
    }
}

/* NOTE: the custom compiler currently intercepts alu_addmod/alu_mulmod at
   the u256 ABI (specialized_function_external in c_backend.ml) and lowers
   both -- including these spliced bodies -- to its built-in software
   u256_addmod/u256_mulmod support routines, so the two overrides below do
   not reach the bigint provider yet. They are kept as the intended routing:
   once the intrinsic defers to a spliced override (or itself targets the
   zkvm_bigint.h contract), ADDMOD/MULMOD join DIV/MOD/SDIV/SMOD on the
   platform provider without further model changes. */
function alu_addmod(a, b, n) = {
    /* ADDMOD */
    if n == 0 then {
        WORD_ZERO
    } else {
        word_addmod_nonzero(a, b, n)
    }
}

function alu_mulmod(a, b, n) = {
    /* MULMOD */
    if n == 0 then {
        WORD_ZERO
    } else {
        word_mulmod_nonzero(a, b, n)
    }
}
