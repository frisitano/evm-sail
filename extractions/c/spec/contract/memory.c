/* C-backed EVM memory for the evm-sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Memory
 * therefore lives in C as one byte arena, but Sail owns every frame's absolute
 * arena pointer and relative established height. Expansion zeroes precisely
 * the newly established interval. The standard ABI identifies positions with
 * absolute offsets so reallocating the arena cannot dangle a Sail value. */
#include "sail.h"
#include "capacity.h"
#include "value_convert.h"
#include "region_access.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *arena;
static size_t arena_cap;

bool evm_memory_configure_capacity(uint64_t capacity)
{
  if (capacity > SIZE_MAX)
    return false;
  size_t requested = (size_t)capacity;
  if (requested > arena_cap) {
    uint8_t *next = (uint8_t *)realloc(arena, requested ? requested : 1);
    if (!next)
      return false;
    arena = next;
    arena_cap = requested;
  }
  return true;
}

static void arena_reserve(size_t need)
{
  evmsail_capacity_observe(EVMSAIL_CAP_MEMORY_BYTES, (uint64_t)need);
  if (need <= arena_cap)
    return;
  size_t ncap = arena_cap ? arena_cap : 4096;
  while (ncap < need) {
    if (ncap > SIZE_MAX / 2) {
      ncap = need;
      break;
    }
    ncap <<= 1;
  }
  uint8_t *nb = (uint8_t *)realloc(arena, ncap);
  if (!nb)
    abort();
  arena = nb;
  arena_cap = ncap;
}

/* Absolute byte access. Sail establishes every accessed range first. */
uint64_t mem_read_byte(uint64_t off)
{
  return off < arena_cap ? (uint64_t)arena[off] : 0;
}

/* Materialize [pointer + established, pointer + required). */
uint64_t evm_memory_expand(uint64_t pointer, uint64_t established, uint64_t required)
{
  if (required < established || pointer > UINT64_MAX - required || pointer > SIZE_MAX ||
      required > SIZE_MAX - (size_t)pointer)
    abort();
  size_t base = (size_t)pointer;
  size_t old_end = base + (size_t)established;
  size_t new_end = base + (size_t)required;
  arena_reserve(new_end);
  if (new_end > old_end)
    memset(arena + old_end, 0, new_end - old_end);
  return pointer;
}

static uint8_t *absolute_write_region(uint64_t off, uint64_t len)
{
  if (len == 0)
    return NULL;
  if (off > arena_cap || len > arena_cap - (size_t)off)
    abort();
  return arena + off;
}

/* MLOAD: the 32-byte big-endian word at off. No establishment -- reads past
 * the extent are zeros, and charge_expansion precedes every MLOAD so the
 * gas-side watermark already covers the range. */
u256 mem_load_word(uint64_t off)
{
  uint8_t *source = absolute_write_region(off, 32);
  return be_bytes_to_sail_word(source);
}

/* MSTORE: the 32-byte big-endian word at off (establish + one memcpy) */
unit mem_store_word(uint64_t off, const u256 w)
{
  uint8_t buf[32];
  sail_word_to_be_bytes(buf, (w));
  uint8_t *d = absolute_write_region(off, 32);
  memcpy(d, buf, 32);
  return UNIT;
}

/* MCOPY: overlapping-safe copy between already established absolute spans. */
unit mem_move(uint64_t dst, uint64_t src, uint64_t len)
{
  if (!len)
    return UNIT;
  uint8_t *destination = absolute_write_region(dst, len);
  uint8_t *source = absolute_write_region(src, len);
  memmove(destination, source, len);
  return UNIT;
}

/* (byte-quantity offset, bits(8) value) -> unit */
unit mem_write_byte(uint64_t off, uint64_t v)
{
  *absolute_write_region(off, 1) = (uint8_t)(v & 0xff);
  return UNIT;
}

/* Read an absolute span minted by subslicing an active or suspended Sail
 * EVM-memory frame pointer. Well-formed slices never exceed an established
 * frame; the capacity check keeps malformed host inputs fail-closed. */
const uint8_t *evm_memory_region(uint64_t off, uint64_t len)
{
  if (len == 0 || off > arena_cap || len > arena_cap - off)
    return NULL;
  return arena + off;
}

const uint8_t *evm_memory_base(void)
{
  return arena;
}
uint64_t evm_memory_capacity(void)
{
  return (uint64_t)arena_cap;
}

uint8_t *evm_memory_write_region(uint64_t off, uint64_t len)
{
  return absolute_write_region(off, len);
}
