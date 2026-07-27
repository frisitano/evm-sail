/* C-backed code_db + packed storage for Sail-built JUMPDEST bitmaps
 * (see code_db.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef CODE_DB_H
#define CODE_DB_H
#include "quantity_abi.h"
#include "sail_abi.h"
#include <stdbool.h>

struct zoptionzIRCodezK;
void code_db_lookup(struct zoptionzIRCodezK *rop, sail_hash h);
uint64_t jumpdest_table_alloc(EVMSAIL_BYTE_QUANTITY_PARAM(code_len));
bool jumpdest_table_store_chunk(uint64_t ref,
                                EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                                EVMSAIL_BYTE_QUANTITY_PARAM(chunk_index),
                                const sail_bits256 chunk);
EVMSAIL_HASH_RETURN code_db_store_indexed_source(
    EVMSAIL_HASH_RESULT(result) uint64_t source_kind, uint64_t off,
    uint64_t len, uint64_t jumpdest_ref);
EVMSAIL_HASH_RETURN code_intern_indexed_delegation(
    EVMSAIL_HASH_RESULT(result) sail_address addr, uint64_t jumpdest_ref);
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len);
bool code_db_lookup_indexed(sail_hash h, uint64_t *off, uint64_t *len,
                            uint64_t *jumpdest_ref);
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later);
EVMSAIL_ADDRESS_RESULT_RECORD_RETURN code_db_read_delegation(
    EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(result) sail_hash h);
bool code_db_read_delegation_address(uint8_t address[20], sail_hash h);

/* A JumpdestRef is an opaque one-based handle into the flat bitmap arena.
 * `code_len` supplies the bound associated with the same code-hash entry. */
bool jumpdest_ref_contains(uint64_t ref,
                           EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                           EVMSAIL_BYTE_QUANTITY_PARAM(i));
#endif
