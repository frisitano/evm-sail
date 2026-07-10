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
#include "code_db.h"
#include "lbits_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* secure trie key derivation: keccak(address) for accounts, keccak(slot) for
 * storage. The account/storage hooks are keyed by raw (address[, slot]); this
 * computes the secure key on every call. NOTE: keccak is pure, so a preimage ->
 * hash memo would avoid re-hashing repeated touches of the same address/slot --
 * removed for now to keep the key path stateless; see TODO.md. */
static void secure_keccak(size_t pre_len, const lbits v, uint64_t out[4]) {
  uint8_t buf[32];
  lbits_to_be_bytes(buf, pre_len, v);
  host_keccak256_bytes(out, buf, pre_len);
}

/* h = keccak(address) -- the account secure trie key, BE words */
static void acct_secure_key(const lbits a, uint64_t h[4]) {
  secure_keccak(20, a, h);
}

#define ACCOUNT_INIT_CAP 64u

/* generic 4-word (big-endian order) comparator, shared by the acct_wset overlay
   and the block-access-list accumulator below */
static int compare_u64x4(const uint64_t *a, const uint64_t *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

/* zeroed account fields, returned by the overlay getters on an absent row */
static const uint64_t account_zero_val[4] = {0, 0, 0, 0};

/* ======================================================================== */
/* EIP-7928 block access list accumulator.                                  */
/*                                                                          */
/* All recording is HARVESTED from the write-set overlays at merge time --   */
/* storage_wset_merge emits the tx's storage changes (dirty rows) and reads   */
/* (touched-but-not-changed members); acct_wset_merge emits balance/nonce/    */
/* code changes -- so there is no per-op BAL hook. Records are keyed by        */
/* keccak(address) (matching the overlays); the raw address the BAL sorts/     */
/* encodes by, and the read-only account set, come from eest_account at       */
/* serialize time (= the accounts touched on the witness path). Changes carry   */
/* the ambient block_access_index (bal_set_index: 0 pre-exec syscalls, i+1 for  */
/* user tx i, N+1 post). Reads survive revert (overlay membership persists).    */
/* bal_recompute_hash returns keccak(rlp(bal)) for block_access_list_hash.      */
/* ======================================================================== */

typedef struct { void *d; uint32_t n, cap, esz; } bal_vec;
static void *bal_vec_push(bal_vec *v) {
  if (v->cap < v->n + 1) {
    uint32_t nc = v->cap ? v->cap * 2 : 16;
    v->d = realloc(v->d, (size_t)nc * v->esz);
    v->cap = nc;
  }
  void *p = (char *)v->d + (size_t)v->n * v->esz;
  v->n++;
  return p;
}
static void bal_vec_clear(bal_vec *v) { free(v->d); v->d = NULL; v->n = 0; v->cap = 0; }

typedef struct { uint64_t ah[4]; uint64_t slot[4]; uint32_t idx; uint64_t val[4]; uint32_t seq; } bal_sto_rec;
typedef struct { uint64_t ah[4]; uint64_t slot[4]; } bal_read_rec;
typedef struct { uint64_t ah[4]; uint32_t idx; uint64_t val[4]; uint32_t seq; } bal_bal_rec;    /* balance */
typedef struct { uint64_t ah[4]; uint32_t idx; uint64_t val;    uint32_t seq; } bal_non_rec;    /* nonce   */
typedef struct { uint64_t ah[4]; uint32_t idx; uint64_t chash[4]; uint32_t seq; } bal_cod_rec;  /* code (bytes resolved at serialize) */

static bal_vec bal_sto  = {NULL, 0, 0, sizeof(bal_sto_rec)};
static bal_vec bal_rds  = {NULL, 0, 0, sizeof(bal_read_rec)};
static bal_vec bal_balc = {NULL, 0, 0, sizeof(bal_bal_rec)};
static bal_vec bal_nonc = {NULL, 0, 0, sizeof(bal_non_rec)};
static bal_vec bal_codc = {NULL, 0, 0, sizeof(bal_cod_rec)};
static uint32_t bal_seq = 0;
static uint32_t bal_index = 0;

unit bal_reset(const unit u) {
  (void)u;
  bal_vec_clear(&bal_sto); bal_vec_clear(&bal_rds); bal_vec_clear(&bal_balc);
  bal_vec_clear(&bal_nonc); bal_vec_clear(&bal_codc);
  bal_seq = 0; bal_index = 0;
  return UNIT;
}
unit bal_set_index(uint64_t n) { bal_index = (uint32_t)n; return UNIT; }

/* harvest helpers -- called from the overlay merges (state_db-internal) */
static void bal_add_storage_change(const uint64_t ah[4], const uint64_t slot[4], const uint64_t val[4]) {
  bal_sto_rec *r = (bal_sto_rec *)bal_vec_push(&bal_sto);
  memcpy(r->ah, ah, 32); memcpy(r->slot, slot, 32); memcpy(r->val, val, 32);
  r->idx = bal_index; r->seq = bal_seq++;
}
static void bal_add_storage_read(const uint64_t ah[4], const uint64_t slot[4]) {
  bal_read_rec *r = (bal_read_rec *)bal_vec_push(&bal_rds);
  memcpy(r->ah, ah, 32); memcpy(r->slot, slot, 32);
}
static void bal_add_balance_change(const uint64_t ah[4], const uint64_t val[4]) {
  bal_bal_rec *r = (bal_bal_rec *)bal_vec_push(&bal_balc);
  memcpy(r->ah, ah, 32); memcpy(r->val, val, 32);
  r->idx = bal_index; r->seq = bal_seq++;
}
static void bal_add_nonce_change(const uint64_t ah[4], uint64_t nonce) {
  bal_non_rec *r = (bal_non_rec *)bal_vec_push(&bal_nonc);
  memcpy(r->ah, ah, 32); r->val = nonce;
  r->idx = bal_index; r->seq = bal_seq++;
}
static void bal_add_code_change(const uint64_t ah[4], const uint64_t chash[4]) {
  bal_cod_rec *r = (bal_cod_rec *)bal_vec_push(&bal_codc);
  memcpy(r->ah, ah, 32); memcpy(r->chash, chash, 32);
  r->idx = bal_index; r->seq = bal_seq++;
}

#define STORAGE_INIT_CAP 64u

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

/* the secure storage key of raw (address, slot): (keccak(a), keccak(s)) */
static void storage_secure_key(const lbits a, const lbits s,
                               uint64_t slot[4], uint64_t ah[4], uint64_t sh[4]) {
  lbits_to_be_words4(slot, s);
  secure_keccak(20, a, ah);
  secure_keccak(32, s, sh);
}

static const uint64_t storage_zero_val[4] = {0, 0, 0, 0};

/* ======================================================================== */
/* WRITE-SET STORAGE OVERLAY (stage 1: replaces the layered eest_storage)     */
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
  uint8_t  was_read;     /* 1: this slot was SLOADed (EIP-7928 storage_reads) */
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
    if (!storage_wset_dirty(e)) {
      /* touched but not net-changed this tx: an EIP-7928 read iff it was SLOADed
         (SLOAD opcode, or an SSTORE's current-value read). A net-zero SSTORE is
         a read in EELS (get_storage recorded; no net change to exclude it). */
      if (e->was_read) {
        bal_add_storage_read(e->acct_hash, e->slot);
        /* cache the resolved base into the block so a later tx reads it from the
           working set instead of re-walking the witness (once per key per block).
           Never downgrade an existing block write; just mark it read. */
        storage_wset_row *b = storage_wset_get(&storage_wset_block, e->acct_hash, e->slot_hash);
        if (!b) {
          b = storage_wset_intern(&storage_wset_block, e->acct_hash, e->slot_hash, e->slot);
          if (b) {
            memcpy(b->current, e->current, sizeof(b->current));
            memcpy(b->original, e->original, sizeof(b->original));
            b->was_read = 1;
          }
        } else {
          b->was_read = 1;
        }
      }
      continue;
    }
    bal_add_storage_change(e->acct_hash, e->slot, e->current);   /* EIP-7928 storage change */
    storage_wset_row *b = storage_wset_get(&storage_wset_block, e->acct_hash, e->slot_hash);
    int fresh = (b == NULL);   /* slot not yet in the block working set */
    if (fresh) b = storage_wset_intern(&storage_wset_block, e->acct_hash, e->slot_hash, e->slot);
    if (!b) continue;
    if (fresh) memcpy(b->original, e->original, sizeof(b->original));  /* freeze pre-state once */
    b->written = 1;
    b->was_read |= e->was_read;
    memcpy(b->current, e->current, sizeof(b->current));
  }
  storage_wset_table_reset(&storage_wset_tx);
  storage_wset_iter_invalidate();
  return UNIT;
}

/* --- reads: per-layer row probe -----------------------------------------
   The layer-precedence semantics (which layer wins a lookup, and what the
   EIP-2200 tx-start original is) live in SAIL (host/state.sail
   k_sload / k_sload_orig over option(StorageEntry)); C only
   answers point queries against one layer's row table. A row is a cached READ
   (was_read: current == original == the resolved base) or WRITTEN (current =
   the write, original = the frozen tx-start value); `written` covers rows
   that were read first and written later. storage_secure_key memoizes the
   slot keccak, so the tx + block probe pair costs one keccak + two cheap
   hashtable gets. Returns 0 = absent, 1 = read, 2 = written; the value words
   land in cur/orig (untouched when absent). */
uint64_t storage_row_probe(uint64_t layer, const lbits a, const lbits s,
                           lbits *cur, lbits *orig) {
  uint64_t slot[4], ah[4], sh[4];
  storage_secure_key(a, s, slot, ah, sh);
  storage_wset_table *t = layer == 0 ? &storage_wset_tx : &storage_wset_block;
  storage_wset_row *e = storage_wset_get(t, ah, sh);
  if (!e || !(e->written || e->was_read)) return 0;
  be_words4_to_lbits(cur, e->current);
  be_words4_to_lbits(orig, e->original);
  return e->written ? 2 : 1;
}

/* --- writes / warm ------------------------------------------------------ */

/* cache a resolved base value on the tx row and mark it read. Called by k_sload
   ONLY on a true working-set miss (no written or cached-read value anywhere), so
   the value is the pre-state base (stateless_storage). was_read = the EIP-7928
   read record AND the "value resolved" flag; current==original==base so the row
   is not dirty (a pure read). is_warm is left untouched (gas is independent). */
unit storage_tx_cache(const lbits a, const lbits s, const lbits v) {
  uint64_t slot[4], ah[4], sh[4], w[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  storage_wset_row *e = storage_wset_intern(&storage_wset_tx, ah, sh, slot);
  if (!e) return UNIT;
  memcpy(e->current, w, sizeof(e->current));
  memcpy(e->original, w, sizeof(e->original));
  e->was_read = 1;
  return UNIT;
}

/* update `current` on the EXISTING tx row and mark it written. Serves both
   SSTORE on a row the tx already holds (a cached read upgrades to written;
   its original is already the tx-start value) and the JStor revert (the row
   a journaled write created; written=1 is idempotent there). An absent row
   (wiped by the EIP-6780 delete) is a no-op, as the old restore was. */
unit storage_tx_update(const lbits a, const lbits s, const lbits v) {
  uint64_t slot[4], ah[4], sh[4], w[4];
  storage_secure_key(a, s, slot, ah, sh);
  lbits_to_be_words4(w, v);
  storage_wset_row *e = storage_wset_get(&storage_wset_tx, ah, sh);
  if (e) { e->written = 1; memcpy(e->current, w, sizeof(e->current)); }
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
  secure_keccak(20, a, h);
  /* EIP-7928: storage reads survive account deletion (EELS accumulates
     block_state.storage_reads independently of the account's live state). This
     tx's read-only members are about to be dropped with the account, so harvest
     them into the BAL read set now -- a created-and-selfdestructed account still
     reports the slots it SLOADed. Its changes are correctly voided by the wipe. */
  for (uint32_t i = 0; i < storage_wset_tx.n; i++) {
    storage_wset_row *e = &storage_wset_tx.rows[i];
    if (compare_u64x4(e->acct_hash, h) == 0 && e->was_read)
      bal_add_storage_read(e->acct_hash, e->slot);
  }
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
     native  -> UNION(eest_storage, storage_wset_block) with block.current
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

/* --- all block rows for an account (every written/read member, dirty or not).
   The pre-state base now lives in the MPT node-db, not a flat cache, so the
   "union" is just the block overlay rows; a slot's pre-state value that the
   block never touched is not enumerated here (its post value equals pre). Used
   by storage_has_live_acct_hash (EIP-158) and the debug post-state dump. --- */
static swb_snap_row *swb_union_rows = NULL;
static uint32_t swb_union_n = 0, swb_union_cap = 0;
static uint64_t swb_union_memo[4];
static int swb_union_valid = 0;

static void swb_union_build(const uint64_t ak[4]) {
  if (swb_union_valid && compare_words(swb_union_memo, ak, 4) == 0) return;
  swb_union_n = 0;
  uint32_t bs, be;
  storage_wset_block_acct_range(ak, &bs, &be);
  for (uint32_t bi = bs; bi < be; bi++) {
    swb_snap_push(&swb_union_rows, &swb_union_n, &swb_union_cap,
                  storage_wset_block.rows[bi].slot, storage_wset_block.rows[bi].current);
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
/* the authenticated pre-state leaf exist) and written. eest_account below   */
/* stays the RESOLVER backing (native seed + witness read-cache); a miss      */
/* here asks it, then stateless_account. storage_root is NOT mutated by       */
/* account writes -- it is the pre-state anchor, and the post-state root is    */
/* derived at compute_root. dirty == written && current != original.          */
/* ======================================================================== */

typedef struct {
  uint64_t hkey[4];
  uint64_t cur_nonce;  uint64_t cur_bal[4];  uint64_t cur_sroot[4];  uint64_t cur_chash[4];
  uint64_t orig_nonce; uint64_t orig_bal[4]; uint64_t orig_sroot[4]; uint64_t orig_chash[4];
  uint8_t raw_addr[20];  /* EIP-7928 BAL account key/order (set on seed/first write) */
  uint8_t base_exists;   /* authenticated pre-state leaf existed */
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
  /* changed vs the frozen tx-start original. A read member is seeded cur==orig
     and only a write can diverge them, so this alone identifies a real change
     (no separate `written` flag needed). */
  return e->cur_nonce != e->orig_nonce ||
         compare_u64x4(e->cur_bal, e->orig_bal) != 0 ||
         compare_u64x4(e->cur_chash, e->orig_chash) != 0 ||
         compare_u64x4(e->cur_sroot, e->orig_sroot) != 0;
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

/* merge the tx overlay into the block base, then clear it. The merge only
   ever updates the block CURRENT; the block ORIGINAL (+ base_exists) is the
   pre-state and is frozen ONCE, when the account first enters the block
   working set (from the tx row, which froze it at cache time from the base
   resolver). A row that already exists here already carries the correct
   frozen original, so the merge must never touch it. */
unit acct_wset_merge(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < acct_wset_tx.n; i++) {
    acct_wset_row *e = &acct_wset_tx.rows[i];
    int dirty = acct_wset_dirty(e);
    if (dirty) {
      /* EIP-7928 per-field changes vs this tx's start (sroot is not a BAL field) */
      if (e->cur_nonce != e->orig_nonce) bal_add_nonce_change(e->hkey, e->cur_nonce);
      if (memcmp(e->cur_bal, e->orig_bal, 32) != 0) bal_add_balance_change(e->hkey, e->cur_bal);
      if (memcmp(e->cur_chash, e->orig_chash, 32) != 0) bal_add_code_change(e->hkey, e->cur_chash);
    }
    /* EVERY tx row reaches the block layer -- reads included: a read member
       caches the resolved pre-state for later txs AND puts the touched
       account into the BAL account list (bal_recompute enumerates the block
       members). Read rows of an existing block row mirror it exactly (the
       cache copied from it and the block layer is frozen mid-tx). */
    acct_wset_row *b = acct_wset_get(&acct_wset_block, e->hkey);
    int fresh = (b == NULL);   /* account not yet in the block working set */
    if (fresh) b = acct_wset_intern(&acct_wset_block, e->hkey);
    if (!b) continue;
    memcpy(b->raw_addr, e->raw_addr, sizeof(b->raw_addr));  /* preserve BAL key */
    if (fresh) {
      /* new to the block WS: freeze the pre-state original from the tx (which
         froze it from the base resolver). Pre-existing rows keep their own. */
      b->orig_nonce = e->orig_nonce;
      memcpy(b->orig_bal, e->orig_bal, sizeof(b->orig_bal));
      memcpy(b->orig_sroot, e->orig_sroot, sizeof(b->orig_sroot));
      memcpy(b->orig_chash, e->orig_chash, sizeof(b->orig_chash));
      b->base_exists = e->base_exists;
    } else {
      b->base_exists |= e->base_exists;
    }
    if (fresh || dirty) {
      b->cur_nonce = e->cur_nonce;
      memcpy(b->cur_bal, e->cur_bal, sizeof(b->cur_bal));
      memcpy(b->cur_sroot, e->cur_sroot, sizeof(b->cur_sroot));
      memcpy(b->cur_chash, e->cur_chash, sizeof(b->cur_chash));
    }
  }
  acct_wset_table_reset(&acct_wset_tx);
  acct_wset_iter_invalidate();
  return UNIT;
}

/* --- reads (overlay only; a miss => Sail asks eest_account / resolver) --- */
/* per-layer account probe (layer 0 = tx, 1 = block) for the Sail
   option(Account) glue (journal_glue.c): returns presence; the field words
   land in the out params (untouched when absent). The tx-over-block
   precedence lives in Sail (account_lookup). */
uint64_t acct_row_probe(uint64_t layer, const lbits a, uint64_t *nonce,
                        lbits *bal, lbits *sroot, lbits *chash) {
  uint64_t h[4];
  acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_get(layer == 0 ? &acct_wset_tx : &acct_wset_block, h);
  if (!e) return 0;
  *nonce = e->cur_nonce;
  le_words4_to_lbits(bal, e->cur_bal);
  le_words4_to_lbits(sroot, e->cur_sroot);
  le_words4_to_lbits(chash, e->cur_chash);
  return 1;
}

/* --- writes / restore / wipe --- */
/* update the current account on the EXISTING tx row (store_account; the JAcct
   undo). load_account always runs before a store and caches every resolution
   into the tx layer (acct_tx_cache), so the row -- with orig + base_exists
   already frozen at the tx-start account -- is guaranteed live. An absent row
   (wiped by the EIP-6780 delete) is a no-op. */
unit acct_tx_update(const lbits a, uint64_t nonce,
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

/* cache a resolved account as a tx-layer READ member (cur == orig = the
   resolution, un-journaled so a frame revert keeps the revealed value) --
   load_account's cache-on-every-read, mirroring storage_tx_cache. base_exists:
   on a block-layer hit the block row's flag is inherited; else the caller's
   flag (the witness walk reached a leaf). Only reached when the tx layer
   misses, so the row is always fresh -- bind unconditionally. */
unit acct_tx_cache(const lbits a, uint64_t nonce, const lbits bal,
                   const lbits sroot, const lbits chash, bool base_exists) {
  uint64_t h[4]; acct_secure_key(a, h);
  acct_wset_row *e = acct_wset_intern(&acct_wset_tx, h);
  if (!e) return UNIT;
  lbits_to_be_bytes(e->raw_addr, 20, a);   /* EIP-7928 BAL account key/order */
  uint64_t b4[4], sr[4], ch[4];
  lbits_to_le_words4(b4, bal);
  lbits_to_le_words4(sr, sroot);
  lbits_to_le_words4(ch, chash);
  e->cur_nonce = nonce;  e->orig_nonce = nonce;
  memcpy(e->cur_bal, b4, sizeof(e->cur_bal));    memcpy(e->orig_bal, b4, sizeof(e->orig_bal));
  memcpy(e->cur_sroot, sr, sizeof(e->cur_sroot)); memcpy(e->orig_sroot, sr, sizeof(e->orig_sroot));
  memcpy(e->cur_chash, ch, sizeof(e->cur_chash)); memcpy(e->orig_chash, ch, sizeof(e->orig_chash));
  acct_wset_row *b = acct_wset_get(&acct_wset_block, h);
  e->base_exists = b ? b->base_exists : (base_exists ? 1 : 0);
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

/* ---- compute_root enumeration over acct_wset_block ---------------------
   witness -> DIRTY block rows (account changed vs pre-state); native ->
   UNION(eest_account, acct_wset_block) with the block current overriding the
   cache. Both feed state_updates_eest_account_row (host/state.sail AcctRow). */

typedef struct {
  uint64_t hkey[4];
  uint64_t nonce; uint64_t bal[4]; uint64_t sroot[4]; uint64_t chash[4];
  uint8_t base_exists;
} acct_snap_row;

static void acct_snap_push(acct_snap_row **rows, uint32_t *n, uint32_t *cap,
                           const uint64_t hkey[4], uint64_t nonce, const uint64_t bal[4],
                           const uint64_t sroot[4], const uint64_t chash[4], uint8_t base_exists) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (acct_snap_row *)realloc(*rows, (size_t)nc * sizeof(acct_snap_row));
    *cap = nc;
  }
  acct_snap_row *r = &(*rows)[*n];
  memcpy(r->hkey, hkey, 32); r->nonce = nonce;
  memcpy(r->bal, bal, 32); memcpy(r->sroot, sroot, 32); memcpy(r->chash, chash, 32);
  r->base_exists = base_exists;
  (*n)++;
}

/* witness: dirty block rows (current values) */
static acct_snap_row *acct_dirty_rows = NULL;
static uint32_t acct_dirty_n = 0, acct_dirty_cap = 0;
static int acct_dirty_valid = 0;

static void acct_dirty_build(void) {
  if (acct_dirty_valid) return;
  acct_dirty_n = 0;
  /* (1) accounts changed in the account overlay (balance / nonce / code) */
  for (uint32_t i = 0; i < acct_wset_block.n; i++) {
    acct_wset_row *r = &acct_wset_block.rows[i];
    if (!acct_wset_dirty(r)) continue;
    acct_snap_push(&acct_dirty_rows, &acct_dirty_n, &acct_dirty_cap,
                   r->hkey, r->cur_nonce, r->cur_bal, r->cur_sroot, r->cur_chash, r->base_exists);
  }
  /* (2) accounts changed ONLY in storage: walk the distinct account hashes of
     the storage block base (sorted by acct_hash, so a group is contiguous) and
     add any not already emitted as a dirty account. Its account fields come
     from the account overlay if written there, else the resolver cache (SSTORE
     loads the account first, so the cache holds it). The leaf is otherwise the
     pre-state account with a re-derived storage root. */
  uint32_t i = 0;
  while (i < storage_wset_block.n) {
    uint64_t ah[4];
    memcpy(ah, storage_wset_block.rows[i].acct_hash, sizeof(ah));
    acct_wset_row *ab = acct_wset_get(&acct_wset_block, ah);
    if (!(ab && acct_wset_dirty(ab))) {   /* not already emitted in step (1) */
      if (ab) {
        /* a block member (written or a read member); cur holds the account
           fields -- pre-state for a read member, committed for a write. SSTORE
           loads the account first, so storage-touched accounts always have one. */
        acct_snap_push(&acct_dirty_rows, &acct_dirty_n, &acct_dirty_cap,
                       ab->hkey, ab->cur_nonce, ab->cur_bal, ab->cur_sroot, ab->cur_chash, ab->base_exists);
      }
    }
    while (i < storage_wset_block.n && compare_words(storage_wset_block.rows[i].acct_hash, ah, 4) == 0) i++;
  }
  acct_dirty_valid = 1;
}

/* all block rows (every written/read member). The pre-state base lives in the
   MPT node-db, not a flat cache, so the "union" is just the block overlay rows;
   an account untouched by the block is not enumerated (post == pre). */
static acct_snap_row *acct_union_rows = NULL;
static uint32_t acct_union_n = 0, acct_union_cap = 0;
static int acct_union_valid = 0;

static void acct_union_build(void) {
  if (acct_union_valid) return;
  acct_union_n = 0;
  for (uint32_t bi = 0; bi < acct_wset_block.n; bi++) {
    const acct_wset_row *b = &acct_wset_block.rows[bi];
    acct_snap_push(&acct_union_rows, &acct_union_n, &acct_union_cap,
                   b->hkey, b->cur_nonce, b->cur_bal, b->cur_sroot, b->cur_chash, b->base_exists);
  }
  acct_union_valid = 1;
}

uint64_t acct_wset_block_dirty_count(const unit u) { (void)u; acct_dirty_build(); return acct_dirty_n; }
void acct_wset_block_dirty_hkey(lbits *rop, uint64_t i) {
  acct_dirty_build(); be_words4_to_lbits(rop, i < acct_dirty_n ? acct_dirty_rows[i].hkey : account_zero_val);
}
uint64_t acct_wset_block_dirty_nonce(uint64_t i) { acct_dirty_build(); return i < acct_dirty_n ? acct_dirty_rows[i].nonce : 0; }
void acct_wset_block_dirty_bal(lbits *rop, uint64_t i) {
  acct_dirty_build(); le_words4_to_lbits(rop, i < acct_dirty_n ? acct_dirty_rows[i].bal : account_zero_val);
}
void acct_wset_block_dirty_sroot(lbits *rop, uint64_t i) {
  acct_dirty_build(); le_words4_to_lbits(rop, i < acct_dirty_n ? acct_dirty_rows[i].sroot : account_zero_val);
}
void acct_wset_block_dirty_chash(lbits *rop, uint64_t i) {
  acct_dirty_build(); le_words4_to_lbits(rop, i < acct_dirty_n ? acct_dirty_rows[i].chash : account_zero_val);
}
bool acct_wset_block_dirty_base_exists(uint64_t i) { acct_dirty_build(); return i < acct_dirty_n ? acct_dirty_rows[i].base_exists : 0; }

uint64_t acct_wset_union_count(const unit u) { (void)u; acct_union_build(); return acct_union_n; }
void acct_wset_union_hkey(lbits *rop, uint64_t i) {
  acct_union_build(); be_words4_to_lbits(rop, i < acct_union_n ? acct_union_rows[i].hkey : account_zero_val);
}
uint64_t acct_wset_union_nonce(uint64_t i) { acct_union_build(); return i < acct_union_n ? acct_union_rows[i].nonce : 0; }
void acct_wset_union_bal(lbits *rop, uint64_t i) {
  acct_union_build(); le_words4_to_lbits(rop, i < acct_union_n ? acct_union_rows[i].bal : account_zero_val);
}
void acct_wset_union_sroot(lbits *rop, uint64_t i) {
  acct_union_build(); le_words4_to_lbits(rop, i < acct_union_n ? acct_union_rows[i].sroot : account_zero_val);
}
void acct_wset_union_chash(lbits *rop, uint64_t i) {
  acct_union_build(); le_words4_to_lbits(rop, i < acct_union_n ? acct_union_rows[i].chash : account_zero_val);
}
bool acct_wset_union_base_exists(uint64_t i) { acct_union_build(); return i < acct_union_n ? acct_union_rows[i].base_exists : 0; }

static void acct_wset_iter_invalidate(void) {
  acct_dirty_valid = 0;
  acct_union_valid = 0;
}

/* ======================================================================== */
/* EIP-7928 block access list: recompute from execution.                    */
/*                                                                          */
/* We rebuild the BAL that EELS builds during execution                     */
/* (amsterdam/block_access_lists.py) and keccak(rlp(bal)) it; the caller     */
/* feeds that as block_access_list_hash into block_header_hash, so the        */
/* existing block_hash check verifies it (store + hash, no input parse).     */
/*                                                                          */
/* The accumulator is keyed by keccak(address) -- matching the overlays and  */
/* the merge hooks -- plus a keccak->raw-address record populated at touch    */
/* time; the BAL sorts/encodes by RAW address. Reads survive revert (the      */
/* records are append-only, never journaled). Changes are the per-tx net      */
/* diff read from the tx overlay before each merge, tagged with the ambient   */
/* block_access_index (0 pre-exec syscalls, i+1 for user tx i, N+1 post).     */
/* ======================================================================== */

/* ----------------------- BAL serialize + keccak ------------------------- */
/* Emit canonical RLP for the accumulated BAL and keccak it. Accounts come from
   eest_account (the witness touch set) sorted by raw address; per account the
   changes/reads are joined from the accumulator by keccak(address). */

typedef struct { uint8_t *d; size_t n, cap; } bbuf;
static void bb_ensure(bbuf *b, size_t add) {
  if (b->n + add > b->cap) {
    size_t nc = b->cap ? b->cap * 2 : 256;
    while (nc < b->n + add) nc *= 2;
    b->d = (uint8_t *)realloc(b->d, nc);
    b->cap = nc;
  }
}
static void bb_byte(bbuf *b, uint8_t x) { bb_ensure(b, 1); b->d[b->n++] = x; }
static void bb_raw(bbuf *b, const uint8_t *p, size_t n) { if (!n) return; bb_ensure(b, n); memcpy(b->d + b->n, p, n); b->n += n; }
static void bb_free(bbuf *b) { free(b->d); b->d = NULL; b->n = 0; b->cap = 0; }

static void rlp_len_prefix(bbuf *b, size_t len, uint8_t off) {
  if (len < 56) { bb_byte(b, (uint8_t)(off + len)); return; }
  uint8_t lb[8]; int m = 0;
  for (size_t x = len; x; x >>= 8) lb[m++] = (uint8_t)(x & 0xff);
  bb_byte(b, (uint8_t)(off + 55 + m));
  for (int i = 0; i < m; i++) bb_byte(b, lb[m - 1 - i]);
}
static void rlp_str(bbuf *b, const uint8_t *p, size_t n) {
  if (n == 1 && p[0] < 0x80) { bb_byte(b, p[0]); return; }
  rlp_len_prefix(b, n, 0x80);
  bb_raw(b, p, n);
}
static void rlp_uint_be(bbuf *b, const uint8_t *be, size_t n) {   /* min-BE: strip leading zeros */
  size_t i = 0; while (i < n && be[i] == 0) i++;
  rlp_str(b, be + i, n - i);
}
static void rlp_list_of(bbuf *parent, const bbuf *payload) {
  rlp_len_prefix(parent, payload->n, 0xc0);
  bb_raw(parent, payload->d, payload->n);
}
static void be_words_be32(uint8_t out[32], const uint64_t w[4]) {   /* w[0]=MS (storage slot/value) */
  for (int i = 0; i < 4; i++) for (int j = 0; j < 8; j++) out[i * 8 + j] = (uint8_t)((w[i] >> (56 - 8 * j)) & 0xff);
}
static void le_words_be32(uint8_t out[32], const uint64_t w[4]) {   /* w[3]=MS (account balance) */
  for (int i = 0; i < 4; i++) { uint64_t x = w[3 - i]; for (int j = 0; j < 8; j++) out[i * 8 + j] = (uint8_t)((x >> (56 - 8 * j)) & 0xff); }
}
static void rlp_u32(bbuf *b, uint32_t v) { uint8_t be[4] = {(uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v}; rlp_uint_be(b, be, 4); }
static void rlp_u64(bbuf *b, uint64_t v) { uint8_t be[8]; for (int j = 0; j < 8; j++) be[j] = (uint8_t)((v >> (56 - 8 * j)) & 0xff); rlp_uint_be(b, be, 8); }

/* first index in a vec sorted by ah (ah at record offset 0) with ah >= target */
static uint32_t bal_ah_lower(const void *base, size_t esz, uint32_t n, const uint64_t ah[4]) {
  uint32_t lo = 0, hi = n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    const uint64_t *m = (const uint64_t *)((const char *)base + (size_t)mid * esz);
    if (compare_u64x4(m, ah) < 0) lo = mid + 1; else hi = mid;
  }
  return lo;
}

static int bal_sto_cmp(const void *x, const void *y) {
  const bal_sto_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  c = compare_u64x4(a->slot, b->slot); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_read_cmp(const void *x, const void *y) {
  const bal_read_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  return compare_u64x4(a->slot, b->slot);
}
static int bal_bal_cmp(const void *x, const void *y) {
  const bal_bal_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_non_cmp(const void *x, const void *y) {
  const bal_non_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_cod_cmp(const void *x, const void *y) {
  const bal_cod_rec *a = x, *b = y; int c = compare_u64x4(a->ah, b->ah); if (c) return c;
  if (a->idx != b->idx) return a->idx < b->idx ? -1 : 1;
  return a->seq < b->seq ? -1 : (a->seq > b->seq ? 1 : 0);
}
static int bal_acc_cmp(const void *x, const void *y) {   /* accounts by raw 20-byte address */
  const acct_wset_row *const *a = x, *const *b = y;
  return memcmp((*a)->raw_addr, (*b)->raw_addr, 20);
}

/* whether (ah, slot) has a storage change (to exclude it from reads) */
static int bal_slot_changed(const uint64_t ah[4], const uint64_t slot[4]) {
  uint32_t i = bal_ah_lower(bal_sto.d, sizeof(bal_sto_rec), bal_sto.n, ah);
  const bal_sto_rec *r = (const bal_sto_rec *)bal_sto.d;
  for (; i < bal_sto.n && compare_u64x4(r[i].ah, ah) == 0; i++)
    if (compare_u64x4(r[i].slot, slot) == 0) return 1;
  return 0;
}

void bal_recompute_hash(lbits *rop, const unit u) {
  (void)u;
  qsort(bal_sto.d, bal_sto.n, sizeof(bal_sto_rec), bal_sto_cmp);
  qsort(bal_rds.d, bal_rds.n, sizeof(bal_read_rec), bal_read_cmp);
  qsort(bal_balc.d, bal_balc.n, sizeof(bal_bal_rec), bal_bal_cmp);
  qsort(bal_nonc.d, bal_nonc.n, sizeof(bal_non_rec), bal_non_cmp);
  qsort(bal_codc.d, bal_codc.n, sizeof(bal_cod_rec), bal_cod_cmp);

  /* the witness touched-account set = the account working-set block members
     (read members seeded by the base resolver + written members from merges);
     eest_account is native-only and never populated on the witness path. */
  uint32_t na = acct_wset_block.n;
  const acct_wset_row **accs = (const acct_wset_row **)malloc((na ? na : 1) * sizeof(*accs));
  for (uint32_t i = 0; i < na; i++) accs[i] = &acct_wset_block.rows[i];
  qsort(accs, na, sizeof(*accs), bal_acc_cmp);

  const bal_sto_rec *sto = (const bal_sto_rec *)bal_sto.d;
  const bal_read_rec *rds = (const bal_read_rec *)bal_rds.d;
  const bal_bal_rec *balc = (const bal_bal_rec *)bal_balc.d;
  const bal_non_rec *nonc = (const bal_non_rec *)bal_nonc.d;
  const bal_cod_rec *codc = (const bal_cod_rec *)bal_codc.d;

  bbuf top = {NULL, 0, 0};
  for (uint32_t ai = 0; ai < na; ai++) {
    const acct_wset_row *acc = accs[ai];
    const uint64_t *ah = acc->hkey;
    bbuf ap = {NULL, 0, 0};
    { uint8_t a20[20]; memcpy(a20, acc->raw_addr, 20); rlp_str(&ap, a20, 20); }

    /* storage_changes: [ [slot, [ [idx, val], ... ]], ... ] */
    bbuf sc = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(sto, sizeof(*sto), bal_sto.n, ah); i < bal_sto.n && compare_u64x4(sto[i].ah, ah) == 0; ) {
      uint32_t j = i;
      bbuf slot_entry = {NULL, 0, 0};
      { uint8_t sbe[32]; be_words_be32(sbe, sto[i].slot); rlp_uint_be(&slot_entry, sbe, 32); }
      bbuf changes = {NULL, 0, 0};
      while (j < bal_sto.n && compare_u64x4(sto[j].ah, ah) == 0 && compare_u64x4(sto[j].slot, sto[i].slot) == 0) {
        uint32_t k = j;
        while (k + 1 < bal_sto.n && compare_u64x4(sto[k + 1].ah, ah) == 0 &&
               compare_u64x4(sto[k + 1].slot, sto[i].slot) == 0 && sto[k + 1].idx == sto[j].idx) k++;
        bbuf pair = {NULL, 0, 0};
        rlp_u32(&pair, sto[k].idx);
        { uint8_t vbe[32]; be_words_be32(vbe, sto[k].val); rlp_uint_be(&pair, vbe, 32); }
        rlp_list_of(&changes, &pair); bb_free(&pair);
        j = k + 1;
      }
      rlp_list_of(&slot_entry, &changes); bb_free(&changes);
      rlp_list_of(&sc, &slot_entry); bb_free(&slot_entry);
      i = j;
    }
    rlp_list_of(&ap, &sc); bb_free(&sc);

    /* storage_reads: distinct touched slots not present in storage_changes */
    bbuf sr = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(rds, sizeof(*rds), bal_rds.n, ah); i < bal_rds.n && compare_u64x4(rds[i].ah, ah) == 0; ) {
      uint32_t j = i;
      while (j + 1 < bal_rds.n && compare_u64x4(rds[j + 1].ah, ah) == 0 && compare_u64x4(rds[j + 1].slot, rds[i].slot) == 0) j++;
      if (!bal_slot_changed(ah, rds[i].slot)) { uint8_t sbe[32]; be_words_be32(sbe, rds[i].slot); rlp_uint_be(&sr, sbe, 32); }
      i = j + 1;
    }
    rlp_list_of(&ap, &sr); bb_free(&sr);

    /* balance_changes: [ [idx, balance], ... ] (final per idx) */
    bbuf bc = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(balc, sizeof(*balc), bal_balc.n, ah); i < bal_balc.n && compare_u64x4(balc[i].ah, ah) == 0; ) {
      uint32_t k = i;
      while (k + 1 < bal_balc.n && compare_u64x4(balc[k + 1].ah, ah) == 0 && balc[k + 1].idx == balc[i].idx) k++;
      bbuf pair = {NULL, 0, 0};
      rlp_u32(&pair, balc[k].idx);
      { uint8_t vbe[32]; le_words_be32(vbe, balc[k].val); rlp_uint_be(&pair, vbe, 32); }
      rlp_list_of(&bc, &pair); bb_free(&pair);
      i = k + 1;
    }
    rlp_list_of(&ap, &bc); bb_free(&bc);

    /* nonce_changes: [ [idx, nonce], ... ] */
    bbuf ncb = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(nonc, sizeof(*nonc), bal_nonc.n, ah); i < bal_nonc.n && compare_u64x4(nonc[i].ah, ah) == 0; ) {
      uint32_t k = i;
      while (k + 1 < bal_nonc.n && compare_u64x4(nonc[k + 1].ah, ah) == 0 && nonc[k + 1].idx == nonc[i].idx) k++;
      bbuf pair = {NULL, 0, 0};
      rlp_u32(&pair, nonc[k].idx);
      rlp_u64(&pair, nonc[k].val);
      rlp_list_of(&ncb, &pair); bb_free(&pair);
      i = k + 1;
    }
    rlp_list_of(&ap, &ncb); bb_free(&ncb);

    /* code_changes: [ [idx, code_bytes], ... ] */
    bbuf cc = {NULL, 0, 0};
    for (uint32_t i = bal_ah_lower(codc, sizeof(*codc), bal_codc.n, ah); i < bal_codc.n && compare_u64x4(codc[i].ah, ah) == 0; ) {
      uint32_t k = i;
      while (k + 1 < bal_codc.n && compare_u64x4(codc[k + 1].ah, ah) == 0 && codc[k + 1].idx == codc[i].idx) k++;
      bbuf pair = {NULL, 0, 0};
      rlp_u32(&pair, codc[k].idx);
      { uint64_t key_be[4]; for (int q = 0; q < 4; q++) key_be[q] = codc[k].chash[3 - q];  /* le-words -> be-words */
        uint64_t clen = 0; const uint8_t *cp = code_db_code_by_words(key_be, &clen);
        rlp_str(&pair, cp ? cp : (const uint8_t *)"", (size_t)clen); }
      rlp_list_of(&cc, &pair); bb_free(&pair);
      i = k + 1;
    }
    rlp_list_of(&ap, &cc); bb_free(&cc);

    rlp_list_of(&top, &ap); bb_free(&ap);
  }

  bbuf out = {NULL, 0, 0};
  rlp_list_of(&out, &top);
  host_keccak256_lbits(rop, out.d ? out.d : (const uint8_t *)"", out.n);
  bb_free(&out); bb_free(&top);
  free(accs);
}
