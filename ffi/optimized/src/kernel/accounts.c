/* Optimized account operations declared by sail/optimised/kernel/accounts.sail. */
#include "evmsail/host/state/primitives.h"
#include "evmsail/host/nodes.h"
#include "evmsail/prelude.h"

#include "evmsail/kernel/accounts.h"
#include "evmsail/spec/primitives/account.h"
#include "host/state/account.h"
#include "host/state/block_access_list.h"
#include "evmsail/spec/host/state.h"
#include "host/state/internal.h"
#include "lib/mpt/trie.h"
#include "primitives/hash.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static struct Account empty_account(void)
{
  struct Account account;
  memset(&account, 0, sizeof(account));
  account.info.storage_root = EVMSAIL_EMPTY_TRIE_ROOT;
  account.info.code_hash = EVMSAIL_KECCAK_EMPTY;
  account.storage_cleared = true;
  return account;
}

static void account_from_view(struct Account *account, const AccountView *view)
{
  account->info.nonce = view->value.nonce;
  account->info.balance = view->value.balance;
  account->info.storage_root = view->storage_root;
  account->info.code_hash = view->value.code_hash;
  account->present = (view->value.exists != 0U);
  account->storage_cleared = view->storage_cleared;
  account->created = view->created;
  account->selfdestructed = view->selfdestructed;
}

void account_preload(bytes32 parent_state_root, AccountId account_id, bytes32 address_hash,
                     struct Account *account, NodeId *storage_root_node)
{
  *account = empty_account();
  struct AccountInfo info;
  bool found = false;
  NodeId terminal_node = EVMSAIL_NODE_ID_EMPTY;
  NodeId account_storage_root_node = EVMSAIL_NODE_ID_EMPTY;
  stateless_account_read(parent_state_root, address_hash, &info, &found, &terminal_node,
                         &account_storage_root_node);
  if (found) {
    account->info = info;
    account->present = true;
    account->storage_cleared = false;
  }
  account_block_initialize(account_id, address_hash, account->info.nonce, account->info.balance,
                           account->info.storage_root, account_storage_root_node,
                           account->info.code_hash, account->present, account->storage_cleared,
                           terminal_node);
  *storage_root_node = account_storage_root_node;
}

struct Account k_aload(bytes32 parent_state_root, bytes20 address)
{
  (void)parent_state_root;
  struct Account account = empty_account();
  AccountView view;
  if (account_transaction_view(&address, &view)) {
    account_from_view(&account, &view);
    return account;
  }

  bal_note_account_touch(address);
  /* bal_note_account_touch already resolved the address and fataled on a BAL
   * miss, so the block view cannot miss here. */
  if (account_block_view(&address, &view)) {
    account_from_view(&account, &view);
  }
  return account;
}

void store_account(bytes20 address, struct Account account)
{
  host_account_update(address, account.info.nonce, account.info.balance, account.info.code_hash,
                      account.present, account.storage_cleared, account.created,
                      account.selfdestructed);
}

void store_account_info(bytes20 address, struct Account account, struct AccountInfo info)
{
  const bool empty = account_fields_empty(info.nonce, &info.balance, &info.code_hash);
  if (empty) {
    storage_tx_clear(address);
  }

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

  if (!hash_equal(&next.info.storage_root, &account.info.storage_root) ||
      next.present != account.present || next.storage_cleared != account.storage_cleared) {
    store_account(address, next);
    return;
  }
  if (!word_equal(&next.info.balance, &account.info.balance)) {
    acct_tx_set_balance(address, next.info.balance);
  }
  if (next.info.nonce != account.info.nonce) {
    acct_tx_set_nonce(address, next.info.nonce);
  }
  if (!hash_equal(&next.info.code_hash, &account.info.code_hash)) {
    acct_tx_set_code_hash(address, next.info.code_hash);
  }
}
