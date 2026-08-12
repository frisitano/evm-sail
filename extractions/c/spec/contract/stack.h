/* C-backed EVM operand stack (see stack.c): per-call-frame flat word
 * arrays addressed through the threaded StackTop cursor, with zero
 * steady-state allocation. Declared here so the Sail-generated C call
 * sites are prototyped via `sail -c --c-include`. Only mach_bits cross
 * the FFI; the spec ABI represents the abstract cursor as the frame
 * height. */
#ifndef STACK_H
#define STACK_H
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdint.h>
uint64_t stack_reset(const unit u); /* one empty base frame (per tx); returns its cursor */
uint64_t operand_stack_push_empty_frame(const unit u); /* call: fresh empty frame; returns its cursor */
unit operand_stack_pop_frame(const unit u);            /* return: release the child's storage */
uint64_t stack_top_height(uint64_t top);               /* words below a cursor */
u256 stack_slot_read(uint64_t top, uint64_t index);    /* index-from-top read */
unit stack_slot_write(uint64_t top, uint64_t index, const u256 w); /* index-from-top write */
unit stack_slot_write_next(uint64_t top, const u256 w); /* next free slot */
uint64_t stack_top_advance(uint64_t top, uint64_t count); /* cursor over pushed slots */
uint64_t stack_top_retreat(uint64_t top, uint64_t count); /* cursor over popped slots */
#endif
