/* Optimized transaction-lifecycle boundary.
 *
 * Account, storage, and BAL representations are private to their respective
 * modules; this file preserves only the ordering constraints between them. */

#include "evmsail/kernel/lifecycle.h"
#include "evmsail/spec/kernel/lifecycle.h"
#include "evmsail/spec/host/state.h"
#include "host/state/internal.h"
#include <stdint.h>

void tx_merge(struct TransactionMergeSemantics semantics, uint32_t current_transaction_epoch)
{
  account_transaction_merge(semantics, current_transaction_epoch);
  storage_transaction_merge(current_transaction_epoch);
  storage_tx_reset();
  acct_tx_reset();
}
