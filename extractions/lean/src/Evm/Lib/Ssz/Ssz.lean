import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess

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

/-! # SSZ source decoding

Scalar readers and variable-list navigation over the private-input byte
source. Concrete container layouts belong in their decoder modules. -/

/-- The width of one entry in an SSZ variable-field offset table
(`uint32`, little-endian). -/
abbrev SSZ_OFF_BYTES : Nat := 4

abbrev SSZ_UINT_BYTES : Nat := 8

/- Type quantifiers: base : Nat, delta : Nat, (stateless_input_valid_range base delta) -/
def ssz_field_offset (base : Nat) (delta : Nat) : Nat :=
  (base + delta)

/- Type quantifiers: base : Nat, delta : Nat, (scratch_valid_range base delta) -/
def scratch_field_offset (base : Nat) (delta : Nat) : Nat :=
  (base + delta)

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 4), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_u32_at (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let byte0 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset)
  let offset1 := (ssz_field_offset offset 1)
  let byte1 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset1)
  let offset2 := (ssz_field_offset offset 2)
  let byte2 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset2)
  let offset3 := (ssz_field_offset offset 3)
  let byte3 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset3)
  let b0 := (Sail.BitVec.zeroExtend byte0 32)
  let b1 := (Sail.BitVec.zeroExtend byte1 32)
  let b2 := (Sail.BitVec.zeroExtend byte2 32)
  let b3 := (Sail.BitVec.zeroExtend byte3 32)
  let shifted1 := (b1 <<< 8)
  let shifted2 := (b2 <<< 16)
  let shifted3 := (b3 <<< 24)
  let high := (shifted2 ||| shifted3)
  let nonzero := (shifted1 ||| high)
  let result := (b0 ||| nonzero)
  (pure (BitVec.toNatInt result))

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 4), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_u32 (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  (ssz_u32_at ⟨_, ⟨_, input⟩⟩ offset)

/-- Reads an offset-table entry after establishing that the dynamic table
position is contained by its enclosing input slice. -/
/- Type quantifiers: k_ex550463_ : Nat, input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex550463_ ∧
  k_ex550463_ ≤ (2 ^ 32 - 1) -/
def ssz_u32_in_slice (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  if (((offset ≤b input.len) && (4 ≤b (input.len -i offset))) : Bool)
  then (ssz_u32_at ⟨_, ⟨_, input⟩⟩ offset)
  else (fatal_error InvalidConfig)

/-- Narrows a wire-bounded SSZ offset at the host byte-position boundary. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 32 - 1) -/
def ssz_offset_to_source_pointer (value : Nat) : Nat :=
  value

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 8), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_ssz_uint (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let byte0 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset)
  let offset1 := (ssz_field_offset offset 1)
  let byte1 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset1)
  let offset2 := (ssz_field_offset offset 2)
  let byte2 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset2)
  let offset3 := (ssz_field_offset offset 3)
  let byte3 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset3)
  let offset4 := (ssz_field_offset offset 4)
  let byte4 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset4)
  let offset5 := (ssz_field_offset offset 5)
  let byte5 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset5)
  let offset6 := (ssz_field_offset offset 6)
  let byte6 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset6)
  let offset7 := (ssz_field_offset offset 7)
  let byte7 ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ offset7)
  (pure ((((((((BitVec.toNatInt byte0) + ((BitVec.toNatInt byte1) *i (2 ^i 8))) + ((BitVec.toNatInt
                    byte2) *i (2 ^i 16))) + ((BitVec.toNatInt byte3) *i (2 ^i 24))) + ((BitVec.toNatInt
                byte4) *i (2 ^i 32))) + ((BitVec.toNatInt byte5) *i (2 ^i 40))) + ((BitVec.toNatInt
            byte6) *i (2 ^i 48))) + ((BitVec.toNatInt byte7) *i (2 ^i 56))))

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (scratch_valid_range offset 8), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_scratch_uint (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let byte0 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset)
  let offset1 := (scratch_field_offset offset 1)
  let byte1 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset1)
  let offset2 := (scratch_field_offset offset 2)
  let byte2 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset2)
  let offset3 := (scratch_field_offset offset 3)
  let byte3 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset3)
  let offset4 := (scratch_field_offset offset 4)
  let byte4 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset4)
  let offset5 := (scratch_field_offset offset 5)
  let byte5 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset5)
  let offset6 := (scratch_field_offset offset 6)
  let byte6 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset6)
  let offset7 := (scratch_field_offset offset 7)
  let byte7 ← do (scratch_byte ⟨_, ⟨_, input⟩⟩ offset7)
  (pure ((((((((BitVec.toNatInt byte0) + ((BitVec.toNatInt byte1) *i (2 ^i 8))) + ((BitVec.toNatInt
                    byte2) *i (2 ^i 16))) + ((BitVec.toNatInt byte3) *i (2 ^i 24))) + ((BitVec.toNatInt
                byte4) *i (2 ^i 32))) + ((BitVec.toNatInt byte5) *i (2 ^i 40))) + ((BitVec.toNatInt
            byte6) *i (2 ^i 48))) + ((BitVec.toNatInt byte7) *i (2 ^i 56))))

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 20), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_addr (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 20) := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let value ← do (stateless_input_slice_load_n ⟨_, ⟨_, input⟩⟩ offset ADDRESS_BYTE_LENGTH)
  (pure (word_to_address value))

/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 32), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_bytes32 (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 32) := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let value ← do (stateless_input_slice_load ⟨_, ⟨_, input⟩⟩ offset)
  (pure (word_to_hash value))

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 255 -/
def ssz_logs_bloom_index (index : Nat) : Nat :=
  (255 - index)

/-- Decodes a fixed 256-byte logs bloom from SSZ wire order. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 256), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_logs_bloom (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM (Vector (BitVec 8) 256) := do
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
      let output_index := (ssz_logs_bloom_index k)
      let source_offset := (ssz_field_offset offset k)
      (pure (vectorUpdate out output_index
          (← (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ source_offset))))
  (pure loop_vars)

/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 31 -/
def ssz_u256_index (index : Nat) : Nat :=
  (31 - index)

/-- Decodes a 32-byte little-endian SSZ integer into an EVM word. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, offset : Nat, (stateless_input_valid_range offset 32), 0
  ≤ input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_u256 (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (offset : Nat) : SailM Nat := do
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
      let source_index := (ssz_u256_index k)
      let source_offset := (ssz_field_offset offset source_index)
      let source_byte ← do (stateless_input_slice_byte ⟨_, ⟨_, input⟩⟩ source_offset)
      let byte_value := (BitVec.toNatInt source_byte)
      let shifted_result := (word_mul_word result 256)
      (pure (word_add_word shifted_result byte_value))
  (pure loop_vars)

