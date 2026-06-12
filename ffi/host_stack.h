/* C-backed EVM operand stack (see host_stack.c): per-call-frame flat word
 * arrays with O(1) push/pop/peek/set and zero steady-state allocation.
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits cross the FFI. */
#ifndef HOST_STACK_H
#define HOST_STACK_H
#include "sail.h"
#include <stdint.h>
unit hs_reset(const unit u);                    /* one empty base frame (per tx) */
unit hs_enter(const unit u);                    /* call: fresh empty frame       */
unit hs_leave(const unit u);                    /* return: restore parent        */
uint64_t hs_depth(const unit u);                /* current frame height          */
unit hs_push_w(const lbits w);                  /* push a whole 256-bit word     */
unit hs_pop(const unit u);                      /* drop top (read it first)      */
void hs_peek_w(lbits *rop, uint64_t n);         /* nth-from-top word (0 = top)   */
unit hs_set_w(uint64_t n, const lbits w);       /* overwrite nth-from-top (SWAP) */
#endif
