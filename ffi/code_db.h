/* C-backed code_db + packed storage for Sail-built JUMPDEST bitmaps
 * (see code_db.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef CODE_DB_H
#define CODE_DB_H
#include "sail.h"
#include <stdbool.h>

/* JumpdestBitmap is list(bits(64)). This header precedes the generated list
 * definition in the model translation unit; code_glue.c compiles against the
 * completed generated header and converts the list exactly once. */
struct node_zz5listz8z5bvz9;
void code_db_store_indexed_source(lbits *rop, uint64_t source_kind,
                                  uint64_t off, uint64_t len,
                                  struct node_zz5listz8z5bvz9 *jumpdests);
void code_intern_indexed_delegation(
    lbits *rop, const lbits addr,
    struct node_zz5listz8z5bvz9 *jumpdests);

/* Scalar core called by code_glue.c after decoding the generated list. */
void code_db_store_indexed_words(lbits *rop, uint64_t source_kind,
                                 uint64_t off, uint64_t len,
                                 const uint64_t *jumpdests, uint64_t nwords);
void code_intern_indexed_delegation_words(lbits *rop, const lbits addr,
                                          const uint64_t *jumpdests,
                                          uint64_t nwords);
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len);
uint64_t code_db_stored_code_length(const lbits h);
uint64_t code_db_stored_code_offset(const lbits h);
uint64_t code_db_stored_jumpdest_ref(const lbits h);
uint64_t code_db_byte_at(uint64_t off);
const uint8_t *code_db_code_by_words(const uint64_t key_be[4], uint64_t *len_out);
unit code_db_copy_stored_code_to_memory(const lbits h, uint64_t dst, uint64_t off, uint64_t len);
void code_db_read_delegation(lbits *rop, const lbits h);

/* A JumpdestRef is a one-based word offset into the flat bitmap arena.
 * `code_len` supplies the bound associated with the same code-hash entry. */
bool jumpdest_ref_contains(uint64_t ref, uint64_t code_len, uint64_t i);
#endif
