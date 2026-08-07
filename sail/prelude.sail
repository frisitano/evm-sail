/* EVM words are bounded mathematical naturals. Runtime quantities use their */
/* own semantic domains; conversion to a word occurs only where the EVM      */
/* exposes such a quantity on its 256-bit operand stack.                     */

/*md
# The prelude

The value model and the 256-bit word ALU — the arithmetic foundation of
the specification. The EVM operates on 256-bit
words (Yellow Paper §9.1); this module defines that word type and gives
every word operation its full two's-complement / unsigned semantics modulo
`2^256`. Only KECCAK-256 and the precompile cryptography at the host
boundary are opaque primitives; everything else is fully defined here.

Operator grounding: `int → bits` is `get_slice_int`; integer division and
modulus are the Euclidean `ediv`/`emod` from `<smt.sail>` together with the
truncating/flooring `tdiv`/`tmod`/`fdiv` from `arith.sail`; subtraction on
bits is `sub_bits`; `xor`/`and`/`or`/`not` are the `*_vec` builtins. The
word comparisons are direct bit-slice definitions rather than opaque host
predicates.

| Name | Value | Description |
| ---- | ----- | ----------- |
| `WORD_BITS` / `WORD_BYTES` | `256` / `32` | The word width |
| `TWO_256` | `2^256` | The word modulus |
| `ZERO_WORD` / `WORD_ZERO` | `0` | The zero word |
| `WORD_ONE` | `1` | The unit word |
| `ZERO_ADDRESS` | `0` | The zero address |
*/
default Order dec

$include <prelude.sail>
$include <smt.sail>
$include <generic_equality.sail>

/* EVM arithmetic quantities are non-negative, so Euclidean division is the
 * specification's ordinary integer division. */
overload operator / = {ediv_int}

/*md
## Types
*/
/*! An 8-bit byte. */
type byte = bits(8)

/*! The EVM 256-bit machine word (YP §9.1).  A transparent range keeps the
    mathematical subtype relation visible: narrower non-negative ranges can
    be passed as words without a model-level conversion. */
type word = range(0, 2 ^ 256 - 1)

/* Transitional constructor spelling for value-producing sites.  Unlike the
 * former newtype constructor this is the identity and introduces no nominal
 * wrapper; construction sites are removed as their bounds become explicit. */
val u256 : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> word
function u256(value) = value

/*! A 20-byte account address (YP §4.1), in canonical protocol byte order. */
type address = vector(20, inc, byte)

/* Transitional constructor spelling for address-producing sites. */
function Address(bytes : vector(20, inc, byte)) -> address = bytes

/*! A KECCAK-256 / storage-key sized digest, in canonical protocol byte order. */
type b256 = vector(32, inc, byte)

/* Transitional constructor spelling for digest-producing sites. */
function B256(bytes : vector(32, inc, byte)) -> b256 = bytes
/*! The common digest type used by trie, code, and block hashes. */
type hash = b256

/*! A host address operation's success flag and address result. */
struct AddressResult = {
    success : bool,
    address : address,
}

/* The word modulus is a type-level singleton because its exact value needs
 * 257 bits.  Code that needs the mathematical value uses `sizeof` without
 * materializing an unsupported runtime integer in optimized builds. */
/*! The type-level modulus of EVM-word arithmetic. */
type word_modulus : Int = 2 ^ 256

/*! A bit count within one EVM word. */
type word_bit_count = range(0, 256)

/*! Interprets a digest as the corresponding big-endian EVM word. */
function hash_to_word(bytes : hash) -> word =
    unsigned(
          bytes[0]
        @ bytes[1]
        @ bytes[2]
        @ bytes[3]
        @ bytes[4]
        @ bytes[5]
        @ bytes[6]
        @ bytes[7]
        @ bytes[8]
        @ bytes[9]
        @ bytes[10]
        @ bytes[11]
        @ bytes[12]
        @ bytes[13]
        @ bytes[14]
        @ bytes[15]
        @ bytes[16]
        @ bytes[17]
        @ bytes[18]
        @ bytes[19]
        @ bytes[20]
        @ bytes[21]
        @ bytes[22]
        @ bytes[23]
        @ bytes[24]
        @ bytes[25]
        @ bytes[26]
        @ bytes[27]
        @ bytes[28]
        @ bytes[29]
        @ bytes[30]
        @ bytes[31],
    )

/*! Interprets one full-width bitvector as an EVM word. This named boundary
    keeps protocol constants readable without hiding conversion chains. */
function word_from_bits(value : bits(256)) -> word = {
    let natural_value = unsigned(value);
    u256(natural_value)
}

/*! Serializes an EVM word as a 32-byte big-endian digest. */
function word_to_hash(value : word) -> hash = {
    let zero_bytes = vector_init(32, 0x00);
    var result : hash = B256(zero_bytes);
    result[0] = get_slice_int(8, value, 248);
    result[1] = get_slice_int(8, value, 240);
    result[2] = get_slice_int(8, value, 232);
    result[3] = get_slice_int(8, value, 224);
    result[4] = get_slice_int(8, value, 216);
    result[5] = get_slice_int(8, value, 208);
    result[6] = get_slice_int(8, value, 200);
    result[7] = get_slice_int(8, value, 192);
    result[8] = get_slice_int(8, value, 184);
    result[9] = get_slice_int(8, value, 176);
    result[10] = get_slice_int(8, value, 168);
    result[11] = get_slice_int(8, value, 160);
    result[12] = get_slice_int(8, value, 152);
    result[13] = get_slice_int(8, value, 144);
    result[14] = get_slice_int(8, value, 136);
    result[15] = get_slice_int(8, value, 128);
    result[16] = get_slice_int(8, value, 120);
    result[17] = get_slice_int(8, value, 112);
    result[18] = get_slice_int(8, value, 104);
    result[19] = get_slice_int(8, value, 96);
    result[20] = get_slice_int(8, value, 88);
    result[21] = get_slice_int(8, value, 80);
    result[22] = get_slice_int(8, value, 72);
    result[23] = get_slice_int(8, value, 64);
    result[24] = get_slice_int(8, value, 56);
    result[25] = get_slice_int(8, value, 48);
    result[26] = get_slice_int(8, value, 40);
    result[27] = get_slice_int(8, value, 32);
    result[28] = get_slice_int(8, value, 24);
    result[29] = get_slice_int(8, value, 16);
    result[30] = get_slice_int(8, value, 8);
    result[31] = get_slice_int(8, value, 0);
    result
}

/*! Converts a word to its low 160-bit address in canonical byte order. */
function word_to_address(value : word) -> address = {
    let zero_bytes = vector_init(20, 0x00);
    var result : address = Address(zero_bytes);
    result[0] = get_slice_int(8, value, 152);
    result[1] = get_slice_int(8, value, 144);
    result[2] = get_slice_int(8, value, 136);
    result[3] = get_slice_int(8, value, 128);
    result[4] = get_slice_int(8, value, 120);
    result[5] = get_slice_int(8, value, 112);
    result[6] = get_slice_int(8, value, 104);
    result[7] = get_slice_int(8, value, 96);
    result[8] = get_slice_int(8, value, 88);
    result[9] = get_slice_int(8, value, 80);
    result[10] = get_slice_int(8, value, 72);
    result[11] = get_slice_int(8, value, 64);
    result[12] = get_slice_int(8, value, 56);
    result[13] = get_slice_int(8, value, 48);
    result[14] = get_slice_int(8, value, 40);
    result[15] = get_slice_int(8, value, 32);
    result[16] = get_slice_int(8, value, 24);
    result[17] = get_slice_int(8, value, 16);
    result[18] = get_slice_int(8, value, 8);
    result[19] = get_slice_int(8, value, 0);
    result
}

/*! Interprets canonical full-width bits directly as a digest. */
function hash_from_bits(value : bits(256)) -> hash = {
    let word_value = word_from_bits(value);
    word_to_hash(word_value)
}

/*! Interprets the low 160 bits of full-width bits as an address. */
function address_from_bits(value : bits(256)) -> address = {
    let word_value = word_from_bits(value);
    word_to_address(word_value)
}

/*! Constructs a small protocol address through the canonical word path. */
val address_from_nat : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> address
function address_from_nat(value) = {
    let word_value = u256(value);
    word_to_address(word_value)
}

/* the all-zero word / address constants */
let ZERO_WORD : word = word_from_bits(0x0000000000000000000000000000000000000000000000000000000000000000)
let ZERO_ADDRESS : address = address_from_bits(0x0000000000000000000000000000000000000000000000000000000000000000)
let ZERO_HASH : hash = hash_from_bits(0x0000000000000000000000000000000000000000000000000000000000000000)

let WORD_ZERO : word = word_from_bits(0x0000000000000000000000000000000000000000000000000000000000000000)
let WORD_ONE : word = word_from_bits(0x0000000000000000000000000000000000000000000000000000000000000001)
let WORD_ALL_ONES : word = word_from_bits(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
let WORD_SIGN_BIT : word = word_from_bits(0x8000000000000000000000000000000000000000000000000000000000000000)

/* Keep every operation on the nominal EVM word inside this small algebra.
 * Arithmetic is stated over mathematical integers and reduced exactly once
 * at the EVM word boundary. */
val word_add_word : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> word
function word_add_word(left, right) = {
    let reduced = tmod_nat(left + right, sizeof(word_modulus));
    u256(reduced)
}

val word_sub_word : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> word
function word_sub_word(left, right) =
    if right <= left then {
        left - right
    } else {
        let maximum : int(2 ^ 256 - 1) = sizeof(word_modulus) - 1;
        (maximum - (right - left)) + 1
    }

function word_and(left : word, right : word) -> word = {
    let left_bits = get_slice_int(256, left, 0);
    let right_bits = get_slice_int(256, right, 0);
    let result_bits = and_vec(left_bits, right_bits);
    let result = unsigned(result_bits);
    u256(result)
}
function word_or(left : word, right : word) -> word = {
    let left_bits = get_slice_int(256, left, 0);
    let right_bits = get_slice_int(256, right, 0);
    let result_bits = or_vec(left_bits, right_bits);
    let result = unsigned(result_bits);
    u256(result)
}
function word_xor(left : word, right : word) -> word = {
    let left_bits = get_slice_int(256, left, 0);
    let right_bits = get_slice_int(256, right, 0);
    let result_bits = xor_vec(left_bits, right_bits);
    let result = unsigned(result_bits);
    u256(result)
}
function word_not(value : word) -> word = {
    let value_bits = get_slice_int(256, value, 0);
    let result_bits = not_vec(value_bits);
    let result = unsigned(result_bits);
    u256(result)
}
function word_bit(value : word, index : range(0, 255)) -> bit = get_slice_int(256, value, 0)[index]
val word_low_byte : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> byte
function word_low_byte(value) = get_slice_int(8, value, 0)

function word_shift_right_one(value : word) -> word = {
    let shifted = tdiv_nat(value, 2);
    u256(shifted)
}

/*md
## Helpers

### Integer ↔ word conversion
*/
/*! `1` if the condition holds, else `0` — the EVM boolean convention. */
function word_of_bool(b : bool) -> word = if b then {
    WORD_ONE
} else {
    WORD_ZERO
}
val word_is_zero : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> bool
function word_is_zero(w) = w == WORD_ZERO

val word_nonzero : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> bool
function word_nonzero(w) = {
    let is_zero = word_is_zero(w);
    not_bool(is_zero)
}

/*! Unsigned word order.  Narrower natural ranges are word subtypes, so the
    same functions also accept statically bounded operands. */
val word_ult : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> bool
function word_ult(a, b) = a < b

val word_ule : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> bool
function word_ule(a, b) = {
    let greater = word_ult(b, a);
    not_bool(greater)
}

/*! Returns the position above a 64-bit value's most significant set bit.
    Sail defines leading-zero count over fixed bitvectors in every semantic
    backend; subtracting it from the vector width also gives zero for zero. */
function u64_bit_length(value : range(0, 2 ^ 64 - 1)) -> range(0, 64) = {
    let value_bits = get_slice_int(64, value, 0);
    let leading : range(0, 64) = count_leading_zeros(value_bits);
    64 - leading
}

/*! Returns the position above a word's most significant set bit.  The
    logarithmic decomposition keeps both the mathematical specification and
    native execution bounded without scanning all 256 bit positions. */
val word_bit_length : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> word_bit_count
function word_bit_length(value) = {
    let limb3_bits = get_slice_int(64, value, 192);
    let limb3 = unsigned(limb3_bits);
    if limb3 != 0 then {
        192 + u64_bit_length(limb3)
    } else {
        let limb2_bits = get_slice_int(64, value, 128);
        let limb2 = unsigned(limb2_bits);
        if limb2 != 0 then {
            128 + u64_bit_length(limb2)
        } else {
            let limb1_bits = get_slice_int(64, value, 64);
            let limb1 = unsigned(limb1_bits);
            if limb1 != 0 then {
                64 + u64_bit_length(limb1)
            } else {
                let limb0_bits = get_slice_int(64, value, 0);
                let limb0 = unsigned(limb0_bits);
                u64_bit_length(limb0)
            }
        }
    }
}

overload word_add = {word_add_word}
overload word_sub = {word_sub_word}

/* Multiplication is exact in the canonical model and reduced only at the word
 * boundary.  The optimized splice lowers the equation to native u256
 * multiplication, whose low half is the same result. */
/*! Multiplies words modulo the 256-bit word modulus. */
val word_mul_word : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> int(mod('left * 'right, 2 ^ 256))
function word_mul_word(a, b) = tmod_nat(a * b, sizeof(word_modulus))
overload word_mul = {word_mul_word}

/* Unsigned Euclidean division over the canonical mathematical carrier. */
/*! Computes an unsigned word quotient, yielding zero for a zero divisor. */
val word_div_word : forall 'dividend 'divisor,
                      0 <= 'dividend & 'dividend < 2 ^ 256 & 0 <= 'divisor & 'divisor < 2 ^ 256. (
        int('dividend),
        int('divisor),
    ) -> word
function word_div_word(dividend, divisor) = {
    if divisor == 0 then {
        WORD_ZERO
    } else {
        let quotient = tdiv_nat(dividend, divisor);
        u256(quotient)
    }
}

overload word_div = {word_div_word}

/*! Computes an unsigned word remainder, yielding zero for a zero divisor. */
val word_mod_word : forall 'dividend 'divisor,
                      0 <= 'dividend & 'dividend < 2 ^ 256 & 0 <= 'divisor & 'divisor < 2 ^ 256. (
        int('dividend),
        int('divisor),
    ) -> word
function word_mod_word(dividend, divisor) = {
    if divisor == 0 then {
        WORD_ZERO
    } else {
        let remainder = tmod_nat(dividend, divisor);
        u256(remainder)
    }
}
overload word_mod = {word_mod_word}

/*! Compares two EVM-word-domain values without widening the narrower operand. */
val word_greater_than_word : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> bool
function word_greater_than_word(left, right) = left > right

/*! Shifts a word left by a bounded count, yielding zero at the width. */
function word_shift_left(value : word, amount : word_bit_count) -> word = {
    let value_bits = get_slice_int(256, value, 0);
    let shifted_bits = sail_shiftleft(value_bits, amount);
    let shifted = unsigned(shifted_bits);
    u256(shifted)
}

/*! Shifts a word right logically by a bounded count. */
function word_shift_right(value : word, amount : word_bit_count) -> word = {
    let value_bits = get_slice_int(256, value, 0);
    let shifted_bits = sail_shiftright(value_bits, amount);
    unsigned(shifted_bits)
}

/*! Returns the number of significant bytes in a word. */
val word_byte_length : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> range(0, 32)
function word_byte_length(value) = {
    let bit_length = word_bit_length(value);
    if bit_length == 0 then {
        0
    } else {
        tdiv_nat(bit_length + 7, 8)
    }
}

/*! Shifts a two's-complement word right while extending its sign bit. */
function word_arithmetic_shift_right(value : word, amount : word_bit_count) -> word = {
    let shifted = word_shift_right(value, amount);
    let sign_bit = word_bit(value, 255);
    if sign_bit == bitone then {
        let sign_fill = word_shift_left(WORD_ALL_ONES, 256 - amount);
        word_or(shifted, sign_fill)
    } else {
        shifted
    }
}

/*! Embeds a canonical-order address into the low 160 bits of an EVM word. */
function address_to_word(bytes : address) -> word =
    unsigned(
          bytes[0]
        @ bytes[1]
        @ bytes[2]
        @ bytes[3]
        @ bytes[4]
        @ bytes[5]
        @ bytes[6]
        @ bytes[7]
        @ bytes[8]
        @ bytes[9]
        @ bytes[10]
        @ bytes[11]
        @ bytes[12]
        @ bytes[13]
        @ bytes[14]
        @ bytes[15]
        @ bytes[16]
        @ bytes[17]
        @ bytes[18]
        @ bytes[19],
    )

function word_negate(value : word) -> word = word_sub(WORD_ZERO, value)

/*! Returns the unsigned magnitude of a two's-complement word. */
function word_abs(value : word) -> word = {
    let sign_bit = word_bit(value, 255);
    if sign_bit == bitone then {
        word_negate(value)
    } else {
        value
    }
}

/*! Signed (two's-complement) 256-bit less-than: sign bits decide when they
    differ, otherwise the unsigned order applies. */
function word_slt(a : word, b : word) -> bool = {
    let a_neg = word_bit(a, 255) == bitone;
    let b_neg = word_bit(b, 255) == bitone;
    if a_neg then {
        if b_neg then {
            word_ult(a, b)
        } else {
            true
        }
    } else if b_neg then {
        false
    } else {
        word_ult(a, b)
    }
}

/*md
## The 256-bit ALU

The word operations behind the arithmetic, comparison, bitwise, and shift
opcodes (YP Appendix H). Names mirror the EVM opcodes; all results are
reduced modulo 2^256.

### Arithmetic
*/
val alu_add : forall 'right, 0 <= 'right & 'right < 2 ^ 256. (word, int('right)) -> word
function alu_add(a, b) = word_add(a, b) /* ADD */
function alu_sub(a : word, b : word) -> word = word_sub(a, b) /* SUB */
val alu_mul : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> int(mod('left * 'right, 2 ^ 256))
function alu_mul(a, b) = word_mul(a, b) /* MUL */

/*! `DIV`: unsigned Euclidean division; division by zero yields `0`
    (YP Appendix H). */
function alu_div(a : word, b : word) -> word =
    word_div(a, b) /* DIV */

/*! `MOD`: unsigned modulus; a zero modulus yields `0`. */
function alu_mod(a : word, b : word) -> word =
    word_mod(a, b) /* MOD */

/*! `SDIV`: signed division, truncating toward zero; division by zero
    yields `0`. */
function alu_sdiv(a : word, b : word) -> word = {
    /* SDIV */
    let divisor_is_zero = word_is_zero(b);
    if divisor_is_zero then {
        WORD_ZERO
    } else {
        let dividend_magnitude = word_abs(a);
        let divisor_magnitude = word_abs(b);
        let quotient = word_div(dividend_magnitude, divisor_magnitude);
        let dividend_sign = word_bit(a, 255);
        let divisor_sign = word_bit(b, 255);
        if (dividend_sign == bitone) != (divisor_sign == bitone) then {
            word_negate(quotient)
        } else {
            quotient
        }
    }
}

/*! `SMOD`: signed remainder, with the sign of the dividend; a zero modulus
    yields `0`. */
function alu_smod(a : word, b : word) -> word = {
    /* SMOD */
    let modulus_is_zero = word_is_zero(b);
    if modulus_is_zero then {
        WORD_ZERO
    } else {
        let dividend_magnitude = word_abs(a);
        let modulus_magnitude = word_abs(b);
        let remainder = word_mod(dividend_magnitude, modulus_magnitude);
        let dividend_sign = word_bit(a, 255);
        if dividend_sign == bitone then {
            word_negate(remainder)
        } else {
            remainder
        }
    }
}

/*! `ADDMOD`: `(a + b) mod n` over the unbounded integers (no intermediate
    2^256 reduction); a zero modulus yields `0`. */
val alu_addmod : forall 'a 'b 'n, 0 <= 'a & 'a < 2 ^ 256 & 0 <= 'b & 'b < 2 ^ 256 & 0 <= 'n & 'n < 2 ^ 256. (
        int('a),
        int('b),
        int('n),
    ) -> word
function alu_addmod(a, b, n) = {
    /* ADDMOD */
    if n == 0 then {
        WORD_ZERO
    } else {
        let remainder = tmod_nat(a + b, n);
        u256(remainder)
    }
}

/*! `MULMOD`: `(a * b) mod n` over the unbounded integers; a zero modulus
    yields `0`. */
val alu_mulmod : forall 'a 'b 'n, 0 <= 'a & 'a < 2 ^ 256 & 0 <= 'b & 'b < 2 ^ 256 & 0 <= 'n & 'n < 2 ^ 256. (
        int('a),
        int('b),
        int('n),
    ) -> word
function alu_mulmod(a, b, n) = {
    /* MULMOD */
    if n == 0 then {
        WORD_ZERO
    } else {
        let remainder = tmod_nat(a * b, n);
        u256(remainder)
    }
}

/*! `EXP` via square-and-multiply over the 256 exponent bits, reduced
    modulo 2^256 at every step. */
function alu_exp(base : word, exponent : word) -> word = {
    var result : word = WORD_ONE;
    var b : word = base;
    var e : word = exponent;
    /* Square-and-multiply over exactly the exponent's significant bits:
       every round beyond the bit length multiplies by one and squares a
       dead base. The final squaring feeds no later round and is skipped. */
    var remaining : word_bit_count = word_bit_length(exponent);
    while remaining > 0 termination_measure(remaining) do {
        let rounds_left = remaining;
        let low_bit = word_bit(e, 0);
        if low_bit == bitone then {
            result = word_mul(result, b)
        };
        if rounds_left > 1 then {
            b = word_mul(b, b)
        };
        e = word_shift_right_one(e);
        remaining = if rounds_left > 0 then rounds_left - 1 else 0
    };
    result
}

/*! `SIGNEXTEND(byte_index, value)`: sign-extends `value` from byte
    `byte_index` (0 = least significant); indices ≥ 31 leave the value
    unchanged. */
function alu_signextend(byte_index : word, value : word) -> word = {
    if byte_index < 32 then {
        let index : range(0, 31) = byte_index;
        let width : word_bit_count = index * 8 + 8;
        let sign_shift : word_bit_count = index * 8 + 7;
        let shifted_sign = word_shift_right(value, sign_shift);
        let isolated_sign = word_and(shifted_sign, WORD_ONE);
        let sign_set = isolated_sign == WORD_ONE;
        let low_mask_end = word_shift_left(WORD_ONE, width);
        let low_mask = word_sub(low_mask_end, WORD_ONE);
        if sign_set then {
            let low_value = word_and(value, low_mask);
            let high_mask = word_not(low_mask);
            word_or(low_value, high_mask)
        } else {
            word_and(value, low_mask)
        }
    } else {
        value
    }
}

/*md
### Comparison

Each comparison pushes `1` or `0`.
*/
function alu_lt(a : word, b : word) -> word = {
    let result = word_ult(a, b);
    word_of_bool(result)
}
function alu_gt(a : word, b : word) -> word = {
    let result = word_ult(b, a);
    word_of_bool(result)
}
function alu_slt(a : word, b : word) -> word = {
    let result = word_slt(a, b);
    word_of_bool(result)
}
function alu_sgt(a : word, b : word) -> word = {
    let result = word_slt(b, a);
    word_of_bool(result)
}
function alu_eq(a : word, b : word) -> word = word_of_bool(a == b)
function alu_iszero(a : word) -> word = {
    let result = word_is_zero(a);
    word_of_bool(result)
}

/*md
### Bitwise
*/
function alu_and(a : word, b : word) -> word = word_and(a, b)
function alu_or(a : word, b : word) -> word = word_or(a, b)
function alu_xor(a : word, b : word) -> word = word_xor(a, b)
function alu_not(a : word) -> word = word_not(a)

/*! `BYTE(i, x)`: the `i`-th most-significant byte of `x` (0 = MSB);
    indices ≥ 32 yield `0`. */
function alu_byte(i : word, x : word) -> word = {
    if i < 32 then {
        let index : range(0, 31) = i;
        let shift : word_bit_count = (31 - index) * 8;
        let shifted = word_shift_right(x, shift);
        let result_byte = word_low_byte(shifted);
        unsigned(result_byte)
    } else {
        WORD_ZERO
    }
}

/*md
### Shifts

The shift amount is the top stack item, per EVM `SHL`/`SHR`/`SAR`
(EIP-145).
*/
/*! `SHL`: logical left shift; amounts ≥ 256 yield `0`. */
function alu_shl(shift_amt : word, v : word) -> word =
    /* SHL */
    if shift_amt < 256 then {
        word_shift_left(v, shift_amt)
    } else {
        WORD_ZERO
    }

/*! `SHR`: logical right shift; amounts ≥ 256 yield `0`. */
function alu_shr(shift_amt : word, v : word) -> word =
    /* SHR */
    if shift_amt < 256 then {
        word_shift_right(v, shift_amt)
    } else {
        WORD_ZERO
    }

/* SAR: arithmetic (sign-propagating) right shift. */
/*! `SAR`: arithmetic (sign-propagating) right shift. */
function alu_sar(shift_amt : word, v : word) -> word = {
    /* SAR */
    if shift_amt < 256 then {
        word_arithmetic_shift_right(v, shift_amt)
    } else {
        let sign_bit = word_bit(v, 255);
        if sign_bit == bitone then {
            WORD_ALL_ONES
        } else {
            WORD_ZERO
        }
    }
}

/*! `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). */
function alu_clz(x : word) -> word = {
    let bit_length = word_bit_length(x);
    u256(256 - bit_length)
}
