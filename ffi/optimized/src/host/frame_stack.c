/* Fixed-layout suspended-frame stack for the optimized ABI.
 *
 * One continuation is suspended per interpreter call frame, so the capacity
 * is the EVM call-depth limit. The generated model header owns
 * zFrameContinuation's layout; the empty pop returns the Kind_zEmpty arm,
 * which is the Sail-visible bottom-of-stack sentinel rather than a failure. */
#include "evmsail/prelude.h"
#include "evmsail/host/frame_stack.h"
#include "workspace.h"

#include <stdint.h>

typedef struct {
  /* Depth-indexed continuation slots, reused across transactions. */
  struct zFrameContinuation *slots;
  /* Number of live suspended frames. */
  uint64_t top;
} FrameStack;

static FrameStack frame_stack;

void frame_stack_workspace_bind(void) {
  WORKSPACE_BIND(frame_stack.slots, GUEST_SUSPENDED_FRAMES);
}

unit frame_stack_reset(unit u) {
  (void)u;
  frame_stack.top = 0;
  return UNIT;
}

unit frame_stack_push(struct zFrameContinuation continuation) {
  if (frame_stack.top >= GUEST_SUSPENDED_FRAMES) GUEST_ABORT();
  frame_stack.slots[frame_stack.top++] = continuation;
  return UNIT;
}

void frame_stack_pop(struct zFrameContinuation *out, unit u) {
  (void)u;
  if (frame_stack.top == 0) {
    out->kind = Kind_zEmpty;
    return;
  }
  *out = frame_stack.slots[--frame_stack.top];
}
