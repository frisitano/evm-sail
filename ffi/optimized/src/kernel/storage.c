/* Optimized storage operations declared by sail/optimised/kernel/storage.sail. */
#include "evmsail/prelude.h"

#include "evmsail/kernel/storage.h"
#include "evmsail/exceptions.h"
#include "host/state/internal.h"
#include "host/state/store.h"
#include "evmsail/kernel/accounts.h"
#include "lib/mpt/trie.h"
#include "evmsail/primitives/crypto.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static enum BlockError witness_error(uint64_t status) {
  return status == 1 ? WitnessDeficient : RlpDecode;
}

bool storage_preload(AccountId account_id, NodeId storage_root_node, U256 slot,
                     StorageId *storage_id, struct StorageValue *value) {
  memset(value, 0, sizeof(*value));

  Hash32 slot_hash;
  NodeId terminal_node = EVMSAIL_NODE_ID_EMPTY;
  bool prestate_exists = false;
  slot_hash = host_keccak_word(slot);
  const StorageId id = storage_schema_insert(account_id, &slot, &slot_hash);
  if (have_exception) return false;
  if (storage_root_node != EVMSAIL_NODE_ID_EMPTY) {
    const uint64_t status = stateless_storage_read(
        storage_root_node, slot_hash, &value->curr, &prestate_exists,
        &terminal_node);
    if (status != 0) {
      throw_invalid_block(witness_error(status), "optimized storage load");
      return false;
    }
  }
  value->orig = value->curr;
  storage_block_initialize(account_id, id, value->curr, terminal_node,
                           prestate_exists);
  if (storage_id != NULL) *storage_id = id;
  return true;
}

struct StorageValue k_sload(
    Hash32 parent_state_root,
    Address address, U256 slot) {
  (void)parent_state_root;
  struct StorageValue value;
  memset(&value, 0, sizeof(value));
  const AccountId account_id = get_account_id(&address);
  if (have_exception) return value;

  StorageView view;
  const StorageViewStatus tx_status =
      storage_transaction_view(account_id, &slot, &view);
  if (tx_status == STORAGE_VIEW_FOUND) {
    value.curr = view.current;
    value.orig = view.original;
    return value;
  }

  bal_note_storage_read(address, slot);
  if (have_exception || tx_status == STORAGE_VIEW_CLEARED) return value;

  if (storage_block_view(account_id, &slot, &view)) {
    value.curr = view.current;
    value.orig = view.original;
    return value;
  }
  if (!have_exception)
    throw_invalid_block(InvalidBlockAccessList,
                        "storage absent from block access list");
  return value;
}

unit k_sstore(Address address, U256 slot,
                      U256 current, U256 original) {
  return host_storage_update(address, slot, current, original);
}
