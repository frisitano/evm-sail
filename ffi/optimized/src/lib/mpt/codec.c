/* Node RLP codec implementation. See codec.h. */
#include "lib/mpt/codec.h"
#include "lib/mpt/common.h"
#include "lib/mpt/paths.h"
#include "evmsail/host/nodes.h"

#include <stdint.h>
#include <string.h>

/* ======================================================================== */
/* NODE RLP                                                                 */
/* ======================================================================== */

/*
 * Witness nodes are authenticated against their parent digest before their
 * contents affect execution. The optimized decoder therefore retains the
 * mandatory span checks but does not repeat RLP minimal-encoding checks.
 *
 * Decoding is deliberately fallible rather than fatal. Witness indexing uses
 * it to discover inline descendants while still allowing malformed,
 * unreachable witness material. A caller that traverses such a node converts
 * the false result directly to fatal_error.
 */
bool mpt_rlp_pop(ByteSpan *cursor, RlpItem *item)
{
  if (cursor->len == 0) {
    return false;
  }

  const uint8_t first = cursor->data[0];
  size_t header = 0;
  size_t content = 0;
  bool list = false;

  if (first <= 0x7f) {
    content = 1;
  } else if (first <= 0xb7) {
    header = 1;
    content = first - 0x80;
  } else if (first <= 0xbf) {
    const size_t width = first - 0xb7;
    if (width > sizeof(size_t) || cursor->len < 1 + width) {
      return false;
    }
    for (size_t index = 0; index < width; ++index) {
      if (content > (SIZE_MAX >> 8)) {
        return false;
      }
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
    if (width > sizeof(size_t) || cursor->len < 1 + width) {
      return false;
    }
    for (size_t index = 0; index < width; ++index) {
      if (content > (SIZE_MAX >> 8)) {
        return false;
      }
      content = (content << 8) | cursor->data[1 + index];
    }
    header = 1 + width;
  }

  if (header > cursor->len || content > cursor->len - header) {
    return false;
  }

  item->raw.data = cursor->data;
  item->raw.len = header + content;
  item->content.data = cursor->data + header;
  item->content.len = content;
  item->list = list;
  cursor->data += item->raw.len;
  cursor->len -= item->raw.len;
  return true;
}

static bool mpt_decode_compact(const RlpItem *field, bool *leaf, NibblePath *path)
{
  if (field->list) {
    return false;
  }

  *path = nibble_path_empty();
  if (field->content.len == 0) {
    *leaf = false;
    return true;
  }
  if (field->content.len > 33) {
    return false;
  }

  const uint8_t first = field->content.data[0];
  const uint8_t flag = first >> 4;
  if (flag > 3) {
    return false;
  }
  *leaf = (flag & 2) != 0;

  const bool odd = (flag & 1) != 0;
  if (odd) {
    path->nibbles[path->len++] = first & 0x0f;
  } else if ((first & 0x0f) != 0) {
    return false;
  }

  for (size_t index = 1; index < field->content.len; ++index) {
    if (path->len > 62) {
      return false;
    }
    path->nibbles[path->len++] = field->content.data[index] >> 4;
    path->nibbles[path->len++] = field->content.data[index] & 0x0f;
  }
  return true;
}

/* ======================================================================== */
/* CHILD REFERENCES AND DECODED NODES                                       */
/* ======================================================================== */

NodeReferenceKind mpt_witness_child_kind(const WitnessChild *ref)
{
  if (ref->encoded == NULL) {
    return NODE_REFERENCE_EMPTY;
  }
  return ref->encoded_len == 32 ? NODE_REFERENCE_HASHED : NODE_REFERENCE_INLINE;
}

/* Non-empty children keep the zero (unresolved) id from the decoder's
 * memset; only a genuinely empty RLP field stamps the empty sentinel. */
static bool mpt_decode_child_reference(const RlpItem *field, const uint8_t **encoded,
                                       uint8_t *encoded_len, NodeId *node_id)
{
  if (field->list) {
    if (field->raw.len >= 32) {
      return false;
    }
    *encoded = field->raw.data;
    *encoded_len = (uint8_t)field->raw.len;
  } else if (field->content.len == 32) {
    *encoded = field->content.data;
    *encoded_len = 32;
  } else if (field->content.len == 0) {
    *encoded = NULL;
    *encoded_len = 0;
    *node_id = MPT_NODE_ID_EMPTY;
  } else {
    return false;
  }
  return true;
}

WitnessChild mpt_decoded_child(DecodedNode *node, unsigned nibble)
{
  return (WitnessChild){
      .encoded = node->child_references[nibble],
      .node_id = &node->child_ids[nibble],
      .encoded_len = node->child_reference_lengths[nibble],
  };
}

bool mpt_decode_trusted_state_node(ByteSpan encoded, DecodedNode *node)
{
  RlpItem outer;
  ByteSpan root = encoded;
  if (!mpt_rlp_pop(&root, &outer) || root.len != 0 || !outer.list) {
    return false;
  }

  ByteSpan fields = outer.content;
  RlpItem item[17];
  unsigned count = 0;
  while (fields.len != 0 && count < 17) {
    if (!mpt_rlp_pop(&fields, &item[count])) {
      return false;
    }
    ++count;
  }
  if (fields.len != 0) {
    return false;
  }

  memset(node, 0, sizeof(*node));
  if (count == 2) {
    bool leaf;
    if (!mpt_decode_compact(&item[0], &leaf, &node->path)) {
      return false;
    }
    if (leaf) {
      node->kind = DECODED_NODE_LEAF;
      node->value = item[1].content;
      return true;
    }
    if (node->path.len == 0) {
      return false;
    }
    node->kind = DECODED_NODE_EXTENSION;
    return mpt_decode_child_reference(&item[1], &node->child_references[0],
                                      &node->child_reference_lengths[0], &node->child_ids[0]);
  }

  if (count != 17) {
    return false;
  }
  node->kind = DECODED_NODE_BRANCH;
  for (unsigned index = 0; index < 16; ++index) {
    if (!mpt_decode_child_reference(&item[index], &node->child_references[index],
                                    &node->child_reference_lengths[index],
                                    &node->child_ids[index])) {
      return false;
    }
  }
  node->value = item[16].content;
  return true;
}
