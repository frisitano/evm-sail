/* C-backed EVM memory for the evm-sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Following
 * revm's SharedMemory, memory lives in C as ONE arena with per-frame
 * checkpoints: frame d owns arena[f_base[d] ..), and a sub-call's frame begins
 * where the (suspended) caller's ESTABLISHED extent ends. The standard ABI
 * identifies slices with absolute arena offsets so growth cannot dangle. The
 * fixed-capacity optimized ABI lowers the same semantic coordinate to a stable
 * validated pointer; any suspended frame's range remains directly addressable.
 *
 * Establishment discipline (no zeroing on frame exit): f_len[d] is the extent
 * the frame has WRITTEN OR ZEROED. Reads beyond it return 0; any write or
 * region grant that extends it first zero-fills the gap, so a dead child's
 * bytes (which live above the parent's extent) can never be observed.
 * mem_frame_leave just pops the checkpoint. The canonical generated-C ABI
 * carries mathematical byte quantities; the production ABI represents the
 * protocol-bounded values directly as uint64_t. */
#include "sail.h"
#include "capacity.h"
#include "value_convert.h"
#include "region_access.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_MAXDEPTH 1100 /* call_depth_limit (1024) + tx frame + slack */

static uint8_t *arena;
static size_t arena_cap;
static size_t f_base[MEMORY_MAXDEPTH];
static size_t f_len[MEMORY_MAXDEPTH]; /* established (written-or-zeroed) extent */
static int h_top = 0;

bool evm_memory_configure_capacity(uint64_t capacity) {
  if (capacity > SIZE_MAX) return false;
  size_t requested = (size_t)capacity;
  if (requested > arena_cap) {
    uint8_t *next = (uint8_t *)realloc(arena, requested ? requested : 1);
    if (!next) return false;
    arena = next;
    arena_cap = requested;
  }
  return true;
}

static void arena_reserve(size_t need) {
  evmsail_capacity_observe(EVMSAIL_CAP_MEMORY_BYTES, (uint64_t)need);
  if (need <= arena_cap) return;
  size_t ncap = arena_cap ? arena_cap : 4096;
  while (ncap < need) {
    if (ncap > SIZE_MAX / 2) {
      ncap = need;
      break;
    }
    ncap <<= 1;
  }
  uint8_t *nb = (uint8_t *)realloc(arena, ncap);
  if (!nb) abort();
  arena = nb;
  arena_cap = ncap;
}

/* extend the current frame's established extent to `end` (frame-relative),
 * zero-filling the gap. Sail has already established and charged the range;
 * a failure here is a host-contract violation rather than EVM OutOfGas. */
static void f_establish(uint64_t end) {
  if (end <= f_len[h_top]) return;
  size_t base = f_base[h_top];
  if (end > SIZE_MAX || base > SIZE_MAX - (size_t)end) abort();
  arena_reserve(base + (size_t)end);
  memset(arena + base + f_len[h_top], 0, (size_t)end - f_len[h_top]);
  f_len[h_top] = (size_t)end;
}

static uint8_t *frame_write_region(uint64_t off, uint64_t len);

/* clear back to a single empty top-level frame (called per tx); the arena
 * allocation is kept for reuse across transactions */
unit mem_clear(const unit u) {
  (void)u;
  h_top = 0;
  f_base[0] = 0;
  f_len[0] = 0;
  return UNIT;
}

/* Enter a sub-call and return the fresh frame's absolute arena base. Sail
 * stores that base in the active frame's nominal EvmMemorySlice. */
static uint64_t mem_frame_enter_value(void) {
  /* The EVM call-depth rule makes this host-capacity failure unreachable. */
  if (h_top + 1 >= MEMORY_MAXDEPTH) abort();
  h_top++;
  f_base[h_top] = f_base[h_top - 1] + f_len[h_top - 1];
  f_len[h_top] = 0;
  return (uint64_t)f_base[h_top];
}

uint64_t mem_frame_enter(const unit u) {
  (void)u;
  return mem_frame_enter_value();
}

/* leave a sub-call: pop the checkpoint. The dead frame's bytes sit above the
 * parent's established extent, so establishment zero-fills over them before
 * the parent could ever read them. */
unit mem_frame_leave(const unit u) {
  (void)u;
  if (h_top > 0) h_top--;
  return UNIT;
}

/* byte-quantity offset -> bits(8) byte (0 beyond the established extent) */
uint64_t mem_read_byte(uint64_t off) {
  uint64_t offset = off;
  return (offset < f_len[h_top])
             ? (uint64_t)arena[f_base[h_top] + offset]
             : 0;
}

/* Current call-frame depth. */
uint64_t hm_depth(const unit u) { (void)u; return (uint64_t)h_top; }

/* Materialize the logical extent whose affordability Sail already proved and
 * return its absolute arena base. region_access.c packages this span in the
 * generated EvmMemorySlice representation. */
uint64_t evm_memory_expand(uint64_t len) {
  f_establish(len);
  return (uint64_t)f_base[h_top];
}

/* establish + a WRITE pointer to [off, off+len) of the current frame (the
 * gas-side watermark is raised by charge_expansion before any copy opcode
 * writes) */
static uint8_t *frame_write_region(uint64_t off, uint64_t len) {
  if (len == 0) return NULL;
  if (off > UINT64_MAX - len) abort();
  f_establish(off + len);
  return arena + f_base[h_top] + off;
}

/* MLOAD: the 32-byte big-endian word at off. No establishment -- reads past
 * the extent are zeros, and charge_expansion precedes every MLOAD so the
 * gas-side watermark already covers the range. */
u256 mem_load_word(uint64_t off) {
  uint64_t offset = off;
  uint8_t buf[32];
  for (int i = 0; i < 32; i++) {
    uint64_t o = offset + (uint64_t)i;
    buf[i] = (o >= offset && o < f_len[h_top])
                 ? arena[f_base[h_top] + o]
                 : 0;
  }
  return (be_bytes_to_sail_word(buf));
}

/* MSTORE: the 32-byte big-endian word at off (establish + one memcpy) */
unit mem_store_word(uint64_t off, const u256 w) {
  uint8_t buf[32];
  sail_word_to_be_bytes(buf, (w));
  uint8_t *d = frame_write_region(off, 32);
  memcpy(d, buf, 32);
  return UNIT;
}

/* MCOPY: overlapping-safe copy within the current frame. Both ranges are
 * established BEFORE either pointer is taken (a second establishment could
 * realloc the arena from under the first pointer). */
unit mem_move(uint64_t dst, uint64_t src, uint64_t len) {
  uint64_t dst_value = dst;
  uint64_t src_value = src;
  uint64_t len_value = len;
  if (!len_value) return UNIT;
  if (src_value > UINT64_MAX - len_value ||
      dst_value > UINT64_MAX - len_value)
    abort();
  f_establish(src_value + len_value);
  f_establish(dst_value + len_value);
  memmove(arena + f_base[h_top] + dst_value,
          arena + f_base[h_top] + src_value, len_value);
  return UNIT;
}

/* (byte-quantity offset, bits(8) value) -> unit */
unit mem_write_byte(uint64_t off, uint64_t v) {
  uint64_t offset = off;
  if (offset == UINT64_MAX) abort();
  f_establish(offset + 1);
  arena[f_base[h_top] + offset] = (uint8_t)(v & 0xff);
  return UNIT;
}

/* Read an absolute span minted by subslicing an active or suspended Sail
 * EVM-memory frame pointer. Well-formed slices never exceed an established
 * frame; the capacity check keeps malformed host inputs fail-closed. */
const uint8_t *evm_memory_region(uint64_t off, uint64_t len) {
  if (len == 0 || off > arena_cap || len > arena_cap - off) return NULL;
  return arena + off;
}

const uint8_t *evm_memory_base(void) { return arena; }
uint64_t evm_memory_capacity(void) { return (uint64_t)arena_cap; }

uint8_t *evm_memory_write_region(uint64_t off, uint64_t len) {
  return frame_write_region(off, len);
}
