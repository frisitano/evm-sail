#pragma once

#include "evmsail/spec/host/journal.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void transient_storage_reset(void);

void transient_storage_write(bytes20 /* arg_0 */, u256 /* arg_1 */, u256 /* arg_2 */);

u256 transient_storage_read(bytes20 /* arg_0 */, u256 /* arg_1 */);

void storage_tx_clear(bytes20 /* arg_0 */);

void storage_tx_reset(void);

bool storage_has_writes(bytes20 /* arg_0 */);

void storage_block_iter_begin(bytes20 /* arg_0 */);

struct StorageBlockIterResult storage_block_iter_next(bytes20 /* arg_0 */);

void acct_tx_reset(void);

void acct_block_iter_begin(void);

struct AcctBlockIterResult acct_block_iter_next(void);

void bal_reset(void);

void warm_reset(uint32_t /* arg_0 */);

bool account_is_warm(bytes20 /* arg_0 */);

void account_mark_warm(bytes20 /* arg_0 */);

void storage_mark_warm(bytes20 /* arg_0 */, u256 /* arg_1 */);

void authorization_tracker_reset(uint16_t /* arg_0 */);

bool authorization_tracker_seen(bytes20 /* arg_0 */);

bool authorization_tracker_originally_delegated(bytes20 /* arg_0 */);

bool authorization_tracker_delegation_set(bytes20 /* arg_0 */);

void authorization_tracker_commit(bytes20 /* arg_0 */, bool /* arg_1 */, bool /* arg_2 */);

void logs_tx_reset(void);

void log_begin(bytes20 /* arg_0 */);

void log_add_topic(u256 /* arg_0 */);

void log_add_data_memory(Bytes /* arg_0 */);

void log_add_data_word(u256 /* arg_0 */);

uint64_t logs_tx_start(void);

uint64_t logs_tx_count(void);

bytes20 log_addr(uint64_t /* arg_0 */);

uint64_t log_topic_count(uint64_t /* arg_0 */);

u256 log_topic(uint64_t /* arg_0 */, uint64_t /* arg_1 */);


#ifdef __cplusplus
}
#endif
