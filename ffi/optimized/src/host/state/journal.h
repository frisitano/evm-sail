/* Private append-only journal used by the optimized semantic-state backend.
 *
 * Public frame operations implement the contracts declared in
 * sail/host/state.sail. State modules call the typed push operations before a
 * rollback-visible mutation; only this subsystem owns journal storage and
 * reverse replay. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_JOURNAL_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_JOURNAL_H

#include "host/state/internal.h"

void state_journal_workspace_bind(uint32_t capacity);

void state_journal_push_transient(AccountId account_id, U256 slot, U256 prior);
void state_journal_push_warm_account(AccountId id, uint32_t prior_epoch);
void state_journal_push_warm_storage(StorageId id, uint32_t prior_epoch);
void state_journal_push_account_balance(AccountId id, U256 prior);
void state_journal_push_account_nonce(AccountId id, uint64_t prior);
void state_journal_push_account_code_hash(AccountId id, Hash32 prior);
void state_journal_push_account_exists(AccountId id, bool prior);
void state_journal_push_account_created(AccountId id, bool prior);
void state_journal_push_account_selfdestructed(AccountId id, bool prior);
void state_journal_push_account_transaction_listed(void);
void state_journal_push_account_storage_listed(AccountId id);
void state_journal_push_log_appended(void);
void state_journal_push_storage_value(StorageId id, U256 prior);
void state_journal_push_storage_row_generation(StorageId id, StorageGeneration prior);
void state_journal_push_account_storage_generation(AccountId account_id, StorageGeneration prior);

#endif
