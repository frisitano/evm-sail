/* C-backed EVM operand stack for the evm-sail model.
 *
 * The stack was a Sail list(word): every PUSH heap-allocated a cons cell (a
 * malloc per stack operation, millions per block). Here each call frame has a
 * flat array of 256-bit words with O(1) push/pop/peek/set; the per-depth
 * arrays are allocated once and REUSED across frames, so steady-state
 * execution performs no allocation. Frames form a stack: hs_enter on call,
 * hs_leave on return, hs_reset per transaction.
 *
 * Words cross the FFI as WHOLE lbits values (one struct copy each way). The
 * fixed-width runtimes (sailfix/sail256, detected by SAIL_INT_LIMBS) hold
 * lbits inline; the stock GMP runtime path converts via mpz import/export
 * (the GMP build is the reference oracle, not the performance path).
 *
 * Bounds policy: the EVM's 1024-word stack limit is enforced by the Sail side
 * (push checks the height and raises StackOverflow); the C side only guards
 * its own capacity. */
#include "sail.h"
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
unit hs_reset(const unit u) {
  (void)u;
  hs_top = 0;
  hs_ensure(0);
  hs_stk[0].n = 0;
  return UNIT;
}

/* enter a call: fresh empty frame (reusing the depth slot's array) */
unit hs_enter(const unit u) {
  (void)u;
  if (hs_top + 1 < HS_MAXDEPTH) {
    hs_top++;
    hs_ensure(hs_top);
    hs_stk[hs_top].n = 0;
  }
  return UNIT;
}

/* leave a call: drop the frame, restore the parent */
unit hs_leave(const unit u) {
  (void)u;
  if (hs_top > 0) hs_top--;
  return UNIT;
}

uint64_t hs_depth(const unit u) { (void)u; return hs_stk[hs_top].n; }

/* drop the top entry (the Sail side reads it first) */
unit hs_pop(const unit u) {
  (void)u;
  hs_frame *f = &hs_stk[hs_top];
  if (f->n) f->n--;
  return UNIT;
}

#ifdef SAIL_INT_LIMBS
/* ---- fixed-width runtimes (sailfix / sail256): lbits = { len, d[4] } ---- */

unit hs_push_w(const lbits w) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && f->n < HS_CAP) {
    uint64_t *e = f->w[f->n++];
    e[0] = w.d[0]; e[1] = w.d[1]; e[2] = w.d[2]; e[3] = w.d[3];
  }
  return UNIT;
}

/* the nth-from-top word (n = 0 is the top); zero if out of range */
void hs_peek_w(lbits *rop, uint64_t n) {
  hs_frame *f = &hs_stk[hs_top];
  rop->len = 256;
  if (!f->w || n >= f->n) { rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0; return; }
  const uint64_t *e = f->w[f->n - 1 - n];
  rop->d[0] = e[0]; rop->d[1] = e[1]; rop->d[2] = e[2]; rop->d[3] = e[3];
}

/* overwrite the nth-from-top word (SWAP) */
unit hs_set_w(uint64_t n, const lbits w) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && n < f->n) {
    uint64_t *e = f->w[f->n - 1 - n];
    e[0] = w.d[0]; e[1] = w.d[1]; e[2] = w.d[2]; e[3] = w.d[3];
  }
  return UNIT;
}

#else
/* ---- stock GMP runtime: lbits = { len, mpz_t* } -- reference path ---- */

unit hs_push_w(const lbits w) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && f->n < HS_CAP) {
    uint64_t *e = f->w[f->n++];
    e[0] = e[1] = e[2] = e[3] = 0;
    size_t cnt = 0;
    mpz_export(e, &cnt, -1, sizeof(uint64_t), 0, 0, *w.bits);   /* LS-limb first */
  }
  return UNIT;
}

void hs_peek_w(lbits *rop, uint64_t n) {
  hs_frame *f = &hs_stk[hs_top];
  rop->len = 256;
  if (!f->w || n >= f->n) { mpz_set_ui(*rop->bits, 0); return; }
  mpz_import(*rop->bits, 4, -1, sizeof(uint64_t), 0, 0, f->w[f->n - 1 - n]);
}

unit hs_set_w(uint64_t n, const lbits w) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && n < f->n) {
    uint64_t *e = f->w[f->n - 1 - n];
    e[0] = e[1] = e[2] = e[3] = 0;
    size_t cnt = 0;
    mpz_export(e, &cnt, -1, sizeof(uint64_t), 0, 0, *w.bits);
  }
  return UNIT;
}

#endif
