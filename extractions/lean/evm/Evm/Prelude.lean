import Evm.Flow

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

noncomputable section
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
open Register
open NodeRef
open MerkleSlot
open JEntry
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
  (pure { quotient := ← (undefined_bitvector 64)
          remainder := ← (undefined_bitvector 64) })

def undefined_WordDivMod (_ : Unit) : SailM WordDivMod := do
  (pure { quotient := ← (undefined_bitvector 256)
          remainder := ← (undefined_bitvector 256) })

def ZERO_WORD : word := 0x0000000000000000000000000000000000000000000000000000000000000000#256

def ZERO_ADDR : address := 0x0000000000000000000000000000000000000000#160

def WORD_ZERO : word := 0x0000000000000000000000000000000000000000000000000000000000000000#256

def WORD_ONE : word := 0x0000000000000000000000000000000000000000000000000000000000000001#256

def WORD_ALL_ONES : word := 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#256

def WORD_SIGN_BIT : word := 0x8000000000000000000000000000000000000000000000000000000000000000#256

/-- Type quantifiers: k_ex159621_ : Bool -/
def word_of_bool (b : Bool) : (BitVec 256) :=
  if (b : Bool)
  then WORD_ONE
  else WORD_ZERO

def word_is_zero (w : (BitVec 256)) : Bool :=
  (w == WORD_ZERO)

def word_nonzero (w : (BitVec 256)) : Bool :=
  (! (word_is_zero w))

def LIMB_ZERO : limb := 0x0000000000000000#64

def LIMB_ONE : limb := 0x0000000000000001#64

def LIMB_MAX : limb := 0xFFFFFFFFFFFFFFFF#64

def limb_ult (a : (BitVec 64)) (b : (BitVec 64)) : Bool :=
  let difference := (a - b)
  let borrow :=
    (((Complement.complement a) &&& b) ||| ((Complement.complement (a ^^^ b)) &&& difference))
  ((borrow &&& 0x8000000000000000#64) != LIMB_ZERO)

def limb_ule (a : (BitVec 64)) (b : (BitVec 64)) : Bool :=
  (! (limb_ult b a))

def limb_checked_add (a : (BitVec 64)) (b : (BitVec 64)) : (Option (BitVec 64)) :=
  let result := (a + b)
  if ((limb_ult result a) : Bool)
  then none
  else (some result)

def limb_checked_sub (a : (BitVec 64)) (b : (BitVec 64)) : (Option (BitVec 64)) :=
  if ((limb_ult a b) : Bool)
  then none
  else (some (a - b))

def limb_checked_mul (value : (BitVec 64)) (factor : (BitVec 64)) : (Option (BitVec 64)) := Id.run do
  let result : limb := LIMB_ZERO
  let addend : limb := value
  let remaining : limb := factor
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
                (let result : limb := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining : limb := (remaining >>> 1)
        let (addend, valid) : ((BitVec 64) × Bool) :=
          if ((valid && ((i <b 63) && (remaining != LIMB_ZERO))) : Bool)
          then
            (let doubled := (limb_checked_add addend addend)
            let (addend, valid) : ((BitVec 64) × Bool) :=
              match doubled with
              | .some value =>
                (let addend : limb := value
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

def limb_divmod (dividend : (BitVec 64)) (divisor : (BitVec 64)) : LimbDivMod := Id.run do
  if ((divisor == LIMB_ZERO) : Bool)
  then
    (pure { quotient := LIMB_ZERO
            remainder := LIMB_ZERO })
  else
    (do
      let quotient : limb := LIMB_ZERO
      let remainder : limb := LIMB_ZERO
      let remaining : limb := dividend
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
            let remainder : limb := ((remainder <<< 1) ||| incoming)
            let remaining : limb := (remaining <<< 1)
            let quotient : limb := (quotient <<< 1)
            let (quotient, remainder) : ((BitVec 64) × (BitVec 64)) :=
              if ((overflow || (limb_ule divisor remainder)) : Bool)
              then
                (let remainder : limb := (remainder - divisor)
                let quotient : limb := (quotient ||| LIMB_ONE)
                (quotient, remainder))
              else (quotient, remainder)
            (quotient, remainder, remaining)
        (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 64) × (BitVec 64)) )
      (pure { quotient := quotient
              remainder := remainder }))

/-- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 256 -/
def word_bit_count_increment (value : Nat) : SailM Nat := do
  if ((value <b 256) : Bool)
  then (pure (value +i 1))
  else
    (do
      assert false "sail/prelude.sail:155.20-155.21"
      throw Error.Exit)

def limb_bit_length (value : (BitVec 64)) : Nat := Id.run do
  let remaining : limb := value
  let length : limb_bit_count := 0
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
            (let length : limb_bit_count := (step +i 1)
            let remaining : limb := (remaining >>> 1)
            (length, remaining))
          else (length, remaining)
        (length, remaining)
    (pure loop_vars) ) : Id (Nat × (BitVec 64)) )
  (pure length)

def word_to_limb (w : (BitVec 256)) : (Option (BitVec 64)) :=
  if (((Sail.BitVec.extractLsb w 255 64) == (BitVec.zero 192)) : Bool)
  then (some (Sail.BitVec.extractLsb w 63 0))
  else none

def limb_to_word (value : (BitVec 64)) : (BitVec 256) :=
  (Sail.BitVec.zeroExtend value 256)

/-- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_nat (value : Nat) : SailM (BitVec 256) := do
  assert (value <b (2 ^i 256)) "sail/prelude.sail:186.26-186.27"
  (pure (get_slice_int 256 value 0))

def word_ult (a : (BitVec 256)) (b : (BitVec 256)) : Bool :=
  if (((Sail.BitVec.extractLsb a 255 192) != (Sail.BitVec.extractLsb b 255 192)) : Bool)
  then (limb_ult (Sail.BitVec.extractLsb a 255 192) (Sail.BitVec.extractLsb b 255 192))
  else
    (if (((Sail.BitVec.extractLsb a 191 128) != (Sail.BitVec.extractLsb b 191 128)) : Bool)
    then (limb_ult (Sail.BitVec.extractLsb a 191 128) (Sail.BitVec.extractLsb b 191 128))
    else
      (if (((Sail.BitVec.extractLsb a 127 64) != (Sail.BitVec.extractLsb b 127 64)) : Bool)
      then (limb_ult (Sail.BitVec.extractLsb a 127 64) (Sail.BitVec.extractLsb b 127 64))
      else (limb_ult (Sail.BitVec.extractLsb a 63 0) (Sail.BitVec.extractLsb b 63 0))))

def word_ule (a : (BitVec 256)) (b : (BitVec 256)) : Bool :=
  (! (word_ult b a))

def word_bit_length (value : (BitVec 256)) : Nat :=
  if (((Sail.BitVec.extractLsb value 255 192) != LIMB_ZERO) : Bool)
  then (192 +i (limb_bit_length (Sail.BitVec.extractLsb value 255 192)))
  else
    (if (((Sail.BitVec.extractLsb value 191 128) != LIMB_ZERO) : Bool)
    then (128 +i (limb_bit_length (Sail.BitVec.extractLsb value 191 128)))
    else
      (if (((Sail.BitVec.extractLsb value 127 64) != LIMB_ZERO) : Bool)
      then (64 +i (limb_bit_length (Sail.BitVec.extractLsb value 127 64)))
      else (limb_bit_length (Sail.BitVec.extractLsb value 63 0))))

def word_checked_add (a : (BitVec 256)) (b : (BitVec 256)) : (Option (BitVec 256)) :=
  let result := (a + b)
  if ((word_ult result a) : Bool)
  then none
  else (some result)

def word_checked_mul_limb (value : (BitVec 256)) (factor : (BitVec 64)) : (Option (BitVec 256)) := Id.run do
  let result : word := ZERO_WORD
  let addend : word := value
  let remaining : limb := factor
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
                (let result : word := value
                (result, valid))
              | none =>
                (let valid : Bool := false
                (result, valid))
            (result, valid))
          else (result, valid)
        let remaining : limb := (remaining >>> 1)
        let (addend, valid) : ((BitVec 256) × Bool) :=
          if ((valid && ((i <b 63) && (remaining != LIMB_ZERO))) : Bool)
          then
            (let doubled := (word_checked_add addend addend)
            let (addend, valid) : ((BitVec 256) × Bool) :=
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
    (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 64) × (BitVec 256) × Bool) )
  if (valid : Bool)
  then (pure (some result))
  else (pure none)

def word_mul (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) := Id.run do
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
        let result : (BitVec 256) :=
          if (((BitVec.access remaining 0) == 1#1) : Bool)
          then (result + addend)
          else result
        let addend : word := (addend <<< 1)
        let remaining : word := (remaining >>> 1)
        (addend, remaining, result)
    (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
  (pure result)

def word_divmod (dividend : (BitVec 256)) (divisor : (BitVec 256)) : WordDivMod := Id.run do
  if ((word_is_zero divisor) : Bool)
  then
    (pure { quotient := WORD_ZERO
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
              if (((BitVec.access remaining 255) == 1#1) : Bool)
              then WORD_ONE
              else WORD_ZERO
            let overflow := ((BitVec.access remainder 255) == 1#1)
            let remainder : word := ((remainder <<< 1) ||| incoming)
            let remaining : word := (remaining <<< 1)
            let quotient : word := (quotient <<< 1)
            let (quotient, remainder) : ((BitVec 256) × (BitVec 256)) :=
              if ((overflow || (word_ule divisor remainder)) : Bool)
              then
                (let remainder : word := (remainder - divisor)
                let quotient : word := (quotient ||| WORD_ONE)
                (quotient, remainder))
              else (quotient, remainder)
            (quotient, remainder, remaining)
        (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
      (pure { quotient := quotient
              remainder := remainder }))

def word_mod_add_reduced (a : (BitVec 256)) (b : (BitVec 256)) (modulus : (BitVec 256)) : (BitVec 256) :=
  let threshold := (modulus - b)
  if ((word_ule threshold a) : Bool)
  then (a - threshold)
  else (a + b)

def word_shift_left_limb (value : (BitVec 256)) (amount : (BitVec 64)) : (BitVec 256) := Id.run do
  let result : word := value
  let remaining : limb := amount
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
            (let result : word := (result <<< 1)
            let remaining : limb := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 256)) )
  (pure result)

def word_shift_right_limb (value : (BitVec 256)) (amount : (BitVec 64)) : (BitVec 256) := Id.run do
  let result : word := value
  let remaining : limb := amount
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
            (let result : word := (result >>> 1)
            let remaining : limb := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 256)) )
  (pure result)

def word_arithmetic_shift_right_limb (value : (BitVec 256)) (amount : (BitVec 64)) : (BitVec 256) := Id.run do
  let result : word := value
  let remaining : limb := amount
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
            (let result : word := (result >>> 1)
            let result : (BitVec 256) :=
              if (negative : Bool)
              then (result ||| WORD_SIGN_BIT)
              else result
            let remaining : limb := (remaining - LIMB_ONE)
            (remaining, result))
          else (remaining, result)
        (remaining, result)
    (pure loop_vars) ) : Id ((BitVec 64) × (BitVec 256)) )
  (pure result)

def word_negate (value : (BitVec 256)) : (BitVec 256) :=
  (WORD_ZERO - value)

def word_abs (value : (BitVec 256)) : (BitVec 256) :=
  if (((BitVec.access value 255) == 1#1) : Bool)
  then (word_negate value)
  else value

def word_slt (a : (BitVec 256)) (b : (BitVec 256)) : Bool :=
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

def word_to_address (w : (BitVec 256)) : (BitVec 160) :=
  (Sail.BitVec.extractLsb w 159 0)

def address_to_word (a : (BitVec 160)) : (BitVec 256) :=
  (Sail.BitVec.zeroExtend a 256)

def alu_add (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (a + b)

def alu_sub (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (a - b)

def alu_mul (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (word_mul a b)

def alu_div (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else (word_divmod a b).quotient

def alu_mod (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else (word_divmod a b).remainder

def alu_sdiv (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let quotient := (word_divmod (word_abs a) (word_abs b)).quotient
    if ((neq_bool ((BitVec.access a 255) == 1#1) ((BitVec.access b 255) == 1#1)) : Bool)
    then (word_negate quotient)
    else quotient)

def alu_smod (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  if ((word_is_zero b) : Bool)
  then WORD_ZERO
  else
    (let remainder := (word_divmod (word_abs a) (word_abs b)).remainder
    if (((BitVec.access a 255) == 1#1) : Bool)
    then (word_negate remainder)
    else remainder)

def alu_addmod (a : (BitVec 256)) (b : (BitVec 256)) (n : (BitVec 256)) : (BitVec 256) :=
  if ((word_is_zero n) : Bool)
  then WORD_ZERO
  else
    (let a_reduced := (word_divmod a n).remainder
    let b_reduced := (word_divmod b n).remainder
    (word_mod_add_reduced a_reduced b_reduced n))

def alu_mulmod (a : (BitVec 256)) (b : (BitVec 256)) (n : (BitVec 256)) : (BitVec 256) := Id.run do
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
            let result : (BitVec 256) :=
              if (((BitVec.access remaining 0) == 1#1) : Bool)
              then (word_mod_add_reduced result addend n)
              else result
            let addend : word := (word_mod_add_reduced addend addend n)
            let remaining : word := (remaining >>> 1)
            (addend, remaining, result)
        (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
      (pure result))

def alu_exp (base : (BitVec 256)) (exponent : (BitVec 256)) : (BitVec 256) := Id.run do
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
        let result : (BitVec 256) :=
          if (((BitVec.access e 0) == 1#1) : Bool)
          then (word_mul result b)
          else result
        let b : word := (word_mul b b)
        let e : word := (e >>> 1)
        (b, e, result)
    (pure loop_vars) ) : Id ((BitVec 256) × (BitVec 256) × (BitVec 256)) )
  (pure result)

def alu_signextend (byte_index : (BitVec 256)) (value : (BitVec 256)) : (BitVec 256) :=
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

def alu_lt (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (word_of_bool (word_ult a b))

def alu_gt (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (word_of_bool (word_ult b a))

def alu_slt (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (word_of_bool (word_slt a b))

def alu_sgt (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (word_of_bool (word_slt b a))

def alu_eq (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (word_of_bool (a == b))

def alu_iszero (a : (BitVec 256)) : (BitVec 256) :=
  (word_of_bool (word_is_zero a))

def alu_and (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (a &&& b)

def alu_or (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (a ||| b)

def alu_xor (a : (BitVec 256)) (b : (BitVec 256)) : (BitVec 256) :=
  (a ^^^ b)

def alu_not (a : (BitVec 256)) : (BitVec 256) :=
  (Complement.complement a)

def alu_byte (i : (BitVec 256)) (x : (BitVec 256)) : (BitVec 256) :=
  match (word_to_limb i) with
  | .some index =>
    (if ((limb_ult index 0x0000000000000020#64) : Bool)
    then
      (let byte_offset := (0x000000000000001F#64 - index)
      let shift := (byte_offset <<< 3)
      (Sail.BitVec.zeroExtend (Sail.BitVec.extractLsb (word_shift_right_limb x shift) 7 0) 256))
    else WORD_ZERO)
  | _ => WORD_ZERO

def alu_shl (shift_amt : (BitVec 256)) (v : (BitVec 256)) : (BitVec 256) :=
  if ((! (word_ult shift_amt 0x0000000000000000000000000000000000000000000000000000000000000100#256)) : Bool)
  then WORD_ZERO
  else (word_shift_left_limb v (Sail.BitVec.extractLsb shift_amt 63 0))

def alu_shr (shift_amt : (BitVec 256)) (v : (BitVec 256)) : (BitVec 256) :=
  if ((! (word_ult shift_amt 0x0000000000000000000000000000000000000000000000000000000000000100#256)) : Bool)
  then WORD_ZERO
  else (word_shift_right_limb v (Sail.BitVec.extractLsb shift_amt 63 0))

def alu_sar (shift_amt : (BitVec 256)) (v : (BitVec 256)) : (BitVec 256) :=
  if ((! (word_ult shift_amt 0x0000000000000000000000000000000000000000000000000000000000000100#256)) : Bool)
  then
    (if (((BitVec.access v 255) == 1#1) : Bool)
    then WORD_ALL_ONES
    else WORD_ZERO)
  else (word_arithmetic_shift_right_limb v (Sail.BitVec.extractLsb shift_amt 63 0))

def alu_clz (x : (BitVec 256)) : SailM (BitVec 256) := do
  let count : word_bit_count := 0
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
                    let count ← (word_bit_count_increment count)
                    (pure (count, found)))
                else
                  (let found : Bool := true
                  (pure (count, found))) ) : SailM (Nat × Bool) )
              (pure (count, found)))
          else (pure (count, found)) ) : SailM (Nat × Bool) )
        (pure (count, found))
    (pure loop_vars) ) : SailM (Nat × Bool) )
  (word_of_nat count)

