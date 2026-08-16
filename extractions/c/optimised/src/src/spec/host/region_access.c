#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint64_t stateless_input_slice_byte(Bytes s, uint32_t off)
{
  if (off < s.len) {
    return stateless_input_byte_at(s, off);
  }
  return UINT64_C(0x00);
}

uint64_t memory_slice_byte(Bytes s, uint32_t off)
{
  if (off < s.len) {
    return memory_slice_byte_at(s, off);
  }
  return UINT64_C(0x00);
}

uint64_t code_slice_byte(Bytes s, uint32_t off)
{
  if (off < s.len) {
    return code_region_byte_at(s, off);
  }
  return UINT64_C(0x00);
}

uint64_t calldata_slice_byte(struct CalldataSlice s, uint32_t off)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_byte(s.variants.InputCalldata, off);
  case Kind_MemoryCalldata:
    return memory_slice_byte(s.variants.MemoryCalldata, off);
  }
}

uint32_t slice_count_nonzero(Bytes s)
{
  return stateless_input_count_nonzero(s);
}

u256 stateless_input_slice_load(Bytes s, uint32_t off)
{
  if (off < s.len) {
    return stateless_input_load_word(s, off);
  }
  return ZERO_WORD;
}

u256 memory_slice_load(Bytes s, uint32_t off)
{
  if (off < s.len) {
    return memory_slice_load_word(s, off);
  }
  return ZERO_WORD;
}

u256 calldata_slice_load(struct CalldataSlice s, uint32_t off)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_load(s.variants.InputCalldata, off);
  case Kind_MemoryCalldata:
    return memory_slice_load(s.variants.MemoryCalldata, off);
  }
}

u256 stateless_input_slice_load_word_offset(Bytes s, u256 off)
{
  if (u256_lt_u64(off, s.len)) {
    uint32_t tmp_3_3751 = (uint32_t)u256_to_u64(off);
    return stateless_input_slice_load(s, tmp_3_3751);
  }
  return ZERO_WORD;
}

u256 memory_slice_load_word_offset(Bytes s, u256 off)
{
  if (u256_lt_u64(off, s.len)) {
    uint32_t tmp_3_3752 = (uint32_t)u256_to_u64(off);
    return memory_slice_load(s, tmp_3_3752);
  }
  return ZERO_WORD;
}

u256 calldata_slice_load_word_offset(struct CalldataSlice s, u256 off)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_load_word_offset(s.variants.InputCalldata, off);
  case Kind_MemoryCalldata:
    return memory_slice_load_word_offset(s.variants.MemoryCalldata, off);
  }
}

u256 stateless_input_slice_load_n(Bytes s, uint32_t off, uint8_t n)
{
  if (off < s.len) {
    return stateless_input_load_n_word(s, off, n);
  }
  return ZERO_WORD;
}

u256 code_slice_load_n(Bytes s, uint32_t off, uint8_t n)
{
  if (off < s.len) {
    return code_region_load_n_word(s, off, n);
  }
  return ZERO_WORD;
}

u256 scratch_slice_load_n(Bytes s, uint32_t off, uint8_t n)
{
  if (off < s.len) {
    return scratch_slice_load_n_word(s, off, n);
  }
  return ZERO_WORD;
}

void stateless_input_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len)
{
  stateless_input_copy_to_memory(s, dst, off, len);
}

void memory_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len)
{
  memory_slice_copy_to_memory(s, dst, off, len);
}

void code_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len)
{
  code_region_copy_to_memory(s, dst, off, len);
}

void output_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len)
{
  output_slice_copy_to_memory(s, dst, off, len);
}

void stateless_input_slice_copy_word_offset(Bytes s, uint32_t dst, u256 off, uint32_t len)
{
  if (u256_lt_u64(off, s.len)) {
    uint32_t tmp_3_3753 = (uint32_t)u256_to_u64(off);
    stateless_input_slice_copy(s, dst, tmp_3_3753, len);
    return;
  }
  stateless_input_copy_to_memory(EMPTY_STATELESS_INPUT_SLICE, dst, UINT32_C(0), len);
}

void memory_slice_copy_word_offset(Bytes s, uint32_t dst, u256 off, uint32_t len)
{
  if (u256_lt_u64(off, s.len)) {
    uint32_t tmp_3_3754 = (uint32_t)u256_to_u64(off);
    memory_slice_copy(s, dst, tmp_3_3754, len);
    return;
  }
  stateless_input_copy_to_memory(EMPTY_STATELESS_INPUT_SLICE, dst, UINT32_C(0), len);
}

void code_slice_copy_word_offset(Bytes s, uint32_t dst, u256 off, uint32_t len)
{
  if (u256_lt_u64(off, s.len)) {
    uint32_t tmp_3_3755 = (uint32_t)u256_to_u64(off);
    code_slice_copy(s, dst, tmp_3_3755, len);
    return;
  }
  stateless_input_copy_to_memory(EMPTY_STATELESS_INPUT_SLICE, dst, UINT32_C(0), len);
}

void calldata_slice_copy_word_offset(struct CalldataSlice s, uint32_t dst, u256 off, uint32_t len)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    stateless_input_slice_copy_word_offset(s.variants.InputCalldata, dst, off, len);
    return;
  case Kind_MemoryCalldata:
    memory_slice_copy_word_offset(s.variants.MemoryCalldata, dst, off, len);
    return;
  }
}

uint64_t calldata_slice_byte_struct_CalldataSlice_uint8_t_to_uint64_t(struct CalldataSlice s, uint8_t off)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(s.variants.InputCalldata, off);
  case Kind_MemoryCalldata:
    return memory_slice_byte_Bytes_uint8_t_to_uint64_t(s.variants.MemoryCalldata, off);
  }
}

u256 calldata_slice_load_struct_CalldataSlice_uint8_t_to_u256(struct CalldataSlice s, uint8_t off)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_load_Bytes_uint8_t_to_u256(s.variants.InputCalldata, off);
  case Kind_MemoryCalldata:
    return memory_slice_load_Bytes_uint8_t_to_u256(s.variants.MemoryCalldata, off);
  }
}

uint64_t log_data_byte(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return log_data_slice_byte_at(s, (uint32_t)off);
  }
  return UINT64_C(0x00);
}

u256 log_data_slice_load_Bytes_uint16_t_to_u256(Bytes s, uint16_t off)
{
  if (off < s.len) {
    return log_data_slice_load_word(s, (uint32_t)off);
  }
  return ZERO_WORD;
}

u256 log_data_slice_load_Bytes_uint8_t_to_u256(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return log_data_slice_load_word(s, (uint32_t)off);
  }
  return ZERO_WORD;
}

uint64_t memory_slice_byte_Bytes_uint8_t_to_uint64_t(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return memory_slice_byte_at(s, (uint32_t)off);
  }
  return UINT64_C(0x00);
}

u256 memory_slice_load_Bytes_uint8_t_to_u256(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return memory_slice_load_word(s, (uint32_t)off);
  }
  return ZERO_WORD;
}

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint32_t count)
{
  return memory_slice_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, count);
}

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint8_t count)
{
  return memory_slice_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint32_t stride, uint8_t width, uint32_t count)
{
  return memory_slice_strided_zero(s, start, stride, (uint32_t)width, count);
}

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint32_t count)
{
  return memory_slice_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, count);
}

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint8_t count)
{
  return memory_slice_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint32_t count)
{
  return memory_slice_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, count);
}

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint8_t count)
{
  return memory_slice_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint32_t count)
{
  return memory_slice_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, count);
}

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint8_t count)
{
  return memory_slice_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

uint64_t output_byte(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return output_slice_byte_at(s, (uint32_t)off);
  }
  return UINT64_C(0x00);
}

void output_slice_copy_Bytes_uint32_t_uint8_t_uint32_t_to_unit(Bytes s, uint32_t dst, uint8_t off, uint32_t len)
{
  output_slice_copy_to_memory(s, dst, (uint32_t)off, len);
}

uint64_t scratch_byte(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return scratch_slice_byte_at(s, (uint32_t)off);
  }
  return UINT64_C(0x00);
}

u256 scratch_slice_load(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return scratch_slice_load_word(s, (uint32_t)off);
  }
  return ZERO_WORD;
}

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint32_t start, uint16_t stride, uint8_t width, uint32_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint32_t start, uint16_t stride, uint8_t width, uint8_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint32_t start, uint32_t stride, uint8_t width, uint32_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint32_t start, uint8_t stride, uint8_t width, uint32_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint32_t start, uint8_t stride, uint8_t width, uint8_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint8_t start, uint16_t stride, uint8_t width, uint32_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint8_t start, uint16_t stride, uint8_t width, uint8_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint8_t start, uint8_t stride, uint8_t width, uint32_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint8_t start, uint8_t stride, uint8_t width, uint8_t count)
{
  switch (s.kind) {
  case Kind_InputCalldata:
    return stateless_input_slice_strided_zero_Bytes_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(s.variants.InputCalldata, start, stride, width, count);
  case Kind_MemoryCalldata:
    return memory_slice_strided_zero_value_Bytes_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(s.variants.MemoryCalldata, start, stride, width, count);
  }
}

uint64_t stateless_input_slice_byte_Bytes_uint16_t_to_uint64_t(Bytes s, uint16_t off)
{
  if (off < s.len) {
    return stateless_input_byte_at(s, (uint32_t)off);
  }
  return UINT64_C(0x00);
}

uint64_t stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return stateless_input_byte_at(s, (uint32_t)off);
  }
  return UINT64_C(0x00);
}

u256 stateless_input_slice_load_Bytes_uint16_t_to_u256(Bytes s, uint16_t off)
{
  if (off < s.len) {
    return stateless_input_load_word(s, (uint32_t)off);
  }
  return ZERO_WORD;
}

u256 stateless_input_slice_load_Bytes_uint8_t_to_u256(Bytes s, uint8_t off)
{
  if (off < s.len) {
    return stateless_input_load_word(s, (uint32_t)off);
  }
  return ZERO_WORD;
}

u256 stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(Bytes s, uint8_t off, uint8_t n)
{
  if (off < s.len) {
    return stateless_input_load_n_word(s, (uint32_t)off, n);
  }
  return ZERO_WORD;
}

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint32_t count)
{
  return stateless_input_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, count);
}

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint8_t count)
{
  return stateless_input_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint32_t stride, uint8_t width, uint32_t count)
{
  return stateless_input_strided_zero(s, start, stride, (uint32_t)width, count);
}

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint32_t count)
{
  return stateless_input_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, count);
}

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint8_t count)
{
  return stateless_input_strided_zero(s, start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint32_t count)
{
  return stateless_input_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, count);
}

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint8_t count)
{
  return stateless_input_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint32_t count)
{
  return stateless_input_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, count);
}

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint8_t count)
{
  return stateless_input_strided_zero(s, (uint32_t)start, (uint32_t)stride, (uint32_t)width, (uint32_t)count);
}

