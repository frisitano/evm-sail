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

static StorageId lookup_storage_id(AccountId account_id, const u256 *slot)
{
  StorageId begin;
  uint32_t count;
  account_storage_range(account_id, &begin, &count);
  for (StorageId id = begin; id < begin + count; id++) {
    if (word_equal(&storage_table.schema[id].slot, slot)) {
      return id;
    }
  }
  return STORAGE_NO_ROW;
}

/* StorageIds are append-stable and grouped into the contiguous interval owned
 * by the active AccountEntry. BAL preload is the sole construction phase;
 * duplicate slots are rejected by the range scan below. */
StorageId storage_schema_insert(AccountId account_id, const u256 *slot, const bytes32 *secure_key)
{
  if (lookup_storage_id(account_id, slot) != STORAGE_NO_ROW) {
    fatal_error(InvalidBlockAccessList);
  }
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
StorageId get_storage_id(AccountId account_id, const u256 *slot)
{
  const StorageId id = lookup_storage_id(account_id, slot);
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

void storage_schema_account_end(AccountId id)
{
  account_storage_range_bind(id, storage_schema_active_begin,
                             storage_table.n - storage_schema_active_begin);
}

/* Construction ordering is owned by the single executor BAL loader. */
void storage_schema_seal(void) {}

StorageId storage_resolve_slot(const u256 slot)
{
  const AccountId account_id = current_account_context_id();
  return lookup_storage_id(account_id, &slot);
}

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
  const StorageId storage_id = lookup_storage_id(account_id, &slot);
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
    const AccountId account_id = account_transaction_id_at(i);
    const uint32_t storage_count = account_transaction_storage_count(account_id);
    for (uint32_t j = 0; j < storage_count; j++) {
      const StorageId storage_id = account_transaction_storage_id_at(account_id, j);
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

/* Returns the transaction-start/committed view used after BAL accounting. */
bool storage_block_view(AccountId account_id, StorageId storage_id, StorageView *view)
{
  const StorageGeneration original_generation =
      account_transaction_original_storage_generation(account_id);
  if (storage_id == STORAGE_NO_ROW) {
    return false;
  }
  /* Every sealed row is linked by BAL preload; EVMSAIL_NODE_ID_UNLINKED never
   * survives loading. */
  const StorageState *state = &storage_table.states[storage_id];
  const bool active = state->transaction_epoch == current_warm_epoch;
  const StorageGeneration visible_generation =
      (int)active ? state->transaction_original_generation : state->storage_generation;
  if (visible_generation != original_generation) {
    view->current = (u256){{0}};
    view->original = (u256){{0}};
    return true;
  }
  view->current = (int)active ? state->transaction_original : state->current;
  view->original = view->current;
  return true;
}

/* Returns an active transaction value. A row invalidated by a fresh storage
 * generation is reported separately so the kernel can record the BAL read
 * before resolving the semantic value to zero. */
StorageViewStatus storage_transaction_view(AccountId account_id, StorageId storage_id,
                                           StorageView *view)
{
  const StorageGeneration generation = account_storage_generation(account_id);
  const StorageGeneration original_generation =
      account_transaction_original_storage_generation(account_id);
  if (storage_id != STORAGE_NO_ROW) {
    const StorageState *state = &storage_table.states[storage_id];
    if (state->transaction_epoch == current_warm_epoch) {
      if (state->storage_generation == generation) {
        view->current = state->current;
        view->original = state->transaction_original;
        return STORAGE_VIEW_FOUND;
      }
      return STORAGE_VIEW_CLEARED;
    }
  }
  return generation == original_generation ? STORAGE_VIEW_MISSING : STORAGE_VIEW_CLEARED;
}

/* Write the active generation directly. Sail supplies the transaction-start
 * original from the preceding semantic SLOAD. Rollback records only fields
 * whose values actually change. */
void storage_update_by_id(StorageId storage_id, u256 v, u256 orig)
{
  const AccountId account_id = current_account_context_id();
  account_transaction_touch(account_id);
  const StorageGeneration generation = account_storage_generation(account_id);
  if (storage_id == STORAGE_NO_ROW) {
    fatal_error(InvalidBlockAccessList);
  }

  StorageState *state = &storage_table.states[storage_id];
  if (state->transaction_epoch != current_warm_epoch) {
    state->transaction_original = orig;
    state->transaction_original_generation =
        account_transaction_original_storage_generation(account_id);
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

/* EELS account_has_storage: a nonempty write map in either the surviving
   transaction generation or the cumulative block overlay counts as storage.
   The authenticated pre-state root is checked in Sail. */
bool storage_has_writes(bytes20 a)
{
  const AccountId account_id = get_account_id(&a);
  const StorageGeneration generation = account_storage_generation(account_id);
  StorageId begin;
  uint32_t count;
  account_storage_range(account_id, &begin, &count);
  for (StorageId index = begin; index < begin + count; index++) {
    const StorageState *state = &storage_table.states[index];
    if (state->storage_generation == generation && !word_all_zero(&state->current)) {
      return true;
    }
  }
  return false;
}

uint32_t storage_trie_candidates(AccountId account_id, StorageId *begin,
                                 StorageGeneration *generation)
{
  uint32_t count;
  account_storage_range(account_id, begin, &count);
  *generation = account_storage_generation(account_id);
  return count;
}

bool storage_trie_binding_order_key(StorageId storage_id, StorageGeneration generation,
                                    NodeId *terminal_node, bytes32 *secure_key)
{
  const StorageTrieBinding *binding = &storage_table.trie_bindings[storage_id];
  if (storage_table.states[storage_id].storage_generation != generation) {
    return false;
  }
  *terminal_node = binding->terminal_node;
  *secure_key = binding->secure_key;
  return true;
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
    const AccountId account_id = account_transaction_id_at(i);
    if (!account_exists(account_id)) {
      continue;
    }

    const StorageGeneration original_generation =
        account_transaction_original_storage_generation(account_id);
    const StorageGeneration final_generation = account_storage_generation(account_id);
    const bool storage_cleared = original_generation != final_generation;
    StorageId range_begin = 0;
    uint32_t storage_count;
    if (storage_cleared) {
      /* A logical clear changes the interpretation of the whole account
       * range, but remains O(1) at the point of execution. Pay the required
       * BAL comparison once, at transaction merge. */
      account_storage_range(account_id, &range_begin, &storage_count);
    } else {
      storage_count = account_transaction_storage_count(account_id);
    }

    for (uint32_t j = 0; j < storage_count; j++) {
      const StorageId storage_id =
          (int)storage_cleared ? range_begin + j : account_transaction_storage_id_at(account_id, j);
      const StorageSchema *schema = &storage_table.schema[storage_id];
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

      bal_note_storage_change(current_transaction_epoch, *account_id_address(account_id),
                              schema->slot, final_value);
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
