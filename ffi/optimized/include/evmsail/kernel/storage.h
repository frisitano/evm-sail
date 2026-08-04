#ifndef EVMSAIL_OPTIMIZED_KERNEL_STORAGE_H
#define EVMSAIL_OPTIMIZED_KERNEL_STORAGE_H

#include "evmsail/prelude.h"
#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include <stdbool.h>

/* Materializes one authenticated block storage value without recording an
 * EVM/BAL access. The BAL initialization pass uses this to construct the
 * block-local storage table before execution begins. */
bool storage_preload(AccountId account_id, NodeId storage_root_node, U256 slot,
                     StorageId *storage_id, struct StorageValue *value);

struct StorageValue k_sload(
    Hash32 parent_state_root,
    Address address, U256 slot);
unit k_sstore(Address address, U256 slot,
                      U256 current, U256 original);

#endif
