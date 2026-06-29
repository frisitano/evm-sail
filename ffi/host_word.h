/* Native 256-bit word predicates (see host_word.c): direct lbits comparison,
 * no word -> integer conversion. Declared here so the Sail-generated C call
 * sites are prototyped via `sail -c --c-include`. */
#ifndef HOST_WORD_H
#define HOST_WORD_H
#include "sail.h"
#include <stdbool.h>
#include <stdint.h>
bool hw_ult(const lbits a, const lbits b);     /* unsigned a < b              */
bool hw_slt(const lbits a, const lbits b);     /* two's-complement a < b      */
bool hw_iszero(const lbits a);                 /* a == 0                      */
/* int64 gas subtraction: gas = range(0,2^63-1) is int64 in the generated ABI.
 * charge() proves a >= b >= 0 before calling, so a - b never under/overflows --
 * this avoids the per-opcode sail_int round-trip (CONVERT/sub_int/to_gas).     */
int64_t gas_sub(int64_t a, int64_t b);         /* a - b (caller proves a>=b>=0)*/
#endif
