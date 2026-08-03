/* Contiguous executor scratch memory. Allocation policy lives in Sail as a
 * visible bump cursor; this backend retains and resolves the backing bytes. */
#include "evmsail/host/scratch.h"
#include "workspace.h"

#include <stddef.h>
#include <string.h>

static uint8_t *scratch_bytes;
static uint64_t scratch_top;
static const uint8_t scratch_empty;

void scratch_workspace_bind(void) {
  WORKSPACE_BIND(scratch_bytes, GUEST_SCRATCH_BYTES);
}

uint8_t *scratch_prepare(uint64_t off, uint64_t len) {
  if (off != scratch_top || len > UINT64_MAX - off) return NULL;
  if (len == 0) return scratch_bytes;
  if (off + len > GUEST_SCRATCH_BYTES) return NULL;
  return scratch_bytes + off;
}

bool scratch_commit(uint64_t off, uint64_t len) {
  if (off != scratch_top || off > GUEST_SCRATCH_BYTES ||
      len > GUEST_SCRATCH_BYTES - off)
    return false;
  scratch_top = off + len;
  return true;
}

uint8_t *scratch_borrow(uint64_t len) {
  if (len > UINT64_MAX - scratch_top ||
      scratch_top + len > GUEST_SCRATCH_BYTES)
    return NULL;
  return scratch_bytes + scratch_top;
}

bool scratch_reserve_at(uint32_t off, uint32_t len) {
  uint64_t off_value = off;
  uint64_t len_value = len;
  if (off_value != scratch_top || len_value > UINT64_MAX - off_value)
    return false;
  return off_value + len_value <= GUEST_SCRATCH_BYTES;
}

unit scratch_truncate(uint32_t len) {
  uint64_t len_value = len;
  if (len_value <= scratch_top) scratch_top = len_value;
  return UNIT;
}

const uint8_t *scratch_region(uint64_t off, uint64_t len) {
  if (len == 0) return &scratch_empty;
  if (off > scratch_top || len > scratch_top - off) return NULL;
  return scratch_bytes + off;
}

const uint8_t *scratch_base(void) { return scratch_bytes; }
