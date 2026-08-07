/* Optimized account-state interface.
 *
 * The implementation owns account schema, direct semantic values, and the
 * account-trie identity binding. Rollback is coordinated by the shared state
 * journal. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_ACCOUNT_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_ACCOUNT_H

#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

struct Account;
struct AcctEntry;

void host_account_update(bytes20 address, uint64_t nonce, u256 balance, bytes32 code_hash,
                         bool exists, bool storage_cleared, bool created, bool selfdestructed);
void acct_tx_set_balance(bytes20 address, u256 balance);
void acct_tx_set_nonce(bytes20 address, uint64_t nonce);
void acct_tx_set_code_hash(bytes20 address, bytes32 code_hash);
/* Minimal borrowed view used while reducing one ordered account-trie binding.
 * Large semantic values remain module-owned and stable for the duration of a
 * post-state-root pass. Mutations continue through the semantic setters above
 * so this module preserves journal and generation invariants. */
typedef struct {
  AccountId account_id;
  const bytes32 *secure_key;
  NodeId storage_base_node;
  const bytes32 *storage_base_root;
  const bytes32 *original_storage_root;
  NodeId terminal_node;
  uint64_t nonce;
  const u256 *balance;
  const bytes32 *code_hash;
  bool current_live;
  bool original_exists;
  bool fields_changed;
} AccountTrieView;

uint32_t account_trie_binding_count(void);
void account_trie_binding_order_key(uint32_t index, NodeId *terminal_node, bytes32 *secure_key);
void account_trie_bindings_permute(uint32_t *destinations, uint32_t count);
bool account_trie_binding_get(uint32_t index, AccountTrieView *view);
void account_block_initialize(AccountId account_id, bytes32 address_hash, uint64_t nonce,
                              u256 balance, bytes32 storage_root, NodeId storage_root_node,
                              bytes32 code_hash, bool exists, bool storage_cleared,
                              NodeId terminal_node);

#endif
