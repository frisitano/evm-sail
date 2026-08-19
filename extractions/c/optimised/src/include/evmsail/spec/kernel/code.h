#pragma once

#include "evmsail/spec/kernel/accounts.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_bool_bytes20
struct tuple_bool_bytes20 {
  bool tup0;
  bytes20 tup1;
};

bytes32 k_code_key(bytes20 a);

bytes32 k_get_codehash(bytes20 a);

void k_deploy_code(bytes20 a, Bytes code);

void k_set_delegation(bytes20 a, bytes20 target);

void k_clear_code(bytes20 a);

bytes20 k_deleg_target(bytes20 a, bool *restrict condition_8_1363);

uint32_t k_get_code_size(bytes20 a);

void k_code_copy(bytes20 a, uint32_t dst, u256 off, uint32_t len);


#ifdef __cplusplus
}
#endif
