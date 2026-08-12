/* Optimized authenticated-storage preload used by the BAL loader. */
#include "evmsail/host/state/primitives.h"
#include "evmsail/host/nodes.h"
#include "evmsail/prelude.h"

#include "evmsail/kernel/storage.h"
#include "evmsail/spec/exceptions.h"
#include "host/state/block_access_list.h"
#include "host/state/internal.h"
#include "host/state/storage.h"
#include "kernel/storage.h"
#include "lib/mpt/trie.h"
#include "evmsail/primitives/crypto.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void storage_preload(AccountId account_id, NodeId storage_root_node, u256 slot,
                     StorageId *storage_id, struct StorageValue *value)
{
  memset(value, 0, sizeof(*value));

  bytes32 slot_hash;
  NodeId terminal_node = EVMSAIL_NODE_ID_EMPTY;
  bool prestate_exists = false;
  slot_hash = host_keccak_word(slot);
  const StorageId id = storage_schema_insert(account_id, &slot, &slot_hash);
  if (storage_root_node != EVMSAIL_NODE_ID_EMPTY) {
    stateless_storage_read(storage_root_node, slot_hash, &value->curr, &prestate_exists,
                           &terminal_node);
  }
  value->orig = value->curr;
  storage_block_initialize(account_id, id, value->curr, terminal_node, prestate_exists);
  *storage_id = id;
}

struct StorageValue storage_load_by_id(AccountId account_id, StorageGeneration generation,
                                       StorageId storage_id, bool mark_warm)
{
  struct StorageValue value;
  memset(&value, 0, sizeof(value));
  if (mark_warm) {
    storage_id_mark_warm(storage_id);
  }

  StorageView view;
  storage_load_view(generation, storage_id, &view);
  bal_note_storage_touch(account_id, storage_id);
  value.curr = view.current;
  value.orig = view.original;
  return value;
}
