/* Optimized persistent-storage interface.
 *
 * The implementation owns storage schema, direct semantic values, and
 * storage-trie authentication metadata. Rollback is coordinated by the
 * shared state journal. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_STORAGE_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_STORAGE_H

#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

struct StorageEntry;

void storage_block_clear(bytes20 address);

/* Minimal borrowed view used while reducing one live storage-trie binding.
 * The slot identity remains module-owned and outside the MPT hot path.
 * Mutations stay behind the semantic setters above so generation and journal
 * invariants remain owned by this module. */
typedef struct {
  const u256 *current;
  const u256 *original;
  const bytes32 *secure_key;
  NodeId terminal_node;
  bool prestate_exists;
} StorageTrieView;

bool storage_trie_binding_live(StorageId storage_id, StorageGeneration generation);
NodeId storage_trie_binding_terminal_node(StorageId storage_id);
const bytes32 *storage_trie_binding_secure_key(StorageId storage_id);
bool storage_trie_binding_get(StorageId storage_id, StorageGeneration generation,
                              StorageTrieView *view);
void storage_block_initialize(AccountId account_id, StorageId storage_id, u256 value,
                              NodeId terminal_node, bool prestate_exists);

#endif
