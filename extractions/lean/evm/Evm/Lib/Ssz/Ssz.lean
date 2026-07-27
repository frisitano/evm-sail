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
open BalIterEntry

/-! # SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules. -/

/-- The width of one entry in an SSZ variable-field offset table
(`uint32`, little-endian). -/
def SSZ_OFF_BYTES : Nat := 4

def SSZ_UINT_BYTES : Nat := 8

/- Type quantifiers: base : Nat, delta : Nat, (source_valid_range base delta) -/
def ssz_field_offset (base : Nat) (delta : Nat) : Nat :=
  (base + delta)

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 4), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def ssz_u32_at (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let b0 ← do (pure (Sail.BitVec.zeroExtend (← (slice_byte ⟨_, ⟨_, input⟩⟩ offset)) 32))
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

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 4), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def ssz_u32 (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  (ssz_u32_at ⟨_, ⟨_, input⟩⟩ offset)

/-- Narrows a wire-bounded SSZ offset at the host byte-position boundary. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 32 - 1) -/
def ssz_offset_to_source_pointer (value : Nat) : Nat :=
  value

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 8), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def decode_ssz_uint (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  (pure ((((((((BitVec.toNatInt (← (slice_byte ⟨_, ⟨_, input⟩⟩ offset))) + ((BitVec.toNatInt
                      (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 1)))) *i (2 ^i 8))) + ((BitVec.toNatInt
                    (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 2)))) *i (2 ^i 16))) + ((BitVec.toNatInt
                  (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 3)))) *i (2 ^i 24))) + ((BitVec.toNatInt
                (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset SSZ_OFF_BYTES)))) *i (2 ^i 32))) + ((BitVec.toNatInt
              (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 5)))) *i (2 ^i 40))) + ((BitVec.toNatInt
            (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 6)))) *i (2 ^i 48))) + ((BitVec.toNatInt
          (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset 7)))) *i (2 ^i 56))))

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 20), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def ssz_addr (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 20) := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  (pure (word_to_address (← (slice_load_n ⟨_, ⟨_, input⟩⟩ offset ADDRESS_BYTE_LENGTH))))

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 32), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def ssz_bytes32 (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 32) := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  (pure (word_to_hash (← (slice_load ⟨_, ⟨_, input⟩⟩ offset))))

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 255 -/
def ssz_logs_bloom_index (index : Nat) : Nat :=
  (255 - index)

/-- Decodes a fixed 256-byte logs bloom from SSZ wire order. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 256), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def ssz_logs_bloom (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 256) := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
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
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (source_valid_range offset 32), 0
  ≤ input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def ssz_u256 (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let result : Nat := WORD_ZERO
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := result
  for k in [loop_k_lower:loop_k_upper:1]i do
    let result := loop_vars
    loop_vars ← do
      (pure (word_add_word (word_mul_word result 256)
          (BitVec.toNatInt
            (← (slice_byte ⟨_, ⟨_, input⟩⟩ (ssz_field_offset offset (ssz_u256_index k)))))))
  (pure loop_vars)

