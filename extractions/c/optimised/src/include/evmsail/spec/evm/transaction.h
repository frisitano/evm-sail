#pragma once

#include "evmsail/spec/evm/interpreter.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct TxUpfrontResult
struct TxUpfrontResult {
  uint64_t authorization_refund;
  bool create_target_prestate_empty;
};

// struct TransactionPreparation
struct TransactionPreparation {
  bool delegated;
  bool ready;
};

// struct TransactionCosts
struct TransactionCosts {
  u256 blob_fee;
  uint32_t blob_gas;
  uint64_t calldata_floor;
  uint64_t intrinsic_execution;
  uint64_t intrinsic_state;
  u256 upfront;
};

// struct IntrinsicGasCost
struct IntrinsicGasCost {
  uint64_t calldata_floor;
  uint64_t execution;
  uint64_t state;
};

// struct tuple_bool_uint_8_uint_64_uint_32
struct tuple_bool_uint_8_uint_64_uint_32 {
  bool tup0;
  uint8_t tup1;
  uint64_t tup2;
  uint32_t tup3;
};

// struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32
struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32 {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
  __int128 tup3;
  StackPointer tup4;
  uint32_t tup5;
  uint32_t tup6;
};

// struct tuple_uint_64_uint_64_uint_8_int_128_StackPointer_uint_32_uint_32
struct tuple_uint_64_uint_64_uint_8_int_128_StackPointer_uint_32_uint_32 {
  uint64_t tup0;
  uint64_t tup1;
  uint8_t tup2;
  __int128 tup3;
  StackPointer tup4;
  uint32_t tup5;
  uint32_t tup6;
};

// struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice
struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice {
  struct TransactionPreparation tup0;
  uint64_t tup1;
  uint64_t tup2;
  uint32_t tup3;
  bytes20 tup4;
  bytes20 tup5;
  struct CodeFields tup6;
  struct CalldataSlice tup7;
};

// struct tuple_TransactionPreparation_uint_8_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice
struct tuple_TransactionPreparation_uint_8_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice {
  struct TransactionPreparation tup0;
  uint8_t tup1;
  uint64_t tup2;
  uint32_t tup3;
  bytes20 tup4;
  bytes20 tup5;
  struct CodeFields tup6;
  struct CalldataSlice tup7;
};

Bytes transaction_initcode_slice(Bytes input);

uint64_t calldata_cost(Bytes input);

uint64_t legacy_intrinsic_gas(struct TransactionFields tx);

uint64_t legacy_calldata_floor(Bytes input);

uint16_t amsterdam_recipient_execution_cost(struct TransactionFields tx);

struct IntrinsicGasCost intrinsic_gas(struct TransactionFields tx);

struct TransactionCosts transaction_costs(struct ProtocolProfileFields profile, struct TransactionFields tx, uint64_t gas_limit, uint32_t excess_blob_gas);

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot(struct TransactionInitialGasFields initial, uint64_t execution, uint64_t state, __int128 state_delta);

struct TransactionGasAllowanceFields transaction_gas_allowance_fields(uint64_t value, uint64_t regular_limit);

uint16_t process_auth(struct Authorization au);

uint64_t authorization_refund_add(uint16_t item, uint64_t accumulated);

uint64_t process_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count);

uint64_t process_auth_list(PreparedAuthorizationList authorizations);

struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth(struct Authorization au, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill);

struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t gas, uint64_t state_gas, uint32_t state_spill);

void warm_access_list_keys(Bytes cursor, bytes20 addr);

void warm_access_list(Bytes cursor);

void prewarm(struct TransactionFields tx);

struct tuple_u256_u256 eff_gas_price_for(u256 base_fee, u256 max_fee, u256 max_priority_fee);

struct TxValidityFields check_transaction_validity(struct TransactionFields tx, struct TransactionGasAllowanceFields allowance);

struct TxUpfrontResult apply_transaction_upfront_effects(struct TransactionFields tx, struct TxValidityFields v, PreparedAuthorizationList authorizations);

struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32 enter_transaction_frame(struct TxValidityFields v);

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_create_transaction_frame(struct TransactionFields tx, bytes20 sender, uint64_t nonce_before, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, uint32_t carried_memory_base, uint32_t carried_memory_height, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir);

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_call_transaction_frame(struct TransactionFields tx, bytes20 sender, bool delegated, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_code_address, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir);

struct TxFrameResultFields run_legacy_transaction_frame(struct TransactionFields tx, struct TxValidityFields v);

struct TxFrameResultFields run_amsterdam_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations);

struct TxFrameResultFields run_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations);

uint64_t remaining_gas_after_refund(__int128 total, uint64_t remaining, uint64_t cap);

struct ReceiptFields settle_transaction(struct TransactionFields tx, struct TxValidityFields v, uint64_t authorization_refund, struct TxFrameResultFields fr);

struct ReceiptFields process_transaction(struct TransactionFields tx, struct TransactionGasAllowanceFields allowance);

struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice prepare_amsterdam_transaction_dispatch(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill);

struct TransactionGasAllowanceFields transaction_gas_allowance(uint64_t value, uint64_t total_limit, uint64_t regular_limit);

struct TransactionInitialGasFields transaction_initial_gas(struct TransactionGasAllowanceFields allowance, uint64_t intrinsic_execution, uint64_t intrinsic_state, uint64_t calldata_floor);

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_TxFrameGasSnapshotFields(struct TransactionInitialGasFields initial, uint64_t execution, uint64_t state, __int128 state_delta);

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint8_t_uint8_t___int128_to_struct_TxFrameGasSnapshotFields(struct TransactionInitialGasFields initial, uint8_t execution, uint8_t state, __int128 state_delta);

u256 validated_word_product_u256_uint32_t_to_u256(u256 value, uint32_t factor);

u256 validated_word_product_u256_uint32_t_to_u256_variant_2(u256 value, uint32_t factor);

u256 validated_word_product_u256_uint64_t_to_u256(u256 value, uint64_t factor);

extern const uint16_t G_transaction;


extern const uint16_t G_txcreate;


extern const uint8_t G_txdatazero;


extern const uint8_t G_txdatanonzero;


extern const uint16_t G_access_list_address;


extern const uint16_t G_access_list_storage_key;


extern const uint16_t PER_AUTH_BASE;


extern const uint16_t PER_EMPTY_ACCOUNT;


extern const uint16_t AMSTERDAM_TX_BASE;


extern const uint16_t AMSTERDAM_CREATE_ACCESS;


extern const uint16_t AMSTERDAM_COLD_ACCOUNT_ACCESS;


extern const uint16_t AMSTERDAM_TX_VALUE_COST;


extern const uint16_t AMSTERDAM_TRANSFER_LOG_COST;


extern const uint16_t AMSTERDAM_ACCESS_LIST_ADDRESS;


extern const uint16_t AMSTERDAM_ACCESS_LIST_SLOT;


extern const uint16_t AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR;


extern const uint16_t AMSTERDAM_ACCESS_LIST_SLOT_FLOOR;


extern const uint16_t AMSTERDAM_AUTH_BASE;


extern const uint8_t AMSTERDAM_CALLDATA_FLOOR_BYTE;



#ifdef __cplusplus
}
#endif
