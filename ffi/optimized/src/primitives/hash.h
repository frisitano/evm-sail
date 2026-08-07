/* Helpers for canonical protocol-order hash bytes in C host stores. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_HASH_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_HASH_H

#include "evmsail/prelude.h"
#include "primitives/value.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const bytes32 EVMSAIL_EMPTY_TRIE_ROOT = {
    .lanes =
        {
            UINT64_C(0xa655cc1b171fe856),
            UINT64_C(0x6ef8c092e64583ff),
            UINT64_C(0xc0ad6c991be0485b),
            UINT64_C(0x21b463e3b52f6201),
        },
};

static const bytes32 EVMSAIL_KECCAK_EMPTY = {
    .lanes =
        {
            UINT64_C(0x3c23f7860146d2c5),
            UINT64_C(0xc003c7dcb27d7e92),
            UINT64_C(0x3b2782ca53b600e5),
            UINT64_C(0x70a4855d04d8fa7b),
        },
};

static inline int hash_compare(const bytes32 *a, const bytes32 *b)
{
  return fixed_lane_compare(a->lanes, b->lanes, 32);
}

static inline int hash_equal(const bytes32 *a, const bytes32 *b)
{
  return a->lanes[0] == b->lanes[0] && a->lanes[1] == b->lanes[1] && a->lanes[2] == b->lanes[2] &&
         a->lanes[3] == b->lanes[3];
}

/* The low 160 bits of the digest, retaining canonical Ethereum byte order. */
static inline bytes20 hash_low_address(const bytes32 *value)
{
  bytes20 result = {{0}};
  memcpy(bytes20_data_mut(&result), bytes32_data(value) + 12, 20);
  return result;
}

#endif
