/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`. */
#ifndef MEMORY_H
#define MEMORY_H
#include "quantity_abi.h"
#include "sail_abi.h"
#include <stdbool.h>
#include <stdint.h>
bool evm_memory_configure_capacity(uint64_t capacity);
unit     mem_clear(const unit u);        /* clear to one empty frame (per tx)   */
#ifdef EVMSAIL_STANDARD_ABI
void mem_frame_enter(sail_int *out, const unit u);
#else
uint64_t mem_frame_enter(const unit u);         /* enter frame; return absolute base   */
#endif
unit     mem_frame_leave(const unit u);          /* leave sub-call: restore parent      */
uint64_t evm_memory_expand(uint64_t len);
uint64_t mem_read_byte(EVMSAIL_BYTE_QUANTITY_PARAM(off));
unit mem_write_byte(EVMSAIL_BYTE_QUANTITY_PARAM(off), uint64_t v);
uint64_t hm_depth(const unit u);                    /* call-frame depth   */
const uint8_t *evm_memory_region(uint64_t off, uint64_t len);
const uint8_t *evm_memory_base(void);
uint64_t evm_memory_capacity(void);
uint8_t *evm_memory_write_region(uint64_t off, uint64_t len);
unit mem_move(EVMSAIL_BYTE_QUANTITY_PARAM(dst),
              EVMSAIL_BYTE_QUANTITY_PARAM(src),
              EVMSAIL_BYTE_QUANTITY_PARAM(len));
EVMSAIL_WORD_RETURN mem_load_word(EVMSAIL_WORD_RESULT(result)
                                  EVMSAIL_BYTE_QUANTITY_PARAM(off));
unit mem_store_word(EVMSAIL_BYTE_QUANTITY_PARAM(off), EVMSAIL_WORD_PARAM(w));
#endif
