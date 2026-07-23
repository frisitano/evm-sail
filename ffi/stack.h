/* C-backed EVM operand stack (see stack.c): per-call-frame flat word
 * arrays with O(1) push/pop/peek/set and zero steady-state allocation.
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits cross the FFI. */
#ifndef STACK_H
#define STACK_H
#include "sail_abi.h"
#include <stdint.h>
unit stack_reset(const unit u);                    /* one empty base frame (per tx) */
unit stack_enter_frame(const unit u);                    /* call: fresh empty frame       */
unit stack_leave_frame(const unit u);                    /* return: restore parent        */
uint64_t stack_depth(const unit u);                /* current frame height          */
unit stack_push_word(EVMSAIL_WORD_PARAM(w));           /* push a whole 256-bit word     */
EVMSAIL_WORD_RETURN stack_pop_word(EVMSAIL_WORD_RESULT(result) const unit u);
EVMSAIL_WORD_RETURN stack_peek_word(EVMSAIL_WORD_RESULT(result) uint64_t n);
unit stack_set_word(uint64_t n, EVMSAIL_WORD_PARAM(w)); /* overwrite nth-from-top (SWAP) */
#endif
