/* C-backed CODE for the evm-sail model: an address-keyed code store and
 * per-call-frame code descriptors.
 *
 * Account code is written rarely (seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. The store keeps each account's bytes
 * AND its JUMPDEST bitmap, built once at write time; entering a call frame
 * binds a DESCRIPTOR (pointer + length + bitmap) to the frame -- O(1), no
 * re-streaming -- and leaving a frame is free because descriptors are indexed
 * by the memory-frame depth, which the frame teardown already pops. CREATE
 * initcode (memory-sourced) and a create-tx's initcode (the streamed tx
 * input) are copied once into per-depth inline buffers.
 *
 * The Sail account store remains the authoritative spec value of acc_code;
 * this is the execution mirror. */
#include "sail.h"
#include "host_mem.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define FC_MAXDEPTH 1100
#define CS_INIT_CAP 256u          /* power of two */

/* ----------------------------- code store ------------------------------ */

typedef struct {
  uint64_t a[3];                  /* address key (BE words)        */
  uint8_t *p;                     /* code bytes                    */
  uint8_t *bm;                    /* JUMPDEST bitmap               */
  uint32_t len, cap;
  uint8_t  used;
} cs_ent;

static cs_ent  *cs_tab;
static uint32_t cs_cap, cs_n;
static cs_ent  *cs_cur;           /* entry selected by cs_begin            */
static uint32_t cs_skip;          /* PUSH-immediate skip while streaming   */

static uint64_t cs_hash(const uint64_t *a) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (int i = 0; i < 3; i++) { h ^= a[i]; h *= 0x100000001b3ull; }
  return h;
}
static cs_ent *cs_find(const uint64_t *a) {
  uint32_t i = (uint32_t)(cs_hash(a) & (cs_cap - 1));
  for (;;) {
    cs_ent *e = &cs_tab[i];
    if (!e->used || (e->a[0] == a[0] && e->a[1] == a[1] && e->a[2] == a[2])) return e;
    i = (i + 1) & (cs_cap - 1);
  }
}
static void cs_grow(void) {
  uint32_t ocap = cs_cap;
  cs_ent *otab = cs_tab;
  cs_cap = ocap ? ocap * 2 : CS_INIT_CAP;
  cs_tab = (cs_ent *)calloc(cs_cap, sizeof(cs_ent));
  for (uint32_t i = 0; i < ocap; i++)
    if (otab[i].used) *cs_find(otab[i].a) = otab[i];
  free(otab);
}

unit cs_reset(const unit u) {     /* per test case: drop every entry */
  (void)u;
  for (uint32_t i = 0; i < cs_cap; i++)
    if (cs_tab && cs_tab[i].used) { free(cs_tab[i].p); free(cs_tab[i].bm); }
  free(cs_tab);
  cs_tab = NULL; cs_cap = 0; cs_n = 0; cs_cur = NULL;
  return UNIT;
}

/* begin (re)writing the code of address (a2,a1,a0); stream bytes via cs_byte */
unit cs_begin(uint64_t a2, uint64_t a1, uint64_t a0) {
  if (!cs_tab) cs_grow();
  uint64_t key[3] = { a2, a1, a0 };
  cs_ent *e = cs_find(key);
  if (!e->used) {
    e->used = 1;
    e->a[0] = a2; e->a[1] = a1; e->a[2] = a0;
    cs_n++;
    if (cs_n * 10 >= cs_cap * 7) { cs_grow(); e = cs_find(key); }
  }
  e->len = 0;
  cs_cur = e;
  cs_skip = 0;
  return UNIT;
}
unit cs_byte(uint64_t b) {
  cs_ent *e = cs_cur;
  if (!e) return UNIT;
  if (e->len >= e->cap) {
    uint32_t n = e->cap ? e->cap * 2 : 64;
    e->p = (uint8_t *)realloc(e->p, n);
    e->bm = (uint8_t *)realloc(e->bm, n / 8 + 1);
    memset(e->bm + (e->cap / 8), 0, (n - e->cap) / 8 + 1);
    e->cap = n;
  }
  if ((e->len & 7) == 0) e->bm[e->len >> 3] = 0;   /* clear stale bits on re-stream */
  e->p[e->len] = (uint8_t)b;
  if (cs_skip) cs_skip--;
  else if (b == 0x5b) e->bm[e->len >> 3] |= (uint8_t)(1u << (e->len & 7));
  else if (b >= 0x60 && b <= 0x7f) cs_skip = (uint32_t)(b - 0x5f);
  e->len++;
  return UNIT;
}
unit cs_del(uint64_t a2, uint64_t a1, uint64_t a0) {   /* account deletion */
  if (!cs_tab) return UNIT;
  uint64_t key[3] = { a2, a1, a0 };
  cs_ent *e = cs_find(key);
  if (e->used) e->len = 0;          /* keep the slot; empty code */
  return UNIT;
}

/* ------------------------- per-frame descriptors ------------------------ */

typedef struct {
  const uint8_t *p;
  const uint8_t *bm;
  uint32_t len;
} fc_desc;

static fc_desc fc[FC_MAXDEPTH];
/* per-depth inline buffers for memory-/txdata-sourced initcode */
static struct { uint8_t *p; uint8_t *bm; uint32_t cap; } fc_inl[FC_MAXDEPTH];

static fc_desc *fc_cur(void) { return &fc[hm_depth(UNIT)]; }

static void fc_build_bitmap(uint8_t *bm, const uint8_t *p, uint32_t len) {
  memset(bm, 0, len / 8 + 1);
  uint32_t skip = 0;
  for (uint32_t i = 0; i < len; i++) {
    uint8_t b = p[i];
    if (skip) skip--;
    else if (b == 0x5b) bm[i >> 3] |= (uint8_t)(1u << (i & 7));
    else if (b >= 0x60 && b <= 0x7f) skip = (uint32_t)(b - 0x5f);
  }
}
static void fc_inl_fit(int d, uint32_t need) {
  if (fc_inl[d].cap < need) {
    uint32_t n = fc_inl[d].cap ? fc_inl[d].cap : 1024;
    while (n < need) n <<= 1;
    fc_inl[d].p = (uint8_t *)realloc(fc_inl[d].p, n);
    fc_inl[d].bm = (uint8_t *)realloc(fc_inl[d].bm, n / 8 + 1);
    fc_inl[d].cap = n;
  }
}

/* current frame's code := the store entry of (a2,a1,a0); returns its length */
uint64_t fc_set_addr(uint64_t a2, uint64_t a1, uint64_t a0) {
  fc_desc *f = fc_cur();
  if (cs_tab) {
    uint64_t key[3] = { a2, a1, a0 };
    cs_ent *e = cs_find(key);
    if (e->used && e->len) { f->p = e->p; f->bm = e->bm; f->len = e->len; return f->len; }
  }
  f->p = NULL; f->bm = NULL; f->len = 0;
  return 0;
}
/* the NEXT child frame's code := THIS frame's memory [off, off+len): a pure
 * alias (the parent's memory is frozen while the child runs; the pointer is
 * captured after an ensure, and the parent cannot realloc while suspended).
 * Only the JUMPDEST bitmap is materialized, into the child's inline slot. */
uint64_t fc_pend_mem(uint64_t off, uint64_t len) {
  int child = (int)hm_depth(UNIT) + 1;
  if (child >= FC_MAXDEPTH) return 0;
  fc_desc *f = &fc[child];
  if (!len) { f->p = NULL; f->bm = NULL; f->len = 0; return 0; }
  const uint8_t *src = hm_rd(off, len);
  fc_inl_fit(child, (uint32_t)len);
  fc_build_bitmap(fc_inl[child].bm, src, (uint32_t)len);
  f->p = src; f->bm = fc_inl[child].bm; f->len = (uint32_t)len;
  return len;
}
/* current frame's code := the streamed tx input (a create-tx's initcode) */
uint64_t fc_set_txd(const unit u) {
  (void)u;
  int d = (int)hm_depth(UNIT);
  fc_desc *f = &fc[d];
  uint64_t len64; const uint8_t *src = txd_ptr(&len64);
  uint32_t len = (uint32_t)len64;
  if (!len) { f->p = NULL; f->bm = NULL; f->len = 0; return 0; }
  fc_inl_fit(d, len);
  memcpy(fc_inl[d].p, src, len);
  fc_build_bitmap(fc_inl[d].bm, fc_inl[d].p, len);
  f->p = fc_inl[d].p; f->bm = fc_inl[d].bm; f->len = len;
  return len;
}
unit fc_set_empty(const unit u) {
  (void)u;
  fc_desc *f = fc_cur();
  f->p = NULL; f->bm = NULL; f->len = 0;
  return UNIT;
}

/* ------------------------------ accessors ------------------------------ */

uint64_t hc_byte(uint64_t i) {
  const fc_desc *f = fc_cur();
  return i < f->len ? f->p[i] : 0;
}

/* the current frame's code length (frame re-entry resync after a child pops) */
uint64_t hc_len(const unit u) { (void)u; return fc_cur()->len; }
bool hj_valid(uint64_t i) {
  const fc_desc *f = fc_cur();
  return i < f->len && ((f->bm[i >> 3] >> (i & 7)) & 1);
}
/* CODECOPY: code[off..off+len) -> memory[dst..), zero-padded past the end */
unit hc_to_mem(uint64_t dst, uint64_t off, uint64_t len) {
  if (!len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (!d) return UNIT;
  const fc_desc *f = fc_cur();
  for (uint64_t k = 0; k < len; k++) {
    uint64_t i = off + k;
    d[k] = i < f->len ? f->p[i] : 0;
  }
  return UNIT;
}

#ifdef SAIL_INT_LIMBS
/* the n-byte PUSH immediate starting at offset i, as a right-aligned word */
void hc_word(lbits *rop, uint64_t i, uint64_t n) {
  const fc_desc *f = fc_cur();
  rop->len = 256;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (uint64_t k = 0; k < n && k < 32; k++) {
    uint64_t byte = (i + k < f->len) ? f->p[i + k] : 0;
    uint64_t bit = (n - 1 - k) * 8;
    rop->d[bit >> 6] |= byte << (bit & 63);
  }
}
/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded) */
void cd_word(lbits *rop, uint64_t i) {
  rop->len = 256;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (int k = 0; k < 32; k++) {
    uint64_t byte = cd_byte(i + (uint64_t)k);
    int bit = (31 - k) * 8;
    rop->d[bit >> 6] |= byte << (bit & 63);
  }
}
#else
void hc_word(lbits *rop, uint64_t i, uint64_t n) {
  const fc_desc *f = fc_cur();
  rop->len = 256;
  mpz_set_ui(*rop->bits, 0);
  for (uint64_t k = 0; k < n && k < 32; k++) {
    uint64_t byte = (i + k < f->len) ? f->p[i + k] : 0;
    mpz_mul_2exp(*rop->bits, *rop->bits, 8);
    mpz_add_ui(*rop->bits, *rop->bits, byte);
  }
}
void cd_word(lbits *rop, uint64_t i) {
  rop->len = 256;
  mpz_set_ui(*rop->bits, 0);
  for (int k = 0; k < 32; k++) {
    mpz_mul_2exp(*rop->bits, *rop->bits, 8);
    mpz_add_ui(*rop->bits, *rop->bits, cd_byte(i + (uint64_t)k));
  }
}
#endif
