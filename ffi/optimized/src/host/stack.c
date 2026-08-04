/* C-backed EVM operand stack for the evm-sail model.
 *
 * The stack was a Sail list(word): every PUSH heap-allocated a cons cell (a
 * malloc per stack operation, millions per block). Here each call frame has a
 * flat array of U256 rows with O(1) push/pop/peek/set; the per-depth frames
 * are bound once to the guest workspace and reused across calls, so execution
 * performs no allocation. Frames form a stack: stack_enter_frame on call,
 * stack_leave_frame on return, stack_reset per transaction.
 *
 * Words cross both FFIs as inferred inline U256 values and are stored as
 * U256 rows directly; limbs[0] is the least-significant limb.
 *
 * Bounds policy: the EVM's protocol limits (1024-word stack, 1024 call
 * depth) are enforced by the Sail EVM layer before these calls, so a bounds
 * miss here is an invariant breach and fails closed. The one exception is
 * stack_peek_word, whose zero-out-of-range result is part of the documented
 * host contract in sail/host/stack.sail. */
#include "evmsail/host/stack.h"
#include "host/stack_ops.h"
#include "workspace.h"

#include <stdint.h>

typedef struct {
  U256 words[GUEST_OPERAND_FRAME_WORDS];
  uint32_t height;
} OperandFrame;

typedef struct {
  OperandFrame *frames;
  uint32_t depth;
} OperandStack;

static OperandStack operand_stack;
static const U256 stack_zero = {{0}};

void stack_workspace_bind(void) {
  WORKSPACE_BIND(operand_stack.frames, GUEST_OPERAND_FRAMES);
}

unit stack_reset(const unit u) {
  (void)u;
  operand_stack.depth = 0;
  operand_stack.frames[0].height = 0;
  return UNIT;
}

unit stack_enter_frame(const unit u) {
  (void)u;
  if (operand_stack.depth + 1u >= GUEST_OPERAND_FRAMES) GUEST_ABORT();
  operand_stack.depth++;
  operand_stack.frames[operand_stack.depth].height = 0;
  return UNIT;
}

unit stack_leave_frame(const unit u) {
  (void)u;
  if (operand_stack.depth == 0) GUEST_ABORT();
  operand_stack.depth--;
  return UNIT;
}

uint16_t stack_depth(const unit u) {
  (void)u;
  return operand_stack.frames[operand_stack.depth].height;
}

U256 stack_pop_word(const unit u) {
  (void)u;
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (frame->height == 0) GUEST_ABORT();
  return frame->words[--frame->height];
}

unit stack_push_word(const U256 word) {
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (frame->height >= GUEST_OPERAND_FRAME_WORDS) GUEST_ABORT();
  frame->words[frame->height++] = word;
  return UNIT;
}

U256 stack_peek_word(uint16_t index) {
  const OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  return index >= frame->height
             ? stack_zero
             : frame->words[frame->height - 1u - index];
}

unit stack_set_word(uint16_t index, U256 word) {
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (index >= frame->height) GUEST_ABORT();
  frame->words[frame->height - 1u - index] = word;
  return UNIT;
}

enum stack_rewrite_status stack_rewrite(
    uint32_t inputs, uint32_t outputs, U256 **rows) {
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (frame->height < inputs)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  const uint32_t retained = frame->height - inputs;
  if (outputs > 1024u - retained)
    return EVMSAIL_STACK_REWRITE_OVERFLOW;

  *rows = &frame->words[retained];
  frame->height = retained + outputs;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_validate(uint32_t inputs, uint32_t outputs) {
  const OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (frame->height < inputs)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  const uint32_t retained = frame->height - inputs;
  if (outputs > 1024u - retained)
    return EVMSAIL_STACK_REWRITE_OVERFLOW;

  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_dup(uint32_t depth) {
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (depth == 0 || frame->height < depth)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;
  if (frame->height == 1024u)
    return EVMSAIL_STACK_REWRITE_OVERFLOW;

  frame->words[frame->height] = frame->words[frame->height - depth];
  frame->height++;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_swap(uint32_t other_depth) {
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  if (other_depth >= frame->height)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  const U256 temporary = frame->words[frame->height - 1u];
  frame->words[frame->height - 1u] =
      frame->words[frame->height - 1u - other_depth];
  frame->words[frame->height - 1u - other_depth] = temporary;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum stack_rewrite_status stack_exchange(
    uint32_t left_depth, uint32_t right_depth) {
  OperandFrame *frame = &operand_stack.frames[operand_stack.depth];
  const uint32_t required =
      left_depth > right_depth ? left_depth : right_depth;
  if (required >= frame->height)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  const U256 temporary = frame->words[frame->height - 1u - left_depth];
  frame->words[frame->height - 1u - left_depth] =
      frame->words[frame->height - 1u - right_depth];
  frame->words[frame->height - 1u - right_depth] = temporary;
  return EVMSAIL_STACK_REWRITE_OK;
}
