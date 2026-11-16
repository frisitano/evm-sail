#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint64_t rlp_uint64_append(uint64_t prefix, uint64_t next)
{
  return (uint64_t)u128_to_u64_unchecked(u128_add_u64(u128_of_u64((prefix * UINT64_C(256))), (uint8_t)next));
}

uint64_t rlp_ref_hdr(Bytes b, bool *restrict condition_8_1415, uint32_t *restrict field_1_8_1416)
{
  uint32_t source_len = b.len;
  if (source_len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  uint64_t first_byte = stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(b, UINT8_C(0));
  if ((uint8_t)first_byte < UINT8_C(128)) {
    (*condition_8_1415) = false;
    (*field_1_8_1416) = UINT32_C(0);
    return UINT64_C(1);
  }
  if ((uint8_t)first_byte < UINT8_C(184)) {
    (*condition_8_1415) = false;
    (*field_1_8_1416) = UINT32_C(1);
    return ((uint64_t)(uint8_t)first_byte - (uint64_t)UINT8_C(128));
  }
  if ((uint8_t)first_byte < UINT8_C(192)) {
    if (((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
      Bytes length_bytes = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183))));
      (*condition_8_1415) = false;
      (*field_1_8_1416) = (((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183)) + (uint32_t)UINT8_C(1));
      return rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes, ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183))));
    }
    fatal_error(RlpDecode);
  }
  if ((uint8_t)first_byte < UINT8_C(248)) {
    (*condition_8_1415) = true;
    (*field_1_8_1416) = UINT32_C(1);
    return ((uint64_t)(uint8_t)first_byte - (uint64_t)UINT8_C(192));
  }
  if (((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
    Bytes length_bytes_3_3547 = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247))));
    (*condition_8_1415) = true;
    (*field_1_8_1416) = (((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247)) + (uint32_t)UINT8_C(1));
    return rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes_3_3547, ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247))));
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
  bool _8_1415_8_1639;
  uint32_t content_off;
  uint64_t content_len = rlp_ref_hdr(cursor, &_8_1415_8_1639, &content_off);
  if (cursor.len < content_off) {
    fatal_error(RlpDecode);
  }
  if (((__int128)(int64_t)cursor.len - (__int128)(int64_t)content_off) < (__int128)content_len) {
    fatal_error(RlpDecode);
  }
  if (u64_lt_u128(UINT8_C(0), u128_add_u64_u64(content_off, content_len)) && (!u64_lt_u128(cursor.len, u128_add_u64_u64(content_off, content_len)))) {
    return ((struct RlpFieldRef){.content_len = (uint32_t)content_len, .is_list = _8_1415_8_1639, .source = (stateless_input_sub_slice(cursor, UINT32_C(0), (uint32_t)u128_to_u64_unchecked(u128_add_u64_u64(content_off, content_len))))});
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
  bool _8_1415_8_1640;
  uint32_t _8_1416_8_1641;
  uint64_t _8_1417_8_1642 = rlp_ref_hdr(item, &_8_1415_8_1640, &_8_1416_8_1641);
  if ((_8_1416_8_1641 <= item_length) && ((__int128)_8_1417_8_1642 == ((__int128)(int64_t)item_length - (__int128)(int64_t)_8_1416_8_1641))) {
    return ((struct RlpFieldRef){.content_len = (uint32_t)_8_1417_8_1642, .is_list = _8_1415_8_1640, .source = item});
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

uint64_t scratch_rlp_ref_hdr(Bytes b, bool *restrict condition_8_1418, uint32_t *restrict field_1_8_1419)
{
  uint32_t source_len = b.len;
  if (source_len == UINT8_C(0)) {
    fatal_error(RlpDecode);
  }
  uint64_t first_byte = scratch_byte(b, UINT8_C(0));
  if ((uint8_t)first_byte < UINT8_C(128)) {
    (*condition_8_1418) = false;
    (*field_1_8_1419) = UINT32_C(0);
    return UINT64_C(1);
  }
  if ((uint8_t)first_byte < UINT8_C(184)) {
    (*condition_8_1418) = false;
    (*field_1_8_1419) = UINT32_C(1);
    return ((uint64_t)(uint8_t)first_byte - (uint64_t)UINT8_C(128));
  }
  if ((uint8_t)first_byte < UINT8_C(192)) {
    if (((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
      Bytes length_bytes = scratch_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183))));
      (*condition_8_1418) = false;
      (*field_1_8_1419) = (((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183)) + (uint32_t)UINT8_C(1));
      return scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes, ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(183))));
    }
    fatal_error(RlpDecode);
  }
  if ((uint8_t)first_byte < UINT8_C(248)) {
    (*condition_8_1418) = true;
    (*field_1_8_1419) = UINT32_C(1);
    return ((uint64_t)(uint8_t)first_byte - (uint64_t)UINT8_C(192));
  }
  if (((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247))) <= ((int64_t)source_len - (int64_t)UINT8_C(1))) {
    Bytes length_bytes_3_3520 = scratch_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(b, UINT8_C(1), ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247))));
    (*condition_8_1418) = true;
    (*field_1_8_1419) = (((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247)) + (uint32_t)UINT8_C(1));
    return scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(length_bytes_3_3520, ((uint8_t)((uint32_t)(uint8_t)first_byte - (uint32_t)UINT8_C(247))));
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
  bool _8_1418_8_1643;
  uint32_t content_off;
  uint64_t content_len = scratch_rlp_ref_hdr(cursor, &_8_1418_8_1643, &content_off);
  if (cursor.len < content_off) {
    fatal_error(RlpDecode);
  }
  if (((__int128)(int64_t)cursor.len - (__int128)(int64_t)content_off) < (__int128)content_len) {
    fatal_error(RlpDecode);
  }
  if (u64_lt_u128(UINT8_C(0), u128_add_u64_u64(content_off, content_len)) && (!u64_lt_u128(cursor.len, u128_add_u64_u64(content_off, content_len)))) {
    return ((struct ScratchRlpFieldRef){.content_len = (uint32_t)content_len, .is_list = _8_1418_8_1643, .source = (scratch_sub_slice(cursor, UINT32_C(0), (uint32_t)u128_to_u64_unchecked(u128_add_u64_u64(content_off, content_len))))});
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
  bool _8_1418_8_1644;
  uint32_t _8_1419_8_1645;
  uint64_t _8_1420_8_1646 = scratch_rlp_ref_hdr(item, &_8_1418_8_1644, &_8_1419_8_1645);
  if ((_8_1419_8_1645 <= item_length) && ((__int128)_8_1420_8_1646 == ((__int128)(int64_t)item_length - (__int128)(int64_t)_8_1419_8_1645))) {
    return ((struct ScratchRlpFieldRef){.content_len = (uint32_t)_8_1420_8_1646, .is_list = _8_1418_8_1644, .source = item});
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
  if (f.is_list || (RLP_WORD_LENGTH_LIMIT < f.content_len)) {
    fatal_error(RlpDecode);
  }
  return scratch_slice_load_n(f.source, (f.source.len - f.content_len), (uint8_t)f.content_len);
}

bool rlp_ref_framing_canonical(struct RlpFieldRef f)
{
  if (f.source.len == UINT8_C(0)) {
    return false;
  }
  if (f.is_list) {
    if (f.content_len <= RLP_SHORT_LENGTH_LIMIT) {
      uint64_t length_byte = rlp_nat_length_byte((uint8_t)f.content_len);
      if (f.source.len == ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)(uint8_t)f.content_len))) {
        return (bool)((stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0))) == ((UINT64_C(0xC0) + length_byte) & UINT64_C(0xFF)));
      }
      return false;
    }
    u256 length_word = rlp_length_word_uint32_t_to_u256(f.content_len);
    uint8_t length_width = rlp_minimal_word_len(length_word);
    uint64_t length_byte_3_3488 = rlp_nat_length_byte(length_width);
    if (f.source.len == (((uint64_t)length_width + (uint64_t)UINT8_C(1)) + (uint64_t)f.content_len)) {
      if ((stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0))) == ((UINT64_C(0xF7) + length_byte_3_3488) & UINT64_C(0xFF))) {
        return eq_u256((stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(f.source, UINT8_C(1), length_width)), length_word);
      }
      return false;
    }
    return false;
  }
  if (f.content_len == UINT8_C(0)) {
    if (f.source.len == UINT8_C(1)) {
      return (bool)((stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0))) == UINT64_C(0x80));
    }
    return false;
  }
  uint64_t first = stateless_input_slice_byte(f.source, (f.source.len - f.content_len));
  if ((f.content_len == UINT8_C(1)) && ((UINT64_C(1) & (first >> UINT8_C(7))) == UINT64_C(0x0))) {
    if (f.source.len == UINT8_C(1)) {
      return (bool)((stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0))) == first);
    }
    return false;
  }
  if (f.content_len <= RLP_SHORT_LENGTH_LIMIT) {
    uint64_t length_byte_3_3489 = rlp_nat_length_byte((uint8_t)f.content_len);
    if (f.source.len == ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)(uint8_t)f.content_len))) {
      return (bool)((stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0))) == ((UINT64_C(0x80) + length_byte_3_3489) & UINT64_C(0xFF)));
    }
    return false;
  }
  u256 length_word_3_3490 = rlp_length_word_uint32_t_to_u256(f.content_len);
  uint8_t length_width_3_3491 = rlp_minimal_word_len(length_word_3_3490);
  uint64_t length_byte_3_3492 = rlp_nat_length_byte(length_width_3_3491);
  if (f.source.len == (((uint64_t)length_width_3_3491 + (uint64_t)UINT8_C(1)) + (uint64_t)f.content_len)) {
    if ((stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(f.source, UINT8_C(0))) == ((UINT64_C(0xB7) + length_byte_3_3492) & UINT64_C(0xFF))) {
      return eq_u256((stateless_input_slice_load_n_Bytes_uint8_t_uint8_t_to_u256(f.source, UINT8_C(1), length_width_3_3491)), length_word_3_3490);
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
  if (rlp_ref_bytes_canonical(f)) {
    if (f.content_len == UINT8_C(0)) {
      return true;
    }
    return (bool)((stateless_input_slice_byte(f.source, (f.source.len - f.content_len))) != UINT64_C(0x00));
  }
  return false;
}

u256 rlp_decode_word(struct RlpFieldRef f)
{
  if (f.is_list || (RLP_WORD_LENGTH_LIMIT < f.content_len)) {
    fatal_error(RlpDecode);
  }
  return stateless_input_slice_load_n(f.source, (f.source.len - f.content_len), (uint8_t)f.content_len);
}

u256 rlp_decode_u256(struct RlpFieldRef f)
{
  if (rlp_item_uint_canonical(f)) {
    return rlp_decode_word(f);
  }
  fatal_error(RlpDecode);
}

uint64_t rlp_decode_uint64(struct RlpFieldRef f)
{
  bool canonical = rlp_item_uint_canonical(f);
  if (!canonical || (RLP_UINT64_LENGTH_LIMIT < f.content_len)) {
    fatal_error(RlpDecode);
  }
  return rlp_uint64_width_Bytes_uint8_t_to_uint64_t((rlp_item_content(f)), (uint8_t)(f.content_len % UINT32_C(9)));
}

uint64_t rlp_uint64_width_Bytes_uint8_t_to_uint64_t(Bytes content, uint8_t width)
{
  if (width == UINT8_C(0)) {
    return UINT64_C(0);
  }
  uint64_t prefix = rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  return rlp_uint64_append(prefix, (stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))))));
}

uint64_t rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(Bytes content, uint8_t width)
{
  uint64_t prefix = rlp_uint64_width_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))));
  return rlp_uint64_append(prefix, (stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(content, ((uint8_t)((uint32_t)width - (uint32_t)UINT8_C(1))))));
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

