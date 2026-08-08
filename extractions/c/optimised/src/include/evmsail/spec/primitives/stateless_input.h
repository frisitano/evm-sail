#pragma once

#include "evmsail/spec/primitives/evm.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct WitnessContext
struct WitnessContext {
  u256 parent_base_fee_per_gas;
  uint32_t parent_blob_gas_used;
  uint32_t parent_excess_blob_gas;
  bytes32 parent_hash;
  bytes32 parent_state_root;
};

// struct StatelessInput
struct StatelessInput {
  struct ChainConfig chain_config;
  struct ExecutionPayload payload;
};


#ifdef __cplusplus
}
#endif
