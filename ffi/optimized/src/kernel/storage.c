/* Optimized storage operations declared by sail/optimised/kernel/storage.sail. */
#include "evmsail/host/state/primitives.h"
#include "evmsail/host/nodes.h"
#include "evmsail/prelude.h"

#include "evmsail/kernel/storage.h"
#include "evmsail/spec/primitives/account.h"
#include "host/state/block_access_list.h"
#include "evmsail/spec/exceptions.h"
#include "host/state/internal.h"
#include "host/state/storage.h"
#include "lib/mpt/trie.h"
#include "evmsail/primitives/crypto.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

bool storage_preload(AccountId account_id, NodeId storage_root_node, U256 slot,
                     StorageId *storage_id, struct StorageValue *value)
{
  memset(value, 0, sizeof(*value));

  Hash32 slot_hash;
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
  return true;
}

struct StorageValue k_sload(Hash32 parent_state_root, Address address, U256 slot)
{
  (void)parent_state_root;
  struct StorageValue value;
  memset(&value, 0, sizeof(value));
  const AccountId account_id = get_account_id(&address);

  StorageView view;
  const StorageViewStatus tx_status = storage_transaction_view(account_id, &slot, &view);
  if (tx_status == STORAGE_VIEW_FOUND) {
    value.curr = view.current;
    value.orig = view.original;
    return value;
  }

  bal_note_storage_read(address, slot);
  if (tx_status == STORAGE_VIEW_CLEARED) {
    return value;
  }

  if (storage_block_view(account_id, &slot, &view)) {
    value.curr = view.current;
    value.orig = view.original;
    return value;
  }
  fatal_error(InvalidBlockAccessList);
}
