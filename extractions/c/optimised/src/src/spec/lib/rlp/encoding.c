#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t RLP_SHORT_LENGTH_LIMIT = UINT8_C(55);


const uint8_t RLP_WORD_LENGTH_LIMIT = UINT8_C(32);


const uint8_t RLP_UINT64_LENGTH_LIMIT = UINT8_C(8);


uint32_t rlp_scratch_small_length(uint8_t value)
{
  return (uint32_t)value;
}

uint64_t rlp_nat_length_byte(uint8_t value)
{
  return (UINT64_C(0xFF) & (value >> UINT64_C(0)));
}

uint8_t rlp_minimal_word_len(u256 w)
{
  uint16_t bit_length = word_bit_length(w);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

uint32_t rlp_materialized_slice_size(uint32_t length_, uint64_t first)
{
  bool tmp_3_3618 = (bool)((length_ == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3618) {
    return UINT32_C(1);
  }
  uint8_t prefix_size = rlp_length_prefix_len_uint32_t_to_uint8_t(length_);
  uint32_t prefix_length = rlp_scratch_small_length(prefix_size);
  return rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(length_, prefix_length);
}

uint32_t rlp_scratch_region_size(Bytes data)
{
  uint64_t first;
  if (data.len == UINT8_C(0)) {
    first = UINT64_C(0x00);
  } else {
    first = scratch_byte(data, UINT8_C(0));
  }
  return rlp_materialized_slice_size(data.len, first);
}

uint32_t rlp_log_data_size(Bytes data)
{
  uint64_t first;
  if (data.len == UINT8_C(0)) {
    first = UINT64_C(0x00);
  } else {
    first = log_data_byte(data, UINT8_C(0));
  }
  return rlp_materialized_slice_size(data.len, first);
}

uint8_t rlp_uint_word_size(u256 w)
{
  uint8_t len = rlp_minimal_word_len(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte(w);
  } else {
    first = UINT64_C(0x00);
  }
  bool tmp_3_3617 = (bool)((len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3617) {
    return UINT8_C(1);
  }
  return ((uint8_t)((uint32_t)len + (uint32_t)UINT8_C(1)));
}

uint32_t rlp_scratch_list_size(uint32_t content_len)
{
  uint8_t prefix_size = rlp_length_prefix_len_uint32_t_to_uint8_t(content_len);
  uint32_t prefix_length = rlp_scratch_small_length(prefix_size);
  return rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(content_len, prefix_length);
}

uint32_t rlp_input_scratch_slice_size(Bytes data)
{
  uint64_t first;
  if (data.len == UINT8_C(0)) {
    first = UINT64_C(0x00);
  } else {
    first = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(data, UINT8_C(0));
  }
  return rlp_materialized_slice_size(data.len, first);
}

uint32_t rlp_scratch_scratch_slice_size(Bytes data)
{
  return rlp_scratch_region_size(data);
}

uint32_t rlp_log_scratch_slice_size(Bytes data)
{
  return rlp_log_data_size(data);
}

void rlp_write_input_slice(Bytes data)
{
  uint64_t first;
  if (data.len == UINT8_C(0)) {
    first = UINT64_C(0x00);
  } else {
    first = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(data, UINT8_C(0));
  }
  rlp_write_string_prefix_uint32_t_uint64_t_to_unit(data.len, first);
  stateless_input_scratch_push_slice(data);
}

void rlp_write_scratch_slice(Bytes data)
{
  uint64_t first;
  if (data.len == UINT8_C(0)) {
    first = UINT64_C(0x00);
  } else {
    first = scratch_byte(data, UINT8_C(0));
  }
  rlp_write_string_prefix_uint32_t_uint64_t_to_unit(data.len, first);
  scratch_scratch_push_slice(data);
}

void rlp_write_log_data_slice(Bytes data)
{
  uint64_t first;
  if (data.len == UINT8_C(0)) {
    first = UINT64_C(0x00);
  } else {
    first = log_data_byte(data, UINT8_C(0));
  }
  rlp_write_string_prefix_uint32_t_uint64_t_to_unit(data.len, first);
  log_data_scratch_push_slice(data);
}

void rlp_write_uint_word(u256 w)
{
  uint8_t len = rlp_minimal_word_len(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte(w);
  } else {
    first = UINT64_C(0x00);
  }
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(len, first);
  u256 word = as_u256(w);
  scratch_push_word_be(word, len);
}

void rlp_write_word(u256 w)
{
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit(WORD_BYTE_LENGTH);
  scratch_push_word_be_u256_uint8_t_to_unit(w, WORD_BYTE_LENGTH);
}

void rlp_write_addr(bytes20 a)
{
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit(ADDRESS_BYTE_LENGTH);
  scratch_push_address(a);
}

struct RlpEncoder rlp_encoder_begin(uint32_t expected_len)
{
  uint32_t scratch_reserve_result_2_2397 = scratch_reserve(expected_len);
  return ((struct RlpEncoder){.expected_len = expected_len, .start = scratch_reserve_result_2_2397});
}

Bytes rlp_encoder_finish(struct RlpEncoder encoder)
{
  Bytes encoded = scratch_finish(encoder.start);
  if (encoded.len != encoder.expected_len) {
    __builtin_trap();
  }
  return encoded;
}

void rlp_encoder_rewind(struct RlpEncoder encoder)
{
  scratch_rewind(encoder.start);
}

struct RlpEncoder rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(uint8_t expected_len)
{
  uint32_t scratch_reserve_result_2_2397 = scratch_reserve_uint8_t_to_uint32_t(expected_len);
  return ((struct RlpEncoder){.expected_len = (uint32_t)expected_len, .start = scratch_reserve_result_2_2397});
}

uint8_t rlp_length_prefix_len_uint16_t_to_uint8_t(uint16_t len)
{
  u256 length_word = rlp_length_word_uint16_t_to_u256(len);
  uint8_t rlp_minimal_word_len_result_2_2440 = rlp_minimal_word_len(length_word);
  return ((uint8_t)((uint32_t)rlp_minimal_word_len_result_2_2440 + (uint32_t)UINT8_C(1)));
}

uint8_t rlp_length_prefix_len_uint32_t_to_uint8_t(uint32_t len)
{
  bool lteq_int_result_2_2439 = (bool)(len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2439) {
    return UINT8_C(1);
  }
  u256 length_word = rlp_length_word_uint32_t_to_u256(len);
  uint8_t rlp_minimal_word_len_result_2_2440 = rlp_minimal_word_len(length_word);
  return ((uint8_t)((uint32_t)rlp_minimal_word_len_result_2_2440 + (uint32_t)UINT8_C(1)));
}

uint8_t rlp_length_prefix_len_uint8_t_to_uint8_t(void)
{
  return UINT8_C(1);
}

uint8_t rlp_length_prefix_len_uint8_t_to_uint8_t_variant_2(uint8_t len)
{
  bool lteq_int_result_2_2439 = (bool)(len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2439) {
    return UINT8_C(1);
  }
  u256 length_word = rlp_length_word_uint8_t_to_u256(len);
  uint8_t rlp_minimal_word_len_result_2_2440 = rlp_minimal_word_len(length_word);
  return ((uint8_t)((uint32_t)rlp_minimal_word_len_result_2_2440 + (uint32_t)UINT8_C(1)));
}

uint8_t rlp_length_prefix_len_uint8_t_to_uint8_t_variant_3(uint8_t len)
{
  u256 length_word = rlp_length_word_uint8_t_to_u256(len);
  uint8_t rlp_minimal_word_len_result_2_2440 = rlp_minimal_word_len(length_word);
  return ((uint8_t)((uint32_t)rlp_minimal_word_len_result_2_2440 + (uint32_t)UINT8_C(1)));
}

u256 rlp_length_word_uint16_t_to_u256(uint16_t value)
{
  return word_of_nat_byte_count_uint16_t_to_u256(value);
}

u256 rlp_length_word_uint32_t_to_u256(uint32_t value)
{
  return word_of_nat_byte_count_uint32_t_to_u256(value);
}

u256 rlp_length_word_uint8_t_to_u256(uint8_t value)
{
  return word_of_nat_byte_count_uint8_t_to_u256(value);
}

uint8_t rlp_list_size_uint8_t_to_uint8_t(uint8_t content_len)
{
  uint8_t rlp_length_prefix_len_result_2_2425 = rlp_length_prefix_len_uint8_t_to_uint8_t();
  return ((uint8_t)((uint32_t)content_len + (uint32_t)rlp_length_prefix_len_result_2_2425));
}

uint8_t rlp_list_size_uint8_t_to_uint8_t_variant_2(uint8_t content_len)
{
  uint8_t rlp_length_prefix_len_result_2_2425 = rlp_length_prefix_len_uint8_t_to_uint8_t_variant_2(content_len);
  return ((uint8_t)((uint32_t)content_len + (uint32_t)rlp_length_prefix_len_result_2_2425));
}

uint8_t rlp_list_size_uint8_t_to_uint8_t_variant_3(uint8_t content_len)
{
  uint8_t rlp_length_prefix_len_result_2_2425 = rlp_length_prefix_len_uint8_t_to_uint8_t_variant_3(content_len);
  return ((uint8_t)((uint32_t)content_len + (uint32_t)rlp_length_prefix_len_result_2_2425));
}

uint8_t rlp_minimal_word_len_u128_to_uint8_t(u128 w)
{
  uint16_t bit_length = word_bit_length_u128_to_uint16_t(w);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

uint8_t rlp_minimal_word_len_uint64_t_to_uint8_t(uint64_t w)
{
  uint16_t bit_length = word_bit_length_uint64_t_to_uint16_t(w);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

uint8_t rlp_minimal_word_len_uint8_t_to_uint8_t(uint8_t w)
{
  uint16_t bit_length = word_bit_length_uint8_t_to_uint16_t(w);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

uint32_t rlp_scratch_length_add_uint16_t_uint32_t_to_uint32_t(uint16_t left, uint32_t right)
{
  return ((uint32_t)left + (uint32_t)(uint16_t)right);
}

uint32_t rlp_scratch_length_add_uint16_t_uint8_t_to_uint32_t(uint16_t left, uint8_t right)
{
  return ((uint32_t)left + (uint32_t)(uint16_t)right);
}

uint32_t rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t(uint32_t left, uint32_t right)
{
  return ((uint32_t)(uint16_t)left + (uint32_t)(uint16_t)right);
}

uint32_t rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_2(uint32_t left, uint32_t right)
{
  if (right <= (UINT32_C(4294967295) - left)) {
    return (right + left);
  }
  fatal_error(RlpDecode);
}

uint32_t rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(uint32_t left, uint32_t right)
{
  uint32_t result_8_1280;
  if (right <= (UINT32_C(4294967295) - left)) {
    result_8_1280 = (uint32_t)((uint64_t)left + (uint64_t)right);
  } else {
    fatal_error(RlpDecode);
  }
  return result_8_1280;
}

uint32_t rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(uint32_t left, uint8_t right)
{
  uint32_t result_8_1281;
  if (right <= (UINT32_C(4294967295) - left)) {
    result_8_1281 = (uint32_t)((uint64_t)left + (uint64_t)right);
  } else {
    fatal_error(RlpDecode);
  }
  return result_8_1281;
}

uint32_t rlp_scratch_length_add_uint8_t_uint32_t_to_uint32_t(uint8_t left, uint32_t right)
{
  uint32_t result_8_1282;
  if (right <= (UINT32_C(4294967295) - (uint32_t)left)) {
    result_8_1282 = (uint32_t)((uint64_t)left + (uint64_t)right);
  } else {
    fatal_error(RlpDecode);
  }
  return result_8_1282;
}

uint8_t rlp_uint_word_size_uint64_t_to_uint8_t(uint64_t w)
{
  uint8_t len = rlp_minimal_word_len_uint64_t_to_uint8_t(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte_uint64_t_to_uint64_t(w);
  } else {
    first = UINT64_C(0x00);
  }
  bool tmp_3_3617 = (bool)((len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3617) {
    return UINT8_C(1);
  }
  return ((uint8_t)((uint32_t)len + (uint32_t)UINT8_C(1)));
}

uint8_t rlp_uint_word_size_uint8_t_to_uint8_t(uint8_t w)
{
  uint8_t len = rlp_minimal_word_len_uint8_t_to_uint8_t(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte_uint8_t_to_uint64_t(w);
  } else {
    first = UINT64_C(0x00);
  }
  bool tmp_3_3617 = (bool)((len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3617) {
    return UINT8_C(1);
  }
  return ((uint8_t)((uint32_t)len + (uint32_t)UINT8_C(1)));
}

void rlp_write_list_prefix_uint16_t_to_unit(uint16_t content_len)
{
  bool lteq_int_result_2_2409 = (bool)(content_len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2409) {
    uint64_t rlp_length_byte_native_result_2_2410 = rlp_length_byte_native((uint8_t)content_len);
    scratch_push_byte(((UINT64_C(0xC0) + rlp_length_byte_native_result_2_2410) & UINT64_C(0xFF)));
    return;
  }
  u256 length_word = rlp_length_word_uint16_t_to_u256(content_len);
  uint8_t length_len = rlp_minimal_word_len(length_word);
  uint64_t rlp_length_byte_native_result_2_2412 = rlp_length_byte_native(length_len);
  scratch_push_byte(((UINT64_C(0xF7) + rlp_length_byte_native_result_2_2412) & UINT64_C(0xFF)));
  scratch_push_word_be(length_word, length_len);
}

void rlp_write_list_prefix_uint32_t_to_unit(uint32_t content_len)
{
  bool lteq_int_result_2_2409 = (bool)(content_len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2409) {
    uint64_t rlp_length_byte_native_result_2_2410 = rlp_length_byte_native((uint8_t)content_len);
    scratch_push_byte(((UINT64_C(0xC0) + rlp_length_byte_native_result_2_2410) & UINT64_C(0xFF)));
    return;
  }
  u256 length_word = rlp_length_word_uint32_t_to_u256(content_len);
  uint8_t length_len = rlp_minimal_word_len(length_word);
  uint64_t rlp_length_byte_native_result_2_2412 = rlp_length_byte_native(length_len);
  scratch_push_byte(((UINT64_C(0xF7) + rlp_length_byte_native_result_2_2412) & UINT64_C(0xFF)));
  scratch_push_word_be(length_word, length_len);
}

void rlp_write_list_prefix_uint8_t_to_unit(uint8_t content_len)
{
  uint64_t rlp_length_byte_native_result_2_2410 = rlp_length_byte_native(content_len);
  scratch_push_byte(((UINT64_C(0xC0) + rlp_length_byte_native_result_2_2410) & UINT64_C(0xFF)));
}

void rlp_write_list_prefix_uint8_t_to_unit_variant_2(uint8_t content_len)
{
  bool lteq_int_result_2_2409 = (bool)(content_len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2409) {
    uint64_t rlp_length_byte_native_result_2_2410 = rlp_length_byte_native(content_len);
    scratch_push_byte(((UINT64_C(0xC0) + rlp_length_byte_native_result_2_2410) & UINT64_C(0xFF)));
    return;
  }
  u256 length_word = rlp_length_word_uint8_t_to_u256(content_len);
  uint8_t length_len = rlp_minimal_word_len(length_word);
  uint64_t rlp_length_byte_native_result_2_2412 = rlp_length_byte_native(length_len);
  scratch_push_byte(((UINT64_C(0xF7) + rlp_length_byte_native_result_2_2412) & UINT64_C(0xFF)));
  scratch_push_word_be(length_word, length_len);
}

void rlp_write_list_prefix_uint8_t_to_unit_variant_3(uint8_t content_len)
{
  u256 length_word = rlp_length_word_uint8_t_to_u256(content_len);
  uint8_t length_len = rlp_minimal_word_len(length_word);
  uint64_t rlp_length_byte_native_result_2_2412 = rlp_length_byte_native(length_len);
  scratch_push_byte(((UINT64_C(0xF7) + rlp_length_byte_native_result_2_2412) & UINT64_C(0xFF)));
  scratch_push_word_be(length_word, length_len);
}

void rlp_write_string_prefix_uint16_t_uint64_t_to_unit(uint16_t len)
{
  u256 length_word = rlp_length_word_uint16_t_to_u256(len);
  uint8_t length_len = rlp_minimal_word_len(length_word);
  uint64_t rlp_length_byte_native_result_2_2420 = rlp_length_byte_native(length_len);
  scratch_push_byte(((UINT64_C(0xB7) + rlp_length_byte_native_result_2_2420) & UINT64_C(0xFF)));
  scratch_push_word_be(length_word, length_len);
}

void rlp_write_string_prefix_uint32_t_uint64_t_to_unit(uint32_t len, uint64_t first)
{
  bool tmp_3_3615 = (bool)((len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3615) {
    return;
  }
  bool lteq_int_result_2_2417 = (bool)(len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2417) {
    uint64_t rlp_length_byte_native_result_2_2418 = rlp_length_byte_native((uint8_t)len);
    scratch_push_byte(((UINT64_C(0x80) + rlp_length_byte_native_result_2_2418) & UINT64_C(0xFF)));
    return;
  }
  u256 length_word = rlp_length_word_uint32_t_to_u256(len);
  uint8_t length_len = rlp_minimal_word_len(length_word);
  uint64_t rlp_length_byte_native_result_2_2420 = rlp_length_byte_native(length_len);
  scratch_push_byte(((UINT64_C(0xB7) + rlp_length_byte_native_result_2_2420) & UINT64_C(0xFF)));
  scratch_push_word_be(length_word, length_len);
}

void rlp_write_string_prefix_uint8_t_uint64_t_to_unit(uint8_t len)
{
  uint64_t rlp_length_byte_native_result_2_2418 = rlp_length_byte_native(len);
  scratch_push_byte(((UINT64_C(0x80) + rlp_length_byte_native_result_2_2418) & UINT64_C(0xFF)));
}

void rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(uint8_t len, uint64_t first)
{
  bool tmp_3_3615 = (bool)((len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3615) {
    return;
  }
  uint64_t rlp_length_byte_native_result_2_2418 = rlp_length_byte_native(len);
  scratch_push_byte(((UINT64_C(0x80) + rlp_length_byte_native_result_2_2418) & UINT64_C(0xFF)));
}

void rlp_write_uint_word_u128_to_unit(u128 w)
{
  uint8_t len = rlp_minimal_word_len_u128_to_uint8_t(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte_u128_to_uint64_t(w);
  } else {
    first = UINT64_C(0x00);
  }
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(len, first);
  u256 word = u256_u128_to_u256(w);
  scratch_push_word_be(word, len);
}

void rlp_write_uint_word_uint64_t_to_unit(uint64_t w)
{
  uint8_t len = rlp_minimal_word_len_uint64_t_to_uint8_t(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte_uint64_t_to_uint64_t(w);
  } else {
    first = UINT64_C(0x00);
  }
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(len, first);
  u256 word = u256_uint64_t_to_u256(w);
  scratch_push_word_be(word, len);
}

void rlp_write_uint_word_uint8_t_to_unit(uint8_t w)
{
  uint8_t len = rlp_minimal_word_len_uint8_t_to_uint8_t(w);
  uint64_t first;
  if (len == UINT8_C(1)) {
    first = word_low_byte_uint8_t_to_uint64_t(w);
  } else {
    first = UINT64_C(0x00);
  }
  rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(len, first);
  u256 word = u256_uint8_t_to_u256(w);
  scratch_push_word_be(word, len);
}

