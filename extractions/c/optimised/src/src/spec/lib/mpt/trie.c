#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint64_t update_child_nibble(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateUnderPrefix:
  {
    struct TriePath path_postfix = updates.relation.variants.UpdateUnderPrefix;
    if ((path_len(path_postfix)) == UINT8_C(0)) {
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
  if (update_under_current_prefix(updates)) {
    return ((uint8_t)((uint32_t)UINT8_C(16) - (uint32_t)(uint8_t)(update_child_nibble(updates))));
  }
  return UINT8_C(0);
}

uint8_t overlay_child_ranges_remaining(struct TrieUpdateCursor updates, bool existing_pending, uint64_t existing_nibble)
{
  bool update_pending = update_under_current_prefix(updates);
  uint8_t update_position;
  if (update_pending) {
    update_position = (uint8_t)(update_child_nibble(updates));
  } else {
    update_position = UINT8_C(16);
  }
  uint8_t existing_position = existing_pending ? (uint8_t)existing_nibble : UINT8_C(16);
  uint8_t next_position = update_position < existing_position ? update_position : existing_position;
  return ((uint8_t)((uint32_t)UINT8_C(17) - (uint32_t)next_position));
}

bool account_trie_update(struct AcctTrieEntry trie_entry, struct TrieUpdate *restrict trieupdate_8_1501)
{
  struct AcctEntry entry = trie_entry.entry;
  struct Account current = entry.value.curr;
  storage_block_iter_begin(entry.addr);
  struct TrieUpdateSource storage_source = StorageTrieUpdates(entry.addr);
  struct TrieUpdateCursor storage_updates = trie_updates_begin(storage_source);
  bool storage_changed = (bool)(!(updates_empty(storage_updates)));
  bytes32 base_storage_root = current.storage_cleared ? EMPTY_TRIE_ROOT : current.info.storage_root;
  bool account_empty = account_info_empty(current.info);
  bytes32 storage_root;
  if (current.present && (!account_empty && storage_changed)) {
    storage_root = (trie_root_cursor(base_storage_root, storage_updates)).root;
  } else {
    storage_root = base_storage_root;
  }
  (*trieupdate_8_1501) = account_update(trie_entry, storage_root);
  bool persisted_account_changed = account_value_changed(entry.value);
  return (bool)(persisted_account_changed || storage_changed);
}

struct TrieRootResult trie_root_cursor(bytes32 base_root, struct TrieUpdateCursor updates)
{
  if (updates_empty(updates)) {
    return ((struct TrieRootResult){.changed = false, .root = base_root});
  }
  struct TrieItem trieitem_8_1803;
  struct TrieUpdateCursor trieupdatecursor_8_1804;
  if (eq_bytes32(base_root, EMPTY_TRIE_ROOT)) {
    trieupdatecursor_8_1804 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(updates, ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)}), UINT8_C(0), &trieitem_8_1803);
  } else {
    Bytes node = node_db_lookup(base_root);
    if (node.len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    } else {
      trieupdatecursor_8_1804 = witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(node, ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)}), updates, UINT8_C(0), &trieitem_8_1803);
    }
  }
  if (updates_empty(trieupdatecursor_8_1804)) {
    return ((struct TrieRootResult){.changed = true, .root = (trie_subtree_root(trieitem_8_1803))});
  }
  fatal_error(WitnessDeficient);
}

struct TrieRootResult trie_root(bytes32 base_root, struct TrieUpdateSource source)
{
  struct TrieUpdateCursor updates = trie_updates_begin(source);
  return trie_root_cursor(base_root, updates);
}

