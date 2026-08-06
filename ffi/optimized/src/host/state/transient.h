/* Private optimized transient-storage interface. The Sail-facing operations
 * retain semantic addresses; rollback uses the already-resolved AccountId. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_TRANSIENT_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_TRANSIENT_H
#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"
#include <stdint.h>

/* Private rollback write: does not append another undo entry. */
void transient_storage_restore(AccountId account_id, U256 slot, U256 v);

#endif
