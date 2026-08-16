#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint64_t update_child_nibble(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateUnderPrefix:
  {
    struct TriePath path_postfix = updates.relation.variants.UpdateUnderPrefix;
    uint8_t postfix_len = path_len(path_postfix);
    if (postfix_len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    }
    return path_nibble_struct_TriePath_uint8_t_to_uint64_t(path_postfix, UINT8_C(0));
  }
  case Kind_UpdateBeyondPrefix:
  case Kind_UpdateSourceExhausted:
    fatal_error(WitnessDeficient);
  }
}

bool update_under_current_prefix(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateUnderPrefix:
    return true;
  case Kind_UpdateBeyondPrefix:
  case Kind_UpdateSourceExhausted:
    return false;
  }
}

uint8_t update_child_ranges_remaining(struct TrieUpdateCursor updates)
{
  bool update_pending = update_under_current_prefix(updates);
  if (update_pending) {
    uint64_t child_nibble = update_child_nibble(updates);
    return ((uint8_t)((uint32_t)UINT8_C(16) - (uint32_t)(uint8_t)child_nibble));
  }
  return UINT8_C(0);
}

uint8_t overlay_child_ranges_remaining(struct TrieUpdateCursor updates, bool existing_pending, uint64_t existing_nibble)
{
  bool update_pending = update_under_current_prefix(updates);
  uint8_t update_position;
  if (update_pending) {
    uint64_t child_nibble = update_child_nibble(updates);
    update_position = (uint8_t)child_nibble;
  } else {
    update_position = UINT8_C(16);
  }
  uint8_t existing_position = existing_pending ? (uint8_t)existing_nibble : UINT8_C(16);
  uint8_t next_position = update_position < existing_position ? update_position : existing_position;
  return ((uint8_t)((uint32_t)UINT8_C(17) - (uint32_t)next_position));
}

struct tuple_TrieUpdate_bool account_trie_update(struct AcctTrieEntry trie_entry)
{
  struct AcctEntry entry = trie_entry.entry;
  struct Account current = entry.value.curr;
  storage_block_iter_begin(entry.addr);
  struct TrieUpdateSource storage_source = StorageTrieUpdates(entry.addr);
  struct TrieUpdateCursor storage_updates = trie_updates_begin(storage_source);
  bool no_storage_changes = updates_empty(storage_updates);
  bool storage_changed = (bool)(!no_storage_changes);
  bytes32 base_storage_root = current.storage_cleared ? EMPTY_TRIE_ROOT : current.info.storage_root;
  bool account_empty = account_info_empty(current.info);
  bytes32 storage_root;
  if (current.present && (!account_empty && storage_changed)) {
    struct TrieRootResult trie_root_cursor_result_2_6 = trie_root_cursor(base_storage_root, storage_updates);
    storage_root = trie_root_cursor_result_2_6.root;
  } else {
    storage_root = base_storage_root;
  }
  struct TrieUpdate update = account_update(trie_entry, storage_root);
  bool persisted_account_changed = account_value_changed(entry.value);
  return ((struct tuple_TrieUpdate_bool){.tup0 = update, .tup1 = (bool)(persisted_account_changed || storage_changed)});
}

struct TrieRootResult trie_root_cursor(bytes32 base_root, struct TrieUpdateCursor updates)
{
  bool no_updates = updates_empty(updates);
  if (no_updates) {
    return ((struct TrieRootResult){.changed = false, .root = base_root});
  }
  struct tuple_TrieItem_TrieUpdateCursor result_2_39;
  if (eq_bytes32(base_root, EMPTY_TRIE_ROOT)) {
    struct TriePath tmp_3_343;
    vector_32_bits_8 tmp_3_344 = internal_vector_init_vector_32_bits_8(INT64_C(32));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(0), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(1), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(2), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(3), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(4), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(5), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(6), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(7), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(8), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(9), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(10), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(11), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(12), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(13), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(14), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(15), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(16), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(17), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(18), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(19), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(20), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(21), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(22), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(23), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(24), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(25), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(26), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(27), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(28), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(29), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(30), UINT64_C(0x00));
    tmp_3_344 = internal_vector_update_vector_32_bits_8(tmp_3_344, INT64_C(31), UINT64_C(0x00));
    tmp_3_343.data = bytes32_zero();
    for (size_t tmp_8_1523 = 0; tmp_8_1523 < 32; ++tmp_8_1523) {
      tmp_3_343.data = fast_unsigned_vector_update_bytes32(tmp_3_343.data, tmp_8_1523, tmp_3_344.data[tmp_8_1523]);
    }
    tmp_3_343.len = UINT8_C(0);
    result_2_39 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(updates, tmp_3_343, UINT8_C(0));
  } else {
    Bytes node = node_db_lookup(base_root);
    if (node.len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    } else {
      struct TriePath tmp_3_341;
      vector_32_bits_8 tmp_3_342 = internal_vector_init_vector_32_bits_8(INT64_C(32));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(0), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(1), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(2), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(3), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(4), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(5), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(6), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(7), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(8), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(9), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(10), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(11), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(12), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(13), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(14), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(15), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(16), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(17), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(18), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(19), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(20), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(21), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(22), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(23), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(24), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(25), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(26), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(27), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(28), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(29), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(30), UINT64_C(0x00));
      tmp_3_342 = internal_vector_update_vector_32_bits_8(tmp_3_342, INT64_C(31), UINT64_C(0x00));
      tmp_3_341.data = bytes32_zero();
      for (size_t tmp_8_1522 = 0; tmp_8_1522 < 32; ++tmp_8_1522) {
        tmp_3_341.data = fast_unsigned_vector_update_bytes32(tmp_3_341.data, tmp_8_1522, tmp_3_342.data[tmp_8_1522]);
      }
      tmp_3_341.len = UINT8_C(0);
      result_2_39 = witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(node, tmp_3_341, updates, UINT8_C(0));
    }
  }
  bool all_updates_consumed = updates_empty(result_2_39.tup1);
  if (all_updates_consumed) {
    bytes32 trie_subtree_root_result_2_40 = trie_subtree_root(result_2_39.tup0);
    return ((struct TrieRootResult){.changed = true, .root = trie_subtree_root_result_2_40});
  }
  fatal_error(WitnessDeficient);
}

struct TrieRootResult trie_root(bytes32 base_root, struct TrieUpdateSource source)
{
  struct TrieUpdateCursor updates = trie_updates_begin(source);
  return trie_root_cursor(base_root, updates);
}

