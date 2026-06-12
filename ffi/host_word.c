/* Native 256-bit word predicates for the evm-sail model.
 *
 * The hot comparison opcodes (LT/GT/SLT/SGT/ISZERO) were implemented as
 * unsigned(a) < unsigned(b): two word -> integer conversions per comparison
 * (each building a 512-bit sail_int) just to compare four limbs. These
 * predicates compare lbits values directly -- one FFI call, no conversion.
 *
 * The fixed-width runtimes (sailfix/sail256, detected by SAIL_INT_LIMBS) hold
 * lbits inline; the stock GMP runtime path uses mpz comparison (the GMP build
 * is the reference oracle, not the performance path). */
#include "sail.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef SAIL_INT_LIMBS
/* ---- fixed-width runtimes: lbits = { len, d[4] }, d[0] least significant */

bool hw_ult(const lbits a, const lbits b) {
  for (int i = 3; i >= 0; i--) {
    if (a.d[i] != b.d[i]) return a.d[i] < b.d[i];
  }
  return false;
}

bool hw_slt(const lbits a, const lbits b) {        /* two's-complement signed < */
  uint64_t sa = a.d[3] >> 63, sb = b.d[3] >> 63;
  if (sa != sb) return sa > sb;                    /* negative < non-negative */
  return hw_ult(a, b);                             /* same sign: magnitude order */
}

bool hw_iszero(const lbits a) {
  return (a.d[0] | a.d[1] | a.d[2] | a.d[3]) == 0;
}

#else
/* ---- stock GMP runtime: lbits = { len, mpz_t* } -- reference path ---- */

bool hw_ult(const lbits a, const lbits b) { return mpz_cmp(*a.bits, *b.bits) < 0; }

bool hw_slt(const lbits a, const lbits b) {
  int sa = mpz_tstbit(*a.bits, 255), sb = mpz_tstbit(*b.bits, 255);
  if (sa != sb) return sa > sb;
  return mpz_cmp(*a.bits, *b.bits) < 0;
}

bool hw_iszero(const lbits a) { return mpz_sgn(*a.bits) == 0; }

#endif
