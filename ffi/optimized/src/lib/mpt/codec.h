/* Stateless MPT node codec: node RLP, hex-prefix decoding, child
 * references, and the decoded structural node view. Pure functions over
 * borrowed byte spans; no module state. Mirrors the Sail lib/mpt codec
 * layer over the paths layer. */
#ifndef EVMSAIL_OPTIMIZED_LIB_MPT_CODEC_H
#define EVMSAIL_OPTIMIZED_LIB_MPT_CODEC_H

#include "lib/mpt/common.h"
#include "lib/mpt/paths.h"

/* ======================================================================== */
/* NODE RLP                                                                 */
/* ======================================================================== */

/* One RLP item popped from a cursor: framed bytes, payload, and list tag. */
typedef struct {
  ByteSpan raw;
  ByteSpan content;
  bool list;
} RlpItem;

bool mpt_rlp_pop(ByteSpan *cursor, RlpItem *item);
bool mpt_rlp_pop_trusted_node(ByteSpan *cursor,
                              RlpItem *item);
bool mpt_decode_compact(const RlpItem *field, bool *leaf,
                        NibblePath *path);

/* ======================================================================== */
/* CHILD REFERENCES AND DECODED NODES                                       */
/* ======================================================================== */

typedef enum {
  NODE_REFERENCE_EMPTY,
  NODE_REFERENCE_INLINE,
  NODE_REFERENCE_HASHED
} NodeReferenceKind;

/*
 * A temporary view of one encoded child edge in an immutable witness node.
 * The bytes remain in the stateless input. node_id points into the decoded
 * parent's zero-initialized child-ID array, where zero means unresolved and
 * MPT_NODE_ID_EMPTY denotes a canonical empty edge. encoded_len also tags the
 * reference: zero is empty, 1..31 is embedded RLP, and 32 is a digest.
 */
typedef struct {
  const uint8_t *encoded;
  NodeId *node_id;
  uint8_t encoded_len;
} WitnessChild;

WitnessChild mpt_empty_witness_child(NodeId *node_id);
WitnessChild mpt_hashed_witness_child(const uint8_t *digest, NodeId *node_id);
WitnessChild mpt_inline_witness_child(const uint8_t *encoded, size_t len,
                                      NodeId *node_id);
NodeReferenceKind mpt_witness_child_kind(const WitnessChild *ref);
bool mpt_witness_child_valid(const WitnessChild *ref);

typedef enum {
  DECODED_NODE_LEAF,
  DECODED_NODE_EXTENSION,
  DECODED_NODE_BRANCH
} DecodedNodeKind;

/*
 * Structural view of one encoded node.
 *
 * Extensions use slot zero; branches use the slot matching each nibble.
 * Decoding clears the entire structure first. Non-empty children therefore
 * begin unresolved (ID zero), while the child decoder stamps UINT32_MAX into
 * genuinely empty RLP fields. Linking an edge replaces zero with the stable
 * NodeId and leaves the borrowed input reference in place.
 */
typedef struct {
  DecodedNodeKind kind;
  NibblePath path;
  ByteSpan value;
  const uint8_t *child_references[16];
  NodeId child_ids[16];
  uint8_t child_reference_lengths[16];
} DecodedNode;

WitnessChild mpt_decoded_child(DecodedNode *node, unsigned nibble);

/* Full-validation decode for freshly generated or untrusted encodings. */
bool mpt_decode_node(ByteSpan encoded, DecodedNode *node);
/* Span-checked decode for witness nodes later authenticated by digest. */
bool mpt_decode_trusted_state_node(ByteSpan encoded,
                                   DecodedNode *node);

#endif
