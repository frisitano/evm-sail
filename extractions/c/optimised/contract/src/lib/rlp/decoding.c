/* Canonical RLP item decoding shared by optimized codecs.
 *
 * One in-place decoder over immutable input bytes, mirroring
 * `sail/lib/rlp/decoding.sail`. Callers layer their own error policy over the
 * boolean core. */
#include "evmsail/prelude.h"

#include "lib/rlp/decoding.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* `width` is 1..8 at both call sites (header byte ranges). */
static bool rlp_parse_be_length(const uint8_t *source, uint32_t available, uint8_t width,
                                uint64_t *length, bool *minimal)
{
  if (available < width) {
    return false;
  }
  uint64_t value = 0;
  for (uint8_t i = 0; i < width; ++i) {
    value = (value << 8) | source[i];
  }
  *length = value;
  *minimal = source[0] != 0;
  return true;
}

bool rlp_parse_item(const uint8_t *source, uint32_t available, rlp_item *item)
{
  if (available == 0) {
    return false;
  }

  const uint8_t header = source[0];
  uint32_t content_offset = 0;
  uint64_t content_len = 0;
  bool is_list = false;
  bool canonical = true;

  if (header < 0x80) {
    content_len = 1;
  } else if (header <= 0xb7) {
    content_offset = 1;
    content_len = (uint64_t)header - 0x80;
  } else if (header <= 0xbf) {
    const uint8_t width = (uint8_t)(header - 0xb7);
    bool minimal = false;
    content_offset = 1 + width;
    if (!rlp_parse_be_length(source + 1, available - 1, width, &content_len, &minimal)) {
      return false;
    }
    canonical = ((minimal && content_len > 55) != 0);
  } else if (header <= 0xf7) {
    is_list = true;
    content_offset = 1;
    content_len = (uint64_t)header - 0xc0;
  } else {
    const uint8_t width = (uint8_t)(header - 0xf7);
    bool minimal = false;
    is_list = true;
    content_offset = 1 + width;
    if (!rlp_parse_be_length(source + 1, available - 1, width, &content_len, &minimal)) {
      return false;
    }
    canonical = ((minimal && content_len > 55) != 0);
  }

  if (content_offset > available || content_len > available - content_offset) {
    return false;
  }
  if (!is_list && header >= 0x80 && header <= 0xb7 && content_len == 1 &&
      source[content_offset] < 0x80) {
    canonical = false;
  }

  const uint32_t bounded_content_len = (uint32_t)content_len;
  item->source = source;
  item->content = source + content_offset;
  item->source_len = content_offset + bounded_content_len;
  item->content_len = bounded_content_len;
  item->is_list = is_list;
  item->canonical = canonical;
  return true;
}

bool rlp_take_item(rlp_cursor *cursor, rlp_item *item)
{
  if (!rlp_parse_item(cursor->next, cursor->remaining, item)) {
    return false;
  }
  cursor->next += item->source_len;
  cursor->remaining -= item->source_len;
  return true;
}

bool rlp_item_list(const rlp_item *item, rlp_cursor *children)
{
  if (!item->is_list) {
    return false;
  }
  children->next = item->content;
  children->remaining = item->content_len;
  return true;
}

bool rlp_exact_list(const uint8_t *source, uint32_t source_len, rlp_cursor *children)
{
  rlp_item root;
  return (rlp_parse_item(source, source_len, &root) && root.source_len == source_len &&
          rlp_item_list(&root, children)) != 0;
}

bool rlp_take_fields(rlp_cursor children, rlp_item *fields, uint32_t field_count)
{
  for (uint32_t i = 0; i < field_count; ++i) {
    if (!rlp_take_item(&children, &fields[i])) {
      return false;
    }
  }
  return children.remaining == 0;
}

bool rlp_canonical_bytes(const rlp_item *item)
{
  return (!item->is_list && item->canonical) != 0;
}

bool rlp_canonical_uint(const rlp_item *item, uint32_t maximum_len)
{
  return (rlp_canonical_bytes(item) && item->content_len <= maximum_len &&
          (item->content_len == 0 || item->content[0] != 0)) != 0;
}

bool rlp_word_raw(const rlp_item *item, u256 *value)
{
  if (item->is_list || item->content_len > 32) {
    return false;
  }
  uint8_t bytes[32] = {0};
  memcpy(bytes + 32 - item->content_len, item->content, item->content_len);
  *value = be_bytes_to_sail_word(bytes);
  return true;
}

bool rlp_quantity_word(const rlp_item *item, u256 *value)
{
  return (item->canonical && (item->content_len == 0 || item->content[0] != 0) &&
          rlp_word_raw(item, value)) != 0;
}

bool rlp_quantity_u64(const rlp_item *item, uint64_t *value)
{
  if (item->is_list || !item->canonical || item->content_len > 8 ||
      (item->content_len != 0 && item->content[0] == 0)) {
    return false;
  }
  uint64_t result = 0;
  for (uint32_t i = 0; i < item->content_len; ++i) {
    result = (result << 8) | item->content[i];
  }
  *value = result;
  return true;
}
