/* C-backed code_db + packed storage for Sail-built JUMPDEST bitmaps
 * (see code/store.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_STORE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_STORE_H
#include "evmsail/prelude.h"
#include <stdbool.h>

struct zoptionzIRCodezK;
struct zStatelessInputSliceFields;
struct zEvmMemorySliceFields;
struct zOutputSliceFields;
struct zCodeRegionSliceFields;
void code_db_lookup(struct zoptionzIRCodezK *rop, Hash32 h);
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

/* A code-region reference is an opaque one-based CodeId naming a table row
 * whose byte pointer was resolved when the row was created; zero is the
 * empty region. The same value is the CodeRegionSlice offset seen by Sail. */
uint64_t code_region_register(const uint8_t *bytes, uint64_t len);
uint64_t code_region_intern_copy(const uint8_t *src, uint64_t len);
Hash32 code_db_store_row(uint64_t region, uint64_t jumpdest_ref);
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len);
bool code_db_lookup_indexed(Hash32 h, uint64_t *off, uint64_t *len,
                            uint64_t *jumpdest_ref);
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later);
uint64_t code_db_analyze_bytes(const uint8_t *src, uint64_t len,
                               bool amsterdam_or_later);
Hash32 code_intern_indexed_delegation(
    Address addr, uint64_t jumpdest_ref);
struct zAddressResult code_db_read_delegation(
     Hash32 h);
bool code_db_read_delegation_address(
    Address *address,
    Hash32 h);

/* JumpdestRef is an opaque table handle: the one-based JumpdestId of the
 * bitmap row. Zero is the empty analysis. */
bool jumpdest_ref_contains(uint64_t ref, uint64_t code_len, uint64_t i);
#endif
