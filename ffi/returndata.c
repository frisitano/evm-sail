#include "returndata.h"
#include "memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HR_MAXDEPTH 1100

typedef struct { uint8_t *p; uint64_t cap, len; } hr_buf;
static hr_buf hr_rd[HR_MAXDEPTH];
static hr_buf hr_pend;

static void hr_fit(hr_buf *b, uint64_t need) {
  if (b->cap < need) {
    uint64_t n = b->cap ? b->cap : 256;
    while (n < need) n <<= 1;
    b->p = (uint8_t *)realloc(b->p, n);
    b->cap = n;
  }
}

unit hr_reset(const unit u) {
  (void)u;
  for (int i = 0; i < HR_MAXDEPTH; i++) hr_rd[i].len = 0;
  hr_pend.len = 0;
  return UNIT;
}

unit hr_clear(const unit u) {
  (void)u;
  hr_rd[hm_depth(UNIT)].len = 0;
  return UNIT;
}

unit hr_discard(const unit u) {
  (void)u;
  hr_pend.len = 0;
  return UNIT;
}

unit hr_capture(uint64_t off, uint64_t len) {
  hr_fit(&hr_pend, len ? len : 1);
  if (len) {
    const uint8_t *p = hm_rd(off, len);
    memcpy(hr_pend.p, p, len);
  }
  hr_pend.len = len;
  return UNIT;
}

unit hr_adopt(const unit u) {
  (void)u;
  hr_buf *slot = &hr_rd[hm_depth(UNIT)];
  hr_buf tmp = *slot;
  *slot = hr_pend;
  hr_pend = tmp;
  hr_pend.len = 0;
  return UNIT;
}

uint64_t hr_len(const unit u) {
  (void)u;
  return hr_rd[hm_depth(UNIT)].len;
}

uint64_t hr_pending_len(const unit u) {
  (void)u;
  return hr_pend.len;
}

uint64_t hr_pending_byte(uint64_t i) {
  return i < hr_pend.len ? hr_pend.p[i] : 0;
}

unit hr_to_mem(uint64_t dst, uint64_t off, uint64_t len) {
  hr_buf *slot = &hr_rd[hm_depth(UNIT)];
  if (!len || off + len > slot->len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (d) memcpy(d, slot->p + off, len);
  return UNIT;
}

unit hr_out_region(uint64_t dst, uint64_t want) {
  hr_buf *slot = &hr_rd[hm_depth(UNIT)];
  uint64_t n = want < slot->len ? want : slot->len;
  if (n) {
    uint8_t *d = hm_wr(dst, n);
    if (d) memcpy(d, slot->p, n);
  }
  return UNIT;
}

uint8_t *returndata_prepare_pending(uint64_t cap) {
  hr_fit(&hr_pend, cap ? cap : 1);
  hr_pend.len = 0;
  return hr_pend.p;
}

unit returndata_set_pending_len(uint64_t len) {
  hr_fit(&hr_pend, len ? len : 1);
  hr_pend.len = len;
  return UNIT;
}
