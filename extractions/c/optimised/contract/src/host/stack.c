/* C-backed EVM operand stack for the evm-sail model.
 *
 * Each call frame has a flat array of u256 rows; the per-depth frames are
 * bound once to the guest workspace and reused across calls, so execution
 * performs no allocation. The active frame's position is the Sail-threaded
 * StackTop cursor, which this optimized ABI represents as a raw pointer to
 * the row one past the top. The cursor lives in the model's `stack_top`
 * register at frame boundaries and is threaded by value through the
 * interpreter between them; this module owns only the row storage and the
 * frame-of-frames structure. Frames form a stack:
 * operand_stack_push_empty_frame on call (returning the child's empty
 * cursor), operand_stack_pop_frame on return (the parent's cursor is
 * restored from its frame checkpoint), and stack_reset per transaction.
 *
 * Words cross both FFIs as inferred inline u256 values and are stored as
 * u256 rows directly; limbs[0] is the least-significant limb.
 *
 * Bounds policy: the EVM's protocol limits (1024-word stack, 1024 call
 * depth) are enforced by the Sail EVM layer (validate_stack guards every
 * instruction's stack effect before its handler runs), so the host
 * mechanism performs no re-checks. */
#include "evmsail/host/stack.h"
#include "evmsail/prelude.h"
#include "workspace.h"

#include <stddef.h>
#include <stdint.h>

#define STACK_ALWAYS_INLINE extern inline __attribute__((__always_inline__))

typedef struct {
  u256 words[GUEST_OPERAND_FRAME_WORDS];
} OperandFrame;

typedef struct {
  OperandFrame *frames;
  uint32_t depth;
} OperandStack;

static OperandStack operand_stack;

void stack_workspace_bind(void)
{
  WORKSPACE_BIND(operand_stack.frames, GUEST_OPERAND_FRAMES);
  operand_stack.depth = 0;
}

StackPointer stack_reset(void)
{
  operand_stack.depth = 0;
  return (StackPointer){.storage = operand_stack.frames[0].words, .height = 0};
}

/* The Sail-enforced 1024 call-depth limit keeps depth below
 * GUEST_OPERAND_FRAMES. */
StackPointer operand_stack_push_empty_frame(void)
{
  operand_stack.depth++;
  return (StackPointer){
      .storage = operand_stack.frames[operand_stack.depth].words,
      .height = 0,
  };
}

void operand_stack_pop_frame(void)
{
  if (operand_stack.depth == 0) {
    GUEST_ABORT();
  }
  operand_stack.depth--;
}

STACK_ALWAYS_INLINE uint16_t stack_top_height(StackPointer top)
{
  return top.height;
}

STACK_ALWAYS_INLINE u256 stack_slot_read(StackPointer top, uint16_t index)
{
  return top.storage[-1 - (ptrdiff_t)index];
}

STACK_ALWAYS_INLINE void stack_slot_write(StackPointer top, uint16_t index, u256 word)
{
  top.storage[-1 - (ptrdiff_t)index] = word;
}

STACK_ALWAYS_INLINE void stack_slot_write_next(StackPointer top, u256 word)
{
  top.storage[0] = word;
}

STACK_ALWAYS_INLINE StackPointer stack_top_advance(StackPointer top, uint16_t count)
{
  top.storage += count;
  top.height = (uint16_t)(top.height + count);
  return top;
}

STACK_ALWAYS_INLINE StackPointer stack_top_retreat(StackPointer top, uint16_t count)
{
  top.storage -= count;
  top.height = (uint16_t)(top.height - count);
  return top;
}
