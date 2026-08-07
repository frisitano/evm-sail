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
#include "evmsail/prelude.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/host/nodes.h"
#include "evmsail/spec/exceptions.h"
#include "host/state/account.h"
#include "evmsail/spec/kernel/lifecycle.h"
#include "host/state/block_access_list.h"
#include "host/state/internal.h"
#include "host/state/journal.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  bytes20 address;
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
  bytes32 secure_key;
  /* Witness node at which authenticated account lookup terminated. */
  NodeId terminal_node;
  /* Root NodeId of the account's authenticated pre-state storage trie. */
  NodeId storage_root_node;
  /* Canonical digest retained even when the root node is not witnessed. */
  bytes32 storage_root;
  /* Whether authenticated pre-state contained this account. */
  uint8_t prestate_exists;
} AccountTrieBinding;

/* Complete account subsystem allocation, backed by the shared guest workspace.
 *
 * entries and states are indexed directly by AccountId and include sentinel
 * row zero. trie_bindings has one element per real account and may be reordered
 * independently. buckets is a power-of-two open-addressed address index;
 * ACCOUNT_ID_NONE marks an empty zeroed bucket and bucket_mask
 * implements modulo by bucket_count. count includes the sentinel, whereas
 * capacity does too. */
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
    .count = 1U,
};

/* The active EVM frame has one deterministic state-owning account. Keep its
 * stable AccountId together with a direct workspace-backed row pointer so
 * frame-local state operations do not repeatedly probe or compare addresses. */
typedef struct {
  AccountId id;
  AccountEntry *entry;
} CurrentAccountContext;

static CurrentAccountContext current_account_context;

/* Distinct AccountIds touched by the current transaction. */
static AccountId *acct_tx_rows;
static uint32_t acct_tx_rows_n = 0;

/* StorageIds touched by a transaction are written into the owning account's
 * BAL-defined storage interval. Each account therefore has an exact-capacity
 * worklist without growth, relocation, or a second global lookup structure. */
static StorageId *transaction_storage_ids;

/* Finds an address in the BAL-preallocated account universe.
 *
 * The first address bytes supply the initial bucket because Ethereum addresses
 * are already hash-derived. Linear probing resolves collisions and full
 * bytes20 equality validates a candidate. A miss returns ACCOUNT_ID_NONE;
 * execution APIs decide whether that miss is a BAL violation. */
AccountId lookup_account_id(const bytes20 *address)
{
  uint32_t bucket = 0;
  bucket = (uint32_t)address->lanes[0];
  bucket &= acct_table.bucket_mask;
  for (;;) {
    const AccountId id = acct_table.buckets[bucket];
    if (id == ACCOUNT_ID_NONE) {
      return ACCOUNT_ID_NONE;
    }
    if (address_equal(&acct_table.entries[id].address, address)) {
      return id;
    }
    bucket = (bucket + 1U) & acct_table.bucket_mask;
  }
}

/* Resolves a required execution account. The BAL is the complete block-local
 * schema, so absence is a protocol error rather than a dynamic insertion. */
AccountId get_account_id(const bytes20 *address)
{
  const AccountId id = lookup_account_id(address);
  if (id == ACCOUNT_ID_NONE) {
    fatal_error(InvalidBlockAccessList);
  }
  return id;
}

void current_account_context_invalidate(void)
{
  current_account_context.id = ACCOUNT_ID_NONE;
  current_account_context.entry = NULL;
}

/* Installs the current frame's state owner once. External account lookups never
 * update this context. */
void current_account_context_enter(bytes20 address)
{
  const AccountId id = lookup_account_id(&address);
  if (id == ACCOUNT_ID_NONE) {
    fatal_error(InvalidBlockAccessList);
  }
  current_account_context.id = id;
  current_account_context.entry = &acct_table.entries[id];
}

AccountId current_account_context_id(void)
{
  if (current_account_context.entry == NULL) {
    GUEST_ABORT();
  }
  return current_account_context.id;
}

/* Suspended frames retain only the stable dense ID. Reconstructing the pointer
 * is O(1) and avoids storing a process pointer in Sail-visible continuation
 * state. */
void current_account_context_restore(AccountId id)
{
  if (id == ACCOUNT_ID_NONE) {
    current_account_context_invalidate();
    return;
  }
  if (id >= acct_table.count) {
    GUEST_ABORT();
  }
  current_account_context.id = id;
  current_account_context.entry = &acct_table.entries[id];
}

/* Reports whether storage updates may contribute to this account's post-state.
 * A nonexistent account cannot own live storage. */
bool account_exists(AccountId id)
{
  return acct_table.states[id].current.exists != 0U;
}

/* Appends one BAL account and installs it in the address index.
 *
 * BAL loading must present strictly increasing, duplicate-free addresses. This
 * gives deterministic AccountIds and validates canonical BAL order while the
 * schema is built. Runtime account creation never reaches this function. */
AccountId account_schema_insert(const bytes20 *address)
{
  if (acct_table.count >= acct_table.capacity || acct_table.bucket_count == 0) {
    GUEST_ABORT();
  }
  if (acct_table.count > ACCOUNT_ID_NONE + 1U &&
      address_compare(&acct_table.entries[acct_table.count - 1U].address, address) >= 0) {
    fatal_error(InvalidBlockAccessList);
  }

  uint32_t bucket = 0;
  bucket = (uint32_t)address->lanes[0];
  bucket &= acct_table.bucket_mask;
  while (acct_table.buckets[bucket] != ACCOUNT_ID_NONE) {
    const AccountId existing = acct_table.buckets[bucket];
    if (address_equal(&acct_table.entries[existing].address, address)) {
      fatal_error(InvalidBlockAccessList);
    }
    bucket = (bucket + 1U) & acct_table.bucket_mask;
  }

  const AccountId id = acct_table.count++;
  acct_table.entries[id] = (AccountEntry){
      .address = *address,
      .storage_begin = UINT32_MAX,
      .storage_count = 0,
      .warm_epoch = 0,
      .storage_generation = STORAGE_INITIAL_GENERATION,
  };
  acct_table.states[id] = (AccountState){0};
  acct_table.buckets[bucket] = id;
  bal_account_schema_initialize(id);
  return id;
}

/* A missing BAL row is observably cold. The later semantic account access is
 * responsible for rejecting a deficient BAL after the access charge is paid. */
bool account_is_warm(bytes20 address)
{
  const AccountId id = lookup_account_id(&address);
  if (id == ACCOUNT_ID_NONE) {
    return false;
  }
  return acct_table.entries[id].warm_epoch >= current_warm_epoch;
}

/* Marks an existing BAL account warm. A missing row remains untracked so a
 * charge-only path can halt out of gas without spuriously rejecting the BAL. */
void account_mark_warm(bytes20 address)
{
  const AccountId id = lookup_account_id(&address);
  if (id == ACCOUNT_ID_NONE) {
    return;
  }

  uint32_t *warm_epoch = &acct_table.entries[id].warm_epoch;
  if (*warm_epoch < current_warm_epoch) {
    const uint32_t prior_epoch = *warm_epoch;
    state_journal_push_warm_account(id, prior_epoch);
    *warm_epoch = current_warm_epoch;
  }
}

/* Applies the inverse of the first warm access recorded by the state journal. */
void account_warm_restore(AccountId id, uint32_t prior_epoch)
{
  acct_table.entries[id].warm_epoch = prior_epoch;
}

/* Returns the next AccountId; row zero is included in this count. */
uint32_t account_id_count(void)
{
  return acct_table.count;
}

/* Returns the canonical address belonging to a stable AccountId. */
const bytes20 *account_id_address(AccountId id)
{
  return &acct_table.entries[id].address;
}

/* Binds the one contiguous StorageId interval assigned during BAL loading.
 * Binding is single-assignment and remains stable for the block. */
void account_storage_range_bind(AccountId id, StorageId begin, uint32_t count)
{
  AccountEntry *entry = &acct_table.entries[id];
  entry->storage_begin = begin;
  entry->storage_count = count;
}

/* Reads an account's already-bound StorageId interval. */
void account_storage_range(AccountId id, StorageId *begin, uint32_t *count)
{
  const AccountEntry *entry = &acct_table.entries[id];
  *begin = entry->storage_begin;
  *count = entry->storage_count;
}

/* Confirms that the decoded BAL shape matches the exact workspace extent and
 * its at-most-half-full power-of-two lookup index. */
void account_schema_prepare(uint32_t account_count)
{
  if (account_count > UINT32_MAX / 2U) {
    GUEST_ABORT();
  }
  uint32_t bucket_count = 1U;
  const uint32_t required = account_count == 0 ? 1U : 2U * account_count;
  while (bucket_count < required) {
    if (bucket_count > UINT32_MAX / 2U) {
      GUEST_ABORT();
    }
    bucket_count <<= 1;
  }
  if (account_count + 1U != acct_table.capacity || bucket_count != acct_table.bucket_count) {
    GUEST_ABORT();
  }
}

/* Captures the transaction-start semantic value once. The shared journal owns
 * rollback; this transaction metadata exists only for SSTORE/BAL comparison
 * and for exact-capacity touched-row iteration at commit. */
void account_transaction_touch(AccountId id)
{
  AccountState *state = &acct_table.states[id];
  if (state->transaction_epoch == current_warm_epoch) {
    return;
  }
  state->original = state->current;
  state->original_storage_generation = acct_table.entries[id].storage_generation;
  state->transaction_epoch = current_warm_epoch;
  state->transaction_storage_count = 0;
  state->created = 0;
  state->selfdestructed = 0;
  state_journal_push_account_transaction_listed();
  acct_tx_rows[acct_tx_rows_n++] = id;
}

/* Applies one field inverse recorded by the shared state journal. */
void account_field_restore(AccountId id, AccountRestoreField field, AccountRestorePrior prior)
{
  AccountState *state = &acct_table.states[id];
  switch (field) {
  case ACCOUNT_FIELD_BALANCE:
    state->current.balance = prior.balance;
    break;
  case ACCOUNT_FIELD_NONCE:
    state->current.nonce = prior.nonce;
    break;
  case ACCOUNT_FIELD_CODE_HASH:
    state->current.code_hash = prior.code_hash;
    break;
  case ACCOUNT_FIELD_EXISTS:
    state->current.exists = (uint8_t)prior.flag;
    break;
  case ACCOUNT_FIELD_CREATED:
    state->created = (uint8_t)prior.flag;
    break;
  case ACCOUNT_FIELD_SELFDESTRUCTED:
    state->selfdestructed = (uint8_t)prior.flag;
    break;
  }
}

uint32_t account_transaction_count(void)
{
  return acct_tx_rows_n;
}

AccountId account_transaction_id_at(uint32_t index)
{
  return acct_tx_rows[index];
}

void account_transaction_pop_last(void)
{
  AccountState *state = &acct_table.states[acct_tx_rows[--acct_tx_rows_n]];
  state->transaction_epoch = 0;
  state->transaction_storage_count = 0;
  state->created = 0;
  state->selfdestructed = 0;
}

/* Stores transaction-touched StorageIds in the owning account's exact BAL
 * interval. No auxiliary capacity, growth, or relocation is required. */
void account_transaction_storage_append(AccountId id, StorageId storage_id)
{
  AccountEntry *entry = &acct_table.entries[id];
  AccountState *state = &acct_table.states[id];
  state_journal_push_account_storage_listed(id);
  transaction_storage_ids[entry->storage_begin + state->transaction_storage_count++] = storage_id;
}

void account_transaction_storage_pop_last(AccountId id)
{
  AccountState *state = &acct_table.states[id];
  const StorageId storage_id = transaction_storage_ids[acct_table.entries[id].storage_begin +
                                                       --state->transaction_storage_count];
  storage_transaction_forget(storage_id);
}

uint32_t account_transaction_storage_count(AccountId id)
{
  return acct_table.states[id].transaction_storage_count;
}

StorageId account_transaction_storage_id_at(AccountId id, uint32_t index)
{
  const AccountEntry *entry = &acct_table.entries[id];
  return transaction_storage_ids[entry->storage_begin + index];
}

/* Returns the account-wide storage incarnation visible to storage.c. */
StorageGeneration account_storage_generation(AccountId id)
{
  return acct_table.entries[id].storage_generation;
}

/* Returns the storage incarnation visible at transaction entry. Accounts not
 * yet touched in this transaction still observe their current block value. */
StorageGeneration account_transaction_original_storage_generation(AccountId id)
{
  const AccountState *state = &acct_table.states[id];
  return state->transaction_epoch == current_warm_epoch ? state->original_storage_generation
                                                        : acct_table.entries[id].storage_generation;
}

/* Restores a generation recorded by the shared state journal during revert. */
void account_storage_generation_restore(AccountId id, StorageGeneration generation)
{
  acct_table.entries[id].storage_generation = generation;
}

/* Clears storage logically in O(1) by advancing the account incarnation.
 * Existing slot rows remain allocated but become invisible until rewritten in
 * the new generation. The generation has its own journal event because a
 * clear and a slot-value change have distinct rollback semantics. */
void account_clear_storage_generation(AccountId id)
{
  account_transaction_touch(id);
  AccountEntry *entry = &acct_table.entries[id];
  /* The generation is ordinary rollback-visible state. Record its inverse
   * before publishing the new storage view; repeated clears therefore unwind
   * in strict journal order without copying or scanning any storage row. BAL
   * reconciliation observes the generation change once at transaction merge. */
  state_journal_push_account_storage_generation(id, entry->storage_generation);
  if (entry->storage_generation == UINT32_MAX) {
    GUEST_ABORT();
  }
  entry->storage_generation++;
}

/* ======================================================================== */
/* LIFECYCLE AND ROLLBACK                                                   */
/* ======================================================================== */

/* Resets all account schema and transaction cursors for a new execution.
 * This is a native/guest world reset, not an operation exposed to Sail. The
 * workspace allocation itself is retained. */
#ifdef EVMSAIL_NATIVE_TEST
void acct_db_reset(void)
{
  acct_table.count = 1u;
  acct_tx_rows_n = 0;
  current_account_context_invalidate();
  return;
}
#endif

/* Discards transaction-only metadata after merge or whole-transaction revert.
 * Semantic values have already been committed directly or restored by the
 * shared journal. */
void acct_tx_reset(void)
{
  for (uint32_t i = 0; i < acct_tx_rows_n; i++) {
    AccountState *state = &acct_table.states[acct_tx_rows[i]];
    state->transaction_epoch = 0;
    state->transaction_storage_count = 0;
    state->created = 0;
    state->selfdestructed = 0;
  }
  acct_tx_rows_n = 0;
}

/* Materializes one BAL account's authenticated pre-state.
 *
 * Only the mutable semantic value is copied into AccountState. The immutable
 * original account encoding remains borrowed from the witness node arena; its
 * lookup terminal and storage-root NodeIds are retained in AccountTrieBinding.
 * Bindings are written at AccountId - 1 while BAL loading is still in stable
 * AccountId order. */
void account_block_initialize(AccountId account_id, bytes32 address_hash, uint64_t nonce,
                              u256 balance, bytes32 storage_root, NodeId storage_root_node,
                              bytes32 code_hash, bool exists, bool storage_cleared,
                              NodeId terminal_node)
{
  AccountState *state = &acct_table.states[account_id];
  state->current.nonce = nonce;
  state->current.balance = balance;
  state->current.code_hash = code_hash;
  state->current.exists = (uint8_t)exists;
  if (storage_cleared) {
    acct_table.entries[account_id].storage_generation++;
  }

  AccountTrieBinding *binding = &acct_table.trie_bindings[account_id - 1U];
  *binding = (AccountTrieBinding){
      .account_id = account_id,
      .secure_key = address_hash,
      .terminal_node = terminal_node,
      .storage_root_node = storage_root_node,
      .storage_root = storage_root,
      .prestate_exists = (uint8_t)exists,
  };
}

/* ======================================================================== */
/* SEMANTIC READS AND AUTHENTICATED FINALIZATION                            */
/* ======================================================================== */

/* Returns the active transaction overlay when this account has been touched
 * in the current transaction. The storage root remains authenticated trie
 * metadata and is assembled with the semantic value at this boundary. */
bool account_transaction_view(const bytes20 *address, AccountView *view)
{
  const AccountId id = lookup_account_id(address);
  if (id == ACCOUNT_ID_NONE) {
    return false;
  }
  const AccountState *state = &acct_table.states[id];
  if (state->transaction_epoch != current_warm_epoch) {
    return false;
  }
  const AccountTrieBinding *binding = &acct_table.trie_bindings[id - 1U];
  if (binding->account_id != id) {
    GUEST_ABORT();
  }
  view->value = state->current;
  view->storage_root = binding->storage_root;
  view->storage_cleared = acct_table.entries[id].storage_generation != STORAGE_INITIAL_GENERATION;
  view->created = (state->created != 0U);
  view->selfdestructed = (state->selfdestructed != 0U);
  return true;
}

/* Returns the value frozen at transaction start, or the committed block value
 * when the account has not yet been touched in this transaction. */
bool account_block_view(const bytes20 *address, AccountView *view)
{
  const AccountId id = lookup_account_id(address);
  if (id == ACCOUNT_ID_NONE) {
    return false;
  }
  const AccountState *state = &acct_table.states[id];
  const AccountTrieBinding *binding = &acct_table.trie_bindings[id - 1U];
  if (binding->account_id != id) {
    GUEST_ABORT();
  }
  const bool active = state->transaction_epoch == current_warm_epoch;
  view->value = (int)active ? state->original : state->current;
  view->storage_root = binding->storage_root;
  const StorageGeneration generation =
      (int)active ? state->original_storage_generation : acct_table.entries[id].storage_generation;
  view->storage_cleared = generation != STORAGE_INITIAL_GENERATION;
  view->created = false;
  view->selfdestructed = false;
  return true;
}

/* Number of real account trie bindings; AccountId sentinel row zero is absent. */
uint32_t account_trie_binding_count(void)
{
  return acct_table.count - 1U;
}

/* Exposes only the compact sort key needed by the MPT-owned sorter. Before the
 * final permutation, index is the binding-array position, not an AccountId. */
void account_trie_binding_order_key(uint32_t index, NodeId *terminal_node, bytes32 *secure_key)
{
  const AccountTrieBinding *binding = &acct_table.trie_bindings[index];
  *terminal_node = binding->terminal_node;
  *secure_key = binding->secure_key;
}

/* MPT finalization owns ordering and supplies a source-index-to-destination
 * permutation in its reusable scratch area. Applying it here keeps bindings
 * inside AccountTable and moves every full binding only along its one cycle. */
void account_trie_bindings_permute(uint32_t *destinations, uint32_t count)
{
  for (uint32_t source = 0; source < count; source++) {
    while (destinations[source] != source) {
      const uint32_t destination = destinations[source];
      const AccountTrieBinding temporary = acct_table.trie_bindings[source];
      acct_table.trie_bindings[source] = acct_table.trie_bindings[destination];
      acct_table.trie_bindings[destination] = temporary;
      const uint32_t destination_swap = destinations[source];
      destinations[source] = destinations[destination];
      destinations[destination] = destination_swap;
    }
  }
}

/* Supplies a minimal borrowed binding view to account-trie consumers.
 *
 * A transaction-local storage clear selects the explicit empty NodeId as the
 * base. fields_changed covers account fields only; storage changes are folded
 * into the storage root while this binding is processed. */
bool account_trie_binding_get(uint32_t index, AccountTrieView *view)
{
  const AccountTrieBinding *binding = &acct_table.trie_bindings[index];
  const AccountId id = binding->account_id;
  const AccountState *state = &acct_table.states[id];
  view->account_id = id;
  view->secure_key = &binding->secure_key;
  view->storage_base_node = acct_table.entries[id].storage_generation != STORAGE_INITIAL_GENERATION
                                ? EVMSAIL_NODE_ID_EMPTY
                                : binding->storage_root_node;
  view->storage_base_root = acct_table.entries[id].storage_generation != STORAGE_INITIAL_GENERATION
                                ? &EVMSAIL_EMPTY_TRIE_ROOT
                                : &binding->storage_root;
  view->original_storage_root = &binding->storage_root;
  view->terminal_node = binding->terminal_node;
  view->nonce = state->current.nonce;
  view->balance = &state->current.balance;
  view->code_hash = &state->current.code_hash;
  view->current_live = ((state->current.exists &&
                         !account_fields_empty(state->current.nonce, &state->current.balance,
                                               &state->current.code_hash)) != 0);
  view->original_exists = (binding->prestate_exists != 0U);
  view->fields_changed = (state->dirty != 0U);
  return true;
}

/* Returns the directly mutable transaction view after capturing its
 * transaction-start value. Callers journal only the field they mutate. */
static AccountState *account_state_for_write(bytes20 address, AccountId *id_out)
{
  const AccountId id = get_account_id(&address);
  account_transaction_touch(id);
  *id_out = id;
  return &acct_table.states[id];
}

/* Replaces every semantic account field in the active transaction view.
 * Storage roots remain trie metadata. A logical clear advances the owning
 * AccountEntry generation rather than adding a boolean to AccountState. */
void host_account_update(bytes20 address, uint64_t nonce, u256 balance, bytes32 code_hash,
                         bool exists, bool storage_cleared, bool created, bool selfdestructed)
{
  const AccountId id = get_account_id(&address);
  account_transaction_touch(id);
  AccountState *state = &acct_table.states[id];
  if (state->current.nonce != nonce) {
    state_journal_push_account_nonce(id, state->current.nonce);
    state->current.nonce = nonce;
  }
  if (!word_equal(&state->current.balance, &balance)) {
    state_journal_push_account_balance(id, state->current.balance);
    state->current.balance = balance;
  }
  if (!hash_equal(&state->current.code_hash, &code_hash)) {
    state_journal_push_account_code_hash(id, state->current.code_hash);
    state->current.code_hash = code_hash;
  }
  if (state->current.exists != (int)exists) {
    state_journal_push_account_exists(id, state->current.exists != 0U);
    state->current.exists = (uint8_t)exists;
  }
  const bool next_created = (state->created || created) != 0;
  if (state->created != (int)next_created) {
    state_journal_push_account_created(id, state->created != 0U);
    state->created = (uint8_t)next_created;
  }
  if (state->selfdestructed != (int)selfdestructed) {
    state_journal_push_account_selfdestructed(id, state->selfdestructed != 0U);
    state->selfdestructed = (uint8_t)selfdestructed;
  }
  if (storage_cleared &&
      acct_table.entries[id].storage_generation == state->original_storage_generation) {
    account_clear_storage_generation(id);
  }
}

/* Updates only the transaction-visible balance. */
void acct_tx_set_balance(bytes20 a, const u256 balance)
{
  AccountId id;
  AccountState *state = account_state_for_write(a, &id);
  if (state && !word_equal(&state->current.balance, &balance)) {
    state_journal_push_account_balance(id, state->current.balance);
    state->current.balance = balance;
  }
}

/* Updates only the transaction-visible nonce. */
void acct_tx_set_nonce(bytes20 a, uint64_t nonce)
{
  AccountId id;
  AccountState *state = account_state_for_write(a, &id);
  if (state && state->current.nonce != nonce) {
    state_journal_push_account_nonce(id, state->current.nonce);
    state->current.nonce = nonce;
  }
}

/* Updates only the transaction-visible code hash. */
void acct_tx_set_code_hash(bytes20 a, bytes32 code_hash)
{
  AccountId id;
  AccountState *state = account_state_for_write(a, &id);
  if (state && !hash_equal(&state->current.code_hash, &code_hash)) {
    state_journal_push_account_code_hash(id, state->current.code_hash);
    state->current.code_hash = code_hash;
  }
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
                               uint32_t current_transaction_epoch)
{
  for (uint32_t i = 0; i < acct_tx_rows_n; i++) {
    const AccountId account_id = acct_tx_rows[i];
    AccountState *state = &acct_table.states[account_id];
    AccountEntry *entry = &acct_table.entries[account_id];
    const bytes20 address = acct_table.entries[account_id].address;
    const bool deleted =
        (state->selfdestructed && (!semantics.delete_only_created || state->created)) != 0;

    if (deleted) {
      state->current.nonce = 0;
      state->current.code_hash = EVMSAIL_KECCAK_EMPTY;
      /* Deletion retires the complete transaction-visible storage
       * incarnation, including an incarnation established by CREATE earlier
       * in this transaction. Advancing only the transaction-start generation
       * leaves post-CREATE writes visible after SELFDESTRUCT and can make a
       * later CREATE2 incorrectly collide with the deleted account. */
      if (entry->storage_generation == UINT32_MAX) {
        GUEST_ABORT();
      }
      entry->storage_generation++;
      if (!semantics.preserve_selfdestruct_balance || word_all_zero(&state->current.balance)) {
        state->current.balance = (u256){{0}};
        state->current.exists = false;
      } else {
        state->current.exists = true;
      }
    }

    const bool storage_was_cleared =
        entry->storage_generation != state->original_storage_generation;

    if (state->current.nonce != state->original.nonce) {
      bal_note_nonce_change(current_transaction_epoch, address, state->current.nonce);
    }
    if (!word_equal(&state->current.balance, &state->original.balance)) {
      bal_note_balance_change(current_transaction_epoch, address, state->current.balance);
    }
    if (!hash_equal(&state->current.code_hash, &state->original.code_hash)) {
      bal_note_code_change(current_transaction_epoch, address, state->current.code_hash);
    }

    const bool changed =
        (state->current.nonce != state->original.nonce ||
         !word_equal(&state->current.balance, &state->original.balance) ||
         !hash_equal(&state->current.code_hash, &state->original.code_hash) ||
         state->current.exists != state->original.exists || storage_was_cleared) != 0;
    if (changed) {
      state->dirty = 1;
    }
  }
}

/* Assigns all account-owned arrays from the single preallocated guest
 * workspace. This performs no heap allocation and does not initialize a block;
 * acct_db_reset/account_schema_prepare establish logical contents later. */
void account_state_workspace_bind(uint32_t account_count, uint32_t storage_count)
{
  if (account_count > UINT32_MAX / 2U) {
    GUEST_ABORT();
  }
  uint32_t bucket_count = 1U;
  const uint32_t required = account_count == 0 ? 1U : 2U * account_count;
  while (bucket_count < required) {
    if (bucket_count > UINT32_MAX / 2U) {
      GUEST_ABORT();
    }
    bucket_count <<= 1;
  }
  acct_table.capacity = account_count + 1U;
  acct_table.bucket_count = bucket_count;
  acct_table.bucket_mask = bucket_count - 1U;
  WORKSPACE_BIND(acct_table.entries, account_count + 1U);
  WORKSPACE_BIND(acct_table.states, account_count + 1U);
  WORKSPACE_BIND(acct_table.trie_bindings, account_count);
  WORKSPACE_BIND(acct_table.buckets, bucket_count);
  WORKSPACE_BIND(acct_tx_rows, account_count);
  WORKSPACE_BIND(transaction_storage_ids, storage_count);
  acct_table.count = 1U;
  current_account_context_invalidate();
}
