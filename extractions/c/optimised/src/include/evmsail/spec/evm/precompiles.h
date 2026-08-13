#pragma once

#include "evmsail/spec/evm/gas.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct PrecompileResult
struct PrecompileResult {
  Bytes output;
  bool success;
};

bool precompile_active_at_fork(enum PrecompileId n);

enum PrecompileId precompile_id_if_active(enum PrecompileId candidate);

struct PrecompileResult precompile_success(Bytes output);

struct PrecompileResult precompile_failure(void);

struct PrecompileResult copied_result(struct CalldataSlice data);

struct PrecompileResult boolean_result(bool value);

struct PrecompileResult run_ecrecover(struct CalldataSlice input);

struct PrecompileResult run_sha256(struct CalldataSlice input);

struct PrecompileResult run_ripemd160(struct CalldataSlice input);

struct PrecompileResult run_modexp(struct CalldataSlice input);

struct PrecompileResult pairing_result(uint8_t result);

struct PrecompileResult run_blake2f(struct CalldataSlice input);

bool kzg_versioned_hash_matches(struct CalldataSlice input);

struct PrecompileResult run_kzg_point_evaluation(struct CalldataSlice input);

struct PrecompileResult run_bls_g1_add(struct CalldataSlice input);

struct PrecompileResult run_bls_g1_msm(struct CalldataSlice input);

struct PrecompileResult run_bls_g2_add(struct CalldataSlice input);

struct PrecompileResult run_bls_g2_msm(struct CalldataSlice input);

struct PrecompileResult run_bls_pairing(struct CalldataSlice input);

struct PrecompileResult run_bls_map_fp_to_g1(struct CalldataSlice input);

struct PrecompileResult run_bls_map_fp2_to_g2(struct CalldataSlice input);

struct PrecompileResult run_p256_verify(struct CalldataSlice input);

struct PrecompileResult run_precompile_slice(enum PrecompileId num, struct CalldataSlice input);

struct PrecompileResult accelerator_result_bool_uint16_t_to_struct_PrecompileResult(bool success, uint16_t output_len);

struct PrecompileResult accelerator_result_bool_uint32_t_to_struct_PrecompileResult(bool success, uint32_t output_len);

struct PrecompileResult accelerator_result_bool_uint8_t_to_struct_PrecompileResult(bool success, uint8_t output_len);

bool bls_g1_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(struct CalldataSlice input, uint8_t base, uint16_t stride, uint32_t count);

bool bls_g1_padding_struct_CalldataSlice_uint8_t_uint8_t_uint32_t_to_bool(struct CalldataSlice input, uint8_t base, uint8_t stride, uint32_t count);

bool bls_g1_padding_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_to_bool(struct CalldataSlice input, uint8_t base, uint8_t stride, uint8_t count);

bool bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(struct CalldataSlice input, uint8_t base, uint16_t stride, uint32_t count);

bool bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint32_t_to_bool_variant_2(struct CalldataSlice input, uint8_t base, uint16_t stride, uint32_t count);

bool bls_g2_padding_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_to_bool(struct CalldataSlice input, uint8_t base, uint16_t stride, uint8_t count);

extern const bytes20 PRECOMPILE_ADDRESS_1;


extern const bytes20 PRECOMPILE_ADDRESS_10;


extern const bytes20 PRECOMPILE_ADDRESS_11;


extern const bytes20 PRECOMPILE_ADDRESS_12;


extern const bytes20 PRECOMPILE_ADDRESS_13;


extern const bytes20 PRECOMPILE_ADDRESS_14;


extern const bytes20 PRECOMPILE_ADDRESS_15;


extern const bytes20 PRECOMPILE_ADDRESS_16;


extern const bytes20 PRECOMPILE_ADDRESS_17;


extern const bytes20 PRECOMPILE_ADDRESS_2;


extern const bytes20 PRECOMPILE_ADDRESS_256;


extern const bytes20 PRECOMPILE_ADDRESS_3;


extern const bytes20 PRECOMPILE_ADDRESS_4;


extern const bytes20 PRECOMPILE_ADDRESS_5;


extern const bytes20 PRECOMPILE_ADDRESS_6;


extern const bytes20 PRECOMPILE_ADDRESS_7;


extern const bytes20 PRECOMPILE_ADDRESS_8;


extern const bytes20 PRECOMPILE_ADDRESS_9;


extern const uint32_t ACCELERATOR_INPUT_MAX;


extern const u256 FIELD_ELEMENTS_PER_BLOB;


extern const u256 BLS_MODULUS;


extern const uint8_t BLAKE2F_INPUT_LENGTH;


extern const uint8_t BLAKE2F_FINAL_BLOCK_OFFSET;


extern const uint8_t BLAKE2F_OUTPUT_LENGTH;


extern const uint8_t KZG_INPUT_LENGTH;


extern const uint8_t KZG_COMMITMENT_OFFSET;


extern const uint8_t KZG_COMMITMENT_LENGTH;


extern const uint8_t BLS_FIELD_PADDING_LENGTH;


extern const uint8_t BLS_PADDED_FIELD_LENGTH;


extern const uint8_t BLS_G1_POINT_LENGTH;


extern const uint16_t BLS_G2_POINT_LENGTH;


extern const uint8_t BLS_G2_FINAL_FIELD_OFFSET;


extern const uint16_t BLS_G1_ADD_INPUT_LENGTH;


extern const uint8_t BLS_G1_MSM_ITEM_LENGTH;


extern const uint16_t BLS_G2_ADD_INPUT_LENGTH;


extern const uint16_t BLS_G2_MSM_ITEM_LENGTH;


extern const uint16_t BLS_PAIRING_ITEM_LENGTH;


extern const uint8_t P256_INPUT_LENGTH;


extern const uint8_t BN254_PAIRING_ITEM_LENGTH;


extern const uint8_t PRECOMPILE_WORD_LENGTH;


extern const uint8_t PRECOMPILE_DOUBLE_WORD_LENGTH;


extern const uint8_t PRECOMPILE_WORD_OFFSET;


extern const uint8_t PRECOMPILE_DOUBLE_WORD_OFFSET;


extern const uint32_t ECRECOVER_S_OFFSET;


extern const uint8_t TWO_COMPONENTS;


extern const uint8_t BLS_G2_POINT_OFFSET;



#ifdef __cplusplus
}
#endif
