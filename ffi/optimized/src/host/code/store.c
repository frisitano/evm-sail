/* C-backed CODE for the evm-sail model.
 *
 * Account code is written rarely (witness seeding, CREATE deploys, EIP-7702
 * delegations) and executed constantly. The subsystem owns three tables:
 *
 *   1. CodeTable is the code-region and content-addressed store. A dense
 *      one-based CodeId names one immutable row holding the byte pointer
 *      resolved ONCE when the row is created; the Sail-visible
 *      CodeRegionSlice offset is exactly that id, so no access ever
 *      re-derives where the bytes live. A separate open-addressed bucket
 *      array maps code hashes to hashed CodeIds, mirroring the
 *      account/storage table layout. Delegation designators are classified
 *      once at store time so the per-CALL delegation probe never re-reads
 *      code bytes.
 *   2. JumpdestTable owns the packed PUSH-aware JUMPDEST bitmaps. A dense
 *      one-based JumpdestId names one row {word span, defining code length};
 *      the Sail-visible jump_table_index is exactly that id.
 *      Standard builds populate completed 256-bit chunks from the explicit
 *      Sail analysis; optimized witness indexing may scan the same bytes
 *      directly in C as one whole-operation lowering.
 *   3. CodeArena stores bytes with no stable backing of their own: deploys
 *      sourced from mutable memory or reusable output and synthesized
 *      delegation designators. Witness code backed by the immutable
 *      stateless input is referenced in place and never copied.
 *
 * CodeId zero, JumpdestId zero, and an empty bucket are sentinels; real rows
 * start at one. Sail's single frame_code Code register is the complete active
 * executable state, and the Sail account store remains the authoritative
 * account-code value; this is the execution mirror. */
#include "evmsail/prelude.h"
#include "host/code/store.h"
#include "evmsail/host/region_access.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Dense one-based row identifiers. Zero is the permanent miss/empty sentinel
 * and must never index table state. */
typedef uint32_t CodeId;
typedef uint32_t JumpdestId;
enum { CODE_ID_NONE = 0 };

/* -------------------------- jumpdest tables ---------------------------- */

/* One completed (or in-construction) JUMPDEST bitmap.
 *
 * code_len is the defining code length: every later access must present the
 * same length, which replaces span re-derivation against the whole word
 * arena with one equality check. The word count is derived from code_len.
 * Construction keeps unused tail bits zero: allocation zero-fills and chunk
 * stores reject bits at or beyond code_len. */
typedef struct {
  /* First word of this bitmap in the shared packed word arena. */
  uint32_t word_base;
  /* Code length the bitmap was allocated for; nonzero for real rows. */
  uint32_t code_len;
} JumpdestEntry;

/* Complete JUMPDEST subsystem allocation, backed by the guest workspace.
 * entries is JumpdestId-indexed and includes sentinel row zero. */
typedef struct {
  /* JumpdestId-indexed bitmap rows; includes sentinel row zero. */
  JumpdestEntry *entries;
  /* Packed bitmap word arena shared by all rows. */
  uint64_t *words;
  /* Next JumpdestId and number of rows including sentinel row zero. */
  uint32_t count;
  /* Maximum count including sentinel row zero. */
  uint32_t capacity;
  /* Number of live words in the shared arena. */
  uint32_t words_len;
} JumpdestTable;

static JumpdestTable jumpdest_table = {
    .count = 1u,
    .capacity = GUEST_JUMPDEST_TABLES + 1u,
};

static uint64_t jumpdest_word_count(uint64_t code_len) {
  return (code_len >> 6) + ((code_len & UINT64_C(63)) != 0);
}

/* Resolves a Sail-supplied reference against its claimed code length. The
 * reference is untrusted 64-bit input; a stale id or a length that does not
 * match the row's defining length is a miss, never an aliased bitmap. */
static const JumpdestEntry *jumpdest_row(uint64_t ref, uint64_t code_len) {
  if (ref == 0 || ref >= jumpdest_table.count) return NULL;
  const JumpdestEntry *row = &jumpdest_table.entries[(size_t)ref];
  if (code_len != row->code_len) return NULL;
  return row;
}

/* Reserve the exact table size before Sail starts analysis. The word span is
 * zeroed so chunks without JUMPDESTs need no writes. Exhaustion returns the
 * empty sentinel; Sail asserts on it. */
uint64_t jumpdest_table_alloc(uint32_t code_len) {
  if (!code_len) return 0;
  const uint64_t nwords = jumpdest_word_count(code_len);
  if (jumpdest_table.count >= jumpdest_table.capacity) return 0;
  if (nwords > GUEST_JUMPDEST_WORDS - jumpdest_table.words_len) return 0;
  const JumpdestId id = jumpdest_table.count;
  JumpdestEntry *row = &jumpdest_table.entries[id];
  row->word_base = jumpdest_table.words_len;
  row->code_len = (uint32_t)code_len;
  memset(jumpdest_table.words + jumpdest_table.words_len, 0,
         (size_t)nwords * sizeof(*jumpdest_table.words));
  jumpdest_table.words_len += (uint32_t)nwords;
  jumpdest_table.count++;
  return id;
}

/* Store one completed Sail chunk. Chunk bit zero describes the first byte in
 * the corresponding 256-byte code span, hence little-endian limb order. Bits
 * at or beyond code_len are rejected to preserve the zero-tail invariant. */
bool jumpdest_table_store_chunk(uint64_t ref, uint32_t code_len,
                                uint32_t chunk_index, U256 chunk) {
  const JumpdestEntry *row = jumpdest_row(ref, code_len);
  if (!row) return false;
  const uint64_t nwords = jumpdest_word_count(row->code_len);
  const uint64_t first = chunk_index * 4;
  if (first >= nwords) return false;

  uint64_t words[4];
  sail_word_to_le_words4(words, chunk);
  uint64_t count = nwords - first;
  if (count > 4) count = 4;
  for (uint64_t i = count; i < 4; i++)
    if (words[i] != 0) return false;
  const uint32_t used = row->code_len & 63u;
  if (first + count == nwords && used && (words[count - 1] >> used) != 0)
    return false;
  memcpy(jumpdest_table.words + row->word_base + (size_t)first, words,
         (size_t)count * sizeof(*words));
  return true;
}

bool jumpdest_ref_contains(uint64_t ref, uint32_t code_len, uint32_t i) {
  if (i >= code_len) return false;
  const JumpdestEntry *row = jumpdest_row(ref, code_len);
  if (!row) return false;
  return ((jumpdest_table.words[row->word_base + (size_t)(i >> 6)] >>
           (i & 63)) &
          UINT64_C(1));
}

/* ----------------------------- code arena ------------------------------ */

/* Append-only bytes owned by the code subsystem. Rows point here whenever
 * the source bytes have no stable backing of their own: deploys sourced from
 * mutable memory or reusable output, and synthesized delegation
 * designators. */
typedef struct {
  /* Workspace-backed byte storage. */
  uint8_t *bytes;
  /* Live length; storage beyond it is unowned. */
  size_t len;
} CodeArena;

static CodeArena code_arena;

/* Copies bytes into stable arena storage, or NULL when exhausted. */
static const uint8_t *code_arena_append(const uint8_t *src, uint64_t len) {
  if (len > GUEST_CODE_BYTES - code_arena.len) return NULL;
  uint8_t *dst = code_arena.bytes + code_arena.len;
  memcpy(dst, src, (size_t)len);
  code_arena.len += (size_t)len;
  return dst;
}

/* ------------------------------ code table ----------------------------- */

/* One immutable code-region row.
 *
 * bytes is the location resolved once at row creation (stateless input or
 * arena; the row does not record which). A row begins anonymous -- a stable
 * executable region with no content identity -- and becomes hashed when
 * code_db_store_row content-addresses it into the bucket index. code_hash,
 * jumpdests, and the cached EIP-7702 designator classification
 * (0xef0100 ++ address) are valid only for hashed rows; the per-CALL
 * delegation probe reads only the cached fields. */
typedef struct {
  /* Byte location resolved when the row was created. */
  const uint8_t *bytes;
  /* Content key: the keccak-256 digest of the code bytes. */
  Hash32 code_hash;
  /* Code length in bytes; real rows are nonzero. */
  uint32_t len;
  /* Completed JUMPDEST analysis for exactly these bytes. */
  JumpdestId jumpdests;
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
  WORKSPACE_BIND(jumpdest_table.entries, GUEST_JUMPDEST_TABLES + 1u);
  WORKSPACE_BIND(jumpdest_table.words, GUEST_JUMPDEST_WORDS);
  WORKSPACE_BIND(code_arena.bytes, GUEST_CODE_BYTES);
  WORKSPACE_BIND(code_table.entries, GUEST_CODE_DB_ENTRIES + 1u);
  WORKSPACE_BIND(code_table.buckets, GUEST_CODE_INDEX_BUCKETS);
}

/* Registers stable bytes as a new anonymous code region and returns its
 * CodeId -- the value that crosses the Sail boundary as the slice offset.
 * Zero reports exhaustion or an invalid span. */
uint64_t code_region_register(const uint8_t *bytes, uint64_t len) {
  if (!bytes || !len || len > UINT32_MAX) return 0;
  if (code_table.count >= code_table.capacity) return 0;
  const CodeId id = code_table.count;
  CodeEntry *row = &code_table.entries[id];
  memset(row, 0, sizeof(*row));
  row->bytes = bytes;
  row->len = (uint32_t)len;
  code_table.count++;
  return id;
}

/* Copies unstable source bytes into the arena and registers the copy. */
uint64_t code_region_intern_copy(const uint8_t *src, uint64_t len) {
  if (!src || !len || len > UINT32_MAX) return 0;
  const uint8_t *stable = code_arena_append(src, len);
  if (!stable) return 0;
  return code_region_register(stable, len);
}

/* Resolve a code-region span: the offset is a CodeId and the length must be
 * the row's whole extent. */
int code_db_resolve_code(uint64_t off, uint64_t len,
                         const uint8_t **p, uint64_t *resolved_len) {
  static const uint8_t empty = 0;
  const uint8_t *src = &empty;
  if (len != 0) {
    if (off == 0 || off >= code_table.count) return 0;
    const CodeEntry *row = &code_table.entries[(size_t)off];
    if (len != row->len) return 0;
    src = row->bytes;
  }
  if (p) *p = src;
  if (resolved_len) *resolved_len = len;
  return 1;
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
 * never calls this; it exists for the in-process harness (test_utils.c) whose
 * clear_memory wipes state between fixtures. Every arena retains its
 * workspace binding and resets only cursors; the bucket index is the sole
 * storage that must be cleared. */
unit code_db_reset(const unit u) {
  (void)u;
  memset(code_table.buckets, 0,
         (size_t)code_table.bucket_count * sizeof(*code_table.buckets));
  code_table.count = 1u;
  jumpdest_table.count = 1u;
  jumpdest_table.words_len = 0;
  code_arena.len = 0;
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

/* Content-address an anonymous region under its keccak hash together with
 * the Sail-supplied analysis, returning that hash; the zero hash reports
 * failure. A hash already present must carry identical bytes and an
 * identical bitmap; the earlier row stays canonical and this row remains
 * anonymous. Empty code (region zero with the empty analysis) stores nothing
 * and returns KECCAK_EMPTY -- a codeless account carries that hash with no
 * table entry. */
Hash32 code_db_store_row(uint64_t region, uint64_t jumpdest_ref) {
  static const uint8_t empty = 0;
  Hash32 key = {{0}};
  if (region == 0) {
    if (jumpdest_ref != 0 || !keccak_bytes(&empty, 0, &key))
      return (Hash32){{0}};
    return key;
  }
  if (region >= code_table.count) return key;
  CodeEntry *row = &code_table.entries[(size_t)region];
  const JumpdestEntry *analysis = jumpdest_row(jumpdest_ref, row->len);
  if (!analysis) return key;
  if (!keccak_bytes(row->bytes, row->len, &key)) return (Hash32){{0}};

  const uint32_t bucket = code_bucket_locate(&key);
  const CodeId existing_id = code_table.buckets[bucket];
  if (existing_id != CODE_ID_NONE) {
    const CodeEntry *e = &code_table.entries[existing_id];
    const JumpdestEntry *stored = jumpdest_row(e->jumpdests, row->len);
    if (e->len != row->len || !stored ||
        memcmp(e->bytes, row->bytes, row->len) != 0 ||
        memcmp(jumpdest_table.words + stored->word_base,
               jumpdest_table.words + analysis->word_base,
               (size_t)jumpdest_word_count(row->len) *
                   sizeof(*jumpdest_table.words)) != 0)
      return (Hash32){{0}};
    return key;
  }
  row->code_hash = key;
  row->jumpdests = (JumpdestId)jumpdest_ref;
  row->hashed = 1;
  row->is_delegation = row->len == 23 && row->bytes[0] == 0xef &&
                       row->bytes[1] == 0x01 && row->bytes[2] == 0x00;
  if (row->is_delegation)
    row->delegate = address_from_be_bytes(row->bytes + 3);
  code_table.buckets[bucket] = (CodeId)region;
  return key;
}

/*
 * Optimized whole-witness-code lowering of analyze_code + code_db_store.
 * PUSH immediate bytes are skipped exactly as in sail/host/code.sail.
 * Amsterdam's EIP-8024 immediate byte is skipped only when valid; an invalid
 * immediate remains the next opcode.
 */
uint64_t code_db_analyze_bytes(const uint8_t *src, uint64_t len,
                               bool amsterdam_or_later) {
  static const uint8_t empty = 0;
  if (len > UINT32_MAX || (len != 0 && src == NULL)) return 0;
  if (len == 0) src = &empty;

  const uint64_t jumpdest_ref =
      len == 0 ? 0 : jumpdest_table_alloc(len);
  if (len != 0 && jumpdest_ref == 0) return 0;

  if (len != 0) {
    const JumpdestEntry *row = jumpdest_row(jumpdest_ref, len);
    if (!row) return 0;
    uint64_t *words = jumpdest_table.words + row->word_base;
    uint64_t position = 0;
    while (position < len) {
      const uint8_t opcode = src[position];
      if (opcode == 0x5b)
        words[(size_t)(position >> 6)] |= UINT64_C(1) << (position & 63);

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

  return jumpdest_ref;
}

/* Whole-operation witness seeding: analyze stable input-backed bytes and
 * content-address them in place. */
bool code_db_insert_analyzed_bytes(const uint8_t *src, uint64_t len,
                                   bool amsterdam_or_later) {
  if (len > UINT32_MAX || (len != 0 && src == NULL)) return false;
  const uint64_t jumpdest_ref =
      code_db_analyze_bytes(src, len, amsterdam_or_later);
  if (len == 0) return jumpdest_ref == 0;
  if (jumpdest_ref == 0) return false;
  const uint64_t region = code_region_register(src, len);
  if (region == 0) return false;
  const Hash32 key = code_db_store_row(region, jumpdest_ref);
  return !hash32_is_zero(&key);
}

/* Intern the 23-byte EIP-7702 delegation designation 0xef0100 ++ addr under its
 * keccak hash with the explicit Sail analysis; return the codeHash. */
Hash32 code_intern_indexed_delegation(
    Address addr, uint64_t jumpdest_ref) {
  uint8_t b[23];
  b[0] = 0xef; b[1] = 0x01; b[2] = 0x00;
  address_to_be_bytes(b + 3, addr);
  const uint64_t region = code_region_intern_copy(b, sizeof b);
  if (region == 0) return (Hash32){{0}};
  return code_db_store_row(region, jumpdest_ref);
}

/* ----------------------- code-hash lookup ------------------------------- */
/* Return the code region and its associated JUMPDEST table as one invariant.
 * the selected model code adapter assembles the generated Code result. */

bool code_db_lookup_indexed(Hash32 h, uint64_t *off, uint64_t *len,
                            uint64_t *jumpdest_ref) {
  const CodeId id = code_table.buckets[code_bucket_locate(&h)];
  if (id == CODE_ID_NONE) return false;
  const CodeEntry *e = &code_table.entries[id];
  *off = id;
  *len = e->len;
  *jumpdest_ref = e->jumpdests;
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
