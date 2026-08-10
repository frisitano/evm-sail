import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Host.RegionAccess
import Evm.Primitives.Crypto
import Evm.Host.Output
import Evm.Evm.Gas

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

/-! # Precompiled contracts

EVM precompile semantics (YP Appendix E and the precompile EIPs). Sail
selects the active address, validates the Ethereum input encoding, and
returns the EVM-visible output; the host functions called here are only
raw zkVM cryptographic accelerators.

## Constants

The fixed offsets and lengths describe the canonical byte layouts accepted
by each precompile; the field modulus and blob element count validate their
respective algebraic inputs. -/

abbrev ACCELERATOR_INPUT_MAX : Nat := 2097152

def FIELD_ELEMENTS_PER_BLOB : word :=
  (word_from_bits 0x0000000000000000000000000000000000000000000000000000000000001000#256)

def BLS_MODULUS : word :=
  (word_from_bits 0x73EDA753299D7D483339D80809A1D80553BDA402FFFE5BFEFFFFFFFF00000001#256)

abbrev BLAKE2F_INPUT_LENGTH : Nat := 213

abbrev BLAKE2F_FINAL_BLOCK_OFFSET : Nat := 212

abbrev BLAKE2F_OUTPUT_LENGTH : Nat := 64

abbrev KZG_INPUT_LENGTH : Nat := 192

abbrev KZG_COMMITMENT_OFFSET : Nat := 96

abbrev KZG_COMMITMENT_LENGTH : Nat := 48

abbrev BLS_FIELD_PADDING_LENGTH : Nat := 16

abbrev BLS_PADDED_FIELD_LENGTH : Nat := 64

abbrev BLS_G1_POINT_LENGTH : Nat := 128

abbrev BLS_G2_POINT_LENGTH : Nat := 256

abbrev BLS_G2_FINAL_FIELD_OFFSET : Nat := 192

abbrev BLS_G1_ADD_INPUT_LENGTH : Nat := 256

abbrev BLS_G1_MSM_ITEM_LENGTH : Nat := 160

abbrev BLS_G2_ADD_INPUT_LENGTH : Nat := 512

abbrev BLS_G2_MSM_ITEM_LENGTH : Nat := 288

abbrev BLS_PAIRING_ITEM_LENGTH : Nat := 384

abbrev P256_INPUT_LENGTH : Nat := 160

abbrev BN254_PAIRING_ITEM_LENGTH : Nat := 192

abbrev PRECOMPILE_WORD_LENGTH : Nat := 32

abbrev PRECOMPILE_DOUBLE_WORD_LENGTH : Nat := 64

abbrev PRECOMPILE_WORD_OFFSET : Nat := 32

abbrev PRECOMPILE_DOUBLE_WORD_OFFSET : Nat := 64

def ECRECOVER_S_OFFSET : source_pointer := 96

abbrev TWO_COMPONENTS : Nat := 2

abbrev BLS_G2_POINT_OFFSET : Nat := 128

/-- A successful result carrying `output`. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def precompile_success (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : PrecompileResult :=
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  { success := true,
    output := ⟨_, ⟨_, output⟩⟩ }

/-- The failed result (empty output; the call reports failure). -/
def precompile_failure (_ : Unit) : PrecompileResult :=
  { success := false,
    output := ⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ }

/- Type quantifiers: k_ex552677_ : Bool, output_len : Nat, (source_valid_length output_len) -/
def accelerator_result (success : Bool) (output_len : Nat) : PrecompileResult :=
  if (success : Bool)
  then
    (let ⟨_, ⟨_, output⟩⟩ := (output_buffer_slice output_len)
    (precompile_success ⟨_, ⟨_, output⟩⟩))
  else (precompile_failure ())

/-- `IDENTITY` (0x04): the input, copied through the output buffer. -/
def copied_result (data : CalldataSlice) : SailM PrecompileResult := do
  let ⟨_, ⟨_, output⟩⟩ ← do (freeze_calldata_output data)
  let input_length := (calldata_slice_length data)
  if ((output.len == input_length) : Bool)
  then (pure (precompile_success ⟨_, ⟨_, output⟩⟩))
  else (pure (precompile_failure ()))

/-- A 32-byte `0`/`1` result word (pairing checks). -/
/- Type quantifiers: k_ex552680_ : Bool -/
def boolean_result (value : Bool) : SailM PrecompileResult := do
  let result_word :=
    if (value : Bool)
    then WORD_ONE
    else WORD_ZERO
  let ⟨_, ⟨_, output⟩⟩ ← do (output_buffer_word result_word)
  (pure (precompile_success ⟨_, ⟨_, output⟩⟩))

/-- `ECRECOVER` (0x01): recovers the signer address; any invalid input
yields a successful call with empty output. -/
def run_ecrecover (input : CalldataSlice) : SailM PrecompileResult := do
  let v ← do (calldata_slice_load input PRECOMPILE_WORD_OFFSET)
  let v_27 := (u256 27)
  let v_28 := (u256 28)
  let valid_v := ((v == v_27) || (v == v_28))
  if (valid_v : Bool)
  then
    (do
      let parity : Nat :=
        if ((v == v_27) : Bool)
        then 0
        else 1
      let message_word ← do (calldata_slice_load input 0)
      let message_hash := (word_to_hash message_word)
      let r ← do (calldata_slice_load input PRECOMPILE_DOUBLE_WORD_OFFSET)
      let s ← do (calldata_slice_load input ECRECOVER_S_OFFSET)
      let (recovered, address) ← do (ecrecover_addr message_hash parity r s)
      if (recovered : Bool)
      then
        (do
          let address_word := (address_to_word address)
          let ⟨_, ⟨_, output⟩⟩ ← do (output_buffer_word address_word)
          (pure (precompile_success ⟨_, ⟨_, output⟩⟩)))
      else (pure (precompile_success ⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩)))
  else (pure (precompile_success ⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩))

/-- `SHA256` (0x02). -/
def run_sha256 (input : CalldataSlice) : SailM PrecompileResult := do
  let digest ← do (calldata_sha256 input)
  let digest_word := (hash_to_word digest)
  let ⟨_, ⟨_, output⟩⟩ ← do (output_buffer_word digest_word)
  (pure (precompile_success ⟨_, ⟨_, output⟩⟩))

/-- `RIPEMD160` (0x03): 20-byte digest, left-padded to 32. -/
def run_ripemd160 (input : CalldataSlice) : SailM PrecompileResult := do
  let success ← do (accelerator_ripemd160 input)
  (pure (accelerator_result success PRECOMPILE_WORD_LENGTH))

/-- `MODEXP` (0x05, EIP-198): arbitrary-precision modular
exponentiation; a zero-length modulus yields empty output, and inputs
beyond the accelerator bound fail the call. -/
def run_modexp (input : CalldataSlice) : SailM PrecompileResult := do
  let base_len ← (( do (pc_word input 0 32) ) : SailM Nat )
  let exponent_len ← (( do (pc_word input 32 32) ) : SailM Nat )
  let modulus_len ← (( do (pc_word input 64 32) ) : SailM Nat )
  if ((modulus_len == 0) : Bool)
  then (pure (precompile_success ⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩))
  else
    (do
      if (((ACCELERATOR_INPUT_MAX <b base_len) || ((ACCELERATOR_INPUT_MAX <b exponent_len) || (ACCELERATOR_INPUT_MAX <b modulus_len))) : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let bounded_base : Nat := base_len
          let bounded_exponent : Nat := exponent_len
          let bounded_modulus : Nat := modulus_len
          let input_end : Nat := (((96 + bounded_base) + bounded_exponent) + bounded_modulus)
          if ((ACCELERATOR_INPUT_MAX <b input_end) : Bool)
          then (pure (precompile_failure ()))
          else
            (do
              let success ← do
                (accelerator_modexp input bounded_base bounded_exponent bounded_modulus)
              (pure (accelerator_result success bounded_modulus)))))

/-- Decodes an accelerator pairing result: values below two denote malformed
input; the low-order parity of a valid value is the pairing outcome. -/
/- Type quantifiers: result : Nat, 0 ≤ result ∧ result ≤ 3 -/
def pairing_result (result : Nat) : SailM PrecompileResult := do
  if ((result <b 2) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let parity := (Nat.mod result 2)
      (boolean_result (parity == 1)))

/-- `BLAKE2F` (0x09, EIP-152): the compression function; the input must
be exactly 213 bytes with a 0/1 final-block flag. -/
def run_blake2f (input : CalldataSlice) : SailM PrecompileResult := do
  let final_byte ← do (calldata_slice_byte input BLAKE2F_FINAL_BLOCK_OFFSET)
  let input_length := (calldata_slice_length input)
  if (((input_length != BLAKE2F_INPUT_LENGTH) || ((final_byte != 0x00#8) && (final_byte != 0x01#8))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let final_block : Nat :=
        if ((final_byte == 0x00#8) : Bool)
        then 0
        else 1
      let rounds ← do (pc_blake2_rounds input)
      let success ← do (accelerator_blake2f input rounds final_block)
      (pure (accelerator_result success BLAKE2F_OUTPUT_LENGTH)))

/-- The EIP-4844 versioned-hash binding: the input's claimed hash must
equal `0x01 ‖ sha256(commitment)[1:]`. -/
def kzg_versioned_hash_matches (input : CalldataSlice) : SailM Bool := do
  let commitment ← do (calldata_sub_slice input KZG_COMMITMENT_OFFSET KZG_COMMITMENT_LENGTH)
  let commitment_hash ← do (calldata_sha256 commitment)
  let expected := commitment_hash
  let expected : (Vector (BitVec 8) 32) := (vectorUpdate expected 0 0x01#8)
  let claimed_word ← do (calldata_slice_load input 0)
  let claimed_hash := (word_to_hash claimed_word)
  (pure (claimed_hash == expected))

/-- `POINT_EVALUATION` (0x0a, EIP-4844): verifies a KZG proof; success
returns the field-elements-per-blob and BLS modulus constants. -/
def run_kzg_point_evaluation (input : CalldataSlice) : SailM PrecompileResult := do
  let input_length := (calldata_slice_length input)
  if ((input_length != KZG_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let versioned_hash_matches ← do (kzg_versioned_hash_matches input)
      let invalid_versioned_hash := (! versioned_hash_matches)
      if (invalid_versioned_hash : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let valid_proof ← do (accelerator_kzg_point_evaluation input)
          if (valid_proof : Bool)
          then
            (do
              let ⟨_, ⟨_, output⟩⟩ ← do
                (output_buffer_words FIELD_ELEMENTS_PER_BLOB BLS_MODULUS)
              (pure (precompile_success ⟨_, ⟨_, output⟩⟩)))
          else (pure (precompile_failure ()))))

/- Type quantifiers: base : Nat, stride : Nat, count : Nat, (source_valid_range base 64) ∧
  (source_valid_length stride) ∧ (source_valid_length count) -/
def bls_g1_padding (input : CalldataSlice) (base : Nat) (stride : Nat) (count : Nat) : SailM Bool := do
  if ((← (slice_strided_zero input base stride BLS_FIELD_PADDING_LENGTH count)) : Bool)
  then
    (do
      (slice_strided_zero input (base + BLS_PADDED_FIELD_LENGTH) stride BLS_FIELD_PADDING_LENGTH
        count))
  else (pure false)

/- Type quantifiers: base : Nat, stride : Nat, count : Nat, (source_valid_range base 192) ∧
  (source_valid_length stride) ∧ (source_valid_length count) -/
def bls_g2_padding (input : CalldataSlice) (base : Nat) (stride : Nat) (count : Nat) : SailM Bool := do
  if ((← (slice_strided_zero input base stride BLS_FIELD_PADDING_LENGTH count)) : Bool)
  then
    (do
      if ((← (slice_strided_zero input (base + BLS_PADDED_FIELD_LENGTH) stride
             BLS_FIELD_PADDING_LENGTH count)) : Bool)
      then
        (do
          if ((← (slice_strided_zero input (base + BLS_G1_POINT_LENGTH) stride
                 BLS_FIELD_PADDING_LENGTH count)) : Bool)
          then
            (do
              (slice_strided_zero input (base + BLS_G2_FINAL_FIELD_OFFSET) stride
                BLS_FIELD_PADDING_LENGTH count))
          else (pure false))
      else (pure false))
  else (pure false)

/-- `BLS12_G1ADD` (0x0b, EIP-2537). -/
def run_bls_g1_add (input : CalldataSlice) : SailM PrecompileResult := do
  let input_length := (calldata_slice_length input)
  if ((input_length != BLS_G1_ADD_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let valid_padding ← do (bls_g1_padding input 0 BLS_G1_POINT_LENGTH TWO_COMPONENTS)
      let invalid_padding := (! valid_padding)
      if (invalid_padding : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let success ← do (accelerator_bls_g1_add input)
          (pure (accelerator_result success BLS_G1_POINT_LENGTH))))

/-- `BLS12_G1MSM` (0x0c, EIP-2537): input is `k` 160-byte pairs. -/
def run_bls_g1_msm (input : CalldataSlice) : SailM PrecompileResult := do
  let length := (calldata_slice_length input)
  let item_length := BLS_G1_MSM_ITEM_LENGTH
  let pairs := (length / item_length)
  if (((length == 0) || (length != (pairs *i item_length))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let valid_padding ← do (bls_g1_padding input 0 BLS_G1_MSM_ITEM_LENGTH pairs)
      let invalid_padding := (! valid_padding)
      if (invalid_padding : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let success ← do (accelerator_bls_g1_msm input)
          (pure (accelerator_result success BLS_G1_POINT_LENGTH))))

/-- `BLS12_G2ADD` (0x0d, EIP-2537). -/
def run_bls_g2_add (input : CalldataSlice) : SailM PrecompileResult := do
  let input_length := (calldata_slice_length input)
  if ((input_length != BLS_G2_ADD_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let valid_padding ← do (bls_g2_padding input 0 BLS_G2_POINT_LENGTH TWO_COMPONENTS)
      let invalid_padding := (! valid_padding)
      if (invalid_padding : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let success ← do (accelerator_bls_g2_add input)
          (pure (accelerator_result success BLS_G2_POINT_LENGTH))))

/-- `BLS12_G2MSM` (0x0e, EIP-2537): input is `k` 288-byte pairs. -/
def run_bls_g2_msm (input : CalldataSlice) : SailM PrecompileResult := do
  let length := (calldata_slice_length input)
  let item_length := BLS_G2_MSM_ITEM_LENGTH
  let pairs := (length / item_length)
  if (((length == 0) || (length != (pairs *i item_length))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let valid_padding ← do (bls_g2_padding input 0 BLS_G2_MSM_ITEM_LENGTH pairs)
      let invalid_padding := (! valid_padding)
      if (invalid_padding : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let success ← do (accelerator_bls_g2_msm input)
          (pure (accelerator_result success BLS_G2_POINT_LENGTH))))

/-- `BLS12_PAIRING_CHECK` (0x0f, EIP-2537): input is `k` 384-byte
G1×G2 pairs. -/
def run_bls_pairing (input : CalldataSlice) : SailM PrecompileResult := do
  let length := (calldata_slice_length input)
  let item_length := BLS_PAIRING_ITEM_LENGTH
  let pairs := (length / item_length)
  if (((length == 0) || (length != (pairs *i item_length))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let pair_count : Nat := pairs
      let valid_g1_padding ← do (bls_g1_padding input 0 BLS_PAIRING_ITEM_LENGTH pair_count)
      let valid_g2_padding ← do
        (bls_g2_padding input BLS_G2_POINT_OFFSET BLS_PAIRING_ITEM_LENGTH pair_count)
      if ((valid_g1_padding && valid_g2_padding) : Bool)
      then
        (do
          let result ← do (accelerator_bls_pairing input)
          (pairing_result result))
      else (pure (precompile_failure ())))

/-- `BLS12_MAP_FP_TO_G1` (0x10, EIP-2537). -/
def run_bls_map_fp_to_g1 (input : CalldataSlice) : SailM PrecompileResult := do
  let input_length := (calldata_slice_length input)
  if ((input_length != BLS_PADDED_FIELD_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let valid_padding ← do
        (slice_strided_zero input 0 BLS_PADDED_FIELD_LENGTH BLS_FIELD_PADDING_LENGTH 1)
      let invalid_padding := (! valid_padding)
      if (invalid_padding : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let success ← do (accelerator_bls_map_fp_to_g1 input)
          (pure (accelerator_result success BLS_G1_POINT_LENGTH))))

/-- `BLS12_MAP_FP2_TO_G2` (0x11, EIP-2537). -/
def run_bls_map_fp2_to_g2 (input : CalldataSlice) : SailM PrecompileResult := do
  let input_length := (calldata_slice_length input)
  if ((input_length != BLS_G1_POINT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let valid_padding ← do
        (slice_strided_zero input 0 BLS_PADDED_FIELD_LENGTH BLS_FIELD_PADDING_LENGTH TWO_COMPONENTS)
      let invalid_padding := (! valid_padding)
      if (invalid_padding : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          let success ← do (accelerator_bls_map_fp2_to_g2 input)
          (pure (accelerator_result success BLS_G2_POINT_LENGTH))))

/-- `P256VERIFY` (0x100, EIP-7951): every malformed or invalid signature
is a successful call with empty output; only a valid signature
returns the word one. -/
def run_p256_verify (input : CalldataSlice) : SailM PrecompileResult := do
  let input_length := (calldata_slice_length input)
  let verified ← do
    if ((input_length == P256_INPUT_LENGTH) : Bool)
    then (accelerator_p256_verify input)
    else (pure false)
  if (verified : Bool)
  then
    (do
      let ⟨_, ⟨_, output⟩⟩ ← do (output_buffer_word WORD_ONE)
      (pure (precompile_success ⟨_, ⟨_, output⟩⟩)))
  else (pure (precompile_success ⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩))

/-- The precompile dispatch: address to implementation. Gas has already
been charged by the caller ([precompile_gas][]). -/
def run_precompile_slice (num : PrecompileId) (input : CalldataSlice) : SailM PrecompileResult := do
  match num with
  | .NotPrecompile => (pure (precompile_failure ()))
  | .Ecrecover => (run_ecrecover input)
  | .Sha256 => (run_sha256 input)
  | .Ripemd160 => (run_ripemd160 input)
  | .Identity => (copied_result input)
  | .Modexp => (run_modexp input)
  | .Bn254Add =>
    (do
      let success ← do (accelerator_bn254_add input)
      (pure (accelerator_result success PRECOMPILE_DOUBLE_WORD_LENGTH)))
  | .Bn254Mul =>
    (do
      let success ← do (accelerator_bn254_mul input)
      (pure (accelerator_result success PRECOMPILE_DOUBLE_WORD_LENGTH)))
  | .Bn254Pairing =>
    (do
      let input_length := (calldata_slice_length input)
      let item_length := BN254_PAIRING_ITEM_LENGTH
      if ((input_length == ((input_length / item_length) *i item_length)) : Bool)
      then
        (do
          let result ← do (accelerator_bn254_pairing input)
          (pairing_result result))
      else (pure (precompile_failure ())))
  | .Blake2f => (run_blake2f input)
  | .KzgPointEvaluation => (run_kzg_point_evaluation input)
  | .BlsG1Add => (run_bls_g1_add input)
  | .BlsG1Msm => (run_bls_g1_msm input)
  | .BlsG2Add => (run_bls_g2_add input)
  | .BlsG2Msm => (run_bls_g2_msm input)
  | .BlsPairing => (run_bls_pairing input)
  | .BlsMapFpToG1 => (run_bls_map_fp_to_g1 input)
  | .BlsMapFp2ToG2 => (run_bls_map_fp2_to_g2 input)
  | .P256Verify => (run_p256_verify input)

