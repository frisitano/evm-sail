/* Fixed word/address/hash conversions shared by the host FFI. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_VALUE_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_VALUE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "fixed_bytes_u64_lanes requires a little-endian optimized target"
#endif

static inline uint8_t byte_value(const uint64_t value) {
  return (uint8_t)value;
}

/*
 * The lane representation assigns semantic byte i to bits
 * (i % 8) * 8 of lane i / 8. Optimized targets are little-endian, so the lane
 * storage itself is the canonical byte sequence used by Ethereum encodings.
 * Keep the semantic length explicit: a 20-byte address occupies three lanes,
 * but its final four storage bytes are padding rather than address bytes.
 */
static inline const uint8_t *address_bytes_const(
    const Address *value) {
  return (const uint8_t *)(const void *)value->lanes;
}

static inline uint8_t *address_bytes(
    Address *value) {
  return (uint8_t *)(void *)value->lanes;
}

static inline const uint8_t *hash_bytes_const(
    const Hash32 *value) {
  return (const uint8_t *)(const void *)value->lanes;
}

static inline uint8_t *hash_bytes(
    Hash32 *value) {
  return (uint8_t *)(void *)value->lanes;
}

static inline bool address_equal(
    const Address *left,
    const Address *right) {
  return left->lanes[0] == right->lanes[0] &&
         left->lanes[1] == right->lanes[1] &&
         (uint32_t)left->lanes[2] == (uint32_t)right->lanes[2];
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
  uint32_t tail;
  memcpy(&tail, right + 16, sizeof(tail));
  return left->lanes[0] == load_u64(right) &&
         left->lanes[1] == load_u64(right + 8) &&
         (uint32_t)left->lanes[2] == tail;
}

static inline bool hash_equal_be_bytes(
    const Hash32 *left, const uint8_t right[32]) {
  return left->lanes[0] == load_u64(right) &&
         left->lanes[1] == load_u64(right + 8) &&
         left->lanes[2] == load_u64(right + 16) &&
         left->lanes[3] == load_u64(right + 24);
}

/* Compare one lane in canonical byte order. Canonical byte zero occupies the
 * lane's least-significant byte so a raw integer comparison would use the
 * wrong significance. Find the first differing byte with three word-sized
 * tests and compare only that byte. This avoids both a byte loop and the
 * out-of-line __bswapdi2 emitted for RV64IM. */
static inline int canonical_lane_compare(uint64_t left,
                                                 uint64_t right) {
  uint64_t difference = left ^ right;
  unsigned shift = 0;
  if (difference == 0) return 0;
  if ((uint32_t)difference == 0) {
    difference >>= 32;
    shift += 32;
  }
  if ((uint16_t)difference == 0) {
    difference >>= 16;
    shift += 16;
  }
  if ((uint8_t)difference == 0) shift += 8;
  const uint8_t left_byte = (uint8_t)(left >> shift);
  const uint8_t right_byte = (uint8_t)(right >> shift);
  return left_byte < right_byte ? -1 : 1;
}

static inline int address_compare(
    const Address *left,
    const Address *right) {
  int order =
      canonical_lane_compare(left->lanes[0], right->lanes[0]);
  if (order != 0) return order;
  order = canonical_lane_compare(left->lanes[1], right->lanes[1]);
  if (order != 0) return order;
  return canonical_lane_compare(
      (uint32_t)left->lanes[2], (uint32_t)right->lanes[2]);
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
  out[0] = __builtin_bswap64(value.lanes[0]);
  out[1] = __builtin_bswap64(value.lanes[1]);
  out[2] = __builtin_bswap64(value.lanes[2]);
  out[3] = __builtin_bswap64(value.lanes[3]);
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
  return (Hash32){
      .lanes = {
          __builtin_bswap64(words[0]),
          __builtin_bswap64(words[1]),
          __builtin_bswap64(words[2]),
          __builtin_bswap64(words[3]),
      },
  };
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
