/* C-backed transient storage table for evm-sail.
 *
 * This file backs EIP-1153 TLOAD/TSTORE only. The public write operation
 * records its prior logical value in kernel_state.c's private checkpoint undo
 * log; rollback uses a separate non-journaling restore path. Persistent
 * storage lives in state_db.c keyed by secure trie keys. */
#include "transient_storage.h"
#include "kernel_state.h"
#include "value_convert.h"
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
unit transient_storage_reset(const unit u) {
  (void)u;
  h_table_clear(&h_transient);
  return UNIT;
}

/* build the 7-word (address, slot) key */
static void transient_key(sail_fixed_bytes_20 addr, sail_u256 slot,
                          uint64_t key[7], uint64_t *hash) {
  uint8_t a[20];
  uint64_t sw[4];
  evmsail_address_to_be_bytes(a, addr);
  sail_word_to_be_words4(sw, slot);
  key[0] = ((uint64_t)a[0] << 24) | ((uint64_t)a[1] << 16) |
           ((uint64_t)a[2] << 8) | (uint64_t)a[3];
  key[1] = 0;
  key[2] = 0;
  for (size_t i = 0; i < 8; i++) key[1] = (key[1] << 8) | a[4 + i];
  for (size_t i = 0; i < 8; i++) key[2] = (key[2] << 8) | a[12 + i];
  key[3] = sw[0];
  key[4] = sw[1];
  key[5] = sw[2];
  key[6] = sw[3];
  *hash = h_hash(key);
}

static h_entry *transient_storage_lookup(sail_fixed_bytes_20 addr, sail_u256 slot) {
  uint64_t key[7], h;
  transient_key(addr, slot, key, &h);
  return h_get(&h_transient, key, h);
}

static unit transient_storage_write_raw(sail_fixed_bytes_20 addr, sail_u256 slot,
                                        sail_u256 v) {
  uint64_t key[7], h;
  transient_key(addr, slot, key, &h);
  uint64_t w[4];
  sail_word_to_be_words4(w, v);
  if (!h_put(&h_transient, key, h, w)) abort();
  return UNIT;
}

/* Store a value and make the update part of the current semantic checkpoint.
 * Transient storage is a total map with default zero, so restoring an absent
 * key as an explicit zero is observationally identical. */
unit transient_storage_write(sail_fixed_bytes_20 addr, const sail_u256 slot,
                             const sail_u256 v) {
  static const uint64_t zero[4] = {0, 0, 0, 0};
  sail_u256 slot_value = (slot);
  sail_u256 value = (v);
  h_entry *entry = transient_storage_lookup(addr, slot_value);
  sail_u256 prior = be_words4_to_sail_word(entry ? entry->val : zero);
  state_journal_push_transient(addr, slot_value, prior);
  return transient_storage_write_raw(addr, slot_value, value);
}

unit transient_storage_restore(sail_fixed_bytes_20 addr, sail_u256 slot,
                               sail_u256 v) {
  return transient_storage_write_raw(addr, slot, v);
}

/* the 256-bit value at (address, slot); 0 if absent */
sail_u256 transient_storage_read(sail_fixed_bytes_20 addr,
                                 const sail_u256 slot) {
  static const uint64_t zero[4] = {0, 0, 0, 0};
  h_entry *entry = transient_storage_lookup(addr, (slot));
  return (be_words4_to_sail_word(entry ? entry->val : zero));
}
