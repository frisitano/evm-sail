#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t HEX_PREFIX_MAX_LENGTH = UINT8_C(33);


struct TriePath path_new(bytes32 data, uint8_t len)
{
  return ((struct TriePath){.data = data, .len = len});
}

struct TriePath hex_prefix_decode_ref(struct RlpFieldRef f, bool *restrict condition_8_1361)
{
  if (f.is_list) {
    fatal_error(RlpDecode);
  }
  if (f.content_len == UINT8_C(0)) {
    (*condition_8_1361) = false;
    return ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
  }
  if (HEX_PREFIX_MAX_LENGTH < f.content_len) {
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
      bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(0), ((UINT64_C(0xF) & (fb >> UINT8_C(0))) << 4) | (UINT64_C(0xF) & ((fast_unsigned_vector_access_bytes32(bytes, UINT8_C(0))) >> UINT8_C(0))));
      bytes32 path_data = B256(bytes);
      (*condition_8_1361) = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1));
      return path_new(path_data, ((uint8_t)((uint32_t)UINT8_C(1) + ((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1))))));
    }
    fatal_error(WitnessDeficient);
  }
  bytes32 path_data_3_3192 = word_to_hash(packed);
  (*condition_8_1361) = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1));
  return path_new(path_data_3_3192, ((uint8_t)((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1)))));
}

uint8_t path_len(struct TriePath path)
{
  return path.len;
}

uint64_t path_nibble(struct TriePath path, uint8_t i)
{
  if ((path_len(path)) <= i) {
    return UINT64_C(0x0);
  }
  uint8_t byte_index = path_byte_index(i);
  if ((i & UINT8_C(1)) == UINT8_C(0)) {
    return (UINT64_C(0xF) & ((fast_unsigned_vector_access_bytes32(path.data, byte_index)) >> UINT8_C(4)));
  }
  return (UINT64_C(0xF) & ((fast_unsigned_vector_access_bytes32(path.data, byte_index)) >> UINT8_C(0)));
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
      bytes = fast_unsigned_vector_update_bytes32(bytes, byte_index, ((UINT64_C(0xF) & ((fast_unsigned_vector_access_bytes32(bytes, byte_index)) >> UINT8_C(4))) << 4) | value);
    }
    return path_new((B256(bytes)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)length_)));
  }
  fatal_error(WitnessDeficient);
}

struct TriePath path_single(uint64_t n)
{
  return path_append_nibble(((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)}), n);
}

struct TriePath path_concat(struct TriePath a, struct TriePath b)
{
  uint8_t alen = path_len(a);
  uint8_t blen = path_len(b);
  if (((uint8_t)((uint32_t)blen + (uint32_t)alen)) > UINT8_C(64)) {
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
    return ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
  }
  if (n == UINT8_C(0)) {
    return path;
  }
  uint8_t remain = ((uint8_t)((uint32_t)length_ - (uint32_t)n));
  struct TriePath result = ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
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
  return (bool)((a.len == b.len) && eq_bytes32(a.data, b.data));
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
    if (a_nibble == (path_nibble(b, length_))) {
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
  return ((uint8_t)((uint32_t)((path_len(path)) >> 1) + (uint32_t)UINT8_C(1)));
}

uint64_t hex_prefix_first_byte(struct TriePath path, bool is_leaf)
{
  uint8_t length_ = path_len(path);
  uint64_t flag = is_leaf ? UINT64_C(0x2) : UINT64_C(0x0);
  if ((length_ & UINT8_C(1)) != UINT8_C(0)) {
    return ((flag | UINT64_C(0x1)) << 4) | (path_nibble_struct_TriePath_uint8_t_to_uint64_t(path, UINT8_C(0)));
  }
  return (flag << 4) | UINT64_C(0x0);
}

struct TriePath scratch_hex_prefix_decode_ref(struct ScratchRlpFieldRef f, bool *restrict condition_8_1493)
{
  if (f.is_list) {
    fatal_error(RlpDecode);
  }
  if (f.content_len == UINT8_C(0)) {
    (*condition_8_1493) = false;
    return ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
  }
  if (HEX_PREFIX_MAX_LENGTH < f.content_len) {
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
      bytes = fast_unsigned_vector_update_bytes32(bytes, UINT8_C(0), ((UINT64_C(0xF) & (fb >> UINT8_C(0))) << 4) | (UINT64_C(0xF) & ((fast_unsigned_vector_access_bytes32(bytes, UINT8_C(0))) >> UINT8_C(0))));
      bytes32 path_data = B256(bytes);
      (*condition_8_1493) = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1));
      return path_new(path_data, ((uint8_t)((uint32_t)UINT8_C(1) + ((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1))))));
    }
    fatal_error(WitnessDeficient);
  }
  bytes32 path_data_3_1007 = word_to_hash(packed);
  (*condition_8_1493) = (bool)((UINT64_C(1) & ((UINT64_C(0xF) & (fb >> UINT8_C(4))) >> UINT8_C(1))) == UINT64_C(0x1));
  return path_new(path_data_3_1007, ((uint8_t)((uint32_t)UINT8_C(2) * ((uint32_t)(uint8_t)f.content_len - (uint32_t)UINT8_C(1)))));
}

uint8_t path_byte_index(uint8_t i)
{
  return (i >> 1);
}

struct TriePath path_drop_struct_TriePath_uint8_t_to_struct_TriePath(struct TriePath path, uint8_t n)
{
  uint8_t length_ = path_len(path);
  if (length_ <= n) {
    return ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
  }
  uint8_t remain = ((uint8_t)((uint32_t)length_ - (uint32_t)n));
  struct TriePath result = ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
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
  if ((path_len(path)) <= n) {
    return ((struct TriePath){.data = ((bytes32){0}), .len = UINT8_C(0)});
  }
  return path;
}

bool path_matches(struct TriePath key, uint8_t pos, struct TriePath seg)
{
  uint8_t segment_len = path_len(seg);
  if ((path_len(key)) < ((uint8_t)((uint32_t)segment_len + (uint32_t)pos))) {
    return false;
  }
  bool ok = true;
  uint8_t offset = UINT8_C(0);
  while (ok && (offset < segment_len)) {
    if (((uint8_t)((uint32_t)offset + (uint32_t)pos)) <= UINT8_C(64)) {
      uint64_t key_nibble = path_nibble(key, ((uint8_t)((uint32_t)offset + (uint32_t)pos)));
      if (key_nibble != (path_nibble(seg, offset))) {
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
  if ((path_len(path)) <= i) {
    return UINT64_C(0x0);
  }
  uint8_t byte_index = path_byte_index(i);
  return (UINT64_C(0xF) & ((fast_unsigned_vector_access_bytes32(path.data, byte_index)) >> UINT8_C(4)));
}

