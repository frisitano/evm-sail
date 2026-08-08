#pragma once

#include "evmsail/spec/kernel/lifecycle.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_u256_bits_64
struct tuple_u256_bits_64 {
  u256 tup0;
  uint64_t tup1;
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

// struct tuple_u256_Bytes
struct tuple_u256_Bytes {
  u256 tup0;
  Bytes tup1;
};

// struct tuple_bool_uint_64
struct tuple_bool_uint_64 {
  bool tup0;
  uint64_t tup1;
};

__int128 validated_refund_add(__int128 left, __int128 right);

void record_refund(__int128 delta);

uint32_t frame_code_len(void);

bool frame_jumpdest_valid(uint32_t dest);

uint64_t conserved_gas_add(uint64_t left, uint64_t right);

uint64_t refill_frame_state_gas(uint64_t g);

__int128 frame_state_gas_used(void);

uint64_t exc_halt(uint64_t g, enum ExceptionKind k);

uint16_t stack_height(uint64_t top);

u256 peek(uint64_t top, uint16_t n);

uint64_t push_word(uint64_t top, u256 w);

uint64_t push_gas(uint64_t top, uint64_t value);

struct tuple_u256_bits_64 pop(uint64_t top);

void stack_set(uint64_t top, uint16_t n, u256 w);

void calldata_install(struct CalldataSlice data);

void returndata_clear(void);

uint32_t returndata_size(void);

void returndata_copy(uint32_t dst, uint32_t off, uint32_t len);

void returndata_copy_prefix(uint32_t dst, uint32_t want);

uint32_t returndata_remaining(uint32_t available, uint32_t offset);

uint64_t validated_returndata_copy(uint64_t g, uint32_t dst, u256 source_offset, u256 length_);

uint64_t returndata_copy_words(uint64_t g, uint32_t dst, u256 source_offset, u256 length_);

uint32_t memory_high_water(Bytes mem);

void memory_reset(void);

struct tuple_Bytes_Bytes memory_expand_to(Bytes mem, uint32_t new_size);

struct tuple_Bytes_Bytes active_memory_slice(Bytes mem, uint32_t off, uint32_t len);

struct tuple_Bytes_Bytes_1 memory_code_slice(Bytes mem, uint32_t off, uint32_t len);

Bytes memory_frame_enter(void);

void memory_frame_leave(Bytes parent);

struct FrameCheckpoint suspend_frame(void);

void restore_frame(struct FrameCheckpoint checkpoint);

void mem_set_byte(uint32_t off, uint64_t v);

u256 mem_load(uint32_t off);

void mem_store(uint32_t off, u256 w);

void mem_store_byte(uint32_t off, u256 w);

void mem_mcopy(uint32_t dst, uint32_t src, uint32_t len);

struct tuple_u256_Bytes mem_keccak(Bytes mem, struct MemoryRangeFields range);

uint64_t conserved_gas_add_uint64_t_uint32_t_to_uint64_t(uint64_t left, uint32_t right);

uint64_t exc_halt_uint64_t_enum_ExceptionKind_to_uint64_t(uint64_t g, enum ExceptionKind k);

u256 peek_uint64_t_uint8_t_to_u256(uint64_t top, uint8_t n);

void record_refund_uint16_t_to_unit(uint16_t delta);

uint64_t refill_frame_state_gas_uint64_t_to_uint64_t(uint64_t g);

void stack_set_uint64_t_uint8_t_u256_to_unit(uint64_t top, uint8_t n, u256 w);

struct tuple_bool_uint_64 validate_stack(uint64_t g, uint64_t top, uint16_t inputs, uint16_t outputs);

__int128 validated_refund_add___int128_uint16_t_to___int128(__int128 left, uint16_t right);

// register zpc
extern uint32_t pc;

// register zgas_remaining
extern uint64_t gas_remaining;

// register zstack_top
extern uint64_t stack_top;

// register zstate_gas_remaining
extern uint64_t state_gas_remaining;

// register zstate_gas_spilled
extern uint32_t state_gas_spilled;

// register zframe_refund
extern __int128 frame_refund;

// register zframe_status
extern struct FrameStatus frame_status;

// register zmessage
extern struct Message message;

// register zcall_depth
extern uint16_t call_depth;

// register zframe_code
extern struct CodeFields frame_code;

extern const uint16_t STACK_LIMIT;


// register zcalldata
extern struct CalldataSlice calldata;

// register zreturndata
extern Bytes returndata;

// register zevm_memory
extern Bytes evm_memory;


#ifdef __cplusplus
}
#endif
