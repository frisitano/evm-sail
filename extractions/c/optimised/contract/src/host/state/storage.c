/* Optimized persistent-storage semantic state and rollback journal.
 *
 * StorageId is the stable account-and-slot identity. This module exclusively
 * owns storage schema, current values, rollback state,
 * and storage-side authenticated-trie metadata. */
#include "evmsail/prelude.h"
#include "evmsail/host/nodes.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/spec/exceptions.h"
#include "host/state/storage.h"
#include "host/state/block_access_list.h"
#include "host/state/internal.h"
#include "host/state/journal.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================== */
/* PERSISTENT STORAGE STATE                                                  */
/*                                                                          */
/* Stable StorageIds index separate arrays grouped by field lifetime and
 * purpose. Storage-clear generations and rollback history are host-private:
 * Sail sees only the effective value and the frozen transaction-start
 * original. BAL preload constructs every row before execution; a later miss
 * is an invalid block-access-list access and never mutates the schema. */
/* ======================================================================== */

typedef struct {
  u256 slot;
  uint32_t warm_epoch;
} StorageSchema;

typedef struct {
  /* Immutable authenticated identity. The terminal belongs to the owning
   * account's storage-witness arena; encoded bytes remain in stateless input. */
  bytes32 secure_key;
  NodeId terminal_node;
  uint8_t prestate_exists;
} StorageTrieBinding;

typedef struct {
  StorageSchema *schema;
  StorageState *states;
  StorageTrieBinding *trie_bindings;
  uint32_t n, cap;
} StorageTable;

static StorageTable storage_table;

static const StorageId STORAGE_NO_ROW = UINT32_MAX;
static StorageId storage_schema_active_begin = 0;

static StorageId storage_resolve_slot_range(StorageId begin, uint32_t count, const u256 slot)
{
  StorageId low = begin;
  StorageId high = begin + count;
  while (low < high) {
    const StorageId middle = low + (high - low) / 2;
    const int ordering = word_compare(&storage_table.schema[middle].slot, &slot);
    if (ordering < 0) {
      low = middle + 1;
    } else if (ordering > 0) {
      high = middle;
    } else {
      return middle;
    }
  }
  return STORAGE_NO_ROW;
}

StorageId storage_resolve_slot(StorageId begin, uint32_t change_count, uint32_t count,
                               const u256 slot)
{
  if (change_count > count || begin > storage_table.n || count > storage_table.n - begin) {
    fatal_error(InvalidBlockAccessList);
  }
  StorageId storage_id = storage_resolve_slot_range(begin, change_count, slot);
  if (storage_id != STORAGE_NO_ROW) {
    return storage_id;
  }
  return storage_resolve_slot_range(begin + change_count, count - change_count, slot);
}

/* StorageIds are append-stable and grouped into the contiguous interval owned
 * by the active AccountEntry. BAL preload is the sole construction phase;
 * duplicate and unordered slots are rejected by the BAL loader. */
StorageId storage_schema_insert(AccountId account_id, const u256 *slot, const bytes32 *secure_key)
{
  (void)account_id;
  /* Capacity equals the measured BAL slot count that sized the workspace
   * binding, so insertion cannot exceed it. */
  const StorageId i = storage_table.n;
  StorageSchema *schema = &storage_table.schema[i];
  StorageTrieBinding *binding = &storage_table.trie_bindings[i];
  *schema = (StorageSchema){
      .slot = *slot,
      .warm_epoch = 0,
  };
  storage_table.states[i] = (StorageState){0};
  *binding = (StorageTrieBinding){
      .secure_key = *secure_key,
      .terminal_node = EVMSAIL_NODE_ID_UNLINKED,
      .prestate_exists = 0,
  };
  bal_storage_schema_initialize(i);
  storage_table.n++;
  return i;
}

/* Execution and BAL history operate only over the sealed, block-local
 * schema. A missing account/slot pair is therefore a BAL violation. */
StorageId get_storage_id(StorageId begin, uint32_t change_count, uint32_t count, const u256 *slot)
{
  const StorageId id = storage_resolve_slot(begin, change_count, count, *slot);
  if (id == STORAGE_NO_ROW) {
    fatal_error(InvalidBlockAccessList);
  }
  return id;
}

void storage_schema_account_begin(AccountId id)
{
  (void)id;
  storage_schema_active_begin = storage_table.n;
}

void storage_schema_account_end(AccountId id, uint32_t change_count)
{
  account_bind_storage_schema(id, storage_schema_active_begin, change_count,
                              storage_table.n - storage_schema_active_begin);
}

/* Construction ordering is owned by the single executor BAL loader. */
void storage_schema_seal(void) {}

bool storage_id_is_warm(StorageId storage_id)
{
  if (storage_id == STORAGE_NO_ROW) {
    return false;
  }
  return storage_table.schema[storage_id].warm_epoch >= current_warm_epoch;
}

void storage_mark_warm(bytes20 address, const u256 slot)
{
  const AccountId account_id = lookup_account_id(&address);
  if (account_id == ACCOUNT_ID_NONE) {
    return;
  }
  const struct AccountExecutionContext context = account_execution_context(address);
  const StorageId storage_id = storage_resolve_slot(
      context.storage_begin, context.storage_change_count, context.storage_count, slot);
  if (storage_id == STORAGE_NO_ROW) {
    return;
  }

  uint32_t *warm_epoch = &storage_table.schema[storage_id].warm_epoch;
  if (*warm_epoch < current_warm_epoch) {
    const uint32_t prior_epoch = *warm_epoch;
    state_journal_push_warm_storage(storage_id, prior_epoch);
    *warm_epoch = current_warm_epoch;
  }
}

void storage_id_mark_warm(StorageId storage_id)
{
  if (storage_id == STORAGE_NO_ROW) {
    return;
  }

  uint32_t *warm_epoch = &storage_table.schema[storage_id].warm_epoch;
  if (*warm_epoch < current_warm_epoch) {
    const uint32_t prior_epoch = *warm_epoch;
    state_journal_push_warm_storage(storage_id, prior_epoch);
    *warm_epoch = current_warm_epoch;
  }
}

void storage_warm_restore(StorageId id, uint32_t prior_epoch)
{
  storage_table.schema[id].warm_epoch = prior_epoch;
}

uint32_t storage_id_count(void)
{
  return storage_table.n;
}

const u256 *storage_id_slot(StorageId id)
{
  return &storage_table.schema[id].slot;
}

/* --- lifecycle -------------------------------------------------------- */

#ifdef EVMSAIL_NATIVE_TEST
/* native-harness reset surface */
/* Whole-overlay wipe (both layers): the harness world reset. */
void storage_db_reset(void)
{
  storage_table.n = 0;
  storage_schema_active_begin = 0;
  return;
}
#endif

/* Transaction scratch reset: retain the bound workspace for the next transaction. */
void storage_tx_reset(void)
{
  for (uint32_t i = 0; i < account_transaction_count(); i++) {
    AccountTransactionStorageView account;
    account_transaction_storage_view_at(i, &account);
    for (uint32_t j = 0; j < account.transaction_storage_count; j++) {
      const StorageId storage_id = account_transaction_storage_id_at(account.account_id, j);
      storage_table.states[storage_id].transaction_epoch = 0;
    }
  }
}

void storage_tx_clear(bytes20 a)
{
  const AccountId account_id = get_account_id(&a);
  account_clear_storage_generation(account_id);
}

/* read member: fresh binds curr == orig = value; existing is only marked */
void storage_block_initialize(AccountId account_id, StorageId storage_id, u256 value,
                              NodeId terminal_node, bool prestate_exists)
{
  StorageState *state = &storage_table.states[storage_id];
  StorageTrieBinding *binding = &storage_table.trie_bindings[storage_id];
  binding->terminal_node = terminal_node;
  binding->prestate_exists = (uint8_t)prestate_exists;
  state->current = value;
  state->original = value;
  state->transaction_original = value;
  /* A pre-state-absent account begins on an explicitly empty incarnation.
   * Stamp its eagerly loaded BAL slots with that same incarnation; otherwise
   * their authenticated zero values would be rejected as stale on first use. */
  state->storage_generation = account_storage_generation(account_id);
  state->transaction_original_generation = state->storage_generation;
}

/* Loads one guaranteed BAL row. BAL touch deduplication is owned by the
 * recorder and is independent of transaction write bookkeeping. */
void storage_load_view(StorageGeneration generation, StorageId storage_id, StorageView *view)
{
  if (storage_id == STORAGE_NO_ROW) {
    fatal_error(InvalidBlockAccessList);
  }
  const StorageState *state = &storage_table.states[storage_id];
  if (state->storage_generation != generation) {
    view->current = (u256){{0}};
    view->original = (u256){{0}};
    return;
  }
  if (state->transaction_epoch == current_warm_epoch) {
    view->current = state->current;
    view->original = state->transaction_original;
    return;
  }
  view->current = state->current;
  view->original = state->current;
}

/* Write the active generation directly. Sail supplies the transaction-start
 * original from the preceding semantic SLOAD. Rollback records only fields
 * whose values actually change. */
void storage_update_by_id(AccountId account_id, StorageGeneration generation,
                          StorageId storage_id, u256 v, u256 orig)
{
  account_transaction_touch(account_id);
  if (storage_id == STORAGE_NO_ROW) {
    fatal_error(InvalidBlockAccessList);
  }

  StorageState *state = &storage_table.states[storage_id];
  if (state->transaction_epoch != current_warm_epoch) {
    state->transaction_original = orig;
    state->transaction_original_generation = state->storage_generation;
    state->transaction_epoch = current_warm_epoch;
    account_transaction_storage_append(account_id, storage_id);
  }
  if (!word_equal(&state->current, &v)) {
    state_journal_push_storage_value(storage_id, state->current);
    state->current = v;
  }
  if (state->storage_generation != generation) {
    state_journal_push_storage_row_generation(storage_id, state->storage_generation);
    state->storage_generation = generation;
  }
}

void storage_value_restore(StorageId id, u256 prior)
{
  storage_table.states[id].current = prior;
}

void storage_generation_restore(StorageId id, StorageGeneration prior)
{
  storage_table.states[id].storage_generation = prior;
}

/* Removes transaction-only bookkeeping for a row whose list insertion is
 * being reverted. Semantic value and generation inverses are separate journal
 * records and are replayed independently. */
void storage_transaction_forget(StorageId id)
{
  storage_table.states[id].transaction_epoch = 0;
}

/* The account's committed storage generation is switched by account merge.
 * Rows from older generations become invisible without a range scan. */
void storage_block_clear(bytes20 a)
{
  (void)get_account_id(&a);
}

/* ---- cumulative storage enumeration -------------------------------------
   Each AccountEntry owns one contiguous StorageId interval. State-root
   traversal consumes a measured secure-key order over exactly that range. */

bool storage_range_has_writes(StorageId begin, uint32_t count, StorageGeneration generation)
{
  for (StorageId index = begin; index < begin + count; index++) {
    const StorageState *state = &storage_table.states[index];
    if (state->storage_generation == generation && !word_all_zero(&state->current)) {
      return true;
    }
  }
  return false;
}

/* Split sort-key accessors: candidate filtering checks generation liveness
 * exactly once, after which the sorter reads terminal nodes unchecked and
 * borrows the 32-byte secure keys only on terminal-node ties. */
bool storage_trie_binding_live(StorageId storage_id, StorageGeneration generation)
{
  return storage_table.states[storage_id].storage_generation == generation;
}

NodeId storage_trie_binding_terminal_node(StorageId storage_id)
{
  return storage_table.trie_bindings[storage_id].terminal_node;
}

const bytes32 *storage_trie_binding_secure_key(StorageId storage_id)
{
  return &storage_table.trie_bindings[storage_id].secure_key;
}

bool storage_trie_binding_get(StorageId storage_id, StorageGeneration generation,
                              StorageTrieView *view)
{
  const StorageState *state = &storage_table.states[storage_id];
  const StorageTrieBinding *binding = &storage_table.trie_bindings[storage_id];
  if (state->storage_generation != generation) {
    return false;
  }
  *view = (StorageTrieView){
      .current = &state->current,
      .original = &state->original,
      .secure_key = &binding->secure_key,
      .terminal_node = binding->terminal_node,
      .prestate_exists = binding->prestate_exists != 0U,
  };
  return true;
}

void storage_transaction_merge(uint32_t current_transaction_epoch)
{
  for (uint32_t i = 0; i < account_transaction_count(); i++) {
    AccountTransactionStorageView account;
    account_transaction_storage_view_at(i, &account);
    if (!account.exists) {
      continue;
    }

    const AccountId account_id = account.account_id;
    const StorageGeneration original_generation = account.original_storage_generation;
    const StorageGeneration final_generation = account.storage_generation;
    const bool storage_cleared = original_generation != final_generation;
    const uint32_t storage_count =
        storage_cleared ? account.storage_count : account.transaction_storage_count;

    for (uint32_t j = 0; j < storage_count; j++) {
      const StorageId storage_id = storage_cleared
                                       ? account.storage_begin + j
                                       : account_transaction_storage_id_at(account_id, j);
      StorageState *state = &storage_table.states[storage_id];
      const bool active = state->transaction_epoch == current_warm_epoch;
      u256 original_value = {{0}};
      if (active) {
        original_value = state->transaction_original;
      } else if (state->storage_generation == original_generation) {
        original_value = state->current;
      }
      const u256 final_value =
          state->storage_generation == final_generation ? state->current : (u256){{0}};
      if (word_equal(&final_value, &original_value)) {
        continue;
      }

      bal_note_storage_change(current_transaction_epoch, account_id, storage_id, final_value);
    }
  }
}

void storage_state_workspace_bind(uint32_t storage_count)
{
  storage_table.cap = storage_count;
  storage_table.n = 0;
  WORKSPACE_BIND(storage_table.schema, storage_count);
  WORKSPACE_BIND(storage_table.states, storage_count);
  WORKSPACE_BIND(storage_table.trie_bindings, storage_count);
}
