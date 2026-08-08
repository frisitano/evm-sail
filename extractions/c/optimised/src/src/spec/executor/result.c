#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t RESULT_METADATA_LENGTH = UINT8_C(5);


void result_prefix(bytes32 root, bool success)
{
  scratch_push_b256_bytes32_uint8_t_to_unit(root, WORD_BYTE_LENGTH);
  uint64_t result_2_87 = success ? UINT64_C(0x01) : UINT64_C(0x00);
  scratch_push_byte(result_2_87);
  scratch_push_byte(UINT64_C(0x25));
  scratch_push_byte(UINT64_C(0x00));
  scratch_push_byte(UINT64_C(0x00));
  scratch_push_byte(UINT64_C(0x00));
}

void commit_validation_result(bytes32 root, bool success, Bytes chain_config)
{
  uint8_t fixed_length = ((uint8_t)((uint32_t)RESULT_METADATA_LENGTH + (uint32_t)WORD_BYTE_LENGTH));
  uint32_t output_length = scratch_length_add(fixed_length, chain_config.len);
  uint32_t start = scratch_reserve(output_length);
  result_prefix(root, success);
  stateless_input_scratch_push_slice(chain_config);
  Bytes output = scratch_finish(start);
  bool written = public_output_write(output);
  if (!written) {
    __builtin_trap();
  }
}

void write_validation_result(struct StatelessInputRef input_ref, bool success)
{
  bytes32 root = htr_new_payload_request_(input_ref);
  commit_validation_result(root, success, input_ref.chain_config);
}

void write_invalid_result(void)
{
  uint32_t start = scratch_reserve_uint8_t_to_uint32_t(UINT8_C(24));
  bytes32 default_chain_config = ZERO_HASH;
  default_chain_config = fast_unsigned_vector_update_bytes32(default_chain_config, UINT8_C(8), UINT64_C(0x0C));
  default_chain_config = fast_unsigned_vector_update_bytes32(default_chain_config, UINT8_C(12), UINT64_C(0x04));
  default_chain_config = fast_unsigned_vector_update_bytes32(default_chain_config, UINT8_C(16), UINT64_C(0x08));
  default_chain_config = fast_unsigned_vector_update_bytes32(default_chain_config, UINT8_C(20), UINT64_C(0x08));
  scratch_push_b256_bytes32_uint8_t_to_unit(default_chain_config, UINT8_C(24));
  Bytes chain_config = scratch_finish(start);
  uint8_t fixed_length = ((uint8_t)((uint32_t)RESULT_METADATA_LENGTH + (uint32_t)WORD_BYTE_LENGTH));
  uint32_t output_length = scratch_length_add(fixed_length, chain_config.len);
  uint32_t output_start = scratch_reserve(output_length);
  result_prefix(ZERO_HASH, false);
  scratch_scratch_push_slice(chain_config);
  Bytes output = scratch_finish(output_start);
  bool written = public_output_write(output);
  if (!written) {
    __builtin_trap();
  }
}

