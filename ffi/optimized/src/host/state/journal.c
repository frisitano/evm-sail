/* Append-only optimized state journal.
 *
 * This is the compact C representation of Sail's StateJournalEntry union.
 * BAL loading replaces semantic account/storage keys with stable dense IDs.
 * Each mutation records only the field it changes; whole account and storage
 * rows are deliberately never copied into this arena. Append-only derived
 * stores record their own inverse event, so frame markers carry no cursors. */
#include "host/state/journal.h"

#include "host/state/runtime.h"
#include "host/state/transient.h"
#include "workspace.h"

typedef enum {
  JOURNAL_TRANSIENT_CHANGED = 1,
  JOURNAL_WARM_ACCOUNT_CHANGED = 2,
  JOURNAL_WARM_STORAGE_CHANGED = 3,
  JOURNAL_ACCOUNT_BALANCE_CHANGED = 4,
  JOURNAL_ACCOUNT_NONCE_CHANGED = 5,
  JOURNAL_ACCOUNT_CODE_HASH_CHANGED = 6,
  JOURNAL_ACCOUNT_EXISTS_CHANGED = 7,
  JOURNAL_ACCOUNT_CREATED_CHANGED = 8,
  JOURNAL_ACCOUNT_SELFDESTRUCTED_CHANGED = 9,
  JOURNAL_ACCOUNT_TRANSACTION_LISTED = 10,
  JOURNAL_ACCOUNT_STORAGE_LISTED = 11,
  JOURNAL_LOG_APPENDED = 12,
  JOURNAL_STORAGE_VALUE_CHANGED = 13,
  JOURNAL_STORAGE_ROW_GENERATION_CHANGED = 14,
  JOURNAL_ACCOUNT_STORAGE_GENERATION_CHANGED = 15,
  JOURNAL_FRAME_CHECKPOINTED = 16,
  JOURNAL_FRAME_COMMITTED = 17,
} StateJournalTag;

typedef struct {
  StateJournalTag tag;
  union {
    struct {
      Address address;
      U256 slot;
      U256 prior;
    } transient;
    struct {
      uint32_t id;
      uint32_t prior_epoch;
    } warm;
    struct {
      AccountId account_id;
      U256 prior;
    } account_balance;
    struct {
      AccountId account_id;
      uint64_t prior;
    } account_nonce;
    struct {
      AccountId account_id;
      Hash32 prior;
    } account_code_hash;
    AccountId account_id;
    struct {
      AccountId account_id;
      uint8_t prior;
    } account_flag;
    struct {
      StorageId storage_id;
      U256 prior;
    } storage_value;
    struct {
      StorageId storage_id;
      StorageGeneration prior;
    } storage_row_generation;
    struct {
      AccountId account_id;
      StorageGeneration prior;
    } storage_generation;
  } data;
} StateJournalEntry;

static StateJournalEntry *entries;
static uint32_t count;

void state_journal_workspace_bind(void) {
  WORKSPACE_BIND(entries, GUEST_STATE_JOURNAL_ENTRIES);
}

static StateJournalEntry *push(StateJournalTag tag) {
  if (count == GUEST_STATE_JOURNAL_ENTRIES) GUEST_ABORT();
  StateJournalEntry *entry = &entries[count++];
  entry->tag = tag;
  return entry;
}

unit state_journal_push_transient(Address address, U256 slot, U256 prior) {
  StateJournalEntry *entry = push(JOURNAL_TRANSIENT_CHANGED);
  entry->data.transient.address = address;
  entry->data.transient.slot = slot;
  entry->data.transient.prior = prior;
  return UNIT;
}

void state_journal_push_warm_account(AccountId id, uint32_t prior_epoch) {
  StateJournalEntry *entry = push(JOURNAL_WARM_ACCOUNT_CHANGED);
  entry->data.warm.id = id;
  entry->data.warm.prior_epoch = prior_epoch;
}

void state_journal_push_warm_storage(StorageId id, uint32_t prior_epoch) {
  StateJournalEntry *entry = push(JOURNAL_WARM_STORAGE_CHANGED);
  entry->data.warm.id = id;
  entry->data.warm.prior_epoch = prior_epoch;
}

void state_journal_push_account_balance(AccountId id, U256 prior) {
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_BALANCE_CHANGED);
  entry->data.account_balance.account_id = id;
  entry->data.account_balance.prior = prior;
}

void state_journal_push_account_nonce(AccountId id, uint64_t prior) {
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_NONCE_CHANGED);
  entry->data.account_nonce.account_id = id;
  entry->data.account_nonce.prior = prior;
}

void state_journal_push_account_code_hash(AccountId id, Hash32 prior) {
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_CODE_HASH_CHANGED);
  entry->data.account_code_hash.account_id = id;
  entry->data.account_code_hash.prior = prior;
}

static void push_account_flag(StateJournalTag tag, AccountId id, bool prior) {
  StateJournalEntry *entry = push(tag);
  entry->data.account_flag.account_id = id;
  entry->data.account_flag.prior = prior;
}

void state_journal_push_account_exists(AccountId id, bool prior) {
  push_account_flag(JOURNAL_ACCOUNT_EXISTS_CHANGED, id, prior);
}

void state_journal_push_account_created(AccountId id, bool prior) {
  push_account_flag(JOURNAL_ACCOUNT_CREATED_CHANGED, id, prior);
}

void state_journal_push_account_selfdestructed(AccountId id, bool prior) {
  push_account_flag(JOURNAL_ACCOUNT_SELFDESTRUCTED_CHANGED, id, prior);
}

void state_journal_push_account_transaction_listed(void) {
  (void)push(JOURNAL_ACCOUNT_TRANSACTION_LISTED);
}

void state_journal_push_account_storage_listed(AccountId id) {
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_STORAGE_LISTED);
  entry->data.account_id = id;
}

void state_journal_push_log_appended(void) {
  (void)push(JOURNAL_LOG_APPENDED);
}

void state_journal_push_storage_value(StorageId id, U256 prior) {
  StateJournalEntry *entry = push(JOURNAL_STORAGE_VALUE_CHANGED);
  entry->data.storage_value.storage_id = id;
  entry->data.storage_value.prior = prior;
}

void state_journal_push_storage_row_generation(StorageId id,
                                               StorageGeneration prior) {
  StateJournalEntry *entry = push(JOURNAL_STORAGE_ROW_GENERATION_CHANGED);
  entry->data.storage_row_generation.storage_id = id;
  entry->data.storage_row_generation.prior = prior;
}

void state_journal_push_account_storage_generation(
    AccountId account_id, StorageGeneration prior) {
  StateJournalEntry *entry =
      push(JOURNAL_ACCOUNT_STORAGE_GENERATION_CHANGED);
  entry->data.storage_generation.account_id = account_id;
  entry->data.storage_generation.prior = prior;
}

static void revert_to(uint32_t entry_count) {
  if (entry_count > count) GUEST_ABORT();
  while (count > entry_count) {
    const StateJournalEntry *entry = &entries[--count];
    switch (entry->tag) {
      case JOURNAL_TRANSIENT_CHANGED:
        transient_storage_restore(entry->data.transient.address,
                                  entry->data.transient.slot,
                                  entry->data.transient.prior);
        break;
      case JOURNAL_WARM_ACCOUNT_CHANGED:
        account_warm_restore((AccountId)entry->data.warm.id,
                             entry->data.warm.prior_epoch);
        break;
      case JOURNAL_WARM_STORAGE_CHANGED:
        storage_warm_restore((StorageId)entry->data.warm.id,
                             entry->data.warm.prior_epoch);
        break;
      case JOURNAL_ACCOUNT_BALANCE_CHANGED:
        account_balance_restore(entry->data.account_balance.account_id,
                                entry->data.account_balance.prior);
        break;
      case JOURNAL_ACCOUNT_NONCE_CHANGED:
        account_nonce_restore(entry->data.account_nonce.account_id,
                              entry->data.account_nonce.prior);
        break;
      case JOURNAL_ACCOUNT_CODE_HASH_CHANGED:
        account_code_hash_restore(entry->data.account_code_hash.account_id,
                                  entry->data.account_code_hash.prior);
        break;
      case JOURNAL_ACCOUNT_EXISTS_CHANGED:
        account_exists_restore(entry->data.account_flag.account_id,
                               entry->data.account_flag.prior);
        break;
      case JOURNAL_ACCOUNT_CREATED_CHANGED:
        account_created_restore(entry->data.account_flag.account_id,
                                entry->data.account_flag.prior);
        break;
      case JOURNAL_ACCOUNT_SELFDESTRUCTED_CHANGED:
        account_selfdestructed_restore(entry->data.account_flag.account_id,
                                       entry->data.account_flag.prior);
        break;
      case JOURNAL_ACCOUNT_TRANSACTION_LISTED:
        account_transaction_pop_last();
        break;
      case JOURNAL_ACCOUNT_STORAGE_LISTED:
        account_transaction_storage_pop_last(entry->data.account_id);
        break;
      case JOURNAL_LOG_APPENDED:
        logs_revert_last();
        break;
      case JOURNAL_STORAGE_VALUE_CHANGED:
        storage_value_restore(entry->data.storage_value.storage_id,
                              entry->data.storage_value.prior);
        break;
      case JOURNAL_STORAGE_ROW_GENERATION_CHANGED:
        storage_generation_restore(
            entry->data.storage_row_generation.storage_id,
            entry->data.storage_row_generation.prior);
        break;
      case JOURNAL_ACCOUNT_STORAGE_GENERATION_CHANGED:
        account_storage_generation_restore(
            entry->data.storage_generation.account_id,
            entry->data.storage_generation.prior);
        break;
      case JOURNAL_FRAME_CHECKPOINTED:
      case JOURNAL_FRAME_COMMITTED:
        /* Structural records have no semantic inverse. */
        break;
      default:
        GUEST_ABORT();
    }
  }
}

unit state_journal_reset(unit ignored) {
  (void)ignored;
  count = 0;
  return UNIT;
}

/* Find the innermost checkpoint not paired with a later commit marker.
 * Walking the tag stream is the journal's defunctionalized control flow:
 * committed child pairs are skipped, so no checkpoint identity needs to cross
 * the Sail/C boundary or live in a suspended frame. */
static uint32_t open_checkpoint_index(void) {
  uint32_t closed = 0;
  for (uint32_t i = count; i > 0; --i) {
    switch (entries[i - 1].tag) {
      case JOURNAL_FRAME_COMMITTED:
        closed++;
        break;
      case JOURNAL_FRAME_CHECKPOINTED:
        if (closed == 0) return i - 1;
        closed--;
        break;
      default:
        break;
    }
  }
  GUEST_ABORT();
}

unit state_journal_checkpoint(unit ignored) {
  (void)ignored;
  (void)push(JOURNAL_FRAME_CHECKPOINTED);
  return UNIT;
}

unit state_journal_revert(unit ignored) {
  (void)ignored;
  const uint32_t marker_index = open_checkpoint_index();
  revert_to(marker_index + 1u);
  count = marker_index;
  return UNIT;
}

unit state_journal_commit(unit ignored) {
  (void)ignored;
  (void)open_checkpoint_index();
  (void)push(JOURNAL_FRAME_COMMITTED);
  return UNIT;
}
