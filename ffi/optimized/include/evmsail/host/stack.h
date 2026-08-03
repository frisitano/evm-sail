/* C-backed EVM operand stack (see stack.c): per-call-frame flat word
 * arrays with O(1) push/pop/peek/set and zero steady-state allocation.
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits cross the FFI. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STACK_H
#define EVMSAIL_OPTIMIZED_HOST_STACK_H
#include "evmsail/prelude.h"
#include <stdint.h>
unit stack_reset(const unit u);                    /* one empty base frame (per tx) */
unit stack_enter_frame(const unit u);                    /* call: fresh empty frame       */
unit stack_leave_frame(const unit u);                    /* return: restore parent        */
uint16_t stack_depth(const unit u);                /* current frame height          */
unit stack_push_word(const U256 w);           /* push a whole 256-bit word     */
U256 stack_pop_word( const unit u);
U256 stack_peek_word(uint16_t n);
unit stack_set_word(uint16_t n, U256 w); /* overwrite nth-from-top (SWAP) */
#endif
