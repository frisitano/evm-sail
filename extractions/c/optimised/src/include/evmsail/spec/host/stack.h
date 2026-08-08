#pragma once

#include "evmsail/spec/host/environment.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint64_t stack_reset(void);

uint64_t operand_stack_push_empty_frame(void);

void operand_stack_pop_frame(void);

uint16_t stack_top_height(uint64_t /* arg_0 */);

u256 stack_slot_read(uint64_t /* arg_0 */, uint16_t /* arg_1 */);

void stack_slot_write(uint64_t /* arg_0 */, uint16_t /* arg_1 */, u256 /* arg_2 */);

uint64_t stack_top_advance(uint64_t /* arg_0 */, uint16_t /* arg_1 */);

uint64_t stack_top_retreat(uint64_t /* arg_0 */, uint16_t /* arg_1 */);


#ifdef __cplusplus
}
#endif
