#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct TrieUpdate EMPTY_TRIE_UPDATE;


void create_letbind_205(void) {

  EMPTY_TRIE_UPDATE = ((struct TrieUpdate){.change = (TrieDelete(UNIT)), .key = ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)})});
}
void kill_letbind_205(void) {
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
    return trie_ref_to_root((trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef_variant_2(subtree, UINT8_C(0))));
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
    if (!(path_prefix_of(child_prefix, child.path))) {
      fatal_error(WitnessDeficient);
    }
    if (branch_mask_has(children.mask, index)) {
      fatal_error(WitnessDeficient);
    }
    struct TrieChildren updated = children;
    updated.mask = branch_mask_set((updated.mask), index);
    updated.children = fast_unsigned_vector_update_vector_16_NodeRef(updated.children, (uint8_t)index, (trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef(child, child_depth)));
    updated.only = child;
    updated.count = next_child_count;
    return updated;
  }
  }
}

struct TrieItem trie_empty_subtree(void)
{
  return ((struct TrieItem){.path = ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)}), .value = (EmptySubtree(UNIT))});
}

struct TrieChildren trie_children_empty(void)
{
  struct NodeRef empty_ref = EmptyRef(UNIT);
  vector_16_NodeRef children = fast_unsigned_vector_init_vector_16_NodeRef(UINT8_C(16), empty_ref);
  return ((struct TrieChildren){.children = children, .count = UINT8_C(0), .mask = UINT64_C(0x0000), .only = (trie_empty_subtree())});
}

struct TrieItem trie_branch(struct TriePath path, struct NodeRef childref)
{
  return ((struct TrieItem){.path = path, .value = (BranchItem(childref))});
}

struct TrieItem trie_children_finish(struct TriePath prefix, struct TrieChildren children)
{
  if (children.count == UINT8_C(0)) {
    return ((struct TrieItem){.path = prefix, .value = (EmptySubtree(UNIT))});
  }
  if (children.count == UINT8_C(1)) {
    return children.only;
  }
  struct NodeRef branch_ref = branch_child_ref(children.mask, children.children);
  return trie_branch(prefix, branch_ref);
}

struct TrieItem trie_leaf(struct TriePath path, struct TrieLeafValue value)
{
  return ((struct TrieItem){.path = path, .value = (LeafItem(value))});
}

struct TrieItem trie_scratch_leaf(struct TriePath path, Bytes value)
{
  return trie_leaf(path, (ScratchTrieLeaf(value)));
}

struct TrieUpdateCursor trie_updates_descend(struct TrieUpdateCursor updates)
{
  switch (updates.relation.kind) {
  case Kind_UpdateUnderPrefix:
  {
    struct TriePath path_postfix = updates.relation.variants.UpdateUnderPrefix;
    if ((path_len(path_postfix)) == UINT8_C(0)) {
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
      return ((struct TrieUpdateCursor){.current = updates.current, .relation = (UpdateUnderPrefix(path_postfix)), .source = updates.source});
    }
    return updates;
  }
}

struct TrieItem trie_input_leaf(struct TriePath path, Bytes value)
{
  return trie_leaf(path, (InputTrieLeaf(value)));
}

struct TrieItem trie_subtree(struct TriePath path, struct NodeRef childref)
{
  return ((struct TrieItem){.path = path, .value = (SubtreeItem(childref))});
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
      struct TrieUpdate _8_1558_8_1797;
      bool _8_1559_8_1798 = account_trie_update(iterator_item.variants.AcctBlockIterRow, &_8_1558_8_1797);
      if (_8_1559_8_1798) {
        result = ((struct TrieUpdateFetch){.available = true, .update = _8_1558_8_1797});
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

struct TrieUpdateCursor trie_updates_pop(struct TrieUpdateCursor updates, struct TrieUpdate *restrict trieupdate_8_1556)
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
      next = ((struct TrieUpdateCursor){.current = successor.update, .relation = (UpdateBeyondPrefix(common_prefix_len)), .source = updates.source});
    } else {
      next = ((struct TrieUpdateCursor){.current = EMPTY_TRIE_UPDATE, .relation = (UpdateSourceExhausted(UNIT)), .source = updates.source});
    }
    (*trieupdate_8_1556) = current;
    return next;
  }
  }
}

struct TrieUpdateCursor updates_subtree(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor, struct TrieItem *restrict trieitem_8_1554)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (!(update_under_current_prefix(updates))) {
    (*trieitem_8_1554) = trie_empty_subtree();
    return updates;
  }
  if (cursor == UINT8_C(64)) {
    switch (updates.relation.kind) {
    case Kind_UpdateUnderPrefix:
    {
      if ((path_len(updates.relation.variants.UpdateUnderPrefix)) != UINT8_C(0)) {
        fatal_error(WitnessDeficient);
      }
      break;
    }
    case Kind_UpdateBeyondPrefix:
    case Kind_UpdateSourceExhausted:
      fatal_error(WitnessDeficient);
    }
    struct TrieUpdate update;
    struct TrieUpdateCursor next = trie_updates_pop(updates, &update);
    if (!(path_eq(update.key, prefix))) {
      fatal_error(WitnessDeficient);
    }
    switch (update.change.kind) {
    case Kind_TrieDelete:
      (*trieitem_8_1554) = trie_empty_subtree();
      return next;
    case Kind_TriePut:
      (*trieitem_8_1554) = trie_scratch_leaf(update.key, update.change.variants.TriePut);
      return next;
    }
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
    struct TrieItem _8_1554_8_1799;
    struct TrieUpdateCursor _8_1555_8_1800 = updates_subtree(descended, child_prefix, next_cursor, &_8_1554_8_1799);
    children = trie_children_add(children, prefix, nib, _8_1554_8_1799);
    struct TrieUpdateCursor rebased = trie_updates_rebase(_8_1555_8_1800, prefix);
    bool rebased_update_pending = update_under_current_prefix(rebased);
    if (rebased_update_pending) {
      if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
    }
    remaining = rebased;
    update_pending = rebased_update_pending;
  }
  (*trieitem_8_1554) = trie_children_finish(prefix, children);
  return remaining;
}

struct TrieUpdateCursor witness_subtree(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor, struct TrieItem *restrict trieitem_8_1552)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, cursor, trieitem_8_1552);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor, trieitem_8_1552);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_413 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_413);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_413);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor, trieitem_8_1552);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_415 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= (int64_t)UINT8_C(15)) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_416 = path_concat(prefix, child_path);
    struct NodeRef childref_3_417 = fast_vector_access_vector_16_NodeRef(children, i);
    bool present = false;
    if (childref_3_417.kind != Kind_EmptyRef) {
      present = true;
    }
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
    struct TrieItem trieitem_8_1878;
    struct TrieUpdateCursor trieupdatecursor_8_1879;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (present) {
        Bytes child = resolve_witness_ref(childref_3_417);
        trieupdatecursor_8_1879 = witness_subtree(child, child_prefix_3_416, descended, next_cursor_3_415, &trieitem_8_1878);
      } else {
        trieupdatecursor_8_1879 = updates_subtree(descended, child_prefix_3_416, next_cursor_3_415, &trieitem_8_1878);
      }
    } else if (present) {
      trieitem_8_1878 = trie_subtree(child_prefix_3_416, childref_3_417);
      trieupdatecursor_8_1879 = remaining;
    } else {
      trieitem_8_1878 = trie_empty_subtree();
      trieupdatecursor_8_1879 = remaining;
    }
    built = trie_children_add(built, prefix, nib, trieitem_8_1878);
    if (update_here) {
      remaining = trie_updates_rebase(trieupdatecursor_8_1879, prefix);
    } else {
      remaining = trieupdatecursor_8_1879;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + (int64_t)UINT8_C(1));
  }
  if (update_under_current_prefix(remaining)) {
    fatal_error(WitnessDeficient);
  }
  (*trieitem_8_1552) = trie_children_finish(prefix, built);
  return remaining;
}

struct TrieUpdateCursor overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct NodeRef childref, struct TriePath child_prefix, struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor, struct TrieItem *restrict trieitem_8_1593)
{
  uint8_t prefix_len = path_len(prefix);
  bool child_below_prefix = path_prefix_of(prefix, child_prefix);
  if ((prefix_len != cursor) || !child_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  if (path_eq(prefix, child_prefix)) {
    if (update_under_current_prefix(updates)) {
      Bytes child = resolve_witness_ref(childref);
      return witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_3(child, child_prefix, updates, cursor, trieitem_8_1593);
    }
    (*trieitem_8_1593) = trie_branch(child_prefix, childref);
    return updates;
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
      if (extension_pending && ((uint8_t)extension_nibble < (uint8_t)update_nibble)) {
        children = trie_children_add(children, prefix, extension_nibble, (trie_branch(child_prefix, childref)));
        extension_pending = false;
      } else {
        struct TriePath next_path = path_single(update_nibble);
        struct TriePath next_prefix = path_concat(prefix, next_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_extension = (bool)(extension_pending && (update_nibble == extension_nibble));
        struct TrieItem trieitem_8_1880;
        struct TrieUpdateCursor trieupdatecursor_8_1881;
        if (consumes_extension) {
          trieupdatecursor_8_1881 = overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(childref, child_prefix, descended, next_prefix, next_cursor, &trieitem_8_1880);
        } else {
          trieupdatecursor_8_1881 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, next_prefix, next_cursor, &trieitem_8_1880);
        }
        children = trie_children_add(children, prefix, update_nibble, trieitem_8_1880);
        if (consumes_extension) {
          extension_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(trieupdatecursor_8_1881, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)update_nibble) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      children = trie_children_add(children, prefix, extension_nibble, (trie_branch(child_prefix, childref)));
      extension_pending = false;
    }
  }
  (*trieitem_8_1593) = trie_children_finish(prefix, children);
  return remaining;
}

struct TrieUpdateCursor overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct NodeRef childref, struct TriePath child_prefix, struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor, struct TrieItem *restrict trieitem_8_1595)
{
  uint8_t prefix_len = path_len(prefix);
  bool child_below_prefix = path_prefix_of(prefix, child_prefix);
  if ((prefix_len != cursor) || !child_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  if (path_eq(prefix, child_prefix)) {
    if (update_under_current_prefix(updates)) {
      Bytes child = resolve_witness_ref(childref);
      return witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(child, child_prefix, updates, cursor, trieitem_8_1595);
    }
    (*trieitem_8_1595) = trie_branch(child_prefix, childref);
    return updates;
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
      if (extension_pending && ((uint8_t)extension_nibble < (uint8_t)update_nibble)) {
        children = trie_children_add(children, prefix, extension_nibble, (trie_branch(child_prefix, childref)));
        extension_pending = false;
      } else {
        struct TriePath next_path = path_single(update_nibble);
        struct TriePath next_prefix = path_concat(prefix, next_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_extension = (bool)(extension_pending && (update_nibble == extension_nibble));
        struct TrieItem trieitem_8_1882;
        struct TrieUpdateCursor trieupdatecursor_8_1883;
        if (consumes_extension) {
          trieupdatecursor_8_1883 = overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(childref, child_prefix, descended, next_prefix, next_cursor, &trieitem_8_1882);
        } else {
          trieupdatecursor_8_1883 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(descended, next_prefix, next_cursor, &trieitem_8_1882);
        }
        children = trie_children_add(children, prefix, update_nibble, trieitem_8_1882);
        if (consumes_extension) {
          extension_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(trieupdatecursor_8_1883, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)update_nibble) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      children = trie_children_add(children, prefix, extension_nibble, (trie_branch(child_prefix, childref)));
      extension_pending = false;
    }
  }
  (*trieitem_8_1595) = trie_children_finish(prefix, children);
  return remaining;
}

struct TrieUpdateCursor overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct TrieUpdateCursor updates, struct TriePath prefix, struct TriePath key, Bytes value, uint8_t cursor, struct TrieItem *restrict trieitem_8_1597)
{
  uint8_t prefix_len = path_len(prefix);
  bool key_below_prefix = path_prefix_of(prefix, key);
  if ((prefix_len != cursor) || !key_below_prefix) {
    fatal_error(WitnessDeficient);
  }
  if (cursor == UINT8_C(64)) {
    if (!(path_eq(prefix, key))) {
      fatal_error(WitnessDeficient);
    }
    if (update_under_current_prefix(updates)) {
      switch (updates.relation.kind) {
      case Kind_UpdateUnderPrefix:
      {
        if ((path_len(updates.relation.variants.UpdateUnderPrefix)) != UINT8_C(0)) {
          fatal_error(WitnessDeficient);
        }
        break;
      }
      case Kind_UpdateBeyondPrefix:
      case Kind_UpdateSourceExhausted:
        fatal_error(WitnessDeficient);
      }
      struct TrieUpdate update;
      struct TrieUpdateCursor next = trie_updates_pop(updates, &update);
      if (!(path_eq(update.key, key))) {
        fatal_error(WitnessDeficient);
      }
      switch (update.change.kind) {
      case Kind_TrieDelete:
        (*trieitem_8_1597) = trie_empty_subtree();
        return next;
      case Kind_TriePut:
        (*trieitem_8_1597) = trie_scratch_leaf(key, update.change.variants.TriePut);
        return next;
      }
    }
    (*trieitem_8_1597) = trie_input_leaf(key, value);
    return updates;
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
      if (leaf_pending && ((uint8_t)leaf_nibble < (uint8_t)update_nibble)) {
        children = trie_children_add(children, prefix, leaf_nibble, (trie_input_leaf(key, value)));
        leaf_pending = false;
      } else {
        struct TriePath child_path = path_single(update_nibble);
        struct TriePath child_prefix = path_concat(prefix, child_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_leaf = (bool)(leaf_pending && (update_nibble == leaf_nibble));
        struct TrieItem trieitem_8_1884;
        struct TrieUpdateCursor trieupdatecursor_8_1885;
        if (consumes_leaf) {
          trieupdatecursor_8_1885 = overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, key, value, next_cursor, &trieitem_8_1884);
        } else {
          trieupdatecursor_8_1885 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, next_cursor, &trieitem_8_1884);
        }
        children = trie_children_add(children, prefix, update_nibble, trieitem_8_1884);
        if (consumes_leaf) {
          leaf_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(trieupdatecursor_8_1885, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)update_nibble) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      children = trie_children_add(children, prefix, leaf_nibble, (trie_input_leaf(key, value)));
      leaf_pending = false;
    }
  }
  (*trieitem_8_1597) = trie_children_finish(prefix, children);
  return remaining;
}

struct TrieUpdateCursor overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct TrieUpdateCursor updates, struct TriePath prefix, struct TriePath key, Bytes value, uint8_t cursor, struct TrieItem *restrict trieitem_8_1599)
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
      if (leaf_pending && ((uint8_t)leaf_nibble < (uint8_t)update_nibble)) {
        children = trie_children_add(children, prefix, leaf_nibble, (trie_input_leaf(key, value)));
        leaf_pending = false;
      } else {
        struct TriePath child_path = path_single(update_nibble);
        struct TriePath child_prefix = path_concat(prefix, child_path);
        struct TrieUpdateCursor descended = trie_updates_descend(remaining);
        bool consumes_leaf = (bool)(leaf_pending && (update_nibble == leaf_nibble));
        struct TrieItem trieitem_8_1886;
        struct TrieUpdateCursor trieupdatecursor_8_1887;
        if (consumes_leaf) {
          trieupdatecursor_8_1887 = overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, key, value, next_cursor, &trieitem_8_1886);
        } else {
          trieupdatecursor_8_1887 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(descended, child_prefix, next_cursor, &trieitem_8_1886);
        }
        children = trie_children_add(children, prefix, update_nibble, trieitem_8_1886);
        if (consumes_leaf) {
          leaf_pending = false;
        }
        struct TrieUpdateCursor rebased = trie_updates_rebase(trieupdatecursor_8_1887, prefix);
        bool rebased_update_pending = update_under_current_prefix(rebased);
        if (rebased_update_pending) {
          if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)update_nibble) {
            fatal_error(WitnessDeficient);
          }
        }
        remaining = rebased;
        update_pending = rebased_update_pending;
      }
    } else {
      children = trie_children_add(children, prefix, leaf_nibble, (trie_input_leaf(key, value)));
      leaf_pending = false;
    }
  }
  (*trieitem_8_1599) = trie_children_finish(prefix, children);
  return remaining;
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

struct TrieUpdateCursor updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor, struct TrieItem *restrict trieitem_8_1618)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (!(update_under_current_prefix(updates))) {
    (*trieitem_8_1618) = trie_empty_subtree();
    return updates;
  }
  if (cursor == UINT8_C(64)) {
    switch (updates.relation.kind) {
    case Kind_UpdateUnderPrefix:
    {
      if ((path_len(updates.relation.variants.UpdateUnderPrefix)) != UINT8_C(0)) {
        fatal_error(WitnessDeficient);
      }
      break;
    }
    case Kind_UpdateBeyondPrefix:
    case Kind_UpdateSourceExhausted:
      fatal_error(WitnessDeficient);
    }
    struct TrieUpdate update;
    struct TrieUpdateCursor next = trie_updates_pop(updates, &update);
    if (!(path_eq(update.key, prefix))) {
      fatal_error(WitnessDeficient);
    }
    switch (update.change.kind) {
    case Kind_TrieDelete:
      (*trieitem_8_1618) = trie_empty_subtree();
      return next;
    case Kind_TriePut:
      (*trieitem_8_1618) = trie_scratch_leaf(update.key, update.change.variants.TriePut);
      return next;
    }
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
    struct TrieItem _8_1618_8_1859;
    struct TrieUpdateCursor _8_1619_8_1860 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, next_cursor, &_8_1618_8_1859);
    children = trie_children_add(children, prefix, nib, _8_1618_8_1859);
    struct TrieUpdateCursor rebased = trie_updates_rebase(_8_1619_8_1860, prefix);
    bool rebased_update_pending = update_under_current_prefix(rebased);
    if (rebased_update_pending) {
      if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
    }
    remaining = rebased;
    update_pending = rebased_update_pending;
  }
  (*trieitem_8_1618) = trie_children_finish(prefix, children);
  return remaining;
}

struct TrieUpdateCursor updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor, struct TrieItem *restrict trieitem_8_1620)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (!(update_under_current_prefix(updates))) {
    (*trieitem_8_1620) = trie_empty_subtree();
    return updates;
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
    struct TrieItem _8_1618_8_1861;
    struct TrieUpdateCursor _8_1619_8_1862 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix, next_cursor, &_8_1618_8_1861);
    children = trie_children_add(children, prefix, nib, _8_1618_8_1861);
    struct TrieUpdateCursor rebased = trie_updates_rebase(_8_1619_8_1862, prefix);
    bool rebased_update_pending = update_under_current_prefix(rebased);
    if (rebased_update_pending) {
      if ((uint8_t)(update_child_nibble(rebased)) <= (uint8_t)nib) {
        fatal_error(WitnessDeficient);
      }
    }
    remaining = rebased;
    update_pending = rebased_update_pending;
  }
  (*trieitem_8_1620) = trie_children_finish(prefix, children);
  return remaining;
}

struct TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor, struct TrieItem *restrict trieitem_8_1622)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, cursor, trieitem_8_1622);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor, trieitem_8_1622);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_413 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_413);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_413);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor, trieitem_8_1622);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_415 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= (int64_t)UINT8_C(15)) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_416 = path_concat(prefix, child_path);
    struct NodeRef childref_3_417 = fast_vector_access_vector_16_NodeRef(children, i);
    bool present = false;
    if (childref_3_417.kind != Kind_EmptyRef) {
      present = true;
    }
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
    struct TrieItem trieitem_8_1892;
    struct TrieUpdateCursor trieupdatecursor_8_1893;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (present) {
        Bytes child = resolve_witness_ref(childref_3_417);
        trieupdatecursor_8_1893 = witness_subtree(child, child_prefix_3_416, descended, next_cursor_3_415, &trieitem_8_1892);
      } else {
        trieupdatecursor_8_1893 = updates_subtree(descended, child_prefix_3_416, next_cursor_3_415, &trieitem_8_1892);
      }
    } else if (present) {
      trieitem_8_1892 = trie_subtree(child_prefix_3_416, childref_3_417);
      trieupdatecursor_8_1893 = remaining;
    } else {
      trieitem_8_1892 = trie_empty_subtree();
      trieupdatecursor_8_1893 = remaining;
    }
    built = trie_children_add(built, prefix, nib, trieitem_8_1892);
    if (update_here) {
      remaining = trie_updates_rebase(trieupdatecursor_8_1893, prefix);
    } else {
      remaining = trieupdatecursor_8_1893;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + (int64_t)UINT8_C(1));
  }
  if (update_under_current_prefix(remaining)) {
    fatal_error(WitnessDeficient);
  }
  (*trieitem_8_1622) = trie_children_finish(prefix, built);
  return remaining;
}

struct TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor, struct TrieItem *restrict trieitem_8_1624)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(updates, prefix, cursor, trieitem_8_1624);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor, trieitem_8_1624);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_413 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_413);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_413);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor, trieitem_8_1624);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_415 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= (int64_t)UINT8_C(15)) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_416 = path_concat(prefix, child_path);
    struct NodeRef childref_3_417 = fast_vector_access_vector_16_NodeRef(children, i);
    bool present = false;
    if (childref_3_417.kind != Kind_EmptyRef) {
      present = true;
    }
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
    struct TrieItem trieitem_8_1894;
    struct TrieUpdateCursor trieupdatecursor_8_1895;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (present) {
        Bytes child = resolve_witness_ref(childref_3_417);
        trieupdatecursor_8_1895 = witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(child, child_prefix_3_416, descended, next_cursor_3_415, &trieitem_8_1894);
      } else {
        trieupdatecursor_8_1895 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(descended, child_prefix_3_416, next_cursor_3_415, &trieitem_8_1894);
      }
    } else if (present) {
      trieitem_8_1894 = trie_subtree(child_prefix_3_416, childref_3_417);
      trieupdatecursor_8_1895 = remaining;
    } else {
      trieitem_8_1894 = trie_empty_subtree();
      trieupdatecursor_8_1895 = remaining;
    }
    built = trie_children_add(built, prefix, nib, trieitem_8_1894);
    if (update_here) {
      remaining = trie_updates_rebase(trieupdatecursor_8_1895, prefix);
    } else {
      remaining = trieupdatecursor_8_1895;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + (int64_t)UINT8_C(1));
  }
  if (update_under_current_prefix(remaining)) {
    fatal_error(WitnessDeficient);
  }
  (*trieitem_8_1624) = trie_children_finish(prefix, built);
  return remaining;
}

struct TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_3(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor, struct TrieItem *restrict trieitem_8_1626)
{
  if ((path_len(prefix)) != cursor) {
    fatal_error(WitnessDeficient);
  }
  if (node.len == UINT8_C(0)) {
    return updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, cursor, trieitem_8_1626);
  }
  struct InputTrieNode decoded = decode_input_trie_node(node);
  if (!(decoded.kind != Kind_InputLeafNode)) {
    struct TriePath key = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(updates, prefix, key, decoded.variants.InputLeafNode.tup1, cursor, trieitem_8_1626);
  }
  if (!(decoded.kind != Kind_InputExtensionNode)) {
    struct TriePath path_3_413 = decoded.variants.InputExtensionNode.tup0;
    uint8_t extension_len = path_len(path_3_413);
    if ((extension_len == UINT8_C(0)) || (UINT8_C(64) < ((uint8_t)((uint32_t)extension_len + (uint32_t)cursor)))) {
      fatal_error(WitnessDeficient);
    }
    struct TriePath child_prefix = path_concat(prefix, path_3_413);
    return overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(decoded.variants.InputExtensionNode.tup1, child_prefix, updates, prefix, cursor, trieitem_8_1626);
  }
  /* complete */
  vector_16_NodeRef children = decoded.variants.InputBranchNode.tup0;
  if ((decoded.variants.InputBranchNode.tup1.len != UINT8_C(0)) || (UINT8_C(64) <= cursor)) {
    fatal_error(WitnessDeficient);
  }
  uint8_t next_cursor_3_415 = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)cursor));
  struct TrieChildren built = trie_children_empty();
  struct TrieUpdateCursor remaining = updates;
  uint64_t nib = UINT64_C(0x0);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= (int64_t)UINT8_C(15)) {
    struct TriePath child_path = path_single(nib);
    struct TriePath child_prefix_3_416 = path_concat(prefix, child_path);
    struct NodeRef childref_3_417 = fast_vector_access_vector_16_NodeRef(children, i);
    bool present = false;
    if (childref_3_417.kind != Kind_EmptyRef) {
      present = true;
    }
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
    struct TrieItem trieitem_8_1896;
    struct TrieUpdateCursor trieupdatecursor_8_1897;
    if (update_here) {
      struct TrieUpdateCursor descended = trie_updates_descend(remaining);
      if (present) {
        Bytes child = resolve_witness_ref(childref_3_417);
        trieupdatecursor_8_1897 = witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(child, child_prefix_3_416, descended, next_cursor_3_415, &trieitem_8_1896);
      } else {
        trieupdatecursor_8_1897 = updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(descended, child_prefix_3_416, next_cursor_3_415, &trieitem_8_1896);
      }
    } else if (present) {
      trieitem_8_1896 = trie_subtree(child_prefix_3_416, childref_3_417);
      trieupdatecursor_8_1897 = remaining;
    } else {
      trieitem_8_1896 = trie_empty_subtree();
      trieupdatecursor_8_1897 = remaining;
    }
    built = trie_children_add(built, prefix, nib, trieitem_8_1896);
    if (update_here) {
      remaining = trie_updates_rebase(trieupdatecursor_8_1897, prefix);
    } else {
      remaining = trieupdatecursor_8_1897;
    }
    nib = ((nib + UINT64_C(0x1)) & UINT64_C(0xF));
    i = (i + (int64_t)UINT8_C(1));
  }
  if (update_under_current_prefix(remaining)) {
    fatal_error(WitnessDeficient);
  }
  (*trieitem_8_1626) = trie_children_finish(prefix, built);
  return remaining;
}

