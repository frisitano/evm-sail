/* Private optimized kernel storage operations. */
#ifndef EVMSAIL_OPTIMIZED_KERNEL_STORAGE_PRIVATE_H
#define EVMSAIL_OPTIMIZED_KERNEL_STORAGE_PRIVATE_H

#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"

struct StorageValue storage_load_by_id(AccountId account_id, StorageGeneration generation,
                                       StorageId storage_id, bool mark_warm);

#endif
