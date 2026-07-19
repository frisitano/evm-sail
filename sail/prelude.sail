/* EVM words stay in fixed-width bits. Runtime quantities use mathematical  */
/* integers; conversion to a word occurs only where the EVM exposes such a  */
/* quantity on its 256-bit operand stack.                                   */

/*md
# The prelude

The value model and the 256-bit word ALU — the arithmetic foundation of
the specification. The EVM operates on 256-bit
words (Yellow Paper §9.1); this module defines that word type and gives
every word operation its full two's-complement / unsigned semantics over
`bits(256)`. Only KECCAK-256 and the precompile cryptography at the host
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
| `ZERO_ADDR` | `0` | The zero address |
*/
default Order dec

$include <prelude.sail>
$include <generic_equality.sail>

/*md
## Types
*/
/*! An 8-bit byte. */
type byte = bits(8)

/*! The EVM 256-bit machine word (YP §9.1), nominally separated from
    other 256-bit protocol values. */
newtype word = U256 : bits(256)

/*! A 20-byte account address (YP §4.1). */
newtype address = Address : vector(20, dec, byte)

/*! A KECCAK-256 / storage-key sized digest. */
newtype b256 = B256 : vector(32, dec, byte)
/*! The common digest type used by trie, code, and block hashes. */
type hash = b256

/*! A fixed-width 64-bit limb used to define word arithmetic. */
type limb    = bits(64)

/*! A host address operation's success flag and nominal address result. */
struct AddressResult = {
    success : bool,
    address : address,
}

/*! The quotient and remainder of one limb division. */
struct LimbDivMod = {
    quotient : limb,
    remainder : limb,
}

/*! The quotient and remainder of one unsigned word division. */
struct WordDivMod = {
    quotient : word,
    remainder : word,
}

/* The byte vectors use Sail's conventional decreasing order: index zero is
 * the least-significant byte. These explicit bridges keep bitvector-specific
 * operations at the edge of the nominal address/hash domains. */
/*! Constructs an address from its 160-bit numerical representation. */
function address_from_bits(value : bits(160)) -> address = {
    var bytes : vector(20, dec, byte) = vector_init(20, 0x00);
    foreach (k from 0 to 19) {
        bytes[k] = sail_shiftright(value, 8 * k)[7 .. 0]
    };
    Address(bytes)
}

/*! Returns an address's 160-bit numerical representation. */
function address_to_bits(Address(bytes) : address) -> bits(160) = {
    var value : bits(160) = sail_zeros(160);
    foreach (k from 0 to 19) {
        value = or_vec(value, sail_shiftleft(sail_zero_extend(bytes[k], 160), 8 * k))
    };
    value
}

/*! Constructs a digest from its 256-bit numerical representation. */
function b256_from_bits(value : bits(256)) -> b256 = {
    var bytes : vector(32, dec, byte) = vector_init(32, 0x00);
    foreach (k from 0 to 31) {
        bytes[k] = sail_shiftright(value, 8 * k)[7 .. 0]
    };
    B256(bytes)
}

/*! Returns a digest's 256-bit numerical representation. */
function b256_to_bits(B256(bytes) : b256) -> bits(256) = {
    var value : bits(256) = sail_zeros(256);
    foreach (k from 0 to 31) {
        value = or_vec(value, sail_shiftleft(sail_zero_extend(bytes[k], 256), 8 * k))
    };
    value
}

function hash_from_bits(value : bits(256)) -> hash = b256_from_bits(value)
function hash_to_bits(value : hash) -> bits(256) = b256_to_bits(value)

function b256_zero() -> b256 = B256(vector_init(32, 0x00))

/* Compare fixed bytes in wire order. The decreasing vector order keeps the
 * most-significant/wire-first byte at index 31. */
/*! Compares two digests as unsigned big-endian integers. */
function b256_lt(B256(left) : b256, B256(right) : b256) -> bool = {
    var less : bool = false;
    var equal : bool = true;
    foreach (offset from 0 to 31) {
        let index = 31 - offset;
        if equal & (left[index] != right[index]) then {
            less = unsigned(left[index]) < unsigned(right[index]);
            equal = false
        }
    };
    less
}

/* Nominal container bridges are defined before the constants that use them.
 * Optimized extraction replaces these bodies with direct fixed-container /
 * four-limb operations. */
function hash_to_word(value : hash) -> word = U256(hash_to_bits(value))
function word_to_hash(U256(value) : word) -> hash = hash_from_bits(value)
function word_to_address(U256(w) : word) -> address = address_from_bits(w[159 .. 0])
function address_to_word(a : address) -> word = U256(sail_zero_extend(address_to_bits(a), 256))

/* the all-zero word / address constants */
let ZERO_WORD : word = U256(0x0000000000000000000000000000000000000000000000000000000000000000)
let ZERO_ADDR : address = word_to_address(ZERO_WORD)
let ZERO_HASH : hash = word_to_hash(ZERO_WORD)

let WORD_ZERO : word = U256(0x0000000000000000000000000000000000000000000000000000000000000000)
let WORD_ONE : word = U256(0x0000000000000000000000000000000000000000000000000000000000000001)
let WORD_ALL_ONES : word = U256(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
let WORD_SIGN_BIT : word = U256(0x8000000000000000000000000000000000000000000000000000000000000000)

/* Keep every operation on the nominal EVM word inside this small algebra.
 * The canonical model still computes over bits(256); the optimized C splice
 * gives the nominal carrier its four-limb representation without exposing a
 * generic Sail bitvector at call sites. */
function word_to_bits(U256(value) : word) -> bits(256) = value
function word_add(U256(left) : word, U256(right) : word) -> word = U256(add_bits(left, right))
function word_sub(U256(left) : word, U256(right) : word) -> word = U256(sub_bits(left, right))
function word_and(U256(left) : word, U256(right) : word) -> word = U256(and_vec(left, right))
function word_or(U256(left) : word, U256(right) : word) -> word = U256(or_vec(left, right))
function word_xor(U256(left) : word, U256(right) : word) -> word = U256(xor_vec(left, right))
function word_not(U256(value) : word) -> word = U256(not_vec(value))
function word_bit(U256(value) : word, index : range(0, 255)) -> bit = value[index]
function word_low_byte(U256(value) : word) -> byte = value[7 .. 0]
function word_limb_0(U256(value) : word) -> limb = value[63 .. 0]
function word_limb_1(U256(value) : word) -> limb = value[127 .. 64]
function word_limb_2(U256(value) : word) -> limb = value[191 .. 128]
function word_limb_3(U256(value) : word) -> limb = value[255 .. 192]

/* Construct a word from little-endian 64-bit limbs.  Keeping this bridge in
 * the nominal word algebra lets optimized backends build their native U256
 * representation directly instead of materialising intermediate lbits. */
function word_from_limbs(limb_0 : limb, limb_1 : limb, limb_2 : limb, limb_3 : limb) -> word =
    U256(append(limb_3, append(limb_2, append(limb_1, limb_0))))
function word_shift_left_one(U256(value) : word) -> word = U256(sail_shiftleft(value, 1))
function word_shift_right_one(U256(value) : word) -> word = U256(sail_shiftright(value, 1))

/*md
## Helpers

### Integer ↔ word conversion
*/
/*! `1` if the condition holds, else `0` — the EVM boolean convention. */
function word_of_bool(b : bool) -> word = if b then { WORD_ONE } else { WORD_ZERO }
function word_is_zero(w : word) -> bool = w == WORD_ZERO
function word_nonzero(w : word) -> bool = not_bool(word_is_zero(w))

let LIMB_ZERO : limb = 0x0000000000000000
let LIMB_ONE : limb = 0x0000000000000001
let LIMB_MAX : limb = 0xFFFFFFFFFFFFFFFF

/* Unsigned comparison via the borrow bit of a - b. This stays entirely in the
 * fixed-width algebra: unlike unsigned(bits(64)), the C backend emits native
 * uint64_t operations and never constructs a sail_int. */
/*! Compares two limbs as unsigned values. */
function limb_ult(a : limb, b : limb) -> bool = {
    let difference = sub_bits(a, b);

    let borrow = or_vec(and_vec(not_vec(a), b), and_vec(not_vec(xor_vec(a, b)), difference));
    and_vec(borrow, 0x8000000000000000) != LIMB_ZERO
}

function limb_ule(a : limb, b : limb) -> bool = not_bool(limb_ult(b, a))

/* Fixed-width checked arithmetic. Plain bits(64) addition and subtraction are
 * modular; callers that model quantities use these operations so wraparound
 * is represented explicitly and mapped to the operation's EVM behaviour. */
/*! Adds limbs and reports fixed-width overflow. */
function limb_checked_add(a : limb, b : limb) -> option(limb) = {
    let result = add_bits(a, b);
    if limb_ult(result, a) then {
        None()
    } else {
        Some(result)
    }
}

/*! Subtracts limbs and reports fixed-width underflow. */
function limb_checked_sub(a : limb, b : limb) -> option(limb) =
    if limb_ult(a, b) then {
        None()
    } else {
        Some(sub_bits(a, b))
    }

/* Fixed-width checked multiplication. */
/*! Multiplies limbs and reports fixed-width overflow. */
function limb_checked_mul(value : limb, factor : limb) -> option(limb) = {
    var result : limb = LIMB_ZERO;
    var addend : limb = value;
    var remaining : limb = factor;
    var valid : bool = true;
    foreach (i from 0 to 63) {
        if valid & ((remaining & LIMB_ONE) == LIMB_ONE) then {
            let added = limb_checked_add(result, addend);
            match added {
                Some(value) => result = value,
                None() => valid = false,
            }
        };
        remaining = sail_shiftright(remaining, 1);
        if valid & (i < 63) & (remaining != LIMB_ZERO) then {
            let doubled = limb_checked_add(addend, addend);
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

/* Unsigned fixed-width long division. The loop consumes one dividend bit per
 * iteration, so neither the model nor generated code crosses through Sail's
 * arbitrary-precision integer representation. */
/*! Computes unsigned limb quotient and remainder by bounded long division. */
function limb_divmod(dividend : limb, divisor : limb) -> LimbDivMod = {
    if divisor == LIMB_ZERO then {
        struct { quotient = LIMB_ZERO, remainder = LIMB_ZERO }
    } else {
        var quotient : limb = LIMB_ZERO;
        var remainder : limb = LIMB_ZERO;
        var remaining : limb = dividend;
        foreach (_step from 0 to 63) {
            let incoming = if remaining[63] == bitone then { LIMB_ONE } else { LIMB_ZERO };
            let overflow = remainder[63] == bitone;
            remainder = or_vec(sail_shiftleft(remainder, 1), incoming);
            remaining = sail_shiftleft(remaining, 1);
            quotient = sail_shiftleft(quotient, 1);
            if overflow | limb_ule(divisor, remainder) then {
                remainder = sub_bits(remainder, divisor);
                quotient = or_vec(quotient, LIMB_ONE)
            }
        };
        struct { quotient = quotient, remainder = remainder }
    }
}

/*! A bit count within one limb. */
type limb_bit_count = range(0, 64)
/*! A bit count within one EVM word. */
type word_bit_count = range(0, 256)

/*! Increments a word bit count without exceeding the word width. */
function word_bit_count_increment(value : word_bit_count) -> word_bit_count =
    if value < 256 then {
        value + 1
    } else {
        assert(false);
        0
    }

/*! Returns the position above a limb's most significant set bit. */
function limb_bit_length(value : limb) -> limb_bit_count = {
    var remaining : limb = value;
    var length : limb_bit_count = 0;
    foreach (step from 0 to 63) {
        if remaining != LIMB_ZERO then {
            length = step + 1;
            remaining = sail_shiftright(remaining, 1)
        }
    };
    length
}

/* Lossless word narrowing. The caller, not this helper, chooses what a value
 * wider than the host quantity domain means for its opcode. */
/*! Narrows a word to one limb when all higher bits are zero. */
function word_to_limb(U256(w) : word) -> option(limb) =
    if w[255 .. 64] == sail_zeros(192) then {
        Some(w[63 .. 0])
    } else {
        None()
    }

function limb_to_word(value : limb) -> word = U256(sail_zero_extend(value, 256))

/* Embed an exact non-negative quantity in an EVM word. Callers establish the
 * protocol bound; the conversion itself does not narrow through a host-sized
 * integer first. */
/*! Embeds a natural number smaller than the word modulus. */
function word_of_nat(value : nat) -> word = {
    assert(value < 2 ^ 256);
    U256(get_slice_int(256, value, 0))
}

/*! Unsigned 256-bit less-than, defined by comparing the four 64-bit limbs
    from most significant down. */
function word_ult(U256(a) : word, U256(b) : word) -> bool =
    if a[255 .. 192] != b[255 .. 192] then {
        limb_ult(a[255 .. 192], b[255 .. 192])
    } else {
        if a[191 .. 128] != b[191 .. 128] then {
            limb_ult(a[191 .. 128], b[191 .. 128])
        } else {
            if a[127 .. 64] != b[127 .. 64] then {
                limb_ult(a[127 .. 64], b[127 .. 64])
            } else {
                limb_ult(a[63 .. 0], b[63 .. 0])
            }
        }
    }

function word_ule(a : word, b : word) -> bool = not_bool(word_ult(b, a))

/*! Returns the position above a word's most significant set bit. */
function word_bit_length(U256(value) : word) -> word_bit_count =
    if value[255 .. 192] != LIMB_ZERO then {
        192 + limb_bit_length(value[255 .. 192])
    } else {
        if value[191 .. 128] != LIMB_ZERO then {
            128 + limb_bit_length(value[191 .. 128])
        } else {
            if value[127 .. 64] != LIMB_ZERO then {
                64 + limb_bit_length(value[127 .. 64])
            } else {
                limb_bit_length(value[63 .. 0])
            }
        }
    }

/*! Adds words and reports unsigned overflow beyond 256 bits. */
function word_checked_add(a : word, b : word) -> option(word) = {
    let result = word_add(a, b);
    if word_ult(result, a) then {
        None()
    } else {
        Some(result)
    }
}

/* Checked multiplication by a host-sized quantity. Transaction gas and blob
 * counts use this rather than widening through Sail's mathematical integer. */
/*! Multiplies a word by one limb and reports overflow beyond 256 bits. */
function word_checked_mul_limb(value : word, factor : limb) -> option(word) = {
    var result : word = ZERO_WORD;
    var addend : word = value;
    var remaining : limb = factor;
    var valid : bool = true;
    foreach (i from 0 to 63) {
        if valid & ((remaining & LIMB_ONE) == LIMB_ONE) then {
            let added = word_checked_add(result, addend);
            match added {
                Some(value) => result = value,
                None() => valid = false,
            }
        };
        remaining = sail_shiftright(remaining, 1);
        if valid & (i < 63) & (remaining != LIMB_ZERO) then {
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

/* Multiplication modulo 2^256. Each step consumes one multiplier bit and all
 * intermediate values remain fixed-width words, which is exactly the EVM's
 * wrapping multiplication algebra. */
/*! Multiplies words modulo the 256-bit word modulus. */
function word_mul(a : word, b : word) -> word = {
    var result : word = WORD_ZERO;
    var addend : word = a;
    var remaining : word = b;
    foreach (_step from 0 to 255) {
        if word_bit(remaining, 0) == bitone then {
            result = word_add(result, addend)
        };
        addend = word_shift_left_one(addend);
        remaining = word_shift_right_one(remaining)
    };
    result
}

/* Unsigned fixed-width long division. The overflow bit records the 257th bit
 * of the trial remainder, avoiding any widening through Sail int. */
/*! Computes unsigned word quotient and remainder by bounded long division. */
function word_divmod(dividend : word, divisor : word) -> WordDivMod = {
    if word_is_zero(divisor) then {
        struct { quotient = WORD_ZERO, remainder = WORD_ZERO }
    } else {
        var quotient : word = WORD_ZERO;
        var remainder : word = WORD_ZERO;
        var remaining : word = dividend;
        foreach (_step from 0 to 255) {
            let incoming = if word_bit(remaining, 255) == bitone then { WORD_ONE } else { WORD_ZERO };
            let overflow = word_bit(remainder, 255) == bitone;
            remainder = word_or(word_shift_left_one(remainder), incoming);
            remaining = word_shift_left_one(remaining);
            quotient = word_shift_left_one(quotient);
            if overflow | word_ule(divisor, remainder) then {
                remainder = word_sub(remainder, divisor);
                quotient = word_or(quotient, WORD_ONE)
            }
        };
        struct { quotient = quotient, remainder = remainder }
    }
}

/* Modular addition for operands already reduced below modulus. The
 * subtraction form avoids overflowing a+b. */
/*! Adds reduced residues modulo a nonzero word modulus without overflow. */
function word_mod_add_reduced(a : word, b : word, modulus : word) -> word = {
    let threshold = word_sub(modulus, b);
    if word_ule(threshold, a) then {
        word_sub(a, threshold)
    } else {
        word_add(a, b)
    }
}

/*! Shifts a word left by a limb-sized amount, yielding zero past the width. */
function word_shift_left_limb(value : word, amount : limb) -> word = {
    var result : word = value;
    var remaining : limb = amount;
    foreach (_step from 0 to 255) {
        if remaining != LIMB_ZERO then {
            result = word_shift_left_one(result);
            remaining = sub_bits(remaining, LIMB_ONE)
        }
    };
    result
}

/*! Shifts a word right logically by a limb-sized amount. */
function word_shift_right_limb(value : word, amount : limb) -> word = {
    var result : word = value;
    var remaining : limb = amount;
    foreach (_step from 0 to 255) {
        if remaining != LIMB_ZERO then {
            result = word_shift_right_one(result);
            remaining = sub_bits(remaining, LIMB_ONE)
        }
    };
    result
}

/*! Shifts a two's-complement word right while extending its sign bit. */
function word_arithmetic_shift_right_limb(value : word, amount : limb) -> word = {
    var result : word = value;
    var remaining : limb = amount;
    let negative = word_bit(value, 255) == bitone;
    foreach (_step from 0 to 255) {
        if remaining != LIMB_ZERO then {
            result = word_shift_right_one(result);
            if negative then {
                result = word_or(result, WORD_SIGN_BIT)
            };
            remaining = sub_bits(remaining, LIMB_ONE)
        }
    };
    result
}

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
function alu_add(a : word, b : word) -> word = word_add(a, b) /* ADD */
function alu_sub(a : word, b : word) -> word = word_sub(a, b) /* SUB */
function alu_mul(a : word, b : word) -> word = word_mul(a, b) /* MUL */

/*! `DIV`: unsigned Euclidean division; division by zero yields `0`
    (YP Appendix H). */
function alu_div(a : word, b : word) -> word =
    /* DIV */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        word_divmod(a, b).quotient
    }

/*! `MOD`: unsigned modulus; a zero modulus yields `0`. */
function alu_mod(a : word, b : word) -> word =
    /* MOD */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        word_divmod(a, b).remainder
    }

/*! `SDIV`: signed division, truncating toward zero; division by zero
    yields `0`. */
function alu_sdiv(a : word, b : word) -> word = {
    /* SDIV */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        let quotient = word_divmod(word_abs(a), word_abs(b)).quotient;
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
        let remainder = word_divmod(word_abs(a), word_abs(b)).remainder;
        if word_bit(a, 255) == bitone then {
            word_negate(remainder)
        } else {
            remainder
        }
    }
}

/*! `ADDMOD`: `(a + b) mod n` over the unbounded integers (no intermediate
    2^256 reduction); a zero modulus yields `0`. */
function alu_addmod(a : word, b : word, n : word) -> word = {
    /* ADDMOD */
    if word_is_zero(n) then {
        WORD_ZERO
    } else {
        let a_reduced = word_divmod(a, n).remainder;
        let b_reduced = word_divmod(b, n).remainder;
        word_mod_add_reduced(a_reduced, b_reduced, n)
    }
}

/*! `MULMOD`: `(a * b) mod n` over the unbounded integers; a zero modulus
    yields `0`. */
function alu_mulmod(a : word, b : word, n : word) -> word = {
    /* MULMOD */
    if word_is_zero(n) then {
        WORD_ZERO
    } else {
        var result : word = WORD_ZERO;
        var addend : word = word_divmod(a, n).remainder;
        var remaining : word = b;
        foreach (_step from 0 to 255) {
            if word_bit(remaining, 0) == bitone then {
                result = word_mod_add_reduced(result, addend, n)
            };
            addend = word_mod_add_reduced(addend, addend, n);
            remaining = word_shift_right_one(remaining)
        };
        result
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
    match word_to_limb(byte_index) {
        Some(index) if limb_ult(index, 0x0000000000000020) => {
            let width = sail_shiftleft(add_bits(index, LIMB_ONE), 3);
            let sign_shift = sub_bits(width, LIMB_ONE);
            let sign_set = word_and(word_shift_right_limb(value, sign_shift), WORD_ONE) == WORD_ONE;
            let low_mask = word_sub(word_shift_left_limb(WORD_ONE, width), WORD_ONE);
            if sign_set then {
                word_or(word_and(value, low_mask), word_not(low_mask))
            } else {
                word_and(value, low_mask)
            }
        },
        _ => value,
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
    match word_to_limb(i) {
        Some(index) if limb_ult(index, 0x0000000000000020) => {
            let byte_offset = sub_bits(0x000000000000001f, index);
            let shift = sail_shiftleft(byte_offset, 3);
            U256(sail_zero_extend(word_low_byte(word_shift_right_limb(x, shift)), 256))
        },
        _ => WORD_ZERO,
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
    if not_bool(word_ult(shift_amt, limb_to_word(0x0000000000000100))) then {
        WORD_ZERO
    } else {
        word_shift_left_limb(v, word_limb_0(shift_amt))
    }

/*! `SHR`: logical right shift; amounts ≥ 256 yield `0`. */
function alu_shr(shift_amt : word, v : word) -> word =
    /* SHR */
    if not_bool(word_ult(shift_amt, limb_to_word(0x0000000000000100))) then {
        WORD_ZERO
    } else {
        word_shift_right_limb(v, word_limb_0(shift_amt))
    }

/* SAR: arithmetic (sign-propagating) right shift. */
/*! `SAR`: arithmetic (sign-propagating) right shift, via floored signed
    division. */
function alu_sar(shift_amt : word, v : word) -> word = {
    /* SAR */
    if not_bool(word_ult(shift_amt, limb_to_word(0x0000000000000100))) then {
        if word_bit(v, 255) == bitone then { WORD_ALL_ONES } else { WORD_ZERO }
    } else {
        word_arithmetic_shift_right_limb(v, word_limb_0(shift_amt))
    }
}

/*! `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). */
function alu_clz(x : word) -> word = {
    var count : word_bit_count = 0;
    var found : bool = false;
    foreach (i from 0 to 255) {
        let biinput_index = 255 - i; /* scan from MSB down */
        if not_bool(found) then {
            if word_bit(x, biinput_index) == bitzero then {
                count = word_bit_count_increment(count)
            } else {
                found = true
            }
        }
    };
    limb_to_word(get_slice_int(64, count, 0))
}
