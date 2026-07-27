#ifndef EVMSAIL_WORD_BYTES_GLUE_H
#define EVMSAIL_WORD_BYTES_GLUE_H

#include "sail_abi.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Optimized-C refinements of the direct endian-conversion equations in
 * prelude.sail. Fixed byte vectors use canonical protocol order, while
 * sail_u256 limbs are least-significant first. Keeping these definitions in
 * the generated model translation unit lets ordinary C optimization inline
 * every load/store into its caller.
 */
static inline uint64_t evmsail_load_be64(const uint8_t bytes[8]) {
  return ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) |
         ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32) |
         ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16) |
         ((uint64_t)bytes[6] << 8) | (uint64_t)bytes[7];
}

static inline uint32_t evmsail_load_be32(const uint8_t bytes[4]) {
  return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
         ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

static inline void evmsail_store_be64(uint8_t bytes[8], uint64_t value) {
  bytes[0] = (uint8_t)(value >> 56);
  bytes[1] = (uint8_t)(value >> 48);
  bytes[2] = (uint8_t)(value >> 40);
  bytes[3] = (uint8_t)(value >> 32);
  bytes[4] = (uint8_t)(value >> 24);
  bytes[5] = (uint8_t)(value >> 16);
  bytes[6] = (uint8_t)(value >> 8);
  bytes[7] = (uint8_t)value;
}

static inline void evmsail_store_be32(uint8_t bytes[4], uint32_t value) {
  bytes[0] = (uint8_t)(value >> 24);
  bytes[1] = (uint8_t)(value >> 16);
  bytes[2] = (uint8_t)(value >> 8);
  bytes[3] = (uint8_t)value;
}

static inline sail_word evmsail_hash_to_word(sail_hash bytes) {
  return (sail_word){
      .limbs =
          {
              evmsail_load_be64(&bytes.bytes[24]),
              evmsail_load_be64(&bytes.bytes[16]),
              evmsail_load_be64(&bytes.bytes[8]),
              evmsail_load_be64(&bytes.bytes[0]),
          },
  };
}

static inline sail_hash evmsail_word_to_hash(sail_word value) {
  sail_hash result;
  evmsail_store_be64(&result.bytes[0], value.limbs[3]);
  evmsail_store_be64(&result.bytes[8], value.limbs[2]);
  evmsail_store_be64(&result.bytes[16], value.limbs[1]);
  evmsail_store_be64(&result.bytes[24], value.limbs[0]);
  return result;
}

static inline bool evmsail_hash_lt(sail_hash left, sail_hash right) {
  uint64_t left_chunk = evmsail_load_be64(&left.bytes[0]);
  uint64_t right_chunk = evmsail_load_be64(&right.bytes[0]);
  if (left_chunk != right_chunk) return left_chunk < right_chunk;

  left_chunk = evmsail_load_be64(&left.bytes[8]);
  right_chunk = evmsail_load_be64(&right.bytes[8]);
  if (left_chunk != right_chunk) return left_chunk < right_chunk;

  left_chunk = evmsail_load_be64(&left.bytes[16]);
  right_chunk = evmsail_load_be64(&right.bytes[16]);
  if (left_chunk != right_chunk) return left_chunk < right_chunk;

  return evmsail_load_be64(&left.bytes[24]) <
         evmsail_load_be64(&right.bytes[24]);
}

static inline sail_address evmsail_word_to_address(sail_word value) {
  sail_address result;
  evmsail_store_be32(&result.bytes[0], (uint32_t)value.limbs[2]);
  evmsail_store_be64(&result.bytes[4], value.limbs[1]);
  evmsail_store_be64(&result.bytes[12], value.limbs[0]);
  return result;
}

static inline sail_word evmsail_address_to_word(sail_address bytes) {
  return (sail_word){
      .limbs =
          {
              evmsail_load_be64(&bytes.bytes[12]),
              evmsail_load_be64(&bytes.bytes[4]),
              evmsail_load_be32(&bytes.bytes[0]),
              0,
          },
  };
}

#endif
