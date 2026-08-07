/* Optimized EIP-7928 block access list state.
 *
 * BAL histories and iteration order are owned here. Entries refer to account
 * and storage state solely through stable dense IDs; no BAL metadata is stored
 * in semantic account or storage rows. */
#include "evmsail/prelude.h"
#include "evmsail/host/state/primitives.h"
#include "host/state/block_access_list.h"
#include "host/state/internal.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BAL_NO_HISTORY UINT32_MAX

/* One per-transaction-epoch change row; rows chain per account or slot
 * through next. word holds four little-endian native limbs, least-significant
 * limb first, for both storage and balance histories. */
typedef struct {
  uint32_t index;
  uint32_t next;
  union {
    uint64_t word[4];
    uint64_t nonce;
    bytes32 code_hash;
  } value;
} BalHistoryRow;

typedef struct {
  uint64_t block_epoch;
  uint32_t balance_head, balance_tail;
  uint32_t nonce_head, nonce_tail;
  uint32_t code_head, code_tail;
  uint8_t touched;
} BalAccountState;

typedef struct {
  uint64_t block_epoch;
  AccountId account_id;
  uint32_t change_head;
  uint32_t change_tail;
  uint8_t read;
} BalStorageState;

static BalHistoryRow *bal_storage_history;
static uint32_t bal_storage_history_n = 0;
static BalHistoryRow *bal_balance_history;
static uint32_t bal_balance_history_n = 0;
static BalHistoryRow *bal_nonce_histories;
static uint32_t bal_nonce_history_n = 0;
static BalHistoryRow *bal_code_histories;
static uint32_t bal_code_history_n = 0;
static BalAccountState *bal_accounts;
static BalStorageState *bal_storage;
/* Separates independent block executions. Transaction ordering within one BAL
 * is carried by each history row's protocol transaction epoch. */
static uint64_t block_epoch = 1;

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

static AccountId *bal_account_order;
static uint32_t bal_account_order_n = 0;
static StorageId *bal_storage_order;
static uint32_t bal_storage_order_n = 0;
static uint32_t bal_account_cursor = 0;
static uint32_t bal_storage_cursor = 0;
static AccountId bal_active_account_index = BAL_NO_HISTORY;
static uint32_t bal_active_storage_history = BAL_NO_HISTORY;
static uint32_t bal_active_field_history = BAL_NO_HISTORY;
static enum bal_stream_phase bal_stream_phase = BAL_STREAM_ACCOUNT;

void block_access_list_workspace_bind(uint32_t account_count, uint32_t storage_count)
{
  WORKSPACE_BIND(bal_storage_history, GUEST_BAL_STORAGE_CHANGES);
  WORKSPACE_BIND(bal_balance_history, GUEST_BAL_BALANCE_CHANGES);
  WORKSPACE_BIND(bal_nonce_histories, GUEST_BAL_NONCE_CHANGES);
  WORKSPACE_BIND(bal_code_histories, GUEST_BAL_CODE_CHANGES);
  /* AccountId zero is reserved for failed BAL-schema lookups. */
  WORKSPACE_BIND(bal_accounts, account_count + 1U);
  WORKSPACE_BIND(bal_storage, storage_count);
  WORKSPACE_BIND(bal_account_order, account_count);
  WORKSPACE_BIND(bal_storage_order, storage_count);
}

/* Exact BAL rows are initialized as their account/storage schema identity is
 * appended. Later activation may therefore inspect the epoch without any
 * allocation-wide clearing pass. */
void bal_account_schema_initialize(AccountId id)
{
  bal_accounts[id].block_epoch = 0;
}

void bal_storage_schema_initialize(StorageId id)
{
  bal_storage[id].block_epoch = 0;
}

static AccountId bal_account_activate(const bytes20 *address)
{
  const AccountId id = get_account_id(address);
  BalAccountState *account = &bal_accounts[id];
  if (account->block_epoch != block_epoch) {
    account->block_epoch = block_epoch;
    account->touched = 0;
    account->balance_head = account->balance_tail = BAL_NO_HISTORY;
    account->nonce_head = account->nonce_tail = BAL_NO_HISTORY;
    account->code_head = account->code_tail = BAL_NO_HISTORY;
    bal_account_order[bal_account_order_n++] = id;
  }
  account->touched = 1;
  return id;
}

static StorageId bal_storage_activate(AccountId account_id, const u256 *slot)
{
  const StorageId storage_id = get_storage_id(account_id, slot);
  BalStorageState *bal = &bal_storage[storage_id];
  if (bal->block_epoch != block_epoch) {
    bal->block_epoch = block_epoch;
    bal->account_id = account_id;
    bal->read = 0;
    bal->change_head = bal->change_tail = BAL_NO_HISTORY;
    bal_storage_order[bal_storage_order_n++] = storage_id;
  }
  return storage_id;
}

/* Returns the chain-tail history row for transaction_epoch, appending a new
 * tail row when the epoch advances. */
static BalHistoryRow *bal_append_history(BalHistoryRow *rows, uint32_t *length, uint32_t capacity,
                                         uint32_t *head, uint32_t *tail, uint32_t transaction_epoch)
{
  if (*tail != BAL_NO_HISTORY) {
    BalHistoryRow *last = &rows[*tail];
    if (last->index == transaction_epoch) {
      return last;
    }
  }
  if (*length >= capacity) {
    GUEST_ABORT();
  }
  const uint32_t position = (*length)++;
  BalHistoryRow *entry = &rows[position];
  entry->index = transaction_epoch;
  entry->next = BAL_NO_HISTORY;
  if (*tail == BAL_NO_HISTORY) {
    *head = position;
  } else {
    rows[*tail].next = position;
  }
  *tail = position;
  return entry;
}

void bal_note_account_touch(bytes20 a)
{
  (void)bal_account_activate(&a);
}

void bal_note_storage_change(uint64_t transaction_epoch, bytes20 address, const u256 slot,
                             const u256 value)
{
  const AccountId account_id = bal_account_activate(&address);
  const StorageId storage_id = bal_storage_activate(account_id, &slot);
  BalStorageState *bal = &bal_storage[storage_id];
  BalHistoryRow *row =
      bal_append_history(bal_storage_history, &bal_storage_history_n, GUEST_BAL_STORAGE_CHANGES,
                         &bal->change_head, &bal->change_tail, (uint32_t)transaction_epoch);
  sail_word_to_le_words4(row->value.word, value);
}

void bal_note_storage_read(bytes20 a, const u256 slot)
{
  const AccountId account_id = bal_account_activate(&a);
  const StorageId storage_id = bal_storage_activate(account_id, &slot);
  bal_storage[storage_id].read = 1;
}

void bal_note_balance_change(uint64_t transaction_epoch, bytes20 address, const u256 value)
{
  const AccountId account_id = bal_account_activate(&address);
  BalAccountState *account = &bal_accounts[account_id];
  BalHistoryRow *row = bal_append_history(bal_balance_history, &bal_balance_history_n,
                                          GUEST_BAL_BALANCE_CHANGES, &account->balance_head,
                                          &account->balance_tail, (uint32_t)transaction_epoch);
  sail_word_to_le_words4(row->value.word, value);
}

void bal_note_nonce_change(uint64_t transaction_epoch, bytes20 address, uint64_t nonce)
{
  const AccountId account_id = bal_account_activate(&address);
  BalAccountState *account = &bal_accounts[account_id];
  BalHistoryRow *row =
      bal_append_history(bal_nonce_histories, &bal_nonce_history_n, GUEST_BAL_NONCE_CHANGES,
                         &account->nonce_head, &account->nonce_tail, (uint32_t)transaction_epoch);
  row->value.nonce = nonce;
}

void bal_note_code_change(uint64_t transaction_epoch, bytes20 address, bytes32 code_hash)
{
  const AccountId account_id = bal_account_activate(&address);
  BalAccountState *account = &bal_accounts[account_id];
  BalHistoryRow *row =
      bal_append_history(bal_code_histories, &bal_code_history_n, GUEST_BAL_CODE_CHANGES,
                         &account->code_head, &account->code_tail, (uint32_t)transaction_epoch);
  row->value.code_hash = code_hash;
}

static int bal_account_order_compare(AccountId a, AccountId b)
{
  return address_compare(account_id_address(a), account_id_address(b));
}

static int bal_storage_order_compare(StorageId a, StorageId b)
{
  const bytes20 *left_address = account_id_address(bal_storage[a].account_id);
  const bytes20 *right_address = account_id_address(bal_storage[b].account_id);
  int comparison = address_compare(left_address, right_address);
  if (comparison) {
    return comparison;
  }
  return word_compare(storage_id_slot(a), storage_id_slot(b));
}

enum BalOrderKind {
  BAL_ORDER_ACCOUNT,
  BAL_ORDER_STORAGE,
};

static int bal_order_compare(enum BalOrderKind kind, uint32_t left, uint32_t right)
{
  return kind == BAL_ORDER_ACCOUNT ? bal_account_order_compare(left, right)
                                   : bal_storage_order_compare(left, right);
}

static void sort_index_order(uint32_t *order, uint32_t length, enum BalOrderKind kind)
{
  for (uint32_t i = 1; i < length; i++) {
    const uint32_t candidate = order[i];
    uint32_t position = i;
    while (position != 0 && bal_order_compare(kind, candidate, order[position - 1]) < 0) {
      order[position] = order[position - 1];
      position--;
    }
    order[position] = candidate;
  }
}

static void bal_reset_cursors(void)
{
  bal_account_cursor = 0;
  bal_storage_cursor = 0;
  bal_active_account_index = BAL_NO_HISTORY;
  bal_active_storage_history = BAL_NO_HISTORY;
  bal_active_field_history = BAL_NO_HISTORY;
  bal_stream_phase = BAL_STREAM_ACCOUNT;
}

void bal_reset(void)
{
  if (block_epoch == UINT64_MAX) {
    GUEST_ABORT();
  }
  block_epoch++;
  bal_storage_history_n = 0;
  bal_balance_history_n = 0;
  bal_nonce_history_n = 0;
  bal_code_history_n = 0;
  bal_account_order_n = 0;
  bal_storage_order_n = 0;
  bal_reset_cursors();
}

void bal_prepare_iter(void)
{
  /*
   * First activation appends each shared row once for this BAL epoch, so
   * preparation sorts only actual BAL members instead of rescanning the
   * larger state caches.
   */
  sort_index_order(bal_account_order, bal_account_order_n, BAL_ORDER_ACCOUNT);
  sort_index_order(bal_storage_order, bal_storage_order_n, BAL_ORDER_STORAGE);
  bal_reset_cursors();
}

static int bal_storage_belongs_to_active(StorageId storage_id)
{
  return bal_storage[storage_id].account_id == bal_active_account_index;
}

enum bal_iter_tag bal_iter_next_probe(AccountId *account_id, StorageId *storage_id, uint64_t *index,
                                      u256 *value, uint64_t *nonce, bytes32 *code_hash)
{
  for (;;) {
    switch (bal_stream_phase) {
    case BAL_STREAM_ACCOUNT: {
      if (bal_account_cursor >= bal_account_order_n) {
        bal_stream_phase = BAL_STREAM_EMPTY;
        return BAL_ITER_EMPTY;
      }
      bal_active_account_index = bal_account_order[bal_account_cursor++];
      const bytes20 *account = account_id_address(bal_active_account_index);
      while (bal_storage_cursor < bal_storage_order_n) {
        const StorageId candidate_id = bal_storage_order[bal_storage_cursor];
        const bytes20 *candidate = account_id_address(bal_storage[candidate_id].account_id);
        if (address_compare(candidate, account) >= 0) {
          break;
        }
        bal_storage_cursor++;
      }
      bal_active_storage_history = BAL_NO_HISTORY;
      bal_active_field_history = BAL_NO_HISTORY;
      bal_stream_phase = BAL_STREAM_STORAGE_CHANGE;
      *account_id = bal_active_account_index;
      return BAL_ITER_ACCOUNT;
    }

    case BAL_STREAM_STORAGE_CHANGE: {
      if (bal_active_storage_history != BAL_NO_HISTORY) {
        const BalHistoryRow *history = &bal_storage_history[bal_active_storage_history];
        const StorageId current_storage_id = bal_storage_order[bal_storage_cursor];
        bal_active_storage_history = history->next;
        *account_id = bal_active_account_index;
        *storage_id = current_storage_id;
        *index = history->index;
        *value = le_words4_to_sail_word(history->value.word);
        if (bal_active_storage_history == BAL_NO_HISTORY) {
          bal_storage_cursor++;
        }
        return BAL_ITER_STORAGE_CHANGE;
      }
      while (bal_storage_cursor < bal_storage_order_n) {
        const StorageId candidate_storage_id = bal_storage_order[bal_storage_cursor];
        if (!bal_storage_belongs_to_active(candidate_storage_id)) {
          break;
        }
        if (bal_storage[candidate_storage_id].change_head != BAL_NO_HISTORY) {
          bal_active_storage_history = bal_storage[candidate_storage_id].change_head;
          break;
        }
        bal_storage_cursor++;
      }
      if (bal_active_storage_history != BAL_NO_HISTORY) {
        continue;
      }
      while (bal_storage_cursor > 0) {
        const StorageId prior_id = bal_storage_order[bal_storage_cursor - 1];
        if (!bal_storage_belongs_to_active(prior_id)) {
          break;
        }
        bal_storage_cursor--;
      }
      bal_stream_phase = BAL_STREAM_STORAGE_READ;
      continue;
    }

    case BAL_STREAM_STORAGE_READ:
      while (bal_storage_cursor < bal_storage_order_n) {
        const StorageId current_storage_id = bal_storage_order[bal_storage_cursor++];
        if (!bal_storage_belongs_to_active(current_storage_id)) {
          bal_storage_cursor--;
          break;
        }
        const BalStorageState *bal = &bal_storage[current_storage_id];
        if (!bal->read || bal->change_head != BAL_NO_HISTORY) {
          continue;
        }
        *account_id = bal_active_account_index;
        *storage_id = current_storage_id;
        return BAL_ITER_STORAGE_READ;
      }
      bal_active_field_history = bal_accounts[bal_active_account_index].balance_head;
      bal_stream_phase = BAL_STREAM_BALANCE;
      continue;

    case BAL_STREAM_BALANCE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const BalHistoryRow *history = &bal_balance_history[bal_active_field_history];
        bal_active_field_history = history->next;
        *account_id = bal_active_account_index;
        *index = history->index;
        *value = le_words4_to_sail_word(history->value.word);
        return BAL_ITER_BALANCE_CHANGE;
      }
      bal_active_field_history = bal_accounts[bal_active_account_index].nonce_head;
      bal_stream_phase = BAL_STREAM_NONCE;
      continue;

    case BAL_STREAM_NONCE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const BalHistoryRow *history = &bal_nonce_histories[bal_active_field_history];
        bal_active_field_history = history->next;
        *account_id = bal_active_account_index;
        *index = history->index;
        *nonce = history->value.nonce;
        return BAL_ITER_NONCE_CHANGE;
      }
      bal_active_field_history = bal_accounts[bal_active_account_index].code_head;
      bal_stream_phase = BAL_STREAM_CODE;
      continue;

    case BAL_STREAM_CODE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const BalHistoryRow *history = &bal_code_histories[bal_active_field_history];
        bal_active_field_history = history->next;
        *account_id = bal_active_account_index;
        *index = history->index;
        *code_hash = history->value.code_hash;
        return BAL_ITER_CODE_CHANGE;
      }
      bal_stream_phase = BAL_STREAM_ACCOUNT_END;
      continue;

    case BAL_STREAM_ACCOUNT_END:
      *account_id = bal_active_account_index;
      bal_active_account_index = BAL_NO_HISTORY;
      bal_stream_phase = BAL_STREAM_ACCOUNT;
      return BAL_ITER_ACCOUNT_END;

    case BAL_STREAM_EMPTY:
      return BAL_ITER_EMPTY;
    }
  }
}
