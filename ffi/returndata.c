#include "returndata.h"
#include "memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RETURNDATA_MAXDEPTH 1100

typedef struct { uint8_t *p; uint64_t cap, len; } returndata_buf;
static returndata_buf returndata_frame[RETURNDATA_MAXDEPTH];
static returndata_buf returndata_pending;

static void returndata_fit(returndata_buf *b, uint64_t need) {
  if (b->cap < need) {
    uint64_t n = b->cap ? b->cap : 256;
    while (n < need) n <<= 1;
    b->p = (uint8_t *)realloc(b->p, n);
    b->cap = n;
  }
}

unit returndata_reset_all(const unit u) {
  (void)u;
  for (int i = 0; i < RETURNDATA_MAXDEPTH; i++) returndata_frame[i].len = 0;
  returndata_pending.len = 0;
  return UNIT;
}

unit returndata_clear_current(const unit u) {
  (void)u;
  returndata_frame[hm_depth(UNIT)].len = 0;
  return UNIT;
}

unit returndata_discard_pending(const unit u) {
  (void)u;
  returndata_pending.len = 0;
  return UNIT;
}

unit returndata_capture_memory(uint64_t off, uint64_t len) {
  returndata_fit(&returndata_pending, len ? len : 1);
  if (len) {
    const uint8_t *p = hm_rd(off, len);
    memcpy(returndata_pending.p, p, len);
  }
  returndata_pending.len = len;
  return UNIT;
}

unit returndata_adopt_pending(const unit u) {
  (void)u;
  returndata_buf *slot = &returndata_frame[hm_depth(UNIT)];
  returndata_buf tmp = *slot;
  *slot = returndata_pending;
  returndata_pending = tmp;
  returndata_pending.len = 0;
  return UNIT;
}

uint64_t returndata_current_len(const unit u) {
  (void)u;
  return returndata_frame[hm_depth(UNIT)].len;
}

uint64_t returndata_pending_len(const unit u) {
  (void)u;
  return returndata_pending.len;
}

uint64_t returndata_pending_byte(uint64_t i) {
  return i < returndata_pending.len ? returndata_pending.p[i] : 0;
}

int returndata_pending_span(const uint8_t **p, uint64_t *len) {
  if (p) *p = returndata_pending.p;
  if (len) *len = returndata_pending.len;
  return 1;
}

unit returndata_copy_current_to_memory(uint64_t dst, uint64_t off, uint64_t len) {
  returndata_buf *slot = &returndata_frame[hm_depth(UNIT)];
  if (!len || off + len > slot->len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (d) memcpy(d, slot->p + off, len);
  return UNIT;
}

unit returndata_copy_current_prefix_to_memory(uint64_t dst, uint64_t want) {
  returndata_buf *slot = &returndata_frame[hm_depth(UNIT)];
  uint64_t n = want < slot->len ? want : slot->len;
  if (n) {
    uint8_t *d = hm_wr(dst, n);
    if (d) memcpy(d, slot->p, n);
  }
  return UNIT;
}

uint8_t *returndata_prepare_pending(uint64_t cap) {
  returndata_fit(&returndata_pending, cap ? cap : 1);
  returndata_pending.len = 0;
  return returndata_pending.p;
}

unit returndata_set_pending_len(uint64_t len) {
  returndata_fit(&returndata_pending, len ? len : 1);
  returndata_pending.len = len;
  return UNIT;
}
