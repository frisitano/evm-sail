#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_DECODING_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_DECODING_H

#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>

/* C-private canonical RLP item decoder shared by the optimized codecs. It
 * mirrors the single Sail decoder in `sail/lib/rlp/decoding.sail`: the core
 * is boolean and callers own their error policy. */
typedef struct {
  const uint8_t *source;
  const uint8_t *content;
  uint32_t source_len;
  uint32_t content_len;
  bool is_list;
  bool canonical;
} rlp_item;

typedef struct {
  const uint8_t *next;
  uint32_t remaining;
} rlp_cursor;

bool rlp_parse_item(const uint8_t *source, uint32_t available, rlp_item *item);
bool rlp_take_item(rlp_cursor *cursor, rlp_item *item);
bool rlp_item_list(const rlp_item *item, rlp_cursor *children);
bool rlp_exact_list(const uint8_t *source, uint32_t source_len, rlp_cursor *children);
bool rlp_take_fields(rlp_cursor children, rlp_item *fields, uint32_t field_count);
bool rlp_canonical_bytes(const rlp_item *item);
bool rlp_canonical_uint(const rlp_item *item, uint32_t maximum_len);
bool rlp_word_raw(const rlp_item *item, u256 *value);
bool rlp_quantity_word(const rlp_item *item, u256 *value);
bool rlp_quantity_u64(const rlp_item *item, uint64_t *value);

#endif
