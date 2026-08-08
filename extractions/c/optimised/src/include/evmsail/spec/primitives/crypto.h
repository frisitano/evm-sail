#pragma once

#include "evmsail/spec/kernel/scratch.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_bool_bytes20
struct tuple_bool_bytes20 {
  bool tup0;
  bytes20 tup1;
};

bytes32 host_keccak_stateless_input(Bytes /* arg_0 */);

bytes32 host_keccak_scratch(Bytes /* arg_0 */);

bytes32 host_keccak_memory(Bytes /* arg_0 */);

bytes32 host_sha256_stateless_input(Bytes /* arg_0 */);

bytes32 host_sha256_memory(Bytes /* arg_0 */);

bytes32 calldata_sha256(struct CalldataSlice input);

struct tuple_bool_bytes20 ecrecover_addr(bytes32 h, uint8_t yparity, u256 r, u256 s);

extern const bytes32 KECCAK_EMPTY;


extern const bytes32 EMPTY_TRIE_ROOT;


extern const u256 SECP_N_HALF;



#ifdef __cplusplus
}
#endif
