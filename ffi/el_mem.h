/* C-backed EVM memory for the EL-IR Sail model (see el_mem.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Only mach_bits (uint64_t) cross the FFI, matching ffi/acc_shim.h. */
#ifndef EL_MEM_H
#define EL_MEM_H
#include "sail.h"
#include <stdint.h>
unit     el_mem_reset(const unit u);        /* clear to one empty frame (per tx)   */
unit     el_mem_push(const unit u);         /* enter sub-call: fresh empty frame   */
unit     el_mem_pop(const unit u);          /* leave sub-call: restore parent      */
uint64_t el_mem_read(uint64_t off);         /* bits(64) -> bits(8): byte at off     */
unit     el_mem_write(uint64_t off, uint64_t v); /* (bits(64), bits(8)) -> unit     */
#endif
