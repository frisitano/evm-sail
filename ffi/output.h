#ifndef EVMSAIL_OUTPUT_H
#define EVMSAIL_OUTPUT_H

#include "sail.h"

#include <stdbool.h>
#include <stdint.h>

/* Scratch storage for the one live frame or precompile output. Sail owns the
 * ByteSlice registers and all frame transitions. */
bool output_buffer_store_source(uint64_t kind, uint64_t off, uint64_t len);
bool output_buffer_store_word(const lbits word);
bool output_buffer_store_words(const lbits first, const lbits second);

int output_buffer_span(const uint8_t **bytes, uint64_t *len);
uint8_t *output_buffer_reserve(uint64_t capacity);
bool output_buffer_finish(uint64_t len);

#endif
