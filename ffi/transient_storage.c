/* C-backed transient storage table for evm-sail.
 *
 * This file backs EIP-1153 TLOAD/TSTORE only. Warm address and warm slot sets
 * live in Sail registers, and persistent storage lives in state_db.c keyed by
 * secure trie keys. Only mach_bits (uint64_t) cross the FFI. */
#include "transient_storage.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HOST_INIT_CAP 1024u /* power of two */

typedef struct {
  uint64_t key[7]; /* addr a2,a1,a0 (BE words) + slot s3..s0 */
  uint64_t val[4]; /* value v3..v0 (BE words, v3 most significant) */
  uint8_t used;
} h_entry;

typedef struct {
  h_entry *tab;
  uint32_t cap; /* power of two */
  uint32_t n;   /* used entries */
} h_table;

static h_table h_transient;

/* cached key selection (transient_storage_key) */
static int h_cur_ok;
static uint64_t h_cur_key[7];
static uint64_t h_cur_hash;

static uint64_t h_hash(const uint64_t *k) {
  uint64_t h = 0xcbf29ce484222325ull; /* FNV-1a over the 7 key words */
  for (int i = 0; i < 7; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) {
      h ^= (w >> (8 * b)) & 0xff;
      h *= 0x100000001b3ull;
    }
  }
  return h;
}

static void h_table_clear(h_table *m) {
  free(m->tab);
  memset(m, 0, sizeof *m);
}

static int h_ensure(h_table *m) {
  if (m->tab) return 1;
  m->cap = HOST_INIT_CAP;
  m->n = 0;
  m->tab = (h_entry *)calloc(m->cap, sizeof(h_entry));
  if (!m->tab) {
    m->cap = 0;
    return 0;
  }
  return 1;
}

/* slot for key in table m (the matching entry or the first free one) */
static uint32_t h_find(h_table *m, const uint64_t *k, uint64_t h) {
  uint32_t i = (uint32_t)(h & (m->cap - 1));
  for (;;) {
    h_entry *e = &m->tab[i];
    if (!e->used || memcmp(e->key, k, sizeof e->key) == 0) return i;
    i = (i + 1) & (m->cap - 1);
  }
}

static int h_grow(h_table *m) {
  uint32_t ocap = m->cap;
  h_entry *otab = m->tab;
  h_entry *ntab = (h_entry *)calloc(ocap * 2, sizeof(h_entry));
  if (!ntab) return 0;

  m->tab = ntab;
  m->cap = ocap * 2;
  m->n = 0;
  for (uint32_t i = 0; i < ocap; i++) {
    if (otab[i].used) {
      uint32_t j = h_find(m, otab[i].key, h_hash(otab[i].key));
      m->tab[j] = otab[i];
      m->n++;
    }
  }
  free(otab);
  return 1;
}

static int h_put(h_table *m, const uint64_t *k, uint64_t h, const uint64_t *v) {
  if (!h_ensure(m)) return 0;
  if (m->n * 10 >= m->cap * 7 && !h_grow(m)) return 0;

  uint32_t i = h_find(m, k, h);
  h_entry *e = &m->tab[i];
  if (!e->used) {
    e->used = 1;
    memcpy(e->key, k, sizeof e->key);
    m->n++;
  }
  memcpy(e->val, v, 4 * sizeof(uint64_t));
  return 1;
}

static h_entry *h_get(h_table *m, const uint64_t *k, uint64_t h) {
  if (!m->tab) return NULL;
  uint32_t i = h_find(m, k, h);
  return m->tab[i].used ? &m->tab[i] : NULL;
}

/* clear transient storage at tx/world reset */
unit transient_storage_reset(uint64_t id) {
  if (id == 0) {
    h_table_clear(&h_transient);
    h_cur_ok = 0;
  }
  return UNIT;
}

/* select a transient-storage key for the word accessors below */
unit transient_storage_key(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0,
                           uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0) {
  if (id != 0 || !h_ensure(&h_transient)) {
    h_cur_ok = 0;
    return UNIT;
  }

  h_cur_key[0] = a2;
  h_cur_key[1] = a1;
  h_cur_key[2] = a0;
  h_cur_key[3] = s3;
  h_cur_key[4] = s2;
  h_cur_key[5] = s1;
  h_cur_key[6] = s0;
  h_cur_hash = h_hash(h_cur_key);
  h_cur_ok = 1;
  return UNIT;
}

/* store the 256-bit value (v3 = most significant) at the selected key */
unit transient_storage_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) {
  if (h_cur_ok) {
    uint64_t v[4] = { v3, v2, v1, v0 };
    (void)h_put(&h_transient, h_cur_key, h_cur_hash, v);
  }
  return UNIT;
}

/* value word i (3 = most significant .. 0) at the selected key; 0 if absent */
uint64_t transient_storage_word(uint64_t i) {
  if (i > 3 || !h_cur_ok) return 0;
  h_entry *e = h_get(&h_transient, h_cur_key, h_cur_hash);
  return e ? e->val[3 - i] : 0;
}
