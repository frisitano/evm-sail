#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes node_db_lookup(bytes32 h)
{
  return nodedb_lookup(h);
}

struct NodeRef input_field_to_ref(struct RlpFieldRef f)
{
  if (f.is_list) {
    if (f.source.len < MPT_HASH_LENGTH) {
      return InputInlineRef(f.source);
    }
    fatal_error(RlpDecode);
  }
  if (f.content_len == MPT_HASH_LENGTH) {
    u256 word = rlp_decode_word(f);
    bytes32 hash = word_to_hash(word);
    return HashRef(hash);
  }
  return EmptyRef(UNIT);
}

struct InputTrieNode decode_input_trie_node(Bytes node)
{
  if (node.len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  Bytes fields = rlp_node_cursor(node);
  struct RlpFieldRef first = rlp_decode_item(fields);
  Bytes fields_1_8 = rlp_cursor_advance(fields, first.source.len);
  struct RlpFieldRef second = rlp_decode_item(fields_1_8);
  Bytes fields_1_9 = rlp_cursor_advance(fields_1_8, second.source.len);
  if (fields_1_9.len == UINT8_C(0)) {
    struct tuple_bool_TriePath hex_prefix_decode_ref_result_2_2118 = hex_prefix_decode_ref(first);
    if (hex_prefix_decode_ref_result_2_2118.tup0) {
      Bytes value = rlp_item_content(second);
      return InputLeafNode(((struct tuple_TriePath_Bytes_1){.tup0 = hex_prefix_decode_ref_result_2_2118.tup1, .tup1 = value}));
    }
    uint8_t path_length = path_len(hex_prefix_decode_ref_result_2_2118.tup1);
    if (path_length == UINT8_C(0)) {
      fatal_error(RlpDecode);
    }
    struct NodeRef child = input_field_to_ref(second);
    return InputExtensionNode(((struct tuple_TriePath_NodeRef){.tup0 = hex_prefix_decode_ref_result_2_2118.tup1, .tup1 = child}));
  }
  struct NodeRef empty_child = EmptyRef(UNIT);
  vector_16_NodeRef children = fast_unsigned_vector_init_vector_16_NodeRef(UINT8_C(16), empty_child);
  struct NodeRef input_field_to_ref_result_2_2120 = input_field_to_ref(first);
  children = fast_unsigned_vector_update_vector_16_NodeRef(children, UINT8_C(0), input_field_to_ref_result_2_2120);
  struct NodeRef input_field_to_ref_result_2_2121 = input_field_to_ref(second);
  children = fast_unsigned_vector_update_vector_16_NodeRef(children, UINT8_C(1), input_field_to_ref_result_2_2121);
  return decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode_variant_2(fields_1_9, UINT8_C(2), children);
}

Bytes resolve_witness_ref(struct NodeRef r)
{
  switch (r.kind) {
  case Kind_EmptyRef:
    return EMPTY_STATELESS_INPUT_SLICE;
  case Kind_InputInlineRef:
    return r.variants.InputInlineRef;
  case Kind_ScratchInlineRef:
    fatal_error(WitnessDeficient);
  case Kind_HashRef:
  {
    Bytes node_3_3169 = node_db_lookup(r.variants.HashRef);
    if (node_3_3169.len == UINT8_C(0)) {
      fatal_error(WitnessDeficient);
    }
    return node_3_3169;
  }
  }
}

uint8_t node_ref_size(struct NodeRef r)
{
  uint8_t tmp_3_991;
  if (r.kind != Kind_EmptyRef) {
    goto case_1489;
  }
  return UINT8_C(1);
case_1489: ;
  if (r.kind != Kind_InputInlineRef) {
    goto case_1488;
  }
  tmp_3_991 = (uint8_t)r.variants.InputInlineRef.len;
  goto finish_match_1485;
case_1488: ;
  if (!(r.kind != Kind_ScratchInlineRef)) {
    return r.variants.ScratchInlineRef.len;
  }
  /* complete */
  tmp_3_991 = UINT8_C(33);
finish_match_1485: ;
  return tmp_3_991;
}

void rlp_write_node_ref(struct NodeRef r)
{
  switch (r.kind) {
  case Kind_EmptyRef:
    scratch_push_byte(UINT64_C(0x80));
    return;
  case Kind_InputInlineRef:
    stateless_input_scratch_push_slice(r.variants.InputInlineRef);
    return;
  case Kind_ScratchInlineRef:
    scratch_push_b256(r.variants.ScratchInlineRef.data, r.variants.ScratchInlineRef.len);
    return;
  case Kind_HashRef:
  {
    u256 hash_word = hash_to_word(r.variants.HashRef);
    rlp_write_word(hash_word);
    return;
  }
  }
}

uint8_t rlp_hex_prefix_size(struct TriePath path, bool is_leaf)
{
  uint8_t encoded_length = hex_prefix_encoded_length(path);
  uint64_t first = hex_prefix_first_byte(path, is_leaf);
  if ((encoded_length == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0))) {
    return UINT8_C(1);
  }
  return ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)encoded_length));
}

void rlp_write_hex_prefix(struct TriePath path, bool is_leaf)
{
  uint8_t length_ = path_len(path);
  uint8_t encoded_length = hex_prefix_encoded_length(path);
  uint64_t first = hex_prefix_first_byte(path, is_leaf);
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(encoded_length, first);
  scratch_push_byte(first);
  uint8_t index = (length_ & UINT8_C(1)) != UINT8_C(0) ? UINT8_C(1) : UINT8_C(0);
  while (index < length_) {
    uint8_t current = ((uint8_t)((uint32_t)index % (uint32_t)UINT8_C(65)));
    uint8_t next = ((uint8_t)(((uint32_t)UINT8_C(1) + (uint32_t)current) % (uint32_t)UINT8_C(65)));
    uint64_t high = path_nibble(path, current);
    uint64_t low = path_nibble(path, next);
    scratch_push_byte((high << 4) | low);
    index = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)next));
  }
}

struct NodeRef child_ref(Bytes encoded)
{
  if (encoded.len < MPT_HASH_LENGTH) {
    struct InlineNode inline_node = inline_node_from_scratch_slice(encoded);
    return ScratchInlineRef(inline_node);
  }
  bytes32 node_hash = host_keccak_scratch(encoded);
  return HashRef(node_hash);
}

uint64_t branch_mask_for(uint64_t index)
{
  return ((UINT64_C(0x0001) << (uint8_t)index) & UINT64_C(0xFFFF));
}

bool branch_mask_has(uint64_t mask, uint64_t index)
{
  uint64_t index_mask = branch_mask_for(index);
  return (bool)((mask & index_mask) != UINT64_C(0x0000));
}

uint64_t branch_mask_set(uint64_t mask, uint64_t index)
{
  uint64_t index_mask = branch_mask_for(index);
  return (mask | index_mask);
}

struct NodeRef input_leaf_child_ref(struct TriePath key, Bytes value)
{
  uint8_t path_size = rlp_hex_prefix_size(key, true);
  uint32_t value_size = rlp_input_scratch_slice_size(value);
  uint32_t content_len = rlp_scratch_length_add_uint8_t_uint32_t_to_uint32_t(path_size, value_size);
  uint32_t encoded_size = rlp_scratch_list_size(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin(encoded_size);
  rlp_write_list_prefix_uint32_t_to_unit(content_len);
  rlp_write_hex_prefix(key, true);
  rlp_write_input_slice(value);
  Bytes encoded = rlp_encoder_finish(encoder);
  struct NodeRef result = child_ref(encoded);
  rlp_encoder_rewind(encoder);
  return result;
}

struct NodeRef scratch_leaf_child_ref(struct TriePath key, Bytes value)
{
  uint8_t path_size = rlp_hex_prefix_size(key, true);
  uint32_t value_size = rlp_scratch_scratch_slice_size(value);
  uint32_t content_len = rlp_scratch_length_add_uint8_t_uint32_t_to_uint32_t(path_size, value_size);
  uint32_t encoded_size = rlp_scratch_list_size(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin(encoded_size);
  rlp_write_list_prefix_uint32_t_to_unit(content_len);
  rlp_write_hex_prefix(key, true);
  rlp_write_scratch_slice(value);
  Bytes encoded = rlp_encoder_finish(encoder);
  struct NodeRef result = child_ref(encoded);
  rlp_encoder_rewind(encoder);
  return result;
}

struct NodeRef leaf_child_ref(struct TriePath key, struct TrieLeafValue value)
{
  switch (value.kind) {
  case Kind_InputTrieLeaf:
    return input_leaf_child_ref(key, value.variants.InputTrieLeaf);
  case Kind_ScratchTrieLeaf:
    return scratch_leaf_child_ref(key, value.variants.ScratchTrieLeaf);
  }
}

struct NodeRef extension_child_ref(struct TriePath key, struct NodeRef childref)
{
  uint8_t path_length = rlp_hex_prefix_size(key, false);
  uint8_t child_length = node_ref_size(childref);
  uint8_t content_len = ((uint8_t)((uint32_t)child_length + (uint32_t)path_length));
  uint8_t encoded_size = rlp_list_size_uint8_t_to_uint8_t_variant_2(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_size);
  rlp_write_list_prefix_uint8_t_to_unit_variant_2(content_len);
  rlp_write_hex_prefix(key, false);
  rlp_write_node_ref(childref);
  Bytes encoded = rlp_encoder_finish(encoder);
  struct NodeRef result = child_ref(encoded);
  rlp_encoder_rewind(encoder);
  return result;
}

struct NodeRef branch_child_ref(uint64_t mask, vector_16_NodeRef children)
{
  uint16_t content_length = UINT16_C(1);
  uint64_t child_bit = UINT64_C(0x0001);
  int64_t tmp_3_945 = (int64_t)UINT8_C(15);
  int64_t tmp_3_946 = (int64_t)UINT8_C(1);
  int64_t i = (int64_t)UINT8_C(0);
  while (i <= tmp_3_945) {
    uint64_t and_vec_result_2_569 = (mask & child_bit);
    if (and_vec_result_2_569 != UINT64_C(0x0000)) {
      uint8_t child_length;
      struct NodeRef plain_vector_access_result_2_568 = fast_vector_access_vector_16_NodeRef(children, i);
      child_length = node_ref_size(plain_vector_access_result_2_568);
      content_length = branch_content_length_add(content_length, child_length);
    } else {
      content_length = branch_content_length_add(content_length, UINT8_C(1));
    }
    child_bit = ((child_bit << UINT64_C(1)) & UINT64_C(0xFFFF));
    i = (i + tmp_3_946);
  }
  uint32_t scratch_content_length = rlp_scratch_length_add_uint16_t_uint8_t_to_uint32_t(content_length, UINT8_C(0));
  uint32_t encoded_size = rlp_scratch_list_size(scratch_content_length);
  struct RlpEncoder encoder = rlp_encoder_begin(encoded_size);
  rlp_write_list_prefix_uint16_t_to_unit(content_length);
  child_bit = UINT64_C(0x0001);
  int64_t tmp_3_951 = (int64_t)UINT8_C(15);
  int64_t tmp_3_952 = (int64_t)UINT8_C(1);
  int64_t i_3_960 = (int64_t)UINT8_C(0);
  while (i_3_960 <= tmp_3_951) {
    uint64_t and_vec_result_2_571 = (mask & child_bit);
    if (and_vec_result_2_571 != UINT64_C(0x0000)) {
      struct NodeRef plain_vector_access_result_2_570 = fast_vector_access_vector_16_NodeRef(children, i_3_960);
      rlp_write_node_ref(plain_vector_access_result_2_570);
    } else {
      scratch_push_byte(UINT64_C(0x80));
    }
    child_bit = ((child_bit << UINT64_C(1)) & UINT64_C(0xFFFF));
    i_3_960 = (i_3_960 + tmp_3_952);
  }
  scratch_push_byte(UINT64_C(0x80));
  Bytes encoded = rlp_encoder_finish(encoder);
  struct NodeRef result = child_ref(encoded);
  rlp_encoder_rewind(encoder);
  return result;
}

bytes32 trie_ref_to_root(struct NodeRef r)
{
  switch (r.kind) {
  case Kind_EmptyRef:
    return EMPTY_TRIE_ROOT;
  case Kind_InputInlineRef:
    return host_keccak_stateless_input(r.variants.InputInlineRef);
  case Kind_ScratchInlineRef:
    return inline_node_hash(r.variants.ScratchInlineRef);
  case Kind_HashRef:
    return r.variants.HashRef;
  }
}

struct NodeRef input_node_to_ref(Bytes node)
{
  if (node.len == UINT8_C(0)) {
    return EmptyRef(UNIT);
  }
  if (node.len < MPT_HASH_LENGTH) {
    return InputInlineRef(node);
  }
  bytes32 node_hash = host_keccak_stateless_input(node);
  return HashRef(node_hash);
}

struct NodeRef scratch_field_to_ref(struct ScratchRlpFieldRef f)
{
  if (f.is_list) {
    struct InlineNode inline_node = inline_node_from_scratch_slice(f.source);
    return ScratchInlineRef(inline_node);
  }
  if (f.content_len == MPT_HASH_LENGTH) {
    u256 word = scratch_rlp_decode_word(f);
    bytes32 hash = word_to_hash(word);
    return HashRef(hash);
  }
  return EmptyRef(UNIT);
}

struct ScratchTrieNode decode_scratch_trie_node(Bytes node)
{
  Bytes fields = scratch_rlp_node_cursor(node);
  struct ScratchRlpFieldRef first = scratch_rlp_decode_item(fields);
  Bytes fields_1_2 = scratch_rlp_cursor_advance(fields, first.source.len);
  struct ScratchRlpFieldRef second = scratch_rlp_decode_item(fields_1_2);
  Bytes fields_1_3 = scratch_rlp_cursor_advance(fields_1_2, second.source.len);
  if (fields_1_3.len == UINT8_C(0)) {
    struct tuple_bool_TriePath scratch_hex_prefix_decode_ref_result_2_545 = scratch_hex_prefix_decode_ref(first);
    if (scratch_hex_prefix_decode_ref_result_2_545.tup0) {
      Bytes value = scratch_rlp_item_content(second);
      return ScratchLeafNode(((struct tuple_TriePath_Bytes){.tup0 = scratch_hex_prefix_decode_ref_result_2_545.tup1, .tup1 = value}));
    }
    uint8_t path_length = path_len(scratch_hex_prefix_decode_ref_result_2_545.tup1);
    if (path_length == UINT8_C(0)) {
      fatal_error(RlpDecode);
    }
    struct NodeRef child = scratch_field_to_ref(second);
    return ScratchExtensionNode(((struct tuple_TriePath_NodeRef){.tup0 = scratch_hex_prefix_decode_ref_result_2_545.tup1, .tup1 = child}));
  }
  struct NodeRef empty_child = EmptyRef(UNIT);
  vector_16_NodeRef children = fast_unsigned_vector_init_vector_16_NodeRef(UINT8_C(16), empty_child);
  struct NodeRef scratch_field_to_ref_result_2_547 = scratch_field_to_ref(first);
  children = fast_unsigned_vector_update_vector_16_NodeRef(children, UINT8_C(0), scratch_field_to_ref_result_2_547);
  struct NodeRef scratch_field_to_ref_result_2_548 = scratch_field_to_ref(second);
  children = fast_unsigned_vector_update_vector_16_NodeRef(children, UINT8_C(1), scratch_field_to_ref_result_2_548);
  return decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode_variant_2(fields_1_3, UINT8_C(2), children);
}

struct NodeRef merge_ext_node(struct TriePath prefix, Bytes childnode)
{
  uint8_t prefix_length = path_len(prefix);
  if (prefix_length == UINT8_C(0)) {
    return input_node_to_ref(childnode);
  }
  if (childnode.len == UINT8_C(0)) {
    return EmptyRef(UNIT);
  }
  struct InputTrieNode decoded = decode_input_trie_node(childnode);
  switch (decoded.kind) {
  case Kind_InputLeafNode:
  {
    struct TriePath merged_path = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
    return input_leaf_child_ref(merged_path, decoded.variants.InputLeafNode.tup1);
  }
  case Kind_InputExtensionNode:
  {
    struct TriePath merged_path_3_927 = path_concat(prefix, decoded.variants.InputExtensionNode.tup0);
    return extension_child_ref(merged_path_3_927, decoded.variants.InputExtensionNode.tup1);
  }
  case Kind_InputBranchNode:
  {
    struct NodeRef childref = input_node_to_ref(childnode);
    return extension_child_ref(prefix, childref);
  }
  }
}

struct NodeRef merge_ext_ref(struct TriePath prefix, struct NodeRef childref)
{
  uint8_t prefix_length = path_len(prefix);
  if (prefix_length == UINT8_C(0)) {
    return childref;
  }
  switch (childref.kind) {
  case Kind_EmptyRef:
    return EmptyRef(UNIT);
  case Kind_HashRef:
    return extension_child_ref(prefix, childref);
  case Kind_InputInlineRef:
  {
    struct InputTrieNode decoded = decode_input_trie_node(childref.variants.InputInlineRef);
    switch (decoded.kind) {
    case Kind_InputLeafNode:
    {
      struct TriePath merged_path = path_concat(prefix, decoded.variants.InputLeafNode.tup0);
      return input_leaf_child_ref(merged_path, decoded.variants.InputLeafNode.tup1);
    }
    case Kind_InputExtensionNode:
    {
      struct TriePath merged_path_3_913 = path_concat(prefix, decoded.variants.InputExtensionNode.tup0);
      return extension_child_ref(merged_path_3_913, decoded.variants.InputExtensionNode.tup1);
    }
    case Kind_InputBranchNode:
      return extension_child_ref(prefix, childref);
    }
  }
  case Kind_ScratchInlineRef:
  {
    Bytes node_slice = inline_node_slice(childref.variants.ScratchInlineRef);
    struct ScratchTrieNode decoded_3_915 = decode_scratch_trie_node(node_slice);
    switch (decoded_3_915.kind) {
    case Kind_ScratchLeafNode:
    {
      struct TriePath merged_path_3_918 = path_concat(prefix, decoded_3_915.variants.ScratchLeafNode.tup0);
      return scratch_leaf_child_ref(merged_path_3_918, decoded_3_915.variants.ScratchLeafNode.tup1);
    }
    case Kind_ScratchExtensionNode:
    {
      struct TriePath merged_path_3_921 = path_concat(prefix, decoded_3_915.variants.ScratchExtensionNode.tup0);
      return extension_child_ref(merged_path_3_921, decoded_3_915.variants.ScratchExtensionNode.tup1);
    }
    case Kind_ScratchBranchNode:
      return extension_child_ref(prefix, childref);
    }
  }
  }
}

struct InputTrieNode decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode(Bytes cursor, uint8_t index, vector_16_NodeRef children)
{
  if (index < UINT8_C(16)) {
    struct RlpFieldRef child = rlp_decode_item(cursor);
    Bytes next = rlp_cursor_advance(cursor, child.source.len);
    vector_16_NodeRef updated = children;
    struct NodeRef input_field_to_ref_result_2_2142 = input_field_to_ref(child);
    updated = fast_unsigned_vector_update_vector_16_NodeRef(updated, index, input_field_to_ref_result_2_2142);
    return decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode(next, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)index)), updated);
  }
  struct RlpFieldRef value = rlp_decode_item(cursor);
  Bytes next_3_3196 = rlp_cursor_advance(cursor, value.source.len);
  rlp_cursor_expect_end(next_3_3196);
  Bytes content = rlp_item_content(value);
  return InputBranchNode(((struct tuple_vector_16_NodeRef_Bytes_1){.tup0 = children, .tup1 = content}));
}

struct InputTrieNode decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode_variant_2(Bytes cursor, uint8_t index, vector_16_NodeRef children)
{
  struct RlpFieldRef child = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, child.source.len);
  vector_16_NodeRef updated = children;
  struct NodeRef input_field_to_ref_result_2_2142 = input_field_to_ref(child);
  updated = fast_unsigned_vector_update_vector_16_NodeRef(updated, index, input_field_to_ref_result_2_2142);
  return decode_input_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_InputTrieNode(next, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)index)), updated);
}

struct ScratchTrieNode decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode(Bytes cursor, uint8_t index, vector_16_NodeRef children)
{
  if (index < UINT8_C(16)) {
    struct ScratchRlpFieldRef child = scratch_rlp_decode_item(cursor);
    Bytes next = scratch_rlp_cursor_advance(cursor, child.source.len);
    vector_16_NodeRef updated = children;
    struct NodeRef scratch_field_to_ref_result_2_554 = scratch_field_to_ref(child);
    updated = fast_unsigned_vector_update_vector_16_NodeRef(updated, index, scratch_field_to_ref_result_2_554);
    return decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode(next, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)index)), updated);
  }
  struct ScratchRlpFieldRef value = scratch_rlp_decode_item(cursor);
  Bytes next_3_937 = scratch_rlp_cursor_advance(cursor, value.source.len);
  scratch_rlp_cursor_expect_end(next_3_937);
  Bytes content = scratch_rlp_item_content(value);
  return ScratchBranchNode(((struct tuple_vector_16_NodeRef_Bytes){.tup0 = children, .tup1 = content}));
}

struct ScratchTrieNode decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode_variant_2(Bytes cursor, uint8_t index, vector_16_NodeRef children)
{
  struct ScratchRlpFieldRef child = scratch_rlp_decode_item(cursor);
  Bytes next = scratch_rlp_cursor_advance(cursor, child.source.len);
  vector_16_NodeRef updated = children;
  struct NodeRef scratch_field_to_ref_result_2_554 = scratch_field_to_ref(child);
  updated = fast_unsigned_vector_update_vector_16_NodeRef(updated, index, scratch_field_to_ref_result_2_554);
  return decode_scratch_branch_node_Bytes_uint8_t_vector_16_NodeRef_to_struct_ScratchTrieNode(next, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)index)), updated);
}

