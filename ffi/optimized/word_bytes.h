#ifndef EVMSAIL_OPTIMIZED_WORD_BYTES_H
#define EVMSAIL_OPTIMIZED_WORD_BYTES_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

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

static inline sail_u256 evmsail_hash_to_word(sail_fixed_bytes_32 bytes) {
  return (sail_u256){
      .limbs =
          {
              evmsail_load_be64(&bytes.bytes[24]),
              evmsail_load_be64(&bytes.bytes[16]),
              evmsail_load_be64(&bytes.bytes[8]),
              evmsail_load_be64(&bytes.bytes[0]),
          },
  };
}

static inline sail_fixed_bytes_32 evmsail_word_to_hash(sail_u256 value) {
  sail_fixed_bytes_32 result;
  evmsail_store_be64(&result.bytes[0], value.limbs[3]);
  evmsail_store_be64(&result.bytes[8], value.limbs[2]);
  evmsail_store_be64(&result.bytes[16], value.limbs[1]);
  evmsail_store_be64(&result.bytes[24], value.limbs[0]);
  return result;
}

static inline sail_fixed_bytes_20 evmsail_word_to_address(sail_u256 value) {
  sail_fixed_bytes_20 result;
  evmsail_store_be32(&result.bytes[0], (uint32_t)value.limbs[2]);
  evmsail_store_be64(&result.bytes[4], value.limbs[1]);
  evmsail_store_be64(&result.bytes[12], value.limbs[0]);
  return result;
}

static inline sail_u256 evmsail_address_to_word(sail_fixed_bytes_20 bytes) {
  return (sail_u256){
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
