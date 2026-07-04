/* C-backed code_db + per-frame code descriptors (see code_db.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef CODE_DB_H
#define CODE_DB_H
#include "sail.h"
#include <stdbool.h>
unit code_db_reset(const unit u);

/* Streaming FFI bridge for Sail's code_db_store_code(hash, list(byte)) helper. */
uint64_t code_db_stream_code_begin(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
unit code_db_stream_code_byte(uint64_t b);
unit code_db_store_ssz_code(uint64_t off, uint64_t len);
uint64_t code_db_stored_code_length(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
unit code_db_copy_stored_code_to_memory(
    uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
    uint64_t dst, uint64_t off, uint64_t len);
void code_db_read_delegation(lbits *rop, uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);

/* Per-frame executable code views. */
uint64_t code_db_frame_set_stored_code(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);
uint64_t code_db_prepare_child_frame_memory_code(uint64_t off, uint64_t len);
uint64_t code_db_frame_set_tx_input_code(const unit u);
unit code_db_frame_clear_code(const unit u);
uint64_t code_db_frame_code_byte(uint64_t i);
uint64_t code_db_frame_code_length(const unit u);
bool code_db_frame_jumpdest_valid(uint64_t i);
unit code_db_copy_frame_code_to_memory(uint64_t dst, uint64_t off, uint64_t len);
void code_db_frame_push_immediate_word(lbits *rop, uint64_t i, uint64_t n);
void code_db_calldata_load_word(lbits *rop, uint64_t i);
#endif
