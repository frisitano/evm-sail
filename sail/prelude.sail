/* ======================================================================== */
/* PRELUDE: the value model and the full 256-bit word ALU                   */
/*                                                                          */
/* The arithmetic foundation of the specification. The EVM operates on      */
/* 256-bit words (YP section 9.1); this file defines that word type and     */
/* gives every word operation its REAL two's-complement / unsigned          */
/* semantics over bits(256). Only KECCAK-256 and the precompile crypto      */
/* at the host boundary are opaque primitives; everything else is fully     */
/* defined here.                                                            */
/*                                                                          */
/* EVM words stay in fixed-width bits. Runtime quantities use mathematical  */
/* integers; conversion to a word occurs only where the EVM exposes such a  */
/* quantity on its 256-bit operand stack.                                   */
/* ======================================================================== */

default Order dec

$include <prelude.sail>

/* ------------------------------- types ---------------------------------- */

type word    = bits(256)

/* EVM 256-bit machine word                       */
type address = bits(160)

/* 20-byte account address                        */
type hash    = bits(256)

/* keccak-256 / storage-key sized digest          */
type byte    = bits(8)

/* 8-bit byte                                     */
type limb    = bits(64)

struct LimbDivMod = {
    quotient : limb,
    remainder : limb,
}

struct WordDivMod = {
    quotient : word,
    remainder : word,
}

/* the all-zero word / address constants */
let ZERO_WORD : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let ZERO_ADDR : address = 0x0000000000000000000000000000000000000000

let WORD_ZERO : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let WORD_ONE : word = 0x0000000000000000000000000000000000000000000000000000000000000001
let WORD_ALL_ONES : word = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
let WORD_SIGN_BIT : word = 0x8000000000000000000000000000000000000000000000000000000000000000

function word_of_bool(b : bool) -> word = if b then { WORD_ONE } else { WORD_ZERO }
function word_is_zero(w : word) -> bool = w == WORD_ZERO
function word_nonzero(w : word) -> bool = not_bool(word_is_zero(w))

let LIMB_ZERO : limb = 0x0000000000000000
let LIMB_ONE : limb = 0x0000000000000001
let LIMB_MAX : limb = 0xFFFFFFFFFFFFFFFF

/* Unsigned comparison via the borrow bit of a - b. This stays entirely in the
 * fixed-width algebra: unlike unsigned(bits(64)), the C backend emits native
 * uint64_t operations and never constructs a sail_int. */
function limb_ult(a : limb, b : limb) -> bool = {
    let difference = sub_bits(a, b);
    let borrow = or_vec(and_vec(not_vec(a), b), and_vec(not_vec(xor_vec(a, b)), difference));
    and_vec(borrow, 0x8000000000000000) != LIMB_ZERO
}

function limb_ule(a : limb, b : limb) -> bool = not_bool(limb_ult(b, a))

/* Fixed-width checked arithmetic. Plain bits(64) addition and subtraction are
 * modular; callers that model quantities use these operations so wraparound
 * is represented explicitly and mapped to the operation's EVM behaviour. */
function limb_checked_add(a : limb, b : limb) -> option(limb) = {
    let result = add_bits(a, b);
    if limb_ult(result, a) then {
        None()
    } else {
        Some(result)
    }
}

function limb_checked_sub(a : limb, b : limb) -> option(limb) =
    if limb_ult(a, b) then {
        None()
    } else {
        Some(sub_bits(a, b))
    }

/* Fixed-width checked multiplication. */
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

type limb_bit_count = range(0, 64)
type word_bit_count = range(0, 256)

function word_bit_count_increment(value : word_bit_count) -> word_bit_count =
    if value < 256 then {
        value + 1
    } else {
        assert(false);
        0
    }

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
function word_to_limb(w : word) -> option(limb) =
    if w[255 .. 64] == sail_zeros(192) then {
        Some(w[63 .. 0])
    } else {
        None()
    }

function limb_to_word(value : limb) -> word = sail_zero_extend(value, 256)

/* Embed an exact non-negative quantity in an EVM word. Callers establish the
 * protocol bound; the conversion itself does not narrow through a host-sized
 * integer first. */
function word_of_nat(value : nat) -> word = {
    assert(value < 2 ^ 256);
    get_slice_int(256, value, 0)
}

function word_ult(a : word, b : word) -> bool =
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

function word_bit_length(value : word) -> word_bit_count =
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

function word_checked_add(a : word, b : word) -> option(word) = {
    let result = add_bits(a, b);
    if word_ult(result, a) then {
        None()
    } else {
        Some(result)
    }
}

/* Checked multiplication by a host-sized quantity. Transaction gas and blob
 * counts use this rather than widening through Sail's mathematical integer. */
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
function word_mul(a : word, b : word) -> word = {
    var result : word = WORD_ZERO;
    var addend : word = a;
    var remaining : word = b;
    foreach (_step from 0 to 255) {
        if remaining[0] == bitone then {
            result = add_bits(result, addend)
        };
        addend = sail_shiftleft(addend, 1);
        remaining = sail_shiftright(remaining, 1)
    };
    result
}

/* Unsigned fixed-width long division. The overflow bit records the 257th bit
 * of the trial remainder, avoiding any widening through Sail int. */
function word_divmod(dividend : word, divisor : word) -> WordDivMod = {
    if word_is_zero(divisor) then {
        struct { quotient = WORD_ZERO, remainder = WORD_ZERO }
    } else {
        var quotient : word = WORD_ZERO;
        var remainder : word = WORD_ZERO;
        var remaining : word = dividend;
        foreach (_step from 0 to 255) {
            let incoming = if remaining[255] == bitone then { WORD_ONE } else { WORD_ZERO };
            let overflow = remainder[255] == bitone;
            remainder = or_vec(sail_shiftleft(remainder, 1), incoming);
            remaining = sail_shiftleft(remaining, 1);
            quotient = sail_shiftleft(quotient, 1);
            if overflow | word_ule(divisor, remainder) then {
                remainder = sub_bits(remainder, divisor);
                quotient = or_vec(quotient, WORD_ONE)
            }
        };
        struct { quotient = quotient, remainder = remainder }
    }
}

/* Modular addition for operands already reduced below modulus. The
 * subtraction form avoids overflowing a+b. */
function word_mod_add_reduced(a : word, b : word, modulus : word) -> word = {
    let threshold = sub_bits(modulus, b);
    if word_ule(threshold, a) then {
        sub_bits(a, threshold)
    } else {
        add_bits(a, b)
    }
}

function word_shift_left_limb(value : word, amount : limb) -> word = {
    var result : word = value;
    var remaining : limb = amount;
    foreach (_step from 0 to 255) {
        if remaining != LIMB_ZERO then {
            result = sail_shiftleft(result, 1);
            remaining = sub_bits(remaining, LIMB_ONE)
        }
    };
    result
}

function word_shift_right_limb(value : word, amount : limb) -> word = {
    var result : word = value;
    var remaining : limb = amount;
    foreach (_step from 0 to 255) {
        if remaining != LIMB_ZERO then {
            result = sail_shiftright(result, 1);
            remaining = sub_bits(remaining, LIMB_ONE)
        }
    };
    result
}

function word_arithmetic_shift_right_limb(value : word, amount : limb) -> word = {
    var result : word = value;
    var remaining : limb = amount;
    let negative = value[255] == bitone;
    foreach (_step from 0 to 255) {
        if remaining != LIMB_ZERO then {
            result = sail_shiftright(result, 1);
            if negative then {
                result = or_vec(result, WORD_SIGN_BIT)
            };
            remaining = sub_bits(remaining, LIMB_ONE)
        }
    };
    result
}

function word_negate(value : word) -> word = sub_bits(WORD_ZERO, value)

function word_abs(value : word) -> word =
    if value[255] == bitone then {
        word_negate(value)
    } else {
        value
    }

function word_slt(a : word, b : word) -> bool = {
    let a_neg = a[255] == bitone;
    let b_neg = b[255] == bitone;
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

/* ------------------- address <-> word interconversion ------------------- */

function word_to_address(w : word) -> address = w[159 .. 0]
function address_to_word(a : address) -> word = sail_zero_extend(a, 256)

/* ============================ the 256-bit ALU =========================== */
/* Names mirror the EVM opcodes. All results are reduced mod 2^256.          */

/* --- arithmetic --- */
function alu_add(a : word, b : word) -> word = add_bits(a, b) /* ADD */
function alu_sub(a : word, b : word) -> word = sub_bits(a, b) /* SUB */
function alu_mul(a : word, b : word) -> word = word_mul(a, b) /* MUL */

function alu_div(a : word, b : word) -> word =
    /* DIV */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        word_divmod(a, b).quotient
    }

function alu_mod(a : word, b : word) -> word =
    /* MOD */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        word_divmod(a, b).remainder
    }

function alu_sdiv(a : word, b : word) -> word = {
    /* SDIV */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        let quotient = word_divmod(word_abs(a), word_abs(b)).quotient;
        if (a[255] == bitone) != (b[255] == bitone) then {
            word_negate(quotient)
        } else {
            quotient
        }
    }
}

function alu_smod(a : word, b : word) -> word = {
    /* SMOD */
    if word_is_zero(b) then {
        WORD_ZERO
    } else {
        let remainder = word_divmod(word_abs(a), word_abs(b)).remainder;
        if a[255] == bitone then {
            word_negate(remainder)
        } else {
            remainder
        }
    }
}

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

function alu_mulmod(a : word, b : word, n : word) -> word = {
    /* MULMOD */
    if word_is_zero(n) then {
        WORD_ZERO
    } else {
        var result : word = WORD_ZERO;
        var addend : word = word_divmod(a, n).remainder;
        var remaining : word = b;
        foreach (_step from 0 to 255) {
            if remaining[0] == bitone then {
                result = word_mod_add_reduced(result, addend, n)
            };
            addend = word_mod_add_reduced(addend, addend, n);
            remaining = sail_shiftright(remaining, 1)
        };
        result
    }
}

/* EXP via square-and-multiply over the 256 exponent bits. */
function alu_exp(base : word, exponent : word) -> word = {
    var result : word = WORD_ONE;
    var b : word = base;
    var e : word = exponent;
    foreach (_step from 0 to 255) {
        if e[0] == bitone then {
            result = word_mul(result, b)
        };
        b = word_mul(b, b);
        e = sail_shiftright(e, 1)
    };
    result
}

/* SIGNEXTEND(byte_index, value): sign-extend `value` from byte `byte_index`. */
function alu_signextend(byte_index : word, value : word) -> word = {
    match word_to_limb(byte_index) {
        Some(index) if limb_ult(index, 0x0000000000000020) => {
            let width = sail_shiftleft(add_bits(index, LIMB_ONE), 3);
            let sign_shift = sub_bits(width, LIMB_ONE);
            let sign_set = and_vec(word_shift_right_limb(value, sign_shift), WORD_ONE) == WORD_ONE;
            let low_mask = sub_bits(word_shift_left_limb(WORD_ONE, width), WORD_ONE);
            if sign_set then {
                or_vec(and_vec(value, low_mask), not_vec(low_mask))
            } else {
                and_vec(value, low_mask)
            }
        },
        _ => value,
    }
}

/* --- comparison (push 1/0) --- */
function alu_lt(a : word, b : word) -> word = word_of_bool(word_ult(a, b))
function alu_gt(a : word, b : word) -> word = word_of_bool(word_ult(b, a))
function alu_slt(a : word, b : word) -> word = word_of_bool(word_slt(a, b))
function alu_sgt(a : word, b : word) -> word = word_of_bool(word_slt(b, a))
function alu_eq(a : word, b : word) -> word = word_of_bool(a == b)
function alu_iszero(a : word) -> word = word_of_bool(word_is_zero(a))

/* --- bitwise --- */
function alu_and(a : word, b : word) -> word = a & b
function alu_or(a : word, b : word) -> word = a | b
function alu_xor(a : word, b : word) -> word = xor_vec(a, b)
function alu_not(a : word) -> word = not_vec(a)

/* BYTE(i, x): the i-th most-significant byte of x (0 = MSB), else 0. */
function alu_byte(i : word, x : word) -> word = {
    match word_to_limb(i) {
        Some(index) if limb_ult(index, 0x0000000000000020) => {
            let byte_offset = sub_bits(0x000000000000001f, index);
            let shift = sail_shiftleft(byte_offset, 3);
            sail_zero_extend(word_shift_right_limb(x, shift)[7 .. 0], 256)
        },
        _ => WORD_ZERO,
    }
}

/* --- shifts (shift amount is the TOP stack item per EVM SHL/SHR/SAR) --- */
function alu_shl(shift_amt : word, v : word) -> word =
    /* SHL */
    if not_bool(word_ult(shift_amt, 0x0000000000000000000000000000000000000000000000000000000000000100)) then {
        WORD_ZERO
    } else {
        word_shift_left_limb(v, shift_amt[63 .. 0])
    }

function alu_shr(shift_amt : word, v : word) -> word =
    /* SHR */
    if not_bool(word_ult(shift_amt, 0x0000000000000000000000000000000000000000000000000000000000000100)) then {
        WORD_ZERO
    } else {
        word_shift_right_limb(v, shift_amt[63 .. 0])
    }

/* SAR: arithmetic (sign-propagating) right shift. */
function alu_sar(shift_amt : word, v : word) -> word = {
    /* SAR */
    if not_bool(word_ult(shift_amt, 0x0000000000000000000000000000000000000000000000000000000000000100)) then {
        if v[255] == bitone then { WORD_ALL_ONES } else { WORD_ZERO }
    } else {
        word_arithmetic_shift_right_limb(v, shift_amt[63 .. 0])
    }
}

/* CLZ: count leading zero bits of a 256-bit word (EIP-7939). */
function alu_clz(x : word) -> word = {
    var count : word_bit_count = 0;
    var found : bool = false;
    foreach (i from 0 to 255) {
        let biinput_index = 255 - i; /* scan from MSB down */
        if not_bool(found) then {
            if x[biinput_index] == bitzero then {
                count = word_bit_count_increment(count)
            } else {
                found = true
            }
        }
    };
    word_of_nat(count)
}
