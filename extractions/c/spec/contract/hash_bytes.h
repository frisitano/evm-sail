/* Helpers for canonical protocol-order hash bytes in C host stores. */
#ifndef EVMSAIL_HASH_BYTES_H
#define EVMSAIL_HASH_BYTES_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline int evmsail_hash_compare(const fixed_bytes_32 *a,
                                       const fixed_bytes_32 *b) {
  return memcmp(a->bytes, b->bytes, sizeof(a->bytes));
}

static inline int evmsail_hash_equal(const fixed_bytes_32 *a,
                                     const fixed_bytes_32 *b) {
  return evmsail_hash_compare(a, b) == 0;
}

static inline fixed_bytes_32 evmsail_hash_from_sail_word(u256 value) {
  fixed_bytes_32 result = {{0}};
  for (size_t i = 0; i < sizeof(result.bytes); ++i) {
    const size_t bit = 8 * (sizeof(result.bytes) - 1 - i);
    result.bytes[i] =
        (uint8_t)(value.limbs[bit / 64] >> (bit % 64));
  }
  return result;
}

static inline u256 evmsail_hash_to_sail_word(const fixed_bytes_32 *value) {
  u256 result = {{0}};
  for (size_t i = 0; i < sizeof(value->bytes); ++i) {
    const size_t bit = 8 * (sizeof(value->bytes) - 1 - i);
    result.limbs[bit / 64] |=
        (uint64_t)value->bytes[i] << (bit % 64);
  }
  return result;
}

#endif
