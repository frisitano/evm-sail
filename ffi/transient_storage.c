/* C-backed transient storage table for evm-sail.
 *
 * This file backs EIP-1153 TLOAD/TSTORE only. Warm address and warm slot sets
 * live in Sail registers, and persistent storage lives in state_db.c keyed by
 * secure trie keys. Only mach_bits (uint64_t) cross the FFI. */
#include "transient_storage.h"
#include "lbits_convert.h"
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

/* select a transient-storage (address, slot) key for the accessors below */
unit transient_storage_key(uint64_t id, const lbits addr, const lbits slot) {
  if (id != 0 || !h_ensure(&h_transient)) {
    h_cur_ok = 0;
    return UNIT;
  }

  uint64_t a[4], sw[4];
  lbits_to_be_words4(a, addr);   /* a[0] = 0 (160-bit value), a[1..3] = address */
  lbits_to_be_words4(sw, slot);
  h_cur_key[0] = a[1];
  h_cur_key[1] = a[2];
  h_cur_key[2] = a[3];
  h_cur_key[3] = sw[0];
  h_cur_key[4] = sw[1];
  h_cur_key[5] = sw[2];
  h_cur_key[6] = sw[3];
  h_cur_hash = h_hash(h_cur_key);
  h_cur_ok = 1;
  return UNIT;
}

/* store the 256-bit value at the selected key */
unit transient_storage_write(const lbits v) {
  if (h_cur_ok) {
    uint64_t w[4];
    lbits_to_be_words4(w, v);
    (void)h_put(&h_transient, h_cur_key, h_cur_hash, w);
  }
  return UNIT;
}

/* the 256-bit value at the selected key; 0 if absent */
void transient_storage_read(lbits *rop, const unit u) {
  (void)u;
  static const uint64_t zero[4] = {0, 0, 0, 0};
  h_entry *e = h_cur_ok ? h_get(&h_transient, h_cur_key, h_cur_hash) : NULL;
  be_words4_to_lbits(rop, e ? e->val : zero);
}
