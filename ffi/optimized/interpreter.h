#ifndef EVMSAIL_OPTIMIZED_INTERPRETER_H
#define EVMSAIL_OPTIMIZED_INTERPRETER_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

/*
 * Optimized-C refinement of the explicit Sail interpreter loop. This header
 * and its implementation are injected only into optimized C builds. Standard
 * C and proof extraction continue to use interpreter.sail directly.
 */
struct zOutputSliceFields;

struct zOutputSliceFields evmsail_interpret(unit);

#endif
