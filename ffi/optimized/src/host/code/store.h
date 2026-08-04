/* C-backed code_db plus optimized-private byte-addressed JUMPDEST storage
 * (see code/store.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_STORE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_STORE_H
#include "evmsail/prelude.h"
#include <stdbool.h>

struct StatelessInputSliceFields;
struct EvmMemorySliceFields;
struct OutputSliceFields;
struct CodeRegionSliceFields;
struct CodeFields;
struct zoptionzIRCodeFieldszK code_db_lookup(Hash32 h);
struct CodeRegionSliceFields code_region_from_input(
    struct StatelessInputSliceFields input);
struct CodeRegionSliceFields code_region_from_memory(
    struct EvmMemorySliceFields input);
struct CodeRegionSliceFields code_region_from_output(
    struct OutputSliceFields input);
struct CodeRegionSliceFields code_region_from_delegation(Address address);
uint8_t *jumpdest_table_alloc(struct CodeRegionSliceFields code);
bool jumpdest_table_mark(uint8_t *table, uint32_t code_len,
                         uint32_t position);

uint8_t *code_region_intern_copy(const uint8_t *src, uint64_t len);
Hash32 code_db_store_indexed(struct CodeFields code);
bool code_db_lookup_view(Hash32 h, const uint8_t **bytes, uint64_t *len,
                         uint8_t **jumpdests);
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later);
uint8_t *code_db_analyze_region(struct CodeRegionSliceFields code,
                                bool amsterdam_or_later);
struct AddressResult code_db_read_delegation(
     Hash32 h);
bool code_db_read_delegation_address(
    Address *address,
    Hash32 h);

bool jumpdest_ref_contains(uint8_t *table, uint32_t code_len,
                           uint32_t i);
#endif
