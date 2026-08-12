/* Host-backed suspended-frame continuation stack. The generated model header
 * owns FrameContinuation's layout; this header only declares the C binding. */
#ifndef EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H

#include "evmsail/prelude.h"

struct FrameContinuation;

void frame_stack_reset(void);
/* Borrow the popped row until the next frame-stack mutation. The threaded
 * interpreter consumes it immediately, avoiding a full checkpoint copy. */
const struct FrameContinuation *frame_stack_pop_borrowed(void);
/* By-value return matches the ABI the generated model declares for the
 * `frame_stack_pop` extern when the generated interpreter loop is kept
 *. */
struct FrameContinuation frame_stack_pop(void);

#endif
