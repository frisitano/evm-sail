/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`. */
#ifndef EVMSAIL_OPTIMIZED_HOST_MEMORY_H
#define EVMSAIL_OPTIMIZED_HOST_MEMORY_H
#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>
unit     mem_clear(const unit u);        /* clear to one empty frame (per tx)   */
uint64_t mem_frame_enter(const unit u);         /* enter frame; return absolute base   */
unit mem_frame_leave(const unit u); /* leave sub-call: restore parent */
uint64_t evm_memory_expand(uint64_t len);
uint64_t mem_read_byte(uint64_t off);
unit mem_write_byte(uint64_t off, uint64_t v);
uint64_t hm_depth(const unit u); /* call-frame depth */
const uint8_t *evm_memory_region(uint64_t off, uint64_t len);
const uint8_t *evm_memory_base(void);
uint64_t evm_memory_capacity(void);
uint8_t *evm_memory_write_region(uint64_t off, uint64_t len);
unit mem_move(uint64_t dst, uint64_t src, uint64_t len);
U256 mem_load_word(uint64_t off);
unit mem_store_word(uint64_t off, const U256 w);
#endif
