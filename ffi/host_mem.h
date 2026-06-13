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
uint64_t hm_depth(const unit u);                    /* call-frame depth   */
const uint8_t *hm_rd(uint64_t off, uint64_t len);  /* ensure + read ptr   */
uint8_t *hm_wr(uint64_t off, uint64_t len);        /* ensure + write ptr  */
unit hm_move(uint64_t dst, uint64_t src, uint64_t len);  /* MCOPY memmove */
unit cd_set(uint64_t off, uint64_t len);   /* next child's calldata = mem range */
unit cd_set_empty(const unit u);
unit cd_set_tx(const unit u);
unit txd_begin(const unit u);
unit txd_byte(uint64_t b);
uint64_t cd_len(const unit u);
uint64_t cd_byte(uint64_t i);
unit cd_to_mem(uint64_t dst, uint64_t off, uint64_t len);
#endif
