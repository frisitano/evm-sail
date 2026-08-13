#pragma once

#include "evmsail/spec/lib/rlp/codecs/block_access_list.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void validate_block_access_list_(Bytes bytes, uint64_t block_gas_limit);


#ifdef __cplusplus
}
#endif
