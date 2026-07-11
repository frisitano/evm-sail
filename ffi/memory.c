/* C-backed EVM memory for the evm-sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Following
 * the sail-riscv pattern, memory lives in C: a flat, lazily-grown byte buffer per
 * call frame with O(1) read/write. Frames form a stack so a sub-call gets fresh
 * memory (mem_frame_enter) and the parent's is restored on return (mem_frame_leave); the
 * Sail side keeps the high-water mark (memory_size) for expansion gas.
 *
 * Only mach_bits cross the FFI (uint64_t), matching the other host FFI modules. */
#include "sail.h"
#include "lbits_convert.h"
#include "host_crypto.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_MAXDEPTH 1100   /* DEPTH_LIMIT(1024) + tx frame + slack */

typedef struct { uint8_t *buf; size_t cap; } h_memframe;

static h_memframe h_stack[MEMORY_MAXDEPTH];
static int h_top = 0;

/* ---- CALLDATA: a per-frame descriptor aliasing the parent's memory ----
 * src: -2 = empty, -1 = the transaction input buffer, >= 0 = a (suspended)
 * ancestor memory frame. Resolved through h_stack at READ time, so a parent
 * realloc never leaves a stale pointer. The aliased frame cannot change while
 * its child executes (frames above it are suspended). */
uint8_t *hm_wr(uint64_t off, uint64_t len);   /* fwd decl (defined below) */

typedef struct { int src; uint64_t off, len; } hm_cd;
static hm_cd cd[MEMORY_MAXDEPTH];
static hm_cd cd_pending = { -2, 0, 0 };

/* The current transaction's input as a SOURCE REFERENCE -- the witness span
 * the decoder carried on Transaction.input_src, never copied. Block system
 * calls stage their 32-byte word into the small owned slot instead
 * (kind = TXIN_KIND_WORD). All txd_* / cd_* reads resolve through it. */
#define TXIN_KIND_WORD UINT64_MAX
static struct { uint64_t kind, off, len; } txin = { 0, 0, 0 };
static uint8_t txin_word[32];

int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **p, uint64_t *resolved_len);

/* a READ pointer to tx-input[i, i+len), NULL past the end / on a bad source */
static const uint8_t *txin_region(uint64_t i, uint64_t len) {
  if (len == 0 || i > txin.len || len > txin.len - i) return NULL;
  if (txin.kind == TXIN_KIND_WORD) return txin_word + i;
  const uint8_t *p = NULL;
  uint64_t rlen = 0;
  if (!evmsail_resolve_byte_source(txin.kind, txin.off + i, len, &p, &rlen) || rlen != len) return NULL;
  return p;
}

static uint8_t txin_byte(uint64_t i) {
  const uint8_t *p = txin_region(i, 1);
  return p ? *p : 0;
}

static void memory_ensure(size_t off) {
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
unit mem_clear(const unit u) {
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
unit mem_frame_enter(const unit u) {
  (void)u;
  if (h_top + 1 < MEMORY_MAXDEPTH) {
    h_top++;
    h_stack[h_top].buf = NULL;
    h_stack[h_top].cap = 0;
    cd[h_top] = cd_pending;
  }
  cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
  return UNIT;
}

/* the NEXT child's calldata := this frame's memory [off, off+len) */
unit calldata_bind_memory(uint64_t off, uint64_t len) {
  cd_pending.src = h_top; cd_pending.off = off; cd_pending.len = len;
  return UNIT;
}
unit calldata_bind_empty(const unit u) {
  (void)u;
  cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
  return UNIT;
}
/* the CURRENT (tx-level) frame's calldata := the streamed tx input */
unit calldata_bind_tx_input(const unit u) {
  (void)u;
  cd[h_top].src = -1; cd[h_top].off = 0; cd[h_top].len = txin.len;
  return UNIT;
}
/* bind the tx input BY REFERENCE (no copy). Fail-closed on a bad or
 * self-referential source, matching the empty-input path. */
uint64_t txdata_bind_source(uint64_t kind, uint64_t off, uint64_t len) {
  txin.kind = 0; txin.off = 0; txin.len = 0;
  if (len == 0) return 0;
  const uint8_t *src = NULL;
  uint64_t rlen = 0;
  if (kind == EVMSAIL_SOURCE_TX_INPUT ||
      !evmsail_resolve_byte_source(kind, off, len, &src, &rlen) || !src || rlen != len) {
    return 0;
  }
  txin.kind = kind; txin.off = off; txin.len = len;
  return len;
}

uint64_t txdata_stage_word(const lbits w) {
  lbits_to_be_bytes(txin_word, 32, w);
  txin.kind = TXIN_KIND_WORD; txin.off = 0; txin.len = 32;
  return 32;
}

uint64_t txdata_count_nonzero(const unit u) {
  (void)u;
  const uint8_t *p = txin_region(0, txin.len);
  uint64_t c = 0;
  if (p) for (uint64_t i = 0; i < txin.len; i++) if (p[i]) c++;
  return c;
}

/* the executing tx's input (a create-tx's initcode source; gas byte reads) */
uint64_t txd_copy(uint8_t *dst, uint64_t cap) {
  uint64_t n = txin.len < cap ? txin.len : cap;
  const uint8_t *p = txin_region(0, n);
  if (p) memcpy(dst, p, n); else n = 0;
  return n;
}
uint64_t txdata_byte_at(uint64_t i)  { return txin_byte(i); }
uint64_t txdata_length(const unit u) { (void)u; return txin.len; }
const uint8_t *txd_rd(uint64_t off, uint64_t len) {
  static const uint8_t zero = 0;
  if (len == 0) return &zero;
  return txin_region(off, len);
}

/* calldata byte i (0 past the end -- and 0 past the source's ALLOCATED cap:
 * an expansion-charged but never-written parent range reads as zeros) */
static uint8_t cd_at(const hm_cd *c, uint64_t i) {
  if (i >= c->len) return 0;
  if (c->src == -1) return txin_byte(i);
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
unit calldata_copy_to_memory(uint64_t dst, uint64_t off, uint64_t len) {
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
unit mem_frame_leave(const unit u) {
  (void)u;
  free(h_stack[h_top].buf);
  h_stack[h_top].buf = NULL;
  h_stack[h_top].cap = 0;
  if (h_top > 0) h_top--;
  return UNIT;
}

/* bits(64) offset -> bits(8) byte (0 if never written / past the buffer) */
uint64_t mem_read_byte(uint64_t off) {
  h_memframe *f = &h_stack[h_top];
  return (off < f->cap) ? (uint64_t)f->buf[off] : 0;
}

/* current call-frame depth (the returndata slots in returndata.c key off it) */
uint64_t hm_depth(const unit u) { (void)u; return (uint64_t)h_top; }

/* ensure capacity (zero-filled) and return a READ pointer to [off, off+len) */
const uint8_t *mem_region(uint64_t off, uint64_t len) {
  static const uint8_t zero = 0;
  if (len == 0) return &zero;
  memory_ensure((size_t)(off + len - 1));
  h_memframe *f = &h_stack[h_top];
  if (off + len > f->cap) return &zero;            /* OOM fallback */
  return f->buf + off;
}

/* ensure capacity and return a WRITE pointer to [off, off+len) (the gas-side
 * watermark is raised by charge_expansion before any copy opcode writes) */
uint8_t *hm_wr(uint64_t off, uint64_t len) {
  if (len == 0) return NULL;
  memory_ensure((size_t)(off + len - 1));
  h_memframe *f = &h_stack[h_top];
  if (off + len > f->cap) return NULL;
  return f->buf + off;
}

/* MLOAD: the 32-byte big-endian word at off. No ensure -- reads past the
 * buffer are zeros (same as mem_read_byte), and charge_expansion precedes
 * every MLOAD so the gas-side watermark already covers the range. */
void mem_load_word(lbits *rop, uint64_t off) {
  h_memframe *f = &h_stack[h_top];
  uint8_t buf[32];
  for (int i = 0; i < 32; i++) {
    uint64_t o = off + (uint64_t)i;
    buf[i] = (o < f->cap) ? f->buf[o] : 0;
  }
  be_bytes_to_lbits(rop, 256, buf, 32);
}

/* MSTORE: the 32-byte big-endian word at off (ensure + one memcpy) */
unit mem_store_word(uint64_t off, const lbits w) {
  uint8_t buf[32];
  lbits_to_be_bytes(buf, 32, w);
  uint8_t *d = hm_wr(off, 32);
  if (d) memcpy(d, buf, 32);
  return UNIT;
}

/* MCOPY: overlapping-safe copy within the current frame */
unit mem_move(uint64_t dst, uint64_t src, uint64_t len) {
  if (len) {
    const uint8_t *s = mem_region(src, len);
    uint8_t *d = hm_wr(dst, len);
    if (s && d) memmove(d, s, len);
  }
  return UNIT;
}

/* (bits(64) offset, bits(8) value) -> unit */
unit mem_write_byte(uint64_t off, uint64_t v) {
  memory_ensure((size_t)off);
  h_memframe *f = &h_stack[h_top];
  if (off < f->cap) f->buf[off] = (uint8_t)(v & 0xff);
  return UNIT;
}
