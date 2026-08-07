/* C-backed EVM operand stack for the evm-sail model.
 *
 * The stack was a Sail list(word): every PUSH heap-allocated a cons cell (a
 * malloc per stack operation, millions per block). Here each call frame has a
 * flat array of u256 rows with O(1) push/pop/peek/set; the per-depth frames
 * are bound once to the guest workspace and reused across calls, so execution
 * performs no allocation. Frames form a stack:
 * operand_stack_push_empty_frame on call, operand_stack_pop_frame on return,
 * and stack_reset per transaction.
 *
 * Words cross both FFIs as inferred inline u256 values and are stored as
 * u256 rows directly; limbs[0] is the least-significant limb.
 *
 * Bounds policy: the EVM's protocol limits (1024-word stack, 1024 call
 * depth) are enforced by the Sail EVM layer before these calls, so the host
 * mechanism performs no re-checks. The one exception is stack_peek_word,
 * whose zero-out-of-range result is part of the documented host contract in
 * sail/host/stack.sail. */
#include "evmsail/host/stack.h"
#include "evmsail/prelude.h"
#include "host/stack_ops.h"
#include "workspace.h"

#include <stdint.h>

/* YP §9.1: the EVM operand-stack item limit. */
#define EVM_STACK_LIMIT 1024u

typedef struct {
  u256 words[GUEST_OPERAND_FRAME_WORDS];
  uint16_t height;
} OperandFrame;

typedef struct {
  OperandFrame *frames;
  uint32_t depth;
} OperandStack;

static OperandStack operand_stack;
static const u256 stack_zero = {{0}};

void stack_workspace_bind(void)
{
  WORKSPACE_BIND(operand_stack.frames, GUEST_OPERAND_FRAMES);
  operand_stack.depth = 0;
  operand_stack.frames[0].height = 0;
}

void stack_reset(void)
{
  operand_stack.depth = 0;
  operand_stack.frames[0].height = 0;
}

void operand_stack_push_empty_frame(void)
{
  if (operand_stack.depth + 1U >= GUEST_OPERAND_FRAMES) {
    GUEST_ABORT();
  }
  operand_stack.depth++;
  operand_stack.frames[operand_stack.depth].height = 0;
}

void operand_stack_pop_frame(void)
{
  if (operand_stack.depth == 0) {
    GUEST_ABORT();
  }
  operand_stack.depth--;
}

uint16_t stack_depth(void)
{
  return operand_stack.frames[operand_stack.depth].height;
}

u256 stack_pop_word(void)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  return frame->words[--frame->height];
}

void stack_push_word(const u256 word)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  frame->words[frame->height++] = word;
}

u256 stack_peek_word(uint16_t index)
{
  const OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  return index >= frame->height ? stack_zero : frame->words[frame->height - 1U - index];
}

void stack_set_word(uint16_t index, u256 word)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  frame->words[frame->height - 1U - index] = word;
}

/* One bounds predicate for an instruction stack effect: `inputs` popped,
 * `outputs` pushed, within the 1024-item limit. */
static enum stack_rewrite_status stack_effect_status(const OperandFrame *frame, uint32_t inputs,
                                                     uint32_t outputs)
{
  if (frame->height < inputs) {
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;
  }
  if (outputs > EVM_STACK_LIMIT - (frame->height - inputs)) {
    return EVMSAIL_STACK_REWRITE_OVERFLOW;
  }
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_rewrite(uint32_t inputs, uint32_t outputs, u256 **rows)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  const enum stack_rewrite_status status = stack_effect_status(frame, inputs, outputs);
  if (status != EVMSAIL_STACK_REWRITE_OK) {
    return status;
  }

  const uint32_t retained = frame->height - inputs;
  *rows = &frame->words[retained];
  /* stack_effect_status proved the new height is at most EVM_STACK_LIMIT. */
  frame->height = (uint16_t)(retained + outputs);
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_validate(uint32_t inputs, uint32_t outputs)
{
  return stack_effect_status(&operand_stack.frames[operand_stack.depth], inputs, outputs);
}

enum stack_rewrite_status stack_dup(uint32_t depth)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (depth == 0 || frame->height < depth) {
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;
  }
  if (frame->height == EVM_STACK_LIMIT) {
    return EVMSAIL_STACK_REWRITE_OVERFLOW;
  }

  frame->words[frame->height] = frame->words[frame->height - depth];
  frame->height++;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_swap(uint32_t other_depth)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (other_depth >= frame->height) {
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;
  }

  const u256 temporary = frame->words[frame->height - 1U];
  frame->words[frame->height - 1U] = frame->words[frame->height - 1U - other_depth];
  frame->words[frame->height - 1U - other_depth] = temporary;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_exchange(uint32_t left_depth, uint32_t right_depth)
{
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  const uint32_t required = left_depth > right_depth ? left_depth : right_depth;
  if (required >= frame->height) {
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;
  }

  const u256 temporary = frame->words[frame->height - 1U - left_depth];
  frame->words[frame->height - 1U - left_depth] = frame->words[frame->height - 1U - right_depth];
  frame->words[frame->height - 1U - right_depth] = temporary;
  return EVMSAIL_STACK_REWRITE_OK;
}
