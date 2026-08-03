/* Optimized semantic-state lifecycle coordinator.
 *
 * Account, storage, and BAL representations are private to their respective
 * modules. This file preserves only the public ordering constraints between
 * those subsystems. */
#include "host/state/internal.h"

void state_store_workspace_bind(void) {
  account_state_workspace_bind();
  storage_state_workspace_bind();
  block_access_list_workspace_bind();
}

unit state_store_tx_merge(struct TransactionMergeSemantics semantics,
                          uint64_t current_transaction_epoch) {
  account_transaction_merge(semantics, current_transaction_epoch);
  storage_transaction_merge(current_transaction_epoch);
  storage_tx_reset(UNIT);
  acct_tx_reset(UNIT);
  return UNIT;
}
