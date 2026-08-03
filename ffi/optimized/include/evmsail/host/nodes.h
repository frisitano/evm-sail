#ifndef EVMSAIL_OPTIMIZED_HOST_NODES_H
#define EVMSAIL_OPTIMIZED_HOST_NODES_H
#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>

/* Stable identity of one node in the immutable witness arena. */
typedef uint32_t NodeId;

#define EVMSAIL_NODE_ID_UNLINKED UINT32_C(0)
#define EVMSAIL_NODE_ID_EMPTY UINT32_MAX

/*
 * Sail-facing node-DB contract over the witness-node table owned by
 * lib/mpt/nodes.c: keccak(node) -> (input span, lazy NodeId).
 *
 * Node encodings remain owned by the stateless input. Decoded parents store a
 * dense child-ID table initialized to EVMSAIL_NODE_ID_UNLINKED. Following a
 * hashed edge resolves its input span here, allocates a stable NodeId, and
 * writes that ID back into the parent. Later traversals therefore avoid this
 * digest index entirely.
 *
 * EVMSAIL_NODE_ID_EMPTY is only an edge sentinel. Neither sentinel is ever
 * bound to an encoded witness node in this index.
 */
unit nodedb_reset(const unit u);
unit nodedb_insert(Hash32 digest, uint64_t input_offset,
                   uint64_t encoded_length);
bool nodedb_lookup_span(Hash32 digest,
                        uint64_t *input_offset, uint64_t *encoded_length);

#endif
