/* C-backed EVM operand stack (see stack.c): per-call-frame flat word
 * arrays with O(1) push/pop/peek/set and zero steady-state allocation.
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits cross the FFI. */
#ifndef STACK_H
#define STACK_H
#include "sail.h"
#include <stdint.h>
unit stack_reset(const unit u);                    /* one empty base frame (per tx) */
unit stack_enter_frame(const unit u);                    /* call: fresh empty frame       */
unit stack_leave_frame(const unit u);                    /* return: restore parent        */
uint64_t stack_depth(const unit u);                /* current frame height          */
unit stack_push_word(const lbits w);                  /* push a whole 256-bit word     */
void stack_pop_word(lbits *rop, const unit u); /* pop + return top (zero when empty) */
void stack_peek_word(lbits *rop, uint64_t n);         /* nth-from-top word (0 = top)   */
unit stack_set_word(uint64_t n, const lbits w);       /* overwrite nth-from-top (SWAP) */
#endif
