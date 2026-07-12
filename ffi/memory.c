/* C-backed EVM memory for the evm-sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Following
 * revm's SharedMemory, memory lives in C as ONE arena with per-frame
 * checkpoints: frame d owns arena[f_base[d] ..), and a sub-call's frame begins
 * where the (suspended) caller's ESTABLISHED extent ends. Offsets -- never
 * pointers -- identify bytes, so arena growth cannot dangle, and any suspended
 * frame's range is addressable as a plain (base + off) slice.
 *
 * Establishment discipline (no zeroing on frame exit): f_len[d] is the extent
 * the frame has WRITTEN OR ZEROED. Reads beyond it return 0; any write or
 * region grant that extends it first zero-fills the gap, so a dead child's
 * bytes (which live above the parent's extent) can never be observed.
 * mem_frame_leave just pops the checkpoint.
 *
 * Only mach_bits cross the FFI (uint64_t), matching the other host FFI modules. */
#include "sail.h"
#include "lbits_convert.h"
#include "host_crypto.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_MAXDEPTH 1100          /* DEPTH_LIMIT(1024) + tx frame + slack */
#define MEMORY_HARDCAP ((size_t)1 << 40) /* gas can never pay for more */

static uint8_t *arena;
static size_t arena_cap;
static size_t f_base[MEMORY_MAXDEPTH];
static size_t f_len[MEMORY_MAXDEPTH]; /* established (written-or-zeroed) extent */
static int h_top = 0;

static int arena_reserve(size_t need) {
  if (need <= arena_cap) return 1;
  if (need >= MEMORY_HARDCAP) return 0;
  size_t ncap = arena_cap ? arena_cap : 4096;
  while (ncap < need) ncap <<= 1;
  uint8_t *nb = (uint8_t *)realloc(arena, ncap);
  if (!nb) return 0; /* OOM: leave the access a no-op / zero read */
  arena = nb;
  arena_cap = ncap;
  return 1;
}

/* extend the current frame's established extent to `end` (frame-relative),
 * zero-filling the gap; 0 on overflow/OOM */
static int f_establish(uint64_t end) {
  if (end <= f_len[h_top]) return 1;
  size_t base = f_base[h_top];
  if (end >= MEMORY_HARDCAP - base) return 0;
  if (!arena_reserve(base + (size_t)end)) return 0;
  memset(arena + base + f_len[h_top], 0, (size_t)end - f_len[h_top]);
  f_len[h_top] = (size_t)end;
  return 1;
}

/* ---- CALLDATA: a per-frame ABSOLUTE arena slice ----
 * src: -2 = empty, -1 = the transaction input reference, 0 = arena bytes at
 * an absolute offset. The caller ESTABLISHES the args range at bind time (it
 * is already gas-charged, and establishment zero-fills the never-written
 * part), so the slice needs no frame identity and no bound beyond its own
 * length; the caller is suspended while its child reads, so the bytes are
 * frozen. */
uint8_t *hm_wr(uint64_t off, uint64_t len); /* fwd decl (defined below) */

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

/* clear back to a single empty top-level frame (called per tx); the arena
 * allocation is kept for reuse across transactions */
unit mem_clear(const unit u) {
  (void)u;
  h_top = 0;
  f_base[0] = 0;
  f_len[0] = 0;
  cd[0].src = -2; cd[0].off = 0; cd[0].len = 0;
  cd_pending = cd[0];
  return UNIT;
}

/* enter a sub-call: checkpoint a fresh frame after the caller's established
 * extent; the pending calldata descriptor (set by the caller just before) is
 * adopted */
unit mem_frame_enter(const unit u) {
  (void)u;
  if (h_top + 1 < MEMORY_MAXDEPTH) {
    h_top++;
    f_base[h_top] = f_base[h_top - 1] + f_len[h_top - 1];
    f_len[h_top] = 0;
    cd[h_top] = cd_pending;
  }
  cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
  return UNIT;
}

/* leave a sub-call: pop the checkpoint. The dead frame's bytes sit above the
 * parent's established extent, so establishment zero-fills over them before
 * the parent could ever read them. */
unit mem_frame_leave(const unit u) {
  (void)u;
  if (h_top > 0) h_top--;
  return UNIT;
}

/* the NEXT child's calldata := this frame's memory [off, off+len), as an
 * absolute arena slice (fail-closed empty on overflow/OOM) */
unit calldata_bind_memory(uint64_t off, uint64_t len) {
  if (len == 0 || off > UINT64_MAX - len || !f_establish(off + len)) {
    cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
    return UNIT;
  }
  cd_pending.src = 0; cd_pending.off = f_base[h_top] + off; cd_pending.len = len;
  return UNIT;
}
unit calldata_bind_empty(const unit u) {
  (void)u;
  cd_pending.src = -2; cd_pending.off = 0; cd_pending.len = 0;
  return UNIT;
}
/* the CURRENT (tx-level) frame's calldata := the bound tx input */
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

/* calldata byte i (0 past the slice's end; the referenced bytes were
 * established at bind time) */
static uint8_t cd_at(const hm_cd *c, uint64_t i) {
  if (i >= c->len) return 0;
  if (c->src == -1) return txin_byte(i);
  if (c->src == 0) return arena[c->off + i];
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

/* bits(64) offset -> bits(8) byte (0 beyond the established extent) */
uint64_t mem_read_byte(uint64_t off) {
  return (off < f_len[h_top]) ? (uint64_t)arena[f_base[h_top] + off] : 0;
}

/* current call-frame depth (the returndata slots in returndata.c key off it) */
uint64_t hm_depth(const unit u) { (void)u; return (uint64_t)h_top; }

/* establish + a READ pointer to [off, off+len) of the current frame */
const uint8_t *mem_region(uint64_t off, uint64_t len) {
  static const uint8_t zero = 0;
  if (len == 0) return &zero;
  if (off > UINT64_MAX - len || !f_establish(off + len)) return &zero; /* OOM/overflow */
  return arena + f_base[h_top] + off;
}

/* establish + a WRITE pointer to [off, off+len) of the current frame (the
 * gas-side watermark is raised by charge_expansion before any copy opcode
 * writes) */
uint8_t *hm_wr(uint64_t off, uint64_t len) {
  if (len == 0) return NULL;
  if (off > UINT64_MAX - len || !f_establish(off + len)) return NULL;
  return arena + f_base[h_top] + off;
}

/* MLOAD: the 32-byte big-endian word at off. No establishment -- reads past
 * the extent are zeros, and charge_expansion precedes every MLOAD so the
 * gas-side watermark already covers the range. */
void mem_load_word(lbits *rop, uint64_t off) {
  uint8_t buf[32];
  for (int i = 0; i < 32; i++) {
    uint64_t o = off + (uint64_t)i;
    buf[i] = (o >= off && o < f_len[h_top]) ? arena[f_base[h_top] + o] : 0;
  }
  be_bytes_to_lbits(rop, 256, buf, 32);
}

/* MSTORE: the 32-byte big-endian word at off (establish + one memcpy) */
unit mem_store_word(uint64_t off, const lbits w) {
  uint8_t buf[32];
  lbits_to_be_bytes(buf, 32, w);
  uint8_t *d = hm_wr(off, 32);
  if (d) memcpy(d, buf, 32);
  return UNIT;
}

/* MCOPY: overlapping-safe copy within the current frame. Both ranges are
 * established BEFORE either pointer is taken (a second establishment could
 * realloc the arena from under the first pointer). */
unit mem_move(uint64_t dst, uint64_t src, uint64_t len) {
  if (!len) return UNIT;
  if (src > UINT64_MAX - len || dst > UINT64_MAX - len) return UNIT;
  if (!f_establish(src + len) || !f_establish(dst + len)) return UNIT;
  memmove(arena + f_base[h_top] + dst, arena + f_base[h_top] + src, len);
  return UNIT;
}

/* (bits(64) offset, bits(8) value) -> unit */
unit mem_write_byte(uint64_t off, uint64_t v) {
  if (off == UINT64_MAX || !f_establish(off + 1)) return UNIT;
  arena[f_base[h_top] + off] = (uint8_t)(v & 0xff);
  return UNIT;
}
