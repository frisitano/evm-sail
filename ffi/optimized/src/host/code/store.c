/* C-backed CODE for the evm-sail model.
 *
 * Account code is written rarely (witness seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. The subsystem owns three tables:
 *
 *   1. Code-region construction resolves stable bytes once and exposes the
 *      pointer directly to optimized Sail. CodeTable is only the
 *      content-addressed store; its open-addressed bucket array maps hashes
 *      to internal CodeIds, mirroring the
 *      account/storage table layout. Delegation designators are classified
 *      once at store time so the per-CALL delegation probe never re-reads
 *      code bytes.
 *   2. PUSH-aware analysis receives a preallocated byte-addressed JUMPDEST
 *      span directly. Stored Code records retain that pointer directly.
 *   3. CodeArena owns both copied code and JUMPDEST bytes. Copied code is for
 *      bytes with no stable backing of their own: deploys
 *      sourced from mutable memory or reusable output and synthesized
 *      delegation designators. Witness code backed by the immutable
 *      stateless input is referenced in place and never copied, while every
 *      nonempty code row receives a JUMPDEST byte span.
 *
 * CodeId zero and an empty bucket are internal sentinels; real rows start at
 * one and never cross the generated-model boundary.
 * Sail's single frame_code Code register is the complete active
 * executable state, and the Sail account store remains the authoritative
 * account-code value; this is the execution mirror. */
#include "evmsail/prelude.h"
#include "host/code/store.h"
#include "evmsail/host/region_access.h"
#include "primitives/hash.h"
#include "workspace.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Dense one-based row identifiers. Zero is the permanent miss/empty sentinel
 * and must never index table state. */
typedef uint32_t CodeId;
enum { CODE_ID_NONE = 0 };

/* ----------------------------- code arena ------------------------------ */

/* Append-only workspace spans owned by the code subsystem. Code bytes are
 * used only when the source has no stable backing of its own. JUMPDEST bytes
 * contain one zero/one flag per byte of every analyzed code row. */
typedef struct {
  /* Copied executable-code storage. */
  uint8_t *bytes;
  size_t bytes_len;
  /* Byte-addressed JUMPDEST flag storage. */
  uint8_t *jumpdests;
  size_t jumpdests_len;
} CodeArena;

static CodeArena code_arena;

/* Copies bytes into stable arena storage, or NULL when exhausted. */
static const uint8_t *code_arena_append(const uint8_t *src, uint64_t len) {
  if (len > GUEST_CODE_BYTES - code_arena.bytes_len) return NULL;
  uint8_t *dst = code_arena.bytes + code_arena.bytes_len;
  memcpy(dst, src, (size_t)len);
  code_arena.bytes_len += (size_t)len;
  return dst;
}

/* ------------------------------ code table ----------------------------- */

/* One immutable code-region row.
 *
 * bytes is the location resolved once at row creation (stateless input or
 * arena; the row does not record which). A row begins anonymous -- a stable
 * executable region with no content identity -- and becomes hashed when
 * code_db_store_row content-addresses it into the bucket index. code_hash
 * and the cached EIP-7702 designator classification
 * (0xef0100 ++ address) are valid only for hashed rows; the per-CALL
 * delegation probe reads only the cached fields. */
typedef struct {
  /* Byte location resolved when the row was created. */
  const uint8_t *bytes;
  /* Content key: the keccak-256 digest of the code bytes. */
  Hash32 code_hash;
  /* Code length in bytes; real rows are nonzero. */
  uint32_t len;
  /* Completed byte-addressed JUMPDEST analysis for exactly these bytes. */
  uint8_t *jumpdests;
  /* Whether this row is content-addressed in the bucket index. */
  uint8_t hashed;
  /* Whether the bytes are a well-formed EIP-7702 delegation designator. */
  uint8_t is_delegation;
  /* Delegate decoded from the designator; zero unless is_delegation. */
  Address delegate;
} CodeEntry;

/* Complete code-table allocation, backed by the guest workspace.
 *
 * entries is CodeId-indexed and includes sentinel row zero. buckets is a
 * power-of-two open-addressed hash index holding hashed CodeIds;
 * CODE_ID_NONE marks an empty bucket. Rows are strictly fewer than buckets,
 * so probing always terminates at a match or an empty bucket. */
typedef struct {
  /* CodeId-indexed immutable rows; includes sentinel row zero. */
  CodeEntry *entries;
  /* Open-addressed code-hash index; zero denotes an empty bucket. */
  CodeId *buckets;
  /* Next CodeId and number of rows including sentinel row zero. */
  uint32_t count;
  /* Maximum count including sentinel row zero. */
  uint32_t capacity;
  /* Power-of-two bucket count. */
  uint32_t bucket_count;
  /* bucket_count - 1, used for wraparound instead of division. */
  uint32_t bucket_mask;
} CodeTable;

_Static_assert(GUEST_CODE_INDEX_BUCKETS >= 2u * GUEST_CODE_DB_ENTRIES,
               "code bucket index must stay under half load");
_Static_assert((GUEST_CODE_INDEX_BUCKETS &
                (GUEST_CODE_INDEX_BUCKETS - 1u)) == 0,
               "code bucket count must be a power of two");

static CodeTable code_table = {
    .count = 1u,
    .capacity = GUEST_CODE_DB_ENTRIES + 1u,
    .bucket_count = GUEST_CODE_INDEX_BUCKETS,
    .bucket_mask = GUEST_CODE_INDEX_BUCKETS - 1u,
};

void code_workspace_bind(void) {
  WORKSPACE_BIND(code_arena.bytes, GUEST_CODE_BYTES);
  WORKSPACE_BIND(code_arena.jumpdests, GUEST_JUMPDEST_BYTES);
  WORKSPACE_BIND(code_table.entries, GUEST_CODE_DB_ENTRIES + 1u);
  WORKSPACE_BIND(code_table.buckets, GUEST_CODE_INDEX_BUCKETS);
}

uint8_t *jumpdest_table_alloc(struct CodeRegionSliceFields code) {
  const uint64_t len = code.len;
  if (len == 0) return NULL;
  if (!code.bytes || len > UINT32_MAX ||
      len > GUEST_JUMPDEST_BYTES - code_arena.jumpdests_len)
    return NULL;
  uint8_t *table = code_arena.jumpdests + code_arena.jumpdests_len;
  code_arena.jumpdests_len += (size_t)len;
  return table;
}

bool jumpdest_table_mark(uint8_t *table, uint32_t code_len,
                         uint32_t position) {
  if (!table || position >= code_len) return false;
  table[position] = 1;
  return true;
}

bool jumpdest_ref_contains(uint8_t *table, uint32_t code_len,
                           uint32_t i) {
  return table != NULL && i < code_len && table[i] != 0;
}

/* Copies unstable source bytes into the stable code arena. */
uint8_t *code_region_intern_copy(const uint8_t *src, uint64_t len) {
  if (!src || !len || len > UINT32_MAX) return NULL;
  return (uint8_t *)(uintptr_t)code_arena_append(src, len);
}

static uint64_t code_db_hash(const Hash32 *key) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (size_t i = 0; i < sizeof(key->bytes); ++i) {
    h ^= key->bytes[i];
    h *= 0x100000001b3ull;
  }
  return h;
}

/* Finds the bucket holding this hash's CodeId, or the empty bucket where it
 * would be inserted. Rows never fill the bucket array, so an unbounded probe
 * chain is a corrupted index. */
static uint32_t code_bucket_locate(const Hash32 *key) {
  uint32_t bucket = (uint32_t)(code_db_hash(key) & code_table.bucket_mask);
  for (uint32_t probes = 0; probes < code_table.bucket_count; probes++) {
    const CodeId id = code_table.buckets[bucket];
    if (id == CODE_ID_NONE) return bucket;
    if (id >= code_table.count) GUEST_ABORT();
    if (hash_equal(&code_table.entries[id].code_hash, key)) return bucket;
    bucket = (bucket + 1u) & code_table.bucket_mask;
  }
  GUEST_ABORT();
}

static const CodeEntry *code_db_get(const Hash32 *key) {
  const CodeId id = code_table.buckets[code_bucket_locate(key)];
  return id == CODE_ID_NONE ? NULL : &code_table.entries[id];
}

/* Reuse-layer / test-harness reset: drop every stored code so a subsequent run
 * starts with an EMPTY code DB. Content-addressed accumulation is otherwise
 * safe, but a warm process reusing this DB across fixtures would let a code
 * blob registered by one fixture satisfy a later negative "code missing" test
 * (the witness deliberately omits that code, expecting valid=false). The model
 * never calls this; it exists for the in-process native harness whose
 * clear_memory wipes state between fixtures. Every arena retains its
 * workspace binding and resets only cursors. It clears the live code rows and
 * JUMPDEST-byte prefix so their next use has the same zero-start contract as a
 * fresh guest, without touching the rest of the 256 MiB native workspace. */
unit code_db_reset(const unit u) {
  (void)u;
  memset(code_table.buckets, 0,
         (size_t)code_table.bucket_count * sizeof(*code_table.buckets));
  if (code_table.count > 1u)
    memset(code_table.entries + 1u, 0,
           (size_t)(code_table.count - 1u) * sizeof(*code_table.entries));
  if (code_arena.jumpdests_len != 0)
    memset(code_arena.jumpdests, 0, code_arena.jumpdests_len);
  code_table.count = 1u;
  code_arena.bytes_len = 0;
  code_arena.jumpdests_len = 0;
  return UNIT;
}

static bool hash32_is_zero(const Hash32 *h) {
  uint8_t combined = 0;
  for (size_t i = 0; i < sizeof(h->bytes); ++i) combined |= h->bytes[i];
  return combined == 0;
}

static bool keccak_bytes(const uint8_t *src, size_t len, Hash32 *key) {
  _Static_assert(sizeof(*key) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  return zkvm_keccak256(src, len,
                        (zkvm_keccak256_hash *)(void *)key) == ZKVM_EOK;
}

/* Content-address direct code and JUMPDEST spans. CodeId is allocated only
 * for a new hash-table row and stays private to this file. */
static Hash32 code_db_store_view(const uint8_t *bytes, uint64_t len,
                                 uint8_t *jumpdests) {
  static const uint8_t empty = 0;
  Hash32 key = {{0}};
  if (len == 0) {
    if (bytes != NULL || jumpdests != NULL || !keccak_bytes(&empty, 0, &key))
      return (Hash32){{0}};
    return key;
  }
  if (!bytes || !jumpdests || len > UINT32_MAX ||
      !keccak_bytes(bytes, (size_t)len, &key))
    return key;

  const uint32_t bucket = code_bucket_locate(&key);
  const CodeId existing_id = code_table.buckets[bucket];
  if (existing_id != CODE_ID_NONE) {
    const CodeEntry *e = &code_table.entries[existing_id];
    if (e->len != len || memcmp(e->bytes, bytes, (size_t)len) != 0 ||
        memcmp(e->jumpdests, jumpdests, (size_t)len) != 0)
      return (Hash32){{0}};
    return key;
  }
  if (code_table.count >= code_table.capacity) return (Hash32){{0}};
  const CodeId id = code_table.count++;
  CodeEntry *row = &code_table.entries[id];
  *row = (CodeEntry){
      .bytes = bytes,
      .code_hash = key,
      .len = (uint32_t)len,
      .jumpdests = jumpdests,
      .hashed = 1,
  };
  row->is_delegation = len == 23 && bytes[0] == 0xef &&
                       bytes[1] == 0x01 && bytes[2] == 0x00;
  if (row->is_delegation)
    row->delegate = address_from_be_bytes(bytes + 3);
  code_table.buckets[bucket] = id;
  return key;
}

Hash32 code_db_store_indexed(struct CodeFields code) {
  return code_db_store_view(code.bytes, code.len, code.jumpdests);
}

/*
 * Optimized whole-witness-code lowering of analyze_code + code_db_store.
 * PUSH immediate bytes are skipped exactly as in sail/host/code.sail.
 * Amsterdam's EIP-8024 immediate byte is skipped only when valid; an invalid
 * immediate remains the next opcode.
 */
uint8_t *code_db_analyze_region(struct CodeRegionSliceFields code,
                                bool amsterdam_or_later) {
  const uint64_t len = code.len;
  if (len == 0) return NULL;
  if (len > UINT32_MAX || !code.bytes) return NULL;
  uint8_t *jumpdests = jumpdest_table_alloc(code);
  if (!jumpdests) return NULL;

  const uint8_t *src = code.bytes;
  uint64_t position = 0;
  while (position < len) {
    const uint8_t opcode = src[position];
    if (opcode == 0x5b)
      jumpdests[position] = 1;

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

  return jumpdests;
}

/* Whole-operation witness seeding: analyze stable input-backed bytes and
 * content-address them in place. */
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later) {
  if (len > UINT32_MAX || (len != 0 && src == NULL)) return false;
  if (len == 0) return true;
  const struct CodeRegionSliceFields code = {
      .bytes = (uint8_t *)(uintptr_t)src,
      .len = len,
  };
  uint8_t *jumpdests = code_db_analyze_region(code, amsterdam_or_later);
  if (!jumpdests) return false;
  const Hash32 key = code_db_store_view(src, len, jumpdests);
  return !hash32_is_zero(&key);
}

/* ----------------------- code-hash lookup ------------------------------- */
/* Return the code region and its associated JUMPDEST table as one invariant.
 * the selected model code adapter assembles the generated Code result. */

bool code_db_lookup_view(Hash32 h, const uint8_t **bytes, uint64_t *len,
                         uint8_t **jumpdests) {
  const CodeId id = code_table.buckets[code_bucket_locate(&h)];
  if (id == CODE_ID_NONE) return false;
  const CodeEntry *e = &code_table.entries[id];
  if (bytes) *bytes = e->bytes;
  if (len) *len = e->len;
  if (jumpdests) *jumpdests = e->jumpdests;
  return true;
}

/* EIP-7702 delegation probe in one call (this runs on every CALL-family
 * target). The classification was computed when the row was stored, so the
 * probe is a table lookup with no byte access. The generated-model binding
 * pairs this flag with the nominal address. */
bool code_db_read_delegation_address(
    Address *address,
    Hash32 h) {
  const CodeEntry *e = code_db_get(&h);
  if (e && e->is_delegation) {
    *address = e->delegate;
    return true;
  }
  *address = (Address){{0}};
  return false;
}
