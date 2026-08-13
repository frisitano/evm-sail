#pragma once

#include "evmsail/spec/lib/mpt/indexed.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct TrieRootResult
struct TrieRootResult {
  bool changed;
  bytes32 root;
};

uint64_t update_child_nibble(struct TrieUpdateCursor updates);

bool update_under_current_prefix(struct TrieUpdateCursor updates);

uint8_t update_child_ranges_remaining(struct TrieUpdateCursor updates);

uint8_t overlay_child_ranges_remaining(struct TrieUpdateCursor updates, bool existing_pending, uint64_t existing_nibble);

struct tuple_TrieItem_TrieUpdateCursor witness_subtree(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor updates_subtree(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor);

struct TrieRootResult trie_root_cursor(bytes32 base_root, struct TrieUpdateCursor updates);

struct TrieRootResult trie_root(bytes32 base_root, struct TrieUpdateSource source);

struct tuple_TrieItem_TrieUpdateCursor overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct NodeRef childref, struct TriePath child_prefix, struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor overlay_extension_subtree_struct_NodeRef_struct_TriePath_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct NodeRef childref, struct TriePath child_prefix, struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct TrieUpdateCursor updates, struct TriePath prefix, struct TriePath key, Bytes value, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor overlay_leaf_subtree_struct_TrieUpdateCursor_struct_TriePath_struct_TriePath_Bytes_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct TrieUpdateCursor updates, struct TriePath prefix, struct TriePath key, Bytes value, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor updates_subtree_struct_TrieUpdateCursor_struct_TriePath_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(struct TrieUpdateCursor updates, struct TriePath prefix, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_2(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor);

struct tuple_TrieItem_TrieUpdateCursor witness_subtree_Bytes_struct_TriePath_struct_TrieUpdateCursor_uint8_t_to_struct_tuple_TrieItem_TrieUpdateCursor_variant_3(Bytes node, struct TriePath prefix, struct TrieUpdateCursor updates, uint8_t cursor);


#ifdef __cplusplus
}
#endif
