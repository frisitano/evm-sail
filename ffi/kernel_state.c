/* C-backed execution-time kernel collections for evm-sail.
 *
 * The EIP-2929 warm address/slot sets, the LOG series, and the call-frame
 * journal (undo log) were Sail registers holding mutable data buffers. They
 * now live here, behind
 * the abstract host interfaces declared in sail/host/state.sail and
 * sail/host/environment.sail. This is
 * a pure refactor: dedup, ordering, and call-frame revert semantics are
 * unchanged.
 *
 * Addresses/words/hashes cross the FFI as whole lbits (stored as 4 big-endian
 * 64-bit words, low 160 bits used for addresses); the refund word, counts,
 * tags, and bytes cross as mach_bits. Arrays grow on demand and are cleared
 * (count reset, allocation retained) at tx/world reset, matching the cached
 * per-frame arrays used elsewhere in the FFI. */
#include "kernel_state.h"
#include "lbits_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------- word helpers ------------------------------ */

/* one 256-bit value as 4 big-endian words (w[0] most significant) */
typedef struct { uint64_t w[4]; } word256;

static inline word256 lb_word(const lbits v) {
  word256 r;
  lbits_to_be_words4(r.w, v);
  return r;
}
static inline void word_out(lbits *rop, const word256 *v) {
  be_words4_to_lbits(rop, v->w);
}
static inline int word_eq(const word256 *a, const word256 *b) {
  return memcmp(a->w, b->w, sizeof a->w) == 0;
}

/* ---------------------------- address vector ---------------------------- */
/* order-insensitive membership set backed by a flat array (contains is a
 * linear scan, as the prior Sail linked list was); used for warm addresses. */
typedef struct { word256 *v; uint32_t n, cap; } addr_vec;

static int av_reserve(addr_vec *m, uint32_t need) {
  if (need <= m->cap) return 1;
  uint32_t cap = m->cap ? m->cap * 2 : 16;
  while (cap < need) cap *= 2;
  word256 *nv = (word256 *)realloc(m->v, cap * sizeof(word256));
  if (!nv) return 0;
  m->v = nv;
  m->cap = cap;
  return 1;
}
static int av_find(const addr_vec *m, const word256 *a) {
  for (uint32_t i = 0; i < m->n; i++)
    if (word_eq(&m->v[i], a)) return (int)i;
  return -1;
}
static void av_append(addr_vec *m, const word256 *a) {
  if (av_reserve(m, m->n + 1)) m->v[m->n++] = *a;
}
/* remove one occurrence (order-insensitive: swap with last) */
static void av_remove_once(addr_vec *m, const word256 *a) {
  int i = av_find(m, a);
  if (i >= 0) m->v[i] = m->v[--m->n];
}

/* ------------------------------ warm sets ------------------------------- */

static addr_vec warm_addr;

typedef struct { word256 a; word256 s; } slot_key;
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
static int sv_find(const slot_vec *m, const word256 *a, const word256 *s) {
  for (uint32_t i = 0; i < m->n; i++)
    if (word_eq(&m->v[i].a, a) && word_eq(&m->v[i].s, s)) return (int)i;
  return -1;
}

unit warm_reset(const unit u) {
  (void)u;
  warm_addr.n = 0;
  warm_slot.n = 0;
  return UNIT;
}
bool warm_addr_touch(const lbits a) {
  word256 k = lb_word(a);
  if (av_find(&warm_addr, &k) >= 0) return true;
  av_append(&warm_addr, &k);
  return false;
}
unit warm_addr_remove(const lbits a) {
  word256 k = lb_word(a);
  av_remove_once(&warm_addr, &k);
  return UNIT;
}
bool warm_slot_touch(const lbits a, const lbits s) {
  word256 ka = lb_word(a), ks = lb_word(s);
  if (sv_find(&warm_slot, &ka, &ks) >= 0) return true;
  if (sv_reserve(&warm_slot, warm_slot.n + 1)) {
    warm_slot.v[warm_slot.n].a = ka;
    warm_slot.v[warm_slot.n].s = ks;
    warm_slot.n++;
  }
  return false;
}
unit warm_slot_remove(const lbits a, const lbits s) {
  word256 ka = lb_word(a), ks = lb_word(s);
  int i = sv_find(&warm_slot, &ka, &ks);
  if (i >= 0) warm_slot.v[i] = warm_slot.v[--warm_slot.n];
  return UNIT;
}

/* ------------------------ ancestor header hashes ------------------------ */
/* Fixed 256-slot table (the protocol's BLOCKHASH depth), distance-indexed:
 * slot j = keccak of the (j+1)-blocks-back witness header. Writes come from
 * the witness-header pass; reads are guarded Sail-side by k_n_headers, so
 * stale slots are unreachable and no reset is needed. */
static word256 hdrhash[256];

unit ancestor_hash_write(uint64_t j, const lbits h) {
  if (j < 256) hdrhash[j] = lb_word(h);
  return UNIT;
}
void ancestor_hash_read(lbits *rop, uint64_t j) {
  static const word256 zero = {{0, 0, 0, 0}};
  word_out(rop, j < 256 ? &hdrhash[j] : &zero);
}

/* -------------------------------- logs ---------------------------------- */

typedef struct {
  word256 a;         /* emitter address                */
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
  return UNIT;
}
/* per-tx reset: records + topics only. The DATA arena persists across the
 * block -- receipt-held LogEntry slices (LogDataSource) reference it until
 * block validation; the full logs_reset above runs per case/block. */
unit logs_tx_reset(const unit u) {
  (void)u;
  logs_n = 0;
  topics_n = 0;
  return UNIT;
}
unit log_begin(const lbits a) {
  if (logrec_reserve(logs_n + 1)) {
    log_rec *r = &logs[logs_n++];
    r->a = lb_word(a);
    r->topic_off = topics_n;
    r->topic_cnt = 0;
    r->data_off = data_n;
    r->data_len = 0;
  }
  return UNIT;
}
unit log_add_topic(const lbits t) {
  if (logs_n && topics_reserve(topics_n + 1)) {
    log_topics[topics_n++] = lb_word(t);
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
/* drop the most recent record and truncate its topics/data back off the ends */
unit log_drop_last(const unit u) {
  (void)u;
  if (logs_n) {
    log_rec *r = &logs[logs_n - 1];
    topics_n = r->topic_off;
    data_n = r->data_off;
    logs_n--;
  }
  return UNIT;
}
uint64_t log_count(const unit u) { (void)u; return logs_n; }
void log_addr(lbits *rop, uint64_t i) {
  static const word256 zero = {{0, 0, 0, 0}};
  word_out(rop, (i < logs_n) ? &logs[i].a : &zero);
}
uint64_t log_topic_count(uint64_t i) { return (i < logs_n) ? logs[i].topic_cnt : 0; }
void log_topic(lbits *rop, uint64_t i, uint64_t j) {
  static const word256 zero = {{0, 0, 0, 0}};
  const word256 *t = &zero;
  if (i < logs_n && j < logs[i].topic_cnt) t = &log_topics[logs[i].topic_off + j];
  word_out(rop, t);
}
uint64_t log_data_len(uint64_t i) { return (i < logs_n) ? logs[i].data_len : 0; }
uint64_t log_data_off(uint64_t i) { return (i < logs_n) ? logs[i].data_off : 0; }

/* bounds-checked view of the log-data arena (the LogDataSource resolver) */
const uint8_t *log_data_region(uint64_t off, uint64_t len) {
  static const uint8_t empty = 0;
  if (off > data_n || len > data_n - off) return NULL;
  return len ? log_data + off : &empty;
}

/* ------------------------------- journal -------------------------------- */
/* tag values: C-internal row tags. The Sail side no longer sees them (its
 * journal boundary is journal_push/journal_pop over whole JEntry values);
 * ffi/journal_glue.c mirrors this enum (GJT_*) for its (en/de)coding. */
enum {
  JT_EMPTY = 0, JT_TRAN = 1, JT_WARMA = 2,
  JT_WARMS = 3, JT_LOG = 4, JT_REFUND = 5
};

typedef struct {
  uint32_t tag;
  word256 a;
  word256 w0;
  word256 w1;
  uint64_t n64;
} jentry;

static jentry *jrn;
static uint32_t jrn_n, jrn_cap;

static jentry *jrn_push(uint32_t tag) {
  if (jrn_n >= jrn_cap) {
    uint32_t cap = jrn_cap ? jrn_cap * 2 : 256;
    jentry *nv = (jentry *)realloc(jrn, cap * sizeof(jentry));
    if (!nv) return NULL;
    jrn = nv;
    jrn_cap = cap;
  }
  jentry *e = &jrn[jrn_n++];
  memset(e, 0, sizeof *e);
  e->tag = tag;
  return e;
}

unit journal_reset(const unit u) { (void)u; jrn_n = 0; return UNIT; }
uint64_t journal_len(const unit u) { (void)u; return jrn_n; }
unit journal_push_tran(const lbits a, const lbits slot, const lbits val) {
  jentry *e = jrn_push(JT_TRAN);
  if (e) {
    e->a = lb_word(a);
    e->w0 = lb_word(slot);
    e->w1 = lb_word(val);
  }
  return UNIT;
}
unit journal_push_warma(const lbits a) {
  jentry *e = jrn_push(JT_WARMA);
  if (e) e->a = lb_word(a);
  return UNIT;
}
unit journal_push_warms(const lbits a, const lbits slot) {
  jentry *e = jrn_push(JT_WARMS);
  if (e) {
    e->a = lb_word(a);
    e->w0 = lb_word(slot);
  }
  return UNIT;
}
unit journal_push_log(const unit u) { (void)u; jrn_push(JT_LOG); return UNIT; }
unit journal_push_refund(uint64_t old) {
  jentry *e = jrn_push(JT_REFUND);
  if (e) e->n64 = old;
  return UNIT;
}
uint64_t journal_top_tag(const unit u) { (void)u; return jrn_n ? jrn[jrn_n - 1].tag : JT_EMPTY; }
unit journal_drop_top(const unit u) { (void)u; if (jrn_n) jrn_n--; return UNIT; }

static const jentry *jrn_top(void) {
  static const jentry zero = {0};
  return jrn_n ? &jrn[jrn_n - 1] : &zero;
}
void journal_top_addr(lbits *rop, const unit u) { (void)u; word_out(rop, &jrn_top()->a); }
void journal_top_slot(lbits *rop, const unit u) { (void)u; word_out(rop, &jrn_top()->w0); }
void journal_top_val(lbits *rop, const unit u) { (void)u; word_out(rop, &jrn_top()->w1); }
uint64_t journal_top_refund(const unit u) { (void)u; return jrn_top()->n64; }
