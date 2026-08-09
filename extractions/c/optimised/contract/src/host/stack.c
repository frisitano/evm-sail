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
#include "host/stack_ops.h"
#include "workspace.h"

#include <stddef.h>
#include <stdint.h>

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

uint64_t stack_reset(void)
{
  operand_stack.depth = 0;
  return (uint64_t)(uintptr_t)operand_stack.frames[0].words;
}

/* The Sail-enforced 1024 call-depth limit keeps depth below
 * GUEST_OPERAND_FRAMES. */
uint64_t operand_stack_push_empty_frame(void)
{
  operand_stack.depth++;
  return (uint64_t)(uintptr_t)operand_stack.frames[operand_stack.depth].words;
}

void operand_stack_pop_frame(void)
{
  if (operand_stack.depth == 0) {
    GUEST_ABORT();
  }
  operand_stack.depth--;
}

/* The interpreter's carried frame base: the deepest row of the active
 * frame, reloaded by the C loop at the same frame boundaries that reload
 * the code slice. */
u256 *stack_frame_base(void)
{
  return operand_stack.frames[operand_stack.depth].words;
}

uint16_t stack_top_height(uint64_t top)
{
  return (uint16_t)((const u256 *)(uintptr_t)top - operand_stack.frames[operand_stack.depth].words);
}

u256 stack_slot_read(uint64_t top, uint16_t index)
{
  const u256 *rows = (const u256 *)(uintptr_t)top;
  return rows[-1 - (ptrdiff_t)index];
}

void stack_slot_write(uint64_t top, uint16_t index, u256 word)
{
  u256 *rows = (u256 *)(uintptr_t)top;
  rows[-1 - (ptrdiff_t)index] = word;
}

uint64_t stack_top_advance(uint64_t top, uint16_t count)
{
  return top + (uint64_t)count * sizeof(u256);
}

uint64_t stack_top_retreat(uint64_t top, uint16_t count)
{
  return top - (uint64_t)count * sizeof(u256);
}
