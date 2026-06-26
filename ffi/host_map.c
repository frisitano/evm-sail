/* C-backed (address, slot) -> 256-bit value maps for the evm-sail model.
 *
 * Maps (by id): 0 = EIP-1153 transient storage, 1 = the EIP-2929 warm-slot set
 * (low word 1 = warm), 2 = the persistent account STORAGE. The Sail assoc lists
 * made every access an O(n) scan; here every access is an O(1)-amortized
 * open-addressing hash lookup.
 *
 * LAYERS (overlay state): a map is a stack of hash-table layers. Reads walk
 * top -> bottom (first hit wins); writes go to the TOP layer. host_map_push opens
 * a call-frame/tx overlay; host_map_pop_commit merges it into the layer below;
 * host_map_pop_discard drops it (frame revert = discard -- no undo journal
 * needed). The BASE layer (bottom) is therefore exactly the state at the start
 * of the current transaction, which gives the EIP-2200 "original" value for
 * free (host_map_base_word). The transient/warm maps simply never push.
 *
 * PRESENCE: host_map_present distinguishes "key present with value 0" from
 * "absent" -- required by the stateless-witness fail-closed lookups.
 *
 * Only mach_bits (uint64_t) cross the FFI, matching ffi/host_mem.c. A key is
 * selected once with host_map_key (which caches map + key + hash); the accessors
 * (host_map_word / host_map_base_word / host_map_present / host_map_store) resolve
 * against the cached key. */
#include "sail.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HOST_NMAPS    3
#define HOST_INIT_CAP 1024u        /* power of two */

typedef struct {
  uint64_t key[7];               /* addr a2,a1,a0 (BE words) + slot s3..s0 */
  uint64_t val[4];               /* value v3..v0 (BE words, v3 most significant) */
  uint8_t  used;
} h_entry;

typedef struct h_layer {
  h_entry *tab;
  uint32_t cap;                  /* power of two */
  uint32_t n;                    /* used entries */
  struct h_layer *below;        /* next layer down (NULL for the base) */
} h_layer;

static h_layer *h_maps[HOST_NMAPS];   /* top layer of each map (NULL until first use) */

/* cached key selection (host_map_key) */
static int      h_cur_id = -1;
static uint64_t h_cur_key[7];
static uint64_t h_cur_hash;

/* flattened-iteration buffer (host_map_count / host_map_sel) */
static h_entry *h_it;
static uint32_t  h_it_n;
static uint32_t  h_it_sel;

static uint64_t h_hash(const uint64_t *k) {
  uint64_t h = 0xcbf29ce484222325ull;            /* FNV-1a over the 7 key words */
  for (int i = 0; i < 7; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) { h ^= (w >> (8 * b)) & 0xff; h *= 0x100000001b3ull; }
  }
  return h;
}

static h_layer *h_layer_new(uint32_t cap, h_layer *below) {
  h_layer *l = (h_layer *)calloc(1, sizeof(h_layer));
  l->tab = (h_entry *)calloc(cap, sizeof(h_entry));
  l->cap = cap;
  l->below = below;
  return l;
}

/* slot for key in layer l (the matching entry or the first free one) */
static uint32_t h_find(h_layer *l, const uint64_t *k, uint64_t h) {
  uint32_t i = (uint32_t)(h & (l->cap - 1));
  for (;;) {
    h_entry *e = &l->tab[i];
    if (!e->used || memcmp(e->key, k, sizeof e->key) == 0) return i;
    i = (i + 1) & (l->cap - 1);
  }
}

static void h_grow(h_layer *l) {
  uint32_t ocap = l->cap;
  h_entry *otab = l->tab;
  l->cap = ocap * 2;
  l->tab = (h_entry *)calloc(l->cap, sizeof(h_entry));
  if (!l->tab) { l->tab = otab; l->cap = ocap; return; }   /* OOM: keep old table */
  for (uint32_t i = 0; i < ocap; i++)
    if (otab[i].used) l->tab[h_find(l, otab[i].key, h_hash(otab[i].key))] = otab[i];
  free(otab);
}

/* insert/update key->val in layer l */
static void h_put(h_layer *l, const uint64_t *k, uint64_t h, const uint64_t *v) {
  uint32_t i = h_find(l, k, h);
  h_entry *e = &l->tab[i];
  if (!e->used) {
    e->used = 1;
    memcpy(e->key, k, sizeof e->key);
    l->n++;
  }
  memcpy(e->val, v, 4 * sizeof(uint64_t));
  if (l->n * 10 >= l->cap * 7) h_grow(l);     /* > 70% load */
}

static h_layer *h_top(uint64_t id) {
  if (id >= HOST_NMAPS) return NULL;
  if (!h_maps[id]) h_maps[id] = h_layer_new(HOST_INIT_CAP, NULL);
  return h_maps[id];
}

/* entry for the cached key, walking layers top -> bottom; NULL if absent */
static h_entry *h_walk(void) {
  if (h_cur_id < 0) return NULL;
  for (h_layer *l = h_maps[h_cur_id]; l; l = l->below) {
    uint32_t i = h_find(l, h_cur_key, h_cur_hash);
    if (l->tab[i].used) return &l->tab[i];
  }
  return NULL;
}

/* ------------------------------ lifecycle ------------------------------- */

/* clear map id back to a single empty base layer (per block/test case) */
unit host_map_reset(uint64_t id) {
  if (id < HOST_NMAPS) {
    h_layer *l = h_maps[id];
    while (l) { h_layer *b = l->below; free(l->tab); free(l); l = b; }
    h_maps[id] = NULL;
    h_cur_id = -1;
  }
  return UNIT;
}

/* open an overlay layer (call-frame / tx scope) */
unit host_map_push(uint64_t id) {
  if (id < HOST_NMAPS) {
    h_layer *top = h_top(id);
    h_maps[id] = h_layer_new(HOST_INIT_CAP, top);
  }
  return UNIT;
}

/* merge the top layer into the layer below (frame/tx commit) */
unit host_map_pop_commit(uint64_t id) {
  if (id < HOST_NMAPS && h_maps[id] && h_maps[id]->below) {
    h_layer *top = h_maps[id];
    h_layer *below = top->below;
    for (uint32_t i = 0; i < top->cap; i++)
      if (top->tab[i].used)
        h_put(below, top->tab[i].key, h_hash(top->tab[i].key), top->tab[i].val);
    h_maps[id] = below;
    free(top->tab); free(top);
  }
  return UNIT;
}

/* drop the top layer (frame/tx revert -- no undo journal needed) */
unit host_map_pop_discard(uint64_t id) {
  if (id < HOST_NMAPS && h_maps[id] && h_maps[id]->below) {
    h_layer *top = h_maps[id];
    h_maps[id] = top->below;
    free(top->tab); free(top);
  }
  return UNIT;
}

/* ------------------------------ key access ------------------------------ */

/* select (map, key): cache it for the accessors below */
unit host_map_key(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0,
                uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0) {
  if (id >= HOST_NMAPS) { h_cur_id = -1; return UNIT; }
  (void)h_top(id);                       /* ensure the base layer exists */
  h_cur_key[0] = a2; h_cur_key[1] = a1; h_cur_key[2] = a0;
  h_cur_key[3] = s3; h_cur_key[4] = s2; h_cur_key[5] = s1; h_cur_key[6] = s0;
  h_cur_hash = h_hash(h_cur_key);
  h_cur_id = (int)id;
  return UNIT;
}

/* store the 256-bit value (v3 = most significant) at the selected key (TOP layer) */
unit host_map_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) {
  if (h_cur_id >= 0) {
    uint64_t v[4] = { v3, v2, v1, v0 };
    h_put(h_maps[h_cur_id], h_cur_key, h_cur_hash, v);
  }
  return UNIT;
}

/* value word i (3 = most significant .. 0) at the selected key; 0 if absent.
 * Walks the overlay stack: the most recent write wins. */
uint64_t host_map_word(uint64_t i) {
  if (i > 3) return 0;
  h_entry *e = h_walk();
  return e ? e->val[3 - i] : 0;
}

/* value word i at the selected key in the BASE layer only (EIP-2200 original:
 * the bottom layer is exactly the state at the start of the current tx). */
uint64_t host_map_base_word(uint64_t i) {
  if (h_cur_id < 0 || i > 3) return 0;
  h_layer *l = h_maps[h_cur_id];
  while (l->below) l = l->below;
  uint32_t j = h_find(l, h_cur_key, h_cur_hash);
  return l->tab[j].used ? l->tab[j].val[3 - i] : 0;
}

/* 1 if the selected key is present in ANY layer (distinguishes an explicit 0
 * from absent -- the stateless-witness fail-closed test). */
uint64_t host_map_present(const unit u) {
  (void)u;
  return h_walk() ? 1 : 0;
}

/* 1 if the selected key is present in the BASE (bottom) layer only -- the
 * EIP-2200 "original" presence test: distinguishes a committed base 0 from a
 * base-absent slot (which a stateless reader resolves from the witness). */
uint64_t host_map_base_present(const unit u) {
  (void)u;
  if (h_cur_id < 0 || !h_maps[h_cur_id]) return 0;
  h_layer *l = h_maps[h_cur_id];
  while (l->below) l = l->below;
  uint32_t j = h_find(l, h_cur_key, h_cur_hash);
  return l->tab[j].used ? 1 : 0;
}

/* ----------------------------- address sweeps --------------------------- */

/* 1 if any key with address (a2,a1,a0) exists in any layer of map id */
uint64_t host_map_has_addr(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0) {
  if (id >= HOST_NMAPS) return 0;
  for (h_layer *l = h_maps[id]; l; l = l->below)
    for (uint32_t i = 0; i < l->cap; i++) {
      h_entry *e = &l->tab[i];
      if (e->used && e->key[0] == a2 && e->key[1] == a1 && e->key[2] == a0) return 1;
    }
  return 0;
}

/* physically remove every key with address (a2,a1,a0) from every layer of map
 * id (EIP-6780 account deletion; runs at tx end when only the base is live).
 * Open addressing requires re-inserting the probe-chain remainder. */
unit host_map_wipe_addr(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0) {
  if (id >= HOST_NMAPS) return UNIT;
  for (h_layer *l = h_maps[id]; l; l = l->below) {
    h_layer fresh = { 0 };
    fresh.cap = l->cap;
    fresh.tab = (h_entry *)calloc(fresh.cap, sizeof(h_entry));
    if (!fresh.tab) return UNIT;
    for (uint32_t i = 0; i < l->cap; i++) {
      h_entry *e = &l->tab[i];
      if (e->used && !(e->key[0] == a2 && e->key[1] == a1 && e->key[2] == a0))
        h_put(&fresh, e->key, h_hash(e->key), e->val);
    }
    free(l->tab);
    l->tab = fresh.tab; l->cap = fresh.cap; l->n = fresh.n;
  }
  return UNIT;
}

/* ------------------------------ enumeration ----------------------------- */

/* flatten map id (all layers merged, top wins) into the iteration buffer and
 * return the entry count. Used by dump_state / the state-root walk (cold). */
uint64_t host_map_count(uint64_t id) {
  free(h_it); h_it = NULL; h_it_n = 0;
  if (id >= HOST_NMAPS || !h_maps[id]) return 0;
  /* merge bottom -> top into a scratch layer so upper layers overwrite */
  h_layer scratch = { 0 };
  scratch.cap = HOST_INIT_CAP;
  scratch.tab = (h_entry *)calloc(scratch.cap, sizeof(h_entry));
  if (!scratch.tab) return 0;
  /* collect layers bottom-first */
  h_layer *stack[64]; int depth = 0;
  for (h_layer *l = h_maps[id]; l && depth < 64; l = l->below) stack[depth++] = l;
  for (int d = depth - 1; d >= 0; d--) {
    h_layer *l = stack[d];
    for (uint32_t i = 0; i < l->cap; i++)
      if (l->tab[i].used)
        h_put(&scratch, l->tab[i].key, h_hash(l->tab[i].key), l->tab[i].val);
  }
  h_it = (h_entry *)calloc(scratch.n ? scratch.n : 1, sizeof(h_entry));
  for (uint32_t i = 0; i < scratch.cap; i++)
    if (scratch.tab[i].used) h_it[h_it_n++] = scratch.tab[i];
  free(scratch.tab);
  return h_it_n;
}

/* select flattened entry j for key/value word reads */
unit host_map_sel(uint64_t j) { h_it_sel = (uint32_t)j; return UNIT; }

/* key word i of the selected flattened entry: 0..2 = addr (a2,a1,a0), 3..6 = slot (s3..s0) */
uint64_t host_map_it_key(uint64_t i) {
  if (h_it_sel >= h_it_n || i > 6) return 0;
  return h_it[h_it_sel].key[i];
}

/* value word i (3 = most significant .. 0) of the selected flattened entry */
uint64_t host_map_it_val(uint64_t i) {
  if (h_it_sel >= h_it_n || i > 3) return 0;
  return h_it[h_it_sel].val[3 - i];
}
