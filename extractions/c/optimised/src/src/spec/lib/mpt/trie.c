#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes trie_lookup(bytes32 root, struct TriePath key)
{
  if (eq_bytes32(root, EMPTY_TRIE_ROOT)) {
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  Bytes node = node_db_lookup(root);
  if (node.len == UINT8_C(0)) {
    fatal_error(WitnessDeficient);
  }
  return trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes_variant_2(node, key, UINT8_C(0));
}

void validate_secured_state_root(bytes32 root)
{
  if (neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(root, EMPTY_TRIE_ROOT)) {
    Bytes node = node_db_lookup(root);
    if (node.len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    }
    validate_secured_witness_node_Bytes_uint8_t_to_unit(node, UINT8_C(64));
    return;
  }
}

uint64_t update_child_nibble(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateUnderPrefix: {
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

uint8_t overlay_child_ranges_remaining(struct TrieUpdateCursor updates, bool existing_pending,
                                       uint64_t existing_nibble)
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

bool account_trie_update(struct AcctTrieEntry trie_entry,
                         struct TrieUpdate *restrict trieupdate_8_1558)
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
  (*trieupdate_8_1558) = account_update(trie_entry, storage_root);
  bool persisted_account_changed = account_value_changed(entry.value);
  return (bool)(persisted_account_changed || storage_changed);
}

struct TrieRootResult trie_root_cursor(bytes32 base_root, struct TrieUpdateCursor updates)
{
  if (updates_empty(updates)) {
    return ((struct TrieRootResult){.changed = false, .root = base_root});
  }
  struct TrieItem trieitem_8_1876;
  struct TrieUpdateCursor trieupdatecursor_8_1877;
  if (eq_bytes32(base_root, EMPTY_TRIE_ROOT)) {
    trieupdatecursor_8_1877 =
        updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(
            updates, ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)}), UINT8_C(0),
            &trieitem_8_1876);
  } else {
    Bytes node = node_db_lookup(base_root);
    if (node.len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    } else {
      trieupdatecursor_8_1877 =
          witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(
              node, ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)}), updates,
              UINT8_C(0), &trieitem_8_1876);
    }
  }
  if (updates_empty(trieupdatecursor_8_1877)) {
    return ((struct TrieRootResult){.changed = true, .root = (trie_subtree_root(trieitem_8_1876))});
  }
  fatal_error(WitnessDeficient);
}

struct TrieRootResult trie_root(bytes32 base_root, struct TrieUpdateSource source)
{
  struct TrieUpdateCursor updates = trie_updates_begin(source);
  return trie_root_cursor(base_root, updates);
}

Bytes trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes(Bytes node, struct TriePath key, uint8_t pos)
{
  if (node.len == UINT8_C(0)) {
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  switch (decoded.kind) {
  case Kind_InputLeafNode: {
    struct TriePath path = decoded.variants.InputLeafNode.tup0;
    if (path_matches(key, pos, path)) {
      uint8_t path_length = path_len(path);
      if (((uint8_t)((uint32_t)path_length + (uint32_t)pos)) == (path_len(key))) {
        return decoded.variants.InputLeafNode.tup1;
      }
      return EMPTY_STATELESS_INPUT_SLICE;
    }
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  case Kind_InputExtensionNode: {
    struct TriePath path_3_3314 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_3314);
    if (extension_len == UINT8_C(0)) {
      return EMPTY_STATELESS_INPUT_SLICE;
    }
    if (path_matches(key, pos, path_3_3314)) {
      if (((uint8_t)((uint32_t)extension_len + (uint32_t)pos)) <= UINT8_C(64)) {
        Bytes child = resolve_witness_ref(decoded.variants.InputExtensionNode.tup1);
        return trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes(
            child, key, ((uint8_t)((uint32_t)extension_len + (uint32_t)pos)));
      }
      return EMPTY_STATELESS_INPUT_SLICE;
    }
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  case Kind_InputBranchNode: {
    uint8_t key_length_3_3317 = path_len(key);
    if (pos == key_length_3_3317) {
      return decoded.variants.InputBranchNode.tup1;
    }
    if (pos < key_length_3_3317) {
      uint64_t child_nibble = path_nibble(key, pos);
      struct NodeRef childref_3_3318 =
          branch_refs_get(decoded.variants.InputBranchNode.tup0, child_nibble);
      Bytes child_3_3319 = resolve_witness_ref(childref_3_3318);
      return trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes(
          child_3_3319, key, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)pos)));
    }
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  }
}

Bytes trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes_variant_2(Bytes node, struct TriePath key,
                                                                 uint8_t pos)
{
  if (node.len == UINT8_C(0)) {
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  switch (decoded.kind) {
  case Kind_InputLeafNode: {
    struct TriePath path = decoded.variants.InputLeafNode.tup0;
    if (path_matches(key, pos, path)) {
      uint8_t path_length = path_len(path);
      if (((uint8_t)((uint32_t)path_length + (uint32_t)pos)) == (path_len(key))) {
        return decoded.variants.InputLeafNode.tup1;
      }
      return EMPTY_STATELESS_INPUT_SLICE;
    }
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  case Kind_InputExtensionNode: {
    struct TriePath path_3_3314 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_3314);
    if (extension_len == UINT8_C(0)) {
      return EMPTY_STATELESS_INPUT_SLICE;
    }
    if (path_matches(key, pos, path_3_3314)) {
      Bytes child = resolve_witness_ref(decoded.variants.InputExtensionNode.tup1);
      return trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes(
          child, key, ((uint8_t)((uint32_t)extension_len + (uint32_t)pos)));
    }
    return EMPTY_STATELESS_INPUT_SLICE;
  }
  case Kind_InputBranchNode: {
    if (pos == (path_len(key))) {
      return decoded.variants.InputBranchNode.tup1;
    }
    uint64_t child_nibble = path_nibble_struct_TriePath_uint8_t_to_uint64_t(key, pos);
    struct NodeRef childref_3_3318 =
        branch_refs_get(decoded.variants.InputBranchNode.tup0, child_nibble);
    Bytes child_3_3319 = resolve_witness_ref(childref_3_3318);
    return trie_walk_Bytes_struct_TriePath_uint8_t_to_Bytes(
        child_3_3319, key, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)pos)));
  }
  }
}

void validate_secured_witness_node_Bytes_uint8_t_to_unit(Bytes node, uint8_t remaining)
{
  if (node.len == UINT8_C(0)) {
    return;
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  switch (decoded.kind) {
  case Kind_InputLeafNode: {
    if ((path_len(decoded.variants.InputLeafNode.tup0)) != remaining) {
      fatal_error(WitnessDeficient);
    }
    break;
  }
  case Kind_InputExtensionNode: {
    struct NodeRef childref = decoded.variants.InputExtensionNode.tup1;
    uint8_t length_ = path_len(decoded.variants.InputExtensionNode.tup0);
    uint8_t next_remaining;
    if ((UINT8_C(0) < length_) && (length_ < remaining)) {
      next_remaining = ((uint8_t)((uint32_t)remaining - (uint32_t)length_));
    } else {
      fatal_error(WitnessDeficient);
    }
    Bytes child;
    Bytes tmp_3_3294;
    switch (childref.kind) {
    case Kind_EmptyRef:
    case Kind_ScratchInlineRef:
      fatal_error(WitnessDeficient);
    case Kind_InputInlineRef:
      tmp_3_3294 = childref.variants.InputInlineRef;
      break;
    case Kind_HashRef:
      tmp_3_3294 = node_db_lookup(childref.variants.HashRef);
      break;
    }
    child = tmp_3_3294;
    if (child.len != UINT8_C(0)) {
      if ((decode_input_trie_node(child)).kind != Kind_InputBranchNode) {
        fatal_error(WitnessDeficient);
      }
    }
    validate_secured_witness_node_Bytes_uint8_t_to_unit_variant_2(child, next_remaining);
    break;
  }
  case Kind_InputBranchNode: {
    vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
    uint8_t next_remaining_3_3306;
    if (decoded.variants.InputBranchNode.tup1.len == UINT8_C(0)) {
      next_remaining_3_3306 = ((uint8_t)((uint32_t)remaining - (uint32_t)UINT8_C(1)));
    } else {
      fatal_error(WitnessDeficient);
    }
    uint8_t occupied = UINT8_C(0);
    int64_t i = (int64_t)UINT8_C(0);
    while (i <= (int64_t)UINT8_C(15)) {
      struct NodeRef plain_vector_access_result_2_2311 =
          fast_vector_access_vector_16_NodeRef(children, i);
      if (plain_vector_access_result_2_2311.kind != Kind_EmptyRef) {
        uint8_t count = occupied;
        if (count < UINT8_C(16)) {
          occupied = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)count));
        }
      }
      Bytes child_3_3307;
      struct NodeRef plain_vector_access_result_2_2313 =
          fast_vector_access_vector_16_NodeRef(children, i);
      Bytes tmp_3_3285;
      switch (plain_vector_access_result_2_2313.kind) {
      case Kind_EmptyRef:
        tmp_3_3285 = EMPTY_STATELESS_INPUT_SLICE;
        break;
      case Kind_InputInlineRef:
        tmp_3_3285 = plain_vector_access_result_2_2313.variants.InputInlineRef;
        break;
      case Kind_ScratchInlineRef:
        fatal_error(WitnessDeficient);
      case Kind_HashRef:
        tmp_3_3285 = node_db_lookup(plain_vector_access_result_2_2313.variants.HashRef);
        break;
      }
      child_3_3307 = tmp_3_3285;
      validate_secured_witness_node_Bytes_uint8_t_to_unit_variant_2(child_3_3307,
                                                                    next_remaining_3_3306);
      i = (i + (int64_t)UINT8_C(1));
    }
    if (occupied < UINT8_C(2)) {
      fatal_error(WitnessDeficient);
    }
  }
  }
}

void validate_secured_witness_node_Bytes_uint8_t_to_unit_variant_2(Bytes node, uint8_t remaining)
{
  if (node.len == UINT8_C(0)) {
    return;
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  switch (decoded.kind) {
  case Kind_InputLeafNode: {
    if ((path_len(decoded.variants.InputLeafNode.tup0)) != remaining) {
      fatal_error(WitnessDeficient);
    }
    break;
  }
  case Kind_InputExtensionNode: {
    struct NodeRef childref = decoded.variants.InputExtensionNode.tup1;
    uint8_t length_ = path_len(decoded.variants.InputExtensionNode.tup0);
    uint8_t next_remaining;
    if ((UINT8_C(0) < length_) && (length_ < remaining)) {
      next_remaining = ((uint8_t)((uint32_t)remaining - (uint32_t)length_));
    } else {
      fatal_error(WitnessDeficient);
    }
    Bytes child;
    Bytes tmp_3_3294;
    switch (childref.kind) {
    case Kind_EmptyRef:
    case Kind_ScratchInlineRef:
      fatal_error(WitnessDeficient);
    case Kind_InputInlineRef:
      tmp_3_3294 = childref.variants.InputInlineRef;
      break;
    case Kind_HashRef:
      tmp_3_3294 = node_db_lookup(childref.variants.HashRef);
      break;
    }
    child = tmp_3_3294;
    if (child.len != UINT8_C(0)) {
      if ((decode_input_trie_node(child)).kind != Kind_InputBranchNode) {
        fatal_error(WitnessDeficient);
      }
    }
    validate_secured_witness_node_Bytes_uint8_t_to_unit_variant_2(child, next_remaining);
    break;
  }
  case Kind_InputBranchNode: {
    vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
    uint8_t next_remaining_3_3306;
    if ((UINT8_C(0) < remaining) && (decoded.variants.InputBranchNode.tup1.len == UINT8_C(0))) {
      next_remaining_3_3306 = ((uint8_t)((uint32_t)remaining - (uint32_t)UINT8_C(1)));
    } else {
      fatal_error(WitnessDeficient);
    }
    uint8_t occupied = UINT8_C(0);
    int64_t i = (int64_t)UINT8_C(0);
    while (i <= (int64_t)UINT8_C(15)) {
      struct NodeRef plain_vector_access_result_2_2311 =
          fast_vector_access_vector_16_NodeRef(children, i);
      if (plain_vector_access_result_2_2311.kind != Kind_EmptyRef) {
        uint8_t count = occupied;
        if (count < UINT8_C(16)) {
          occupied = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)count));
        }
      }
      Bytes child_3_3307;
      struct NodeRef plain_vector_access_result_2_2313 =
          fast_vector_access_vector_16_NodeRef(children, i);
      Bytes tmp_3_3285;
      switch (plain_vector_access_result_2_2313.kind) {
      case Kind_EmptyRef:
        tmp_3_3285 = EMPTY_STATELESS_INPUT_SLICE;
        break;
      case Kind_InputInlineRef:
        tmp_3_3285 = plain_vector_access_result_2_2313.variants.InputInlineRef;
        break;
      case Kind_ScratchInlineRef:
        fatal_error(WitnessDeficient);
      case Kind_HashRef:
        tmp_3_3285 = node_db_lookup(plain_vector_access_result_2_2313.variants.HashRef);
        break;
      }
      child_3_3307 = tmp_3_3285;
      validate_secured_witness_node_Bytes_uint8_t_to_unit_variant_2(child_3_3307,
                                                                    next_remaining_3_3306);
      i = (i + (int64_t)UINT8_C(1));
    }
    if (occupied < UINT8_C(2)) {
      fatal_error(WitnessDeficient);
    }
  }
  }
}
