#pragma once

#include "evmsail/spec/host/memory.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bool output_buffer_store_memory(Bytes /* arg_0 */);

bool output_buffer_store_input(Bytes /* arg_0 */);

bool output_buffer_store_word(u256 /* arg_0 */);

bool output_buffer_store_words(u256 /* arg_0 */, u256 /* arg_1 */);

bool public_output_write(Bytes /* arg_0 */);

Bytes freeze_memory_output(Bytes data);

Bytes freeze_input_output(Bytes data);

Bytes freeze_calldata_output(struct CalldataSlice data);

Bytes output_buffer_word(u256 value);

Bytes output_buffer_words(u256 first, u256 second);

Bytes output_buffer_slice_uint16_t_to_Bytes(uint16_t len);

Bytes output_buffer_slice_uint32_t_to_Bytes(uint32_t len);

Bytes output_buffer_slice_uint8_t_to_Bytes(uint8_t len);


#ifdef __cplusplus
}
#endif
