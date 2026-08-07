/* C-backed EVM operand stack (see stack.c): per-call-frame flat word
 * arrays with O(1) push/pop/peek/set and zero steady-state allocation.
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Only mach_bits cross the FFI. */
#ifndef STACK_H
#define STACK_H
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdint.h>
unit stack_reset(const unit u); /* one empty base frame (per tx) */
unit operand_stack_push_empty_frame(const unit u); /* call: fresh empty frame */
unit operand_stack_pop_frame(const unit u);        /* return: restore parent */
uint64_t stack_depth(const unit u);                 /* current frame height */
unit stack_push_word(const u256 w);           /* push a whole 256-bit word     */
u256 stack_pop_word( const unit u);
u256 stack_peek_word( uint64_t n);
unit stack_set_word(uint64_t n, const u256 w); /* overwrite nth-from-top (SWAP) */
#endif
