/* C-backed code_db plus optimized-private byte-addressed JUMPDEST storage
 * (see code/store.c). Sail-facing declarations live in evmsail/host/code.h;
 * only the optimized-private store operations are declared here. */
#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_STORE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_STORE_H
#include "evmsail/prelude.h"
#include "evmsail/host/code.h"
#include <stdbool.h>

struct StatelessInputSliceFields;
struct EvmMemorySliceFields;
struct OutputSliceFields;
struct CodeRegionSliceFields;
struct CodeFields;
bool jumpdest_table_mark(uint8_t *table, uint32_t code_len, uint32_t position);

uint8_t *code_region_intern_copy(const uint8_t *src, uint32_t len);
bool code_db_lookup_view(Hash32 h, const uint8_t **bytes, uint32_t *len, uint8_t **jumpdests);
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len, bool amsterdam_or_later);
#endif
