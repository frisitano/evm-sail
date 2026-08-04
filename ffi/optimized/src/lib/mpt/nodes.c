/* Optimized witness-node table.
 *
 * This module owns every representation of an immutable witness node behind
 * one stable identity (NodeId):
 *
 *   1. WitnessIndexEntry is the digest probe column: an open-addressed pure
 *      digest -> NodeId map. Insertion is eager, so every occupied bucket
 *      already names a fully bound payload row and the column never changes
 *      after indexing completes.
 *   2. WitnessNode is the payload column, indexed directly by NodeId: the
 *      borrowed encoding span, the memoized digest, and the memoized decoded
 *      view. A generation stamp resets the logical arena in O(1).
 *   3. WitnessProbeMemo caches the most recent digest probe, hits and misses
 *      alike, because consecutive operations often ask for the same digest
 *      while linking and then walking a child.
 *   4. WitnessNodeTable gathers both columns, the memo, and the id allocator
 *      behind one singleton.
 *
 * NodeIds follow witness-insert order for input nodes; inline (embedded)
 * nodes and storage-root identities draw later ids from the same counter at
 * first traversal. Node encodings stay borrowed from the stateless input for
 * the lifetime of the guest; this table never copies them. */
#include "lib/mpt/nodes.h"

#include "evmsail/host/region_access.h"
#include "primitives/hash.h"
#include "workspace.h"

#include <string.h>

/* One digest probe bucket. The bucket position IS the node identity:
 * NodeId = bucket index + 1, so nothing but the key is stored. An all-zero
 * digest marks an empty bucket; bucket keys are always self-computed keccak
 * outputs, which are never zero. */
typedef struct {
  Hash32 digest;
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
  Hash32 digest;
  uint32_t generation;
} WitnessNode;

/* Memo of the most recent digest probe, hits and misses alike. */
typedef struct {
  Hash32 digest;
  NodeId node_id;
  bool primed;
} WitnessProbeMemo;

/* Complete witness-node table allocation, backed by the guest workspace.
 *
 * index is an open-addressed digest column whose power-of-two bucket_count is
 * sized from the decoded witness-node count at or below 50% load (mirroring
 * account_schema_prepare); bucket_mask implements modulo by bucket_count.
 * nodes is the dense NodeId-indexed payload column spanning the bucket
 * range plus the dynamic tail. next_node_id allocates the dynamic tail
 * (inline nodes and storage-root identities) above the active bucket span;
 * generation is the current logical arena epoch. */
typedef struct {
  WitnessIndexEntry *index;
  WitnessNode *nodes;
  WitnessProbeMemo memo;
  uint32_t index_count;
  uint32_t bucket_count;
  uint32_t bucket_mask;
  NodeId next_node_id;
  uint32_t generation;
} WitnessNodeTable;

/* Singleton witness-node table. Column storage is assigned once from the
 * shared guest workspace by mpt_node_table_workspace_bind. */
static WitnessNodeTable node_table = {
    .next_node_id = 1u,
    .generation = 1,
};

_Static_assert(GUEST_WITNESS_INDEX_ENTRIES + 1u < GUEST_MPT_WITNESS_NODES,
               "payload column must span the bucket range plus dynamic ids");

void mpt_node_table_workspace_bind(void) {
  WORKSPACE_BIND(node_table.index, GUEST_WITNESS_INDEX_ENTRIES);
  WORKSPACE_BIND(node_table.nodes, GUEST_MPT_WITNESS_NODES);
}

/* Bucket positions bias by one so bucket zero cannot alias the unresolved
 * edge sentinel. */
static NodeId bucket_node_id(uint32_t bucket) { return bucket + 1u; }

static bool witness_bucket_empty(const WitnessIndexEntry *entry) {
  static const Hash32 zero_digest = {{0}};
  return hash_equal(&entry->digest, &zero_digest);
}

/* ======================================================================== */
/* LIFECYCLE                                                                */
/* ======================================================================== */

void mpt_node_index_clear(void) {
  if (node_table.bucket_count != 0)
    memset(node_table.index, 0,
           node_table.bucket_count * sizeof(*node_table.index));
  node_table.index_count = 0;
  node_table.bucket_count = 0;
  node_table.bucket_mask = 0;
  node_table.next_node_id = 1u;
  node_table.memo.primed = false;
}

void mpt_node_arena_reset(void) {
  ++node_table.generation;
  if (node_table.generation == 0) {
    memset(node_table.nodes, 0,
           GUEST_MPT_WITNESS_NODES * sizeof(*node_table.nodes));
    node_table.generation = 1;
  }
}

/* Sizes the digest probe column from the decoded witness-node count.
 *
 * A power-of-two table at or below 50% load makes masking and linear probing
 * cheap. The guest workspace starts zeroed, and mpt_node_index_clear clears
 * the previous active range before a reused native run, so preparation only
 * establishes the new logical bounds. */
void mpt_node_index_prepare(uint64_t node_count) {
  uint32_t bucket_count = 1u;
  const uint64_t required = node_count == 0 ? 1u : 2u * node_count;
  while (bucket_count < required &&
         bucket_count < GUEST_WITNESS_INDEX_ENTRIES)
    bucket_count <<= 1;
  node_table.bucket_count = bucket_count;
  node_table.bucket_mask = bucket_count - 1u;
  node_table.index_count = 0;
  node_table.next_node_id = bucket_node_id(bucket_count);
  node_table.memo.primed = false;
}

/* ======================================================================== */
/* PAYLOAD ROWS                                                             */
/* ======================================================================== */

bool mpt_allocate_node_id(NodeId *node_id) {
  if (node_table.next_node_id >= GUEST_MPT_WITNESS_NODES)
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  *node_id = node_table.next_node_id++;
  return true;
}

/* Claims a freshly allocated payload row for the current generation. */
static WitnessNode *witness_node_init(NodeId node_id, ByteSpan encoded) {
  WitnessNode *row = &node_table.nodes[node_id];
  row->data = encoded.data;
  row->len = encoded.len;
  row->decoded = false;
  row->digest_known = false;
  row->generation = node_table.generation;
  return row;
}

/* Resolves a NodeId to its current-generation payload row, verifying that
 * any caller-supplied encoding matches the recorded one. */
static bool witness_node_row(NodeId node_id,
                             ByteSpan encoded, WitnessNode **row_out) {
  if (node_id == MPT_NODE_ID_UNLINKED || node_id == MPT_NODE_ID_EMPTY ||
      node_id >= GUEST_MPT_WITNESS_NODES)
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  WitnessNode *row = &node_table.nodes[node_id];
  if (row->generation != node_table.generation)
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  if (encoded.len != 0 &&
      (row->len != encoded.len || row->data != encoded.data))
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  *row_out = row;
  return true;
}

bool mpt_node_span(NodeId node_id, ByteSpan *encoded) {
  if (node_id == MPT_NODE_ID_EMPTY) {
    encoded->data = NULL;
    encoded->len = 0;
    return true;
  }
  WitnessNode *row = NULL;
  if (!witness_node_row(node_id, (ByteSpan){NULL, 0}, &row))
    return false;
  /* Identity-only bindings carry a digest but no witness material. */
  if (row->len == 0)
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  encoded->data = row->data;
  encoded->len = row->len;
  return true;
}

bool mpt_node_digest(ByteSpan encoded, NodeId node_id,
                     Hash32 *digest) {
  WitnessNode *row = NULL;
  if (!witness_node_row(node_id, encoded, &row))
    return false;
  if (!row->digest_known) {
    if (!mpt_keccak(encoded.data, encoded.len, &row->digest))
      return false;
    row->digest_known = true;
  }
  *digest = row->digest;
  return true;
}

bool mpt_decode_cached_node(ByteSpan encoded, NodeId node_id,
                            DecodedNode **node_out) {
  WitnessNode *row = NULL;
  if (!witness_node_row(node_id, encoded, &row))
    return false;
  if (!row->decoded) {
    if (!mpt_decode_trusted_state_node(encoded, &row->node))
      return false;
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
static uint32_t witness_digest_bucket(const Hash32 *digest) {
  uint32_t bucket = 0;
  memcpy(&bucket, digest->bytes, sizeof(bucket));
  return bucket & node_table.bucket_mask;
}

bool mpt_node_table_insert(const Hash32 *digest, ByteSpan encoded) {
  node_table.memo.primed = false;
  /* The Sail-facing nodedb_insert ABI may arrive without a preceding count;
   * fall back to the full workspace capacity in that case. */
  if (node_table.bucket_count == 0)
    mpt_node_index_prepare(GUEST_WITNESS_INDEX_ENTRIES / 2u);
  if (encoded.len == 0 || encoded.data == NULL)
    return false;
  if ((node_table.index_count + 1) * 4 >= node_table.bucket_count * 3)
    return false;

  uint32_t bucket = witness_digest_bucket(digest);
  for (uint32_t probes = 0; probes < node_table.bucket_count; probes++) {
    WitnessIndexEntry *entry = &node_table.index[bucket];
    if (witness_bucket_empty(entry)) {
      entry->digest = *digest;
      ++node_table.index_count;

      WitnessNode *row = witness_node_init(bucket_node_id(bucket), encoded);
      row->digest = *digest;
      row->digest_known = true;
      /* Eager structural decode. A malformed encoding is tolerated here and
       * fails only if traversal reaches it, matching the reference's
       * per-root witness walk; the shared latch is restored so the attempt
       * cannot poison the operation status. */
      const uint64_t prior_status = mpt_status;
      row->decoded = mpt_decode_trusted_state_node(encoded, &row->node);
      mpt_status = prior_status;
      return true;
    }
    if (hash_equal(&entry->digest, digest))
      return true;
    bucket = (bucket + 1) & node_table.bucket_mask;
  }
  GUEST_ABORT();
}

static void witness_index_probe(const Hash32 *digest) {
  WitnessProbeMemo *memo = &node_table.memo;
  if (memo->primed && hash_equal(&memo->digest, digest))
    return;

  memo->digest = *digest;
  memo->node_id = MPT_NODE_ID_UNLINKED;
  memo->primed = true;
  if (node_table.bucket_count == 0)
    return;
  uint32_t bucket = witness_digest_bucket(digest);
  for (uint32_t probes = 0; probes < node_table.bucket_count; probes++) {
    const WitnessIndexEntry *entry = &node_table.index[bucket];
    if (witness_bucket_empty(entry))
      return;
    if (hash_equal(&entry->digest, digest)) {
      memo->node_id = bucket_node_id(bucket);
      return;
    }
    bucket = (bucket + 1) & node_table.bucket_mask;
  }
}

bool mpt_node_index_lookup(const Hash32 *digest, NodeId *node_id) {
  witness_index_probe(digest);
  if (node_id)
    *node_id = node_table.memo.node_id;
  return node_table.memo.node_id != MPT_NODE_ID_UNLINKED;
}

bool mpt_node_index_span(const Hash32 *digest, ByteSpan *encoded) {
  NodeId node_id = MPT_NODE_ID_UNLINKED;
  if (!mpt_node_index_lookup(digest, &node_id))
    return false;
  const WitnessNode *row = &node_table.nodes[node_id];
  if (row->generation != node_table.generation || row->len == 0)
    return false;
  encoded->data = row->data;
  encoded->len = row->len;
  return true;
}

/* ======================================================================== */
/* EDGE LINKING AND ROOT BINDING                                            */
/* ======================================================================== */

bool mpt_link_witness_child(WitnessChild *ref,
                            ByteSpan *encoded, NodeId *node_id) {
  if (!mpt_witness_child_valid(ref))
    return mpt_fail(MPT_WITNESS_DEFICIENT);
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
    if (!mpt_allocate_node_id(node_id))
      return false;
    encoded->data = ref->encoded;
    encoded->len = ref->encoded_len;
    (void)witness_node_init(*node_id, *encoded);
  } else {
    const Hash32 digest = hash_from_be_bytes(ref->encoded);
    if (!mpt_node_index_lookup(&digest, node_id))
      return mpt_fail(MPT_WITNESS_DEFICIENT);
    if (!mpt_node_span(*node_id, encoded))
      return false;
  }
  *ref->node_id = *node_id;
  return true;
}

bool mpt_bind_root(const Hash32 *root, NodeId *node_id) {
  if (hash_equal(root, &EVMSAIL_EMPTY_TRIE_ROOT)) {
    *node_id = MPT_NODE_ID_EMPTY;
    return true;
  }
  if (!mpt_node_index_lookup(root, node_id))
    return mpt_fail(MPT_WITNESS_DEFICIENT);
  return true;
}

/*
 * Bind an account's authenticated storage-root identity without requiring
 * its witness node. Read-only accounts may never traverse this trie; if the
 * node is present in the witness it is already bound, and otherwise any
 * later traversal fails closed in mpt_node_span.
 */
bool mpt_bind_storage_root_identity(const Hash32 *root,
                                    NodeId *node_id) {
  if (hash_equal(root, &EVMSAIL_EMPTY_TRIE_ROOT)) {
    *node_id = MPT_NODE_ID_EMPTY;
    return true;
  }
  if (mpt_node_index_lookup(root, node_id))
    return true;
  if (!mpt_allocate_node_id(node_id))
    return false;
  WitnessNode *row = witness_node_init(*node_id, (ByteSpan){NULL, 0});
  row->digest = *root;
  row->digest_known = true;
  return true;
}

Hash32 mpt_storage_root_hash(NodeId root_node) {
  if (root_node == MPT_NODE_ID_EMPTY)
    return EVMSAIL_EMPTY_TRIE_ROOT;
  if (root_node == MPT_NODE_ID_UNLINKED ||
      root_node >= GUEST_MPT_WITNESS_NODES) {
    GUEST_ABORT();
  }
  const WitnessNode *row = &node_table.nodes[root_node];
  if (row->generation != node_table.generation || !row->digest_known)
    GUEST_ABORT();
  return row->digest;
}
