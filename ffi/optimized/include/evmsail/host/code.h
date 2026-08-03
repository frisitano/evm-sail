#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/* Public C implementations of the contracts declared by sail/host/code.sail. */
struct zoptionzIRCodezK code_db_lookup(Hash32 hash);
struct CodeRegionSliceFields code_region_from_input(
    struct StatelessInputSliceFields input);
struct CodeRegionSliceFields code_region_from_memory(
    struct EvmMemorySliceFields input);
struct CodeRegionSliceFields code_region_from_output(
    struct OutputSliceFields input);
uint64_t jumpdest_table_alloc(uint32_t code_len);
bool jumpdest_table_store_chunk(uint64_t ref, uint32_t code_len,
                                uint32_t chunk_index, U256 chunk);
Hash32 code_db_store_indexed(
    struct CodeRegionSliceFields code, uint64_t jumpdest_ref);
bool jumpdest_ref_contains(uint64_t ref, uint32_t code_len, uint32_t index);
Hash32 code_intern_indexed_delegation(
    Address address, uint64_t jumpdest_ref);
struct AddressResult code_db_read_delegation(
    Hash32 hash);

/* Optimized override owned by sail/optimised/host/code.sail. */
uint64_t code_db_analyze_indexed(struct CodeRegionSliceFields code,
                                 bool amsterdam_or_later);

#endif
