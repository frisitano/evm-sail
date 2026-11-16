#pragma once

#include "evmsail/spec/primitives/evm.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct WitnessContext
struct WitnessContext {
  u256 parent_base_fee_per_gas;
  uint32_t parent_blob_gas_used;
  uint32_t parent_excess_blob_gas;
  uint64_t parent_gas_limit;
  uint64_t parent_gas_used;
  bool parent_has_base_fee;
  bytes32 parent_hash;
  bytes32 parent_state_root;
};

// struct StatelessInput
struct StatelessInput {
  uint64_t chain_id;
  struct ExecutionPayload payload;
};


#ifdef __cplusplus
}
#endif
