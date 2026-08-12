/* Specification-ABI suspended-frame continuation stack.
 * Storage is reserved without initialization on the first push, so a guest
 * that never enters a child frame pays no allocation cost and deeper calls
 * require no reallocation or copying. The optimized ABI is inline. The
 * standard ABI owns the GMP-backed fields directly and initializes only the
 * slots it reaches. */
#include EVMSAIL_MODEL_H
#include "frame_stack.h"

#include <stdint.h>
#include <stdlib.h>

#define FRAME_STACK_CAPACITY UINT64_C(1024)

static struct zFrameContinuation *frame_slots;
static uint64_t frame_top;
static uint64_t frame_initialized;

static void frame_checkpoint_clone(struct zFrameCheckpoint *dst,
  const struct zFrameCheckpoint *src) {
  *dst = *src;
}

static void frame_checkpoint_drop(struct zFrameCheckpoint *checkpoint) {
  (void)checkpoint;
}

static void frame_continuation_drop(struct zFrameContinuation *continuation) {
  switch (continuation->kind) {
  case Kind_zResumeCall:
    frame_checkpoint_drop(
        &continuation->variants.zResumeCall.zcheckpoint);
    break;
  case Kind_zResumeCreate:
    frame_checkpoint_drop(
        &continuation->variants.zResumeCreate.zcheckpoint);
    break;
  default:
    break;
  }
}

static void frame_continuation_clone(struct zFrameContinuation *dst,
                                     const struct zFrameContinuation *src) {
  frame_continuation_drop(dst);
  *dst = *src;

  switch (src->kind) {
  case Kind_zResumeCall:
    frame_checkpoint_clone(&dst->variants.zResumeCall.zcheckpoint,
                           &src->variants.zResumeCall.zcheckpoint);
    break;
  case Kind_zResumeCreate:
    frame_checkpoint_clone(&dst->variants.zResumeCreate.zcheckpoint,
                           &src->variants.zResumeCreate.zcheckpoint);
    break;
  default:
    break;
  }
}
static void frame_stack_ensure(void) {
  if (frame_slots != NULL) return;
  frame_slots =
      malloc((size_t)FRAME_STACK_CAPACITY * sizeof(*frame_slots));
  if (frame_slots == NULL) abort();
}

unit frame_stack_reset(unit u) {
  (void)u;
  frame_top = 0;
  return UNIT;
}

unit frame_stack_push(struct zFrameContinuation continuation) {
  if (frame_top >= FRAME_STACK_CAPACITY) abort();
  frame_stack_ensure();
  if (frame_top == frame_initialized) {
    frame_slots[frame_top].kind = Kind_zEmpty;
    frame_initialized++;
  }
  frame_continuation_clone(&frame_slots[frame_top], &continuation);
  frame_top++;
  return UNIT;
}

void frame_stack_pop(struct zFrameContinuation *out, unit u) {
  (void)u;
  if (frame_top == 0) return;
  if (frame_slots == NULL) abort();
  frame_top--;
  frame_continuation_clone(out, &frame_slots[frame_top]);
}
