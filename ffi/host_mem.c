/* C-backed EVM memory for the EL-IR Sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Following
 * the sail-riscv pattern, memory lives in C: a flat, lazily-grown byte buffer per
 * call frame with O(1) read/write. Frames form a stack so a sub-call gets fresh
 * memory (host_mem_push) and the parent's is restored on return (host_mem_pop); the
 * Sail side keeps the high-water mark (memory_size) for expansion gas.
 *
 * Only mach_bits cross the FFI (uint64_t), matching ffi/acc_shim.c. */
#include "sail.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HOST_MEM_MAXDEPTH 1100   /* DEPTH_LIMIT(1024) + tx frame + slack */

typedef struct { uint8_t *buf; size_t cap; } h_memframe;

static h_memframe h_stack[HOST_MEM_MAXDEPTH];
static int h_top = 0;   /* index of the current (innermost) frame */

static void host_mem_ensure(size_t off) {
  h_memframe *f = &h_stack[h_top];
  if (off < f->cap) return;
  size_t ncap = f->cap ? f->cap : 4096;
  while (ncap <= off) ncap <<= 1;
  uint8_t *nb = (uint8_t *)realloc(f->buf, ncap);
  if (!nb) return;                       /* OOM: leave write a no-op */
  memset(nb + f->cap, 0, ncap - f->cap); /* new bytes are zero-initialised */
  f->buf = nb;
  f->cap = ncap;
}

/* clear all frames back to a single empty top-level frame (called per tx) */
unit host_mem_reset(const unit u) {
  (void)u;
  for (int i = 0; i <= h_top; i++) {
    free(h_stack[i].buf);
    h_stack[i].buf = NULL;
    h_stack[i].cap = 0;
  }
  h_top = 0;
  return UNIT;
}

/* enter a sub-call: push the current frame, start a fresh empty one */
unit host_mem_push(const unit u) {
  (void)u;
  if (h_top + 1 < HOST_MEM_MAXDEPTH) {
    h_top++;
    h_stack[h_top].buf = NULL;
    h_stack[h_top].cap = 0;
  }
  return UNIT;
}

/* leave a sub-call: discard the current frame, restore the parent */
unit host_mem_pop(const unit u) {
  (void)u;
  free(h_stack[h_top].buf);
  h_stack[h_top].buf = NULL;
  h_stack[h_top].cap = 0;
  if (h_top > 0) h_top--;
  return UNIT;
}

/* bits(64) offset -> bits(8) byte (0 if never written / past the buffer) */
uint64_t host_mem_read(uint64_t off) {
  h_memframe *f = &h_stack[h_top];
  return (off < f->cap) ? (uint64_t)f->buf[off] : 0;
}

/* (bits(64) offset, bits(8) value) -> unit */
unit host_mem_write(uint64_t off, uint64_t v) {
  host_mem_ensure((size_t)off);
  h_memframe *f = &h_stack[h_top];
  if (off < f->cap) f->buf[off] = (uint8_t)(v & 0xff);
  return UNIT;
}
