/* C-backed witness node-db for the evm-sail stateless reader.
 *
 * keccak(node) -> (offset, len) span into the SSZ witness buffer. See
 * trie_node_db.h for the rationale (kills the O(N^2) Sail assoc-list scan). The
 * keccak is computed Sail-side (over the materialized node) and the resulting
 * span is recorded here; lookups are O(1)-amortized open addressing. Mirrors the
 * conventions of the other C hash tables: keys/values cross the FFI as whole
 * fixed hash values, FNV-1a over the key words, power-of-two capacity. Internal key
 * arrays are little-endian word order (index 0 = least significant). */
#include "trie_node_db.h"
#include "value_convert.h"
#include <stdlib.h>
#include <string.h>

#define ND_INIT_CAP 1024u /* power of two */

typedef struct {
  uint64_t key[4]; /* keccak(node): k0 (low) .. k3 (high) */
  uint64_t off;
  uint64_t len;
  uint8_t used;
} nd_entry;

static nd_entry *nd_tab = NULL;
static uint32_t nd_cap = 0;
static uint32_t nd_n = 0;

/* Memoized last lookup. */
static uint64_t nd_memo_key[4];
static uint64_t nd_memo_off;
static uint64_t nd_memo_len; /* 0 = absent (witness nodes are never empty) */
static int nd_memo_valid = 0;

static uint64_t nd_hash(const uint64_t *k) {
  uint64_t h = 0xcbf29ce484222325ull; /* FNV-1a over the 4 key words */
  for (int i = 0; i < 4; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) {
      h ^= (w >> (8 * b)) & 0xff;
      h *= 0x100000001b3ull;
    }
  }
  return h;
}

static void nd_grow(void);

static void nd_put(const uint64_t *k, uint64_t off, uint64_t len) {
  if ((nd_n + 1) * 4 >= nd_cap * 3)
    nd_grow();
  uint32_t m = nd_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (nd_tab[i].used) {
    if (memcmp(nd_tab[i].key, k, 32) == 0) { /* same node: keep first span */
      return;
    }
    i = (i + 1) & m;
  }
  memcpy(nd_tab[i].key, k, 32);
  nd_tab[i].off = off;
  nd_tab[i].len = len;
  nd_tab[i].used = 1;
  nd_n++;
}

static void nd_grow(void) {
  uint32_t oc = nd_cap;
  nd_entry *ot = nd_tab;
  nd_cap = oc ? oc * 2 : ND_INIT_CAP;
  nd_tab = (nd_entry *)calloc(nd_cap, sizeof(nd_entry));
  nd_n = 0;
  for (uint32_t i = 0; i < oc; i++)
    if (ot[i].used)
      nd_put(ot[i].key, ot[i].off, ot[i].len);
  free(ot);
}

unit nodedb_reset(const unit u) {
  (void)u;
  free(nd_tab);
  nd_tab = (nd_entry *)calloc(ND_INIT_CAP, sizeof(nd_entry));
  nd_cap = ND_INIT_CAP;
  nd_n = 0;
  nd_memo_valid = 0;
  return UNIT;
}

unit nodedb_insert(sail_hash kh, EVMSAIL_BYTE_QUANTITY_PARAM(off),
                   EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  uint64_t k[4];
  sail_hash_to_le_words4(k, kh);
  if (!nd_tab)
    nodedb_reset(UNIT);
  nd_put(k, evmsail_byte_quantity_value(off),
         evmsail_byte_quantity_value(len));
  nd_memo_valid = 0;
  return UNIT;
}

/* Memoized point lookup. */
static void nd_find(sail_hash kh) {
  uint64_t k[4];
  sail_hash_to_le_words4(k, kh);
  if (nd_memo_valid && memcmp(nd_memo_key, k, 32) == 0)
    return;
  memcpy(nd_memo_key, k, 32);
  nd_memo_off = 0;
  nd_memo_len = 0;
  nd_memo_valid = 1;
  if (!nd_tab)
    return;
  uint32_t m = nd_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (nd_tab[i].used) {
    if (memcmp(nd_tab[i].key, k, 32) == 0) {
      nd_memo_off = nd_tab[i].off;
      nd_memo_len = nd_tab[i].len;
      return;
    }
    i = (i + 1) & m;
  }
}

bool nodedb_lookup_span(sail_hash kh, uint64_t *off, uint64_t *len) {
  nd_find(kh);
  if (off)
    *off = nd_memo_off;
  if (len)
    *len = nd_memo_len;
  return nd_memo_len != 0;
}
