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

struct zoptionzIRAccountzK;
struct zoptionzIRAcctTrieEntryzK;
struct zAccount;
struct zAcctEntry;

void acct_block_get(struct zoptionzIRAccountzK *result, Address address);
void acct_block_iter_next(struct zoptionzIRAcctTrieEntryzK *result, unit u);

unit acct_db_reset(const unit u);
unit account_update(Address address, uint64_t nonce, U256 balance,
                    Hash32 code_hash, bool exists, bool storage_cleared,
                    bool created, bool selfdestructed);
unit acct_tx_set_balance(Address address, U256 balance);
unit acct_tx_set_nonce(Address address, uint64_t nonce);
unit acct_tx_set_code_hash(Address address, Hash32 code_hash);
unit acct_tx_reset(const unit u);

uint64_t acct_dump_count(const unit u);
U256 acct_dump_hkey(uint64_t index);
Address acct_dump_address(uint64_t index);
uint64_t acct_dump_nonce(uint64_t index);
U256 acct_dump_balance(uint64_t index);
U256 acct_dump_storage_root(uint64_t index);
Hash32 acct_dump_post_storage_root(uint64_t index);
U256 acct_dump_code_hash(uint64_t index);

uint32_t account_trie_binding_count(void);
void account_trie_binding_order_key(uint32_t index, NodeId *terminal_node,
                                    Hash32 *secure_key);
void account_trie_bindings_permute(uint32_t *destinations, uint32_t count);
void acct_block_update_meta_at(
    uint32_t index, Address *address, Hash32 *address_hash,
    NodeId *storage_base_node, NodeId *original_storage_root_node,
    bool *current_live, bool *original_exists, bool *fields_changed,
    NodeId *terminal_node);
void acct_block_update_current_at(uint32_t index, uint64_t *nonce,
                                  U256 *balance, Hash32 *code_hash);
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
