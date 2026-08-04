/* Host-backed suspended-frame continuation stack. The generated model header
 * owns FrameContinuation's layout; this header only declares the C binding. */
#ifndef EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_FRAME_STACK_H

#include "evmsail/prelude.h"

struct FrameContinuation;

unit frame_stack_reset(unit u);
unit frame_stack_push(struct FrameContinuation continuation);
void frame_stack_pop(struct FrameContinuation *out, unit u);

#endif
