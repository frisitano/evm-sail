/* C-backed EVM memory for the evm-sail model (see memory.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`. */
#ifndef EVMSAIL_OPTIMIZED_HOST_MEMORY_H
#define EVMSAIL_OPTIMIZED_HOST_MEMORY_H
#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>
uint64_t evm_memory_expand(uint64_t pointer, uint64_t established, uint64_t required);
uint64_t mem_read_byte(uint64_t off);
const uint8_t *evm_memory_base(void);
uint64_t evm_memory_capacity(void);
uint8_t *evm_memory_write_region(uint64_t off, uint64_t len);
#endif
