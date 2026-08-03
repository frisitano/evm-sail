#ifndef EVMSAIL_OPTIMIZED_KERNEL_ACCOUNTS_H
#define EVMSAIL_OPTIMIZED_KERNEL_ACCOUNTS_H

#include "evmsail/prelude.h"
#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include <stdbool.h>

/* Materializes one authenticated block account into the AccountId assigned by
 * BAL schema construction, without recording an EVM/BAL access. */
bool account_preload(Hash32 parent_state_root, AccountId account_id,
                     Hash32 address_hash, struct zAccount *account,
                     NodeId *storage_root_node);

struct zAccount k_aload(
    Hash32 parent_state_root,
    Address address);
unit store_account(Address address,
                           struct zAccount account);
unit store_account_info(Address address,
                                struct zAccount account,
                                struct zAccountInfo info);

#endif
