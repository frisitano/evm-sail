#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/* Public C implementations of the contracts declared by sail/host/state.sail. */
void transient_storage_reset(void);
void transient_storage_write(bytes20 address, u256 slot, u256 value);
u256 transient_storage_read(bytes20 address, u256 slot);

void state_journal_reset(void);
void state_journal_checkpoint(void);
void state_journal_revert(void);
void state_journal_commit(void);

void storage_tx_clear(bytes20 address);
void storage_tx_reset(void);
bool storage_has_writes(bytes20 address);
void storage_block_clear(bytes20 address);

void acct_tx_set_balance(bytes20 address, u256 balance);
void acct_tx_set_nonce(bytes20 address, uint64_t nonce);
void acct_tx_set_code_hash(bytes20 address, bytes32 code_hash);
void acct_tx_reset(void);

void bal_reset(void);
void bal_prepare_iter(void);
void bal_note_account_touch(bytes20 address);
void bal_note_storage_change(uint64_t index, bytes20 address, u256 slot, u256 value);
void bal_note_storage_read(bytes20 address, u256 slot);
void bal_note_balance_change(uint64_t index, bytes20 address, u256 value);
void bal_note_nonce_change(uint64_t index, bytes20 address, uint64_t nonce);
void bal_note_code_change(uint64_t index, bytes20 address, bytes32 code_hash);

void warm_reset(uint32_t current_transaction_epoch);
bool account_is_warm(bytes20 address);
void account_mark_warm(bytes20 address);
void storage_mark_warm(bytes20 address, u256 slot);

void authorization_tracker_reset(uint16_t count_hint);
bool authorization_tracker_seen(bytes20 authority);
bool authorization_tracker_originally_delegated(bytes20 authority);
bool authorization_tracker_delegation_set(bytes20 authority);
void authorization_tracker_commit(bytes20 authority, bool originally_delegated,
                                  bool sets_delegation);

void logs_tx_reset(void);
void log_begin(bytes20 address);
void log_add_topic(u256 topic);
void log_add_data_memory(Bytes data);
void log_add_data_word(u256 value);
uint64_t logs_tx_start(void);
uint64_t logs_tx_count(void);
bytes20 log_addr(uint64_t index);
uint64_t log_topic_count(uint64_t index);
u256 log_topic(uint64_t log_index, uint64_t topic_index);
uint32_t log_data_off(uint64_t index);
uint32_t log_data_len(uint64_t index);
Bytes host_log_data_slice(uint64_t index);

#endif
