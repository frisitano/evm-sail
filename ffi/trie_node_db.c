/* C-backed witness node-db for the evm-sail stateless reader.
 *
 * keccak(node) -> (offset, len) span into the SSZ witness buffer. See
 * trie_node_db.h for the rationale (kills the O(N^2) Sail assoc-list scan). The
 * keccak is computed Sail-side (over the materialized node) and the resulting
 * span is recorded here; lookups are O(1)-amortized open addressing. Mirrors the
 * conventions of the other C hash tables: keys/values cross the FFI as whole
 * lbits values, FNV-1a over the key words, power-of-two capacity. Internal key
 * arrays are little-endian word order (index 0 = least significant). */
#include "trie_node_db.h"
#include "lbits_convert.h"
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

/* cached result of the last nodedb_sel hit */
static uint64_t nd_sel_off;
static uint64_t nd_sel_len;

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
  nd_sel_off = 0;
  nd_sel_len = 0;
  return UNIT;
}

unit nodedb_insert(const lbits kh, uint64_t off, uint64_t len) {
  uint64_t k[4];
  lbits_to_le_words4(k, kh);
  if (!nd_tab)
    nodedb_reset(UNIT);
  nd_put(k, off, len);
  return UNIT;
}

/* 1 if keccak key is present (and caches its span), 0 otherwise */
bool nodedb_sel(const lbits kh) {
  uint64_t k[4];
  lbits_to_le_words4(k, kh);
  if (!nd_tab)
    return 0;
  uint32_t m = nd_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (nd_tab[i].used) {
    if (memcmp(nd_tab[i].key, k, 32) == 0) {
      nd_sel_off = nd_tab[i].off;
      nd_sel_len = nd_tab[i].len;
      return 1;
    }
    i = (i + 1) & m;
  }
  return 0;
}

uint64_t nodedb_sel_off(const unit u) {
  (void)u;
  return nd_sel_off;
}

uint64_t nodedb_sel_len(const unit u) {
  (void)u;
  return nd_sel_len;
}

/* ===========================================================================
 * Witness account-leaf store: keccak(addr) -> (offset, len) span of the account
 * leaf node in the witness buffer (see trie_node_db.h). Recorded during the state
 * re-root walk; a stateless account read looks the span up here and decodes the
 * leaf lazily. Same open-addressing convention as the node-db (keccak key, span
 * value), separate globals.
 * =========================================================================== */
#define AD_INIT_CAP 1024u /* power of two */

typedef struct {
  uint64_t key[4]; /* keccak(addr): k0 (low) .. k3 (high) */
  uint64_t off;
  uint64_t len;
  uint8_t used;
} ad_entry;

static ad_entry *ad_tab = NULL;
static uint32_t ad_cap = 0;
static uint32_t ad_n = 0;
static uint64_t ad_sel_off;
static uint64_t ad_sel_len;

static void ad_grow(void);

static void ad_put(const uint64_t *k, uint64_t off, uint64_t len) {
  if ((ad_n + 1) * 4 >= ad_cap * 3)
    ad_grow();
  uint32_t m = ad_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (ad_tab[i].used) {
    if (memcmp(ad_tab[i].key, k, 32) == 0) /* same account: keep first span */
      return;
    i = (i + 1) & m;
  }
  memcpy(ad_tab[i].key, k, 32);
  ad_tab[i].off = off;
  ad_tab[i].len = len;
  ad_tab[i].used = 1;
  ad_n++;
}

static void ad_grow(void) {
  uint32_t oc = ad_cap;
  ad_entry *ot = ad_tab;
  ad_cap = oc ? oc * 2 : AD_INIT_CAP;
  ad_tab = (ad_entry *)calloc(ad_cap, sizeof(ad_entry));
  ad_n = 0;
  for (uint32_t i = 0; i < oc; i++)
    if (ot[i].used)
      ad_put(ot[i].key, ot[i].off, ot[i].len);
  free(ot);
}

unit acctdb_reset(const unit u) {
  (void)u;
  free(ad_tab);
  ad_tab = (ad_entry *)calloc(AD_INIT_CAP, sizeof(ad_entry));
  ad_cap = AD_INIT_CAP;
  ad_n = 0;
  ad_sel_off = 0;
  ad_sel_len = 0;
  return UNIT;
}

unit acctdb_insert(const lbits kh, uint64_t off, uint64_t len) {
  uint64_t k[4];
  lbits_to_le_words4(k, kh);
  if (!ad_tab)
    acctdb_reset(UNIT);
  ad_put(k, off, len);
  return UNIT;
}

bool acctdb_sel(const lbits kh) {
  uint64_t k[4];
  lbits_to_le_words4(k, kh);
  if (!ad_tab)
    return 0;
  uint32_t m = ad_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (ad_tab[i].used) {
    if (memcmp(ad_tab[i].key, k, 32) == 0) {
      ad_sel_off = ad_tab[i].off;
      ad_sel_len = ad_tab[i].len;
      return 1;
    }
    i = (i + 1) & m;
  }
  return 0;
}

uint64_t acctdb_sel_off(const unit u) {
  (void)u;
  return ad_sel_off;
}
uint64_t acctdb_sel_len(const unit u) {
  (void)u;
  return ad_sel_len;
}

/* iteration (storage-harvest pass): acctdb_at(i) caches row i for the accessors */
static uint64_t ad_it_key[4];
static uint64_t ad_it_off, ad_it_len;
uint64_t acctdb_count(const unit u) {
  (void)u;
  return ad_n;
}
unit acctdb_at(uint64_t idx) {
  uint32_t seen = 0;
  if (!ad_tab)
    return UNIT;
  for (uint32_t i = 0; i < ad_cap; i++)
    if (ad_tab[i].used) {
      if (seen == (uint32_t)idx) {
        for (int w = 0; w < 4; w++)
          ad_it_key[w] = ad_tab[i].key[w];
        ad_it_off = ad_tab[i].off;
        ad_it_len = ad_tab[i].len;
        return UNIT;
      }
      seen++;
    }
  return UNIT;
}
void acctdb_at_key(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, ad_it_key);
}
uint64_t acctdb_at_off(const unit u) {
  (void)u;
  return ad_it_off;
}
uint64_t acctdb_at_len(const unit u) {
  (void)u;
  return ad_it_len;
}

/* ===========================================================================
 * Witness storage-slot store: (keccak(addr), keccak(slot)) -> 256-bit value.
 * The harvested, authenticated pre-state storage (every account's storage trie
 * is re-rooted against its storage_root during the witness pass, and its
 * leaves land here). Stateless SLOAD/k_orig look a slot up here on a working-set
 * miss (O(1), no trie walk), and the post-state-root pass enumerates it to
 * include witnessed-but-unmodified slots. Same open-addressing convention as the
 * node-db; supports point lookup (slotdb_sel) and iteration (slotdb_count/_at).
 * =========================================================================== */
#define SD_INIT_CAP 1024u /* power of two */

typedef struct {
  uint64_t key[8]; /* keccak(addr) k3..k0 (0..3), keccak(slot) k3..k0 (4..7) */
  uint64_t val[4]; /* slot value v3..v0 (v3 most significant) */
  uint8_t existed; /* authenticated pre-state slot existed */
  uint8_t used;
} sd_entry;

static sd_entry *sd_tab = NULL;
static uint32_t sd_cap = 0;
static uint32_t sd_n = 0;
static uint64_t sd_sel_val[4];                              /* last sd_sel hit */
static uint64_t sd_sel_existed;                             /* last sd_sel provenance */
static uint64_t sd_it_acct[4], sd_it_slot[4], sd_it_val[4]; /* last sd_at row */
static uint64_t sd_it_existed;                              /* last sd_at provenance */

/* Accounts whose storage harvest preserved at least one blinded child. Their
 * slotdb rows are only a working-set slice, not a complete live storage map, so
 * post-state delete handling must use the preserving overlay instead of a full
 * rebuild from harvested leaves. */
#define IC_INIT_CAP 64u /* power of two */

typedef struct {
  uint64_t key[4]; /* keccak(addr): k0 (low) .. k3 (high) */
  uint8_t used;
} ic_entry;

static ic_entry *ic_tab = NULL;
static uint32_t ic_cap = 0;
static uint32_t ic_n = 0;

static uint64_t sd_hash(const uint64_t *k) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (int i = 0; i < 8; i++) {
    uint64_t w = k[i];
    for (int b = 0; b < 8; b++) {
      h ^= (w >> (8 * b)) & 0xff;
      h *= 0x100000001b3ull;
    }
  }
  return h;
}

static void sd_grow(void);
static void ic_grow(void);

static uint8_t sd_val_nonzero(const uint64_t *v) {
  return (v[0] | v[1] | v[2] | v[3]) != 0 ? 1 : 0;
}

static void ic_reset(void) {
  free(ic_tab);
  ic_tab = (ic_entry *)calloc(IC_INIT_CAP, sizeof(ic_entry));
  ic_cap = IC_INIT_CAP;
  ic_n = 0;
}

static void ic_put(const uint64_t *k) {
  if ((ic_n + 1) * 4 >= ic_cap * 3)
    ic_grow();
  uint32_t m = ic_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (ic_tab[i].used) {
    if (memcmp(ic_tab[i].key, k, 32) == 0)
      return;
    i = (i + 1) & m;
  }
  memcpy(ic_tab[i].key, k, 32);
  ic_tab[i].used = 1;
  ic_n++;
}

static void ic_grow(void) {
  uint32_t oc = ic_cap;
  ic_entry *ot = ic_tab;
  ic_cap = oc ? oc * 2 : IC_INIT_CAP;
  ic_tab = (ic_entry *)calloc(ic_cap, sizeof(ic_entry));
  ic_n = 0;
  for (uint32_t i = 0; i < oc; i++)
    if (ot[i].used)
      ic_put(ot[i].key);
  free(ot);
}

unit storage_mark_incomplete(const lbits a) {
  uint64_t k[4];
  lbits_to_le_words4(k, a);
  if (!ic_tab)
    ic_reset();
  ic_put(k);
  return UNIT;
}

bool storage_harvest_complete(const lbits a) {
  uint64_t k[4];
  lbits_to_le_words4(k, a);
  if (!ic_tab)
    return 1;
  uint32_t m = ic_cap - 1;
  uint32_t i = (uint32_t)nd_hash(k) & m;
  while (ic_tab[i].used) {
    if (memcmp(ic_tab[i].key, k, 32) == 0)
      return 0;
    i = (i + 1) & m;
  }
  return 1;
}

static void sd_put(const uint64_t *k, const uint64_t *v) {
  if ((sd_n + 1) * 4 >= sd_cap * 3)
    sd_grow();
  uint32_t m = sd_cap - 1;
  uint32_t i = (uint32_t)sd_hash(k) & m;
  while (sd_tab[i].used) {
    if (memcmp(sd_tab[i].key, k, 64) == 0) { /* same (addr,slot): overwrite */
      memcpy(sd_tab[i].val, v, 32);
      sd_tab[i].existed = sd_val_nonzero(v);
      return;
    }
    i = (i + 1) & m;
  }
  memcpy(sd_tab[i].key, k, 64);
  memcpy(sd_tab[i].val, v, 32);
  sd_tab[i].existed = sd_val_nonzero(v);
  sd_tab[i].used = 1;
  sd_n++;
}

static void sd_grow(void) {
  uint32_t oc = sd_cap;
  sd_entry *ot = sd_tab;
  sd_cap = oc ? oc * 2 : SD_INIT_CAP;
  sd_tab = (sd_entry *)calloc(sd_cap, sizeof(sd_entry));
  sd_n = 0;
  for (uint32_t i = 0; i < oc; i++)
    if (ot[i].used)
      sd_put(ot[i].key, ot[i].val);
  free(ot);
}

unit slotdb_reset(const unit u) {
  (void)u;
  free(sd_tab);
  sd_tab = (sd_entry *)calloc(SD_INIT_CAP, sizeof(sd_entry));
  sd_cap = SD_INIT_CAP;
  sd_n = 0;
  sd_sel_existed = 0;
  sd_it_existed = 0;
  ic_reset();
  return UNIT;
}

unit slotdb_insert(const lbits acct, const lbits slot, const lbits val) {
  uint64_t k[8], v[4];
  lbits_to_le_words4(k, acct);
  lbits_to_le_words4(k + 4, slot);
  lbits_to_le_words4(v, val);
  if (!sd_tab)
    slotdb_reset(UNIT);
  sd_put(k, v);
  return UNIT;
}

/* 1 if (addr,slot) is present (caching its value for slotdb_selval), else 0 */
bool slotdb_sel(const lbits acct, const lbits slot) {
  uint64_t k[8];
  lbits_to_le_words4(k, acct);
  lbits_to_le_words4(k + 4, slot);
  sd_sel_existed = 0;
  if (!sd_tab)
    return 0;
  uint32_t m = sd_cap - 1;
  uint32_t i = (uint32_t)sd_hash(k) & m;
  while (sd_tab[i].used) {
    if (memcmp(sd_tab[i].key, k, 64) == 0) {
      memcpy(sd_sel_val, sd_tab[i].val, 32);
      sd_sel_existed = sd_tab[i].existed;
      return 1;
    }
    i = (i + 1) & m;
  }
  return 0;
}
void slotdb_selval(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, sd_sel_val);
}
bool slotdb_sel_existed(const unit u) {
  (void)u;
  return sd_sel_existed;
}

/* iteration over all entries (post-state-root pass): slotdb_at(i) caches row i */
uint64_t slotdb_count(const unit u) {
  (void)u;
  return sd_n;
}
unit slotdb_at(uint64_t idx) {
  uint32_t seen = 0;
  sd_it_existed = 0;
  if (!sd_tab)
    return UNIT;
  for (uint32_t i = 0; i < sd_cap; i++) {
    if (sd_tab[i].used) {
      if (seen == (uint32_t)idx) {
        for (int w = 0; w < 4; w++) {
          sd_it_acct[w] = sd_tab[i].key[w];
          sd_it_slot[w] = sd_tab[i].key[4 + w];
          sd_it_val[w] = sd_tab[i].val[w];
        }
        sd_it_existed = sd_tab[i].existed;
        return UNIT;
      }
      seen++;
    }
  }
  return UNIT;
}
/* whole 256-bit acct key / slot key / value of the cached row; the internal
 * arrays store w0 (low) at index 0 .. w3 (high) at index 3 (no flip). */
void slotdb_at_acct(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, sd_it_acct);
}
void slotdb_at_slot(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, sd_it_slot);
}
void slotdb_at_val(lbits *rop, const unit u) {
  (void)u;
  le_words4_to_lbits(rop, sd_it_val);
}
bool slotdb_at_existed(const unit u) {
  (void)u;
  return sd_it_existed;
}
