/* C-backed account and persistent-storage state database for evm-sail.
 *
 * Accounts are split into two ordered tables keyed by keccak256(address):
 *
 *   cache   - accounts materialized from fixtures or authenticated pre-state;
 *   updates - accounts mutated by execution.
 *
 * Point reads prefer updates and then fall back to cache. Stateless post-state
 * roots iterate only updates; native fixture roots iterate the cache/update
 * union with updates overriding cache rows.
 *
 * Persistent storage uses the same cache/update shape, keyed by
 * (keccak256(address), keccak256(slot)), with frame overlays for revert. */
#include "state_db.h"
#include "host_crypto.h"
#include "code_db.h"
#include "lbits_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* secure trie key derivation: keccak(address) for accounts, keccak(slot) for
 * storage. The account/storage hooks are keyed by raw (address[, slot]); this
 * computes the secure key on every call. NOTE: keccak is pure, so a preimage ->
 * hash memo would avoid re-hashing repeated touches of the same address/slot --
 * removed for now to keep the key path stateless; see TODO.md. */
static void secure_keccak(size_t pre_len, const lbits v, uint64_t out[4]) {
  uint8_t buf[32];
  lbits_to_be_bytes(buf, pre_len, v);
  host_keccak256_bytes(out, buf, pre_len);
}

/* h = keccak(address) -- the account secure trie key, BE words */
static void acct_secure_key(const lbits a, uint64_t h[4]) {
  secure_keccak(20, a, h);
}

#define ACCOUNT_INIT_CAP 64u

/* generic 4-word (big-endian order) comparator, shared by the account tables
   and the block-access-list accumulator below */
static int compare_u64x4(const uint64_t *a, const uint64_t *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

/* zeroed account fields, returned by the overlay getters on an absent row */
static const uint64_t account_zero_val[4] = {0, 0, 0, 0};

/* ======================================================================== */
/* EIP-7928 block access list accumulator.                                  */
/*                                                                          */
/* All recording is HARVESTED from the tx overlays at merge time by SAIL     */
/* (host/kernel/lifecycle.sail k_tx_merge decides the records); the           */
/* bal_note_* sinks below only append -- so there is no per-op BAL hook.      */
/* Records are keyed by                                                        */
/* keccak(address) (matching the overlays); the raw address the BAL sorts/     */
/* encodes by, and the read-only account set, come from eest_account at       */
/* serialize time (= the accounts touched on the witness path). Changes carry   */
/* the ambient block_access_index (bal_set_index: 0 pre-exec syscalls, i+1 for  */
/* user tx i, N+1 post). Reads survive revert (overlay membership persists).    */
/* bal_recompute_hash returns keccak(rlp(bal)) for block_access_list_hash.      */
/* ======================================================================== */

typedef struct { void *d; uint32_t n, cap, esz; } bal_vec;
static void *bal_vec_push(bal_vec *v) {
  if (v->cap < v->n + 1) {
    uint32_t nc = v->cap ? v->cap * 2 : 16;
    v->d = realloc(v->d, (size_t)nc * v->esz);
    v->cap = nc;
  }
  void *p = (char *)v->d + (size_t)v->n * v->esz;
  v->n++;
  return p;
}
static void bal_vec_clear(bal_vec *v) { free(v->d); v->d = NULL; v->n = 0; v->cap = 0; }

typedef struct { uint64_t ah[4]; uint64_t slot[4]; uint32_t idx; uint64_t val[4]; uint32_t seq; } bal_sto_rec;
typedef struct { uint64_t ah[4]; uint64_t slot[4]; } bal_read_rec;
typedef struct { uint64_t ah[4]; uint32_t idx; uint64_t val[4]; uint32_t seq; } bal_bal_rec;    /* balance */
typedef struct { uint64_t ah[4]; uint32_t idx; uint64_t val;    uint32_t seq; } bal_non_rec;    /* nonce   */
typedef struct { uint64_t ah[4]; uint32_t idx; uint64_t chash[4]; uint32_t seq; } bal_cod_rec;  /* code (bytes resolved at serialize) */

static bal_vec bal_sto  = {NULL, 0, 0, sizeof(bal_sto_rec)};
static bal_vec bal_rds  = {NULL, 0, 0, sizeof(bal_read_rec)};
static bal_vec bal_balc = {NULL, 0, 0, sizeof(bal_bal_rec)};
static bal_vec bal_nonc = {NULL, 0, 0, sizeof(bal_non_rec)};
static bal_vec bal_codc = {NULL, 0, 0, sizeof(bal_cod_rec)};
static uint32_t bal_seq = 0;
static uint32_t bal_index = 0;

unit bal_reset(const unit u) {
  (void)u;
  bal_vec_clear(&bal_sto); bal_vec_clear(&bal_rds); bal_vec_clear(&bal_balc);
  bal_vec_clear(&bal_nonc); bal_vec_clear(&bal_codc);
  bal_seq = 0; bal_index = 0;
  return UNIT;
}
unit bal_set_index(uint64_t n) { bal_index = (uint32_t)n; return UNIT; }

/* harvest helpers -- called from the overlay merges (state_db-internal) */
static void bal_add_storage_change(const uint64_t ah[4], const uint64_t slot[4], const uint64_t val[4]) {
  bal_sto_rec *r = (bal_sto_rec *)bal_vec_push(&bal_sto);
  memcpy(r->ah, ah, 32); memcpy(r->slot, slot, 32); memcpy(r->val, val, 32);
  r->idx = bal_index; r->seq = bal_seq++;
}
static void bal_add_storage_read(const uint64_t ah[4], const uint64_t slot[4]) {
  bal_read_rec *r = (bal_read_rec *)bal_vec_push(&bal_rds);
  memcpy(r->ah, ah, 32); memcpy(r->slot, slot, 32);
}
static void bal_add_balance_change(const uint64_t ah[4], const uint64_t val[4]) {
  bal_bal_rec *r = (bal_bal_rec *)bal_vec_push(&bal_balc);
  memcpy(r->ah, ah, 32); memcpy(r->val, val, 32);
  r->idx = bal_index; r->seq = bal_seq++;
}
static void bal_add_nonce_change(const uint64_t ah[4], uint64_t nonce) {
  bal_non_rec *r = (bal_non_rec *)bal_vec_push(&bal_nonc);
  memcpy(r->ah, ah, 32); r->val = nonce;
  r->idx = bal_index; r->seq = bal_seq++;
}
static void bal_add_code_change(const uint64_t ah[4], const uint64_t chash[4]) {
  bal_cod_rec *r = (bal_cod_rec *)bal_vec_push(&bal_codc);
  memcpy(r->ah, ah, 32); memcpy(r->chash, chash, 32);
  r->idx = bal_index; r->seq = bal_seq++;
}

/* Sail-facing EIP-7928 record sinks (host/kernel/lifecycle.sail k_tx_merge does the
   change/read detection; these only append records). Layouts match the row
   fields the serializer decodes: hashes/slots/storage values BE words,
   balance/code-hash LE words. */
unit bal_note_storage_change(const lbits a, const lbits slot, const lbits val) {
  uint64_t a4[4], s4[4], v4[4];
  secure_keccak(20, a, a4); lbits_to_be_words4(s4, slot); lbits_to_be_words4(v4, val);
  bal_add_storage_change(a4, s4, v4);
  return UNIT;
}
unit bal_note_storage_read(const lbits a, const lbits slot) {
  uint64_t a4[4], s4[4];
  secure_keccak(20, a, a4); lbits_to_be_words4(s4, slot);
  bal_add_storage_read(a4, s4);
  return UNIT;
}
unit bal_note_balance_change(const lbits a, const lbits val) {
  uint64_t a4[4], v4[4];
  secure_keccak(20, a, a4); lbits_to_le_words4(v4, val);
  bal_add_balance_change(a4, v4);
  return UNIT;
}
unit bal_note_nonce_change(const lbits a, uint64_t nonce) {
  uint64_t a4[4];
  secure_keccak(20, a, a4);
  bal_add_nonce_change(a4, nonce);
  return UNIT;
}
unit bal_note_code_change(const lbits a, const lbits chash) {
  uint64_t a4[4], c4[4];
  secure_keccak(20, a, a4); lbits_to_le_words4(c4, chash);
  bal_add_code_change(a4, c4);
  return UNIT;
}

#define STORAGE_INIT_CAP 64u

static int compare_words(const uint64_t *a, const uint64_t *b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static int storage_key_cmp(const uint64_t *aa, uint64_t ag, const uint64_t *as,
                           const uint64_t *ba, uint64_t bg, const uint64_t *bs) {
  int c = compare_words(aa, ba, 4);
  if (c) return c;
  c = compare_words(as, bs, 4);
  if (c) return c;
  if (ag < bg) return -1;
  if (ag > bg) return 1;
  return 0;
}

/* the secure storage key of raw (address, slot): (keccak(a), keccak(s)) */
static void storage_secure_key(const lbits a, const lbits s,
                               uint64_t slot[4], uint64_t ah[4], uint64_t sh[4]) {
  lbits_to_be_words4(slot, s);
  secure_keccak(20, a, ah);
  secure_keccak(32, s, sh);
}

/* 20 raw address bytes -> lbits of length 160 */
static void addr20_to_lbits(lbits *rop, const uint8_t a[20]) {
  uint64_t be[4] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++)  be[1] = (be[1] << 8) | a[i];
  for (int i = 4; i < 12; i++) be[2] = (be[2] << 8) | a[i];
  for (int i = 12; i < 20; i++) be[3] = (be[3] << 8) | a[i];
  be_words4_to_lbits(rop, be);
  rop->len = 160;
}

static const uint64_t storage_zero_val[4] = {0, 0, 0, 0};

/* ======================================================================== */
/* PERSISTENT STORAGE STATE                                                   */
/*                                                                          */
/* Two flat tables keyed by (keccak(addr), keccak(slot), generation), each   */
/* kept sorted in that order. Generation zero is the transaction-global      */
/* original/access set. Generations >= 1 are current-value overlays retained */
/* for O(1) storage clear/revert. Block rows always use generation zero.      */
/*   storage_tx_table    -- immutable originals/accesses plus generation      */
/*                overlays. Generation-zero membership survives reverts.     */
/*   storage_block_table -- block cache and net changes over authenticated    */
/*                pre-state.                                                  */
/* A block/overlay row carries (original, current); dirty == written &&       */
/* current!=original.                                                         */
/* The base read-through target below both (native seeded map / witness MPT   */
/* point-get) is resolved in Sail; a miss here means "ask the base".          */
/* ======================================================================== */

typedef struct {
  uint64_t acct_hash[4];
  uint64_t generation;
  uint64_t slot_hash[4];
  uint64_t slot[4];
  uint8_t  raw_addr[20]; /* address preimage (tx rows; set by storage_tx_cache) */
  uint64_t current[4];   /* live value (valid iff written) */
  uint64_t original[4];  /* gen0: tx original; overlay: generation base; block: pre-state */
  uint8_t  written;      /* 1: a write reached this row; 0: read-only member */
  uint8_t  was_read;     /* 1: this slot was SLOADed (EIP-7928 storage_reads) */
} storage_state_row;

typedef struct { storage_state_row *rows; uint32_t n, cap; } storage_state_table;

static storage_state_table storage_tx_table    = {NULL, 0, 0};
static storage_state_table storage_block_table = {NULL, 0, 0};
static uint32_t storage_tx_pop_cursor = 0; /* k_tx_merge drain position */
enum { STORAGE_BASE_GENERATION = 0, STORAGE_INITIAL_GENERATION = 1 };
static uint64_t storage_next_generation = STORAGE_INITIAL_GENERATION;

typedef struct {
  uint64_t acct_hash[4];
  uint64_t active_generation;
} storage_epoch_row;

typedef struct {
  storage_epoch_row *rows;
  uint32_t n, cap;
} storage_epoch_table;

enum { STORAGE_UNDO_WRITE = 1, STORAGE_UNDO_CLEAR = 2 };

typedef struct {
  uint8_t tag;
  uint8_t prior_written;
  uint64_t acct_hash[4];
  uint64_t generation;
  uint64_t slot_hash[4];
  uint64_t prior_current[4];
} storage_undo_entry;

static storage_epoch_table storage_epochs = {NULL, 0, 0};
static storage_undo_entry *storage_undo = NULL;
static uint32_t storage_undo_n = 0, storage_undo_cap = 0;

/* drop the per-account compute_root snapshots when the block base changes
   (defined with the snapshot builders below) */
static void storage_dump_invalidate(void);

static uint32_t storage_table_find(const storage_state_table *t, const uint64_t ah[4],
                                  uint64_t generation, const uint64_t sh[4],
                                  int *found) {
  uint32_t lo = 0, hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    int c = storage_key_cmp(t->rows[mid].acct_hash, t->rows[mid].generation,
                            t->rows[mid].slot_hash, ah, generation, sh);
    if (c < 0) lo = mid + 1; else hi = mid;
  }
  *found = (lo < t->n &&
            storage_key_cmp(t->rows[lo].acct_hash, t->rows[lo].generation,
                            t->rows[lo].slot_hash, ah, generation, sh) == 0);
  return lo;
}

static storage_state_row *storage_table_get(storage_state_table *t,
                                          const uint64_t ah[4],
                                          uint64_t generation,
                                          const uint64_t sh[4]) {
  int f = 0;
  uint32_t i = storage_table_find(t, ah, generation, sh, &f);
  return f ? &t->rows[i] : NULL;
}

/* insert-if-absent, keeping the table sorted; new rows are zeroed with the
   keys set (written == 0 == read-only member) */
static storage_state_row *storage_table_intern(storage_state_table *t,
                                             const uint64_t ah[4],
                                             uint64_t generation,
                                             const uint64_t sh[4],
                                             const uint64_t slot[4]) {
  int f = 0;
  uint32_t i = storage_table_find(t, ah, generation, sh, &f);
  if (f) return &t->rows[i];
  if (t->cap < t->n + 1) {
    uint32_t nc = t->cap ? t->cap * 2 : STORAGE_INIT_CAP;
    while (nc < t->n + 1) nc *= 2;
    storage_state_row *nr = (storage_state_row *)realloc(t->rows, (size_t)nc * sizeof(storage_state_row));
    if (!nr) return NULL;
    t->rows = nr; t->cap = nc;
  }
  if (i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(storage_state_row));
  storage_state_row *e = &t->rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->acct_hash, ah, sizeof(e->acct_hash));
  e->generation = generation;
  memcpy(e->slot_hash, sh, sizeof(e->slot_hash));
  memcpy(e->slot, slot, sizeof(e->slot));
  t->n++;
  return e;
}

static void storage_table_reset(storage_state_table *t) {
  free(t->rows);
  t->rows = NULL; t->n = 0; t->cap = 0;
}

static void storage_table_account_range(const storage_state_table *t,
                                    const uint64_t ah[4], uint32_t *start,
                                    uint32_t *end) {
  uint32_t lo = 0, hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_words(t->rows[mid].acct_hash, ah, 4) < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  uint32_t e = lo;
  while (e < t->n && compare_words(t->rows[e].acct_hash, ah, 4) == 0) e++;
  *start = lo;
  *end = e;
}

static uint32_t storage_epoch_find(const uint64_t ah[4], int *found) {
  uint32_t lo = 0, hi = storage_epochs.n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_words(storage_epochs.rows[mid].acct_hash, ah, 4) < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  *found = lo < storage_epochs.n &&
           compare_words(storage_epochs.rows[lo].acct_hash, ah, 4) == 0;
  return lo;
}

static storage_epoch_row *storage_epoch_get(const uint64_t ah[4]) {
  int found = 0;
  uint32_t i = storage_epoch_find(ah, &found);
  return found ? &storage_epochs.rows[i] : NULL;
}

static storage_epoch_row *storage_epoch_intern(const uint64_t ah[4]) {
  int found = 0;
  uint32_t i = storage_epoch_find(ah, &found);
  if (found) return &storage_epochs.rows[i];
  if (storage_epochs.cap < storage_epochs.n + 1) {
    uint32_t cap = storage_epochs.cap ? storage_epochs.cap * 2 : 16;
    storage_epoch_row *rows = realloc(storage_epochs.rows,
                                      (size_t)cap * sizeof(*rows));
    if (!rows) return NULL;
    storage_epochs.rows = rows;
    storage_epochs.cap = cap;
  }
  if (i < storage_epochs.n)
    memmove(&storage_epochs.rows[i + 1], &storage_epochs.rows[i],
            (size_t)(storage_epochs.n - i) * sizeof(storage_epoch_row));
  storage_epoch_row *e = &storage_epochs.rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->acct_hash, ah, sizeof(e->acct_hash));
  e->active_generation = STORAGE_INITIAL_GENERATION;
  storage_epochs.n++;
  return e;
}

static uint64_t storage_active_generation(const uint64_t ah[4]) {
  storage_epoch_row *e = storage_epoch_get(ah);
  return e ? e->active_generation : STORAGE_INITIAL_GENERATION;
}

static storage_undo_entry *storage_undo_push(uint8_t tag) {
  if (storage_undo_cap < storage_undo_n + 1) {
    uint32_t cap = storage_undo_cap ? storage_undo_cap * 2 : 64;
    storage_undo_entry *entries = realloc(storage_undo,
                                          (size_t)cap * sizeof(*entries));
    if (!entries) return NULL;
    storage_undo = entries;
    storage_undo_cap = cap;
  }
  storage_undo_entry *entry = &storage_undo[storage_undo_n++];
  memset(entry, 0, sizeof(*entry));
  entry->tag = tag;
  return entry;
}

/* --- lifecycle -------------------------------------------------------- */

/* full world wipe (between independent blocks/fixtures) */
/* whole-overlay wipe (both layers): the HARNESS world reset
   (test_utils.c evmsail_clear_memory); no Sail caller. */
unit storage_db_reset(const unit u) {
  (void)u;
  storage_tx_pop_cursor = 0;
  storage_next_generation = STORAGE_INITIAL_GENERATION;
  storage_table_reset(&storage_tx_table);
  storage_table_reset(&storage_block_table);
  free(storage_epochs.rows);
  storage_epochs.rows = NULL;
  storage_epochs.n = storage_epochs.cap = 0;
  free(storage_undo);
  storage_undo = NULL;
  storage_undo_n = storage_undo_cap = 0;
  storage_dump_invalidate();
  return UNIT;
}

/* Transaction scratch reset: retain the allocation for the next transaction. */
unit storage_tx_reset(const unit u) {
  (void)u;
  storage_tx_table.n = 0;
  storage_tx_pop_cursor = 0;
  storage_next_generation = STORAGE_INITIAL_GENERATION;
  storage_epochs.n = 0;
  storage_undo_n = 0;
  return UNIT;
}

uint64_t storage_tx_checkpoint(const unit u) {
  (void)u;
  return storage_undo_n;
}

unit storage_tx_revert(uint64_t checkpoint) {
  if (checkpoint > storage_undo_n) abort();
  while (storage_undo_n > checkpoint) {
    storage_undo_entry *undo = &storage_undo[--storage_undo_n];
    if (undo->tag == STORAGE_UNDO_WRITE) {
      storage_state_row *row = storage_table_get(
          &storage_tx_table, undo->acct_hash, undo->generation,
          undo->slot_hash);
      if (row) {
        row->written = undo->prior_written;
        memcpy(row->current, undo->prior_current, sizeof(row->current));
      }
    } else if (undo->tag == STORAGE_UNDO_CLEAR) {
      storage_epoch_row *epoch = storage_epoch_intern(undo->acct_hash);
      if (epoch) epoch->active_generation = undo->generation;
    } else {
      abort();
    }
  }
  return UNIT;
}

unit storage_tx_clear(const lbits a) {
  uint64_t ah[4];
  secure_keccak(20, a, ah);
  storage_epoch_row *epoch = storage_epoch_intern(ah);
  if (!epoch) return UNIT;
  storage_undo_entry *undo = storage_undo_push(STORAGE_UNDO_CLEAR);
  if (!undo) return UNIT;
  memcpy(undo->acct_hash, ah, sizeof(undo->acct_hash));
  undo->generation = epoch->active_generation;
  if (storage_next_generation == UINT64_MAX) abort();
  epoch->active_generation = ++storage_next_generation;
  return UNIT;
}

/* Drain one transaction merge entry by joining a generation-zero access row
   with the same key's active write generation. Rows for a key are adjacent in
   generation order. The return status is 1 for a read-only access and 2 for a
   surviving write; obsolete generations stay host-private rollback history. */
uint64_t storage_tx_pop_probe(lbits *addr, lbits *slot, lbits *curr,
                              lbits *orig, lbits *tx_orig) {
  while (storage_tx_pop_cursor < storage_tx_table.n) {
    const uint32_t start = storage_tx_pop_cursor;
    const storage_state_row *base =
        &storage_tx_table.rows[start];
    if (base->generation != STORAGE_BASE_GENERATION) {
      storage_tx_pop_cursor++;
      continue;
    }

    uint32_t end = start + 1;
    while (end < storage_tx_table.n &&
           compare_words(storage_tx_table.rows[end].acct_hash,
                         base->acct_hash, 4) == 0 &&
           compare_words(storage_tx_table.rows[end].slot_hash,
                         base->slot_hash, 4) == 0)
      end++;
    storage_tx_pop_cursor = end;

    const uint64_t generation = storage_active_generation(base->acct_hash);
    const storage_state_row *overlay = NULL;
    addr20_to_lbits(addr, base->raw_addr);
    be_words4_to_lbits(slot, base->slot);
    be_words4_to_lbits(tx_orig, base->original);
    for (uint32_t i = start + 1; i < end; i++) {
      const storage_state_row *candidate = &storage_tx_table.rows[i];
      if (candidate->generation == generation) {
        overlay = candidate;
        break;
      }
    }
    if (overlay && overlay->written) {
      be_words4_to_lbits(curr, overlay->current);
      be_words4_to_lbits(orig, overlay->original);
      return 2;
    }
    return 1;
  }
  return 0;
}

/* block-layer propagation hooks (pure mechanism; Sail chose the action).
   Keyed by (keccak(address), slot); the slot's secure key is recomputed. */
/* net change: a fresh block row freezes orig as the pre-state; curr lands */
unit storage_block_put_raw(const lbits a, const lbits s_, const lbits curr,
                           const lbits orig) {
  uint64_t slot[4], a4[4], sh[4], c4[4], o4[4];
  storage_secure_key(a, s_, slot, a4, sh);
  storage_state_row *b = storage_table_get(&storage_block_table, a4, 0, sh);
  int fresh = (b == NULL);
  if (fresh) b = storage_table_intern(&storage_block_table, a4, 0, sh, slot);
  if (!b) return UNIT;
  lbits_to_be_words4(c4, curr);
  if (fresh) { lbits_to_be_words4(o4, orig); memcpy(b->original, o4, sizeof(b->original)); }
  b->written = 1;
  memcpy(b->current, c4, sizeof(b->current));
  storage_dump_invalidate();
  return UNIT;
}

/* read member: fresh binds curr == orig = value; existing is only marked */
unit storage_block_cache_raw(const lbits a, const lbits s_, const lbits v) {
  uint64_t slot[4], a4[4], sh[4], v4[4];
  storage_secure_key(a, s_, slot, a4, sh);
  storage_state_row *b = storage_table_get(&storage_block_table, a4, 0, sh);
  if (!b) {
    b = storage_table_intern(&storage_block_table, a4, 0, sh, slot);
    if (b) {
      lbits_to_be_words4(v4, v);
      memcpy(b->current, v4, sizeof(b->current));
      memcpy(b->original, v4, sizeof(b->original));
      b->was_read = 1;
    }
  } else {
    b->was_read = 1;
  }
  storage_dump_invalidate();
  return UNIT;
}


/* --- reads: per-layer row probe -----------------------------------------
   A tx read resolves the current overlay over generation zero. After a clear,
   an overlay miss resolves to zero and still interns a generation-zero access
   row so BAL evidence survives frame reverts. */
uint64_t storage_row_probe(uint64_t layer, const lbits a, const lbits s,
                           lbits *cur, lbits *orig) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  if (layer != 0) {
    storage_state_row *e = storage_table_get(
        &storage_block_table, ah, STORAGE_BASE_GENERATION, sh);
    if (!e || !(e->written || e->was_read)) return 0;
    be_words4_to_lbits(cur, e->current);
    be_words4_to_lbits(orig, e->original);
    return e->written ? 2 : 1;
  }

  const uint64_t generation = storage_active_generation(ah);
  storage_state_row *base = storage_table_get(
      &storage_tx_table, ah, STORAGE_BASE_GENERATION, sh);
  storage_state_row *overlay = storage_table_get(
      &storage_tx_table, ah, generation, sh);
  if (overlay && overlay->written) {
    if (!base) abort();
    base->was_read = 1;
    be_words4_to_lbits(cur, overlay->current);
    be_words4_to_lbits(orig, base->original);
    return 2;
  }
  if (generation != STORAGE_INITIAL_GENERATION) {
    if (!base) {
      base = storage_table_intern(&storage_tx_table, ah,
                                 STORAGE_BASE_GENERATION, sh, slot);
      if (!base) return 0;
      lbits_to_be_bytes(base->raw_addr, 20, a);
      memcpy(base->current, storage_zero_val, sizeof(base->current));
      memcpy(base->original, storage_zero_val, sizeof(base->original));
    }
    base->was_read = 1;
    be_words4_to_lbits(cur, storage_zero_val);
    be_words4_to_lbits(orig, base->original);
    return 1;
  }
  if (!base || !base->was_read) return 0;
  be_words4_to_lbits(cur, base->original);
  be_words4_to_lbits(orig, base->original);
  return 1;
}

/* --- writes / warm ------------------------------------------------------ */

/* Bind an authenticated tx-start value in generation zero. This set is
   immutable except for its monotone access marker. */
unit storage_tx_cache_raw(const lbits a, const lbits s, const lbits v) {
  uint64_t slot[4], ah[4], sh[4], w[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  storage_state_row *e = storage_table_get(
      &storage_tx_table, ah, STORAGE_BASE_GENERATION, sh);
  if (!e) {
    e = storage_table_intern(&storage_tx_table, ah,
                            STORAGE_BASE_GENERATION, sh, slot);
    if (!e) return UNIT;
    lbits_to_be_bytes(e->raw_addr, 20, a);
    memcpy(e->current, w, sizeof(e->current));
    memcpy(e->original, w, sizeof(e->original));
  }
  if (!e) return UNIT;
  e->was_read = 1;
  return UNIT;
}

/* Write the active overlay. SSTORE's preceding read guarantees a generation
   zero row; a fresh overlay starts from the tx original in generation one or
   zero after a storage clear. */
unit storage_tx_update_raw(const lbits a, const lbits s, const lbits v) {
  uint64_t slot[4], ah[4], sh[4], w[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  const uint64_t generation = storage_active_generation(ah);
  storage_state_row *base = storage_table_get(
      &storage_tx_table, ah, STORAGE_BASE_GENERATION, sh);
  if (!base) return UNIT;
  uint64_t prior[4];
  if (generation == STORAGE_INITIAL_GENERATION)
    memcpy(prior, base->original, sizeof(prior));
  else
    memcpy(prior, storage_zero_val, sizeof(prior));
  storage_state_row *e = storage_table_get(&storage_tx_table, ah, generation, sh);
  if (!e) {
    e = storage_table_intern(&storage_tx_table, ah, generation, sh, slot);
    if (!e) return UNIT;
    lbits_to_be_bytes(e->raw_addr, 20, a);
    memcpy(e->current, prior, sizeof(e->current));
    memcpy(e->original, prior, sizeof(e->original));
  }
  storage_undo_entry *undo = storage_undo_push(STORAGE_UNDO_WRITE);
  if (!undo) return UNIT;
  memcpy(undo->acct_hash, ah, sizeof(undo->acct_hash));
  undo->generation = generation;
  memcpy(undo->slot_hash, sh, sizeof(undo->slot_hash));
  memcpy(undo->prior_current, e->current, sizeof(undo->prior_current));
  undo->prior_written = e->written;
  e->written = 1;
  memcpy(e->current, w, sizeof(e->current));
  return UNIT;
}

/* drop every row for one account_hash, compacting in place (keeps the table
   sorted since the account's rows are a contiguous block) */
static void storage_table_remove_account(storage_state_table *t, const uint64_t ah[4]) {
  uint32_t w = 0;
  for (uint32_t i = 0; i < t->n; i++) {
    if (compare_words(t->rows[i].acct_hash, ah, 4) == 0) continue;
    if (w != i) t->rows[w] = t->rows[i];
    w++;
  }
  t->n = w;
}

/* Clear the generation-free block storage for an account. Transaction
   generations are retained until transaction reset for checkpoint reverts. */
unit storage_block_clear(const lbits a) {
  uint64_t h[4];
  secure_keccak(20, a, h);
  storage_table_remove_account(&storage_block_table, h);
  storage_dump_invalidate();
  return UNIT;
}

/* ---- cumulative storage enumeration -------------------------------------
   Both tables are sorted by (acct_hash, slot_hash), so an account's entries
   form a contiguous range found by binary search on acct_hash. The Sail state
   trie consumes net changes over the authenticated MPT anchor directly. */

typedef struct { uint64_t slot[4]; uint64_t val[4]; } storage_dump_entry;

/* [start,end) block rows with acct_hash == ak */
static void storage_block_account_range(const uint64_t ak[4], uint32_t *start, uint32_t *end) {
  storage_table_account_range(&storage_block_table, ak, start, end);
}

/* EELS account_has_storage: a nonempty write map in either the surviving
   transaction generation or the cumulative block overlay counts as storage.
   The authenticated pre-state root is checked in Sail. */
bool storage_has_writes(const lbits a) {
  uint64_t ah[4];
  uint32_t start, end;
  secure_keccak(20, a, ah);

  storage_table_account_range(&storage_tx_table, ah, &start, &end);
  const uint64_t generation = storage_active_generation(ah);
  for (uint32_t i = start; i < end; i++) {
    const storage_state_row *row = &storage_tx_table.rows[i];
    if (row->generation == generation && row->written) return true;
  }

  storage_table_account_range(&storage_block_table, ah, &start, &end);
  for (uint32_t i = start; i < end; i++) {
    if (storage_block_table.rows[i].written) return true;
  }
  return false;
}

static void storage_dump_push(storage_dump_entry **rows, uint32_t *n, uint32_t *cap,
                          const uint64_t slot[4], const uint64_t val[4]) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (storage_dump_entry *)realloc(*rows, (size_t)nc * sizeof(storage_dump_entry));
    *cap = nc;
  }
  memcpy((*rows)[*n].slot, slot, 32);
  memcpy((*rows)[*n].val, val, 32);
  (*n)++;
}

/* Direct iteration over an account's cumulative storage map. */
uint64_t storage_block_count(const lbits a) {
  uint64_t k[4]; uint32_t bs, be;
  secure_keccak(20, a, k);
  storage_block_account_range(k, &bs, &be);
  return be - bs;
}

static const storage_state_row *storage_block_at(const lbits a, uint64_t i) {
  uint64_t k[4]; uint32_t bs, be;
  secure_keccak(20, a, k);
  storage_block_account_range(k, &bs, &be);
  return bs + i < be ? &storage_block_table.rows[bs + i] : NULL;
}

bool storage_block_changed(const lbits a, uint64_t i) {
  const storage_state_row *entry = storage_block_at(a, i);
  return entry && entry->written &&
         memcmp(entry->current, entry->original, sizeof(entry->current)) != 0;
}

void storage_block_slot(lbits *rop, const lbits a, uint64_t i) {
  const storage_state_row *entry = storage_block_at(a, i);
  be_words4_to_lbits(rop, entry ? entry->slot : storage_zero_val);
}

void storage_block_current(lbits *rop, const lbits a, uint64_t i) {
  const storage_state_row *entry = storage_block_at(a, i);
  be_words4_to_lbits(rop, entry ? entry->current : storage_zero_val);
}

/* All cumulative block entries for an account, including read-only entries.
   The authenticated base lives in the MPT node DB, so untouched base slots are
   intentionally absent. This materialized view exists only for debug dumps. */
static storage_dump_entry *storage_dump_entries = NULL;
static uint32_t storage_dump_len = 0, storage_dump_cap = 0;
static uint64_t storage_dump_account_hash[4];
static int storage_dump_valid = 0;

static void storage_dump_build(const uint64_t ak[4]) {
  if (storage_dump_valid && compare_words(storage_dump_account_hash, ak, 4) == 0) return;
  storage_dump_len = 0;
  uint32_t bs, be;
  storage_block_account_range(ak, &bs, &be);
  for (uint32_t bi = bs; bi < be; bi++) {
    storage_dump_push(&storage_dump_entries, &storage_dump_len, &storage_dump_cap,
                  storage_block_table.rows[bi].slot, storage_block_table.rows[bi].current);
  }
  memcpy(storage_dump_account_hash, ak, sizeof(storage_dump_account_hash));
  storage_dump_valid = 1;
}

static void storage_dump_invalidate(void) {
  storage_dump_valid = 0;
}

uint64_t storage_dump_count(const lbits ak) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); storage_dump_build(k); return storage_dump_len;
}
void storage_dump_slot(lbits *rop, const lbits ak, uint64_t j) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); storage_dump_build(k);
  be_words4_to_lbits(rop, j < storage_dump_len ? storage_dump_entries[j].slot : storage_zero_val);
}
void storage_dump_value(lbits *rop, const lbits ak, uint64_t j) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); storage_dump_build(k);
  be_words4_to_lbits(rop, j < storage_dump_len ? storage_dump_entries[j].val : storage_zero_val);
}

/* ======================================================================== */
/* ACCOUNT STATE                                                              */
/*                                                                            */
/* A transaction table over a cumulative block table, keyed by                */
/* keccak(address), each sorted by                                             */
/* hkey. A row carries the current AND the tx-start/pre-state original        */
/* account (nonce, balance, storage_root, code_hash). A miss here means the   */
/* account was never touched; k_aload resolves it from the block layer or     */
/* the witness walk (stateless_account). storage_root is NOT mutated by       */
/* account writes -- it is the pre-state anchor, and the post-state root is    */
/* derived at compute_root. dirty == written && current != original.          */
/* ======================================================================== */

typedef struct {
  uint64_t hkey[4];
  uint64_t cur_nonce;  uint64_t cur_bal[4];  uint64_t cur_sroot[4];  uint64_t cur_chash[4];
  uint8_t cur_exists, cur_storage_cleared, cur_created, cur_selfdestructed;
  uint64_t orig_nonce; uint64_t orig_bal[4]; uint64_t orig_sroot[4]; uint64_t orig_chash[4];
  uint8_t orig_exists, orig_storage_cleared, orig_created, orig_selfdestructed;
  uint8_t raw_addr[20];  /* EIP-7928 BAL account key/order (set on seed/first write) */
  uint32_t snapshot_cursor;
} acct_state_row;

typedef struct { acct_state_row *rows; uint32_t n, cap; } acct_state_table;

static acct_state_table acct_tx_table    = {NULL, 0, 0};
static acct_state_table acct_block_table = {NULL, 0, 0};
static uint32_t acct_tx_pop_cursor = 0; /* k_tx_merge drain position */

typedef struct {
  uint64_t hkey[4];
  uint64_t nonce;
  uint64_t balance[4];
  uint64_t storage_root[4];
  uint64_t code_hash[4];
  uint32_t snapshot_cursor;
  uint8_t exists;
  uint8_t storage_cleared;
  uint8_t selfdestructed;
} acct_undo_entry;

static acct_undo_entry *acct_undo = NULL;
static uint32_t acct_undo_n = 0, acct_undo_cap = 0;
static const uint32_t ACCT_NO_SNAPSHOT = UINT32_MAX;
static uint32_t acct_active_snapshot = UINT32_MAX;


/* per-account compute_root snapshots are invalidated when the block base
   changes (defined with the builders below) */
static void acct_dump_invalidate(void);

static uint32_t acct_table_find(const acct_state_table *t, const uint64_t h[4], int *found) {
  uint32_t lo = 0, hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_u64x4(t->rows[mid].hkey, h) < 0) lo = mid + 1; else hi = mid;
  }
  *found = (lo < t->n && compare_u64x4(t->rows[lo].hkey, h) == 0);
  return lo;
}

static acct_state_row *acct_table_get(acct_state_table *t, const uint64_t h[4]) {
  int f = 0;
  uint32_t i = acct_table_find(t, h, &f);
  return f ? &t->rows[i] : NULL;
}

static acct_state_row *acct_table_intern(acct_state_table *t, const uint64_t h[4]) {
  int f = 0;
  uint32_t i = acct_table_find(t, h, &f);
  if (f) return &t->rows[i];
  if (t->cap < t->n + 1) {
    uint32_t nc = t->cap ? t->cap * 2 : ACCOUNT_INIT_CAP;
    while (nc < t->n + 1) nc *= 2;
    acct_state_row *nr = (acct_state_row *)realloc(t->rows, (size_t)nc * sizeof(acct_state_row));
    if (!nr) return NULL;
    t->rows = nr; t->cap = nc;
  }
  if (i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(acct_state_row));
  acct_state_row *e = &t->rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->hkey, h, sizeof(e->hkey));
  e->snapshot_cursor = ACCT_NO_SNAPSHOT;
  t->n++;
  return e;
}

static void acct_table_reset(acct_state_table *t) {
  free(t->rows);
  t->rows = NULL; t->n = 0; t->cap = 0;
}

/* The active undo cursor is the lazy snapshot identity. Each account saves its
   reversible projection at most once at that cursor; successful child frames
   retain their entries because an enclosing frame may still revert them. */
static int acct_snapshot_for_write(acct_state_row *row) {
  if (acct_active_snapshot == ACCT_NO_SNAPSHOT ||
      row->snapshot_cursor == acct_active_snapshot)
    return 1;
  if (acct_undo_cap < acct_undo_n + 1) {
    uint32_t cap = acct_undo_cap ? acct_undo_cap * 2 : 64;
    acct_undo_entry *entries = realloc(acct_undo,
                                       (size_t)cap * sizeof(*entries));
    if (!entries) return 0;
    acct_undo = entries;
    acct_undo_cap = cap;
  }
  acct_undo_entry *undo = &acct_undo[acct_undo_n++];
  memcpy(undo->hkey, row->hkey, sizeof(undo->hkey));
  undo->nonce = row->cur_nonce;
  memcpy(undo->balance, row->cur_bal, sizeof(undo->balance));
  memcpy(undo->storage_root, row->cur_sroot, sizeof(undo->storage_root));
  memcpy(undo->code_hash, row->cur_chash, sizeof(undo->code_hash));
  undo->snapshot_cursor = row->snapshot_cursor;
  undo->exists = row->cur_exists;
  undo->storage_cleared = row->cur_storage_cleared;
  undo->selfdestructed = row->cur_selfdestructed;
  row->snapshot_cursor = acct_active_snapshot;
  return 1;
}

/* --- lifecycle --- */
/* whole-overlay wipe (both layers): the HARNESS world reset; no Sail caller. */
unit acct_db_reset(const unit u) {
  (void)u;
  acct_tx_pop_cursor = 0;
  acct_table_reset(&acct_tx_table);
  acct_table_reset(&acct_block_table);
  free(acct_undo);
  acct_undo = NULL;
  acct_undo_n = acct_undo_cap = 0;
  acct_active_snapshot = ACCT_NO_SNAPSHOT;
  acct_dump_invalidate();
  return UNIT;
}

unit acct_tx_reset(const unit u) {
  (void)u;
  acct_tx_table.n = 0;
  acct_tx_pop_cursor = 0;
  acct_undo_n = 0;
  acct_active_snapshot = ACCT_NO_SNAPSHOT;
  return UNIT;
}

uint64_t acct_tx_checkpoint(const unit u) {
  (void)u;
  uint32_t prior = acct_active_snapshot;
  uint32_t cursor = acct_undo_n;
  acct_active_snapshot = cursor;
  /* The checkpoint is opaque to Sail: high = parent identity, low = cursor. */
  return ((uint64_t)prior << 32) | cursor;
}

unit acct_tx_revert(uint64_t checkpoint) {
  uint32_t cursor = (uint32_t)checkpoint;
  uint32_t prior = (uint32_t)(checkpoint >> 32);
  if (cursor > acct_undo_n) abort();
  while (acct_undo_n > cursor) {
    const acct_undo_entry *undo = &acct_undo[--acct_undo_n];
    acct_state_row *current = acct_table_get(&acct_tx_table, undo->hkey);
    if (!current) abort();
    current->cur_nonce = undo->nonce;
    memcpy(current->cur_bal, undo->balance, sizeof(current->cur_bal));
    memcpy(current->cur_sroot, undo->storage_root, sizeof(current->cur_sroot));
    memcpy(current->cur_chash, undo->code_hash, sizeof(current->cur_chash));
    current->cur_exists = undo->exists;
    current->cur_storage_cleared = undo->storage_cleared;
    current->cur_selfdestructed = undo->selfdestructed;
    current->snapshot_cursor = undo->snapshot_cursor;
  }
  acct_active_snapshot = prior;
  return UNIT;
}

/* merge the tx overlay into the block base, then clear it. The merge only
   ever updates the block CURRENT; the block ORIGINAL is the
   pre-state and is frozen ONCE, when the account first enters the block
   cumulative table (from the tx row, which froze it at cache time from the base
   resolver). A row that already exists here already carries the correct
   frozen original, so the merge must never touch it. */
/* drain-one pop for the Sail merge (k_tx_merge): SIDE-EFFECT-FREE hand-over
   of tx account row [cursor] incl. the address preimage (the block row's BAL
   serialization key); on drain the tx table resets and 0 returns (else
   1). Sail decides records + propagation per row. */
uint64_t acct_tx_pop_probe(lbits *addr,
                           uint64_t *cn, lbits *cb, lbits *cs, lbits *cc,
                           bool *ce, bool *csc, bool *ccr, bool *csd,
                           uint64_t *on, lbits *ob, lbits *os, lbits *oc,
                           bool *oe, bool *osc, bool *ocr, bool *osd) {
  if (acct_tx_pop_cursor >= acct_tx_table.n) {
    return 0;
  }
  const acct_state_row *e = &acct_tx_table.rows[acct_tx_pop_cursor++];
  addr20_to_lbits(addr, e->raw_addr);
  *cn = e->cur_nonce;
  le_words4_to_lbits(cb, e->cur_bal);
  le_words4_to_lbits(cs, e->cur_sroot);
  le_words4_to_lbits(cc, e->cur_chash);
  *ce = e->cur_exists;
  *csc = e->cur_storage_cleared;
  *ccr = e->cur_created;
  *csd = e->cur_selfdestructed;
  *on = e->orig_nonce;
  le_words4_to_lbits(ob, e->orig_bal);
  le_words4_to_lbits(os, e->orig_sroot);
  le_words4_to_lbits(oc, e->orig_chash);
  *oe = e->orig_exists;
  *osc = e->orig_storage_cleared;
  *ocr = e->orig_created;
  *osd = e->orig_selfdestructed;
  return 1;
}

/* block-layer propagation hooks (pure mechanism; Sail chose the action).
   Keyed by the address preimage: the row keeps it for BAL serialization. */
static acct_state_row *acct_block_bind(const lbits a, int *fresh) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_state_row *b = acct_table_get(&acct_block_table, h);
  *fresh = (b == NULL);
  if (*fresh) b = acct_table_intern(&acct_block_table, h);
  if (b) lbits_to_be_bytes(b->raw_addr, 20, a);
  return b;
}

/* changed account: fresh freezes orig; curr always lands */
unit acct_block_write_raw(const lbits a, uint64_t nonce, const lbits bal,
                          const lbits sroot, const lbits chash,
                          bool exists, bool storage_cleared,
                          uint64_t ononce, const lbits obal,
                          const lbits osroot, const lbits ochash,
                          bool oexists, bool ostorage_cleared) {
  int fresh = 0;
  acct_state_row *b = acct_block_bind(a, &fresh);
  if (!b) return UNIT;
  if (fresh) {
    b->orig_nonce = ononce;
    lbits_to_le_words4(b->orig_bal, obal);
    lbits_to_le_words4(b->orig_sroot, osroot);
    lbits_to_le_words4(b->orig_chash, ochash);
    b->orig_exists = oexists;
    b->orig_storage_cleared = ostorage_cleared;
  }
  b->cur_nonce = nonce;
  lbits_to_le_words4(b->cur_bal, bal);
  lbits_to_le_words4(b->cur_sroot, sroot);
  lbits_to_le_words4(b->cur_chash, chash);
  b->cur_exists = exists;
  b->cur_storage_cleared = storage_cleared;
  acct_dump_invalidate();
  return UNIT;
}

/* read member: fresh binds curr == orig; an existing row is untouched */
unit acct_block_cache_raw(const lbits a, uint64_t nonce, const lbits bal,
                          const lbits sroot, const lbits chash,
                          bool exists, bool storage_cleared) {
  int fresh = 0;
  acct_state_row *b = acct_block_bind(a, &fresh);
  if (!b) return UNIT;
  if (fresh) {
    b->cur_nonce = nonce;  b->orig_nonce = nonce;
    lbits_to_le_words4(b->cur_bal, bal);    lbits_to_le_words4(b->orig_bal, bal);
    lbits_to_le_words4(b->cur_sroot, sroot); lbits_to_le_words4(b->orig_sroot, sroot);
    lbits_to_le_words4(b->cur_chash, chash); lbits_to_le_words4(b->orig_chash, chash);
    b->cur_exists = exists; b->orig_exists = exists;
    b->cur_storage_cleared = storage_cleared;
    b->orig_storage_cleared = storage_cleared;
    acct_dump_invalidate();
  }
  return UNIT;
}

/* --- reads (overlay only; a miss => Sail asks eest_account / resolver) --- */
/* per-layer account probe (layer 0 = tx, 1 = block) for the Sail
   option(Account) glue (journal_glue.c): returns presence; the field words
   land in the out params (untouched when absent). The tx-over-block
   precedence lives in Sail (account_lookup). */
uint64_t acct_row_probe(uint64_t layer, const lbits a, uint64_t *nonce,
                        lbits *bal, lbits *sroot, lbits *chash,
                        bool *exists, bool *storage_cleared,
                        bool *created, bool *selfdestructed) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_state_row *e = acct_table_get(layer == 0 ? &acct_tx_table : &acct_block_table, h);
  if (!e) return 0;
  *nonce = e->cur_nonce;
  le_words4_to_lbits(bal, e->cur_bal);
  le_words4_to_lbits(sroot, e->cur_sroot);
  le_words4_to_lbits(chash, e->cur_chash);
  *exists = e->cur_exists;
  *storage_cleared = e->cur_storage_cleared;
  *created = e->cur_created;
  *selfdestructed = e->cur_selfdestructed;
  return 1;
}

/* Direct iteration over the cumulative account map. */
uint64_t acct_block_count(const unit u) {
  (void)u;
  return acct_block_table.n;
}

bool acct_block_changed(uint64_t i) {
  if (i >= acct_block_table.n) return false;
  const acct_state_row *entry = &acct_block_table.rows[i];
  return entry->cur_nonce != entry->orig_nonce ||
         memcmp(entry->cur_bal, entry->orig_bal, sizeof(entry->cur_bal)) != 0 ||
         memcmp(entry->cur_sroot, entry->orig_sroot, sizeof(entry->cur_sroot)) != 0 ||
         memcmp(entry->cur_chash, entry->orig_chash, sizeof(entry->cur_chash)) != 0 ||
         entry->cur_exists != entry->orig_exists ||
         entry->cur_storage_cleared != entry->orig_storage_cleared;
}

void acct_block_address(lbits *rop, uint64_t i) {
  if (i >= acct_block_table.n) {
    static const uint8_t zero_address[20] = {0};
    addr20_to_lbits(rop, zero_address);
    return;
  }
  addr20_to_lbits(rop, acct_block_table.rows[i].raw_addr);
}

/* --- writes / restore / wipe --- */
/* update the current account on the EXISTING tx row. k_aload always runs
   before a store and caches every resolution into the tx layer
   (acct_tx_cache), so the row -- with orig already frozen at the tx-start
   account -- is guaranteed live. The host undo log snapshots the prior
   current row for frame rollback. */
unit acct_tx_update_raw(const lbits a, uint64_t nonce,
                        const lbits bal, const lbits sroot, const lbits chash,
                        bool exists, bool storage_cleared, bool created,
                        bool selfdestructed) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_state_row *e = acct_table_get(&acct_tx_table, h);
  if (!e) return UNIT;
  if (!acct_snapshot_for_write(e)) return UNIT;
  uint64_t b4[4], sr[4], ch[4];
  lbits_to_le_words4(b4, bal);
  lbits_to_le_words4(sr, sroot);
  lbits_to_le_words4(ch, chash);
  e->cur_nonce = nonce;
  memcpy(e->cur_bal, b4, sizeof(e->cur_bal));
  memcpy(e->cur_sroot, sr, sizeof(e->cur_sroot));
  memcpy(e->cur_chash, ch, sizeof(e->cur_chash));
  e->cur_exists = exists;
  e->cur_storage_cleared = storage_cleared;
  e->cur_created |= created;
  e->cur_selfdestructed = selfdestructed;
  return UNIT;
}

unit acct_tx_set_balance(const lbits a, const lbits balance) {
  uint64_t h[4], value[4];
  acct_secure_key(a, h);
  lbits_to_le_words4(value, balance);
  acct_state_row *e = acct_table_get(&acct_tx_table, h);
  if (!e || compare_u64x4(e->cur_bal, value) == 0) return UNIT;
  if (!acct_snapshot_for_write(e)) return UNIT;
  memcpy(e->cur_bal, value, sizeof(e->cur_bal));
  return UNIT;
}

unit acct_tx_set_nonce(const lbits a, uint64_t nonce) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_state_row *e = acct_table_get(&acct_tx_table, h);
  if (!e || e->cur_nonce == nonce) return UNIT;
  if (!acct_snapshot_for_write(e)) return UNIT;
  e->cur_nonce = nonce;
  return UNIT;
}

unit acct_tx_set_code_hash(const lbits a, const lbits code_hash) {
  uint64_t h[4], value[4];
  acct_secure_key(a, h);
  lbits_to_le_words4(value, code_hash);
  acct_state_row *e = acct_table_get(&acct_tx_table, h);
  if (!e || compare_u64x4(e->cur_chash, value) == 0) return UNIT;
  if (!acct_snapshot_for_write(e)) return UNIT;
  memcpy(e->cur_chash, value, sizeof(e->cur_chash));
  return UNIT;
}

/* cache a resolved account as a tx-layer READ member (cur == orig = the
   resolution, un-journaled so a frame revert keeps the revealed value) --
   k_aload's cache-on-every-read, mirroring storage_tx_cache.
   misses, so the row is always fresh -- bind unconditionally. */
unit acct_tx_cache_raw(const lbits a, uint64_t nonce, const lbits bal,
                       const lbits sroot, const lbits chash, bool exists,
                       bool storage_cleared, bool created,
                       bool selfdestructed) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_state_row *e = acct_table_intern(&acct_tx_table, h);
  if (!e) return UNIT;
  lbits_to_be_bytes(e->raw_addr, 20, a);   /* EIP-7928 BAL account key/order */
  uint64_t b4[4], sr[4], ch[4];
  lbits_to_le_words4(b4, bal);
  lbits_to_le_words4(sr, sroot);
  lbits_to_le_words4(ch, chash);
  e->cur_nonce = nonce;  e->orig_nonce = nonce;
  memcpy(e->cur_bal, b4, sizeof(e->cur_bal));    memcpy(e->orig_bal, b4, sizeof(e->orig_bal));
  memcpy(e->cur_sroot, sr, sizeof(e->cur_sroot)); memcpy(e->orig_sroot, sr, sizeof(e->orig_sroot));
  memcpy(e->cur_chash, ch, sizeof(e->cur_chash)); memcpy(e->orig_chash, ch, sizeof(e->orig_chash));
  e->cur_exists = exists; e->orig_exists = exists;
  e->cur_storage_cleared = storage_cleared;
  e->orig_storage_cleared = storage_cleared;
  e->cur_created = created; e->orig_created = created;
  e->cur_selfdestructed = selfdestructed;
  e->orig_selfdestructed = selfdestructed;
  return UNIT;
}

static void acct_table_remove(acct_state_table *t, const uint64_t h[4]) {
  int f = 0;
  uint32_t i = acct_table_find(t, h, &f);
  if (!f) return;
  if (i + 1 < t->n)
    memmove(&t->rows[i], &t->rows[i + 1], (size_t)(t->n - i - 1) * sizeof(acct_state_row));
  t->n--;
}

/* ---- debug enumeration over acct_block_table ----------------------------
   The snapshot contains existing cumulative account values for the native
   post-state dump. State-root construction consumes acct_block_probe_row
   directly. */

typedef struct {
  uint64_t hkey[4];
  uint64_t nonce; uint64_t bal[4]; uint64_t sroot[4]; uint64_t chash[4];

} acct_dump_entry;

static void acct_dump_push(acct_dump_entry **rows, uint32_t *n, uint32_t *cap,
                           const uint64_t hkey[4], uint64_t nonce, const uint64_t bal[4],
                           const uint64_t sroot[4], const uint64_t chash[4]) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (acct_dump_entry *)realloc(*rows, (size_t)nc * sizeof(acct_dump_entry));
    *cap = nc;
  }
  acct_dump_entry *r = &(*rows)[*n];
  memcpy(r->hkey, hkey, 32); r->nonce = nonce;
  memcpy(r->bal, bal, 32); memcpy(r->sroot, sroot, 32); memcpy(r->chash, chash, 32);
  (*n)++;
}

/* Existing touched accounts, materialized for the harness dump. */
static acct_dump_entry *acct_dump_entries = NULL;
static uint32_t acct_dump_len = 0, acct_dump_cap = 0;
static int acct_dump_valid = 0;

static void acct_dump_build(void) {
  if (acct_dump_valid) return;
  acct_dump_len = 0;
  for (uint32_t bi = 0; bi < acct_block_table.n; bi++) {
    const acct_state_row *b = &acct_block_table.rows[bi];
    if (!b->cur_exists) continue;
    acct_dump_push(&acct_dump_entries, &acct_dump_len, &acct_dump_cap,
                   b->hkey, b->cur_nonce, b->cur_bal, b->cur_sroot, b->cur_chash);
  }
  acct_dump_valid = 1;
}

uint64_t acct_dump_count(const unit u) { (void)u; acct_dump_build(); return acct_dump_len; }
void acct_dump_hkey(lbits *rop, uint64_t i) {
  acct_dump_build(); be_words4_to_lbits(rop, i < acct_dump_len ? acct_dump_entries[i].hkey : account_zero_val);
}
uint64_t acct_dump_nonce(uint64_t i) { acct_dump_build(); return i < acct_dump_len ? acct_dump_entries[i].nonce : 0; }
void acct_dump_balance(lbits *rop, uint64_t i) {
  acct_dump_build(); le_words4_to_lbits(rop, i < acct_dump_len ? acct_dump_entries[i].bal : account_zero_val);
}
void acct_dump_storage_root(lbits *rop, uint64_t i) {
  acct_dump_build(); le_words4_to_lbits(rop, i < acct_dump_len ? acct_dump_entries[i].sroot : account_zero_val);
}
void acct_dump_code_hash(lbits *rop, uint64_t i) {
  acct_dump_build(); le_words4_to_lbits(rop, i < acct_dump_len ? acct_dump_entries[i].chash : account_zero_val);
}

static void acct_dump_invalidate(void) {
  acct_dump_valid = 0;
}

/* ======================================================================== */
/* EIP-7928 block access list: recompute from execution.                    */
/*                                                                          */
/* We rebuild the BAL that EELS builds during execution                     */
/* (amsterdam/block_access_lists.py) and keccak(rlp(bal)) it; the caller     */
/* feeds that as block_access_list_hash into block_header_hash, so the        */
/* existing block_hash check verifies it (store + hash, no input parse).     */
/*                                                                          */
/* The accumulator is keyed by keccak(address) -- matching the overlays and  */
/* the merge hooks -- plus a keccak->raw-address record populated at touch    */
/* time; the BAL sorts/encodes by RAW address. Reads survive revert (the      */
/* records are append-only, never journaled). Changes are the per-tx net      */
/* diff read from the tx overlay before each merge, tagged with the ambient   */
/* block_access_index (0 pre-exec syscalls, i+1 for user tx i, N+1 post).     */
/* ======================================================================== */

/* ----------------------- BAL serialize + keccak ------------------------- */
/* Emit canonical RLP for the accumulated BAL and keccak it. Accounts come from
   eest_account (the witness touch set) sorted by raw address; per account the
   changes/reads are joined from the accumulator by keccak(address). */

typedef struct { uint8_t *d; size_t n, cap; } bbuf;
static void bb_ensure(bbuf *b, size_t add) {
  if (b->n + add > b->cap) {
    size_t nc = b->cap ? b->cap * 2 : 256;
    while (nc < b->n + add) nc *= 2;
    b->d = (uint8_t *)realloc(b->d, nc);
    b->cap = nc;
  }
}
static void bb_byte(bbuf *b, uint8_t x) { bb_ensure(b, 1); b->d[b->n++] = x; }
static void bb_raw(bbuf *b, const uint8_t *p, size_t n) { if (!n) return; bb_ensure(b, n); memcpy(b->d + b->n, p, n); b->n += n; }
static void bb_free(bbuf *b) { free(b->d); b->d = NULL; b->n = 0; b->cap = 0; }

static void rlp_len_prefix(bbuf *b, size_t len, uint8_t off) {
  if (len < 56) { bb_byte(b, (uint8_t)(off + len)); return; }
  uint8_t lb[8]; int m = 0;
  for (size_t x = len; x; x >>= 8) lb[m++] = (uint8_t)(x & 0xff);
  bb_byte(b, (uint8_t)(off + 55 + m));
  for (int i = 0; i < m; i++) bb_byte(b, lb[m - 1 - i]);
}
static void rlp_str(bbuf *b, const uint8_t *p, size_t n) {
  if (n == 1 && p[0] < 0x80) { bb_byte(b, p[0]); return; }
  rlp_len_prefix(b, n, 0x80);
  bb_raw(b, p, n);
}
static void rlp_uint_be(bbuf *b, const uint8_t *be, size_t n) {   /* min-BE: strip leading zeros */
  size_t i = 0; while (i < n && be[i] == 0) i++;
  rlp_str(b, be + i, n - i);
}
static void rlp_list_of(bbuf *parent, const bbuf *payload) {
  rlp_len_prefix(parent, payload->n, 0xc0);
  bb_raw(parent, payload->d, payload->n);
}
static void be_words_be32(uint8_t out[32], const uint64_t w[4]) {   /* w[0]=MS (storage slot/value) */
  for (int i = 0; i < 4; i++) for (int j = 0; j < 8; j++) out[i * 8 + j] = (uint8_t)((w[i] >> (56 - 8 * j)) & 0xff);
}
static void le_words_be32(uint8_t out[32], const uint64_t w[4]) {   /* w[3]=MS (account balance) */
  for (int i = 0; i < 4; i++) { uint64_t x = w[3 - i]; for (int j = 0; j < 8; j++) out[i * 8 + j] = (uint8_t)((x >> (56 - 8 * j)) & 0xff); }
}
static void rlp_u32(bbuf *b, uint32_t v) { uint8_t be[4] = {(uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v}; rlp_uint_be(b, be, 4); }
static void rlp_u64(bbuf *b, uint64_t v) { uint8_t be[8]; for (int j = 0; j < 8; j++) be[j] = (uint8_t)((v >> (56 - 8 * j)) & 0xff); rlp_uint_be(b, be, 8); }

/* first index in a vec sorted by ah (ah at record offset 0) with ah >= target */
static uint32_t bal_ah_lower(const void *base, size_t esz, uint32_t n, const uint64_t ah[4]) {
  uint32_t lo = 0, hi = n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    const uint64_t *m = (const uint64_t *)((const char *)base + (size_t)mid * esz);
    if (compare_u64x4(m, ah) < 0) lo = mid + 1; else hi = mid;
  }
  return lo;
}

static int bal_sto_cmp(const void *x, const void *y) {
  const bal_sto_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  c = compare_u64x4(a->slot, b->slot); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_read_cmp(const void *x, const void *y) {
  const bal_read_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  return compare_u64x4(a->slot, b->slot);
}
static int bal_bal_cmp(const void *x, const void *y) {
  const bal_bal_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_non_cmp(const void *x, const void *y) {
  const bal_non_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_cod_cmp(const void *x, const void *y) {
  const bal_cod_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_acc_cmp(const void *x, const void *y) {   /* accounts by raw 20-byte address */
  const acct_state_row *const *a = x, *const *b = y;
  return memcmp((*a)->raw_addr, (*b)->raw_addr, 20);
}

/* whether (ah, slot) has a storage change (to exclude it from reads) */
static int bal_slot_changed(const uint64_t ah[4], const uint64_t slot[4]) {
  uint32_t i = bal_ah_lower(bal_sto.d, sizeof(bal_sto_rec), bal_sto.n, ah);
  const bal_sto_rec *r = (const bal_sto_rec *)bal_sto.d;
  for (; i < bal_sto.n && compare_u64x4(r[i].ah, ah) == 0; i++)
    if (compare_u64x4(r[i].slot, slot) == 0) return 1;
  return 0;
}

void bal_recompute_hash(lbits *rop, const unit u) {
  (void)u;
  qsort(bal_sto.d, bal_sto.n, sizeof(bal_sto_rec), bal_sto_cmp);
  qsort(bal_rds.d, bal_rds.n, sizeof(bal_read_rec), bal_read_cmp);
  qsort(bal_balc.d, bal_balc.n, sizeof(bal_bal_rec), bal_bal_cmp);
  qsort(bal_nonc.d, bal_nonc.n, sizeof(bal_non_rec), bal_non_cmp);
  qsort(bal_codc.d, bal_codc.n, sizeof(bal_cod_rec), bal_cod_cmp);

  /* The witness touched-account set is the cumulative account table
     (read members seeded by the base resolver + written members from merges);
     eest_account is native-only and never populated on the witness path. */
  uint32_t na = acct_block_table.n;
  const acct_state_row **accs = (const acct_state_row **)malloc((na ? na : 1) * sizeof(*accs));
  for (uint32_t i = 0; i < na; i++) accs[i] = &acct_block_table.rows[i];
  qsort(accs, na, sizeof(*accs), bal_acc_cmp);

  const bal_sto_rec *sto = (const bal_sto_rec *)bal_sto.d;
  const bal_read_rec *rds = (const bal_read_rec *)bal_rds.d;
  const bal_bal_rec *balc = (const bal_bal_rec *)bal_balc.d;
  const bal_non_rec *nonc = (const bal_non_rec *)bal_nonc.d;
  const bal_cod_rec *codc = (const bal_cod_rec *)bal_codc.d;

  bbuf top = {NULL, 0, 0};
  for (uint32_t ai = 0; ai < na; ai++) {
    const acct_state_row *acc = accs[ai];
    const uint64_t *ah = acc->hkey;
    bbuf ap = {NULL, 0, 0};
    { uint8_t a20[20]; memcpy(a20, acc->raw_addr, 20); rlp_str(&ap, a20, 20); }

    /* storage_changes: [ [slot, [ [idx, val], ... ]], ... ] */
    bbuf sc = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(sto, sizeof(*sto), bal_sto.n, ah); i < bal_sto.n && compare_u64x4(sto[i].ah, ah) == 0; ) {
      uint32_t j = i;
      bbuf slot_entry = {NULL, 0, 0};
      { uint8_t sbe[32]; be_words_be32(sbe, sto[i].slot); rlp_uint_be(&slot_entry, sbe, 32); }
      bbuf changes = {NULL, 0, 0};
      while (j < bal_sto.n && compare_u64x4(sto[j].ah, ah) == 0 && compare_u64x4(sto[j].slot, sto[i].slot) == 0) {
        uint32_t k = j;
        while (k + 1 < bal_sto.n && compare_u64x4(sto[k + 1].ah, ah) == 0 &&
               compare_u64x4(sto[k + 1].slot, sto[i].slot) == 0 && sto[k + 1].idx == sto[j].idx) k++;
        bbuf pair = {NULL, 0, 0};
        rlp_u32(&pair, sto[k].idx);
        { uint8_t vbe[32]; be_words_be32(vbe, sto[k].val); rlp_uint_be(&pair, vbe, 32); }
        rlp_list_of(&changes, &pair); bb_free(&pair);
        j = k + 1;
      }
      rlp_list_of(&slot_entry, &changes); bb_free(&changes);
      rlp_list_of(&sc, &slot_entry); bb_free(&slot_entry);
      i = j;
    }
    rlp_list_of(&ap, &sc); bb_free(&sc);

    /* storage_reads: distinct touched slots not present in storage_changes */
    bbuf sr = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(rds, sizeof(*rds), bal_rds.n, ah); i < bal_rds.n && compare_u64x4(rds[i].ah, ah) == 0; ) {
      uint32_t j = i;
      while (j + 1 < bal_rds.n && compare_u64x4(rds[j + 1].ah, ah) == 0 && compare_u64x4(rds[j + 1].slot, rds[i].slot) == 0) j++;
      if (!bal_slot_changed(ah, rds[i].slot)) { uint8_t sbe[32]; be_words_be32(sbe, rds[i].slot); rlp_uint_be(&sr, sbe, 32); }
      i = j + 1;
    }
    rlp_list_of(&ap, &sr); bb_free(&sr);

    /* balance_changes: [ [idx, balance], ... ] (final per idx) */
    bbuf bc = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(balc, sizeof(*balc), bal_balc.n, ah); i < bal_balc.n && compare_u64x4(balc[i].ah, ah) == 0; ) {
      uint32_t k = i;
      while (k + 1 < bal_balc.n && compare_u64x4(balc[k + 1].ah, ah) == 0 && balc[k + 1].idx == balc[i].idx) k++;
      bbuf pair = {NULL, 0, 0};
      rlp_u32(&pair, balc[k].idx);
      { uint8_t vbe[32]; le_words_be32(vbe, balc[k].val); rlp_uint_be(&pair, vbe, 32); }
      rlp_list_of(&bc, &pair); bb_free(&pair);
      i = k + 1;
    }
    rlp_list_of(&ap, &bc); bb_free(&bc);

    /* nonce_changes: [ [idx, nonce], ... ] */
    bbuf ncb = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(nonc, sizeof(*nonc), bal_nonc.n, ah); i < bal_nonc.n && compare_u64x4(nonc[i].ah, ah) == 0; ) {
      uint32_t k = i;
      while (k + 1 < bal_nonc.n && compare_u64x4(nonc[k + 1].ah, ah) == 0 && nonc[k + 1].idx == nonc[i].idx) k++;
      bbuf pair = {NULL, 0, 0};
      rlp_u32(&pair, nonc[k].idx);
      rlp_u64(&pair, nonc[k].val);
      rlp_list_of(&ncb, &pair); bb_free(&pair);
      i = k + 1;
    }
    rlp_list_of(&ap, &ncb); bb_free(&ncb);

    /* code_changes: [ [idx, code_bytes], ... ] */
    bbuf cc = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(codc, sizeof(*codc), bal_codc.n, ah); i < bal_codc.n && compare_u64x4(codc[i].ah, ah) == 0; ) {
      uint32_t k = i;
      while (k + 1 < bal_codc.n && compare_u64x4(codc[k + 1].ah, ah) == 0 && codc[k + 1].idx == codc[i].idx) k++;
      bbuf pair = {NULL, 0, 0};
      rlp_u32(&pair, codc[k].idx);
      { uint64_t key_be[4]; for (int q = 0; q < 4; q++) key_be[q] = codc[k].chash[3 - q];  /* le-words -> be-words */
        uint64_t clen = 0; const uint8_t *cp = code_db_code_by_words(key_be, &clen);
        rlp_str(&pair, cp ? cp : (const uint8_t *)"", (size_t)clen); }
      rlp_list_of(&cc, &pair); bb_free(&pair);
      i = k + 1;
    }
    rlp_list_of(&ap, &cc); bb_free(&cc);

    rlp_list_of(&top, &ap); bb_free(&ap);
  }

  bbuf out = {NULL, 0, 0};
  rlp_list_of(&out, &top);
  host_keccak256_lbits(rop, out.d ? out.d : (const uint8_t *)"", out.n);
  bb_free(&out); bb_free(&top);
  free(accs);
}
