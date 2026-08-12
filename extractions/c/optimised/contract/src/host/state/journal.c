/* Append-only optimized state journal.
 *
 * This is the compact C representation of Sail's StateJournalEntry union.
 * BAL loading replaces semantic account/storage keys with stable dense IDs.
 * Each mutation records only the field it changes; whole account and storage
 * rows are deliberately never copied into this arena. Append-only derived
 * stores record their own inverse event, so frame markers carry no cursors. */
#include "host/state/journal.h"

#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"
#include "host/state/internal.h"
#include "host/state/logs.h"
#include "host/state/transient.h"
#include "workspace.h"
#include <stdint.h>

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
      AccountId account_id;
      u256 slot;
      u256 prior;
    } transient;
    struct {
      uint32_t id;
      uint32_t prior_epoch;
    } warm;
    struct {
      AccountId account_id;
      AccountRestorePrior prior;
    } account_field;
    AccountId account_id;
    struct {
      StorageId storage_id;
      u256 prior;
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
static uint32_t capacity;

void state_journal_workspace_bind(uint32_t entry_capacity)
{
  capacity = entry_capacity;
  WORKSPACE_BIND(entries, capacity);
}

static StateJournalEntry *push(StateJournalTag tag)
{
  if (count >= capacity) {
    GUEST_ABORT();
  }
  StateJournalEntry *entry = &entries[count++];
  entry->tag = tag;
  return entry;
}

void state_journal_push_transient(AccountId account_id, u256 slot, u256 prior)
{
  StateJournalEntry *entry = push(JOURNAL_TRANSIENT_CHANGED);
  entry->data.transient.account_id = account_id;
  entry->data.transient.slot = slot;
  entry->data.transient.prior = prior;
}

void state_journal_push_warm_account(AccountId id, uint32_t prior_epoch)
{
  StateJournalEntry *entry = push(JOURNAL_WARM_ACCOUNT_CHANGED);
  entry->data.warm.id = id;
  entry->data.warm.prior_epoch = prior_epoch;
}

void state_journal_push_warm_storage(StorageId id, uint32_t prior_epoch)
{
  StateJournalEntry *entry = push(JOURNAL_WARM_STORAGE_CHANGED);
  entry->data.warm.id = id;
  entry->data.warm.prior_epoch = prior_epoch;
}

void state_journal_push_account_balance(AccountId id, u256 prior)
{
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_BALANCE_CHANGED);
  entry->data.account_field.account_id = id;
  entry->data.account_field.prior.balance = prior;
}

void state_journal_push_account_nonce(AccountId id, uint64_t prior)
{
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_NONCE_CHANGED);
  entry->data.account_field.account_id = id;
  entry->data.account_field.prior.nonce = prior;
}

void state_journal_push_account_code_hash(AccountId id, bytes32 prior)
{
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_CODE_HASH_CHANGED);
  entry->data.account_field.account_id = id;
  entry->data.account_field.prior.code_hash = prior;
}

static void push_account_flag(StateJournalTag tag, AccountId id, bool prior)
{
  StateJournalEntry *entry = push(tag);
  entry->data.account_field.account_id = id;
  entry->data.account_field.prior.flag = prior;
}

void state_journal_push_account_exists(AccountId id, bool prior)
{
  push_account_flag(JOURNAL_ACCOUNT_EXISTS_CHANGED, id, prior);
}

void state_journal_push_account_created(AccountId id, bool prior)
{
  push_account_flag(JOURNAL_ACCOUNT_CREATED_CHANGED, id, prior);
}

void state_journal_push_account_selfdestructed(AccountId id, bool prior)
{
  push_account_flag(JOURNAL_ACCOUNT_SELFDESTRUCTED_CHANGED, id, prior);
}

void state_journal_push_account_transaction_listed(void)
{
  (void)push(JOURNAL_ACCOUNT_TRANSACTION_LISTED);
}

void state_journal_push_account_storage_listed(AccountId id)
{
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_STORAGE_LISTED);
  entry->data.account_id = id;
}

void state_journal_push_log_appended(void)
{
  (void)push(JOURNAL_LOG_APPENDED);
}

void state_journal_push_storage_value(StorageId id, u256 prior)
{
  StateJournalEntry *entry = push(JOURNAL_STORAGE_VALUE_CHANGED);
  entry->data.storage_value.storage_id = id;
  entry->data.storage_value.prior = prior;
}

void state_journal_push_storage_row_generation(StorageId id, StorageGeneration prior)
{
  StateJournalEntry *entry = push(JOURNAL_STORAGE_ROW_GENERATION_CHANGED);
  entry->data.storage_row_generation.storage_id = id;
  entry->data.storage_row_generation.prior = prior;
}

void state_journal_push_account_storage_generation(AccountId account_id, StorageGeneration prior)
{
  StateJournalEntry *entry = push(JOURNAL_ACCOUNT_STORAGE_GENERATION_CHANGED);
  entry->data.storage_generation.account_id = account_id;
  entry->data.storage_generation.prior = prior;
}

void state_journal_revert(void)
{
  uint32_t committed_checkpoints = 0;

  while (count > 0) {
    const StateJournalEntry *entry = &entries[--count];
    switch (entry->tag) {
    case JOURNAL_TRANSIENT_CHANGED:
      transient_storage_restore(entry->data.transient.account_id, entry->data.transient.slot,
                                entry->data.transient.prior);
      break;
    case JOURNAL_WARM_ACCOUNT_CHANGED:
      account_warm_restore((AccountId)entry->data.warm.id, entry->data.warm.prior_epoch);
      break;
    case JOURNAL_WARM_STORAGE_CHANGED:
      storage_warm_restore((StorageId)entry->data.warm.id, entry->data.warm.prior_epoch);
      break;
    case JOURNAL_ACCOUNT_BALANCE_CHANGED:
      account_field_restore(entry->data.account_field.account_id, ACCOUNT_FIELD_BALANCE,
                            entry->data.account_field.prior);
      break;
    case JOURNAL_ACCOUNT_NONCE_CHANGED:
      account_field_restore(entry->data.account_field.account_id, ACCOUNT_FIELD_NONCE,
                            entry->data.account_field.prior);
      break;
    case JOURNAL_ACCOUNT_CODE_HASH_CHANGED:
      account_field_restore(entry->data.account_field.account_id, ACCOUNT_FIELD_CODE_HASH,
                            entry->data.account_field.prior);
      break;
    case JOURNAL_ACCOUNT_EXISTS_CHANGED:
      account_field_restore(entry->data.account_field.account_id, ACCOUNT_FIELD_EXISTS,
                            entry->data.account_field.prior);
      break;
    case JOURNAL_ACCOUNT_CREATED_CHANGED:
      account_field_restore(entry->data.account_field.account_id, ACCOUNT_FIELD_CREATED,
                            entry->data.account_field.prior);
      break;
    case JOURNAL_ACCOUNT_SELFDESTRUCTED_CHANGED:
      account_field_restore(entry->data.account_field.account_id, ACCOUNT_FIELD_SELFDESTRUCTED,
                            entry->data.account_field.prior);
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
      storage_value_restore(entry->data.storage_value.storage_id, entry->data.storage_value.prior);
      break;
    case JOURNAL_STORAGE_ROW_GENERATION_CHANGED:
      storage_generation_restore(entry->data.storage_row_generation.storage_id,
                                 entry->data.storage_row_generation.prior);
      break;
    case JOURNAL_ACCOUNT_STORAGE_GENERATION_CHANGED:
      account_storage_generation_restore(entry->data.storage_generation.account_id,
                                         entry->data.storage_generation.prior);
      break;
    case JOURNAL_FRAME_CHECKPOINTED:
      if (committed_checkpoints == 0) {
        return;
      }
      committed_checkpoints--;
      break;
    case JOURNAL_FRAME_COMMITTED:
      committed_checkpoints++;
      break;
    }
  }

  GUEST_ABORT();
}

void state_journal_reset(void)
{
  count = 0;
}

void state_journal_checkpoint(void)
{
  (void)push(JOURNAL_FRAME_CHECKPOINTED);
}

void state_journal_commit(void)
{
  /* Commit is O(1); checkpoint/commit pairing is enforced by the Sail frame
   * lifecycle. */
  (void)push(JOURNAL_FRAME_COMMITTED);
}
