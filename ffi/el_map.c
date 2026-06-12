/* C-backed (address, slot) -> 256-bit value maps for the EL-IR Sail model.
 *
 * Maps (by id): 0 = EIP-1153 transient storage, 1 = the EIP-2929 warm-slot set
 * (low word 1 = warm), 2 = the persistent account STORAGE. The Sail assoc lists
 * made every access an O(n) scan; here every access is an O(1)-amortized
 * open-addressing hash lookup.
 *
 * LAYERS (overlay state): a map is a stack of hash-table layers. Reads walk
 * top -> bottom (first hit wins); writes go to the TOP layer. el_map_push opens
 * a call-frame/tx overlay; el_map_pop_commit merges it into the layer below;
 * el_map_pop_discard drops it (frame revert = discard -- no undo journal
 * needed). The BASE layer (bottom) is therefore exactly the state at the start
 * of the current transaction, which gives the EIP-2200 "original" value for
 * free (el_map_base_word). The transient/warm maps simply never push.
 *
 * PRESENCE: el_map_present distinguishes "key present with value 0" from
 * "absent" -- required by the stateless-witness fail-closed lookups.
 *
 * Only mach_bits (uint64_t) cross the FFI, matching ffi/el_mem.c. A key is
 * selected once with el_map_key (which caches map + key + hash); the accessors
 * (el_map_word / el_map_base_word / el_map_present / el_map_store) resolve
 * against the cached key. */
#include "sail.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define EL_NMAPS    3
#define EL_INIT_CAP 1024u        /* power of two */

typedef struct {
  uint64_t key[7];               /* addr a2,a1,a0 (BE words) + slot s3..s0 */
  uint64_t val[4];               /* value v3..v0 (BE words, v3 most significant) */
  uint8_t  used;
} el_entry;

typedef struct el_layer {
  el_entry *tab;
  uint32_t cap;                  /* power of two */
  uint32_t n;                    /* used entries */
  struct el_layer *below;        /* next layer down (NULL for the base) */
} el_layer;

static el_layer *el_maps[EL_NMAPS];   /* top layer of each map (NULL until first use) */

/* cached key selection (el_map_key) */
static int      el_cur_id = -1;
static uint64_t el_cur_key[7];
static uint64_t el_cur_hash;

/* flattened-iteration buffer (el_map_count / el_map_sel) */
static el_entry *el_it;
static uint32_t  el_it_n;
static uint32_t  el_it_sel;

static uint64_t el_hash(const uint64_t *k) {
  uint64_t h = 0xcbf29ce484222325ull;            /* FNV-1a over the 7 key words */
  for (int i = 0; i < 7; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) { h ^= (w >> (8 * b)) & 0xff; h *= 0x100000001b3ull; }
  }
  return h;
}

static el_layer *el_layer_new(uint32_t cap, el_layer *below) {
  el_layer *l = (el_layer *)calloc(1, sizeof(el_layer));
  l->tab = (el_entry *)calloc(cap, sizeof(el_entry));
  l->cap = cap;
  l->below = below;
  return l;
}

/* slot for key in layer l (the matching entry or the first free one) */
static uint32_t el_find(el_layer *l, const uint64_t *k, uint64_t h) {
  uint32_t i = (uint32_t)(h & (l->cap - 1));
  for (;;) {
    el_entry *e = &l->tab[i];
    if (!e->used || memcmp(e->key, k, sizeof e->key) == 0) return i;
    i = (i + 1) & (l->cap - 1);
  }
}

static void el_grow(el_layer *l) {
  uint32_t ocap = l->cap;
  el_entry *otab = l->tab;
  l->cap = ocap * 2;
  l->tab = (el_entry *)calloc(l->cap, sizeof(el_entry));
  if (!l->tab) { l->tab = otab; l->cap = ocap; return; }   /* OOM: keep old table */
  for (uint32_t i = 0; i < ocap; i++)
    if (otab[i].used) l->tab[el_find(l, otab[i].key, el_hash(otab[i].key))] = otab[i];
  free(otab);
}

/* insert/update key->val in layer l */
static void el_put(el_layer *l, const uint64_t *k, uint64_t h, const uint64_t *v) {
  uint32_t i = el_find(l, k, h);
  el_entry *e = &l->tab[i];
  if (!e->used) {
    e->used = 1;
    memcpy(e->key, k, sizeof e->key);
    l->n++;
  }
  memcpy(e->val, v, 4 * sizeof(uint64_t));
  if (l->n * 10 >= l->cap * 7) el_grow(l);     /* > 70% load */
}

static el_layer *el_top(uint64_t id) {
  if (id >= EL_NMAPS) return NULL;
  if (!el_maps[id]) el_maps[id] = el_layer_new(EL_INIT_CAP, NULL);
  return el_maps[id];
}

/* entry for the cached key, walking layers top -> bottom; NULL if absent */
static el_entry *el_walk(void) {
  if (el_cur_id < 0) return NULL;
  for (el_layer *l = el_maps[el_cur_id]; l; l = l->below) {
    uint32_t i = el_find(l, el_cur_key, el_cur_hash);
    if (l->tab[i].used) return &l->tab[i];
  }
  return NULL;
}

/* ------------------------------ lifecycle ------------------------------- */

/* clear map id back to a single empty base layer (per block/test case) */
unit el_map_reset(uint64_t id) {
  if (id < EL_NMAPS) {
    el_layer *l = el_maps[id];
    while (l) { el_layer *b = l->below; free(l->tab); free(l); l = b; }
    el_maps[id] = NULL;
    el_cur_id = -1;
  }
  return UNIT;
}

/* open an overlay layer (call-frame / tx scope) */
unit el_map_push(uint64_t id) {
  if (id < EL_NMAPS) {
    el_layer *top = el_top(id);
    el_maps[id] = el_layer_new(EL_INIT_CAP, top);
  }
  return UNIT;
}

/* merge the top layer into the layer below (frame/tx commit) */
unit el_map_pop_commit(uint64_t id) {
  if (id < EL_NMAPS && el_maps[id] && el_maps[id]->below) {
    el_layer *top = el_maps[id];
    el_layer *below = top->below;
    for (uint32_t i = 0; i < top->cap; i++)
      if (top->tab[i].used)
        el_put(below, top->tab[i].key, el_hash(top->tab[i].key), top->tab[i].val);
    el_maps[id] = below;
    free(top->tab); free(top);
  }
  return UNIT;
}

/* drop the top layer (frame/tx revert -- no undo journal needed) */
unit el_map_pop_discard(uint64_t id) {
  if (id < EL_NMAPS && el_maps[id] && el_maps[id]->below) {
    el_layer *top = el_maps[id];
    el_maps[id] = top->below;
    free(top->tab); free(top);
  }
  return UNIT;
}

/* ------------------------------ key access ------------------------------ */

/* select (map, key): cache it for the accessors below */
unit el_map_key(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0,
                uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0) {
  if (id >= EL_NMAPS) { el_cur_id = -1; return UNIT; }
  (void)el_top(id);                       /* ensure the base layer exists */
  el_cur_key[0] = a2; el_cur_key[1] = a1; el_cur_key[2] = a0;
  el_cur_key[3] = s3; el_cur_key[4] = s2; el_cur_key[5] = s1; el_cur_key[6] = s0;
  el_cur_hash = el_hash(el_cur_key);
  el_cur_id = (int)id;
  return UNIT;
}

/* store the 256-bit value (v3 = most significant) at the selected key (TOP layer) */
unit el_map_store(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) {
  if (el_cur_id >= 0) {
    uint64_t v[4] = { v3, v2, v1, v0 };
    el_put(el_maps[el_cur_id], el_cur_key, el_cur_hash, v);
  }
  return UNIT;
}

/* value word i (3 = most significant .. 0) at the selected key; 0 if absent.
 * Walks the overlay stack: the most recent write wins. */
uint64_t el_map_word(uint64_t i) {
  if (i > 3) return 0;
  el_entry *e = el_walk();
  return e ? e->val[3 - i] : 0;
}

/* value word i at the selected key in the BASE layer only (EIP-2200 original:
 * the bottom layer is exactly the state at the start of the current tx). */
uint64_t el_map_base_word(uint64_t i) {
  if (el_cur_id < 0 || i > 3) return 0;
  el_layer *l = el_maps[el_cur_id];
  while (l->below) l = l->below;
  uint32_t j = el_find(l, el_cur_key, el_cur_hash);
  return l->tab[j].used ? l->tab[j].val[3 - i] : 0;
}

/* 1 if the selected key is present in ANY layer (distinguishes an explicit 0
 * from absent -- the stateless-witness fail-closed test). */
uint64_t el_map_present(const unit u) {
  (void)u;
  return el_walk() ? 1 : 0;
}

/* ----------------------------- address sweeps --------------------------- */

/* 1 if any key with address (a2,a1,a0) exists in any layer of map id */
uint64_t el_map_has_addr(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0) {
  if (id >= EL_NMAPS) return 0;
  for (el_layer *l = el_maps[id]; l; l = l->below)
    for (uint32_t i = 0; i < l->cap; i++) {
      el_entry *e = &l->tab[i];
      if (e->used && e->key[0] == a2 && e->key[1] == a1 && e->key[2] == a0) return 1;
    }
  return 0;
}

/* physically remove every key with address (a2,a1,a0) from every layer of map
 * id (EIP-6780 account deletion; runs at tx end when only the base is live).
 * Open addressing requires re-inserting the probe-chain remainder. */
unit el_map_wipe_addr(uint64_t id, uint64_t a2, uint64_t a1, uint64_t a0) {
  if (id >= EL_NMAPS) return UNIT;
  for (el_layer *l = el_maps[id]; l; l = l->below) {
    el_layer fresh = { 0 };
    fresh.cap = l->cap;
    fresh.tab = (el_entry *)calloc(fresh.cap, sizeof(el_entry));
    if (!fresh.tab) return UNIT;
    for (uint32_t i = 0; i < l->cap; i++) {
      el_entry *e = &l->tab[i];
      if (e->used && !(e->key[0] == a2 && e->key[1] == a1 && e->key[2] == a0))
        el_put(&fresh, e->key, el_hash(e->key), e->val);
    }
    free(l->tab);
    l->tab = fresh.tab; l->cap = fresh.cap; l->n = fresh.n;
  }
  return UNIT;
}

/* ------------------------------ enumeration ----------------------------- */

/* flatten map id (all layers merged, top wins) into the iteration buffer and
 * return the entry count. Used by dump_state / the state-root walk (cold). */
uint64_t el_map_count(uint64_t id) {
  free(el_it); el_it = NULL; el_it_n = 0;
  if (id >= EL_NMAPS || !el_maps[id]) return 0;
  /* merge bottom -> top into a scratch layer so upper layers overwrite */
  el_layer scratch = { 0 };
  scratch.cap = EL_INIT_CAP;
  scratch.tab = (el_entry *)calloc(scratch.cap, sizeof(el_entry));
  if (!scratch.tab) return 0;
  /* collect layers bottom-first */
  el_layer *stack[64]; int depth = 0;
  for (el_layer *l = el_maps[id]; l && depth < 64; l = l->below) stack[depth++] = l;
  for (int d = depth - 1; d >= 0; d--) {
    el_layer *l = stack[d];
    for (uint32_t i = 0; i < l->cap; i++)
      if (l->tab[i].used)
        el_put(&scratch, l->tab[i].key, el_hash(l->tab[i].key), l->tab[i].val);
  }
  el_it = (el_entry *)calloc(scratch.n ? scratch.n : 1, sizeof(el_entry));
  for (uint32_t i = 0; i < scratch.cap; i++)
    if (scratch.tab[i].used) el_it[el_it_n++] = scratch.tab[i];
  free(scratch.tab);
  return el_it_n;
}

/* select flattened entry j for key/value word reads */
unit el_map_sel(uint64_t j) { el_it_sel = (uint32_t)j; return UNIT; }

/* key word i of the selected flattened entry: 0..2 = addr (a2,a1,a0), 3..6 = slot (s3..s0) */
uint64_t el_map_it_key(uint64_t i) {
  if (el_it_sel >= el_it_n || i > 6) return 0;
  return el_it[el_it_sel].key[i];
}

/* value word i (3 = most significant .. 0) of the selected flattened entry */
uint64_t el_map_it_val(uint64_t i) {
  if (el_it_sel >= el_it_n || i > 3) return 0;
  return el_it[el_it_sel].val[3 - i];
}
