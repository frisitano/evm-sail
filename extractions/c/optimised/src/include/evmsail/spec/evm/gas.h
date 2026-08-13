#pragma once

#include "evmsail/spec/evm/machine.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct SstoreCosts
struct SstoreCosts {
  uint64_t execution;
  __int128 refund;
  uint64_t state_charge;
  uint32_t state_credit;
};

#ifndef SAIL_FIXED_VECTOR_VECTOR_128_UINT_16_DEFINED
#define SAIL_FIXED_VECTOR_VECTOR_128_UINT_16_DEFINED
typedef struct vector_128_uint_16 {
  size_t len;
  uint16_t data[128];
} vector_128_uint_16;
#endif

// struct tuple_bool_uint_64
struct tuple_bool_uint_64 {
  bool tup0;
  uint64_t tup1;
};

// struct tuple_bool_uint_8
struct tuple_bool_uint_8 {
  bool tup0;
  uint8_t tup1;
};

// struct tuple_bool_uint_64_uint_64_uint_32
struct tuple_bool_uint_64_uint_64_uint_32 {
  bool tup0;
  uint64_t tup1;
  uint64_t tup2;
  uint32_t tup3;
};

// struct tuple_bool_uint_64_uint_8_uint_32
struct tuple_bool_uint_64_uint_8_uint_32 {
  bool tup0;
  uint64_t tup1;
  uint8_t tup2;
  uint32_t tup3;
};

u256 protocol_word(u256 value);

u256 fake_exponential_word(struct BlobScheduleFields schedule, uint32_t numerator);

u256 blob_base_fee(uint8_t fork, struct BlobScheduleFields schedule, uint32_t limit, uint32_t excess_blob_gas);

uint32_t block_blob_gas_add(uint8_t maximum_count, uint32_t accumulated, uint32_t transaction);

uint32_t next_excess_blob_gas(struct ProtocolProfileFields profile, uint32_t parent_excess_blob_gas, uint32_t parent_blob_gas_used, u256 parent_base_fee_per_gas);

uint16_t sstore_clear_refund(void);

uint32_t state_gas_spill_room(uint32_t left);

struct tuple_uint_64_uint_64_uint_32 credit_state_gas_refund(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint32_t amount);

struct tuple_uint_64_uint_32 return_child_state_gas(uint64_t parent_remaining, uint32_t parent_spilled, uint64_t child_remaining, uint32_t child_spilled);

uint64_t refund_gas(uint64_t g, uint64_t amount);

uint64_t gas_sub(uint64_t left, uint64_t right);

__attribute__((__always_inline__)) u256 memory_word_count_word(u256 byte_len);

__attribute__((__always_inline__)) uint64_t memory_required_size(u256 start, u256 size);

__attribute__((__always_inline__)) struct MemoryAccessFields memory_access(u256 start, u256 size);

__attribute__((__always_inline__)) Bytes expand_memory(Bytes mem, uint32_t required_size);

uint16_t account_cost(bool warm);

uint16_t external_code_read_cost(void);

uint16_t sload_cost(bool warm);

uint16_t call_value_cost(void);

uint16_t create_access_cost(void);

struct GasCharge code_deployment_execution_cost(uint32_t byte_len, uint64_t available);

uint64_t code_deployment_state_cost(uint32_t byte_len);

u256 pc_word(struct CalldataSlice input, uint32_t start, uint8_t byte_count);

u256 pc_word_after_declared_field(struct CalldataSlice input, uint32_t prefix, u256 declared_length, uint8_t byte_count);

uint32_t pc_blake2_rounds(struct CalldataSlice input);

struct GasCharge modexp_gas(struct CalldataSlice input, uint64_t available);

struct GasCharge precompile_gas(enum PrecompileId num, struct CalldataSlice input, uint64_t available);

uint16_t amsterdam_storage_access_cost(bool cold);

uint64_t sstore_sentry_cost(bool cold);

struct SstoreCosts legacy_sstore_costs(u256 original, u256 current, u256 new_value, bool cold);

struct SstoreCosts amsterdam_sstore_costs(u256 original, u256 current, u256 new_value, bool cold);

struct SstoreCosts sstore_costs(u256 original, u256 current, u256 new_value, bool cold);

struct tuple_bool_uint_64 charge_word_scaled_gas(uint64_t g, uint16_t per_unit, u256 units);

struct tuple_bool_uint_64 charge_memory_word_gas(uint64_t g, uint16_t base, uint16_t per_word, u256 size);

struct tuple_bool_uint_64 charge_keccak_gas(uint64_t g, u256 size);

struct tuple_bool_uint_64 charge_copy_gas(uint64_t g, u256 size);

uint64_t exp_gas(u256 exponent);

uint32_t transaction_initcode_gas(uint32_t byte_len);

uint64_t call_gas_cap_word(uint64_t available, u256 requested);

uint32_t block_blob_gas_add_uint8_t_uint32_t_uint32_t_to_uint32_t(uint8_t maximum_count, uint32_t accumulated, uint32_t transaction);

struct GasCharge bls_msm_gas(vector_128_uint_16 table, uint16_t base, uint16_t maxd, uint32_t k, uint64_t available);

__attribute__((__always_inline__)) struct tuple_bool_uint_64 charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(uint64_t g, uint16_t amount);

__attribute__((__always_inline__)) struct tuple_bool_uint_64 charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(uint64_t g, uint32_t amount);

__attribute__((__always_inline__)) struct tuple_bool_uint_64 charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(uint64_t g, uint64_t amount);

__attribute__((__always_inline__)) struct tuple_bool_uint_64 charge_uint64_t_uint8_t_to_struct_tuple_bool_uint_64(uint64_t g, uint8_t amount);

struct tuple_bool_uint_64 charge_copy_gas_uint64_t_u256_to_struct_tuple_bool_uint_64(uint64_t g, u256 size);

struct tuple_bool_uint_64_uint_64_uint_32 charge_deployment_state_gas(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t amount);

struct tuple_bool_uint_64 charge_log_gas(uint64_t g, uint8_t num_topics, u256 size);

__attribute__((__always_inline__)) struct tuple_bool_uint_64 charge_memory_expansion(uint64_t g, Bytes mem, uint64_t required_size);

struct tuple_bool_uint_64 charge_memory_word_gas_uint64_t_uint16_t_uint16_t_u256_to_struct_tuple_bool_uint_64(uint64_t g, uint16_t base, uint16_t per_word, u256 size);

struct tuple_bool_uint_64 charge_memory_word_gas_uint64_t_uint16_t_uint8_t_u256_to_struct_tuple_bool_uint_64(uint64_t g, uint16_t base, uint8_t per_word, u256 size);

struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint32_t amount);

struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint32_t amount);

struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t amount);

struct tuple_bool_uint_64 charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(uint64_t g, uint8_t per_unit, u256 units);

struct tuple_bool_uint_64 charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64_variant_2(uint64_t g, uint8_t per_unit, u256 units);

struct tuple_bool_uint_64 check_execution_gas_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(uint64_t g, uint32_t amount);

struct tuple_bool_uint_64 check_execution_gas_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(uint64_t g, uint64_t amount);

struct tuple_bool_uint_64_uint_64_uint_32 debit_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint32_t amount);

struct tuple_bool_uint_64_uint_64_uint_32 debit_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint32_t amount);

struct tuple_bool_uint_64_uint_64_uint_32 debit_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t amount);

bool deployed_code_size_allowed(uint32_t size);

struct GasCharge fixed_precompile_gas_uint16_t_uint64_t_to_struct_GasCharge(uint16_t cost, uint64_t available);

struct GasCharge fixed_precompile_gas_uint32_t_uint64_t_to_struct_GasCharge(uint32_t cost, uint64_t available);

struct GasCharge fixed_precompile_gas_uint8_t_uint64_t_to_struct_GasCharge(uint8_t cost, uint64_t available);

bool initcode_size_allowed(uint32_t size);

struct GasCharge linear_gas_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_GasCharge(uint16_t base, uint16_t per_unit, uint32_t units, uint64_t available);

struct GasCharge linear_gas_uint16_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(uint16_t base, uint8_t per_unit, uint32_t units, uint64_t available);

struct GasCharge linear_gas_uint8_t_uint8_t_uint32_t_uint64_t_to_struct_GasCharge(uint8_t base, uint8_t per_unit, uint32_t units, uint64_t available);

__attribute__((__always_inline__)) uint64_t mem_cost(uint32_t words);

__attribute__((__always_inline__)) struct MemoryAccessFields memory_access_u256_u256_to_struct_MemoryAccessFields(u256 start, u256 size);

__attribute__((__always_inline__)) struct MemoryAccessFields memory_access_u256_u256_to_struct_MemoryAccessFields_variant_2(u256 start, u256 size);

__attribute__((__always_inline__)) uint64_t memory_required_size_u256_u256_to_uint64_t(u256 start, u256 size);

__attribute__((__always_inline__)) uint64_t memory_required_size_u256_u256_to_uint64_t_variant_2(u256 start, u256 size);

uint16_t memory_word_count_uint32_t_to_uint16_t(uint32_t byte_len);

uint32_t memory_word_count_uint32_t_to_uint32_t(uint32_t byte_len);

__attribute__((__always_inline__)) u256 memory_word_count_word_u256_to_u256(u256 byte_len);

u256 pc_word_struct_CalldataSlice_uint8_t_uint8_t_to_u256(struct CalldataSlice input, uint8_t start, uint8_t byte_count);

u256 pc_word_after_declared_field_struct_CalldataSlice_uint8_t_u256_uint8_t_to_u256(struct CalldataSlice input, uint8_t prefix, u256 declared_length, uint8_t byte_count);

uint32_t state_gas_spill_add_uint32_t___int128_to_uint32_t(uint32_t left, __int128 right);

uint32_t state_gas_spill_add_uint32_t_int64_t_to_uint32_t(uint32_t left, int64_t right);

uint32_t state_gas_spill_add_uint32_t_int64_t_to_uint32_t_variant_2(uint32_t left, int64_t right);

uint32_t state_gas_spill_add_uint32_t_uint32_t_to_uint32_t(uint32_t left, uint32_t right);

uint32_t state_gas_spill_room_uint32_t_to_uint32_t(uint32_t left);

extern const uint16_t G_zero;


extern const uint16_t G_jumpdest;


extern const uint16_t G_base;


extern const uint16_t G_verylow;


extern const uint16_t G_low;


extern const uint16_t G_mid;


extern const uint16_t G_high;


extern const uint16_t G_warm_access;


extern const uint16_t G_cold_sload;


extern const uint16_t G_cold_account;


extern const uint16_t G_keccak;


extern const uint8_t G_keccak_word;


extern const uint16_t G_copy_word;


extern const uint8_t G_memory;


extern const uint16_t G_log;


extern const uint16_t G_logtopic;


extern const uint16_t G_logdata;


extern const uint16_t G_exp;


extern const uint16_t G_expbyte;


extern const uint16_t G_sset;


extern const uint16_t G_sreset;


extern const uint16_t R_sclear_pre_london;


extern const uint16_t R_sclear_london;


extern const uint16_t R_selfdestruct_pre_london;


extern const uint16_t G_create;


extern const uint8_t G_codedeposit;


extern const uint16_t G_callvalue;


extern const uint64_t G_callstipend;


extern const uint16_t G_newaccount;


extern const uint16_t G_selfdestruct;


extern const uint8_t G_initcode_word;


extern const uint16_t G_amsterdam_cold_account_access;


extern const uint16_t G_amsterdam_cold_storage_access;


extern const uint16_t G_amsterdam_storage_write;


extern const uint16_t G_amsterdam_account_write;


extern const uint16_t G_amsterdam_call_value;


extern const uint16_t G_amsterdam_create_access;


extern const uint16_t G_amsterdam_state_byte;


extern const uint32_t G_amsterdam_state_storage_set;


extern const uint32_t G_amsterdam_state_new_account;


extern const uint32_t G_amsterdam_state_auth_base;


extern const uint16_t R_amsterdam_storage_clear;


extern const uint64_t G_sstore_sentry;


extern const vector_128_uint_16 BLS_G1_DISCOUNT;


extern const vector_128_uint_16 BLS_G2_DISCOUNT;



#ifdef __cplusplus
}
#endif
