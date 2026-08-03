/* Optimized transaction-lifecycle boundary. */
#include "evmsail/prelude.h"

#include "host/state/store.h"
#include "evmsail/kernel/lifecycle.h"

unit tx_merge(struct zTransactionMergeSemantics semantics,
              uint64_t current_transaction_epoch) {
  return state_store_tx_merge(semantics, current_transaction_epoch);
}
