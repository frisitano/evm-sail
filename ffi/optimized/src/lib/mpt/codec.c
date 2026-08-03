/* Node RLP codec implementation. See codec.h. */
#include "lib/mpt/codec.h"

#include <string.h>

/* ======================================================================== */
/* NODE RLP                                                                 */
/* ======================================================================== */

static bool mpt_decode_length(const uint8_t *data,
                              size_t len, size_t width, size_t *value) {
  if (width == 0 || width > sizeof(size_t) || width > len || data[0] == 0)
    return mpt_fail(MPT_RLP_DECODE);
  size_t result = 0;
  for (size_t i = 0; i < width; ++i) {
    if (result > (SIZE_MAX >> 8))
      return mpt_fail(MPT_RLP_DECODE);
    result = (result << 8) | data[i];
  }
  *value = result;
  return true;
}

bool mpt_rlp_pop(ByteSpan *cursor,
                 RlpItem *item) {
  if (cursor->len == 0)
    return mpt_fail(MPT_RLP_DECODE);
  const uint8_t first = cursor->data[0];
  size_t header = 0, content = 0;
  bool list = false;
  if (first <= 0x7f) {
    header = 0;
    content = 1;
  } else if (first <= 0xb7) {
    header = 1;
    content = first - 0x80;
    if (content == 1 && cursor->len > 1 && cursor->data[1] < 0x80)
      return mpt_fail(MPT_RLP_DECODE);
  } else if (first <= 0xbf) {
    const size_t width = first - 0xb7;
    if (cursor->len < 1 + width ||
        !mpt_decode_length(cursor->data + 1, cursor->len - 1, width,
                           &content))
      return false;
    if (content <= 55)
      return mpt_fail(MPT_RLP_DECODE);
    header = 1 + width;
  } else if (first <= 0xf7) {
    list = true;
    header = 1;
    content = first - 0xc0;
  } else {
    list = true;
    const size_t width = first - 0xf7;
    if (cursor->len < 1 + width ||
        !mpt_decode_length(cursor->data + 1, cursor->len - 1, width,
                           &content))
      return false;
    if (content <= 55)
      return mpt_fail(MPT_RLP_DECODE);
    header = 1 + width;
  }
  if (header > cursor->len || content > cursor->len - header)
    return mpt_fail(MPT_RLP_DECODE);
  item->raw.data = cursor->data;
  item->raw.len = header + content;
  item->content.data = cursor->data + header;
  item->content.len = content;
  item->list = list;
  cursor->data += item->raw.len;
  cursor->len -= item->raw.len;
  return true;
}

/*
 * State and storage witness nodes are the canonical output of the preceding
 * valid state transition. The optimized guest authenticates their exact
 * encoded bytes back to the supplied parent root, so it does not need to
 * repeat RLP's minimal-encoding checks while consuming the original-node
 * stream.
 * Span and length checks remain mandatory because the witness is still
 * untrusted memory until that authentication completes.
 */
bool mpt_rlp_pop_trusted_node(ByteSpan *cursor, RlpItem *item) {
  if (cursor->len == 0)
    return mpt_fail(MPT_RLP_DECODE);
  const uint8_t first = cursor->data[0];
  size_t header = 0, content = 0;
  bool list = false;
  if (first <= 0x7f) {
    content = 1;
  } else if (first <= 0xb7) {
    header = 1;
    content = first - 0x80;
  } else if (first <= 0xbf) {
    const size_t width = first - 0xb7;
    if (width > sizeof(size_t) || cursor->len < 1 + width)
      return mpt_fail(MPT_RLP_DECODE);
    for (size_t index = 0; index < width; ++index) {
      if (content > (SIZE_MAX >> 8))
        return mpt_fail(MPT_RLP_DECODE);
      content = (content << 8) | cursor->data[1 + index];
    }
    header = 1 + width;
  } else if (first <= 0xf7) {
    list = true;
    header = 1;
    content = first - 0xc0;
  } else {
    list = true;
    const size_t width = first - 0xf7;
    if (width > sizeof(size_t) || cursor->len < 1 + width)
      return mpt_fail(MPT_RLP_DECODE);
    for (size_t index = 0; index < width; ++index) {
      if (content > (SIZE_MAX >> 8))
        return mpt_fail(MPT_RLP_DECODE);
      content = (content << 8) | cursor->data[1 + index];
    }
    header = 1 + width;
  }
  if (header > cursor->len || content > cursor->len - header)
    return mpt_fail(MPT_RLP_DECODE);
  item->raw.data = cursor->data;
  item->raw.len = header + content;
  item->content.data = cursor->data + header;
  item->content.len = content;
  item->list = list;
  cursor->data += item->raw.len;
  cursor->len -= item->raw.len;
  return true;
}

bool mpt_decode_compact(const RlpItem *field,
                        bool *leaf, NibblePath *path) {
  if (field->list)
    return mpt_fail(MPT_RLP_DECODE);
  *path = nibble_path_empty();
  if (field->content.len == 0) {
    *leaf = false;
    return true;
  }
  if (field->content.len > 33)
    return mpt_fail(MPT_RLP_DECODE);
  const uint8_t first = field->content.data[0];
  const uint8_t flag = first >> 4;
  *leaf = (flag & 2) != 0;
  const bool odd = (flag & 1) != 0;
  if (odd)
    path->nibbles[path->len++] = first & 0x0f;
  for (size_t i = 1; i < field->content.len; ++i) {
    if (path->len > 62)
      return mpt_fail(MPT_WITNESS_DEFICIENT);
    path->nibbles[path->len++] = field->content.data[i] >> 4;
    path->nibbles[path->len++] = field->content.data[i] & 0x0f;
  }
  return true;
}

/* ======================================================================== */
/* CHILD REFERENCES AND DECODED NODES                                       */
/* ======================================================================== */

WitnessChild mpt_empty_witness_child(NodeId *node_id) {
  *node_id = MPT_NODE_ID_EMPTY;
  return (WitnessChild){
      .encoded = NULL,
      .node_id = node_id,
      .encoded_len = 0,
  };
}

WitnessChild mpt_hashed_witness_child(const uint8_t *digest,
                                              NodeId *node_id) {
  WitnessChild ref = {
      .encoded = digest,
      .node_id = node_id,
      .encoded_len = 32,
  };
  return ref;
}

WitnessChild mpt_inline_witness_child(const uint8_t *encoded,
                                              size_t len, NodeId *node_id) {
  WitnessChild ref = {
      .encoded = encoded,
      .node_id = node_id,
      .encoded_len = (uint8_t)len,
  };
  return ref;
}

NodeReferenceKind
mpt_witness_child_kind(const WitnessChild *ref) {
  if (ref->encoded == NULL)
    return NODE_REFERENCE_EMPTY;
  return ref->encoded_len == 32 ? NODE_REFERENCE_HASHED : NODE_REFERENCE_INLINE;
}

bool mpt_witness_child_valid(const WitnessChild *ref) {
  if (ref->node_id == NULL)
    return false;
  if (ref->encoded == NULL)
    return ref->encoded_len == 0 && *ref->node_id == MPT_NODE_ID_EMPTY;
  if (ref->encoded_len == 0 || ref->encoded_len > 32 ||
      *ref->node_id == MPT_NODE_ID_EMPTY)
    return false;
  return true;
}

static bool mpt_decode_child_reference(const RlpItem *field,
                                       const uint8_t **encoded,
                                       uint8_t *encoded_len,
                                       NodeId *node_id) {
  WitnessChild child;
  if (field->list) {
    if (field->raw.len >= 32)
      return mpt_fail(MPT_WITNESS_DEFICIENT);
    child = mpt_inline_witness_child(field->raw.data, field->raw.len, node_id);
  } else if (field->content.len == 32) {
    child = mpt_hashed_witness_child(field->content.data, node_id);
  } else if (field->content.len == 0) {
    child = mpt_empty_witness_child(node_id);
  } else {
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  }
  *encoded = child.encoded;
  *encoded_len = child.encoded_len;
  return true;
}

WitnessChild mpt_decoded_child(DecodedNode *node,
                                       unsigned nibble) {
  return (WitnessChild){
      .encoded = node->child_references[nibble],
      .node_id = &node->child_ids[nibble],
      .encoded_len = node->child_reference_lengths[nibble],
  };
}

static bool mpt_decode_node_impl(ByteSpan encoded,
                                 DecodedNode *node, bool trusted) {
  RlpItem outer;
  ByteSpan root = encoded;
  if (!(trusted ? mpt_rlp_pop_trusted_node(&root, &outer)
                : mpt_rlp_pop(&root, &outer)) ||
      root.len != 0 || !outer.list)
    return mpt_fail(MPT_RLP_DECODE);
  ByteSpan fields = outer.content;
  RlpItem item[17];
  unsigned count = 0;
  while (fields.len && count < 17) {
    const bool popped =
        trusted ? mpt_rlp_pop_trusted_node(&fields, &item[count])
                : mpt_rlp_pop(&fields, &item[count]);
    if (!popped)
      return false;
    ++count;
  }
  if (fields.len != 0)
    return mpt_fail(MPT_RLP_DECODE);
  memset(node, 0, sizeof(*node));
  if (count == 2) {
    bool leaf;
    if (!mpt_decode_compact(&item[0], &leaf, &node->path))
      return false;
    if (leaf) {
      node->kind = DECODED_NODE_LEAF;
      node->value = item[1].content;
      return true;
    }
    if (node->path.len == 0)
      return mpt_fail(MPT_RLP_DECODE);
    node->kind = DECODED_NODE_EXTENSION;
    return mpt_decode_child_reference(&item[1], &node->child_references[0],
                                      &node->child_reference_lengths[0],
                                      &node->child_ids[0]);
  }
  if (count != 17)
    return mpt_fail(MPT_RLP_DECODE);
  node->kind = DECODED_NODE_BRANCH;
  for (unsigned i = 0; i < 16; ++i)
    if (!mpt_decode_child_reference(&item[i], &node->child_references[i],
                                    &node->child_reference_lengths[i],
                                    &node->child_ids[i]))
      return false;
  node->value = item[16].content;
  return true;
}

bool mpt_decode_node(ByteSpan encoded,
                     DecodedNode *node) {
  return mpt_decode_node_impl(encoded, node, false);
}

bool mpt_decode_trusted_state_node(ByteSpan encoded,
                                   DecodedNode *node) {
  return mpt_decode_node_impl(encoded, node, true);
}

