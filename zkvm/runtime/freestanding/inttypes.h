/* Freestanding <inttypes.h> shim: just the fixed-width types + the few format
 * macros sail.c may reference. */
#ifndef ZKVM_FREESTANDING_INTTYPES_H
#define ZKVM_FREESTANDING_INTTYPES_H
#include <stdint.h>
#define PRId64 "ld"
#define PRIu64 "lu"
#define PRIx64 "lx"
#define PRIX64 "lX"
#define PRIi64 "li"
#endif
