/* C-backed raw-address -> account overlay map for the evm-sail kernel.
 *
 * Execution wants raw-address lookup: BALANCE, nonce bumps, transfers, and code
 * lookup should not hash just to find the mutable account row. State-root
 * commitment wants secure-trie order: keccak256(address), lexicographic. This
 * file therefore keeps an open-addressing raw lookup table and exposes
 * acctmap_at(i) through a lazily rebuilt snapshot sorted by the secure key that
 * Sail passes on each store. Only uint64 crosses the FFI. */
#include "host_acctmap.h"
#include <stdlib.h>
#include <string.h>

#define AM_INIT_CAP 1024u /* power of two */

typedef struct {
  uint64_t key[3];   /* address a2,a1,a0 (BE words; a2 holds the top 32 bits) */
  uint64_t hkey[4];  /* keccak(address) h3,h2,h1,h0 (h3 most significant) */
  uint64_t nonce;    /* account nonce (< 2^64) */
  uint64_t bal[4];   /* balance v3..v0 (index 3 = most significant) */
  uint64_t sroot[4]; /* storage root */
  uint64_t chash[4]; /* code hash */
  uint8_t dirty;      /* post-state update: execution mutated this account */
  uint8_t existed;    /* authenticated pre-state leaf existed */
  uint8_t used;
} am_entry;

static am_entry *am_tab = NULL;
static uint32_t am_cap = 0;
static uint32_t am_n = 0;

/* sorted snapshot for acctmap_at(i), ordered by secure trie key */
static am_entry *am_ord = NULL;
static uint32_t am_ord_n = 0;
static int am_ord_valid = 0;
static am_entry *am_dirty_ord = NULL;
static uint32_t am_dirty_ord_n = 0;
static int am_dirty_ord_valid = 0;

/* cached key selection (acctmap_key) */
static uint64_t am_cur_key[3];
static uint64_t am_cur_hash;
static int am_cur_valid = 0;

/* cached iteration row (acctmap_at) */
static am_entry am_it;
static int am_it_ok = 0;

static uint64_t am_hash(const uint64_t *k) {
  uint64_t h = 0xcbf29ce484222325ull; /* FNV-1a over the 3 raw address words */
  for (int i = 0; i < 3; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) {
      h ^= (w >> (8 * b)) & 0xff;
      h *= 0x100000001b3ull;
    }
  }
  return h;
}

static int am_word4_cmp(const uint64_t *a, const uint64_t *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static int am_word3_cmp(const uint64_t *a, const uint64_t *b) {
  for (int i = 0; i < 3; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static int am_entry_cmp_secure(const void *pa, const void *pb) {
  const am_entry *a = (const am_entry *)pa;
  const am_entry *b = (const am_entry *)pb;
  int c = am_word4_cmp(a->hkey, b->hkey);
  return c ? c : am_word3_cmp(a->key, b->key);
}

static void am_invalidate_order(void) {
  free(am_ord);
  am_ord = NULL;
  am_ord_n = 0;
  am_ord_valid = 0;
  free(am_dirty_ord);
  am_dirty_ord = NULL;
  am_dirty_ord_n = 0;
  am_dirty_ord_valid = 0;
  am_it_ok = 0;
}

static void am_grow(void);

/* slot for key in the table (matching entry or first free) */
static uint32_t am_find(const uint64_t *k, uint64_t h) {
  uint32_t m = am_cap - 1;
  uint32_t i = (uint32_t)h & m;
  while (am_tab[i].used && memcmp(am_tab[i].key, k, 24) != 0)
    i = (i + 1) & m;
  return i;
}

static am_entry *am_put(const uint64_t *k, uint64_t h) {
  if ((am_n + 1) * 4 >= am_cap * 3)
    am_grow();
  uint32_t i = am_find(k, h);
  if (!am_tab[i].used) {
    memset(&am_tab[i], 0, sizeof(am_entry));
    memcpy(am_tab[i].key, k, 24);
    am_tab[i].used = 1;
    am_n++;
  }
  return &am_tab[i];
}

static void am_grow(void) {
  uint32_t oc = am_cap;
  am_entry *ot = am_tab;
  am_cap = oc ? oc * 2 : AM_INIT_CAP;
  am_tab = (am_entry *)calloc(am_cap, sizeof(am_entry));
  am_n = 0;
  for (uint32_t i = 0; i < oc; i++)
    if (ot[i].used) {
      am_entry *e = am_put(ot[i].key, am_hash(ot[i].key));
      *e = ot[i];
    }
  free(ot);
}

static void am_build_snapshot(am_entry **ord, uint32_t *ord_n, int *valid, int dirty_only) {
  if (*valid)
    return;
  free(*ord);
  *ord = (am_entry *)calloc(am_n ? am_n : 1, sizeof(am_entry));
  *ord_n = 0;
  if (!*ord) {
    *valid = 1;
    return;
  }
  for (uint32_t i = 0; i < am_cap; i++)
    if (am_tab[i].used && (!dirty_only || am_tab[i].dirty))
      (*ord)[(*ord_n)++] = am_tab[i];
  qsort(*ord, *ord_n, sizeof(am_entry), am_entry_cmp_secure);
  *valid = 1;
}

static void am_build_order(void) {
  am_build_snapshot(&am_ord, &am_ord_n, &am_ord_valid, 0);
}

static void am_build_dirty_order(void) {
  am_build_snapshot(&am_dirty_ord, &am_dirty_ord_n, &am_dirty_ord_valid, 1);
}

unit acctmap_reset(const unit u) {
  (void)u;
  free(am_tab);
  am_tab = (am_entry *)calloc(AM_INIT_CAP, sizeof(am_entry));
  am_cap = AM_INIT_CAP;
  am_n = 0;
  am_cur_valid = 0;
  am_invalidate_order();
  return UNIT;
}

unit acctmap_key(uint64_t a2, uint64_t a1, uint64_t a0) {
  if (!am_tab)
    acctmap_reset(UNIT);
  am_cur_key[0] = a2;
  am_cur_key[1] = a1;
  am_cur_key[2] = a0;
  am_cur_hash = am_hash(am_cur_key);
  am_cur_valid = 1;
  return UNIT;
}

/* entry for the cached key, or NULL if absent */
static am_entry *am_cur(void) {
  if (!am_cur_valid || !am_tab)
    return NULL;
  uint32_t i = am_find(am_cur_key, am_cur_hash);
  return am_tab[i].used ? &am_tab[i] : NULL;
}

uint64_t acctmap_present(const unit u) {
  (void)u;
  return am_cur() ? 1 : 0;
}

static unit acctmap_write(uint8_t dirty,
                          uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                          uint64_t nonce,
                          uint64_t b3, uint64_t b2, uint64_t b1, uint64_t b0,
                          uint64_t sr3, uint64_t sr2, uint64_t sr1, uint64_t sr0,
                          uint64_t ch3, uint64_t ch2, uint64_t ch1, uint64_t ch0) {
  if (!am_cur_valid)
    return UNIT;
  am_entry *e = am_put(am_cur_key, am_cur_hash);
  e->hkey[0] = h3; e->hkey[1] = h2; e->hkey[2] = h1; e->hkey[3] = h0;
  e->nonce = nonce;
  e->bal[3] = b3; e->bal[2] = b2; e->bal[1] = b1; e->bal[0] = b0;
  e->sroot[3] = sr3; e->sroot[2] = sr2; e->sroot[1] = sr1; e->sroot[0] = sr0;
  e->chash[3] = ch3; e->chash[2] = ch2; e->chash[1] = ch1; e->chash[0] = ch0;
  if (dirty) {
    e->dirty = 1;
  } else {
    e->existed = 0;
  }
  am_invalidate_order();
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

unit acctmap_mark_existing(const unit u) {
  (void)u;
  am_entry *e = am_cur();
  if (e) {
    e->existed = 1;
    am_invalidate_order();
  }
  return UNIT;
}

uint64_t acctmap_nonce(const unit u) {
  (void)u;
  am_entry *e = am_cur();
  return e ? e->nonce : 0;
}
uint64_t acctmap_bal(uint64_t w) {
  am_entry *e = am_cur();
  return (e && w <= 3) ? e->bal[w] : 0;
}
uint64_t acctmap_sroot(uint64_t w) {
  am_entry *e = am_cur();
  return (e && w <= 3) ? e->sroot[w] : 0;
}
uint64_t acctmap_chash(uint64_t w) {
  am_entry *e = am_cur();
  return (e && w <= 3) ? e->chash[w] : 0;
}

/* physically remove the key (EIP-6780 account deletion at tx end). Open
 * addressing requires rebuilding the table so probe chains stay intact. */
unit acctmap_remove(uint64_t a2, uint64_t a1, uint64_t a0) {
  if (!am_tab)
    return UNIT;
  uint64_t k[3] = {a2, a1, a0};
  uint32_t i = am_find(k, am_hash(k));
  if (!am_tab[i].used)
    return UNIT;
  am_entry *ot = am_tab;
  uint32_t oc = am_cap;
  am_tab = (am_entry *)calloc(am_cap, sizeof(am_entry));
  am_n = 0;
  for (uint32_t j = 0; j < oc; j++)
    if (ot[j].used && memcmp(ot[j].key, k, 24) != 0) {
      am_entry *e = am_put(ot[j].key, am_hash(ot[j].key));
      *e = ot[j];
    }
  free(ot);
  am_cur_valid = 0;
  am_invalidate_order();
  return UNIT;
}

/* iteration (post-state root): acctmap_at(i) caches secure-key sorted row i */
uint64_t acctmap_count(const unit u) {
  (void)u;
  return am_n;
}

uint64_t acctmap_dirty_count(const unit u) {
  (void)u;
  if (!am_tab)
    return 0;
  am_build_dirty_order();
  return am_dirty_ord_n;
}

unit acctmap_at(uint64_t idx) {
  am_it_ok = 0;
  if (!am_tab)
    return UNIT;
  am_build_order();
  if (idx < am_ord_n) {
    am_it = am_ord[idx];
    am_it_ok = 1;
  }
  return UNIT;
}

unit acctmap_dirty_at(uint64_t idx) {
  am_it_ok = 0;
  if (!am_tab)
    return UNIT;
  am_build_dirty_order();
  if (idx < am_dirty_ord_n) {
    am_it = am_dirty_ord[idx];
    am_it_ok = 1;
  }
  return UNIT;
}

uint64_t acctmap_at_key(uint64_t w) { return (am_it_ok && w <= 2) ? am_it.key[w] : 0; }
uint64_t acctmap_at_nonce(const unit u) {
  (void)u;
  return am_it_ok ? am_it.nonce : 0;
}
uint64_t acctmap_at_bal(uint64_t w) { return (am_it_ok && w <= 3) ? am_it.bal[w] : 0; }
uint64_t acctmap_at_sroot(uint64_t w) { return (am_it_ok && w <= 3) ? am_it.sroot[w] : 0; }
uint64_t acctmap_at_chash(uint64_t w) { return (am_it_ok && w <= 3) ? am_it.chash[w] : 0; }
uint64_t acctmap_at_existed(const unit u) {
  (void)u;
  return am_it_ok ? am_it.existed : 0;
}
