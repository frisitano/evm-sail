/* Optimized witness-node table.
 *
 * This module owns every representation of an immutable witness node behind
 * one stable identity (NodeId):
 *
 *   1. WitnessNode is a dense append-only payload row. Input nodes and every
 *      recursively decoded inline child are appended before the region is
 *      sealed, so later traversal never grows it.
 *   2. WitnessIndexEntry is a post-seal open-addressed digest -> NodeId map.
 *      Bucket position is not node identity.
 *      WitnessNode is indexed directly by NodeId: the
 *      borrowed encoding span, the memoized digest, and the memoized decoded
 *      view. A generation stamp resets the logical arena in O(1).
 *   3. WitnessProbeMemo caches the most recent digest probe, hits and misses
 *      alike, because consecutive operations often ask for the same digest
 *      while linking and then walking a child.
 *   4. WitnessNodeTable gathers both columns and the memo
 *      behind one singleton.
 *
 * NodeIds follow witness-insert order with each input node immediately
 * followed by its recursively discovered inline descendants. Node encodings
 * stay borrowed from the stateless input for
 * the lifetime of the guest; this table never copies them. */
#include "lib/mpt/nodes.h"

#include "evmsail/host/nodes.h"
#include "evmsail/prelude.h"
#include "lib/mpt/codec.h"
#include "lib/mpt/common.h"
#include "evmsail/spec/exceptions.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"

#include <stdint.h>
#include <string.h>

/* One digest probe bucket. A zeroed node_id denotes an empty bucket, so
 * every possible digest value (including all zeroes) remains representable. */
typedef struct {
  bytes32 digest;
  NodeId node_id;
} WitnessIndexEntry;

/* One materialized immutable witness node (NodeId-indexed payload row). */
typedef struct {
  const uint8_t *data;
  size_t len;
  /* node holds the memoized structural decode once decoded is set. Input
   * nodes are decoded eagerly at insert; a failed eager decode leaves this
   * clear so the malformed node errors only if traversal reaches it. */
  bool decoded;
  DecodedNode node;
  /* digest holds this node's keccak once digest_known is set. */
  bool digest_known;
  bytes32 digest;
  uint32_t generation;
} WitnessNode;

/* Memo of the most recent digest probe, hits and misses alike. */
typedef struct {
  bytes32 digest;
  NodeId node_id;
  bool primed;
} WitnessProbeMemo;

/* Complete witness-node table allocation, backed by the guest workspace.
 *
 * nodes is grown contiguously from the workspace and then sealed. index is
 * claimed only afterward and is sized from the exact row count at or below
 * 50% load. bucket_mask implements modulo by bucket_count. */
typedef struct {
  WitnessIndexEntry *index;
  WitnessNode *nodes;
  WitnessProbeMemo memo;
  uint32_t index_count;
  uint32_t bucket_count;
  uint32_t bucket_mask;
  uint32_t node_count;
  uint32_t generation;
  bool sealed;
} WitnessNodeTable;

/* Singleton witness-node table. Column storage is assigned once from the
 * shared guest workspace by mpt_node_table_workspace_bind. */
static WitnessNodeTable node_table = {.generation = 1};

static bool witness_bucket_empty(const WitnessIndexEntry *entry)
{
  return entry->node_id == MPT_NODE_ID_UNLINKED;
}

/* ======================================================================== */
/* LIFECYCLE                                                                */
/* ======================================================================== */

void mpt_node_arena_reset(void)
{
  ++node_table.generation;
  if (node_table.generation == 0) {
    node_table.generation = 1;
  }
  node_table.index = NULL;
  node_table.nodes = NULL;
  node_table.index_count = 0;
  node_table.bucket_count = 0;
  node_table.bucket_mask = 0;
  node_table.node_count = 0;
  node_table.sealed = false;
  node_table.memo.primed = false;
}

void mpt_node_arena_begin(void)
{
  workspace_begin_witness_nodes();
  node_table.nodes =
      (WitnessNode *)workspace_claim(1U, sizeof(*node_table.nodes), _Alignof(WitnessNode));
  node_table.node_count = 1U;
}

/* Seal the exact dense payload extent, then claim its digest index. */
bool mpt_node_index_seal(void)
{
  if (node_table.nodes == NULL || node_table.sealed) {
    return false;
  }
  uint32_t bucket_count = 1U;
  const uint64_t real_nodes = node_table.node_count - 1U;
  const uint64_t required = real_nodes == 0 ? 1U : 2U * real_nodes;
  if (required > UINT32_MAX) {
    return false;
  }
  while (bucket_count < required) {
    if (bucket_count > UINT32_MAX / 2U) {
      return false;
    }
    bucket_count <<= 1;
  }
  node_table.index = (WitnessIndexEntry *)workspace_claim(bucket_count, sizeof(*node_table.index),
                                                          _Alignof(WitnessIndexEntry));
  node_table.bucket_count = bucket_count;
  node_table.bucket_mask = bucket_count - 1U;
  node_table.index_count = 0;
  node_table.memo.primed = false;
  for (NodeId node_id = 1U; node_id < node_table.node_count; ++node_id) {
    const WitnessNode *row = &node_table.nodes[node_id];
    if (!row->digest_known) {
      continue;
    }
    uint32_t bucket = 0;
    bucket = (uint32_t)row->digest.lanes[0];
    bucket &= node_table.bucket_mask;
    for (;;) {
      WitnessIndexEntry *entry = &node_table.index[bucket];
      if (witness_bucket_empty(entry)) {
        entry->digest = row->digest;
        entry->node_id = node_id;
        ++node_table.index_count;
        break;
      }
      if (hash_equal(&entry->digest, &row->digest)) {
        break;
      }
      bucket = (bucket + 1U) & node_table.bucket_mask;
    }
  }
  node_table.sealed = true;
  workspace_seal_witness_nodes();
  return true;
}

/* ======================================================================== */
/* PAYLOAD ROWS                                                             */
/* ======================================================================== */

/* Append a payload row while the node region is open. */
static WitnessNode *witness_node_append(ByteSpan encoded, NodeId *node_id)
{
  if (node_table.nodes == NULL || node_table.sealed || node_table.node_count == MPT_NODE_ID_EMPTY) {
    return NULL;
  }
  WitnessNode *row = (WitnessNode *)workspace_claim(1U, sizeof(*row), _Alignof(WitnessNode));
  if (row != node_table.nodes + node_table.node_count) {
    GUEST_ABORT();
  }
  *node_id = node_table.node_count++;
  row->data = encoded.data;
  row->len = encoded.len;
  row->decoded = false;
  row->digest_known = false;
  row->generation = node_table.generation;
  return row;
}

/* Resolves a NodeId to its current-generation payload row. */
static bool witness_node_row(NodeId node_id, WitnessNode **row_out)
{
  if (node_id == MPT_NODE_ID_UNLINKED || node_id == MPT_NODE_ID_EMPTY ||
      node_id >= node_table.node_count) {
    fatal_error(WitnessDeficient);
  }
  WitnessNode *row = &node_table.nodes[node_id];
  if (row->generation != node_table.generation) {
    fatal_error(WitnessDeficient);
  }
  *row_out = row;
  return true;
}

bool mpt_node_span(NodeId node_id, ByteSpan *encoded)
{
  if (node_id == MPT_NODE_ID_EMPTY) {
    encoded->data = NULL;
    encoded->len = 0;
    return true;
  }
  WitnessNode *row = NULL;
  if (!witness_node_row(node_id, &row)) {
    return false;
  }
  /* Identity-only bindings carry a digest but no witness material. */
  if (row->len == 0) {
    fatal_error(WitnessDeficient);
  }
  encoded->data = row->data;
  encoded->len = row->len;
  return true;
}

bool mpt_node_digest(NodeId node_id, bytes32 *digest)
{
  WitnessNode *row = NULL;
  if (!witness_node_row(node_id, &row)) {
    return false;
  }
  if (!row->digest_known) {
    if (!mpt_keccak(row->data, row->len, &row->digest)) {
      return false;
    }
    row->digest_known = true;
  }
  *digest = row->digest;
  return true;
}

bool mpt_decode_cached_node(NodeId node_id, DecodedNode **node_out)
{
  WitnessNode *row = NULL;
  if (!witness_node_row(node_id, &row)) {
    return false;
  }
  if (!row->decoded) {
    if (!mpt_decode_trusted_state_node((ByteSpan){row->data, row->len}, &row->node)) {
      fatal_error(RlpDecode);
    }
    row->decoded = true;
  }
  *node_out = &row->node;
  return true;
}

/* ======================================================================== */
/* DIGEST PROBE COLUMN                                                      */
/* ======================================================================== */

/* The key is already a keccak digest, so its first bytes are a uniformly distributed
 * bucket seed and needs no additional mixing. */
static bool witness_append_inline(WitnessChild *ref);

static uint32_t witness_digest_bucket(const bytes32 *digest)
{
  uint32_t bucket = 0;
  bucket = (uint32_t)digest->lanes[0];
  return bucket & node_table.bucket_mask;
}

/* Inline child nodes form a finite RLP tree bounded by the witness input. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool witness_expand_decoded(DecodedNode *node)
{
  unsigned child_count = 0;
  if (node->kind == DECODED_NODE_EXTENSION) {
    child_count = 1;
  } else if (node->kind == DECODED_NODE_BRANCH) {
    child_count = 16;
  }
  for (unsigned nibble = 0; nibble < child_count; ++nibble) {
    WitnessChild child = mpt_decoded_child(node, nibble);
    if (mpt_witness_child_kind(&child) == NODE_REFERENCE_INLINE && !witness_append_inline(&child)) {
      return false;
    }
  }
  return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool witness_append_inline(WitnessChild *ref)
{
  const ByteSpan encoded = {ref->encoded, ref->encoded_len};
  NodeId node_id = MPT_NODE_ID_UNLINKED;
  WitnessNode *row = witness_node_append(encoded, &node_id);
  if (row == NULL) {
    return false;
  }
  *ref->node_id = node_id;
  if (!mpt_keccak(encoded.data, encoded.len, &row->digest)) {
    return false;
  }
  row->digest_known = true;
  row->decoded = mpt_decode_trusted_state_node(encoded, &row->node);
  return (!row->decoded || witness_expand_decoded(&row->node)) != 0;
}

bool mpt_node_table_insert(const bytes32 *digest, ByteSpan encoded)
{
  node_table.memo.primed = false;
  if (encoded.len == 0 || node_table.sealed) {
    return false;
  }
  NodeId node_id = MPT_NODE_ID_UNLINKED;
  WitnessNode *row = witness_node_append(encoded, &node_id);
  if (row == NULL) {
    return false;
  }
  row->digest = *digest;
  row->digest_known = true;
  row->decoded = mpt_decode_trusted_state_node(encoded, &row->node);
  if (!row->decoded) {
    return true;
  }
  return witness_expand_decoded(&row->node);
}

static void witness_index_probe(const bytes32 *digest)
{
  WitnessProbeMemo *memo = &node_table.memo;
  if (memo->primed && hash_equal(&memo->digest, digest)) {
    return;
  }

  memo->digest = *digest;
  memo->node_id = MPT_NODE_ID_UNLINKED;
  memo->primed = true;
  if (node_table.bucket_count == 0) {
    return;
  }
  uint32_t bucket = witness_digest_bucket(digest);
  for (uint32_t probes = 0; probes < node_table.bucket_count; probes++) {
    const WitnessIndexEntry *entry = &node_table.index[bucket];
    if (witness_bucket_empty(entry)) {
      return;
    }
    if (hash_equal(&entry->digest, digest)) {
      memo->node_id = entry->node_id;
      return;
    }
    bucket = (bucket + 1U) & node_table.bucket_mask;
  }
}

static bool mpt_node_index_lookup(const bytes32 *digest, NodeId *node_id)
{
  witness_index_probe(digest);
  if (node_id) {
    *node_id = node_table.memo.node_id;
  }
  return node_table.memo.node_id != MPT_NODE_ID_UNLINKED;
}

bool mpt_node_index_span(const bytes32 *digest, ByteSpan *encoded)
{
  NodeId node_id = MPT_NODE_ID_UNLINKED;
  if (!mpt_node_index_lookup(digest, &node_id)) {
    return false;
  }
  const WitnessNode *row = &node_table.nodes[node_id];
  if (row->generation != node_table.generation || row->len == 0) {
    return false;
  }
  encoded->data = row->data;
  encoded->len = row->len;
  return true;
}

/* ======================================================================== */
/* EDGE LINKING AND ROOT BINDING                                            */
/* ======================================================================== */

bool mpt_link_witness_child(WitnessChild *ref, ByteSpan *encoded, NodeId *node_id)
{
  const NodeReferenceKind kind = mpt_witness_child_kind(ref);
  if (kind == NODE_REFERENCE_EMPTY) {
    *ref->node_id = MPT_NODE_ID_EMPTY;
    *node_id = MPT_NODE_ID_EMPTY;
    encoded->data = NULL;
    encoded->len = 0;
    return true;
  }
  if (*ref->node_id != MPT_NODE_ID_UNLINKED) {
    *node_id = *ref->node_id;
    return mpt_node_span(*node_id, encoded);
  }

  if (kind == NODE_REFERENCE_INLINE) {
    fatal_error(WitnessDeficient);
  } else {
    const bytes32 digest = hash_from_be_bytes(ref->encoded);
    if (!mpt_node_index_lookup(&digest, node_id)) {
      fatal_error(WitnessDeficient);
    }
    if (!mpt_node_span(*node_id, encoded)) {
      return false;
    }
  }
  *ref->node_id = *node_id;
  return true;
}

bool mpt_bind_root(const bytes32 *root, NodeId *node_id)
{
  if (hash_equal(root, &EVMSAIL_EMPTY_TRIE_ROOT)) {
    *node_id = MPT_NODE_ID_EMPTY;
    return true;
  }
  if (!mpt_node_index_lookup(root, node_id)) {
    fatal_error(WitnessDeficient);
  }
  return true;
}

/*
 * Bind an account's authenticated storage-root identity without requiring
 * its witness node. Read-only accounts may never traverse this trie; if the
 * node is present in the witness it is already bound, and otherwise any
 * later traversal fails closed in mpt_node_span.
 */
bool mpt_bind_storage_root_identity(const bytes32 *root, NodeId *node_id)
{
  if (hash_equal(root, &EVMSAIL_EMPTY_TRIE_ROOT)) {
    *node_id = MPT_NODE_ID_EMPTY;
    return true;
  }
  if (!mpt_node_index_lookup(root, node_id)) {
    *node_id = MPT_NODE_ID_UNLINKED;
  }
  return true;
}
