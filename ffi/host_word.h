/* Native 256-bit word predicates (see host_word.c): direct lbits comparison,
 * no word -> integer conversion. Declared here so the Sail-generated C call
 * sites are prototyped via `sail -c --c-include`. */
#ifndef HOST_WORD_H
#define HOST_WORD_H
#include "sail.h"
#include <stdbool.h>
bool hw_ult(const lbits a, const lbits b);     /* unsigned a < b              */
bool hw_slt(const lbits a, const lbits b);     /* two's-complement a < b      */
bool hw_iszero(const lbits a);                 /* a == 0                      */
#endif
