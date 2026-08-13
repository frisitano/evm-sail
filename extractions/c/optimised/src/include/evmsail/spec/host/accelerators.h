#pragma once

#include "evmsail/spec/primitives/code.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bool accelerator_ripemd160(struct CalldataSlice /* arg_0 */);

bool accelerator_modexp(struct CalldataSlice /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */);

bool accelerator_bn254_add(struct CalldataSlice /* arg_0 */);

bool accelerator_bn254_mul(struct CalldataSlice /* arg_0 */);

uint8_t accelerator_bn254_pairing(struct CalldataSlice /* arg_0 */);

bool accelerator_blake2f(struct CalldataSlice /* arg_0 */, uint32_t /* arg_1 */, uint8_t /* arg_2 */);

bool accelerator_kzg_point_evaluation(struct CalldataSlice /* arg_0 */);

bool accelerator_bls_g1_add(struct CalldataSlice /* arg_0 */);

bool accelerator_bls_g1_msm(struct CalldataSlice /* arg_0 */);

bool accelerator_bls_g2_add(struct CalldataSlice /* arg_0 */);

bool accelerator_bls_g2_msm(struct CalldataSlice /* arg_0 */);

uint8_t accelerator_bls_pairing(struct CalldataSlice /* arg_0 */);

bool accelerator_bls_map_fp_to_g1(struct CalldataSlice /* arg_0 */);

bool accelerator_bls_map_fp2_to_g2(struct CalldataSlice /* arg_0 */);

bool accelerator_p256_verify(struct CalldataSlice /* arg_0 */);

struct AddressResult precompile_ecrecover_hash_sig(bytes32 /* arg_0 */, uint8_t /* arg_1 */, u256 /* arg_2 */, u256 /* arg_3 */);


#ifdef __cplusplus
}
#endif
