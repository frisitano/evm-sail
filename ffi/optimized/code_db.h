/* C-backed code_db + packed storage for Sail-built JUMPDEST bitmaps
 * (see code_db.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef CODE_DB_H
#define CODE_DB_H
#include "quantity_abi.h"
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdbool.h>

bool code_db_configure_capacities(uint64_t code_bytes,
                                  uint64_t jumpdest_words);
struct zoptionzIRCodezK;
struct zStatelessInputSliceFields;
struct zMemorySliceFields;
struct zOutputSliceFields;
struct zCodeRegionSliceFields;
void code_db_lookup(struct zoptionzIRCodezK *rop, sail_fixed_bytes_32 h);
struct zCodeRegionSliceFields code_region_from_input(
    struct zStatelessInputSliceFields input);
struct zCodeRegionSliceFields code_region_from_memory(
    struct zMemorySliceFields input);
struct zCodeRegionSliceFields code_region_from_output(
    struct zOutputSliceFields input);
uint64_t jumpdest_table_alloc(EVMSAIL_BYTE_QUANTITY_PARAM(code_len));
bool jumpdest_table_store_chunk(uint64_t ref,
                                EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                                EVMSAIL_BYTE_QUANTITY_PARAM(chunk_index),
                                const sail_u256 chunk);
sail_fixed_bytes_32 code_db_store_indexed_bytes(
    const uint8_t *src, uint64_t len, uint64_t jumpdest_ref);
sail_fixed_bytes_32 code_intern_indexed_delegation(
    sail_fixed_bytes_20 addr, uint64_t jumpdest_ref);
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len);
bool code_db_owned_contains(uint64_t pointer, uint64_t len);
bool code_db_lookup_indexed(sail_fixed_bytes_32 h, uint64_t *off, uint64_t *len,
                            uint64_t *jumpdest_ref);
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later);
bool code_db_append_region(const uint8_t *src, uint64_t len, uint64_t *off);
struct zAddressResult code_db_read_delegation(
     sail_fixed_bytes_32 h);
bool code_db_read_delegation_address(uint8_t address[20], sail_fixed_bytes_32 h);

/* JumpdestRef is an opaque table handle. Standard/measurement builds use a
 * one-based arena index; fixed optimized builds use the table's address. */
bool jumpdest_ref_contains(uint64_t ref,
                           EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                           EVMSAIL_BYTE_QUANTITY_PARAM(i));
#endif
