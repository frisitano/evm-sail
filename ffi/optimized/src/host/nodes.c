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
#include "evmsail/prelude.h"
#include "evmsail/spec/exceptions.h"
#include "evmsail/spec/host/region_access.h"
#include "lib/mpt/common.h"
#include "lib/mpt/nodes.h"
#include <stdint.h>
#include <stddef.h>

void nodedb_reset(void)
{
  mpt_node_arena_reset();
}

void nodedb_insert(Hash32 digest, uint32_t input_offset, uint32_t encoded_length)
{
  const struct StatelessInputSliceFields input = stateless_input();
  if (input_offset > input.len || encoded_length > input.len - input_offset) {
    fatal_error(WitnessDeficient);
  }
  if (!mpt_node_table_insert(&digest,
                             (ByteSpan){input.bytes + input_offset, (size_t)encoded_length})) {
    fatal_error(WitnessDeficient);
  }
}

bool nodedb_lookup_span(Hash32 digest, uint32_t *input_offset, uint32_t *encoded_length)
{
  ByteSpan encoded;
  uint32_t off = 0;
  if (!mpt_node_index_span(&digest, &encoded) || encoded.len > UINT32_MAX ||
      !stateless_input_offset(encoded.data, encoded.len, &off)) {
    if (input_offset) {
      *input_offset = 0;
    }
    if (encoded_length) {
      *encoded_length = 0;
    }
    return false;
  }
  if (input_offset) {
    *input_offset = off;
  }
  if (encoded_length) {
    *encoded_length = (uint32_t)encoded.len;
  }
  return true;
}
