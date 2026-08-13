#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t HEX_PREFIX_MAX_LENGTH = UINT8_C(33);


struct TriePath path_new(bytes32 data, uint8_t len)
{
  return ((struct TriePath){.data = data, .len = len});
}

struct tuple_bool_TriePath hex_prefix_decode_ref(struct RlpFieldRef f)
{
  if (f.is_list) {
    fatal_error(RlpDecode);
  }
  if (f.content_len == UINT8_C(0)) {
    struct TriePath tmp_3_3447;
    vector_32_bits_8 tmp_3_3448 = internal_vector_init_vector_32_bits_8(INT64_C(32));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(0), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(1), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(2), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(3), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(4), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(5), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(6), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(7), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(8), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(9), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(10), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(11), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(12), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(13), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(14), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(15), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(16), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(17), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(18), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(19), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(20), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(21), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(22), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(23), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(24), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(25), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(26), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(27), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(28), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(29), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(30), UINT64_C(0x00));
    tmp_3_3448 = internal_vector_update_vector_32_bits_8(tmp_3_3448, INT64_C(31), UINT64_C(0x00));
    tmp_3_3447.data = bytes32_zero();
    for (size_t tmp_8_1565 = 0; tmp_8_1565 < 32; ++tmp_8_1565) {
      tmp_3_3447.data = fast_unsigned_vector_update_bytes32(tmp_3_3447.data, tmp_8_1565, tmp_3_3448.data[tmp_8_1565]);
    }
    tmp_3_3447.len = UINT8_C(0);
    return ((struct tuple_bool_TriePath){.tup0 = false, .tup1 = tmp_3_3447});
  }
  uint8_t maximum_length = HEX_PREFIX_MAX_LENGTH;
  if (maximum_length < f.content_len) {
    fatal_error(RlpDecode);
  }
  Bytes content = stateless_input_sub_slice(f.source, (f.source.len - f.content_len), f.content_len);
  uint64_t fb = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(content, UINT8_C(0));
  Bytes tail = stateless_input_slice_suffix_Bytes_uint8_t_to_Bytes(content, UINT8_C(1));
  u256 packed = stateless_input_slice_load_Bytes_uint8_t_to_u256(tail, UINT8_C(0));
  if ((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(0))) == UINT64_C(0x1)) {
    if (((uint8_t)((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1)))) < UINT8_C(64)) {
      u256 shifted = word_shift_right_u256_uint8_t_to_u256(packed, UINT8_C(4));
      bytes32 bytes = word_to_hash(shifted);
      uint64_t plain_vector_access_result_2_2151 = fast_unsigned_vector_access_bytes32(bytes, UINT8_C(0));
      bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(0), ((UINT64_C(0xF) & (fb >> UINT8_C(0))) << 4) | (UINT64_C(0xF) & (plain_vector_access_result_2_2151 >> UINT8_C(0))));
      bytes32 path_data = B256(bytes);
      struct TriePath path = path_new(path_data, ((uint8_t)((uint32_t)UINT8_C(1) + ((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1))))));
      return ((struct tuple_bool_TriePath){.tup0 = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1)), .tup1 = path});
    }
    fatal_error(WitnessDeficient);
  }
  bytes32 path_data_3_3450 = word_to_hash(packed);
  struct TriePath path_3_3451 = path_new(path_data_3_3450, ((uint8_t)((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1)))));
  return ((struct tuple_bool_TriePath){.tup0 = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1)), .tup1 = path_3_3451});
}

uint8_t path_len(struct TriePath path)
{
  return path.len;
}

uint64_t path_nibble(struct TriePath path, uint8_t i)
{
  uint8_t length_ = path_len(path);
  if (length_ <= i) {
    return UINT64_C(0x0);
  }
  uint8_t byte_index = path_byte_index(i);
  if ((i & UINT8_C(1)) == UINT8_C(0)) {
    uint64_t plain_vector_access_result_2_2131 = fast_unsigned_vector_access_bytes32(path.data, byte_index);
    return (UINT64_C(0xF) & (plain_vector_access_result_2_2131 >> UINT8_C(4)));
  }
  uint64_t plain_vector_access_result_2_2132 = fast_unsigned_vector_access_bytes32(path.data, byte_index);
  return (UINT64_C(0xF) & (plain_vector_access_result_2_2132 >> UINT8_C(0)));
}

struct TriePath path_append_nibble(struct TriePath path, uint64_t value)
{
  uint8_t length_ = path_len(path);
  if (length_ < UINT8_C(64)) {
    bytes32 bytes = path.data;
    uint8_t byte_index = path_byte_index(length_);
    if ((length_ & UINT8_C(1)) == UINT8_C(0)) {
      bytes = fast_unsigned_vector_update_bytes32(bytes, byte_index, (value << 4) | UINT64_C(0x0));
    } else {
      uint64_t plain_vector_access_result_2_634 = fast_unsigned_vector_access_bytes32(bytes, byte_index);
      bytes = fast_unsigned_vector_update_bytes32(bytes, byte_index, ((UINT64_C(0xF) & (plain_vector_access_result_2_634 >> UINT8_C(4))) << 4) | value);
    }
    bytes32 path_data = B256(bytes);
    return path_new(path_data, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)length_)));
  }
  fatal_error(WitnessDeficient);
}

struct TriePath path_single(uint64_t n)
{
  struct TriePath tmp_3_1038;
  vector_32_bits_8 tmp_3_1039 = internal_vector_init_vector_32_bits_8(INT64_C(32));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(0), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(1), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(2), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(3), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(4), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(5), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(6), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(7), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(8), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(9), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(10), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(11), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(12), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(13), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(14), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(15), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(16), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(17), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(18), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(19), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(20), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(21), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(22), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(23), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(24), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(25), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(26), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(27), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(28), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(29), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(30), UINT64_C(0x00));
  tmp_3_1039 = internal_vector_update_vector_32_bits_8(tmp_3_1039, INT64_C(31), UINT64_C(0x00));
  tmp_3_1038.data = bytes32_zero();
  for (size_t tmp_8_1566 = 0; tmp_8_1566 < 32; ++tmp_8_1566) {
    tmp_3_1038.data = fast_unsigned_vector_update_bytes32(tmp_3_1038.data, tmp_8_1566, tmp_3_1039.data[tmp_8_1566]);
  }
  tmp_3_1038.len = UINT8_C(0);
  return path_append_nibble(tmp_3_1038, n);
}

struct TriePath path_concat(struct TriePath a, struct TriePath b)
{
  uint8_t alen = path_len(a);
  uint8_t blen = path_len(b);
  bool lteq_int_result_2_629 = (bool)(((uint8_t)((uint32_t)blen + (uint32_t)alen)) <= UINT8_C(64));
  if (!lteq_int_result_2_629) {
    fatal_error(WitnessDeficient);
  }
  struct TriePath result = a;
  uint8_t index = UINT8_C(0);
  while (index < blen) {
    uint64_t nibble = path_nibble(b, index);
    result = path_append_nibble(result, nibble);
    uint8_t current_index = index;
    if (current_index < UINT8_C(64)) {
      index = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)current_index));
    } else {
      fatal_error(WitnessDeficient);
    }
  }
  return result;
}

struct TriePath path_drop(struct TriePath path, uint8_t n)
{
  uint8_t length_ = path_len(path);
  if (length_ <= n) {
    struct TriePath tmp_3_1032;
    vector_32_bits_8 tmp_3_1033 = internal_vector_init_vector_32_bits_8(INT64_C(32));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(0), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(1), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(2), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(3), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(4), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(5), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(6), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(7), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(8), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(9), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(10), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(11), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(12), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(13), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(14), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(15), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(16), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(17), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(18), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(19), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(20), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(21), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(22), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(23), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(24), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(25), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(26), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(27), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(28), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(29), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(30), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(31), UINT64_C(0x00));
    tmp_3_1032.data = bytes32_zero();
    for (size_t tmp_8_1568 = 0; tmp_8_1568 < 32; ++tmp_8_1568) {
      tmp_3_1032.data = fast_unsigned_vector_update_bytes32(tmp_3_1032.data, tmp_8_1568, tmp_3_1033.data[tmp_8_1568]);
    }
    tmp_3_1032.len = UINT8_C(0);
    return tmp_3_1032;
  }
  if (n == UINT8_C(0)) {
    return path;
  }
  uint8_t remain = ((uint8_t)((uint32_t)length_ - (uint32_t)n));
  struct TriePath result;
  struct TriePath tmp_3_1024;
  vector_32_bits_8 tmp_3_1025 = internal_vector_init_vector_32_bits_8(INT64_C(32));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(0), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(1), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(2), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(3), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(4), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(5), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(6), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(7), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(8), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(9), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(10), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(11), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(12), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(13), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(14), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(15), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(16), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(17), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(18), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(19), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(20), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(21), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(22), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(23), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(24), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(25), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(26), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(27), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(28), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(29), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(30), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(31), UINT64_C(0x00));
  tmp_3_1024.data = bytes32_zero();
  for (size_t tmp_8_1567 = 0; tmp_8_1567 < 32; ++tmp_8_1567) {
    tmp_3_1024.data = fast_unsigned_vector_update_bytes32(tmp_3_1024.data, tmp_8_1567, tmp_3_1025.data[tmp_8_1567]);
  }
  tmp_3_1024.len = UINT8_C(0);
  result = tmp_3_1024;
  uint8_t offset = UINT8_C(0);
  while (offset < remain) {
    uint8_t candidate = ((uint8_t)((uint32_t)offset + (uint32_t)n));
    if (candidate > UINT8_C(64)) {
      __builtin_trap();
    }
    uint64_t nibble = path_nibble(path, candidate);
    result = path_append_nibble(result, nibble);
    uint8_t current_offset = offset;
    if (current_offset < UINT8_C(64)) {
      offset = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)current_offset));
    } else {
      fatal_error(WitnessDeficient);
    }
  }
  return result;
}

bool path_eq(struct TriePath a, struct TriePath b)
{
  if (a.len == b.len) {
    return eq_bytes32(a.data, b.data);
  }
  return false;
}

bool path_prefix_of(struct TriePath prefix, struct TriePath path)
{
  return path_matches(path, UINT8_C(0), prefix);
}

uint8_t common_prefix_length(struct TriePath a, struct TriePath b)
{
  uint8_t alen = path_len(a);
  uint8_t blen = path_len(b);
  uint8_t stop = alen < blen ? alen : blen;
  uint8_t length_ = UINT8_C(0);
  bool matching = true;
  while (matching && (length_ < stop)) {
    uint64_t a_nibble = path_nibble(a, length_);
    uint64_t b_nibble = path_nibble(b, length_);
    if (a_nibble == b_nibble) {
      uint8_t current_length = length_;
      if (current_length < UINT8_C(64)) {
        length_ = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)current_length));
      } else {
        fatal_error(WitnessDeficient);
      }
    } else {
      matching = false;
    }
  }
  return length_;
}

uint8_t hex_prefix_encoded_length(struct TriePath path)
{
  uint8_t length_ = path_len(path);
  return ((uint8_t)((uint32_t)(length_ >> 1) + (uint32_t)UINT8_C(1)));
}

uint64_t hex_prefix_first_byte(struct TriePath path, bool is_leaf)
{
  uint8_t length_ = path_len(path);
  uint64_t flag = is_leaf ? UINT64_C(0x2) : UINT64_C(0x0);
  if ((length_ & UINT8_C(1)) != UINT8_C(0)) {
    uint64_t first_nibble = path_nibble_struct_TriePath_uint8_t_to_uint64_t(path, UINT8_C(0));
    return ((flag | UINT64_C(0x1)) << 4) | first_nibble;
  }
  return (flag << 4) | UINT64_C(0x0);
}

struct tuple_bool_TriePath scratch_hex_prefix_decode_ref(struct ScratchRlpFieldRef f)
{
  if (f.is_list) {
    fatal_error(RlpDecode);
  }
  if (f.content_len == UINT8_C(0)) {
    struct TriePath tmp_3_1014;
    vector_32_bits_8 tmp_3_1015 = internal_vector_init_vector_32_bits_8(INT64_C(32));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(0), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(1), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(2), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(3), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(4), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(5), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(6), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(7), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(8), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(9), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(10), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(11), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(12), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(13), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(14), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(15), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(16), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(17), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(18), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(19), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(20), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(21), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(22), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(23), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(24), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(25), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(26), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(27), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(28), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(29), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(30), UINT64_C(0x00));
    tmp_3_1015 = internal_vector_update_vector_32_bits_8(tmp_3_1015, INT64_C(31), UINT64_C(0x00));
    tmp_3_1014.data = bytes32_zero();
    for (size_t tmp_8_1569 = 0; tmp_8_1569 < 32; ++tmp_8_1569) {
      tmp_3_1014.data = fast_unsigned_vector_update_bytes32(tmp_3_1014.data, tmp_8_1569, tmp_3_1015.data[tmp_8_1569]);
    }
    tmp_3_1014.len = UINT8_C(0);
    return ((struct tuple_bool_TriePath){.tup0 = false, .tup1 = tmp_3_1014});
  }
  uint8_t maximum_length = HEX_PREFIX_MAX_LENGTH;
  if (maximum_length < f.content_len) {
    fatal_error(RlpDecode);
  }
  Bytes content = scratch_sub_slice(f.source, (f.source.len - f.content_len), f.content_len);
  uint64_t fb = scratch_byte(content, UINT8_C(0));
  Bytes tail = scratch_slice_suffix_Bytes_uint8_t_to_Bytes(content, UINT8_C(1));
  u256 packed = scratch_slice_load(tail, UINT8_C(0));
  if ((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(0))) == UINT64_C(0x1)) {
    if (((uint8_t)((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1)))) < UINT8_C(64)) {
      u256 shifted = word_shift_right_u256_uint8_t_to_u256(packed, UINT8_C(4));
      bytes32 bytes = word_to_hash(shifted);
      uint64_t plain_vector_access_result_2_603 = fast_unsigned_vector_access_bytes32(bytes, UINT8_C(0));
      bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(0), ((UINT64_C(0xF) & (fb >> UINT8_C(0))) << 4) | (UINT64_C(0xF) & (plain_vector_access_result_2_603 >> UINT8_C(0))));
      bytes32 path_data = B256(bytes);
      struct TriePath path = path_new(path_data, ((uint8_t)((uint32_t)UINT8_C(1) + ((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1))))));
      return ((struct tuple_bool_TriePath){.tup0 = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1)), .tup1 = path});
    }
    fatal_error(WitnessDeficient);
  }
  bytes32 path_data_3_1017 = word_to_hash(packed);
  struct TriePath path_3_1018 = path_new(path_data_3_1017, ((uint8_t)((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1)))));
  return ((struct tuple_bool_TriePath){.tup0 = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1)), .tup1 = path_3_1018});
}

uint8_t path_byte_index(uint8_t i)
{
  return (i >> 1);
}

struct TriePath path_drop_struct_TriePath_uint8_t_to_struct_TriePath(struct TriePath path, uint8_t n)
{
  uint8_t length_ = path_len(path);
  if (length_ <= n) {
    struct TriePath tmp_3_1032;
    vector_32_bits_8 tmp_3_1033 = internal_vector_init_vector_32_bits_8(INT64_C(32));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(0), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(1), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(2), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(3), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(4), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(5), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(6), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(7), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(8), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(9), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(10), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(11), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(12), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(13), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(14), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(15), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(16), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(17), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(18), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(19), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(20), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(21), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(22), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(23), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(24), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(25), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(26), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(27), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(28), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(29), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(30), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(31), UINT64_C(0x00));
    tmp_3_1032.data = bytes32_zero();
    for (size_t tmp_8_1571 = 0; tmp_8_1571 < 32; ++tmp_8_1571) {
      tmp_3_1032.data = fast_unsigned_vector_update_bytes32(tmp_3_1032.data, tmp_8_1571, tmp_3_1033.data[tmp_8_1571]);
    }
    tmp_3_1032.len = UINT8_C(0);
    return tmp_3_1032;
  }
  uint8_t remain = ((uint8_t)((uint32_t)length_ - (uint32_t)n));
  struct TriePath result;
  struct TriePath tmp_3_1024;
  vector_32_bits_8 tmp_3_1025 = internal_vector_init_vector_32_bits_8(INT64_C(32));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(0), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(1), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(2), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(3), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(4), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(5), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(6), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(7), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(8), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(9), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(10), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(11), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(12), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(13), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(14), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(15), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(16), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(17), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(18), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(19), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(20), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(21), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(22), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(23), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(24), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(25), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(26), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(27), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(28), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(29), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(30), UINT64_C(0x00));
  tmp_3_1025 = internal_vector_update_vector_32_bits_8(tmp_3_1025, INT64_C(31), UINT64_C(0x00));
  tmp_3_1024.data = bytes32_zero();
  for (size_t tmp_8_1570 = 0; tmp_8_1570 < 32; ++tmp_8_1570) {
    tmp_3_1024.data = fast_unsigned_vector_update_bytes32(tmp_3_1024.data, tmp_8_1570, tmp_3_1025.data[tmp_8_1570]);
  }
  tmp_3_1024.len = UINT8_C(0);
  result = tmp_3_1024;
  uint8_t offset = UINT8_C(0);
  while (offset < remain) {
    uint8_t candidate = ((uint8_t)((uint32_t)offset + (uint32_t)n));
    if (candidate > UINT8_C(64)) {
      __builtin_trap();
    }
    uint64_t nibble = path_nibble(path, candidate);
    result = path_append_nibble(result, nibble);
    uint8_t current_offset = offset;
    if (current_offset < UINT8_C(64)) {
      offset = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)current_offset));
    } else {
      fatal_error(WitnessDeficient);
    }
  }
  return result;
}

struct TriePath path_drop_struct_TriePath_uint8_t_to_struct_TriePath_variant_2(struct TriePath path, uint8_t n)
{
  uint8_t length_ = path_len(path);
  if (length_ <= n) {
    struct TriePath tmp_3_1032;
    vector_32_bits_8 tmp_3_1033 = internal_vector_init_vector_32_bits_8(INT64_C(32));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(0), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(1), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(2), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(3), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(4), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(5), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(6), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(7), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(8), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(9), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(10), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(11), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(12), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(13), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(14), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(15), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(16), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(17), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(18), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(19), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(20), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(21), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(22), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(23), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(24), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(25), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(26), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(27), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(28), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(29), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(30), UINT64_C(0x00));
    tmp_3_1033 = internal_vector_update_vector_32_bits_8(tmp_3_1033, INT64_C(31), UINT64_C(0x00));
    tmp_3_1032.data = bytes32_zero();
    for (size_t tmp_8_1572 = 0; tmp_8_1572 < 32; ++tmp_8_1572) {
      tmp_3_1032.data = fast_unsigned_vector_update_bytes32(tmp_3_1032.data, tmp_8_1572, tmp_3_1033.data[tmp_8_1572]);
    }
    tmp_3_1032.len = UINT8_C(0);
    return tmp_3_1032;
  }
  return path;
}

bool path_matches(struct TriePath key, uint8_t pos, struct TriePath seg)
{
  uint8_t segment_len = path_len(seg);
  uint8_t key_len = path_len(key);
  if (key_len < ((uint8_t)((uint32_t)segment_len + (uint32_t)pos))) {
    return false;
  }
  bool ok = true;
  uint8_t offset = UINT8_C(0);
  while (ok && (offset < segment_len)) {
    uint8_t key_index = ((uint8_t)((uint32_t)offset + (uint32_t)pos));
    if (key_index <= UINT8_C(64)) {
      uint64_t key_nibble = path_nibble(key, key_index);
      uint64_t segment_nibble = path_nibble(seg, offset);
      if (key_nibble != segment_nibble) {
        ok = false;
      }
    } else {
      ok = false;
    }
    uint8_t current_offset = offset;
    if (current_offset < UINT8_C(64)) {
      offset = ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)current_offset));
    } else {
      fatal_error(WitnessDeficient);
    }
  }
  return ok;
}

uint64_t path_nibble_struct_TriePath_uint8_t_to_uint64_t(struct TriePath path, uint8_t i)
{
  uint8_t length_ = path_len(path);
  if (length_ <= i) {
    return UINT64_C(0x0);
  }
  uint8_t byte_index = path_byte_index(i);
  uint64_t plain_vector_access_result_2_2131 = fast_unsigned_vector_access_bytes32(path.data, byte_index);
  return (UINT64_C(0xF) & (plain_vector_access_result_2_2131 >> UINT8_C(4)));
}

