/* Optimized account operations declared by sail/optimised/kernel/accounts.sail. */
#include "evmsail/prelude.h"

#include "evmsail/kernel/accounts.h"
#include "evmsail/exceptions.h"
#include "host/state/internal.h"
#include "host/state/store.h"
#include "lib/mpt/trie.h"
#include "primitives/hash.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static bool account_word_zero(U256 value) {
  return (value.limbs[0] | value.limbs[1] | value.limbs[2] |
          value.limbs[3]) == 0;
}

static struct zAccount empty_account(void) {
  struct zAccount account;
  memset(&account, 0, sizeof(account));
  account.zinfo.zstorage_root = EVMSAIL_EMPTY_TRIE_ROOT;
  account.zinfo.zcode_hash = EVMSAIL_KECCAK_EMPTY;
  account.zstorage_cleared = true;
  return account;
}

static bool account_info_empty(const struct zAccountInfo *info) {
  return info->znonce == 0 && account_word_zero(info->zbalance) &&
         hash_equal(&info->zcode_hash, &EVMSAIL_KECCAK_EMPTY);
}

static void account_from_view(struct zAccount *account,
                              const AccountView *view) {
  account->zinfo.znonce = view->value.nonce;
  account->zinfo.zbalance = view->value.balance;
  account->zinfo.zstorage_root = view->storage_root;
  account->zinfo.zcode_hash = view->value.code_hash;
  account->zpresent = view->value.exists;
  account->zstorage_cleared = view->storage_cleared;
  account->zcreated = view->created;
  account->zselfdestructed = view->selfdestructed;
}

static enum zBlockError witness_error(uint64_t status) {
  return status == 1 ? zWitnessDeficient : zRlpDecode;
}

bool account_preload(Hash32 parent_state_root, AccountId account_id,
                     Hash32 address_hash, struct zAccount *account,
                     NodeId *storage_root_node) {
  *account = empty_account();
  struct zAccountInfo info;
  bool found = false;
  NodeId terminal_node = EVMSAIL_NODE_ID_EMPTY;
  NodeId account_storage_root_node = EVMSAIL_NODE_ID_EMPTY;
  const uint64_t status = stateless_account_read(
      parent_state_root, address_hash, &info, &found, &terminal_node,
      &account_storage_root_node);
  if (status != 0) {
    throw_invalid_block(witness_error(status), "optimized account preload");
    return false;
  }
  if (found) {
    account->zinfo = info;
    account->zpresent = true;
    account->zstorage_cleared = false;
  }
  account_block_initialize(
      account_id, address_hash, account->zinfo.znonce, account->zinfo.zbalance,
      account_storage_root_node, account->zinfo.zcode_hash, account->zpresent,
      account->zstorage_cleared, terminal_node);
  if (have_exception) return false;
  *storage_root_node = account_storage_root_node;
  return true;
}

struct zAccount k_aload(
    Hash32 parent_state_root,
    Address address) {
  (void)parent_state_root;
  struct zAccount account = empty_account();
  AccountView view;
  if (account_transaction_view(&address, &view)) {
    account_from_view(&account, &view);
    return account;
  }

  bal_note_account_touch(address);
  if (have_exception) return account;
  if (account_block_view(&address, &view)) {
    account_from_view(&account, &view);
    return account;
  }
  (void)get_account_id(&address);
  return account;
}

unit store_account(Address address,
                           struct zAccount account) {
  return account_update(
      address, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zcode_hash, account.zpresent, account.zstorage_cleared,
      account.zcreated, account.zselfdestructed);
}

unit store_account_info(Address address,
                                struct zAccount account,
                                struct zAccountInfo info) {
  const bool empty = account_info_empty(&info);
  if (empty) storage_tx_clear(address);

  struct zAccount next = account;
  if (empty) {
    memset(&next.zinfo, 0, sizeof(next.zinfo));
    next.zinfo.zstorage_root = account.zinfo.zstorage_root;
    next.zinfo.zcode_hash = EVMSAIL_KECCAK_EMPTY;
    next.zpresent = false;
    next.zstorage_cleared = true;
  } else {
    next.zinfo = info;
    next.zpresent = true;
  }

  if (!hash_equal(&next.zinfo.zstorage_root,
                          &account.zinfo.zstorage_root) ||
      next.zpresent != account.zpresent ||
      next.zstorage_cleared != account.zstorage_cleared)
    return store_account(address, next);
  if (!word_equal(&next.zinfo.zbalance, &account.zinfo.zbalance))
    acct_tx_set_balance(address, next.zinfo.zbalance);
  if (next.zinfo.znonce != account.zinfo.znonce)
    acct_tx_set_nonce(address, next.zinfo.znonce);
  if (!hash_equal(&next.zinfo.zcode_hash, &account.zinfo.zcode_hash))
    acct_tx_set_code_hash(address, next.zinfo.zcode_hash);
  return UNIT;
}
