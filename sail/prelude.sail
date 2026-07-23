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
val U256 : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> word
function U256(value) = value

/*! A 20-byte account address (YP §4.1). */
type address = vector(20, dec, byte)

/* Transitional constructor spelling for address-producing sites. */
function Address(bytes : vector(20, dec, byte)) -> address = bytes

/*! A KECCAK-256 / storage-key sized digest. */
type b256 = vector(32, dec, byte)

/* Transitional constructor spelling for digest-producing sites. */
function B256(bytes : vector(32, dec, byte)) -> b256 = bytes
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
    unsigned(from_bytes_le(32, bytes))

/*! Serializes an EVM word as a 32-byte big-endian digest. */
function word_to_hash(value : word) -> hash =
    B256(to_bytes_le(32, get_slice_int(256, value, 0)))

/*! Compares two digests as unsigned big-endian integers. */
function hash_lt(left : hash, right : hash) -> bool =
    hash_to_word(left) < hash_to_word(right)

/*! Converts a word to its low 160-bit address. */
function word_to_address(value : word) -> address =
    Address(to_bytes_le(20, get_slice_int(160, value, 0)))

/* the all-zero word / address constants */
let ZERO_WORD : word = U256(unsigned(0x0000000000000000000000000000000000000000000000000000000000000000))
let ZERO_ADDRESS : address = Address(vector_init(20, 0x00))
let ZERO_HASH : hash = B256(vector_init(32, 0x00))

let WORD_ZERO : word = U256(unsigned(0x0000000000000000000000000000000000000000000000000000000000000000))
let WORD_ONE : word = U256(unsigned(0x0000000000000000000000000000000000000000000000000000000000000001))
let WORD_ALL_ONES : word = U256(unsigned(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff))
let WORD_SIGN_BIT : word = U256(unsigned(0x8000000000000000000000000000000000000000000000000000000000000000))

/* Keep every operation on the nominal EVM word inside this small algebra.
 * Arithmetic is stated over mathematical integers and reduced exactly once
 * at the EVM word boundary. */
val word_add_word : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> word
function word_add_word(left, right) =
    U256(tmod_nat(left + right, sizeof(word_modulus)))

val word_sub_word : forall 'left 'right, 0 <= 'left & 'left < 2 ^ 256 & 0 <= 'right & 'right < 2 ^ 256. (
        int('left),
        int('right),
    ) -> word
function word_sub_word(left, right) =
    U256(emod_positive(left - right, sizeof(word_modulus)))

function word_and(left : word, right : word) -> word =
    U256(unsigned(and_vec(get_slice_int(256, left, 0), get_slice_int(256, right, 0))))
function word_or(left : word, right : word) -> word =
    U256(unsigned(or_vec(get_slice_int(256, left, 0), get_slice_int(256, right, 0))))
function word_xor(left : word, right : word) -> word =
    U256(unsigned(xor_vec(get_slice_int(256, left, 0), get_slice_int(256, right, 0))))
function word_not(value : word) -> word = U256(unsigned(not_vec(get_slice_int(256, value, 0))))
function word_bit(value : word, index : range(0, 255)) -> bit = get_slice_int(256, value, 0)[index]
val word_low_byte : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> byte
function word_low_byte(value) = get_slice_int(8, value, 0)

function word_shift_left_one(value : word) -> word =
    U256(tmod_nat(value * 2, sizeof(word_modulus)))
function word_shift_right_one(value : word) -> word =
    U256(tdiv_nat(value, 2))

/*md
## Helpers

### Integer ↔ word conversion
*/
/*! `1` if the condition holds, else `0` — the EVM boolean convention. */
function word_of_bool(b : bool) -> word = if b then { WORD_ONE } else { WORD_ZERO }
val word_is_zero : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> bool
function word_is_zero(w) = w == WORD_ZERO

val word_nonzero : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> bool
function word_nonzero(w) = not_bool(word_is_zero(w))

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
function word_ule(a, b) = not_bool(word_ult(b, a))

/*! Returns the position above an 8-bit value's most significant set bit. */
function byte_bit_length(value : range(0, 2 ^ 8 - 1)) -> range(0, 8) =
    if value < 2 ^ 4 then {
        if value < 2 ^ 2 then {
            if value < 2 ^ 1 then { value } else { 2 }
        } else {
            if value < 2 ^ 3 then { 3 } else { 4 }
        }
    } else {
        if value < 2 ^ 6 then { if value < 2 ^ 5 then { 5 } else { 6 } } else { if value < 2 ^ 7 then { 7 } else { 8 } }
    }

/*! Returns the position above a 16-bit value's most significant set bit. */
function u16_bit_length(value : range(0, 2 ^ 16 - 1)) -> range(0, 16) =
    if value < 2 ^ 8 then {
        byte_bit_length(value)
    } else {
        8 + byte_bit_length(tdiv_nat(value, 2 ^ 8))
    }

/*! Returns the position above a 32-bit value's most significant set bit. */
function u32_bit_length(value : range(0, 2 ^ 32 - 1)) -> range(0, 32) =
    if value < 2 ^ 16 then {
        u16_bit_length(value)
    } else {
        16 + u16_bit_length(tdiv_nat(value, 2 ^ 16))
    }

/*! Returns the position above a 64-bit value's most significant set bit. */
function u64_bit_length(value : range(0, 2 ^ 64 - 1)) -> range(0, 64) =
    if value < 2 ^ 32 then {
        u32_bit_length(value)
    } else {
        32 + u32_bit_length(tdiv_nat(value, 2 ^ 32))
    }

/*! Returns the position above a 128-bit value's most significant set bit. */
function u128_bit_length(value : range(0, 2 ^ 128 - 1)) -> range(0, 128) =
    if value < 2 ^ 64 then {
        u64_bit_length(value)
    } else {
        64 + u64_bit_length(tdiv_nat(value, 2 ^ 64))
    }

/*! Returns the position above a word's most significant set bit.  The
    logarithmic decomposition keeps both the mathematical specification and
    native execution bounded without scanning all 256 bit positions. */
val word_bit_length : forall 'value, 0 <= 'value & 'value < 2 ^ 256. int('value) -> word_bit_count
function word_bit_length(value) = {
    let limb3 = unsigned(get_slice_int(64, value, 192));
    if limb3 != 0 then {
        192 + u64_bit_length(limb3)
    } else {
        let limb2 = unsigned(get_slice_int(64, value, 128));
        if limb2 != 0 then {
            128 + u64_bit_length(limb2)
        } else {
            let limb1 = unsigned(get_slice_int(64, value, 64));
            if limb1 != 0 then {
                64 + u64_bit_length(limb1)
            } else {
                u64_bit_length(unsigned(get_slice_int(64, value, 0)))
            }
        }
    }
}

overload word_add = {word_add_word}
overload word_sub = {word_sub_word}

/* Multiplication is exact in the canonical model and reduced only at the word
 * boundary.  The optimized splice lowers the equation to native U256
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
        U256(tdiv_nat(dividend, divisor))
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
        U256(tmod_nat(dividend, divisor))
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
    U256(unsigned(sail_shiftleft(get_slice_int(256, value, 0), amount)))
}

/*! Shifts a word right logically by a bounded count. */
function word_shift_right(value : word, amount : word_bit_count) -> word =
    unsigned(sail_shiftright(get_slice_int(256, value, 0), amount))

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
    if word_bit(value, 255) == bitone then {
        let sign_fill = word_shift_left(WORD_ALL_ONES, 256 - amount);
        word_or(shifted, sign_fill)
    } else {
        shifted
    }
}

/*! Embeds an address's bytes into the low 160 bits of an EVM word. */
function address_to_word(bytes : address) -> word =
    unsigned(from_bytes_le(20, bytes))

function word_negate(value : word) -> word = word_sub(WORD_ZERO, value)

/*! Returns the unsigned magnitude of a two's-complement word. */
function word_abs(value : word) -> word =
    if word_bit(value, 255) == bitone then {
        word_negate(value)
    } else {
        value
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
    } else {
        if b_neg then {
            false
        } else {
            word_ult(a, b)
        }
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
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        let quotient = word_div(word_abs(a), word_abs(b));
        if (word_bit(a, 255) == bitone) != (word_bit(b, 255) == bitone) then {
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
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        let remainder = word_mod(word_abs(a), word_abs(b));
        if word_bit(a, 255) == bitone then {
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
        U256(tmod_nat(a + b, n))
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
        U256(tmod_nat(a * b, n))
    }
}

/*! `EXP` via square-and-multiply over the 256 exponent bits, reduced
    modulo 2^256 at every step. */
function alu_exp(base : word, exponent : word) -> word = {
    var result : word = WORD_ONE;
    var b : word = base;
    var e : word = exponent;
    foreach (_step from 0 to 255) {
        if word_bit(e, 0) == bitone then {
            result = word_mul(result, b)
        };
        b = word_mul(b, b);
        e = word_shift_right_one(e)
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
        let sign_set = word_and(word_shift_right(value, sign_shift), WORD_ONE) == WORD_ONE;
        let low_mask = word_sub(word_shift_left(WORD_ONE, width), WORD_ONE);
        if sign_set then {
            word_or(word_and(value, low_mask), word_not(low_mask))
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
function alu_lt(a : word, b : word) -> word = word_of_bool(word_ult(a, b))
function alu_gt(a : word, b : word) -> word = word_of_bool(word_ult(b, a))
function alu_slt(a : word, b : word) -> word = word_of_bool(word_slt(a, b))
function alu_sgt(a : word, b : word) -> word = word_of_bool(word_slt(b, a))
function alu_eq(a : word, b : word) -> word = word_of_bool(a == b)
function alu_iszero(a : word) -> word = word_of_bool(word_is_zero(a))

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
        unsigned(word_low_byte(word_shift_right(x, shift)))
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
        if word_bit(v, 255) == bitone then { WORD_ALL_ONES } else { WORD_ZERO }
    }
}

/*! `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). */
function alu_clz(x : word) -> word = U256(256 - word_bit_length(x))
