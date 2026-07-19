import Evm.Flow

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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

def undefined_LimbDivMod (_ : Unit) : SailM LimbDivMod := do
  (pure { quotient := ← (undefined_bitvector 64),
          remainder := ← (undefined_bitvector 64) })

/-- Constructs an address from its 160-bit numerical representation. -/
def address_from_bits (value : (BitVec 160)) : address := Id.run do
  let bytes : (Vector (BitVec 8) 20) := (vectorInit 0x00#8)
  let bytes ← (( do
    let loop_k_lower := 0
    let loop_k_upper := 19
    let mut loop_vars := bytes
    for k in [loop_k_lower:loop_k_upper:1]i do
      let bytes := loop_vars
      loop_vars := (vectorUpdate bytes k (Sail.BitVec.extractLsb (value >>> (8 *i k)) 7 0))
    (pure loop_vars) ) : Id (Vector (BitVec 8) 20) )
  (pure (Address bytes))

/-- Returns an address's 160-bit numerical representation. -/
def address_to_bits (app_0 : address) : (BitVec 160) := Id.run do
  let .Address bytes := app_0
  let value : (BitVec 160) := (BitVec.zero 160)
  let loop_k_lower := 0
  let loop_k_upper := 19
  let mut loop_vars := value
  for k in [loop_k_lower:loop_k_upper:1]i do
    let value := loop_vars
    loop_vars := (value ||| ((Sail.BitVec.zeroExtend (GetElem?.getElem! bytes k) 160) <<< (8 *i k)))
  (pure loop_vars)

/-- Constructs a digest from its 256-bit numerical representation. -/
def b256_from_bits (value : (BitVec 256)) : b256 := Id.run do
  let bytes : (Vector (BitVec 8) 32) := (vectorInit 0x00#8)
  let bytes ← (( do
    let loop_k_lower := 0
    let loop_k_upper := 31
    let mut loop_vars := bytes
    for k in [loop_k_lower:loop_k_upper:1]i do
      let bytes := loop_vars
      loop_vars := (vectorUpdate bytes k (Sail.BitVec.extractLsb (value >>> (8 *i k)) 7 0))
    (pure loop_vars) ) : Id (Vector (BitVec 8) 32) )
  (pure (B256 bytes))

/-- Returns a digest's 256-bit numerical representation. -/
def b256_to_bits (app_0 : b256) : (BitVec 256) := Id.run do
  let .B256 bytes := app_0
  let value : (BitVec 256) := (BitVec.zero 256)
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := value
  for k in [loop_k_lower:loop_k_upper:1]i do
    let value := loop_vars
    loop_vars := (value ||| ((Sail.BitVec.zeroExtend (GetElem?.getElem! bytes k) 256) <<< (8 *i k)))
  (pure loop_vars)

def hash_from_bits (value : (BitVec 256)) : hash :=
  (b256_from_bits value)

def hash_to_bits (value : hash) : (BitVec 256) :=
  (b256_to_bits value)

def b256_zero (_ : Unit) : b256 :=
  (B256 (vectorInit 0x00#8))

/-- Compares two digests as unsigned big-endian integers. -/
def b256_lt (typ_0 : b256) (typ_1 : b256) : Bool := Id.run do
  let .B256 left : b256 := typ_0
  let .B256 right : b256 := typ_1
  let less : Bool := false
  let equal : Bool := true
  let (equal, less) ← (( do
    let loop_offset_lower := 0
    let loop_offset_upper := 31
    let mut loop_vars := (equal, less)
    for offset in [loop_offset_lower:loop_offset_upper:1]i do
      let (equal, less) := loop_vars
      loop_vars :=
        let index := (31 -i offset)
        let (equal, less) : (Bool × Bool) :=
          if ((equal && ((GetElem?.getElem! left index) != (GetElem?.getElem! right index))) : Bool)
          then
            (let less : Bool :=
              ((BitVec.toNatInt (GetElem?.getElem! left index)) <b (BitVec.toNatInt
                  (GetElem?.getElem! right index)))
            let equal : Bool := false
            (equal, less))
          else (equal, less)
        (equal, less)
    (pure loop_vars) ) : Id (Bool × Bool) )
  (pure less)

def hash_to_word (value : hash) : word :=
  (U256 (hash_to_bits value))

def word_to_hash (app_0 : word) : hash :=
  let .U256 value := app_0
  (hash_from_bits value)

def word_to_address (app_0 : word) : address :=
  let .U256 w := app_0
  (address_from_bits (Sail.BitVec.extractLsb w 159 0))

def address_to_word (a : address) : word :=
  (U256 (Sail.BitVec.zeroExtend (address_to_bits a) 256))

def ZERO_WORD : word :=
  (U256 0x0000000000000000000000000000000000000000000000000000000000000000#256)

def ZERO_ADDR : address := (word_to_address ZERO_WORD)

def ZERO_HASH : hash := (word_to_hash ZERO_WORD)

def WORD_ZERO : word :=
  (U256 0x0000000000000000000000000000000000000000000000000000000000000000#256)

def WORD_ONE : word := (U256 0x0000000000000000000000000000000000000000000000000000000000000001#256)

def WORD_ALL_ONES : word :=
  (U256 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#256)

def WORD_SIGN_BIT : word :=
  (U256 0x8000000000000000000000000000000000000000000000000000000000000000#256)

def word_to_bits (app_0 : word) : (BitVec 256) :=
  let .U256 value := app_0
  value

def word_add (typ_0 : word) (typ_1 : word) : word :=
  let .U256 left : word := typ_0
  let .U256 right : word := typ_1
  (U256 (left + right))

def word_sub (typ_0 : word) (typ_1 : word) : word :=
  let .U256 left : word := typ_0
  let .U256 right : word := typ_1
  (U256 (left - right))

def word_and (typ_0 : word) (typ_1 : word) : word :=
  let .U256 left : word := typ_0
  let .U256 right : word := typ_1
  (U256 (left &&& right))

def word_or (typ_0 : word) (typ_1 : word) : word :=
  let .U256 left : word := typ_0
  let .U256 right : word := typ_1
  (U256 (left ||| right))

def word_xor (typ_0 : word) (typ_1 : word) : word :=
  let .U256 left : word := typ_0
  let .U256 right : word := typ_1
  (U256 (left ^^^ right))

def word_not (app_0 : word) : word :=
  let .U256 value := app_0
  (U256 (Complement.complement value))

/- Type quantifiers: k_ex161164_ : Nat, 0 ≤ k_ex161164_ ∧ k_ex161164_ ≤ 255 -/
def word_bit (typ_0 : word) (index : Nat) : (BitVec 1) :=
  let .U256 value : word := typ_0
  (BitVec.access value index)

def word_low_byte (app_0 : word) : byte :=
  let .U256 value := app_0
  (Sail.BitVec.extractLsb value 7 0)

def word_limb_0 (app_0 : word) : limb :=
  let .U256 value := app_0
  (Sail.BitVec.extractLsb value 63 0)

def word_limb_1 (app_0 : word) : limb :=
  let .U256 value := app_0
  (Sail.BitVec.extractLsb value 127 64)

def word_limb_2 (app_0 : word) : limb :=
  let .U256 value := app_0
  (Sail.BitVec.extractLsb value 191 128)

def word_limb_3 (app_0 : word) : limb :=
  let .U256 value := app_0
  (Sail.BitVec.extractLsb value 255 192)

def word_from_limbs (limb_0 : limb) (limb_1 : limb) (limb_2 : limb) (limb_3 : limb) : word :=
  (U256 (limb_3 +++ (limb_2 +++ (limb_1 +++ limb_0))))

def word_shift_left_one (app_0 : word) : word :=
  let .U256 value := app_0
  (U256 (value <<< 1))

def word_shift_right_one (app_0 : word) : word :=
  let .U256 value := app_0
  (U256 (value >>> 1))

/-- `1` if the condition holds, else `0` — the EVM boolean convention. -/
/- Type quantifiers: k_ex161165_ : Bool -/
def word_of_bool (b : Bool) : word :=
  if (b : Bool)
  then WORD_ONE
  else WORD_ZERO

def word_is_zero (w : word) : Bool :=
  (w == WORD_ZERO)

def word_nonzero (w : word) : Bool :=
  (! (word_is_zero w))

def LIMB_ZERO : limb := 0x0000000000000000#64

def LIMB_ONE : limb := 0x0000000000000001#64

def LIMB_MAX : limb := 0xFFFFFFFFFFFFFFFF#64

/-- Compares two limbs as unsigned values. -/
def limb_ult (a : limb) (b : limb) : Bool :=
  let difference := (a - b)
  let borrow :=
    (((Complement.complement a) &&& b) ||| ((Complement.complement (a ^^^ b)) &&& difference))
  ((borrow &&& 0x8000000000000000#64) != LIMB_ZERO)

def limb_ule (a : limb) (b : limb) : Bool :=
  (! (limb_ult b a))

/-- Adds limbs and reports fixed-width overflow. -/
def limb_checked_add (a : limb) (b : limb) : (Option limb) :=
  let result := (a + b)
  if ((limb_ult result a) : Bool)
  then none
  else (some result)

/-- Subtracts limbs and reports fixed-width underflow. -/
def limb_checked_sub (a : limb) (b : limb) : (Option limb) :=
  if ((limb_ult a b) : Bool)
  then none
  else (some (a - b))

/-- Multiplies limbs and reports fixed-width overflow. -/
def limb_checked_mul (value : limb) (factor : limb) : (Option limb) := Id.run do
  let result : (BitVec 64) := LIMB_ZERO
  let addend : (BitVec 64) := value
  let remaining : (BitVec 64) := factor
  let valid : Bool := true
  let (addend, remaining, result, valid) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 63
    let mut loop_vars := (addend, remaining, result, valid)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (addend, remaining, result, valid) := loop_vars
      loop_vars :=
        let (result, valid) : ((BitVec 64) × Bool) :=
          if ((valid && ((remaining &&& LIMB_ONE) == LIMB_ONE)) : Bool)
          then
            (let added := (limb_checked_add result addend)
            let (result, valid) : ((BitVec 64) × Bool) :=
              match added with
              | .some value =>
                (let result : (BitVec 64) := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining : (BitVec 64) := (remaining >>> 1)
        let (addend, valid) : ((BitVec 64) × Bool) :=
          if ((valid && ((i <b 63) && (remaining != LIMB_ZERO))) : Bool)
          then
            (let doubled := (limb_checked_add addend addend)
            let (addend, valid) : ((BitVec 64) × Bool) :=
              match doubled with
              | .some value =>
                (let addend : (BitVec 64) := value
                (addend, valid))
              | none =>
                (let valid : Bool := false
                (addend, valid))
            (addend, valid))
          else (addend, valid)
        (addend, remaining, result, valid)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 64) × (BitVec 64) × Bool) )
  if (valid : Bool)
  then (pure (some result))
  else (pure none)

/-- Computes unsigned limb quotient and remainder by bounded long division. -/
def limb_divmod (dividend : limb) (divisor : limb) : LimbDivMod := Id.run do
  if ((divisor == LIMB_ZERO) : Bool)
  then
    (pure { quotient := LIMB_ZERO,
            remainder := LIMB_ZERO })
  else
    (do
      let quotient : (BitVec 64) := LIMB_ZERO
      let remainder : (BitVec 64) := LIMB_ZERO
      let remaining : (BitVec 64) := dividend
      let (quotient, remainder, remaining) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 63
        let mut loop_vars := (quotient, remainder, remaining)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (quotient, remainder, remaining) := loop_vars
          loop_vars :=
            let incoming :=
              if (((BitVec.access remaining 63) == 1#1) : Bool)
              then LIMB_ONE
              else LIMB_ZERO
            let overflow := ((BitVec.access remainder 63) == 1#1)
            let remainder : (BitVec 64) := ((remainder <<< 1) ||| incoming)
            let remaining : (BitVec 64) := (remaining <<< 1)
            let quotient : (BitVec 64) := (quotient <<< 1)
            let (quotient, remainder) : ((BitVec 64) × (BitVec 64)) :=
              if ((overflow || (limb_ule divisor remainder)) : Bool)
              then
                (let remainder : (BitVec 64) := (remainder - divisor)
                let quotient : (BitVec 64) := (quotient ||| LIMB_ONE)
                (quotient, remainder))
              else (quotient, remainder)
            (quotient, remainder, remaining)
        (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 64) × (BitVec 64)) )
      (pure { quotient := quotient,
              remainder := remainder }))

/-- Increments a word bit count without exceeding the word width. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 256 -/
def word_bit_count_increment (value : word_bit_count) : SailM word_bit_count := do
  let value := (value).value
  let semanticResult ← do
    if ((value <b 256) : Bool)
    then (pure (value + 1))
    else
      (do
        assert false "sail/prelude.sail:293.20-293.21"
        throw Error.Exit)
  pure (⟨semanticResult⟩)

/-- Returns the position above a limb's most significant set bit. -/
def limb_bit_length (value : limb) : limb_bit_count := Id.run do
  let semanticResult ← do
    let remaining : (BitVec 64) := value
    let length : Nat := 0
    let (length, remaining) ← (( do
      let loop_step_lower := 0
      let loop_step_upper := 63
      let mut loop_vars := (length, remaining)
      for step in [loop_step_lower:loop_step_upper:1]i do
        let (length, remaining) := loop_vars
        loop_vars :=
          let (length, remaining) : (Nat × (BitVec 64)) :=
            if ((remaining != LIMB_ZERO) : Bool)
            then
              (let length : Nat := (step + 1)
              let remaining : (BitVec 64) := (remaining >>> 1)
              (length, remaining))
            else (length, remaining)
          (length, remaining)
      (pure loop_vars) ) : Id (Nat × (BitVec 64)) )
    (pure length)
  pure (⟨semanticResult⟩)

/-- Narrows a word to one limb when all higher bits are zero. -/
def word_to_limb (app_0 : word) : (Option limb) :=
  let .U256 w := app_0
  if (((Sail.BitVec.extractLsb w 255 64) == (BitVec.zero 192)) : Bool)
  then (some (Sail.BitVec.extractLsb w 63 0))
  else none

def limb_to_word (value : limb) : word :=
  (U256 (Sail.BitVec.zeroExtend value 256))

/-- Embeds a natural number smaller than the word modulus. -/
/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_nat (value : Nat) : SailM word := do
  assert (value <b (2 ^i 256)) "sail/prelude.sail:327.26-327.27"
  (pure (U256 (get_slice_int 256 value 0)))

/-- Unsigned 256-bit less-than, defined by comparing the four 64-bit limbs
from most significant down. -/
def word_ult (typ_0 : word) (typ_1 : word) : Bool :=
  let .U256 a : word := typ_0
  let .U256 b : word := typ_1
  if (((Sail.BitVec.extractLsb a 255 192) != (Sail.BitVec.extractLsb b 255 192)) : Bool)
  then (limb_ult (Sail.BitVec.extractLsb a 255 192) (Sail.BitVec.extractLsb b 255 192))
  else
    (if (((Sail.BitVec.extractLsb a 191 128) != (Sail.BitVec.extractLsb b 191 128)) : Bool)
    then (limb_ult (Sail.BitVec.extractLsb a 191 128) (Sail.BitVec.extractLsb b 191 128))
    else
      (if (((Sail.BitVec.extractLsb a 127 64) != (Sail.BitVec.extractLsb b 127 64)) : Bool)
      then (limb_ult (Sail.BitVec.extractLsb a 127 64) (Sail.BitVec.extractLsb b 127 64))
      else (limb_ult (Sail.BitVec.extractLsb a 63 0) (Sail.BitVec.extractLsb b 63 0))))

def word_ule (a : word) (b : word) : Bool :=
  (! (word_ult b a))

/-- Returns the position above a word's most significant set bit. -/
def word_bit_length (app_0 : word) : word_bit_count :=
  ⟨let .U256 value := app_0
  if (((Sail.BitVec.extractLsb value 255 192) != LIMB_ZERO) : Bool)
  then (192 + ((limb_bit_length (Sail.BitVec.extractLsb value 255 192))).value)
  else
    (if (((Sail.BitVec.extractLsb value 191 128) != LIMB_ZERO) : Bool)
    then (128 + ((limb_bit_length (Sail.BitVec.extractLsb value 191 128))).value)
    else
      (if (((Sail.BitVec.extractLsb value 127 64) != LIMB_ZERO) : Bool)
      then (64 + ((limb_bit_length (Sail.BitVec.extractLsb value 127 64))).value)
      else ((limb_bit_length (Sail.BitVec.extractLsb value 63 0))).value))⟩

/-- Adds words and reports unsigned overflow beyond 256 bits. -/
def word_checked_add (a : word) (b : word) : (Option word) :=
  let result := (word_add a b)
  if ((word_ult result a) : Bool)
  then none
  else (some result)

/-- Multiplies a word by one limb and reports overflow beyond 256 bits. -/
def word_checked_mul_limb (value : word) (factor : limb) : (Option word) := Id.run do
  let result : word := ZERO_WORD
  let addend : word := value
  let remaining : (BitVec 64) := factor
  let valid : Bool := true
  let (addend, remaining, result, valid) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 63
    let mut loop_vars := (addend, remaining, result, valid)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (addend, remaining, result, valid) := loop_vars
      loop_vars :=
        let (result, valid) : (word × Bool) :=
          if ((valid && ((remaining &&& LIMB_ONE) == LIMB_ONE)) : Bool)
          then
            (let added := (word_checked_add result addend)
            let (result, valid) : (word × Bool) :=
              match added with
              | .some value =>
                (let result : word := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining : (BitVec 64) := (remaining >>> 1)
        let (addend, valid) : (word × Bool) :=
          if ((valid && ((i <b 63) && (remaining != LIMB_ZERO))) : Bool)
          then
            (let doubled := (word_checked_add addend addend)
            let (addend, valid) : (word × Bool) :=
              match doubled with
              | .some value =>
                (let addend : word := value
                (addend, valid))
              | none =>
                (let valid : Bool := false
                (addend, valid))
            (addend, valid))
          else (addend, valid)
        (addend, remaining, result, valid)
    (pure loop_vars) ) : Id (word × (BitVec 64) × word × Bool) )
  if (valid : Bool)
  then (pure (some result))
  else (pure none)

/-- Multiplies words modulo the 256-bit word modulus. -/
def word_mul (a : word) (b : word) : word := Id.run do
  let result : word := WORD_ZERO
  let addend : word := a
  let remaining : word := b
  let (addend, remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (addend, remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (addend, remaining, result) := loop_vars
      loop_vars :=
        let result : word :=
          if (((word_bit remaining 0) == 1#1) : Bool)
          then (word_add result addend)
          else result
        let addend : word := (word_shift_left_one addend)
        let remaining : word := (word_shift_right_one remaining)
        (addend, remaining, result)
    (pure loop_vars) ) : Id (word × word × word) )
  (pure result)

/-- Computes unsigned word quotient and remainder by bounded long division. -/
def word_divmod (dividend : word) (divisor : word) : WordDivMod := Id.run do
  if ((word_is_zero divisor) : Bool)
  then
    (pure { quotient := WORD_ZERO,
            remainder := WORD_ZERO })
  else
    (do
      let quotient : word := WORD_ZERO
      let remainder : word := WORD_ZERO
      let remaining : word := dividend
      let (quotient, remainder, remaining) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 255
        let mut loop_vars := (quotient, remainder, remaining)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (quotient, remainder, remaining) := loop_vars
          loop_vars :=
            let incoming :=
              if (((word_bit remaining 255) == 1#1) : Bool)
              then WORD_ONE
              else WORD_ZERO
            let overflow := ((word_bit remainder 255) == 1#1)
            let remainder : word := (word_or (word_shift_left_one remainder) incoming)
            let remaining : word := (word_shift_left_one remaining)
            let quotient : word := (word_shift_left_one quotient)
            let (quotient, remainder) : (word × word) :=
              if ((overflow || (word_ule divisor remainder)) : Bool)
              then
                (let remainder : word := (word_sub remainder divisor)
                let quotient : word := (word_or quotient WORD_ONE)
                (quotient, remainder))
              else (quotient, remainder)
            (quotient, remainder, remaining)
        (pure loop_vars) ) : Id (word × word × word) )
      (pure { quotient := quotient,
              remainder := remainder }))

/-- Adds reduced residues modulo a nonzero word modulus without overflow. -/
def word_mod_add_reduced (a : word) (b : word) (modulus : word) : word :=
  let threshold := (word_sub modulus b)
  if ((word_ule threshold a) : Bool)
  then (word_sub a threshold)
  else (word_add a b)

/-- Shifts a word left by a limb-sized amount, yielding zero past the width. -/
def word_shift_left_limb (value : word) (amount : limb) : word := Id.run do
  let result : word := value
  let remaining : (BitVec 64) := amount
  let (remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (remaining, result) := loop_vars
      loop_vars :=
        let (remaining, result) : ((BitVec 64) × word) :=
          if ((remaining != LIMB_ZERO) : Bool)
          then
            (let result : word := (word_shift_left_one result)
            let remaining : (BitVec 64) := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × word) )
  (pure result)

/-- Shifts a word right logically by a limb-sized amount. -/
def word_shift_right_limb (value : word) (amount : limb) : word := Id.run do
  let result : word := value
  let remaining : (BitVec 64) := amount
  let (remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (remaining, result) := loop_vars
      loop_vars :=
        let (remaining, result) : ((BitVec 64) × word) :=
          if ((remaining != LIMB_ZERO) : Bool)
          then
            (let result : word := (word_shift_right_one result)
            let remaining : (BitVec 64) := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × word) )
  (pure result)

/-- Shifts a two's-complement word right while extending its sign bit. -/
def word_arithmetic_shift_right_limb (value : word) (amount : limb) : word := Id.run do
  let result : word := value
  let remaining : (BitVec 64) := amount
  let negative := ((word_bit value 255) == 1#1)
  let (remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (remaining, result) := loop_vars
      loop_vars :=
        let (remaining, result) : ((BitVec 64) × word) :=
          if ((remaining != LIMB_ZERO) : Bool)
          then
            (let result : word := (word_shift_right_one result)
            let result : word :=
              if (negative : Bool)
              then (word_or result WORD_SIGN_BIT)
              else result
            let remaining : (BitVec 64) := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × word) )
  (pure result)

def word_negate (value : word) : word :=
  (word_sub WORD_ZERO value)

/-- Returns the unsigned magnitude of a two's-complement word. -/
def word_abs (value : word) : word :=
  if (((word_bit value 255) == 1#1) : Bool)
  then (word_negate value)
  else value

/-- Signed (two's-complement) 256-bit less-than: sign bits decide when they
differ, otherwise the unsigned order applies. -/
def word_slt (a : word) (b : word) : Bool :=
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

def alu_add (a : word) (b : word) : word :=
  (word_add a b)

def alu_sub (a : word) (b : word) : word :=
  (word_sub a b)

def alu_mul (a : word) (b : word) : word :=
  (word_mul a b)

/-- `DIV`: unsigned Euclidean division; division by zero yields `0`
(YP Appendix H). -/
def alu_div (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else (word_divmod a b).quotient

/-- `MOD`: unsigned modulus; a zero modulus yields `0`. -/
def alu_mod (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else (word_divmod a b).remainder

/-- `SDIV`: signed division, truncating toward zero; division by zero
yields `0`. -/
def alu_sdiv (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let quotient := (word_divmod (word_abs a) (word_abs b)).quotient
    if ((neq_bool ((word_bit a 255) == 1#1) ((word_bit b 255) == 1#1)) : Bool)
    then (word_negate quotient)
    else quotient)

/-- `SMOD`: signed remainder, with the sign of the dividend; a zero modulus
yields `0`. -/
def alu_smod (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let remainder := (word_divmod (word_abs a) (word_abs b)).remainder
    if (((word_bit a 255) == 1#1) : Bool)
    then (word_negate remainder)
    else remainder)

/-- `ADDMOD`: `(a + b) mod n` over the unbounded integers (no intermediate
2^256 reduction); a zero modulus yields `0`. -/
def alu_addmod (a : word) (b : word) (n : word) : word :=
  if ((word_is_zero n) : Bool)
  then WORD_ZERO
  else
    (let a_reduced := (word_divmod a n).remainder
    let b_reduced := (word_divmod b n).remainder
    (word_mod_add_reduced a_reduced b_reduced n))

/-- `MULMOD`: `(a * b) mod n` over the unbounded integers; a zero modulus
yields `0`. -/
def alu_mulmod (a : word) (b : word) (n : word) : word := Id.run do
  if ((word_is_zero n) : Bool)
  then (pure WORD_ZERO)
  else
    (do
      let result : word := WORD_ZERO
      let addend : word := (word_divmod a n).remainder
      let remaining : word := b
      let (addend, remaining, result) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 255
        let mut loop_vars := (addend, remaining, result)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (addend, remaining, result) := loop_vars
          loop_vars :=
            let result : word :=
              if (((word_bit remaining 0) == 1#1) : Bool)
              then (word_mod_add_reduced result addend n)
              else result
            let addend : word := (word_mod_add_reduced addend addend n)
            let remaining : word := (word_shift_right_one remaining)
            (addend, remaining, result)
        (pure loop_vars) ) : Id (word × word × word) )
      (pure result))

/-- `EXP` via square-and-multiply over the 256 exponent bits, reduced
modulo 2^256 at every step. -/
def alu_exp (base : word) (exponent : word) : word := Id.run do
  let result : word := WORD_ONE
  let b : word := base
  let e : word := exponent
  let (b, e, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (b, e, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (b, e, result) := loop_vars
      loop_vars :=
        let result : word :=
          if (((word_bit e 0) == 1#1) : Bool)
          then (word_mul result b)
          else result
        let b : word := (word_mul b b)
        let e : word := (word_shift_right_one e)
        (b, e, result)
    (pure loop_vars) ) : Id (word × word × word) )
  (pure result)

/-- `SIGNEXTEND(byte_index, value)`: sign-extends `value` from byte
`byte_index` (0 = least significant); indices ≥ 31 leave the value
unchanged. -/
def alu_signextend (byte_index : word) (value : word) : word :=
  match (word_to_limb byte_index) with
  | .some index =>
    (if ((limb_ult index 0x0000000000000020#64) : Bool)
    then
      (let width := ((index + LIMB_ONE) <<< 3)
      let sign_shift := (width - LIMB_ONE)
      let sign_set := ((word_and (word_shift_right_limb value sign_shift) WORD_ONE) == WORD_ONE)
      let low_mask := (word_sub (word_shift_left_limb WORD_ONE width) WORD_ONE)
      if (sign_set : Bool)
      then (word_or (word_and value low_mask) (word_not low_mask))
      else (word_and value low_mask))
    else value)
  | _ => value

def alu_lt (a : word) (b : word) : word :=
  (word_of_bool (word_ult a b))

def alu_gt (a : word) (b : word) : word :=
  (word_of_bool (word_ult b a))

def alu_slt (a : word) (b : word) : word :=
  (word_of_bool (word_slt a b))

def alu_sgt (a : word) (b : word) : word :=
  (word_of_bool (word_slt b a))

def alu_eq (a : word) (b : word) : word :=
  (word_of_bool (a == b))

def alu_iszero (a : word) : word :=
  (word_of_bool (word_is_zero a))

def alu_and (a : word) (b : word) : word :=
  (word_and a b)

def alu_or (a : word) (b : word) : word :=
  (word_or a b)

def alu_xor (a : word) (b : word) : word :=
  (word_xor a b)

def alu_not (a : word) : word :=
  (word_not a)

/-- `BYTE(i, x)`: the `i`-th most-significant byte of `x` (0 = MSB);
indices ≥ 32 yield `0`. -/
def alu_byte (i : word) (x : word) : word :=
  match (word_to_limb i) with
  | .some index =>
    (if ((limb_ult index 0x0000000000000020#64) : Bool)
    then
      (let byte_offset := (0x000000000000001F#64 - index)
      let shift := (byte_offset <<< 3)
      (U256 (Sail.BitVec.zeroExtend (word_low_byte (word_shift_right_limb x shift)) 256)))
    else WORD_ZERO)
  | _ => WORD_ZERO

/-- `SHL`: logical left shift; amounts ≥ 256 yield `0`. -/
def alu_shl (shift_amt : word) (v : word) : word :=
  if ((! (word_ult shift_amt (limb_to_word 0x0000000000000100#64))) : Bool)
  then WORD_ZERO
  else (word_shift_left_limb v (word_limb_0 shift_amt))

/-- `SHR`: logical right shift; amounts ≥ 256 yield `0`. -/
def alu_shr (shift_amt : word) (v : word) : word :=
  if ((! (word_ult shift_amt (limb_to_word 0x0000000000000100#64))) : Bool)
  then WORD_ZERO
  else (word_shift_right_limb v (word_limb_0 shift_amt))

/-- `SAR`: arithmetic (sign-propagating) right shift, via floored signed
division. -/
def alu_sar (shift_amt : word) (v : word) : word :=
  if ((! (word_ult shift_amt (limb_to_word 0x0000000000000100#64))) : Bool)
  then
    (if (((word_bit v 255) == 1#1) : Bool)
    then WORD_ALL_ONES
    else WORD_ZERO)
  else (word_arithmetic_shift_right_limb v (word_limb_0 shift_amt))

/-- `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). -/
def alu_clz (x : word) : SailM word := do
  let count : Nat := 0
  let found : Bool := false
  let (count, found) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 255
    let mut loop_vars := (count, found)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (count, found) := loop_vars
      loop_vars ← do
        let biinput_index := (255 -i i)
        let (count, found) ← (( do
          if ((! found) : Bool)
          then
            (do
              let (count, found) ← (( do
                if (((word_bit x biinput_index) == 0#1) : Bool)
                then
                  (do
                    let count ←
                      (do
                          let semanticResult ← (word_bit_count_increment ⟨count⟩)
                          pure ((semanticResult).value))
                    (pure (count, found)))
                else
                  (let found : Bool := true
                  (pure (count, found))) ) : SailM (Nat × Bool) )
              (pure (count, found)))
          else (pure (count, found)) ) : SailM (Nat × Bool) )
        (pure (count, found))
    (pure loop_vars) ) : SailM (Nat × Bool) )
  (pure (limb_to_word (get_slice_int 64 count 0)))

