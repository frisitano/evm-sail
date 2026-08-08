#pragma once

#include "evmsail/spec/executor/block_access_list.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct BlockGasUsageFields
struct BlockGasUsageFields {
  uint64_t execution;
  u128 receipts;
  uint64_t state;
};

// struct BlockExecutionResult
struct BlockExecutionResult {
  uint32_t blob_gas_used;
  uint64_t execution_gas_used;
  bytes20 first_tx_recipient;
  uint64_t header_gas_used;
  struct LogSeriesRef logs;
  bytes32 receipts_root;
  uint64_t state_gas_used;
};

struct BlockGasUsageFields block_gas_usage_empty(void);

void run_block_start_system_calls(void);

struct BlockExecutionResult execute_block_transactions(struct BoundedSszListRef transactions, Bytes public_keys, Bytes expected_deposits);

void apply_withdrawals(struct BoundedSszListRef withdrawals);

void apply_block_end_state(struct BlockBody body);

struct BlockExecutionResult execute_block_body(struct BlockBody body, struct StatelessInputRef input_ref);

struct BlockGasUsageFields block_gas_usage_add_struct_BlockGasUsageFields_uint64_t_uint64_t_uint64_t_to_struct_BlockGasUsageFields(struct BlockGasUsageFields usage, uint64_t add_execution, uint64_t add_state, uint64_t add_receipt);

struct BlockGasUsageFields block_gas_usage_add_struct_BlockGasUsageFields_uint64_t_uint8_t_uint64_t_to_struct_BlockGasUsageFields(struct BlockGasUsageFields usage, uint64_t add_execution, uint8_t add_state, uint64_t add_receipt);

extern const uint64_t PRE_MERGE_BLOCK_REWARD;



#ifdef __cplusplus
}
#endif
