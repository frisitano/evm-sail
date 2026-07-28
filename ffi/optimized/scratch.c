/* Contiguous executor scratch memory. Allocation policy lives in Sail as a
 * visible bump cursor; this backend retains and resolves the backing bytes. */
#include "scratch.h"
#include "capacity.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define SCRATCH_HARDCAP (UINT64_C(1) << 40)

static uint8_t *scratch_bytes;
static uint64_t scratch_top;
static uint64_t scratch_cap;
static const uint8_t scratch_empty;

bool scratch_configure_capacity(uint64_t capacity) {
  if (capacity > SCRATCH_HARDCAP || capacity > SIZE_MAX) return false;
  if (capacity > scratch_cap) {
    uint8_t *next = realloc(scratch_bytes, capacity ? (size_t)capacity : 1);
    if (!next) return false;
    scratch_bytes = next;
    scratch_cap = capacity;
  }
  scratch_top = 0;
  return true;
}

static bool scratch_ensure(uint64_t need) {
  evmsail_capacity_observe(EVMSAIL_CAP_SCRATCH_BYTES, need);
#ifdef EVMSAIL_CAPACITY_FIXED
  return need <= evmsail_capacity_limit(EVMSAIL_CAP_SCRATCH_BYTES) &&
         need <= scratch_cap;
#endif
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
  if (!scratch_ensure(off + len)) return NULL;
  return scratch_bytes + off;
}

bool scratch_commit(uint64_t off, uint64_t len) {
  if (off != scratch_top || off > scratch_cap || len > scratch_cap - off)
    return false;
  scratch_top = off + len;
  return true;
}

uint8_t *scratch_borrow(uint64_t len) {
  if (len > UINT64_MAX - scratch_top ||
      !scratch_ensure(scratch_top + len))
    return NULL;
  return scratch_bytes + scratch_top;
}

bool scratch_reserve_at(EVMSAIL_BYTE_QUANTITY_PARAM(off),
                        EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  uint64_t off_value = evmsail_byte_quantity_value(off);
  uint64_t len_value = evmsail_byte_quantity_value(len);
  if (off_value != scratch_top || len_value > UINT64_MAX - off_value)
    return false;
  return scratch_ensure(off_value + len_value);
}

unit scratch_truncate(EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  uint64_t len_value = evmsail_byte_quantity_value(len);
  if (len_value <= scratch_top) scratch_top = len_value;
  return UNIT;
}

const uint8_t *scratch_region(uint64_t off, uint64_t len) {
  if (len == 0) return &scratch_empty;
#ifdef EVMSAIL_POINTER_ABI
  uintptr_t base = (uintptr_t)scratch_bytes;
  uintptr_t pointer = (uintptr_t)off;
  if (!scratch_bytes || pointer < base || pointer - base > scratch_top ||
      len > scratch_top - (uint64_t)(pointer - base))
    return NULL;
  return (const uint8_t *)pointer;
#else
  if (off > scratch_top || len > scratch_top - off) return NULL;
  return scratch_bytes + off;
#endif
}

const uint8_t *scratch_base(void) { return scratch_bytes; }
uint64_t scratch_length(void) { return scratch_top; }
