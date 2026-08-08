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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def u256 (value : Nat) : Nat :=
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
  (BitVec.toNatInt
    ((GetElem?.getElem! bytes 0) +++ ((GetElem?.getElem! bytes 1) +++ ((GetElem?.getElem! bytes 2) +++ ((GetElem?.getElem!
              bytes 3) +++ ((GetElem?.getElem! bytes 4) +++ ((GetElem?.getElem! bytes 5) +++ ((GetElem?.getElem!
                    bytes 6) +++ ((GetElem?.getElem! bytes 7) +++ ((GetElem?.getElem! bytes 8) +++ ((GetElem?.getElem!
                          bytes 9) +++ ((GetElem?.getElem! bytes 10) +++ ((GetElem?.getElem! bytes
                              11) +++ ((GetElem?.getElem! bytes 12) +++ ((GetElem?.getElem! bytes 13) +++ ((GetElem?.getElem!
                                    bytes 14) +++ ((GetElem?.getElem! bytes 15) +++ ((GetElem?.getElem!
                                        bytes 16) +++ ((GetElem?.getElem! bytes 17) +++ ((GetElem?.getElem!
                                            bytes 18) +++ ((GetElem?.getElem! bytes 19) +++ ((GetElem?.getElem!
                                                bytes 20) +++ ((GetElem?.getElem! bytes 21) +++ ((GetElem?.getElem!
                                                    bytes 22) +++ ((GetElem?.getElem! bytes 23) +++ ((GetElem?.getElem!
                                                        bytes 24) +++ ((GetElem?.getElem! bytes 25) +++ ((GetElem?.getElem!
                                                            bytes 26) +++ ((GetElem?.getElem! bytes
                                                              27) +++ ((GetElem?.getElem! bytes 28) +++ ((GetElem?.getElem!
                                                                  bytes 29) +++ ((GetElem?.getElem!
                                                                    bytes 30) +++ (GetElem?.getElem!
                                                                    bytes 31)))))))))))))))))))))))))))))))))

/-- Interprets one full-width bitvector as an EVM word. This named boundary
keeps protocol constants readable without hiding conversion chains. -/
def word_from_bits (value : (BitVec 256)) : Nat :=
  let natural_value := (BitVec.toNatInt value)
  (u256 natural_value)

/-- Serializes an EVM word as a 32-byte big-endian digest. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_hash (value : Nat) : (Vector (BitVec 8) 32) :=
  let zero_bytes := (vectorInit 0x00#8)
  let result : (Vector (BitVec 8) 32) := (B256 zero_bytes)
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 0 (get_slice_int 8 value 248))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 1 (get_slice_int 8 value 240))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 2 (get_slice_int 8 value 232))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 3 (get_slice_int 8 value 224))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 4 (get_slice_int 8 value 216))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 5 (get_slice_int 8 value 208))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 6 (get_slice_int 8 value 200))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 7 (get_slice_int 8 value 192))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 8 (get_slice_int 8 value 184))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 9 (get_slice_int 8 value 176))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 10 (get_slice_int 8 value 168))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 11 (get_slice_int 8 value 160))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 12 (get_slice_int 8 value 152))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 13 (get_slice_int 8 value 144))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 14 (get_slice_int 8 value 136))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 15 (get_slice_int 8 value 128))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 16 (get_slice_int 8 value 120))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 17 (get_slice_int 8 value 112))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 18 (get_slice_int 8 value 104))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 19 (get_slice_int 8 value 96))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 20 (get_slice_int 8 value 88))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 21 (get_slice_int 8 value 80))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 22 (get_slice_int 8 value 72))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 23 (get_slice_int 8 value 64))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 24 (get_slice_int 8 value 56))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 25 (get_slice_int 8 value 48))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 26 (get_slice_int 8 value 40))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 27 (get_slice_int 8 value 32))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 28 (get_slice_int 8 value 24))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 29 (get_slice_int 8 value 16))
  let result : (Vector (BitVec 8) 32) := (vectorUpdate result 30 (get_slice_int 8 value 8))
  (vectorUpdate result 31 (get_slice_int 8 value 0))

/-- Converts a word to its low 160-bit address in canonical byte order. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_address (value : Nat) : (Vector (BitVec 8) 20) :=
  let zero_bytes := (vectorInit 0x00#8)
  let result : (Vector (BitVec 8) 20) := (Address zero_bytes)
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 0 (get_slice_int 8 value 152))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 1 (get_slice_int 8 value 144))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 2 (get_slice_int 8 value 136))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 3 (get_slice_int 8 value 128))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 4 (get_slice_int 8 value 120))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 5 (get_slice_int 8 value 112))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 6 (get_slice_int 8 value 104))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 7 (get_slice_int 8 value 96))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 8 (get_slice_int 8 value 88))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 9 (get_slice_int 8 value 80))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 10 (get_slice_int 8 value 72))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 11 (get_slice_int 8 value 64))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 12 (get_slice_int 8 value 56))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 13 (get_slice_int 8 value 48))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 14 (get_slice_int 8 value 40))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 15 (get_slice_int 8 value 32))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 16 (get_slice_int 8 value 24))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 17 (get_slice_int 8 value 16))
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 18 (get_slice_int 8 value 8))
  (vectorUpdate result 19 (get_slice_int 8 value 0))

/-- Interprets canonical full-width bits directly as a digest. -/
def hash_from_bits (value : (BitVec 256)) : (Vector (BitVec 8) 32) :=
  let word_value := (word_from_bits value)
  (word_to_hash word_value)

/-- Interprets the low 160 bits of full-width bits as an address. -/
def address_from_bits (value : (BitVec 256)) : (Vector (BitVec 8) 20) :=
  let word_value := (word_from_bits value)
  (word_to_address word_value)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def address_from_nat (value : Nat) : (Vector (BitVec 8) 20) :=
  let word_value := (u256 value)
  (word_to_address word_value)

def ZERO_WORD : word :=
  (word_from_bits 0x0000000000000000000000000000000000000000000000000000000000000000#256)

def ZERO_ADDRESS : address :=
  (address_from_bits 0x0000000000000000000000000000000000000000000000000000000000000000#256)

def ZERO_HASH : hash :=
  (hash_from_bits 0x0000000000000000000000000000000000000000000000000000000000000000#256)

def WORD_ZERO : word :=
  (word_from_bits 0x0000000000000000000000000000000000000000000000000000000000000000#256)

def WORD_ONE : word :=
  (word_from_bits 0x0000000000000000000000000000000000000000000000000000000000000001#256)

def WORD_ALL_ONES : word :=
  (word_from_bits 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#256)

def WORD_SIGN_BIT : word :=
  (word_from_bits 0x8000000000000000000000000000000000000000000000000000000000000000#256)

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_add_word (left : Nat) (right : Nat) : Nat :=
  let reduced := (Nat.mod (left + right) (2 ^i 256))
  (u256 reduced)

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_sub_word (left : Nat) (right : Nat) : Nat :=
  if ((right ≤b left) : Bool)
  then (left - right)
  else
    (let maximum : Nat := ((2 ^i 256) - 1)
    ((maximum - (right - left)) + 1))

/-- Bitwise conjunction of two words. -/
/- Type quantifiers: k_ex547326_ : Nat, k_ex547325_ : Nat, 0 ≤ k_ex547325_ ∧
  k_ex547325_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547326_ ∧ k_ex547326_ ≤ (2 ^ 256 - 1) -/
def word_and (left : Nat) (right : Nat) : Nat :=
  let left_bits := (get_slice_int 256 left 0)
  let right_bits := (get_slice_int 256 right 0)
  let result_bits := (left_bits &&& right_bits)
  let result := (BitVec.toNatInt result_bits)
  (u256 result)

/-- Bitwise disjunction of two words. -/
/- Type quantifiers: k_ex547328_ : Nat, k_ex547327_ : Nat, 0 ≤ k_ex547327_ ∧
  k_ex547327_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547328_ ∧ k_ex547328_ ≤ (2 ^ 256 - 1) -/
def word_or (left : Nat) (right : Nat) : Nat :=
  let left_bits := (get_slice_int 256 left 0)
  let right_bits := (get_slice_int 256 right 0)
  let result_bits := (left_bits ||| right_bits)
  let result := (BitVec.toNatInt result_bits)
  (u256 result)

/-- Bitwise exclusive-or of two words. -/
/- Type quantifiers: k_ex547330_ : Nat, k_ex547329_ : Nat, 0 ≤ k_ex547329_ ∧
  k_ex547329_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547330_ ∧ k_ex547330_ ≤ (2 ^ 256 - 1) -/
def word_xor (left : Nat) (right : Nat) : Nat :=
  let left_bits := (get_slice_int 256 left 0)
  let right_bits := (get_slice_int 256 right 0)
  let result_bits := (left_bits ^^^ right_bits)
  let result := (BitVec.toNatInt result_bits)
  (u256 result)

/-- Bitwise complement of a word. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_not (value : Nat) : Nat :=
  let value_bits := (get_slice_int 256 value 0)
  let result_bits := (Complement.complement value_bits)
  let result := (BitVec.toNatInt result_bits)
  (u256 result)

/- Type quantifiers: k_ex547333_ : Nat, k_ex547332_ : Nat, 0 ≤ k_ex547332_ ∧
  k_ex547332_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547333_ ∧ k_ex547333_ ≤ 255 -/
def word_bit (value : Nat) (index : Nat) : (BitVec 1) :=
  (BitVec.access (get_slice_int 256 value 0) index)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_low_byte (value : Nat) : (BitVec 8) :=
  (get_slice_int 8 value 0)

/-- Logical right shift by one bit position, as truncating division by
two. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_shift_right_one (value : Nat) : Nat :=
  let shifted := (Nat.div value 2)
  (u256 shifted)

/-- `1` if the condition holds, else `0` — the EVM boolean convention. -/
/- Type quantifiers: k_ex547335_ : Bool -/
def word_of_bool (b : Bool) : Nat :=
  if (b : Bool)
  then WORD_ONE
  else WORD_ZERO

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def word_is_zero (w : Nat) : Bool :=
  (w == WORD_ZERO)

/- Type quantifiers: w : Nat, 0 ≤ w ∧ w < (2 ^ 256) -/
def word_nonzero (w : Nat) : Bool :=
  let is_zero := (word_is_zero w)
  (! is_zero)

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_ult (a : Nat) (b : Nat) : Bool :=
  (a <b b)

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_ule (a : Nat) (b : Nat) : Bool :=
  let greater := (word_ult b a)
  (! greater)

/-- Returns the position above a 64-bit value's most significant set bit.
Sail defines leading-zero count over fixed bitvectors in every semantic
backend; subtracting it from the vector width also gives zero for zero. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def u64_bit_length (value : Nat) : Nat :=
  let value_bits := (get_slice_int 64 value 0)
  let leading : Nat := (BitVec.countLeadingZeros value_bits)
  (64 - leading)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_bit_length (value : Nat) : Nat :=
  let limb3_bits := (get_slice_int 64 value 192)
  let limb3 := (BitVec.toNatInt limb3_bits)
  if ((limb3 != 0) : Bool)
  then (192 + (u64_bit_length limb3))
  else
    (let limb2_bits := (get_slice_int 64 value 128)
    let limb2 := (BitVec.toNatInt limb2_bits)
    if ((limb2 != 0) : Bool)
    then (128 + (u64_bit_length limb2))
    else
      (let limb1_bits := (get_slice_int 64 value 64)
      let limb1 := (BitVec.toNatInt limb1_bits)
      if ((limb1 != 0) : Bool)
      then (64 + (u64_bit_length limb1))
      else
        (let limb0_bits := (get_slice_int 64 value 0)
        let limb0 := (BitVec.toNatInt limb0_bits)
        (u64_bit_length limb0))))

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def word_mul_word (a : Nat) (b : Nat) : Nat :=
  (Nat.mod (a *i b) (2 ^i 256))

/- Type quantifiers: dividend : Nat, divisor : Nat, 0 ≤ dividend ∧
  dividend < (2 ^ 256) ∧ 0 ≤ divisor ∧ divisor < (2 ^ 256) -/
def word_div_word (dividend : Nat) (divisor : Nat) : Nat :=
  if ((divisor == 0) : Bool)
  then WORD_ZERO
  else
    (let quotient := (Nat.div dividend divisor)
    (u256 quotient))

/- Type quantifiers: dividend : Nat, divisor : Nat, 0 ≤ dividend ∧
  dividend < (2 ^ 256) ∧ 0 ≤ divisor ∧ divisor < (2 ^ 256) -/
def word_mod_word (dividend : Nat) (divisor : Nat) : Nat :=
  if ((divisor == 0) : Bool)
  then WORD_ZERO
  else
    (let remainder := (Nat.mod dividend divisor)
    (u256 remainder))

/- Type quantifiers: left : Nat, right : Nat, 0 ≤ left ∧
  left < (2 ^ 256) ∧ 0 ≤ right ∧ right < (2 ^ 256) -/
def word_greater_than_word (left : Nat) (right : Nat) : Bool :=
  (left >b right)

/-- Shifts a word left by a bounded count, yielding zero at the width. -/
/- Type quantifiers: k_ex547338_ : Nat, k_ex547337_ : Nat, 0 ≤ k_ex547337_ ∧
  k_ex547337_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547338_ ∧ k_ex547338_ ≤ 256 -/
def word_shift_left (value : Nat) (amount : Nat) : Nat :=
  let value_bits := (get_slice_int 256 value 0)
  let shifted_bits := (value_bits <<< amount)
  let shifted := (BitVec.toNatInt shifted_bits)
  (u256 shifted)

/-- Shifts a word right logically by a bounded count. -/
/- Type quantifiers: k_ex547340_ : Nat, k_ex547339_ : Nat, 0 ≤ k_ex547339_ ∧
  k_ex547339_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547340_ ∧ k_ex547340_ ≤ 256 -/
def word_shift_right (value : Nat) (amount : Nat) : Nat :=
  let value_bits := (get_slice_int 256 value 0)
  let shifted_bits := (value_bits >>> amount)
  (BitVec.toNatInt shifted_bits)

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value < (2 ^ 256) -/
def word_byte_length (value : Nat) : Nat :=
  let bit_length := (word_bit_length value)
  if ((bit_length == 0) : Bool)
  then 0
  else (Nat.div (bit_length + 7) 8)

/-- Shifts a two's-complement word right while extending its sign bit. -/
/- Type quantifiers: k_ex547342_ : Nat, k_ex547341_ : Nat, 0 ≤ k_ex547341_ ∧
  k_ex547341_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547342_ ∧ k_ex547342_ ≤ 256 -/
def word_arithmetic_shift_right (value : Nat) (amount : Nat) : Nat :=
  let shifted := (word_shift_right value amount)
  let sign_bit := (word_bit value 255)
  if ((sign_bit == 1#1) : Bool)
  then
    (let sign_fill := (word_shift_left WORD_ALL_ONES (256 - amount))
    (word_or shifted sign_fill))
  else shifted

/-- Embeds a canonical-order address into the low 160 bits of an EVM word. -/
def address_to_word (bytes : (Vector (BitVec 8) 20)) : Nat :=
  (BitVec.toNatInt
    ((GetElem?.getElem! bytes 0) +++ ((GetElem?.getElem! bytes 1) +++ ((GetElem?.getElem! bytes 2) +++ ((GetElem?.getElem!
              bytes 3) +++ ((GetElem?.getElem! bytes 4) +++ ((GetElem?.getElem! bytes 5) +++ ((GetElem?.getElem!
                    bytes 6) +++ ((GetElem?.getElem! bytes 7) +++ ((GetElem?.getElem! bytes 8) +++ ((GetElem?.getElem!
                          bytes 9) +++ ((GetElem?.getElem! bytes 10) +++ ((GetElem?.getElem! bytes
                              11) +++ ((GetElem?.getElem! bytes 12) +++ ((GetElem?.getElem! bytes 13) +++ ((GetElem?.getElem!
                                    bytes 14) +++ ((GetElem?.getElem! bytes 15) +++ ((GetElem?.getElem!
                                        bytes 16) +++ ((GetElem?.getElem! bytes 17) +++ ((GetElem?.getElem!
                                            bytes 18) +++ (GetElem?.getElem! bytes 19)))))))))))))))))))))

/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_negate (value : Nat) : Nat :=
  (word_sub_word WORD_ZERO value)

/-- Returns the unsigned magnitude of a two's-complement word. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_abs (value : Nat) : Nat :=
  let sign_bit := (word_bit value 255)
  if ((sign_bit == 1#1) : Bool)
  then (word_negate value)
  else value

/-- Signed (two's-complement) 256-bit less-than: sign bits decide when they
differ, otherwise the unsigned order applies. -/
/- Type quantifiers: k_ex547346_ : Nat, k_ex547345_ : Nat, 0 ≤ k_ex547345_ ∧
  k_ex547345_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547346_ ∧ k_ex547346_ ≤ (2 ^ 256 - 1) -/
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

/- Type quantifiers: k_ex547349_ : Nat, k_ex547348_ : Nat, 0 ≤ k_ex547348_ ∧
  k_ex547348_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547349_ ∧ k_ex547349_ ≤ (2 ^ 256 - 1) -/
def alu_sub (a : Nat) (b : Nat) : Nat :=
  (word_sub_word a b)

/- Type quantifiers: a : Nat, b : Nat, 0 ≤ a ∧ a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) -/
def alu_mul (a : Nat) (b : Nat) : Nat :=
  (word_mul_word a b)

/-- `DIV`: unsigned Euclidean division; division by zero yields `0`
(YP Appendix H). -/
/- Type quantifiers: k_ex547351_ : Nat, k_ex547350_ : Nat, 0 ≤ k_ex547350_ ∧
  k_ex547350_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547351_ ∧ k_ex547351_ ≤ (2 ^ 256 - 1) -/
def alu_div (a : Nat) (b : Nat) : Nat :=
  (word_div_word a b)

/-- `MOD`: unsigned modulus; a zero modulus yields `0`. -/
/- Type quantifiers: k_ex547353_ : Nat, k_ex547352_ : Nat, 0 ≤ k_ex547352_ ∧
  k_ex547352_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547353_ ∧ k_ex547353_ ≤ (2 ^ 256 - 1) -/
def alu_mod (a : Nat) (b : Nat) : Nat :=
  (word_mod_word a b)

/-- `SDIV`: signed division, truncating toward zero; division by zero
yields `0`. -/
/- Type quantifiers: k_ex547355_ : Nat, k_ex547354_ : Nat, 0 ≤ k_ex547354_ ∧
  k_ex547354_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547355_ ∧ k_ex547355_ ≤ (2 ^ 256 - 1) -/
def alu_sdiv (a : Nat) (b : Nat) : Nat :=
  let divisor_is_zero := (word_is_zero b)
  if (divisor_is_zero : Bool)
  then WORD_ZERO
  else
    (let dividend_magnitude := (word_abs a)
    let divisor_magnitude := (word_abs b)
    let quotient := (word_div_word dividend_magnitude divisor_magnitude)
    let dividend_sign := (word_bit a 255)
    let divisor_sign := (word_bit b 255)
    if ((neq_bool (dividend_sign == 1#1) (divisor_sign == 1#1)) : Bool)
    then (word_negate quotient)
    else quotient)

/-- `SMOD`: signed remainder, with the sign of the dividend; a zero modulus
yields `0`. -/
/- Type quantifiers: k_ex547357_ : Nat, k_ex547356_ : Nat, 0 ≤ k_ex547356_ ∧
  k_ex547356_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547357_ ∧ k_ex547357_ ≤ (2 ^ 256 - 1) -/
def alu_smod (a : Nat) (b : Nat) : Nat :=
  let modulus_is_zero := (word_is_zero b)
  if (modulus_is_zero : Bool)
  then WORD_ZERO
  else
    (let dividend_magnitude := (word_abs a)
    let modulus_magnitude := (word_abs b)
    let remainder := (word_mod_word dividend_magnitude modulus_magnitude)
    let dividend_sign := (word_bit a 255)
    if ((dividend_sign == 1#1) : Bool)
    then (word_negate remainder)
    else remainder)

/- Type quantifiers: a : Nat, b : Nat, n : Nat, 0 ≤ a ∧
  a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) ∧ 0 ≤ n ∧ n < (2 ^ 256) -/
def alu_addmod (a : Nat) (b : Nat) (n : Nat) : Nat :=
  if ((n == 0) : Bool)
  then WORD_ZERO
  else
    (let remainder := (Nat.mod (a + b) n)
    (u256 remainder))

/- Type quantifiers: a : Nat, b : Nat, n : Nat, 0 ≤ a ∧
  a < (2 ^ 256) ∧ 0 ≤ b ∧ b < (2 ^ 256) ∧ 0 ≤ n ∧ n < (2 ^ 256) -/
def alu_mulmod (a : Nat) (b : Nat) (n : Nat) : Nat :=
  if ((n == 0) : Bool)
  then WORD_ZERO
  else
    (let remainder := (Nat.mod (a *i b) n)
    (u256 remainder))

/-- `EXP` via square-and-multiply over the 256 exponent bits, reduced
modulo 2^256 at every step. -/
/- Type quantifiers: k_ex547359_ : Nat, k_ex547358_ : Nat, 0 ≤ k_ex547358_ ∧
  k_ex547358_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547359_ ∧ k_ex547359_ ≤ (2 ^ 256 - 1) -/
def alu_exp (base : Nat) (exponent : Nat) : SailM Nat := do
  let result : Nat := WORD_ONE
  let b : Nat := base
  let e : Nat := exponent
  let remaining : Nat := (word_bit_length exponent)
  let (b, e, remaining, result) ← (( do
    let loop_vars ← whileFuelM (fuel :=remaining) (fun (b, e, remaining, result) => (pure (remaining >b 0))) (b, e, remaining, result)
      fun (b, e, remaining, result) => do
        assert true "loop dummy assert"
        let rounds_left := remaining
        let low_bit := (word_bit e 0)
        let result : Nat :=
          if ((low_bit == 1#1) : Bool)
          then
            (let result : Nat := (word_mul_word result b)
            result)
          else result
        let b : Nat :=
          if ((rounds_left >b 1) : Bool)
          then
            (let b : Nat := (word_mul_word b b)
            b)
          else b
        let e : Nat := (word_shift_right_one e)
        let remaining : Nat :=
          if ((rounds_left >b 0) : Bool)
          then (rounds_left - 1)
          else 0
        (pure (b, e, remaining, result))
    (pure loop_vars) ) : SailM (Nat × Nat × Nat × Nat) )
  (pure result)

/-- `SIGNEXTEND(byte_index, value)`: sign-extends `value` from byte
`byte_index` (0 = least significant); indices ≥ 31 leave the value
unchanged. -/
/- Type quantifiers: k_ex547361_ : Nat, k_ex547360_ : Nat, 0 ≤ k_ex547360_ ∧
  k_ex547360_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547361_ ∧ k_ex547361_ ≤ (2 ^ 256 - 1) -/
def alu_signextend (byte_index : Nat) (value : Nat) : Nat :=
  if ((byte_index <b 32) : Bool)
  then
    (let index : Nat := byte_index
    let width : Nat := ((index *i 8) + 8)
    let sign_shift : Nat := ((index *i 8) + 7)
    let shifted_sign := (word_shift_right value sign_shift)
    let isolated_sign := (word_and shifted_sign WORD_ONE)
    let sign_set := (isolated_sign == WORD_ONE)
    let low_mask_end := (word_shift_left WORD_ONE width)
    let low_mask := (word_sub_word low_mask_end WORD_ONE)
    if (sign_set : Bool)
    then
      (let low_value := (word_and value low_mask)
      let high_mask := (word_not low_mask)
      (word_or low_value high_mask))
    else (word_and value low_mask))
  else value

/-- `LT`: `1` when `a` is strictly below `b`, unsigned. -/
/- Type quantifiers: k_ex547363_ : Nat, k_ex547362_ : Nat, 0 ≤ k_ex547362_ ∧
  k_ex547362_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547363_ ∧ k_ex547363_ ≤ (2 ^ 256 - 1) -/
def alu_lt (a : Nat) (b : Nat) : Nat :=
  let result := (word_ult a b)
  (word_of_bool result)

/-- `GT`: `1` when `a` is strictly above `b`, unsigned. -/
/- Type quantifiers: k_ex547365_ : Nat, k_ex547364_ : Nat, 0 ≤ k_ex547364_ ∧
  k_ex547364_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547365_ ∧ k_ex547365_ ≤ (2 ^ 256 - 1) -/
def alu_gt (a : Nat) (b : Nat) : Nat :=
  let result := (word_ult b a)
  (word_of_bool result)

/-- `SLT`: `1` when `a` is strictly below `b`, two's-complement signed. -/
/- Type quantifiers: k_ex547367_ : Nat, k_ex547366_ : Nat, 0 ≤ k_ex547366_ ∧
  k_ex547366_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547367_ ∧ k_ex547367_ ≤ (2 ^ 256 - 1) -/
def alu_slt (a : Nat) (b : Nat) : Nat :=
  let result := (word_slt a b)
  (word_of_bool result)

/-- `SGT`: `1` when `a` is strictly above `b`, two's-complement signed. -/
/- Type quantifiers: k_ex547369_ : Nat, k_ex547368_ : Nat, 0 ≤ k_ex547368_ ∧
  k_ex547368_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547369_ ∧ k_ex547369_ ≤ (2 ^ 256 - 1) -/
def alu_sgt (a : Nat) (b : Nat) : Nat :=
  let result := (word_slt b a)
  (word_of_bool result)

/- Type quantifiers: k_ex547371_ : Nat, k_ex547370_ : Nat, 0 ≤ k_ex547370_ ∧
  k_ex547370_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547371_ ∧ k_ex547371_ ≤ (2 ^ 256 - 1) -/
def alu_eq (a : Nat) (b : Nat) : Nat :=
  (word_of_bool (a == b))

/-- `ISZERO`: `1` exactly when the operand is zero. -/
/- Type quantifiers: a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_iszero (a : Nat) : Nat :=
  let result := (word_is_zero a)
  (word_of_bool result)

/- Type quantifiers: k_ex547374_ : Nat, k_ex547373_ : Nat, 0 ≤ k_ex547373_ ∧
  k_ex547373_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547374_ ∧ k_ex547374_ ≤ (2 ^ 256 - 1) -/
def alu_and (a : Nat) (b : Nat) : Nat :=
  (word_and a b)

/- Type quantifiers: k_ex547376_ : Nat, k_ex547375_ : Nat, 0 ≤ k_ex547375_ ∧
  k_ex547375_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547376_ ∧ k_ex547376_ ≤ (2 ^ 256 - 1) -/
def alu_or (a : Nat) (b : Nat) : Nat :=
  (word_or a b)

/- Type quantifiers: k_ex547378_ : Nat, k_ex547377_ : Nat, 0 ≤ k_ex547377_ ∧
  k_ex547377_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547378_ ∧ k_ex547378_ ≤ (2 ^ 256 - 1) -/
def alu_xor (a : Nat) (b : Nat) : Nat :=
  (word_xor a b)

/- Type quantifiers: a : Nat, 0 ≤ a ∧ a ≤ (2 ^ 256 - 1) -/
def alu_not (a : Nat) : Nat :=
  (word_not a)

/-- `BYTE(i, x)`: the `i`-th most-significant byte of `x` (0 = MSB);
indices ≥ 32 yield `0`. -/
/- Type quantifiers: k_ex547381_ : Nat, k_ex547380_ : Nat, 0 ≤ k_ex547380_ ∧
  k_ex547380_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547381_ ∧ k_ex547381_ ≤ (2 ^ 256 - 1) -/
def alu_byte (i : Nat) (x : Nat) : Nat :=
  if ((i <b 32) : Bool)
  then
    (let index : Nat := i
    let shift : Nat := ((31 - index) *i 8)
    let shifted := (word_shift_right x shift)
    let result_byte := (word_low_byte shifted)
    (BitVec.toNatInt result_byte))
  else WORD_ZERO

/-- `SHL`: logical left shift; amounts ≥ 256 yield `0`. -/
/- Type quantifiers: k_ex547383_ : Nat, k_ex547382_ : Nat, 0 ≤ k_ex547382_ ∧
  k_ex547382_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547383_ ∧ k_ex547383_ ≤ (2 ^ 256 - 1) -/
def alu_shl (shift_amt : Nat) (v : Nat) : Nat :=
  if ((shift_amt <b 256) : Bool)
  then (word_shift_left v shift_amt)
  else WORD_ZERO

/-- `SHR`: logical right shift; amounts ≥ 256 yield `0`. -/
/- Type quantifiers: k_ex547385_ : Nat, k_ex547384_ : Nat, 0 ≤ k_ex547384_ ∧
  k_ex547384_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547385_ ∧ k_ex547385_ ≤ (2 ^ 256 - 1) -/
def alu_shr (shift_amt : Nat) (v : Nat) : Nat :=
  if ((shift_amt <b 256) : Bool)
  then (word_shift_right v shift_amt)
  else WORD_ZERO

/-- `SAR`: arithmetic (sign-propagating) right shift. -/
/- Type quantifiers: k_ex547387_ : Nat, k_ex547386_ : Nat, 0 ≤ k_ex547386_ ∧
  k_ex547386_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex547387_ ∧ k_ex547387_ ≤ (2 ^ 256 - 1) -/
def alu_sar (shift_amt : Nat) (v : Nat) : Nat :=
  if ((shift_amt <b 256) : Bool)
  then (word_arithmetic_shift_right v shift_amt)
  else
    (let sign_bit := (word_bit v 255)
    if ((sign_bit == 1#1) : Bool)
    then WORD_ALL_ONES
    else WORD_ZERO)

/-- `CLZ`: the count of leading zero bits of a 256-bit word (EIP-7939). -/
/- Type quantifiers: x : Nat, 0 ≤ x ∧ x ≤ (2 ^ 256 - 1) -/
def alu_clz (x : Nat) : Nat :=
  let bit_length := (word_bit_length x)
  (u256 (256 - bit_length))

