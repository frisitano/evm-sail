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

uint64_t storage_dump_count(U256 address_hash);
U256 storage_dump_slot(U256 address_hash, uint64_t index);
U256 storage_dump_value(U256 address_hash, uint64_t index);

uint32_t storage_update_candidates(Address address, StorageId *begin,
                                   StorageGeneration *generation);
bool storage_update_order_key(StorageId storage_id,
                              StorageGeneration generation,
                              NodeId *terminal_node, Hash32 *secure_key);
uint64_t storage_block_update_probe(
    StorageId storage_id, U256 *slot, U256 *current, U256 *original,
    Hash32 *secure_key, NodeId *terminal_node, bool *prestate_exists);
unit storage_block_iter_begin(Address address);
uint64_t storage_block_iter_next_probe(Address address, U256 *slot,
                                       U256 *current, U256 *original,
                                       Hash32 *address_hash,
                                       Hash32 *slot_hash);
void storage_block_initialize(AccountId account_id, StorageId storage_id,
                              U256 value, NodeId terminal_node,
                              bool prestate_exists);

#endif
