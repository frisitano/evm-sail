#pragma once

#include "evmsail/spec/lib/mpt/trie.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

Bytes encode_state_account(struct AccountInfo info, bytes32 storage_root);

Bytes encode_storage_value(u256 value);


#ifdef __cplusplus
}
#endif
