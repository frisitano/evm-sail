/* C-backed EVM operand stack for the evm-sail model.
 *
 * The stack was a Sail list(word): every PUSH heap-allocated a cons cell (a
 * malloc per stack operation, millions per block). Here each call frame has a
 * flat array of 256-bit words; the per-depth arrays are allocated once and
 * REUSED across frames, so steady-state execution performs no allocation.
 *
 * The active frame's position is the Sail-threaded StackTop cursor, which
 * this specification ABI represents as the frame height. The cursor lives in
 * the model's `stack_top` register at frame boundaries and is threaded by
 * value through the interpreter between them; this module owns only the word
 * storage and the frame-of-frames structure. Frames form a stack:
 * operand_stack_push_empty_frame on call (returning the child's empty
 * cursor), operand_stack_pop_frame on return (the parent's cursor is
 * restored from its frame checkpoint), and stack_reset per transaction.
 *
 * Words cross both FFIs as inferred inline u256 values and map directly
 * to the cached four-limb rows here.
 *
 * Bounds policy: the EVM's 1024-word stack limit is enforced by the Sail
 * side (validate_stack guards every instruction's stack effect before its
 * handler runs); the C side only guards its own capacity. */
#include "sail.h"
#include "stack.h"
#include "value_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HS_MAXDEPTH 1100        /* call-depth limit 1024 + tx frame + slack */
#define HS_CAP      1032        /* per-frame words: EVM limit 1024 + slack  */

typedef struct {
  uint64_t (*w)[4];             /* HS_CAP x 4 limbs, [0] = least significant */
} hs_frame;

_Static_assert(sizeof(u256) == sizeof(uint64_t[4]),
               "optimized stack rows must match u256");

static hs_frame hs_stk[HS_MAXDEPTH];
static int hs_top = 0;

static void hs_ensure(int i) {
  if (!hs_stk[i].w)
    hs_stk[i].w = (uint64_t (*)[4])calloc(HS_CAP, sizeof(uint64_t[4]));
}

/* clear to a single empty base frame (per transaction); arrays stay cached */
uint64_t stack_reset(const unit u) {
  (void)u;
  hs_top = 0;
  hs_ensure(0);
  return 0;
}

/* enter a call: fresh empty frame (reusing the depth slot's array) */
uint64_t operand_stack_push_empty_frame(const unit u) {
  (void)u;
  if (hs_top + 1 < HS_MAXDEPTH) {
    hs_top++;
    hs_ensure(hs_top);
  }
  return 0;
}

/* leave a call: drop the frame storage; the parent cursor is restored from
 * its checkpointed stack_top register value */
unit operand_stack_pop_frame(const unit u) {
  (void)u;
  if (hs_top > 0) hs_top--;
  return UNIT;
}

uint64_t stack_top_height(uint64_t top) { return top; }

/* the word `index` slots below the cursor's top (0 is the top); the Sail
 * side has validated index < height */
u256 stack_slot_read(uint64_t top, uint64_t index) {
  hs_frame *f = &hs_stk[hs_top];
  return (le_words4_to_sail_word(f->w[top - 1 - index]));
}

unit stack_slot_write(uint64_t top, uint64_t index, const u256 w) {
  hs_frame *f = &hs_stk[hs_top];
  sail_word_to_le_words4(f->w[top - 1 - index], (w));
  return UNIT;
}

uint64_t stack_top_advance(uint64_t top, uint64_t count) { return top + count; }

uint64_t stack_top_retreat(uint64_t top, uint64_t count) { return top - count; }
