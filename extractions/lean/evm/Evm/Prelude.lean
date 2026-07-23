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
def U256 (value : Nat) : word :=
  ⟨value⟩

def Address (bytes : (Vector byte 20)) : address :=
  bytes

def B256 (bytes : (Vector byte 32)) : b256 :=
  bytes

def undefined_AddressResult (_ : Unit) : SailM AddressResult := do
  (pure { success := ← (undefined_bool ()),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))) })

/-- Interprets a digest as the corresponding big-endian EVM word. -/
def hash_to_word (bytes : hash) : word :=
  ⟨(BitVec.toNatInt (from_bytes_le (n := 32) bytes))⟩

/-- Serializes an EVM word as a 32-byte big-endian digest. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_hash (value : word) : hash :=
  let value := (value).value
  (B256 (to_bytes_le (n := 32) (get_slice_int 256 value 0)))

/-- Compares two digests as unsigned big-endian integers. -/
def hash_lt (left : hash) (right : hash) : Bool :=
  (((hash_to_word left)).value <b ((hash_to_word right)).value)

/-- Converts a word to its low 160-bit address. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_address (value : word) : address :=
  let value := (value).value
  (Address (to_bytes_le (n := 20) (get_slice_int 160 value 0)))

def ZERO_WORD : word :=
  ⟨((U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000000000#256))).value⟩

def ZERO_ADDRESS : address := (Address (vectorInit 0x00#8))

def ZERO_HASH : hash := (B256 (vectorInit 0x00#8))

def WORD_ZERO : word :=
  ⟨((U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000000000#256))).value⟩

def WORD_ONE : word :=
  ⟨((U256 (BitVec.toNatInt 0x0000000000000000000000000000000000000000000000000000000000000001#256))).value⟩

def WORD_ALL_ONES : word :=
  ⟨((U256 (BitVec.toNatInt 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#256))).value⟩

def WORD_SIGN_BIT : word :=
  ⟨((U256 (BitVec.toNatInt 0x8000000000000000000000000000000000000000000000000000000000000000#256))).value⟩

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_add_word (left : Nat) (right : Nat) : word :=
  ⟨((U256 (Nat.mod (left + right) (2 ^i 256)))).value⟩

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_sub_word (left : Nat) (right : Nat) : word :=
  ⟨((U256 (Int.emod (left -i right) (2 ^i 256)))).value⟩

/- Type quantifiers: k_ex406293_ : Nat, k_ex406292_ : Nat, 0 ≤ k_ex406292_ ∧
  k_ex406292_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406293_ ∧ k_ex406293_ ≤ (2 ^ 256 - 1) -/
def word_and (left : word) (right : word) : word :=
  let left := (left).value
  let right := (right).value
  ⟨((U256 (BitVec.toNatInt ((get_slice_int 256 left 0) &&& (get_slice_int 256 right 0))))).value⟩

/- Type quantifiers: k_ex406295_ : Nat, k_ex406294_ : Nat, 0 ≤ k_ex406294_ ∧
  k_ex406294_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406295_ ∧ k_ex406295_ ≤ (2 ^ 256 - 1) -/
def word_or (left : word) (right : word) : word :=
  let left := (left).value
  let right := (right).value
  ⟨((U256 (BitVec.toNatInt ((get_slice_int 256 left 0) ||| (get_slice_int 256 right 0))))).value⟩

/- Type quantifiers: k_ex406297_ : Nat, k_ex406296_ : Nat, 0 ≤ k_ex406296_ ∧
  k_ex406296_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406297_ ∧ k_ex406297_ ≤ (2 ^ 256 - 1) -/
def word_xor (left : word) (right : word) : word :=
  let left := (left).value
  let right := (right).value
  ⟨((U256 (BitVec.toNatInt ((get_slice_int 256 left 0) ^^^ (get_slice_int 256 right 0))))).value⟩

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_not (value : word) : word :=
  let value := (value).value
  ⟨((U256 (BitVec.toNatInt (Complement.complement (get_slice_int 256 value 0))))).value⟩

/- Type quantifiers: k_ex406300_ : Nat, k_ex406299_ : Nat, 0 ≤ k_ex406299_ ∧
  k_ex406299_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406300_ ∧ k_ex406300_ ≤ 255 -/
def word_bit (value : word) (index : Nat) : (BitVec 1) :=
  let value := (value).value
  (BitVec.access (get_slice_int 256 value 0) index)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_low_byte (value : Nat) : byte :=
  (get_slice_int 8 value 0)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_shift_left_one (value : word) : word :=
  let value := (value).value
  ⟨((U256 (Nat.mod (value *i 2) (2 ^i 256)))).value⟩

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_shift_right_one (value : word) : word :=
  let value := (value).value
  ⟨((U256 (Nat.div value 2))).value⟩

/-- `1` if the condition holds, else `0` — the EVM boolean convention. -/
/- Type quantifiers: k_ex406303_ : Bool -/
def word_of_bool (b : Bool) : word :=
  ⟨if (b : Bool)
  then (WORD_ONE).value
  else (WORD_ZERO).value⟩

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def word_is_zero (w : Nat) : Bool :=
  (w == (WORD_ZERO).value)

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
def word_bit_length (value : Nat) : word_bit_count :=
  ⟨let limb3 := (BitVec.toNatInt (get_slice_int 64 value 192))
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
      else (u64_bit_length (BitVec.toNatInt (get_slice_int 64 value 0)))))⟩

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_mul_word (a : Nat) (b : Nat) : Nat :=
  (Nat.mod (a *i b) (2 ^i 256))

/- Type quantifiers: dividend : Nat, divisor : Nat, 0 ≤ dividend ∧
  dividend < (2 ^ 256) ∧ 0 ≤ divisor ∧ divisor < (2 ^ 256) -/
def word_div_word (dividend : Nat) (divisor : Nat) : word :=
  ⟨if ((divisor == 0) : Bool)
  then (WORD_ZERO).value
  else ((U256 (Nat.div dividend divisor))).value⟩

/- Type quantifiers: dividend : Nat, divisor : Nat, 0 ≤ dividend ∧
  dividend < (2 ^ 256) ∧ 0 ≤ divisor ∧ divisor < (2 ^ 256) -/
def word_mod_word (dividend : Nat) (divisor : Nat) : word :=
  ⟨if ((divisor == 0) : Bool)
  then (WORD_ZERO).value
  else ((U256 (Nat.mod dividend divisor))).value⟩

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_greater_than_word (left : Nat) (right : Nat) : Bool :=
  (left >b right)

/-- Shifts a word left by a bounded count, yielding zero at the width. -/
/- Type quantifiers: k_ex406310_ : Nat, k_ex406309_ : Nat, 0 ≤ k_ex406309_ ∧
  k_ex406309_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406310_ ∧ k_ex406310_ ≤ 256 -/
def word_shift_left (value : word) (amount : word_bit_count) : word :=
  let value := (value).value
  let amount := (amount).value
  ⟨((U256 (BitVec.toNatInt ((get_slice_int 256 value 0) <<< amount)))).value⟩

/-- Shifts a word right logically by a bounded count. -/
/- Type quantifiers: k_ex406312_ : Nat, k_ex406311_ : Nat, 0 ≤ k_ex406311_ ∧
  k_ex406311_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406312_ ∧ k_ex406312_ ≤ 256 -/
def word_shift_right (value : word) (amount : word_bit_count) : word :=
  let value := (value).value
  let amount := (amount).value
  ⟨(BitVec.toNatInt ((get_slice_int 256 value 0) >>> amount))⟩

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_byte_length (value : Nat) : Nat :=
  let bit_length := ((word_bit_length value)).value
  if ((bit_length == 0) : Bool)
  then 0
  else (Nat.div (bit_length + 7) 8)

/-- Shifts a two's-complement word right while extending its sign bit. -/
/- Type quantifiers: k_ex406314_ : Nat, k_ex406313_ : Nat, 0 ≤ k_ex406313_ ∧
  k_ex406313_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406314_ ∧ k_ex406314_ ≤ 256 -/
def word_arithmetic_shift_right (value : word) (amount : word_bit_count) : word :=
  let value := (value).value
  let amount := (amount).value
  ⟨let shifted := ((word_shift_right ⟨value⟩ ⟨amount⟩)).value
  if (((word_bit ⟨value⟩ 255) == 1#1) : Bool)
  then
    (let sign_fill := ((word_shift_left ⟨(WORD_ALL_ONES).value⟩ ⟨(256 - amount)⟩)).value
    ((word_or ⟨shifted⟩ ⟨sign_fill⟩)).value)
  else shifted⟩

/-- Embeds an address's bytes into the low 160 bits of an EVM word. -/
def address_to_word (bytes : address) : word :=
  ⟨(BitVec.toNatInt (from_bytes_le (n := 20) bytes))⟩

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_negate (value : word) : word :=
  let value := (value).value
  ⟨((word_sub_word (WORD_ZERO).value value)).value⟩

/-- Returns the unsigned magnitude of a two's-complement word. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_abs (value : word) : word :=
  let value := (value).value
  ⟨if (((word_bit ⟨value⟩ 255) == 1#1) : Bool)
  then ((word_negate ⟨value⟩)).value
  else value⟩

/-- Signed (two's-complement) 256-bit less-than: sign bits decide when they
differ, otherwise the unsigned order applies. -/
/- Type quantifiers: k_ex406318_ : Nat, k_ex406317_ : Nat, 0 ≤ k_ex406317_ ∧
  k_ex406317_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406318_ ∧ k_ex406318_ ≤ (2 ^ 256 - 1) -/
def word_slt (a : word) (b : word) : Bool :=
  let a := (a).value
  let b := (b).value
  let a_neg := ((word_bit ⟨a⟩ 255) == 1#1)
  let b_neg := ((word_bit ⟨b⟩ 255) == 1#1)
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
def alu_add (a : word) (b : Nat) : word :=
  let a := (a).value
  ⟨((word_add_word a b)).value⟩

/- Type quantifiers: k_ex406321_ : Nat, k_ex406320_ : Nat, 0 ≤ k_ex406320_ ∧
  k_ex406320_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406321_ ∧ k_ex406321_ ≤ (2 ^ 256 - 1) -/
def alu_sub (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_sub_word a b)).value⟩

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def alu_mul (a : Nat) (b : Nat) : Nat :=
  (word_mul_word a b)

/-- `DIV`: unsigned Euclidean division; division by zero yields `0`
(YP Appendix H). -/
/- Type quantifiers: k_ex406323_ : Nat, k_ex406322_ : Nat, 0 ≤ k_ex406322_ ∧
  k_ex406322_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406323_ ∧ k_ex406323_ ≤ (2 ^ 256 - 1) -/
def alu_div (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_div_word a b)).value⟩

/-- `MOD`: unsigned modulus; a zero modulus yields `0`. -/
/- Type quantifiers: k_ex406325_ : Nat, k_ex406324_ : Nat, 0 ≤ k_ex406324_ ∧
  k_ex406324_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406325_ ∧ k_ex406325_ ≤ (2 ^ 256 - 1) -/
def alu_mod (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_mod_word a b)).value⟩

/-- `SDIV`: signed division, truncating toward zero; division by zero
yields `0`. -/
/- Type quantifiers: k_ex406327_ : Nat, k_ex406326_ : Nat, 0 ≤ k_ex406326_ ∧
  k_ex406326_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406327_ ∧ k_ex406327_ ≤ (2 ^ 256 - 1) -/
def alu_sdiv (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨if ((word_is_zero b) : Bool)
  then (WORD_ZERO).value
  else
    (let quotient := ((word_div_word ((word_abs ⟨a⟩)).value ((word_abs ⟨b⟩)).value)).value
    if ((neq_bool ((word_bit ⟨a⟩ 255) == 1#1) ((word_bit ⟨b⟩ 255) == 1#1)) : Bool)
    then ((word_negate ⟨quotient⟩)).value
    else quotient)⟩

/-- `SMOD`: signed remainder, with the sign of the dividend; a zero modulus
yields `0`. -/
/- Type quantifiers: k_ex406329_ : Nat, k_ex406328_ : Nat, 0 ≤ k_ex406328_ ∧
  k_ex406328_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406329_ ∧ k_ex406329_ ≤ (2 ^ 256 - 1) -/
def alu_smod (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨if ((word_is_zero b) : Bool)
  then (WORD_ZERO).value
  else
    (let remainder := ((word_mod_word ((word_abs ⟨a⟩)).value ((word_abs ⟨b⟩)).value)).value
    if (((word_bit ⟨a⟩ 255) == 1#1) : Bool)
    then ((word_negate ⟨remainder⟩)).value
    else remainder)⟩

/- Type quantifiers: a : Nat, b : Nat, n : Nat, 0 ≤ a ∧
  a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) ∧ 0 ≤ n ∧ n < (2 ^ 256) -/
def alu_addmod (a : Nat) (b : Nat) (n : Nat) : word :=
  ⟨if ((n == 0) : Bool)
  then (WORD_ZERO).value
  else ((U256 (Nat.mod (a + b) n))).value⟩

/- Type quantifiers: a : Nat, b : Nat, n : Nat, 0 ≤ a ∧
  a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) ∧ 0 ≤ n ∧ n < (2 ^ 256) -/
def alu_mulmod (a : Nat) (b : Nat) (n : Nat) : word :=
  ⟨if ((n == 0) : Bool)
  then (WORD_ZERO).value
  else ((U256 (Nat.mod (a *i b) n))).value⟩

/-- `EXP` via square-and-multiply over the 256 exponent bits, reduced
modulo 2^256 at every step. -/
/- Type quantifiers: k_ex406331_ : Nat, k_ex406330_ : Nat, 0 ≤ k_ex406330_ ∧
  k_ex406330_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406331_ ∧ k_ex406331_ ≤ (2 ^ 256 - 1) -/
def alu_exp (base : word) (exponent : word) : word := Id.run do
  let base := (base).value
  let exponent := (exponent).value
  let publicResult ← do
    let result : Nat := (WORD_ONE).value
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
            if (((word_bit ⟨e⟩ 0) == 1#1) : Bool)
            then
              (let result : Nat := (word_mul_word result b)
              result)
            else result
          let b : Nat := (word_mul_word b b)
          let e : Nat := ((word_shift_right_one ⟨e⟩)).value
          (b, e, result)
      (pure loop_vars) ) : Id (Nat × Nat × Nat) )
    (pure result)
  pure (⟨publicResult⟩)

/-- `SIGNEXTEND(byte_index, value)`: sign-extends `value` from byte
`byte_index` (0 = least significant); indices ≥ 31 leave the value
unchanged. -/
/- Type quantifiers: k_ex406333_ : Nat, k_ex406332_ : Nat, 0 ≤ k_ex406332_ ∧
  k_ex406332_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406333_ ∧ k_ex406333_ ≤ (2 ^ 256 - 1) -/
def alu_signextend (byte_index : word) (value : word) : word :=
  let byte_index := (byte_index).value
  let value := (value).value
  ⟨if ((byte_index <b 32) : Bool)
  then
    (let index : Nat := byte_index
    let width : Nat := ((index *i 8) + 8)
    let sign_shift : Nat := ((index *i 8) + 7)
    let sign_set :=
      (((word_and ⟨((word_shift_right ⟨value⟩ ⟨sign_shift⟩)).value⟩
          ⟨(WORD_ONE).value⟩)).value == (WORD_ONE).value)
    let low_mask :=
      ((word_sub_word ((word_shift_left ⟨(WORD_ONE).value⟩ ⟨width⟩)).value (WORD_ONE).value)).value
    if (sign_set : Bool)
    then
      ((word_or ⟨((word_and ⟨value⟩ ⟨low_mask⟩)).value⟩
        ⟨((word_not ⟨low_mask⟩)).value⟩)).value
    else ((word_and ⟨value⟩ ⟨low_mask⟩)).value)
  else value⟩

/- Type quantifiers: k_ex406335_ : Nat, k_ex406334_ : Nat, 0 ≤ k_ex406334_ ∧
  k_ex406334_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406335_ ∧ k_ex406335_ ≤ (2 ^ 256 - 1) -/
def alu_lt (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_of_bool (word_ult a b))).value⟩

/- Type quantifiers: k_ex406337_ : Nat, k_ex406336_ : Nat, 0 ≤ k_ex406336_ ∧
  k_ex406336_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406337_ ∧ k_ex406337_ ≤ (2 ^ 256 - 1) -/
def alu_gt (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_of_bool (word_ult b a))).value⟩

/- Type quantifiers: k_ex406339_ : Nat, k_ex406338_ : Nat, 0 ≤ k_ex406338_ ∧
  k_ex406338_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406339_ ∧ k_ex406339_ ≤ (2 ^ 256 - 1) -/
def alu_slt (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_of_bool (word_slt ⟨a⟩ ⟨b⟩))).value⟩

/- Type quantifiers: k_ex406341_ : Nat, k_ex406340_ : Nat, 0 ≤ k_ex406340_ ∧
  k_ex406340_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406341_ ∧ k_ex406341_ ≤ (2 ^ 256 - 1) -/
def alu_sgt (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_of_bool (word_slt ⟨b⟩ ⟨a⟩))).value⟩

/- Type quantifiers: k_ex406343_ : Nat, k_ex406342_ : Nat, 0 ≤ k_ex406342_ ∧
  k_ex406342_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406343_ ∧ k_ex406343_ ≤ (2 ^ 256 - 1) -/
def alu_eq (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_of_bool (a == b))).value⟩

/- Type quantifiers: a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_iszero (a : word) : word :=
  let a := (a).value
  ⟨((word_of_bool (word_is_zero a))).value⟩

/- Type quantifiers: k_ex406346_ : Nat, k_ex406345_ : Nat, 0 ≤ k_ex406345_ ∧
  k_ex406345_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406346_ ∧ k_ex406346_ ≤ (2 ^ 256 - 1) -/
def alu_and (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_and ⟨a⟩ ⟨b⟩)).value⟩

/- Type quantifiers: k_ex406348_ : Nat, k_ex406347_ : Nat, 0 ≤ k_ex406347_ ∧
  k_ex406347_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406348_ ∧ k_ex406348_ ≤ (2 ^ 256 - 1) -/
def alu_or (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_or ⟨a⟩ ⟨b⟩)).value⟩

/- Type quantifiers: k_ex406350_ : Nat, k_ex406349_ : Nat, 0 ≤ k_ex406349_ ∧
  k_ex406349_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406350_ ∧ k_ex406350_ ≤ (2 ^ 256 - 1) -/
def alu_xor (a : word) (b : word) : word :=
  let a := (a).value
  let b := (b).value
  ⟨((word_xor ⟨a⟩ ⟨b⟩)).value⟩

/- Type quantifiers: a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_not (a : word) : word :=
  let a := (a).value
  ⟨((word_not ⟨a⟩)).value⟩

/-- `BYTE(i, x)`: the `i`-th most-significant byte of `x` (0 = MSB);
indices ≥ 32 yield `0`. -/
/- Type quantifiers: k_ex406353_ : Nat, k_ex406352_ : Nat, 0 ≤ k_ex406352_ ∧
  k_ex406352_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406353_ ∧ k_ex406353_ ≤ (2 ^ 256 - 1) -/
def alu_byte (i : word) (x : word) : word :=
  let i := (i).value
  let x := (x).value
  ⟨if ((i <b 32) : Bool)
  then
    (let index : Nat := i
    let shift : Nat := ((31 - index) *i 8)
    (BitVec.toNatInt (word_low_byte ((word_shift_right ⟨x⟩ ⟨shift⟩)).value)))
  else (WORD_ZERO).value⟩

/-- `SHL`: logical left shift; amounts ≥ 256 yield `0`. -/
/- Type quantifiers: k_ex406355_ : Nat, k_ex406354_ : Nat, 0 ≤ k_ex406354_ ∧
  k_ex406354_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406355_ ∧ k_ex406355_ ≤ (2 ^ 256 - 1) -/
def alu_shl (shift_amt : word) (v : word) : word :=
  let shift_amt := (shift_amt).value
  let v := (v).value
  ⟨if ((shift_amt <b 256) : Bool)
  then ((word_shift_left ⟨v⟩ ⟨shift_amt⟩)).value
  else (WORD_ZERO).value⟩

/-- `SHR`: logical right shift; amounts ≥ 256 yield `0`. -/
/- Type quantifiers: k_ex406357_ : Nat, k_ex406356_ : Nat, 0 ≤ k_ex406356_ ∧
  k_ex406356_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406357_ ∧ k_ex406357_ ≤ (2 ^ 256 - 1) -/
def alu_shr (shift_amt : word) (v : word) : word :=
  let shift_amt := (shift_amt).value
  let v := (v).value
  ⟨if ((shift_amt <b 256) : Bool)
  then ((word_shift_right ⟨v⟩ ⟨shift_amt⟩)).value
  else (WORD_ZERO).value⟩

/-- `SAR`: arithmetic (sign-propagating) right shift. -/
/- Type quantifiers: k_ex406359_ : Nat, k_ex406358_ : Nat, 0 ≤ k_ex406358_ ∧
  k_ex406358_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex406359_ ∧ k_ex406359_ ≤ (2 ^ 256 - 1) -/
def alu_sar (shift_amt : word) (v : word) : word :=
  let shift_amt := (shift_amt).value
  let v := (v).value
  ⟨if ((shift_amt <b 256) : Bool)
  then ((word_arithmetic_shift_right ⟨v⟩ ⟨shift_amt⟩)).value
  else
    (if (((word_bit ⟨v⟩ 255) == 1#1) : Bool)
    then (WORD_ALL_ONES).value
    else (WORD_ZERO).value)⟩

/-- `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). -/
/- Type quantifiers: x : Nat, 0 ≤ x ∧ x ≤ (2 ^ 256 - 1) -/
def alu_clz (x : word) : word :=
  let x := (x).value
  ⟨((U256 (256 - ((word_bit_length x)).value))).value⟩

