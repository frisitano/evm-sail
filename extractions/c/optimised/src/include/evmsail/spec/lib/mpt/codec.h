#pragma once

#include "evmsail/spec/lib/mpt/nodes.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_TriePath_NodeRef
struct tuple_TriePath_NodeRef {
  struct TriePath tup0;
  struct NodeRef tup1;
};

// struct tuple_TriePath_Bytes
struct tuple_TriePath_Bytes {
  struct TriePath tup0;
  Bytes tup1;
};

// struct tuple_vector_16_NodeRef_Bytes
struct tuple_vector_16_NodeRef_Bytes {
  vector_16_NodeRef tup0;
  Bytes tup1;
};

// union ScratchTrieNode
enum kind_ScratchTrieNode { Kind_ScratchBranchNode, Kind_ScratchExtensionNode, Kind_ScratchLeafNode };

struct ScratchTrieNode {
  enum kind_ScratchTrieNode kind;
  union {
    struct { struct tuple_vector_16_NodeRef_Bytes ScratchBranchNode; };
    struct { struct tuple_TriePath_NodeRef ScratchExtensionNode; };
    struct { struct tuple_TriePath_Bytes ScratchLeafNode; };
  } variants;
};

// struct tuple_TriePath_Bytes_1
struct tuple_TriePath_Bytes_1 {
  struct TriePath tup0;
  Bytes tup1;
};

// struct tuple_vector_16_NodeRef_Bytes_1
struct tuple_vector_16_NodeRef_Bytes_1 {
  vector_16_NodeRef tup0;
  Bytes tup1;
};

// union InputTrieNode
enum kind_InputTrieNode { Kind_InputBranchNode, Kind_InputExtensionNode, Kind_InputLeafNode };

struct InputTrieNode {
  enum kind_InputTrieNode kind;
  union {
    struct { struct tuple_vector_16_NodeRef_Bytes_1 InputBranchNode; };
    struct { struct tuple_TriePath_NodeRef InputExtensionNode; };
    struct { struct tuple_TriePath_Bytes_1 InputLeafNode; };
  } variants;
};

Bytes node_db_lookup(bytes32 h);

struct NodeRef input_field_to_ref(struct RlpFieldRef f);

struct InputTrieNode decode_input_trie_node(Bytes node);

Bytes resolve_witness_ref(struct NodeRef r);

uint8_t node_ref_size(struct NodeRef r);

void rlp_write_node_ref(struct NodeRef r);

uint8_t rlp_hex_prefix_size(struct TriePath path, bool is_leaf);

void rlp_write_hex_prefix(struct TriePath path, bool is_leaf);

struct NodeRef child_ref(Bytes encoded);

uint64_t branch_mask_for(uint64_t index);

bool branch_mask_has(uint64_t mask, uint64_t index);

uint64_t branch_mask_set(uint64_t mask, uint64_t index);

struct NodeRef input_leaf_child_ref(struct TriePath key, Bytes value);

struct NodeRef scratch_leaf_child_ref(struct TriePath key, Bytes value);

struct NodeRef leaf_child_ref(struct TriePath key, struct TrieLeafValue value);

struct NodeRef extension_child_ref(struct TriePath key, struct NodeRef childref);

struct NodeRef branch_child_ref(uint64_t mask, vector_16_NodeRef children);

bytes32 trie_ref_to_root(struct NodeRef r);

struct NodeRef input_node_to_ref(Bytes node);

struct NodeRef scratch_field_to_ref(struct ScratchRlpFieldRef f);

struct ScratchTrieNode decode_scratch_trie_node(Bytes node);

struct NodeRef merge_ext_node(struct TriePath prefix, Bytes childnode);

struct NodeRef merge_ext_ref(struct TriePath prefix, struct NodeRef childref);

struct InputTrieNode decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode(Bytes cursor, uint8_t index, vector_16_NodeRef children);

struct InputTrieNode decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode_variant_2(Bytes cursor, uint8_t index, vector_16_NodeRef children);

struct ScratchTrieNode decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode(Bytes cursor, uint8_t index, vector_16_NodeRef children);

struct ScratchTrieNode decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode_variant_2(Bytes cursor, uint8_t index, vector_16_NodeRef children);


#ifdef __cplusplus
}
#endif
