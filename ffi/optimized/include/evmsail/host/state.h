#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/* Public C implementations of the contracts declared by sail/host/state.sail. */
unit transient_storage_reset(unit ignored);
unit transient_storage_write(Address address,
                             U256 slot, U256 value);
U256 transient_storage_read(Address address,
                                 U256 slot);

unit state_journal_reset(unit ignored);
unit state_journal_checkpoint(unit ignored);
unit state_journal_revert(unit ignored);
unit state_journal_commit(unit ignored);

unit storage_tx_clear(Address address);
unit storage_tx_reset(unit ignored);
bool storage_has_writes(Address address);
unit storage_block_clear(Address address);
unit storage_block_iter_begin(Address address);

unit acct_tx_set_balance(Address address,
                         U256 balance);
unit acct_tx_set_nonce(Address address, uint64_t nonce);
unit acct_tx_set_code_hash(Address address,
                           Hash32 code_hash);
unit acct_tx_reset(unit ignored);
unit acct_block_iter_begin(unit ignored);

unit bal_reset(unit ignored);
unit bal_prepare_iter(unit ignored);
unit bal_note_account_touch(Address address);
unit bal_note_storage_change(uint64_t index,
                             Address address,
                             U256 slot, U256 value);
unit bal_note_storage_read(Address address,
                           U256 slot);
unit bal_note_balance_change(uint64_t index,
                             Address address,
                             U256 value);
unit bal_note_nonce_change(uint64_t index,
                           Address address,
                           uint64_t nonce);
unit bal_note_code_change(uint64_t index,
                          Address address,
                          Hash32 code_hash);

unit warm_reset(uint64_t current_transaction_epoch);
bool account_is_warm(Address address);
unit account_mark_warm(Address address);
bool storage_is_warm(Address address, U256 slot);
unit storage_mark_warm(Address address, U256 slot);

unit authorization_tracker_reset(uint64_t count_hint);
bool authorization_tracker_seen(Address authority);
bool authorization_tracker_originally_delegated(
    Address authority);
bool authorization_tracker_delegation_set(
    Address authority);
unit authorization_tracker_commit(Address authority,
                                  bool originally_delegated,
                                  bool sets_delegation);

unit logs_tx_reset(unit ignored);
unit log_begin(Address address);
unit log_add_topic(U256 topic);
unit log_add_data_memory(struct zEvmMemorySliceFields data);
unit log_add_data_word(U256 value);
uint64_t logs_tx_start(unit ignored);
uint64_t logs_tx_count(unit ignored);
Address log_addr(uint64_t index);
uint64_t log_topic_count(uint64_t index);
U256 log_topic(uint64_t log_index, uint64_t topic_index);
uint64_t log_data_off(uint64_t index);
uint64_t log_data_len(uint64_t index);

#ifdef EVMSAIL_NATIVE_DEBUG_AGGREGATES
void acct_block_get(struct zoptionzIRAccountzK *out,
                    Address address);
#endif

void storage_block_iter_next(struct zoptionzIRStorageTrieEntryzK *out,
                             Address address);
void acct_block_iter_next(struct zoptionzIRAcctTrieEntryzK *out, unit ignored);

#endif
