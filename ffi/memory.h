/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Only mach_bits (uint64_t) cross the FFI, matching the other host FFI modules. */
#ifndef MEMORY_H
#define MEMORY_H
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
unit txin_begin(const unit u);
unit txin_byte(uint64_t b);
/* stage the whole tx input in one call (no Sail byte loop):
 *   txin_set_from_source: copy a resolved byte source (the witness span for a
 *     stateless tx; a self-reference for the already-staged native-runner input)
 *     into the tx-input buffer; returns the staged length (0 on a bad source).
 *   txin_set_word: stage a 32-byte big-endian word (a block system call's input).
 *   txd_count_nonzero: number of nonzero bytes in the staged input (EIP-2028 /
 *     EIP-7623 calldata gas, counted C-side). */
uint64_t txin_set_from_source(uint64_t kind, uint64_t off, uint64_t len);
uint64_t txin_set_word(const lbits w);
uint64_t txd_count_nonzero(const unit u);
uint64_t txd_copy(uint8_t *dst, uint64_t cap);
uint64_t txd_at(uint64_t i);
uint64_t txd_length(const unit u);
const uint8_t *txd_rd(uint64_t off, uint64_t len);
uint64_t cd_byte(uint64_t i);
unit cd_to_mem(uint64_t dst, uint64_t off, uint64_t len);
#endif
