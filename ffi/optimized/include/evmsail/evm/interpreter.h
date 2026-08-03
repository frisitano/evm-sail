#ifndef EVMSAIL_OPTIMIZED_EVM_INTERPRETER_H
#define EVMSAIL_OPTIMIZED_EVM_INTERPRETER_H

#include "evmsail/prelude.h"

/*
 * Optimized-C refinement of the explicit Sail interpreter loop. This header
 * and its implementation are injected only into optimized C builds. Standard
 * C and proof extraction continue to use interpreter.sail directly.
 */
struct zOutputSliceFields;

struct zOutputSliceFields interpret(unit);

#endif
