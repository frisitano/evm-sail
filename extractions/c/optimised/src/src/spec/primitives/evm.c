#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct TxEnvFields tx_env(bytes20 origin, u256 gas_price, struct BlobHashesFields blob_hashes)
{
  return ((struct TxEnvFields){.blob_hashes = blob_hashes, .gas_price = gas_price, .origin = origin});
}

struct TransactionInitialGasFields transaction_initial_gas_fields(uint64_t total, uint64_t regular, uint64_t intrinsic_execution, uint64_t intrinsic_state, uint64_t calldata_floor, uint64_t execution, uint64_t state)
{
  return ((struct TransactionInitialGasFields){.admitted_limit = total, .calldata_floor = calldata_floor, .execution_remaining = execution, .intrinsic_execution = intrinsic_execution, .intrinsic_state = intrinsic_state, .regular_limit = regular, .state_remaining = state});
}

struct TxValidityFields tx_validity_fields(bytes20 sender, uint64_t nonce_before, struct TransactionInitialGasFields gas, u256 blob_fee, u256 gas_price, u256 priority_fee)
{
  return ((struct TxValidityFields){.blob_fee = blob_fee, .gas = gas, .gas_price = gas_price, .nonce_before = nonce_before, .priority_fee = priority_fee, .sender = sender});
}

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_fields(uint64_t limit, uint64_t regular, uint64_t calldata_floor, uint64_t remaining, uint64_t state_used)
{
  return ((struct TxFrameGasSnapshotFields){.admitted_limit = limit, .calldata_floor = calldata_floor, .regular_limit = regular, .remaining = remaining, .state_used = state_used});
}

struct TransactionInitialGasFields transaction_initial_gas_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TransactionInitialGasFields(uint64_t total, uint64_t regular, uint64_t intrinsic_execution, uint64_t intrinsic_state, uint64_t calldata_floor, uint64_t execution, uint8_t state)
{
  return ((struct TransactionInitialGasFields){.admitted_limit = total, .calldata_floor = calldata_floor, .execution_remaining = execution, .intrinsic_execution = intrinsic_execution, .intrinsic_state = intrinsic_state, .regular_limit = regular, .state_remaining = (uint64_t)state});
}

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TxFrameGasSnapshotFields(uint64_t limit, uint64_t regular, uint64_t calldata_floor, uint64_t remaining, uint8_t state_used)
{
  return ((struct TxFrameGasSnapshotFields){.admitted_limit = limit, .calldata_floor = calldata_floor, .regular_limit = regular, .remaining = remaining, .state_used = (uint64_t)state_used});
}

