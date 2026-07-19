/* C-backed CODE for the evm-sail model: a code-hash-keyed code_db, an
 * append-only byte arena, and packed JUMPDEST-table storage.
 *
 * Account code is written rarely (seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. Each code_db entry names an absolute
 * span in the arena plus a JumpdestRef into a flat arena allocated once from
 * the code length and populated with completed 256-bit chunks by Sail. C never
 * analyzes opcodes: it only stores packed words and answers membership
 * queries. Sail's single frame_code Code register is the complete active
 * executable state.
 *
 * The Sail account store remains the authoritative account-code value;
 * this is the execution mirror. */
#include "sail.h"
#include "byte_slice_glue.h"
#include "lbits_convert.h"
#include "host_crypto.h"
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

static uint64_t jumpdest_word_count(uint64_t code_len) {
  return (code_len >> 6) + ((code_len & UINT64_C(63)) != 0);
}

static int jumpdest_table_span(uint64_t ref, uint64_t code_len,
                               size_t *base_out, uint64_t *nwords_out) {
  if (!ref || !code_len || code_len > UINT32_MAX) return 0;
  uint64_t nwords = jumpdest_word_count(code_len);
  uint64_t base64 = ref - 1;
  if (base64 > SIZE_MAX) return 0;
  size_t base = (size_t)base64;
  if (base > jumpdest_arena_len ||
      nwords > (uint64_t)(jumpdest_arena_len - base)) return 0;
  if (base_out) *base_out = base;
  if (nwords_out) *nwords_out = nwords;
  return 1;
}

static int jumpdest_table_matches(uint64_t ref, uint32_t code_len) {
  if (!code_len) return ref == 0;
  size_t base = 0;
  uint64_t nwords = 0;
  if (!jumpdest_table_span(ref, code_len, &base, &nwords)) return 0;
  uint32_t used = code_len & 63u;
  return !used || (jumpdest_arena[base + (size_t)nwords - 1] >> used) == 0;
}

/* Reserve the exact table size before Sail starts analysis. The arena is
 * zeroed so chunks without JUMPDESTs need no writes. */
uint64_t jumpdest_table_alloc(EVMSAIL_BYTE_QUANTITY_PARAM(code_len)) {
  uint64_t code_len_value = evmsail_byte_quantity_value(code_len);
  if (!code_len_value || code_len_value > UINT32_MAX) return 0;
  uint64_t nwords = jumpdest_word_count(code_len_value);
  if (nwords > SIZE_MAX - jumpdest_arena_len) return 0;
  size_t off = jumpdest_arena_len;
  size_t end = off + (size_t)nwords;
  if (!jumpdest_arena_reserve(end)) return 0;
  memset(jumpdest_arena + off, 0, (size_t)nwords * sizeof(*jumpdest_arena));
  jumpdest_arena_len = end;
  return (uint64_t)off + 1;
}

/* Store one completed Sail chunk. Chunk bit zero describes the first byte in
 * the corresponding 256-byte code span, hence little-endian limb order. */
bool jumpdest_table_store_chunk(uint64_t ref,
                                EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                                EVMSAIL_BYTE_QUANTITY_PARAM(chunk_index),
                                const sail_bits256 chunk) {
  uint64_t code_len_value = evmsail_byte_quantity_value(code_len);
  uint64_t chunk_index_value = evmsail_byte_quantity_value(chunk_index);
  size_t base = 0;
  uint64_t nwords = 0;
  if (!jumpdest_table_span(ref, code_len_value, &base, &nwords) ||
      chunk_index_value > UINT64_MAX / 4)
    return false;
  uint64_t first = chunk_index_value * 4;
  if (first >= nwords) return false;

  uint64_t words[4];
#ifdef EVMSAIL_STANDARD_ABI
  lbits_to_le_words4(words, chunk);
#else
  memcpy(words, chunk.limbs, sizeof(words));
#endif
  uint64_t count = nwords - first;
  if (count > 4) count = 4;
  for (uint64_t i = count; i < 4; i++)
    if (words[i] != 0) return false;
  uint32_t used = (uint32_t)(code_len_value & 63u);
  if (first + count == nwords && used && (words[count - 1] >> used) != 0)
    return false;
  memcpy(jumpdest_arena + base + (size_t)first, words,
         (size_t)count * sizeof(*words));
  return true;
}

bool jumpdest_ref_contains(uint64_t ref,
                           EVMSAIL_BYTE_QUANTITY_PARAM(code_len),
                           EVMSAIL_BYTE_QUANTITY_PARAM(i)) {
  uint64_t code_len_value = evmsail_byte_quantity_value(code_len);
  uint64_t index_value = evmsail_byte_quantity_value(i);
  if (ref == 0 || index_value >= code_len_value) return false;
  uint64_t base = ref - 1;
  uint64_t word = index_value >> 6;
  if (word > UINT64_MAX - base) return false;
  uint64_t index = base + word;
  return index < jumpdest_arena_len &&
         ((jumpdest_arena[(size_t)index] >> (index_value & 63)) & UINT64_C(1));
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
                          uint32_t len, uint64_t jumpdest_ref) {
  if (!len) return jumpdest_ref == 0;
  if (!jumpdest_table_matches(jumpdest_ref, len)) return 0;
  if (!code_db) code_db_grow();
  code_db_ent *e = code_db_find(key);
  if (e->used && e->len) {
    if (e->len != len || memcmp(code_arena + e->off, src, len) != 0 ||
        e->jumpdest_ref == 0) return 0;
    size_t old_base = 0, new_base = 0;
    uint64_t nwords = 0;
    if (!jumpdest_table_span(e->jumpdest_ref, len, &old_base, &nwords) ||
        !jumpdest_table_span(jumpdest_ref, len, &new_base, NULL)) return 0;
    return memcmp(jumpdest_arena + old_base, jumpdest_arena + new_base,
                  (size_t)nwords * sizeof(*jumpdest_arena)) == 0;
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
  e->off = (uint64_t)code_arena_len;
  memmove(code_arena + code_arena_len, src, len);
  code_arena_len += len;
  e->len = len;
  e->jumpdest_ref = jumpdest_ref;
  return 1;
}

EVMSAIL_HASH_RETURN code_db_store_indexed_source(
    EVMSAIL_HASH_RESULT(result) uint64_t source_kind, uint64_t off,
    uint64_t len, uint64_t jumpdest_ref) {
  uint64_t key[4] = {0, 0, 0, 0};
  uint8_t key_bytes[32];
  if (len > UINT32_MAX ||
      !jumpdest_table_matches(jumpdest_ref, (uint32_t)len)) {
    be_words4_to_be_bytes(key_bytes, key);
    EVMSAIL_RETURN_HASH_BE_BYTES(result, key_bytes);
  }
  const uint8_t *src = NULL;
  uint64_t source_len = 0;
  if (!evmsail_resolve_byte_source(source_kind, off, len, &src, &source_len) ||
      !src || source_len != len) {
    be_words4_to_be_bytes(key_bytes, key);
    EVMSAIL_RETURN_HASH_BE_BYTES(result, key_bytes);
  }
  host_keccak256_bytes(key, src, len);
  if (!code_db_intern(key, src, (uint32_t)len, jumpdest_ref))
    memset(key, 0, sizeof key);
  be_words4_to_be_bytes(key_bytes, key);
  EVMSAIL_RETURN_HASH_BE_BYTES(result, key_bytes);
}

/* Intern the 23-byte EIP-7702 delegation designation 0xef0100 ++ addr under its
 * keccak hash with the explicit Sail analysis; return the codeHash. */
EVMSAIL_HASH_RETURN code_intern_indexed_delegation(
    EVMSAIL_HASH_RESULT(result) sail_address addr, uint64_t jumpdest_ref) {
  uint8_t b[23];
  b[0] = 0xef; b[1] = 0x01; b[2] = 0x00;
  evmsail_address_to_be_bytes(b + 3, addr);
  uint64_t key[4] = {0, 0, 0, 0};
  uint8_t key_bytes[32];
  host_keccak256_bytes(key, b, sizeof b);
  if (!code_db_intern(key, b, (uint32_t)sizeof b, jumpdest_ref))
    memset(key, 0, sizeof key);
  be_words4_to_be_bytes(key_bytes, key);
  EVMSAIL_RETURN_HASH_BE_BYTES(result, key_bytes);
}

/* ----------------------- code-hash lookup ------------------------------- */
/* Return the code span and its associated JUMPDEST table as one invariant.
 * code_glue.c assembles the generated Code result. */

static const code_db_ent *code_db_get(sail_hash h) {
  if (!code_db) return NULL;
  uint64_t key[4];
  sail_hash_to_be_words4(key, h);
  const code_db_ent *e = code_db_find(key);
  return (e->used && e->len) ? e : NULL;
}

bool code_db_lookup_indexed(sail_hash h, uint64_t *off, uint64_t *len,
                            uint64_t *jumpdest_ref) {
  const code_db_ent *e = code_db_get(h);
  if (!e) return false;
  *off = e->off;
  *len = e->len;
  *jumpdest_ref = e->jumpdest_ref;
  return true;
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

/* EIP-7702 delegation probe in one call (this runs on every CALL-family
 * target; reading the code any other way would be O(|code|) or a code-db
 * walk).  The generated-model glue pairs this flag with the nominal address. */
bool code_db_read_delegation_address(uint8_t address[20], sail_hash h) {
  const code_db_ent *e = code_db_get(h);
  const uint8_t *p = e ? code_arena + e->off : NULL;
  bool deleg = e && e->len == 23 && p[0] == 0xef && p[1] == 0x01 && p[2] == 0x00;
  memset(address, 0, 20);
  if (deleg) memcpy(address, p + 3, 20);
  return deleg;
}
