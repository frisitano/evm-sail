/* C-backed transient storage table for evm-sail.
 *
 * This file backs EIP-1153 TLOAD/TSTORE only. The public write operation
 * records its prior logical value in the append-only state journal; rollback
 * uses a separate non-journaling restore path. Persistent storage is keyed by
 * semantic account/slot IDs. */
#include "host/state/transient.h"
#include "evmsail/host/state/primitives.h"
#include "evmsail/prelude.h"
#include "host/state/internal.h"
#include "host/state/journal.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  AccountId account_id;
  U256 slot;
} TransientKey;

typedef struct {
  TransientKey key;
  U256 value;
  uint32_t epoch;
} TransientEntry;

typedef struct {
  TransientEntry *entries;
  uint32_t count;
  uint32_t epoch;
  uint32_t capacity;
  uint32_t mask;
} TransientTable;

static TransientTable transient_table = {.epoch = 1U};

void transient_workspace_bind(uint32_t capacity)
{
  if (capacity == 0 || (capacity & (capacity - 1U)) != 0) {
    GUEST_ABORT();
  }
  transient_table.capacity = capacity;
  transient_table.mask = capacity - 1U;
  WORKSPACE_BIND(transient_table.entries, capacity);
}

static uint64_t transient_key_hash(const TransientKey *key)
{
  uint64_t h = 0xcbf29ce484222325ULL;
  for (size_t i = 0; i < sizeof(key->account_id); ++i) {
    h ^= ((const uint8_t *)&key->account_id)[i];
    h *= 0x100000001b3ULL;
  }
  const uint8_t *slot_bytes = (const uint8_t *)&key->slot;
  for (size_t i = 0; i < sizeof(key->slot); ++i) {
    h ^= slot_bytes[i];
    h *= 0x100000001b3ULL;
  }
  return h;
}

static void transient_table_clear(void)
{
  transient_table.epoch++;
  if (transient_table.epoch == 0) {
    GUEST_ABORT();
  }
  transient_table.count = 0;
}

/* Returns whether two account-specific transient-storage keys are equal. */
static bool transient_key_equal(const TransientKey *left, const TransientKey *right)
{
  return (left->account_id == right->account_id && word_equal(&left->slot, &right->slot)) != 0;
}

/* Finds either the matching entry or the first free open-addressed bucket. */
static uint32_t transient_table_find(const TransientKey *key, uint64_t hash)
{
  uint32_t i = (uint32_t)(hash & transient_table.mask);
  for (;;) {
    TransientEntry *entry = &transient_table.entries[i];
    if (entry->epoch != transient_table.epoch || transient_key_equal(&entry->key, key)) {
      return i;
    }
    i = (i + 1) & transient_table.mask;
  }
}

static void transient_table_set(const TransientKey *key, U256 value)
{
  const uint32_t i = transient_table_find(key, transient_key_hash(key));
  TransientEntry *entry = &transient_table.entries[i];
  if (entry->epoch != transient_table.epoch) {
    if ((uint64_t)transient_table.count * 10 >= (uint64_t)transient_table.capacity * 7) {
      GUEST_ABORT();
    }
    entry->epoch = transient_table.epoch;
    entry->key = *key;
    transient_table.count++;
  }
  entry->value = value;
}

static TransientEntry *transient_table_get(const TransientKey *key)
{
  const uint32_t i = transient_table_find(key, transient_key_hash(key));
  return transient_table.entries[i].epoch == transient_table.epoch ? &transient_table.entries[i]
                                                                   : NULL;
}

/* clear transient storage at tx/world reset */
void transient_storage_reset(void)
{
  transient_table_clear();
}

/* Store a value and make the update part of the current semantic checkpoint.
 * Transient storage is a total map with default zero, so restoring an absent
 * key as an explicit zero is observationally identical. */
void transient_storage_write(Address addr, const U256 slot, const U256 v)
{
  const AccountId account_id = get_account_id(&addr);
  const TransientKey key = {.account_id = account_id, .slot = slot};
  TransientEntry *entry = transient_table_get(&key);
  const U256 prior = entry ? entry->value : (U256){{0}};
  state_journal_push_transient(account_id, slot, prior);
  transient_table_set(&key, v);
}

void transient_storage_restore(AccountId account_id, U256 slot, U256 v)
{
  const TransientKey key = {.account_id = account_id, .slot = slot};
  transient_table_set(&key, v);
}

/* the 256-bit value at (address, slot); 0 if absent */
U256 transient_storage_read(Address addr, const U256 slot)
{
  const AccountId account_id = get_account_id(&addr);
  const TransientKey key = {.account_id = account_id, .slot = slot};
  TransientEntry *entry = transient_table_get(&key);
  return entry ? entry->value : (U256){{0}};
}
