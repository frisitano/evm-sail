#pragma once

#include "evmsail/spec/primitives/fork.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct GasLimitsFields
struct GasLimitsFields {
  uint64_t block_limit;
  uint32_t system_regular_limit;
  uint8_t system_state_limit;
  uint64_t transaction_regular_limit;
  uint64_t transaction_total_limit;
};

// struct ChainConfig
struct ChainConfig {uint64_t chain_id;};

// struct BlobScheduleFields
struct BlobScheduleFields {
  uint32_t base_fee_update_fraction;
  uint8_t max;
  uint8_t target;
};

// struct ProtocolProfileFields
struct ProtocolProfileFields {
  struct BlobScheduleFields blob_schedule;
  uint32_t deployed_code_size_limit;
  uint32_t excess_blob_gas_limit;
  uint8_t fork;
  uint32_t initcode_size_limit;
  uint8_t refund_divisor;
  uint8_t transaction_blob_limit;
  uint64_t transaction_regular_gas_limit;
  uint64_t transaction_total_gas_limit;
};

// struct ExecutionProfileFields
struct ExecutionProfileFields {
  struct GasLimitsFields gas;
  struct ProtocolProfileFields protocol;
};

struct ProtocolProfileFields pack_protocol_profile(struct ProtocolProfileFields profile);

struct GasLimitsFields gas_limits_for(struct ProtocolProfileFields profile, uint64_t block_limit);

struct ExecutionProfileFields execution_profile_for(struct ProtocolProfileFields protocol, uint64_t block_limit);

struct ProtocolProfileFields schema_protocol_profile(uint64_t schema_fork);

void create_letbind_56(void);
void kill_letbind_56(void);


void create_letbind_57(void);
void kill_letbind_57(void);


uint32_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(uint8_t fork, uint8_t target, uint8_t maximum, uint32_t denominator);

uint8_t compute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(uint8_t fork, uint8_t target, uint8_t maximum, uint8_t denominator);

struct ExecutionProfileFields execution_profile_for_struct_ProtocolProfileFields_uint8_t_to_struct_ExecutionProfileFields(struct ProtocolProfileFields protocol, uint8_t block_limit);

struct GasLimitsFields gas_limits_for_struct_ProtocolProfileFields_uint8_t_to_struct_GasLimitsFields(struct ProtocolProfileFields profile, uint8_t block_limit);

extern struct ProtocolProfileFields DEFAULT_PROTOCOL_PROFILE;


extern struct ExecutionProfileFields DEFAULT_EXECUTION_PROFILE;



#ifdef __cplusplus
}
#endif
