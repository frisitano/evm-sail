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

unit acct_db_reset(const unit u);
unit host_account_update(Address address, uint64_t nonce, U256 balance,
                         Hash32 code_hash, bool exists, bool storage_cleared,
                         bool created, bool selfdestructed);
unit acct_tx_set_balance(Address address, U256 balance);
unit acct_tx_set_nonce(Address address, uint64_t nonce);
unit acct_tx_set_code_hash(Address address, Hash32 code_hash);
unit acct_tx_reset(const unit u);

/* Minimal borrowed view used while reducing one ordered account-trie binding.
 * Large semantic values remain module-owned and stable for the duration of a
 * post-state-root pass. Mutations continue through the semantic setters above
 * so this module preserves journal and generation invariants. */
typedef struct {
  AccountId account_id;
  const Hash32 *secure_key;
  NodeId storage_base_node;
  NodeId original_storage_root_node;
  NodeId terminal_node;
  uint64_t nonce;
  const U256 *balance;
  const Hash32 *code_hash;
  bool current_live;
  bool original_exists;
  bool fields_changed;
} AccountTrieView;

uint32_t account_trie_binding_count(void);
void account_trie_binding_order_key(uint32_t index, NodeId *terminal_node,
                                    Hash32 *secure_key);
void account_trie_bindings_permute(uint32_t *destinations, uint32_t count);
bool account_trie_binding_get(uint32_t index, AccountTrieView *view);
unit acct_block_iter_begin(const unit u);
uint64_t acct_block_iter_next_probe(
    Address *address, uint64_t *current_nonce, U256 *current_balance,
    Hash32 *current_storage_root, Hash32 *current_code_hash,
    bool *current_exists, bool *current_storage_cleared, bool *current_created,
    bool *current_selfdestructed, uint64_t *original_nonce,
    U256 *original_balance, Hash32 *original_storage_root,
    Hash32 *original_code_hash, bool *original_exists,
    bool *original_storage_cleared, bool *original_created,
    bool *original_selfdestructed, Hash32 *address_hash);
unit account_block_initialize(AccountId account_id, Hash32 address_hash,
                              uint64_t nonce, U256 balance,
                              NodeId storage_root_node, Hash32 code_hash,
                              bool exists, bool storage_cleared,
                              NodeId terminal_node);

#endif
