#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes encode_state_account(struct AccountInfo info, bytes32 storage_root)
{
  uint8_t nonce_length = rlp_uint_word_size_uint64_t_to_uint8_t(info.nonce);
  uint8_t content_len = ((uint8_t)((uint32_t)UINT8_C(33) + ((uint32_t)UINT8_C(33) + ((uint32_t)(rlp_uint_word_size(info.balance)) + (uint32_t)nonce_length))));
  uint8_t encoded_length = rlp_list_size_uint8_t_to_uint8_t_variant_3(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_length);
  rlp_write_list_prefix_uint8_t_to_unit_variant_3(content_len);
  rlp_write_uint_word_uint64_t_to_unit(info.nonce);
  rlp_write_uint_word(info.balance);
  rlp_write_word((hash_to_word(storage_root)));
  rlp_write_word((hash_to_word(info.code_hash)));
  return rlp_encoder_finish(encoder);
}

Bytes encode_storage_value(u256 value)
{
  uint8_t encoded_len = rlp_uint_word_size(value);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_len);
  rlp_write_uint_word(value);
  return rlp_encoder_finish(encoder);
}

