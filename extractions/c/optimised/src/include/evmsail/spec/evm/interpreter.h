#pragma once

#include "evmsail/spec/evm/execute.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct CreateSemantics
struct CreateSemantics {bool uses_salt;};

// struct CallSemantics
struct CallSemantics {
  bool enters_static_context;
  bool inherits_caller_and_value;
  bool takes_value;
  bool transfers_value;
  bool uses_target_address;
};

// struct tuple_uint_32_u256
struct tuple_uint_32_u256 {
  uint32_t tup0;
  u256 tup1;
};

// struct tuple_uint_32_bits_8
struct tuple_uint_32_bits_8 {
  uint32_t tup0;
  uint64_t tup1;
};

// struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes
struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
  __int128 tup3;
  struct FrameStatus tup4;
  Bytes tup5;
};

u256 read_push(Bytes code, uint32_t offset, uint8_t n);

__attribute__((__always_inline__)) bool opcode_available(uint8_t opcode, uint8_t fork);

__attribute__((__always_inline__)) u256 decode_push_immediate(struct CodeFields frame_code, uint32_t *restrict immediate_offset, uint8_t width);

__attribute__((__always_inline__)) uint64_t decode_deep_immediate(struct CodeFields frame_code, uint32_t *restrict immediate_offset, enum DeepStackOperation operation);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup_encoded(uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap_encoded(uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log_encoded(bytes20 carried_address, bool carried_is_static, uint32_t memory_base, uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp, uint32_t *restrict memory);

__attribute__((__always_inline__)) struct OpcodeOutcome execute_deep_stack_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp);

Bytes frame_output(struct FrameStatus frame_status);

bool frame_succeeded(struct FrameStatus frame_status);

struct CodeFields executable_code(bytes20 target, bool dele, bytes20 dtgt);

struct CallSemantics call_semantics(enum CallKind kind);

uint16_t call_stack_inputs(enum CallKind kind);

struct FrameTransition run_call(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind);

struct CreateSemantics create_semantics(enum CreateKind kind);

uint16_t create_stack_inputs(enum CreateKind kind);

struct FrameTransition run_create(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind);

struct FrameTransition resume_call(struct CallContinuation continuation, Bytes output, uint32_t child_memory_base, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status);

struct FrameTransition resume_create(struct CreateContinuation continuation, Bytes output, uint32_t child_memory_base, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status, uint64_t child_state_gas_reservoir);

struct FrameTransition resume_frame(struct FrameContinuation continuation, Bytes output, uint32_t child_memory_base, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status, uint64_t child_state_gas_reservoir);

struct FrameTransition run_frame_entry_encoded(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, uint8_t opcode);

Bytes interpret_uint32_t_uint8_t_uint8_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint8_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint32_t initial_gas, uint8_t initial_state_gas, uint8_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, uint32_t initial_memory_base, uint32_t initial_memory_height, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint8_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata, uint64_t *restrict field_0_8_1523, uint64_t *restrict field_1_8_1524, uint32_t *restrict field_2_8_1525, __int128 *restrict field_3_8_1526, struct FrameStatus *restrict framestatus_8_1527);

Bytes interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint64_t initial_gas, uint64_t initial_state_gas, uint32_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, uint32_t initial_memory_base, uint32_t initial_memory_height, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint64_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata, uint64_t *restrict field_0_8_1529, uint64_t *restrict field_1_8_1530, uint32_t *restrict field_2_8_1531, __int128 *restrict field_3_8_1532, struct FrameStatus *restrict framestatus_8_1533);

struct FrameTransition run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind);

struct FrameTransition run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_FrameTransition(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind);


#ifdef __cplusplus
}
#endif
