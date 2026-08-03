#ifndef EVMSAIL_OPTIMIZED_EXECUTOR_BLOCK_ACCESS_LIST_H
#define EVMSAIL_OPTIMIZED_EXECUTOR_BLOCK_ACCESS_LIST_H

#include "evmsail/prelude.h"

unit initialize_block_access_list_state(
    struct zStatelessInputSliceFields bytes, Hash32 parent_state_root);
unit validate_block_access_list(
    struct zStatelessInputSliceFields bytes, uint64_t block_gas_limit);

#endif
