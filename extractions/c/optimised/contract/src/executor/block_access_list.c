/* Optimized Block Access List authentication.
 *
 * This file implements the high-level override declared by
 * sail/optimised/executor/block_access_list.sail. The decoder consumes the
 * canonical RLP and the C-private BAL stream together so the optimized guest
 * does not materialize intermediate Sail lists or aggregate rows. */
#include "evmsail/host/state/primitives.h"
#include "evmsail/host/nodes.h"
#include "evmsail/prelude.h"
#include "evmsail/kernel/accounts.h"
#include "evmsail/kernel/storage.h"
#include "evmsail/primitives/crypto.h"
#include "evmsail/spec/exceptions.h"
#include "evmsail/spec/primitives/account.h"
#include "host/code/store.h"
#include "host/state/block_access_list.h"
#include "host/state/internal.h"
#include "lib/rlp/decoding.h"
#include "primitives/hash.h"
#include "evmsail/host/region_access.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Complete EIP-7928 authentication.  The spec build keeps the recursive Sail
 * decoder; the optimized build parses the immutable input in place through the
 * shared RLP core and compares it directly with the C-private BAL stream.
 */
/* One input-side BAL event. The RLP bytes stay borrowed from stateless input;
 * dense IDs are resolved once while the block-local state schema is built. */
typedef struct {
  const uint8_t *expected;
  uint32_t expected_len;
  AccountId account_id;
  StorageId storage_id;
  uint8_t tag;
} BalExpectedEntry;

static BalExpectedEntry *bal_expected;
static uint32_t bal_expected_count;
static uint32_t bal_expected_capacity;
static uint64_t bal_expected_item_count;

void bal_input_workspace_bind(uint32_t expected_count)
{
  bal_expected_capacity = expected_count;
  WORKSPACE_BIND(bal_expected, expected_count);
}

/* The BAL slice is StatelessInputSliceAtMost(2^30) by type, so every item
 * source length fits the 32-bit entry field. */
static void bal_expect(enum bal_iter_tag tag, AccountId account_id, StorageId storage_id,
                       const rlp_item *item)
{
  if (bal_expected_count >= bal_expected_capacity) {
    GUEST_ABORT();
  }
  BalExpectedEntry *entry = &bal_expected[bal_expected_count++];
  entry->tag = (uint8_t)tag;
  entry->account_id = account_id;
  entry->storage_id = storage_id;
  entry->expected = item == NULL ? NULL : item->source;
  entry->expected_len = item == NULL ? 0 : (uint32_t)item->source_len;
}

/* Fatal BAL input errors terminate validation at their detection point. */
static void bal_parse_item(const uint8_t *source, uint32_t available, rlp_item *item)
{
  if (!rlp_parse_item(source, available, item)) {
    fatal_error(RlpDecode);
  }
}

static void bal_take(rlp_cursor *cursor, rlp_item *item)
{
  if (!rlp_take_item(cursor, item)) {
    fatal_error(RlpDecode);
  }
}

static void bal_list(const rlp_item *item, rlp_cursor *children)
{
  if (!item->canonical || !rlp_item_list(item, children)) {
    fatal_error(InvalidBlockAccessList);
  }
}

static void bal_bytes(const rlp_item *item)
{
  if (!rlp_canonical_bytes(item)) {
    fatal_error(InvalidBlockAccessList);
  }
}

static void bal_uint(const rlp_item *item, uint32_t maximum_len)
{
  if (!rlp_canonical_uint(item, maximum_len)) {
    fatal_error(InvalidBlockAccessList);
  }
}

static void bal_end(const rlp_cursor *cursor)
{
  if (cursor->remaining != 0) {
    fatal_error(InvalidBlockAccessList);
  }
}

static u256 bal_decode_word(const rlp_item *item)
{
  u256 value;
  bal_uint(item, 32);
  /* bal_uint established a canonical uint of at most 32 bytes, so the raw
   * word conversion cannot fail. */
  (void)rlp_word_raw(item, &value);
  return value;
}

typedef struct {
  uint32_t account_count;
  uint32_t storage_count;
  uint32_t expected_count;
} BalShape;

/* Shape counters are bounded far below 2^32 by the 2^30 input slice type. */
static void bal_shape_add(uint32_t *value, uint32_t amount)
{
  *value += amount;
}

static void bal_measure_change_list(const rlp_item *field, BalShape *shape)
{
  rlp_cursor changes;
  bal_list(field, &changes);
  while (changes.remaining != 0) {
    rlp_item ignored;
    bal_take(&changes, &ignored);
    bal_shape_add(&shape->expected_count, 1U);
  }
}

static void bal_measure_storage_changes(const rlp_item *field, BalShape *shape)
{
  rlp_cursor slots;
  bal_list(field, &slots);
  while (slots.remaining != 0) {
    rlp_item entry;
    rlp_item slot;
    rlp_item changes_item;
    rlp_cursor fields;
    rlp_cursor changes;
    bal_take(&slots, &entry);
    bal_list(&entry, &fields);
    bal_take(&fields, &slot);
    bal_take(&fields, &changes_item);
    bal_end(&fields);
    bal_list(&changes_item, &changes);
    if (changes.remaining == 0) {
      fatal_error(InvalidBlockAccessList);
    }
    bal_shape_add(&shape->storage_count, 1U);
    while (changes.remaining != 0) {
      rlp_item ignored;
      bal_take(&changes, &ignored);
      bal_shape_add(&shape->expected_count, 1U);
    }
  }
}

static void bal_measure_storage_reads(const rlp_item *field, BalShape *shape)
{
  rlp_cursor reads;
  bal_list(field, &reads);
  while (reads.remaining != 0) {
    rlp_item ignored;
    bal_take(&reads, &ignored);
    bal_shape_add(&shape->storage_count, 1U);
    bal_shape_add(&shape->expected_count, 1U);
  }
}

static void bal_measure_accounts(rlp_cursor accounts, BalShape *shape)
{
  while (accounts.remaining != 0) {
    rlp_item account_item;
    rlp_item address_item;
    rlp_item storage_changes;
    rlp_item storage_reads;
    rlp_item balance_changes;
    rlp_item nonce_changes;
    rlp_item code_changes;
    rlp_cursor fields;
    bal_take(&accounts, &account_item);
    bal_list(&account_item, &fields);
    bal_take(&fields, &address_item);
    bal_take(&fields, &storage_changes);
    bal_take(&fields, &storage_reads);
    bal_take(&fields, &balance_changes);
    bal_take(&fields, &nonce_changes);
    bal_take(&fields, &code_changes);
    bal_end(&fields);
    bal_shape_add(&shape->account_count, 1U);
    bal_shape_add(&shape->expected_count, 2U);
    bal_measure_storage_changes(&storage_changes, shape);
    bal_measure_storage_reads(&storage_reads, shape);
    bal_measure_change_list(&balance_changes, shape);
    bal_measure_change_list(&nonce_changes, shape);
    bal_measure_change_list(&code_changes, shape);
  }
}

/* Shared per-slot preload for storage changes and reads: strict slot order
 * (duplicates are rejected by the schema insert) and item count. */
static StorageId bal_preload_slot(AccountId account_id, NodeId storage_root_node, u256 slot,
                                  u256 *previous_slot, bool *have_previous_slot)
{
  if (*have_previous_slot && word_compare(previous_slot, &slot) >= 0) {
    fatal_error(InvalidBlockAccessList);
  }
  *previous_slot = slot;
  *have_previous_slot = true;

  StorageId storage_id;
  struct StorageValue value;
  storage_preload(account_id, storage_root_node, slot, &storage_id, &value);
  ++bal_expected_item_count;
  return storage_id;
}

static void bal_initialize_storage_changes(const rlp_item *field, AccountId account_id,
                                           NodeId storage_root_node)
{
  rlp_cursor slots;
  u256 previous_slot = {{0}};
  bool have_previous_slot = false;
  bal_list(field, &slots);
  while (slots.remaining != 0) {
    rlp_item entry;
    rlp_cursor fields;
    rlp_item slot_item;
    rlp_item changes_item;
    rlp_cursor changes;
    bal_take(&slots, &entry);
    bal_list(&entry, &fields);
    bal_take(&fields, &slot_item);
    bal_take(&fields, &changes_item);
    bal_end(&fields);
    bal_list(&changes_item, &changes);
    if (changes.remaining == 0) {
      fatal_error(InvalidBlockAccessList);
    }
    const u256 slot = bal_decode_word(&slot_item);
    const StorageId storage_id =
        bal_preload_slot(account_id, storage_root_node, slot, &previous_slot, &have_previous_slot);
    while (changes.remaining != 0) {
      rlp_item pair;
      bal_take(&changes, &pair);
      bal_expect(BAL_ITER_STORAGE_CHANGE, account_id, storage_id, &pair);
    }
  }
}

static void bal_initialize_storage_reads(const rlp_item *field, AccountId account_id,
                                         NodeId storage_root_node)
{
  rlp_cursor reads;
  u256 previous_slot = {{0}};
  bool have_previous_slot = false;
  bal_list(field, &reads);
  while (reads.remaining != 0) {
    rlp_item slot_item;
    bal_take(&reads, &slot_item);
    const u256 slot = bal_decode_word(&slot_item);
    const StorageId storage_id =
        bal_preload_slot(account_id, storage_root_node, slot, &previous_slot, &have_previous_slot);
    bal_expect(BAL_ITER_STORAGE_READ, account_id, storage_id, NULL);
  }
}

static void bal_initialize_account_changes(const rlp_item *field, AccountId account_id,
                                           enum bal_iter_tag tag)
{
  rlp_cursor changes;
  bal_list(field, &changes);
  while (changes.remaining != 0) {
    rlp_item pair;
    bal_take(&changes, &pair);
    bal_expect(tag, account_id, UINT32_MAX, &pair);
  }
}

void initialize_block_access_list_state(Bytes bytes, bytes32 parent_state_root,
                                        uint32_t transaction_count)
{
  bal_expected_count = 0;
  bal_expected_item_count = 0;

  const uint8_t *source = bytes.bytes;
  rlp_item root;
  bal_parse_item(source, bytes.len, &root);
  if (root.source_len != bytes.len) {
    fatal_error(RlpDecode);
  }

  rlp_cursor accounts;
  bytes20 previous_address = {{0}};
  bool have_previous_address = false;
  bal_list(&root, &accounts);

  BalShape shape = {0};
  bal_measure_accounts(accounts, &shape);
  workspace_prepare_execution(shape.account_count, shape.storage_count, shape.expected_count,
                              transaction_count);

  while (accounts.remaining != 0) {
    rlp_item account_item;
    rlp_cursor fields;
    rlp_item address_item;
    rlp_item storage_changes;
    rlp_item storage_reads;
    rlp_item balance_changes;
    rlp_item nonce_changes;
    rlp_item code_changes;
    bal_take(&accounts, &account_item);
    bal_list(&account_item, &fields);
    bal_take(&fields, &address_item);
    bal_take(&fields, &storage_changes);
    bal_take(&fields, &storage_reads);
    bal_take(&fields, &balance_changes);
    bal_take(&fields, &nonce_changes);
    bal_take(&fields, &code_changes);
    bal_end(&fields);
    bal_bytes(&address_item);
    if (address_item.content_len != 20) {
      fatal_error(InvalidBlockAccessList);
    }

    const bytes20 address = address_from_be_bytes(address_item.content);
    if (have_previous_address && address_compare(&previous_address, &address) >= 0) {
      fatal_error(InvalidBlockAccessList);
    }
    previous_address = address;
    have_previous_address = true;
    const bytes32 address_hash = host_keccak_address(address);
    const AccountId account_id = account_schema_insert(&address);
    struct Account account;
    NodeId storage_root_node = EVMSAIL_NODE_ID_EMPTY;
    account_preload(parent_state_root, account_id, address_hash, &account, &storage_root_node);
    bal_expect(BAL_ITER_ACCOUNT, account_id, UINT32_MAX, NULL);

    storage_schema_account_begin(account_id);
    const uint32_t storage_changes_begin = storage_id_count();
    bal_initialize_storage_changes(&storage_changes, account_id, storage_root_node);
    const uint32_t storage_change_count = storage_id_count() - storage_changes_begin;
    bal_initialize_storage_reads(&storage_reads, account_id, storage_root_node);
    storage_schema_account_end(account_id, storage_change_count);

    bal_initialize_account_changes(&balance_changes, account_id, BAL_ITER_BALANCE_CHANGE);
    bal_initialize_account_changes(&nonce_changes, account_id, BAL_ITER_NONCE_CHANGE);
    bal_initialize_account_changes(&code_changes, account_id, BAL_ITER_CODE_CHANGE);
    bal_expect(BAL_ITER_ACCOUNT_END, account_id, UINT32_MAX, NULL);
    ++bal_expected_item_count;
  }

  if (account_id_count() != shape.account_count + 1U || storage_id_count() != shape.storage_count ||
      bal_expected_count != shape.expected_count) {
    GUEST_ABORT();
  }
  storage_schema_seal();
}

static void bal_expect_word(const rlp_item *item, u256 expected)
{
  bal_uint(item, 32);
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, expected);
  const uint64_t zero_prefix = 32 - item->content_len;
  for (uint64_t i = 0; i < zero_prefix; ++i) {
    if (bytes[i] != 0) {
      fatal_error(InvalidBlockAccessList);
    }
  }
  if (memcmp(bytes + zero_prefix, item->content, item->content_len) != 0) {
    fatal_error(InvalidBlockAccessList);
  }
}

static void bal_expect_u64(const rlp_item *item, uint64_t expected)
{
  bal_uint(item, 8);
  uint64_t actual = 0;
  for (uint64_t i = 0; i < item->content_len; ++i) {
    actual = (actual << 8) | item->content[i];
  }
  if (actual != expected) {
    fatal_error(InvalidBlockAccessList);
  }
}

static void bal_expect_code(const rlp_item *item, bytes32 code_hash)
{
  bal_bytes(item);

  const uint8_t *code = NULL;
  uint32_t code_len = 0;
  if (!hash_equal(&code_hash, &EVMSAIL_KECCAK_EMPTY)) {
    if (!code_db_lookup_view(code_hash, &code, &code_len, NULL)) {
      fatal_error(WitnessDeficient);
    }
  }
  if (item->content_len != code_len ||
      (code_len != 0 && memcmp(item->content, code, code_len) != 0)) {
    fatal_error(InvalidBlockAccessList);
  }
}

/* One `[index, value]` pair splitter; the recorder tag selects the value
 * comparator. */
static void bal_expect_pair(const rlp_item *pair, enum bal_iter_tag tag, uint64_t expected_index,
                            u256 expected_word, uint64_t expected_nonce, bytes32 code_hash)
{
  rlp_cursor fields;
  rlp_item index;
  rlp_item value;
  bal_list(pair, &fields);
  bal_take(&fields, &index);
  bal_take(&fields, &value);
  bal_end(&fields);
  bal_expect_u64(&index, expected_index);
  switch (tag) {
  case BAL_ITER_STORAGE_CHANGE:
  case BAL_ITER_BALANCE_CHANGE:
    bal_expect_word(&value, expected_word);
    return;
  case BAL_ITER_NONCE_CHANGE:
    bal_expect_u64(&value, expected_nonce);
    return;
  case BAL_ITER_CODE_CHANGE:
    bal_expect_code(&value, code_hash);
    return;
  case BAL_ITER_EMPTY:
  case BAL_ITER_ACCOUNT:
  case BAL_ITER_STORAGE_READ:
  case BAL_ITER_ACCOUNT_END:
    fatal_error(InvalidBlockAccessList);
  }
}

static void bal_validate_expected(const BalExpectedEntry *expected)
{
  AccountId account_id = UINT32_MAX;
  StorageId storage_id = UINT32_MAX;
  uint64_t index = 0;
  uint64_t nonce = 0;
  u256 value = {{0}};
  bytes32 code_hash = {{0}};
  const enum bal_iter_tag tag =
      bal_iter_next_probe(&account_id, &storage_id, &index, &value, &nonce, &code_hash);

  if (tag != (enum bal_iter_tag)expected->tag || account_id != expected->account_id) {
    fatal_error(InvalidBlockAccessList);
  }
  if ((tag == BAL_ITER_STORAGE_CHANGE || tag == BAL_ITER_STORAGE_READ) &&
      storage_id != expected->storage_id) {
    fatal_error(InvalidBlockAccessList);
  }

  if (expected->expected == NULL) {
    return;
  }
  rlp_item item;
  bal_parse_item(expected->expected, expected->expected_len, &item);
  if (item.source_len != expected->expected_len) {
    fatal_error(RlpDecode);
  }
  bal_expect_pair(&item, tag, index, value, nonce, code_hash);
}

/* Both consumers receive the same decoded block field under the same fork
 * gate, so the expected stream needs no source-identity re-check. */
void validate_block_access_list(Bytes bytes, uint64_t block_gas_limit)
{
  (void)bytes;
  bal_prepare_iter();
  for (uint32_t i = 0; i < bal_expected_count; ++i) {
    bal_validate_expected(&bal_expected[i]);
  }

  AccountId account_id = UINT32_MAX;
  StorageId storage_id = UINT32_MAX;
  uint64_t index = 0;
  uint64_t nonce = 0;
  u256 value = {{0}};
  bytes32 code_hash = {{0}};
  if (bal_iter_next_probe(&account_id, &storage_id, &index, &value, &nonce, &code_hash) !=
      BAL_ITER_EMPTY) {
    fatal_error(InvalidBlockAccessList);
  }

  if (bal_expected_item_count > block_gas_limit / UINT64_C(2000)) {
    fatal_error(BlockAccessListTooLarge);
  }
}
