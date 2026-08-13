#pragma once

#include "evmsail/spec/host/scratch.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint32_t scratch_begin(void);

uint32_t scratch_reserve(uint32_t len);

void scratch_push_byte(uint64_t data);

void stateless_input_scratch_push_slice(Bytes data);

void scratch_scratch_push_slice(Bytes data);

void log_data_scratch_push_slice(Bytes data);

void output_scratch_push_slice(Bytes data);

void scratch_push_address(bytes20 data);

void scratch_push_b256(bytes32 data, uint8_t len);

void scratch_push_word_be(u256 data, uint8_t len);

Bytes scratch_finish(uint32_t start);

void scratch_rewind(uint32_t mark);

void scratch_reset(void);

uint32_t scratch_length_add(uint8_t left, uint32_t right);

void scratch_push_b256_bytes32_uint8_t_to_unit(bytes32 data, uint8_t len);

void scratch_push_word_be_u256_uint8_t_to_unit(u256 data, uint8_t len);

uint32_t scratch_reserve_uint8_t_to_uint32_t(uint8_t len);

// register zscratch_arena
extern Bytes scratch_arena;


#ifdef __cplusplus
}
#endif
