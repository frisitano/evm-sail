#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct TrieUpdate EMPTY_TRIE_UPDATE;


void create_letbind_208(void) {

  struct TrieUpdate let_value_3_264;
  struct TrieChange TrieDelete_result_2_0 = TrieDelete(UNIT);
  struct TrieUpdate tmp_3_261;
  tmp_3_261.change = TrieDelete_result_2_0;
  struct TriePath tmp_3_262;
  vector_32_bits_8 tmp_3_263 = internal_vector_init_vector_32_bits_8(INT64_C(32));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(0), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(1), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(2), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(3), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(4), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(5), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(6), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(7), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(8), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(9), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(10), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(11), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(12), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(13), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(14), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(15), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(16), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(17), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(18), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(19), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(20), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(21), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(22), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(23), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(24), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(25), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(26), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(27), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(28), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(29), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(30), UINT64_C(0x00));
  tmp_3_263 = internal_vector_update_vector_32_bits_8(tmp_3_263, INT64_C(31), UINT64_C(0x00));
  tmp_3_262.data = bytes32_zero();
  for (size_t tmp_8_1520 = 0; tmp_8_1520 < 32; ++tmp_8_1520) {
    tmp_3_262.data = fast_unsigned_vector_update_bytes32(tmp_3_262.data, tmp_8_1520, tmp_3_263.data[tmp_8_1520]);
  }
  tmp_3_262.len = UINT8_C(0);
  tmp_3_261.key = tmp_3_262;
  let_value_3_264 = tmp_3_261;
  EMPTY_TRIE_UPDATE = let_value_3_264;
}
void kill_letbind_208(void) {
}

bytes32 trie_subtree_root(struct TrieItem subtree)
{
  switch (subtree.value.kind) {
  case Kind_EmptySubtree:
    return EMPTY_TRIE_ROOT;
  case Kind_BranchItem:
  case Kind_LeafItem:
  case Kind_SubtreeItem:
  {
    struct NodeRef root_ref = trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef_variant_2(subtree, UINT8_C(0));
    return trie_ref_to_root(root_ref);
  }
  }
}

bool updates_empty(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateSourceExhausted:
    return true;
  case Kind_UpdateBeyondPrefix:
  case Kind_UpdateUnderPrefix:
    return false;
  }
}

struct TrieChildren trie_children_add(struct TrieChildren children, struct TriePath prefix, uint64_t index, struct TrieItem child)
{
  switch (child.value.kind) {
  case Kind_EmptySubtree:
    return children;
  case Kind_BranchItem:
  case Kind_LeafItem:
  case Kind_SubtreeItem:
  {
    uint8_t depth = path_len(prefix);
    uint8_t child_depth;
    if (depth < UINT8_C(64)) {
      child_depth = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)depth));
    } else {
      fatal_error(WitnessDeficient);
    }
    uint8_t next_child_count;
    if (children.count < UINT8_C(16)) {
      next_child_count = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)children.count));
    } else {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_segment = path_single(index);
    struct TriePath child_prefix = path_concat(prefix, child_segment);
    bool path_below_child = path_prefix_of(child_prefix, child.path);
    if (!path_below_child) {
      fatal_error(WitnessDeficient);
    }
    bool child_already_present = branch_mask_has(children.mask, index);
    if (child_already_present) {
      fatal_error(WitnessDeficient);
    }
    struct TrieChildren updated = children;
    uint64_t result_2_464 = updated.mask;
    updated.mask = branch_mask_set(result_2_464, index);
    vector_16_NodeRef result_2_465 = updated.children;
    struct NodeRef trie_child_ref_result_2_467 = trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef(child, child_depth);
    updated.children = fast_unsigned_vector_update_vector_16_NodeRef(result_2_465, (uint8_t)index, trie_child_ref_result_2_467);
    updated.only = child;
    updated.count = next_child_count;
    return updated;
  }
  }
}

struct TrieItem trie_empty_subtree(void)
{
  struct TrieItemValue EmptySubtree_result_2_462 = EmptySubtree(UNIT);
  struct TrieItem tmp_3_857;
  struct TriePath tmp_3_858;
  vector_32_bits_8 tmp_3_859 = internal_vector_init_vector_32_bits_8(INT64_C(32));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(0), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(1), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(2), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(3), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(4), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(5), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(6), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(7), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(8), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(9), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(10), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(11), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(12), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(13), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(14), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(15), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(16), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(17), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(18), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(19), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(20), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(21), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(22), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(23), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(24), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(25), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(26), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(27), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(28), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(29), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(30), UINT64_C(0x00));
  tmp_3_859 = internal_vector_update_vector_32_bits_8(tmp_3_859, INT64_C(31), UINT64_C(0x00));
  tmp_3_858.data = bytes32_zero();
  for (size_t tmp_8_1521 = 0; tmp_8_1521 < 32; ++tmp_8_1521) {
    tmp_3_858.data = fast_unsigned_vector_update_bytes32(tmp_3_858.data, tmp_8_1521, tmp_3_859.data[tmp_8_1521]);
  }
  tmp_3_858.len = UINT8_C(0);
  tmp_3_857.path = tmp_3_858;
  tmp_3_857.value = EmptySubtree_result_2_462;
  return tmp_3_857;
}

struct TrieChildren trie_children_empty(void)
{
  struct NodeRef empty_ref = EmptyRef(UNIT);
  vector_16_NodeRef children = fast_unsigned_vector_init_vector_16_NodeRef(UINT8_C(16), empty_ref);
  struct TrieItem empty_subtree = trie_empty_subtree();
  return ((struct TrieChildren){.children = children, .count = UINT8_C(0), .mask = UINT64_C(0x0000), .only = empty_subtree});
}

struct TrieItem trie_branch(struct TriePath path, struct NodeRef childref)
{
  struct TrieItemValue BranchItem_result_2_461 = BranchItem(childref);
  return ((struct TrieItem){.path = path, .value = BranchItem_result_2_461});
}

struct TrieItem trie_children_finish(struct TriePath prefix, struct TrieChildren children)
{
  if (children.count == UINT8_C(0)) {
    struct TrieItemValue EmptySubtree_result_2_456 = EmptySubtree(UNIT);
    return ((struct TrieItem){.path = prefix, .value = EmptySubtree_result_2_456});
  }
  if (children.count == UINT8_C(1)) {
    return children.only;
  }
  struct NodeRef branch_ref = branch_child_ref(children.mask, children.children);
  return trie_branch(prefix, branch_ref);
}

struct TrieItem trie_leaf(struct TriePath path, struct TrieLeafValue value)
{
  struct TrieItemValue LeafItem_result_2_453 = LeafItem(value);
  return ((struct TrieItem){.path = path, .value = LeafItem_result_2_453});
}

struct TrieItem trie_scratch_leaf(struct TriePath path, Bytes value)
{
  struct TrieLeafValue leaf_value = ScratchTrieLeaf(value);
  return trie_leaf(path, leaf_value);
}

struct TrieUpdateCursor trie_updates_descend(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateUnderPrefix:
  {
    struct TriePath path_postfix = updates.relation.variants.UpdateUnderPrefix;
    uint8_t postfix_len = path_len(path_postfix);
    if (postfix_len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_postfix = path_drop_struct_TriePath_uint8_t_to_struct_TriePath(path_postfix, UINT8_C(1));
    struct TrieUpdateCursor descended = updates;
    descended.relation = UpdateUnderPrefix(child_postfix);
    return descended;
  }
  case Kind_UpdateBeyondPrefix:
  case Kind_UpdateSourceExhausted:
    fatal_error(WitnessDeficient);
  }
}

struct TrieUpdateCursor trie_updates_rebase(struct TrieUpdateCursor updates, struct TriePath prefix)
{
  uint8_t prefix_len = path_len(prefix);
  switch (updates.relation.kind) {
  case Kind_UpdateSourceExhausted:
  case Kind_UpdateUnderPrefix:
    return updates;
  case Kind_UpdateBeyondPrefix:
    if (prefix_len <= updates.relation.variants.UpdateBeyondPrefix) {
      struct TriePath path_postfix = path_drop(updates.current.key, prefix_len);
      struct TrieUpdateRelation UpdateUnderPrefix_result_2_448 = UpdateUnderPrefix(path_postfix);
      return ((struct TrieUpdateCursor){.current = updates.current, .relation = UpdateUnderPrefix_result_2_448, .source = updates.source});
    }
    return updates;
  }
}

struct TrieItem trie_input_leaf(struct TriePath path, Bytes value)
{
  struct TrieLeafValue leaf_value = InputTrieLeaf(value);
  return trie_leaf(path, leaf_value);
}

struct TrieItem trie_subtree(struct TriePath path, struct NodeRef childref)
{
  struct TrieItemValue SubtreeItem_result_2_439 = SubtreeItem(childref);
  return ((struct TrieItem){.path = path, .value = SubtreeItem_result_2_439});
}

struct TrieUpdateFetch next_changed_account_trie_update(void)
{
  bool searching = true;
  struct TrieUpdateFetch result = ((struct TrieUpdateFetch){.available = false, .update = EMPTY_TRIE_UPDATE});
  while (searching) {
    struct AcctBlockIterResult iterator_item = acct_block_iter_next();
    switch (iterator_item.kind) {
    case Kind_AcctBlockIterRow:
    {
      struct tuple_TrieUpdate_bool account_trie_update_result_2_12 = account_trie_update(iterator_item.variants.AcctBlockIterRow);
      if (account_trie_update_result_2_12.tup1) {
        result = ((struct TrieUpdateFetch){.available = true, .update = account_trie_update_result_2_12.tup0});
        searching = false;
      }
      break;
    }
    case Kind_AcctBlockIterExhausted:
      searching = false;
      break;
    }
  }
  return result;
}

struct TrieUpdateFetch trie_update_source_next(struct TrieUpdateSource source)
{
  switch (source.kind) {
  case Kind_StorageTrieUpdates:
    return next_storage_trie_update(source.variants.StorageTrieUpdates);
  case Kind_ChangedAccountTrieUpdates:
    return next_changed_account_trie_update();
  }
}

struct TrieUpdateCursor trie_updates_begin(struct TrieUpdateSource source)
{
  struct TrieUpdateFetch first = trie_update_source_next(source);
  struct TrieUpdateRelation relation;
  if (first.available) {
    relation = UpdateUnderPrefix(first.update.key);
  } else {
    relation = UpdateSourceExhausted(UNIT);
  }
  return ((struct TrieUpdateCursor){.current = first.update, .relation = relation, .source = source});
}

struct tuple_TrieUpdate_TrieUpdateCursor trie_updates_pop(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateSourceExhausted:
    fatal_error(WitnessDeficient);
  case Kind_UpdateBeyondPrefix:
  case Kind_UpdateUnderPrefix:
  {
    struct TrieUpdate current = updates.current;
    struct TrieUpdateFetch successor = trie_update_source_next(updates.source);
    struct TrieUpdateCursor next;
    if (successor.available) {
      uint8_t common_prefix_len = common_prefix_length(current.key, successor.update.key);
      struct TrieUpdateRelation UpdateBeyondPrefix_result_2_49 = UpdateBeyondPrefix(common_prefix_len);
      next = ((struct TrieUpdateCursor){.current = successor.update, .relation = UpdateBeyondPrefix_result_2_49, .source = updates.source});
    } else {
      struct TrieUpdateRelation UpdateSourceExhausted_result_2_54 = UpdateSourceExhausted(UNIT);
      next = ((struct TrieUpdateCursor){.current = EMPTY_TRIE_UPDATE, .relation = UpdateSourceExhausted_result_2_54, .source = updates.source});
    }
    return ((struct tuple_TrieUpdate_TrieUpdateCursor){.tup0 = current, .tup1 = next});
  }
  }
}

struct tuple_TrieItem_TrieUpdateCursor updates_subtree(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  bool has_update = update_under_current_prefix(updates);
  if (!has_update) {
    struct TrieItem trie_empty_subtree_result_2_58 = trie_empty_subtree();
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_58, .tup1 = updates});
  }
  if (cursor == UINT8_C(64)) {
    switch (updates.relation.kind) {
    case Kind_UpdateUnderPrefix:
    {
      uint8_t postfix_len = path_len(updates.relation.variants.UpdateUnderPrefix);
      if (postfix_len != UINT8_C(0)) {
        fatal_error(WitnessDeficient);
      }
      break;
    }
    case Kind_UpdateBeyondPrefix:
    case Kind_UpdateSourceExhausted:
      fatal_error(WitnessDeficient);
    }
    struct tuple_TrieUpdate_TrieUpdateCursor trie_updates_pop_result_2_62 = trie_updates_pop(updates);
    struct TrieUpdate update = trie_updates_pop_result_2_62.tup0;
    bool update_matches_prefix = path_eq(update.key, prefix);
    if (!update_matches_prefix) {
      fatal_error(WitnessDeficient);
    }
    struct TrieItem tmp_3_378;
    switch (update.change.kind) {
    case Kind_TrieDelete:
      tmp_3_378 = trie_empty_subtree();
      break;
    case Kind_TriePut:
      tmp_3_378 = trie_scratch_leaf(update.key, update.change.variants.TriePut);
      break;
    }
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = tmp_3_378, .tup1 = trie_updates_pop_result_2_62.tup1});
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool update_pending = update_under_current_prefix(remaining);
  while (update_pending) {
    uint64_t nib = update_child_nibble(remaining);
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix = path_concat(prefix, child_path);
    struct TrieUpdateCursor descended = trie_updates_descend(remaining);
    struct tuple_TrieItem_TrieUpdateCursor updates_subtree_result_2_67 = updates_subtree(descended, child_prefix, next_cursor);
    children = trie_children_add(children, prefix, nib, updates_subtree_result_2_67.tup0);
    struct TrieUpdateCursor rebased = trie_updates_rebase(updates_subtree_result_2_67.tup1, prefix);
    bool rebased_update_pending = update_under_current_prefix(rebased);
    if (rebased_update_pending) {
      uint64_t rebased_nibble = update_child_nibble(rebased);
      if ((uint8_t)rebased_nibble <= (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
    }
    remaining = rebased;
    update_pending = rebased_update_pending;
  }
  struct TrieItem trie_children_finish_result_2_69 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_69, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor witness_subtree(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, cursor);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_411 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_411);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_411);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_413 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t tmp_3_390 = (int64_t)UINT8_C(15);
  int64_t tmp_3_391 = (int64_t)UINT8_C(1);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= tmp_3_390) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_414 = path_concat(prefix, child_path);
    struct NodeRef childref_3_415 = fast_vector_access_vector_16_NodeRef(children, i);
    bool tmp_3_392 = (bool)(childref_3_415.kind != Kind_EmptyRef);
    bool update_pending = update_under_current_prefix(remaining);
    bool update_here;
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      if ((uint8_t)update_nibble < (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
      update_here = (bool)(update_nibble == nib);
    } else {
      update_here = false;
    }
    struct tuple_TrieItem_TrieUpdateCursor result_2_80;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (tmp_3_392) {
        Bytes child = resolve_witness_ref(childref_3_415);
        result_2_80 = witness_subtree(child, child_prefix_3_414, descended, next_cursor_3_413);
      } else {
        result_2_80 = updates_subtree(descended, child_prefix_3_414, next_cursor_3_413);
      }
    } else if (tmp_3_392) {
      struct TrieItem trie_subtree_result_2_78 = trie_subtree(child_prefix_3_414, childref_3_415);
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_subtree_result_2_78, .tup1 = remaining});
    } else {
      struct TrieItem trie_empty_subtree_result_2_79 = trie_empty_subtree();
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_79, .tup1 = remaining});
    }
    built = trie_children_add(built, prefix, nib, result_2_80.tup0);
    if (update_here) {
      remaining = trie_updates_rebase(result_2_80.tup1, prefix);
    } else {
      remaining = result_2_80.tup1;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + tmp_3_391);
  }
  bool update_pending_3_417 = update_under_current_prefix(remaining);
  if (update_pending_3_417) {
    fatal_error(WitnessDeficient);
  }
  struct TrieItem trie_children_finish_result_2_82 = trie_children_finish(prefix, built);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_82, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct NodeRef childref, struct TriePath child_prefix, struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  bool child_below_prefix = path_prefix_of(prefix, child_prefix);
  if ((prefix_len != cursor) || !child_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  bool at_child_prefix = path_eq(prefix, child_prefix);
  if (at_child_prefix) {
    bool has_update = update_under_current_prefix(updates);
    if (has_update) {
      Bytes child = resolve_witness_ref(childref);
      return witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_3(child, child_prefix, updates, cursor);
    }
    struct TrieItem trie_branch_result_2_15 = trie_branch(child_prefix, childref);
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_branch_result_2_15, .tup1 = updates});
  }
  if (cursor == UINT8_C(64)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  uint64_t extension_nibble = path_nibble(child_prefix, cursor);
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool extension_pending = true;
  bool update_pending = update_under_current_prefix(remaining);
  while (extension_pending || update_pending) {
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      uint8_t update_index = (uint8_t)update_nibble;
      if (extension_pending && ((uint8_t)extension_nibble < update_index)) {
        struct TrieItem extension = trie_branch(child_prefix, childref);
        children = trie_children_add(children, prefix, extension_nibble, extension);
        extension_pending = false;
      } else {
        struct TriePath next_path = path_single(update_nibble);
        struct TriePath next_prefix = path_concat(prefix, next_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_extension = (bool)(extension_pending && (update_nibble == extension_nibble));
        struct tuple_TrieItem_TrieUpdateCursor result_2_18;
        if (consumes_extension) {
          result_2_18 = overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(childref, child_prefix, descended, next_prefix, next_cursor);
        } else {
          result_2_18 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, next_prefix, next_cursor);
        }
        children = trie_children_add(children, prefix, update_nibble, result_2_18.tup0);
        if (consumes_extension) {
          extension_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(result_2_18.tup1, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          uint64_t rebased_nibble = update_child_nibble(rebased);
          if ((uint8_t)rebased_nibble <= update_index) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      struct TrieItem extension_3_304 = trie_branch(child_prefix, childref);
      children = trie_children_add(children, prefix, extension_nibble, extension_3_304);
      extension_pending = false;
    }
  }
  struct TrieItem trie_children_finish_result_2_20 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_20, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct NodeRef childref, struct TriePath child_prefix, struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  bool child_below_prefix = path_prefix_of(prefix, child_prefix);
  if ((prefix_len != cursor) || !child_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  bool at_child_prefix = path_eq(prefix, child_prefix);
  if (at_child_prefix) {
    bool has_update = update_under_current_prefix(updates);
    if (has_update) {
      Bytes child = resolve_witness_ref(childref);
      return witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(child, child_prefix, updates, cursor);
    }
    struct TrieItem trie_branch_result_2_15 = trie_branch(child_prefix, childref);
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_branch_result_2_15, .tup1 = updates});
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  uint64_t extension_nibble = path_nibble_struct_TriePath_uint8_t_to_uint64_t(child_prefix, cursor);
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool extension_pending = true;
  bool update_pending = update_under_current_prefix(remaining);
  while (extension_pending || update_pending) {
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      uint8_t update_index = (uint8_t)update_nibble;
      if (extension_pending && ((uint8_t)extension_nibble < update_index)) {
        struct TrieItem extension = trie_branch(child_prefix, childref);
        children = trie_children_add(children, prefix, extension_nibble, extension);
        extension_pending = false;
      } else {
        struct TriePath next_path = path_single(update_nibble);
        struct TriePath next_prefix = path_concat(prefix, next_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_extension = (bool)(extension_pending && (update_nibble == extension_nibble));
        struct tuple_TrieItem_TrieUpdateCursor result_2_18;
        if (consumes_extension) {
          result_2_18 = overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(childref, child_prefix, descended, next_prefix, next_cursor);
        } else {
          result_2_18 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(descended, next_prefix, next_cursor);
        }
        children = trie_children_add(children, prefix, update_nibble, result_2_18.tup0);
        if (consumes_extension) {
          extension_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(result_2_18.tup1, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          uint64_t rebased_nibble = update_child_nibble(rebased);
          if ((uint8_t)rebased_nibble <= update_index) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      struct TrieItem extension_3_304 = trie_branch(child_prefix, childref);
      children = trie_children_add(children, prefix, extension_nibble, extension_3_304);
      extension_pending = false;
    }
  }
  struct TrieItem trie_children_finish_result_2_20 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_20, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct TrieUpdateCursor updates, struct TriePath prefix, struct TriePath key, Bytes value, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  bool key_below_prefix = path_prefix_of(prefix, key);
  if ((prefix_len != cursor) || !key_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  if (cursor == UINT8_C(64)) {
    bool key_matches_prefix = path_eq(prefix, key);
    if (!key_matches_prefix) {
      fatal_error(WitnessDeficient);
    }
    bool has_update = update_under_current_prefix(updates);
    if (has_update) {
      switch (updates.relation.kind) {
      case Kind_UpdateUnderPrefix:
      {
        uint8_t postfix_len = path_len(updates.relation.variants.UpdateUnderPrefix);
        if (postfix_len != UINT8_C(0)) {
          fatal_error(WitnessDeficient);
        }
        break;
      }
      case Kind_UpdateBeyondPrefix:
      case Kind_UpdateSourceExhausted:
        fatal_error(WitnessDeficient);
      }
      struct tuple_TrieUpdate_TrieUpdateCursor trie_updates_pop_result_2_27 = trie_updates_pop(updates);
      struct TrieUpdate update = trie_updates_pop_result_2_27.tup0;
      bool update_matches_key = path_eq(update.key, key);
      if (!update_matches_key) {
        fatal_error(WitnessDeficient);
      }
      struct TrieItem tmp_3_334;
      switch (update.change.kind) {
      case Kind_TrieDelete:
        tmp_3_334 = trie_empty_subtree();
        break;
      case Kind_TriePut:
        tmp_3_334 = trie_scratch_leaf(key, update.change.variants.TriePut);
        break;
      }
      return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = tmp_3_334, .tup1 = trie_updates_pop_result_2_27.tup1});
    }
    struct TrieItem trie_input_leaf_result_2_31 = trie_input_leaf(key, value);
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_input_leaf_result_2_31, .tup1 = updates});
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  uint64_t leaf_nibble = path_nibble(key, cursor);
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool leaf_pending = true;
  bool update_pending = update_under_current_prefix(remaining);
  while (leaf_pending || update_pending) {
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      uint8_t update_index = (uint8_t)update_nibble;
      if (leaf_pending && ((uint8_t)leaf_nibble < update_index)) {
        struct TrieItem leaf = trie_input_leaf(key, value);
        children = trie_children_add(children, prefix, leaf_nibble, leaf);
        leaf_pending = false;
      } else {
        struct TriePath child_path = path_single(update_nibble);
        struct TriePath child_prefix = path_concat(prefix, child_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_leaf = (bool)(leaf_pending && (update_nibble == leaf_nibble));
        struct tuple_TrieItem_TrieUpdateCursor result_2_33;
        if (consumes_leaf) {
          result_2_33 = overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, key, value, next_cursor);
        } else {
          result_2_33 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, next_cursor);
        }
        children = trie_children_add(children, prefix, update_nibble, result_2_33.tup0);
        if (consumes_leaf) {
          leaf_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(result_2_33.tup1, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          uint64_t rebased_nibble = update_child_nibble(rebased);
          if ((uint8_t)rebased_nibble <= update_index) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      struct TrieItem leaf_3_340 = trie_input_leaf(key, value);
      children = trie_children_add(children, prefix, leaf_nibble, leaf_3_340);
      leaf_pending = false;
    }
  }
  struct TrieItem trie_children_finish_result_2_35 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_35, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct TrieUpdateCursor updates, struct TriePath prefix, struct TriePath key, Bytes value, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  bool key_below_prefix = path_prefix_of(prefix, key);
  if ((prefix_len != cursor) || !key_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  uint64_t leaf_nibble = path_nibble_struct_TriePath_uint8_t_to_uint64_t(key, cursor);
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool leaf_pending = true;
  bool update_pending = update_under_current_prefix(remaining);
  while (leaf_pending || update_pending) {
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      uint8_t update_index = (uint8_t)update_nibble;
      if (leaf_pending && ((uint8_t)leaf_nibble < update_index)) {
        struct TrieItem leaf = trie_input_leaf(key, value);
        children = trie_children_add(children, prefix, leaf_nibble, leaf);
        leaf_pending = false;
      } else {
        struct TriePath child_path = path_single(update_nibble);
        struct TriePath child_prefix = path_concat(prefix, child_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_leaf = (bool)(leaf_pending && (update_nibble == leaf_nibble));
        struct tuple_TrieItem_TrieUpdateCursor result_2_33;
        if (consumes_leaf) {
          result_2_33 = overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, key, value, next_cursor);
        } else {
          result_2_33 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(descended, child_prefix, next_cursor);
        }
        children = trie_children_add(children, prefix, update_nibble, result_2_33.tup0);
        if (consumes_leaf) {
          leaf_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(result_2_33.tup1, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          uint64_t rebased_nibble = update_child_nibble(rebased);
          if ((uint8_t)rebased_nibble <= update_index) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      struct TrieItem leaf_3_340 = trie_input_leaf(key, value);
      children = trie_children_add(children, prefix, leaf_nibble, leaf_3_340);
      leaf_pending = false;
    }
  }
  struct TrieItem trie_children_finish_result_2_35 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_35, .tup1 = remaining});
}

struct NodeRef trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef(struct TrieItem it, uint8_t depth)
{
  struct TriePath suffix = path_drop_struct_TriePath_uint8_t_to_struct_TriePath(it.path, depth);
  uint8_t suffix_len = path_len(suffix);
  switch (it.value.kind) {
  case Kind_EmptySubtree:
    return EmptyRef(UNIT);
  case Kind_LeafItem:
    return leaf_child_ref(suffix, it.value.variants.LeafItem);
  case Kind_BranchItem:
    if (suffix_len == UINT8_C(0)) {
      return it.value.variants.BranchItem;
    }
    return merge_ext_ref(suffix, it.value.variants.BranchItem);
  case Kind_SubtreeItem:
  {
    if (suffix_len == UINT8_C(0)) {
      return it.value.variants.SubtreeItem;
    }
    switch (it.value.variants.SubtreeItem.kind) {
    case Kind_HashRef:
    {
      Bytes node = node_db_lookup(it.value.variants.SubtreeItem.variants.HashRef);
      if (node.len == UINT8_C(0)) {
        fatal_error(WitnessDeficient);
      }
      return merge_ext_node(suffix, node);
    }
    case Kind_EmptyRef:
    case Kind_InputInlineRef:
    case Kind_ScratchInlineRef:
      return merge_ext_ref(suffix, it.value.variants.SubtreeItem);
    }
  }
  }
}

struct NodeRef trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef_variant_2(struct TrieItem it, uint8_t depth)
{
  struct TriePath suffix = path_drop_struct_TriePath_uint8_t_to_struct_TriePath_variant_2(it.path, depth);
  uint8_t suffix_len = path_len(suffix);
  switch (it.value.kind) {
  case Kind_EmptySubtree:
    return EmptyRef(UNIT);
  case Kind_LeafItem:
    return leaf_child_ref(suffix, it.value.variants.LeafItem);
  case Kind_BranchItem:
    if (suffix_len == UINT8_C(0)) {
      return it.value.variants.BranchItem;
    }
    return merge_ext_ref(suffix, it.value.variants.BranchItem);
  case Kind_SubtreeItem:
  {
    if (suffix_len == UINT8_C(0)) {
      return it.value.variants.SubtreeItem;
    }
    switch (it.value.variants.SubtreeItem.kind) {
    case Kind_HashRef:
    {
      Bytes node = node_db_lookup(it.value.variants.SubtreeItem.variants.HashRef);
      if (node.len == UINT8_C(0)) {
        fatal_error(WitnessDeficient);
      }
      return merge_ext_node(suffix, node);
    }
    case Kind_EmptyRef:
    case Kind_InputInlineRef:
    case Kind_ScratchInlineRef:
      return merge_ext_ref(suffix, it.value.variants.SubtreeItem);
    }
  }
  }
}

struct tuple_TrieItem_TrieUpdateCursor updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  bool has_update = update_under_current_prefix(updates);
  if (!has_update) {
    struct TrieItem trie_empty_subtree_result_2_58 = trie_empty_subtree();
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_58, .tup1 = updates});
  }
  if (cursor == UINT8_C(64)) {
    switch (updates.relation.kind) {
    case Kind_UpdateUnderPrefix:
    {
      uint8_t postfix_len = path_len(updates.relation.variants.UpdateUnderPrefix);
      if (postfix_len != UINT8_C(0)) {
        fatal_error(WitnessDeficient);
      }
      break;
    }
    case Kind_UpdateBeyondPrefix:
    case Kind_UpdateSourceExhausted:
      fatal_error(WitnessDeficient);
    }
    struct tuple_TrieUpdate_TrieUpdateCursor trie_updates_pop_result_2_62 = trie_updates_pop(updates);
    struct TrieUpdate update = trie_updates_pop_result_2_62.tup0;
    bool update_matches_prefix = path_eq(update.key, prefix);
    if (!update_matches_prefix) {
      fatal_error(WitnessDeficient);
    }
    struct TrieItem tmp_3_378;
    switch (update.change.kind) {
    case Kind_TrieDelete:
      tmp_3_378 = trie_empty_subtree();
      break;
    case Kind_TriePut:
      tmp_3_378 = trie_scratch_leaf(update.key, update.change.variants.TriePut);
      break;
    }
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = tmp_3_378, .tup1 = trie_updates_pop_result_2_62.tup1});
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool update_pending = update_under_current_prefix(remaining);
  while (update_pending) {
    uint64_t nib = update_child_nibble(remaining);
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix = path_concat(prefix, child_path);
    struct TrieUpdateCursor descended = trie_updates_descend(remaining);
    struct tuple_TrieItem_TrieUpdateCursor updates_subtree_result_2_67 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, next_cursor);
    children = trie_children_add(children, prefix, nib, updates_subtree_result_2_67.tup0);
    struct TrieUpdateCursor rebased = trie_updates_rebase(updates_subtree_result_2_67.tup1, prefix);
    bool rebased_update_pending = update_under_current_prefix(rebased);
    if (rebased_update_pending) {
      uint64_t rebased_nibble = update_child_nibble(rebased);
      if ((uint8_t)rebased_nibble <= (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
    }
    remaining = rebased;
    update_pending = rebased_update_pending;
  }
  struct TrieItem trie_children_finish_result_2_69 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_69, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  bool has_update = update_under_current_prefix(updates);
  if (!has_update) {
    struct TrieItem trie_empty_subtree_result_2_58 = trie_empty_subtree();
    return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_58, .tup1 = updates});
  }
  uint8_t next_cursor = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren children = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  bool update_pending = update_under_current_prefix(remaining);
  while (update_pending) {
    uint64_t nib = update_child_nibble(remaining);
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix = path_concat(prefix, child_path);
    struct TrieUpdateCursor descended = trie_updates_descend(remaining);
    struct tuple_TrieItem_TrieUpdateCursor updates_subtree_result_2_67 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, next_cursor);
    children = trie_children_add(children, prefix, nib, updates_subtree_result_2_67.tup0);
    struct TrieUpdateCursor rebased = trie_updates_rebase(updates_subtree_result_2_67.tup1, prefix);
    bool rebased_update_pending = update_under_current_prefix(rebased);
    if (rebased_update_pending) {
      uint64_t rebased_nibble = update_child_nibble(rebased);
      if ((uint8_t)rebased_nibble <= (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
    }
    remaining = rebased;
    update_pending = rebased_update_pending;
  }
  struct TrieItem trie_children_finish_result_2_69 = trie_children_finish(prefix, children);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_69, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, cursor);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_411 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_411);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_411);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_413 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t tmp_3_390 = (int64_t)UINT8_C(15);
  int64_t tmp_3_391 = (int64_t)UINT8_C(1);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= tmp_3_390) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_414 = path_concat(prefix, child_path);
    struct NodeRef childref_3_415 = fast_vector_access_vector_16_NodeRef(children, i);
    bool tmp_3_392 = (bool)(childref_3_415.kind != Kind_EmptyRef);
    bool update_pending = update_under_current_prefix(remaining);
    bool update_here;
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      if ((uint8_t)update_nibble < (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
      update_here = (bool)(update_nibble == nib);
    } else {
      update_here = false;
    }
    struct tuple_TrieItem_TrieUpdateCursor result_2_80;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (tmp_3_392) {
        Bytes child = resolve_witness_ref(childref_3_415);
        result_2_80 = witness_subtree(child, child_prefix_3_414, descended, next_cursor_3_413);
      } else {
        result_2_80 = updates_subtree(descended, child_prefix_3_414, next_cursor_3_413);
      }
    } else if (tmp_3_392) {
      struct TrieItem trie_subtree_result_2_78 = trie_subtree(child_prefix_3_414, childref_3_415);
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_subtree_result_2_78, .tup1 = remaining});
    } else {
      struct TrieItem trie_empty_subtree_result_2_79 = trie_empty_subtree();
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_79, .tup1 = remaining});
    }
    built = trie_children_add(built, prefix, nib, result_2_80.tup0);
    if (update_here) {
      remaining = trie_updates_rebase(result_2_80.tup1, prefix);
    } else {
      remaining = result_2_80.tup1;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + tmp_3_391);
  }
  bool update_pending_3_417 = update_under_current_prefix(remaining);
  if (update_pending_3_417) {
    fatal_error(WitnessDeficient);
  }
  struct TrieItem trie_children_finish_result_2_82 = trie_children_finish(prefix, built);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_82, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(updates, prefix, cursor);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_411 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_411);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_411);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_413 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t tmp_3_390 = (int64_t)UINT8_C(15);
  int64_t tmp_3_391 = (int64_t)UINT8_C(1);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= tmp_3_390) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_414 = path_concat(prefix, child_path);
    struct NodeRef childref_3_415 = fast_vector_access_vector_16_NodeRef(children, i);
    bool tmp_3_392 = (bool)(childref_3_415.kind != Kind_EmptyRef);
    bool update_pending = update_under_current_prefix(remaining);
    bool update_here;
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      if ((uint8_t)update_nibble < (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
      update_here = (bool)(update_nibble == nib);
    } else {
      update_here = false;
    }
    struct tuple_TrieItem_TrieUpdateCursor result_2_80;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (tmp_3_392) {
        Bytes child = resolve_witness_ref(childref_3_415);
        result_2_80 = witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(child, child_prefix_3_414, descended, next_cursor_3_413);
      } else {
        result_2_80 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(descended, child_prefix_3_414, next_cursor_3_413);
      }
    } else if (tmp_3_392) {
      struct TrieItem trie_subtree_result_2_78 = trie_subtree(child_prefix_3_414, childref_3_415);
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_subtree_result_2_78, .tup1 = remaining});
    } else {
      struct TrieItem trie_empty_subtree_result_2_79 = trie_empty_subtree();
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_79, .tup1 = remaining});
    }
    built = trie_children_add(built, prefix, nib, result_2_80.tup0);
    if (update_here) {
      remaining = trie_updates_rebase(result_2_80.tup1, prefix);
    } else {
      remaining = result_2_80.tup1;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + tmp_3_391);
  }
  bool update_pending_3_417 = update_under_current_prefix(remaining);
  if (update_pending_3_417) {
    fatal_error(WitnessDeficient);
  }
  struct TrieItem trie_children_finish_result_2_82 = trie_children_finish(prefix, built);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_82, .tup1 = remaining});
}

struct tuple_TrieItem_TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_3(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor)
{
  uint8_t prefix_len = path_len(prefix);
  if (prefix_len != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, cursor);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_411 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_411);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_411);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_413 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t tmp_3_390 = (int64_t)UINT8_C(15);
  int64_t tmp_3_391 = (int64_t)UINT8_C(1);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= tmp_3_390) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_414 = path_concat(prefix, child_path);
    struct NodeRef childref_3_415 = fast_vector_access_vector_16_NodeRef(children, i);
    bool tmp_3_392 = (bool)(childref_3_415.kind != Kind_EmptyRef);
    bool update_pending = update_under_current_prefix(remaining);
    bool update_here;
    if (update_pending) {
      uint64_t update_nibble = update_child_nibble(remaining);
      if ((uint8_t)update_nibble < (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
      update_here = (bool)(update_nibble == nib);
    } else {
      update_here = false;
    }
    struct tuple_TrieItem_TrieUpdateCursor result_2_80;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (tmp_3_392) {
        Bytes child = resolve_witness_ref(childref_3_415);
        result_2_80 = witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(child, child_prefix_3_414, descended, next_cursor_3_413);
      } else {
        result_2_80 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix_3_414, next_cursor_3_413);
      }
    } else if (tmp_3_392) {
      struct TrieItem trie_subtree_result_2_78 = trie_subtree(child_prefix_3_414, childref_3_415);
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_subtree_result_2_78, .tup1 = remaining});
    } else {
      struct TrieItem trie_empty_subtree_result_2_79 = trie_empty_subtree();
      result_2_80 = ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_empty_subtree_result_2_79, .tup1 = remaining});
    }
    built = trie_children_add(built, prefix, nib, result_2_80.tup0);
    if (update_here) {
      remaining = trie_updates_rebase(result_2_80.tup1, prefix);
    } else {
      remaining = result_2_80.tup1;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + tmp_3_391);
  }
  bool update_pending_3_417 = update_under_current_prefix(remaining);
  if (update_pending_3_417) {
    fatal_error(WitnessDeficient);
  }
  struct TrieItem trie_children_finish_result_2_82 = trie_children_finish(prefix, built);
  return ((struct tuple_TrieItem_TrieUpdateCursor){.tup0 = trie_children_finish_result_2_82, .tup1 = remaining});
}

