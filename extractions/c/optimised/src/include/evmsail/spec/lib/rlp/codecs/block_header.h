#pragma once

#include "evmsail/spec/executor/block.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bytes32 block_header_hash_(struct BlockHeader header, bytes32 transactions_root, bytes32 withdrawals_root, bytes32 requests_hash, bytes32 block_access_list_hash);


#ifdef __cplusplus
}
#endif
