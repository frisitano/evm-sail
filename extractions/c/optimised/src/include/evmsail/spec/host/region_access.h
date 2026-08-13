#pragma once

#include "evmsail/spec/primitives/rlp.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

Bytes stateless_input(void);

uint64_t stateless_input_byte_at(Bytes /* arg_0 */, uint32_t /* arg_1 */);

uint64_t memory_slice_byte_at(Bytes /* arg_0 */, uint32_t /* arg_1 */);

uint64_t code_region_byte_at(Bytes /* arg_0 */, uint32_t /* arg_1 */);

uint64_t scratch_slice_byte_at(Bytes /* arg_0 */, uint32_t /* arg_1 */);

uint64_t log_data_slice_byte_at(Bytes /* arg_0 */, uint32_t /* arg_1 */);

uint64_t output_slice_byte_at(Bytes /* arg_0 */, uint32_t /* arg_1 */);

uint32_t stateless_input_count_nonzero(Bytes /* arg_0 */);

bool stateless_input_strided_zero(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */, uint32_t /* arg_4 */);

bool memory_slice_strided_zero(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */, uint32_t /* arg_4 */);

u256 stateless_input_load_word(Bytes /* arg_0 */, uint32_t /* arg_1 */);

u256 memory_slice_load_word(Bytes /* arg_0 */, uint32_t /* arg_1 */);

u256 scratch_slice_load_word(Bytes /* arg_0 */, uint32_t /* arg_1 */);

u256 log_data_slice_load_word(Bytes /* arg_0 */, uint32_t /* arg_1 */);

u256 stateless_input_load_n_word(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint8_t /* arg_2 */);

u256 code_region_load_n_word(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint8_t /* arg_2 */);

u256 scratch_slice_load_n_word(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint8_t /* arg_2 */);

void stateless_input_copy_to_memory(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */);

void memory_slice_copy_to_memory(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */);

void code_region_copy_to_memory(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */);

void output_slice_copy_to_memory(Bytes /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, uint32_t /* arg_3 */);

bool scratch_input_slices_equal(Bytes /* arg_0 */, Bytes /* arg_1 */);

bool log_input_slices_equal(Bytes /* arg_0 */, Bytes /* arg_1 */);

uint64_t stateless_input_slice_byte(Bytes s, uint32_t off);

uint64_t memory_slice_byte(Bytes s, uint32_t off);

uint64_t code_slice_byte(Bytes s, uint32_t off);

uint64_t calldata_slice_byte(struct CalldataSlice s, uint32_t off);

uint32_t slice_count_nonzero(Bytes s);

u256 stateless_input_slice_load(Bytes s, uint32_t off);

u256 memory_slice_load(Bytes s, uint32_t off);

u256 calldata_slice_load(struct CalldataSlice s, uint32_t off);

u256 stateless_input_slice_load_word_offset(Bytes s, u256 off);

u256 memory_slice_load_word_offset(Bytes s, u256 off);

u256 calldata_slice_load_word_offset(struct CalldataSlice s, u256 off);

u256 stateless_input_slice_load_n(Bytes s, uint32_t off, uint8_t n);

u256 code_slice_load_n(Bytes s, uint32_t off, uint8_t n);

u256 scratch_slice_load_n(Bytes s, uint32_t off, uint8_t n);

void stateless_input_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len);

void memory_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len);

void code_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len);

void output_slice_copy(Bytes s, uint32_t dst, uint32_t off, uint32_t len);

void stateless_input_slice_copy_word_offset(Bytes s, uint32_t dst, u256 off, uint32_t len);

void memory_slice_copy_word_offset(Bytes s, uint32_t dst, u256 off, uint32_t len);

void code_slice_copy_word_offset(Bytes s, uint32_t dst, u256 off, uint32_t len);

void calldata_slice_copy_word_offset(struct CalldataSlice s, uint32_t dst, u256 off, uint32_t len);

uint64_t calldata_slice_byte_struct_CalldataSlice_uint8_t_to_uint64_t(struct CalldataSlice s, uint8_t off);

u256 calldata_slice_load_struct_CalldataSlice_uint8_t_to_u256(struct CalldataSlice s, uint8_t off);

uint64_t log_data_byte(Bytes s, uint8_t off);

u256 log_data_slice_load_Bytes_uint16_t_to_u256(Bytes s, uint16_t off);

u256 log_data_slice_load_Bytes_uint8_t_to_u256(Bytes s, uint8_t off);

uint64_t memory_slice_byte_Bytes_uint8_t_to_uint64_t(Bytes s, uint8_t off);

u256 memory_slice_load_Bytes_uint8_t_to_u256(Bytes s, uint8_t off);

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint32_t count);

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint8_t count);

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint32_t stride, uint8_t width, uint32_t count);

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint32_t count);

bool memory_slice_strided_zero_value_Bytes_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint8_t count);

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint32_t count);

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint8_t count);

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint32_t count);

bool memory_slice_strided_zero_value_Bytes_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint8_t count);

uint64_t output_byte(Bytes s, uint8_t off);

void output_slice_copy_Bytes_uint32_t_uint8_t_uint32_t_to_unit(Bytes s, uint32_t dst, uint8_t off, uint32_t len);

uint64_t scratch_byte(Bytes s, uint8_t off);

u256 scratch_slice_load(Bytes s, uint8_t off);

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint32_t start, uint16_t stride, uint8_t width, uint32_t count);

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint32_t start, uint16_t stride, uint8_t width, uint8_t count);

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint32_t start, uint32_t stride, uint8_t width, uint32_t count);

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint32_t start, uint8_t stride, uint8_t width, uint32_t count);

bool slice_strided_zero_struct_CalldataSlice_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint32_t start, uint8_t stride, uint8_t width, uint8_t count);

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint8_t start, uint16_t stride, uint8_t width, uint32_t count);

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint8_t start, uint16_t stride, uint8_t width, uint8_t count);

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(struct CalldataSlice s, uint8_t start, uint8_t stride, uint8_t width, uint32_t count);

bool slice_strided_zero_struct_CalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(struct CalldataSlice s, uint8_t start, uint8_t stride, uint8_t width, uint8_t count);

uint64_t stateless_input_slice_byte_Bytes_uint16_t_to_uint64_t(Bytes s, uint16_t off);

uint64_t stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(Bytes s, uint8_t off);

u256 stateless_input_slice_load_Bytes_uint16_t_to_u256(Bytes s, uint16_t off);

u256 stateless_input_slice_load_Bytes_uint8_t_to_u256(Bytes s, uint8_t off);

u256 stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(Bytes s, uint8_t off, uint8_t n);

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint32_t count);

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint16_t stride, uint8_t width, uint8_t count);

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint32_t stride, uint8_t width, uint32_t count);

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint32_t count);

bool stateless_input_slice_strided_zero_Bytes_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint32_t start, uint8_t stride, uint8_t width, uint8_t count);

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint32_t count);

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint16_t stride, uint8_t width, uint8_t count);

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint32_t count);

bool stateless_input_slice_strided_zero_Bytes_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(Bytes s, uint8_t start, uint8_t stride, uint8_t width, uint8_t count);


#ifdef __cplusplus
}
#endif
