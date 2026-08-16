#pragma once

#include "evmsail/spec/evm/instructions.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct AccountExecutionContext
struct AccountExecutionContext {
  uint32_t account_id;
  uint32_t storage_begin;
  uint32_t storage_change_count;
  uint32_t storage_count;
  uint32_t storage_generation;
};

// struct tuple_uint_64_OpcodeOutcome
struct tuple_uint_64_OpcodeOutcome {
  uint64_t tup0;
  struct OpcodeOutcome tup1;
};

// struct tuple_uint_32_uint_64_OpcodeOutcome
struct tuple_uint_32_uint_64_OpcodeOutcome {
  uint32_t tup0;
  uint64_t tup1;
  struct OpcodeOutcome tup2;
};

// struct tuple_LogTopics_StackPointer
struct tuple_LogTopics_StackPointer {
  struct LogTopics tup0;
  StackPointer tup1;
};

// struct tuple_uint_64_StackPointer_OpcodeOutcome
struct tuple_uint_64_StackPointer_OpcodeOutcome {
  uint64_t tup0;
  StackPointer tup1;
  struct OpcodeOutcome tup2;
};

// struct tuple_uint_64_StackPointer_uint_32_OpcodeOutcome
struct tuple_uint_64_StackPointer_uint_32_OpcodeOutcome {
  uint64_t tup0;
  StackPointer tup1;
  uint32_t tup2;
  struct OpcodeOutcome tup3;
};

// struct tuple_uint_8_StackPointer_uint_32_OpcodeOutcome
struct tuple_uint_8_StackPointer_uint_32_OpcodeOutcome {
  uint8_t tup0;
  StackPointer tup1;
  uint32_t tup2;
  struct OpcodeOutcome tup3;
};

// struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_OpcodeOutcome
struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_OpcodeOutcome {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
  __int128 tup3;
  StackPointer tup4;
  struct OpcodeOutcome tup5;
};

// struct tuple_uint_32_uint_64_StackPointer_OpcodeOutcome
struct tuple_uint_32_uint_64_StackPointer_OpcodeOutcome {
  uint32_t tup0;
  uint64_t tup1;
  StackPointer tup2;
  struct OpcodeOutcome tup3;
};

// struct tuple_uint_64_StackPointer_uint_32_FrameStatus
struct tuple_uint_64_StackPointer_uint_32_FrameStatus {
  uint64_t tup0;
  StackPointer tup1;
  uint32_t tup2;
  struct FrameStatus tup3;
};

// struct tuple_uint_64_uint_64_uint_32_StackPointer_uint_32_FrameStatus
struct tuple_uint_64_uint_64_uint_32_StackPointer_uint_32_FrameStatus {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
  StackPointer tup3;
  uint32_t tup4;
  struct FrameStatus tup5;
};

// struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_FrameStatus
struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_FrameStatus {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
  __int128 tup3;
  StackPointer tup4;
  struct FrameStatus tup5;
};

__attribute__((__always_inline__)) struct FrameStatus opcode_frame_status(struct OpcodeOutcome result);

__attribute__((__always_inline__)) struct OpcodeOutcome guard_static(uint64_t *restrict g, bool is_static);

__attribute__((__always_inline__)) struct OpcodeOutcome do_jump(uint32_t *restrict pc_in, uint64_t *restrict g, struct CodeFields frame_code, u256 destination_value);

struct OpcodeOutcome guard_stack(StackPointer carried_sp, uint16_t inputs, uint16_t outputs);

struct tuple_LogTopics_StackPointer pop_log_topics(uint8_t count, StackPointer sp_in);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_add(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mul(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sub(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_div(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sdiv(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_smod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_addmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mulmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_signextend(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_lt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sgt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_eq(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_iszero(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_and(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_or(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_xor(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_not(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_byte(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shl(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shr(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sar(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_clz(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_keccak256(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_address(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_origin(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_caller(bytes20 carried_caller, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_callvalue(u256 carried_value, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gasprice(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatasize(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldataload(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatacopy(struct CalldataSlice carried_calldata, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codesize(struct CodeFields carried_code, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codecopy(struct CodeFields carried_code, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_balance(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_selfbalance(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodesize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodecopy(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodehash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatasize(Bytes carried_returndata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatacopy(Bytes carried_returndata, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blockhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_coinbase(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_timestamp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_number(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slotnum(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_prevrandao(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gaslimit(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_chainid(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_basefee(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobbasefee(u256 blob_fee, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pop(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mload(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore8(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_msize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mcopy(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

struct AccountExecutionContext account_execution_context(bytes20 /* arg_0 */);

__attribute__((__always_inline__)) struct AccountExecutionContext account_execution_context_(bytes20 address);

__attribute__((__always_inline__)) struct AccountExecutionContext refresh_account_execution_context(struct AccountExecutionContext context, bytes20 previous_address, bytes20 next_address);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sload(struct AccountExecutionContext context, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sstore(struct AccountExecutionContext context, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tload(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tstore(bytes20 carried_address, bool carried_is_static, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jump(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpi(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pc(uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gas(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpdest(uint64_t *restrict carried_gas);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n, u256 v);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dupn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swapn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exchange(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log(bytes20 carried_address, bool carried_is_static, uint32_t memory_base, uint8_t n, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

struct FrameStatus execute_stop(void);

__attribute__((__always_inline__)) struct FrameStatus execute_return(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct FrameStatus execute_revert(uint64_t carried_state_gas_reservoir, uint32_t memory_base, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_invalid(uint64_t *restrict carried_gas);

__attribute__((__always_inline__)) struct FrameStatus execute_selfdestruct(bytes20 carried_address, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp);

struct OpcodeOutcome guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(StackPointer carried_sp, uint16_t inputs, uint8_t outputs);

struct OpcodeOutcome guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome(StackPointer carried_sp, uint8_t inputs, uint8_t outputs);

struct OpcodeOutcome guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2(StackPointer carried_sp, uint8_t inputs, uint8_t outputs);

struct OpcodeOutcome guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3(StackPointer carried_sp, uint8_t inputs, uint8_t outputs);


#ifdef __cplusplus
}
#endif
