/* C-backed EVM operand stack (see stack.c). The cursor points one u256 past
 * the live stack top and carries its already-validated semantic height. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_STACK_H
#include <stdbool.h>
#include <stdint.h>

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct {
  uint64_t limbs[4];
} u256;
#endif

typedef struct {
  u256 *storage;
  uint16_t height;
} StackPointer;

static inline bool eq_StackPointer(StackPointer lhs, StackPointer rhs)
{
  return lhs.storage == rhs.storage && lhs.height == rhs.height;
}

StackPointer stack_reset(void);
StackPointer operand_stack_push_empty_frame(void);
void operand_stack_pop_frame(void);
uint16_t stack_top_height(StackPointer top);
u256 stack_slot_read(StackPointer top, uint16_t index);
void stack_slot_write(StackPointer top, uint16_t index, u256 word);
void stack_slot_write_next(StackPointer top, u256 word);
StackPointer stack_top_advance(StackPointer top, uint16_t count);
StackPointer stack_top_retreat(StackPointer top, uint16_t count);
#endif
