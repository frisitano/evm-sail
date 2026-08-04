/* Optimized account semantic state.
 *
 * This module owns three related but deliberately separate kinds of state:
 *
 *   1. AccountEntry is the immutable block-local identity and schema. A stable
 *      AccountId indexes it for the whole block.
 *   2. AccountState is the directly mutable semantic value. The shared state
 *      journal records field-level inverse operations before rollback-visible
 *      mutations; it never copies the complete state.
 *   3. AccountTrieBinding connects semantic accounts to their authenticated
 *      witness positions. Bindings may be reordered for final MPT reduction;
 *      their embedded AccountId remains the stable route back to state.
 *
 * Account ID zero and an empty account-table bucket are sentinels. Real
 * accounts therefore start at one.
 * Storage values and storage rollback are owned by storage.c; this module owns
 * only the account-wide storage range and incarnation/generation. */
#include "host/state/internal.h"
#include "host/state/journal.h"
#include "host/state/runtime.h"
#include "evmsail/exceptions.h"
#include "lib/mpt/trie.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const U256 account_zero = {{0}};

/* ======================================================================== */
/* ACCOUNT STATE                                                            */
/*                                                                          */
/* AccountId indexes semantic state. Authenticated trie bindings form their  */
/* own finalization stream and carry the AccountId needed to reach that       */
/* state. This avoids a second AccountId ordering array.                      */
/* ======================================================================== */

/* Immutable block-local account identity plus inexpensive execution metadata.
 *
 * storage_begin/storage_count identify the account's contiguous StorageId
 * range, populated during BAL loading. UINT32_MAX means the range has not yet
 * been bound. warm_epoch implements EIP-2929 without clearing every account
 * between transactions. storage_generation identifies the currently visible
 * storage incarnation; clearing storage increments it in O(1). */
typedef struct {
  /* Canonical 20-byte address and the table's lookup identity. */
  Address address;
  /* First StorageId in this account's contiguous BAL-defined interval. */
  StorageId storage_begin;
  /* Number of StorageIds owned by this account. */
  uint32_t storage_count;
  /* Last transaction epoch in which the address was marked warm. */
  uint32_t warm_epoch;
  /* Storage incarnation against which transaction slot logs are interpreted. */
  StorageGeneration storage_generation;
} AccountEntry;

/* Authenticated-state metadata for one semantic account.
 *
 * secure_key is keccak256(address). terminal_node is the position at which the
 * account lookup terminated in the immutable account witness. The storage
 * root is represented by its witness NodeId, not copied as a digest. Node zero
 * is the explicit empty-trie node; EVMSAIL_NODE_ID_UNLINKED means no binding.
 * prestate_exists distinguishes replacement/deletion from insertion.
 *
 * Bindings initially occupy AccountId - 1 during BAL materialization. MPT
 * finalization later permutes them by (terminal_node, secure_key), after which
 * account_id is the only supported route back to AccountEntry/AccountState. */
typedef struct {
  /* Stable semantic account selected by this reorderable binding. */
  AccountId account_id;
  /* Canonical secure account-trie key: keccak256(address). */
  Hash32 secure_key;
  /* Witness node at which authenticated account lookup terminated. */
  NodeId terminal_node;
  /* Root NodeId of the account's authenticated pre-state storage trie. */
  NodeId storage_root_node;
  /* Whether authenticated pre-state contained this account. */
  uint8_t prestate_exists;
} AccountTrieBinding;

/* Complete account subsystem allocation, backed by the shared guest workspace.
 *
 * entries and states are indexed directly by AccountId and include sentinel
 * row zero. trie_bindings has one element per real account and may be reordered
 * independently. buckets is a power-of-two open-addressed address index;
 * ACCOUNT_ID_NONE marks an empty bucket and bucket_mask implements modulo by
 * bucket_count. count includes the sentinel, whereas capacity does too. */
typedef struct {
  /* AccountId-indexed immutable identities; includes sentinel row zero. */
  AccountEntry *entries;
  /* AccountId-indexed mutable semantic state; includes sentinel row zero. */
  AccountState *states;
  /* Reorderable authenticated bindings; contains only real accounts. */
  AccountTrieBinding *trie_bindings;
  /* Open-addressed address index; zero denotes an empty bucket. */
  AccountId *buckets;
  /* Next AccountId and number of rows including sentinel row zero. */
  uint32_t count;
  /* Maximum count including sentinel row zero. */
  uint32_t capacity;
  /* Active power-of-two bucket count chosen from the BAL size. */
  uint32_t bucket_count;
  /* bucket_count - 1, used for wraparound instead of division. */
  uint32_t bucket_mask;
} AccountTable;

/* Singleton block-local account table. count begins at one to reserve sentinel
 * AccountId zero; backing pointers are assigned by account_state_workspace_bind. */
static AccountTable acct_table = {
    .count = 1u,
    .capacity = GUEST_STATE_ACCOUNTS + 1u,
};

/* Distinct AccountIds touched by the current transaction. */
static AccountId *acct_tx_rows;
static uint32_t acct_tx_rows_n = 0;

/* StorageIds touched by a transaction are written into the owning account's
 * BAL-defined storage interval. Each account therefore has an exact-capacity
 * worklist without growth, relocation, or a second global lookup structure. */
static StorageId *transaction_storage_ids;

/* Finds an address in the BAL-preallocated account universe.
 *
 * The first address bytes supply the initial bucket because Ethereum addresses are
 * already hash-derived. Linear probing resolves collisions and full Address
 * equality validates a candidate. A miss returns ACCOUNT_ID_NONE; execution
 * APIs decide whether that miss is a BAL violation. */
AccountId lookup_account_id(const Address *address) {
  uint32_t bucket = 0;
  memcpy(&bucket, address->bytes, sizeof(bucket));
  bucket &= acct_table.bucket_mask;
  for (uint32_t probes = 0; probes < acct_table.bucket_count; probes++) {
    const AccountId id = acct_table.buckets[bucket];
    if (id == ACCOUNT_ID_NONE) return ACCOUNT_ID_NONE;
    if (id >= acct_table.count) GUEST_ABORT();
    if (address_equal(&acct_table.entries[id].address, address)) return id;
    bucket = (bucket + 1u) & acct_table.bucket_mask;
  }
  GUEST_ABORT();
}

/* Resolves a required execution account. The BAL is the complete block-local
 * schema, so absence is a protocol error rather than a dynamic insertion. */
AccountId get_account_id(const Address *address) {
  const AccountId id = lookup_account_id(address);
  if (id == ACCOUNT_ID_NONE) {
    throw_invalid_block(InvalidBlockAccessList,
                        "account absent from block access list");
  }
  return id;
}

/* Reports whether storage updates may contribute to this account's post-state.
 * A nonexistent account cannot own live storage. */
bool account_exists(AccountId id) {
  return id != ACCOUNT_ID_NONE && id < acct_table.count &&
         acct_table.states[id].current.exists;
}

/* Appends one BAL account and installs it in the address index.
 *
 * BAL loading must present strictly increasing, duplicate-free addresses. This
 * gives deterministic AccountIds and validates canonical BAL order while the
 * schema is built. Runtime account creation never reaches this function. */
AccountId account_schema_insert(const Address *address) {
  if (acct_table.count >= acct_table.capacity ||
      acct_table.bucket_count == 0)
    GUEST_ABORT();
  if (acct_table.count > ACCOUNT_ID_NONE + 1u &&
      address_compare(&acct_table.entries[acct_table.count - 1u].address,
                      address) >= 0) {
    throw_invalid_block(InvalidBlockAccessList,
                        "BAL accounts are not strictly increasing");
    return ACCOUNT_ID_NONE;
  }

  uint32_t bucket = 0;
  memcpy(&bucket, address->bytes, sizeof(bucket));
  bucket &= acct_table.bucket_mask;
  for (uint32_t probes = 0; probes < acct_table.bucket_count; probes++) {
    const AccountId existing = acct_table.buckets[bucket];
    if (existing == ACCOUNT_ID_NONE) {
      const AccountId i = acct_table.count;
      acct_table.entries[i].address = *address;
      acct_table.entries[i].storage_begin = UINT32_MAX;
      acct_table.entries[i].storage_generation = STORAGE_INITIAL_GENERATION;
      acct_table.buckets[bucket] = i;
      acct_table.count++;
      return i;
    }
    if (existing >= acct_table.count) GUEST_ABORT();
    if (address_equal(&acct_table.entries[existing].address, address)) {
      throw_invalid_block(InvalidBlockAccessList,
                          "duplicate BAL account");
      return ACCOUNT_ID_NONE;
    }
    bucket = (bucket + 1u) & acct_table.bucket_mask;
  }
  GUEST_ABORT();
}

/* A missing BAL row is observably cold. The later semantic account access is
 * responsible for rejecting a deficient BAL after the access charge is paid. */
bool account_is_warm(Address address) {
  const AccountId id = lookup_account_id(&address);
  if (id == ACCOUNT_ID_NONE) return false;
  return acct_table.entries[id].warm_epoch >= current_warm_epoch;
}

/* Marks an existing BAL account warm. A missing row remains untracked so a
 * charge-only path can halt out of gas without spuriously rejecting the BAL. */
unit account_mark_warm(Address address) {
  const AccountId id = lookup_account_id(&address);
  if (id == ACCOUNT_ID_NONE) return UNIT;

  uint32_t *warm_epoch = &acct_table.entries[id].warm_epoch;
  if (*warm_epoch < current_warm_epoch) {
    const uint32_t prior_epoch = *warm_epoch;
    state_journal_push_warm_account(id, prior_epoch);
    *warm_epoch = current_warm_epoch;
  }
  return UNIT;
}

/* Applies the inverse of the first warm access recorded by the state journal. */
void account_warm_restore(AccountId id, uint32_t prior_epoch) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  acct_table.entries[id].warm_epoch = prior_epoch;
}

/* Returns the next AccountId; row zero is included in this count. */
uint32_t account_id_count(void) { return acct_table.count; }

/* Returns the canonical address belonging to a stable AccountId. */
const Address *account_id_address(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  return &acct_table.entries[id].address;
}

/* Binds the one contiguous StorageId interval assigned during BAL loading.
 * Binding is single-assignment and remains stable for the block. */
void account_storage_range_bind(AccountId id, StorageId begin,
                                uint32_t count) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count ||
      begin > GUEST_STATE_STORAGE ||
      count > GUEST_STATE_STORAGE - begin)
    GUEST_ABORT();
  AccountEntry *entry = &acct_table.entries[id];
  if (entry->storage_begin != UINT32_MAX) GUEST_ABORT();
  entry->storage_begin = begin;
  entry->storage_count = count;
}

/* Reads an account's already-bound StorageId interval. */
void account_storage_range(AccountId id, StorageId *begin,
                           uint32_t *count) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count || !begin || !count)
    GUEST_ABORT();
  const AccountEntry *entry = &acct_table.entries[id];
  if (entry->storage_begin == UINT32_MAX) GUEST_ABORT();
  *begin = entry->storage_begin;
  *count = entry->storage_count;
}

/* Sizes the address index from the decoded BAL account count.
 *
 * A power-of-two table at or below 50% load makes masking and linear probing
 * cheap while retaining deterministic, allocation-free workspace usage. */
void account_schema_prepare(uint32_t account_count) {
  if (acct_table.count != ACCOUNT_ID_NONE + 1u ||
      acct_table.bucket_count != 0 ||
      account_count > GUEST_STATE_ACCOUNTS)
    GUEST_ABORT();

  uint32_t bucket_count = 1u;
  const uint32_t required = account_count == 0 ? 1u : 2u * account_count;
  while (bucket_count < required) bucket_count <<= 1;
  if (bucket_count > GUEST_ACCOUNT_INDEX_BUCKETS) GUEST_ABORT();

  acct_table.bucket_count = bucket_count;
  acct_table.bucket_mask = bucket_count - 1u;
}

/* Captures the transaction-start semantic value once. The shared journal owns
 * rollback; this transaction metadata exists only for SSTORE/BAL comparison
 * and for exact-capacity touched-row iteration at commit. */
void account_transaction_touch(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  AccountState *state = &acct_table.states[id];
  if (state->transaction_epoch == current_warm_epoch) return;
  state->original = state->current;
  state->original_storage_generation =
      acct_table.entries[id].storage_generation;
  state->transaction_epoch = current_warm_epoch;
  state->transaction_storage_count = 0;
  state->created = 0;
  state->selfdestructed = 0;
  if (acct_tx_rows_n >= GUEST_STATE_ACCOUNTS) GUEST_ABORT();
  state_journal_push_account_transaction_listed();
  acct_tx_rows[acct_tx_rows_n++] = id;
}

static AccountState *account_state_for_restore(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  return &acct_table.states[id];
}

void account_balance_restore(AccountId id, U256 prior) {
  account_state_for_restore(id)->current.balance = prior;
}

void account_nonce_restore(AccountId id, uint64_t prior) {
  account_state_for_restore(id)->current.nonce = prior;
}

void account_code_hash_restore(AccountId id, Hash32 prior) {
  account_state_for_restore(id)->current.code_hash = prior;
}

void account_exists_restore(AccountId id, bool prior) {
  account_state_for_restore(id)->current.exists = prior;
}

void account_created_restore(AccountId id, bool prior) {
  account_state_for_restore(id)->created = prior;
}

void account_selfdestructed_restore(AccountId id, bool prior) {
  account_state_for_restore(id)->selfdestructed = prior;
}

uint32_t account_transaction_count(void) { return acct_tx_rows_n; }

AccountId account_transaction_id_at(uint32_t index) {
  if (index >= acct_tx_rows_n) GUEST_ABORT();
  return acct_tx_rows[index];
}

void account_transaction_pop_last(void) {
  if (acct_tx_rows_n == 0) GUEST_ABORT();
  AccountState *state = &acct_table.states[acct_tx_rows[--acct_tx_rows_n]];
  state->transaction_epoch = 0;
  state->transaction_storage_count = 0;
  state->created = 0;
  state->selfdestructed = 0;
}

/* Stores transaction-touched StorageIds in the owning account's exact BAL
 * interval. No auxiliary capacity, growth, or relocation is required. */
void account_transaction_storage_append(AccountId id, StorageId storage_id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  AccountEntry *entry = &acct_table.entries[id];
  AccountState *state = &acct_table.states[id];
  if (entry->storage_begin == UINT32_MAX ||
      state->transaction_storage_count >= entry->storage_count)
    GUEST_ABORT();
  state_journal_push_account_storage_listed(id);
  transaction_storage_ids[entry->storage_begin +
                          state->transaction_storage_count++] = storage_id;
}

void account_transaction_storage_pop_last(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  AccountState *state = &acct_table.states[id];
  if (state->transaction_storage_count == 0) GUEST_ABORT();
  const StorageId storage_id =
      transaction_storage_ids[acct_table.entries[id].storage_begin +
                              --state->transaction_storage_count];
  storage_transaction_forget(storage_id);
}

uint32_t account_transaction_storage_count(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  return acct_table.states[id].transaction_storage_count;
}

StorageId account_transaction_storage_id_at(AccountId id, uint32_t index) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  const AccountEntry *entry = &acct_table.entries[id];
  if (index >= acct_table.states[id].transaction_storage_count) GUEST_ABORT();
  return transaction_storage_ids[entry->storage_begin + index];
}

/* Returns the account-wide storage incarnation visible to storage.c. */
StorageGeneration account_storage_generation(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  return acct_table.entries[id].storage_generation;
}

/* Returns the storage incarnation visible at transaction entry. Accounts not
 * yet touched in this transaction still observe their current block value. */
StorageGeneration account_transaction_original_storage_generation(
    AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  const AccountState *state = &acct_table.states[id];
  return state->transaction_epoch == current_warm_epoch
             ? state->original_storage_generation
             : acct_table.entries[id].storage_generation;
}

/* Restores a generation recorded by the shared state journal during revert. */
void account_storage_generation_restore(AccountId id,
                                        StorageGeneration generation) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count ||
      generation < STORAGE_INITIAL_GENERATION)
    GUEST_ABORT();
  acct_table.entries[id].storage_generation = generation;
}

/* Clears storage logically in O(1) by advancing the account incarnation.
 * Existing slot rows remain allocated but become invisible until rewritten in
 * the new generation. The generation has its own journal event because a
 * clear and a slot-value change have distinct rollback semantics. */
void account_clear_storage_generation(AccountId id) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count) GUEST_ABORT();
  account_transaction_touch(id);
  AccountEntry *entry = &acct_table.entries[id];
  /* The generation is ordinary rollback-visible state. Record its inverse
   * before publishing the new storage view; repeated clears therefore unwind
   * in strict journal order without copying or scanning any storage row. BAL
   * reconciliation observes the generation change once at transaction merge. */
  state_journal_push_account_storage_generation(
      id, entry->storage_generation);
  /* TODO(proof): establish in Sail that the number of storage clears for one
   * account cannot saturate StorageGeneration during a block. */
  entry->storage_generation++;
}

/* ======================================================================== */
/* LIFECYCLE AND ROLLBACK                                                   */
/* ======================================================================== */

/* Resets all account schema and transaction cursors for a new execution.
 * This is a native/guest world reset, not an operation exposed to Sail. The
 * workspace allocation itself is retained. */
unit acct_db_reset(const unit u) {
  (void)u;
  if (acct_table.bucket_count != 0)
    memset(acct_table.buckets, 0,
           acct_table.bucket_count * sizeof(*acct_table.buckets));
  if (acct_table.count > 1u) {
    memset(acct_table.entries + 1u, 0,
           (size_t)(acct_table.count - 1u) * sizeof(*acct_table.entries));
    memset(acct_table.states + 1u, 0,
           (size_t)(acct_table.count - 1u) * sizeof(*acct_table.states));
  }
  acct_table.count = 1u;
  acct_table.bucket_count = 0;
  acct_table.bucket_mask = 0;
  acct_tx_rows_n = 0;
  return UNIT;
}

/* Discards transaction-only metadata after merge or whole-transaction revert.
 * Semantic values have already been committed directly or restored by the
 * shared journal. */
unit acct_tx_reset(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < acct_tx_rows_n; i++) {
    AccountState *state = &acct_table.states[acct_tx_rows[i]];
    state->transaction_epoch = 0;
    state->transaction_storage_count = 0;
    state->created = 0;
    state->selfdestructed = 0;
  }
  acct_tx_rows_n = 0;
  return UNIT;
}

/* Materializes one BAL account's authenticated pre-state.
 *
 * Only the mutable semantic value is copied into AccountState. The immutable
 * original account encoding remains borrowed from the witness node arena; its
 * lookup terminal and storage-root NodeIds are retained in AccountTrieBinding.
 * Bindings are written at AccountId - 1 while BAL loading is still in stable
 * AccountId order. */
unit account_block_initialize(AccountId id, Hash32 address_hash,
                              uint64_t nonce, U256 bal,
                              NodeId storage_root_node, Hash32 chash,
                              bool exists, bool storage_cleared,
                              NodeId terminal_node) {
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count ||
      terminal_node == EVMSAIL_NODE_ID_UNLINKED ||
      storage_root_node == EVMSAIL_NODE_ID_UNLINKED ||
      id > GUEST_STATE_ACCOUNTS)
    GUEST_ABORT();
  AccountState *state = &acct_table.states[id];
  state->current.nonce = nonce;
  state->current.balance = bal;
  state->current.code_hash = chash;
  state->current.exists = exists;
  if (storage_cleared)
    acct_table.entries[id].storage_generation++;

  AccountTrieBinding *binding = &acct_table.trie_bindings[id - 1u];
  *binding = (AccountTrieBinding){
      .account_id = id,
      .secure_key = address_hash,
      .terminal_node = terminal_node,
      .storage_root_node = storage_root_node,
      .prestate_exists = exists,
  };
  return UNIT;
}

/* ======================================================================== */
/* SEMANTIC READS AND AUTHENTICATED FINALIZATION                            */
/* ======================================================================== */

/* Returns the active transaction overlay when this account has been touched
 * in the current transaction. The storage root remains authenticated trie
 * metadata and is assembled with the semantic value at this boundary. */
bool account_transaction_view(const Address *address, AccountView *view) {
  const AccountId id = lookup_account_id(address);
  if (id == ACCOUNT_ID_NONE) return 0;
  const AccountState *state = &acct_table.states[id];
  if (state->transaction_epoch != current_warm_epoch) return false;
  const AccountTrieBinding *binding = &acct_table.trie_bindings[id - 1u];
  if (binding->account_id != id) GUEST_ABORT();
  view->value = state->current;
  view->storage_root = mpt_storage_root_hash(binding->storage_root_node);
  view->storage_cleared =
      acct_table.entries[id].storage_generation != STORAGE_INITIAL_GENERATION;
  view->created = state->created;
  view->selfdestructed = state->selfdestructed;
  return true;
}

/* Returns the value frozen at transaction start, or the committed block value
 * when the account has not yet been touched in this transaction. */
bool account_block_view(const Address *address, AccountView *view) {
  const AccountId id = lookup_account_id(address);
  if (id == ACCOUNT_ID_NONE) return false;
  const AccountState *state = &acct_table.states[id];
  const AccountTrieBinding *binding = &acct_table.trie_bindings[id - 1u];
  if (binding->account_id != id) GUEST_ABORT();
  const bool active = state->transaction_epoch == current_warm_epoch;
  view->value = active ? state->original : state->current;
  view->storage_root = mpt_storage_root_hash(binding->storage_root_node);
  const StorageGeneration generation =
      active ? state->original_storage_generation
             : acct_table.entries[id].storage_generation;
  view->storage_cleared = generation != STORAGE_INITIAL_GENERATION;
  view->created = false;
  view->selfdestructed = false;
  return true;
}

/* Cursor implementing the Sail account-trie iterator contract. Calling begin
 * first orders AccountTrieBinding in the MPT module's reusable scratch. */
static uint32_t acct_block_iter_position = 0;
static bool acct_block_iter_active = false;

/* Number of real account trie bindings; AccountId sentinel row zero is absent. */
uint32_t account_trie_binding_count(void) {
  return acct_table.count - 1u;
}

/* Exposes only the compact sort key needed by the MPT-owned sorter. Before the
 * final permutation, index is the binding-array position, not an AccountId. */
void account_trie_binding_order_key(uint32_t index, NodeId *terminal_node,
                                    Hash32 *secure_key) {
  if (index >= account_trie_binding_count()) GUEST_ABORT();
  const AccountTrieBinding *binding = &acct_table.trie_bindings[index];
  if (binding->account_id == ACCOUNT_ID_NONE ||
      binding->account_id >= acct_table.count ||
      binding->terminal_node == EVMSAIL_NODE_ID_UNLINKED)
    GUEST_ABORT();
  *terminal_node = binding->terminal_node;
  *secure_key = binding->secure_key;
}

/* MPT finalization owns ordering and supplies a source-index-to-destination
 * permutation in its reusable scratch area. Applying it here keeps bindings
 * inside AccountTable and moves every full binding only along its one cycle. */
void account_trie_bindings_permute(uint32_t *destinations, uint32_t count) {
  if (count != account_trie_binding_count()) GUEST_ABORT();
  for (uint32_t source = 0; source < count; source++) {
    while (destinations[source] != source) {
      const uint32_t destination = destinations[source];
      if (destination >= count) GUEST_ABORT();
      const AccountTrieBinding temporary = acct_table.trie_bindings[source];
      acct_table.trie_bindings[source] =
          acct_table.trie_bindings[destination];
      acct_table.trie_bindings[destination] = temporary;
      const uint32_t destination_swap = destinations[source];
      destinations[source] = destinations[destination];
      destinations[destination] = destination_swap;
    }
  }
}

/* Starts Sail-model iteration in canonical update order. The preparatory sort
 * is idempotent and changes only AccountTrieBinding order, never AccountIds. */
unit acct_block_iter_begin(const unit u) {
  (void)u;
  (void)mpt_account_updates_prepare();
  acct_block_iter_position = 0;
  acct_block_iter_active = true;
  return UNIT;
}

/* Resolves an ordered binding position to its stable semantic AccountId. */
static AccountId acct_block_update_id_at(uint32_t index) {
  if (index >= account_trie_binding_count()) GUEST_ABORT();
  const AccountTrieBinding *binding = &acct_table.trie_bindings[index];
  const AccountId id = binding->account_id;
  if (id == ACCOUNT_ID_NONE || id >= acct_table.count ||
      binding->terminal_node == EVMSAIL_NODE_ID_UNLINKED)
    GUEST_ABORT();
  return id;
}

/* EIP-161 account emptiness over semantic fields; existence is considered by
 * the caller because an absent account and an empty existing account differ. */
static bool acct_state_empty(const AccountState *state) {
  return state->current.nonce == 0 &&
         word_equal(&state->current.balance, &account_zero) &&
         hash_equal(&state->current.code_hash, &EVMSAIL_KECCAK_EMPTY);
}

/* Supplies a minimal borrowed binding view to account-trie consumers.
 *
 * A transaction-local storage clear selects the explicit empty NodeId as the
 * base. fields_changed covers account fields only; storage changes are folded
 * into the storage root while this binding is processed. */
bool account_trie_binding_get(uint32_t index, AccountTrieView *view) {
  if (view == NULL || index >= account_trie_binding_count()) return false;
  const AccountId id = acct_block_update_id_at(index);
  const AccountState *state = &acct_table.states[id];
  const AccountTrieBinding *binding = &acct_table.trie_bindings[index];
  view->account_id = id;
  view->secure_key = &binding->secure_key;
  view->storage_base_node =
      acct_table.entries[id].storage_generation != STORAGE_INITIAL_GENERATION
                           ? EVMSAIL_NODE_ID_EMPTY
                           : binding->storage_root_node;
  view->original_storage_root_node = binding->storage_root_node;
  view->terminal_node = binding->terminal_node;
  view->nonce = state->current.nonce;
  view->balance = &state->current.balance;
  view->code_hash = &state->current.code_hash;
  view->current_live = state->current.exists && !acct_state_empty(state);
  view->original_exists = binding->prestate_exists;
  view->fields_changed = state->dirty;
  return true;
}

/* Materializes the next ordered binding for the explicit Sail state-trie
 * equations retained by standard C and proof extraction. Original fields
 * intentionally mirror the current semantic value; optimized root reduction
 * compares current account RLP against the authenticated pre-state RLP. */
uint64_t acct_block_iter_next_probe(Address *addr, uint64_t *cn,
                                    U256 *cb, Hash32 *cs,
                                    Hash32 *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    U256 *ob, Hash32 *os,
                                    Hash32 *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd,
                                    Hash32 *address_hash) {
  if (!acct_block_iter_active) return 0;
  while (acct_block_iter_position < account_trie_binding_count()) {
    const AccountTrieBinding *binding =
        &acct_table.trie_bindings[acct_block_iter_position++];
    const AccountId id = binding->account_id;
    if (id == ACCOUNT_ID_NONE || id >= acct_table.count ||
        binding->terminal_node == EVMSAIL_NODE_ID_UNLINKED)
      continue;

    const AccountState *state = &acct_table.states[id];
    const Hash32 storage_root =
        mpt_storage_root_hash(binding->storage_root_node);
    *addr = acct_table.entries[id].address;
    *cn = state->current.nonce;
    *cb = state->current.balance;
    *cs = storage_root;
    *cc = state->current.code_hash;
    *ce = state->current.exists;
    *csc = acct_table.entries[id].storage_generation !=
           account_transaction_original_storage_generation(id);
    *ccr = false;
    *csd = false;
    *on = state->current.nonce;
    *ob = state->current.balance;
    *os = storage_root;
    *oc = state->current.code_hash;
    *oe = binding->prestate_exists;
    *osc = false;
    *ocr = false;
    *osd = false;
    if (address_hash) *address_hash = binding->secure_key;
    return 1;
  }
  return 0;
}

/* Returns the directly mutable transaction view after capturing its
 * transaction-start value. Callers journal only the field they mutate. */
static AccountState *account_state_for_write(Address address,
                                             AccountId *id_out) {
  const AccountId id = get_account_id(&address);
  if (have_exception) return NULL;
  account_transaction_touch(id);
  *id_out = id;
  return &acct_table.states[id];
}

/* Replaces every semantic account field in the active transaction view.
 * Storage roots remain trie metadata. A logical clear advances the owning
 * AccountEntry generation rather than adding a boolean to AccountState. */
unit host_account_update(Address a, uint64_t nonce, U256 bal, Hash32 chash,
                    bool exists, bool storage_cleared, bool created,
                    bool selfdestructed) {
  const AccountId id = get_account_id(&a);
  if (have_exception) return UNIT;
  account_transaction_touch(id);
  AccountState *state = &acct_table.states[id];
  if (state->current.nonce != nonce) {
    state_journal_push_account_nonce(id, state->current.nonce);
    state->current.nonce = nonce;
  }
  if (!word_equal(&state->current.balance, &bal)) {
    state_journal_push_account_balance(id, state->current.balance);
    state->current.balance = bal;
  }
  if (!hash_equal(&state->current.code_hash, &chash)) {
    state_journal_push_account_code_hash(id, state->current.code_hash);
    state->current.code_hash = chash;
  }
  if (state->current.exists != exists) {
    state_journal_push_account_exists(id, state->current.exists);
    state->current.exists = exists;
  }
  const bool next_created = state->created || created;
  if (state->created != next_created) {
    state_journal_push_account_created(id, state->created);
    state->created = next_created;
  }
  if (state->selfdestructed != selfdestructed) {
    state_journal_push_account_selfdestructed(id, state->selfdestructed);
    state->selfdestructed = selfdestructed;
  }
  if (storage_cleared &&
      acct_table.entries[id].storage_generation ==
          state->original_storage_generation)
    account_clear_storage_generation(id);
  return UNIT;
}

/* Updates only the transaction-visible balance. */
unit acct_tx_set_balance(Address a, const U256 balance) {
  AccountId id;
  AccountState *state = account_state_for_write(a, &id);
  if (state && !word_equal(&state->current.balance, &balance)) {
    state_journal_push_account_balance(id, state->current.balance);
    state->current.balance = balance;
  }
  return UNIT;
}

/* Updates only the transaction-visible nonce. */
unit acct_tx_set_nonce(Address a, uint64_t nonce) {
  AccountId id;
  AccountState *state = account_state_for_write(a, &id);
  if (state && state->current.nonce != nonce) {
    state_journal_push_account_nonce(id, state->current.nonce);
    state->current.nonce = nonce;
  }
  return UNIT;
}

/* Updates only the transaction-visible code hash. */
unit acct_tx_set_code_hash(Address a, Hash32 code_hash) {
  AccountId id;
  AccountState *state = account_state_for_write(a, &id);
  if (state && !hash_equal(&state->current.code_hash, &code_hash)) {
    state_journal_push_account_code_hash(id, state->current.code_hash);
    state->current.code_hash = code_hash;
  }
  return UNIT;
}

/*
 * Commits the final transaction overlay into AccountState.
 *
 * acct_tx_rows order is deliberately unobservable. Account lifecycle effects
 * are resolved before storage.c drains the transaction overlay so deletion and
 * storage-clear decisions select the correct storage incarnation. BAL changes
 * are emitted only for semantic fields whose committed values actually differ.
 * dirty is monotonic for the block and drives final account-trie consideration.
 * The standard executable retains the structurally identical Sail equation.
 */
void account_transaction_merge(struct TransactionMergeSemantics semantics,
                               uint64_t current_transaction_epoch) {
  for (uint32_t i = 0; i < acct_tx_rows_n; i++) {
    const AccountId account_id = acct_tx_rows[i];
    AccountState *state = &acct_table.states[account_id];
    AccountEntry *entry = &acct_table.entries[account_id];
    const Address address = acct_table.entries[account_id].address;
    const bool deleted = state->selfdestructed &&
                         (!semantics.delete_only_created || state->created);

    if (deleted) {
      state->current.nonce = 0;
      state->current.code_hash = EVMSAIL_KECCAK_EMPTY;
      /* Deletion retires the complete transaction-visible storage
       * incarnation, including an incarnation established by CREATE earlier
       * in this transaction. Advancing only the transaction-start generation
       * leaves post-CREATE writes visible after SELFDESTRUCT and can make a
       * later CREATE2 incorrectly collide with the deleted account. */
      entry->storage_generation++;
      if (!semantics.preserve_selfdestruct_balance ||
          word_equal(&state->current.balance, &account_zero)) {
        state->current.balance = account_zero;
        state->current.exists = false;
      } else {
        state->current.exists = true;
      }
    }

    const bool storage_was_cleared =
        entry->storage_generation !=
        state->original_storage_generation;
    if (deleted || storage_was_cleared)
      storage_block_clear(address);

    if (state->current.nonce != state->original.nonce)
      bal_note_nonce_change(current_transaction_epoch, address,
                            state->current.nonce);
    if (!word_equal(&state->current.balance, &state->original.balance))
      bal_note_balance_change(
          current_transaction_epoch, address, state->current.balance);
    if (!hash_equal(&state->current.code_hash, &state->original.code_hash))
      bal_note_code_change(current_transaction_epoch, address,
                           state->current.code_hash);

    const bool changed =
        state->current.nonce != state->original.nonce ||
        !word_equal(&state->current.balance, &state->original.balance) ||
        !hash_equal(&state->current.code_hash, &state->original.code_hash) ||
        state->current.exists != state->original.exists ||
        storage_was_cleared;
    if (changed) state->dirty = 1;
  }

}

/* Assigns all account-owned arrays from the single preallocated guest
 * workspace. This performs no heap allocation and does not initialize a block;
 * acct_db_reset/account_schema_prepare establish logical contents later. */
void account_state_workspace_bind(void) {
  WORKSPACE_BIND(acct_table.entries, GUEST_STATE_ACCOUNTS + 1u);
  WORKSPACE_BIND(acct_table.states, GUEST_STATE_ACCOUNTS + 1u);
  WORKSPACE_BIND(acct_table.trie_bindings, GUEST_STATE_ACCOUNTS);
  WORKSPACE_BIND(acct_table.buckets, GUEST_ACCOUNT_INDEX_BUCKETS);
  WORKSPACE_BIND(acct_tx_rows, GUEST_STATE_ACCOUNTS);
  WORKSPACE_BIND(transaction_storage_ids, GUEST_STATE_STORAGE);
}
