/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`. */
#ifndef MEMORY_H
#define MEMORY_H
#include "quantity_abi.h"
#include "sail.h"
#include <stdbool.h>
#include <stdint.h>
unit     mem_clear(const unit u);        /* clear to one empty frame (per tx)   */
#ifdef EVMSAIL_STANDARD_ABI
void mem_frame_enter(sail_int *out, const unit u);
#else
uint64_t mem_frame_enter(const unit u);         /* enter frame; return absolute base   */
#endif
unit     mem_frame_leave(const unit u);          /* leave sub-call: restore parent      */
bool mem_expand(EVMSAIL_BYTE_QUANTITY_PARAM(len));
uint64_t mem_read_byte(EVMSAIL_BYTE_QUANTITY_PARAM(off));
unit mem_write_byte(EVMSAIL_BYTE_QUANTITY_PARAM(off), uint64_t v);
uint64_t hm_depth(const unit u);                    /* call-frame depth   */
const uint8_t *evm_memory_region(uint64_t off, uint64_t len);
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
unit mem_move(EVMSAIL_BYTE_QUANTITY_PARAM(dst),
              EVMSAIL_BYTE_QUANTITY_PARAM(src),
              EVMSAIL_BYTE_QUANTITY_PARAM(len));
void mem_load_word(lbits *rop, EVMSAIL_BYTE_QUANTITY_PARAM(off));
unit mem_store_word(EVMSAIL_BYTE_QUANTITY_PARAM(off), const lbits w);
#endif
