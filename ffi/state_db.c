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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ACCOUNT_INIT_CAP 64u

typedef struct {
  uint64_t hkey[4];  /* keccak(address) h3,h2,h1,h0 (h3 most significant) */
  uint64_t nonce;    /* account nonce (< 2^64) */
  uint64_t bal[4];   /* balance v3..v0 (index 3 = most significant) */
  uint64_t sroot[4]; /* storage root */
  uint64_t chash[4]; /* code hash */
  uint8_t base_exists; /* authenticated pre-state leaf existed */
} account_row;

typedef struct {
  account_row *rows;
  uint32_t n;
  uint32_t cap;
} account_table;

static account_table account_cache = {NULL, 0, 0};
static account_table account_updates = {NULL, 0, 0};

/* sorted cache/update union for acctmap_at(i) */
static account_row *account_order = NULL;
static uint32_t account_order_count = 0;
static int account_order_valid = 0;

/* cached key selection (acctmap_key) */
static uint64_t selected_account_hash[4];
static int selected_account_valid = 0;

/* cached iteration row (acctmap_at / acctmap_update_at) */
static account_row account_iter;
static int account_iter_valid = 0;

static int compare_u64x4(const uint64_t *a, const uint64_t *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static int account_hash_cmp(const uint64_t *ah, const uint64_t *bh) {
  return compare_u64x4(ah, bh);
}

static int account_row_hash_cmp(const account_row *r, const uint64_t *h) {
  return account_hash_cmp(r->hkey, h);
}

static int account_row_cmp(const account_row *a, const account_row *b) {
  return account_hash_cmp(a->hkey, b->hkey);
}

static void account_invalidate_order(void) {
  free(account_order);
  account_order = NULL;
  account_order_count = 0;
  account_order_valid = 0;
  account_iter_valid = 0;
}

static void account_table_reset(account_table *t) {
  free(t->rows);
  t->rows = NULL;
  t->n = 0;
  t->cap = 0;
}

static int account_table_reserve(account_table *t, uint32_t need) {
  if (t->cap >= need)
    return 1;
  uint32_t nc = t->cap ? t->cap * 2 : ACCOUNT_INIT_CAP;
  while (nc < need)
    nc *= 2;
  account_row *nr = (account_row *)realloc(t->rows, (size_t)nc * sizeof(account_row));
  if (!nr)
    return 0;
  t->rows = nr;
  t->cap = nc;
  return 1;
}

static uint32_t account_table_find(const account_table *t, const uint64_t *h, int *found) {
  uint32_t lo = 0;
  uint32_t hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    int c = account_row_hash_cmp(&t->rows[mid], h);
    if (c < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  *found = (lo < t->n && account_row_hash_cmp(&t->rows[lo], h) == 0);
  return lo;
}

static account_row *account_table_get(account_table *t, const uint64_t *h) {
  int found = 0;
  uint32_t i = account_table_find(t, h, &found);
  return found ? &t->rows[i] : NULL;
}

static const account_row *account_table_const_get(const account_table *t, const uint64_t *h) {
  int found = 0;
  uint32_t i = account_table_find(t, h, &found);
  return found ? &t->rows[i] : NULL;
}

static account_row *account_table_put(account_table *t, const uint64_t *h) {
  int found = 0;
  uint32_t i = account_table_find(t, h, &found);
  if (found)
    return &t->rows[i];
  if (!account_table_reserve(t, t->n + 1))
    return NULL;
  if (i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(account_row));
  memset(&t->rows[i], 0, sizeof(account_row));
  memcpy(t->rows[i].hkey, h, sizeof(t->rows[i].hkey));
  t->n++;
  return &t->rows[i];
}

static void account_table_remove(account_table *t, const uint64_t *h) {
  int found = 0;
  uint32_t i = account_table_find(t, h, &found);
  if (!found)
    return;
  if (i + 1 < t->n)
    memmove(&t->rows[i], &t->rows[i + 1], (size_t)(t->n - i - 1) * sizeof(account_row));
  t->n--;
}

static void account_row_set(account_row *e,
                           uint64_t nonce,
                           uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                           uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                           uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0) {
  e->nonce = nonce;
  e->bal[3] = b3; e->bal[2] = b2; e->bal[1] = b1; e->bal[0] = b0;
  e->sroot[3] = sr3; e->sroot[2] = sr2; e->sroot[1] = sr1; e->sroot[0] = sr0;
  e->chash[3] = ch3; e->chash[2] = ch2; e->chash[1] = ch1; e->chash[0] = ch0;
}

static void account_build_order(void) {
  if (account_order_valid)
    return;
  free(account_order);
  uint32_t max_n = account_cache.n + account_updates.n;
  account_order = (account_row *)calloc(max_n ? max_n : 1, sizeof(account_row));
  account_order_count = 0;
  if (!account_order) {
    account_order_valid = 1;
    return;
  }

  uint32_t i = 0;
  uint32_t j = 0;
  while (i < account_cache.n || j < account_updates.n) {
    if (i == account_cache.n) {
      account_order[account_order_count++] = account_updates.rows[j++];
    } else if (j == account_updates.n) {
      account_order[account_order_count++] = account_cache.rows[i++];
    } else {
      int c = account_row_cmp(&account_cache.rows[i], &account_updates.rows[j]);
      if (c < 0) {
        account_order[account_order_count++] = account_cache.rows[i++];
      } else if (c > 0) {
        account_order[account_order_count++] = account_updates.rows[j++];
      } else {
        account_order[account_order_count++] = account_updates.rows[j++];
        i++;
      }
    }
  }
  account_order_valid = 1;
}

unit acctmap_reset(const unit u) {
  (void)u;
  account_table_reset(&account_cache);
  account_table_reset(&account_updates);
  selected_account_valid = 0;
  account_invalidate_order();
  return UNIT;
}

unit acctmap_key(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  selected_account_hash[0] = h3;
  selected_account_hash[1] = h2;
  selected_account_hash[2] = h1;
  selected_account_hash[3] = h0;
  selected_account_valid = 1;
  return UNIT;
}

/* current account row: execution-visible updates override cache rows */
static account_row *selected_account_row(void) {
  if (!selected_account_valid)
    return NULL;
  account_row *u = account_table_get(&account_updates, selected_account_hash);
  return u ? u : account_table_get(&account_cache, selected_account_hash);
}

uint64_t acctmap_present(const unit u) {
  (void)u;
  return selected_account_row() ? 1 : 0;
}

static unit acctmap_write(uint8_t update,
                          uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                          uint64_t nonce,
                          uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                          uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                          uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0) {
  if (!selected_account_valid)
    return UNIT;

  uint64_t h[4] = {h3, h2, h1, h0};
  account_table *target = update ? &account_updates : &account_cache;
  account_row *e = account_table_put(target, h);
  if (!e)
    return UNIT;

  if (update) {
    const account_row *base = account_table_const_get(&account_cache, h);
    if (!e->base_exists && base)
      e->base_exists = base->base_exists;
  } else {
    e->base_exists = 0;
  }

  account_row_set(e, nonce, b3, b2, b1, b0, sr3, sr2, sr1, sr0, ch3, ch2, ch1, ch0);
  account_invalidate_order();
  return UNIT;
}

unit acctmap_seed(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                  uint64_t nonce,
                  uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                  uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                  uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0) {
  return acctmap_write(0, h3, h2, h1, h0, nonce, b3, b2, b1, b0,
                       sr3, sr2, sr1, sr0, ch3, ch2, ch1, ch0);
}

unit acctmap_store(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                   uint64_t nonce,
                   uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                   uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                   uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0) {
  return acctmap_write(1, h3, h2, h1, h0, nonce, b3, b2, b1, b0,
                       sr3, sr2, sr1, sr0, ch3, ch2, ch1, ch0);
}

unit acctmap_mark_base_exists(const unit u) {
  (void)u;
  if (!selected_account_valid)
    return UNIT;
  account_row *e = account_table_get(&account_cache, selected_account_hash);
  if (e) {
    e->base_exists = 1;
    account_row *urow = account_table_get(&account_updates, selected_account_hash);
    if (urow)
      urow->base_exists = 1;
    account_invalidate_order();
  }
  return UNIT;
}

uint64_t acctmap_nonce(const unit u) {
  (void)u;
  account_row *e = selected_account_row();
  return e ? e->nonce : 0;
}
uint64_t acctmap_bal(uint64_t w) {
  account_row *e = selected_account_row();
  return (e && w <= 3) ? e->bal[w] : 0;
}
uint64_t acctmap_sroot(uint64_t w) {
  account_row *e = selected_account_row();
  return (e && w <= 3) ? e->sroot[w] : 0;
}
uint64_t acctmap_chash(uint64_t w) {
  account_row *e = selected_account_row();
  return (e && w <= 3) ? e->chash[w] : 0;
}

unit acctmap_remove(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  uint64_t h[4] = {h3, h2, h1, h0};
  account_table_remove(&account_cache, h);
  account_table_remove(&account_updates, h);
  selected_account_valid = 0;
  account_invalidate_order();
  return UNIT;
}

/* iteration (post-state root): acctmap_at(i) caches secure-key sorted row i */
uint64_t acctmap_count(const unit u) {
  (void)u;
  account_build_order();
  return account_order_count;
}

uint64_t acctmap_update_count(const unit u) {
  (void)u;
  return account_updates.n;
}

unit acctmap_at(uint64_t idx) {
  account_iter_valid = 0;
  account_build_order();
  if (idx < account_order_count) {
    account_iter = account_order[idx];
    account_iter_valid = 1;
  }
  return UNIT;
}

unit acctmap_update_at(uint64_t idx) {
  account_iter_valid = 0;
  if (idx < account_updates.n) {
    account_iter = account_updates.rows[idx];
    account_iter_valid = 1;
  }
  return UNIT;
}

uint64_t acctmap_at_hkey(uint64_t w) { return (account_iter_valid && w <= 3) ? account_iter.hkey[w] : 0; }
uint64_t acctmap_at_nonce(const unit u) {
  (void)u;
  return account_iter_valid ? account_iter.nonce : 0;
}
uint64_t acctmap_at_bal(uint64_t w) { return (account_iter_valid && w <= 3) ? account_iter.bal[w] : 0; }
uint64_t acctmap_at_sroot(uint64_t w) { return (account_iter_valid && w <= 3) ? account_iter.sroot[w] : 0; }
uint64_t acctmap_at_chash(uint64_t w) { return (account_iter_valid && w <= 3) ? account_iter.chash[w] : 0; }
uint64_t acctmap_at_base_exists(const unit u) {
  (void)u;
  return account_iter_valid ? account_iter.base_exists : 0;
}

#define STORAGE_INIT_CAP 64u

typedef struct {
  uint64_t acct_hash[4];          /* keccak(address), h3..h0 */
  uint64_t slot_hash[4];          /* keccak(slot), h3..h0 */
  uint64_t slot[4];               /* raw slot s3..s0 */
  uint64_t val[4];                /* value v3..v0 */
} storage_row;

typedef struct {
  storage_row *rows;
  uint32_t n;
  uint32_t cap;
} storage_table;

typedef struct storage_layer {
  storage_table table;
  struct storage_layer *below;
} storage_layer;

static storage_table storage_cache = {NULL, 0, 0};
static storage_layer *storage_updates = NULL;

static uint64_t selected_storage_slot[4];
static uint64_t selected_storage_account_hash[4];
static uint64_t selected_storage_slot_hash[4];
static int selected_storage_valid = 0;

static storage_row *storage_iter_rows = NULL;
static uint32_t storage_iter_count = 0;
static uint32_t storage_iter_selected = 0;
static int storage_iter_updates_only = -1;

static int compare_words(const uint64_t *a, const uint64_t *b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static int storage_key_cmp(const uint64_t *aa, const uint64_t *as,
                     const uint64_t *ba, const uint64_t *bs) {
  int c = compare_words(aa, ba, 4);
  if (c) return c;
  return compare_words(as, bs, 4);
}

static int storage_row_key_cmp(const storage_row *e,
                           const uint64_t *acct_hash,
                           const uint64_t *slot_hash) {
  return storage_key_cmp(e->acct_hash, e->slot_hash, acct_hash, slot_hash);
}

static void storage_iter_clear(void) {
  free(storage_iter_rows);
  storage_iter_rows = NULL;
  storage_iter_count = 0;
  storage_iter_selected = 0;
  storage_iter_updates_only = -1;
}

static void storage_table_reset(storage_table *t) {
  free(t->rows);
  t->rows = NULL;
  t->n = 0;
  t->cap = 0;
}

static int storage_table_reserve(storage_table *t, uint32_t need) {
  if (t->cap >= need)
    return 1;
  uint32_t nc = t->cap ? t->cap * 2 : STORAGE_INIT_CAP;
  while (nc < need)
    nc *= 2;
  storage_row *nr = (storage_row *)realloc(t->rows, (size_t)nc * sizeof(storage_row));
  if (!nr)
    return 0;
  t->rows = nr;
  t->cap = nc;
  return 1;
}

static uint32_t storage_table_find(const storage_table *t,
                             const uint64_t *acct_hash,
                             const uint64_t *slot_hash,
                             int *found) {
  uint32_t lo = 0;
  uint32_t hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    int c = storage_row_key_cmp(&t->rows[mid], acct_hash, slot_hash);
    if (c < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  *found = (lo < t->n && storage_row_key_cmp(&t->rows[lo], acct_hash, slot_hash) == 0);
  return lo;
}

static storage_row *storage_table_get(storage_table *t,
                            const uint64_t *acct_hash,
                            const uint64_t *slot_hash) {
  int found = 0;
  uint32_t i = storage_table_find(t, acct_hash, slot_hash, &found);
  return found ? &t->rows[i] : NULL;
}

static storage_row *storage_table_put(storage_table *t, const storage_row *src) {
  int found = 0;
  uint32_t i = storage_table_find(t, src->acct_hash, src->slot_hash, &found);
  if (found) {
    t->rows[i] = *src;
    return &t->rows[i];
  }
  if (!storage_table_reserve(t, t->n + 1))
    return NULL;
  if (i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(storage_row));
  t->rows[i] = *src;
  t->n++;
  return &t->rows[i];
}

static void storage_table_remove_account_hash(storage_table *t, const uint64_t *acct_hash) {
  uint32_t w = 0;
  for (uint32_t r = 0; r < t->n; r++) {
    if (compare_words(t->rows[r].acct_hash, acct_hash, 4) != 0)
      t->rows[w++] = t->rows[r];
  }
  t->n = w;
}

static storage_layer *storage_layer_new(storage_layer *below) {
  storage_layer *l = (storage_layer *)calloc(1, sizeof(storage_layer));
  if (!l)
    return NULL;
  l->below = below;
  return l;
}

static storage_layer *storage_update_top(void) {
  if (!storage_updates)
    storage_updates = storage_layer_new(NULL);
  return storage_updates;
}

static storage_layer *storage_update_base(void) {
  storage_layer *l = storage_update_top();
  if (!l)
    return NULL;
  while (l->below)
    l = l->below;
  return l;
}

static void storage_free_layers(storage_layer *l) {
  while (l) {
    storage_layer *b = l->below;
    storage_table_reset(&l->table);
    free(l);
    l = b;
  }
}

static storage_row selected_storage_row(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) {
  storage_row e;
  memset(&e, 0, sizeof(e));
  memcpy(e.acct_hash, selected_storage_account_hash, sizeof(e.acct_hash));
  memcpy(e.slot_hash, selected_storage_slot_hash, sizeof(e.slot_hash));
  memcpy(e.slot, selected_storage_slot, sizeof(e.slot));
  e.val[0] = v3;
  e.val[1] = v2;
  e.val[2] = v1;
  e.val[3] = v0;
  return e;
}

static storage_row *storage_walk_updates(void) {
  if (!selected_storage_valid)
    return NULL;
  for (storage_layer *l = storage_updates; l; l = l->below) {
    storage_row *e = storage_table_get(&l->table, selected_storage_account_hash, selected_storage_slot_hash);
    if (e)
      return e;
  }
  return NULL;
}

static storage_row *storage_walk(void) {
  storage_row *e = storage_walk_updates();
  return e ? e : storage_table_get(&storage_cache, selected_storage_account_hash, selected_storage_slot_hash);
}

static void storage_overlay_table(storage_table *dst, const storage_table *src) {
  for (uint32_t i = 0; i < src->n; i++)
    (void)storage_table_put(dst, &src->rows[i]);
}

static uint64_t storage_build_iter(int updates_only) {
  storage_iter_clear();
  storage_table scratch = {NULL, 0, 0};
  if (!updates_only)
    storage_overlay_table(&scratch, &storage_cache);

  storage_layer *stack[64];
  int depth = 0;
  for (storage_layer *l = storage_updates; l && depth < 64; l = l->below)
    stack[depth++] = l;
  for (int d = depth - 1; d >= 0; d--)
    storage_overlay_table(&scratch, &stack[d]->table);

  if (scratch.n) {
    storage_iter_rows = (storage_row *)calloc(scratch.n, sizeof(storage_row));
    if (storage_iter_rows) {
      memcpy(storage_iter_rows, scratch.rows, (size_t)scratch.n * sizeof(storage_row));
      storage_iter_count = scratch.n;
      storage_iter_updates_only = updates_only ? 1 : 0;
    }
  }
  storage_table_reset(&scratch);
  return storage_iter_count;
}

unit storage_map_reset(const unit u) {
  (void)u;
  storage_table_reset(&storage_cache);
  storage_free_layers(storage_updates);
  storage_updates = NULL;
  selected_storage_valid = 0;
  storage_iter_clear();
  return UNIT;
}

unit storage_map_push(const unit u) {
  (void)u;
  storage_layer *top = storage_update_top();
  storage_layer *n = storage_layer_new(top);
  if (n)
    storage_updates = n;
  return UNIT;
}

unit storage_map_pop_commit(const unit u) {
  (void)u;
  if (storage_updates && storage_updates->below) {
    storage_layer *top = storage_updates;
    storage_layer *below = top->below;
    storage_overlay_table(&below->table, &top->table);
    storage_updates = below;
    storage_table_reset(&top->table);
    free(top);
  }
  storage_iter_clear();
  return UNIT;
}

unit storage_map_pop_discard(const unit u) {
  (void)u;
  if (storage_updates && storage_updates->below) {
    storage_layer *top = storage_updates;
    storage_updates = top->below;
    storage_table_reset(&top->table);
    free(top);
  }
  storage_iter_clear();
  return UNIT;
}

unit storage_map_key(uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0,
                     uint64_t ah3, uint64_t ah2, uint64_t ah1, uint64_t ah0,
                     uint64_t sh3, uint64_t sh2, uint64_t sh1, uint64_t sh0) {
  selected_storage_slot[0] = s3; selected_storage_slot[1] = s2; selected_storage_slot[2] = s1; selected_storage_slot[3] = s0;
  selected_storage_account_hash[0] = ah3; selected_storage_account_hash[1] = ah2; selected_storage_account_hash[2] = ah1; selected_storage_account_hash[3] = ah0;
  selected_storage_slot_hash[0] = sh3; selected_storage_slot_hash[1] = sh2; selected_storage_slot_hash[2] = sh1; selected_storage_slot_hash[3] = sh0;
  selected_storage_valid = 1;
  return UNIT;
}

unit storage_map_seed(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) {
  if (selected_storage_valid) {
    storage_row e = selected_storage_row(v3, v2, v1, v0);
    (void)storage_table_put(&storage_cache, &e);
    storage_iter_clear();
  }
  return UNIT;
}

unit storage_map_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) {
  storage_layer *top = storage_update_top();
  if (selected_storage_valid && top) {
    storage_row e = selected_storage_row(v3, v2, v1, v0);
    (void)storage_table_put(&top->table, &e);
    storage_iter_clear();
  }
  return UNIT;
}

uint64_t storage_map_word(uint64_t i) {
  if (i > 3)
    return 0;
  storage_row *e = storage_walk();
  return e ? e->val[3 - i] : 0;
}

uint64_t storage_map_base_word(uint64_t i) {
  if (i > 3)
    return 0;
  storage_layer *base = storage_update_base();
  storage_row *e = base ? storage_table_get(&base->table, selected_storage_account_hash, selected_storage_slot_hash) : NULL;
  if (!e)
    e = storage_table_get(&storage_cache, selected_storage_account_hash, selected_storage_slot_hash);
  return e ? e->val[3 - i] : 0;
}

uint64_t storage_map_present(const unit u) {
  (void)u;
  return storage_walk() ? 1 : 0;
}

uint64_t storage_map_base_present(const unit u) {
  (void)u;
  storage_layer *base = storage_update_base();
  if (base && storage_table_get(&base->table, selected_storage_account_hash, selected_storage_slot_hash))
    return 1;
  return storage_table_get(&storage_cache, selected_storage_account_hash, selected_storage_slot_hash) ? 1 : 0;
}

uint64_t storage_map_has_acct_hash(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  uint64_t h[4] = {h3, h2, h1, h0};
  for (uint32_t i = 0; i < storage_cache.n; i++)
    if (compare_words(storage_cache.rows[i].acct_hash, h, 4) == 0)
      return 1;
  for (storage_layer *l = storage_updates; l; l = l->below)
    for (uint32_t i = 0; i < l->table.n; i++)
      if (compare_words(l->table.rows[i].acct_hash, h, 4) == 0)
        return 1;
  return 0;
}

unit storage_map_wipe_acct_hash(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  uint64_t h[4] = {h3, h2, h1, h0};
  storage_table_remove_account_hash(&storage_cache, h);
  for (storage_layer *l = storage_updates; l; l = l->below)
    storage_table_remove_account_hash(&l->table, h);
  storage_iter_clear();
  return UNIT;
}

uint64_t storage_map_count(const unit u) {
  (void)u;
  return storage_build_iter(0);
}

uint64_t storage_map_update_count(const unit u) {
  (void)u;
  return storage_build_iter(1);
}

unit storage_map_at(uint64_t j) {
  if (!storage_iter_rows || storage_iter_updates_only != 0 || j >= storage_iter_count)
    (void)storage_build_iter(0);
  storage_iter_selected = (uint32_t)j;
  return UNIT;
}

unit storage_map_update_at(uint64_t j) {
  if (!storage_iter_rows || storage_iter_updates_only != 1 || j >= storage_iter_count)
    (void)storage_build_iter(1);
  storage_iter_selected = (uint32_t)j;
  return UNIT;
}

uint64_t storage_map_it_acct_hash(uint64_t i) {
  if (storage_iter_selected >= storage_iter_count || i > 3)
    return 0;
  return storage_iter_rows[storage_iter_selected].acct_hash[3 - i];
}

uint64_t storage_map_it_slot(uint64_t i) {
  if (storage_iter_selected >= storage_iter_count || i > 3)
    return 0;
  return storage_iter_rows[storage_iter_selected].slot[3 - i];
}

uint64_t storage_map_it_val(uint64_t i) {
  if (storage_iter_selected >= storage_iter_count || i > 3)
    return 0;
  return storage_iter_rows[storage_iter_selected].val[3 - i];
}
