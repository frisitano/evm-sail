#pragma once

#include "evmsail/spec/lib/mpt/codec.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// union TrieUpdateSource
enum kind_TrieUpdateSource { Kind_ChangedAccountTrieUpdates, Kind_StorageTrieUpdates };

struct TrieUpdateSource {
  enum kind_TrieUpdateSource kind;
  union {
    struct { unit ChangedAccountTrieUpdates; };
    struct { bytes20 StorageTrieUpdates; };
  } variants;
};

// union TrieUpdateRelation
enum kind_TrieUpdateRelation { Kind_UpdateBeyondPrefix, Kind_UpdateSourceExhausted, Kind_UpdateUnderPrefix };

struct TrieUpdateRelation {
  enum kind_TrieUpdateRelation kind;
  union {
    struct { uint8_t UpdateBeyondPrefix; };
    struct { unit UpdateSourceExhausted; };
    struct { struct TriePath UpdateUnderPrefix; };
  } variants;
};

// union TrieChange
enum kind_TrieChange { Kind_TrieDelete, Kind_TriePut };

struct TrieChange {
  enum kind_TrieChange kind;
  union {
    struct { unit TrieDelete; };
    struct { Bytes TriePut; };
  } variants;
};

// struct TrieUpdate
struct TrieUpdate {
  struct TrieChange change;
  struct TriePath key;
};

// struct TrieUpdateFetch
struct TrieUpdateFetch {
  bool available;
  struct TrieUpdate update;
};

// struct TrieUpdateCursor
struct TrieUpdateCursor {
  struct TrieUpdate current;
  struct TrieUpdateRelation relation;
  struct TrieUpdateSource source;
};

// union TrieItemValue
enum kind_TrieItemValue { Kind_BranchItem, Kind_EmptySubtree, Kind_LeafItem, Kind_SubtreeItem };

struct TrieItemValue {
  enum kind_TrieItemValue kind;
  union {
    struct { struct NodeRef BranchItem; };
    struct { unit EmptySubtree; };
    struct { struct TrieLeafValue LeafItem; };
    struct { struct NodeRef SubtreeItem; };
  } variants;
};

// struct TrieItem
struct TrieItem {
  struct TriePath path;
  struct TrieItemValue value;
};

// struct TrieChildren
struct TrieChildren {
  vector_16_NodeRef children;
  uint8_t count;
  uint64_t mask;
  struct TrieItem only;
};

// struct tuple_TrieUpdate_TrieUpdateCursor
struct tuple_TrieUpdate_TrieUpdateCursor {
  struct TrieUpdate tup0;
  struct TrieUpdateCursor tup1;
};

// struct tuple_TrieItem_TrieUpdateCursor
struct tuple_TrieItem_TrieUpdateCursor {
  struct TrieItem tup0;
  struct TrieUpdateCursor tup1;
};

// struct tuple_TrieUpdate_bool
struct tuple_TrieUpdate_bool {
  struct TrieUpdate tup0;
  bool tup1;
};

void create_letbind_206(void);
void kill_letbind_206(void);


bytes32 trie_subtree_root(struct TrieItem subtree);

bool updates_empty(struct TrieUpdateCursor updates);

struct TrieChildren trie_children_add(struct TrieChildren children, struct TriePath prefix, uint64_t index, struct TrieItem child);

struct TrieItem trie_empty_subtree(void);

struct TrieChildren trie_children_empty(void);

struct TrieItem trie_branch(struct TriePath path, struct NodeRef childref);

struct TrieItem trie_children_finish(struct TriePath prefix, struct TrieChildren children);

struct TrieItem trie_leaf(struct TriePath path, struct TrieLeafValue value);

struct TrieItem trie_scratch_leaf(struct TriePath path, Bytes value);

struct TrieUpdateCursor trie_updates_descend(struct TrieUpdateCursor updates);

struct TrieUpdateCursor trie_updates_rebase(struct TrieUpdateCursor updates, struct TriePath prefix);

struct TrieItem trie_input_leaf(struct TriePath path, Bytes value);

struct TrieItem trie_subtree(struct TriePath path, struct NodeRef childref);

struct tuple_TrieUpdate_TrieUpdateCursor trie_updates_pop(struct TrieUpdateCursor updates);

struct TrieUpdateCursor trie_updates_begin(struct TrieUpdateSource source);

struct TrieUpdateFetch trie_update_source_next(struct TrieUpdateSource source);

struct NodeRef trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef(struct TrieItem it, uint8_t depth);

struct NodeRef trie_child_ref_struct_TrieItem_uint8_t_to_struct_NodeRef_variant_2(struct TrieItem it, uint8_t depth);

extern struct TrieUpdate EMPTY_TRIE_UPDATE;



#ifdef __cplusplus
}
#endif
