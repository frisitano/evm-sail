/* C-backed CODE for the evm-sail model: a code-hash-keyed code_db and
 * per-call-frame code descriptors.
 *
 * Account code is written rarely (seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. The code_db owns each code byte slice
 * and its JUMPDEST bitmap, built once at write time; entering a call frame
 * binds a DESCRIPTOR (bytes + length + bitmap) to the frame -- O(1), no
 * re-streaming -- and leaving a frame is free because descriptors are indexed
 * by the memory-frame depth, which the frame teardown already pops. Witness
 * code is copied from the SSZ input into the database. CREATE initcode
 * (memory-sourced) and a create-tx's initcode (the streamed tx input) are
 * copied once into per-depth inline buffers.
 *
 * The Sail account store remains the authoritative account-code value;
 * this is the execution mirror. */
#include "sail.h"
#include "host_crypto.h"
#include "memory.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define FC_MAXDEPTH 1100
#define CODE_DB_INIT_CAP 256u     /* power of two */

/* ------------------------------ code_db -------------------------------- */

typedef struct {
  uint64_t a[4];                  /* codeHash key (BE 64-bit limbs) */
  uint8_t *p;                     /* owned code bytes                */
  uint8_t *bm;                    /* owned JUMPDEST bitmap           */
  uint32_t len, cap;
  uint8_t  used;
} code_db_ent;

static code_db_ent *code_db;
static uint32_t code_db_cap, code_db_n;
static code_db_ent *code_db_cur;  /* entry selected by code_db_stream_code_begin */
static uint32_t code_db_skip;     /* PUSH-immediate skip while streaming */

static uint8_t code_db_byte(const code_db_ent *e, uint64_t i) {
  return (e && i < e->len) ? e->p[i] : 0;
}

static uint64_t code_db_hash(const uint64_t *a) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (int i = 0; i < 4; i++) { h ^= a[i]; h *= 0x100000001b3ull; }
  return h;
}
static code_db_ent *code_db_find(const uint64_t *a) {
  uint32_t i = (uint32_t)(code_db_hash(a) & (code_db_cap - 1));
  for (;;) {
    code_db_ent *e = &code_db[i];
    if (!e->used || (e->a[0] == a[0] && e->a[1] == a[1] && e->a[2] == a[2] && e->a[3] == a[3]))
      return e;
    i = (i + 1) & (code_db_cap - 1);
  }
}
static void code_db_grow(void) {
  uint32_t ocap = code_db_cap;
  code_db_ent *otab = code_db;
  code_db_cap = ocap ? ocap * 2 : CODE_DB_INIT_CAP;
  code_db = (code_db_ent *)calloc(code_db_cap, sizeof(code_db_ent));
  for (uint32_t i = 0; i < ocap; i++)
    if (otab[i].used) *code_db_find(otab[i].a) = otab[i];
  free(otab);
}

static void code_db_fit(code_db_ent *e, uint32_t need) {
  if (e->cap < need) {
    uint32_t old = e->cap;
    uint32_t n = old ? old : 64;
    while (n < need) n <<= 1;
    e->p = (uint8_t *)realloc(e->p, n);
    e->bm = (uint8_t *)realloc(e->bm, n / 8 + 1);
    memset(e->bm + (old / 8), 0, (n - old) / 8 + 1);
    e->cap = n;
  }
}

static void code_db_build_jumpdest_bitmap(uint8_t *bm, const uint8_t *p, uint32_t len) {
  memset(bm, 0, len / 8 + 1);
  uint32_t skip = 0;
  for (uint32_t i = 0; i < len; i++) {
    uint8_t b = p[i];
    if (skip) skip--;
    else if (b == 0x5b) bm[i >> 3] |= (uint8_t)(1u << (i & 7));
    else if (b >= 0x60 && b <= 0x7f) skip = (uint32_t)(b - 0x5f);
  }
}

unit code_db_reset(const unit u) {     /* per test case: drop every entry */
  (void)u;
  for (uint32_t i = 0; i < code_db_cap; i++)
    if (code_db && code_db[i].used) {
      free(code_db[i].p);
      free(code_db[i].bm);
    }
  free(code_db);
  code_db = NULL; code_db_cap = 0; code_db_n = 0; code_db_cur = NULL;
  return UNIT;
}

/* begin storing the code whose keccak is (h3,h2,h1,h0); content-addressed --
 * returns 1 if this hash is new (caller streams the bytes via
 * code_db_stream_code_byte), 0 if it
 * is already stored (same hash == same bytes, so streaming is skipped). */
uint64_t code_db_stream_code_begin(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  if (!code_db) code_db_grow();
  uint64_t key[4] = { h3, h2, h1, h0 };
  code_db_ent *e = code_db_find(key);
  if (e->used && e->len) { code_db_cur = e; return 0; }   /* already present */
  if (!e->used) {
    e->used = 1;
    e->a[0] = h3; e->a[1] = h2; e->a[2] = h1; e->a[3] = h0;
    code_db_n++;
    if (code_db_n * 10 >= code_db_cap * 7) { code_db_grow(); e = code_db_find(key); }
  }
  e->len = 0;
  code_db_cur = e;
  code_db_skip = 0;
  return 1;
}
unit code_db_stream_code_byte(uint64_t b) {
  code_db_ent *e = code_db_cur;
  if (!e) return UNIT;
  if (e->len >= e->cap) code_db_fit(e, e->len + 1);
  if ((e->len & 7) == 0) e->bm[e->len >> 3] = 0;   /* clear stale bits on re-stream */
  e->p[e->len] = (uint8_t)b;
  if (code_db_skip) code_db_skip--;
  else if (b == 0x5b) e->bm[e->len >> 3] |= (uint8_t)(1u << (e->len & 7));
  else if (b >= 0x60 && b <= 0x7f) code_db_skip = (uint32_t)(b - 0x5f);
  e->len++;
  return UNIT;
}

unit code_db_store_source(uint64_t source_kind, uint64_t off, uint64_t len) {
  if (!len || len > UINT32_MAX) return UNIT;
  const uint8_t *src = NULL;
  uint64_t source_len = 0;
  if (!evmsail_resolve_byte_source(source_kind, off, len, &src, &source_len))
    return UNIT;
  if (!src || source_len != len) return UNIT;

  uint64_t key[4] = {0, 0, 0, 0};
  host_keccak256_bytes(key, src, len);

  if (!code_db) code_db_grow();
  code_db_ent *e = code_db_find(key);
  if (e->used && e->len) return UNIT;   /* already present */
  if (!e->used) {
    e->used = 1;
    e->a[0] = key[0]; e->a[1] = key[1]; e->a[2] = key[2]; e->a[3] = key[3];
    code_db_n++;
    if (code_db_n * 10 >= code_db_cap * 7) { code_db_grow(); e = code_db_find(key); }
  }

  uint32_t n = (uint32_t)len;
  code_db_fit(e, n);
  memcpy(e->p, src, n);
  code_db_build_jumpdest_bitmap(e->bm, e->p, n);
  e->len = n;
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

int code_db_frame_resolve_code(uint64_t off, uint64_t len,
                               const uint8_t **p, uint64_t *resolved_len) {
  static const uint8_t empty = 0;
  fc_desc *f = fc_cur();
  if (len == 0) {
    if (p) *p = &empty;
    if (resolved_len) *resolved_len = 0;
    return 1;
  }
  if (!f->p || off > f->len || len > (uint64_t)f->len - off) return 0;
  if (p) *p = f->p + off;
  if (resolved_len) *resolved_len = len;
  return 1;
}

static uint8_t fc_byte_at(const fc_desc *f, uint64_t i) {
  return (i < f->len) ? f->p[i] : 0;
}

static void fc_build_bitmap_bytes(uint8_t *bm, const uint8_t *p, uint32_t len) {
  code_db_build_jumpdest_bitmap(bm, p, len);
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

/* current frame's code := the store entry for codeHash (h3,h2,h1,h0); len */
uint64_t code_db_frame_set_stored_code(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  fc_desc *f = fc_cur();
  if (code_db) {
    uint64_t key[4] = { h3, h2, h1, h0 };
    code_db_ent *e = code_db_find(key);
    if (e->used && e->len) {
      f->p = e->p;
      f->bm = e->bm;
      f->len = e->len;
      return f->len;
    }
  }
  f->p = NULL; f->bm = NULL; f->len = 0;
  return 0;
}
/* the NEXT child frame's code := a copy of THIS frame's memory [off, off+len).
 * Code is immutable once the frame starts, so we avoid keeping a pointer into
 * mutable caller memory even though the caller is suspended while the child
 * runs. */
uint64_t code_db_prepare_child_frame_memory_code(uint64_t off, uint64_t len) {
  int child = (int)hm_depth(UNIT) + 1;
  if (child >= FC_MAXDEPTH) return 0;
  fc_desc *f = &fc[child];
  if (!len) { f->p = NULL; f->bm = NULL; f->len = 0; return 0; }
  const uint8_t *src = hm_rd(off, len);
  fc_inl_fit(child, (uint32_t)len);
  memcpy(fc_inl[child].p, src, (uint32_t)len);
  fc_build_bitmap_bytes(fc_inl[child].bm, fc_inl[child].p, (uint32_t)len);
  f->p = fc_inl[child].p; f->bm = fc_inl[child].bm; f->len = (uint32_t)len;
  return len;
}
/* current frame's code := the streamed tx input (a create-tx's initcode) */
uint64_t code_db_frame_set_tx_input_code(const unit u) {
  (void)u;
  int d = (int)hm_depth(UNIT);
  fc_desc *f = &fc[d];
  uint64_t len64 = txd_length(UNIT);
  uint32_t len = len64 > UINT32_MAX ? UINT32_MAX : (uint32_t)len64;
  if (!len) { f->p = NULL; f->bm = NULL; f->len = 0; return 0; }
  fc_inl_fit(d, len);
  txd_copy(fc_inl[d].p, len);
  fc_build_bitmap_bytes(fc_inl[d].bm, fc_inl[d].p, len);
  f->p = fc_inl[d].p; f->bm = fc_inl[d].bm; f->len = len;
  return len;
}
unit code_db_frame_clear_code(const unit u) {
  (void)u;
  fc_desc *f = fc_cur();
  f->p = NULL; f->bm = NULL; f->len = 0;
  return UNIT;
}

/* --------------------- address-keyed read accessors --------------------- */
/* EXTCODESIZE / EXTCODECOPY / EXTCODEHASH read the store directly: the Sail
 * account code value defines the value, but walking it is O(|code|) per opcode. */

static const code_db_ent *code_db_get(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  if (!code_db) return NULL;
  uint64_t key[4] = { h3, h2, h1, h0 };
  const code_db_ent *e = code_db_find(key);
  return (e->used && e->len) ? e : NULL;
}

uint64_t code_db_stored_code_length(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  const code_db_ent *e = code_db_get(h3, h2, h1, h0);
  return e ? e->len : 0;
}

/* EXTCODECOPY: code(hash)[off..off+len) -> memory[dst..), zero-padded */
unit code_db_copy_stored_code_to_memory(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                                        uint64_t dst, uint64_t off, uint64_t len) {
  if (!len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (!d) return UNIT;
  const code_db_ent *e = code_db_get(h3, h2, h1, h0);
  for (uint64_t k = 0; k < len; k++) {
    uint64_t i = off + k;
    d[k] = (i >= off) ? code_db_byte(e, i) : 0;
  }
  return UNIT;
}

/* EIP-7702 delegation probe: (is_designation << 160) | target, in one call
 * (this runs on every CALL-family target; reading the code any other way
 * would be O(|code|) or a code-db walk) */
void code_db_read_delegation(lbits *rop, uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  const code_db_ent *e = code_db_get(h3, h2, h1, h0);
  int deleg = e && e->len == 23 && e->p[0] == 0xef && e->p[1] == 0x01 && e->p[2] == 0x00;
#ifdef SAIL_INT_LIMBS
  rop->len = 168;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  if (deleg) {
    uint64_t w0 = 0, w1 = 0, w2 = 0;
    for (int i = 0;  i < 4;  i++) w2 = (w2 << 8) | e->p[3 + (uint64_t)i];       /* bits 159..128 */
    for (int i = 4;  i < 12; i++) w1 = (w1 << 8) | e->p[3 + (uint64_t)i];       /* bits 127..64  */
    for (int i = 12; i < 20; i++) w0 = (w0 << 8) | e->p[3 + (uint64_t)i];       /* bits  63..0   */
    rop->d[0] = w0; rop->d[1] = w1; rop->d[2] = w2 | (1ull << 32);  /* flag: bit 160 */
  }
#else
  rop->len = 168;
  mpz_set_ui(*rop->bits, 0);
  if (deleg) {
    mpz_set_ui(*rop->bits, 1);
    mpz_mul_2exp(*rop->bits, *rop->bits, 160);
    mpz_t t; mpz_init(t);
    mpz_set_ui(t, 0);
    for (int i = 0; i < 20; i++) {
      mpz_mul_2exp(t, t, 8);
      mpz_add_ui(t, t, e->p[3 + (uint64_t)i]);
    }
    mpz_add(*rop->bits, *rop->bits, t);
    mpz_clear(t);
  }
#endif
}

/* ------------------------------ accessors ------------------------------ */

uint64_t code_db_frame_code_byte(uint64_t i) {
  const fc_desc *f = fc_cur();
  return fc_byte_at(f, i);
}

/* the current frame's code length (frame re-entry resync after a child pops) */
uint64_t code_db_frame_code_length(const unit u) { (void)u; return fc_cur()->len; }
bool code_db_frame_jumpdest_valid(uint64_t i) {
  const fc_desc *f = fc_cur();
  return i < f->len && ((f->bm[i >> 3] >> (i & 7)) & 1);
}
/* CODECOPY: code[off..off+len) -> memory[dst..), zero-padded past the end */
unit code_db_copy_frame_code_to_memory(uint64_t dst, uint64_t off, uint64_t len) {
  if (!len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (!d) return UNIT;
  const fc_desc *f = fc_cur();
  for (uint64_t k = 0; k < len; k++) {
    uint64_t i = off + k;
    /* i < off => uint64 overflow of a past-end (truncated 256-bit) offset */
    d[k] = (i >= off) ? fc_byte_at(f, i) : 0;
  }
  return UNIT;
}

#ifdef SAIL_INT_LIMBS
/* the n-byte PUSH immediate starting at offset i, as a right-aligned word */
void code_db_frame_push_immediate_word(lbits *rop, uint64_t i, uint64_t n) {
  const fc_desc *f = fc_cur();
  rop->len = 256;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (uint64_t k = 0; k < n && k < 32; k++) {
    uint64_t byte = fc_byte_at(f, i + k);
    uint64_t bit = (n - 1 - k) * 8;
    rop->d[bit >> 6] |= byte << (bit & 63);
  }
}
/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded) */
void code_db_calldata_load_word(lbits *rop, uint64_t i) {
  rop->len = 256;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (int k = 0; k < 32; k++) {
    uint64_t byte = cd_byte(i + (uint64_t)k);
    int bit = (31 - k) * 8;
    rop->d[bit >> 6] |= byte << (bit & 63);
  }
}
#else
void code_db_frame_push_immediate_word(lbits *rop, uint64_t i, uint64_t n) {
  const fc_desc *f = fc_cur();
  rop->len = 256;
  mpz_set_ui(*rop->bits, 0);
  for (uint64_t k = 0; k < n && k < 32; k++) {
    uint64_t byte = fc_byte_at(f, i + k);
    mpz_mul_2exp(*rop->bits, *rop->bits, 8);
    mpz_add_ui(*rop->bits, *rop->bits, byte);
  }
}
void code_db_calldata_load_word(lbits *rop, uint64_t i) {
  rop->len = 256;
  mpz_set_ui(*rop->bits, 0);
  for (int k = 0; k < 32; k++) {
    mpz_mul_2exp(*rop->bits, *rop->bits, 8);
    mpz_add_ui(*rop->bits, *rop->bits, cd_byte(i + (uint64_t)k));
  }
}
#endif
