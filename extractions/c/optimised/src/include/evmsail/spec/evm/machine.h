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

// struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32
struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32 {
  struct FrameCheckpoint tup0;
  StackPointer tup1;
  uint32_t tup2;
  uint32_t tup3;
};

__int128 validated_refund_add(__int128 left, __int128 right);

__int128 record_refund(__int128 refund, __int128 delta);

uint32_t frame_code_len(struct CodeFields frame_code);

bool frame_jumpdest_valid(struct CodeFields frame_code, uint32_t dest);

uint64_t conserved_gas_add(uint64_t available, uint64_t credit);

__int128 frame_state_gas_used(uint64_t state_gas_reservoir, uint64_t state_gas_remaining, uint32_t state_gas_spilled);

__attribute__((__always_inline__)) struct ExceptionalStateTransition exceptional_state(uint64_t state_gas_remaining, uint32_t state_gas_spilled, uint64_t state_gas_reservoir, enum ExceptionKind k);

__attribute__((__always_inline__)) uint16_t stack_height(StackPointer top);

__attribute__((__always_inline__)) u256 read_stack_word(StackPointer sp);

__attribute__((__always_inline__)) void write_stack_word(StackPointer sp, u256 value);

__attribute__((__always_inline__)) void stack_set(StackPointer top, uint16_t n, u256 w);

Bytes returndata_clear(void);

uint32_t returndata_size(Bytes returndata);

void returndata_copy(Bytes returndata, uint32_t dst, uint32_t off, uint32_t len);

void returndata_copy_prefix(Bytes returndata, uint32_t dst, uint32_t want);

uint32_t returndata_remaining(uint32_t available, uint32_t offset);

uint32_t memory_high_water(uint32_t height);

uint32_t memory_absolute(uint32_t base, uint32_t relative);

uint32_t memory_parent_base(uint32_t child_base, uint32_t parent_height);

__attribute__((__always_inline__)) uint32_t expand_memory(uint32_t base, uint32_t height, uint32_t requested_height);

Bytes active_memory_slice(uint32_t base, uint32_t mem, uint32_t off, uint32_t len);

Bytes memory_code_slice(uint32_t base, uint32_t mem, uint32_t off, uint32_t len);

void mem_set_byte(uint32_t base, uint32_t off, uint64_t v);

u256 mem_load(uint32_t base, uint32_t off);

void mem_store(uint32_t base, uint32_t off, u256 w);

void mem_store_byte(uint32_t base, uint32_t off, u256 w);

void mem_mcopy(uint32_t base, uint32_t dst, uint32_t src, uint32_t len);

u256 mem_keccak(uint32_t base, uint32_t mem, struct MemoryRangeFields range);

uint64_t conserved_gas_add_uint64_t_uint32_t_to_uint64_t(uint64_t available, uint32_t credit);

__int128 record_refund___int128_uint16_t_to___int128(__int128 refund, uint16_t delta);

__attribute__((__always_inline__)) void stack_set_StackPointer_uint8_t_u256_to_unit(StackPointer top, uint8_t n, u256 w);

uint32_t suspend_frame(uint32_t pc, uint64_t gas_remaining, StackPointer stack_top, uint32_t memory_base, uint32_t memory_height, uint8_t state_gas_remaining, uint32_t state_gas_spilled, __int128 frame_refund, struct FrameStatus frame_status, struct Message message, struct CodeFields frame_code, struct CalldataSlice calldata, struct FrameCheckpoint *restrict framecheckpoint_8_1553, StackPointer *restrict stackpointer_8_1554, uint32_t *restrict field_2_8_1555);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint16_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint16_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint16_t_uint8_t_to_enum_StackValidation(StackPointer top, uint16_t inputs, uint8_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation(StackPointer top, uint8_t inputs, uint8_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_2(StackPointer top, uint8_t inputs, uint8_t outputs);

__attribute__((__always_inline__)) enum StackValidation validate_stack_StackPointer_uint8_t_uint8_t_to_enum_StackValidation_variant_3(StackPointer top, uint8_t inputs, uint8_t outputs);

__int128 validated_refund_add___int128_uint16_t_to___int128(__int128 left, uint16_t right);

extern const uint16_t STACK_LIMIT;


extern const uint32_t MEMORY_HEIGHT_ZERO;


extern const uint32_t MEMORY_BASE_ZERO;



#ifdef __cplusplus
}
#endif
