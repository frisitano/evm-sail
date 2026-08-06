/* Optimized witness-node table interface.
 *
 * The implementation owns every representation of an immutable witness node
 * behind one table singleton. Insertion is eager: indexing a witness node
 * allocates its NodeId, records its borrowed encoding, and attempts its
 * structural decode immediately, so the digest index is a pure
 * digest -> NodeId map and NodeIds follow witness-insert order. Inline
 * (embedded) nodes are materialized recursively during that append phase;
 * missing storage roots retain their digest without consuming a row. The data
 * model is documented at the top of nodes.c; the Sail-facing nodedb_* ABI in
 * host/nodes.c adapts the entry points declared here. */
#ifndef EVMSAIL_OPTIMIZED_LIB_MPT_NODES_H
#define EVMSAIL_OPTIMIZED_LIB_MPT_NODES_H

#include "lib/mpt/codec.h"

/* --- lifecycle --------------------------------------------------------- */

/* O(1) logical payload reset before indexing a new stateless input. */
void mpt_node_arena_reset(void);
/* Open the append-only payload region and reserve NodeId zero. */
void mpt_node_arena_begin(void);
/* Seal payload rows, then build the exact digest index. */
bool mpt_node_index_seal(void);

/* --- eager insertion and digest lookup ---------------------------------- */

/* Index one witness node: allocates its NodeId, stores the borrowed
 * encoding, memoizes the digest, and eagerly attempts the structural
 * decode. A malformed encoding is tolerated here and fails only if
 * traversal reaches it, matching the reference's per-root walk. Duplicate
 * digests keep the first binding. Returns false when capacity is exhausted;
 * the node is then simply absent and later resolution fails closed. */
bool mpt_node_table_insert(const Hash32 *digest, ByteSpan encoded);
/* Resolve a digest directly to its borrowed input encoding. */
bool mpt_node_index_span(const Hash32 *digest, ByteSpan *encoded);

/* --- payload column ----------------------------------------------------- */

/* Borrowed encoding of a materialized node; fails closed on identity-only
 * bindings whose node material is absent from the witness. */
bool mpt_node_span(NodeId node_id, ByteSpan *encoded);
/* Memoized keccak of a node's encoding. */
bool mpt_node_digest(NodeId node_id, Hash32 *digest);
/* Memoized structural decode; returns a pointer into the payload column. */
bool mpt_decode_cached_node(NodeId node_id, DecodedNode **node_out);

/* --- edge linking and root binding -------------------------------------- */

/* Resolve one decoded child edge to a stable NodeId plus its encoding,
 * stamping the id back into the parent's child slot. */
bool mpt_link_witness_child(WitnessChild *ref, ByteSpan *encoded, NodeId *node_id);
/* Resolve a trie root digest to its witness NodeId; fails closed when the
 * root node is absent from the witness. */
bool mpt_bind_root(const Hash32 *root, NodeId *node_id);
/* Bind a storage root to a real witness node when present. Missing material
 * remains unresolved; the owning account retains the root digest. */
bool mpt_bind_storage_root_identity(const Hash32 *root, NodeId *node_id);

#endif
