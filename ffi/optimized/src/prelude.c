/* Optimized bindings for the conversion equations in prelude.sail. Fixed
 * byte vectors use canonical protocol order, while U256 limbs are
 * least-significant first. */
#include "evmsail/prelude.h"

#include "evmsail/primitives/word.h"
#include <stdint.h>

static inline uint64_t load_be64(const uint8_t bytes[8])
{
  return ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) | ((uint64_t)bytes[2] << 40) |
         ((uint64_t)bytes[3] << 32) | ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16) |
         ((uint64_t)bytes[6] << 8) | (uint64_t)bytes[7];
}

static inline uint32_t load_be32(const uint8_t bytes[4])
{
  return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) |
         (uint32_t)bytes[3];
}

U256 hash_to_word(Hash32 bytes)
{
  return (U256){
      .limbs =
          {
              load_be64(bytes.bytes + 24),
              load_be64(bytes.bytes + 16),
              load_be64(bytes.bytes + 8),
              load_be64(bytes.bytes),
          },
  };
}

Hash32 word_to_hash(U256 value)
{
  Hash32 result;
  store_be64(result.bytes, value.limbs[3]);
  store_be64(result.bytes + 8, value.limbs[2]);
  store_be64(result.bytes + 16, value.limbs[1]);
  store_be64(result.bytes + 24, value.limbs[0]);
  return result;
}

uint8_t u64_bit_length(uint64_t value)
{
  return value == UINT64_C(0) ? (uint8_t)0 : (uint8_t)(64 - __builtin_clzll(value));
}

Address word_to_address(U256 value)
{
  return address_from_word(value);
}

U256 address_to_word(Address bytes)
{
  return (U256){
      .limbs =
          {
              load_be64(bytes.bytes + 12),
              load_be64(bytes.bytes + 4),
              load_be32(bytes.bytes),
              0,
          },
  };
}
