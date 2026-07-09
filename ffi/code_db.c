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
#include "lbits_convert.h"
#include "host_crypto.h"
#include "memory.h"
#include "returndata.h"
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

/* Reuse-layer / test-harness reset: drop every stored code so a subsequent run
 * starts with an EMPTY code DB. Content-addressed accumulation is otherwise
 * safe, but a warm process reusing this DB across fixtures would let a code
 * blob registered by one fixture satisfy a later negative "code missing" test
 * (the witness deliberately omits that code, expecting valid=false). The model
 * never calls this; it exists for the in-process harness (test_utils.c) whose
 * evmsail_clear_memory wipes state between fixtures. Frees each entry's owned
 * code/bitmap buffers, keeps the (zeroed) table allocation for reuse. */
unit code_db_reset(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < code_db_cap; i++) {
    if (code_db[i].used) {
      free(code_db[i].p);
      free(code_db[i].bm);
    }
  }
  if (code_db) memset(code_db, 0, (size_t)code_db_cap * sizeof(code_db_ent));
  code_db_n = 0;
  code_db_cur = NULL;
  code_db_skip = 0;
  return UNIT;
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

/* begin storing the code whose keccak is `h`; content-addressed --
 * returns 1 if this hash is new (caller streams the bytes via
 * code_db_stream_code_byte), 0 if it
 * is already stored (same hash == same bytes, so streaming is skipped). */
uint64_t code_db_stream_code_begin(const lbits h) {
  if (!code_db) code_db_grow();
  uint64_t key[4];
  lbits_to_be_words4(key, h);
  code_db_ent *e = code_db_find(key);
  if (e->used && e->len) { code_db_cur = e; return 0; }   /* already present */
  if (!e->used) {
    e->used = 1;
    memcpy(e->a, key, sizeof(e->a));
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

/* Content-address `src[0..len)` under the precomputed keccak `key`: find-or-insert
 * the entry, fit the buffer, copy the bytes, and build the JUMPDEST bitmap. A
 * hash already present with bytes is left untouched (the store is de-duplicated
 * and append-only). Empty code interns nothing -- a codeless account carries
 * KECCAK_EMPTY with no store entry (stored_code_length of a missing key is 0). */
static void code_db_intern(const uint64_t key[4], const uint8_t *src, uint32_t len) {
  if (!len) return;
  if (!code_db) code_db_grow();
  code_db_ent *e = code_db_find(key);
  if (e->used && e->len) return;   /* already present */
  if (!e->used) {
    e->used = 1;
    e->a[0] = key[0]; e->a[1] = key[1]; e->a[2] = key[2]; e->a[3] = key[3];
    code_db_n++;
    if (code_db_n * 10 >= code_db_cap * 7) { code_db_grow(); e = code_db_find(key); }
  }
  code_db_fit(e, len);
  memcpy(e->p, src, len);
  code_db_build_jumpdest_bitmap(e->bm, e->p, len);
  e->len = len;
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
  code_db_intern(key, src, (uint32_t)len);
  return UNIT;
}

/* Intern the pending returndata buffer (a CREATE/CREATE2 deploy's returned
 * code) under its keccak hash in one call; return the codeHash. */
void code_intern_returndata(lbits *rop, const unit u) {
  (void)u;
  const uint8_t *p = NULL;
  uint64_t len = 0;
  returndata_pending_span(&p, &len);
  uint64_t key[4] = {0, 0, 0, 0};
  host_keccak256_bytes(key, p, len);
  if (len <= UINT32_MAX) code_db_intern(key, p, (uint32_t)len);
  be_words4_to_lbits(rop, key);
}

/* Intern the 23-byte EIP-7702 delegation designation 0xef0100 ++ addr under its
 * keccak hash in one call; return the codeHash. */
void code_intern_delegation(lbits *rop, const lbits addr) {
  uint8_t b[23];
  b[0] = 0xef; b[1] = 0x01; b[2] = 0x00;
  lbits_to_be_bytes(b + 3, 20, addr);
  uint64_t key[4] = {0, 0, 0, 0};
  host_keccak256_bytes(key, b, sizeof b);
  code_db_intern(key, b, (uint32_t)sizeof b);
  be_words4_to_lbits(rop, key);
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

/* current frame's code := the store entry for codeHash `h`; len */
uint64_t frame_code_bind_stored(const lbits h) {
  fc_desc *f = fc_cur();
  if (code_db) {
    uint64_t key[4];
    lbits_to_be_words4(key, h);
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
uint64_t frame_code_bind_memory(uint64_t off, uint64_t len) {
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
uint64_t frame_code_bind_tx_input(const unit u) {
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
unit frame_code_clear(const unit u) {
  (void)u;
  fc_desc *f = fc_cur();
  f->p = NULL; f->bm = NULL; f->len = 0;
  return UNIT;
}

/* --------------------- address-keyed read accessors --------------------- */
/* EXTCODESIZE / EXTCODECOPY / EXTCODEHASH read the store directly: the Sail
 * account code value defines the value, but walking it is O(|code|) per opcode. */

static const code_db_ent *code_db_get(const lbits h) {
  if (!code_db) return NULL;
  uint64_t key[4];
  lbits_to_be_words4(key, h);
  const code_db_ent *e = code_db_find(key);
  return (e->used && e->len) ? e : NULL;
}

uint64_t code_db_stored_code_length(const lbits h) {
  const code_db_ent *e = code_db_get(h);
  return e ? e->len : 0;
}

/* EIP-7928 BAL code_changes: raw deployed code bytes for a code hash given as BE
   64-bit limbs (avoids constructing an lbits). NULL/0 for KECCAK_EMPTY / missing. */
const uint8_t *code_db_code_by_words(const uint64_t key_be[4], uint64_t *len_out) {
  *len_out = 0;
  if (!code_db) return NULL;
  const code_db_ent *e = code_db_find(key_be);
  if (!(e->used && e->len)) return NULL;
  *len_out = e->len;
  return e->p;
}

/* EXTCODECOPY: code(hash)[off..off+len) -> memory[dst..), zero-padded */
unit code_db_copy_stored_code_to_memory(const lbits h, uint64_t dst, uint64_t off, uint64_t len) {
  if (!len) return UNIT;
  uint8_t *d = hm_wr(dst, len);
  if (!d) return UNIT;
  const code_db_ent *e = code_db_get(h);
  for (uint64_t k = 0; k < len; k++) {
    uint64_t i = off + k;
    d[k] = (i >= off) ? code_db_byte(e, i) : 0;
  }
  return UNIT;
}

/* EIP-7702 delegation probe: (is_designation << 160) | target, in one call
 * (this runs on every CALL-family target; reading the code any other way
 * would be O(|code|) or a code-db walk) */
void code_db_read_delegation(lbits *rop, const lbits h) {
  const code_db_ent *e = code_db_get(h);
  int deleg = e && e->len == 23 && e->p[0] == 0xef && e->p[1] == 0x01 && e->p[2] == 0x00;
  uint8_t b[21] = {0};
  if (deleg) {
    b[0] = 0x01;                    /* is_designation flag: bit 160 */
    memcpy(b + 1, e->p + 3, 20);    /* target address              */
  }
  be_bytes_to_lbits(rop, 168, b, sizeof b);
}

/* ------------------------------ accessors ------------------------------ */

uint64_t frame_code_byte(uint64_t i) {
  const fc_desc *f = fc_cur();
  return fc_byte_at(f, i);
}

/* the current frame's code length (frame re-entry resync after a child pops) */
uint64_t frame_code_length(const unit u) { (void)u; return fc_cur()->len; }
bool frame_jumpdest_valid(uint64_t i) {
  const fc_desc *f = fc_cur();
  return i < f->len && ((f->bm[i >> 3] >> (i & 7)) & 1);
}
/* CODECOPY: code[off..off+len) -> memory[dst..), zero-padded past the end */
unit frame_code_copy_to_memory(uint64_t dst, uint64_t off, uint64_t len) {
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

/* the n-byte PUSH immediate starting at offset i, as a right-aligned word */
void frame_push_immediate_word(lbits *rop, uint64_t i, uint64_t n) {
  const fc_desc *f = fc_cur();
  uint8_t b[32];
  uint64_t cnt = n < 32 ? n : 32;
  for (uint64_t k = 0; k < cnt; k++) b[k] = (uint8_t)fc_byte_at(f, i + k);
  be_bytes_to_lbits(rop, 256, b, (size_t)cnt);
}
/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded) */
void calldata_load_word(lbits *rop, uint64_t i) {
  uint8_t b[32];
  for (int k = 0; k < 32; k++) b[k] = (uint8_t)cd_byte(i + (uint64_t)k);
  be_bytes_to_lbits(rop, 256, b, 32);
}
