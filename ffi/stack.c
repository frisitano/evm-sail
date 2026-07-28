/* C-backed EVM operand stack for the evm-sail model.
 *
 * The stack was a Sail list(word): every PUSH heap-allocated a cons cell (a
 * malloc per stack operation, millions per block). Here each call frame has a
 * flat array of 256-bit words with O(1) push/pop/peek/set; the per-depth
 * arrays are allocated once and REUSED across frames, so steady-state
 * execution performs no allocation. Frames form a stack: stack_enter_frame on call,
 * stack_leave_frame on return, stack_reset per transaction.
 *
 * Words cross both FFIs as inferred inline sail_u256 values and map directly
 * to the cached four-limb rows here.
 *
 * Bounds policy: the EVM's 1024-word stack limit is enforced by the Sail side
 * (push checks the height and raises StackOverflow); the C side only guards
 * its own capacity. */
#include "sail.h"
#include "optimized_stack.h"
#include "value_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HS_MAXDEPTH 1100        /* call-depth limit 1024 + tx frame + slack */
#define HS_CAP      1032        /* per-frame words: EVM limit 1024 + slack  */

typedef struct {
  uint64_t (*w)[4];             /* HS_CAP x 4 limbs, [0] = least significant */
  uint32_t n;                   /* current height                            */
} hs_frame;

_Static_assert(sizeof(sail_u256) == sizeof(uint64_t[4]),
               "optimized stack rows must match sail_u256");

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
EVMSAIL_WORD_RETURN stack_pop_word(EVMSAIL_WORD_RESULT(result) const unit u) {
  (void)u;
  static const uint64_t zero[4] = {0, 0, 0, 0};
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && f->n) {
    f->n--;
    EVMSAIL_RETURN_WORD(result, le_words4_to_sail_word(f->w[f->n]));
  }
  EVMSAIL_RETURN_WORD(result, le_words4_to_sail_word(zero));
}

unit stack_push_word(EVMSAIL_WORD_PARAM(w)) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && f->n < HS_CAP)
    sail_word_to_le_words4(f->w[f->n++], EVMSAIL_WORD_VALUE(w));
  return UNIT;
}

/* the nth-from-top word (n = 0 is the top); zero if out of range */
EVMSAIL_WORD_RETURN stack_peek_word(EVMSAIL_WORD_RESULT(result) uint64_t n) {
  static const uint64_t zero[4] = {0, 0, 0, 0};
  hs_frame *f = &hs_stk[hs_top];
  EVMSAIL_RETURN_WORD(
      result,
      le_words4_to_sail_word((!f->w || n >= f->n) ? zero
                                                   : f->w[f->n - 1 - n]));
}

/* overwrite the nth-from-top word (SWAP) */
unit stack_set_word(uint64_t n, EVMSAIL_WORD_PARAM(w)) {
  hs_frame *f = &hs_stk[hs_top];
  if (f->w && n < f->n)
    sail_word_to_le_words4(f->w[f->n - 1 - n], EVMSAIL_WORD_VALUE(w));
  return UNIT;
}

enum evmsail_stack_rewrite_status evmsail_stack_rewrite(
    uint32_t inputs, uint32_t outputs, sail_u256 **rows) {
  hs_frame *f = &hs_stk[hs_top];
  if (!f->w || f->n < inputs)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  const uint32_t retained = f->n - inputs;
  if (outputs > 1024 - retained)
    return EVMSAIL_STACK_REWRITE_OVERFLOW;

  *rows = (sail_u256 *)&f->w[retained];
  f->n = retained + outputs;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum evmsail_stack_rewrite_status evmsail_stack_dup(uint32_t depth) {
  hs_frame *f = &hs_stk[hs_top];
  if (!f->w || depth == 0 || f->n < depth)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;
  if (f->n == 1024)
    return EVMSAIL_STACK_REWRITE_OVERFLOW;

  memcpy(f->w[f->n], f->w[f->n - depth], sizeof(f->w[f->n]));
  f->n++;
  return EVMSAIL_STACK_REWRITE_OK;
}

enum evmsail_stack_rewrite_status evmsail_stack_swap(uint32_t other_depth) {
  hs_frame *f = &hs_stk[hs_top];
  if (!f->w || other_depth >= f->n)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  uint64_t temporary[4];
  memcpy(temporary, f->w[f->n - 1], sizeof(temporary));
  memcpy(f->w[f->n - 1], f->w[f->n - 1 - other_depth],
         sizeof(f->w[f->n - 1]));
  memcpy(f->w[f->n - 1 - other_depth], temporary, sizeof(temporary));
  return EVMSAIL_STACK_REWRITE_OK;
}

enum evmsail_stack_rewrite_status evmsail_stack_exchange(
    uint32_t left_depth, uint32_t right_depth) {
  hs_frame *f = &hs_stk[hs_top];
  const uint32_t required =
      left_depth > right_depth ? left_depth : right_depth;
  if (!f->w || required >= f->n)
    return EVMSAIL_STACK_REWRITE_UNDERFLOW;

  uint64_t temporary[4];
  memcpy(temporary, f->w[f->n - 1 - left_depth], sizeof(temporary));
  memcpy(f->w[f->n - 1 - left_depth], f->w[f->n - 1 - right_depth],
         sizeof(f->w[f->n - 1 - left_depth]));
  memcpy(f->w[f->n - 1 - right_depth], temporary, sizeof(temporary));
  return EVMSAIL_STACK_REWRITE_OK;
}
