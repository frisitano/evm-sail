#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t RESULT_LENGTH = UINT8_C(43);


void commit_validation_result(bytes32 root, bool success, uint64_t chain_id, uint64_t schema_id)
{
  uint32_t start = scratch_reserve_uint8_t_to_uint32_t(RESULT_LENGTH);
  scratch_push_b256_bytes32_uint8_t_to_unit(root, WORD_BYTE_LENGTH);
  uint64_t success_byte = success ? UINT64_C(0x01) : UINT64_C(0x00);
  scratch_push_byte(success_byte);
  bytes32 identity = htr_uint(chain_id);
  identity = fast_unsigned_vector_update_bytes32(identity, UINT8_C(8), (UINT64_C(0xFF) & (schema_id >> UINT8_C(0))));
  identity = fast_unsigned_vector_update_bytes32(identity, UINT8_C(9), (UINT64_C(0xFF) & (schema_id >> UINT8_C(8))));
  scratch_push_b256_bytes32_uint8_t_to_unit(identity, UINT8_C(10));
  Bytes output = scratch_finish(start);
  bool written = public_output_write(output);
  if (!written) {
    __builtin_trap();
  }
}

void write_validation_result(struct StatelessInputRef input_ref, bool success)
{
  commit_validation_result((htr_new_payload_request_(input_ref)), success, input_ref.chain_id, input_ref.schema_id);
}

void write_invalid_result(void)
{
  commit_validation_result_bytes32_bool_uint8_t_uint64_t_to_unit(ZERO_HASH, false, UINT8_C(0), UINT64_C(0x0000));
}

void commit_validation_result_bytes32_bool_uint8_t_uint64_t_to_unit(bytes32 root, bool success, uint8_t chain_id, uint64_t schema_id)
{
  uint32_t start = scratch_reserve_uint8_t_to_uint32_t(RESULT_LENGTH);
  scratch_push_b256_bytes32_uint8_t_to_unit(root, WORD_BYTE_LENGTH);
  uint64_t success_byte = success ? UINT64_C(0x01) : UINT64_C(0x00);
  scratch_push_byte(success_byte);
  bytes32 identity = htr_uint_uint8_t_to_bytes32(chain_id);
  identity = fast_unsigned_vector_update_bytes32(identity, UINT8_C(8), (UINT64_C(0xFF) & (schema_id >> UINT8_C(0))));
  identity = fast_unsigned_vector_update_bytes32(identity, UINT8_C(9), (UINT64_C(0xFF) & (schema_id >> UINT8_C(8))));
  scratch_push_b256_bytes32_uint8_t_to_unit(identity, UINT8_C(10));
  Bytes output = scratch_finish(start);
  bool written = public_output_write(output);
  if (!written) {
    __builtin_trap();
  }
}

