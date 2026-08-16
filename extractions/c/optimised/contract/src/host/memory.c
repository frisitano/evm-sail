/* C-backed EVM memory for the evm-sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Memory
 * therefore lives in one fixed workspace arena. Sail owns every frame's
 * absolute arena pointer and relative established height; the host owns no
 * frame stack or cursor. Expansion zeroes precisely the newly established
 * interval, which also prevents a reused parent/child span observing stale
 * bytes. */
#include "evmsail/prelude.h"
#include "primitives/value.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/memory.h"
#include "workspace.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t *arena;

void memory_workspace_bind(void)
{
  WORKSPACE_BIND(arena, GUEST_EVM_MEMORY_BYTES);
}

static uint8_t *absolute_region(uint64_t off, uint64_t len)
{
  if (off > GUEST_EVM_MEMORY_BYTES || len > GUEST_EVM_MEMORY_BYTES - off) {
    GUEST_ABORT();
  }
  return len == 0 ? NULL : arena + off;
}

/* Absolute byte access. Sail establishes every accessed range first. */
uint64_t mem_read_byte(uint64_t off)
{
  return (uint64_t)*absolute_region(off, 1);
}

/* Materialize [pointer + established, pointer + required). */
uint64_t evm_memory_expand(uint64_t pointer, uint64_t established, uint64_t required)
{
  if (required < established || pointer > GUEST_EVM_MEMORY_BYTES ||
      required > GUEST_EVM_MEMORY_BYTES - pointer) {
    GUEST_ABORT();
  }
  if (required > established) {
    memset(arena + pointer + established, 0, (size_t)(required - established));
  }
  return pointer;
}

/* MLOAD: the 32-byte big-endian word at off. No establishment -- reads past
 * the extent are zeros, and charge_expansion precedes every MLOAD so the
 * gas-side watermark already covers the range. */
u256 mem_load_word(uint32_t off)
{
  return be_bytes_to_sail_word(absolute_region(off, 32));
}

/* MSTORE: the 32-byte big-endian word at off (establish + one memcpy) */
void mem_store_word(uint32_t off, u256 w)
{
  uint8_t *d = absolute_region(off, 32);
  sail_word_to_be_bytes(d, w);
}

/* MCOPY: overlapping-safe copy between established absolute spans. */
void mem_move(uint32_t dst, uint32_t src, uint32_t len)
{
  if (!len) {
    return;
  }
  uint8_t *destination = absolute_region(dst, len);
  uint8_t *source = absolute_region(src, len);
  memmove(destination, source, len);
}

/* (byte-quantity offset, bits(8) value) -> unit */
void mem_write_byte(uint32_t off, uint64_t v)
{
  *absolute_region(off, 1) = (uint8_t)(v & 0xff);
}

const uint8_t *evm_memory_base(void)
{
  return arena;
}
uint64_t evm_memory_capacity(void)
{
  return GUEST_EVM_MEMORY_BYTES;
}

uint8_t *evm_memory_write_region(uint64_t off, uint64_t len)
{
  return absolute_region(off, len);
}
