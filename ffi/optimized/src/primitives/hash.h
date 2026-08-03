/* Helpers for canonical protocol-order hash bytes in C host stores. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_HASH_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_HASH_H

#include "evmsail/prelude.h"
#include "primitives/value.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const Hash32 EVMSAIL_EMPTY_TRIE_ROOT = {{
    UINT64_C(0xa655cc1b171fe856),
    UINT64_C(0x6ef8c092e64583ff),
    UINT64_C(0xc0ad6c991be0485b),
    UINT64_C(0x21b463e3b52f6201),
}};

static const Hash32 EVMSAIL_KECCAK_EMPTY = {{
    UINT64_C(0x3c23f7860146d2c5),
    UINT64_C(0xc003c7dcb27d7e92),
    UINT64_C(0x3b2782ca53b600e5),
    UINT64_C(0x70a4855d04d8fa7b),
}};

static inline int hash_compare(const Hash32 *a,
                                       const Hash32 *b) {
  int order = canonical_lane_compare(a->lanes[0], b->lanes[0]);
  if (order != 0) return order;
  order = canonical_lane_compare(a->lanes[1], b->lanes[1]);
  if (order != 0) return order;
  order = canonical_lane_compare(a->lanes[2], b->lanes[2]);
  if (order != 0) return order;
  return canonical_lane_compare(a->lanes[3], b->lanes[3]);
}

static inline int hash_equal(const Hash32 *a,
                                     const Hash32 *b) {
  return a->lanes[0] == b->lanes[0] && a->lanes[1] == b->lanes[1] &&
         a->lanes[2] == b->lanes[2] && a->lanes[3] == b->lanes[3];
}

static inline Hash32 hash_from_sail_word(U256 value) {
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

static inline U256 hash_to_sail_word(const Hash32 *value) {
  return (U256){
      .limbs =
          {
              __builtin_bswap64(value->lanes[3]),
              __builtin_bswap64(value->lanes[2]),
              __builtin_bswap64(value->lanes[1]),
              __builtin_bswap64(value->lanes[0]),
          },
  };
}

/* The low 160 bits of the digest, retaining canonical Ethereum byte order. */
static inline Address hash_low_address(
    const Hash32 *value) {
  return (Address){
      .lanes =
          {
              (value->lanes[1] >> 32) | (value->lanes[2] << 32),
              (value->lanes[2] >> 32) | (value->lanes[3] << 32),
              value->lanes[3] >> 32,
          },
  };
}

#endif
