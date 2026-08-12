#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_WORD_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_WORD_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* rv64ima has no rev8 and gcc lowers __builtin_bswap* to libgcc calls; this
 * shift/mask form inlines at every use. Clang folds it back to rev/rev32. */
#ifndef OPTIMIZED_BSWAP_INLINE
#define OPTIMIZED_BSWAP_INLINE
static inline __attribute__((__always_inline__)) uint64_t bswap64(uint64_t v)
{
  v = ((v & UINT64_C(0x00ff00ff00ff00ff)) << 8) | ((v >> 8) & UINT64_C(0x00ff00ff00ff00ff));
  v = ((v & UINT64_C(0x0000ffff0000ffff)) << 16) | ((v >> 16) & UINT64_C(0x0000ffff0000ffff));
  return (v << 32) | (v >> 32);
}

static inline __attribute__((__always_inline__)) uint32_t bswap32(uint32_t v)
{
  v = (uint32_t)(((v & UINT32_C(0x00ff00ff)) << 8) | ((v >> 8) & UINT32_C(0x00ff00ff)));
  return (uint32_t)((v << 16) | (v >> 16));
}
#endif /* OPTIMIZED_BSWAP_INLINE */

/*
 * Optimized-C refinements of the direct endian-conversion equations in
 * prelude.sail. Fixed byte vectors use canonical protocol order, while
 * u256 limbs are least-significant first. Keeping these definitions in
 * the generated model translation unit lets ordinary C optimization inline
 * every load/store into its caller.
 */
static inline __attribute__((__always_inline__)) void store_be64(uint8_t bytes[8], uint64_t value)
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

static inline __attribute__((__always_inline__)) void store_be32(uint8_t bytes[4], uint32_t value)
{
  bytes[0] = (uint8_t)(value >> 24);
  bytes[1] = (uint8_t)(value >> 16);
  bytes[2] = (uint8_t)(value >> 8);
  bytes[3] = (uint8_t)value;
}

static inline __attribute__((__always_inline__)) bytes20 address_from_word(u256 value)
{
  bytes20 result = {{0}};
  result.lanes[0] = (uint64_t)bswap32((uint32_t)value.limbs[2]) |
                    ((bswap64(value.limbs[1]) & UINT64_C(0xffffffff)) << 32);
  result.lanes[1] =
      (bswap64(value.limbs[1]) >> 32) | ((bswap64(value.limbs[0]) & UINT64_C(0xffffffff)) << 32);
  result.lanes[2] = bswap64(value.limbs[0]) >> 32;
  return result;
}

/* ---- shared fixed-value machinery (consolidated from the former
 * src/primitives/value.h; that path remains as an include shim). ---- */

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__)
#error "optimized fixed-byte lanes require compiler byte-order definitions"
#elif __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "the optimized EVM Sail backend requires a little-endian target"
#endif

_Static_assert(sizeof(bytes20) == 3 * sizeof(uint64_t), "bytes20 must contain three lanes");
_Static_assert(sizeof(bytes32) == 4 * sizeof(uint64_t), "bytes32 must contain four lanes");
_Static_assert(sizeof(u256) == 4 * sizeof(uint64_t), "u256 must contain four limbs");

/* On the optimized little-endian target, Sail byte index i is byte i of the
 * lane object representation.  Character pointers may alias any C object, so
 * these views expose canonical protocol bytes without copying. */
static inline __attribute__((__always_inline__)) const uint8_t *bytes20_data(const bytes20 *value)
{
  return (const uint8_t *)value->lanes;
}

static inline __attribute__((__always_inline__)) uint8_t *bytes20_data_mut(bytes20 *value)
{
  return (uint8_t *)value->lanes;
}

static inline __attribute__((__always_inline__)) const uint8_t *bytes32_data(const bytes32 *value)
{
  return (const uint8_t *)value->lanes;
}

static inline __attribute__((__always_inline__)) uint8_t *bytes32_data_mut(bytes32 *value)
{
  return (uint8_t *)value->lanes;
}

/* Sail byte index i occupies bits 8 * (i % 8) of lane i / 8. */
static inline __attribute__((__always_inline__)) uint8_t fixed_lane_byte(const uint64_t *lanes, size_t index)
{
  return (uint8_t)(lanes[index / 8] >> (8 * (index % 8)));
}

static inline __attribute__((__always_inline__)) void fixed_lane_set_byte(uint64_t *lanes, size_t index, uint8_t byte)
{
  const size_t shift = 8 * (index % 8);
  const uint64_t mask = UINT64_C(0xff) << shift;
  lanes[index / 8] = (lanes[index / 8] & ~mask) | ((uint64_t)byte << shift);
}

static inline __attribute__((__always_inline__)) uint8_t address_byte(const bytes20 *value, size_t index)
{
  return fixed_lane_byte(value->lanes, index);
}

static inline __attribute__((__always_inline__)) uint8_t hash_byte(const bytes32 *value, size_t index)
{
  return fixed_lane_byte(value->lanes, index);
}

static inline __attribute__((__always_inline__)) bool address_equal(const bytes20 *left, const bytes20 *right)
{
  return (bool)(left->lanes[0] == right->lanes[0] && left->lanes[1] == right->lanes[1] &&
                left->lanes[2] == right->lanes[2]);
}

static inline __attribute__((__always_inline__)) int fixed_lane_compare(const uint64_t *left, const uint64_t *right, size_t length)
{
  /* Protocol ordering is lexicographic by byte.  A numeric comparison of the
   * little-endian lanes would give the wrong significance order; memcmp also
   * maps to the guest's accelerated byte-comparison operation. */
  return memcmp(left, right, length);
}

static inline __attribute__((__always_inline__)) uint64_t load_u64(const uint8_t bytes[8])
{
  uint64_t value;
  memcpy(&value, bytes, sizeof(value));
  return value;
}

static inline __attribute__((__always_inline__)) void store_u64(uint8_t bytes[8], uint64_t value)
{
  memcpy(bytes, &value, sizeof(value));
}

static inline __attribute__((__always_inline__)) int address_compare(const bytes20 *left, const bytes20 *right)
{
  return fixed_lane_compare(left->lanes, right->lanes, 20);
}

static inline __attribute__((__always_inline__)) bool word_equal(const u256 *left, const u256 *right)
{
  return (left->limbs[0] == right->limbs[0] && left->limbs[1] == right->limbs[1] &&
          left->limbs[2] == right->limbs[2] && left->limbs[3] == right->limbs[3]) != 0;
}

static inline __attribute__((__always_inline__)) bool word_all_zero(const u256 *value)
{
  return (value->limbs[0] | value->limbs[1] | value->limbs[2] | value->limbs[3]) == 0;
}

static inline __attribute__((__always_inline__)) int word_compare(const u256 *left, const u256 *right)
{
  for (size_t limb = 4; limb-- > 0;) {
    if (left->limbs[limb] < right->limbs[limb]) {
      return -1;
    }
    if (left->limbs[limb] > right->limbs[limb]) {
      return 1;
    }
  }
  return 0;
}

static inline __attribute__((__always_inline__)) void address_to_be_bytes(uint8_t out[20], bytes20 value)
{
  memcpy(out, bytes20_data(&value), 20);
}

static inline __attribute__((__always_inline__)) bytes20 address_from_be_bytes(const uint8_t in[20])
{
  bytes20 result = {{0}};
  memcpy(bytes20_data_mut(&result), in, 20);
  return result;
}

static inline __attribute__((__always_inline__)) bytes32 hash_from_be_bytes(const uint8_t in[32])
{
  bytes32 result;
  memcpy(bytes32_data_mut(&result), in, 32);
  return result;
}

static inline __attribute__((__always_inline__)) void hash_to_be_bytes(uint8_t out[32], bytes32 value)
{
  memcpy(out, bytes32_data(&value), 32);
}

static inline __attribute__((__always_inline__)) void sail_word_to_be_words4(uint64_t out[4], u256 value)
{
  out[0] = value.limbs[3];
  out[1] = value.limbs[2];
  out[2] = value.limbs[1];
  out[3] = value.limbs[0];
}

static inline __attribute__((__always_inline__)) void sail_word_to_le_words4(uint64_t out[4], u256 value)
{
  out[0] = value.limbs[0];
  out[1] = value.limbs[1];
  out[2] = value.limbs[2];
  out[3] = value.limbs[3];
}

static inline __attribute__((__always_inline__)) u256 be_words4_to_sail_word(const uint64_t words[4])
{
  u256 result = {{words[3], words[2], words[1], words[0]}};
  return result;
}

static inline __attribute__((__always_inline__)) u256 le_words4_to_sail_word(const uint64_t words[4])
{
  u256 result = {{words[0], words[1], words[2], words[3]}};
  return result;
}

static inline __attribute__((__always_inline__)) void sail_word_to_be_bytes(uint8_t out[32], u256 value)
{
  store_u64(out, bswap64(value.limbs[3]));
  store_u64(out + 8, bswap64(value.limbs[2]));
  store_u64(out + 16, bswap64(value.limbs[1]));
  store_u64(out + 24, bswap64(value.limbs[0]));
}

static inline __attribute__((__always_inline__)) u256 be_bytes_to_sail_word(const uint8_t in[32])
{
  return (u256){
      .limbs =
          {
              bswap64(load_u64(in + 24)),
              bswap64(load_u64(in + 16)),
              bswap64(load_u64(in + 8)),
              bswap64(load_u64(in)),
          },
  };
}

#endif
