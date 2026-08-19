#pragma once

#include "evmsail/spec/kernel/scratch.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef SAIL_FIXED_VECTOR_VECTOR_32_BITS_8_DEFINED
#define SAIL_FIXED_VECTOR_VECTOR_32_BITS_8_DEFINED
typedef struct vector_32_bits_8 {
  size_t len;
  uint64_t data[32];
} vector_32_bits_8;
#endif

bytes32 host_keccak_stateless_input(Bytes /* arg_0 */);

bytes32 host_keccak_scratch(Bytes /* arg_0 */);

bytes32 host_keccak_memory(Bytes /* arg_0 */);

bytes32 host_sha256_stateless_input(Bytes /* arg_0 */);

bytes32 host_sha256_memory(Bytes /* arg_0 */);

bytes32 calldata_sha256(struct CalldataSlice input);

struct AddressResult ecrecover_addr(bytes32 h, uint8_t yparity, u256 r, u256 s);

extern const bytes32 KECCAK_EMPTY;


extern const bytes32 EMPTY_TRIE_ROOT;


extern const u256 SECP_N_HALF;



#ifdef __cplusplus
}
#endif
