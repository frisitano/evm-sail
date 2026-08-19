#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t MPT_HASH_LENGTH = UINT8_C(32);


struct InlineNode inline_node_from_scratch_slice(Bytes bytes)
{
  if (bytes.len < MPT_HASH_LENGTH) {
    u256 encoded = scratch_slice_load(bytes, UINT8_C(0));
    return ((struct InlineNode){.data = (word_to_hash(encoded)), .len = (uint8_t)bytes.len});
  }
  fatal_error(WitnessDeficient);
}

Bytes inline_node_slice(struct InlineNode node)
{
  uint32_t start = scratch_reserve_uint8_t_to_uint32_t(node.len);
  scratch_push_b256(node.data, node.len);
  return scratch_finish(start);
}

bytes32 inline_node_hash(struct InlineNode node)
{
  uint32_t mark = scratch_begin();
  Bytes encoded = inline_node_slice(node);
  bytes32 digest = host_keccak_scratch(encoded);
  scratch_rewind(mark);
  return digest;
}

uint16_t branch_content_length_add(uint16_t current, uint8_t addition)
{
  if (addition <= ((uint16_t)((uint32_t)UINT16_C(529) - (uint32_t)current))) {
    return ((uint16_t)((uint32_t)current + (uint32_t)(uint16_t)addition));
  }
  fatal_error(RlpDecode);
}

