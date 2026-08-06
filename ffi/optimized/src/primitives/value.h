/* Fixed word/address/hash conversions shared by the host FFI. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_VALUE_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_VALUE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Fixed-slice representations currently expose a mutable byte pointer even
 * for semantically read-only input, code, and scratch views. Qualified and
 * unqualified pointers have the same representation; copy that representation
 * without a cast so the one-way ABI adaptation stays explicit. Callers must
 * never write through the returned pointer. */
static inline uint8_t *sail_read_only_bytes(const uint8_t *bytes)
{
  uint8_t *result;
  _Static_assert(sizeof(result) == sizeof(bytes), "qualified pointer sizes must match");
  memcpy((void *)&result, (const void *)&bytes, sizeof(result));
  return result;
}

/*
 * The optimized fixed-byte representation stores the canonical protocol byte
 * sequence directly, without host-endian padding or lane reinterpretation.
 */
static inline const uint8_t *address_bytes_const(const Address *value)
{
  return value->bytes;
}

static inline uint8_t *address_bytes(Address *value)
{
  return value->bytes;
}

static inline const uint8_t *hash_bytes_const(const Hash32 *value)
{
  return value->bytes;
}

static inline uint8_t *hash_bytes(Hash32 *value)
{
  return value->bytes;
}

static inline bool address_equal(const Address *left, const Address *right)
{
  return memcmp(left->bytes, right->bytes, sizeof(left->bytes)) == 0;
}

static inline uint64_t load_u64(const uint8_t bytes[8])
{
  uint64_t value;
  memcpy(&value, bytes, sizeof(value));
  return value;
}

static inline void store_u64(uint8_t bytes[8], uint64_t value)
{
  memcpy(bytes, &value, sizeof(value));
}

static inline int address_compare(const Address *left, const Address *right)
{
  return memcmp(left->bytes, right->bytes, sizeof(left->bytes));
}

static inline bool word_equal(const U256 *left, const U256 *right)
{
  return (left->limbs[0] == right->limbs[0] && left->limbs[1] == right->limbs[1] &&
          left->limbs[2] == right->limbs[2] && left->limbs[3] == right->limbs[3]) != 0;
}

static inline bool word_all_zero(const U256 *value)
{
  return (value->limbs[0] | value->limbs[1] | value->limbs[2] | value->limbs[3]) == 0;
}

static inline int word_compare(const U256 *left, const U256 *right)
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

static inline void address_to_be_bytes(uint8_t out[20], Address value)
{
  memcpy(out, address_bytes_const(&value), 20);
}

static inline Address address_from_be_bytes(const uint8_t in[20])
{
  Address result = {{0}};
  memcpy(address_bytes(&result), in, 20);
  return result;
}

static inline Hash32 hash_from_be_bytes(const uint8_t in[32])
{
  Hash32 result;
  memcpy(hash_bytes(&result), in, 32);
  return result;
}

static inline void sail_word_to_be_words4(uint64_t out[4], U256 value)
{
  out[0] = value.limbs[3];
  out[1] = value.limbs[2];
  out[2] = value.limbs[1];
  out[3] = value.limbs[0];
}

static inline void sail_word_to_le_words4(uint64_t out[4], U256 value)
{
  out[0] = value.limbs[0];
  out[1] = value.limbs[1];
  out[2] = value.limbs[2];
  out[3] = value.limbs[3];
}

static inline U256 be_words4_to_sail_word(const uint64_t words[4])
{
  U256 result = {{words[3], words[2], words[1], words[0]}};
  return result;
}

static inline U256 le_words4_to_sail_word(const uint64_t words[4])
{
  U256 result = {{words[0], words[1], words[2], words[3]}};
  return result;
}

static inline void sail_word_to_be_bytes(uint8_t out[32], U256 value)
{
  store_u64(out, __builtin_bswap64(value.limbs[3]));
  store_u64(out + 8, __builtin_bswap64(value.limbs[2]));
  store_u64(out + 16, __builtin_bswap64(value.limbs[1]));
  store_u64(out + 24, __builtin_bswap64(value.limbs[0]));
}

static inline U256 be_bytes_to_sail_word(const uint8_t in[32])
{
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

#endif
