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

unit storage_db_reset(const unit u);
unit host_storage_update(Address address, U256 slot, U256 value,
                         U256 original);
unit storage_tx_clear(Address address);
unit storage_tx_reset(const unit u);
bool storage_has_writes(Address address);
unit storage_block_clear(Address address);

/* Minimal borrowed view used while reducing one live storage-trie binding.
 * The slot identity remains module-owned and outside the MPT hot path.
 * Mutations stay behind the semantic setters above so generation and journal
 * invariants remain owned by this module. */
typedef struct {
  const U256 *current;
  const U256 *original;
  const Hash32 *secure_key;
  NodeId terminal_node;
  bool prestate_exists;
} StorageTrieView;

uint32_t storage_trie_candidates(AccountId account_id, StorageId *begin,
                                 StorageGeneration *generation);
bool storage_trie_binding_order_key(StorageId storage_id,
                                    StorageGeneration generation,
                                    NodeId *terminal_node,
                                    Hash32 *secure_key);
bool storage_trie_binding_get(StorageId storage_id,
                              StorageGeneration generation,
                              StorageTrieView *view);
unit storage_block_iter_begin(Address address);
uint64_t storage_block_iter_next_probe(Address address, U256 *slot,
                                       U256 *current, U256 *original,
                                       Hash32 *address_hash,
                                       Hash32 *slot_hash);
void storage_block_initialize(AccountId account_id, StorageId storage_id,
                              U256 value, NodeId terminal_node,
                              bool prestate_exists);

#endif
