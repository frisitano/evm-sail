/* C-backed CODE for the evm-sail model: a code-hash-keyed code_db, an
 * append-only byte arena, and packed JUMPDEST-table storage.
 *
 * Account code is written rarely (seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. Each code_db entry names an absolute
 * span in the arena plus a JumpdestRef into a flat arena populated from the
 * completed bitmap produced by Sail's index_code pass. C never analyzes
 * opcodes: it only stores packed words and answers membership queries. Sail's
 * single frame_code IndexedCode register is the complete active executable
 * state.
 *
 * The Sail account store remains the authoritative account-code value;
 * this is the execution mirror. */
#include "sail.h"
#include "lbits_convert.h"
#include "host_crypto.h"
#include "memory.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CODE_DB_INIT_CAP 256u     /* power of two */

/* -------------------------- jumpdest tables ---------------------------- */

static uint64_t *jumpdest_arena;
static size_t jumpdest_arena_cap, jumpdest_arena_len;

static int jumpdest_arena_reserve(size_t need) {
  if (need <= jumpdest_arena_cap) return 1;
  size_t n = jumpdest_arena_cap ? jumpdest_arena_cap : 256;
  while (n < need) {
    if (n > SIZE_MAX / 2) return 0;
    n <<= 1;
  }
  uint64_t *p = (uint64_t *)realloc(jumpdest_arena, n * sizeof(*p));
  if (!p) return 0;
  jumpdest_arena = p;
  jumpdest_arena_cap = n;
  return 1;
}

static int jumpdest_bitmap_matches(const uint64_t *words, uint64_t nwords,
                                   uint32_t code_len) {
  uint64_t expected = ((uint64_t)code_len + 63) / 64;
  if (nwords != expected || (expected && !words)) return 0;
  uint32_t used = code_len & 63u;
  if (used && (words[expected - 1] >> used) != 0) return 0;
  return 1;
}

/* Append one completed Sail bitmap. The one-based word offset is the
 * JumpdestRef exposed to the model; zero remains the empty/invalid sentinel. */
static uint64_t jumpdest_arena_insert(const uint64_t *words, uint64_t nwords) {
  if (!nwords || nwords > SIZE_MAX - jumpdest_arena_len) return 0;
  size_t off = jumpdest_arena_len;
  size_t end = off + (size_t)nwords;
  if (!jumpdest_arena_reserve(end)) return 0;
  memcpy(jumpdest_arena + off, words, (size_t)nwords * sizeof(*words));
  jumpdest_arena_len = end;
  return (uint64_t)off + 1;
}

bool jumpdest_ref_contains(uint64_t ref, uint64_t code_len, uint64_t i) {
  if (ref == 0 || i >= code_len) return false;
  uint64_t base = ref - 1;
  uint64_t word = i >> 6;
  if (word > UINT64_MAX - base) return false;
  uint64_t index = base + word;
  return index < jumpdest_arena_len &&
         ((jumpdest_arena[(size_t)index] >> (i & 63)) & UINT64_C(1));
}

/* ------------------------------ code_db -------------------------------- */

typedef struct {
  uint64_t a[4];                  /* codeHash key (BE 64-bit limbs) */
  uint64_t off;                   /* absolute offset in code_arena   */
  uint64_t jumpdest_ref;          /* resolved Sail-built bitmap      */
  uint32_t len;
  uint8_t  used;
} code_db_ent;

static code_db_ent *code_db;
static uint32_t code_db_cap, code_db_n;
static uint8_t *code_arena;
static size_t code_arena_cap, code_arena_len;

static uint8_t code_db_byte(const code_db_ent *e, uint64_t i) {
  return (e && i < e->len) ? code_arena[e->off + i] : 0;
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
 * evmsail_clear_memory wipes state between fixtures. The code, table, and
 * jumpdest arenas retain their allocations but reset their logical lengths. */
unit code_db_reset(const unit u) {
  (void)u;
  if (code_db) memset(code_db, 0, (size_t)code_db_cap * sizeof(code_db_ent));
  code_db_n = 0;
  code_arena_len = 0;
  jumpdest_arena_len = 0;
  return UNIT;
}

static int code_arena_reserve(size_t need) {
  if (need <= code_arena_cap) return 1;
  size_t n = code_arena_cap ? code_arena_cap : 4096;
  while (n < need) {
    if (n > SIZE_MAX / 2) return 0;
    n <<= 1;
  }
  uint8_t *p = (uint8_t *)realloc(code_arena, n);
  if (!p) return 0;
  code_arena = p;
  code_arena_cap = n;
  return 1;
}

/* Content-address `src[0..len)` under the precomputed keccak `key`: find or
 * insert the bytes and Sail-supplied analysis result. A hash already present
 * with bytes is left untouched. Empty code interns nothing -- a codeless
 * account carries KECCAK_EMPTY with no store entry. */
static int code_db_intern(const uint64_t key[4], const uint8_t *src,
                          uint32_t len, const uint64_t *jumpdests,
                          uint64_t nwords) {
  if (!len) return nwords == 0;
  if (!jumpdest_bitmap_matches(jumpdests, nwords, len)) return 0;
  if (!code_db) code_db_grow();
  code_db_ent *e = code_db_find(key);
  if (e->used && e->len) {
    if (e->len != len || memcmp(code_arena + e->off, src, len) != 0 ||
        e->jumpdest_ref == 0) return 0;
    uint64_t base = e->jumpdest_ref - 1;
    if (base > jumpdest_arena_len ||
        nwords > (uint64_t)jumpdest_arena_len - base) return 0;
    return memcmp(jumpdest_arena + (size_t)base, jumpdests,
                  (size_t)nwords * sizeof(*jumpdests)) == 0;
  }
  if (!e->used) {
    e->used = 1;
    e->a[0] = key[0]; e->a[1] = key[1]; e->a[2] = key[2]; e->a[3] = key[3];
    code_db_n++;
    if (code_db_n * 10 >= code_db_cap * 7) { code_db_grow(); e = code_db_find(key); }
  }
  /* A source may itself be a CodeSource sub-slice. Preserve its arena offset
   * across reserve/realloc before copying the new (content-addressed) entry. */
  int src_in_arena = 0;
  size_t src_off = 0;
  if (code_arena && src) {
    uintptr_t base = (uintptr_t)code_arena;
    uintptr_t addr = (uintptr_t)src;
    if (addr >= base && addr - base <= code_arena_len &&
        len <= code_arena_len - (size_t)(addr - base)) {
      src_in_arena = 1;
      src_off = (size_t)(addr - base);
    }
  }
  if (len > SIZE_MAX - code_arena_len ||
      !code_arena_reserve(code_arena_len + len)) return 0;
  if (src_in_arena) src = code_arena + src_off;
  uint64_t jumpdest_ref = jumpdest_arena_insert(jumpdests, nwords);
  if (!jumpdest_ref) return 0;
  e->off = (uint64_t)code_arena_len;
  memmove(code_arena + code_arena_len, src, len);
  code_arena_len += len;
  e->len = len;
  e->jumpdest_ref = jumpdest_ref;
  return 1;
}

void code_db_store_indexed_words(lbits *rop, uint64_t source_kind,
                                 uint64_t off, uint64_t len,
                                 const uint64_t *jumpdests,
                                 uint64_t nwords) {
  uint64_t key[4] = {0, 0, 0, 0};
  if (len > UINT32_MAX ||
      !jumpdest_bitmap_matches(jumpdests, nwords, (uint32_t)len)) {
    be_words4_to_lbits(rop, key);
    return;
  }
  const uint8_t *src = NULL;
  uint64_t source_len = 0;
  if (!evmsail_resolve_byte_source(source_kind, off, len, &src, &source_len) ||
      !src || source_len != len) {
    be_words4_to_lbits(rop, key);
    return;
  }
  host_keccak256_bytes(key, src, len);
  if (!code_db_intern(key, src, (uint32_t)len, jumpdests, nwords))
    memset(key, 0, sizeof key);
  be_words4_to_lbits(rop, key);
}

/* Intern the 23-byte EIP-7702 delegation designation 0xef0100 ++ addr under its
 * keccak hash with the explicit Sail analysis; return the codeHash. */
void code_intern_indexed_delegation_words(lbits *rop, const lbits addr,
                                          const uint64_t *jumpdests,
                                          uint64_t nwords) {
  uint8_t b[23];
  b[0] = 0xef; b[1] = 0x01; b[2] = 0x00;
  lbits_to_be_bytes(b + 3, 20, addr);
  uint64_t key[4] = {0, 0, 0, 0};
  host_keccak256_bytes(key, b, sizeof b);
  if (!code_db_intern(key, b, (uint32_t)sizeof b, jumpdests, nwords))
    memset(key, 0, sizeof key);
  be_words4_to_lbits(rop, key);
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

uint64_t code_db_stored_code_offset(const lbits h) {
  const code_db_ent *e = code_db_get(h);
  return e ? e->off : 0;
}

uint64_t code_db_stored_jumpdest_ref(const lbits h) {
  const code_db_ent *e = code_db_get(h);
  return e ? e->jumpdest_ref : 0;
}

uint64_t code_db_byte_at(uint64_t off) {
  return off < code_arena_len ? code_arena[off] : 0;
}

/* Resolve an absolute CodeSource arena span. Offsets, unlike pointers, remain
 * valid when the arena allocation grows. */
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len) {
  static const uint8_t empty = 0;
  const uint8_t *src = &empty;
  if (len != 0) {
    if (off > code_arena_len || len > code_arena_len - off) return 0;
    src = code_arena + off;
  }
  if (p) *p = src;
  if (resolved_len) *resolved_len = len;
  return 1;
}

/* EIP-7928 BAL code_changes: raw deployed code bytes for a code hash given as BE
   64-bit limbs (avoids constructing an lbits). NULL/0 for KECCAK_EMPTY / missing. */
const uint8_t *code_db_code_by_words(const uint64_t key_be[4], uint64_t *len_out) {
  *len_out = 0;
  if (!code_db) return NULL;
  const code_db_ent *e = code_db_find(key_be);
  if (!(e->used && e->len)) return NULL;
  *len_out = e->len;
  return code_arena + e->off;
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
  const uint8_t *p = e ? code_arena + e->off : NULL;
  int deleg = e && e->len == 23 && p[0] == 0xef && p[1] == 0x01 && p[2] == 0x00;
  uint8_t b[21] = {0};
  if (deleg) {
    b[0] = 0x01;                    /* is_designation flag: bit 160 */
    memcpy(b + 1, p + 3, 20);       /* target address              */
  }
  be_bytes_to_lbits(rop, 168, b, sizeof b);
}
