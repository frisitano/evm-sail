import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Host.EvmByteSlice

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

/-! # SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules. -/

/-- The width of one entry in an SSZ variable-field offset table
(`uint32`, little-endian). -/
def SSZ_OFF_BYTES : Nat := 4

def SSZ_UINT_BYTES : Nat := 8

/- Type quantifiers: base : Nat, delta : Nat, source_valid_range(base, delta) -/
def ssz_field_offset (base : Nat) (delta : Nat) : Nat :=
  (base + delta)

/- Type quantifiers: k_ex409490_ : Nat, k_ex409489_ : Nat, offset : Nat, source_valid_range(offset, 4), 0
  ≤ k_ex409489_ ∧ 0 ≤ k_ex409490_ -/
def ssz_u32_at (input : EvmByteSlice) (offset : Nat) : SailM ssz_offset := do
  let input := ((input).2).2
  let publicResult ← do
    let b0 ← do
      (pure (Sail.BitVec.zeroExtend (← (slice_byte ⟨_, ⟨_, input⟩⟩ offset)) 32))
    let b1 ← do
      (pure (Sail.BitVec.zeroExtend
          (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 1))) 32))
    let b2 ← do
      (pure (Sail.BitVec.zeroExtend
          (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 2))) 32))
    let b3 ← do
      (pure (Sail.BitVec.zeroExtend
          (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 3))) 32))
    (pure (BitVec.toNatInt (b0 ||| ((b1 <<< 8) ||| ((b2 <<< 16) ||| (b3 <<< 24))))))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409504_ : Nat, k_ex409503_ : Nat, offset : Nat, source_valid_range(offset, 4), 0
  ≤ k_ex409503_ ∧ 0 ≤ k_ex409504_ -/
def ssz_u32 (input : EvmByteSlice) (offset : Nat) : SailM ssz_offset := do
  let input := ((input).2).2
  let publicResult ← do
    (do
        let publicResult ← (ssz_u32_at ⟨_, ⟨_, input⟩⟩ offset)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

/-- Narrows a wire-bounded SSZ offset at the host byte-position boundary. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 32 - 1) -/
def ssz_offset_to_source_pointer (value : ssz_offset) : source_pointer :=
  let value := (value).value
  value

/- Type quantifiers: k_ex409519_ : Nat, k_ex409518_ : Nat, offset : Nat, source_valid_range(offset, 8), 0
  ≤ k_ex409518_ ∧ 0 ≤ k_ex409519_ -/
def decode_ssz_uint (input : EvmByteSlice) (offset : Nat) : SailM ssz_uint := do
  let input := ((input).2).2
  let publicResult ← do
    (pure ((((((((BitVec.toNatInt (← (slice_byte ⟨_, ⟨_, input⟩⟩ offset))) + ((BitVec.toNatInt
                        (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 1)))) *i (2 ^i 8))) + ((BitVec.toNatInt
                      (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 2)))) *i (2 ^i 16))) + ((BitVec.toNatInt
                    (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 3)))) *i (2 ^i 24))) + ((BitVec.toNatInt
                  (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset SSZ_OFF_BYTES)))) *i (2 ^i 32))) + ((BitVec.toNatInt
                (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 5)))) *i (2 ^i 40))) + ((BitVec.toNatInt
              (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 6)))) *i (2 ^i 48))) + ((BitVec.toNatInt
            (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 7)))) *i (2 ^i 56))))
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409533_ : Nat, k_ex409532_ : Nat, offset : Nat, source_valid_range(offset, 20), 0
  ≤ k_ex409532_ ∧ 0 ≤ k_ex409533_ -/
def ssz_addr (input : EvmByteSlice) (offset : Nat) : SailM address := do
  let input := ((input).2).2
  (pure (word_to_address
      ⟨((← (slice_load_n ⟨_, ⟨_, input⟩⟩ offset ADDRESS_BYTE_LENGTH))).value⟩))

/- Type quantifiers: k_ex409547_ : Nat, k_ex409546_ : Nat, offset : Nat, source_valid_range(offset, 32), 0
  ≤ k_ex409546_ ∧ 0 ≤ k_ex409547_ -/
def ssz_bytes32 (input : EvmByteSlice) (offset : Nat) : SailM hash := do
  let input := ((input).2).2
  (pure (word_to_hash ⟨((← (slice_load ⟨_, ⟨_, input⟩⟩ offset))).value⟩))

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 255 -/
def ssz_logs_bloom_index (index : Nat) : Nat :=
  (255 - index)

/-- Decodes a fixed 256-byte logs bloom from SSZ wire order. -/
/- Type quantifiers: k_ex409561_ : Nat, k_ex409560_ : Nat, offset : Nat, source_valid_range(offset, 256), 0
  ≤ k_ex409560_ ∧ 0 ≤ k_ex409561_ -/
def ssz_logs_bloom (input : EvmByteSlice) (offset : Nat) : SailM LogsBloom := do
  let input := ((input).2).2
  let out : (Vector (BitVec 8) 256) := (vectorInit 0x00#8)
  let loop_k_lower := 0
  let loop_k_upper := 255
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars ← do
      (pure (vectorUpdate out (ssz_logs_bloom_index k)
          (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset k)))))
  (pure loop_vars)

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 31 -/
def ssz_u256_index (index : Nat) : Nat :=
  (31 - index)

/-- Decodes a 32-byte little-endian SSZ integer into an EVM word. -/
/- Type quantifiers: k_ex409575_ : Nat, k_ex409574_ : Nat, offset : Nat, source_valid_range(offset, 32), 0
  ≤ k_ex409574_ ∧ 0 ≤ k_ex409575_ -/
def ssz_u256 (input : EvmByteSlice) (offset : Nat) : SailM word := do
  let input := ((input).2).2
  let publicResult ← do
    let result : Nat := (WORD_ZERO).value
    let loop_k_lower := 0
    let loop_k_upper := 31
    let mut loop_vars := result
    for k in [loop_k_lower:loop_k_upper:1]i do
      let result := loop_vars
      loop_vars ← do
        (pure ((word_add_word (word_mul_word result 256)
            (BitVec.toNatInt
              (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset (ssz_u256_index k))))))).value)
    (pure loop_vars)
  pure (⟨publicResult⟩)

