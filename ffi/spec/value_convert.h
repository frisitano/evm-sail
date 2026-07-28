/* Fixed word/address/hash conversions shared by the host FFI. */
#ifndef EVMSAIL_VALUE_CONVERT_H
#define EVMSAIL_VALUE_CONVERT_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline uint8_t evmsail_byte_value(const uint64_t value) {
  return (uint8_t)value;
}

static inline void evmsail_address_to_be_bytes(
    uint8_t out[20], sail_fixed_bytes_20 value) {
  memcpy(out, value.bytes, sizeof(value.bytes));
}

static inline void evmsail_hash_to_be_bytes(
    uint8_t out[32], sail_fixed_bytes_32 value) {
  memcpy(out, value.bytes, sizeof(value.bytes));
}

static inline sail_fixed_bytes_20 evmsail_address_from_be_bytes(
    const uint8_t in[20]) {
  sail_fixed_bytes_20 result;
  memcpy(result.bytes, in, sizeof(result.bytes));
  return result;
}

static inline sail_fixed_bytes_32 evmsail_hash_from_be_bytes(
    const uint8_t in[32]) {
  sail_fixed_bytes_32 result;
  memcpy(result.bytes, in, sizeof(result.bytes));
  return result;
}

static inline void evmsail_address_set_be_bytes(
    sail_fixed_bytes_20 *out, const uint8_t in[20]) {
  *out = evmsail_address_from_be_bytes(in);
}

static inline void evmsail_hash_set_be_bytes(
    sail_fixed_bytes_32 *out, const uint8_t in[32]) {
  *out = evmsail_hash_from_be_bytes(in);
}

static inline void evmsail_address_assign(
    sail_fixed_bytes_20 *out, sail_fixed_bytes_20 value) {
  *out = value;
}

static inline void evmsail_hash_assign(
    sail_fixed_bytes_32 *out, sail_fixed_bytes_32 value) {
  *out = value;
}

static inline void sail_word_to_be_words4(uint64_t out[4], sail_u256 value) {
  out[0] = value.limbs[3];
  out[1] = value.limbs[2];
  out[2] = value.limbs[1];
  out[3] = value.limbs[0];
}

static inline void sail_word_to_le_words4(uint64_t out[4], sail_u256 value) {
  out[0] = value.limbs[0];
  out[1] = value.limbs[1];
  out[2] = value.limbs[2];
  out[3] = value.limbs[3];
}

static inline sail_u256 be_words4_to_sail_word(const uint64_t words[4]) {
  sail_u256 result = {{words[3], words[2], words[1], words[0]}};
  return result;
}

static inline sail_u256 le_words4_to_sail_word(const uint64_t words[4]) {
  sail_u256 result = {{words[0], words[1], words[2], words[3]}};
  return result;
}

static inline void be_words4_to_be_bytes(uint8_t out[32],
                                         const uint64_t words[4]) {
  for (size_t word = 0; word < 4; ++word)
    for (size_t byte = 0; byte < 8; ++byte)
      out[word * 8 + byte] =
          (uint8_t)(words[word] >> (56 - byte * 8));
}

static inline void sail_word_to_be_bytes(uint8_t out[32], sail_u256 value) {
  for (size_t i = 0; i < 32; ++i) {
    const size_t bit = 8 * (31 - i);
    out[i] = (uint8_t)(value.limbs[bit / 64] >> (bit % 64));
  }
}

static inline sail_u256 be_bytes_to_sail_word(const uint8_t in[32]) {
  sail_u256 result = {{0}};
  for (size_t i = 0; i < 32; ++i) {
    const size_t bit = 8 * (31 - i);
    result.limbs[bit / 64] |= (uint64_t)in[i] << (bit % 64);
  }
  return result;
}

static inline void sail_hash_to_be_words4(uint64_t out[4], sail_fixed_bytes_32 value) {
  uint8_t bytes[32];
  evmsail_hash_to_be_bytes(bytes, value);
  for (size_t word = 0; word < 4; ++word) {
    out[word] = 0;
    for (size_t byte = 0; byte < 8; ++byte)
      out[word] = (out[word] << 8) | bytes[word * 8 + byte];
  }
}

static inline void sail_hash_to_le_words4(uint64_t out[4], sail_fixed_bytes_32 value) {
  uint64_t words[4];
  sail_hash_to_be_words4(words, value);
  out[0] = words[3];
  out[1] = words[2];
  out[2] = words[1];
  out[3] = words[0];
}

static inline sail_fixed_bytes_32 be_words4_to_sail_hash(const uint64_t words[4]) {
  uint8_t bytes[32];
  sail_fixed_bytes_32 result;
  be_words4_to_be_bytes(bytes, words);
  evmsail_hash_set_be_bytes(&result, bytes);
  return result;
}

static inline sail_fixed_bytes_32 le_words4_to_sail_hash(const uint64_t words[4]) {
  const uint64_t big_endian[4] = {words[3], words[2], words[1], words[0]};
  return be_words4_to_sail_hash(big_endian);
}

static inline sail_fixed_bytes_20 be_bytes_to_sail_address(const uint8_t in[20]) {
  sail_fixed_bytes_20 result;
  evmsail_address_set_be_bytes(&result, in);
  return result;
}

#endif
