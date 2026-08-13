#pragma once

#include "evmsail/spec/lib/rlp/codecs/state.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

struct TrieUpdate account_update(struct AcctTrieEntry trie_entry, bytes32 storage_root);

bool account_value_changed(struct AcctValue value);

struct TrieUpdate storage_update(struct StorageTrieEntry trie_entry);

bool storage_value_changed(struct StorageValue value);

struct TrieUpdateFetch next_storage_trie_update(bytes20 addr);

struct TrieUpdateFetch next_changed_account_trie_update(void);

struct tuple_TrieUpdate_bool account_trie_update(struct AcctTrieEntry trie_entry);

bytes32 compute_state_root(void);


#ifdef __cplusplus
}
#endif
