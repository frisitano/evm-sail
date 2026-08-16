#pragma once

#include "evmsail/spec/host/frame_stack.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void mem_write_byte(uint32_t /* arg_0 */, uint64_t /* arg_1 */);

void mem_expand(uint32_t /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */);

Bytes mem_view(uint32_t /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */);

void mem_move(uint32_t /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */);

u256 mem_load_word(uint32_t /* arg_0 */);

void mem_store_word(uint32_t /* arg_0 */, u256 /* arg_1 */);


#ifdef __cplusplus
}
#endif
