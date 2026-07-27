/* C-backed CODE for the evm-sail model: a code-hash-keyed code_db, an
 * append-only byte arena, and packed JUMPDEST-table storage.
 *
 * Account code is written rarely (seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. Each code_db entry names an absolute
 * span in the arena plus a JumpdestRef into a flat arena allocated once from
 * the code length. Standard builds populate completed 256-bit chunks from the
 * explicit Sail analysis. Optimized witness indexing may scan the same bytes
 * directly in C as one whole-operation lowering. Sail's single frame_code Code
 * register is the complete active executable state.
 *
 * The Sail account store remains the authoritative account-code value;
 * this is the execution mirror. */
#include "sail.h"
#include "byte_slice_glue.h"
#include "hash_bytes.h"
#include "value_convert.h"
#include "zkvm_accelerators.h"
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

static uint64_t jumpdest_table_alloc_value(uint64_t code_len_value) {
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

/* Reserve the exact table size before Sail starts analysis. The arena is
 * zeroed so chunks without JUMPDESTs need no writes. */
uint64_t jumpdest_table_alloc(EVMSAIL_BYTE_QUANTITY_PARAM(code_len)) {
  return jumpdest_table_alloc_value(evmsail_byte_quantity_value(code_len));
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
  sail_word_to_le_words4(words, chunk);
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
  sail_hash key;             /* codeHash digest bytes          */
  uint64_t off;                   /* absolute offset in code_arena   */
  uint64_t jumpdest_ref;          /* resolved Sail-built bitmap      */
  uint32_t len;
  uint8_t  used;
} code_db_ent;

static code_db_ent *code_db;
static uint32_t code_db_cap, code_db_n;
static uint8_t *code_arena;
static size_t code_arena_cap, code_arena_len;

static uint64_t code_db_hash(const sail_hash *key) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (int i = 0; i < 4; i++) {
    uint64_t word;
    memcpy(&word, key->bytes + i * sizeof(word), sizeof(word));
    h ^= word;
    h *= 0x100000001b3ull;
  }
  return h;
}
static code_db_ent *code_db_find(const sail_hash *key) {
  uint32_t i = (uint32_t)(code_db_hash(key) & (code_db_cap - 1));
  for (;;) {
    code_db_ent *e = &code_db[i];
    if (!e->used || evmsail_hash_equal(&e->key, key))
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
    if (otab[i].used) *code_db_find(&otab[i].key) = otab[i];
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
static int code_db_intern(const sail_hash *key, const uint8_t *src,
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
    e->key = *key;
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

/*
 * Optimized whole-witness-code lowering of analyze_code + code_db_store.
 * PUSH immediate bytes are skipped exactly as in sail/host/code.sail.
 * Amsterdam's EIP-8024 immediate byte is skipped only when valid; an invalid
 * immediate remains the next opcode.
 */
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later) {
  static const uint8_t empty = 0;
  if (len > UINT32_MAX || (len != 0 && src == NULL)) return false;
  if (len == 0) src = &empty;

  const uint64_t jumpdest_ref =
      len == 0 ? 0 : jumpdest_table_alloc_value(len);
  if (len != 0 && jumpdest_ref == 0) return false;

  if (len != 0) {
    const size_t base = (size_t)(jumpdest_ref - 1);
    uint64_t position = 0;
    while (position < len) {
      const uint8_t opcode = src[position];
      if (opcode == 0x5b)
        jumpdest_arena[base + (size_t)(position >> 6)] |=
            UINT64_C(1) << (position & 63);

      uint64_t step = 1;
      if (opcode >= 0x60 && opcode <= 0x7f) {
        step = (uint64_t)opcode - 0x5e;
      } else if (amsterdam_or_later &&
                 (opcode == 0xe6 || opcode == 0xe7 || opcode == 0xe8)) {
        const uint8_t immediate =
            position + 1 < len ? src[position + 1] : 0;
        const bool valid =
            opcode == 0xe8 ? (immediate <= 81 || immediate >= 128)
                           : (immediate <= 90 || immediate >= 128);
        if (valid) step = 2;
      }

      if (step >= len - position) break;
      position += step;
    }
  }

  sail_hash key = {{0}};
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(src, (size_t)len, &digest) != ZKVM_EOK)
    return false;
  memcpy(key.bytes, digest.data, sizeof(key.bytes));
  return code_db_intern(&key, src, (uint32_t)len, jumpdest_ref);
}

EVMSAIL_HASH_RETURN code_db_store_indexed_source(
    EVMSAIL_HASH_RESULT(result) uint64_t source_kind, uint64_t off,
    uint64_t len, uint64_t jumpdest_ref) {
  sail_hash key = {{0}};
  if (len > UINT32_MAX ||
      !jumpdest_table_matches(jumpdest_ref, (uint32_t)len)) {
    EVMSAIL_RETURN_HASH_BE_BYTES(result, key.bytes);
  }
  const uint8_t *src = NULL;
  uint64_t source_len = 0;
  if (!evmsail_resolve_byte_source(source_kind, off, len, &src, &source_len) ||
      !src || source_len != len) {
    EVMSAIL_RETURN_HASH_BE_BYTES(result, key.bytes);
  }
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(src, (size_t)len, &digest) == ZKVM_EOK)
    memcpy(key.bytes, digest.data, sizeof(key.bytes));
  if (!code_db_intern(&key, src, (uint32_t)len, jumpdest_ref))
    memset(&key, 0, sizeof(key));
  EVMSAIL_RETURN_HASH_BE_BYTES(result, key.bytes);
}

/* Intern the 23-byte EIP-7702 delegation designation 0xef0100 ++ addr under its
 * keccak hash with the explicit Sail analysis; return the codeHash. */
EVMSAIL_HASH_RETURN code_intern_indexed_delegation(
    EVMSAIL_HASH_RESULT(result) sail_address addr, uint64_t jumpdest_ref) {
  uint8_t b[23];
  b[0] = 0xef; b[1] = 0x01; b[2] = 0x00;
  evmsail_address_to_be_bytes(b + 3, addr);
  sail_hash key = {{0}};
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(b, sizeof(b), &digest) == ZKVM_EOK)
    memcpy(key.bytes, digest.data, sizeof(key.bytes));
  if (!code_db_intern(&key, b, (uint32_t)sizeof b, jumpdest_ref))
    memset(&key, 0, sizeof(key));
  EVMSAIL_RETURN_HASH_BE_BYTES(result, key.bytes);
}

/* ----------------------- code-hash lookup ------------------------------- */
/* Return the code span and its associated JUMPDEST table as one invariant.
 * code_glue.c assembles the generated Code result. */

static const code_db_ent *code_db_get(sail_hash h) {
  if (!code_db) return NULL;
  sail_hash key = h;
  const code_db_ent *e = code_db_find(&key);
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
