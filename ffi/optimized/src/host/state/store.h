/* Optimized semantic-state subsystem.
 *
 * Account, persistent-storage, and BAL representations are independently
 * owned. This aggregate header exposes their interfaces to the few adapters
 * that coordinate more than one subsystem. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_STORE_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_STORE_H

#include "host/state/account.h"
#include "host/state/block_access_list.h"
#include "host/state/storage.h"

#include <stdbool.h>
#include <stdint.h>

/* Merge the active transaction into cumulative block state, emit its BAL
 * history, and discard the transaction overlays. */
unit state_store_tx_merge(struct TransactionMergeSemantics semantics,
                          uint64_t current_transaction_epoch);

#endif
