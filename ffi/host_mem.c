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
static int h_top = 0;

/* ---- CALLDATA: a per-frame descriptor aliasing the parent's memory ----
 * src: -2 = empty, -1 = the transaction input buffer, >= 0 = a (suspended)
 * ancestor memory frame. Resolved through h_stack at READ time, so a parent
 * realloc never leaves a stale pointer. The aliased frame cannot change while
 * its child executes (frames above it are suspended). */
uint8_t *hm_wr(uint64_t off, uint64_t len);   /* fwd decl (defined below) */

typedef struct { int src; uint64_t off, len; } hm_cd;
static hm_cd cd[HOST_MEM_MAXDEPTH];
static hm_cd cd_pending = { -2, 0, 0 };
static uint8_t *txd; static uint64_t txd_cap, txd_len;   /* index of the current (innermost) frame */

static void host_mem_ensure(size_t off) {
  h_memframe *f = &h_stack[h_top];
  if (off < f->cap) return;
  if (off >= ((size_t)1 << 40)) return;            /* hard cap: gas can never pay for this */
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
  cd[0].src = -2; cd[0].off = 0; cd[0].len = 0;
  cd_pending = cd[0];
  return UNIT;
}

/* enter a sub-call: push the current frame, start a fresh empty one; the
 * pending calldata descriptor (set by the caller just before) is adopted */
unit host_mem_push(const unit u) {
  (void)u;
  if (h_top + 1 < HOST_MEM_MAXDEPTH) {
    h_top++;
    h_stack[h_top].buf = NULL;
    h_stack[h_top].cap = 0;
    cd[h_top] = cd_pending;
  }
  cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
  return UNIT;
}

/* the NEXT child's calldata := this frame's memory [off, off+len) */
unit cd_set(uint64_t off, uint64_t len) {
  cd_pending.src = h_top; cd_pending.off = off; cd_pending.len = len;
  return UNIT;
}
unit cd_set_empty(const unit u) {
  (void)u;
  cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
  return UNIT;
}
/* the CURRENT (tx-level) frame's calldata := the streamed tx input */
unit cd_set_tx(const unit u) {
  (void)u;
  cd[h_top].src = -1; cd[h_top].off = 0; cd[h_top].len = txd_len;
  return UNIT;
}
unit txd_begin(const unit u) { (void)u; txd_len = 0; return UNIT; }
unit txd_byte(uint64_t b) {
  if (txd_len >= txd_cap) {
    uint64_t n = txd_cap ? txd_cap * 2 : 1024;
    txd = (uint8_t *)realloc(txd, n);
    txd_cap = n;
  }
  txd[txd_len++] = (uint8_t)b;
  return UNIT;
}

uint64_t cd_len(const unit u) { (void)u; return cd[h_top].len; }

/* the streamed tx input (a create-tx's initcode source) */
const uint8_t *txd_ptr(uint64_t *len) { *len = txd_len; return txd; }

/* calldata byte i (0 past the end -- and 0 past the source's ALLOCATED cap:
 * an expansion-charged but never-written parent range reads as zeros) */
static uint8_t cd_at(const hm_cd *c, uint64_t i) {
  if (i >= c->len) return 0;
  if (c->src == -1) return i < txd_len ? txd[i] : 0;
  if (c->src >= 0) {
    const h_memframe *f = &h_stack[c->src];
    uint64_t p = c->off + i;
    return (f->buf && p < f->cap) ? f->buf[p] : 0;
  }
  return 0;
}
uint64_t cd_byte(uint64_t i) { return cd_at(&cd[h_top], i); }

/* CALLDATACOPY: calldata[off..off+len) -> memory[dst..), zero-padded */
unit cd_to_mem(uint64_t dst, uint64_t off, uint64_t len) {
  if (!len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (!d) return UNIT;
  const hm_cd *c = &cd[h_top];
  for (uint64_t k = 0; k < len; k++) d[k] = cd_at(c, off + k);
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

/* current call-frame depth (the returndata slots in acc_shim key off it) */
uint64_t hm_depth(const unit u) { (void)u; return (uint64_t)h_top; }

/* ensure capacity (zero-filled) and return a READ pointer to [off, off+len) */
const uint8_t *hm_rd(uint64_t off, uint64_t len) {
  static const uint8_t zero = 0;
  if (len == 0) return &zero;
  host_mem_ensure((size_t)(off + len - 1));
  h_memframe *f = &h_stack[h_top];
  if (off + len > f->cap) return &zero;            /* OOM fallback */
  return f->buf + off;
}

/* ensure capacity and return a WRITE pointer to [off, off+len) (the gas-side
 * watermark is raised by charge_expansion before any copy opcode writes) */
uint8_t *hm_wr(uint64_t off, uint64_t len) {
  if (len == 0) return NULL;
  host_mem_ensure((size_t)(off + len - 1));
  h_memframe *f = &h_stack[h_top];
  if (off + len > f->cap) return NULL;
  return f->buf + off;
}

/* MCOPY: overlapping-safe copy within the current frame */
unit hm_move(uint64_t dst, uint64_t src, uint64_t len) {
  if (len) {
    const uint8_t *s = hm_rd(src, len);
    uint8_t *d = hm_wr(dst, len);
    if (s && d) memmove(d, s, len);
  }
  return UNIT;
}

/* (bits(64) offset, bits(8) value) -> unit */
unit host_mem_write(uint64_t off, uint64_t v) {
  host_mem_ensure((size_t)off);
  h_memframe *f = &h_stack[h_top];
  if (off < f->cap) f->buf[off] = (uint8_t)(v & 0xff);
  return UNIT;
}
