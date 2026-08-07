/* C-backed execution-time kernel collections for evm-sail.
 *
 * The EIP-2929 warm address/slot sets, the LOG series, and semantic call-frame
 * checkpoints were Sail-visible mutable data structures. They now live here,
 * behind
 * the abstract host interfaces declared in sail/host/state.sail and
 * sail/host/environment.sail. This is
 * a pure refactor: dedup, ordering, and call-frame revert semantics are
 * unchanged.
 *
 * Addresses, words, and hashes cross through the selected standard/optimized
 * ABI and are stored without Sail runtime wrappers; counts, tags, and bytes
 * cross as scalars. Arrays grow on demand and are cleared
 * (count reset, allocation retained) at tx/world reset, matching the cached
 * per-frame arrays used elsewhere in the FFI. */
#include "kernel_state.h"
#include "capacity.h"
#include "hash_bytes.h"
#include "value_convert.h"
#include "state_db.h"
#include "transient_storage.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------- word helpers ------------------------------ */

/* one 256-bit value as 4 big-endian words (w[0] most significant) */
typedef struct { uint64_t w[4]; } word256;

typedef struct { uint8_t b[20]; } address160;

static inline word256 sail_word256(u256 v) {
  word256 r;
  sail_word_to_be_words4(r.w, v);
  return r;
}
static inline address160 sail_address160(fixed_bytes_20 v) {
  address160 r;
  evmsail_address_to_be_bytes(r.b, v);
  return r;
}
static inline int word_eq(const word256 *a, const word256 *b) {
  return memcmp(a->w, b->w, sizeof a->w) == 0;
}
static inline int address_eq(const address160 *a, const address160 *b) {
  return memcmp(a->b, b->b, sizeof a->b) == 0;
}

/* Warm-set insertions append their inverse operations to the private journal
 * declared below. Keeping these helpers private prevents any journal shape or
 * cursor from becoming part of the generated Sail ABI. */
static void journal_push_warm_address(const address160 *a);
static void journal_push_warm_slot(const address160 *a, const word256 *s);

/* ---------------------------- address vector ---------------------------- */
/* order-insensitive membership set backed by a flat array (contains is a
 * linear scan, as the prior Sail linked list was); used for warm addresses. */
typedef struct { address160 *v; uint32_t n, cap; } addr_vec;

static int av_reserve(addr_vec *m, uint32_t need) {
  if (need <= m->cap) return 1;
  uint32_t cap = m->cap ? m->cap * 2 : 16;
  while (cap < need) cap *= 2;
  address160 *nv = (address160 *)realloc(m->v, cap * sizeof(address160));
  if (!nv) return 0;
  m->v = nv;
  m->cap = cap;
  return 1;
}
static int av_find(const addr_vec *m, const address160 *a) {
  for (uint32_t i = 0; i < m->n; i++)
    if (address_eq(&m->v[i], a)) return (int)i;
  return -1;
}
/* remove one occurrence (order-insensitive: swap with last) */
static void av_remove_once(addr_vec *m, const address160 *a) {
  int i = av_find(m, a);
  if (i >= 0) m->v[i] = m->v[--m->n];
}

/* ------------------------------ warm sets ------------------------------- */

static addr_vec warm_addr;

typedef struct { address160 a; word256 s; } slot_key;
typedef struct { slot_key *v; uint32_t n, cap; } slot_vec;

static slot_vec warm_slot;

static int sv_reserve(slot_vec *m, uint32_t need) {
  if (need <= m->cap) return 1;
  uint32_t cap = m->cap ? m->cap * 2 : 16;
  while (cap < need) cap *= 2;
  slot_key *nv = (slot_key *)realloc(m->v, cap * sizeof(slot_key));
  if (!nv) return 0;
  m->v = nv;
  m->cap = cap;
  return 1;
}
static int sv_find(const slot_vec *m, const address160 *a, const word256 *s) {
  for (uint32_t i = 0; i < m->n; i++)
    if (address_eq(&m->v[i].a, a) && word_eq(&m->v[i].s, s)) return (int)i;
  return -1;
}

unit warm_reset(uint64_t current_transaction_epoch) {
  (void)current_transaction_epoch;
  warm_addr.n = 0;
  warm_slot.n = 0;
  return UNIT;
}
bool account_is_warm(fixed_bytes_20 a) {
  address160 k = sail_address160(a);
  return av_find(&warm_addr, &k) >= 0;
}

unit account_mark_warm(fixed_bytes_20 a) {
  address160 k = sail_address160(a);
  if (av_find(&warm_addr, &k) >= 0) return UNIT;
  if (!av_reserve(&warm_addr, warm_addr.n + 1)) abort();
  journal_push_warm_address(&k);
  warm_addr.v[warm_addr.n++] = k;
  return UNIT;
}

bool storage_is_warm(fixed_bytes_20 a, const u256 s) {
  address160 ka = sail_address160(a);
  word256 ks = sail_word256((s));
  return sv_find(&warm_slot, &ka, &ks) >= 0;
}

unit storage_mark_warm(fixed_bytes_20 a, const u256 s) {
  address160 ka = sail_address160(a);
  word256 ks = sail_word256((s));
  if (sv_find(&warm_slot, &ka, &ks) >= 0) return UNIT;
  if (!sv_reserve(&warm_slot, warm_slot.n + 1)) abort();
  journal_push_warm_slot(&ka, &ks);
  warm_slot.v[warm_slot.n].a = ka;
  warm_slot.v[warm_slot.n].s = ks;
  warm_slot.n++;
  return UNIT;
}

/* --------------------- EIP-7702 authority tracker ---------------------- */

typedef struct {
  address160 key;
  uint8_t used;
  uint8_t originally_delegated;
  uint8_t delegation_set;
} auth_tracker_entry;

static auth_tracker_entry *auth_tracker;
static uint32_t auth_tracker_cap;

static uint64_t auth_tracker_hash(const address160 *a) {
  uint64_t h = UINT64_C(1469598103934665603);
  for (unsigned i = 0; i < sizeof a->b; ++i) {
    h ^= a->b[i];
    h *= UINT64_C(1099511628211);
  }
  h ^= h >> 32;
  h *= UINT64_C(0xd6e8feb86659fd93);
  h ^= h >> 32;
  return h;
}

static auth_tracker_entry *auth_tracker_find(const address160 *key,
                                              int insert) {
  if (auth_tracker_cap == 0) return NULL;
  uint32_t slot = (uint32_t)auth_tracker_hash(key) & (auth_tracker_cap - 1);
  for (;;) {
    auth_tracker_entry *entry = &auth_tracker[slot];
    if (!entry->used) {
      if (!insert) return NULL;
      entry->used = 1;
      entry->key = *key;
      entry->originally_delegated = 0;
      entry->delegation_set = 0;
      return entry;
    }
    if (address_eq(&entry->key, key)) return entry;
    slot = (slot + 1) & (auth_tracker_cap - 1);
  }
}

unit authorization_tracker_reset(uint64_t count_hint) {
  uint64_t need = count_hint < 8 ? 16 : count_hint * 2;
  uint32_t cap = 16;
  while ((uint64_t)cap < need && cap <= UINT32_MAX / 2) cap *= 2;
  if ((uint64_t)cap < need) abort();
  if (cap > auth_tracker_cap) {
    auth_tracker_entry *next =
        (auth_tracker_entry *)realloc(auth_tracker, cap * sizeof(*next));
    if (next == NULL) abort();
    auth_tracker = next;
    auth_tracker_cap = cap;
  }
  memset(auth_tracker, 0, auth_tracker_cap * sizeof(*auth_tracker));
  return UNIT;
}

bool authorization_tracker_seen(fixed_bytes_20 authority) {
  address160 key = sail_address160(authority);
  return auth_tracker_find(&key, 0) != NULL;
}

bool authorization_tracker_originally_delegated(fixed_bytes_20 authority) {
  address160 key = sail_address160(authority);
  auth_tracker_entry *entry = auth_tracker_find(&key, 0);
  return entry != NULL && entry->originally_delegated != 0;
}

bool authorization_tracker_delegation_set(fixed_bytes_20 authority) {
  address160 key = sail_address160(authority);
  auth_tracker_entry *entry = auth_tracker_find(&key, 0);
  return entry != NULL && entry->delegation_set != 0;
}

unit authorization_tracker_commit(fixed_bytes_20 authority,
                                  bool originally_delegated,
                                  bool sets_delegation) {
  address160 key = sail_address160(authority);
  auth_tracker_entry *entry = auth_tracker_find(&key, 1);
  if (originally_delegated) entry->originally_delegated = 1;
  if (sets_delegation) entry->delegation_set = 1;
  return UNIT;
}

/* ------------------------ ancestor header hashes ------------------------ */
/* Fixed 256-slot table (the protocol's BLOCKHASH depth), distance-indexed:
 * slot j = keccak of the (j+1)-blocks-back witness header. Writes come from
 * the witness-header pass; reads are guarded Sail-side by k_n_headers, so
 * stale slots are unreachable and no reset is needed. */
static fixed_bytes_32 hdrhash[256];

unit ancestor_hash_write(uint64_t j, fixed_bytes_32 h) {
  if (j < 256) hdrhash[j] = h;
  return UNIT;
}
fixed_bytes_32 ancestor_hash_read( uint64_t j) {
  static const fixed_bytes_32 zero = {{0}};
  const fixed_bytes_32 *value = j < 256 ? &hdrhash[j] : &zero;
  return (*value);
}

/* -------------------------------- logs ---------------------------------- */

typedef struct {
  address160 a;      /* emitter address                */
  uint32_t topic_off; /* start index into log_topics    */
  uint32_t topic_cnt;
  uint32_t data_off;  /* start index into log_data      */
  uint32_t data_len;
} log_rec;

static log_rec *logs;
static uint32_t logs_n, logs_cap;
static word256 *log_topics;
static uint32_t topics_n, topics_cap;
static uint8_t *log_data;
static uint32_t data_n, data_cap;
static uint32_t tx_log_start;
static uint64_t block_logs_bloom[32];

static void journal_push_log(void);

bool log_data_configure_capacity(uint64_t capacity) {
  if (capacity > UINT32_MAX || capacity > SIZE_MAX) return false;
  if (capacity > data_cap) {
    uint8_t *next =
        (uint8_t *)realloc(log_data, (size_t)(capacity ? capacity : 1));
    if (!next) return false;
    log_data = next;
    data_cap = (uint32_t)capacity;
  }
  data_n = 0;
  return true;
}

static int logrec_reserve(uint32_t need) {
  if (need <= logs_cap) return 1;
  uint32_t cap = logs_cap ? logs_cap * 2 : 64;
  while (cap < need) cap *= 2;
  log_rec *nv = (log_rec *)realloc(logs, cap * sizeof(log_rec));
  if (!nv) return 0;
  logs = nv;
  logs_cap = cap;
  return 1;
}
static int topics_reserve(uint32_t need) {
  if (need <= topics_cap) return 1;
  uint32_t cap = topics_cap ? topics_cap * 2 : 128;
  while (cap < need) cap *= 2;
  word256 *nv = (word256 *)realloc(log_topics, cap * sizeof(word256));
  if (!nv) return 0;
  log_topics = nv;
  topics_cap = cap;
  return 1;
}
static int data_reserve(uint32_t need) {
  evmsail_capacity_observe(EVMSAIL_CAP_LOG_DATA_BYTES, need);
  if (need <= data_cap) return 1;
  uint32_t cap = data_cap ? data_cap * 2 : 1024;
  while (cap < need) cap *= 2;
  uint8_t *nv = (uint8_t *)realloc(log_data, cap);
  if (!nv) return 0;
  log_data = nv;
  data_cap = cap;
  return 1;
}

unit logs_reset(const unit u) {
  (void)u;
  logs_n = 0;
  topics_n = 0;
  data_n = 0;
  tx_log_start = 0;
  memset(block_logs_bloom, 0, sizeof(block_logs_bloom));
  return UNIT;
}
/* Per-tx reset starts a new view. Records, topics, and data all persist across
 * the block so receipts retain stable indices and source-backed payloads. */
unit logs_tx_reset(const unit u) {
  (void)u;
  tx_log_start = logs_n;
  return UNIT;
}
uint64_t logs_tx_start(const unit u) {
  (void)u;
  return tx_log_start;
}
uint64_t logs_tx_count(const unit u) {
  (void)u;
  return logs_n - tx_log_start;
}
unit log_begin(fixed_bytes_20 a) {
  if (logrec_reserve(logs_n + 1)) {
    log_rec *r = &logs[logs_n++];
    r->a = sail_address160(a);
    r->topic_off = topics_n;
    r->topic_cnt = 0;
    r->data_off = data_n;
    r->data_len = 0;
    journal_push_log();
  }
  return UNIT;
}
unit log_add_topic(u256 t) {
  if (logs_n && topics_reserve(topics_n + 1)) {
    log_topics[topics_n++] = sail_word256(t);
    logs[logs_n - 1].topic_cnt++;
  }
  return UNIT;
}
unit log_add_data_bulk(const uint8_t *p, uint64_t n) {
  if (logs_n && n && data_reserve(data_n + (uint32_t)n)) {
    memcpy(log_data + data_n, p, (size_t)n);
    data_n += (uint32_t)n;
    logs[logs_n - 1].data_len += (uint32_t)n;
  }
  return UNIT;
}
unit log_add_data_word(u256 value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  return log_add_data_bulk(bytes, sizeof(bytes));
}
void logs_revert_last(void) {
  if (logs_n == 0) abort();
  const log_rec *removed = &logs[--logs_n];
  topics_n = removed->topic_off;
  data_n = removed->data_off;
}
uint64_t log_count(const unit u) { (void)u; return logs_n; }
fixed_bytes_20 log_addr( uint64_t i) {
  static const address160 zero = {{0}};
  const address160 *value = (i < logs_n) ? &logs[i].a : &zero;
  return evmsail_address_from_be_bytes(value->b);
}
uint64_t log_topic_count(uint64_t i) { return (i < logs_n) ? logs[i].topic_cnt : 0; }
u256 log_topic( uint64_t i,
                               uint64_t j) {
  static const word256 zero = {{0, 0, 0, 0}};
  const word256 *t = &zero;
  if (i < logs_n && j < logs[i].topic_cnt) t = &log_topics[logs[i].topic_off + j];
  return (be_words4_to_sail_word(t->w));
}

static void logs_bloom_write_digest(uint8_t out[256],
                                    const zkvm_keccak256_hash *digest) {
  for (size_t i = 0; i < 3; ++i) {
    uint16_t bit =
        (uint16_t)(((uint16_t)(digest->data[2 * i] & 0x07) << 8) |
                   digest->data[2 * i + 1]);
    out[255 - (bit >> 3)] |= (uint8_t)(UINT8_C(1) << (bit & 7));
    const size_t word = bit >> 6;
    const uint64_t mask = UINT64_C(1) << (bit & 63);
    block_logs_bloom[word] |= mask;
  }
}

static bool logs_bloom_write_bytes(uint8_t out[256], const uint8_t *bytes,
                                   size_t length) {
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(bytes, length, &digest) != ZKVM_EOK)
    return false;
  logs_bloom_write_digest(out, &digest);
  return true;
}

unit evmsail_block_logs_bloom_reset(unit ignored) {
  (void)ignored;
  memset(block_logs_bloom, 0, sizeof(block_logs_bloom));
  return UNIT;
}

static fixed_bytes_256 logs_bloom_from_words(const uint64_t words[32]) {
  fixed_bytes_256 bloom;
  memcpy(bloom.bytes, words, sizeof(bloom.bytes));
  return bloom;
}

bool evmsail_receipt_logs_bloom_write(uint64_t start, uint64_t count,
                                      uint8_t out[256]) {
  if (start > logs_n || count > logs_n - start)
    return false;

  memset(out, 0, 256);
  for (uint64_t offset = 0; offset < count; ++offset) {
    const log_rec *record = &logs[start + offset];
    if (!logs_bloom_write_bytes(out, record->a.b, sizeof(record->a.b)))
      return false;
    for (uint32_t topic = 0; topic < record->topic_cnt; ++topic) {
      uint8_t bytes[32];
      const word256 *value = &log_topics[record->topic_off + topic];
      be_words4_to_be_bytes(bytes, value->w);
      if (!logs_bloom_write_bytes(out, bytes, sizeof(bytes)))
        return false;
    }
  }
  return true;
}

fixed_bytes_256 evmsail_block_logs_bloom(unit ignored) {
  (void)ignored;
  return logs_bloom_from_words(block_logs_bloom);
}

uint64_t log_data_len(uint64_t i) { return (i < logs_n) ? logs[i].data_len : 0; }
uint64_t log_data_off(uint64_t i) {
  if (i >= logs_n || logs[i].data_len == 0) return 0;
  return logs[i].data_off;
}

/* bounds-checked view of the log-data arena (the LogDataSource resolver) */
const uint8_t *log_data_region(uint64_t off, uint64_t len) {
  static const uint8_t empty = 0;
  if (off > data_n || len > data_n - off) return NULL;
  return len ? log_data + off : &empty;
}

const uint8_t *log_data_base(void) { return log_data; }
uint64_t log_data_length(void) { return data_n; }

/* ------------------------- private undo journal ------------------------- */
/* These tags are backend implementation details. The account and storage
 * entries point into their own typed, GMP-owning undo arenas; this compact
 * stream records only global reversal order and structural frame markers. */
enum {
  JT_TRAN = 1,
  JT_WARMA,
  JT_WARMS,
  JT_ACCOUNT_UNDO,
  JT_STORAGE_UNDO,
  JT_LOG,
  JT_FRAME_CHECKPOINT,
  JT_FRAME_COMMIT,
};

typedef struct {
  uint32_t tag;
  address160 a;
  word256 w0;
  word256 w1;
} jentry;

static jentry *jrn;
static uint32_t jrn_n, jrn_cap;

static jentry *jrn_push(uint32_t tag) {
  if (jrn_n >= jrn_cap) {
    uint32_t cap = jrn_cap ? jrn_cap * 2 : 256;
    jentry *nv = (jentry *)realloc(jrn, cap * sizeof(jentry));
    if (!nv) abort();
    jrn = nv;
    jrn_cap = cap;
  }
  jentry *e = &jrn[jrn_n++];
  memset(e, 0, sizeof *e);
  e->tag = tag;
  return e;
}

unit state_journal_push_transient(fixed_bytes_20 a, u256 slot,
                                  u256 prior) {
  jentry *e = jrn_push(JT_TRAN);
  e->a = sail_address160(a);
  e->w0 = sail_word256(slot);
  e->w1 = sail_word256(prior);
  return UNIT;
}

static void journal_push_warm_address(const address160 *a) {
  jentry *e = jrn_push(JT_WARMA);
  e->a = *a;
}

static void journal_push_warm_slot(const address160 *a, const word256 *s) {
  jentry *e = jrn_push(JT_WARMS);
  e->a = *a;
  e->w0 = *s;
}

void state_journal_push_account_undo(void) {
  (void)jrn_push(JT_ACCOUNT_UNDO);
}

void state_journal_push_storage_undo(void) {
  (void)jrn_push(JT_STORAGE_UNDO);
}

static void journal_push_log(void) { (void)jrn_push(JT_LOG); }

/* Return the innermost checkpoint that has not been paired with a commit.
 * Committed child markers remain in the stream so an enclosing revert still
 * observes and reverses their mutations. */
uint32_t state_journal_current_frame_marker(void) {
  uint32_t closed = 0;
  for (uint32_t i = jrn_n; i > 0; --i) {
    const uint32_t tag = jrn[i - 1].tag;
    if (tag == JT_FRAME_COMMIT) {
      closed++;
    } else if (tag == JT_FRAME_CHECKPOINT) {
      if (closed == 0) return i - 1;
      closed--;
    }
  }
  if (closed != 0) abort();
  return UINT32_MAX;
}

static uint32_t journal_require_open_frame(void) {
  const uint32_t marker = state_journal_current_frame_marker();
  if (marker == UINT32_MAX) abort();
  return marker;
}

static void journal_revert_to(uint32_t target) {
  if (target > jrn_n) abort();
  while (jrn_n > target) {
    const jentry *e = &jrn[--jrn_n];
    if (e->tag == JT_TRAN) {
      fixed_bytes_20 address = be_bytes_to_sail_address(e->a.b);
      u256 slot = be_words4_to_sail_word(e->w0.w);
      u256 prior = be_words4_to_sail_word(e->w1.w);
      transient_storage_restore(address, slot, prior);
    } else if (e->tag == JT_WARMA) {
      av_remove_once(&warm_addr, &e->a);
    } else if (e->tag == JT_WARMS) {
      int i = sv_find(&warm_slot, &e->a, &e->w0);
      if (i < 0) abort();
      warm_slot.v[i] = warm_slot.v[--warm_slot.n];
    } else if (e->tag == JT_ACCOUNT_UNDO) {
      acct_tx_revert_last();
    } else if (e->tag == JT_STORAGE_UNDO) {
      storage_tx_revert_last();
    } else if (e->tag == JT_LOG) {
      logs_revert_last();
    } else if (e->tag == JT_FRAME_CHECKPOINT ||
               e->tag == JT_FRAME_COMMIT) {
      /* Nested structural markers carry no semantic state of their own. */
    } else {
      abort();
    }
  }
}

/* -------------------------- state journal frames ----------------------- */

unit state_journal_reset(const unit u) {
  (void)u;
  jrn_n = 0;
  return UNIT;
}

unit state_journal_checkpoint(const unit u) {
  (void)u;
  (void)jrn_push(JT_FRAME_CHECKPOINT);
  return UNIT;
}

unit state_journal_revert(const unit u) {
  (void)u;
  const uint32_t marker = journal_require_open_frame();
  journal_revert_to(marker + 1);
  if (jrn_n != marker + 1 || jrn[marker].tag != JT_FRAME_CHECKPOINT)
    abort();
  jrn_n = marker;
  return UNIT;
}

unit state_journal_commit(const unit u) {
  (void)u;
  (void)journal_require_open_frame();
  (void)jrn_push(JT_FRAME_COMMIT);
  return UNIT;
}
