/* Contiguous executor scratch memory. Allocation policy lives in Sail as a
 * visible bump cursor; this backend retains and resolves the backing bytes. */
#include "scratch.h"

#include "host_crypto.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define SCRATCH_HARDCAP (UINT64_C(1) << 40)

static uint8_t *scratch_bytes;
static uint64_t scratch_top;
static uint64_t scratch_cap;
static const uint8_t scratch_empty;

static bool scratch_reserve(uint64_t need) {
  if (need <= scratch_cap) return true;
  if (need > SCRATCH_HARDCAP || need > SIZE_MAX) return false;

  uint64_t cap = scratch_cap ? scratch_cap : 4096;
  while (cap < need) {
    if (cap > SCRATCH_HARDCAP / 2) {
      cap = SCRATCH_HARDCAP;
      break;
    }
    cap *= 2;
  }
  uint8_t *next = realloc(scratch_bytes, (size_t)cap);
  if (!next) return false;
  scratch_bytes = next;
  scratch_cap = cap;
  return true;
}

uint8_t *scratch_prepare(uint64_t off, uint64_t len) {
  if (off != scratch_top || len > UINT64_MAX - off) return NULL;
  if (len == 0) return scratch_bytes;
  if (!scratch_reserve(off + len)) return NULL;
  return scratch_bytes + off;
}

bool scratch_commit(uint64_t off, uint64_t len) {
  if (off != scratch_top || off > scratch_cap || len > scratch_cap - off)
    return false;
  scratch_top = off + len;
  return true;
}

bool scratch_append_source(uint64_t dst, uint64_t source, uint64_t off,
                           uint64_t len) {
  if (dst != scratch_top || len > UINT64_MAX - dst) return false;
  if (len == 0) return true;
  if (!scratch_reserve(dst + len)) return false;

  const uint8_t *input = NULL;
  uint64_t input_len = 0;
  if (!evmsail_resolve_byte_source(source, off, len, &input, &input_len) ||
      input_len != len)
    return false;
  memmove(scratch_bytes + dst, input, (size_t)len);
  scratch_top = dst + len;
  return true;
}

unit scratch_truncate(uint64_t len) {
  if (len <= scratch_top) scratch_top = len;
  return UNIT;
}

const uint8_t *scratch_region(uint64_t off, uint64_t len) {
  if (len == 0) return &scratch_empty;
  if (off > scratch_top || len > scratch_top - off) return NULL;
  return scratch_bytes + off;
}
