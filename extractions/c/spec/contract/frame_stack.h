/* Host-backed suspended-frame continuation stack. The generated model header
 * owns zFrameContinuation's layout; this header only declares the C binding. */
#ifndef FRAME_STACK_H
#define FRAME_STACK_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

struct zFrameContinuation;

unit frame_stack_reset(unit u);
unit frame_stack_push(struct zFrameContinuation continuation);
void frame_stack_pop(struct zFrameContinuation *out, unit u);

#endif
