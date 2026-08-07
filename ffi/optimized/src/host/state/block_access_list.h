/* Optimized EIP-7928 block-access-list interface.
 *
 * BAL history and canonical iteration order are private to the implementation;
 * entries refer to semantic account and storage state through stable IDs. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_BLOCK_ACCESS_LIST_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_BLOCK_ACCESS_LIST_H

#include "evmsail/prelude.h"
#include "evmsail/host/state/primitives.h"

#include <stdint.h>

void bal_prepare_iter(void);

enum bal_iter_tag {
  BAL_ITER_EMPTY = 0,
  BAL_ITER_ACCOUNT = 1,
  BAL_ITER_STORAGE_CHANGE = 2,
  BAL_ITER_STORAGE_READ = 3,
  BAL_ITER_BALANCE_CHANGE = 4,
  BAL_ITER_NONCE_CHANGE = 5,
  BAL_ITER_CODE_CHANGE = 6,
  BAL_ITER_ACCOUNT_END = 7
};

enum bal_iter_tag bal_iter_next_probe(AccountId *account_id, StorageId *storage_id, uint64_t *index,
                                      u256 *value, uint64_t *nonce, bytes32 *code_hash);

void bal_note_storage_change(uint64_t transaction_epoch, bytes20 address, u256 slot, u256 value);
void bal_note_account_touch(bytes20 address);
void bal_note_storage_read(bytes20 address, u256 slot);
void bal_note_balance_change(uint64_t transaction_epoch, bytes20 address, u256 value);
void bal_note_nonce_change(uint64_t transaction_epoch, bytes20 address, uint64_t nonce);
void bal_note_code_change(uint64_t transaction_epoch, bytes20 address, bytes32 code_hash);

#endif
