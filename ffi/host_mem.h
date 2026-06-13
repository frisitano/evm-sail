/* C-backed EVM memory for the EL-IR Sail model (see host_mem.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Only mach_bits (uint64_t) cross the FFI, matching ffi/acc_shim.h. */
#ifndef HOST_MEM_H
#define HOST_MEM_H
#include "sail.h"
#include <stdint.h>
unit     host_mem_reset(const unit u);        /* clear to one empty frame (per tx)   */
unit     host_mem_push(const unit u);         /* enter sub-call: fresh empty frame   */
unit     host_mem_pop(const unit u);          /* leave sub-call: restore parent      */
uint64_t host_mem_read(uint64_t off);         /* bits(64) -> bits(8): byte at off     */
unit     host_mem_write(uint64_t off, uint64_t v); /* (bits(64), bits(8)) -> unit     */
const uint8_t *hm_rd(uint64_t off, uint64_t len);  /* ensure + read ptr   */
uint8_t *hm_wr(uint64_t off, uint64_t len);        /* ensure + write ptr  */
unit hm_move(uint64_t dst, uint64_t src, uint64_t len);  /* MCOPY memmove */
#endif
