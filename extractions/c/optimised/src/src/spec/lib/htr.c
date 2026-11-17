#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes32 htr_uint(uint64_t v)
{
  bytes32 bytes = ZERO_HASH;
  bytes =
      fast_unsigned_vector_update_bytes32(bytes, UINT8_C(0), (UINT64_C(0xFF) & (v >> UINT64_C(0))));
  bytes =
      fast_unsigned_vector_update_bytes32(bytes, UINT8_C(1), (UINT64_C(0xFF) & (v >> UINT64_C(8))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(2),
                                              (UINT64_C(0xFF) & (v >> UINT64_C(16))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(3),
                                              (UINT64_C(0xFF) & (v >> UINT64_C(24))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(4),
                                              (UINT64_C(0xFF) & (v >> UINT64_C(32))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(5),
                                              (UINT64_C(0xFF) & (v >> UINT64_C(40))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(6),
                                              (UINT64_C(0xFF) & (v >> UINT64_C(48))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(7),
                                              (UINT64_C(0xFF) & (v >> UINT64_C(56))));
  return B256(bytes);
}

bytes32 htr_new_payload_request_(struct StatelessInputRef input_ref)
{
  return htr_new_payload_request(input_ref);
}

bytes32 htr_uint_uint8_t_to_bytes32(uint8_t v)
{
  bytes32 bytes = ZERO_HASH;
  bytes =
      fast_unsigned_vector_update_bytes32(bytes, UINT8_C(0), (UINT64_C(0xFF) & (v >> UINT64_C(0))));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(1), (UINT64_C(0xFF) & UINT64_C(0)));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(2), (UINT64_C(0xFF) & UINT64_C(0)));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(3), (UINT64_C(0xFF) & UINT64_C(0)));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(4), (UINT64_C(0xFF) & UINT64_C(0)));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(5), (UINT64_C(0xFF) & UINT64_C(0)));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(6), (UINT64_C(0xFF) & UINT64_C(0)));
  bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(7), (UINT64_C(0xFF) & UINT64_C(0)));
  return B256(bytes);
}
