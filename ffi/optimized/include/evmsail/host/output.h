#ifndef EVMSAIL_OPTIMIZED_HOST_OUTPUT_H
#define EVMSAIL_OPTIMIZED_HOST_OUTPUT_H

#include "evmsail/prelude.h"

struct zOutputSliceFields output_buffer_slice(uint64_t len);

#include <stdbool.h>
#include <stdint.h>

/* Scratch storage for the one live frame or precompile output. */
bool output_buffer_store_bytes(const uint8_t *source, uint64_t len);
bool output_buffer_store_word(const U256 word);
bool output_buffer_store_words(const U256 first,
                               const U256 second);

int output_buffer_span(const uint8_t **bytes, uint64_t *len);
uint8_t *output_buffer_reserve(uint64_t capacity);
bool output_buffer_finish(uint64_t len);

#endif
