#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint16_t block_header_rlp_lengths(struct BlockHeader header, uint16_t *restrict field_0_8_1568)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t difficulty_length;
  if (profile.fork < Paris) {
    difficulty_length = rlp_uint_word_size(header.prev_randao);
  } else {
    difficulty_length = rlp_uint_word_size_uint8_t_to_uint8_t(UINT8_C(0));
  }
  uint8_t number_length = rlp_uint_word_size_uint64_t_to_uint8_t(header.number);
  uint8_t gas_limit_length = rlp_uint_word_size_uint64_t_to_uint8_t(header.gas_limit);
  uint8_t gas_used_length = rlp_uint_word_size_uint64_t_to_uint8_t(header.gas_used);
  uint8_t timestamp_length = rlp_uint_word_size_uint64_t_to_uint8_t(header.timestamp);
  uint32_t extra_data_length = rlp_input_scratch_slice_size(header.extra_data);
  uint32_t content_length = UINT32_C(219);
  content_length = rlp_scratch_length_add_uint32_t_uint16_t_to_uint32_t(content_length, ((uint16_t)((uint32_t)LOGS_BLOOM_BYTE_LENGTH + (uint32_t)UINT16_C(3))));
  content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, difficulty_length);
  content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, number_length);
  content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, gas_limit_length);
  content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, gas_used_length);
  content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, timestamp_length);
  content_length = rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(content_length, extra_data_length);
  content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, ((uint8_t)((uint32_t)EIGHT_BYTE_LENGTH + (uint32_t)UINT8_C(1))));
  if (profile.fork >= London) {
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, (rlp_uint_word_size(header.base_fee)));
  }
  if (profile.fork >= Shanghai) {
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, UINT8_C(33));
  }
  if (profile.fork >= Cancun) {
    uint8_t blob_gas_used_length = rlp_uint_word_size_uint32_t_to_uint8_t(header.blob_gas_used);
    uint8_t excess_blob_gas_length = rlp_uint_word_size_uint32_t_to_uint8_t(header.excess_blob_gas);
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, blob_gas_used_length);
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, excess_blob_gas_length);
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, UINT8_C(33));
  }
  if (profile.fork >= Prague) {
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, UINT8_C(33));
  }
  if (profile.fork >= Amsterdam) {
    uint8_t slot_number_length = rlp_uint_word_size_uint64_t_to_uint8_t(header.slot_number);
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, UINT8_C(33));
    content_length = rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(content_length, slot_number_length);
  }
  if (UINT16_C(749) < content_length) {
    fatal_error(RlpDecode);
  }
  uint16_t tmp_3_685 = (uint16_t)(content_length % UINT32_C(750));
  (*field_0_8_1568) = tmp_3_685;
  return rlp_list_size_uint16_t_to_uint16_t(tmp_3_685);
}

bytes32 block_header_hash_(struct BlockHeader header, bytes32 transactions_root, bytes32 withdrawals_root, bytes32 requests_hash, bytes32 block_access_list_hash)
{
  return block_header_hash(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash);
}

