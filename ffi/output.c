#include "output.h"

#include "byte_slice_glue.h"
#include "lbits_convert.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t *bytes;
  uint64_t capacity;
  uint64_t length;
} OutputBuffer;

static OutputBuffer buffer;

static bool reserve(uint64_t need) {
  if (need > SIZE_MAX) return false;
  if (buffer.capacity >= need && buffer.bytes) return true;
  uint64_t capacity = buffer.capacity ? buffer.capacity : 256;
  while (capacity < need) {
    if (capacity > UINT64_MAX / 2) {
      capacity = need;
      break;
    }
    capacity *= 2;
  }
  uint8_t *bytes = (uint8_t *)realloc(buffer.bytes, (size_t)capacity);
  if (!bytes) return false;
  buffer.bytes = bytes;
  buffer.capacity = capacity;
  return true;
}

bool output_buffer_store_source(uint64_t kind, uint64_t off, uint64_t len) {
  const uint8_t *source = NULL;
  uint64_t resolved_len = 0;
  if (!evmsail_resolve_byte_source(kind, off, len, &source, &resolved_len) ||
      resolved_len != len || (len && !source) || (len && !reserve(len))) {
    buffer.length = 0;
    return false;
  }
  if (len) memmove(buffer.bytes, source, (size_t)len);
  buffer.length = len;
  return true;
}

bool output_buffer_store_word(const sail_word word) {
  if (!reserve(32)) {
    buffer.length = 0;
    return false;
  }
  sail_word_to_be_bytes(buffer.bytes, word);
  buffer.length = 32;
  return true;
}

bool output_buffer_store_words(const sail_word first, const sail_word second) {
  if (!reserve(64)) {
    buffer.length = 0;
    return false;
  }
  sail_word_to_be_bytes(buffer.bytes, first);
  sail_word_to_be_bytes(buffer.bytes + 32, second);
  buffer.length = 64;
  return true;
}

int output_buffer_span(const uint8_t **bytes, uint64_t *len) {
  if (bytes) *bytes = buffer.bytes;
  if (len) *len = buffer.length;
  return 1;
}

uint8_t *output_buffer_reserve(uint64_t capacity) {
  if (!reserve(capacity ? capacity : 1)) {
    buffer.length = 0;
    return NULL;
  }
  buffer.length = 0;
  return buffer.bytes;
}

bool output_buffer_finish(uint64_t len) {
  if (len > buffer.capacity || (len && !buffer.bytes)) {
    buffer.length = 0;
    return false;
  }
  buffer.length = len;
  return true;
}
