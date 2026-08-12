/* Host-backed suspended-frame continuation stack. The generated model header
 * owns FrameContinuation's layout; this header only declares the C binding. */
#ifndef EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H

#include "evmsail/prelude.h"

struct FrameContinuation;

void frame_stack_reset(void);
/* By-value return matches the ABI the generated model declares for the
 * `frame_stack_pop` extern when the generated interpreter loop is kept
 *. */
struct FrameContinuation frame_stack_pop(void);

#endif
