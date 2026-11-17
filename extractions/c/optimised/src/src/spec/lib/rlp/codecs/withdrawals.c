#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes withdrawal_rlp(Bytes withdrawal)
{
  uint64_t index = decode_ssz_uint_Bytes_uint8_t_to_uint64_t(withdrawal, WD_INDEX);
  uint64_t validator_index =
      decode_ssz_uint_Bytes_uint8_t_to_uint64_t(withdrawal, WD_VALIDATOR_INDEX);
  Bytes address = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(withdrawal, WD_ADDRESS,
                                                                           ADDRESS_BYTE_LENGTH);
  uint64_t amount = decode_ssz_uint_Bytes_uint8_t_to_uint64_t(withdrawal, WD_AMOUNT);
  uint8_t index_length = rlp_uint_word_size_uint64_t_to_uint8_t(index);
  uint8_t validator_index_length = rlp_uint_word_size_uint64_t_to_uint8_t(validator_index);
  uint8_t address_length = rlp_input_slice_size_Bytes_to_uint8_t(address);
  uint8_t content_length = (uint8_t)u128_to_u64_unchecked(u128_add_u64(
      u128_of_u64(((uint8_t)((uint32_t)address_length +
                             ((uint32_t)validator_index_length + (uint32_t)index_length)))),
      (rlp_uint_word_size_uint64_t_to_uint8_t(amount))));
  if (UINT8_C(48) < content_length) {
    fatal_error(RlpDecode);
  }
  uint8_t content_len =
      (uint8_t)u128_to_u64_unchecked(u128_mod_u64(u128_of_u64(content_length), UINT8_C(49)));
  uint8_t encoded_length = rlp_list_size_uint8_t_to_uint8_t(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_length);
  rlp_write_list_prefix_uint8_t_to_unit(content_len);
  rlp_write_uint_word_uint64_t_to_unit(index);
  rlp_write_uint_word_uint64_t_to_unit(validator_index);
  rlp_write_input_slice(address);
  rlp_write_uint_word_uint64_t_to_unit(amount);
  return rlp_encoder_finish(encoder);
}
