/* ======================================================================== */
/* evm-sail prelude: value model + the full 256-bit word ALU                    */
/*                                                                           */
/* This is the arithmetic foundation of the specification. Every EVM word        */
/* operation is given REAL two's-complement / unsigned semantics over        */
/* bits(256); only KECCAK-256 (and the precompile crypto, declared           */
/* elsewhere) is an opaque primitive. The operators follow the Sail 0.20.1   */
/* base prelude: int->bits is get_slice_int; integer div/mod come from       */
/* <smt.sail> (ediv/emod) and arith.sail (tdiv/tmod/fdiv); subtraction on    */
/* bits is sub_bits; xor/and/or/not are the *_vec builtins.                  */
/* ======================================================================== */

default Order dec

$include <prelude.sail>
$include <smt.sail>

/* ------------------------------- types ---------------------------------- */

type word    = bits(256)   /* EVM 256-bit machine word                       */
type address = bits(160)   /* 20-byte account address                        */
type hash    = bits(256)   /* keccak-256 / storage-key sized digest          */
type byte    = bits(8)

/* Gas is a non-negative integer counter (Rust u64 in the impl). */
/* Gas is a 64-bit-representable quantity (Sail compiles range types that fit
 * int64 to machine integers -- the gas counter and schedule run native). The
 * cap 2^62 exceeds any real gas limit, so SATURATING at it preserves the
 * charge-then-out-of-gas semantics for attacker-controlled sizes (a clamped
 * astronomic charge still exhausts any frame). */
type gas = range(0, 4611686018427387904)
function to_gas(x : int) -> gas =
  if x < 0 then 0
  else if x <= 4611686018427387904 then x
  else 4611686018427387904

let WORD_BYTES  : int = 32
let WORD_BITS   : int = 256
/* 2^256, the EVM word modulus. */
let TWO_256 : int = 115792089237316195423570985008687907853269984665640564039457584007913129639936

/* ------------------------- int <-> word helpers ------------------------- */

/* Low 256 bits of a (possibly huge / negative) integer. */
function word_of_int(n : int) -> word = get_slice_int(256, n, 0)

/* Unsigned / signed interpretation of a word. */
function uint(w : word) -> int = unsigned(w)
function sint(w : word) -> int = signed(w)

function word_of_bool(b : bool) -> word = if b then word_of_int(1) else word_of_int(0)
function word_is_zero(w : word) -> bool = unsigned(w) == 0
function word_nonzero(w : word) -> bool = unsigned(w) != 0

/* pow2(n) = 2^n is a typed builtin from arith.sail (forall 'n. int(2^'n)). */

/* ----------------------- generic list utilities ------------------------ */
/* Sail lists have no builtin length/append/index; these are used throughout
 * for byte buffers (calldata/returndata/memory/code) and log lists. */

function list_len forall ('a : Type). (xs : list('a)) -> int =
  match xs { [||] => 0, _ :: t => 1 + list_len(t) }

function list_append forall ('a : Type). (xs : list('a), ys : list('a)) -> list('a) =
  match xs { [||] => ys, h :: t => h :: list_append(t, ys) }

function rev_onto forall ('a : Type). (xs : list('a), acc : list('a)) -> list('a) =
  match xs { [||] => acc, h :: t => rev_onto(t, h :: acc) }
function list_reverse forall ('a : Type). (xs : list('a)) -> list('a) = rev_onto(xs, [||])

/* i-th element (0-based) or `dflt` if out of range. */
function list_nth forall ('a : Type). (xs : list('a), i : int, dflt : 'a) -> 'a =
  match xs { [||] => dflt, h :: t => if i <= 0 then h else list_nth(t, i - 1, dflt) }

/* set the i-th element (0-based); out-of-range leaves the list unchanged. */
function list_set forall ('a : Type). (xs : list('a), i : int, v : 'a) -> list('a) =
  match xs { [||] => [||], h :: t => if i <= 0 then v :: t else h :: list_set(t, i - 1, v) }

/* byte from a buffer at index i, zero outside [0, len). */
function bytes_get(data : list(byte), i : int) -> byte =
  if i < 0 then 0x00 else list_nth(data, i, 0x00)

/* read a big-endian 32-byte word from a buffer at byte offset `off`, zero-padded. */
function bytes_load_word(data : list(byte), off : int) -> word = {
  var acc : word = word_of_int(0);
  foreach (k from 0 to 31) {
    acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(bytes_get(data, off + k), 256))
  };
  acc
}

/* zero-padded slice data[off .. off+len). */
/* O(off + len): drop `off` then take `len` in a single left-to-right pass
 * (was O(len^2 + len*off) via list_append + per-element list_nth). Past the end
 * pads with 0x00, matching bytes_get's default. */
function bytes_slice(data : list(byte), off : int, len : int) -> list(byte) = {
  var rest : list(byte) = data;
  var i : int = 0;
  while i < off do { match rest { _ :: t => rest = t, [||] => () }; i = i + 1 };
  var out : list(byte) = [||];
  var j : int = 0;
  while j < len do {
    match rest { h :: t => { out = h :: out; rest = t }, [||] => out = 0x00 :: out };
    j = j + 1
  };
  list_reverse(out)
}

/* ------------------- address <-> word interconversion ------------------- */

function word_to_address(w : word) -> address = w[159 .. 0]
function address_to_word(a : address) -> word = sail_zero_extend(a, 256)

/* ============================ the 256-bit ALU =========================== */
/* Names mirror the EVM opcodes. All results are reduced mod 2^256.          */

/* --- arithmetic --- */
function alu_add(a : word, b : word) -> word = a + b                 /* ADD */
function alu_sub(a : word, b : word) -> word = sub_bits(a, b)        /* SUB */
function alu_mul(a : word, b : word) -> word =                      /* MUL */
  word_of_int(unsigned(a) * unsigned(b))

function alu_div(a : word, b : word) -> word =                      /* DIV */
  if word_is_zero(b) then word_of_int(0)
  else word_of_int(ediv_int(unsigned(a), unsigned(b)))

function alu_mod(a : word, b : word) -> word =                      /* MOD */
  if word_is_zero(b) then word_of_int(0)
  else word_of_int(emod_int(unsigned(a), unsigned(b)))

function alu_sdiv(a : word, b : word) -> word =                     /* SDIV */
  if word_is_zero(b) then word_of_int(0)
  else word_of_int(tdiv_int(signed(a), signed(b)))

function alu_smod(a : word, b : word) -> word =                     /* SMOD */
  if word_is_zero(b) then word_of_int(0)
  else word_of_int(tmod_int(signed(a), signed(b)))

function alu_addmod(a : word, b : word, n : word) -> word =         /* ADDMOD */
  if word_is_zero(n) then word_of_int(0)
  else word_of_int(emod_int(unsigned(a) + unsigned(b), unsigned(n)))

function alu_mulmod(a : word, b : word, n : word) -> word =         /* MULMOD */
  if word_is_zero(n) then word_of_int(0)
  else word_of_int(emod_int(unsigned(a) * unsigned(b), unsigned(n)))

/* EXP via square-and-multiply over the 256 exponent bits. */
function alu_exp(base : word, exponent : word) -> word = {
  var result : int = 1;
  var b : int = unsigned(base);
  var e : int = unsigned(exponent);
  foreach (k from 0 to 255) {
    let _ = k;
    if emod_int(e, 2) == 1 then result = emod_int(result * b, TWO_256) else ();
    b = emod_int(b * b, TWO_256);
    e = ediv_int(e, 2)
  };
  word_of_int(result)
}

/* SIGNEXTEND(byte_index, value): sign-extend `value` from byte `byte_index`. */
function alu_signextend(byte_index : word, value : word) -> word = {
  let bi = unsigned(byte_index);
  if bi >= 31 then value
  else {
    let bit_pos = bi * 8 + 7;             /* sign bit position */
    let sign_set = (unsigned(value) >= pow2(bit_pos))
                   & (emod_int(ediv_int(unsigned(value), pow2(bit_pos)), 2) == 1);
    let low_mask = pow2(bit_pos + 1) - 1; /* keep bits [0 .. bit_pos]        */
    let low = emod_int(unsigned(value), pow2(bit_pos + 1));
    if sign_set
    then word_of_int(low + (TWO_256 - 1) - low_mask)  /* set high bits to 1  */
    else word_of_int(low)
  }
}

/* --- comparison (push 1/0) --- */
function alu_lt(a : word, b : word)  -> word = word_of_bool(unsigned(a) <  unsigned(b))
function alu_gt(a : word, b : word)  -> word = word_of_bool(unsigned(a) >  unsigned(b))
function alu_slt(a : word, b : word) -> word = word_of_bool(signed(a)   <  signed(b))
function alu_sgt(a : word, b : word) -> word = word_of_bool(signed(a)   >  signed(b))
function alu_eq(a : word, b : word)  -> word = word_of_bool(a == b)
function alu_iszero(a : word)        -> word = word_of_bool(word_is_zero(a))

/* --- bitwise --- */
function alu_and(a : word, b : word) -> word = a & b
function alu_or(a : word, b : word)  -> word = a | b
function alu_xor(a : word, b : word) -> word = xor_vec(a, b)
function alu_not(a : word)           -> word = not_vec(a)

/* BYTE(i, x): the i-th most-significant byte of x (0 = MSB), else 0. */
function alu_byte(i : word, x : word) -> word = {
  let idx = unsigned(i);
  if idx >= 32 then word_of_int(0)
  else {
    let shift = (31 - idx) * 8;
    word_of_int(emod_int(ediv_int(unsigned(x), pow2(shift)), 256))
  }
}

/* --- shifts (shift amount is the TOP stack item per EVM SHL/SHR/SAR) --- */
function alu_shl(shift_amt : word, v : word) -> word =              /* SHL */
  if unsigned(shift_amt) >= 256 then word_of_int(0)
  else sail_shiftleft(v, unsigned(shift_amt))

function alu_shr(shift_amt : word, v : word) -> word =              /* SHR */
  if unsigned(shift_amt) >= 256 then word_of_int(0)
  else sail_shiftright(v, unsigned(shift_amt))

/* SAR: arithmetic (sign-propagating) right shift via floored signed div. */
function alu_sar(shift_amt : word, v : word) -> word = {            /* SAR */
  let s = unsigned(shift_amt);
  let s2 = if s > 256 then 256 else s;
  word_of_int(fdiv_int(signed(v), pow2(s2)))
}

/* CLZ: count leading zero bits of a 256-bit word (EIP-7939). */
function alu_clz(x : word) -> word = {
  var count : int = 0;
  var found : bool = false;
  foreach (i from 0 to 255) {
    let bit_index = 255 - i;             /* scan from MSB down */
    if not_bool(found) then {
      if x[bit_index] == bitzero then count = count + 1 else found = true
    } else ()
  };
  word_of_int(count)
}

/* KECCAK-256 is a real Sail implementation (evm/keccak.sail), included
 * after this prelude. Self-contained so it compiles to the RISC-V target. */
