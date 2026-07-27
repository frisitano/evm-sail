/* Helpers for canonical protocol-order hash bytes in C host stores. */
#ifndef EVMSAIL_HASH_BYTES_H
#define EVMSAIL_HASH_BYTES_H

#include "sail_abi.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline int evmsail_hash_compare(const sail_hash *a,
                                       const sail_hash *b) {
  return memcmp(a->bytes, b->bytes, sizeof(a->bytes));
}

static inline int evmsail_hash_equal(const sail_hash *a,
                                     const sail_hash *b) {
  return evmsail_hash_compare(a, b) == 0;
}

static inline sail_hash evmsail_hash_from_sail_word(sail_word value) {
  sail_hash result = {{0}};
  for (size_t i = 0; i < sizeof(result.bytes); ++i) {
    const size_t bit = 8 * (sizeof(result.bytes) - 1 - i);
    result.bytes[i] =
        (uint8_t)(value.limbs[bit / 64] >> (bit % 64));
  }
  return result;
}

static inline sail_word evmsail_hash_to_sail_word(const sail_hash *value) {
  sail_word result = {{0}};
  for (size_t i = 0; i < sizeof(value->bytes); ++i) {
    const size_t bit = 8 * (sizeof(value->bytes) - 1 - i);
    result.limbs[bit / 64] |=
        (uint64_t)value->bytes[i] << (bit % 64);
  }
  return result;
}

#endif
