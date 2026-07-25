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
#include "value_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* secure trie key derivation: keccak(address) for accounts, keccak(slot) for
 * storage. The account/storage hooks are keyed by raw (address[, slot]); this
 * computes the secure key on every call. NOTE: keccak is pure, so a preimage ->
 * hash memo would avoid re-hashing repeated touches of the same address/slot --
 * removed for now to keep the key path stateless. */
static void secure_keccak_address(sail_address value, uint64_t out[4]) {
  uint8_t bytes[20];
  evmsail_address_to_be_bytes(bytes, value);
  host_keccak256_bytes(out, bytes, sizeof(bytes));
}

static void secure_keccak_word(sail_word value, uint64_t out[4]) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  host_keccak256_bytes(out, bytes, sizeof(bytes));
}

/* h = keccak(address) -- the account secure trie key, BE words */
static void acct_secure_key(sail_address a, uint64_t h[4]) {
  secure_keccak_address(a, h);
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
/* Account and storage-read evidence is recorded at the semantic access      */
/* boundary and is monotone across frame reverts. Net changes are harvested  */
/* from the transaction write tables at merge time. This evidence is kept    */
/* independently of the state caches: cache membership is not an access log. */
/* Records carry the ambient block_access_index (0 pre-exec syscalls, i+1    */
/* for user transaction i, N+1 post).                                        */
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
typedef struct { uint64_t ah[4]; uint8_t raw_addr[20]; } bal_acc_rec;
typedef struct { bal_acc_rec *rows; uint32_t n, cap; } bal_acc_table;

static bal_vec bal_sto  = {NULL, 0, 0, sizeof(bal_sto_rec)};
static bal_vec bal_rds  = {NULL, 0, 0, sizeof(bal_read_rec)};
static bal_vec bal_balc = {NULL, 0, 0, sizeof(bal_bal_rec)};
static bal_vec bal_nonc = {NULL, 0, 0, sizeof(bal_non_rec)};
static bal_vec bal_codc = {NULL, 0, 0, sizeof(bal_cod_rec)};
static bal_acc_table bal_acc = {NULL, 0, 0};
static const bal_acc_rec **bal_acc_order = NULL;
static uint32_t bal_acc_order_cap = 0;
static uint32_t bal_seq = 0;
static int bal_prepared = 0;
typedef struct {
  uint32_t begin;
  uint32_t end;
  uint32_t position;
} bal_cursor;
static uint32_t bal_account_cursor = 0;
static const bal_acc_rec *bal_active_account = NULL;
static bal_cursor bal_storage_slot_change_cursor = {0, 0, 0};
static bal_cursor bal_storage_read_cursor = {0, 0, 0};
static bal_cursor bal_active_storage_changes = {0, 0, 0};
static bal_cursor bal_balance_change_cursor = {0, 0, 0};
static bal_cursor bal_nonce_change_cursor = {0, 0, 0};
static bal_cursor bal_code_change_cursor = {0, 0, 0};

static uint32_t bal_acc_find(const uint64_t ah[4], int *found) {
  uint32_t lo = 0, hi = bal_acc.n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_u64x4(bal_acc.rows[mid].ah, ah) < 0) lo = mid + 1;
    else hi = mid;
  }
  *found = lo < bal_acc.n && compare_u64x4(bal_acc.rows[lo].ah, ah) == 0;
  return lo;
}

static void bal_touch_account_key(sail_address a, const uint64_t ah[4]) {
  int found = 0;
  uint32_t i = bal_acc_find(ah, &found);
  if (found) return;
  if (bal_acc.cap < bal_acc.n + 1) {
    uint32_t cap = bal_acc.cap ? bal_acc.cap * 2 : 16;
    bal_acc_rec *rows = realloc(bal_acc.rows, (size_t)cap * sizeof(*rows));
    if (!rows) return;
    bal_acc.rows = rows;
    bal_acc.cap = cap;
  }
  if (i < bal_acc.n)
    memmove(&bal_acc.rows[i + 1], &bal_acc.rows[i],
            (size_t)(bal_acc.n - i) * sizeof(*bal_acc.rows));
  memcpy(bal_acc.rows[i].ah, ah, sizeof(bal_acc.rows[i].ah));
  evmsail_address_to_be_bytes(bal_acc.rows[i].raw_addr, a);
  bal_acc.n++;
  bal_prepared = 0;
}

unit bal_reset(const unit u) {
  (void)u;
  bal_vec_clear(&bal_sto); bal_vec_clear(&bal_rds); bal_vec_clear(&bal_balc);
  bal_vec_clear(&bal_nonc); bal_vec_clear(&bal_codc);
  free(bal_acc.rows); bal_acc.rows = NULL; bal_acc.n = bal_acc.cap = 0;
  free(bal_acc_order); bal_acc_order = NULL; bal_acc_order_cap = 0;
  bal_seq = 0;
  bal_prepared = 0;
  bal_account_cursor = 0;
  bal_active_account = NULL;
  bal_storage_slot_change_cursor = (bal_cursor){0, 0, 0};
  bal_storage_read_cursor = (bal_cursor){0, 0, 0};
  bal_active_storage_changes = (bal_cursor){0, 0, 0};
  bal_balance_change_cursor = (bal_cursor){0, 0, 0};
  bal_nonce_change_cursor = (bal_cursor){0, 0, 0};
  bal_code_change_cursor = (bal_cursor){0, 0, 0};
  return UNIT;
}

/* harvest helpers -- called from the overlay merges (state_db-internal) */
static void bal_add_storage_change(uint32_t index, const uint64_t ah[4],
                                   const uint64_t slot[4],
                                   const uint64_t val[4]) {
  bal_sto_rec *r = (bal_sto_rec *)bal_vec_push(&bal_sto);
  memcpy(r->ah, ah, 32); memcpy(r->slot, slot, 32); memcpy(r->val, val, 32);
  r->idx = index; r->seq = bal_seq++;
  bal_prepared = 0;
}
static void bal_add_storage_read(const uint64_t ah[4], const uint64_t slot[4]) {
  bal_read_rec *r = (bal_read_rec *)bal_vec_push(&bal_rds);
  memcpy(r->ah, ah, 32); memcpy(r->slot, slot, 32);
  bal_prepared = 0;
}
static void bal_add_balance_change(uint32_t index, const uint64_t ah[4],
                                   const uint64_t val[4]) {
  bal_bal_rec *r = (bal_bal_rec *)bal_vec_push(&bal_balc);
  memcpy(r->ah, ah, 32); memcpy(r->val, val, 32);
  r->idx = index; r->seq = bal_seq++;
  bal_prepared = 0;
}
static void bal_add_nonce_change(uint32_t index, const uint64_t ah[4],
                                 uint64_t nonce) {
  bal_non_rec *r = (bal_non_rec *)bal_vec_push(&bal_nonc);
  memcpy(r->ah, ah, 32); r->val = nonce;
  r->idx = index; r->seq = bal_seq++;
  bal_prepared = 0;
}
static void bal_add_code_change(uint32_t index, const uint64_t ah[4],
                                const uint64_t chash[4]) {
  bal_cod_rec *r = (bal_cod_rec *)bal_vec_push(&bal_codc);
  memcpy(r->ah, ah, 32); memcpy(r->chash, chash, 32);
  r->idx = index; r->seq = bal_seq++;
  bal_prepared = 0;
}

/* Sail-facing EIP-7928 record sinks. Layouts match the row
   fields the serializer decodes: hashes/slots/storage values BE words,
   balance/code-hash LE words. */
unit bal_note_account_touch(sail_address a) {
  uint64_t a4[4];
  secure_keccak_address(a, a4);
  bal_touch_account_key(a, a4);
  return UNIT;
}
unit bal_note_storage_change(uint64_t index, sail_address a,
                             EVMSAIL_WORD_PARAM(slot),
                             EVMSAIL_WORD_PARAM(val)) {
  uint64_t a4[4], s4[4], v4[4];
  secure_keccak_address(a, a4);
  sail_word_to_be_words4(s4, EVMSAIL_WORD_VALUE(slot));
  sail_word_to_be_words4(v4, EVMSAIL_WORD_VALUE(val));
  bal_touch_account_key(a, a4);
  bal_add_storage_change((uint32_t)index, a4, s4, v4);
  return UNIT;
}
unit bal_note_storage_read(sail_address a, EVMSAIL_WORD_PARAM(slot)) {
  uint64_t a4[4], s4[4];
  secure_keccak_address(a, a4);
  sail_word_to_be_words4(s4, EVMSAIL_WORD_VALUE(slot));
  bal_touch_account_key(a, a4);
  bal_add_storage_read(a4, s4);
  return UNIT;
}
unit bal_note_balance_change(uint64_t index, sail_address a,
                             EVMSAIL_WORD_PARAM(val)) {
  uint64_t a4[4], v4[4];
  secure_keccak_address(a, a4);
  sail_word_to_le_words4(v4, EVMSAIL_WORD_VALUE(val));
  bal_touch_account_key(a, a4);
  bal_add_balance_change((uint32_t)index, a4, v4);
  return UNIT;
}
unit bal_note_nonce_change(uint64_t index, sail_address a, uint64_t nonce) {
  uint64_t a4[4];
  secure_keccak_address(a, a4);
  bal_touch_account_key(a, a4);
  bal_add_nonce_change((uint32_t)index, a4, nonce);
  return UNIT;
}
unit bal_note_code_change(uint64_t index, sail_address a, sail_hash chash) {
  uint64_t a4[4], c4[4];
  secure_keccak_address(a, a4);
  sail_hash_to_le_words4(c4, chash);
  bal_touch_account_key(a, a4);
  bal_add_code_change((uint32_t)index, a4, c4);
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
static void storage_secure_key(sail_address a, sail_word s,
                               uint64_t slot[4], uint64_t ah[4], uint64_t sh[4]) {
  sail_word_to_be_words4(slot, s);
  secure_keccak_address(a, ah);
  secure_keccak_word(s, sh);
}

static const uint64_t storage_zero_val[4] = {0, 0, 0, 0};

/* ======================================================================== */
/* PERSISTENT STORAGE STATE                                                  */
/*                                                                          */
/* The block table is the cumulative read cache/update map. The transaction  */
/* table contains writes only. Storage-clear generations and their rollback  */
/* history are host-private: Sail sees only the active value and its          */
/* transaction-start original. A miss in both layers asks the authenticated  */
/* base in Sail.                                                             */
/* ======================================================================== */

typedef struct {
  uint64_t acct_hash[4];
  uint64_t generation;
  uint64_t slot_hash[4];
  uint64_t slot[4];
  uint8_t  raw_addr[20];
  uint64_t current[4];
  uint64_t original[4];
  uint8_t written;
} storage_state_row;

typedef struct { storage_state_row *rows; uint32_t n, cap; } storage_state_table;

static storage_state_table storage_tx_table    = {NULL, 0, 0};
static storage_state_table storage_block_table = {NULL, 0, 0};
static uint32_t storage_tx_pop_cursor = 0; /* k_tx_merge drain position */
enum { STORAGE_BLOCK_GENERATION = 0, STORAGE_INITIAL_GENERATION = 1 };
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
  uint8_t prior_present;
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

/* Insert-if-absent, keeping the table sorted. */
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

static void storage_table_remove(storage_state_table *t,
                                 const uint64_t ah[4], uint64_t generation,
                                 const uint64_t sh[4]) {
  int found = 0;
  uint32_t i = storage_table_find(t, ah, generation, sh, &found);
  if (!found) return;
  if (i + 1 < t->n)
    memmove(&t->rows[i], &t->rows[i + 1],
            (size_t)(t->n - i - 1) * sizeof(*t->rows));
  t->n--;
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
      if (undo->prior_present) {
        storage_state_row *row = storage_table_get(
            &storage_tx_table, undo->acct_hash, undo->generation,
            undo->slot_hash);
        if (!row) abort();
        memcpy(row->current, undo->prior_current, sizeof(row->current));
      } else {
        storage_table_remove(&storage_tx_table, undo->acct_hash,
                             undo->generation, undo->slot_hash);
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

unit storage_tx_clear(sail_address a) {
  uint64_t ah[4];
  secure_keccak_address(a, ah);
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

/* Drain active transaction writes. Obsolete generations remain private
   rollback history and are skipped. */
uint64_t storage_tx_pop_probe(sail_address *addr, sail_word *slot,
                              sail_word *curr, sail_word *orig) {
  while (storage_tx_pop_cursor < storage_tx_table.n) {
    const storage_state_row *row =
        &storage_tx_table.rows[storage_tx_pop_cursor++];
    if (row->generation != storage_active_generation(row->acct_hash))
      continue;
    *addr = be_bytes_to_sail_address(row->raw_addr);
    *slot = be_words4_to_sail_word(row->slot);
    *curr = be_words4_to_sail_word(row->current);
    *orig = be_words4_to_sail_word(row->original);
    return 1;
  }
  return 0;
}

/* block-layer propagation hooks (pure mechanism; Sail chose the action).
   Keyed by (keccak(address), slot); the slot's secure key is recomputed. */
/* net change: a fresh block row freezes orig as the pre-state; curr lands */
unit storage_block_put_raw(sail_address a, sail_word s_, sail_word curr,
                           sail_word orig) {
  uint64_t slot[4], a4[4], sh[4], c4[4], o4[4];
  storage_secure_key(a, s_, slot, a4, sh);
  storage_state_row *b = storage_table_get(
      &storage_block_table, a4, STORAGE_BLOCK_GENERATION, sh);
  int fresh = (b == NULL);
  if (fresh) b = storage_table_intern(
      &storage_block_table, a4, STORAGE_BLOCK_GENERATION, sh, slot);
  if (!b) return UNIT;
  sail_word_to_be_words4(c4, curr);
  if (fresh) {
    sail_word_to_be_words4(o4, orig);
    memcpy(b->original, o4, sizeof(b->original));
  }
  memcpy(b->current, c4, sizeof(b->current));
  b->written = 1;
  storage_dump_invalidate();
  return UNIT;
}

/* read member: fresh binds curr == orig = value; existing is only marked */
unit storage_block_cache_raw(sail_address a, sail_word s_, sail_word v) {
  uint64_t slot[4], a4[4], sh[4], v4[4];
  storage_secure_key(a, s_, slot, a4, sh);
  storage_state_row *b = storage_table_get(
      &storage_block_table, a4, STORAGE_BLOCK_GENERATION, sh);
  if (!b) {
    b = storage_table_intern(
        &storage_block_table, a4, STORAGE_BLOCK_GENERATION, sh, slot);
    if (b) {
      sail_word_to_be_words4(v4, v);
      memcpy(b->current, v4, sizeof(b->current));
      memcpy(b->original, v4, sizeof(b->original));
    }
  }
  storage_dump_invalidate();
  return UNIT;
}


/* A transaction miss after a storage clear resolves to zero instead of
   falling through to the block/witness base. */
uint64_t storage_row_probe(uint64_t layer, sail_address a, sail_word s,
                           sail_word *cur, sail_word *orig) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  if (layer != 0) {
    storage_state_row *e = storage_table_get(
        &storage_block_table, ah, STORAGE_BLOCK_GENERATION, sh);
    if (!e) return 0;
    *cur = be_words4_to_sail_word(e->current);
    *orig = be_words4_to_sail_word(e->original);
    return 1;
  }

  const uint64_t generation = storage_active_generation(ah);
  storage_state_row *row = storage_table_get(
      &storage_tx_table, ah, generation, sh);
  if (row) {
    *cur = be_words4_to_sail_word(row->current);
    *orig = be_words4_to_sail_word(row->original);
    return 1;
  }
  if (generation != STORAGE_INITIAL_GENERATION) {
    *cur = be_words4_to_sail_word(storage_zero_val);
    *orig = be_words4_to_sail_word(storage_zero_val);
    return 1;
  }
  return 0;
}

/* Write the active generation. Sail supplies the transaction original from
   the preceding semantic SLOAD. */
unit storage_tx_update_raw(sail_address a, sail_word s, sail_word v,
                           sail_word orig) {
  uint64_t slot[4], ah[4], sh[4], w[4], o[4];
  storage_secure_key(a, s, slot, ah, sh);
  sail_word_to_be_words4(w, v);
  sail_word_to_be_words4(o, orig);
  const uint64_t generation = storage_active_generation(ah);
  storage_state_row *e = storage_table_get(&storage_tx_table, ah, generation, sh);
  const int fresh = (e == NULL);
  storage_undo_entry *undo = storage_undo_push(STORAGE_UNDO_WRITE);
  if (!undo) return UNIT;
  memcpy(undo->acct_hash, ah, sizeof(undo->acct_hash));
  undo->generation = generation;
  memcpy(undo->slot_hash, sh, sizeof(undo->slot_hash));
  undo->prior_present = !fresh;
  if (!fresh) memcpy(undo->prior_current, e->current, sizeof(undo->prior_current));
  if (!e) {
    e = storage_table_intern(&storage_tx_table, ah, generation, sh, slot);
    if (!e) {
      storage_undo_n--;
      return UNIT;
    }
    evmsail_address_to_be_bytes(e->raw_addr, a);
    memcpy(e->original, o, sizeof(e->original));
  }
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
unit storage_block_clear(sail_address a) {
  uint64_t h[4];
  secure_keccak_address(a, h);
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
bool storage_has_writes(sail_address a) {
  uint64_t ah[4];
  uint32_t start, end;
  secure_keccak_address(a, ah);

  storage_table_account_range(&storage_tx_table, ah, &start, &end);
  const uint64_t generation = storage_active_generation(ah);
  for (uint32_t i = start; i < end; i++) {
    const storage_state_row *row = &storage_tx_table.rows[i];
    if (row->generation == generation &&
        compare_words(row->current, storage_zero_val, 4) != 0)
      return true;
  }

  storage_table_account_range(&storage_block_table, ah, &start, &end);
  for (uint32_t i = start; i < end; i++) {
    if (compare_words(storage_block_table.rows[i].current,
                      storage_zero_val, 4) != 0)
      return true;
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

/* Non-destructive ascending iteration over one account's cumulative storage
   rows. Keeping the cursor private avoids exporting a host-sized index into
   the Sail model while leaving the underlying table available for debug. */
static uint64_t storage_block_iter_key[4];
static uint32_t storage_block_iter_position = 0;
static uint32_t storage_block_iter_end = 0;
static bool storage_block_iter_active = false;

unit storage_block_iter_begin(sail_address a) {
  secure_keccak_address(a, storage_block_iter_key);
  storage_block_account_range(storage_block_iter_key,
                              &storage_block_iter_position,
                              &storage_block_iter_end);
  storage_block_iter_active = true;
  return UNIT;
}

uint64_t storage_block_iter_next_probe(sail_address a, sail_word *slot,
                                       sail_word *curr, sail_word *orig) {
  uint64_t key[4];
  secure_keccak_address(a, key);
  if (!storage_block_iter_active ||
      compare_words(key, storage_block_iter_key, 4) != 0 ||
      storage_block_iter_end <= storage_block_iter_position)
    return 0;
  const storage_state_row *entry =
      &storage_block_table.rows[storage_block_iter_position++];
  *slot = be_words4_to_sail_word(entry->slot);
  *curr = be_words4_to_sail_word(entry->current);
  *orig = be_words4_to_sail_word(entry->original);
  return 1;
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

uint64_t storage_dump_count(sail_word ak) {
  uint64_t k[4]; sail_word_to_be_words4(k, ak); storage_dump_build(k); return storage_dump_len;
}
sail_word storage_dump_slot(sail_word ak, uint64_t j) {
  uint64_t k[4]; sail_word_to_be_words4(k, ak); storage_dump_build(k);
  return be_words4_to_sail_word(j < storage_dump_len ? storage_dump_entries[j].slot : storage_zero_val);
}
sail_word storage_dump_value(sail_word ak, uint64_t j) {
  uint64_t k[4]; sail_word_to_be_words4(k, ak); storage_dump_build(k);
  return be_words4_to_sail_word(j < storage_dump_len ? storage_dump_entries[j].val : storage_zero_val);
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
  /* Derived traversal cache; never exposed as semantic account state. */
  uint64_t post_sroot[4];
  uint32_t snapshot_cursor;
  uint8_t written;
} acct_state_row;

typedef struct { acct_state_row *rows; uint32_t n, cap; } acct_state_table;

static acct_state_table acct_tx_table    = {NULL, 0, 0};
static acct_state_table acct_block_table = {NULL, 0, 0};
static uint32_t acct_tx_pop_cursor = 0; /* k_tx_merge drain position */
static uint32_t *acct_tx_pop_order = NULL;
static uint32_t acct_tx_pop_order_cap = 0;

typedef struct {
  uint64_t hkey[4];
  uint64_t nonce;
  uint64_t balance[4];
  uint64_t storage_root[4];
  uint64_t code_hash[4];
  uint32_t snapshot_cursor;
  uint8_t prior_present;
  uint8_t exists;
  uint8_t storage_cleared;
  uint8_t created;
  uint8_t selfdestructed;
} acct_undo_entry;

static acct_undo_entry *acct_undo = NULL;
static uint32_t acct_undo_n = 0, acct_undo_cap = 0;
static const uint32_t ACCT_NO_SNAPSHOT = UINT32_MAX;
static uint32_t acct_active_snapshot = UINT32_MAX;
static uint32_t acct_next_snapshot = 0;


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

static void acct_table_remove(acct_state_table *t, const uint64_t h[4]) {
  int found = 0;
  uint32_t i = acct_table_find(t, h, &found);
  if (!found) return;
  if (i + 1 < t->n)
    memmove(&t->rows[i], &t->rows[i + 1],
            (size_t)(t->n - i - 1) * sizeof(*t->rows));
  t->n--;
}

static int acct_raw_address_cmp(const void *a, const void *b) {
  uint32_t ai = *(const uint32_t *)a;
  uint32_t bi = *(const uint32_t *)b;
  return memcmp(acct_tx_table.rows[ai].raw_addr,
                acct_tx_table.rows[bi].raw_addr, 20);
}

static void acct_tx_prepare_pop_order(void) {
  if (acct_tx_pop_order_cap < acct_tx_table.n) {
    uint32_t cap = acct_tx_pop_order_cap ? acct_tx_pop_order_cap : ACCOUNT_INIT_CAP;
    while (cap < acct_tx_table.n) cap *= 2;
    uint32_t *order = realloc(acct_tx_pop_order, (size_t)cap * sizeof(*order));
    if (!order) abort();
    acct_tx_pop_order = order;
    acct_tx_pop_order_cap = cap;
  }
  for (uint32_t i = 0; i < acct_tx_table.n; i++) acct_tx_pop_order[i] = i;
  if (acct_tx_table.n > 1)
    qsort(acct_tx_pop_order, acct_tx_table.n,
          sizeof(*acct_tx_pop_order), acct_raw_address_cmp);
}

static acct_state_row *acct_tx_bind_write(sail_address a,
                                          const uint64_t h[4], int *fresh) {
  acct_state_row *row = acct_table_get(&acct_tx_table, h);
  *fresh = (row == NULL);
  if (!*fresh) return row;

  const acct_state_row *base = acct_table_get(&acct_block_table, h);
  if (!base) return NULL;
  row = acct_table_intern(&acct_tx_table, h);
  if (!row) return NULL;
  evmsail_address_to_be_bytes(row->raw_addr, a);
  row->cur_nonce = row->orig_nonce = base->cur_nonce;
  memcpy(row->cur_bal, base->cur_bal, sizeof(row->cur_bal));
  memcpy(row->orig_bal, base->cur_bal, sizeof(row->orig_bal));
  memcpy(row->cur_sroot, base->cur_sroot, sizeof(row->cur_sroot));
  memcpy(row->orig_sroot, base->cur_sroot, sizeof(row->orig_sroot));
  memcpy(row->cur_chash, base->cur_chash, sizeof(row->cur_chash));
  memcpy(row->orig_chash, base->cur_chash, sizeof(row->orig_chash));
  row->cur_exists = row->orig_exists = base->cur_exists;
  row->cur_storage_cleared = row->orig_storage_cleared =
      base->cur_storage_cleared;
  row->cur_created = row->orig_created = 0;
  row->cur_selfdestructed = row->orig_selfdestructed = 0;
  return row;
}

/* The active undo cursor is the lazy snapshot identity. Each account saves its
   reversible projection at most once at that cursor; successful child frames
   retain their entries because an enclosing frame may still revert them. */
static int acct_snapshot_for_write(acct_state_row *row, int prior_present) {
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
  memset(undo, 0, sizeof(*undo));
  memcpy(undo->hkey, row->hkey, sizeof(undo->hkey));
  undo->prior_present = prior_present;
  if (prior_present) {
    undo->nonce = row->cur_nonce;
    memcpy(undo->balance, row->cur_bal, sizeof(undo->balance));
    memcpy(undo->storage_root, row->cur_sroot, sizeof(undo->storage_root));
    memcpy(undo->code_hash, row->cur_chash, sizeof(undo->code_hash));
  }
  undo->snapshot_cursor = row->snapshot_cursor;
  undo->exists = row->cur_exists;
  undo->storage_cleared = row->cur_storage_cleared;
  undo->created = row->cur_created;
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
  free(acct_tx_pop_order);
  acct_tx_pop_order = NULL;
  acct_tx_pop_order_cap = 0;
  free(acct_undo);
  acct_undo = NULL;
  acct_undo_n = acct_undo_cap = 0;
  acct_active_snapshot = ACCT_NO_SNAPSHOT;
  acct_next_snapshot = 0;
  acct_dump_invalidate();
  return UNIT;
}

unit acct_tx_reset(const unit u) {
  (void)u;
  acct_tx_table.n = 0;
  acct_tx_pop_cursor = 0;
  acct_undo_n = 0;
  acct_active_snapshot = ACCT_NO_SNAPSHOT;
  acct_next_snapshot = 0;
  return UNIT;
}

uint64_t acct_tx_checkpoint(const unit u) {
  (void)u;
  uint32_t prior = acct_active_snapshot;
  uint32_t cursor = acct_undo_n;
  if (acct_next_snapshot == ACCT_NO_SNAPSHOT) abort();
  acct_active_snapshot = acct_next_snapshot++;
  /* The checkpoint is opaque to Sail: high = parent identity, low = cursor. */
  return ((uint64_t)prior << 32) | cursor;
}

unit acct_tx_revert(uint64_t checkpoint) {
  uint32_t cursor = (uint32_t)checkpoint;
  uint32_t prior = (uint32_t)(checkpoint >> 32);
  if (cursor > acct_undo_n) abort();
  while (acct_undo_n > cursor) {
    const acct_undo_entry *undo = &acct_undo[--acct_undo_n];
    if (!undo->prior_present) {
      acct_table_remove(&acct_tx_table, undo->hkey);
      continue;
    }
    acct_state_row *current = acct_table_get(&acct_tx_table, undo->hkey);
    if (!current) abort();
    current->cur_nonce = undo->nonce;
    memcpy(current->cur_bal, undo->balance, sizeof(current->cur_bal));
    memcpy(current->cur_sroot, undo->storage_root, sizeof(current->cur_sroot));
    memcpy(current->cur_chash, undo->code_hash, sizeof(current->cur_chash));
    current->cur_exists = undo->exists;
    current->cur_storage_cleared = undo->storage_cleared;
    current->cur_created = undo->created;
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
uint64_t acct_tx_pop_probe(sail_address *addr,
                           uint64_t *cn, sail_word *cb,
                           sail_hash *cs, sail_hash *cc,
                           bool *ce, bool *csc, bool *ccr, bool *csd,
                           uint64_t *on, sail_word *ob,
                           sail_hash *os, sail_hash *oc,
                           bool *oe, bool *osc, bool *ocr, bool *osd) {
  if (acct_tx_pop_cursor == 0) acct_tx_prepare_pop_order();
  if (acct_tx_pop_cursor >= acct_tx_table.n) {
    return 0;
  }
  const acct_state_row *e =
      &acct_tx_table.rows[acct_tx_pop_order[acct_tx_pop_cursor++]];
  *addr = be_bytes_to_sail_address(e->raw_addr);
  *cn = e->cur_nonce;
  *cb = le_words4_to_sail_word(e->cur_bal);
  *cs = le_words4_to_sail_hash(e->cur_sroot);
  *cc = le_words4_to_sail_hash(e->cur_chash);
  *ce = e->cur_exists;
  *csc = e->cur_storage_cleared;
  *ccr = e->cur_created;
  *csd = e->cur_selfdestructed;
  *on = e->orig_nonce;
  *ob = le_words4_to_sail_word(e->orig_bal);
  *os = le_words4_to_sail_hash(e->orig_sroot);
  *oc = le_words4_to_sail_hash(e->orig_chash);
  *oe = e->orig_exists;
  *osc = e->orig_storage_cleared;
  *ocr = e->orig_created;
  *osd = e->orig_selfdestructed;
  return 1;
}

/* block-layer propagation hooks (pure mechanism; Sail chose the action).
   Keyed by the address preimage: the row keeps it for BAL serialization. */
static acct_state_row *acct_block_bind(sail_address a, int *fresh) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_state_row *b = acct_table_get(&acct_block_table, h);
  *fresh = (b == NULL);
  if (*fresh) b = acct_table_intern(&acct_block_table, h);
  if (b) evmsail_address_to_be_bytes(b->raw_addr, a);
  return b;
}

/* changed account: fresh freezes orig; curr always lands */
unit acct_block_write_raw(sail_address a, uint64_t nonce, sail_word bal,
                          sail_hash sroot, sail_hash chash,
                          bool exists, bool storage_cleared,
                          uint64_t ononce, sail_word obal,
                          sail_hash osroot, sail_hash ochash,
                          bool oexists, bool ostorage_cleared) {
  int fresh = 0;
  acct_state_row *b = acct_block_bind(a, &fresh);
  if (!b) return UNIT;
  if (fresh) {
    b->orig_nonce = ononce;
    sail_word_to_le_words4(b->orig_bal, obal);
    sail_hash_to_le_words4(b->orig_sroot, osroot);
    sail_hash_to_le_words4(b->orig_chash, ochash);
    b->orig_exists = oexists;
    b->orig_storage_cleared = ostorage_cleared;
  }
  b->cur_nonce = nonce;
  sail_word_to_le_words4(b->cur_bal, bal);
  sail_hash_to_le_words4(b->cur_sroot, sroot);
  sail_hash_to_le_words4(b->cur_chash, chash);
  b->cur_exists = exists;
  b->cur_storage_cleared = storage_cleared;
  b->written = 1;
  acct_dump_invalidate();
  return UNIT;
}

/* read member: fresh binds curr == orig; an existing row is untouched */
unit acct_block_cache_raw(sail_address a, uint64_t nonce, sail_word bal,
                          sail_hash sroot, sail_hash chash,
                          bool exists, bool storage_cleared) {
  int fresh = 0;
  acct_state_row *b = acct_block_bind(a, &fresh);
  if (!b) return UNIT;
  if (fresh) {
    b->cur_nonce = nonce;  b->orig_nonce = nonce;
    sail_word_to_le_words4(b->cur_bal, bal);
    sail_word_to_le_words4(b->orig_bal, bal);
    sail_hash_to_le_words4(b->cur_sroot, sroot);
    sail_hash_to_le_words4(b->orig_sroot, sroot);
    sail_hash_to_le_words4(b->cur_chash, chash);
    sail_hash_to_le_words4(b->orig_chash, chash);
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
uint64_t acct_row_probe(uint64_t layer, sail_address a, uint64_t *nonce,
                        sail_word *bal, sail_hash *sroot, sail_hash *chash,
                        bool *exists, bool *storage_cleared,
                        bool *created, bool *selfdestructed) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_state_row *e = acct_table_get(layer == 0 ? &acct_tx_table : &acct_block_table, h);
  if (!e) return 0;
  *nonce = e->cur_nonce;
  *bal = le_words4_to_sail_word(e->cur_bal);
  *sroot = le_words4_to_sail_hash(e->cur_sroot);
  *chash = le_words4_to_sail_hash(e->cur_chash);
  *exists = e->cur_exists;
  *storage_cleared = e->cur_storage_cleared;
  *created = e->cur_created;
  *selfdestructed = e->cur_selfdestructed;
  return 1;
}

/* Selects rows that may have a net protocol change without deciding whether
   they actually do. Sail owns the current/original equality check. */
static bool acct_block_row_is_update_candidate(const acct_state_row *entry) {
  if (entry->written)
    return true;

  uint32_t start, end;
  storage_block_account_range(entry->hkey, &start, &end);
  for (uint32_t i = start; i < end; i++) {
    const storage_state_row *storage = &storage_block_table.rows[i];
    if (storage->written)
      return true;
  }
  return false;
}

/* Non-destructive ascending iteration over account update candidates. */
static uint32_t acct_block_iter_position = 0;
static bool acct_block_iter_active = false;

/* The debug iterator deliberately has independent state and includes cached,
   read-only rows. It is never used by the protocol state-root path. */
static uint32_t acct_debug_iter_position = 0;
static bool acct_debug_iter_active = false;

unit acct_block_iter_begin(const unit u) {
  (void)u;
  acct_block_iter_position = 0;
  acct_block_iter_active = true;
  return UNIT;
}

unit acct_debug_iter_begin(const unit u) {
  (void)u;
  acct_debug_iter_position = 0;
  acct_debug_iter_active = true;
  return UNIT;
}

unit acct_post_storage_root_store(sail_address a, sail_hash root) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_state_row *entry = acct_table_get(&acct_block_table, h);
  if (entry) sail_hash_to_le_words4(entry->post_sroot, root);
  return UNIT;
}

EVMSAIL_HASH_RETURN acct_post_storage_root_read(
    EVMSAIL_HASH_RESULT(result) sail_address a) {
  uint64_t h[4];
  acct_secure_key(a, h);
  const acct_state_row *entry = acct_table_get(&acct_block_table, h);
  const uint64_t *le = entry ? entry->post_sroot : account_zero_val;
  const uint64_t be[4] = {le[3], le[2], le[1], le[0]};
  uint8_t bytes[32];
  be_words4_to_be_bytes(bytes, be);
  EVMSAIL_RETURN_HASH_BE_BYTES(result, bytes);
}

static uint64_t acct_iter_next_probe(uint32_t *position, bool *active,
                                     const bool candidates_only,
                                     sail_address *addr, uint64_t *cn,
                                     sail_word *cb, sail_hash *cs,
                                     sail_hash *cc, bool *ce, bool *csc,
                                     bool *ccr, bool *csd, uint64_t *on,
                                     sail_word *ob, sail_hash *os,
                                     sail_hash *oc, bool *oe, bool *osc,
                                     bool *ocr, bool *osd) {
  if (!*active)
    return 0;
  const acct_state_row *entry = NULL;
  while (*position < acct_block_table.n) {
    const acct_state_row *candidate = &acct_block_table.rows[(*position)++];
    if (!candidates_only || acct_block_row_is_update_candidate(candidate)) {
      entry = candidate;
      break;
    }
  }
  if (!entry) return 0;
  *addr = be_bytes_to_sail_address(entry->raw_addr);
  *cn = entry->cur_nonce;
  *cb = le_words4_to_sail_word(entry->cur_bal);
  *cs = le_words4_to_sail_hash(entry->cur_sroot);
  *cc = le_words4_to_sail_hash(entry->cur_chash);
  *ce = entry->cur_exists;
  *csc = entry->cur_storage_cleared;
  *ccr = entry->cur_created;
  *csd = entry->cur_selfdestructed;
  *on = entry->orig_nonce;
  *ob = le_words4_to_sail_word(entry->orig_bal);
  *os = le_words4_to_sail_hash(entry->orig_sroot);
  *oc = le_words4_to_sail_hash(entry->orig_chash);
  *oe = entry->orig_exists;
  *osc = entry->orig_storage_cleared;
  *ocr = entry->orig_created;
  *osd = entry->orig_selfdestructed;
  return 1;
}

uint64_t acct_block_iter_next_probe(sail_address *addr, uint64_t *cn,
                                    sail_word *cb, sail_hash *cs,
                                    sail_hash *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_word *ob, sail_hash *os,
                                    sail_hash *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd) {
  return acct_iter_next_probe(
      &acct_block_iter_position, &acct_block_iter_active, true, addr, cn, cb,
      cs, cc, ce, csc, ccr, csd, on, ob, os, oc, oe, osc, ocr, osd);
}

uint64_t acct_debug_iter_next_probe(sail_address *addr, uint64_t *cn,
                                    sail_word *cb, sail_hash *cs,
                                    sail_hash *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_word *ob, sail_hash *os,
                                    sail_hash *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd) {
  return acct_iter_next_probe(
      &acct_debug_iter_position, &acct_debug_iter_active, false, addr, cn, cb,
      cs, cc, ce, csc, ccr, csd, on, ob, os, oc, oe, osc, ocr, osd);
}

/* Transaction rows are allocated lazily on the first write by cloning the
   cumulative block value established by k_aload. */
unit acct_tx_update_raw(sail_address a, uint64_t nonce,
                        sail_word bal, sail_hash sroot, sail_hash chash,
                        bool exists, bool storage_cleared, bool created,
                        bool selfdestructed) {
  uint64_t h[4];
  int fresh = 0;
  acct_secure_key(a, h);
  acct_state_row *e = acct_tx_bind_write(a, h, &fresh);
  if (!e) return UNIT;
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  uint64_t b4[4], sr[4], ch[4];
  sail_word_to_le_words4(b4, bal);
  sail_hash_to_le_words4(sr, sroot);
  sail_hash_to_le_words4(ch, chash);
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

unit acct_tx_set_balance(sail_address a, EVMSAIL_WORD_PARAM(balance)) {
  uint64_t h[4], value[4];
  int fresh = 0;
  acct_secure_key(a, h);
  sail_word_to_le_words4(value, EVMSAIL_WORD_VALUE(balance));
  acct_state_row *e = acct_tx_bind_write(a, h, &fresh);
  if (!e) return UNIT;
  if (compare_u64x4(e->cur_bal, value) == 0) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  memcpy(e->cur_bal, value, sizeof(e->cur_bal));
  return UNIT;
}

unit acct_tx_set_nonce(sail_address a, uint64_t nonce) {
  uint64_t h[4];
  int fresh = 0;
  acct_secure_key(a, h);
  acct_state_row *e = acct_tx_bind_write(a, h, &fresh);
  if (!e) return UNIT;
  if (e->cur_nonce == nonce) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  e->cur_nonce = nonce;
  return UNIT;
}

unit acct_tx_set_code_hash(sail_address a, sail_hash code_hash) {
  uint64_t h[4], value[4];
  int fresh = 0;
  acct_secure_key(a, h);
  sail_hash_to_le_words4(value, code_hash);
  acct_state_row *e = acct_tx_bind_write(a, h, &fresh);
  if (!e) return UNIT;
  if (compare_u64x4(e->cur_chash, value) == 0) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) acct_table_remove(&acct_tx_table, h);
    return UNIT;
  }
  memcpy(e->cur_chash, value, sizeof(e->cur_chash));
  return UNIT;
}

/* ---- debug enumeration over acct_block_table ----------------------------
   The snapshot contains existing cumulative account values for the native
   post-state dump. State-root construction consumes acct_block_probe_row
   directly. */

typedef struct {
  uint64_t hkey[4];
  uint8_t raw_addr[20];
  uint64_t nonce; uint64_t bal[4]; uint64_t sroot[4]; uint64_t chash[4];

} acct_dump_entry;

static void acct_dump_push(acct_dump_entry **rows, uint32_t *n, uint32_t *cap,
                           const uint64_t hkey[4], const uint8_t raw_addr[20],
                           uint64_t nonce, const uint64_t bal[4],
                           const uint64_t sroot[4], const uint64_t chash[4]) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (acct_dump_entry *)realloc(*rows, (size_t)nc * sizeof(acct_dump_entry));
    *cap = nc;
  }
  acct_dump_entry *r = &(*rows)[*n];
  memcpy(r->hkey, hkey, 32); memcpy(r->raw_addr, raw_addr, 20); r->nonce = nonce;
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
                   b->hkey, b->raw_addr, b->cur_nonce, b->cur_bal,
                   b->cur_sroot, b->cur_chash);
  }
  acct_dump_valid = 1;
}

uint64_t acct_dump_count(const unit u) { (void)u; acct_dump_build(); return acct_dump_len; }
sail_word acct_dump_hkey(uint64_t i) {
  acct_dump_build();
  return be_words4_to_sail_word(i < acct_dump_len ? acct_dump_entries[i].hkey : account_zero_val);
}
sail_address acct_dump_address(uint64_t i) {
  acct_dump_build();
  return be_bytes_to_sail_address(i < acct_dump_len
                                      ? acct_dump_entries[i].raw_addr
                                      : (const uint8_t[20]){0});
}
uint64_t acct_dump_nonce(uint64_t i) { acct_dump_build(); return i < acct_dump_len ? acct_dump_entries[i].nonce : 0; }
sail_word acct_dump_balance(uint64_t i) {
  acct_dump_build(); return le_words4_to_sail_word(i < acct_dump_len ? acct_dump_entries[i].bal : account_zero_val);
}
sail_word acct_dump_storage_root(uint64_t i) {
  acct_dump_build(); return le_words4_to_sail_word(i < acct_dump_len ? acct_dump_entries[i].sroot : account_zero_val);
}
sail_word acct_dump_code_hash(uint64_t i) {
  acct_dump_build(); return le_words4_to_sail_word(i < acct_dump_len ? acct_dump_entries[i].chash : account_zero_val);
}

static void acct_dump_invalidate(void) {
  acct_dump_valid = 0;
}

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
  const bal_acc_rec *const *a = x, *const *b = y;
  return memcmp((*a)->raw_addr, (*b)->raw_addr, 20);
}

unit bal_prepare(const unit u) {
  (void)u;
  if (!bal_prepared) {
    qsort(bal_sto.d, bal_sto.n, sizeof(bal_sto_rec), bal_sto_cmp);
    qsort(bal_rds.d, bal_rds.n, sizeof(bal_read_rec), bal_read_cmp);
    qsort(bal_balc.d, bal_balc.n, sizeof(bal_bal_rec), bal_bal_cmp);
    qsort(bal_nonc.d, bal_nonc.n, sizeof(bal_non_rec), bal_non_cmp);
    qsort(bal_codc.d, bal_codc.n, sizeof(bal_cod_rec), bal_cod_cmp);
    if (bal_acc_order_cap < bal_acc.n) {
      const bal_acc_rec **order =
          realloc(bal_acc_order, (size_t)bal_acc.n * sizeof(*order));
      if (!order) return UNIT;
      bal_acc_order = order;
      bal_acc_order_cap = bal_acc.n;
    }
    for (uint32_t i = 0; i < bal_acc.n; i++)
      bal_acc_order[i] = &bal_acc.rows[i];
    qsort(bal_acc_order, bal_acc.n, sizeof(*bal_acc_order), bal_acc_cmp);
    bal_prepared = 1;
  }
  bal_account_cursor = 0;
  bal_active_account = NULL;
  bal_storage_slot_change_cursor = (bal_cursor){0, 0, 0};
  bal_storage_read_cursor = (bal_cursor){0, 0, 0};
  bal_active_storage_changes = (bal_cursor){0, 0, 0};
  bal_balance_change_cursor = (bal_cursor){0, 0, 0};
  bal_nonce_change_cursor = (bal_cursor){0, 0, 0};
  bal_code_change_cursor = (bal_cursor){0, 0, 0};
  return UNIT;
}

static void bal_ensure_prepared(void) {
  if (!bal_prepared) (void)bal_prepare(UNIT);
}

static uint32_t bal_range(const void *base, size_t esz, uint32_t n,
                          const bal_acc_rec *account, uint32_t *begin) {
  if (!account) {
    *begin = 0;
    return 0;
  }
  uint32_t lo = bal_ah_lower(base, esz, n, account->ah);
  uint32_t hi = lo;
  while (hi < n) {
    const uint64_t *ah =
        (const uint64_t *)((const char *)base + (size_t)hi * esz);
    if (compare_u64x4(ah, account->ah) != 0) break;
    hi++;
  }
  *begin = lo;
  return hi - lo;
}

static bal_cursor bal_select_range(const void *base, size_t element_size,
                                   uint32_t count,
                                   const bal_acc_rec *account) {
  uint32_t begin;
  uint32_t length =
      bal_range(base, element_size, count, account, &begin);
  return (bal_cursor){begin, begin + length, begin};
}

uint64_t bal_account_next_probe(sail_address *address) {
  bal_ensure_prepared();
  if (!bal_prepared || bal_account_cursor >= bal_acc.n) return 0;
  bal_active_account = bal_acc_order[bal_account_cursor++];
  bal_storage_slot_change_cursor =
      bal_select_range(bal_sto.d, sizeof(bal_sto_rec), bal_sto.n,
                       bal_active_account);
  bal_storage_read_cursor =
      bal_select_range(bal_rds.d, sizeof(bal_read_rec), bal_rds.n,
                       bal_active_account);
  bal_active_storage_changes = (bal_cursor){0, 0, 0};
  bal_balance_change_cursor =
      bal_select_range(bal_balc.d, sizeof(bal_bal_rec), bal_balc.n,
                       bal_active_account);
  bal_nonce_change_cursor =
      bal_select_range(bal_nonc.d, sizeof(bal_non_rec), bal_nonc.n,
                       bal_active_account);
  bal_code_change_cursor =
      bal_select_range(bal_codc.d, sizeof(bal_cod_rec), bal_codc.n,
                       bal_active_account);
  *address = be_bytes_to_sail_address(bal_active_account->raw_addr);
  return 1;
}

uint64_t bal_storage_slot_next_probe(sail_word *slot, uint64_t *has_change,
                                     uint64_t *index, sail_word *value) {
  const bal_sto_rec *change =
      bal_storage_slot_change_cursor.position <
              bal_storage_slot_change_cursor.end
          ? &((const bal_sto_rec *)bal_sto.d)
                 [bal_storage_slot_change_cursor.position]
          : NULL;
  const bal_read_rec *read =
      bal_storage_read_cursor.position < bal_storage_read_cursor.end
          ? &((const bal_read_rec *)bal_rds.d)
                 [bal_storage_read_cursor.position]
          : NULL;
  if (!change && !read) {
    bal_active_storage_changes = (bal_cursor){0, 0, 0};
    return 0;
  }

  const uint64_t *selected_slot =
      !change ? read->slot
              : (!read || compare_u64x4(change->slot, read->slot) <= 0)
                    ? change->slot
                    : read->slot;

  uint32_t change_begin = bal_storage_slot_change_cursor.position;
  while (bal_storage_slot_change_cursor.position <
             bal_storage_slot_change_cursor.end &&
         compare_u64x4(
             ((const bal_sto_rec *)bal_sto.d)
                 [bal_storage_slot_change_cursor.position]
                     .slot,
             selected_slot) == 0)
    bal_storage_slot_change_cursor.position++;
  bal_active_storage_changes =
      (bal_cursor){change_begin, bal_storage_slot_change_cursor.position,
                   change_begin};

  while (bal_storage_read_cursor.position < bal_storage_read_cursor.end &&
         compare_u64x4(
             ((const bal_read_rec *)bal_rds.d)[bal_storage_read_cursor.position]
                 .slot,
             selected_slot) == 0)
    bal_storage_read_cursor.position++;

  *slot = be_words4_to_sail_word(selected_slot);
  *has_change =
      bal_active_storage_changes.position < bal_active_storage_changes.end;
  if (*has_change) {
    const bal_sto_rec *record =
        &((const bal_sto_rec *)bal_sto.d)
             [bal_active_storage_changes.position++];
    *index = record->idx;
    *value = be_words4_to_sail_word(record->val);
  }
  return 1;
}

uint64_t bal_storage_change_next_probe(uint64_t *index, sail_word *value) {
  if (bal_active_storage_changes.position >= bal_active_storage_changes.end)
    return 0;
  const bal_sto_rec *record =
      &((const bal_sto_rec *)bal_sto.d)[bal_active_storage_changes.position++];
  *index = record->idx;
  *value = be_words4_to_sail_word(record->val);
  return 1;
}

uint64_t bal_balance_change_next_probe(uint64_t *index, sail_word *value) {
  if (bal_balance_change_cursor.position >= bal_balance_change_cursor.end)
    return 0;
  const bal_bal_rec *record =
      &((const bal_bal_rec *)bal_balc.d)[bal_balance_change_cursor.position++];
  *index = record->idx;
  *value = le_words4_to_sail_word(record->val);
  return 1;
}

uint64_t bal_nonce_change_next_probe(uint64_t *index, uint64_t *value) {
  if (bal_nonce_change_cursor.position >= bal_nonce_change_cursor.end) return 0;
  const bal_non_rec *record =
      &((const bal_non_rec *)bal_nonc.d)[bal_nonce_change_cursor.position++];
  *index = record->idx;
  *value = record->val;
  return 1;
}

uint64_t bal_code_change_next_probe(uint64_t *index, sail_hash *code_hash) {
  if (bal_code_change_cursor.position >= bal_code_change_cursor.end) return 0;
  const bal_cod_rec *record =
      &((const bal_cod_rec *)bal_codc.d)[bal_code_change_cursor.position++];
  *index = record->idx;
  *code_hash = le_words4_to_sail_hash(record->chash);
  return 1;
}
