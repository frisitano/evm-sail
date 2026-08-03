/* Optimized persistent-storage semantic state and rollback journal.
 *
 * StorageId is the stable account-and-slot identity. This module exclusively
 * owns storage schema, current values, rollback state,
 * and storage-side authenticated-trie metadata. */
#include "host/state/internal.h"
#include "host/state/journal.h"
#include "evmsail/exceptions.h"
#include "evmsail/primitives/crypto.h"
#include "lib/mpt/trie.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const U256 storage_zero = {{0}};

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
  U256 slot;
  uint32_t warm_epoch;
} StorageSchema;

typedef struct {
  /* Immutable authenticated identity. The terminal belongs to the owning
   * account's storage-witness arena; encoded bytes remain in stateless input. */
  Hash32 secure_key;
  NodeId terminal_node;
  uint8_t prestate_exists;
} StorageTrieBinding;

typedef struct {
  StorageSchema *schema;
  StorageState *states;
  StorageTrieBinding *trie_bindings;
  uint32_t n, cap;
} StorageTable;

static StorageTable storage_table = {
    .cap = GUEST_STATE_STORAGE,
};
static Address storage_block_iter_key;
static uint32_t storage_block_iter_position = 0;
static uint32_t storage_block_iter_end = 0;
static bool storage_block_iter_active = false;

static const StorageId STORAGE_NO_ROW = UINT32_MAX;
static AccountId storage_schema_active_account = UINT32_MAX;
static StorageId storage_schema_active_begin = 0;
static bool storage_schema_is_sealed = false;

/* drop the per-account compute_root snapshots when the block base changes
   (defined with the snapshot builders below) */
static void storage_dump_invalidate(void);

static void storage_account_range(AccountId account_id, StorageId *begin,
                                  uint32_t *count) {
  if (storage_schema_active_account == account_id) {
    *begin = storage_schema_active_begin;
    *count = storage_table.n - storage_schema_active_begin;
    return;
  }
  account_storage_range(account_id, begin, count);
}

static StorageId lookup_storage_id(AccountId account_id, const U256 *slot) {
  StorageId begin;
  uint32_t count;
  storage_account_range(account_id, &begin, &count);
  if (begin > storage_table.n || count > storage_table.n - begin) GUEST_ABORT();
  for (StorageId id = begin; id < begin + count; id++) {
    if (word_equal(&storage_table.schema[id].slot, slot)) return id;
  }
  return STORAGE_NO_ROW;
}

/* StorageIds are append-stable and grouped into the contiguous interval owned
 * by the active AccountEntry. BAL preload is the sole construction phase. */
StorageId storage_schema_insert(AccountId account_id, const U256 *slot,
                                const Hash32 *secure_key) {
  if (account_id == ACCOUNT_ID_NONE || account_id >= account_id_count())
    GUEST_ABORT();
  if (storage_schema_is_sealed ||
      storage_schema_active_account != account_id)
    GUEST_ABORT();
  if (lookup_storage_id(account_id, slot) != STORAGE_NO_ROW) {
    throw_invalid_block(zInvalidBlockAccessList,
                        "duplicate BAL storage slot");
    return STORAGE_NO_ROW;
  }
  if (storage_table.n >= storage_table.cap) GUEST_ABORT();
  const StorageId i = storage_table.n;
  StorageSchema *schema = &storage_table.schema[i];
  StorageState *state = &storage_table.states[i];
  StorageTrieBinding *binding = &storage_table.trie_bindings[i];
  memset(schema, 0, sizeof(*schema));
  memset(state, 0, sizeof(*state));
  memset(binding, 0, sizeof(*binding));
  schema->slot = *slot;
  binding->secure_key = *secure_key;
  binding->terminal_node = EVMSAIL_NODE_ID_UNLINKED;
  storage_table.n++;
  return i;
}

/* Execution and BAL history operate only over the sealed, block-local
 * schema. A missing account/slot pair is therefore a BAL violation. */
StorageId get_storage_id(AccountId account_id, const U256 *slot) {
  const StorageId id = lookup_storage_id(account_id, slot);
  if (id == STORAGE_NO_ROW) {
    throw_invalid_block(zInvalidBlockAccessList,
                        "storage absent from block access list");
  }
  return id;
}

void storage_schema_account_begin(AccountId id) {
  if (storage_schema_is_sealed || storage_schema_active_account != UINT32_MAX ||
      id == ACCOUNT_ID_NONE || id >= account_id_count())
    GUEST_ABORT();
  storage_schema_active_account = id;
  storage_schema_active_begin = storage_table.n;
}

void storage_schema_account_end(AccountId id) {
  if (storage_schema_is_sealed || storage_schema_active_account != id ||
      storage_schema_active_begin > storage_table.n)
    GUEST_ABORT();
  account_storage_range_bind(id, storage_schema_active_begin,
                             storage_table.n - storage_schema_active_begin);
  storage_schema_active_account = UINT32_MAX;
}

void storage_schema_seal(void) {
  if (storage_schema_active_account != UINT32_MAX) GUEST_ABORT();
  storage_schema_is_sealed = true;
}

bool storage_is_warm(Address address, const U256 slot) {
  const AccountId account_id = lookup_account_id(&address);
  if (account_id == ACCOUNT_ID_NONE) return false;
  const StorageId storage_id = lookup_storage_id(account_id, &slot);
  if (storage_id == STORAGE_NO_ROW) return false;
  return storage_table.schema[storage_id].warm_epoch >= current_warm_epoch;
}

unit storage_mark_warm(Address address, const U256 slot) {
  const AccountId account_id = lookup_account_id(&address);
  if (account_id == ACCOUNT_ID_NONE) return UNIT;
  const StorageId storage_id = lookup_storage_id(account_id, &slot);
  if (storage_id == STORAGE_NO_ROW) return UNIT;

  uint32_t *warm_epoch = &storage_table.schema[storage_id].warm_epoch;
  if (*warm_epoch < current_warm_epoch) {
    const uint32_t prior_epoch = *warm_epoch;
    state_journal_push_warm_storage(storage_id, prior_epoch);
    *warm_epoch = current_warm_epoch;
  }
  return UNIT;
}

void storage_warm_restore(StorageId id, uint32_t prior_epoch) {
  if (id >= storage_table.n) GUEST_ABORT();
  storage_table.schema[id].warm_epoch = prior_epoch;
}

uint32_t storage_id_count(void) { return storage_table.n; }

const U256 *storage_id_slot(StorageId id) {
  if (id >= storage_table.n) GUEST_ABORT();
  return &storage_table.schema[id].slot;
}

/* --- lifecycle -------------------------------------------------------- */

/* Whole-overlay wipe (both layers): the harness world reset. */
unit storage_db_reset(const unit u) {
  (void)u;
  storage_table.n = 0;
  storage_schema_active_account = UINT32_MAX;
  storage_schema_active_begin = 0;
  storage_schema_is_sealed = false;
  storage_block_iter_position = storage_block_iter_end = 0;
  storage_block_iter_active = false;
  storage_dump_invalidate();
  return UNIT;
}

/* Transaction scratch reset: retain the bound workspace for the next transaction. */
unit storage_tx_reset(const unit u) {
  (void)u;
  for (uint32_t i = 0; i < account_transaction_count(); i++) {
    const AccountId account_id = account_transaction_id_at(i);
    const uint32_t storage_count =
        account_transaction_storage_count(account_id);
    for (uint32_t j = 0; j < storage_count; j++) {
      const StorageId storage_id =
          account_transaction_storage_id_at(account_id, j);
      if (storage_id >= storage_table.n) GUEST_ABORT();
      storage_table.states[storage_id].transaction_epoch = 0;
    }
  }
  return UNIT;
}

unit storage_tx_clear(Address a) {
  const AccountId account_id = get_account_id(&a);
  if (have_exception) return UNIT;
  account_clear_storage_generation(account_id);
  return UNIT;
}

/* read member: fresh binds curr == orig = value; existing is only marked */
void storage_block_initialize(AccountId account_id, StorageId storage_id,
                              U256 value, NodeId terminal_node,
                              bool prestate_exists) {
  if (account_id == ACCOUNT_ID_NONE || account_id >= account_id_count() ||
      storage_id >= storage_table.n ||
      terminal_node == EVMSAIL_NODE_ID_UNLINKED)
    GUEST_ABORT();
  StorageState *state = &storage_table.states[storage_id];
  StorageTrieBinding *binding = &storage_table.trie_bindings[storage_id];
  if (binding->terminal_node != EVMSAIL_NODE_ID_UNLINKED ||
      state->storage_generation != 0)
    GUEST_ABORT();
  binding->terminal_node = terminal_node;
  binding->prestate_exists = prestate_exists;
  state->current = value;
  state->original = value;
  state->transaction_original = value;
  /* A pre-state-absent account begins on an explicitly empty incarnation.
   * Stamp its eagerly loaded BAL slots with that same incarnation; otherwise
   * their authenticated zero values would be rejected as stale on first use. */
  state->storage_generation = account_storage_generation(account_id);
  state->transaction_original_generation = state->storage_generation;
  storage_dump_invalidate();
}


/* Returns the transaction-start/committed view used after BAL accounting. */
bool storage_block_view(AccountId account_id, const U256 *slot,
                        StorageView *view) {
  const StorageGeneration original_generation =
      account_transaction_original_storage_generation(account_id);
  const StorageId storage_id = lookup_storage_id(account_id, slot);
  if (storage_id == STORAGE_NO_ROW) return false;
  const StorageState *state = &storage_table.states[storage_id];
  const StorageTrieBinding *binding = &storage_table.trie_bindings[storage_id];
  if (binding->terminal_node == EVMSAIL_NODE_ID_UNLINKED) return false;
  const bool active = state->transaction_epoch == current_warm_epoch;
  const StorageGeneration visible_generation =
      active ? state->transaction_original_generation
             : state->storage_generation;
  if (visible_generation != original_generation) {
    view->current = storage_zero;
    view->original = storage_zero;
    return true;
  }
  view->current = active ? state->transaction_original : state->current;
  view->original = view->current;
  return true;
}

/* Returns an active transaction value. A row invalidated by a fresh storage
 * generation is reported separately so the kernel can record the BAL read
 * before resolving the semantic value to zero. */
StorageViewStatus storage_transaction_view(AccountId account_id,
                                           const U256 *slot,
                                           StorageView *view) {
  const StorageGeneration generation = account_storage_generation(account_id);
  const StorageGeneration original_generation =
      account_transaction_original_storage_generation(account_id);
  const StorageId storage_id = lookup_storage_id(account_id, slot);
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
  return generation == original_generation ? STORAGE_VIEW_MISSING
                                           : STORAGE_VIEW_CLEARED;
}

/* Write the active generation directly. Sail supplies the transaction-start
 * original from the preceding semantic SLOAD. Rollback records only fields
 * whose values actually change. */
unit storage_update(Address a, U256 s, U256 v, U256 orig) {
  const AccountId account_id = get_account_id(&a);
  if (have_exception) return UNIT;
  account_transaction_touch(account_id);
  const StorageGeneration generation =
      account_storage_generation(account_id);
  const StorageId storage_id =
      lookup_storage_id(account_id, &s);
  if (storage_id == STORAGE_NO_ROW) {
    throw_invalid_block(zInvalidBlockAccessList,
                        "storage write absent from block access list");
    return UNIT;
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
    state_journal_push_storage_row_generation(
        storage_id, state->storage_generation);
    state->storage_generation = generation;
  }
  return UNIT;
}

void storage_value_restore(StorageId id, U256 prior) {
  if (id >= storage_table.n) GUEST_ABORT();
  storage_table.states[id].current = prior;
  storage_dump_invalidate();
}

void storage_generation_restore(StorageId id, StorageGeneration prior) {
  if (id >= storage_table.n) GUEST_ABORT();
  storage_table.states[id].storage_generation = prior;
  storage_dump_invalidate();
}

/* Removes transaction-only bookkeeping for a row whose list insertion is
 * being reverted. Semantic value and generation inverses are separate journal
 * records and are replayed independently. */
void storage_transaction_forget(StorageId id) {
  if (id >= storage_table.n) GUEST_ABORT();
  storage_table.states[id].transaction_epoch = 0;
}

/* The account's committed storage generation is switched by account merge.
 * Rows from older generations become invisible without a range scan. */
unit storage_block_clear(Address a) {
  (void)get_account_id(&a);
  if (have_exception) return UNIT;
  storage_dump_invalidate();
  return UNIT;
}

/* ---- cumulative storage enumeration -------------------------------------
   Each AccountEntry owns one contiguous StorageId interval. State-root
   traversal consumes a measured secure-key order over exactly that range. */

typedef struct {
  U256 slot;
  U256 value;
} StorageDumpEntry;

/* EELS account_has_storage: a nonempty write map in either the surviving
   transaction generation or the cumulative block overlay counts as storage.
   The authenticated pre-state root is checked in Sail. */
bool storage_has_writes(Address a) {
  const AccountId account_id = get_account_id(&a);
  if (have_exception) return false;
  const StorageGeneration generation =
      account_storage_generation(account_id);
  StorageId begin;
  uint32_t count;
  account_storage_range(account_id, &begin, &count);
  for (StorageId index = begin; index < begin + count; index++) {
    const StorageState *state = &storage_table.states[index];
    if (storage_table.trie_bindings[index].terminal_node !=
            EVMSAIL_NODE_ID_UNLINKED &&
        state->storage_generation == generation &&
        !word_equal(&state->current, &storage_zero))
      return true;
  }
  return false;
}

static void storage_dump_push(StorageDumpEntry *rows, uint32_t *length,
                              const U256 *slot,
                              U256 value) {
  if (*length >= GUEST_STATE_STORAGE) GUEST_ABORT();
  rows[*length].slot = *slot;
  rows[*length].value = value;
  (*length)++;
}

uint32_t storage_update_candidates(Address address, StorageId *begin,
                                   StorageGeneration *generation) {
  const AccountId account_id = get_account_id(&address);
  if (have_exception) {
    *begin = 0;
    *generation = STORAGE_INITIAL_GENERATION;
    return 0;
  }
  uint32_t count;
  account_storage_range(account_id, begin, &count);
  *generation = account_storage_generation(account_id);
  return count;
}

bool storage_update_order_key(StorageId storage_id,
                              StorageGeneration generation,
                              NodeId *terminal_node, Hash32 *secure_key) {
  if (storage_id >= storage_table.n) GUEST_ABORT();
  const StorageTrieBinding *binding =
      &storage_table.trie_bindings[storage_id];
  if (binding->terminal_node == EVMSAIL_NODE_ID_UNLINKED ||
      storage_table.states[storage_id].storage_generation != generation)
    return false;
  *terminal_node = binding->terminal_node;
  *secure_key = binding->secure_key;
  return true;
}

uint64_t storage_block_update_probe(StorageId storage_id, U256 *slot,
                                    U256 *curr, U256 *orig,
                                    Hash32 *slot_hash,
                                    NodeId *terminal_node,
                                    bool *prestate_exists) {
  if (storage_id >= storage_table.n) GUEST_ABORT();
  const StorageSchema *schema = &storage_table.schema[storage_id];
  const StorageState *state = &storage_table.states[storage_id];
  const StorageTrieBinding *binding =
      &storage_table.trie_bindings[storage_id];
  *slot = schema->slot;
  *curr = state->current;
  *orig = state->original;
  *slot_hash = binding->secure_key;
  *terminal_node = binding->terminal_node;
  *prestate_exists = binding->prestate_exists;
  return 1;
}

unit storage_block_iter_begin(Address a) {
  storage_block_iter_key = a;
  storage_block_iter_end = mpt_storage_updates_prepare(a);
  storage_block_iter_position = 0;
  storage_block_iter_active = true;
  return UNIT;
}

uint64_t storage_block_iter_next_probe(Address a, U256 *slot,
                                       U256 *curr, U256 *orig,
                                       Hash32 *address_hash,
                                       Hash32 *slot_hash) {
  if (!storage_block_iter_active ||
      !address_equal(&a, &storage_block_iter_key) ||
      storage_block_iter_end <= storage_block_iter_position)
    return 0;
  NodeId terminal_node;
  bool prestate_exists;
  const StorageId index = storage_block_iter_position++;
  const StorageId storage_id = mpt_storage_update_id_at(index);
  const AccountId account_id = get_account_id(&a);
  if (have_exception) return 0;
  *address_hash = host_keccak_address(*account_id_address(account_id));
  return storage_block_update_probe(storage_id, slot, curr, orig, slot_hash,
                                    &terminal_node, &prestate_exists);
}

/* All cumulative block entries for an account, including read-only entries.
   The authenticated base lives in the MPT node DB, so untouched base slots are
   intentionally absent. This materialized view exists only for debug dumps. */
static StorageDumpEntry *storage_dump_entries;
static uint32_t storage_dump_len = 0;
static Hash32 storage_dump_account_hash;
static int storage_dump_valid = 0;

static void storage_dump_build(const Hash32 *ak) {
  if (storage_dump_valid &&
      hash_equal(&storage_dump_account_hash, ak)) return;
  storage_dump_len = 0;
  for (AccountId account_id = 1u;
       account_id < account_id_count(); account_id++) {
    const Hash32 account_hash =
        host_keccak_address(*account_id_address(account_id));
    if (!hash_equal(&account_hash, ak)) continue;
    StorageId begin;
    uint32_t count;
    account_storage_range(account_id, &begin, &count);
    for (StorageId id = begin; id < begin + count; id++) {
      if (storage_table.trie_bindings[id].terminal_node ==
              EVMSAIL_NODE_ID_UNLINKED ||
          storage_table.states[id].storage_generation !=
              account_storage_generation(account_id))
        continue;
      storage_dump_push(storage_dump_entries, &storage_dump_len,
                        &storage_table.schema[id].slot,
                        storage_table.states[id].current);
    }
    break;
  }
  storage_dump_account_hash = *ak;
  storage_dump_valid = 1;
}

static void storage_dump_invalidate(void) {
  storage_dump_valid = 0;
}

uint64_t storage_dump_count(U256 ak) {
  Hash32 key = hash_from_sail_word(ak);
  storage_dump_build(&key);
  return storage_dump_len;
}
U256 storage_dump_slot(U256 ak, uint64_t j) {
  Hash32 key = hash_from_sail_word(ak);
  storage_dump_build(&key);
  static const U256 zero = {{0}};
  return j < storage_dump_len ? storage_dump_entries[j].slot : zero;
}
U256 storage_dump_value(U256 ak, uint64_t j) {
  Hash32 key = hash_from_sail_word(ak);
  storage_dump_build(&key);
  return j < storage_dump_len ? storage_dump_entries[j].value : storage_zero;
}

void storage_transaction_merge(uint64_t current_transaction_epoch) {
  for (uint32_t i = 0; i < account_transaction_count(); i++) {
    const AccountId account_id = account_transaction_id_at(i);
    if (!account_exists(account_id)) continue;

    const StorageGeneration original_generation =
        account_transaction_original_storage_generation(account_id);
    const StorageGeneration final_generation =
        account_storage_generation(account_id);
    const bool storage_cleared =
        original_generation != final_generation;
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
      const StorageId storage_id = storage_cleared
                                       ? range_begin + j
                                       : account_transaction_storage_id_at(
                                             account_id, j);
      if (storage_id >= storage_table.n) GUEST_ABORT();
      const StorageSchema *schema = &storage_table.schema[storage_id];
      StorageState *state = &storage_table.states[storage_id];
      const bool active =
          state->transaction_epoch == current_warm_epoch;
      const U256 original_value =
          active
              ? state->transaction_original
              : (state->storage_generation == original_generation
                     ? state->current
                     : storage_zero);
      const U256 final_value =
          state->storage_generation == final_generation
              ? state->current
              : storage_zero;
      if (word_equal(&final_value, &original_value)) continue;

      bal_note_storage_change(current_transaction_epoch,
                              *account_id_address(account_id), schema->slot,
                              final_value);
      account_diagnostics_invalidate(account_id);
      storage_dump_invalidate();
    }
  }
}

void storage_state_workspace_bind(void) {
  WORKSPACE_BIND(storage_table.schema, GUEST_STATE_STORAGE);
  WORKSPACE_BIND(storage_table.states, GUEST_STATE_STORAGE);
  WORKSPACE_BIND(storage_table.trie_bindings, GUEST_STATE_STORAGE);

#ifdef EVMSAIL_NATIVE_DEBUG_AGGREGATES
  WORKSPACE_BIND(storage_dump_entries, GUEST_STATE_STORAGE);
#endif
}
