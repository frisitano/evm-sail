/* Fixed-layout suspended-frame stack for the optimized ABI.
 *
 * One continuation is suspended per interpreter call frame, so the capacity
 * is the EVM call-depth limit. The generated model header owns
 * FrameContinuation's layout; the empty pop returns the Kind_Empty arm,
 * which is the Sail-visible bottom-of-stack sentinel rather than a failure. */
#include "evmsail/host/frame_stack.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/spec/primitives/evm.h"
#include "host/state/internal.h"
#include "workspace.h"

#include <stdint.h>

typedef struct {
  /* Depth-indexed continuation slots, reused across transactions. */
  struct FrameContinuation *slots;
  /* Parent frame-owner contexts, parallel to continuation slots. */
  AccountId *current_account_ids;
  /* Number of live suspended frames. */
  uint64_t top;
} FrameStack;

static FrameStack frame_stack;

void frame_stack_workspace_bind(void)
{
  WORKSPACE_BIND(frame_stack.slots, GUEST_SUSPENDED_FRAMES);
  WORKSPACE_BIND(frame_stack.current_account_ids, GUEST_SUSPENDED_FRAMES);
}

void frame_stack_reset(void)
{
  frame_stack.top = 0;
  current_account_context_invalidate();
}

void frame_stack_push(struct FrameContinuation continuation)
{
  if (frame_stack.top >= GUEST_SUSPENDED_FRAMES) {
    GUEST_ABORT();
  }
  frame_stack.current_account_ids[frame_stack.top] = current_account_context_id();
  frame_stack.slots[frame_stack.top++] = continuation;
}

void frame_stack_pop(struct FrameContinuation *out)
{
  if (frame_stack.top == 0) {
    out->kind = Kind_Empty;
    return;
  }
  frame_stack.top--;
  *out = frame_stack.slots[frame_stack.top];
  current_account_context_restore(frame_stack.current_account_ids[frame_stack.top]);
}
