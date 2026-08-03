/* Optimized EIP-7928 block access list state.
 *
 * BAL histories and iteration order are owned here. Entries refer to account
 * and storage state solely through stable dense IDs; no BAL metadata is stored
 * in semantic account or storage rows. */
#include "host/state/internal.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int storage_key_cmp(const Address *aa, const U256 *as,
                           const Address *ba, const U256 *bs) {
  const int address_order = address_compare(aa, ba);
  return address_order != 0 ? address_order : word_compare(as, bs);
}

#define BAL_NO_HISTORY UINT32_MAX

typedef struct {
  uint32_t index;
  uint32_t next;
  uint64_t value[4];
} BalWordHistory;

typedef struct {
  uint32_t index;
  uint32_t next;
  uint64_t value;
} BalNonceHistory;

typedef struct {
  uint32_t index;
  uint32_t next;
  Hash32 value;
} BalCodeHistory;

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

static BalWordHistory *bal_storage_history;
static uint32_t bal_storage_history_n = 0;
static BalWordHistory *bal_balance_history;
static uint32_t bal_balance_history_n = 0;
static BalNonceHistory *bal_nonce_histories;
static uint32_t bal_nonce_history_n = 0;
static BalCodeHistory *bal_code_histories;
static uint32_t bal_code_history_n = 0;
static BalAccountState *bal_accounts;
static BalStorageState *bal_storage;
/* Separates independent block executions. Transaction ordering within one BAL
 * is carried by each history row's protocol transaction epoch. */
static uint64_t block_epoch = 1;
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

void block_access_list_workspace_bind(void) {
  WORKSPACE_BIND(bal_storage_history, GUEST_BAL_STORAGE_CHANGES);
  WORKSPACE_BIND(bal_balance_history, GUEST_BAL_BALANCE_CHANGES);
  WORKSPACE_BIND(bal_nonce_histories, GUEST_BAL_NONCE_CHANGES);
  WORKSPACE_BIND(bal_code_histories, GUEST_BAL_CODE_CHANGES);
  /* AccountId zero is reserved for failed BAL-schema lookups. */
  WORKSPACE_BIND(bal_accounts, GUEST_STATE_ACCOUNTS + 1u);
  WORKSPACE_BIND(bal_storage, GUEST_STATE_STORAGE);
  WORKSPACE_BIND(bal_account_order, GUEST_STATE_ACCOUNTS);
  WORKSPACE_BIND(bal_storage_order, GUEST_STATE_STORAGE);
}

static AccountId bal_account_activate(const Address *address) {
  if (bal_prepared) GUEST_ABORT();
  const AccountId id = get_account_id(address);
  if (have_exception) return ACCOUNT_ID_NONE;
  BalAccountState *account = &bal_accounts[id];
  if (account->block_epoch != block_epoch) {
    account->block_epoch = block_epoch;
    account->touched = 0;
    account->balance_head = account->balance_tail = BAL_NO_HISTORY;
    account->nonce_head = account->nonce_tail = BAL_NO_HISTORY;
    account->code_head = account->code_tail = BAL_NO_HISTORY;
    if (bal_account_order_n >= GUEST_STATE_ACCOUNTS) GUEST_ABORT();
    bal_account_order[bal_account_order_n++] = id;
  }
  account->touched = 1;
  return id;
}

static StorageId bal_storage_activate(
    AccountId account_id, const Address *address, const U256 *slot) {
  (void)address;
  if (account_id == ACCOUNT_ID_NONE) return BAL_NO_HISTORY;
  const StorageId storage_id = get_storage_id(account_id, slot);
  if (have_exception) return BAL_NO_HISTORY;
  BalStorageState *bal = &bal_storage[storage_id];
  if (bal->block_epoch != block_epoch) {
    bal->block_epoch = block_epoch;
    bal->account_id = account_id;
    bal->read = 0;
    bal->change_head = bal->change_tail = BAL_NO_HISTORY;
    if (bal_storage_order_n >= GUEST_STATE_STORAGE) GUEST_ABORT();
    bal_storage_order[bal_storage_order_n++] = storage_id;
  } else if (bal->account_id != account_id)
    GUEST_ABORT();
  return storage_id;
}

static void bal_append_word(BalWordHistory *rows, uint32_t *length,
                            uint32_t capacity, uint32_t *head,
                            uint32_t *tail, uint32_t transaction_epoch,
                            const uint64_t value[4]) {
  if (*tail != BAL_NO_HISTORY) {
    BalWordHistory *last = &rows[*tail];
    if (last->index == transaction_epoch) {
      memcpy(last->value, value, sizeof(last->value));
      return;
    }
    if (last->index > transaction_epoch) GUEST_ABORT();
  }
  if (*length >= capacity) GUEST_ABORT();
  const uint32_t position = (*length)++;
  BalWordHistory *entry = &rows[position];
  entry->index = transaction_epoch;
  entry->next = BAL_NO_HISTORY;
  memcpy(entry->value, value, sizeof(entry->value));
  if (*tail == BAL_NO_HISTORY)
    *head = position;
  else
    rows[*tail].next = position;
  *tail = position;
}

static void bal_append_nonce(uint32_t *head, uint32_t *tail,
                             uint32_t transaction_epoch, uint64_t value) {
  if (*tail != BAL_NO_HISTORY) {
    BalNonceHistory *last = &bal_nonce_histories[*tail];
    if (last->index == transaction_epoch) {
      last->value = value;
      return;
    }
    if (last->index > transaction_epoch) GUEST_ABORT();
  }
  if (bal_nonce_history_n >= GUEST_BAL_NONCE_CHANGES) GUEST_ABORT();
  const uint32_t position = bal_nonce_history_n++;
  BalNonceHistory *entry = &bal_nonce_histories[position];
  entry->index = transaction_epoch;
  entry->next = BAL_NO_HISTORY;
  entry->value = value;
  if (*tail == BAL_NO_HISTORY)
    *head = position;
  else
    bal_nonce_histories[*tail].next = position;
  *tail = position;
}

static void bal_append_code(uint32_t *head, uint32_t *tail,
                            uint32_t transaction_epoch,
                            const Hash32 *value) {
  if (*tail != BAL_NO_HISTORY) {
    BalCodeHistory *last = &bal_code_histories[*tail];
    if (last->index == transaction_epoch) {
      last->value = *value;
      return;
    }
    if (last->index > transaction_epoch) GUEST_ABORT();
  }
  if (bal_code_history_n >= GUEST_BAL_CODE_CHANGES) GUEST_ABORT();
  const uint32_t position = bal_code_history_n++;
  BalCodeHistory *entry = &bal_code_histories[position];
  entry->index = transaction_epoch;
  entry->next = BAL_NO_HISTORY;
  entry->value = *value;
  if (*tail == BAL_NO_HISTORY)
    *head = position;
  else
    bal_code_histories[*tail].next = position;
  *tail = position;
}

unit bal_note_account_touch(Address a) {
  (void)bal_account_activate(&a);
  return UNIT;
}

unit bal_note_storage_change(uint64_t transaction_epoch, Address a,
                             const U256 slot,
                             const U256 value) {
  uint64_t value_words[4];
  sail_word_to_be_words4(value_words, (value));
  const AccountId account_id = bal_account_activate(&a);
  if (account_id == ACCOUNT_ID_NONE) return UNIT;
  const StorageId storage_id =
      bal_storage_activate(account_id, &a, &slot);
  if (storage_id == BAL_NO_HISTORY) return UNIT;
  BalStorageState *bal = &bal_storage[storage_id];
  bal_append_word(bal_storage_history, &bal_storage_history_n,
                  GUEST_BAL_STORAGE_CHANGES, &bal->change_head,
                  &bal->change_tail, (uint32_t)transaction_epoch, value_words);
  return UNIT;
}

unit bal_note_storage_read(Address a, const U256 slot) {
  const AccountId account_id = bal_account_activate(&a);
  if (account_id == ACCOUNT_ID_NONE) return UNIT;
  const StorageId storage_id =
      bal_storage_activate(account_id, &a, &slot);
  if (storage_id == BAL_NO_HISTORY) return UNIT;
  bal_storage[storage_id].read = 1;
  return UNIT;
}

unit bal_note_balance_change(uint64_t transaction_epoch, Address a,
                             const U256 value) {
  uint64_t value_words[4];
  sail_word_to_le_words4(value_words, (value));
  const AccountId account_id = bal_account_activate(&a);
  if (account_id == ACCOUNT_ID_NONE) return UNIT;
  BalAccountState *account = &bal_accounts[account_id];
  bal_append_word(bal_balance_history, &bal_balance_history_n,
                  GUEST_BAL_BALANCE_CHANGES, &account->balance_head,
                  &account->balance_tail, (uint32_t)transaction_epoch,
                  value_words);
  return UNIT;
}

unit bal_note_nonce_change(uint64_t transaction_epoch, Address a,
                           uint64_t nonce) {
  const AccountId account_id = bal_account_activate(&a);
  if (account_id == ACCOUNT_ID_NONE) return UNIT;
  BalAccountState *account = &bal_accounts[account_id];
  bal_append_nonce(&account->nonce_head, &account->nonce_tail,
                   (uint32_t)transaction_epoch, nonce);
  return UNIT;
}

unit bal_note_code_change(uint64_t transaction_epoch, Address a,
                          Hash32 code_hash) {
  const AccountId account_id = bal_account_activate(&a);
  if (account_id == ACCOUNT_ID_NONE) return UNIT;
  BalAccountState *account = &bal_accounts[account_id];
  bal_append_code(&account->code_head, &account->code_tail,
                  (uint32_t)transaction_epoch, &code_hash);
  return UNIT;
}

static int bal_account_order_compare(AccountId a, AccountId b) {
  return address_compare(account_id_address(a), account_id_address(b));
}

static int bal_storage_order_compare(StorageId a, StorageId b) {
  const Address *left_address =
      account_id_address(bal_storage[a].account_id);
  const Address *right_address =
      account_id_address(bal_storage[b].account_id);
  int comparison = address_compare(left_address, right_address);
  if (comparison) return comparison;
  return word_compare(storage_id_slot(a), storage_id_slot(b));
}

enum BalOrderKind {
  BAL_ORDER_ACCOUNT,
  BAL_ORDER_STORAGE,
};

static int bal_order_compare(enum BalOrderKind kind, uint32_t left,
                             uint32_t right) {
  return kind == BAL_ORDER_ACCOUNT
             ? bal_account_order_compare(left, right)
             : bal_storage_order_compare(left, right);
}

static void sort_index_order(uint32_t *order, uint32_t length,
                             enum BalOrderKind kind) {
  for (uint32_t i = 1; i < length; i++) {
    const uint32_t candidate = order[i];
    uint32_t position = i;
    while (position != 0 &&
           bal_order_compare(kind, candidate, order[position - 1]) < 0) {
      order[position] = order[position - 1];
      position--;
    }
    order[position] = candidate;
  }
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
  if (block_epoch == UINT64_MAX) {
    for (AccountId i = 1u; i < account_id_count(); i++)
      bal_accounts[i].block_epoch = 0;
    for (uint32_t i = 0; i < storage_id_count(); i++)
      bal_storage[i].block_epoch = 0;
    block_epoch = 1;
  } else {
    block_epoch++;
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
  sort_index_order(bal_account_order, bal_account_order_n,
                   BAL_ORDER_ACCOUNT);
  sort_index_order(bal_storage_order, bal_storage_order_n,
                   BAL_ORDER_STORAGE);
  for (uint32_t i = 0; i < bal_account_order_n; i++) {
    const AccountId index = bal_account_order[i];
    if (index == ACCOUNT_ID_NONE || index >= account_id_count()) GUEST_ABORT();
    const BalAccountState *current = &bal_accounts[index];
    if (current->block_epoch != block_epoch || !current->touched)
      GUEST_ABORT();
    if (i > 0) {
      if (address_compare(account_id_address(bal_account_order[i - 1]),
                          account_id_address(index)) >= 0)
        GUEST_ABORT();
    }
  }
  for (uint32_t i = 0; i < bal_storage_order_n; i++) {
    const StorageId index = bal_storage_order[i];
    if (index >= storage_id_count()) GUEST_ABORT();
    if (bal_storage[index].block_epoch != block_epoch) GUEST_ABORT();
    if (i > 0) {
      const StorageId prior = bal_storage_order[i - 1];
      if (storage_key_cmp(
              account_id_address(bal_storage[prior].account_id),
              storage_id_slot(prior),
              account_id_address(bal_storage[index].account_id),
              storage_id_slot(index)) >= 0)
        GUEST_ABORT();
    }
  }
  bal_prepared = 1;
  bal_reset_cursors();
  return UNIT;
}

static const Address *bal_active_account(void) {
  if (bal_active_account_index == BAL_NO_HISTORY) return NULL;
  return account_id_address(bal_active_account_index);
}

static int bal_storage_belongs_to_active(StorageId storage_id) {
  return bal_active_account_index != BAL_NO_HISTORY &&
         storage_id < storage_id_count() &&
         bal_storage[storage_id].block_epoch == block_epoch &&
         bal_storage[storage_id].account_id == bal_active_account_index;
}

enum bal_iter_tag bal_iter_next_probe(AccountId *account_id,
                                      StorageId *storage_id,
                                      uint64_t *index, U256 *value,
                                      uint64_t *nonce, Hash32 *code_hash) {
  if (!bal_prepared) (void)bal_prepare_iter(UNIT);
  for (;;) {
    const Address *account = bal_active_account();
    switch (bal_stream_phase) {
    case BAL_STREAM_ACCOUNT: {
      if (bal_account_cursor >= bal_account_order_n) {
        bal_stream_phase = BAL_STREAM_EMPTY;
        return BAL_ITER_EMPTY;
      }
      bal_active_account_index = bal_account_order[bal_account_cursor++];
      account = bal_active_account();
      while (bal_storage_cursor < bal_storage_order_n) {
        const StorageId candidate_id =
            bal_storage_order[bal_storage_cursor];
        const Address *candidate = account_id_address(
            bal_storage[candidate_id].account_id);
        if (address_compare(candidate, account) >= 0)
          break;
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
        const BalWordHistory *history =
            &bal_storage_history[bal_active_storage_history];
        const StorageId current_storage_id =
            bal_storage_order[bal_storage_cursor];
        bal_active_storage_history = history->next;
        *account_id = bal_active_account_index;
        *storage_id = current_storage_id;
        *index = history->index;
        *value = be_words4_to_sail_word(history->value);
        if (bal_active_storage_history == BAL_NO_HISTORY)
          bal_storage_cursor++;
        return BAL_ITER_STORAGE_CHANGE;
      }
      while (bal_storage_cursor < bal_storage_order_n) {
        const StorageId storage_id =
            bal_storage_order[bal_storage_cursor];
        if (!bal_storage_belongs_to_active(storage_id)) break;
        if (bal_storage[storage_id].change_head != BAL_NO_HISTORY) {
          bal_active_storage_history =
              bal_storage[storage_id].change_head;
          break;
        }
        bal_storage_cursor++;
      }
      if (bal_active_storage_history != BAL_NO_HISTORY) continue;
      while (bal_storage_cursor > 0) {
        const StorageId prior_id =
            bal_storage_order[bal_storage_cursor - 1];
        if (!bal_storage_belongs_to_active(prior_id)) break;
        bal_storage_cursor--;
      }
      bal_stream_phase = BAL_STREAM_STORAGE_READ;
      continue;
    }

    case BAL_STREAM_STORAGE_READ:
      while (bal_storage_cursor < bal_storage_order_n) {
        const StorageId current_storage_id =
            bal_storage_order[bal_storage_cursor++];
        if (!bal_storage_belongs_to_active(current_storage_id)) {
          bal_storage_cursor--;
          break;
        }
        const BalStorageState *bal = &bal_storage[current_storage_id];
        if (!bal->read || bal->change_head != BAL_NO_HISTORY)
          continue;
        *account_id = bal_active_account_index;
        *storage_id = current_storage_id;
        return BAL_ITER_STORAGE_READ;
      }
      bal_active_field_history =
          bal_accounts[bal_active_account_index].balance_head;
      bal_stream_phase = BAL_STREAM_BALANCE;
      continue;

    case BAL_STREAM_BALANCE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const BalWordHistory *history =
            &bal_balance_history[bal_active_field_history];
        bal_active_field_history = history->next;
        *account_id = bal_active_account_index;
        *index = history->index;
        *value = le_words4_to_sail_word(history->value);
        return BAL_ITER_BALANCE_CHANGE;
      }
      bal_active_field_history =
          bal_accounts[bal_active_account_index].nonce_head;
      bal_stream_phase = BAL_STREAM_NONCE;
      continue;

    case BAL_STREAM_NONCE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const BalNonceHistory *history =
            &bal_nonce_histories[bal_active_field_history];
        bal_active_field_history = history->next;
        *account_id = bal_active_account_index;
        *index = history->index;
        *nonce = history->value;
        return BAL_ITER_NONCE_CHANGE;
      }
      bal_active_field_history =
          bal_accounts[bal_active_account_index].code_head;
      bal_stream_phase = BAL_STREAM_CODE;
      continue;

    case BAL_STREAM_CODE:
      if (bal_active_field_history != BAL_NO_HISTORY) {
        const BalCodeHistory *history =
            &bal_code_histories[bal_active_field_history];
        bal_active_field_history = history->next;
        *account_id = bal_active_account_index;
        *index = history->index;
        *code_hash = history->value;
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
