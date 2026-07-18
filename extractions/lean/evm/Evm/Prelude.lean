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

open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
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

def undefined_WordDivMod (_ : Unit) : SailM WordDivMod := do
  (pure { quotient := ← (undefined_bitvector 256),
          remainder := ← (undefined_bitvector 256) })

def ZERO_WORD : word := 0x0000000000000000000000000000000000000000000000000000000000000000#256

def ZERO_ADDR : address := 0x0000000000000000000000000000000000000000#160

def WORD_ZERO : word := 0x0000000000000000000000000000000000000000000000000000000000000000#256

def WORD_ONE : word := 0x0000000000000000000000000000000000000000000000000000000000000001#256

def WORD_ALL_ONES : word := 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#256

def WORD_SIGN_BIT : word := 0x8000000000000000000000000000000000000000000000000000000000000000#256

/-- Type quantifiers: k_ex160742_ : Bool -/
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

def limb_ult (a : limb) (b : limb) : Bool :=
  let difference := (a - b)
  let borrow :=
    (((Complement.complement a) &&& b) ||| ((Complement.complement (a ^^^ b)) &&& difference))
  ((borrow &&& 0x8000000000000000#64) != LIMB_ZERO)

def limb_ule (a : limb) (b : limb) : Bool :=
  (! (limb_ult b a))

def limb_checked_add (a : limb) (b : limb) : (Option limb) :=
  let result := (a + b)
  if ((limb_ult result a) : Bool)
  then none
  else (some result)

def limb_checked_sub (a : limb) (b : limb) : (Option limb) :=
  if ((limb_ult a b) : Bool)
  then none
  else (some (a - b))

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

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 256 -/
def word_bit_count_increment (value : word_bit_count) : SailM word_bit_count := do
  let value := (value).value
  let semanticResult ← do
    if ((value <b 256) : Bool)
    then (pure (value + 1))
    else
      (do
        assert false "sail/prelude.sail:188.20-188.21"
        throw Error.Exit)
  pure (⟨semanticResult⟩)

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

def word_to_limb (w : word) : (Option limb) :=
  if (((Sail.BitVec.extractLsb w 255 64) == (BitVec.zero 192)) : Bool)
  then (some (Sail.BitVec.extractLsb w 63 0))
  else none

def limb_to_word (value : limb) : word :=
  (Sail.BitVec.zeroExtend value 256)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_nat (value : Nat) : SailM word := do
  assert (value <b (2 ^i 256)) "sail/prelude.sail:222.26-222.27"
  (pure (get_slice_int 256 value 0))

def word_ult (a : word) (b : word) : Bool :=
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

def word_bit_length (value : word) : word_bit_count :=
  ⟨if (((Sail.BitVec.extractLsb value 255 192) != LIMB_ZERO) : Bool)
  then (192 + ((limb_bit_length (Sail.BitVec.extractLsb value 255 192))).value)
  else
    (if (((Sail.BitVec.extractLsb value 191 128) != LIMB_ZERO) : Bool)
    then (128 + ((limb_bit_length (Sail.BitVec.extractLsb value 191 128))).value)
    else
      (if (((Sail.BitVec.extractLsb value 127 64) != LIMB_ZERO) : Bool)
      then (64 + ((limb_bit_length (Sail.BitVec.extractLsb value 127 64))).value)
      else ((limb_bit_length (Sail.BitVec.extractLsb value 63 0))).value))⟩

def word_checked_add (a : word) (b : word) : (Option word) :=
  let result := (a + b)
  if ((word_ult result a) : Bool)
  then none
  else (some result)

def word_checked_mul_limb (value : word) (factor : limb) : (Option word) := Id.run do
  let result : (BitVec 256) := ZERO_WORD
  let addend : (BitVec 256) := value
  let remaining : (BitVec 64) := factor
  let valid : Bool := true
  let (addend, remaining, result, valid) ← (( do
    let loop_i_lower := 0
    let loop_i_upper := 63
    let mut loop_vars := (addend, remaining, result, valid)
    for i in [loop_i_lower:loop_i_upper:1]i do
      let (addend, remaining, result, valid) := loop_vars
      loop_vars :=
        let (result, valid) : ((BitVec 256) × Bool) :=
          if ((valid && ((remaining &&& LIMB_ONE) == LIMB_ONE)) : Bool)
          then
            (let added := (word_checked_add result addend)
            let (result, valid) : ((BitVec 256) × Bool) :=
              match added with
              | .some value =>
                (let result : (BitVec 256) := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining : (BitVec 64) := (remaining >>> 1)
        let (addend, valid) : ((BitVec 256) × Bool) :=
          if ((valid && ((i <b 63) && (remaining != LIMB_ZERO))) : Bool)
          then
            (let doubled := (word_checked_add addend addend)
            let (addend, valid) : ((BitVec 256) × Bool) :=
              match doubled with
              | .some value =>
                (let addend : (BitVec 256) := value
                (addend, valid))
              | none =>
                (let valid : Bool := false
                (addend, valid))
            (addend, valid))
          else (addend, valid)
        (addend, remaining, result, valid)
    (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 64) × (BitVec 256) × Bool) )
  if (valid : Bool)
  then (pure (some result))
  else (pure none)

def word_mul (a : word) (b : word) : word := Id.run do
  let result : (BitVec 256) := WORD_ZERO
  let addend : (BitVec 256) := a
  let remaining : (BitVec 256) := b
  let (addend, remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (addend, remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (addend, remaining, result) := loop_vars
      loop_vars :=
        let result : (BitVec 256) :=
          if (((BitVec.access remaining 0) == 1#1) : Bool)
          then (result + addend)
          else result
        let addend : (BitVec 256) := (addend <<< 1)
        let remaining : (BitVec 256) := (remaining >>> 1)
        (addend, remaining, result)
    (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
  (pure result)

def word_divmod (dividend : word) (divisor : word) : WordDivMod := Id.run do
  if ((word_is_zero divisor) : Bool)
  then
    (pure { quotient := WORD_ZERO,
            remainder := WORD_ZERO })
  else
    (do
      let quotient : (BitVec 256) := WORD_ZERO
      let remainder : (BitVec 256) := WORD_ZERO
      let remaining : (BitVec 256) := dividend
      let (quotient, remainder, remaining) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 255
        let mut loop_vars := (quotient, remainder, remaining)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (quotient, remainder, remaining) := loop_vars
          loop_vars :=
            let incoming :=
              if (((BitVec.access remaining 255) == 1#1) : Bool)
              then WORD_ONE
              else WORD_ZERO
            let overflow := ((BitVec.access remainder 255) == 1#1)
            let remainder : (BitVec 256) := ((remainder <<< 1) ||| incoming)
            let remaining : (BitVec 256) := (remaining <<< 1)
            let quotient : (BitVec 256) := (quotient <<< 1)
            let (quotient, remainder) : ((BitVec 256) × (BitVec 256)) :=
              if ((overflow || (word_ule divisor remainder)) : Bool)
              then
                (let remainder : (BitVec 256) := (remainder - divisor)
                let quotient : (BitVec 256) := (quotient ||| WORD_ONE)
                (quotient, remainder))
              else (quotient, remainder)
            (quotient, remainder, remaining)
        (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
      (pure { quotient := quotient,
              remainder := remainder }))

def word_mod_add_reduced (a : word) (b : word) (modulus : word) : word :=
  let threshold := (modulus - b)
  if ((word_ule threshold a) : Bool)
  then (a - threshold)
  else (a + b)

def word_shift_left_limb (value : word) (amount : limb) : word := Id.run do
  let result : (BitVec 256) := value
  let remaining : (BitVec 64) := amount
  let (remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (remaining, result) := loop_vars
      loop_vars :=
        let (remaining, result) : ((BitVec 64) × (BitVec 256)) :=
          if ((remaining != LIMB_ZERO) : Bool)
          then
            (let result : (BitVec 256) := (result <<< 1)
            let remaining : (BitVec 64) := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 256)) )
  (pure result)

def word_shift_right_limb (value : word) (amount : limb) : word := Id.run do
  let result : (BitVec 256) := value
  let remaining : (BitVec 64) := amount
  let (remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (remaining, result) := loop_vars
      loop_vars :=
        let (remaining, result) : ((BitVec 64) × (BitVec 256)) :=
          if ((remaining != LIMB_ZERO) : Bool)
          then
            (let result : (BitVec 256) := (result >>> 1)
            let remaining : (BitVec 64) := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 256)) )
  (pure result)

def word_arithmetic_shift_right_limb (value : word) (amount : limb) : word := Id.run do
  let result : (BitVec 256) := value
  let remaining : (BitVec 64) := amount
  let negative := ((BitVec.access value 255) == 1#1)
  let (remaining, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (remaining, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (remaining, result) := loop_vars
      loop_vars :=
        let (remaining, result) : ((BitVec 64) × (BitVec 256)) :=
          if ((remaining != LIMB_ZERO) : Bool)
          then
            (let result : (BitVec 256) := (result >>> 1)
            let result : (BitVec 256) :=
              if (negative : Bool)
              then (result ||| WORD_SIGN_BIT)
              else result
            let remaining : (BitVec 64) := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 256)) )
  (pure result)

def word_negate (value : word) : word :=
  (WORD_ZERO - value)

def word_abs (value : word) : word :=
  if (((BitVec.access value 255) == 1#1) : Bool)
  then (word_negate value)
  else value

def word_slt (a : word) (b : word) : Bool :=
  let a_neg := ((BitVec.access a 255) == 1#1)
  let b_neg := ((BitVec.access b 255) == 1#1)
  if (a_neg : Bool)
  then
    (if (b_neg : Bool)
    then (word_ult a b)
    else true)
  else
    (if (b_neg : Bool)
    then false
    else (word_ult a b))

def word_to_address (w : word) : address :=
  (Sail.BitVec.extractLsb w 159 0)

def address_to_word (a : address) : word :=
  (Sail.BitVec.zeroExtend a 256)

def alu_add (a : word) (b : word) : word :=
  (a + b)

def alu_sub (a : word) (b : word) : word :=
  (a - b)

def alu_mul (a : word) (b : word) : word :=
  (word_mul a b)

def alu_div (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else (word_divmod a b).quotient

def alu_mod (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else (word_divmod a b).remainder

def alu_sdiv (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let quotient := (word_divmod (word_abs a) (word_abs b)).quotient
    if ((neq_bool ((BitVec.access a 255) == 1#1) ((BitVec.access b 255) == 1#1)) : Bool)
    then (word_negate quotient)
    else quotient)

def alu_smod (a : word) (b : word) : word :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let remainder := (word_divmod (word_abs a) (word_abs b)).remainder
    if (((BitVec.access a 255) == 1#1) : Bool)
    then (word_negate remainder)
    else remainder)

def alu_addmod (a : word) (b : word) (n : word) : word :=
  if ((word_is_zero n) : Bool)
  then WORD_ZERO
  else
    (let a_reduced := (word_divmod a n).remainder
    let b_reduced := (word_divmod b n).remainder
    (word_mod_add_reduced a_reduced b_reduced n))

def alu_mulmod (a : word) (b : word) (n : word) : word := Id.run do
  if ((word_is_zero n) : Bool)
  then (pure WORD_ZERO)
  else
    (do
      let result : (BitVec 256) := WORD_ZERO
      let addend : (BitVec 256) := (word_divmod a n).remainder
      let remaining : (BitVec 256) := b
      let (addend, remaining, result) ← (( do
        let loop__step_lower := 0
        let loop__step_upper := 255
        let mut loop_vars := (addend, remaining, result)
        for _step in [loop__step_lower:loop__step_upper:1]i do
          let (addend, remaining, result) := loop_vars
          loop_vars :=
            let result : (BitVec 256) :=
              if (((BitVec.access remaining 0) == 1#1) : Bool)
              then (word_mod_add_reduced result addend n)
              else result
            let addend : (BitVec 256) := (word_mod_add_reduced addend addend n)
            let remaining : (BitVec 256) := (remaining >>> 1)
            (addend, remaining, result)
        (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
      (pure result))

def alu_exp (base : word) (exponent : word) : word := Id.run do
  let result : (BitVec 256) := WORD_ONE
  let b : (BitVec 256) := base
  let e : (BitVec 256) := exponent
  let (b, e, result) ← (( do
    let loop__step_lower := 0
    let loop__step_upper := 255
    let mut loop_vars := (b, e, result)
    for _step in [loop__step_lower:loop__step_upper:1]i do
      let (b, e, result) := loop_vars
      loop_vars :=
        let result : (BitVec 256) :=
          if (((BitVec.access e 0) == 1#1) : Bool)
          then (word_mul result b)
          else result
        let b : (BitVec 256) := (word_mul b b)
        let e : (BitVec 256) := (e >>> 1)
        (b, e, result)
    (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
  (pure result)

def alu_signextend (byte_index : word) (value : word) : word :=
  match (word_to_limb byte_index) with
  | .some index =>
    (if ((limb_ult index 0x0000000000000020#64) : Bool)
    then
      (let width := ((index + LIMB_ONE) <<< 3)
      let sign_shift := (width - LIMB_ONE)
      let sign_set := (((word_shift_right_limb value sign_shift) &&& WORD_ONE) == WORD_ONE)
      let low_mask := ((word_shift_left_limb WORD_ONE width) - WORD_ONE)
      if (sign_set : Bool)
      then ((value &&& low_mask) ||| (Complement.complement low_mask))
      else (value &&& low_mask))
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
  (a &&& b)

def alu_or (a : word) (b : word) : word :=
  (a ||| b)

def alu_xor (a : word) (b : word) : word :=
  (a ^^^ b)

def alu_not (a : word) : word :=
  (Complement.complement a)

def alu_byte (i : word) (x : word) : word :=
  match (word_to_limb i) with
  | .some index =>
    (if ((limb_ult index 0x0000000000000020#64) : Bool)
    then
      (let byte_offset := (0x000000000000001F#64 - index)
      let shift := (byte_offset <<< 3)
      (Sail.BitVec.zeroExtend (Sail.BitVec.extractLsb (word_shift_right_limb x shift) 7 0) 256))
    else WORD_ZERO)
  | _ => WORD_ZERO

def alu_shl (shift_amt : word) (v : word) : word :=
  if ((! (word_ult shift_amt 0x0000000000000000000000000000000000000000000000000000000000000100#256)) : Bool)
  then WORD_ZERO
  else (word_shift_left_limb v (Sail.BitVec.extractLsb shift_amt 63 0))

def alu_shr (shift_amt : word) (v : word) : word :=
  if ((! (word_ult shift_amt 0x0000000000000000000000000000000000000000000000000000000000000100#256)) : Bool)
  then WORD_ZERO
  else (word_shift_right_limb v (Sail.BitVec.extractLsb shift_amt 63 0))

def alu_sar (shift_amt : word) (v : word) : word :=
  if ((! (word_ult shift_amt 0x0000000000000000000000000000000000000000000000000000000000000100#256)) : Bool)
  then
    (if (((BitVec.access v 255) == 1#1) : Bool)
    then WORD_ALL_ONES
    else WORD_ZERO)
  else (word_arithmetic_shift_right_limb v (Sail.BitVec.extractLsb shift_amt 63 0))

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
                if (((BitVec.access x biinput_index) == 0#1) : Bool)
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
  (word_of_nat count)

