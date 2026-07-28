/* C-backed account and persistent-storage state database for evm-sail.
 *
 * Accounts have one physical table keyed by raw address. Each row carries the
 * cumulative block value, an optional transaction projection, and BAL
 * metadata. Persistent storage follows the same design in one table keyed by
 * raw (address, slot); its generation is transaction lifecycle metadata, not
 * part of row identity.
 *
 * Each row caches the secure trie digests derived when witness material is
 * first loaded. Those digests order state-root traversal but never identify a
 * table row. */
#include "state_db.h"
#include "hash_bytes.h"
#include "value_convert.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Secure trie key derivation is deliberately separate from table identity.
 * These helpers are called only when a raw-key row first gains authenticated
 * witness material (or for a defensive write-before-read fallback). */
static void secure_keccak_address(sail_fixed_bytes_20 value, sail_fixed_bytes_32 *out) {
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(value.bytes, sizeof(value.bytes), &digest) == ZKVM_EOK)
    memcpy(out->bytes, digest.data, sizeof(out->bytes));
  else
    memset(out, 0, sizeof(*out));
}

static void secure_keccak_word(sail_u256 value, sail_fixed_bytes_32 *out) {
  uint8_t bytes[32];
  zkvm_keccak256_hash digest = {{0}};
  sail_word_to_be_bytes(bytes, value);
  if (zkvm_keccak256(bytes, sizeof(bytes), &digest) == ZKVM_EOK)
    memcpy(out->bytes, digest.data, sizeof(out->bytes));
  else
    memset(out, 0, sizeof(*out));
}

void acct_secure_key(sail_fixed_bytes_20 address, sail_fixed_bytes_32 *address_hash) {
  secure_keccak_address(address, address_hash);
}

void storage_secure_key(sail_u256 slot, sail_fixed_bytes_32 *slot_hash) {
  secure_keccak_word(slot, slot_hash);
}

#define ACCOUNT_INIT_CAP 64u

/* Generic 4-word (big-endian order) comparator shared by state and BAL code. */
static int compare_u64x4(const uint64_t *a, const uint64_t *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

/* zeroed account fields, returned by the overlay getters on an absent row */
static const uint64_t account_zero_val[4] = {0, 0, 0, 0};
static const sail_fixed_bytes_32 account_empty_code_hash = {{
    0xc5, 0xd2, 0x46, 0x01, 0x86, 0xf7, 0x23, 0x3c,
    0x92, 0x7e, 0x7d, 0xb2, 0xdc, 0xc7, 0x03, 0xc0,
    0xe5, 0x00, 0xb6, 0x53, 0xca, 0x82, 0x27, 0x3b,
    0x7b, 0xfa, 0xd8, 0x04, 0x5d, 0x85, 0xa4, 0x70,
}};

/* ======================================================================== */
/* EIP-7928 block access list metadata.                                     */
/*                                                                          */
/* BAL identity is shared with the cumulative account/storage maps below.    */
/* Rows own only epoch-local flags and compact history links; change values   */
/* live in append-only arenas and therefore never duplicate address/slot     */
/* keys. Same-index updates overwrite the history tail, matching the former   */
/* sorted-map semantics without an insertion-time memmove.                   */
/* ======================================================================== */

#define BAL_NO_HISTORY UINT32_MAX

typedef struct {
  uint32_t index;
  uint32_t next;
  uint64_t value[4];
} bal_word_history;

typedef struct {
  uint32_t index;
  uint32_t next;
  uint64_t value;
} bal_nonce_history;

typedef struct {
  uint32_t index;
  uint32_t next;
  sail_fixed_bytes_32 value;
} bal_code_history;

static bal_word_history *bal_storage_history = NULL;
static uint32_t bal_storage_history_n = 0, bal_storage_history_cap = 0;
static bal_word_history *bal_balance_history = NULL;
static uint32_t bal_balance_history_n = 0, bal_balance_history_cap = 0;
static bal_nonce_history *bal_nonce_histories = NULL;
static uint32_t bal_nonce_history_n = 0, bal_nonce_history_cap = 0;
static bal_code_history *bal_code_histories = NULL;
static uint32_t bal_code_history_n = 0, bal_code_history_cap = 0;
static uint64_t bal_epoch = 1;
static int bal_prepared = 0;

enum bal_stream_phase {
  BAL_STREAM_ACCOUNT = 0,
  BAL_STREAM_STORAGE_CHANGE = 1,
  BAL_STREAM_STORAGE_READ = 2,
  BAL_STREAM_BALANCE = 3,
  BAL_STREAM_NONCE = 4,
  BAL_STREAM_CODE = 5,
  BAL_STREAM_ACCOUNT_END = 6,
  BAL_STREAM_EMPTY = 7
};

static uint32_t *bal_account_order = NULL;
static uint32_t bal_account_order_n = 0, bal_account_order_cap = 0;
static uint32_t *bal_storage_order = NULL;
static uint32_t bal_storage_order_n = 0, bal_storage_order_cap = 0;
static uint32_t bal_account_cursor = 0;
static uint32_t bal_storage_cursor = 0;
static uint32_t bal_active_account_index = BAL_NO_HISTORY;
static uint32_t bal_active_storage_history = BAL_NO_HISTORY;
static uint32_t bal_active_field_history = BAL_NO_HISTORY;
static enum bal_stream_phase bal_stream_phase = BAL_STREAM_ACCOUNT;

#define STORAGE_INIT_CAP 64u

static int compare_words(const uint64_t *a, const uint64_t *b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static int storage_key_cmp(const uint8_t aa[20], const uint64_t as[4],
                           uint64_t ag, const uint8_t ba[20],
                           const uint64_t bs[4], uint64_t bg) {
  (void)ag;
  (void)bg;
  int c = memcmp(aa, ba, 20);
  if (c) return c;
  return compare_words(as, bs, 4);
}

static void storage_raw_key(sail_fixed_bytes_20 a, sail_u256 s,
                            uint8_t address[20], uint64_t slot[4]) {
  evmsail_address_to_be_bytes(address, a);
  sail_word_to_be_words4(slot, s);
}

static bool acct_cached_secure_key(sail_fixed_bytes_20 a, sail_fixed_bytes_32 *out);
static void acct_block_update_mark_raw(const uint8_t address[20]);

static const uint64_t storage_zero_val[4] = {0, 0, 0, 0};

/* ======================================================================== */
/* PERSISTENT STORAGE STATE                                                  */
/*                                                                          */
/* One table owns the cumulative read cache/update value and the optional    */
/* transaction write projection. Storage-clear generations and their         */
/* rollback history are host-private: Sail sees only the active value and its */
/* transaction-start original. A miss in both semantic layers asks the       */
/* authenticated base in Sail.                                               */
/* ======================================================================== */

typedef struct {
  uint8_t  raw_addr[20];
  uint64_t slot[4];
  uint64_t generation;
  sail_fixed_bytes_32 acct_hash;
  sail_fixed_bytes_32 slot_hash;
  /* Cumulative block value and frozen pre-block original. */
  uint64_t current[4];
  uint64_t original[4];
  /* Live transaction overlay and its transaction-start original. */
  uint64_t tx_current[4];
  uint64_t tx_original[4];
  uint64_t bal_epoch;
  uint32_t bal_change_head;
  uint32_t bal_change_tail;
  uint8_t written;
  uint8_t state_valid;
  uint8_t secure_keys_valid;
  uint8_t bal_read;
  uint8_t tx_active;
  uint8_t tx_listed;
} storage_state_row;

typedef struct {
  storage_state_row *rows;
  uint32_t n, cap;
  uint32_t *buckets;
  uint32_t bucket_cap;
  uint8_t indexed;
} storage_state_table;

/* The aliases name semantic projections; both resolve to this single table. */
static storage_state_table storage_table = {NULL, 0, 0, NULL, 0, 1};
#define storage_tx_table storage_table
#define storage_block_table storage_table
static uint32_t *storage_tx_rows = NULL;
static uint32_t storage_tx_rows_n = 0;
static uint32_t storage_tx_rows_cap = 0;
static uint8_t storage_block_iter_key[20];
static uint32_t *storage_block_iter_order = NULL;
static uint32_t storage_block_iter_order_cap = 0;
static uint32_t storage_block_iter_position = 0;
static uint32_t storage_block_iter_end = 0;
static bool storage_block_iter_active = false;
static uint32_t *storage_block_raw_order = NULL;
static uint32_t storage_block_raw_order_cap = 0;
static uint8_t storage_block_raw_order_valid = 0;
static uint32_t storage_tx_pop_cursor = 0; /* k_tx_merge drain position */
enum { STORAGE_BLOCK_GENERATION = 0, STORAGE_INITIAL_GENERATION = 1 };
static uint64_t storage_next_generation = STORAGE_INITIAL_GENERATION;

typedef struct {
  uint8_t raw_addr[20];
  sail_fixed_bytes_32 acct_hash;
  uint64_t active_generation;
} storage_epoch_row;

typedef struct {
  storage_epoch_row *rows;
  uint32_t n, cap;
} storage_epoch_table;

enum { STORAGE_UNDO_WRITE = 1, STORAGE_UNDO_CLEAR = 2 };

typedef struct {
  uint8_t tag;
  uint8_t prior_present;
  uint8_t raw_addr[20];
  uint64_t slot[4];
  uint64_t generation;
  uint64_t prior_current[4];
  uint64_t prior_original[4];
} storage_undo_entry;

static storage_epoch_table storage_epochs = {NULL, 0, 0};
static storage_undo_entry *storage_undo = NULL;
static uint32_t storage_undo_n = 0, storage_undo_cap = 0;

/* drop the per-account compute_root snapshots when the block base changes
   (defined with the snapshot builders below) */
static void storage_dump_invalidate(void);
static void storage_block_account_range(const uint8_t address[20],
                                        uint32_t *start, uint32_t *end);

static uint64_t point_hash_mix(uint64_t value) {
  value ^= value >> 30;
  value *= UINT64_C(0xbf58476d1ce4e5b9);
  value ^= value >> 27;
  value *= UINT64_C(0x94d049bb133111eb);
  return value ^ (value >> 31);
}

static uint64_t point_hash_rotate_left(uint64_t value, uint32_t shift) {
  return (value << shift) | (value >> (64 - shift));
}

static uint64_t acct_point_hash_seed(const uint8_t address[20]) {
  uint64_t first, second;
  uint32_t tail;
  memcpy(&first, address, sizeof(first));
  memcpy(&second, address + sizeof(first), sizeof(second));
  memcpy(&tail, address + sizeof(first) + sizeof(second), sizeof(tail));
  return first ^ point_hash_rotate_left(second, 23) ^
         ((uint64_t)tail * UINT64_C(0x9e3779b97f4a7c15));
}

static uint64_t storage_point_hash(const uint8_t address[20],
                                   const uint64_t slot[4],
                                   uint64_t generation) {
  (void)generation;
  const uint64_t folded =
      acct_point_hash_seed(address) ^
      point_hash_rotate_left(slot[0], 7) ^
      point_hash_rotate_left(slot[1], 19) ^
      point_hash_rotate_left(slot[2], 37) ^
      point_hash_rotate_left(slot[3], 53);
  return point_hash_mix(folded);
}

static void storage_table_reindex(storage_state_table *table,
                                  uint32_t bucket_cap) {
  uint32_t *buckets = calloc(bucket_cap, sizeof(*buckets));
  if (!buckets) abort();
  for (uint32_t i = 0; i < table->n; i++) {
    const storage_state_row *row = &table->rows[i];
    uint32_t bucket =
        (uint32_t)storage_point_hash(row->raw_addr, row->slot,
                                     row->generation) &
        (bucket_cap - 1);
    while (buckets[bucket] != 0)
      bucket = (bucket + 1) & (bucket_cap - 1);
    buckets[bucket] = i + 1;
  }
  free(table->buckets);
  table->buckets = buckets;
  table->bucket_cap = bucket_cap;
}

static void storage_table_prepare_index(storage_state_table *table,
                                        uint32_t row_count) {
  if (!table->indexed) return;
  uint32_t bucket_cap = table->bucket_cap ? table->bucket_cap : 128;
  while ((uint64_t)row_count * 10 >= (uint64_t)bucket_cap * 7)
    bucket_cap *= 2;
  if (bucket_cap != table->bucket_cap)
    storage_table_reindex(table, bucket_cap);
}

static uint32_t storage_table_find(const storage_state_table *t,
                                  const uint8_t address[20],
                                  const uint64_t slot[4],
                                  uint64_t generation,
                                  int *found) {
  uint32_t lo = 0, hi = t->n;
  if (t->indexed) {
    if (t->bucket_cap == 0) {
      *found = 0;
      return t->n;
    }
    uint32_t bucket =
        (uint32_t)storage_point_hash(address, slot, generation) &
        (t->bucket_cap - 1);
    while (t->buckets[bucket] != 0) {
      uint32_t index = t->buckets[bucket] - 1;
      const storage_state_row *row = &t->rows[index];
      if (storage_key_cmp(row->raw_addr, row->slot, row->generation,
                          address, slot, generation) == 0) {
        *found = 1;
        return index;
      }
      bucket = (bucket + 1) & (t->bucket_cap - 1);
    }
    *found = 0;
    return t->n;
  }
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    int c = storage_key_cmp(t->rows[mid].raw_addr, t->rows[mid].slot,
                            t->rows[mid].generation, address, slot,
                            generation);
    if (c < 0) lo = mid + 1; else hi = mid;
  }
  *found = (lo < t->n &&
            storage_key_cmp(t->rows[lo].raw_addr, t->rows[lo].slot,
                            t->rows[lo].generation, address, slot,
                            generation) == 0);
  return lo;
}

static storage_state_row *storage_table_get(storage_state_table *t,
                                             const uint8_t address[20],
                                             const uint64_t slot[4],
                                             uint64_t generation) {
  int f = 0;
  uint32_t i = storage_table_find(t, address, slot, generation, &f);
  return f ? &t->rows[i] : NULL;
}

/* Insert if absent. Rows are append-stable and indexed by open addressing.
 * Ordering is constructed only as a temporary view for protocol serialization;
 * transaction execution and merge always consume insertion order. */
static storage_state_row *storage_table_intern(storage_state_table *t,
                                                const uint8_t address[20],
                                                const uint64_t slot[4],
                                                uint64_t generation,
                                                const sail_fixed_bytes_32 *ah,
                                                const sail_fixed_bytes_32 *sh) {
  int f = 0;
  uint32_t i = storage_table_find(t, address, slot, generation, &f);
  if (f) return &t->rows[i];
  storage_table_prepare_index(t, t->n + 1);
  if (t->cap < t->n + 1) {
    uint32_t nc = t->cap ? t->cap * 2 : STORAGE_INIT_CAP;
    while (nc < t->n + 1) nc *= 2;
    storage_state_row *nr = (storage_state_row *)realloc(t->rows, (size_t)nc * sizeof(storage_state_row));
    if (!nr) return NULL;
    t->rows = nr; t->cap = nc;
  }
  if (!t->indexed && i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(storage_state_row));
  if (t->indexed) i = t->n;
  storage_state_row *e = &t->rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->raw_addr, address, sizeof(e->raw_addr));
  memcpy(e->slot, slot, sizeof(e->slot));
  e->generation = STORAGE_INITIAL_GENERATION;
  e->acct_hash = *ah;
  e->slot_hash = *sh;
  e->bal_change_head = BAL_NO_HISTORY;
  e->bal_change_tail = BAL_NO_HISTORY;
  t->n++;
  if (t->indexed) {
    uint32_t bucket =
        (uint32_t)storage_point_hash(address, slot, generation) &
        (t->bucket_cap - 1);
    while (t->buckets[bucket] != 0)
      bucket = (bucket + 1) & (t->bucket_cap - 1);
    t->buckets[bucket] = i + 1;
    storage_block_raw_order_valid = 0;
  }
  return e;
}

static void storage_table_reset(storage_state_table *t) {
  const uint8_t indexed = t->indexed;
  free(t->rows);
  free(t->buckets);
  t->rows = NULL; t->n = 0; t->cap = 0;
  t->buckets = NULL; t->bucket_cap = 0;
  t->indexed = indexed;
}

static uint32_t storage_epoch_find(const uint8_t address[20], int *found) {
  uint32_t lo = 0, hi = storage_epochs.n;
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (memcmp(storage_epochs.rows[mid].raw_addr, address, 20) < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  *found = lo < storage_epochs.n &&
           memcmp(storage_epochs.rows[lo].raw_addr, address, 20) == 0;
  return lo;
}

static storage_epoch_row *storage_epoch_get(const uint8_t address[20]) {
  int found = 0;
  uint32_t i = storage_epoch_find(address, &found);
  return found ? &storage_epochs.rows[i] : NULL;
}

static storage_epoch_row *storage_epoch_intern(const uint8_t address[20],
                                               const sail_fixed_bytes_32 *ah) {
  int found = 0;
  uint32_t i = storage_epoch_find(address, &found);
  if (found) return &storage_epochs.rows[i];
  if (storage_epochs.cap < storage_epochs.n + 1) {
    uint32_t cap = storage_epochs.cap ? storage_epochs.cap * 2 : 16;
    storage_epoch_row *rows = realloc(storage_epochs.rows,
                                      (size_t)cap * sizeof(*rows));
    if (!rows) return NULL;
    storage_epochs.rows = rows;
    storage_epochs.cap = cap;
  }
  if (i < storage_epochs.n)
    memmove(&storage_epochs.rows[i + 1], &storage_epochs.rows[i],
            (size_t)(storage_epochs.n - i) * sizeof(storage_epoch_row));
  storage_epoch_row *e = &storage_epochs.rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->raw_addr, address, sizeof(e->raw_addr));
  e->acct_hash = *ah;
  e->active_generation = STORAGE_INITIAL_GENERATION;
  storage_epochs.n++;
  return e;
}

static uint64_t storage_active_generation(const uint8_t address[20]) {
  storage_epoch_row *e = storage_epoch_get(address);
  return e ? e->active_generation : STORAGE_INITIAL_GENERATION;
}

static storage_undo_entry *storage_undo_push(uint8_t tag) {
  if (storage_undo_cap < storage_undo_n + 1) {
    uint32_t cap = storage_undo_cap ? storage_undo_cap * 2 : 64;
    storage_undo_entry *entries = realloc(storage_undo,
                                          (size_t)cap * sizeof(*entries));
    if (!entries) return NULL;
    storage_undo = entries;
    storage_undo_cap = cap;
  }
  storage_undo_entry *entry = &storage_undo[storage_undo_n++];
  memset(entry, 0, sizeof(*entry));
  entry->tag = tag;
  return entry;
}

static void storage_tx_list(storage_state_row *row) {
  if (row->tx_listed) return;
  if (storage_tx_rows_cap < storage_tx_rows_n + 1) {
    uint32_t cap = storage_tx_rows_cap ? storage_tx_rows_cap * 2 : 64;
    uint32_t *rows =
        realloc(storage_tx_rows, (size_t)cap * sizeof(*rows));
    if (!rows) abort();
    storage_tx_rows = rows;
    storage_tx_rows_cap = cap;
  }
  storage_tx_rows[storage_tx_rows_n++] =
      (uint32_t)(row - storage_table.rows);
  row->tx_listed = 1;
}

/* --- lifecycle -------------------------------------------------------- */

/* full world wipe (between independent blocks/fixtures) */
/* whole-overlay wipe (both layers): the HARNESS world reset
   (test_utils.c evmsail_clear_memory); no Sail caller. */
unit storage_db_reset(const unit u) {
  (void)u;
  storage_tx_pop_cursor = 0;
  storage_next_generation = STORAGE_INITIAL_GENERATION;
  storage_table_reset(&storage_table);
  free(storage_tx_rows);
  storage_tx_rows = NULL;
  storage_tx_rows_n = storage_tx_rows_cap = 0;
  free(storage_epochs.rows);
  storage_epochs.rows = NULL;
  storage_epochs.n = storage_epochs.cap = 0;
  free(storage_undo);
  storage_undo = NULL;
  storage_undo_n = storage_undo_cap = 0;
  free(storage_block_iter_order);
  storage_block_iter_order = NULL;
  storage_block_iter_order_cap = 0;
  free(storage_block_raw_order);
  storage_block_raw_order = NULL;
  storage_block_raw_order_cap = 0;
  storage_block_raw_order_valid = 0;
  storage_block_iter_position = storage_block_iter_end = 0;
  storage_block_iter_active = false;
  storage_dump_invalidate();
  return UNIT;
}

/* Transaction scratch reset: retain the allocation for the next transaction. */
unit storage_tx_reset(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < storage_tx_rows_n; i++) {
    storage_state_row *row = &storage_table.rows[storage_tx_rows[i]];
    row->tx_active = 0;
    row->tx_listed = 0;
  }
  storage_tx_rows_n = 0;
  storage_tx_pop_cursor = 0;
  storage_next_generation = STORAGE_INITIAL_GENERATION;
  storage_epochs.n = 0;
  storage_undo_n = 0;
  return UNIT;
}

uint64_t storage_tx_checkpoint(const unit u) {
  (void)u;
  return storage_undo_n;
}

unit storage_tx_revert(uint64_t checkpoint) {
  if (checkpoint > storage_undo_n) abort();
  while (storage_undo_n > checkpoint) {
    storage_undo_entry *undo = &storage_undo[--storage_undo_n];
    if (undo->tag == STORAGE_UNDO_WRITE) {
      storage_state_row *row = storage_table_get(
          &storage_table, undo->raw_addr, undo->slot, undo->generation);
      if (!row) abort();
      if (undo->prior_present) {
        row->tx_active = 1;
        row->generation = undo->generation;
        memcpy(row->tx_current, undo->prior_current,
               sizeof(row->tx_current));
        memcpy(row->tx_original, undo->prior_original,
               sizeof(row->tx_original));
      } else {
        row->tx_active = 0;
      }
    } else if (undo->tag == STORAGE_UNDO_CLEAR) {
      sail_fixed_bytes_32 ah;
      if (!acct_cached_secure_key(be_bytes_to_sail_address(undo->raw_addr),
                                  &ah))
        secure_keccak_address(be_bytes_to_sail_address(undo->raw_addr), &ah);
      storage_epoch_row *epoch = storage_epoch_intern(undo->raw_addr, &ah);
      if (epoch) epoch->active_generation = undo->generation;
    } else {
      abort();
    }
  }
  return UNIT;
}

unit storage_tx_clear(sail_fixed_bytes_20 a) {
  uint8_t address[20];
  sail_fixed_bytes_32 ah;
  evmsail_address_to_be_bytes(address, a);
  if (!acct_cached_secure_key(a, &ah)) secure_keccak_address(a, &ah);
  storage_epoch_row *epoch = storage_epoch_intern(address, &ah);
  if (!epoch) return UNIT;
  storage_undo_entry *undo = storage_undo_push(STORAGE_UNDO_CLEAR);
  if (!undo) return UNIT;
  memcpy(undo->raw_addr, address, sizeof(undo->raw_addr));
  undo->generation = epoch->active_generation;
  if (storage_next_generation == UINT64_MAX) abort();
  epoch->active_generation = ++storage_next_generation;
  return UNIT;
}

/* Drain active transaction writes. Obsolete generations remain private
   rollback history and are skipped. */
uint64_t storage_tx_pop_probe(sail_fixed_bytes_20 *addr, sail_u256 *slot,
                              sail_u256 *curr, sail_u256 *orig) {
  while (storage_tx_pop_cursor < storage_tx_rows_n) {
    const storage_state_row *row =
        &storage_table.rows[storage_tx_rows[storage_tx_pop_cursor++]];
    if (!row->tx_active ||
        row->generation != storage_active_generation(row->raw_addr))
      continue;
    *addr = be_bytes_to_sail_address(row->raw_addr);
    *slot = be_words4_to_sail_word(row->slot);
    *curr = be_words4_to_sail_word(row->tx_current);
    *orig = be_words4_to_sail_word(row->tx_original);
    return 1;
  }
  return 0;
}

/* block-layer propagation hooks (pure mechanism; Sail chose the action).
   Point identity is raw (address, slot). Cached secure keys are copied from
   the transaction row and are never recomputed on a normal merge. */
/* net change: a fresh block row freezes orig as the pre-state; curr lands */
unit storage_block_put_raw(sail_fixed_bytes_20 a, sail_u256 s_, sail_u256 curr,
                           sail_u256 orig) {
  uint8_t address[20];
  uint64_t slot[4], c4[4], o4[4];
  sail_fixed_bytes_32 ah, sh;
  storage_raw_key(a, s_, address, slot);
  const uint64_t generation = storage_active_generation(address);
  storage_state_row *b =
      storage_table_get(&storage_table, address, slot, generation);
  if (b && b->secure_keys_valid) {
    ah = b->acct_hash;
    sh = b->slot_hash;
  } else {
    if (!acct_cached_secure_key(a, &ah)) secure_keccak_address(a, &ah);
    secure_keccak_word(s_, &sh);
  }
  int fresh = (b == NULL || !b->state_valid);
  if (!b)
    b = storage_table_intern(&storage_table, address, slot, generation, &ah,
                             &sh);
  if (!b) return UNIT;
  if (!b->secure_keys_valid) {
    b->acct_hash = ah;
    b->slot_hash = sh;
    b->secure_keys_valid = 1;
  }
  sail_word_to_be_words4(c4, curr);
  if (fresh) {
    sail_word_to_be_words4(o4, orig);
    memcpy(b->original, o4, sizeof(b->original));
  }
  memcpy(b->current, c4, sizeof(b->current));
  b->written = 1;
  b->state_valid = 1;
  acct_block_update_mark_raw(address);
  storage_dump_invalidate();
  return UNIT;
}

/* read member: fresh binds curr == orig = value; existing is only marked */
unit storage_block_cache_raw(sail_fixed_bytes_20 a, sail_u256 s_, sail_fixed_bytes_32 slot_hash,
                             sail_u256 v) {
  uint8_t address[20];
  uint64_t slot[4], v4[4];
  sail_fixed_bytes_32 ah;
  storage_raw_key(a, s_, address, slot);
  if (!acct_cached_secure_key(a, &ah)) secure_keccak_address(a, &ah);
  storage_state_row *b = storage_table_get(
      &storage_block_table, address, slot, STORAGE_BLOCK_GENERATION);
  if (!b) {
    b = storage_table_intern(
        &storage_block_table, address, slot, STORAGE_BLOCK_GENERATION, &ah,
        &slot_hash);
  }
  if (b && !b->state_valid) {
    b->acct_hash = ah;
    b->slot_hash = slot_hash;
    b->secure_keys_valid = 1;
    sail_word_to_be_words4(v4, v);
    memcpy(b->current, v4, sizeof(b->current));
    memcpy(b->original, v4, sizeof(b->original));
    b->state_valid = 1;
  }
  storage_dump_invalidate();
  return UNIT;
}


/* A hit is an actual cached row in the requested semantic layer. A fresh
   transaction storage generation is intentionally not reported as a hit:
   k_sload must record the first slot read in the BAL before resolving that
   separately-observed clear generation to zero. */
uint64_t storage_row_probe(uint64_t layer, sail_fixed_bytes_20 a, sail_u256 s,
                           sail_u256 *cur, sail_u256 *orig) {
  uint8_t address[20];
  uint64_t slot[4];
  storage_raw_key(a, s, address, slot);
  if (layer != 0) {
    storage_state_row *e = storage_table_get(
        &storage_block_table, address, slot, STORAGE_BLOCK_GENERATION);
    if (!e || !e->state_valid) return 0;
    *cur = be_words4_to_sail_word(e->current);
    *orig = be_words4_to_sail_word(e->original);
    return 1;
  }

  const uint64_t generation = storage_active_generation(address);
  storage_state_row *row = storage_table_get(
      &storage_tx_table, address, slot, generation);
  if (row && row->tx_active && row->generation == generation) {
    *cur = be_words4_to_sail_word(row->tx_current);
    *orig = be_words4_to_sail_word(row->tx_original);
    return 1;
  }
  if (generation != STORAGE_INITIAL_GENERATION) return 2;
  return 0;
}

/* Write the active generation. Sail supplies the transaction original from
   the preceding semantic SLOAD. */
unit storage_tx_update_raw(sail_fixed_bytes_20 a, sail_u256 s, sail_u256 v,
                           sail_u256 orig) {
  uint8_t address[20];
  uint64_t slot[4], w[4], o[4];
  sail_fixed_bytes_32 ah, sh;
  storage_raw_key(a, s, address, slot);
  sail_word_to_be_words4(w, v);
  sail_word_to_be_words4(o, orig);
  const uint64_t generation = storage_active_generation(address);
  storage_state_row *e =
      storage_table_get(&storage_tx_table, address, slot, generation);
  const int prior_present = e && e->tx_active;
  const int fresh_generation =
      !prior_present || e->generation != generation;
  if (!e || !e->secure_keys_valid) {
    if (!acct_cached_secure_key(a, &ah)) secure_keccak_address(a, &ah);
    secure_keccak_word(s, &sh);
  }
  storage_undo_entry *undo = storage_undo_push(STORAGE_UNDO_WRITE);
  if (!undo) return UNIT;
  memcpy(undo->raw_addr, address, sizeof(undo->raw_addr));
  memcpy(undo->slot, slot, sizeof(undo->slot));
  undo->generation = prior_present ? e->generation : generation;
  undo->prior_present = prior_present;
  if (prior_present) {
    memcpy(undo->prior_current, e->tx_current,
           sizeof(undo->prior_current));
    memcpy(undo->prior_original, e->tx_original,
           sizeof(undo->prior_original));
  }
  if (!e) {
    e = storage_table_intern(&storage_tx_table, address, slot, generation,
                             &ah, &sh);
    if (!e) {
      storage_undo_n--;
      return UNIT;
    }
  }
  if (!e->secure_keys_valid) {
    e->acct_hash = ah;
    e->slot_hash = sh;
    e->secure_keys_valid = 1;
  }
  if (fresh_generation)
    memcpy(e->tx_original, o, sizeof(e->tx_original));
  memcpy(e->tx_current, w, sizeof(e->tx_current));
  e->generation = generation;
  e->tx_active = 1;
  storage_tx_list(e);
  return UNIT;
}

/* Clear the generation-free block storage for an account. Transaction
   generations are retained until transaction reset for checkpoint reverts. */
unit storage_block_clear(sail_fixed_bytes_20 a) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  uint32_t start, end;
  storage_block_account_range(address, &start, &end);
  for (uint32_t i = start; i < end; i++) {
    storage_state_row *row =
        &storage_block_table.rows[storage_block_raw_order[i]];
    row->state_valid = 0;
    row->written = 0;
  }
  storage_dump_invalidate();
  return UNIT;
}

/* ---- cumulative storage enumeration -------------------------------------
   Block rows are stable arena entries. A lazy raw-key order supports grouped
   account traversal; state-root traversal then orders that group by each
   row's cached secure slot key. */

typedef struct { uint64_t slot[4]; uint64_t val[4]; } storage_dump_entry;

static int storage_block_raw_order_cmp(const void *a, const void *b) {
  const storage_state_row *left =
      &storage_block_table.rows[*(const uint32_t *)a];
  const storage_state_row *right =
      &storage_block_table.rows[*(const uint32_t *)b];
  return storage_key_cmp(left->raw_addr, left->slot, left->generation,
                         right->raw_addr, right->slot, right->generation);
}

static void storage_block_prepare_raw_order(void) {
  if (storage_block_raw_order_valid) return;
  if (storage_block_raw_order_cap < storage_block_table.n) {
    uint32_t cap =
        storage_block_raw_order_cap ? storage_block_raw_order_cap : 64;
    while (cap < storage_block_table.n) cap *= 2;
    uint32_t *order =
        realloc(storage_block_raw_order, (size_t)cap * sizeof(*order));
    if (!order) abort();
    storage_block_raw_order = order;
    storage_block_raw_order_cap = cap;
  }
  for (uint32_t i = 0; i < storage_block_table.n; i++)
    storage_block_raw_order[i] = i;
  if (storage_block_table.n > 1)
    qsort(storage_block_raw_order, storage_block_table.n,
          sizeof(*storage_block_raw_order), storage_block_raw_order_cmp);
  storage_block_raw_order_valid = 1;
}

/* [start,end) positions in raw order with raw address == address. */
static void storage_block_account_range(const uint8_t address[20],
                                        uint32_t *start, uint32_t *end) {
  storage_block_prepare_raw_order();
  uint32_t lo = 0, hi = storage_block_table.n;
  while (lo < hi) {
    const uint32_t mid = lo + ((hi - lo) >> 1);
    const storage_state_row *row =
        &storage_block_table.rows[storage_block_raw_order[mid]];
    if (memcmp(row->raw_addr, address, 20) < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  uint32_t finish = lo;
  while (finish < storage_block_table.n) {
    const storage_state_row *row =
        &storage_block_table.rows[storage_block_raw_order[finish]];
    if (memcmp(row->raw_addr, address, 20) != 0) break;
    finish++;
  }
  *start = lo;
  *end = finish;
}

/* EELS account_has_storage: a nonempty write map in either the surviving
   transaction generation or the cumulative block overlay counts as storage.
   The authenticated pre-state root is checked in Sail. */
bool storage_has_writes(sail_fixed_bytes_20 a) {
  uint8_t address[20];
  uint32_t start, end;
  evmsail_address_to_be_bytes(address, a);

  const uint64_t generation = storage_active_generation(address);
  for (uint32_t i = 0; i < storage_tx_rows_n; i++) {
    const storage_state_row *row =
        &storage_table.rows[storage_tx_rows[i]];
    if (memcmp(row->raw_addr, address, sizeof(row->raw_addr)) == 0 &&
        row->tx_active &&
        row->generation == generation &&
        compare_words(row->tx_current, storage_zero_val, 4) != 0)
      return true;
  }

  if (generation != STORAGE_INITIAL_GENERATION) return false;
  storage_block_account_range(address, &start, &end);
  for (uint32_t i = start; i < end; i++) {
    const storage_state_row *row =
        &storage_block_table.rows[storage_block_raw_order[i]];
    if (row->state_valid &&
        compare_words(row->current,
                      storage_zero_val, 4) != 0)
      return true;
  }
  return false;
}

static void storage_dump_push(storage_dump_entry **rows, uint32_t *n, uint32_t *cap,
                          const uint64_t slot[4], const uint64_t val[4]) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (storage_dump_entry *)realloc(*rows, (size_t)nc * sizeof(storage_dump_entry));
    *cap = nc;
  }
  memcpy((*rows)[*n].slot, slot, 32);
  memcpy((*rows)[*n].val, val, 32);
  (*n)++;
}

/* Non-destructive ascending iteration over one account's cumulative storage
   rows. Keeping the cursor private avoids exporting a host-sized index into
   the Sail model while leaving the underlying table available for debug. */
static int storage_secure_order_cmp(const void *a, const void *b) {
  const storage_state_row *aa =
      &storage_block_table.rows[*(const uint32_t *)a];
  const storage_state_row *bb =
      &storage_block_table.rows[*(const uint32_t *)b];
  return evmsail_hash_compare(&aa->slot_hash, &bb->slot_hash);
}

uint32_t storage_block_updates_prepare(sail_fixed_bytes_20 a) {
  uint32_t start, end;
  evmsail_address_to_be_bytes(storage_block_iter_key, a);
  storage_block_account_range(storage_block_iter_key, &start, &end);
  uint32_t count = 0;
  for (uint32_t i = start; i < end; i++)
    if (storage_block_table.rows[storage_block_raw_order[i]].state_valid)
      count++;
  if (storage_block_iter_order_cap < count) {
    uint32_t *order =
        realloc(storage_block_iter_order, (size_t)count * sizeof(*order));
    if (!order && count) abort();
    storage_block_iter_order = order;
    storage_block_iter_order_cap = count;
  }
  uint32_t order_length = 0;
  for (uint32_t i = start; i < end; i++)
    if (storage_block_table.rows[storage_block_raw_order[i]].state_valid)
      storage_block_iter_order[order_length++] = storage_block_raw_order[i];
  if (order_length != count) abort();
  if (count > 1)
    qsort(storage_block_iter_order, count, sizeof(*storage_block_iter_order),
          storage_secure_order_cmp);
  storage_block_iter_end = count;
  return count;
}

uint64_t storage_block_update_probe_at(uint32_t index, sail_u256 *slot,
                                       sail_u256 *curr, sail_u256 *orig,
                                       sail_fixed_bytes_32 *address_hash,
                                       sail_fixed_bytes_32 *slot_hash) {
  if (index >= storage_block_iter_end) return 0;
  const storage_state_row *entry =
      &storage_block_table.rows[storage_block_iter_order[index]];
  *slot = be_words4_to_sail_word(entry->slot);
  *curr = be_words4_to_sail_word(entry->current);
  *orig = be_words4_to_sail_word(entry->original);
  *address_hash = entry->acct_hash;
  *slot_hash = entry->slot_hash;
  return 1;
}

unit storage_block_iter_begin(sail_fixed_bytes_20 a) {
  (void)storage_block_updates_prepare(a);
  storage_block_iter_position = 0;
  storage_block_iter_active = true;
  return UNIT;
}

uint64_t storage_block_iter_next_probe(sail_fixed_bytes_20 a, sail_u256 *slot,
                                       sail_u256 *curr, sail_u256 *orig,
                                       sail_fixed_bytes_32 *address_hash,
                                       sail_fixed_bytes_32 *slot_hash) {
  uint8_t key[20];
  evmsail_address_to_be_bytes(key, a);
  if (!storage_block_iter_active ||
      memcmp(key, storage_block_iter_key, 20) != 0 ||
      storage_block_iter_end <= storage_block_iter_position)
    return 0;
  return storage_block_update_probe_at(storage_block_iter_position++, slot,
                                       curr, orig, address_hash, slot_hash);
}

/* All cumulative block entries for an account, including read-only entries.
   The authenticated base lives in the MPT node DB, so untouched base slots are
   intentionally absent. This materialized view exists only for debug dumps. */
static storage_dump_entry *storage_dump_entries = NULL;
static uint32_t storage_dump_len = 0, storage_dump_cap = 0;
static sail_fixed_bytes_32 storage_dump_account_hash;
static int storage_dump_valid = 0;

static void storage_dump_build(const sail_fixed_bytes_32 *ak) {
  if (storage_dump_valid &&
      evmsail_hash_equal(&storage_dump_account_hash, ak)) return;
  storage_dump_len = 0;
  for (uint32_t bi = 0; bi < storage_block_table.n; bi++) {
    if (!storage_block_table.rows[bi].state_valid ||
        !evmsail_hash_equal(&storage_block_table.rows[bi].acct_hash, ak))
      continue;
    storage_dump_push(&storage_dump_entries, &storage_dump_len, &storage_dump_cap,
                  storage_block_table.rows[bi].slot, storage_block_table.rows[bi].current);
  }
  storage_dump_account_hash = *ak;
  storage_dump_valid = 1;
}

static void storage_dump_invalidate(void) {
  storage_dump_valid = 0;
}

uint64_t storage_dump_count(sail_u256 ak) {
  sail_fixed_bytes_32 key = evmsail_hash_from_sail_word(ak);
  storage_dump_build(&key);
  return storage_dump_len;
}
sail_u256 storage_dump_slot(sail_u256 ak, uint64_t j) {
  sail_fixed_bytes_32 key = evmsail_hash_from_sail_word(ak);
  storage_dump_build(&key);
  return be_words4_to_sail_word(j < storage_dump_len ? storage_dump_entries[j].slot : storage_zero_val);
}
sail_u256 storage_dump_value(sail_u256 ak, uint64_t j) {
  sail_fixed_bytes_32 key = evmsail_hash_from_sail_word(ak);
  storage_dump_build(&key);
  return be_words4_to_sail_word(j < storage_dump_len ? storage_dump_entries[j].val : storage_zero_val);
}

/* ======================================================================== */
/* ACCOUNT STATE                                                              */
/*                                                                            */
/* Append-stable indexed transaction and cumulative block tables, both keyed */
/* by raw address. A row carries the current AND the                           */
/* tx-start/pre-state original                                                */
/* account (nonce, balance, storage_root, code_hash). A miss here means the   */
/* account was never touched; k_aload resolves it from the block layer or     */
/* the witness walk (stateless_account). storage_root is NOT mutated by       */
/* account writes -- it is the pre-state anchor, and the post-state root is    */
/* derived at compute_root. dirty == written && current != original.          */
/* ======================================================================== */

typedef struct {
  uint8_t raw_addr[20];
  sail_fixed_bytes_32 hkey;       /* cached secure trie key, not table identity */
  uint64_t cur_nonce;  uint64_t cur_bal[4];
  sail_fixed_bytes_32 cur_sroot, cur_chash;
  uint8_t cur_exists, cur_storage_cleared, cur_created, cur_selfdestructed;
  uint64_t orig_nonce; uint64_t orig_bal[4];
  sail_fixed_bytes_32 orig_sroot, orig_chash;
  uint8_t orig_exists, orig_storage_cleared, orig_created, orig_selfdestructed;
  /* Transaction-start snapshot for the active overlay. */
  uint64_t tx_orig_nonce; uint64_t tx_orig_bal[4];
  sail_fixed_bytes_32 tx_orig_sroot, tx_orig_chash;
  uint8_t tx_orig_exists, tx_orig_storage_cleared;
  /* Derived traversal cache; never exposed as semantic account state. */
  sail_fixed_bytes_32 post_sroot;
  uint32_t snapshot_cursor;
  uint64_t bal_epoch;
  uint32_t bal_balance_head, bal_balance_tail;
  uint32_t bal_nonce_head, bal_nonce_tail;
  uint32_t bal_code_head, bal_code_tail;
  uint8_t written;
  uint8_t state_valid;
  uint8_t secure_key_valid;
  uint8_t bal_touched;
  uint8_t tx_active;
  uint8_t tx_listed;
  uint8_t update_listed;
} acct_state_row;

typedef struct {
  acct_state_row *rows;
  uint32_t n, cap;
  uint32_t *buckets;
  uint32_t bucket_cap;
  uint8_t indexed;
} acct_state_table;

/* The aliases name semantic projections; both resolve to this single table. */
static acct_state_table acct_table = {NULL, 0, 0, NULL, 0, 1};
#define acct_tx_table acct_table
#define acct_block_table acct_table
static uint32_t *acct_tx_rows = NULL;
static uint32_t acct_tx_rows_n = 0;
static uint32_t acct_tx_rows_cap = 0;
static uint32_t *acct_block_iter_order = NULL;
static uint32_t acct_block_iter_order_cap = 0;
static uint32_t acct_block_iter_end = 0;
static uint32_t *acct_block_update_rows = NULL;
static uint32_t acct_block_update_rows_n = 0;
static uint32_t acct_block_update_rows_cap = 0;
static uint32_t acct_tx_pop_cursor = 0; /* k_tx_merge drain position */

typedef struct {
  uint8_t raw_addr[20];
  uint64_t nonce;
  uint64_t balance[4];
  sail_fixed_bytes_32 storage_root;
  sail_fixed_bytes_32 code_hash;
  uint32_t snapshot_cursor;
  uint8_t prior_present;
  uint8_t exists;
  uint8_t storage_cleared;
  uint8_t created;
  uint8_t selfdestructed;
} acct_undo_entry;

static acct_undo_entry *acct_undo = NULL;
static uint32_t acct_undo_n = 0, acct_undo_cap = 0;
static const uint32_t ACCT_NO_SNAPSHOT = UINT32_MAX;
static uint32_t acct_active_snapshot = UINT32_MAX;
static uint32_t acct_next_snapshot = 0;


/* per-account compute_root snapshots are invalidated when the block base
   changes (defined with the builders below) */
static void acct_dump_invalidate(void);

static uint64_t acct_point_hash(const uint8_t address[20]) {
  return point_hash_mix(acct_point_hash_seed(address));
}

static void acct_table_reindex(acct_state_table *table, uint32_t bucket_cap) {
  uint32_t *buckets = calloc(bucket_cap, sizeof(*buckets));
  if (!buckets) abort();
  for (uint32_t i = 0; i < table->n; i++) {
    uint32_t bucket =
        (uint32_t)acct_point_hash(table->rows[i].raw_addr) &
        (bucket_cap - 1);
    while (buckets[bucket] != 0)
      bucket = (bucket + 1) & (bucket_cap - 1);
    buckets[bucket] = i + 1;
  }
  free(table->buckets);
  table->buckets = buckets;
  table->bucket_cap = bucket_cap;
}

static void acct_table_prepare_index(acct_state_table *table,
                                     uint32_t row_count) {
  if (!table->indexed) return;
  uint32_t bucket_cap = table->bucket_cap ? table->bucket_cap : 128;
  while ((uint64_t)row_count * 10 >= (uint64_t)bucket_cap * 7)
    bucket_cap *= 2;
  if (bucket_cap != table->bucket_cap)
    acct_table_reindex(table, bucket_cap);
}

static uint32_t acct_table_find(const acct_state_table *t,
                                const uint8_t address[20], int *found) {
  uint32_t lo = 0, hi = t->n;
  if (t->indexed) {
    if (t->bucket_cap == 0) {
      *found = 0;
      return t->n;
    }
    uint32_t bucket =
        (uint32_t)acct_point_hash(address) & (t->bucket_cap - 1);
    while (t->buckets[bucket] != 0) {
      const uint32_t index = t->buckets[bucket] - 1;
      if (memcmp(t->rows[index].raw_addr, address, 20) == 0) {
        *found = 1;
        return index;
      }
      bucket = (bucket + 1) & (t->bucket_cap - 1);
    }
    *found = 0;
    return t->n;
  }
  while (lo < hi) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (memcmp(t->rows[mid].raw_addr, address, 20) < 0)
      lo = mid + 1;
    else
      hi = mid;
  }
  *found = lo < t->n &&
           memcmp(t->rows[lo].raw_addr, address, 20) == 0;
  return lo;
}

static acct_state_row *acct_table_get(acct_state_table *t,
                                      const uint8_t address[20]) {
  int f = 0;
  uint32_t i = acct_table_find(t, address, &f);
  return f ? &t->rows[i] : NULL;
}

static void acct_block_update_list(acct_state_row *row) {
  if (row->update_listed) return;
  if (acct_block_update_rows_cap < acct_block_update_rows_n + 1) {
    uint32_t cap =
        acct_block_update_rows_cap ? acct_block_update_rows_cap * 2 : 64;
    uint32_t *rows =
        realloc(acct_block_update_rows, (size_t)cap * sizeof(*rows));
    if (!rows) abort();
    acct_block_update_rows = rows;
    acct_block_update_rows_cap = cap;
  }
  acct_block_update_rows[acct_block_update_rows_n++] =
      (uint32_t)(row - acct_table.rows);
  row->update_listed = 1;
}

static void acct_block_update_mark_raw(const uint8_t address[20]) {
  acct_state_row *row = acct_table_get(&acct_block_table, address);
  if (!row || !row->state_valid) abort();
  acct_block_update_list(row);
}

static bool acct_cached_secure_key(sail_fixed_bytes_20 a, sail_fixed_bytes_32 *out) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  const acct_state_row *row = acct_table_get(&acct_block_table, address);
  if (!row || !row->secure_key_valid) return false;
  *out = row->hkey;
  return true;
}

static acct_state_row *acct_table_intern(acct_state_table *t,
                                         const uint8_t address[20],
                                         const sail_fixed_bytes_32 *h) {
  int f = 0;
  uint32_t i = acct_table_find(t, address, &f);
  if (f) return &t->rows[i];
  acct_table_prepare_index(t, t->n + 1);
  if (t->cap < t->n + 1) {
    uint32_t nc = t->cap ? t->cap * 2 : ACCOUNT_INIT_CAP;
    while (nc < t->n + 1) nc *= 2;
    acct_state_row *nr = (acct_state_row *)realloc(t->rows, (size_t)nc * sizeof(acct_state_row));
    if (!nr) return NULL;
    t->rows = nr; t->cap = nc;
  }
  if (!t->indexed && i < t->n)
    memmove(&t->rows[i + 1], &t->rows[i], (size_t)(t->n - i) * sizeof(acct_state_row));
  if (t->indexed) i = t->n;
  acct_state_row *e = &t->rows[i];
  memset(e, 0, sizeof(*e));
  memcpy(e->raw_addr, address, sizeof(e->raw_addr));
  e->hkey = *h;
  e->snapshot_cursor = ACCT_NO_SNAPSHOT;
  e->bal_balance_head = e->bal_balance_tail = BAL_NO_HISTORY;
  e->bal_nonce_head = e->bal_nonce_tail = BAL_NO_HISTORY;
  e->bal_code_head = e->bal_code_tail = BAL_NO_HISTORY;
  t->n++;
  if (t->indexed) {
    uint32_t bucket =
        (uint32_t)acct_point_hash(address) & (t->bucket_cap - 1);
    while (t->buckets[bucket] != 0)
      bucket = (bucket + 1) & (t->bucket_cap - 1);
    t->buckets[bucket] = i + 1;
  }
  return e;
}

static void acct_table_reset(acct_state_table *t) {
  const uint8_t indexed = t->indexed;
  free(t->rows);
  free(t->buckets);
  t->rows = NULL; t->n = 0; t->cap = 0;
  t->buckets = NULL; t->bucket_cap = 0;
  t->indexed = indexed;
}

static void acct_tx_list(acct_state_row *row) {
  if (row->tx_listed) return;
  if (acct_tx_rows_cap < acct_tx_rows_n + 1) {
    uint32_t cap = acct_tx_rows_cap ? acct_tx_rows_cap * 2 : 64;
    uint32_t *rows = realloc(acct_tx_rows, (size_t)cap * sizeof(*rows));
    if (!rows) abort();
    acct_tx_rows = rows;
    acct_tx_rows_cap = cap;
  }
  acct_tx_rows[acct_tx_rows_n++] = (uint32_t)(row - acct_table.rows);
  row->tx_listed = 1;
}

static acct_state_row *acct_tx_bind_write(sail_fixed_bytes_20 a,
                                          int *fresh) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  acct_state_row *row = acct_table_get(&acct_tx_table, address);
  if (!row || !row->state_valid) return NULL;
  *fresh = !row->tx_active;
  if (!*fresh) return row;
  row->tx_orig_nonce = row->cur_nonce;
  memcpy(row->tx_orig_bal, row->cur_bal, sizeof(row->tx_orig_bal));
  row->tx_orig_sroot = row->cur_sroot;
  row->tx_orig_chash = row->cur_chash;
  row->tx_orig_exists = row->cur_exists;
  row->tx_orig_storage_cleared = row->cur_storage_cleared;
  row->tx_active = 1;
  acct_tx_list(row);
  return row;
}

/* The active undo cursor is the lazy snapshot identity. Each account saves its
   reversible projection at most once at that cursor; successful child frames
   retain their entries because an enclosing frame may still revert them. */
static int acct_snapshot_for_write(acct_state_row *row, int prior_present) {
  if (acct_active_snapshot == ACCT_NO_SNAPSHOT ||
      row->snapshot_cursor == acct_active_snapshot)
    return 1;
  if (acct_undo_cap < acct_undo_n + 1) {
    uint32_t cap = acct_undo_cap ? acct_undo_cap * 2 : 64;
    acct_undo_entry *entries = realloc(acct_undo,
                                       (size_t)cap * sizeof(*entries));
    if (!entries) return 0;
    acct_undo = entries;
    acct_undo_cap = cap;
  }
  acct_undo_entry *undo = &acct_undo[acct_undo_n++];
  memset(undo, 0, sizeof(*undo));
  memcpy(undo->raw_addr, row->raw_addr, sizeof(undo->raw_addr));
  undo->prior_present = prior_present;
  undo->nonce = row->cur_nonce;
  memcpy(undo->balance, row->cur_bal, sizeof(undo->balance));
  undo->storage_root = row->cur_sroot;
  undo->code_hash = row->cur_chash;
  undo->snapshot_cursor = row->snapshot_cursor;
  undo->exists = row->cur_exists;
  undo->storage_cleared = row->cur_storage_cleared;
  undo->created = row->cur_created;
  undo->selfdestructed = row->cur_selfdestructed;
  row->snapshot_cursor = acct_active_snapshot;
  return 1;
}

/* --- lifecycle --- */
/* whole-overlay wipe (both layers): the HARNESS world reset; no Sail caller. */
unit acct_db_reset(const unit u) {
  (void)u;
  acct_tx_pop_cursor = 0;
  acct_table_reset(&acct_table);
  free(acct_tx_rows);
  acct_tx_rows = NULL;
  acct_tx_rows_n = acct_tx_rows_cap = 0;
  free(acct_block_iter_order);
  acct_block_iter_order = NULL;
  acct_block_iter_order_cap = 0;
  acct_block_iter_end = 0;
  free(acct_block_update_rows);
  acct_block_update_rows = NULL;
  acct_block_update_rows_n = acct_block_update_rows_cap = 0;
  free(acct_undo);
  acct_undo = NULL;
  acct_undo_n = acct_undo_cap = 0;
  acct_active_snapshot = ACCT_NO_SNAPSHOT;
  acct_next_snapshot = 0;
  acct_dump_invalidate();
  return UNIT;
}

unit acct_tx_reset(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < acct_tx_rows_n; i++) {
    acct_state_row *row = &acct_table.rows[acct_tx_rows[i]];
    if (row->tx_active) {
      row->cur_nonce = row->tx_orig_nonce;
      memcpy(row->cur_bal, row->tx_orig_bal, sizeof(row->cur_bal));
      row->cur_sroot = row->tx_orig_sroot;
      row->cur_chash = row->tx_orig_chash;
      row->cur_exists = row->tx_orig_exists;
      row->cur_storage_cleared = row->tx_orig_storage_cleared;
      row->cur_created = 0;
      row->cur_selfdestructed = 0;
      row->tx_active = 0;
    }
    row->tx_listed = 0;
    row->snapshot_cursor = ACCT_NO_SNAPSHOT;
  }
  acct_tx_rows_n = 0;
  acct_tx_pop_cursor = 0;
  acct_undo_n = 0;
  acct_active_snapshot = ACCT_NO_SNAPSHOT;
  acct_next_snapshot = 0;
  return UNIT;
}

uint64_t acct_tx_checkpoint(const unit u) {
  (void)u;
  uint32_t prior = acct_active_snapshot;
  uint32_t cursor = acct_undo_n;
  if (acct_next_snapshot == ACCT_NO_SNAPSHOT) abort();
  acct_active_snapshot = acct_next_snapshot++;
  /* The checkpoint is opaque to Sail: high = parent identity, low = cursor. */
  return ((uint64_t)prior << 32) | cursor;
}

unit acct_tx_revert(uint64_t checkpoint) {
  uint32_t cursor = (uint32_t)checkpoint;
  uint32_t prior = (uint32_t)(checkpoint >> 32);
  if (cursor > acct_undo_n) abort();
  while (acct_undo_n > cursor) {
    const acct_undo_entry *undo = &acct_undo[--acct_undo_n];
    acct_state_row *current =
        acct_table_get(&acct_tx_table, undo->raw_addr);
    if (!current) abort();
    current->cur_nonce = undo->nonce;
    memcpy(current->cur_bal, undo->balance, sizeof(current->cur_bal));
    current->cur_sroot = undo->storage_root;
    current->cur_chash = undo->code_hash;
    current->cur_exists = undo->exists;
    current->cur_storage_cleared = undo->storage_cleared;
    current->cur_created = undo->created;
    current->cur_selfdestructed = undo->selfdestructed;
    current->snapshot_cursor = undo->snapshot_cursor;
    current->tx_active = undo->prior_present;
  }
  acct_active_snapshot = prior;
  return UNIT;
}

/* Merge the transaction projection into the cumulative projection, then clear
   it. The cumulative ORIGINAL is the pre-block state and is frozen once when
   the account first enters the shared row. */
/* Drain-one pop for the standard Sail merge in transaction insertion order.
   The order is not protocol-visible; only the account-before-storage phase
   boundary matters. */
uint64_t acct_tx_pop_probe(sail_fixed_bytes_20 *addr,
                           uint64_t *cn, sail_u256 *cb,
                           sail_fixed_bytes_32 *cs, sail_fixed_bytes_32 *cc,
                           bool *ce, bool *csc, bool *ccr, bool *csd,
                           uint64_t *on, sail_u256 *ob,
                           sail_fixed_bytes_32 *os, sail_fixed_bytes_32 *oc,
                           bool *oe, bool *osc, bool *ocr, bool *osd) {
  const acct_state_row *e = NULL;
  while (acct_tx_pop_cursor < acct_tx_rows_n) {
    const acct_state_row *candidate =
        &acct_table.rows[acct_tx_rows[acct_tx_pop_cursor++]];
    if (candidate->tx_active) {
      e = candidate;
      break;
    }
  }
  if (!e) return 0;
  *addr = be_bytes_to_sail_address(e->raw_addr);
  *cn = e->cur_nonce;
  *cb = le_words4_to_sail_word(e->cur_bal);
  *cs = e->cur_sroot;
  *cc = e->cur_chash;
  *ce = e->cur_exists;
  *csc = e->cur_storage_cleared;
  *ccr = e->cur_created;
  *csd = e->cur_selfdestructed;
  *on = e->tx_orig_nonce;
  *ob = le_words4_to_sail_word(e->tx_orig_bal);
  *os = e->tx_orig_sroot;
  *oc = e->tx_orig_chash;
  *oe = e->tx_orig_exists;
  *osc = e->tx_orig_storage_cleared;
  *ocr = false;
  *osd = false;
  return 1;
}

/* Block-layer propagation hooks. Raw address is the point key; secure_key is
   supplied on witness materialization and copied thereafter. */
static acct_state_row *acct_block_bind(sail_fixed_bytes_20 a,
                                       const sail_fixed_bytes_32 *secure_key,
                                       int *fresh) {
  uint8_t address[20];
  sail_fixed_bytes_32 fallback;
  evmsail_address_to_be_bytes(address, a);
  acct_state_row *b = acct_table_get(&acct_table, address);
  *fresh = (b == NULL || !b->state_valid);
  if (!b) {
    if (!secure_key) {
      secure_keccak_address(a, &fallback);
      secure_key = &fallback;
    }
    b = acct_table_intern(&acct_table, address, secure_key);
  }
  if (b && !b->secure_key_valid) {
    if (!secure_key) {
      secure_keccak_address(a, &fallback);
      secure_key = &fallback;
    }
    b->hkey = *secure_key;
    b->secure_key_valid = 1;
  }
  return b;
}

/* changed account: fresh freezes orig; curr always lands */
unit acct_block_write_raw(sail_fixed_bytes_20 a, uint64_t nonce, sail_u256 bal,
                          sail_fixed_bytes_32 sroot, sail_fixed_bytes_32 chash,
                          bool exists, bool storage_cleared,
                          uint64_t ononce, sail_u256 obal,
                          sail_fixed_bytes_32 osroot, sail_fixed_bytes_32 ochash,
                          bool oexists, bool ostorage_cleared) {
  int fresh = 0;
  acct_state_row *b = acct_block_bind(a, NULL, &fresh);
  if (!b) return UNIT;
  if (fresh) {
    b->orig_nonce = ononce;
    sail_word_to_le_words4(b->orig_bal, obal);
    b->orig_sroot = osroot;
    b->orig_chash = ochash;
    b->orig_exists = oexists;
    b->orig_storage_cleared = ostorage_cleared;
  }
  b->cur_nonce = nonce;
  sail_word_to_le_words4(b->cur_bal, bal);
  b->cur_sroot = sroot;
  b->cur_chash = chash;
  b->cur_exists = exists;
  b->cur_storage_cleared = storage_cleared;
  /* acct_block_write is the transaction-end commit boundary. */
  b->cur_created = 0;
  b->cur_selfdestructed = 0;
  b->written = 1;
  b->state_valid = 1;
  acct_block_update_list(b);
  /* A transaction row written into this same physical row is now committed. */
  b->tx_active = 0;
  acct_dump_invalidate();
  return UNIT;
}

/* read member: fresh binds curr == orig; an existing row is untouched */
unit acct_block_cache_raw(sail_fixed_bytes_20 a, sail_fixed_bytes_32 address_hash,
                          uint64_t nonce, sail_u256 bal,
                          sail_fixed_bytes_32 sroot, sail_fixed_bytes_32 chash,
                          bool exists, bool storage_cleared) {
  int fresh = 0;
  acct_state_row *b = acct_block_bind(a, &address_hash, &fresh);
  if (!b) return UNIT;
  if (fresh) {
    b->cur_nonce = nonce;  b->orig_nonce = nonce;
    sail_word_to_le_words4(b->cur_bal, bal);
    sail_word_to_le_words4(b->orig_bal, bal);
    b->cur_sroot = sroot;
    b->orig_sroot = b->cur_sroot;
    b->cur_chash = chash;
    b->orig_chash = b->cur_chash;
    b->cur_exists = exists; b->orig_exists = exists;
    b->cur_storage_cleared = storage_cleared;
    b->orig_storage_cleared = storage_cleared;
    b->state_valid = 1;
    acct_dump_invalidate();
  }
  return UNIT;
}

/* --- reads (overlay only; a miss => Sail asks eest_account / resolver) --- */
/* per-layer account probe (layer 0 = tx, 1 = block) for the Sail
   option(Account) adapter: returns presence; the field words
   land in the out params (untouched when absent). The tx-over-block
   precedence lives in Sail (account_lookup). */
uint64_t acct_row_probe(uint64_t layer, sail_fixed_bytes_20 a, uint64_t *nonce,
                        sail_u256 *bal, sail_fixed_bytes_32 *sroot, sail_fixed_bytes_32 *chash,
                        bool *exists, bool *storage_cleared,
                        bool *created, bool *selfdestructed) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  acct_state_row *e = acct_table_get(&acct_table, address);
  if (!e || !e->state_valid || (layer == 0 && !e->tx_active)) return 0;
  *nonce = e->cur_nonce;
  *bal = le_words4_to_sail_word(e->cur_bal);
  *sroot = e->cur_sroot;
  *chash = e->cur_chash;
  *exists = e->cur_exists;
  *storage_cleared = e->cur_storage_cleared;
  *created = e->cur_created;
  *selfdestructed = e->cur_selfdestructed;
  return 1;
}

/* Non-destructive ascending iteration over account update candidates. */
static uint32_t acct_block_iter_position = 0;
static bool acct_block_iter_active = false;

static int acct_secure_order_cmp(const void *a, const void *b) {
  const acct_state_row *aa = &acct_block_table.rows[*(const uint32_t *)a];
  const acct_state_row *bb = &acct_block_table.rows[*(const uint32_t *)b];
  return evmsail_hash_compare(&aa->hkey, &bb->hkey);
}

uint32_t acct_block_updates_prepare(void) {
  if (acct_block_iter_order_cap < acct_block_update_rows_n) {
    uint32_t *order = realloc(
        acct_block_iter_order,
        (size_t)acct_block_update_rows_n * sizeof(*acct_block_iter_order));
    if (!order && acct_block_update_rows_n) abort();
    acct_block_iter_order = order;
    acct_block_iter_order_cap = acct_block_update_rows_n;
  }
  for (uint32_t i = 0; i < acct_block_update_rows_n; i++)
    acct_block_iter_order[i] = acct_block_update_rows[i];
  if (acct_block_update_rows_n > 1)
    qsort(acct_block_iter_order, acct_block_update_rows_n,
          sizeof(*acct_block_iter_order), acct_secure_order_cmp);
  acct_block_iter_end = acct_block_update_rows_n;
  return acct_block_iter_end;
}

unit acct_block_iter_begin(const unit u) {
  (void)u;
  (void)acct_block_updates_prepare();
  acct_block_iter_position = 0;
  acct_block_iter_active = true;
  return UNIT;
}

unit acct_post_storage_root_store(sail_fixed_bytes_20 a,
                                  sail_fixed_bytes_32 root) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  acct_state_row *entry = acct_table_get(&acct_block_table, address);
  if (entry) entry->post_sroot = root;
  return UNIT;
}

sail_fixed_bytes_32 acct_post_storage_root_read(sail_fixed_bytes_20 a) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  const acct_state_row *entry = acct_table_get(&acct_block_table, address);
  static const sail_fixed_bytes_32 zero = {{0}};
  return (entry ? entry->post_sroot : zero);
}

static uint64_t acct_entry_probe(const acct_state_row *entry,
                                 sail_fixed_bytes_20 *addr, uint64_t *cn,
                                 sail_u256 *cb, sail_fixed_bytes_32 *cs,
                                 sail_fixed_bytes_32 *cc, bool *ce, bool *csc,
                                 bool *ccr, bool *csd, uint64_t *on,
                                 sail_u256 *ob, sail_fixed_bytes_32 *os,
                                 sail_fixed_bytes_32 *oc, bool *oe, bool *osc,
                                 bool *ocr, bool *osd,
                                 sail_fixed_bytes_32 *address_hash) {
  if (!entry || !entry->state_valid) return 0;
  *addr = be_bytes_to_sail_address(entry->raw_addr);
  *cn = entry->cur_nonce;
  *cb = le_words4_to_sail_word(entry->cur_bal);
  *cs = entry->cur_sroot;
  *cc = entry->cur_chash;
  *ce = entry->cur_exists;
  *csc = entry->cur_storage_cleared;
  *ccr = entry->cur_created;
  *csd = entry->cur_selfdestructed;
  *on = entry->orig_nonce;
  *ob = le_words4_to_sail_word(entry->orig_bal);
  *os = entry->orig_sroot;
  *oc = entry->orig_chash;
  *oe = entry->orig_exists;
  *osc = entry->orig_storage_cleared;
  *ocr = entry->orig_created;
  *osd = entry->orig_selfdestructed;
  if (address_hash) *address_hash = entry->hkey;
  return 1;
}

static uint64_t acct_iter_next_probe(uint32_t *position, bool *active,
                                     const bool candidates_only,
                                     const uint32_t *order,
                                     sail_fixed_bytes_20 *addr, uint64_t *cn,
                                     sail_u256 *cb, sail_fixed_bytes_32 *cs,
                                     sail_fixed_bytes_32 *cc, bool *ce, bool *csc,
                                     bool *ccr, bool *csd, uint64_t *on,
                                     sail_u256 *ob, sail_fixed_bytes_32 *os,
                                     sail_fixed_bytes_32 *oc, bool *oe, bool *osc,
                                     bool *ocr, bool *osd,
                                     sail_fixed_bytes_32 *address_hash) {
  if (!*active) return 0;
  const uint32_t end =
      candidates_only ? acct_block_iter_end : acct_block_table.n;
  while (*position < end) {
    const uint32_t index = order ? order[(*position)++] : (*position)++;
    const acct_state_row *entry = &acct_block_table.rows[index];
    if (acct_entry_probe(entry, addr, cn, cb, cs, cc, ce, csc, ccr, csd, on,
                         ob, os, oc, oe, osc, ocr, osd, address_hash))
      return 1;
  }
  return 0;
}

uint64_t acct_block_update_probe_at(uint32_t index, sail_fixed_bytes_20 *addr,
                                    uint64_t *cn, sail_u256 *cb,
                                    sail_fixed_bytes_32 *cs, sail_fixed_bytes_32 *cc, bool *ce,
                                    bool *csc, bool *ccr, bool *csd,
                                    uint64_t *on, sail_u256 *ob,
                                    sail_fixed_bytes_32 *os, sail_fixed_bytes_32 *oc, bool *oe,
                                    bool *osc, bool *ocr, bool *osd,
                                    sail_fixed_bytes_32 *address_hash,
                                    sail_fixed_bytes_32 *post_storage_root) {
  if (index >= acct_block_iter_end) return 0;
  const acct_state_row *entry =
      &acct_block_table.rows[acct_block_iter_order[index]];
  if (!acct_entry_probe(entry, addr, cn, cb, cs, cc, ce, csc, ccr, csd, on,
                        ob, os, oc, oe, osc, ocr, osd, address_hash))
    return 0;
  if (post_storage_root) *post_storage_root = entry->post_sroot;
  return 1;
}

unit acct_block_update_post_storage_store_at(uint32_t index, sail_fixed_bytes_32 root) {
  if (index < acct_block_iter_end) {
    acct_state_row *entry =
        &acct_block_table.rows[acct_block_iter_order[index]];
    entry->post_sroot = root;
  }
  return UNIT;
}

uint64_t acct_block_iter_next_probe(sail_fixed_bytes_20 *addr, uint64_t *cn,
                                    sail_u256 *cb, sail_fixed_bytes_32 *cs,
                                    sail_fixed_bytes_32 *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    sail_u256 *ob, sail_fixed_bytes_32 *os,
                                    sail_fixed_bytes_32 *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd,
                                    sail_fixed_bytes_32 *address_hash) {
  return acct_iter_next_probe(
      &acct_block_iter_position, &acct_block_iter_active, true,
      acct_block_iter_order, addr, cn, cb, cs, cc, ce, csc, ccr, csd, on, ob,
      os, oc, oe, osc, ocr, osd, address_hash);
}

/* A first transaction write activates the transaction projection already
   carried by the cumulative account row established by k_aload. */
unit acct_tx_update_raw(sail_fixed_bytes_20 a, uint64_t nonce,
                        sail_u256 bal, sail_fixed_bytes_32 sroot, sail_fixed_bytes_32 chash,
                        bool exists, bool storage_cleared, bool created,
                        bool selfdestructed) {
  int fresh = 0;
  acct_state_row *e = acct_tx_bind_write(a, &fresh);
  if (!e) return UNIT;
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  uint64_t b4[4];
  sail_word_to_le_words4(b4, bal);
  e->cur_nonce = nonce;
  memcpy(e->cur_bal, b4, sizeof(e->cur_bal));
  e->cur_sroot = sroot;
  e->cur_chash = chash;
  e->cur_exists = exists;
  e->cur_storage_cleared = storage_cleared;
  e->cur_created |= created;
  e->cur_selfdestructed = selfdestructed;
  return UNIT;
}

unit acct_tx_set_balance(sail_fixed_bytes_20 a, const sail_u256 balance) {
  uint64_t value[4];
  int fresh = 0;
  sail_word_to_le_words4(value, (balance));
  acct_state_row *e = acct_tx_bind_write(a, &fresh);
  if (!e) return UNIT;
  if (compare_u64x4(e->cur_bal, value) == 0) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  memcpy(e->cur_bal, value, sizeof(e->cur_bal));
  return UNIT;
}

unit acct_tx_set_nonce(sail_fixed_bytes_20 a, uint64_t nonce) {
  int fresh = 0;
  acct_state_row *e = acct_tx_bind_write(a, &fresh);
  if (!e) return UNIT;
  if (e->cur_nonce == nonce) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  e->cur_nonce = nonce;
  return UNIT;
}

unit acct_tx_set_code_hash(sail_fixed_bytes_20 a, sail_fixed_bytes_32 code_hash) {
  sail_fixed_bytes_32 value = code_hash;
  int fresh = 0;
  acct_state_row *e = acct_tx_bind_write(a, &fresh);
  if (!e) return UNIT;
  if (evmsail_hash_equal(&e->cur_chash, &value)) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  if (!acct_snapshot_for_write(e, !fresh)) {
    if (fresh) e->tx_active = 0;
    return UNIT;
  }
  e->cur_chash = value;
  return UNIT;
}

/*
 * Optimized transaction-end merge. Transaction row order is deliberately
 * unobservable: accounts are completed first so deletion/storage-clear
 * decisions are committed before the active storage generation is drained.
 * The standard executable retains the structurally identical Sail equation.
 */
unit evmsail_tx_merge(bool cancun_or_later, bool amsterdam_or_later,
                      uint64_t block_access_index) {
  for (uint32_t i = 0; i < acct_tx_rows_n; i++) {
    acct_state_row *tx = &acct_table.rows[acct_tx_rows[i]];
    if (!tx->tx_active) continue;
    const bool deleted =
        tx->cur_selfdestructed && (!cancun_or_later || tx->cur_created);

    if (deleted) {
      tx->cur_nonce = 0;
      tx->cur_chash = account_empty_code_hash;
      tx->cur_storage_cleared = true;
      if (!amsterdam_or_later ||
          compare_u64x4(tx->cur_bal, account_zero_val) == 0) {
        memcpy(tx->cur_bal, account_zero_val, sizeof(tx->cur_bal));
        tx->cur_exists = false;
      } else {
        tx->cur_exists = true;
      }

      /* Retire writes from the deleted incarnation before storage drain. */
      storage_tx_clear(be_bytes_to_sail_address(tx->raw_addr));
    }

    if (deleted ||
        (tx->cur_storage_cleared && !tx->tx_orig_storage_cleared))
      storage_block_clear(be_bytes_to_sail_address(tx->raw_addr));

    const sail_fixed_bytes_20 address = be_bytes_to_sail_address(tx->raw_addr);
    if (tx->cur_nonce != tx->tx_orig_nonce)
      bal_note_nonce_change(block_access_index, address, tx->cur_nonce);
    if (compare_u64x4(tx->cur_bal, tx->tx_orig_bal) != 0)
      bal_note_balance_change(
          block_access_index, address, le_words4_to_sail_word(tx->cur_bal));
    if (!evmsail_hash_equal(&tx->cur_chash, &tx->tx_orig_chash))
      bal_note_code_change(block_access_index, address, tx->cur_chash);

    tx->cur_created = false;
    tx->cur_selfdestructed = false;
    const bool changed =
        tx->cur_nonce != tx->tx_orig_nonce ||
        compare_u64x4(tx->cur_bal, tx->tx_orig_bal) != 0 ||
        !evmsail_hash_equal(&tx->cur_sroot, &tx->tx_orig_sroot) ||
        !evmsail_hash_equal(&tx->cur_chash, &tx->tx_orig_chash) ||
        tx->cur_exists != tx->tx_orig_exists ||
        tx->cur_storage_cleared != tx->tx_orig_storage_cleared;
    if (changed) {
      /*
       * The transaction worklist already names the shared physical row.
       * Commit its live projection in place instead of converting the key and
       * probing the aliased cumulative table again.
       */
      tx->written = 1;
      tx->state_valid = 1;
      tx->tx_active = 0;
      acct_block_update_list(tx);
      acct_dump_invalidate();
    }
  }

  for (uint32_t i = 0; i < storage_tx_rows_n; i++) {
    storage_state_row *tx =
        &storage_table.rows[storage_tx_rows[i]];
    if (!tx->tx_active ||
        tx->generation != storage_active_generation(tx->raw_addr))
      continue;
    acct_state_row *account =
        acct_table_get(&acct_block_table, tx->raw_addr);
    if (!account || !account->state_valid || !account->cur_exists ||
        compare_u64x4(tx->tx_current, tx->tx_original) == 0)
      continue;

    const sail_fixed_bytes_20 address = be_bytes_to_sail_address(tx->raw_addr);
    const sail_u256 slot = be_words4_to_sail_word(tx->slot);
    const sail_u256 current = be_words4_to_sail_word(tx->tx_current);
    bal_note_storage_change(block_access_index, address, slot, current);
    /*
     * As above, this is already the cumulative storage row. Freeze its
     * pre-block original only when the row had no cumulative value, then land
     * the transaction value without a second composite-key lookup.
     */
    if (!tx->state_valid)
      memcpy(tx->original, tx->tx_original, sizeof(tx->original));
    memcpy(tx->current, tx->tx_current, sizeof(tx->current));
    tx->written = 1;
    tx->state_valid = 1;
    acct_block_update_list(account);
    storage_dump_invalidate();
  }

  acct_tx_reset(UNIT);
  storage_tx_reset(UNIT);
  return UNIT;
}

/* ---- debug enumeration over the cumulative account projection -----------
   The snapshot contains existing cumulative account values for the native
   post-state dump. State-root construction consumes acct_block_probe_row
   directly. */

typedef struct {
  sail_fixed_bytes_32 hkey;
  uint8_t raw_addr[20];
  uint64_t nonce; uint64_t bal[4];
  sail_fixed_bytes_32 sroot, chash;

} acct_dump_entry;

static void acct_dump_push(acct_dump_entry **rows, uint32_t *n, uint32_t *cap,
                           const sail_fixed_bytes_32 *hkey,
                           const uint8_t raw_addr[20],
                           uint64_t nonce, const uint64_t bal[4],
                           const sail_fixed_bytes_32 *sroot,
                           const sail_fixed_bytes_32 *chash) {
  if (*cap < *n + 1) {
    uint32_t nc = *cap ? *cap * 2 : 16;
    while (nc < *n + 1) nc *= 2;
    *rows = (acct_dump_entry *)realloc(*rows, (size_t)nc * sizeof(acct_dump_entry));
    *cap = nc;
  }
  acct_dump_entry *r = &(*rows)[*n];
  r->hkey = *hkey;
  memcpy(r->raw_addr, raw_addr, 20); r->nonce = nonce;
  memcpy(r->bal, bal, 32);
  r->sroot = *sroot;
  r->chash = *chash;
  (*n)++;
}

/* Existing touched accounts, materialized for the harness dump. */
static acct_dump_entry *acct_dump_entries = NULL;
static uint32_t acct_dump_len = 0, acct_dump_cap = 0;
static int acct_dump_valid = 0;

static void acct_dump_build(void) {
  if (acct_dump_valid) return;
  acct_dump_len = 0;
  for (uint32_t bi = 0; bi < acct_block_table.n; bi++) {
    const acct_state_row *b = &acct_block_table.rows[bi];
    if (!b->state_valid || !b->cur_exists) continue;
    acct_dump_push(&acct_dump_entries, &acct_dump_len, &acct_dump_cap,
                   &b->hkey, b->raw_addr, b->cur_nonce, b->cur_bal,
                   &b->cur_sroot, &b->cur_chash);
  }
  acct_dump_valid = 1;
}

uint64_t acct_dump_count(const unit u) { (void)u; acct_dump_build(); return acct_dump_len; }
sail_u256 acct_dump_hkey(uint64_t i) {
  acct_dump_build();
  if (i < acct_dump_len)
    return evmsail_hash_to_sail_word(&acct_dump_entries[i].hkey);
  return be_words4_to_sail_word(account_zero_val);
}
sail_fixed_bytes_20 acct_dump_address(uint64_t i) {
  acct_dump_build();
  return be_bytes_to_sail_address(i < acct_dump_len
                                      ? acct_dump_entries[i].raw_addr
                                      : (const uint8_t[20]){0});
}
uint64_t acct_dump_nonce(uint64_t i) { acct_dump_build(); return i < acct_dump_len ? acct_dump_entries[i].nonce : 0; }
sail_u256 acct_dump_balance(uint64_t i) {
  acct_dump_build(); return le_words4_to_sail_word(i < acct_dump_len ? acct_dump_entries[i].bal : account_zero_val);
}
sail_u256 acct_dump_storage_root(uint64_t i) {
  static const sail_fixed_bytes_32 zero = {{0}};
  acct_dump_build();
  return evmsail_hash_to_sail_word(
      i < acct_dump_len ? &acct_dump_entries[i].sroot : &zero);
}
sail_u256 acct_dump_code_hash(uint64_t i) {
  static const sail_fixed_bytes_32 zero = {{0}};
  acct_dump_build();
  return evmsail_hash_to_sail_word(
      i < acct_dump_len ? &acct_dump_entries[i].chash : &zero);
}

static void acct_dump_invalidate(void) {
  acct_dump_valid = 0;
}

static void *bal_reserve(void *rows, uint32_t *cap, uint32_t need,
                         size_t element_size) {
  if (*cap >= need) return rows;
  uint32_t next = *cap ? *cap * 2 : 64;
  while (next < need) next *= 2;
  rows = realloc(rows, (size_t)next * element_size);
  if (!rows) abort();
  *cap = next;
  return rows;
}

static acct_state_row *bal_account_activate(const uint8_t address[20]) {
  static const sail_fixed_bytes_32 zero = {{0}};
  if (bal_prepared) abort();
  acct_state_row *row = acct_table_get(&acct_block_table, address);
  if (!row)
    row = acct_table_intern(&acct_block_table, address, &zero);
  if (!row) abort();
  if (row->bal_epoch != bal_epoch) {
    row->bal_epoch = bal_epoch;
    row->bal_touched = 0;
    row->bal_balance_head = row->bal_balance_tail = BAL_NO_HISTORY;
    row->bal_nonce_head = row->bal_nonce_tail = BAL_NO_HISTORY;
    row->bal_code_head = row->bal_code_tail = BAL_NO_HISTORY;
    bal_account_order = bal_reserve(
        bal_account_order, &bal_account_order_cap, bal_account_order_n + 1,
        sizeof(*bal_account_order));
    bal_account_order[bal_account_order_n++] =
        (uint32_t)(row - acct_block_table.rows);
  }
  row->bal_touched = 1;
  return row;
}

static storage_state_row *bal_storage_activate(const uint8_t address[20],
                                               const uint64_t slot[4]) {
  static const sail_fixed_bytes_32 zero = {{0}};
  storage_state_row *row = storage_table_get(
      &storage_block_table, address, slot, STORAGE_BLOCK_GENERATION);
  if (!row)
    row = storage_table_intern(&storage_block_table, address, slot,
                               STORAGE_BLOCK_GENERATION, &zero, &zero);
  if (!row) abort();
  if (row->bal_epoch != bal_epoch) {
    row->bal_epoch = bal_epoch;
    row->bal_read = 0;
    row->bal_change_head = row->bal_change_tail = BAL_NO_HISTORY;
    bal_storage_order = bal_reserve(
        bal_storage_order, &bal_storage_order_cap, bal_storage_order_n + 1,
        sizeof(*bal_storage_order));
    bal_storage_order[bal_storage_order_n++] =
        (uint32_t)(row - storage_block_table.rows);
  }
  return row;
}

static void bal_append_word(bal_word_history **rows, uint32_t *length,
                            uint32_t *capacity, uint32_t *head,
                            uint32_t *tail, uint32_t index,
                            const uint64_t value[4]) {
  if (*tail != BAL_NO_HISTORY) {
    bal_word_history *last = &(*rows)[*tail];
    if (last->index == index) {
      memcpy(last->value, value, sizeof(last->value));
      return;
    }
    if (last->index > index) abort();
  }
  *rows = bal_reserve(*rows, capacity, *length + 1, sizeof(**rows));
  const uint32_t position = (*length)++;
  bal_word_history *entry = &(*rows)[position];
  entry->index = index;
  entry->next = BAL_NO_HISTORY;
  memcpy(entry->value, value, sizeof(entry->value));
  if (*tail == BAL_NO_HISTORY)
    *head = position;
  else
    (*rows)[*tail].next = position;
  *tail = position;
}

static void bal_append_nonce(uint32_t *head, uint32_t *tail, uint32_t index,
                             uint64_t value) {
  if (*tail != BAL_NO_HISTORY) {
    bal_nonce_history *last = &bal_nonce_histories[*tail];
    if (last->index == index) {
      last->value = value;
      return;
    }
    if (last->index > index) abort();
  }
  bal_nonce_histories =
      bal_reserve(bal_nonce_histories, &bal_nonce_history_cap,
                  bal_nonce_history_n + 1, sizeof(*bal_nonce_histories));
  const uint32_t position = bal_nonce_history_n++;
  bal_nonce_history *entry = &bal_nonce_histories[position];
  entry->index = index;
  entry->next = BAL_NO_HISTORY;
  entry->value = value;
  if (*tail == BAL_NO_HISTORY)
    *head = position;
  else
    bal_nonce_histories[*tail].next = position;
  *tail = position;
}

static void bal_append_code(uint32_t *head, uint32_t *tail, uint32_t index,
                            const sail_fixed_bytes_32 *value) {
  if (*tail != BAL_NO_HISTORY) {
    bal_code_history *last = &bal_code_histories[*tail];
    if (last->index == index) {
      last->value = *value;
      return;
    }
    if (last->index > index) abort();
  }
  bal_code_histories =
      bal_reserve(bal_code_histories, &bal_code_history_cap,
                  bal_code_history_n + 1, sizeof(*bal_code_histories));
  const uint32_t position = bal_code_history_n++;
  bal_code_history *entry = &bal_code_histories[position];
  entry->index = index;
  entry->next = BAL_NO_HISTORY;
  entry->value = *value;
  if (*tail == BAL_NO_HISTORY)
    *head = position;
  else
    bal_code_histories[*tail].next = position;
  *tail = position;
}

unit bal_note_account_touch(sail_fixed_bytes_20 a) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  (void)bal_account_activate(address);
  return UNIT;
}

unit bal_note_storage_change(uint64_t index, sail_fixed_bytes_20 a,
                             const sail_u256 slot,
                             const sail_u256 value) {
  uint8_t address[20];
  uint64_t slot_words[4], value_words[4];
  evmsail_address_to_be_bytes(address, a);
  sail_word_to_be_words4(slot_words, (slot));
  sail_word_to_be_words4(value_words, (value));
  (void)bal_account_activate(address);
  storage_state_row *row = bal_storage_activate(address, slot_words);
  bal_append_word(&bal_storage_history, &bal_storage_history_n,
                  &bal_storage_history_cap, &row->bal_change_head,
                  &row->bal_change_tail, (uint32_t)index, value_words);
  return UNIT;
}

unit bal_note_storage_read(sail_fixed_bytes_20 a, const sail_u256 slot) {
  uint8_t address[20];
  uint64_t slot_words[4];
  evmsail_address_to_be_bytes(address, a);
  sail_word_to_be_words4(slot_words, (slot));
  (void)bal_account_activate(address);
  bal_storage_activate(address, slot_words)->bal_read = 1;
  return UNIT;
}

unit bal_note_balance_change(uint64_t index, sail_fixed_bytes_20 a,
                             const sail_u256 value) {
  uint8_t address[20];
  uint64_t value_words[4];
  evmsail_address_to_be_bytes(address, a);
  sail_word_to_le_words4(value_words, (value));
  acct_state_row *row = bal_account_activate(address);
  bal_append_word(&bal_balance_history, &bal_balance_history_n,
                  &bal_balance_history_cap, &row->bal_balance_head,
                  &row->bal_balance_tail, (uint32_t)index, value_words);
  return UNIT;
}

unit bal_note_nonce_change(uint64_t index, sail_fixed_bytes_20 a, uint64_t nonce) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  acct_state_row *row = bal_account_activate(address);
  bal_append_nonce(&row->bal_nonce_head, &row->bal_nonce_tail,
                   (uint32_t)index, nonce);
  return UNIT;
}

unit bal_note_code_change(uint64_t index, sail_fixed_bytes_20 a, sail_fixed_bytes_32 code_hash) {
  uint8_t address[20];
  evmsail_address_to_be_bytes(address, a);
  acct_state_row *row = bal_account_activate(address);
  bal_append_code(&row->bal_code_head, &row->bal_code_tail,
                  (uint32_t)index, &code_hash);
  return UNIT;
}

static int bal_account_order_compare(const void *a, const void *b) {
  const acct_state_row *left =
      &acct_block_table.rows[*(const uint32_t *)a];
  const acct_state_row *right =
      &acct_block_table.rows[*(const uint32_t *)b];
  return memcmp(left->raw_addr, right->raw_addr, sizeof(left->raw_addr));
}

static int bal_storage_order_compare(const void *a, const void *b) {
  const storage_state_row *left =
      &storage_block_table.rows[*(const uint32_t *)a];
  const storage_state_row *right =
      &storage_block_table.rows[*(const uint32_t *)b];
  int comparison =
      memcmp(left->raw_addr, right->raw_addr, sizeof(left->raw_addr));
  if (comparison) return comparison;
  return compare_u64x4(left->slot, right->slot);
}

static void bal_reset_cursors(void) {
  bal_account_cursor = 0;
  bal_storage_cursor = 0;
  bal_active_account_index = BAL_NO_HISTORY;
  bal_active_storage_history = BAL_NO_HISTORY;
  bal_active_field_history = BAL_NO_HISTORY;
  bal_stream_phase = BAL_STREAM_ACCOUNT;
}

unit bal_reset(const unit u) {
  (void)u;
  if (bal_epoch == UINT64_MAX) {
    for (uint32_t i = 0; i < acct_block_table.n; i++)
      acct_block_table.rows[i].bal_epoch = 0;
    for (uint32_t i = 0; i < storage_block_table.n; i++)
      storage_block_table.rows[i].bal_epoch = 0;
    bal_epoch = 1;
  } else {
    bal_epoch++;
  }
  bal_storage_history_n = 0;
  bal_balance_history_n = 0;
  bal_nonce_history_n = 0;
  bal_code_history_n = 0;
  bal_account_order_n = 0;
  bal_storage_order_n = 0;
  bal_prepared = 0;
  bal_reset_cursors();
  return UNIT;
}

unit bal_prepare_iter(const unit u) {
  (void)u;
  /*
   * First activation appends each shared row once for this BAL epoch, so
   * preparation sorts only actual BAL members instead of rescanning the
   * larger state caches.
   */
  if (bal_account_order_n > 1)
    qsort(bal_account_order, bal_account_order_n,
          sizeof(*bal_account_order), bal_account_order_compare);
  if (bal_storage_order_n > 1)
    qsort(bal_storage_order, bal_storage_order_n,
          sizeof(*bal_storage_order), bal_storage_order_compare);
  bal_prepared = 1;
  bal_reset_cursors();
  return UNIT;
}

static void bal_ensure_prepared(void) {
  if (!bal_prepared) (void)bal_prepare_iter(UNIT);
}

static const acct_state_row *bal_active_account(void) {
  if (bal_active_account_index == BAL_NO_HISTORY) return NULL;
  return &acct_block_table.rows[bal_active_account_index];
}

static int bal_storage_belongs_to_active(const storage_state_row *storage) {
  const acct_state_row *account = bal_active_account();
  return account &&
         memcmp(storage->raw_addr, account->raw_addr,
                sizeof(storage->raw_addr)) == 0;
}

enum bal_iter_tag bal_iter_next_probe(sail_fixed_bytes_20 *address, sail_u256 *slot,
                                      uint64_t *index, sail_u256 *value,
                                      uint64_t *nonce, sail_fixed_bytes_32 *code_hash) {
  bal_ensure_prepared();
  for (;;) {
    const acct_state_row *account = bal_active_account();
    switch (bal_stream_phase) {
    case BAL_STREAM_ACCOUNT: {
      if (bal_account_cursor >= bal_account_order_n) {
        bal_stream_phase = BAL_STREAM_EMPTY;
        return BAL_ITER_EMPTY;
      }
      bal_active_account_index = bal_account_order[bal_account_cursor++];
      account = bal_active_account();
      while (bal_storage_cursor < bal_storage_order_n) {
        const storage_state_row *candidate =
            &storage_block_table.rows[bal_storage_order[bal_storage_cursor]];
        if (memcmp(candidate->raw_addr, account->raw_addr, 20) >= 0) break;
        bal_storage_cursor++;
      }
      bal_active_storage_history = BAL_NO_HISTORY;
      bal_active_field_history = BAL_NO_HISTORY;
      bal_stream_phase = BAL_STREAM_STORAGE_CHANGE;
      *address = be_bytes_to_sail_address(account->raw_addr);
      return BAL_ITER_ACCOUNT;
    }

    case BAL_STREAM_STORAGE_CHANGE: {
      if (bal_active_storage_history != BAL_NO_HISTORY) {
        const bal_word_history *history =
            &bal_storage_history[bal_active_storage_history];
        const storage_state_row *storage =
            &storage_block_table.rows[bal_storage_order[bal_storage_cursor]];
        bal_active_storage_history = history->next;
        *slot = be_words4_to_sail_word(storage->slot);
        *index = history->index;
        *value = be_words4_to_sail_word(history->value);
        if (bal_active_storage_history == BAL_NO_HISTORY)
          bal_storage_cursor++;
        return BAL_ITER_STORAGE_CHANGE;
      }
      while (bal_storage_cursor < bal_storage_order_n) {
        const storage_state_row *storage =
            &storage_block_table.rows[bal_storage_order[bal_storage_cursor]];
        if (!bal_storage_belongs_to_active(storage)) break;
        if (storage->bal_change_head != BAL_NO_HISTORY) {
          bal_active_storage_history = storage->bal_change_head;
          break;
        }
        bal_storage_cursor++;
      }
      if (bal_active_storage_history != BAL_NO_HISTORY) continue;
      while (bal_storage_cursor > 0) {
        const storage_state_row *prior =
            &storage_block_table.rows[bal_storage_order[bal_storage_cursor - 1]];
        if (!bal_storage_belongs_to_active(prior)) break;
        bal_storage_cursor--;
      }
      bal_stream_phase = BAL_STREAM_STORAGE_READ;
      continue;
    }

    case BAL_STREAM_STORAGE_READ:
      while (bal_storage_cursor < bal_storage_order_n) {
        const storage_state_row *storage =
            &storage_block_table.rows[bal_storage_order[bal_storage_cursor++]];
        if (!bal_storage_belongs_to_active(storage)) {
          bal_storage_cursor--;
          break;
        }
        if (!storage->bal_read ||
            storage->bal_change_head != BAL_NO_HISTORY)
          continue;
        *slot = be_words4_to_sail_word(storage->slot);
        return BAL_ITER_STORAGE_READ;
      }
      bal_active_field_history = account->bal_balance_head;
      bal_stream_phase = BAL_STREAM_BALANCE;
      continue;

    case BAL_STREAM_BALANCE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const bal_word_history *history =
            &bal_balance_history[bal_active_field_history];
        bal_active_field_history = history->next;
        *index = history->index;
        *value = le_words4_to_sail_word(history->value);
        return BAL_ITER_BALANCE_CHANGE;
      }
      bal_active_field_history = account->bal_nonce_head;
      bal_stream_phase = BAL_STREAM_NONCE;
      continue;

    case BAL_STREAM_NONCE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const bal_nonce_history *history =
            &bal_nonce_histories[bal_active_field_history];
        bal_active_field_history = history->next;
        *index = history->index;
        *nonce = history->value;
        return BAL_ITER_NONCE_CHANGE;
      }
      bal_active_field_history = account->bal_code_head;
      bal_stream_phase = BAL_STREAM_CODE;
      continue;

    case BAL_STREAM_CODE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const bal_code_history *history =
            &bal_code_histories[bal_active_field_history];
        bal_active_field_history = history->next;
        *index = history->index;
        *code_hash = history->value;
        return BAL_ITER_CODE_CHANGE;
      }
      bal_stream_phase = BAL_STREAM_ACCOUNT_END;
      continue;

    case BAL_STREAM_ACCOUNT_END:
      bal_active_account_index = BAL_NO_HISTORY;
      bal_stream_phase = BAL_STREAM_ACCOUNT;
      return BAL_ITER_ACCOUNT_END;

    case BAL_STREAM_EMPTY:
      return BAL_ITER_EMPTY;
    }
  }
}
