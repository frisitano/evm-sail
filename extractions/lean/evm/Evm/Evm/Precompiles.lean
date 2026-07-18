import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Host.EvmByteSlice
import Evm.Host.Output
import Evm.Host.Kernel.Environment
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

def ACCELERATOR_INPUT_MAX : Nat := 2097152

def FIELD_ELEMENTS_PER_BLOB : word :=
  0x0000000000000000000000000000000000000000000000000000000000001000#256

def BLS_MODULUS : word := 0x73EDA753299D7D483339D80809A1D80553BDA402FFFE5BFEFFFFFFFF00000001#256

def BLAKE2F_INPUT_LENGTH : byte_length := (ByteQuantity 213)

def BLAKE2F_FINAL_BLOCK_OFFSET : source_pointer := (ByteQuantity 212)

def BLAKE2F_OUTPUT_LENGTH : byte_length := DOUBLE_WORD_BYTE_LENGTH

def KZG_INPUT_LENGTH : byte_length := (ByteQuantity 192)

def KZG_COMMITMENT_OFFSET : source_pointer := (ByteQuantity 96)

def KZG_COMMITMENT_LENGTH : byte_length := (ByteQuantity 48)

def BLS_FIELD_PADDING_LENGTH : byte_length := (ByteQuantity 16)

def BLS_PADDED_FIELD_LENGTH : byte_length := DOUBLE_WORD_BYTE_LENGTH

def BLS_G1_POINT_LENGTH : byte_length := (ByteQuantity 128)

def BLS_G2_POINT_LENGTH : byte_length := (ByteQuantity 256)

def BLS_G2_FINAL_FIELD_OFFSET : source_pointer := (ByteQuantity 192)

def BLS_G1_ADD_INPUT_LENGTH : byte_length := (ByteQuantity 256)

def BLS_G1_MSM_ITEM_LENGTH : byte_length := (ByteQuantity 160)

def BLS_G2_ADD_INPUT_LENGTH : byte_length := (ByteQuantity 512)

def BLS_G2_MSM_ITEM_LENGTH : byte_length := (ByteQuantity 288)

def BLS_PAIRING_ITEM_LENGTH : byte_length := (ByteQuantity 384)

def P256_INPUT_LENGTH : byte_length := (ByteQuantity 160)

def BN254_PAIRING_ITEM_LENGTH : byte_length := (ByteQuantity 192)

def PRECOMPILE_WORD_LENGTH : byte_length := WORD_BYTE_LENGTH

def PRECOMPILE_DOUBLE_WORD_LENGTH : byte_length := DOUBLE_WORD_BYTE_LENGTH

def ECRECOVER_S_OFFSET : source_pointer := (ByteQuantity 96)

def TWO_COMPONENTS : byte_length := (ByteQuantity 2)

def precompile_success (output : EvmByteSlice) : PrecompileResult :=
  { success := true,
    output := output }

def precompile_failure (_ : Unit) : PrecompileResult :=
  { success := false,
    output := EMPTY_SLICE }

/-- Type quantifiers: k_ex160956_ : Bool -/
def accelerator_result (success : Bool) (output_len : byte_length) : PrecompileResult :=
  if (success : Bool)
  then (precompile_success (output_buffer_slice output_len))
  else (precompile_failure ())

def copied_result (data : EvmByteSlice) : SailM PrecompileResult := do
  let output ← do (freeze_output data)
  if ((byte_quantity_equal output.len data.len) : Bool)
  then (pure (precompile_success output))
  else (pure (precompile_failure ()))

/-- Type quantifiers: k_ex160957_ : Bool -/
def boolean_result (value : Bool) : SailM PrecompileResult := do
  (pure (precompile_success
      (← (output_buffer_word
          (if (value : Bool)
          then WORD_ONE
          else WORD_ZERO)))))

/-- Type quantifiers: n : Nat, 1 ≤ n ∧ n ≤ 256 -/
def is_precompile (n : precompile_id) : SailM Bool := do
  let n := (n).value
  let base := (n ≤b 4)
  let byzantium ← do
    (pure ((5 ≤b n) && ((n ≤b 8) && (fork_gteq (← readReg k_fork) Byzantium))))
  let istanbul ← do (pure ((n == 9) && (fork_gteq (← readReg k_fork) Istanbul)))
  let kzg ← do (pure ((n == 10) && (fork_gteq (← readReg k_fork) Cancun)))
  let bls ← do (pure ((11 ≤b n) && ((n ≤b 17) && (fork_gteq (← readReg k_fork) Prague))))
  let p256 ← do (pure ((n == 256) && (fork_gteq (← readReg k_fork) Osaka)))
  (pure (base || (byzantium || (istanbul || (kzg || (bls || p256))))))

def run_ecrecover (input : EvmByteSlice) : SailM PrecompileResult := do
  let v ← do (slice_load input PRECOMPILE_WORD_LENGTH)
  let valid_v ← do (pure ((v == (← (word_of_nat 27))) || (v == (← (word_of_nat 28)))))
  if (valid_v : Bool)
  then
    (do
      let parity ← (( do
        if ((v == (← (word_of_nat 27))) : Bool)
        then (pure 0)
        else (pure 1) ) : SailM Nat )
      let (recovered, address) ← do
        (ecrecover_addr (← (slice_load input BYTE_ZERO)) ⟨parity⟩
          (← (slice_load input PRECOMPILE_DOUBLE_WORD_LENGTH))
          (← (slice_load input ECRECOVER_S_OFFSET)))
      if (recovered : Bool)
      then (pure (precompile_success (← (output_buffer_word (address_to_word address)))))
      else (pure (precompile_success EMPTY_SLICE)))
  else (pure (precompile_success EMPTY_SLICE))

def run_sha256 (input : EvmByteSlice) : SailM PrecompileResult := do
  (pure (precompile_success (← (output_buffer_word (← (sha256_segments [(BytesSlice input)]))))))

def run_ripemd160 (input : EvmByteSlice) : SailM PrecompileResult := do
  (pure (accelerator_result (← (accelerator_ripemd160 input)) PRECOMPILE_WORD_LENGTH))

def run_modexp (input : EvmByteSlice) : SailM PrecompileResult := do
  let base_len ← (( do (pure (BitVec.toNatInt (← (pc_word input 0 32)))) ) : SailM Nat )
  let exponent_len ← (( do (pure (BitVec.toNatInt (← (pc_word input 32 32)))) ) : SailM Nat )
  let modulus_len ← (( do (pure (BitVec.toNatInt (← (pc_word input 64 32)))) ) : SailM Nat )
  if ((modulus_len == 0) : Bool)
  then (pure (precompile_success EMPTY_SLICE))
  else
    (do
      let input_end := (((96 + base_len) + exponent_len) + modulus_len)
      if (((ACCELERATOR_INPUT_MAX <b base_len) || ((ACCELERATOR_INPUT_MAX <b exponent_len) || ((ACCELERATOR_INPUT_MAX <b modulus_len) || (ACCELERATOR_INPUT_MAX <b input_end)))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result
            (← (accelerator_modexp input (ByteQuantity base_len) (ByteQuantity exponent_len)
                (ByteQuantity modulus_len))) (ByteQuantity modulus_len))))

def pairing_result (result : (BitVec 2)) : SailM PrecompileResult := do
  if (((BitVec.access result 1) == 0#1) : Bool)
  then (pure (precompile_failure ()))
  else (boolean_result ((BitVec.access result 0) == 1#1))

def run_blake2f (input : EvmByteSlice) : SailM PrecompileResult := do
  let final_byte ← do (slice_byte input BLAKE2F_FINAL_BLOCK_OFFSET)
  if (((byte_quantity_not_equal input.len BLAKE2F_INPUT_LENGTH) || ((final_byte != 0x00#8) && (final_byte != 0x01#8))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      let final_block : Nat :=
        if ((final_byte == 0x00#8) : Bool)
        then 0
        else 1
      (pure (accelerator_result
          (← (accelerator_blake2f input ⟨((← (pc_blake2_rounds input))).value⟩
              ⟨final_block⟩)) BLAKE2F_OUTPUT_LENGTH)))

def kzg_versioned_hash_matches (input : EvmByteSlice) : SailM Bool := do
  let commitment_hash ← do
    (sha256_segments
      [(BytesSlice (← (sub_slice input KZG_COMMITMENT_OFFSET KZG_COMMITMENT_LENGTH)))])
  let expected : (BitVec 256) := (0x01#8 +++ (Sail.BitVec.extractLsb commitment_hash 247 0))
  (pure ((← (slice_load input BYTE_ZERO)) == expected))

def run_kzg_point_evaluation (input : EvmByteSlice) : SailM PrecompileResult := do
  if ((byte_quantity_not_equal input.len KZG_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (kzg_versioned_hash_matches input))) : Bool)
      then (pure (precompile_failure ()))
      else
        (do
          if ((← (accelerator_kzg_point_evaluation input)) : Bool)
          then
            (pure (precompile_success
                (← (output_buffer_words FIELD_ELEMENTS_PER_BLOB BLS_MODULUS))))
          else (pure (precompile_failure ()))))

def bls_g1_padding (input : EvmByteSlice) (base : source_pointer) (stride : byte_length) (count : byte_length) : SailM Bool := do
  (pure ((← (slice_strided_zero input base stride BLS_FIELD_PADDING_LENGTH count)) && (← (slice_strided_zero
          input (← (byte_quantity_add base BLS_PADDED_FIELD_LENGTH)) stride
          BLS_FIELD_PADDING_LENGTH count))))

def bls_g2_padding (input : EvmByteSlice) (base : source_pointer) (stride : byte_length) (count : byte_length) : SailM Bool := do
  (pure ((← (slice_strided_zero input base stride BLS_FIELD_PADDING_LENGTH count)) && ((← (slice_strided_zero
            input (← (byte_quantity_add base BLS_PADDED_FIELD_LENGTH)) stride
            BLS_FIELD_PADDING_LENGTH count)) && ((← (slice_strided_zero input
              (← (byte_quantity_add base BLS_G1_POINT_LENGTH)) stride BLS_FIELD_PADDING_LENGTH
              count)) && (← (slice_strided_zero input
              (← (byte_quantity_add base BLS_G2_FINAL_FIELD_OFFSET)) stride
              BLS_FIELD_PADDING_LENGTH count))))))

def run_bls_g1_add (input : EvmByteSlice) : SailM PrecompileResult := do
  if ((byte_quantity_not_equal input.len BLS_G1_ADD_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g1_padding input BYTE_ZERO BLS_G1_POINT_LENGTH TWO_COMPONENTS))) : Bool)
      then (pure (precompile_failure ()))
      else (pure (accelerator_result (← (accelerator_bls_g1_add input)) BLS_G1_POINT_LENGTH)))

def run_bls_g1_msm (input : EvmByteSlice) : SailM PrecompileResult := do
  let length := input.len
  let pairs ← do (byte_quantity_quotient length BLS_G1_MSM_ITEM_LENGTH)
  if (((byte_quantity_equal length BYTE_ZERO) || (byte_quantity_not_equal length
         (← (byte_quantity_mul pairs BLS_G1_MSM_ITEM_LENGTH)))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g1_padding input BYTE_ZERO BLS_G1_MSM_ITEM_LENGTH pairs))) : Bool)
      then (pure (precompile_failure ()))
      else (pure (accelerator_result (← (accelerator_bls_g1_msm input)) BLS_G1_POINT_LENGTH)))

def run_bls_g2_add (input : EvmByteSlice) : SailM PrecompileResult := do
  if ((byte_quantity_not_equal input.len BLS_G2_ADD_INPUT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g2_padding input BYTE_ZERO BLS_G2_POINT_LENGTH TWO_COMPONENTS))) : Bool)
      then (pure (precompile_failure ()))
      else (pure (accelerator_result (← (accelerator_bls_g2_add input)) BLS_G2_POINT_LENGTH)))

def run_bls_g2_msm (input : EvmByteSlice) : SailM PrecompileResult := do
  let length := input.len
  let pairs ← do (byte_quantity_quotient length BLS_G2_MSM_ITEM_LENGTH)
  if (((byte_quantity_equal length BYTE_ZERO) || (byte_quantity_not_equal length
         (← (byte_quantity_mul pairs BLS_G2_MSM_ITEM_LENGTH)))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (bls_g2_padding input BYTE_ZERO BLS_G2_MSM_ITEM_LENGTH pairs))) : Bool)
      then (pure (precompile_failure ()))
      else (pure (accelerator_result (← (accelerator_bls_g2_msm input)) BLS_G2_POINT_LENGTH)))

def run_bls_pairing (input : EvmByteSlice) : SailM PrecompileResult := do
  let length := input.len
  let pairs ← do (byte_quantity_quotient length BLS_PAIRING_ITEM_LENGTH)
  if (((byte_quantity_equal length BYTE_ZERO) || (byte_quantity_not_equal length
         (← (byte_quantity_mul pairs BLS_PAIRING_ITEM_LENGTH)))) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if (((← (bls_g1_padding input BYTE_ZERO BLS_PAIRING_ITEM_LENGTH pairs)) && (← (bls_g2_padding
               input BLS_G1_POINT_LENGTH BLS_PAIRING_ITEM_LENGTH pairs))) : Bool)
      then (pairing_result (← (accelerator_bls_pairing input)))
      else (pure (precompile_failure ())))

def run_bls_map_fp_to_g1 (input : EvmByteSlice) : SailM PrecompileResult := do
  if ((byte_quantity_not_equal input.len BLS_PADDED_FIELD_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (slice_strided_zero input BYTE_ZERO BLS_PADDED_FIELD_LENGTH
               BLS_FIELD_PADDING_LENGTH BYTE_ONE))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_map_fp_to_g1 input)) BLS_G1_POINT_LENGTH)))

def run_bls_map_fp2_to_g2 (input : EvmByteSlice) : SailM PrecompileResult := do
  if ((byte_quantity_not_equal input.len BLS_G1_POINT_LENGTH) : Bool)
  then (pure (precompile_failure ()))
  else
    (do
      if ((! (← (slice_strided_zero input BYTE_ZERO BLS_PADDED_FIELD_LENGTH
               BLS_FIELD_PADDING_LENGTH TWO_COMPONENTS))) : Bool)
      then (pure (precompile_failure ()))
      else
        (pure (accelerator_result (← (accelerator_bls_map_fp2_to_g2 input)) BLS_G2_POINT_LENGTH)))

def run_p256_verify (input : EvmByteSlice) : SailM PrecompileResult := do
  let verified ← do
    if ((byte_quantity_equal input.len P256_INPUT_LENGTH) : Bool)
    then (accelerator_p256_verify input)
    else (pure false)
  if (verified : Bool)
  then (pure (precompile_success (← (output_buffer_word WORD_ONE))))
  else (pure (precompile_success EMPTY_SLICE))

/-- Type quantifiers: k_ex160960_ : Nat, 1 ≤ k_ex160960_ ∧ k_ex160960_ ≤ 256 -/
def run_precompile_slice (num : precompile_id) (input : EvmByteSlice) : SailM PrecompileResult := do
  let num := (num).value
  match num with
  | 1 => (run_ecrecover input)
  | 2 => (run_sha256 input)
  | 3 => (run_ripemd160 input)
  | 4 => (copied_result input)
  | 5 => (run_modexp input)
  | 6 =>
    (pure (accelerator_result (← (accelerator_bn254_add input)) PRECOMPILE_DOUBLE_WORD_LENGTH))
  | 7 =>
    (pure (accelerator_result (← (accelerator_bn254_mul input)) PRECOMPILE_DOUBLE_WORD_LENGTH))
  | 8 =>
    (do
      if ((byte_quantity_equal input.len
           (← (byte_quantity_mul
               (← (byte_quantity_quotient input.len BN254_PAIRING_ITEM_LENGTH))
               BN254_PAIRING_ITEM_LENGTH))) : Bool)
      then (pairing_result (← (accelerator_bn254_pairing input)))
      else (pure (precompile_failure ())))
  | 9 => (run_blake2f input)
  | 10 => (run_kzg_point_evaluation input)
  | 11 => (run_bls_g1_add input)
  | 12 => (run_bls_g1_msm input)
  | 13 => (run_bls_g2_add input)
  | 14 => (run_bls_g2_msm input)
  | 15 => (run_bls_pairing input)
  | 16 => (run_bls_map_fp_to_g1 input)
  | 17 => (run_bls_map_fp2_to_g2 input)
  | 256 => (run_p256_verify input)
  | _ => (pure (precompile_failure ()))

