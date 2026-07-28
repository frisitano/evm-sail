#ifndef PRECOMPILES_H
#define PRECOMPILES_H

#include "sail_abi.h"

#include <stdbool.h>
#include <stdint.h>

/* Granular adapters to the zkVM accelerator ABI. Precompile selection,
 * validation, and EVM-visible result construction live in Sail. Operations
 * with byte-array results fill the shared precompile output arena. */
bool accelerator_ripemd160_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_modexp_bytes(const uint8_t *bytes, uint64_t len,
                              uint64_t base_len, uint64_t exponent_len,
                              uint64_t modulus_len);
bool accelerator_bn254_add_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_bn254_mul_bytes(const uint8_t *bytes, uint64_t len);
uint64_t accelerator_bn254_pairing_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_blake2f_bytes(const uint8_t *bytes, uint64_t len,
                               uint64_t rounds, uint64_t final_block);
bool accelerator_kzg_point_evaluation_bytes(const uint8_t *bytes,
                                            uint64_t len);
bool accelerator_bls_g1_add_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_bls_g1_msm_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_bls_g2_add_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_bls_g2_msm_bytes(const uint8_t *bytes, uint64_t len);
uint64_t accelerator_bls_pairing_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_bls_map_fp_to_g1_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_bls_map_fp2_to_g2_bytes(const uint8_t *bytes, uint64_t len);
bool accelerator_p256_verify_bytes(const uint8_t *bytes, uint64_t len);

/* Fixed-width signature primitive shared by transaction execution. */
EVMSAIL_ADDRESS_RESULT_RECORD_RETURN precompile_ecrecover_hash_sig(
    EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(result) sail_hash h,
    uint64_t yparity, EVMSAIL_WORD_PARAM(r), EVMSAIL_WORD_PARAM(s));
bool precompile_ecrecover_hash_sig_address(uint8_t address[20], sail_hash h,
                                           uint64_t yparity, sail_word r,
                                           sail_word s);

#endif
