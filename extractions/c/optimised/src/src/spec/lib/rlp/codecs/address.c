#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes20 create_address(bytes20 sender, uint64_t nonce)
{
  uint8_t nonce_length = rlp_uint_word_size_uint64_t_to_uint8_t(nonce);
  uint8_t content_len = ((uint8_t)((uint32_t)nonce_length + (uint32_t)UINT8_C(21)));
  uint8_t encoded_len = rlp_list_size_uint8_t_to_uint8_t(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_len);
  rlp_write_list_prefix_uint8_t_to_unit(content_len);
  rlp_write_addr(sender);
  rlp_write_uint_word_uint64_t_to_unit(nonce);
  Bytes encoded = rlp_encoder_finish(encoder);
  bytes32 digest = host_keccak_scratch(encoded);
  u256 digest_word = hash_to_word(digest);
  bytes20 address = word_to_address(digest_word);
  rlp_encoder_rewind(encoder);
  return address;
}

