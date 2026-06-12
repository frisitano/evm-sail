/* C extern for the EL-IR EEST runner: read the next whitespace-delimited
 * decimal integer from stdin into a Sail arbitrary-precision int.
 * Signature matches Sail's C ABI for `val next_int : unit -> int` -- the result
 * mpz is passed in directly (sail_int = mpz_t). EOF yields 0 (stops the loop). */
#include "runner_ffi.h"
#include <stdio.h>

void next_int(sail_int rop, const unit u) {
  (void)u;
  char buf[160];
  if (scanf(" %159s", buf) != 1) { mpz_set_si(rop, 0); return; }
  mpz_set_str(rop, buf, 10);
}
