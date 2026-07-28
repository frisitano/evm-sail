#ifndef EVMSAIL_OUTPUT_H
#define EVMSAIL_OUTPUT_H

#include "sail_abi.h"

#include <stdbool.h>
#include <stdint.h>

bool output_buffer_configure_capacity(uint64_t capacity);
/* Scratch storage for the one live frame or precompile output. */
bool output_buffer_store_bytes(const uint8_t *source, uint64_t len);
bool output_buffer_store_word(EVMSAIL_WORD_PARAM(word));
bool output_buffer_store_words(EVMSAIL_WORD_PARAM(first),
                               EVMSAIL_WORD_PARAM(second));

int output_buffer_span(const uint8_t **bytes, uint64_t *len);
uint8_t *output_buffer_reserve(uint64_t capacity);
bool output_buffer_finish(uint64_t len);

#endif
