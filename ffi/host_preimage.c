/* C-backed pre-image cache for the evm-sail stateless reader.
 *
 * raw key -> keccak(key), the secure-trie key derivation memoized (see
 * host_preimage.h). Two open-addressing maps, addr (3-word key) and slot
 * (4-word key); value is the 256-bit keccak (4 words). keccak itself is computed
 * Sail-side; this only caches it. FNV-1a over the key words, power-of-two
 * capacity, only uint64 crosses the FFI -- same conventions as host_map.c. */
#include "host_preimage.h"
#include <stdlib.h>
#include <string.h>

#define PI_INIT_CAP 1024u /* power of two */

/* a key is up to 4 words; addr uses 3 (word 3 = 0), slot uses 4. */
typedef struct {
  uint64_t key[4];
  uint64_t hash[4]; /* keccak: h3..h0 (index 3 most significant) */
  uint8_t used;
} pi_entry;

typedef struct {
  pi_entry *tab;
  uint32_t cap;
  uint32_t n;
  int nkey;        /* 3 (addr) or 4 (slot) */
  uint64_t cur[4]; /* cached selected key */
  uint64_t curhash;
  int cur_valid;
} pi_map;

static pi_map pa = {0}, ps = {0};

static uint64_t pi_hash(const uint64_t *k, int nkey) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (int i = 0; i < nkey; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) {
      h ^= (w >> (8 * b)) & 0xff;
      h *= 0x100000001b3ull;
    }
  }
  return h;
}

static void pi_grow(pi_map *m);

static uint32_t pi_find(pi_map *m, const uint64_t *k, uint64_t h) {
  uint32_t i = (uint32_t)h & (m->cap - 1);
  while (m->tab[i].used && memcmp(m->tab[i].key, k, (size_t)m->nkey * 8) != 0)
    i = (i + 1) & (m->cap - 1);
  return i;
}

static void pi_grow(pi_map *m) {
  uint32_t oc = m->cap;
  pi_entry *ot = m->tab;
  m->cap = oc ? oc * 2 : PI_INIT_CAP;
  m->tab = (pi_entry *)calloc(m->cap, sizeof(pi_entry));
  m->n = 0;
  for (uint32_t i = 0; i < oc; i++)
    if (ot[i].used) {
      uint32_t j = pi_find(m, ot[i].key, pi_hash(ot[i].key, m->nkey));
      m->tab[j] = ot[i];
      m->n++;
    }
  free(ot);
}

static void pi_reset(pi_map *m, int nkey) {
  free(m->tab);
  m->tab = (pi_entry *)calloc(PI_INIT_CAP, sizeof(pi_entry));
  m->cap = PI_INIT_CAP;
  m->n = 0;
  m->nkey = nkey;
  m->cur_valid = 0;
}

static void pi_select(pi_map *m, const uint64_t *k) {
  if (!m->tab)
    pi_reset(m, m->nkey ? m->nkey : 4);
  for (int i = 0; i < 4; i++)
    m->cur[i] = k[i];
  m->curhash = pi_hash(m->cur, m->nkey);
  m->cur_valid = 1;
}

static uint64_t pi_present(pi_map *m) {
  if (!m->cur_valid || !m->tab)
    return 0;
  uint32_t i = pi_find(m, m->cur, m->curhash);
  return m->tab[i].used ? 1 : 0;
}

static void pi_store(pi_map *m, const uint64_t *hv) {
  if (!m->cur_valid)
    return;
  if ((m->n + 1) * 4 >= m->cap * 3)
    pi_grow(m);
  uint32_t i = pi_find(m, m->cur, m->curhash);
  if (!m->tab[i].used) {
    memcpy(m->tab[i].key, m->cur, 32);
    m->tab[i].used = 1;
    m->n++;
  }
  memcpy(m->tab[i].hash, hv, 32);
}

static uint64_t pi_word(pi_map *m, uint64_t w) {
  if (!m->cur_valid || !m->tab || w > 3)
    return 0;
  uint32_t i = pi_find(m, m->cur, m->curhash);
  return m->tab[i].used ? m->tab[i].hash[w] : 0;
}

/* ------------------------------ address map ----------------------------- */
unit pa_reset(const unit u) {
  (void)u;
  pi_reset(&pa, 3);
  return UNIT;
}
unit pa_key(uint64_t a2, uint64_t a1, uint64_t a0) {
  uint64_t k[4] = {a2, a1, a0, 0};
  if (!pa.tab)
    pi_reset(&pa, 3);
  pi_select(&pa, k);
  return UNIT;
}
uint64_t pa_present(const unit u) {
  (void)u;
  return pi_present(&pa);
}
unit pa_store(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  uint64_t hv[4] = {h0, h1, h2, h3};
  pi_store(&pa, hv);
  return UNIT;
}
uint64_t pa_word(uint64_t w) { return pi_word(&pa, w); }

/* ------------------------------- slot map ------------------------------- */
unit ps_reset(const unit u) {
  (void)u;
  pi_reset(&ps, 4);
  return UNIT;
}
unit ps_key(uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0) {
  uint64_t k[4] = {s3, s2, s1, s0};
  if (!ps.tab)
    pi_reset(&ps, 4);
  pi_select(&ps, k);
  return UNIT;
}
uint64_t ps_present(const unit u) {
  (void)u;
  return pi_present(&ps);
}
unit ps_store(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0) {
  uint64_t hv[4] = {h0, h1, h2, h3};
  pi_store(&ps, hv);
  return UNIT;
}
uint64_t ps_word(uint64_t w) { return pi_word(&ps, w); }
