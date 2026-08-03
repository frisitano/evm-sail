/*
 * Sail-facing node-DB ABI (sail/host/nodes.sail).
 *
 * The witness-node table itself lives in lib/mpt/nodes.c: one module owning
 * the digest -> NodeId probe column, the eagerly bound NodeId payload rows,
 * and id allocation. These adapters translate the contract's offset-based
 * view of the stateless input to the table's borrowed spans; nodedb_lookup
 * (slice-returning) is implemented by host/region_access.c over
 * nodedb_lookup_span.
 */
#include "evmsail/host/nodes.h"

#include "evmsail/host/region_access.h"
#include "lib/mpt/nodes.h"

unit nodedb_reset(const unit ignored) {
  (void)ignored;
  mpt_node_index_clear();
  return UNIT;
}

unit nodedb_insert(Hash32 digest, uint64_t input_offset,
                   uint64_t encoded_length) {
  const uint8_t *bytes = stateless_input_ptr(input_offset, encoded_length);
  if (bytes)
    (void)mpt_node_table_insert(
        &digest, (ByteSpan){bytes, (size_t)encoded_length});
  return UNIT;
}

bool nodedb_lookup_span(Hash32 digest,
                        uint64_t *input_offset, uint64_t *encoded_length) {
  ByteSpan encoded;
  uint64_t off = 0;
  if (!mpt_node_index_span(&digest, &encoded) ||
      !stateless_input_offset(encoded.data, encoded.len, &off)) {
    if (input_offset)
      *input_offset = 0;
    if (encoded_length)
      *encoded_length = 0;
    return false;
  }
  if (input_offset)
    *input_offset = off;
  if (encoded_length)
    *encoded_length = encoded.len;
  return true;
}
