/* Fixed-layout suspended-frame stack for the optimized ABI.
 *
 * One continuation is suspended per interpreter call frame, so the capacity
 * is the EVM call-depth limit. The generated model header owns
 * FrameContinuation's layout; the empty pop returns the Kind_Empty arm,
 * which is the Sail-visible bottom-of-stack sentinel rather than a failure. */
#include "evmsail/host/frame_stack.h"
#include "evmsail/spec/primitives/evm.h"
#include "workspace.h"

#include <stdint.h>

typedef struct {
  /* Depth-indexed continuation slots, reused across transactions. */
  struct FrameContinuation *slots;
  /* Number of live suspended frames. */
  uint64_t top;
} FrameStack;

static FrameStack frame_stack;

void frame_stack_workspace_bind(void)
{
  WORKSPACE_BIND(frame_stack.slots, GUEST_SUSPENDED_FRAMES);
}

void frame_stack_reset(void)
{
  frame_stack.top = 0;
}

void frame_stack_push(struct FrameContinuation continuation)
{
  if (frame_stack.top >= GUEST_SUSPENDED_FRAMES) {
    GUEST_ABORT();
  }
  frame_stack.slots[frame_stack.top++] = continuation;
}

struct FrameContinuation frame_stack_pop(void)
{
  const struct FrameContinuation *continuation = frame_stack_pop_borrowed();
  if (continuation == NULL) {
    return (struct FrameContinuation){.kind = Kind_Empty};
  }
  return *continuation;
}

const struct FrameContinuation *frame_stack_pop_borrowed(void)
{
  if (frame_stack.top == 0) {
    return NULL;
  }
  return &frame_stack.slots[--frame_stack.top];
}
