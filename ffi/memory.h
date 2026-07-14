/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Only mach_bits (uint64_t) cross the FFI, matching the other host FFI modules. */
#ifndef MEMORY_H
#define MEMORY_H
#include "sail.h"
#include <stdbool.h>
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
uint64_t slice_byte_at_source(uint64_t kind, uint64_t off, uint64_t len,
                              uint64_t i);
uint64_t slice_count_nonzero_source(uint64_t kind, uint64_t off, uint64_t len);
bool slice_strided_zero_source(uint64_t kind, uint64_t off, uint64_t len,
                               uint64_t start, uint64_t stride, uint64_t width,
                               uint64_t count);
void slice_load_word_source(lbits *rop, uint64_t kind, uint64_t off,
                            uint64_t len, uint64_t i);
void slice_load_n_word_source(lbits *rop, uint64_t kind, uint64_t off,
                              uint64_t len, uint64_t i, uint64_t n);
unit slice_copy_to_memory_source(uint64_t kind, uint64_t off, uint64_t len,
                                 uint64_t dst, uint64_t i, uint64_t n);
unit mem_move(uint64_t dst, uint64_t src, uint64_t len);  /* MCOPY memmove */
void mem_load_word(lbits *rop, uint64_t off);      /* MLOAD: 32-byte BE word  */
unit mem_store_word(uint64_t off, const lbits w);  /* MSTORE: 32-byte BE word */
#endif
