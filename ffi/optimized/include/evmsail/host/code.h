#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/* Public C implementations of the contracts declared by sail/host/code.sail. */
struct zoptionzIRCodeFieldszK code_db_lookup(Hash32 hash);
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
Hash32 code_db_store_indexed(struct CodeFields code);
bool jumpdest_ref_contains(uint8_t *table, uint32_t code_len,
                           uint32_t index);
struct AddressResult code_db_read_delegation(
    Hash32 hash);

/* Optimized override owned by sail/optimised/host/code.sail. */
uint8_t *code_db_analyze_indexed(struct CodeRegionSliceFields code,
                                 bool amsterdam_or_later);

#endif
