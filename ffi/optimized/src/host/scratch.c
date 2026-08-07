/* Contiguous executor scratch memory. Allocation policy lives in Sail as a
 * visible bump cursor; this backend retains and resolves the backing bytes. */
#include "evmsail/host/scratch.h"
#include "workspace.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t *scratch_bytes;
static uint64_t scratch_top;
static const uint8_t scratch_empty = 0;

void scratch_workspace_bind(void)
{
  WORKSPACE_BIND(scratch_bytes, GUEST_SCRATCH_BYTES);
}

/* Offsets and lengths are 32-bit-bounded arena quantities, so off + len
 * cannot overflow the capacity comparison. */
uint8_t *scratch_prepare(uint64_t off, uint64_t len)
{
  if (off != scratch_top) {
    return NULL;
  }
  if (len == 0) {
    return scratch_bytes;
  }
  if (off + len > GUEST_SCRATCH_BYTES) {
    return NULL;
  }
  return scratch_bytes + off;
}

bool scratch_commit(uint64_t off, uint64_t len)
{
  if (off != scratch_top || off > GUEST_SCRATCH_BYTES || len > GUEST_SCRATCH_BYTES - off) {
    return false;
  }
  scratch_top = off + len;
  return true;
}

uint8_t *scratch_borrow(uint64_t len)
{
  if (scratch_top + len > GUEST_SCRATCH_BYTES) {
    return NULL;
  }
  return scratch_bytes + scratch_top;
}

bool scratch_reserve_at(uint32_t off, uint32_t len)
{
  return scratch_prepare(off, len) != NULL;
}

void scratch_truncate(uint32_t len)
{
  uint64_t len_value = len;
  if (len_value <= scratch_top) {
    scratch_top = len_value;
  }
}

const uint8_t *scratch_region(uint64_t off, uint64_t len)
{
  if (len == 0) {
    return &scratch_empty;
  }
  if (off > scratch_top || len > scratch_top - off) {
    return NULL;
  }
  return scratch_bytes + off;
}

const uint8_t *scratch_base(void)
{
  return scratch_bytes;
}
