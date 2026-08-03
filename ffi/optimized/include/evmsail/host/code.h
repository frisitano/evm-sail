#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/* Public C implementations of the contracts declared by sail/host/code.sail. */
void code_db_lookup(struct zoptionzIRCodezK *result,
                    Hash32 hash);
struct zCodeRegionSliceFields code_region_from_input(
    struct zStatelessInputSliceFields input);
struct zCodeRegionSliceFields code_region_from_memory(
    struct zEvmMemorySliceFields input);
struct zCodeRegionSliceFields code_region_from_output(
    struct zOutputSliceFields input);
uint64_t jumpdest_table_alloc(uint64_t code_len);
bool jumpdest_table_store_chunk(uint64_t ref, uint64_t code_len,
                                uint64_t chunk_index,
                                const U256 chunk);
Hash32 code_db_store_indexed(
    struct zCodeRegionSliceFields code, uint64_t jumpdest_ref);
bool jumpdest_ref_contains(uint64_t ref, uint64_t code_len, uint64_t index);
Hash32 code_intern_indexed_delegation(
    Address address, uint64_t jumpdest_ref);
struct zAddressResult code_db_read_delegation(
    Hash32 hash);

/* Optimized override owned by sail/optimised/host/code.sail. */
uint64_t code_db_analyze_indexed(struct zCodeRegionSliceFields code,
                                 bool amsterdam_or_later);

#endif
