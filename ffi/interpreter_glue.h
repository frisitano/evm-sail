#ifndef EVMSAIL_INTERPRETER_GLUE_H
#define EVMSAIL_INTERPRETER_GLUE_H

#include "sail_abi.h"

/*
 * Optimized-C refinement of the explicit Sail interpreter loop. This header
 * and its implementation are injected only into optimized C builds. Standard
 * C and proof extraction continue to use interpreter.sail directly.
 */
struct zOutputSliceFields;

struct zOutputSliceFields evmsail_interpret(unit);

#endif
