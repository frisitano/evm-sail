#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint64_t rlp_uint64_append(uint64_t prefix, uint64_t next)
{
  uint64_t result_8_195;
  uint8_t tmp_3_3419 = (uint8_t)next;
  result_8_195 = u128_to_u64_unchecked(u128_add_u64(u128_of_u64((prefix * UINT64_C(256))), tmp_3_3419));
  return result_8_195;
}

struct tuple_bool_uint_32_uint_64 rlp_ref_hdr(Bytes b)
{
  uint32_t source_len = b.len;
  if (source_len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  uint64_t first_byte = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(b, UINT8_C(0));
  uint8_t tmp_3_3411 = (uint8_t)first_byte;
  if (tmp_3_3411 < UINT8_C(128)) {
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = false, .tup1 = UINT32_C(0), .tup2 = UINT64_C(1)});
  }
  if (tmp_3_3411 < UINT8_C(184)) {
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = false, .tup1 = UINT32_C(1), .tup2 = ((uint64_t)tmp_3_3411 - (uint64_t)UINT8_C(128))});
  }
  if (tmp_3_3411 < UINT8_C(192)) {
    if (((uint8_t)((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(183))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
      Bytes length_bytes = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(183))));
      uint64_t rlp_uint64_width_result_2_2162 = rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes, ((uint8_t)((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(183))));
      return ((struct tuple_bool_uint_32_uint_64){.tup0 = false, .tup1 = (((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(183)) + (uint32_t)UINT8_C(1)), .tup2 = rlp_uint64_width_result_2_2162});
    }
    fatal_error(RlpDecode);
  }
  if (tmp_3_3411 < UINT8_C(248)) {
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = true, .tup1 = UINT32_C(1), .tup2 = ((uint64_t)tmp_3_3411 - (uint64_t)UINT8_C(192))});
  }
  if (((uint8_t)((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(247))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
    Bytes length_bytes_3_3418 = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(247))));
    uint64_t rlp_uint64_width_result_2_2168 = rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes_3_3418, ((uint8_t)((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(247))));
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = true, .tup1 = (((uint32_t)tmp_3_3411 - (uint32_t)UINT8_C(247)) + (uint32_t)UINT8_C(1)), .tup2 = rlp_uint64_width_result_2_2168});
  }
  fatal_error(RlpDecode);
}

Bytes rlp_decode_list(struct RlpFieldRef f)
{
  if (f.is_list) {
    return stateless_input_sub_slice(f.source, (f.source.len - f.content_len), f.content_len);
  }
  fatal_error(RlpDecode);
}

struct RlpFieldRef rlp_decode_item(Bytes cursor)
{
  if (cursor.len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  struct tuple_bool_uint_32_uint_64 rlp_ref_hdr_result_2_2137 = rlp_ref_hdr(cursor);
  uint32_t content_off = rlp_ref_hdr_result_2_2137.tup1;
  uint64_t content_len = rlp_ref_hdr_result_2_2137.tup2;
  if (cursor.len < content_off) {
    fatal_error(RlpDecode);
  }
  if (((__int128)(int64_t)cursor.len - (__int128)(int64_t)content_off) < (__int128)content_len) {
    fatal_error(RlpDecode);
  }
  u128 full_len = u128_add_u64_u64(content_off, content_len);
  bool tmp_3_3405 = (bool)(u64_lt_u128(UINT8_C(0), full_len) && (!u64_lt_u128(cursor.len, full_len)));
  if (tmp_3_3405) {
    Bytes field_source;
    uint32_t tmp_3_3813 = (uint32_t)u128_to_u64(full_len);
    field_source = stateless_input_sub_slice(cursor, UINT32_C(0), tmp_3_3813);
    struct RlpFieldRef tmp_3_3407;
    tmp_3_3407.content_len = (uint32_t)content_len;
    tmp_3_3407.is_list = rlp_ref_hdr_result_2_2137.tup0;
    tmp_3_3407.source = field_source;
    return tmp_3_3407;
  }
  fatal_error(RlpDecode);
}

Bytes rlp_cursor_advance(Bytes cursor, uint32_t consumed)
{
  return stateless_input_slice_suffix(cursor, consumed);
}

void rlp_cursor_expect_end(Bytes cursor)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  fatal_error(RlpDecode);
}

struct RlpFieldRef rlp_single_ref(Bytes item)
{
  uint32_t item_length = item.len;
  if (item_length == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  struct tuple_bool_uint_32_uint_64 rlp_ref_hdr_result_2_2129 = rlp_ref_hdr(item);
  uint64_t content_len = rlp_ref_hdr_result_2_2129.tup2;
  bool tmp_3_3394 = (bool)((rlp_ref_hdr_result_2_2129.tup1 <= item_length) && ((__int128)content_len == ((__int128)(int64_t)item_length - (__int128)(int64_t)rlp_ref_hdr_result_2_2129.tup1)));
  if (tmp_3_3394) {
    struct RlpFieldRef tmp_3_3396;
    tmp_3_3396.content_len = (uint32_t)content_len;
    tmp_3_3396.is_list = rlp_ref_hdr_result_2_2129.tup0;
    tmp_3_3396.source = item;
    return tmp_3_3396;
  }
  fatal_error(RlpDecode);
}

Bytes rlp_node_cursor(Bytes node)
{
  if (node.len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  struct RlpFieldRef item = rlp_single_ref(node);
  return rlp_decode_list(item);
}

Bytes rlp_item_content(struct RlpFieldRef f)
{
  return stateless_input_sub_slice(f.source, (f.source.len - f.content_len), f.content_len);
}

struct tuple_bool_uint_32_uint_64 scratch_rlp_ref_hdr(Bytes b)
{
  uint32_t source_len = b.len;
  if (source_len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  uint64_t first_byte = scratch_byte(b, UINT8_C(0));
  uint8_t tmp_3_3384 = (uint8_t)first_byte;
  if (tmp_3_3384 < UINT8_C(128)) {
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = false, .tup1 = UINT32_C(0), .tup2 = UINT64_C(1)});
  }
  if (tmp_3_3384 < UINT8_C(184)) {
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = false, .tup1 = UINT32_C(1), .tup2 = ((uint64_t)tmp_3_3384 - (uint64_t)UINT8_C(128))});
  }
  if (tmp_3_3384 < UINT8_C(192)) {
    if (((uint8_t)((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(183))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
      Bytes length_bytes = scratch_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(183))));
      uint64_t scratch_rlp_uint64_width_result_2_2110 = scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes, ((uint8_t)((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(183))));
      return ((struct tuple_bool_uint_32_uint_64){.tup0 = false, .tup1 = (((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(183)) + (uint32_t)UINT8_C(1)), .tup2 = scratch_rlp_uint64_width_result_2_2110});
    }
    fatal_error(RlpDecode);
  }
  if (tmp_3_3384 < UINT8_C(248)) {
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = true, .tup1 = UINT32_C(1), .tup2 = ((uint64_t)tmp_3_3384 - (uint64_t)UINT8_C(192))});
  }
  if (((uint8_t)((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(247))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
    Bytes length_bytes_3_3391 = scratch_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(247))));
    uint64_t scratch_rlp_uint64_width_result_2_2116 = scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes_3_3391, ((uint8_t)((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(247))));
    return ((struct tuple_bool_uint_32_uint_64){.tup0 = true, .tup1 = (((uint32_t)tmp_3_3384 - (uint32_t)UINT8_C(247)) + (uint32_t)UINT8_C(1)), .tup2 = scratch_rlp_uint64_width_result_2_2116});
  }
  fatal_error(RlpDecode);
}

Bytes scratch_rlp_decode_list(struct ScratchRlpFieldRef f)
{
  if (f.is_list) {
    return scratch_sub_slice(f.source, (f.source.len - f.content_len), f.content_len);
  }
  fatal_error(RlpDecode);
}

struct ScratchRlpFieldRef scratch_rlp_decode_item(Bytes cursor)
{
  if (cursor.len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  struct tuple_bool_uint_32_uint_64 scratch_rlp_ref_hdr_result_2_2085 = scratch_rlp_ref_hdr(cursor);
  uint32_t content_off = scratch_rlp_ref_hdr_result_2_2085.tup1;
  uint64_t content_len = scratch_rlp_ref_hdr_result_2_2085.tup2;
  if (cursor.len < content_off) {
    fatal_error(RlpDecode);
  }
  if (((__int128)(int64_t)cursor.len - (__int128)(int64_t)content_off) < (__int128)content_len) {
    fatal_error(RlpDecode);
  }
  u128 full_len = u128_add_u64_u64(content_off, content_len);
  bool tmp_3_3378 = (bool)(u64_lt_u128(UINT8_C(0), full_len) && (!u64_lt_u128(cursor.len, full_len)));
  if (tmp_3_3378) {
    Bytes field_source;
    uint32_t tmp_3_3814 = (uint32_t)u128_to_u64(full_len);
    field_source = scratch_sub_slice(cursor, UINT32_C(0), tmp_3_3814);
    struct ScratchRlpFieldRef tmp_3_3380;
    tmp_3_3380.content_len = (uint32_t)content_len;
    tmp_3_3380.is_list = scratch_rlp_ref_hdr_result_2_2085.tup0;
    tmp_3_3380.source = field_source;
    return tmp_3_3380;
  }
  fatal_error(RlpDecode);
}

Bytes scratch_rlp_cursor_advance(Bytes cursor, uint32_t consumed)
{
  return scratch_slice_suffix(cursor, consumed);
}

void scratch_rlp_cursor_expect_end(Bytes cursor)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  fatal_error(RlpDecode);
}

struct ScratchRlpFieldRef scratch_rlp_single_ref(Bytes item)
{
  uint32_t item_length = item.len;
  if (item_length == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  struct tuple_bool_uint_32_uint_64 scratch_rlp_ref_hdr_result_2_2077 = scratch_rlp_ref_hdr(item);
  uint64_t content_len = scratch_rlp_ref_hdr_result_2_2077.tup2;
  bool tmp_3_3367 = (bool)((scratch_rlp_ref_hdr_result_2_2077.tup1 <= item_length) && ((__int128)content_len == ((__int128)(int64_t)item_length - (__int128)(int64_t)scratch_rlp_ref_hdr_result_2_2077.tup1)));
  if (tmp_3_3367) {
    struct ScratchRlpFieldRef tmp_3_3369;
    tmp_3_3369.content_len = (uint32_t)content_len;
    tmp_3_3369.is_list = scratch_rlp_ref_hdr_result_2_2077.tup0;
    tmp_3_3369.source = item;
    return tmp_3_3369;
  }
  fatal_error(RlpDecode);
}

Bytes scratch_rlp_node_cursor(Bytes node)
{
  struct ScratchRlpFieldRef item = scratch_rlp_single_ref(node);
  return scratch_rlp_decode_list(item);
}

Bytes scratch_rlp_item_content(struct ScratchRlpFieldRef f)
{
  return scratch_sub_slice(f.source, (f.source.len - f.content_len), f.content_len);
}

u256 scratch_rlp_decode_word(struct ScratchRlpFieldRef f)
{
  uint32_t n = f.content_len;
  bool result_2_2065 = (bool)(f.is_list || (RLP_WORD_LENGTH_LIMIT < n));
  if (result_2_2065) {
    fatal_error(RlpDecode);
  }
  return scratch_slice_load_n(f.source, (f.source.len - n), (uint8_t)n);
}

bool rlp_ref_framing_canonical(struct RlpFieldRef f)
{
  if (f.source.len == UINT8_C(0)) {
    return false;
  }
  if (f.is_list) {
    bool lteq_int_result_2_2020 = (bool)(f.content_len <= RLP_SHORT_LENGTH_LIMIT);
    if (lteq_int_result_2_2020) {
      uint64_t length_byte = rlp_nat_length_byte((uint8_t)f.content_len);
      if (f.source.len == ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)(uint8_t)f.content_len))) {
        uint64_t result_2_2023 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0));
        return (bool)(result_2_2023 == ((UINT64_C(0xC0) + length_byte) & UINT64_C(0xFF)));
      }
      return false;
    }
    u256 length_word = rlp_length_word_uint32_t_to_u256(f.content_len);
    uint8_t length_width = rlp_minimal_word_len(length_word);
    uint64_t length_byte_3_3359 = rlp_nat_length_byte(length_width);
    if (f.source.len == (((uint64_t)length_width + (uint64_t)UINT8_C(1)) + (uint64_t)f.content_len)) {
      uint64_t result_2_2029 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0));
      if (result_2_2029 == ((UINT64_C(0xF7) + length_byte_3_3359) & UINT64_C(0xFF))) {
        u256 result_2_2033 = stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(f.source, UINT8_C(1), length_width);
        return eq_u256(result_2_2033, length_word);
      }
      return false;
    }
    return false;
  }
  if (f.content_len == UINT8_C(0)) {
    if (f.source.len == UINT8_C(1)) {
      uint64_t result_2_2038 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0));
      return (bool)(result_2_2038 == UINT64_C(0x80));
    }
    return false;
  }
  uint64_t first = stateless_input_slice_byte(f.source, (f.source.len - f.content_len));
  bool tmp_3_3350 = (bool)((f.content_len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0)));
  if (tmp_3_3350) {
    if (f.source.len == UINT8_C(1)) {
      uint64_t result_2_2044 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0));
      return (bool)(result_2_2044 == first);
    }
    return false;
  }
  bool lteq_int_result_2_2046 = (bool)(f.content_len <= RLP_SHORT_LENGTH_LIMIT);
  if (lteq_int_result_2_2046) {
    uint64_t length_byte_3_3360 = rlp_nat_length_byte((uint8_t)f.content_len);
    if (f.source.len == ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)(uint8_t)f.content_len))) {
      uint64_t result_2_2049 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0));
      return (bool)(result_2_2049 == ((UINT64_C(0x80) + length_byte_3_3360) & UINT64_C(0xFF)));
    }
    return false;
  }
  u256 length_word_3_3361 = rlp_length_word_uint32_t_to_u256(f.content_len);
  uint8_t length_width_3_3362 = rlp_minimal_word_len(length_word_3_3361);
  uint64_t length_byte_3_3363 = rlp_nat_length_byte(length_width_3_3362);
  if (f.source.len == (((uint64_t)length_width_3_3362 + (uint64_t)UINT8_C(1)) + (uint64_t)f.content_len)) {
    uint64_t result_2_2055 = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0));
    if (result_2_2055 == ((UINT64_C(0xB7) + length_byte_3_3363) & UINT64_C(0xFF))) {
      u256 result_2_2059 = stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(f.source, UINT8_C(1), length_width_3_3362);
      return eq_u256(result_2_2059, length_word_3_3361);
    }
    return false;
  }
  return false;
}

bool rlp_ref_bytes_canonical(struct RlpFieldRef f)
{
  if (f.is_list) {
    return false;
  }
  return rlp_ref_framing_canonical(f);
}

bool rlp_item_uint_canonical(struct RlpFieldRef f)
{
  bool bytes_canonical = rlp_ref_bytes_canonical(f);
  if (bytes_canonical) {
    if (f.content_len == UINT8_C(0)) {
      return true;
    }
    uint64_t result_2_2014 = stateless_input_slice_byte(f.source, (f.source.len - f.content_len));
    return (bool)(result_2_2014 != UINT64_C(0x00));
  }
  return false;
}

u256 rlp_decode_word(struct RlpFieldRef f)
{
  uint32_t n = f.content_len;
  bool result_2_2003 = (bool)(f.is_list || (RLP_WORD_LENGTH_LIMIT < n));
  if (result_2_2003) {
    fatal_error(RlpDecode);
  }
  return stateless_input_slice_load_n(f.source, (f.source.len - n), (uint8_t)n);
}

u256 rlp_decode_u256(struct RlpFieldRef f)
{
  bool canonical = rlp_item_uint_canonical(f);
  if (canonical) {
    return rlp_decode_word(f);
  }
  fatal_error(RlpDecode);
}

uint64_t rlp_decode_uint64(struct RlpFieldRef f)
{
  bool canonical = rlp_item_uint_canonical(f);
  bool tmp_3_3344 = (bool)(!canonical || (RLP_UINT64_LENGTH_LIMIT < f.content_len));
  if (tmp_3_3344) {
    fatal_error(RlpDecode);
  }
  Bytes content = rlp_item_content(f);
  uint8_t tmp_3_3346 = (uint8_t)(f.content_len % UINT32_C(9));
  return rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, tmp_3_3346);
}

uint64_t rlp_uint64_width_Bytes_uint8_t_to_uint64_t(Bytes content, uint8_t width)
{
  if (width == UINT8_C(0)) {
    return UINT64_C(0);
  }
  uint64_t prefix = rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  uint64_t next = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  return rlp_uint64_append(prefix, next);
}

uint64_t rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(Bytes content, uint8_t width)
{
  uint64_t prefix = rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  uint64_t next = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  return rlp_uint64_append(prefix, next);
}

uint64_t scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t(Bytes content, uint8_t width)
{
  if (width == UINT8_C(0)) {
    return UINT64_C(0);
  }
  uint64_t prefix = scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  uint64_t next = scratch_byte(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  return rlp_uint64_append(prefix, next);
}

uint64_t scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(Bytes content, uint8_t width)
{
  uint64_t prefix = scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  uint64_t next = scratch_byte(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  return rlp_uint64_append(prefix, next);
}

