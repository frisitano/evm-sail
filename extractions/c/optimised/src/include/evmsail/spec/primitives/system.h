#pragma once

#include "evmsail/spec/primitives/chain_config.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef SAIL_FIXED_VECTOR_VECTOR_20_BITS_8_DEFINED
#define SAIL_FIXED_VECTOR_VECTOR_20_BITS_8_DEFINED
typedef struct vector_20_bits_8 {
  size_t len;
  uint64_t data[20];
} vector_20_bits_8;
#endif

extern const bytes20 SYSTEM_ADDRESS;


extern const bytes20 BEACON_ROOTS_ADDR;


extern const bytes20 HISTORY_STORAGE_ADDR;


extern const bytes20 WITHDRAWAL_REQUEST_ADDR;


extern const bytes20 CONSOLIDATION_REQUEST_ADDR;


extern const bytes20 BUILDER_DEPOSIT_REQUEST_ADDR;


extern const bytes20 BUILDER_EXIT_REQUEST_ADDR;


extern const bytes20 DEPOSIT_CONTRACT_ADDR;


extern const u256 DEPOSIT_EVENT_TOPIC;


extern const bytes20 EIP7708_SYSTEM_ADDRESS;


extern const u256 EIP7708_TRANSFER_TOPIC;



#ifdef __cplusplus
}
#endif
