/* C-backed code_db + packed storage for Sail-built JUMPDEST bitmaps
 * (see code_db.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef CODE_DB_H
#define CODE_DB_H
#include "quantity_abi.h"
#include "sail.h"
#include <stdbool.h>

struct zoptionzIRCodezK;
void code_db_lookup(struct zoptionzIRCodezK *rop, const lbits h);
uint64_t jumpdest_table_alloc(EVMSAIL_BYTE_QUANTITY_PARAM(code_len));
bool jumpdest_table_store_chunk(uint64_t ref,
                                EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                                EVMSAIL_BYTE_QUANTITY_PARAM(chunk_index),
                                const lbits chunk);
void code_db_store_indexed_source(lbits *rop, uint64_t source_kind,
                                  uint64_t off, uint64_t len,
                                  uint64_t jumpdest_ref);
void code_intern_indexed_delegation(lbits *rop, const lbits addr,
                                    uint64_t jumpdest_ref);
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len);
bool code_db_lookup_indexed(const lbits h, uint64_t *off, uint64_t *len,
                            uint64_t *jumpdest_ref);
const uint8_t *code_db_code_by_words(const uint64_t key_be[4], uint64_t *len_out);
void code_db_read_delegation(lbits *rop, const lbits h);

/* A JumpdestRef is an opaque one-based handle into the flat bitmap arena.
 * `code_len` supplies the bound associated with the same code-hash entry. */
bool jumpdest_ref_contains(uint64_t ref,
                           EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                           EVMSAIL_BYTE_QUANTITY_PARAM(i));
#endif
