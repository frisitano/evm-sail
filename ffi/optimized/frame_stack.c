/* Fixed-layout suspended-frame stack for the optimized ABI. */
#include EVMSAIL_MODEL_H
#include "frame_stack.h"

#include <stdint.h>
#include <stdlib.h>

#define FRAME_STACK_CAPACITY UINT64_C(1024)

static struct zFrameContinuation *frame_slots;
static uint64_t frame_top;

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
  frame_slots[frame_top++] = continuation;
  return UNIT;
}

void frame_stack_pop(struct zFrameContinuation *out, unit u) {
  (void)u;
  if (frame_top == 0) {
    out->kind = Kind_zEmpty;
    return;
  }
  if (frame_slots == NULL) abort();
  *out = frame_slots[--frame_top];
}
