#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const bytes20 PRECOMPILE_ADDRESS_1 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(16777216)
  },
};


const bytes20 PRECOMPILE_ADDRESS_10 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(167772160)
  },
};


const bytes20 PRECOMPILE_ADDRESS_11 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(184549376)
  },
};


const bytes20 PRECOMPILE_ADDRESS_12 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(201326592)
  },
};


const bytes20 PRECOMPILE_ADDRESS_13 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(218103808)
  },
};


const bytes20 PRECOMPILE_ADDRESS_14 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(234881024)
  },
};


const bytes20 PRECOMPILE_ADDRESS_15 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(251658240)
  },
};


const bytes20 PRECOMPILE_ADDRESS_16 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(268435456)
  },
};


const bytes20 PRECOMPILE_ADDRESS_17 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(285212672)
  },
};


const bytes20 PRECOMPILE_ADDRESS_2 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(33554432)
  },
};


const bytes20 PRECOMPILE_ADDRESS_256 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(65536)
  },
};


const bytes20 PRECOMPILE_ADDRESS_3 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(50331648)
  },
};


const bytes20 PRECOMPILE_ADDRESS_4 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(67108864)
  },
};


const bytes20 PRECOMPILE_ADDRESS_5 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(83886080)
  },
};


const bytes20 PRECOMPILE_ADDRESS_6 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(100663296)
  },
};


const bytes20 PRECOMPILE_ADDRESS_7 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(117440512)
  },
};


const bytes20 PRECOMPILE_ADDRESS_8 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(134217728)
  },
};


const bytes20 PRECOMPILE_ADDRESS_9 = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(150994944)
  },
};


const uint32_t ACCELERATOR_INPUT_MAX = UINT32_C(2097152);


const u256 FIELD_ELEMENTS_PER_BLOB = (u256){{UINT64_C(4096), UINT64_C(0), UINT64_C(0), UINT64_C(0)}};


const u256 BLS_MODULUS = (u256){{UINT64_C(18446744069414584321), UINT64_C(6034159408538082302), UINT64_C(3691218898639771653), UINT64_C(8353516859464449352)}};


const uint8_t BLAKE2F_INPUT_LENGTH = UINT8_C(213);


const uint8_t BLAKE2F_FINAL_BLOCK_OFFSET = UINT8_C(212);


const uint8_t BLAKE2F_OUTPUT_LENGTH = UINT8_C(64);


const uint8_t KZG_INPUT_LENGTH = UINT8_C(192);


const uint8_t KZG_COMMITMENT_OFFSET = UINT8_C(96);


const uint8_t KZG_COMMITMENT_LENGTH = UINT8_C(48);


const uint8_t BLS_FIELD_PADDING_LENGTH = UINT8_C(16);


const uint8_t BLS_PADDED_FIELD_LENGTH = UINT8_C(64);


const uint8_t BLS_G1_POINT_LENGTH = UINT8_C(128);


const uint16_t BLS_G2_POINT_LENGTH = UINT16_C(256);


const uint8_t BLS_G2_FINAL_FIELD_OFFSET = UINT8_C(192);


const uint16_t BLS_G1_ADD_INPUT_LENGTH = UINT16_C(256);


const uint8_t BLS_G1_MSM_ITEM_LENGTH = UINT8_C(160);


const uint16_t BLS_G2_ADD_INPUT_LENGTH = UINT16_C(512);


const uint16_t BLS_G2_MSM_ITEM_LENGTH = UINT16_C(288);


const uint16_t BLS_PAIRING_ITEM_LENGTH = UINT16_C(384);


const uint8_t P256_INPUT_LENGTH = UINT8_C(160);


const uint8_t BN254_PAIRING_ITEM_LENGTH = UINT8_C(192);


const uint8_t PRECOMPILE_WORD_LENGTH = UINT8_C(32);


const uint8_t PRECOMPILE_DOUBLE_WORD_LENGTH = UINT8_C(64);


const uint8_t PRECOMPILE_WORD_OFFSET = UINT8_C(32);


const uint8_t PRECOMPILE_DOUBLE_WORD_OFFSET = UINT8_C(64);


const uint32_t ECRECOVER_S_OFFSET = UINT32_C(96);


const uint8_t TWO_COMPONENTS = UINT8_C(2);


const uint8_t BLS_G2_POINT_OFFSET = UINT8_C(128);


bool precompile_active_at_fork(enum PrecompileId n)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  switch (n) {
  case NotPrecompile:
    return false;
  case Ecrecover:
  case Sha256:
  case Ripemd160:
  case Identity:
    return true;
  case Modexp:
  case Bn254Add:
  case Bn254Mul:
  case Bn254Pairing:
    return (bool)(execution_profile.protocol.fork >= Byzantium);
  case Blake2f:
    return (bool)(execution_profile.protocol.fork >= Istanbul);
  case KzgPointEvaluation:
    return (bool)(execution_profile.protocol.fork >= Cancun);
  case BlsG1Add:
  case BlsG1Msm:
  case BlsG2Add:
  case BlsG2Msm:
  case BlsPairing:
  case BlsMapFpToG1:
  case BlsMapFp2ToG2:
    return (bool)(execution_profile.protocol.fork >= Prague);
  case P256Verify:
    return (bool)(execution_profile.protocol.fork >= Osaka);
  }
}

enum PrecompileId precompile_id_if_active(enum PrecompileId candidate)
{
  bool active = precompile_active_at_fork(candidate);
  if (active) {
    return candidate;
  }
  return NotPrecompile;
}

enum PrecompileId precompile_id_for_address(bytes20 bytes)
{
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_1)) {
    return precompile_id_if_active(Ecrecover);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_2)) {
    return precompile_id_if_active(Sha256);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_3)) {
    return precompile_id_if_active(Ripemd160);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_4)) {
    return precompile_id_if_active(Identity);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_5)) {
    return precompile_id_if_active(Modexp);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_6)) {
    return precompile_id_if_active(Bn254Add);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_7)) {
    return precompile_id_if_active(Bn254Mul);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_8)) {
    return precompile_id_if_active(Bn254Pairing);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_9)) {
    return precompile_id_if_active(Blake2f);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_10)) {
    return precompile_id_if_active(KzgPointEvaluation);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_11)) {
    return precompile_id_if_active(BlsG1Add);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_12)) {
    return precompile_id_if_active(BlsG1Msm);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_13)) {
    return precompile_id_if_active(BlsG2Add);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_14)) {
    return precompile_id_if_active(BlsG2Msm);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_15)) {
    return precompile_id_if_active(BlsPairing);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_16)) {
    return precompile_id_if_active(BlsMapFpToG1);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_17)) {
    return precompile_id_if_active(BlsMapFp2ToG2);
  }
  if (eq_bytes20(bytes, PRECOMPILE_ADDRESS_256)) {
    return precompile_id_if_active(P256Verify);
  }
  return NotPrecompile;
}

struct PrecompileResult precompile_success(Bytes output)
{
  return ((struct PrecompileResult){.output = output, .success = true});
}

struct PrecompileResult precompile_failure(void)
{
  return ((struct PrecompileResult){.output = EMPTY_OUTPUT_SLICE, .success = false});
}

struct PrecompileResult copied_result(struct CalldataSlice data)
{
  Bytes output = freeze_calldata_output(data);
  uint32_t input_length = calldata_slice_length(data);
  if (output.len == input_length) {
    return precompile_success(output);
  }
  return precompile_failure();
}

struct PrecompileResult boolean_result(bool value)
{
  u256 result_word = value ? WORD_ONE : WORD_ZERO;
  Bytes output = output_buffer_word(result_word);
  return precompile_success(output);
}

struct PrecompileResult run_ecrecover(struct CalldataSlice input)
{
  u256 v = calldata_slice_load_struct_CalldataSlice_uint8_t_to_u256(input, PRECOMPILE_WORD_OFFSET);
  if (eq_u256(v, (u256){{UINT64_C(27), UINT64_C(0), UINT64_C(0), UINT64_C(0)}}) || eq_u256(v, (u256){{UINT64_C(28), UINT64_C(0), UINT64_C(0), UINT64_C(0)}})) {
    uint8_t parity = eq_u256(v, (u256){{UINT64_C(27), UINT64_C(0), UINT64_C(0), UINT64_C(0)}}) ? UINT8_C(0) : UINT8_C(1);
    u256 message_word = calldata_slice_load_struct_CalldataSlice_uint8_t_to_u256(input, UINT8_C(0));
    bytes32 message_hash = word_to_hash(message_word);
    u256 r = calldata_slice_load_struct_CalldataSlice_uint8_t_to_u256(input, PRECOMPILE_DOUBLE_WORD_OFFSET);
    u256 s = calldata_slice_load(input, ECRECOVER_S_OFFSET);
    struct AddressResult recovered = ecrecover_addr(message_hash, parity, r, s);
    if (recovered.success) {
      u256 address_word = address_to_word(recovered.address);
      Bytes output = output_buffer_word(address_word);
      return precompile_success(output);
    }
    return precompile_success(EMPTY_OUTPUT_SLICE);
  }
  return precompile_success(EMPTY_OUTPUT_SLICE);
}

struct PrecompileResult run_sha256(struct CalldataSlice input)
{
  bytes32 digest = calldata_sha256(input);
  u256 digest_word = hash_to_word(digest);
  Bytes output = output_buffer_word(digest_word);
  return precompile_success(output);
}

struct PrecompileResult run_ripemd160(struct CalldataSlice input)
{
  bool success = accelerator_ripemd160(input);
  return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success, PRECOMPILE_WORD_LENGTH);
}

struct PrecompileResult run_modexp(struct CalldataSlice input)
{
  u256 base_len = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(0), UINT8_C(32));
  u256 exponent_len = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(32), UINT8_C(32));
  u256 modulus_len = pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(input, UINT8_C(64), UINT8_C(32));
  if (u256_eq_u64(modulus_len, UINT8_C(0))) {
    return precompile_success(EMPTY_OUTPUT_SLICE);
  }
  bool lt_int_result_2_1819 = u64_lt_u256(ACCELERATOR_INPUT_MAX, base_len);
  bool tmp_3_2963;
  if (lt_int_result_2_1819) {
    tmp_3_2963 = true;
  } else {
    bool lt_int_result_2_1818 = u64_lt_u256(ACCELERATOR_INPUT_MAX, exponent_len);
    tmp_3_2963 = (bool)(lt_int_result_2_1818 || u64_lt_u256(ACCELERATOR_INPUT_MAX, modulus_len));
  }
  if (tmp_3_2963) {
    return precompile_failure();
  }
  uint32_t bounded_base = (uint32_t)u256_to_u64(base_len);
  uint32_t bounded_exponent = (uint32_t)u256_to_u64(exponent_len);
  uint32_t bounded_modulus = (uint32_t)u256_to_u64(modulus_len);
  uint32_t input_end;
  uint32_t result_2_1823 = (uint32_t)u320_to_u64(u320_add(u320_of_u64((bounded_base + UINT32_C(96))), u320_of_u64(bounded_exponent)));
  input_end = (uint32_t)u320_to_u64_unchecked(u320_add(u320_of_u64(result_2_1823), u320_of_u64(bounded_modulus)));
  if (ACCELERATOR_INPUT_MAX < input_end) {
    return precompile_failure();
  }
  bool success = accelerator_modexp(input, bounded_base, bounded_exponent, bounded_modulus);
  return accelerator_result_bool_uint32_t_to_struct_PrecompileResult(success, bounded_modulus);
}

struct PrecompileResult pairing_result(uint8_t result)
{
  if (result < UINT8_C(2)) {
    return precompile_failure();
  }
  return boolean_result((bool)((result & UINT8_C(1)) == UINT8_C(1)));
}

struct PrecompileResult run_blake2f(struct CalldataSlice input)
{
  uint64_t final_byte = calldata_slice_byte_struct_CalldataSlice_uint8_t_to_uint64_t(input, BLAKE2F_FINAL_BLOCK_OFFSET);
  uint32_t input_length = calldata_slice_length(input);
  bool neq_int_result_2_1812 = (bool)(input_length != BLAKE2F_INPUT_LENGTH);
  if (neq_int_result_2_1812 || ((final_byte != UINT64_C(0x00)) && (final_byte != UINT64_C(0x01)))) {
    return precompile_failure();
  }
  uint8_t final_block = final_byte == UINT64_C(0x00) ? UINT8_C(0) : UINT8_C(1);
  uint32_t rounds = pc_blake2_rounds(input);
  bool success = accelerator_blake2f(input, rounds, final_block);
  return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success, BLAKE2F_OUTPUT_LENGTH);
}

bool kzg_versioned_hash_matches(struct CalldataSlice input)
{
  struct CalldataSlice commitment = calldata_sub_slice(input, KZG_COMMITMENT_OFFSET, KZG_COMMITMENT_LENGTH);
  bytes32 commitment_hash = calldata_sha256(commitment);
  bytes32 expected = commitment_hash;
  expected = fast_unsigned_vector_update_bytes32(expected, UINT8_C(0), UINT64_C(0x01));
  u256 claimed_word = calldata_slice_load_struct_CalldataSlice_uint8_t_to_u256(input, UINT8_C(0));
  bytes32 claimed_hash = word_to_hash(claimed_word);
  return eq_bytes32(claimed_hash, expected);
}

struct PrecompileResult run_kzg_point_evaluation(struct CalldataSlice input)
{
  uint32_t input_length = calldata_slice_length(input);
  if (input_length != KZG_INPUT_LENGTH) {
    return precompile_failure();
  }
  bool versioned_hash_matches = kzg_versioned_hash_matches(input);
  if (versioned_hash_matches) {
    bool valid_proof = accelerator_kzg_point_evaluation(input);
    if (valid_proof) {
      Bytes output = output_buffer_words(FIELD_ELEMENTS_PER_BLOB, BLS_MODULUS);
      return precompile_success(output);
    }
    return precompile_failure();
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_g1_add(struct CalldataSlice input)
{
  uint32_t input_length = calldata_slice_length(input);
  if (input_length != BLS_G1_ADD_INPUT_LENGTH) {
    return precompile_failure();
  }
  bool valid_padding = bls_g1_padding_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_to_bool(input, UINT8_C(0), BLS_G1_POINT_LENGTH, TWO_COMPONENTS);
  if (valid_padding) {
    bool success = accelerator_bls_g1_add(input);
    return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success, BLS_G1_POINT_LENGTH);
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_g1_msm(struct CalldataSlice input)
{
  uint32_t length_ = calldata_slice_length(input);
  uint8_t item_length = BLS_G1_MSM_ITEM_LENGTH;
  uint32_t pairs = (length_ / (uint32_t)item_length);
  if ((length_ == UINT8_C(0)) || (length_ != (pairs * (uint32_t)item_length))) {
    return precompile_failure();
  }
  bool valid_padding = bls_g1_padding_struct_CalldataSlice_uint8_t_uint8_t_uint32_t_to_bool(input, UINT8_C(0), BLS_G1_MSM_ITEM_LENGTH, pairs);
  if (valid_padding) {
    bool success = accelerator_bls_g1_msm(input);
    return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success, BLS_G1_POINT_LENGTH);
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_g2_add(struct CalldataSlice input)
{
  uint32_t input_length = calldata_slice_length(input);
  if (input_length != BLS_G2_ADD_INPUT_LENGTH) {
    return precompile_failure();
  }
  bool valid_padding = bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_to_bool(input, UINT8_C(0), BLS_G2_POINT_LENGTH, TWO_COMPONENTS);
  if (valid_padding) {
    bool success = accelerator_bls_g2_add(input);
    return accelerator_result_bool_uint16_t_to_struct_PrecompileResult(success, BLS_G2_POINT_LENGTH);
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_g2_msm(struct CalldataSlice input)
{
  uint32_t length_ = calldata_slice_length(input);
  uint16_t item_length = BLS_G2_MSM_ITEM_LENGTH;
  uint32_t pairs = (length_ / (uint32_t)item_length);
  if ((length_ == UINT8_C(0)) || (length_ != (pairs * (uint32_t)item_length))) {
    return precompile_failure();
  }
  bool valid_padding = bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(input, UINT8_C(0), BLS_G2_MSM_ITEM_LENGTH, pairs);
  if (valid_padding) {
    bool success = accelerator_bls_g2_msm(input);
    return accelerator_result_bool_uint16_t_to_struct_PrecompileResult(success, BLS_G2_POINT_LENGTH);
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_pairing(struct CalldataSlice input)
{
  uint32_t length_ = calldata_slice_length(input);
  uint16_t item_length = BLS_PAIRING_ITEM_LENGTH;
  uint32_t pairs = (length_ / (uint32_t)item_length);
  if ((length_ == UINT8_C(0)) || (length_ != (pairs * (uint32_t)item_length))) {
    return precompile_failure();
  }
  bool valid_g1_padding = bls_g1_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(input, UINT8_C(0), BLS_PAIRING_ITEM_LENGTH, pairs);
  bool valid_g2_padding = bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool_variant_2(input, BLS_G2_POINT_OFFSET, BLS_PAIRING_ITEM_LENGTH, pairs);
  if (valid_g1_padding && valid_g2_padding) {
    uint8_t result = accelerator_bls_pairing(input);
    return pairing_result(result);
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_map_fp_to_g1(struct CalldataSlice input)
{
  uint32_t input_length = calldata_slice_length(input);
  if (input_length != BLS_PADDED_FIELD_LENGTH) {
    return precompile_failure();
  }
  bool valid_padding = slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(input, UINT8_C(0), BLS_PADDED_FIELD_LENGTH, BLS_FIELD_PADDING_LENGTH, UINT8_C(1));
  if (valid_padding) {
    bool success = accelerator_bls_map_fp_to_g1(input);
    return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success, BLS_G1_POINT_LENGTH);
  }
  return precompile_failure();
}

struct PrecompileResult run_bls_map_fp2_to_g2(struct CalldataSlice input)
{
  uint32_t input_length = calldata_slice_length(input);
  if (input_length != BLS_G1_POINT_LENGTH) {
    return precompile_failure();
  }
  bool valid_padding = slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(input, UINT8_C(0), BLS_PADDED_FIELD_LENGTH, BLS_FIELD_PADDING_LENGTH, TWO_COMPONENTS);
  if (valid_padding) {
    bool success = accelerator_bls_map_fp2_to_g2(input);
    return accelerator_result_bool_uint16_t_to_struct_PrecompileResult(success, BLS_G2_POINT_LENGTH);
  }
  return precompile_failure();
}

struct PrecompileResult run_p256_verify(struct CalldataSlice input)
{
  uint32_t input_length = calldata_slice_length(input);
  bool verified;
  if (input_length == P256_INPUT_LENGTH) {
    verified = accelerator_p256_verify(input);
  } else {
    verified = false;
  }
  if (verified) {
    Bytes output = output_buffer_word(WORD_ONE);
    return precompile_success(output);
  }
  return precompile_success(EMPTY_OUTPUT_SLICE);
}

struct PrecompileResult run_precompile_slice(enum PrecompileId num, struct CalldataSlice input)
{
  switch (num) {
  case NotPrecompile:
    return precompile_failure();
  case Ecrecover:
    return run_ecrecover(input);
  case Sha256:
    return run_sha256(input);
  case Ripemd160:
    return run_ripemd160(input);
  case Identity:
    return copied_result(input);
  case Modexp:
    return run_modexp(input);
  case Bn254Add:
  {
    bool success = accelerator_bn254_add(input);
    return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success, PRECOMPILE_DOUBLE_WORD_LENGTH);
  }
  case Bn254Mul:
  {
    bool success_3_2950 = accelerator_bn254_mul(input);
    return accelerator_result_bool_uint8_t_to_struct_PrecompileResult(success_3_2950, PRECOMPILE_DOUBLE_WORD_LENGTH);
  }
  case Bn254Pairing:
  {
    uint32_t input_length = calldata_slice_length(input);
    uint8_t item_length = BN254_PAIRING_ITEM_LENGTH;
    if (input_length == ((input_length / (uint32_t)item_length) * (uint32_t)item_length)) {
      uint8_t result = accelerator_bn254_pairing(input);
      return pairing_result(result);
    }
    return precompile_failure();
  }
  case Blake2f:
    return run_blake2f(input);
  case KzgPointEvaluation:
    return run_kzg_point_evaluation(input);
  case BlsG1Add:
    return run_bls_g1_add(input);
  case BlsG1Msm:
    return run_bls_g1_msm(input);
  case BlsG2Add:
    return run_bls_g2_add(input);
  case BlsG2Msm:
    return run_bls_g2_msm(input);
  case BlsPairing:
    return run_bls_pairing(input);
  case BlsMapFpToG1:
    return run_bls_map_fp_to_g1(input);
  case BlsMapFp2ToG2:
    return run_bls_map_fp2_to_g2(input);
  case P256Verify:
    return run_p256_verify(input);
  }
}

struct PrecompileResult accelerator_result_bool_uint16_t_to_struct_PrecompileResult(bool success, uint16_t output_len)
{
  if (success) {
    Bytes output = output_buffer_slice_uint16_t_to_Bytes(output_len);
    return precompile_success(output);
  }
  return precompile_failure();
}

struct PrecompileResult accelerator_result_bool_uint32_t_to_struct_PrecompileResult(bool success, uint32_t output_len)
{
  if (success) {
    Bytes output = output_buffer_slice_uint32_t_to_Bytes(output_len);
    return precompile_success(output);
  }
  return precompile_failure();
}

struct PrecompileResult accelerator_result_bool_uint8_t_to_struct_PrecompileResult(bool success, uint8_t output_len)
{
  if (success) {
    Bytes output = output_buffer_slice_uint8_t_to_Bytes(output_len);
    return precompile_success(output);
  }
  return precompile_failure();
}

bool bls_g1_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(struct CalldataSlice input, uint8_t base, uint16_t stride, uint32_t count)
{
  bool slice_strided_zero_result_2_1809 = slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(input, base, stride, BLS_FIELD_PADDING_LENGTH, count);
  if (slice_strided_zero_result_2_1809) {
    uint8_t integer_result_3_3673 = ((uint8_t)((uint32_t)BLS_PADDED_FIELD_LENGTH + (uint32_t)base));
    return slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3673, stride, BLS_FIELD_PADDING_LENGTH, count);
  }
  return false;
}

bool bls_g1_padding_struct_CalldataSlice_uint8_t_uint8_t_uint32_t_to_bool(struct CalldataSlice input, uint8_t base, uint8_t stride, uint32_t count)
{
  bool slice_strided_zero_result_2_1809 = slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(input, base, stride, BLS_FIELD_PADDING_LENGTH, count);
  if (slice_strided_zero_result_2_1809) {
    uint8_t integer_result_3_3666 = ((uint8_t)((uint32_t)BLS_PADDED_FIELD_LENGTH + (uint32_t)base));
    return slice_strided_zero_struct_CalldataSlice_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3666, stride, BLS_FIELD_PADDING_LENGTH, count);
  }
  return false;
}

bool bls_g1_padding_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_to_bool(struct CalldataSlice input, uint8_t base, uint8_t stride, uint8_t count)
{
  bool slice_strided_zero_result_2_1809 = slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(input, base, stride, BLS_FIELD_PADDING_LENGTH, count);
  if (slice_strided_zero_result_2_1809) {
    uint8_t integer_result_3_3665 = ((uint8_t)((uint32_t)BLS_PADDED_FIELD_LENGTH + (uint32_t)base));
    return slice_strided_zero_struct_CalldataSlice_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(input, (uint32_t)integer_result_3_3665, stride, BLS_FIELD_PADDING_LENGTH, count);
  }
  return false;
}

bool bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(struct CalldataSlice input, uint8_t base, uint16_t stride, uint32_t count)
{
  bool slice_strided_zero_result_2_1807 = slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(input, base, stride, BLS_FIELD_PADDING_LENGTH, count);
  if (slice_strided_zero_result_2_1807) {
    bool result_2_1806;
    uint8_t integer_result_3_3670 = ((uint8_t)((uint32_t)BLS_PADDED_FIELD_LENGTH + (uint32_t)base));
    result_2_1806 = slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3670, stride, BLS_FIELD_PADDING_LENGTH, count);
    if (result_2_1806) {
      bool result_2_1805;
      uint8_t integer_result_3_3671 = ((uint8_t)((uint32_t)BLS_G1_POINT_LENGTH + (uint32_t)base));
      result_2_1805 = slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3671, stride, BLS_FIELD_PADDING_LENGTH, count);
      if (result_2_1805) {
        uint8_t integer_result_3_3672 = ((uint8_t)((uint32_t)BLS_G2_FINAL_FIELD_OFFSET + (uint32_t)base));
        return slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3672, stride, BLS_FIELD_PADDING_LENGTH, count);
      }
      return false;
    }
    return false;
  }
  return false;
}

bool bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool_variant_2(struct CalldataSlice input, uint8_t base, uint16_t stride, uint32_t count)
{
  bool slice_strided_zero_result_2_1807 = slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(input, base, stride, BLS_FIELD_PADDING_LENGTH, count);
  if (slice_strided_zero_result_2_1807) {
    bool result_2_1806;
    uint8_t integer_result_3_3674 = ((uint8_t)((uint32_t)BLS_PADDED_FIELD_LENGTH + (uint32_t)base));
    result_2_1806 = slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3674, stride, BLS_FIELD_PADDING_LENGTH, count);
    if (result_2_1806) {
      bool result_2_1805;
      uint16_t integer_result_3_3675 = ((uint16_t)((uint32_t)(uint16_t)base + (uint32_t)(uint16_t)BLS_G1_POINT_LENGTH));
      result_2_1805 = slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3675, stride, BLS_FIELD_PADDING_LENGTH, count);
      if (result_2_1805) {
        uint16_t integer_result_3_3676 = ((uint16_t)((uint32_t)(uint16_t)base + (uint32_t)(uint16_t)BLS_G2_FINAL_FIELD_OFFSET));
        return slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(input, (uint32_t)integer_result_3_3676, stride, BLS_FIELD_PADDING_LENGTH, count);
      }
      return false;
    }
    return false;
  }
  return false;
}

bool bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_to_bool(struct CalldataSlice input, uint8_t base, uint16_t stride, uint8_t count)
{
  bool slice_strided_zero_result_2_1807 = slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(input, base, stride, BLS_FIELD_PADDING_LENGTH, count);
  if (slice_strided_zero_result_2_1807) {
    bool result_2_1806;
    uint8_t integer_result_3_3667 = ((uint8_t)((uint32_t)BLS_PADDED_FIELD_LENGTH + (uint32_t)base));
    result_2_1806 = slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(input, (uint32_t)integer_result_3_3667, stride, BLS_FIELD_PADDING_LENGTH, count);
    if (result_2_1806) {
      bool result_2_1805;
      uint8_t integer_result_3_3668 = ((uint8_t)((uint32_t)BLS_G1_POINT_LENGTH + (uint32_t)base));
      result_2_1805 = slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(input, (uint32_t)integer_result_3_3668, stride, BLS_FIELD_PADDING_LENGTH, count);
      if (result_2_1805) {
        uint8_t integer_result_3_3669 = ((uint8_t)((uint32_t)BLS_G2_FINAL_FIELD_OFFSET + (uint32_t)base));
        return slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(input, (uint32_t)integer_result_3_3669, stride, BLS_FIELD_PADDING_LENGTH, count);
      }
      return false;
    }
    return false;
  }
  return false;
}

