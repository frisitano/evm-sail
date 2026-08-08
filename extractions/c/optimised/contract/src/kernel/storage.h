/* Private optimized kernel storage operations. */
#ifndef EVMSAIL_OPTIMIZED_KERNEL_STORAGE_PRIVATE_H
#define EVMSAIL_OPTIMIZED_KERNEL_STORAGE_PRIVATE_H

#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"

struct StorageValue storage_load_by_id(bytes32 parent_state_root, bytes20 address, u256 slot,
                                       StorageId storage_id);

#endif
