/* Host-backed suspended-frame continuation stack. The generated model header
 * owns FrameContinuation's layout; this header only declares the C binding. */
#ifndef EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H

#include "evmsail/prelude.h"

struct FrameContinuation;

void frame_stack_reset(void);
void frame_stack_pop(struct FrameContinuation *out);

#endif
