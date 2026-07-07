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

/* enumeration (post-state root): index-argument getters, no cursor. `row`
   reads the secure-key sorted cache/update union; `update_row` reads the
   update table alone. Out-of-range indices read as zeros. */
uint64_t acctmap_count(const unit u) {
  (void)u;
  account_build_order();
  return account_order_count;
}

uint64_t acctmap_update_count(const unit u) {
  (void)u;
  return account_updates.n;
}

static const account_row *acctmap_row_ptr(uint64_t idx) {
  account_build_order();
  return idx < account_order_count ? &account_order[idx] : NULL;
}

static const account_row *acctmap_update_row_ptr(uint64_t idx) {
  return idx < account_updates.n ? &account_updates.rows[idx] : NULL;
}

void acctmap_row_hkey(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_row_ptr(idx);
  be_words4_to_lbits(rop, r ? r->hkey : account_zero_val);
}
uint64_t acctmap_row_nonce(uint64_t idx) {
  const account_row *r = acctmap_row_ptr(idx);
  return r ? r->nonce : 0;
}
void acctmap_row_bal(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_row_ptr(idx);
  le_words4_to_lbits(rop, r ? r->bal : account_zero_val);
}
void acctmap_row_sroot(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_row_ptr(idx);
  le_words4_to_lbits(rop, r ? r->sroot : account_zero_val);
}
void acctmap_row_chash(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_row_ptr(idx);
  le_words4_to_lbits(rop, r ? r->chash : account_zero_val);
}
bool acctmap_row_base_exists(uint64_t idx) {
  const account_row *r = acctmap_row_ptr(idx);
  return r ? r->base_exists : 0;
}

void acctmap_update_row_hkey(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_update_row_ptr(idx);
  be_words4_to_lbits(rop, r ? r->hkey : account_zero_val);
}
uint64_t acctmap_update_row_nonce(uint64_t idx) {
  const account_row *r = acctmap_update_row_ptr(idx);
  return r ? r->nonce : 0;
}
void acctmap_update_row_bal(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_update_row_ptr(idx);
  le_words4_to_lbits(rop, r ? r->bal : account_zero_val);
}
void acctmap_update_row_sroot(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_update_row_ptr(idx);
  le_words4_to_lbits(rop, r ? r->sroot : account_zero_val);
}
void acctmap_update_row_chash(lbits *rop, uint64_t idx) {
  const account_row *r = acctmap_update_row_ptr(idx);
  le_words4_to_lbits(rop, r ? r->chash : account_zero_val);
}
bool acctmap_update_row_base_exists(uint64_t idx) {
  const account_row *r = acctmap_update_row_ptr(idx);
  return r ? r->base_exists : 0;
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

/* the native pre-state base cache (seeded via storage_map_seed; read by
   host/base_native.sail's stateless_storage). The live working set is the
   write-set overlay further below. */
static storage_table storage_cache = {NULL, 0, 0};

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

unit storage_map_reset(const unit u) {
  (void)u;
  storage_table_reset(&storage_cache);
  /* the keccak memos survive resets: keccak is pure, so cached hashes stay
   * valid across transactions/blocks */
  return UNIT;
}

unit storage_map_seed(const lbits a, const lbits s, const lbits v) {
  uint64_t slot[4], ah[4], sh[4], w[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  storage_row e = storage_make_row(ah, sh, slot, w);
  (void)storage_table_put(&storage_cache, &e);
  return UNIT;
}

static const uint64_t storage_zero_val[4] = {0, 0, 0, 0};

/* native base resolver backing (stateless_storage for the native runner): read
 * the seeded pre-state cache DIRECTLY, ignoring the write-set overlay and the
 * old update layers. The write-set model keeps the cache purely as the base
 * k/v store; a miss is a genuine absent slot, which the native resolver treats
 * as 0. */
bool storage_cache_present(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  return storage_table_get(&storage_cache, ah, sh) ? 1 : 0;
}

void storage_cache_load(lbits *rop, const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_row *e = storage_table_get(&storage_cache, ah, sh);
  be_words4_to_lbits(rop, e ? e->val : storage_zero_val);
}

/* wipe an address's rows from the native base cache (EIP-6780 delete; the
   overlay is wiped separately by storage_wset_wipe_addr) */
unit storage_map_wipe_addr(const lbits a) {
  uint64_t h[4];
  kmemo_keccak(&addr_keccak_memo, 20, a, h);
  storage_table_remove_account_hash(&storage_cache, h);
  return UNIT;
}

/* ======================================================================== */
/* WRITE-SET STORAGE OVERLAY (stage 1: replaces the layered storage_map)     */
/*                                                                          */
/* Two flat tables keyed by (keccak(addr), keccak(slot)), each kept sorted   */
/* by (acct_hash, slot_hash):                                                */
/*   storage_wset_tx    -- per-transaction overlay: every touched slot (reads AND     */
/*                writes). Membership == touched. Never removed on revert     */
/*                (the journal rewinds `current`); merged into storage_wset_block and  */
/*                cleared at tx end. `is_warm` (EIP-2929) lives here.         */
/*   storage_wset_block -- block base: writes only = the net-change set vs the         */
/*                authenticated pre-state = the compute_root cursor.          */
/* A row carries (original, current); dirty == written && current!=original.  */
/* The base read-through target below both (native seeded map / witness MPT   */
/* point-get) is resolved in Sail; a miss here means "ask the base".          */
/* ======================================================================== */

typedef struct {
  uint64_t acct_hash[4];
  uint64_t slot_hash[4];
  uint64_t slot[4];
  uint64_t current[4];   /* live value    (valid iff written) */
  uint64_t original[4];  /* tx-start value (tx) / block pre-state (block); valid iff written */
  uint8_t  written;      /* 1: a write reached this row; 0: read-only member */
  uint8_t  is_warm;      /* EIP-2929 warm bit (tx table only) */
} storage_wset_row;

typedef struct { storage_wset_row *rows; uint32_t n, cap; } storage_wset_table;

static storage_wset_table storage_wset_tx    = {NULL, 0, 0};
static storage_wset_table storage_wset_block = {NULL, 0, 0};

/* drop the per-account compute_root snapshots when the block base changes
   (defined with the snapshot builders below) */
static void storage_wset_iter_invalidate(void);

static uint32_t storage_wset_find(const storage_wset_table *t, const uint64_t ah[4],
                         const uint64_t sh[4], int *found) {
  uint32_t lo = 0, hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    int c = storage_key_cmp(t->rows[mid].acct_hash, t->rows[mid].slot_hash, ah, sh);
    if (c < 0) lo = mid + 1; else hi = mid;
  }
  *found = (lo < t->n &&
            storage_key_cmp(t->rows[lo].acct_hash, t->rows[lo].slot_hash, ah, sh) == 0);
  return lo;
}

static storage_wset_row *storage_wset_get(storage_wset_table *t, const uint64_t ah[4], const uint64_t sh[4]) {
  int f = 0;
  uint32_t i = storage_wset_find(t, ah, sh, &f);
  return f ? &t->rows[i] : NULL;
}

/* insert-if-absent, keeping the table sorted; new rows are zeroed with the
   keys set (written == 0 == read-only member) */
static storage_wset_row *storage_wset_intern(storage_wset_table *t, const uint64_t ah[4], const uint64_t sh[4],
                           const uint64_t slot[4]) {
  int f = 0;
  uint32_t i = storage_wset_find(t, ah, sh, &f);
  if (f) return &t->rows[i];
  if (t->cap < t->n + 1) {
    uint32_t nc = t->cap ? t->cap * 2 : STORAGE_INIT_CAP;
    while (nc < t->n + 1) nc *= 2;
    storage_wset_row *nr = (storage_wset_row *)realloc(t->rows, (size_t)nc * sizeof(storage_wset_row));
    if (!nr) return NULL;
    t->rows = nr; t->cap = nc;
  }
  if (i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(storage_wset_row));
  storage_wset_row *e = &t->rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->acct_hash, ah, sizeof(e->acct_hash));
  memcpy(e->slot_hash, sh, sizeof(e->slot_hash));
  memcpy(e->slot, slot, sizeof(e->slot));
  t->n++;
  return e;
}

static void storage_wset_table_reset(storage_wset_table *t) {
  free(t->rows);
  t->rows = NULL; t->n = 0; t->cap = 0;
}

static int storage_wset_dirty(const storage_wset_row *e) {
  return e->written && compare_words(e->current, e->original, 4) != 0;
}

/* --- lifecycle -------------------------------------------------------- */

/* full world wipe (between independent blocks/fixtures) */
unit storage_wset_reset(const unit u) {
  (void)u;
  storage_wset_table_reset(&storage_wset_tx);
  storage_wset_table_reset(&storage_wset_block);
  storage_wset_iter_invalidate();
  return UNIT;
}

/* drop the per-tx overlay (called after BAL extraction + merge at tx end,
   and to discard a fully-reverted tx). Clears membership + is_warm. */
unit storage_wset_tx_clear(const unit u) {
  (void)u;
  storage_wset_table_reset(&storage_wset_tx);
  return UNIT;
}

/* merge dirty tx rows into the block base, then clear the tx overlay. Only
   dirty (real net change) rows reach the base; reads / net-zero writes do not.
   base.original is frozen at the block pre-state (set once, when the key first
   becomes a block write); base.current tracks the latest committed value. */
unit storage_wset_merge(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < storage_wset_tx.n; i++) {
    storage_wset_row *e = &storage_wset_tx.rows[i];
    if (!storage_wset_dirty(e)) continue;
    storage_wset_row *b = storage_wset_intern(&storage_wset_block, e->acct_hash, e->slot_hash, e->slot);
    if (!b) continue;
    if (!b->written) {                 /* first write to reach the base */
      b->written = 1;
      memcpy(b->original, e->original, sizeof(b->original));
    }
    memcpy(b->current, e->current, sizeof(b->current));
  }
  storage_wset_table_reset(&storage_wset_tx);
  storage_wset_iter_invalidate();
  return UNIT;
}

/* --- reads (over tx overlay then block base; a miss => ask the base) --- */

/* present in the working set (tx or block has a WRITE for the slot) */
bool storage_wset_present(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  if (e && e->written) return 1;
  storage_wset_row *b = storage_wset_get(&storage_wset_block, ah, sh);
  return (b && b->written) ? 1 : 0;
}

/* the live value (tx write, else block write, else 0 -- caller resolves the
   base on a miss via storage_wset_present) */
void storage_wset_load(lbits *rop, const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  if (e && e->written) { be_words4_to_lbits(rop, e->current); return; }
  storage_wset_row *b = storage_wset_get(&storage_wset_block, ah, sh);
  be_words4_to_lbits(rop, (b && b->written) ? b->current : storage_zero_val);
}

/* EIP-2200 original: the tx-start value = block base (committed prior txs),
   ignoring the current tx's overlay; a miss => ask the base. */
bool storage_wset_base_present(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *b = storage_wset_get(&storage_wset_block, ah, sh);
  return (b && b->written) ? 1 : 0;
}

void storage_wset_base_load(lbits *rop, const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *b = storage_wset_get(&storage_wset_block, ah, sh);
  be_words4_to_lbits(rop, (b && b->written) ? b->current : storage_zero_val);
}

/* --- writes / touches / warm ------------------------------------------ */

/* record a touch (read) -- ensure tx membership so the slot is in the BAL
   read set; leaves the row read-only (written unchanged) */
unit storage_wset_touch(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  (void)storage_wset_intern(&storage_wset_tx, ah, sh, slot);
  return UNIT;
}

/* the value a frame-revert must restore `current` to (JStor payload): the
   current tx value if already written this tx, else the tx-start `orig`. */
void storage_wset_prior(lbits *rop, const lbits a, const lbits s, const lbits orig) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  if (e && e->written) { be_words4_to_lbits(rop, e->current); return; }
  uint64_t o[4];
  lbits_to_be_words4(o, orig);
  be_words4_to_lbits(rop, o);
}

/* write v; `orig` (the tx-start value) is adopted only on the FIRST write */
unit storage_wset_write(const lbits a, const lbits s, const lbits v, const lbits orig) {
  uint64_t slot[4], ah[4], sh[4], w[4], o[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  lbits_to_be_words4(o, orig);
  storage_wset_row *e = storage_wset_intern(&storage_wset_tx, ah, sh, slot);
  if (!e) return UNIT;
  if (!e->written) { e->written = 1; memcpy(e->original, o, sizeof(e->original)); }
  memcpy(e->current, w, sizeof(e->current));
  return UNIT;
}

/* restore `current` to `prior` (JStor revert). Keeps the row (membership). */
unit storage_wset_restore(const lbits a, const lbits s, const lbits prior) {
  uint64_t slot[4], ah[4], sh[4], p[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(p, prior);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  if (e) memcpy(e->current, p, sizeof(e->current));
  return UNIT;
}

/* EIP-2929 warm: set is_warm on the tx row (interning it); return whether it
   was cold (so the caller journals JWarmS only on the cold->warm transition) */
bool storage_wset_warm(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *e = storage_wset_intern(&storage_wset_tx, ah, sh, slot);
  if (!e) return 0;
  int was_cold = !e->is_warm;
  e->is_warm = 1;
  return was_cold ? 1 : 0;
}

bool storage_wset_is_warm(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  return (e && e->is_warm) ? 1 : 0;
}

/* un-warm on revert (JWarmS undo): only the frame that flipped cold->warm
   pushed the entry, so this fires exactly once per transition */
unit storage_wset_unwarm(const lbits a, const lbits s) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  if (e) e->is_warm = 0;
  return UNIT;
}

/* drop every row for one account_hash, compacting in place (keeps the table
   sorted since the account's rows are a contiguous block) */
static void storage_wset_table_remove_acct(storage_wset_table *t, const uint64_t ah[4]) {
  uint32_t w = 0;
  for (uint32_t i = 0; i < t->n; i++) {
    if (compare_words(t->rows[i].acct_hash, ah, 4) == 0) continue;
    if (w != i) t->rows[w] = t->rows[i];
    w++;
  }
  t->n = w;
}

/* wipe an address's overlay rows (tx + block): EIP-6780 tx-end deletion. */
unit storage_wset_wipe_addr(const lbits a) {
  uint64_t h[4];
  kmemo_keccak(&addr_keccak_memo, 20, a, h);
  storage_wset_table_remove_acct(&storage_wset_tx, h);
  storage_wset_table_remove_acct(&storage_wset_block, h);
  storage_wset_iter_invalidate();
  return UNIT;
}

/* ---- compute_root enumeration over storage_wset_block ------------------
   Both tables are sorted by (acct_hash, slot_hash), so an account's rows form
   a contiguous range found by binary search on acct_hash. Two per-account
   snapshots feed the two backends' storage_updates:
     witness -> DIRTY block rows (current != original): the net-change set over
                the authenticated MPT anchor.
     native  -> UNION(storage_cache, storage_wset_block) with block.current
                overriding the cache: the full post-state over the empty anchor. */

typedef struct { uint64_t slot[4]; uint64_t val[4]; } swb_snap_row;

/* [start,end) block rows with acct_hash == ak */
static void storage_wset_block_acct_range(const uint64_t ak[4], uint32_t *start, uint32_t *end) {
  uint32_t lo = 0, hi = storage_wset_block.n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_words(storage_wset_block.rows[mid].acct_hash, ak, 4) < 0) lo = mid + 1; else hi = mid;
  }
  uint32_t e = lo;
  while (e < storage_wset_block.n && compare_words(storage_wset_block.rows[e].acct_hash, ak, 4) == 0) e++;
  *start = lo; *end = e;
}

/* [start,end) storage_cache rows with acct_hash == ak */
static void storage_cache_acct_range(const uint64_t ak[4], uint32_t *start, uint32_t *end) {
  uint32_t lo = 0, hi = storage_cache.n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_words(storage_cache.rows[mid].acct_hash, ak, 4) < 0) lo = mid + 1; else hi = mid;
  }
  uint32_t e = lo;
  while (e < storage_cache.n && compare_words(storage_cache.rows[e].acct_hash, ak, 4) == 0) e++;
  *start = lo; *end = e;
}

static void swb_snap_push(swb_snap_row **rows, uint32_t *n, uint32_t *cap,
                          const uint64_t slot[4], const uint64_t val[4]) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (swb_snap_row *)realloc(*rows, (size_t)nc * sizeof(swb_snap_row));
    *cap = nc;
  }
  memcpy((*rows)[*n].slot, slot, 32);
  memcpy((*rows)[*n].val, val, 32);
  (*n)++;
}

/* --- witness: dirty block rows --- */
static swb_snap_row *swb_dirty_rows = NULL;
static uint32_t swb_dirty_n = 0, swb_dirty_cap = 0;
static uint64_t swb_dirty_memo[4];
static int swb_dirty_valid = 0;

static void swb_dirty_build(const uint64_t ak[4]) {
  if (swb_dirty_valid && compare_words(swb_dirty_memo, ak, 4) == 0) return;
  swb_dirty_n = 0;
  uint32_t s, e;
  storage_wset_block_acct_range(ak, &s, &e);
  for (uint32_t i = s; i < e; i++) {
    storage_wset_row *r = &storage_wset_block.rows[i];
    if (!storage_wset_dirty(r)) continue;
    swb_snap_push(&swb_dirty_rows, &swb_dirty_n, &swb_dirty_cap, r->slot, r->current);
  }
  memcpy(swb_dirty_memo, ak, sizeof(swb_dirty_memo));
  swb_dirty_valid = 1;
}

uint64_t storage_wset_block_dirty_count(const lbits ak) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); swb_dirty_build(k); return swb_dirty_n;
}
void storage_wset_block_dirty_slot(lbits *rop, const lbits ak, uint64_t j) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); swb_dirty_build(k);
  be_words4_to_lbits(rop, j < swb_dirty_n ? swb_dirty_rows[j].slot : storage_zero_val);
}
void storage_wset_block_dirty_val(lbits *rop, const lbits ak, uint64_t j) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); swb_dirty_build(k);
  be_words4_to_lbits(rop, j < swb_dirty_n ? swb_dirty_rows[j].val : storage_zero_val);
}

/* --- native: cache UNION block (block.current wins on a shared slot) --- */
static swb_snap_row *swb_union_rows = NULL;
static uint32_t swb_union_n = 0, swb_union_cap = 0;
static uint64_t swb_union_memo[4];
static int swb_union_valid = 0;

static void swb_union_build(const uint64_t ak[4]) {
  if (swb_union_valid && compare_words(swb_union_memo, ak, 4) == 0) return;
  swb_union_n = 0;
  uint32_t cs, ce, bs, be;
  storage_cache_acct_range(ak, &cs, &ce);
  storage_wset_block_acct_range(ak, &bs, &be);
  uint32_t ci = cs, bi = bs;
  while (ci < ce || bi < be) {
    if (bi >= be) {
      swb_snap_push(&swb_union_rows, &swb_union_n, &swb_union_cap,
                    storage_cache.rows[ci].slot, storage_cache.rows[ci].val);
      ci++;
    } else if (ci >= ce) {
      swb_snap_push(&swb_union_rows, &swb_union_n, &swb_union_cap,
                    storage_wset_block.rows[bi].slot, storage_wset_block.rows[bi].current);
      bi++;
    } else {
      int c = compare_words(storage_cache.rows[ci].slot_hash, storage_wset_block.rows[bi].slot_hash, 4);
      if (c < 0) {
        swb_snap_push(&swb_union_rows, &swb_union_n, &swb_union_cap,
                      storage_cache.rows[ci].slot, storage_cache.rows[ci].val);
        ci++;
      } else if (c > 0) {
        swb_snap_push(&swb_union_rows, &swb_union_n, &swb_union_cap,
                      storage_wset_block.rows[bi].slot, storage_wset_block.rows[bi].current);
        bi++;
      } else {                 /* same slot: block overrides cache */
        swb_snap_push(&swb_union_rows, &swb_union_n, &swb_union_cap,
                      storage_wset_block.rows[bi].slot, storage_wset_block.rows[bi].current);
        ci++; bi++;
      }
    }
  }
  memcpy(swb_union_memo, ak, sizeof(swb_union_memo));
  swb_union_valid = 1;
}

static void storage_wset_iter_invalidate(void) {
  swb_dirty_valid = 0;
  swb_union_valid = 0;
}

uint64_t storage_wset_union_count(const lbits ak) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); swb_union_build(k); return swb_union_n;
}
void storage_wset_union_slot(lbits *rop, const lbits ak, uint64_t j) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); swb_union_build(k);
  be_words4_to_lbits(rop, j < swb_union_n ? swb_union_rows[j].slot : storage_zero_val);
}
void storage_wset_union_val(lbits *rop, const lbits ak, uint64_t j) {
  uint64_t k[4]; lbits_to_be_words4(k, ak); swb_union_build(k);
  be_words4_to_lbits(rop, j < swb_union_n ? swb_union_rows[j].val : storage_zero_val);
}

/* ======================================================================== */
/* WRITE-SET ACCOUNT OVERLAY (stage 2: two-level account working set)         */
/*                                                                            */
/* Mirrors the storage write-set: a per-tx overlay (acct_wset_tx) over a      */
/* block base (acct_wset_block), keyed by keccak(address), each sorted by     */
/* hkey. A row carries the current AND the tx-start/pre-state original        */
/* account (nonce, balance, storage_root, code_hash) plus base_exists (did    */
/* the authenticated pre-state leaf exist) and written. account_cache below   */
/* stays the RESOLVER backing (native seed + witness read-cache); a miss      */
/* here asks it, then stateless_account_load. storage_root is NOT mutated by  */
/* account writes -- it is the pre-state anchor, and the post-state root is    */
/* derived at compute_root. dirty == written && current != original.          */
/* ======================================================================== */

typedef struct {
  uint64_t hkey[4];
  uint64_t cur_nonce;  uint64_t cur_bal[4];  uint64_t cur_sroot[4];  uint64_t cur_chash[4];
  uint64_t orig_nonce; uint64_t orig_bal[4]; uint64_t orig_sroot[4]; uint64_t orig_chash[4];
  uint8_t base_exists;   /* authenticated pre-state leaf existed */
  uint8_t written;       /* a store reached this row */
} acct_wset_row;

typedef struct { acct_wset_row *rows; uint32_t n, cap; } acct_wset_table;

static acct_wset_table acct_wset_tx    = {NULL, 0, 0};
static acct_wset_table acct_wset_block = {NULL, 0, 0};

/* per-account compute_root snapshots are invalidated when the block base
   changes (defined with the builders below) */
static void acct_wset_iter_invalidate(void);

static uint32_t acct_wset_find(const acct_wset_table *t, const uint64_t h[4], int *found) {
  uint32_t lo = 0, hi = t->n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (compare_u64x4(t->rows[mid].hkey, h) < 0) lo = mid + 1; else hi = mid;
  }
  *found = (lo < t->n && compare_u64x4(t->rows[lo].hkey, h) == 0);
  return lo;
}

static acct_wset_row *acct_wset_get(acct_wset_table *t, const uint64_t h[4]) {
  int f = 0;
  uint32_t i = acct_wset_find(t, h, &f);
  return f ? &t->rows[i] : NULL;
}

static acct_wset_row *acct_wset_intern(acct_wset_table *t, const uint64_t h[4]) {
  int f = 0;
  uint32_t i = acct_wset_find(t, h, &f);
  if (f) return &t->rows[i];
  if (t->cap < t->n + 1) {
    uint32_t nc = t->cap ? t->cap * 2 : ACCOUNT_INIT_CAP;
    while (nc < t->n + 1) nc *= 2;
    acct_wset_row *nr = (acct_wset_row *)realloc(t->rows, (size_t)nc * sizeof(acct_wset_row));
    if (!nr) return NULL;
    t->rows = nr; t->cap = nc;
  }
  if (i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(acct_wset_row));
  acct_wset_row *e = &t->rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->hkey, h, sizeof(e->hkey));
  t->n++;
  return e;
}

static void acct_wset_table_reset(acct_wset_table *t) {
  free(t->rows);
  t->rows = NULL; t->n = 0; t->cap = 0;
}

/* current-vs-original: account writes only touch nonce/bal/chash (sroot is the
   pre-state anchor, base_exists is metadata), so those three decide dirtiness */
static int acct_wset_dirty(const acct_wset_row *e) {
  return e->written &&
         (e->cur_nonce != e->orig_nonce ||
          compare_u64x4(e->cur_bal, e->orig_bal) != 0 ||
          compare_u64x4(e->cur_chash, e->orig_chash) != 0 ||
          compare_u64x4(e->cur_sroot, e->orig_sroot) != 0);
}

/* the live row (tx write over committed block write); NULL on a miss */
static acct_wset_row *acct_wset_live(const uint64_t h[4]) {
  acct_wset_row *e = acct_wset_get(&acct_wset_tx, h);
  if (e && e->written) return e;
  acct_wset_row *b = acct_wset_get(&acct_wset_block, h);
  return (b && b->written) ? b : NULL;
}

/* --- lifecycle --- */
unit acct_wset_reset(const unit u) {
  (void)u;
  acct_wset_table_reset(&acct_wset_tx);
  acct_wset_table_reset(&acct_wset_block);
  acct_wset_iter_invalidate();
  return UNIT;
}

unit acct_wset_tx_clear(const unit u) {
  (void)u;
  acct_wset_table_reset(&acct_wset_tx);
  return UNIT;
}

/* merge dirty tx rows into the block base, then clear the tx overlay.
   base.original + base_exists freeze at the block pre-state (set once). */
unit acct_wset_merge(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < acct_wset_tx.n; i++) {
    acct_wset_row *e = &acct_wset_tx.rows[i];
    if (!acct_wset_dirty(e)) continue;
    acct_wset_row *b = acct_wset_intern(&acct_wset_block, e->hkey);
    if (!b) continue;
    if (!b->written) {
      b->written = 1;
      b->orig_nonce = e->orig_nonce;
      memcpy(b->orig_bal, e->orig_bal, sizeof(b->orig_bal));
      memcpy(b->orig_sroot, e->orig_sroot, sizeof(b->orig_sroot));
      memcpy(b->orig_chash, e->orig_chash, sizeof(b->orig_chash));
      b->base_exists = e->base_exists;
    }
    b->cur_nonce = e->cur_nonce;
    memcpy(b->cur_bal, e->cur_bal, sizeof(b->cur_bal));
    memcpy(b->cur_sroot, e->cur_sroot, sizeof(b->cur_sroot));
    memcpy(b->cur_chash, e->cur_chash, sizeof(b->cur_chash));
  }
  acct_wset_table_reset(&acct_wset_tx);
  acct_wset_iter_invalidate();
  return UNIT;
}

/* --- reads (overlay only; a miss => Sail asks account_cache / resolver) --- */
bool acct_wset_present(const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  return acct_wset_live(h) ? 1 : 0;
}
uint64_t acct_wset_nonce(const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_live(h);
  return e ? e->cur_nonce : 0;
}
void acct_wset_bal(lbits *rop, const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_live(h);
  le_words4_to_lbits(rop, e ? e->cur_bal : account_zero_val);
}
void acct_wset_sroot(lbits *rop, const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_live(h);
  le_words4_to_lbits(rop, e ? e->cur_sroot : account_zero_val);
}
void acct_wset_chash(lbits *rop, const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_live(h);
  le_words4_to_lbits(rop, e ? e->cur_chash : account_zero_val);
}

/* --- writes / restore / wipe --- */
/* write the current account into the tx overlay; on the FIRST write this tx
   the original + base_exists freeze at the tx-start account, read from the
   block base if committed there, else the resolver cache (load_account always
   runs before a store, so the cache holds the resolved pre-state). */
unit acct_wset_write(const lbits a, uint64_t nonce,
                     const lbits bal, const lbits sroot, const lbits chash) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_intern(&acct_wset_tx, h);
  if (!e) return UNIT;
  if (!e->written) {
    e->written = 1;
    acct_wset_row *b = acct_wset_get(&acct_wset_block, h);
    if (b && b->written) {
      e->orig_nonce = b->cur_nonce;
      memcpy(e->orig_bal, b->cur_bal, sizeof(e->orig_bal));
      memcpy(e->orig_sroot, b->cur_sroot, sizeof(e->orig_sroot));
      memcpy(e->orig_chash, b->cur_chash, sizeof(e->orig_chash));
      e->base_exists = b->base_exists;
    } else {
      const account_row *c = account_table_const_get(&account_cache, h);
      if (c) {
        e->orig_nonce = c->nonce;
        memcpy(e->orig_bal, c->bal, sizeof(e->orig_bal));
        memcpy(e->orig_sroot, c->sroot, sizeof(e->orig_sroot));
        memcpy(e->orig_chash, c->chash, sizeof(e->orig_chash));
        e->base_exists = c->base_exists;
      }
      /* else absent pre-state: original stays zeroed, base_exists 0 */
    }
  }
  uint64_t b4[4], sr[4], ch[4];
  lbits_to_le_words4(b4, bal);
  lbits_to_le_words4(sr, sroot);
  lbits_to_le_words4(ch, chash);
  e->cur_nonce = nonce;
  memcpy(e->cur_bal, b4, sizeof(e->cur_bal));
  memcpy(e->cur_sroot, sr, sizeof(e->cur_sroot));
  memcpy(e->cur_chash, ch, sizeof(e->cur_chash));
  return UNIT;
}

/* JAcct undo: restore the tx-overlay current account to a prior value; keeps
   the row (membership) and its frozen original. */
unit acct_wset_restore(const lbits a, uint64_t nonce,
                       const lbits bal, const lbits sroot, const lbits chash) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_get(&acct_wset_tx, h);
  if (!e) return UNIT;
  uint64_t b4[4], sr[4], ch[4];
  lbits_to_le_words4(b4, bal);
  lbits_to_le_words4(sr, sroot);
  lbits_to_le_words4(ch, chash);
  e->cur_nonce = nonce;
  memcpy(e->cur_bal, b4, sizeof(e->cur_bal));
  memcpy(e->cur_sroot, sr, sizeof(e->cur_sroot));
  memcpy(e->cur_chash, ch, sizeof(e->cur_chash));
  return UNIT;
}

static void acct_wset_table_remove_hkey(acct_wset_table *t, const uint64_t h[4]) {
  int f = 0;
  uint32_t i = acct_wset_find(t, h, &f);
  if (!f) return;
  if (i + 1 < t->n)
    memmove(&t->rows[i], &t->rows[i + 1], (size_t)(t->n - i - 1) * sizeof(acct_wset_row));
  t->n--;
}

/* EIP-6780 tx-end delete: drop the address's overlay rows (tx + block) */
unit acct_wset_wipe_addr(const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_table_remove_hkey(&acct_wset_tx, h);
  acct_wset_table_remove_hkey(&acct_wset_block, h);
  acct_wset_iter_invalidate();
  return UNIT;
}

/* --- base (block-only) reads: the tx-start account (committed prior txs) --- */
bool acct_wset_base_present(const lbits a) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *b = acct_wset_get(&acct_wset_block, h);
  return (b && b->written) ? 1 : 0;
}

static void acct_wset_iter_invalidate(void) { }
