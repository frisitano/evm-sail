#pragma once

#include "evmsail/spec/kernel/lifecycle.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum StackValidation
enum StackValidation { StackValid, StackUnderflowFailure, StackOverflowFailure };

// struct tuple_uint_64_uint_64_uint_32
struct tuple_uint_64_uint_64_uint_32 {
  uint64_t tup0;
  uint64_t tup1;
  uint32_t tup2;
};

// struct tuple_uint_64_uint_64_uint_8
struct tuple_uint_64_uint_64_uint_8 {
  uint64_t tup0;
  uint64_t tup1;
  uint8_t tup2;
};

// struct tuple_uint_64_uint_32_FrameStatus
struct tuple_uint_64_uint_32_FrameStatus {
  uint64_t tup0;
  uint32_t tup1;
  struct FrameStatus tup2;
};

// struct tuple_Bytes_Bytes
struct tuple_Bytes_Bytes {
  Bytes tup0;
  Bytes tup1;
};

// struct tuple_Bytes_Bytes_1
struct tuple_Bytes_Bytes_1 {
  Bytes tup0;
  Bytes tup1;
};

// struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice
struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice {
  uint32_t tup0;
  uint64_t tup1;
  struct CalldataSlice tup10;
  StackPointer tup2;
  Bytes tup3;
  uint64_t tup4;
  uint32_t tup5;
  __int128 tup6;
  struct FrameStatus tup7;
  struct Message tup8;
  struct CodeFields tup9;
};

// struct tuple_u256_Bytes
struct tuple_u256_Bytes {
  u256 tup0;
  Bytes tup1;
};

// struct tuple_FrameCheckpoint_StackPointer_Bytes
struct tuple_FrameCheckpoint_StackPointer_Bytes {
  struct FrameCheckpoint tup0;
  StackPointer tup1;
  Bytes tup2;
};

__int128 validated_refund_add(__int128 left, __int128 right);

__int128 record_refund(__int128 refund, __int128 delta);

uint32_t frame_code_len(struct CodeFields frame_code);

bool frame_jumpdest_valid(struct CodeFields frame_code, uint32_t dest);

uint64_t conserved_gas_add(uint64_t available, uint64_t credit);

struct tuple_uint_64_uint_64_uint_32 refill_frame_state_gas(uint64_t g, uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t state_gas_reservoir);

__int128 frame_state_gas_used(uint64_t state_gas_reservoir, uint64_t state_gas_remaining, uint32_t state_gas_spilled);

__attribute__((__always_inline__)) struct FrameStatus exceptional_state(uint64_t *restrict state_gas_remaining, uint32_t *restrict state_gas_spilled, uint64_t state_gas_reservoir, enum ExceptionKind k);

__attribute__((__always_inline__)) uint16_t stack_height(StackPointer top);

__attribute__((__always_inline__)) u256 read_stack_word(StackPointer sp);

__attribute__((__always_inline__)) void write_stack_word(StackPointer sp, u256 value);

__attribute__((__always_inline__)) void stack_set(StackPointer top, uint16_t n, u256 w);

Bytes returndata_clear(void);

uint32_t returndata_size(Bytes returndata);

void returndata_copy(Bytes returndata, uint32_t dst, uint32_t off, uint32_t len);

void returndata_copy_prefix(Bytes returndata, uint32_t dst, uint32_t want);

uint32_t returndata_remaining(uint32_t available, uint32_t offset);

uint32_t memory_high_water(Bytes mem);

Bytes memory_reset(void);

struct tuple_Bytes_Bytes memory_expand_to(Bytes mem, uint32_t new_size);

struct tuple_Bytes_Bytes active_memory_slice(Bytes mem, uint32_t off, uint32_t len);

struct tuple_Bytes_Bytes_1 memory_code_slice(Bytes mem, uint32_t off, uint32_t len);

Bytes memory_frame_enter(void);

Bytes memory_frame_leave(Bytes parent);

struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice restore_frame(struct FrameCheckpoint checkpoint);

void mem_set_byte(uint32_t off, uint64_t v);

u256 mem_load(uint32_t off);

void mem_store(uint32_t off, u256 w);

void mem_store_byte(uint32_t off, u256 w);

void mem_mcopy(uint32_t dst, uint32_t src, uint32_t len);

struct tuple_u256_Bytes mem_keccak(Bytes mem, struct MemoryRangeFields range);

uint64_t conserved_gas_add_uint64_t_uint32_t_to_uint64_t(uint64_t available, uint32_t credit);

__int128 record_refund___int128_uint16_t_to___int128(__int128 refund, uint16_t delta);

__attribute__((__always_inline__)) void stack_set_StackPointer_uint8_t_u256_to_unit(StackPointer top, uint8_t n, u256 w);

struct tuple_FrameCheckpoint_StackPointer_Bytes suspend_frame(uint32_t pc, uint64_t gas_remaining, StackPointer stack_top, Bytes evm_memory, uint8_t state_gas_remaining, uint32_t state_gas_spilled, __int128 frame_refund, struct FrameStatus frame_status, struct Message message, struct CodeFields frame_code, struct CalldataSlice calldata);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint16_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint16_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint8_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint8_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation(StackPointer top, uint8_t inputs, uint8_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_2(StackPointer top, uint8_t inputs, uint8_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_3(StackPointer top, uint8_t inputs, uint8_t outputs);

__int128 validated_refund_add___int128_uint16_t_to___int128(__int128 left, uint16_t right);

extern const uint16_t STACK_LIMIT;



#ifdef __cplusplus
}
#endif
