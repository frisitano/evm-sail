/* C-backed EVM operand stack (see stack.c): per-call-frame flat word
 * arrays with O(1) push/pop/peek/set and zero steady-state allocation.
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits cross the FFI. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_STACK_H
#include "evmsail/prelude.h"
#include <stdint.h>
#endif
