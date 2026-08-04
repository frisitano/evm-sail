/* Optimized Block Access List authentication.
 *
 * This file implements the high-level override declared by
 * sail/optimised/executor/block_access_list.sail. The decoder consumes the
 * canonical RLP and the C-private BAL stream together so the optimized guest
 * does not materialize intermediate Sail lists or aggregate rows. */
#include "evmsail/prelude.h"
#include "evmsail/executor/block_access_list.h"
#include "evmsail/exceptions.h"
#include "evmsail/kernel/accounts.h"
#include "evmsail/kernel/storage.h"
#include "evmsail/primitives/crypto.h"
#include "host/code/store.h"
#include "host/state/internal.h"
#include "primitives/hash.h"
#include "evmsail/host/region_access.h"
#include "host/state/store.h"
#include "primitives/value.h"
#include "workspace.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Complete EIP-7928 authentication.  The spec build keeps the recursive Sail
 * decoder; the optimized build parses the immutable input in place and
 * compares it directly with the C-private BAL stream.
 */
typedef struct {
  const uint8_t *source;
  const uint8_t *content;
  uint64_t source_len;
  uint64_t content_len;
  bool is_list;
  bool canonical;
} bal_rlp_item;

typedef struct {
  const uint8_t *next;
  uint64_t remaining;
} bal_rlp_cursor;

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
static uint64_t bal_expected_item_count;
static const uint8_t *bal_expected_source;
static uint64_t bal_expected_source_len;
static bool bal_expected_ready;

void bal_input_workspace_bind(void) {
  WORKSPACE_BIND(bal_expected, GUEST_BAL_EXPECTED_ENTRIES);
}

static bool bal_fail(enum BlockError reason, const char *location) {
  throw_invalid_block(reason, location);
  return false;
}

static bool bal_expect(enum bal_iter_tag tag, AccountId account_id,
                       StorageId storage_id, const bal_rlp_item *item) {
  if (bal_expected_count >= GUEST_BAL_EXPECTED_ENTRIES) GUEST_ABORT();
  if (item != NULL && item->source_len > UINT32_MAX)
    return bal_fail(InvalidBlockAccessList, "BAL expected value length");
  BalExpectedEntry *entry = &bal_expected[bal_expected_count++];
  entry->tag = (uint8_t)tag;
  entry->account_id = account_id;
  entry->storage_id = storage_id;
  entry->expected = item == NULL ? NULL : item->source;
  entry->expected_len = item == NULL ? 0 : (uint32_t)item->source_len;
  return true;
}

static bool bal_parse_length(const uint8_t *source, uint64_t available,
                                       uint64_t width, uint64_t *length,
                                       bool *minimal) {
  if (available < width) return false;
  uint64_t value = 0;
  for (uint64_t i = 0; i < width; ++i)
    value = (value << 8) | source[i];
  *length = value;
  *minimal = source[0] != 0;
  return true;
}

static bool bal_parse_item(const uint8_t *source, uint64_t available,
                                     bal_rlp_item *item) {
  if (available == 0)
    return bal_fail(RlpDecode, "BAL RLP item");

  const uint8_t header = source[0];
  uint64_t content_offset = 0;
  uint64_t content_len = 0;
  bool is_list = false;
  bool canonical = true;

  if (header < 0x80) {
    content_len = 1;
  } else if (header <= 0xb7) {
    content_offset = 1;
    content_len = (uint64_t)header - 0x80;
  } else if (header <= 0xbf) {
    const uint64_t width = (uint64_t)header - 0xb7;
    bool minimal = false;
    content_offset = 1 + width;
    if (!bal_parse_length(source + 1, available - 1, width,
                                    &content_len, &minimal))
      return bal_fail(RlpDecode, "BAL RLP string length");
    canonical = minimal && content_len > 55;
  } else if (header <= 0xf7) {
    is_list = true;
    content_offset = 1;
    content_len = (uint64_t)header - 0xc0;
  } else {
    const uint64_t width = (uint64_t)header - 0xf7;
    bool minimal = false;
    is_list = true;
    content_offset = 1 + width;
    if (!bal_parse_length(source + 1, available - 1, width,
                                    &content_len, &minimal))
      return bal_fail(RlpDecode, "BAL RLP list length");
    canonical = minimal && content_len > 55;
  }

  if (content_offset > available ||
      content_len > available - content_offset)
    return bal_fail(RlpDecode, "BAL RLP bounds");

  if (!is_list && header >= 0x80 && header <= 0xb7 &&
      content_len == 1 && source[content_offset] < 0x80)
    canonical = false;

  item->source = source;
  item->content = source + content_offset;
  item->source_len = content_offset + content_len;
  item->content_len = content_len;
  item->is_list = is_list;
  item->canonical = canonical;
  return true;
}

static bool bal_take(bal_rlp_cursor *cursor,
                               bal_rlp_item *item) {
  if (!bal_parse_item(cursor->next, cursor->remaining, item))
    return false;
  cursor->next += item->source_len;
  cursor->remaining -= item->source_len;
  return true;
}

static bool bal_list(const bal_rlp_item *item,
                               bal_rlp_cursor *children) {
  if (!item->is_list || !item->canonical)
    return bal_fail(InvalidBlockAccessList,
                              "BAL canonical list");
  children->next = item->content;
  children->remaining = item->content_len;
  return true;
}

static bool bal_bytes(const bal_rlp_item *item) {
  if (item->is_list || !item->canonical)
    return bal_fail(InvalidBlockAccessList,
                              "BAL canonical bytes");
  return true;
}

static bool bal_uint(const bal_rlp_item *item,
                               uint64_t maximum_len) {
  if (!bal_bytes(item)) return false;
  if (item->content_len > maximum_len ||
      (item->content_len != 0 && item->content[0] == 0))
    return bal_fail(InvalidBlockAccessList,
                              "BAL canonical integer");
  return true;
}

static bool bal_end(const bal_rlp_cursor *cursor) {
  if (cursor->remaining != 0)
    return bal_fail(InvalidBlockAccessList,
                              "BAL trailing fields");
  return true;
}

static bool bal_decode_word(const bal_rlp_item *item, U256 *value) {
  if (!bal_uint(item, 32)) return false;
  uint8_t bytes[32] = {0};
  memcpy(bytes + 32 - item->content_len, item->content, item->content_len);
  *value = be_bytes_to_sail_word(bytes);
  return true;
}

static bool bal_initialize_storage_changes(const bal_rlp_item *field,
                                           AccountId account_id,
                                           NodeId storage_root_node) {
  bal_rlp_cursor slots;
  U256 previous_slot = {{0}};
  bool have_previous_slot = false;
  if (!bal_list(field, &slots)) return false;
  while (slots.remaining != 0) {
    bal_rlp_item entry;
    bal_rlp_cursor fields;
    bal_rlp_item slot_item;
    bal_rlp_item changes_item;
    bal_rlp_cursor changes;
    U256 slot;
    if (!bal_take(&slots, &entry) || !bal_list(&entry, &fields) ||
        !bal_take(&fields, &slot_item) ||
        !bal_take(&fields, &changes_item) || !bal_end(&fields) ||
        !bal_list(&changes_item, &changes) || changes.remaining == 0 ||
        !bal_decode_word(&slot_item, &slot)) {
      if (!have_exception)
        bal_fail(InvalidBlockAccessList, "BAL storage-change shape");
      return false;
    }
    if (have_previous_slot && word_compare(&previous_slot, &slot) >= 0)
      return bal_fail(InvalidBlockAccessList,
                      "BAL storage changes are not strictly increasing");
    previous_slot = slot;
    have_previous_slot = true;

    const StorageId expected_storage_id = storage_id_count();
    StorageId storage_id;
    struct StorageValue value;
    if (!storage_preload(account_id, storage_root_node, slot, &storage_id,
                         &value))
      return false;
    if (storage_id != expected_storage_id ||
        storage_id_count() != expected_storage_id + 1u)
      return bal_fail(InvalidBlockAccessList,
                      "duplicate BAL storage slot");

    ++bal_expected_item_count;
    while (changes.remaining != 0) {
      bal_rlp_item pair;
      if (!bal_take(&changes, &pair) ||
          !bal_expect(BAL_ITER_STORAGE_CHANGE, account_id, storage_id, &pair))
        return false;
    }
  }
  return true;
}

static bool bal_initialize_storage_reads(const bal_rlp_item *field,
                                         AccountId account_id,
                                         NodeId storage_root_node) {
  bal_rlp_cursor reads;
  U256 previous_slot = {{0}};
  bool have_previous_slot = false;
  if (!bal_list(field, &reads)) return false;
  while (reads.remaining != 0) {
    bal_rlp_item slot_item;
    U256 slot;
    if (!bal_take(&reads, &slot_item) ||
        !bal_decode_word(&slot_item, &slot))
      return false;
    if (have_previous_slot && word_compare(&previous_slot, &slot) >= 0)
      return bal_fail(InvalidBlockAccessList,
                      "BAL storage reads are not strictly increasing");
    previous_slot = slot;
    have_previous_slot = true;

    const StorageId expected_storage_id = storage_id_count();
    StorageId storage_id;
    struct StorageValue value;
    if (!storage_preload(account_id, storage_root_node, slot, &storage_id,
                         &value))
      return false;
    if (storage_id != expected_storage_id ||
        storage_id_count() != expected_storage_id + 1u)
      return bal_fail(InvalidBlockAccessList,
                      "duplicate BAL storage slot");
    if (!bal_expect(BAL_ITER_STORAGE_READ, account_id, storage_id, NULL))
      return false;
    ++bal_expected_item_count;
  }
  return true;
}

static bool bal_initialize_account_changes(const bal_rlp_item *field,
                                           AccountId account_id,
                                           enum bal_iter_tag tag) {
  bal_rlp_cursor changes;
  if (!bal_list(field, &changes)) return false;
  while (changes.remaining != 0) {
    bal_rlp_item pair;
    if (!bal_take(&changes, &pair) ||
        !bal_expect(tag, account_id, UINT32_MAX, &pair))
      return false;
  }
  return true;
}

unit initialize_block_access_list_state(
    struct StatelessInputSliceFields bytes, Hash32 parent_state_root) {
  bal_expected_count = 0;
  bal_expected_item_count = 0;
  bal_expected_source = NULL;
  bal_expected_source_len = 0;
  bal_expected_ready = false;

  const uint8_t *source = bytes.bytes;
  if (source == NULL && bytes.len != 0) {
    bal_fail(RlpDecode, "BAL preload input");
    return UNIT;
  }

  bal_rlp_item root;
  if (!bal_parse_item(source, bytes.len, &root)) return UNIT;
  if (root.source_len != bytes.len) {
    bal_fail(RlpDecode, "BAL preload root length");
    return UNIT;
  }

  bal_rlp_cursor accounts;
  Address previous_address = {{0}};
  bool have_previous_address = false;
  if (!bal_list(&root, &accounts)) return UNIT;

  uint32_t account_count = 0;
  bal_rlp_cursor account_counter = accounts;
  while (account_counter.remaining != 0) {
    bal_rlp_item account_item;
    if (!bal_take(&account_counter, &account_item)) return UNIT;
    if (account_count == GUEST_STATE_ACCOUNTS) {
      bal_fail(InvalidBlockAccessList, "BAL account capacity");
      return UNIT;
    }
    account_count++;
  }
  account_schema_prepare(account_count);

  while (accounts.remaining != 0) {
    bal_rlp_item account_item;
    bal_rlp_cursor fields;
    bal_rlp_item address_item;
    bal_rlp_item storage_changes;
    bal_rlp_item storage_reads;
    bal_rlp_item balance_changes;
    bal_rlp_item nonce_changes;
    bal_rlp_item code_changes;
    if (!bal_take(&accounts, &account_item) ||
        !bal_list(&account_item, &fields) ||
        !bal_take(&fields, &address_item) ||
        !bal_take(&fields, &storage_changes) ||
        !bal_take(&fields, &storage_reads) ||
        !bal_take(&fields, &balance_changes) ||
        !bal_take(&fields, &nonce_changes) ||
        !bal_take(&fields, &code_changes) || !bal_end(&fields) ||
        !bal_bytes(&address_item) || address_item.content_len != 20) {
      if (!have_exception)
        bal_fail(InvalidBlockAccessList, "BAL preload account shape");
      return UNIT;
    }

    const Address address = address_from_be_bytes(address_item.content);
    if (have_previous_address &&
        address_compare(&previous_address, &address) >= 0) {
      bal_fail(InvalidBlockAccessList,
               "BAL accounts are not strictly increasing");
      return UNIT;
    }
    previous_address = address;
    have_previous_address = true;
    const Hash32 address_hash = host_keccak_address(address);
    const AccountId account_id =
        account_schema_insert(&address);
    struct Account account;
    NodeId storage_root_node = EVMSAIL_NODE_ID_EMPTY;
    if (account_id == ACCOUNT_ID_NONE ||
        !account_preload(parent_state_root, account_id, address_hash, &account,
                         &storage_root_node) ||
        !bal_expect(BAL_ITER_ACCOUNT, account_id, UINT32_MAX, NULL))
      return UNIT;

    storage_schema_account_begin(account_id);
    if (!bal_initialize_storage_changes(&storage_changes, account_id,
                                        storage_root_node) ||
        !bal_initialize_storage_reads(&storage_reads, account_id,
                                      storage_root_node))
      return UNIT;
    storage_schema_account_end(account_id);

    if (!bal_initialize_account_changes(&balance_changes, account_id,
                                        BAL_ITER_BALANCE_CHANGE) ||
        !bal_initialize_account_changes(&nonce_changes, account_id,
                                        BAL_ITER_NONCE_CHANGE) ||
        !bal_initialize_account_changes(&code_changes, account_id,
                                        BAL_ITER_CODE_CHANGE) ||
        !bal_expect(BAL_ITER_ACCOUNT_END, account_id, UINT32_MAX, NULL))
      return UNIT;
    ++bal_expected_item_count;
  }

  storage_schema_seal();
  bal_expected_source = source;
  bal_expected_source_len = bytes.len;
  bal_expected_ready = true;
  return UNIT;
}

static bool bal_decode_u64(const bal_rlp_item *item,
                                     uint64_t *value) {
  if (!bal_uint(item, 8)) return false;
  uint64_t result = 0;
  for (uint64_t i = 0; i < item->content_len; ++i)
    result = (result << 8) | item->content[i];
  *value = result;
  return true;
}

static bool bal_word_equal(const bal_rlp_item *item,
                                     U256 expected) {
  if (!bal_uint(item, 32)) return false;
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, expected);
  const uint64_t zero_prefix = 32 - item->content_len;
  for (uint64_t i = 0; i < zero_prefix; ++i)
    if (bytes[i] != 0)
      return bal_fail(InvalidBlockAccessList,
                                "BAL word width");
  if (memcmp(bytes + zero_prefix, item->content, item->content_len) != 0)
    return bal_fail(InvalidBlockAccessList,
                              "BAL word mismatch");
  return true;
}

static bool bal_u64_equal(const bal_rlp_item *item,
                                    uint64_t expected) {
  uint64_t actual = 0;
  if (!bal_decode_u64(item, &actual)) return false;
  if (actual != expected)
    return bal_fail(InvalidBlockAccessList,
                              "BAL integer mismatch");
  return true;
}

static bool bal_pair_word(const bal_rlp_item *pair,
                                    uint64_t expected_index,
                                    U256 expected_value) {
  bal_rlp_cursor fields;
  bal_rlp_item index;
  bal_rlp_item value;
  if (!bal_list(pair, &fields) ||
      !bal_take(&fields, &index) ||
      !bal_take(&fields, &value) ||
      !bal_end(&fields))
    return false;
  return bal_u64_equal(&index, expected_index) &&
         bal_word_equal(&value, expected_value);
}

static bool bal_pair_nonce(const bal_rlp_item *pair,
                                     uint64_t expected_index,
                                     uint64_t expected_value) {
  bal_rlp_cursor fields;
  bal_rlp_item index;
  bal_rlp_item value;
  if (!bal_list(pair, &fields) ||
      !bal_take(&fields, &index) ||
      !bal_take(&fields, &value) ||
      !bal_end(&fields))
    return false;
  return bal_u64_equal(&index, expected_index) &&
         bal_u64_equal(&value, expected_value);
}

static bool bal_code_equal(const bal_rlp_item *item,
                                     Hash32 code_hash) {
  if (!bal_bytes(item)) return false;

  const uint8_t *code = NULL;
  uint64_t code_len = 0;
  if (!hash_equal(&code_hash, &EVMSAIL_KECCAK_EMPTY)) {
    if (!code_db_lookup_view(code_hash, &code, &code_len, NULL))
      return bal_fail(WitnessDeficient,
                                "BAL code lookup");
  }
  if (item->content_len != code_len ||
      (code_len != 0 && memcmp(item->content, code, code_len) != 0))
    return bal_fail(InvalidBlockAccessList,
                              "BAL code mismatch");
  return true;
}

static bool bal_pair_code(const bal_rlp_item *pair,
                                    uint64_t expected_index,
                                    Hash32 code_hash) {
  bal_rlp_cursor fields;
  bal_rlp_item index;
  bal_rlp_item code;
  if (!bal_list(pair, &fields) ||
      !bal_take(&fields, &index) ||
      !bal_take(&fields, &code) ||
      !bal_end(&fields))
    return false;
  return bal_u64_equal(&index, expected_index) &&
         bal_code_equal(&code, code_hash);
}

static bool bal_validate_expected(uint32_t position,
                                  const BalExpectedEntry *expected) {
  AccountId account_id = UINT32_MAX;
  StorageId storage_id = UINT32_MAX;
  uint64_t index = 0;
  uint64_t nonce = 0;
  U256 value = {{0}};
  Hash32 code_hash = {{0}};
  const enum bal_iter_tag tag = bal_iter_next_probe(
      &account_id, &storage_id, &index, &value, &nonce, &code_hash);

  (void)position;

  if (tag != (enum bal_iter_tag)expected->tag ||
      account_id != expected->account_id)
    return bal_fail(InvalidBlockAccessList, "BAL recorder shape");
  if ((tag == BAL_ITER_STORAGE_CHANGE || tag == BAL_ITER_STORAGE_READ) &&
      storage_id != expected->storage_id)
    return bal_fail(InvalidBlockAccessList, "BAL storage identity");

  if (expected->expected == NULL) return true;
  bal_rlp_item item;
  if (!bal_parse_item(expected->expected, expected->expected_len, &item) ||
      item.source_len != expected->expected_len)
    return false;

  switch (tag) {
  case BAL_ITER_STORAGE_CHANGE:
  case BAL_ITER_BALANCE_CHANGE:
    return bal_pair_word(&item, index, value);
  case BAL_ITER_NONCE_CHANGE:
    return bal_pair_nonce(&item, index, nonce);
  case BAL_ITER_CODE_CHANGE:
    return bal_pair_code(&item, index, code_hash);
  default:
    return bal_fail(InvalidBlockAccessList, "BAL unexpected value");
  }
}

unit validate_block_access_list(struct StatelessInputSliceFields bytes,
                                uint64_t block_gas_limit) {
  const uint8_t *source = bytes.bytes;
  if (!bal_expected_ready || source != bal_expected_source ||
      bytes.len != bal_expected_source_len) {
    bal_fail(InvalidBlockAccessList, "BAL initialization");
    return UNIT;
  }

  (void)bal_prepare_iter(UNIT);
  for (uint32_t i = 0; i < bal_expected_count; ++i) {
    if (!bal_validate_expected(i, &bal_expected[i])) return UNIT;
  }

  AccountId account_id = UINT32_MAX;
  StorageId storage_id = UINT32_MAX;
  uint64_t index = 0;
  uint64_t nonce = 0;
  U256 value = {{0}};
  Hash32 code_hash = {{0}};
  if (bal_iter_next_probe(&account_id, &storage_id, &index, &value, &nonce,
                          &code_hash) != BAL_ITER_EMPTY) {
    bal_fail(InvalidBlockAccessList, "BAL recorder trailing entry");
    return UNIT;
  }

  if (bal_expected_item_count > block_gas_limit / UINT64_C(2000))
    bal_fail(BlockAccessListTooLarge, "BAL item gas");
  return UNIT;
}
