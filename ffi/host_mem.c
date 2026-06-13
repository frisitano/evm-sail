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
/* The block's transaction inputs, indexed by tx position. Each tx's data is
 * loaded into its slot once (by the runner / stateless decoder); the executing
 * tx is SELECTED by index (txin_activate). The Transaction struct carries only
 * the index -- the bytes live here in memory, never duplicated in a Sail list. */
#define TXIN_MAX 256
/* A slot is either OWNED (buf is malloc'd, filled byte-by-byte: EEST / synthetic
 * txs whose bytes originate in Sail) or a VIEW (p aliases an external buffer --
 * the stateless SSZ input -- so the tx operates directly over it, zero copy).
 * Readers only touch (p, len); p == buf for an owned slot. */
typedef struct { uint8_t *buf; const uint8_t *p; uint64_t len, cap; } txin_slot;
static txin_slot txin[TXIN_MAX];
static int txin_fill = 0;   /* slot currently being populated  */
static int txin_sel  = 0;   /* slot of the executing tx        */

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
  cd[h_top].src = -1; cd[h_top].off = 0; cd[h_top].len = txin[txin_sel].len;
  return UNIT;
}
/* begin populating an OWNED tx slot `idx` (resets length; storage is cached) */
unit txin_begin(uint64_t idx) {
  txin_fill = (idx < TXIN_MAX) ? (int)idx : 0;
  txin[txin_fill].len = 0;
  txin[txin_fill].p = txin[txin_fill].buf;
  return UNIT;
}
unit txin_byte(uint64_t b) {
  txin_slot *s = &txin[txin_fill];
  if (s->len >= s->cap) {
    uint64_t n = s->cap ? s->cap * 2 : 1024;
    s->buf = (uint8_t *)realloc(s->buf, n);
    s->cap = n;
  }
  s->buf[s->len++] = (uint8_t)b;
  s->p = s->buf;
  return UNIT;
}
/* point tx slot `idx` at an EXTERNAL buffer [ptr, ptr+len) (no copy). The tx
 * then reads its input directly over that memory (the stateless SSZ input). */
unit txin_view(uint64_t idx, const uint8_t *ptr, uint64_t len) {
  txin_slot *s = &txin[(idx < TXIN_MAX) ? (int)idx : 0];
  s->p = ptr; s->len = len;
  return UNIT;
}
/* select tx slot `idx` as the executing transaction; returns its input length */
uint64_t txin_activate(uint64_t idx) {
  txin_sel = (idx < TXIN_MAX) ? (int)idx : 0;
  return txin[txin_sel].len;
}

uint64_t cd_len(const unit u) { (void)u; return cd[h_top].len; }

/* the executing tx's input (a create-tx's initcode source; gas byte reads) */
const uint8_t *txd_ptr(uint64_t *len) { *len = txin[txin_sel].len; return txin[txin_sel].p; }
uint64_t txd_at(uint64_t i)  { return i < txin[txin_sel].len ? txin[txin_sel].p[i] : 0; }
uint64_t txd_length(const unit u) { (void)u; return txin[txin_sel].len; }

/* calldata byte i (0 past the end -- and 0 past the source's ALLOCATED cap:
 * an expansion-charged but never-written parent range reads as zeros) */
static uint8_t cd_at(const hm_cd *c, uint64_t i) {
  if (i >= c->len) return 0;
  if (c->src == -1) return i < txin[txin_sel].len ? txin[txin_sel].p[i] : 0;
  if (c->src >= 0) {
    const h_memframe *f = &h_stack[c->src];
    uint64_t p = c->off + i;
    return (f->buf && p < f->cap) ? f->buf[p] : 0;
  }
  return 0;
}
uint64_t cd_byte(uint64_t i) { return cd_at(&cd[h_top], i); }

/* CALLDATACOPY: calldata[off..off+len) -> memory[dst..), zero-padded.
 * `off` arrives truncated to 64 bits from a 256-bit EVM offset; a past-end
 * source offset must zero-fill the WHOLE dest, so guard against the uint64
 * wraparound of `off + k` re-aliasing back into the real calldata. */
unit cd_to_mem(uint64_t dst, uint64_t off, uint64_t len) {
  if (!len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (!d) return UNIT;
  const hm_cd *c = &cd[h_top];
  for (uint64_t k = 0; k < len; k++) {
    uint64_t i = off + k;
    d[k] = (i < off) ? 0 : cd_at(c, i); /* i < off => uint64 overflow => past-end */
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
