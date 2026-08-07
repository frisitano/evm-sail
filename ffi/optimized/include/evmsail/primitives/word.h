#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_WORD_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_WORD_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Optimized-C refinements of the direct endian-conversion equations in
 * prelude.sail. Fixed byte vectors use canonical protocol order, while
 * u256 limbs are least-significant first. Keeping these definitions in
 * the generated model translation unit lets ordinary C optimization inline
 * every load/store into its caller.
 */
static inline void store_be64(uint8_t bytes[8], uint64_t value)
{
  bytes[0] = (uint8_t)(value >> 56);
  bytes[1] = (uint8_t)(value >> 48);
  bytes[2] = (uint8_t)(value >> 40);
  bytes[3] = (uint8_t)(value >> 32);
  bytes[4] = (uint8_t)(value >> 24);
  bytes[5] = (uint8_t)(value >> 16);
  bytes[6] = (uint8_t)(value >> 8);
  bytes[7] = (uint8_t)value;
}

static inline void store_be32(uint8_t bytes[4], uint32_t value)
{
  bytes[0] = (uint8_t)(value >> 24);
  bytes[1] = (uint8_t)(value >> 16);
  bytes[2] = (uint8_t)(value >> 8);
  bytes[3] = (uint8_t)value;
}

static inline bytes20 address_from_word(u256 value)
{
  bytes20 result = {{0}};
  result.lanes[0] = (uint64_t)__builtin_bswap32((uint32_t)value.limbs[2]) |
                    ((__builtin_bswap64(value.limbs[1]) & UINT64_C(0xffffffff)) << 32);
  result.lanes[1] = (__builtin_bswap64(value.limbs[1]) >> 32) |
                    ((__builtin_bswap64(value.limbs[0]) & UINT64_C(0xffffffff)) << 32);
  result.lanes[2] = __builtin_bswap64(value.limbs[0]) >> 32;
  return result;
}

#endif
