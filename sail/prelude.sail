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
$include <smt.sail>

/*md
## Types
*/

/*! The EVM 256-bit machine word (YP §9.1). */
type word    = bits(256)

/*! A 20-byte account address (YP §4.1). */
type address = bits(160)

/*! A KECCAK-256 / storage-key sized digest. */
type hash    = bits(256)

/*! An 8-bit byte. */
type byte    = bits(8)

/*! A 64-bit unsigned quantity (nonces, gas fields, list counts). */
type u64     = bits(64)

/* the all-zero word / address constants */
let ZERO_WORD : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let ZERO_ADDR : address = 0x0000000000000000000000000000000000000000

/*! Gas is a non-negative counter, bounded to `range(0, 2^63 - 1)` =
    `INT64_MAX`. The bound must exceed the largest tx/block gas limit a
    fixture can set: the EEST state-test suite uses gas limits up to
    `0x7fe0000000000001` (~9.21e18, just under 2^63) and a block gas limit
    up to `INT64_MAX` itself, so a smaller cap (e.g. 2^62) would saturate
    the initial frame gas to half its true value and corrupt the whole
    63/64 call-gas cascade (EIP-150). Saturating to this bound ([to_gas][])
    still preserves charge-then-out-of-gas semantics for
    attacker-controlled sizes: a clamped astronomic charge exhausts any
    frame, just as the true charge would. `INT64_MAX` keeps the counter in
    a machine int. */
type gas = range(0, 9223372036854775807)

/*! Saturates an integer into the [gas][type-gas] range. A typed range
    guard for the generated code; see [gas][type-gas] for why saturation
    preserves semantics. */
function to_gas(x : int) -> gas =
    if x < 0 then { 0 } else { if x <= 9223372036854775807 then { x } else { 9223372036854775807 } }

/*! Gas subtraction used by [charge][]. The caller proves `a >= b >= 0`;
    [to_gas][] is therefore only a typed range guard for the generated
    code. */
function gas_sub(a : gas, b : gas) -> gas = to_gas(a - b)

/*! Signed 64-bit counter (a machine int in the generated code). The
    gas-refund counter is the one genuinely signed quantity in the model
    (EIP-2200/EIP-3529 deltas un-clear as well as clear); its magnitude is
    gas-bounded, so the clamp in [to_i64][] is a typed range guard, never
    semantics. */
type i64 = range(- 9223372036854775808, 9223372036854775807)

/*! Clamps an integer into the [i64][type-i64] range; a typed range guard,
    never semantics. */
function to_i64(x : int) -> i64 =
    if x < -9223372036854775808 then {
        -9223372036854775808
    } else {
        if x <= 9223372036854775807 then { x } else { 9223372036854775807 }
    }

let WORD_BYTES : int = 32
let WORD_BITS : int = 256

/*! 2^256, the EVM word modulus. */
let TWO_256 : int = 115792089237316195423570985008687907853269984665640564039457584007913129639936

/*md
## Helpers

### Integer ↔ word conversion
*/

/*! The low 256 bits of a (possibly huge or negative) integer: all word
    arithmetic reduces modulo 2^256 through this function. */
function word_of_int(n : int) -> word = get_slice_int(256, n, 0)

let WORD_ZERO : word = 0x0000000000000000000000000000000000000000000000000000000000000000
let WORD_ONE : word = 0x0000000000000000000000000000000000000000000000000000000000000001

/*! `1` if the condition holds, else `0` — the EVM boolean convention. */
function word_of_bool(b : bool) -> word = if b then { WORD_ONE } else { WORD_ZERO }

function word_is_zero(w : word) -> bool = w == WORD_ZERO
function word_nonzero(w : word) -> bool = not_bool(word_is_zero(w))

function word64_ult(a : bits(64), b : bits(64)) -> bool = unsigned(a) < unsigned(b)

/*! Unsigned 256-bit less-than, defined by comparing the four 64-bit limbs
    from most significant down. */
function word_ult(a : word, b : word) -> bool =
    if a[255 .. 192] != b[255 .. 192] then {
        word64_ult(a[255 .. 192], b[255 .. 192])
    } else {
        if a[191 .. 128] != b[191 .. 128] then {
            word64_ult(a[191 .. 128], b[191 .. 128])
        } else {
            if a[127 .. 64] != b[127 .. 64] then {
                word64_ult(a[127 .. 64], b[127 .. 64])
            } else {
                word64_ult(a[63 .. 0], b[63 .. 0])
            }
        }
    }

/*! Signed (two's-complement) 256-bit less-than: sign bits decide when they
    differ, otherwise the unsigned order applies. */
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

/* pow2(n) = 2^n is a typed builtin from arith.sail (forall 'n. int(2^'n)). */

/*md
### List utilities

Sail lists have no builtin length/append/index; these are used throughout
for byte buffers (calldata, returndata, memory, code) and log lists.
*/

/*! The length of a list, by iterative traversal. */
function list_len forall ('a : Type). (xs : list('a)) -> int = {
    var rest : list('a) = xs;
    var length : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            _ :: tail => {
                length = length + 1;
                rest = tail
            },
        }
    };
    length
}

/*! Reverses `xs` onto `acc`; the accumulator form underlying
    [list_reverse][] and [list_append][]. */
function rev_onto forall ('a : Type). (xs : list('a), acc : list('a)) -> list('a) = {
    var rest : list('a) = xs;
    var result : list('a) = acc;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            head :: tail => {
                result = head :: result;
                rest = tail
            },
        }
    };
    result
}

function list_reverse forall ('a : Type). (xs : list('a)) -> list('a) = rev_onto(xs, [||])

function list_append forall ('a : Type). (xs : list('a), ys : list('a)) -> list('a) =
    rev_onto(list_reverse(xs), ys)

/* list concatenation operator (sail-riscv operator-overload style; Sail's
 * built-in @ is bitvector concat and is not overloadable) */
infixl 6 ++
overload operator ++ = {list_append}

/*! Indexes a materialized byte list, returning `0x00` out of range. Random
    access is deliberately not exposed as a generic list operation:
    source-backed input, code, calldata, and memory use O(1) `ByteSlice`
    reads instead. */
function bytes_get(data : list(byte), i : int) -> byte =
    if i < 0 then {
        0x00
    } else {
        var rest : list(byte) = data;
        var remaining : int = i;
        var result : byte = 0x00;
        var more : bool = true;
        while more do {
            match rest {
                [||] => more = false,
                head :: tail => if remaining == 0 then {
                    result = head;
                    more = false
                } else {
                    remaining = remaining - 1;
                    rest = tail
                },
            }
        };
        result
    }

/*! The zero-padded slice `data[off .. off+len)`. O(off + len): drop `off`
    then take `len` in a single left-to-right pass. Past the end it pads
    with `0x00`, matching [bytes_get][]'s default. */
function bytes_slice(data : list(byte), off : int, len : int) -> list(byte) = {
    var rest : list(byte) = data;
    var i : int = 0;
    while i < off do {
        match rest {
            _ :: t => rest = t,
            [||] => (),
        };
        i = i + 1
    };
    var out : list(byte) = [||];
    var j : int = 0;
    while j < len do {
        match rest {
            h :: t => {
                out = h :: out;
                rest = t
            },
            [||] => out = 0x00 :: out,
        };
        j = j + 1
    };
    list_reverse(out)
}

/*md
### Address ↔ word conversion
*/

/*! Truncates a word to its low 160 bits, the address interpretation used
    by `CALL`-family and `CREATE` operands (YP §7). */
function word_to_address(w : word) -> address = w[159 .. 0]

/*! Zero-extends an address to a word, as pushed by `ADDRESS`, `CALLER`,
    `ORIGIN`, and `COINBASE`. */
function address_to_word(a : address) -> word = sail_zero_extend(a, 256)

/*md
## The 256-bit ALU

The word operations behind the arithmetic, comparison, bitwise, and shift
opcodes (YP Appendix H). Names mirror the EVM opcodes; all results are
reduced modulo 2^256.

### Arithmetic
*/

function alu_add(a : word, b : word) -> word = a + b /* ADD */

function alu_sub(a : word, b : word) -> word = sub_bits(a, b) /* SUB */

function alu_mul(a : word, b : word) -> word =
    /* MUL */
    word_of_int(unsigned(a) * unsigned(b))

/*! `DIV`: unsigned Euclidean division; division by zero yields `0`
    (YP Appendix H). */
function alu_div(a : word, b : word) -> word =
    /* DIV */
    if word_is_zero(b) then {
        word_of_int(0)
    } else {
        word_of_int(ediv_int(unsigned(a), unsigned(b)))
    }

/*! `MOD`: unsigned modulus; a zero modulus yields `0`. */
function alu_mod(a : word, b : word) -> word =
    /* MOD */
    if word_is_zero(b) then {
        word_of_int(0)
    } else {
        word_of_int(emod_int(unsigned(a), unsigned(b)))
    }

/*! `SDIV`: signed division, truncating toward zero; division by zero
    yields `0`. */
function alu_sdiv(a : word, b : word) -> word =
    /* SDIV */
    if word_is_zero(b) then {
        word_of_int(0)
    } else {
        word_of_int(tdiv_int(signed(a), signed(b)))
    }

/*! `SMOD`: signed remainder, with the sign of the dividend; a zero modulus
    yields `0`. */
function alu_smod(a : word, b : word) -> word =
    /* SMOD */
    if word_is_zero(b) then {
        word_of_int(0)
    } else {
        word_of_int(tmod_int(signed(a), signed(b)))
    }

/*! `ADDMOD`: `(a + b) mod n` over the unbounded integers (no intermediate
    2^256 reduction); a zero modulus yields `0`. */
function alu_addmod(a : word, b : word, n : word) -> word =
    /* ADDMOD */
    if word_is_zero(n) then {
        word_of_int(0)
    } else {
        word_of_int(emod_int(unsigned(a) + unsigned(b), unsigned(n)))
    }

/*! `MULMOD`: `(a * b) mod n` over the unbounded integers; a zero modulus
    yields `0`. */
function alu_mulmod(a : word, b : word, n : word) -> word =
    /* MULMOD */
    if word_is_zero(n) then {
        word_of_int(0)
    } else {
        word_of_int(emod_int(unsigned(a) * unsigned(b), unsigned(n)))
    }

/*! `EXP` via square-and-multiply over the 256 exponent bits, reduced
    modulo 2^256 at every step. */
function alu_exp(base : word, exponent : word) -> word = {
    var result : int = 1;
    var b : int = unsigned(base);
    var e : int = unsigned(exponent);
    foreach (k from 0 to 255) {
        let _ = k;
        if emod_int(e, 2) == 1 then {
            result = emod_int(result * b, TWO_256)
        };
        b = emod_int(b * b, TWO_256);
        e = ediv_int(e, 2)
    };
    word_of_int(result)
}

/*! `SIGNEXTEND(byte_index, value)`: sign-extends `value` from byte
    `byte_index` (0 = least significant); indices ≥ 31 leave the value
    unchanged. */
function alu_signextend(byte_index : word, value : word) -> word = {
    let bi = unsigned(byte_index);
    if bi >= 31 then {
        value
    } else {
        let bit_pos = bi * 8 + 7; /* sign bit position */
        let sign_set = (unsigned(value) >= pow2(bit_pos)) & (emod_int(ediv_int(unsigned(value), pow2(bit_pos)), 2) == 1);
        let low_mask = pow2(bit_pos + 1) - 1; /* keep bits [0 .. bit_pos]        */
        let low = emod_int(unsigned(value), pow2(bit_pos + 1));
        if sign_set then {
            word_of_int(low + (TWO_256 - 1) - low_mask)
            /* set high bits to 1  */
        } else {
            word_of_int(low)
        }
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

function alu_and(a : word, b : word) -> word = a & b
function alu_or(a : word, b : word) -> word = a | b
function alu_xor(a : word, b : word) -> word = xor_vec(a, b)
function alu_not(a : word) -> word = not_vec(a)

/*! `BYTE(i, x)`: the `i`-th most-significant byte of `x` (0 = MSB);
    indices ≥ 32 yield `0`. */
function alu_byte(i : word, x : word) -> word = {
    let idx = unsigned(i);
    if idx >= 32 then {
        word_of_int(0)
    } else {
        let shift = (31 - idx) * 8;
        word_of_int(emod_int(ediv_int(unsigned(x), pow2(shift)), 256))
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
    if unsigned(shift_amt) >= 256 then {
        word_of_int(0)
    } else {
        sail_shiftleft(v, unsigned(shift_amt))
    }

/*! `SHR`: logical right shift; amounts ≥ 256 yield `0`. */
function alu_shr(shift_amt : word, v : word) -> word =
    /* SHR */
    if unsigned(shift_amt) >= 256 then {
        word_of_int(0)
    } else {
        sail_shiftright(v, unsigned(shift_amt))
    }

/*! `SAR`: arithmetic (sign-propagating) right shift, via floored signed
    division. */
function alu_sar(shift_amt : word, v : word) -> word = {
    /* SAR */
    let s = unsigned(shift_amt);
    let s2 = if s > 256 then { 256 } else { s };
    word_of_int(fdiv_int(signed(v), pow2(s2)))
}

/*! `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). */
function alu_clz(x : word) -> word = {
    var count : int = 0;
    var found : bool = false;
    foreach (i from 0 to 255) {
        let biinput_index = 255 - i; /* scan from MSB down */
        if not_bool(found) then {
            if x[biinput_index] == bitzero then {
                count = count + 1
            } else {
                found = true
            }
        }
    };
    word_of_int(count)
}
