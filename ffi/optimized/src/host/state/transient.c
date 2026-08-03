/* C-backed transient storage table for evm-sail.
 *
 * This file backs EIP-1153 TLOAD/TSTORE only. The public write operation
 * records its prior logical value in the append-only state journal; rollback
 * uses a separate non-journaling restore path. Persistent storage is keyed by
 * semantic account/slot IDs. */
#include "host/state/transient.h"
#include "host/state/journal.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdint.h>
#include <string.h>

typedef struct {
  Address address;
  U256 slot;
} TransientKey;

typedef struct {
  TransientKey key;
  U256 value;
  uint8_t used;
} TransientEntry;

typedef struct {
  TransientEntry *entries;
  uint32_t count;
} TransientTable;

static TransientTable transient_table;

void transient_workspace_bind(void) {
  WORKSPACE_BIND(transient_table.entries, GUEST_TRANSIENT_STORAGE_ENTRIES);
}

static uint64_t transient_key_hash(const TransientKey *key) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (size_t i = 0; i < sizeof(key->address.bytes); ++i) {
    h ^= key->address.bytes[i];
    h *= 0x100000001b3ull;
  }
  for (size_t i = 0; i < sizeof(key->slot.limbs); ++i) {
    h ^= ((const uint8_t *)(const void *)key->slot.limbs)[i];
    h *= 0x100000001b3ull;
  }
  return h;
}

static void transient_table_clear(TransientTable *table) {
  memset(table->entries, 0,
         GUEST_TRANSIENT_STORAGE_ENTRIES * sizeof(*table->entries));
  table->count = 0;
}

/* Returns whether two account-specific transient-storage keys are equal. */
static bool transient_key_equal(const TransientKey *left,
                                const TransientKey *right) {
  return address_equal(&left->address, &right->address) &&
         word_equal(&left->slot, &right->slot);
}

/* Finds either the matching entry or the first free open-addressed bucket. */
static uint32_t transient_table_find(TransientTable *table,
                                     const TransientKey *key,
                                     uint64_t hash) {
  uint32_t i =
      (uint32_t)(hash & (GUEST_TRANSIENT_STORAGE_ENTRIES - 1));
  for (;;) {
    TransientEntry *entry = &table->entries[i];
    if (!entry->used || transient_key_equal(&entry->key, key)) return i;
    i = (i + 1) & (GUEST_TRANSIENT_STORAGE_ENTRIES - 1);
  }
}

static bool transient_table_set(TransientTable *table,
                                const TransientKey *key, U256 value) {
  if (table->count * 10 >= GUEST_TRANSIENT_STORAGE_ENTRIES * 7) return false;

  const uint32_t i = transient_table_find(
      table, key, transient_key_hash(key));
  TransientEntry *entry = &table->entries[i];
  if (!entry->used) {
    entry->used = 1;
    entry->key = *key;
    table->count++;
  }
  entry->value = value;
  return true;
}

static TransientEntry *transient_table_get(TransientTable *table,
                                           const TransientKey *key) {
  const uint32_t i = transient_table_find(
      table, key, transient_key_hash(key));
  return table->entries[i].used ? &table->entries[i] : NULL;
}

/* clear transient storage at tx/world reset */
unit transient_storage_reset(const unit u) {
  (void)u;
  transient_table_clear(&transient_table);
  return UNIT;
}

static TransientKey transient_key_make(Address address, U256 slot) {
  TransientKey key = {
      .address = address,
      .slot = slot,
  };
  return key;
}

static TransientEntry *transient_storage_lookup(Address address, U256 slot) {
  TransientKey key = transient_key_make(address, slot);
  return transient_table_get(&transient_table, &key);
}

static unit transient_storage_set(Address address, U256 slot, U256 value) {
  TransientKey key = transient_key_make(address, slot);
  if (!transient_table_set(&transient_table, &key, value)) GUEST_ABORT();
  return UNIT;
}

/* Store a value and make the update part of the current semantic checkpoint.
 * Transient storage is a total map with default zero, so restoring an absent
 * key as an explicit zero is observationally identical. */
unit transient_storage_write(Address addr, const U256 slot,
                             const U256 v) {
  static const U256 zero = {{0}};
  U256 slot_value = (slot);
  U256 value = (v);
  TransientEntry *entry = transient_storage_lookup(addr, slot_value);
  U256 prior = entry ? entry->value : zero;
  state_journal_push_transient(addr, slot_value, prior);
  return transient_storage_set(addr, slot_value, value);
}

unit transient_storage_restore(Address addr, U256 slot,
                               U256 v) {
  return transient_storage_set(addr, slot, v);
}

/* the 256-bit value at (address, slot); 0 if absent */
U256 transient_storage_read(Address addr,
                                 const U256 slot) {
  static const U256 zero = {{0}};
  TransientEntry *entry = transient_storage_lookup(addr, (slot));
  return entry ? entry->value : zero;
}
