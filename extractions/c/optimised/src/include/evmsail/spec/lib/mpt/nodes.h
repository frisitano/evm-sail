#pragma once

#include "evmsail/spec/lib/mpt/primitives.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// union TrieLeafValue
enum kind_TrieLeafValue { Kind_InputTrieLeaf, Kind_ScratchTrieLeaf };

struct TrieLeafValue {
  enum kind_TrieLeafValue kind;
  union {
    struct { Bytes InputTrieLeaf; };
    struct { Bytes ScratchTrieLeaf; };
  } variants;
};

// struct InlineNode
struct InlineNode {
  bytes32 data;
  uint8_t len;
};

// union NodeRef
enum kind_NodeRef { Kind_EmptyRef, Kind_HashRef, Kind_InputInlineRef, Kind_ScratchInlineRef };

struct NodeRef {
  enum kind_NodeRef kind;
  union {
    struct { unit EmptyRef; };
    struct { bytes32 HashRef; };
    struct { Bytes InputInlineRef; };
    struct { struct InlineNode ScratchInlineRef; };
  } variants;
};

#ifndef SAIL_FIXED_VECTOR_VECTOR_16_NODEREF_DEFINED
#define SAIL_FIXED_VECTOR_VECTOR_16_NODEREF_DEFINED
typedef struct vector_16_NodeRef {
  size_t len;
  struct NodeRef data[16];
} vector_16_NodeRef;
#endif

struct InlineNode inline_node_from_scratch_slice(Bytes bytes);

Bytes inline_node_slice(struct InlineNode node);

bytes32 inline_node_hash(struct InlineNode node);

uint16_t branch_content_length_add(uint16_t current, uint8_t addition);

extern const uint8_t MPT_HASH_LENGTH;



#ifdef __cplusplus
}
#endif
