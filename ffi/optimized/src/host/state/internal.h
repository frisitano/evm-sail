/* Private coordination surface for the optimized semantic-state modules.
 *
 * Account, storage, and BAL data structures remain private to their owning
 * translation units.  Only stable dense identifiers and lifecycle operations
 * cross module boundaries. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_INTERNAL_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_INTERNAL_H

#include "evmsail/host/state/primitives.h"
#include "host/state/store.h"

enum { STORAGE_INITIAL_GENERATION = 1 };

/* Semantic account fields committed by an Ethereum account leaf, except for
 * storage_root.  code_hash remains here because it is mutable semantic account
 * state; storage_root is derived from storage-trie reduction and therefore
 * lives as a NodeId in AccountTrieBinding until final account RLP encoding. */
typedef struct {
  uint64_t nonce;
  U256 balance;
  Hash32 code_hash;
  uint8_t exists;
} AccountValue;

/* Directly mutable account state. original is captured on the
 * first account or storage mutation in a transaction. Rollback records only
 * the individual field changed by an operation; this structure is never
 * copied into the journal. */
typedef struct {
  AccountValue current;
  AccountValue original;
  StorageGeneration original_storage_generation;
  uint32_t transaction_epoch;
  uint32_t transaction_storage_count;
  uint8_t created;
  uint8_t selfdestructed;
  uint8_t dirty;
} AccountState;

/* Directly mutable storage state. original is the authenticated block-start
 * value used by root construction; transaction_original is the frozen value
 * used by SSTORE pricing and BAL change validation. */
typedef struct {
  U256 current;
  U256 original;
  U256 transaction_original;
  StorageGeneration storage_generation;
  StorageGeneration transaction_original_generation;
  uint32_t transaction_epoch;
} StorageState;

/* Complete semantic account view returned to optimized kernel operations.
 * The storage root is authenticated trie metadata rather than part of the
 * directly mutable AccountValue, so the state owner assembles it here. */
typedef struct {
  AccountValue value;
  Hash32 storage_root;
  bool storage_cleared;
  bool created;
  bool selfdestructed;
} AccountView;

/* Storage lookup distinguishes an absent BAL identity from a known slot made
 * logically zero by an account-wide storage-generation change. */
typedef enum {
  STORAGE_VIEW_MISSING,
  STORAGE_VIEW_FOUND,
  STORAGE_VIEW_CLEARED,
} StorageViewStatus;

typedef struct {
  U256 current;
  U256 original;
} StorageView;

uint32_t account_id_count(void);
void account_schema_prepare(uint32_t account_count);
AccountId account_schema_insert(const Address *address);
const Address *account_id_address(AccountId id);
AccountId lookup_account_id(const Address *address);
AccountId get_account_id(const Address *address);
void account_storage_range_bind(AccountId id, StorageId begin,
                                uint32_t count);
void account_storage_range(AccountId id, StorageId *begin,
                           uint32_t *count);
void account_warm_restore(AccountId id, uint32_t prior_epoch);
StorageGeneration account_storage_generation(AccountId id);
StorageGeneration account_transaction_original_storage_generation(
    AccountId id);
void account_storage_generation_restore(AccountId id,
                                        StorageGeneration generation);
void account_clear_storage_generation(AccountId id);
bool account_exists(AccountId id);
bool account_transaction_view(const Address *address, AccountView *view);
bool account_block_view(const Address *address, AccountView *view);
void account_diagnostics_invalidate(AccountId id);
void account_balance_restore(AccountId id, U256 prior);
void account_nonce_restore(AccountId id, uint64_t prior);
void account_code_hash_restore(AccountId id, Hash32 prior);
void account_exists_restore(AccountId id, bool prior);
void account_created_restore(AccountId id, bool prior);
void account_selfdestructed_restore(AccountId id, bool prior);
void account_transaction_touch(AccountId id);
uint32_t account_transaction_count(void);
AccountId account_transaction_id_at(uint32_t index);
void account_transaction_pop_last(void);
void account_transaction_storage_append(AccountId id, StorageId storage_id);
void account_transaction_storage_pop_last(AccountId id);
uint32_t account_transaction_storage_count(AccountId id);
StorageId account_transaction_storage_id_at(AccountId id, uint32_t index);

extern uint32_t current_warm_epoch;

StorageId storage_schema_insert(AccountId account_id, const U256 *slot,
                                const Hash32 *secure_key);
StorageId get_storage_id(AccountId account_id, const U256 *slot);
uint32_t storage_id_count(void);
const U256 *storage_id_slot(StorageId id);
void storage_schema_account_begin(AccountId id);
void storage_schema_account_end(AccountId id);
void storage_schema_seal(void);
void storage_warm_restore(StorageId id, uint32_t prior_epoch);
void storage_value_restore(StorageId id, U256 prior);
void storage_generation_restore(StorageId id, StorageGeneration prior);
void storage_transaction_forget(StorageId id);
StorageViewStatus storage_transaction_view(AccountId account_id,
                                           const U256 *slot,
                                           StorageView *view);
bool storage_block_view(AccountId account_id, const U256 *slot,
                        StorageView *view);

void account_state_workspace_bind(void);
void storage_state_workspace_bind(void);
void block_access_list_workspace_bind(void);

void account_transaction_merge(struct zTransactionMergeSemantics semantics,
                               uint64_t current_transaction_epoch);
void storage_transaction_merge(uint64_t current_transaction_epoch);

#endif
