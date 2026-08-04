/* Fixed word/address/hash conversions shared by the host FFI. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_VALUE_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_VALUE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline uint8_t byte_value(const uint64_t value) {
  return (uint8_t)value;
}

/*
 * The optimized fixed-byte representation stores the canonical protocol byte
 * sequence directly, without host-endian padding or lane reinterpretation.
 */
static inline const uint8_t *address_bytes_const(
    const Address *value) {
  return value->bytes;
}

static inline uint8_t *address_bytes(
    Address *value) {
  return value->bytes;
}

static inline const uint8_t *hash_bytes_const(
    const Hash32 *value) {
  return value->bytes;
}

static inline uint8_t *hash_bytes(
    Hash32 *value) {
  return value->bytes;
}

static inline bool address_equal(
    const Address *left,
    const Address *right) {
  return memcmp(left->bytes, right->bytes, sizeof(left->bytes)) == 0;
}

static inline uint64_t load_u64(const uint8_t bytes[8]) {
  uint64_t value;
  memcpy(&value, bytes, sizeof(value));
  return value;
}

static inline void store_u64(uint8_t bytes[8], uint64_t value) {
  memcpy(bytes, &value, sizeof(value));
}

static inline bool address_equal_be_bytes(
    const Address *left, const uint8_t right[20]) {
  return memcmp(left->bytes, right, sizeof(left->bytes)) == 0;
}

static inline bool hash_equal_be_bytes(
    const Hash32 *left, const uint8_t right[32]) {
  return memcmp(left->bytes, right, sizeof(left->bytes)) == 0;
}

static inline int address_compare(
    const Address *left,
    const Address *right) {
  return memcmp(left->bytes, right->bytes, sizeof(left->bytes));
}

static inline bool word_equal(const U256 *left,
                                      const U256 *right) {
  return left->limbs[0] == right->limbs[0] &&
         left->limbs[1] == right->limbs[1] &&
         left->limbs[2] == right->limbs[2] &&
         left->limbs[3] == right->limbs[3];
}

static inline int word_compare(const U256 *left,
                                       const U256 *right) {
  for (size_t limb = 4; limb-- > 0;) {
    if (left->limbs[limb] < right->limbs[limb]) return -1;
    if (left->limbs[limb] > right->limbs[limb]) return 1;
  }
  return 0;
}

static inline void address_to_be_bytes(
    uint8_t out[20], Address value) {
  memcpy(out, address_bytes_const(&value), 20);
}

static inline void hash_to_be_bytes(
    uint8_t out[32], Hash32 value) {
  memcpy(out, hash_bytes_const(&value), 32);
}

static inline Address address_from_be_bytes(
    const uint8_t in[20]) {
  Address result = {{0}};
  memcpy(address_bytes(&result), in, 20);
  return result;
}

static inline Hash32 hash_from_be_bytes(
    const uint8_t in[32]) {
  Hash32 result;
  memcpy(hash_bytes(&result), in, 32);
  return result;
}

static inline void address_set_be_bytes(
    Address *out, const uint8_t in[20]) {
  *out = address_from_be_bytes(in);
}

static inline void hash_set_be_bytes(
    Hash32 *out, const uint8_t in[32]) {
  *out = hash_from_be_bytes(in);
}

static inline void address_assign(
    Address *out, Address value) {
  *out = value;
}

static inline void hash_assign(
    Hash32 *out, Hash32 value) {
  *out = value;
}

static inline void sail_word_to_be_words4(uint64_t out[4], U256 value) {
  out[0] = value.limbs[3];
  out[1] = value.limbs[2];
  out[2] = value.limbs[1];
  out[3] = value.limbs[0];
}

static inline void sail_word_to_le_words4(uint64_t out[4], U256 value) {
  out[0] = value.limbs[0];
  out[1] = value.limbs[1];
  out[2] = value.limbs[2];
  out[3] = value.limbs[3];
}

static inline U256 be_words4_to_sail_word(const uint64_t words[4]) {
  U256 result = {{words[3], words[2], words[1], words[0]}};
  return result;
}

static inline U256 le_words4_to_sail_word(const uint64_t words[4]) {
  U256 result = {{words[0], words[1], words[2], words[3]}};
  return result;
}

static inline void be_words4_to_be_bytes(uint8_t out[32],
                                         const uint64_t words[4]) {
  for (size_t word = 0; word < 4; ++word)
    for (size_t byte = 0; byte < 8; ++byte)
      out[word * 8 + byte] =
          (uint8_t)(words[word] >> (56 - byte * 8));
}

static inline void sail_word_to_be_bytes(uint8_t out[32], U256 value) {
  store_u64(out, __builtin_bswap64(value.limbs[3]));
  store_u64(out + 8, __builtin_bswap64(value.limbs[2]));
  store_u64(out + 16, __builtin_bswap64(value.limbs[1]));
  store_u64(out + 24, __builtin_bswap64(value.limbs[0]));
}

static inline U256 be_bytes_to_sail_word(const uint8_t in[32]) {
  return (U256){
      .limbs =
          {
              __builtin_bswap64(load_u64(in + 24)),
              __builtin_bswap64(load_u64(in + 16)),
              __builtin_bswap64(load_u64(in + 8)),
              __builtin_bswap64(load_u64(in)),
          },
  };
}

static inline void sail_hash_to_be_words4(uint64_t out[4], Hash32 value) {
  out[0] = __builtin_bswap64(load_u64(value.bytes));
  out[1] = __builtin_bswap64(load_u64(value.bytes + 8));
  out[2] = __builtin_bswap64(load_u64(value.bytes + 16));
  out[3] = __builtin_bswap64(load_u64(value.bytes + 24));
}

static inline void sail_hash_to_le_words4(uint64_t out[4], Hash32 value) {
  uint64_t words[4];
  sail_hash_to_be_words4(words, value);
  out[0] = words[3];
  out[1] = words[2];
  out[2] = words[1];
  out[3] = words[0];
}

static inline Hash32 be_words4_to_sail_hash(const uint64_t words[4]) {
  Hash32 result;
  store_u64(result.bytes, __builtin_bswap64(words[0]));
  store_u64(result.bytes + 8, __builtin_bswap64(words[1]));
  store_u64(result.bytes + 16, __builtin_bswap64(words[2]));
  store_u64(result.bytes + 24, __builtin_bswap64(words[3]));
  return result;
}

static inline Hash32 le_words4_to_sail_hash(const uint64_t words[4]) {
  const uint64_t big_endian[4] = {words[3], words[2], words[1], words[0]};
  return be_words4_to_sail_hash(big_endian);
}

static inline Address be_bytes_to_sail_address(const uint8_t in[20]) {
  Address result;
  address_set_be_bytes(&result, in);
  return result;
}

#endif
