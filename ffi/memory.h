/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Only mach_bits (uint64_t) cross the FFI, matching the other host FFI modules. */
#ifndef MEMORY_H
#define MEMORY_H
#include "sail.h"
#include <stdint.h>
unit     mem_clear(const unit u);        /* clear to one empty frame (per tx)   */
unit     mem_frame_enter(const unit u);         /* enter sub-call: fresh empty frame   */
unit     mem_frame_leave(const unit u);          /* leave sub-call: restore parent      */
uint64_t mem_read_byte(uint64_t off);         /* bits(64) -> bits(8): byte at off     */
unit     mem_write_byte(uint64_t off, uint64_t v); /* (bits(64), bits(8)) -> unit     */
uint64_t hm_depth(const unit u);                    /* call-frame depth   */
const uint8_t *mem_region(uint64_t off, uint64_t len);  /* ensure + read ptr   */
uint8_t *hm_wr(uint64_t off, uint64_t len);        /* ensure + write ptr  */
uint64_t mem_establish_absolute(uint64_t off, uint64_t len);
const uint8_t *mem_arena_ptr(uint64_t abs);
const uint8_t *mem_arena_region(uint64_t off, uint64_t len);
uint64_t slice_byte_at(uint64_t kind, uint64_t off, uint64_t len, uint64_t i);
void slice_load_word(lbits *rop, uint64_t kind, uint64_t off, uint64_t len, uint64_t i);
void slice_load_n_word(lbits *rop, uint64_t kind, uint64_t off, uint64_t len,
                       uint64_t i, uint64_t n);
unit slice_copy_to_memory(uint64_t kind, uint64_t off, uint64_t len,
                          uint64_t dst, uint64_t i, uint64_t n);
unit mem_move(uint64_t dst, uint64_t src, uint64_t len);  /* MCOPY memmove */
void mem_load_word(lbits *rop, uint64_t off);      /* MLOAD: 32-byte BE word  */
unit mem_store_word(uint64_t off, const lbits w);  /* MSTORE: 32-byte BE word */
/* bind/stage the tx input in one call (no Sail byte loop):
 *   txdata_bind_source: retain a resolved byte-source reference.
 *   txdata_stage_word: stage a 32-byte big-endian word (a block system call's input).
 *   txdata_count_nonzero: number of nonzero bytes in the staged input (EIP-2028 /
 *     EIP-7623 calldata gas, counted C-side). */
uint64_t txdata_bind_source(uint64_t kind, uint64_t off, uint64_t len);
uint64_t txdata_stage_word(const lbits w);
uint64_t txdata_count_nonzero(const unit u);
uint64_t txd_copy(uint8_t *dst, uint64_t cap);
uint64_t txdata_byte_at(uint64_t i);
uint64_t txdata_length(const unit u);
const uint8_t *txd_rd(uint64_t off, uint64_t len);
#endif
