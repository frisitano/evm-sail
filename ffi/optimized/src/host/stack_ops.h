/* Optimized interpreter access to the C-private operand-stack rows.
 *
 * These helpers are not Sail host axioms. They are used only by the optimized
 * C interpreter, which carries the StackTop cursor as a raw row pointer in a
 * loop local and validates every instruction's stack effect against the
 * frame base before touching rows. The Sail opcode bodies continue to use
 * the stack host axioms in evmsail/host/stack.h.
 */
#ifndef EVMSAIL_OPTIMIZED_STACK_OPS_H
#define EVMSAIL_OPTIMIZED_STACK_OPS_H

#include "evmsail/prelude.h"

/* The active frame's deepest row: the base the carried cursor is measured
 * against. Reloaded by the interpreter at the same frame boundaries that
 * reload the code slice. */
u256 *stack_frame_base(void);

#endif
