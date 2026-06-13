/* Native stub for the SSZ private-input readers (io.sail externs). The native
 * EEST runner does not perform stateless witness validation -- it drives tests
 * through its own protocol and roots the post-state from the live accounts --
 * so the witness path (compiled into the model now that io.sail is in core) is
 * never executed here; these return empty input. The zkVM guest provides the
 * REAL readers in zkvm/runtime/el_input.c. */
#include "sail.h"
#include <stdint.h>
void     ssz_src_len(sail_int rop, const unit u) { (void)u; mpz_set_ui(rop, 0); }
uint64_t ssz_src_byte(sail_int idx)              { (void)idx; return 0; }
uint64_t ssz_src_le(sail_int off, sail_int n)    { (void)off; (void)n; return 0; }
uint64_t ssz_src_be(sail_int off, sail_int n)    { (void)off; (void)n; return 0; }
