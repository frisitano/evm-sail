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
#include "lbits_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ---- keccak memo: address -> keccak(address), slot -> keccak(slot) ----
 * The account and storage hooks are keyed by raw (address[, slot]); the secure
 * trie keys are computed here and memoized (keccak is a pure function), so
 * repeated touches of the same address or slot never re-hash. Open addressing,
 * FNV-1a over the preimage words, power-of-two capacity. */
#define KMEMO_INIT_CAP 256u

typedef struct {
  uint64_t key[4]; /* preimage as BE words (address zero-extended to 256 bits) */
  uint64_t h[4];   /* keccak(preimage), BE words */
  uint8_t used;
} kmemo_ent;

typedef struct {
  kmemo_ent *tab;
  uint32_t cap;
  uint32_t n;
} kmemo;

static kmemo addr_keccak_memo = {NULL, 0, 0};
static kmemo slot_keccak_memo = {NULL, 0, 0};

static uint64_t kmemo_hash(const uint64_t k[4]) {
  uint64_t h = 0xcbf29ce484222325ull; /* FNV-1a over the 4 key words */
  for (int i = 0; i < 4; i++) {
    h ^= k[i];
    h *= 0x100000001b3ull;
  }
  return h;
}

static uint32_t kmemo_find(const kmemo *m, const uint64_t k[4]) {
  uint32_t i = (uint32_t)kmemo_hash(k) & (m->cap - 1);
  while (m->tab[i].used && memcmp(m->tab[i].key, k, 32) != 0)
    i = (i + 1) & (m->cap - 1);
  return i;
}

/* make room for one more entry; 0 if the memo is unusable (alloc failure) */
static int kmemo_room(kmemo *m) {
  if (!m->tab) {
    m->tab = (kmemo_ent *)calloc(KMEMO_INIT_CAP, sizeof(kmemo_ent));
    if (!m->tab)
      return 0;
    m->cap = KMEMO_INIT_CAP;
    m->n = 0;
  }
  if ((m->n + 1) * 4 >= m->cap * 3) {
    kmemo_ent *ntab = (kmemo_ent *)calloc(m->cap * 2, sizeof(kmemo_ent));
    if (!ntab)
      return (m->n + 1) < m->cap; /* old table still usable while not full */
    kmemo old = *m;
    m->tab = ntab;
    m->cap = old.cap * 2;
    m->n = 0;
    for (uint32_t i = 0; i < old.cap; i++)
      if (old.tab[i].used) {
        uint32_t j = kmemo_find(m, old.tab[i].key);
        m->tab[j] = old.tab[i];
        m->n++;
      }
    free(old.tab);
  }
  return 1;
}

/* out = keccak(low pre_len bytes of v), memoized in m */
static void kmemo_keccak(kmemo *m, size_t pre_len, const lbits v, uint64_t out[4]) {
  uint64_t k[4];
  uint8_t buf[32];
  lbits_to_be_words4(k, v);
  if (!kmemo_room(m)) { /* allocation failed: hash without caching */
    lbits_to_be_bytes(buf, pre_len, v);
    host_keccak256_bytes(out, buf, pre_len);
    return;
  }
  uint32_t i = kmemo_find(m, k);
  if (!m->tab[i].used) {
    lbits_to_be_bytes(buf, pre_len, v);
    memcpy(m->tab[i].key, k, 32);
    host_keccak256_bytes(m->tab[i].h, buf, pre_len);
    m->tab[i].used = 1;
    m->n++;
  }
  memcpy(out, m->tab[i].h, 32);
}

/* h = memoized keccak(address) -- the account secure trie key, BE words */
static void acct_secure_key(const lbits a, uint64_t h[4]) {
  kmemo_keccak(&addr_keccak_memo, 20, a, h);
}

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

/* bal/sroot/chash are 4 little-endian-ordered words each (index 3 = MS) */
static void account_row_set(account_row *e,
                           uint64_t nonce,
                           const uint64_t bal[4],
                           const uint64_t sroot[4],
                           const uint64_t chash[4]) {
  e->nonce = nonce;
  memcpy(e->bal, bal, sizeof(e->bal));
  memcpy(e->sroot, sroot, sizeof(e->sroot));
  memcpy(e->chash, chash, sizeof(e->chash));
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
  account_invalidate_order();
  return UNIT;
}

/* the account row bound to address `a` (updates override cache rows) */
static account_row *acct_row_of(const lbits a) {
  uint64_t h[4];
  acct_secure_key(a, h);
  account_row *u = account_table_get(&account_updates, h);
  return u ? u : account_table_get(&account_cache, h);
}

bool acctmap_present(const lbits a) {
  return acct_row_of(a) ? 1 : 0;
}

static unit acctmap_write(uint8_t update, const lbits a, uint64_t nonce,
                          const lbits bal, const lbits sroot, const lbits chash) {
  uint64_t h[4];
  acct_secure_key(a, h);
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

  uint64_t b[4], sr[4], ch[4];
  lbits_to_le_words4(b, bal);
  lbits_to_le_words4(sr, sroot);
  lbits_to_le_words4(ch, chash);
  account_row_set(e, nonce, b, sr, ch);
  account_invalidate_order();
  return UNIT;
}

unit acctmap_seed(const lbits a, uint64_t nonce,
                  const lbits bal, const lbits sroot, const lbits chash) {
  return acctmap_write(0, a, nonce, bal, sroot, chash);
}

unit acctmap_store(const lbits a, uint64_t nonce,
                   const lbits bal, const lbits sroot, const lbits chash) {
  return acctmap_write(1, a, nonce, bal, sroot, chash);
}

unit acctmap_mark_base_exists(const lbits a) {
  uint64_t h[4];
  acct_secure_key(a, h);
  account_row *e = account_table_get(&account_cache, h);
  if (e) {
    e->base_exists = 1;
    account_row *urow = account_table_get(&account_updates, h);
    if (urow)
      urow->base_exists = 1;
    account_invalidate_order();
  }
  return UNIT;
}

uint64_t acctmap_nonce(const lbits a) {
  account_row *e = acct_row_of(a);
  return e ? e->nonce : 0;
}
static const uint64_t account_zero_val[4] = {0, 0, 0, 0};

void acctmap_bal(lbits *rop, const lbits a) {
  account_row *e = acct_row_of(a);
  le_words4_to_lbits(rop, e ? e->bal : account_zero_val);
}
void acctmap_sroot(lbits *rop, const lbits a) {
  account_row *e = acct_row_of(a);
  le_words4_to_lbits(rop, e ? e->sroot : account_zero_val);
}
void acctmap_chash(lbits *rop, const lbits a) {
  account_row *e = acct_row_of(a);
  le_words4_to_lbits(rop, e ? e->chash : account_zero_val);
}

unit acctmap_remove(const lbits a) {
  uint64_t h[4];
  acct_secure_key(a, h);
  account_table_remove(&account_cache, h);
  account_table_remove(&account_updates, h);
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

void acctmap_at_hkey(lbits *rop, const unit u) {
  (void)u;
  be_words4_to_lbits(rop, account_iter_valid ? account_iter.hkey : account_zero_val);
}
uint64_t acctmap_at_nonce(const unit u) {
  (void)u;
  return account_iter_valid ? account_iter.nonce : 0;
}
void acctmap_at_bal(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, account_iter_valid ? account_iter.bal : account_zero_val);
}
void acctmap_at_sroot(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, account_iter_valid ? account_iter.sroot : account_zero_val);
}
void acctmap_at_chash(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, account_iter_valid ? account_iter.chash : account_zero_val);
}
bool acctmap_at_base_exists(const unit u) {
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

/* the secure storage key of raw (address, slot): (keccak(a), keccak(s)) */
static void storage_secure_key(const lbits a, const lbits s,
                               uint64_t slot[4], uint64_t ah[4], uint64_t sh[4]) {
  lbits_to_be_words4(slot, s);
  kmemo_keccak(&addr_keccak_memo, 20, a, ah);
  kmemo_keccak(&slot_keccak_memo, 32, s, sh);
}

static storage_row storage_make_row(const uint64_t ah[4], const uint64_t sh[4],
                                    const uint64_t slot[4], const uint64_t v[4]) {
  storage_row e;
  memset(&e, 0, sizeof(e));
  memcpy(e.acct_hash, ah, sizeof(e.acct_hash));
  memcpy(e.slot_hash, sh, sizeof(e.slot_hash));
  memcpy(e.slot, slot, sizeof(e.slot));
  memcpy(e.val, v, sizeof(e.val));
  return e;
}

static storage_row *storage_walk_updates(const uint64_t ah[4], const uint64_t sh[4]) {
  for (storage_layer *l = storage_updates; l; l = l->below) {
    storage_row *e = storage_table_get(&l->table, ah, sh);
    if (e)
      return e;
  }
  return NULL;
}

static storage_row *storage_walk(const uint64_t ah[4], const uint64_t sh[4]) {
  storage_row *e = storage_walk_updates(ah, sh);
  return e ? e : storage_table_get(&storage_cache, ah, sh);
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
  storage_iter_clear();
  /* the keccak memos survive resets: keccak is pure, so cached hashes stay
   * valid across transactions/blocks */
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

unit storage_map_seed(const lbits a, const lbits s, const lbits v) {
  uint64_t slot[4], ah[4], sh[4], w[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  storage_row e = storage_make_row(ah, sh, slot, w);
  (void)storage_table_put(&storage_cache, &e);
  storage_iter_clear();
  return UNIT;
}

unit storage_map_store(const lbits a, const lbits s, const lbits v) {
  storage_layer *top = storage_update_top();
  if (top) {
    uint64_t slot[4], ah[4], sh[4], w[4];
    storage_secure_key(a, s, slot, ah, sh);
    lbits_to_be_words4(w, v);
    storage_row e = storage_make_row(ah, sh, slot, w);
    (void)storage_table_put(&top->table, &e);
    storage_iter_clear();
  }
  return UNIT;
}

static const uint64_t storage_zero_val[4] = {0, 0, 0, 0};

/* the 256-bit value at (address, slot) across all layers; 0 if absent */
void storage_map_load(lbits *rop, const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_row *e = storage_walk(ah, sh);
  be_words4_to_lbits(rop, e ? e->val : storage_zero_val);
}

/* the value in the BASE layer only; 0 if absent */
void storage_map_base_load(lbits *rop, const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_layer *base = storage_update_base();
  storage_row *e = base ? storage_table_get(&base->table, ah, sh) : NULL;
  if (!e)
    e = storage_table_get(&storage_cache, ah, sh);
  be_words4_to_lbits(rop, e ? e->val : storage_zero_val);
}

bool storage_map_present(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  return storage_walk(ah, sh) ? 1 : 0;
}

bool storage_map_base_present(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_layer *base = storage_update_base();
  if (base && storage_table_get(&base->table, ah, sh))
    return 1;
  return storage_table_get(&storage_cache, ah, sh) ? 1 : 0;
}

unit storage_map_wipe_addr(const lbits a) {
  uint64_t h[4];
  kmemo_keccak(&addr_keccak_memo, 20, a, h);
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

void storage_map_it_acct_hash(lbits *rop, const unit u) {
  (void)u;
  be_words4_to_lbits(rop, storage_iter_selected < storage_iter_count
                              ? storage_iter_rows[storage_iter_selected].acct_hash
                              : storage_zero_val);
}

void storage_map_it_slot(lbits *rop, const unit u) {
  (void)u;
  be_words4_to_lbits(rop, storage_iter_selected < storage_iter_count
                              ? storage_iter_rows[storage_iter_selected].slot
                              : storage_zero_val);
}

void storage_map_it_val(lbits *rop, const unit u) {
  (void)u;
  be_words4_to_lbits(rop, storage_iter_selected < storage_iter_count
                              ? storage_iter_rows[storage_iter_selected].val
                              : storage_zero_val);
}
