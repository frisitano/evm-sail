import Evm.Flow
import Evm.Vector

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

namespace Evm

open ConcurrencyInterfaceV1

open Defs
namespace Functions

open option
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open ByteRegionResult
open BlockError

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def U256 (value : Nat) : Nat :=
  value

def Address (bytes : (Vector (BitVec 8) 20)) : (Vector (BitVec 8) 20) :=
  bytes

def B256 (bytes : (Vector (BitVec 8) 32)) : (Vector (BitVec 8) 32) :=
  bytes

def undefined_AddressResult (_ : Unit) : SailM AddressResult := do
  (pure { success := ← (undefined_bool ()),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))) })

/-- Interprets a digest as the corresponding big-endian EVM word. -/
def hash_to_word (bytes : (Vector (BitVec 8) 32)) : Nat :=
  (BitVec.toNatInt (from_bytes_le (n := 32) bytes))

/-- Serializes an EVM word as a 32-byte big-endian digest. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_hash (value : Nat) : (Vector (BitVec 8) 32) :=
  (B256 (to_bytes_le (n := 32) (get_slice_int 256 value 0)))

/-- Compares two digests as unsigned big-endian integers. -/
def hash_lt (left : (Vector (BitVec 8) 32)) (right : (Vector (BitVec 8) 32)) : Bool :=
  ((hash_to_word left) <b (hash_to_word right))

/-- Converts a word to its low 160-bit address. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_address (value : Nat) : (Vector (BitVec 8) 20) :=
  (Address (to_bytes_le (n := 20) (get_slice_int 160 value 0)))

def ZERO_WORD : word :=
  (U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000000000#256))

def ZERO_ADDRESS : address := (Address (vectorInit 0x00#8))

def ZERO_HASH : hash := (B256 (vectorInit 0x00#8))

def WORD_ZERO : word :=
  (U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000000000#256))

def WORD_ONE : word :=
  (U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000000001#256))

def WORD_ALL_ONES : word :=
  (U256 (BitVec.toNatInt 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#256))

def WORD_SIGN_BIT : word :=
  (U256 (BitVec.toNatInt 0x8000000000000000000000000000000000000000000000000000000000000000#256))

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_add_word (left : Nat) (right : Nat) : Nat :=
  (U256 (Nat.mod (left + right) (2 ^i 256)))

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_sub_word (left : Nat) (right : Nat) : Nat :=
  (U256 (Int.emod (left -i right) (2 ^i 256)))

/- Type quantifiers: k_ex414450_ : Nat, k_ex414449_ : Nat, 0 ≤ k_ex414449_ ∧
  k_ex414449_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414450_ ∧ k_ex414450_ ≤ (2 ^ 256 - 1) -/
def word_and (left : Nat) (right : Nat) : Nat :=
  (U256 (BitVec.toNatInt ((get_slice_int 256 left 0) &&& (get_slice_int 256 right 0))))

/- Type quantifiers: k_ex414452_ : Nat, k_ex414451_ : Nat, 0 ≤ k_ex414451_ ∧
  k_ex414451_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414452_ ∧ k_ex414452_ ≤ (2 ^ 256 - 1) -/
def word_or (left : Nat) (right : Nat) : Nat :=
  (U256 (BitVec.toNatInt ((get_slice_int 256 left 0) ||| (get_slice_int 256 right 0))))

/- Type quantifiers: k_ex414454_ : Nat, k_ex414453_ : Nat, 0 ≤ k_ex414453_ ∧
  k_ex414453_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414454_ ∧ k_ex414454_ ≤ (2 ^ 256 - 1) -/
def word_xor (left : Nat) (right : Nat) : Nat :=
  (U256 (BitVec.toNatInt ((get_slice_int 256 left 0) ^^^ (get_slice_int 256 right 0))))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_not (value : Nat) : Nat :=
  (U256 (BitVec.toNatInt (Complement.complement (get_slice_int 256 value 0))))

/- Type quantifiers: k_ex414457_ : Nat, k_ex414456_ : Nat, 0 ≤ k_ex414456_ ∧
  k_ex414456_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414457_ ∧ k_ex414457_ ≤ 255 -/
def word_bit (value : Nat) (index : Nat) : (BitVec 1) :=
  (BitVec.access (get_slice_int 256 value 0) index)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_low_byte (value : Nat) : (BitVec 8) :=
  (get_slice_int 8 value 0)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_shift_left_one (value : Nat) : Nat :=
  (U256 (Nat.mod (value *i 2) (2 ^i 256)))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_shift_right_one (value : Nat) : Nat :=
  (U256 (Nat.div value 2))

/-- `1` if the condition holds, else `0` — the EVM boolean convention. -/
/- Type quantifiers: k_ex414460_ : Bool -/
def word_of_bool (b : Bool) : Nat :=
  if (b : Bool)
  then WORD_ONE
  else WORD_ZERO

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def word_is_zero (w : Nat) : Bool :=
  (w == WORD_ZERO)

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def word_nonzero (w : Nat) : Bool :=
  (! (word_is_zero w))

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_ult (a : Nat) (b : Nat) : Bool :=
  (a <b b)

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_ule (a : Nat) (b : Nat) : Bool :=
  (! (word_ult b a))

/-- Returns the position above an 8-bit value's most significant set bit. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 8 - 1) -/
def byte_bit_length (value : Nat) : Nat :=
  if ((value <b (2 ^i 4)) : Bool)
  then
    (if ((value <b (2 ^i 2)) : Bool)
    then
      (if ((value <b (2 ^i 1)) : Bool)
      then value
      else 2)
    else
      (if ((value <b (2 ^i 3)) : Bool)
      then 3
      else 4))
  else
    (if ((value <b (2 ^i 6)) : Bool)
    then
      (if ((value <b (2 ^i 5)) : Bool)
      then 5
      else 6)
    else
      (if ((value <b (2 ^i 7)) : Bool)
      then 7
      else 8))

/-- Returns the position above a 16-bit value's most significant set bit. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 16 - 1) -/
def u16_bit_length (value : Nat) : Nat :=
  if ((value <b (2 ^i 8)) : Bool)
  then (byte_bit_length value)
  else (8 + (byte_bit_length (Nat.div value (2 ^i 8))))

/-- Returns the position above a 32-bit value's most significant set bit. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 32 - 1) -/
def u32_bit_length (value : Nat) : Nat :=
  if ((value <b (2 ^i 16)) : Bool)
  then (u16_bit_length value)
  else (16 + (u16_bit_length (Nat.div value (2 ^i 16))))

/-- Returns the position above a 64-bit value's most significant set bit. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def u64_bit_length (value : Nat) : Nat :=
  if ((value <b (2 ^i 32)) : Bool)
  then (u32_bit_length value)
  else (32 + (u32_bit_length (Nat.div value (2 ^i 32))))

/-- Returns the position above a 128-bit value's most significant set bit. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 128 - 1) -/
def u128_bit_length (value : Nat) : Nat :=
  if ((value <b (2 ^i 64)) : Bool)
  then (u64_bit_length value)
  else (64 + (u64_bit_length (Nat.div value (2 ^i 64))))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_bit_length (value : Nat) : Nat :=
  let limb3 := (BitVec.toNatInt (get_slice_int 64 value 192))
  if ((limb3 != 0) : Bool)
  then (192 + (u64_bit_length limb3))
  else
    (let limb2 := (BitVec.toNatInt (get_slice_int 64 value 128))
    if ((limb2 != 0) : Bool)
    then (128 + (u64_bit_length limb2))
    else
      (let limb1 := (BitVec.toNatInt (get_slice_int 64 value 64))
      if ((limb1 != 0) : Bool)
      then (64 + (u64_bit_length limb1))
      else (u64_bit_length (BitVec.toNatInt (get_slice_int 64 value 0)))))

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_mul_word (a : Nat) (b : Nat) : Nat :=
  (Nat.mod (a *i b) (2 ^i 256))

/- Type quantifiers: dividend : Nat, divisor : Nat, 0 ≤ dividend ∧
  dividend < (2 ^ 256) ∧ 0 ≤ divisor ∧ divisor < (2 ^ 256) -/
def word_div_word (dividend : Nat) (divisor : Nat) : Nat :=
  if ((divisor == 0) : Bool)
  then WORD_ZERO
  else (U256 (Nat.div dividend divisor))

/- Type quantifiers: dividend : Nat, divisor : Nat, 0 ≤ dividend ∧
  dividend < (2 ^ 256) ∧ 0 ≤ divisor ∧ divisor < (2 ^ 256) -/
def word_mod_word (dividend : Nat) (divisor : Nat) : Nat :=
  if ((divisor == 0) : Bool)
  then WORD_ZERO
  else (U256 (Nat.mod dividend divisor))

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_greater_than_word (left : Nat) (right : Nat) : Bool :=
  (left >b right)

/-- Shifts a word left by a bounded count, yielding zero at the width. -/
/- Type quantifiers: k_ex414467_ : Nat, k_ex414466_ : Nat, 0 ≤ k_ex414466_ ∧
  k_ex414466_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414467_ ∧ k_ex414467_ ≤ 256 -/
def word_shift_left (value : Nat) (amount : Nat) : Nat :=
  (U256 (BitVec.toNatInt ((get_slice_int 256 value 0) <<< amount)))

/-- Shifts a word right logically by a bounded count. -/
/- Type quantifiers: k_ex414469_ : Nat, k_ex414468_ : Nat, 0 ≤ k_ex414468_ ∧
  k_ex414468_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414469_ ∧ k_ex414469_ ≤ 256 -/
def word_shift_right (value : Nat) (amount : Nat) : Nat :=
  (BitVec.toNatInt ((get_slice_int 256 value 0) >>> amount))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_byte_length (value : Nat) : Nat :=
  let bit_length := (word_bit_length value)
  if ((bit_length == 0) : Bool)
  then 0
  else (Nat.div (bit_length + 7) 8)

/-- Shifts a two's-complement word right while extending its sign bit. -/
/- Type quantifiers: k_ex414471_ : Nat, k_ex414470_ : Nat, 0 ≤ k_ex414470_ ∧
  k_ex414470_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414471_ ∧ k_ex414471_ ≤ 256 -/
def word_arithmetic_shift_right (value : Nat) (amount : Nat) : Nat :=
  let shifted := (word_shift_right value amount)
  if (((word_bit value 255) == 1#1) : Bool)
  then
    (let sign_fill := (word_shift_left WORD_ALL_ONES (256 - amount))
    (word_or shifted sign_fill))
  else shifted

/-- Embeds an address's bytes into the low 160 bits of an EVM word. -/
def address_to_word (bytes : (Vector (BitVec 8) 20)) : Nat :=
  (BitVec.toNatInt (from_bytes_le (n := 20) bytes))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_negate (value : Nat) : Nat :=
  (word_sub_word WORD_ZERO value)

/-- Returns the unsigned magnitude of a two's-complement word. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_abs (value : Nat) : Nat :=
  if (((word_bit value 255) == 1#1) : Bool)
  then (word_negate value)
  else value

/-- Signed (two's-complement) 256-bit less-than: sign bits decide when they
differ, otherwise the unsigned order applies. -/
/- Type quantifiers: k_ex414475_ : Nat, k_ex414474_ : Nat, 0 ≤ k_ex414474_ ∧
  k_ex414474_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414475_ ∧ k_ex414475_ ≤ (2 ^ 256 - 1) -/
def word_slt (a : Nat) (b : Nat) : Bool :=
  let a_neg := ((word_bit a 255) == 1#1)
  let b_neg := ((word_bit b 255) == 1#1)
  if (a_neg : Bool)
  then
    (if (b_neg : Bool)
    then (word_ult a b)
    else true)
  else
    (if (b_neg : Bool)
    then false
    else (word_ult a b))

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ b ∧ b < (2 ^ 256), 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_add (a : Nat) (b : Nat) : Nat :=
  (word_add_word a b)

/- Type quantifiers: k_ex414478_ : Nat, k_ex414477_ : Nat, 0 ≤ k_ex414477_ ∧
  k_ex414477_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414478_ ∧ k_ex414478_ ≤ (2 ^ 256 - 1) -/
def alu_sub (a : Nat) (b : Nat) : Nat :=
  (word_sub_word a b)

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def alu_mul (a : Nat) (b : Nat) : Nat :=
  (word_mul_word a b)

/-- `DIV`: unsigned Euclidean division; division by zero yields `0`
(YP Appendix H). -/
/- Type quantifiers: k_ex414480_ : Nat, k_ex414479_ : Nat, 0 ≤ k_ex414479_ ∧
  k_ex414479_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414480_ ∧ k_ex414480_ ≤ (2 ^ 256 - 1) -/
def alu_div (a : Nat) (b : Nat) : Nat :=
  (word_div_word a b)

/-- `MOD`: unsigned modulus; a zero modulus yields `0`. -/
/- Type quantifiers: k_ex414482_ : Nat, k_ex414481_ : Nat, 0 ≤ k_ex414481_ ∧
  k_ex414481_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414482_ ∧ k_ex414482_ ≤ (2 ^ 256 - 1) -/
def alu_mod (a : Nat) (b : Nat) : Nat :=
  (word_mod_word a b)

/-- `SDIV`: signed division, truncating toward zero; division by zero
yields `0`. -/
/- Type quantifiers: k_ex414484_ : Nat, k_ex414483_ : Nat, 0 ≤ k_ex414483_ ∧
  k_ex414483_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414484_ ∧ k_ex414484_ ≤ (2 ^ 256 - 1) -/
def alu_sdiv (a : Nat) (b : Nat) : Nat :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let quotient := (word_div_word (word_abs a) (word_abs b))
    if ((neq_bool ((word_bit a 255) == 1#1) ((word_bit b 255) == 1#1)) : Bool)
    then (word_negate quotient)
    else quotient)

/-- `SMOD`: signed remainder, with the sign of the dividend; a zero modulus
yields `0`. -/
/- Type quantifiers: k_ex414486_ : Nat, k_ex414485_ : Nat, 0 ≤ k_ex414485_ ∧
  k_ex414485_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414486_ ∧ k_ex414486_ ≤ (2 ^ 256 - 1) -/
def alu_smod (a : Nat) (b : Nat) : Nat :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let remainder := (word_mod_word (word_abs a) (word_abs b))
    if (((word_bit a 255) == 1#1) : Bool)
    then (word_negate remainder)
    else remainder)

/- Type quantifiers: a : Nat, b : Nat, n : Nat, 0 ≤ a ∧
  a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) ∧ 0 ≤ n ∧ n < (2 ^ 256) -/
def alu_addmod (a : Nat) (b : Nat) (n : Nat) : Nat :=
  if ((n == 0) : Bool)
  then WORD_ZERO
  else (U256 (Nat.mod (a + b) n))

/- Type quantifiers: a : Nat, b : Nat, n : Nat, 0 ≤ a ∧
  a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) ∧ 0 ≤ n ∧ n < (2 ^ 256) -/
def alu_mulmod (a : Nat) (b : Nat) (n : Nat) : Nat :=
  if ((n == 0) : Bool)
  then WORD_ZERO
  else (U256 (Nat.mod (a *i b) n))

/-- `EXP` via square-and-multiply over the 256 exponent bits, reduced
modulo 2^256 at every step. -/
/- Type quantifiers: k_ex414488_ : Nat, k_ex414487_ : Nat, 0 ≤ k_ex414487_ ∧
  k_ex414487_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414488_ ∧ k_ex414488_ ≤ (2 ^ 256 - 1) -/
def alu_exp (base : Nat) (exponent : Nat) : Nat := Id.run do
  let result : Nat := WORD_ONE
  let b : Nat := base
  let e : Nat := exponent
  let (b, e, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (b, e, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (b, e, result) := loop_vars
      loop_vars :=
        let result : Nat :=
          if (((word_bit e 0) == 1#1) : Bool)
          then
            (let result : Nat := (word_mul_word result b)
            result)
          else result
        let b : Nat := (word_mul_word b b)
        let e : Nat := (word_shift_right_one e)
        (b, e, result)
    (pure loop_vars) ) : Id (Nat × Nat × Nat) )
  (pure result)

/-- `SIGNEXTEND(byte_index, value)`: sign-extends `value` from byte
`byte_index` (0 = least significant); indices ≥ 31 leave the value
unchanged. -/
/- Type quantifiers: k_ex414490_ : Nat, k_ex414489_ : Nat, 0 ≤ k_ex414489_ ∧
  k_ex414489_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414490_ ∧ k_ex414490_ ≤ (2 ^ 256 - 1) -/
def alu_signextend (byte_index : Nat) (value : Nat) : Nat :=
  if ((byte_index <b 32) : Bool)
  then
    (let index : Nat := byte_index
    let width : Nat := ((index *i 8) + 8)
    let sign_shift : Nat := ((index *i 8) + 7)
    let sign_set := ((word_and (word_shift_right value sign_shift) WORD_ONE) == WORD_ONE)
    let low_mask := (word_sub_word (word_shift_left WORD_ONE width) WORD_ONE)
    if (sign_set : Bool)
    then (word_or (word_and value low_mask) (word_not low_mask))
    else (word_and value low_mask))
  else value

/- Type quantifiers: k_ex414492_ : Nat, k_ex414491_ : Nat, 0 ≤ k_ex414491_ ∧
  k_ex414491_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414492_ ∧ k_ex414492_ ≤ (2 ^ 256 - 1) -/
def alu_lt (a : Nat) (b : Nat) : Nat :=
  (word_of_bool (word_ult a b))

/- Type quantifiers: k_ex414494_ : Nat, k_ex414493_ : Nat, 0 ≤ k_ex414493_ ∧
  k_ex414493_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414494_ ∧ k_ex414494_ ≤ (2 ^ 256 - 1) -/
def alu_gt (a : Nat) (b : Nat) : Nat :=
  (word_of_bool (word_ult b a))

/- Type quantifiers: k_ex414496_ : Nat, k_ex414495_ : Nat, 0 ≤ k_ex414495_ ∧
  k_ex414495_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414496_ ∧ k_ex414496_ ≤ (2 ^ 256 - 1) -/
def alu_slt (a : Nat) (b : Nat) : Nat :=
  (word_of_bool (word_slt a b))

/- Type quantifiers: k_ex414498_ : Nat, k_ex414497_ : Nat, 0 ≤ k_ex414497_ ∧
  k_ex414497_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414498_ ∧ k_ex414498_ ≤ (2 ^ 256 - 1) -/
def alu_sgt (a : Nat) (b : Nat) : Nat :=
  (word_of_bool (word_slt b a))

/- Type quantifiers: k_ex414500_ : Nat, k_ex414499_ : Nat, 0 ≤ k_ex414499_ ∧
  k_ex414499_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414500_ ∧ k_ex414500_ ≤ (2 ^ 256 - 1) -/
def alu_eq (a : Nat) (b : Nat) : Nat :=
  (word_of_bool (a == b))

/- Type quantifiers: a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_iszero (a : Nat) : Nat :=
  (word_of_bool (word_is_zero a))

/- Type quantifiers: k_ex414503_ : Nat, k_ex414502_ : Nat, 0 ≤ k_ex414502_ ∧
  k_ex414502_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414503_ ∧ k_ex414503_ ≤ (2 ^ 256 - 1) -/
def alu_and (a : Nat) (b : Nat) : Nat :=
  (word_and a b)

/- Type quantifiers: k_ex414505_ : Nat, k_ex414504_ : Nat, 0 ≤ k_ex414504_ ∧
  k_ex414504_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414505_ ∧ k_ex414505_ ≤ (2 ^ 256 - 1) -/
def alu_or (a : Nat) (b : Nat) : Nat :=
  (word_or a b)

/- Type quantifiers: k_ex414507_ : Nat, k_ex414506_ : Nat, 0 ≤ k_ex414506_ ∧
  k_ex414506_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414507_ ∧ k_ex414507_ ≤ (2 ^ 256 - 1) -/
def alu_xor (a : Nat) (b : Nat) : Nat :=
  (word_xor a b)

/- Type quantifiers: a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_not (a : Nat) : Nat :=
  (word_not a)

/-- `BYTE(i, x)`: the `i`-th most-significant byte of `x` (0 = MSB);
indices ≥ 32 yield `0`. -/
/- Type quantifiers: k_ex414510_ : Nat, k_ex414509_ : Nat, 0 ≤ k_ex414509_ ∧
  k_ex414509_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414510_ ∧ k_ex414510_ ≤ (2 ^ 256 - 1) -/
def alu_byte (i : Nat) (x : Nat) : Nat :=
  if ((i <b 32) : Bool)
  then
    (let index : Nat := i
    let shift : Nat := ((31 - index) *i 8)
    (BitVec.toNatInt (word_low_byte (word_shift_right x shift))))
  else WORD_ZERO

/-- `SHL`: logical left shift; amounts ≥ 256 yield `0`. -/
/- Type quantifiers: k_ex414512_ : Nat, k_ex414511_ : Nat, 0 ≤ k_ex414511_ ∧
  k_ex414511_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414512_ ∧ k_ex414512_ ≤ (2 ^ 256 - 1) -/
def alu_shl (shift_amt : Nat) (v : Nat) : Nat :=
  if ((shift_amt <b 256) : Bool)
  then (word_shift_left v shift_amt)
  else WORD_ZERO

/-- `SHR`: logical right shift; amounts ≥ 256 yield `0`. -/
/- Type quantifiers: k_ex414514_ : Nat, k_ex414513_ : Nat, 0 ≤ k_ex414513_ ∧
  k_ex414513_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414514_ ∧ k_ex414514_ ≤ (2 ^ 256 - 1) -/
def alu_shr (shift_amt : Nat) (v : Nat) : Nat :=
  if ((shift_amt <b 256) : Bool)
  then (word_shift_right v shift_amt)
  else WORD_ZERO

/-- `SAR`: arithmetic (sign-propagating) right shift. -/
/- Type quantifiers: k_ex414516_ : Nat, k_ex414515_ : Nat, 0 ≤ k_ex414515_ ∧
  k_ex414515_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex414516_ ∧ k_ex414516_ ≤ (2 ^ 256 - 1) -/
def alu_sar (shift_amt : Nat) (v : Nat) : Nat :=
  if ((shift_amt <b 256) : Bool)
  then (word_arithmetic_shift_right v shift_amt)
  else
    (if (((word_bit v 255) == 1#1) : Bool)
    then WORD_ALL_ONES
    else WORD_ZERO)

/-- `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). -/
/- Type quantifiers: x : Nat, 0 ≤ x ∧ x ≤ (2 ^ 256 - 1) -/
def alu_clz (x : Nat) : Nat :=
  (U256 (256 - (word_bit_length x)))

