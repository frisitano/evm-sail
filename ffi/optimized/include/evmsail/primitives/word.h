#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_WORD_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_WORD_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Optimized-C refinements of the direct endian-conversion equations in
 * prelude.sail. Fixed byte vectors use canonical protocol order, while
 * U256 limbs are least-significant first. Keeping these definitions in
 * the generated model translation unit lets ordinary C optimization inline
 * every load/store into its caller.
 */
static inline uint64_t load_be64(const uint8_t bytes[8]) {
  return ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) |
         ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32) |
         ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16) |
         ((uint64_t)bytes[6] << 8) | (uint64_t)bytes[7];
}

static inline uint32_t load_be32(const uint8_t bytes[4]) {
  return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
         ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

static inline void store_be64(uint8_t bytes[8], uint64_t value) {
  bytes[0] = (uint8_t)(value >> 56);
  bytes[1] = (uint8_t)(value >> 48);
  bytes[2] = (uint8_t)(value >> 40);
  bytes[3] = (uint8_t)(value >> 32);
  bytes[4] = (uint8_t)(value >> 24);
  bytes[5] = (uint8_t)(value >> 16);
  bytes[6] = (uint8_t)(value >> 8);
  bytes[7] = (uint8_t)value;
}

static inline void store_be32(uint8_t bytes[4], uint32_t value) {
  bytes[0] = (uint8_t)(value >> 24);
  bytes[1] = (uint8_t)(value >> 16);
  bytes[2] = (uint8_t)(value >> 8);
  bytes[3] = (uint8_t)value;
}

static inline U256 word_from_hash(
    Hash32 bytes) {
  return (U256){
      .limbs =
          {
              __builtin_bswap64(bytes.lanes[3]),
              __builtin_bswap64(bytes.lanes[2]),
              __builtin_bswap64(bytes.lanes[1]),
              __builtin_bswap64(bytes.lanes[0]),
          },
  };
}

static inline Hash32 hash_from_word(
    U256 value) {
  return (Hash32){
      .lanes =
          {
              __builtin_bswap64(value.limbs[3]),
              __builtin_bswap64(value.limbs[2]),
              __builtin_bswap64(value.limbs[1]),
              __builtin_bswap64(value.limbs[0]),
          },
  };
}

static inline uint64_t bit_length_u64(uint64_t value) {
  return value == UINT64_C(0)
             ? UINT64_C(0)
             : UINT64_C(64) - (uint64_t)__builtin_clzll(value);
}

static inline Address address_from_word(
    U256 value) {
  return (Address){
      .lanes = {
          (uint64_t)__builtin_bswap32((uint32_t)value.limbs[2]) |
              ((uint64_t)__builtin_bswap32((uint32_t)(value.limbs[1] >> 32))
               << 32),
          (uint64_t)__builtin_bswap32((uint32_t)value.limbs[1]) |
              ((uint64_t)__builtin_bswap32((uint32_t)(value.limbs[0] >> 32))
               << 32),
          (uint64_t)__builtin_bswap32((uint32_t)value.limbs[0]),
      },
  };
}

static inline U256 word_from_address(
    Address bytes) {
  return (U256){
      .limbs =
          {
              ((uint64_t)__builtin_bswap32((uint32_t)(bytes.lanes[1] >> 32))
               << 32) |
                  __builtin_bswap32((uint32_t)bytes.lanes[2]),
              ((uint64_t)__builtin_bswap32((uint32_t)(bytes.lanes[0] >> 32))
               << 32) |
                  __builtin_bswap32((uint32_t)bytes.lanes[1]),
              __builtin_bswap32((uint32_t)bytes.lanes[0]),
              0,
          },
  };
}

#endif
