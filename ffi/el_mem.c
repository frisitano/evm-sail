/* C-backed EVM memory for the EL-IR Sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Following
 * the sail-riscv pattern, memory lives in C: a flat, lazily-grown byte buffer per
 * call frame with O(1) read/write. Frames form a stack so a sub-call gets fresh
 * memory (el_mem_push) and the parent's is restored on return (el_mem_pop); the
 * Sail side keeps the high-water mark (memory_size) for expansion gas.
 *
 * Only mach_bits cross the FFI (uint64_t), matching ffi/acc_shim.c. */
#include "sail.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define EL_MEM_MAXDEPTH 1100   /* DEPTH_LIMIT(1024) + tx frame + slack */

typedef struct { uint8_t *buf; size_t cap; } el_memframe;

static el_memframe el_stack[EL_MEM_MAXDEPTH];
static int el_top = 0;   /* index of the current (innermost) frame */

static void el_mem_ensure(size_t off) {
  el_memframe *f = &el_stack[el_top];
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
unit el_mem_reset(const unit u) {
  (void)u;
  for (int i = 0; i <= el_top; i++) {
    free(el_stack[i].buf);
    el_stack[i].buf = NULL;
    el_stack[i].cap = 0;
  }
  el_top = 0;
  return UNIT;
}

/* enter a sub-call: push the current frame, start a fresh empty one */
unit el_mem_push(const unit u) {
  (void)u;
  if (el_top + 1 < EL_MEM_MAXDEPTH) {
    el_top++;
    el_stack[el_top].buf = NULL;
    el_stack[el_top].cap = 0;
  }
  return UNIT;
}

/* leave a sub-call: discard the current frame, restore the parent */
unit el_mem_pop(const unit u) {
  (void)u;
  free(el_stack[el_top].buf);
  el_stack[el_top].buf = NULL;
  el_stack[el_top].cap = 0;
  if (el_top > 0) el_top--;
  return UNIT;
}

/* bits(64) offset -> bits(8) byte (0 if never written / past the buffer) */
uint64_t el_mem_read(uint64_t off) {
  el_memframe *f = &el_stack[el_top];
  return (off < f->cap) ? (uint64_t)f->buf[off] : 0;
}

/* (bits(64) offset, bits(8) value) -> unit */
unit el_mem_write(uint64_t off, uint64_t v) {
  el_mem_ensure((size_t)off);
  el_memframe *f = &el_stack[el_top];
  if (off < f->cap) f->buf[off] = (uint8_t)(v & 0xff);
  return UNIT;
}
