#include "evmsail/host/output.h"

#include "evmsail/prelude.h"
#include "evmsail/host/types.h"
#include "primitives/value.h"
#include "workspace.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  uint8_t bytes[GUEST_OUTPUT_BYTES];
  uint64_t length;
} OutputBuffer;

static OutputBuffer *buffer;

void output_workspace_bind(void)
{
  WORKSPACE_BIND(buffer, 1);
}

/* A nonempty slice always carries a resolved source pointer. */
bool output_buffer_store_bytes(const uint8_t *source, uint64_t len)
{
  if (len > GUEST_OUTPUT_BYTES) {
    buffer->length = 0;
    return false;
  }
  if (len) {
    memmove(buffer->bytes, source, (size_t)len);
  }
  buffer->length = len;
  return true;
}

bool output_buffer_store_word(const u256 word)
{
  sail_word_to_be_bytes(buffer->bytes, word);
  buffer->length = 32;
  return true;
}

bool output_buffer_store_words(const u256 first, const u256 second)
{
  sail_word_to_be_bytes(buffer->bytes, first);
  sail_word_to_be_bytes(buffer->bytes + 32, second);
  buffer->length = 64;
  return true;
}

void output_buffer_span(const uint8_t **bytes, uint64_t *len)
{
  if (bytes) {
    *bytes = buffer->bytes;
  }
  if (len) {
    *len = buffer->length;
  }
}

uint8_t *output_buffer_reserve(uint64_t capacity)
{
  if (capacity > GUEST_OUTPUT_BYTES) {
    buffer->length = 0;
    return NULL;
  }
  buffer->length = 0;
  return buffer->bytes;
}

bool output_buffer_finish(uint64_t len)
{
  if (len > GUEST_OUTPUT_BYTES) {
    buffer->length = 0;
    return false;
  }
  buffer->length = len;
  return true;
}
/* Sail optimized_output_buffer_slice is called only with the length of the
 * buffer contents just stored (sail/optimised/host/output.sail). */
Bytes output_buffer_slice(uint32_t len)
{
  Bytes out;
  out.bytes = len == 0 ? NULL : buffer->bytes;
  out.len = len;
  return out;
}
