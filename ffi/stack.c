/* C-backed EVM operand stack for the evm-sail model.
 *
 * The stack was a Sail list(word): every PUSH heap-allocated a cons cell (a
 * malloc per stack operation, millions per block). Here each call frame has a
 * flat array of 256-bit words with O(1) push/pop/peek/set; the per-depth
 * arrays are allocated once and REUSED across frames, so steady-state
 * execution performs no allocation. Frames form a stack: stack_enter_frame on call,
 * stack_leave_frame on return, stack_reset per transaction.
 *
 * Words cross the FFI as WHOLE lbits values (one struct copy each way); both
 * runtime representations are handled by ffi/lbits_convert.h.
 *
 * Bounds policy: the EVM's 1024-word stack limit is enforced by the Sail side
 * (push checks the height and raises StackOverflow); the C side only guards
 * its own capacity. */
#include "sail.h"
#include "lbits_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HS_MAXDEPTH 1100        /* call-depth limit 1024 + tx frame + slack */
#define HS_CAP      1032        /* per-frame words: EVM limit 1024 + slack  */

typedef struct {
  uint64_t (*w)[4];             /* HS_CAP x 4 limbs, [0] = least significant */
  uint32_t n;                   /* current height                            */
} hs_frame;

static hs_frame hs_stk[HS_MAXDEPTH];
static int hs_top = 0;

static void hs_ensure(int i) {
  if (!hs_stk[i].w)
    hs_stk[i].w = (uint64_t (*)[4])calloc(HS_CAP, sizeof(uint64_t[4]));
}

/* clear to a single empty base frame (per transaction); arrays stay cached */
unit stack_reset(const unit u) {
  (void)u;
  hs_top = 0;
  hs_ensure(0);
  hs_stk[0].n = 0;
  return UNIT;
}

/* enter a call: fresh empty frame (reusing the depth slot's array) */
unit stack_enter_frame(const unit u) {
  (void)u;
  if (hs_top + 1 < HS_MAXDEPTH) {
    hs_top++;
    hs_ensure(hs_top);
    hs_stk[hs_top].n = 0;
  }
  return UNIT;
}

/* leave a call: drop the frame, restore the parent */
unit stack_leave_frame(const unit u) {
  (void)u;
  if (hs_top > 0) hs_top--;
  return UNIT;
}

uint64_t stack_depth(const unit u) { (void)u; return hs_stk[hs_top].n; }

/* pop AND return the top word (zero when empty; the Sail side guards
   underflow before calling) -- one crossing per POP instead of peek+drop */
void stack_pop_word(lbits *rop, const unit u) {
  (void)u;
  static const uint64_t zero[4] = {0, 0, 0, 0};
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && f->n) { f->n--; le_words4_to_lbits(rop, f->w[f->n]); }
  else le_words4_to_lbits(rop, zero);
}

unit stack_push_word(const lbits w) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && f->n < HS_CAP) lbits_to_le_words4(f->w[f->n++], w);
  return UNIT;
}

/* the nth-from-top word (n = 0 is the top); zero if out of range */
void stack_peek_word(lbits *rop, uint64_t n) {
  static const uint64_t zero[4] = {0, 0, 0, 0};
  hs_frame *f = &hs_stk[hs_top];
  le_words4_to_lbits(rop, (!f->w || n >= f->n) ? zero : f->w[f->n - 1 - n]);
}

/* overwrite the nth-from-top word (SWAP) */
unit stack_set_word(uint64_t n, const lbits w) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && n < f->n) lbits_to_le_words4(f->w[f->n - 1 - n], w);
  return UNIT;
}
