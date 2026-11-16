#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

StackPointer stack_reset_(void)
{
  return stack_reset();
}

StackPointer operand_stack_push_empty_frame_(void)
{
  return operand_stack_push_empty_frame();
}

uint16_t stack_top_height_(StackPointer top)
{
  return top.height;
}

u256 stack_slot_read_(StackPointer top, uint16_t index)
{
  return stack_slot_read(top, index);
}

void stack_slot_write_(StackPointer top, uint16_t index, u256 value)
{
  stack_slot_write(top, index, value);
}

u256 stack_slot_read_StackPointer_uint8_t_to_u256(StackPointer top, uint8_t index)
{
  return stack_slot_read(top, (uint16_t)index);
}

void stack_slot_write_StackPointer_uint8_t_u256_to_unit(StackPointer top, uint8_t index, u256 value)
{
  stack_slot_write(top, (uint16_t)index, value);
}

StackPointer stack_top_advance_(StackPointer top, uint8_t count)
{
  return stack_top_advance(top, (uint16_t)count);
}

StackPointer stack_top_retreat_(StackPointer top, uint8_t count)
{
  return stack_top_retreat(top, (uint16_t)count);
}
