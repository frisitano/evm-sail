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

static struct Account empty_account(void) {
  struct Account account;
  memset(&account, 0, sizeof(account));
  account.info.storage_root = EVMSAIL_EMPTY_TRIE_ROOT;
  account.info.code_hash = EVMSAIL_KECCAK_EMPTY;
  account.storage_cleared = true;
  return account;
}

static bool account_info_is_empty(const struct AccountInfo *info) {
  return info->nonce == 0 && account_word_zero(info->balance) &&
         hash_equal(&info->code_hash, &EVMSAIL_KECCAK_EMPTY);
}

static void account_from_view(struct Account *account,
                              const AccountView *view) {
  account->info.nonce = view->value.nonce;
  account->info.balance = view->value.balance;
  account->info.storage_root = view->storage_root;
  account->info.code_hash = view->value.code_hash;
  account->present = view->value.exists;
  account->storage_cleared = view->storage_cleared;
  account->created = view->created;
  account->selfdestructed = view->selfdestructed;
}

static enum BlockError witness_error(uint64_t status) {
  return status == 1 ? WitnessDeficient : RlpDecode;
}

bool account_preload(Hash32 parent_state_root, AccountId account_id,
                     Hash32 address_hash, struct Account *account,
                     NodeId *storage_root_node) {
  *account = empty_account();
  struct AccountInfo info;
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
    account->info = info;
    account->present = true;
    account->storage_cleared = false;
  }
  account_block_initialize(
      account_id, address_hash, account->info.nonce, account->info.balance,
      account_storage_root_node, account->info.code_hash, account->present,
      account->storage_cleared, terminal_node);
  if (have_exception) return false;
  *storage_root_node = account_storage_root_node;
  return true;
}

struct Account k_aload(
    Hash32 parent_state_root,
    Address address) {
  (void)parent_state_root;
  struct Account account = empty_account();
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
                           struct Account account) {
  return host_account_update(
      address, account.info.nonce, account.info.balance,
      account.info.code_hash, account.present, account.storage_cleared,
      account.created, account.selfdestructed);
}

unit store_account_info(Address address,
                                struct Account account,
                                struct AccountInfo info) {
  const bool empty = account_info_is_empty(&info);
  if (empty) storage_tx_clear(address);

  struct Account next = account;
  if (empty) {
    memset(&next.info, 0, sizeof(next.info));
    next.info.storage_root = account.info.storage_root;
    next.info.code_hash = EVMSAIL_KECCAK_EMPTY;
    next.present = false;
    next.storage_cleared = true;
  } else {
    next.info = info;
    next.present = true;
  }

  if (!hash_equal(&next.info.storage_root,
                          &account.info.storage_root) ||
      next.present != account.present ||
      next.storage_cleared != account.storage_cleared)
    return store_account(address, next);
  if (!word_equal(&next.info.balance, &account.info.balance))
    acct_tx_set_balance(address, next.info.balance);
  if (next.info.nonce != account.info.nonce)
    acct_tx_set_nonce(address, next.info.nonce);
  if (!hash_equal(&next.info.code_hash, &account.info.code_hash))
    acct_tx_set_code_hash(address, next.info.code_hash);
  return UNIT;
}
