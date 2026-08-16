/* Private coordination surface for the optimized semantic-state modules.
 *
 * Account, storage, and BAL data structures remain private to their owning
 * translation units.  Only stable dense identifiers and lifecycle operations
 * cross module boundaries. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_INTERNAL_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_INTERNAL_H

#include "evmsail/host/state/primitives.h"
#include "host/state/account.h"
#include "host/state/block_access_list.h"
#include "host/state/storage.h"
#include "primitives/hash.h"
#include "primitives/value.h"

enum { STORAGE_INITIAL_GENERATION = 1 };

/* Maps a nonzero stateless witness-read status onto the block error raised by
 * the optimized preload operations. */
static inline enum FatalError witness_error(uint64_t status)
{
  return status == 1 ? WitnessDeficient : RlpDecode;
}

/* EIP-161 emptiness over semantic account fields; existence is considered by
 * the caller because an absent account and an empty existing account differ. */
static inline bool account_fields_empty(uint64_t nonce, const u256 *balance,
                                        const bytes32 *code_hash)
{
  return (nonce == 0 && word_all_zero(balance) && hash_equal(code_hash, &EVMSAIL_KECCAK_EMPTY)) !=
         0;
}

/* Semantic account fields committed by an Ethereum account leaf, except for
 * storage_root.  code_hash remains here because it is mutable semantic account
 * state; storage_root is derived from storage-trie reduction and therefore
 * lives in AccountTrieBinding until final account RLP encoding. */
typedef struct {
  uint64_t nonce;
  u256 balance;
  bytes32 code_hash;
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
  u256 current;
  u256 original;
  u256 transaction_original;
  StorageGeneration storage_generation;
  StorageGeneration transaction_original_generation;
  uint32_t transaction_epoch;
} StorageState;

/* Complete semantic account view returned to optimized kernel operations.
 * The storage root is authenticated trie metadata rather than part of the
 * directly mutable AccountValue, so the state owner assembles it here. */
typedef struct {
  AccountValue value;
  bytes32 storage_root;
  bool storage_cleared;
  bool created;
  bool selfdestructed;
} AccountView;

/* Account-owned storage metadata carried once per transaction worklist row.
 * Storage consumes this value without projecting fields back out of the
 * account table. */
typedef struct {
  AccountId account_id;
  StorageId storage_begin;
  uint32_t storage_count;
  uint32_t transaction_storage_count;
  StorageGeneration original_storage_generation;
  StorageGeneration storage_generation;
  bool exists;
} AccountTransactionStorageView;

typedef struct {
  u256 current;
  u256 original;
} StorageView;

/* Journal-restorable account fields; each entry restores exactly one field,
 * so the prior values share one payload union. */
typedef enum {
  ACCOUNT_FIELD_BALANCE,
  ACCOUNT_FIELD_NONCE,
  ACCOUNT_FIELD_CODE_HASH,
  ACCOUNT_FIELD_EXISTS,
  ACCOUNT_FIELD_CREATED,
  ACCOUNT_FIELD_SELFDESTRUCTED,
} AccountRestoreField;

typedef union {
  u256 balance;
  uint64_t nonce;
  bytes32 code_hash;
  bool flag;
} AccountRestorePrior;

uint32_t account_id_count(void);
AccountId account_schema_insert(const bytes20 *address);
const bytes20 *account_id_address(AccountId id);
struct AccountExecutionContext account_execution_context(bytes20 address);
AccountId lookup_account_id(const bytes20 *address);
AccountId get_account_id(const bytes20 *address);
void account_bind_storage_schema(AccountId id, StorageId begin, uint32_t change_count,
                                 uint32_t count);
void account_warm_restore(AccountId id, uint32_t prior_epoch);
StorageGeneration account_storage_generation(AccountId id);
void account_storage_generation_restore(AccountId id, StorageGeneration generation);
void account_clear_storage_generation(AccountId id);
bool account_exists(AccountId id);
bool account_transaction_view(const bytes20 *address, AccountView *view);
bool account_block_view(const bytes20 *address, AccountView *view);
void account_field_restore(AccountId id, AccountRestoreField field, AccountRestorePrior prior);
void account_transaction_touch(AccountId id);
uint32_t account_transaction_count(void);
void account_transaction_storage_view_at(uint32_t index, AccountTransactionStorageView *view);
void account_transaction_pop_last(void);
void account_transaction_storage_append(AccountId id, StorageId storage_id);
void account_transaction_storage_pop_last(AccountId id);
uint32_t account_transaction_storage_count(AccountId id);
StorageId account_transaction_storage_id_at(AccountId id, uint32_t index);

extern uint32_t current_warm_epoch;

StorageId storage_schema_insert(AccountId account_id, const u256 *slot, const bytes32 *secure_key);
StorageId get_storage_id(StorageId begin, uint32_t change_count, uint32_t count, const u256 *slot);
StorageId storage_resolve_slot(StorageId begin, uint32_t change_count, uint32_t count, u256 slot);
bool storage_id_is_warm(StorageId storage_id);
void storage_id_mark_warm(StorageId storage_id);
void storage_update_by_id(AccountId account_id, StorageGeneration generation, StorageId storage_id,
                          u256 value, u256 original);
uint32_t storage_id_count(void);
const u256 *storage_id_slot(StorageId id);
void storage_schema_account_begin(AccountId id);
void storage_schema_account_end(AccountId id, uint32_t change_count);
void storage_schema_seal(void);
void storage_warm_restore(StorageId id, uint32_t prior_epoch);
void storage_value_restore(StorageId id, u256 prior);
void storage_generation_restore(StorageId id, StorageGeneration prior);
void storage_transaction_forget(StorageId id);
void storage_load_view(StorageGeneration generation, StorageId storage_id, StorageView *view);
bool storage_range_has_writes(StorageId begin, uint32_t count, StorageGeneration generation);

void account_state_workspace_bind(uint32_t account_count, uint32_t storage_count);
void storage_state_workspace_bind(uint32_t storage_count);
void block_access_list_workspace_bind(uint32_t account_count, uint32_t storage_count);
void bal_account_schema_initialize(AccountId id);
void bal_storage_schema_initialize(StorageId id);

void account_transaction_merge(struct TransactionMergeSemantics semantics,
                               uint32_t current_transaction_epoch);
void storage_transaction_merge(uint32_t current_transaction_epoch);

#endif
