#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

// register zscratch_arena
Bytes scratch_arena;

uint32_t scratch_begin(void)
{
  return scratch_arena.len;
}

uint32_t scratch_reserve(uint32_t len)
{
  Bytes arena = scratch_arena;
  bool reserved = scratch_reserve_at(arena.len, len);
  if (!reserved) {
    __builtin_trap();
  }
  return arena.len;
}

void scratch_push_byte(uint64_t data)
{
  scratch_arena = scratch_store_byte((scratch_arena).len, data);
}

void stateless_input_scratch_push_slice(Bytes data)
{
  if (data.len != UINT8_C(0)) {
    scratch_arena = scratch_store_stateless_input((scratch_arena).len, data);
    return;
  }
}

void scratch_scratch_push_slice(Bytes data)
{
  if (data.len != UINT8_C(0)) {
    scratch_arena = scratch_store_scratch((scratch_arena).len, data);
    return;
  }
}

void log_data_scratch_push_slice(Bytes data)
{
  if (data.len != UINT8_C(0)) {
    scratch_arena = scratch_store_log_data((scratch_arena).len, data);
    return;
  }
}

void output_scratch_push_slice(Bytes data)
{
  if (data.len != UINT8_C(0)) {
    scratch_arena = scratch_store_output((scratch_arena).len, data);
    return;
  }
}

void scratch_push_address(bytes20 data)
{
  scratch_arena = scratch_store_address((scratch_arena).len, data);
}

void scratch_push_b256(bytes32 data, uint8_t len)
{
  if (len != UINT8_C(0)) {
    scratch_arena = scratch_store_b256((scratch_arena).len, data, len);
    return;
  }
}

void scratch_push_word_be(u256 data, uint8_t len)
{
  if (len != UINT8_C(0)) {
    scratch_arena = scratch_store_word((scratch_arena).len, data, len);
    return;
  }
}

Bytes scratch_finish(uint32_t start)
{
  Bytes arena = scratch_arena;
  uint32_t stop_offset = arena.len;
  if (start > stop_offset) {
    __builtin_trap();
  }
  return scratch_sub_slice(arena, start, (stop_offset - start));
}

void scratch_rewind(uint32_t mark)
{
  Bytes arena = scratch_arena;
  if (mark <= arena.len) {
    scratch_arena = scratch_sub_slice_Bytes_uint8_t_uint32_t_to_Bytes(arena, UINT8_C(0), mark);
    scratch_truncate(mark);
    return;
  }
  __builtin_trap();
}

void scratch_reset(void)
{
  scratch_arena = EMPTY_SCRATCH_SLICE;
  scratch_truncate(UINT32_C(0));
}

uint32_t scratch_length_add(uint8_t left, uint32_t right)
{
  if (right > (UINT32_C(4294967295) - (uint32_t)left)) {
    __builtin_trap();
  }
  return (right + (uint32_t)left);
}

void scratch_push_b256_bytes32_uint8_t_to_unit(bytes32 data, uint8_t len)
{
  scratch_arena = scratch_store_b256((scratch_arena).len, data, len);
}

void scratch_push_word_be_u256_uint8_t_to_unit(u256 data, uint8_t len)
{
  scratch_arena = scratch_store_word((scratch_arena).len, data, len);
}

uint32_t scratch_reserve_uint8_t_to_uint32_t(uint8_t len)
{
  Bytes arena = scratch_arena;
  bool reserved = scratch_reserve_at(arena.len, (uint32_t)len);
  if (!reserved) {
    __builtin_trap();
  }
  return arena.len;
}

