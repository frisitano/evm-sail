#pragma once

#include "evmsail/spec/host/environment.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

StackPointer stack_reset(void);

StackPointer operand_stack_push_empty_frame(void);

void operand_stack_pop_frame(void);

u256 stack_slot_read(StackPointer /* arg_0 */, uint16_t /* arg_1 */);

void stack_slot_write(StackPointer /* arg_0 */, uint16_t /* arg_1 */, u256 /* arg_2 */);

StackPointer stack_top_advance(StackPointer /* arg_0 */, uint16_t /* arg_1 */);

StackPointer stack_top_retreat(StackPointer /* arg_0 */, uint16_t /* arg_1 */);

__attribute__((__always_inline__)) StackPointer stack_reset_(void);

__attribute__((__always_inline__)) StackPointer operand_stack_push_empty_frame_(void);

__attribute__((__always_inline__)) uint16_t stack_top_height_(StackPointer top);

__attribute__((__always_inline__)) u256 stack_slot_read_(StackPointer top, uint16_t index);

__attribute__((__always_inline__)) void stack_slot_write_(StackPointer top, uint16_t index, u256 value);

__attribute__((__always_inline__)) u256 stack_slot_read_StackPointer_uint8_t_to_u256(StackPointer top, uint8_t index);

__attribute__((__always_inline__)) void stack_slot_write_StackPointer_uint8_t_u256_to_unit(StackPointer top, uint8_t index, u256 value);

__attribute__((__always_inline__)) StackPointer stack_top_advance_(StackPointer top, uint8_t count);

__attribute__((__always_inline__)) StackPointer stack_top_retreat_(StackPointer top, uint8_t count);


#ifdef __cplusplus
}
#endif
