/* Optimized bindings for the conversion equations in prelude.sail. Fixed
 * byte vectors use canonical protocol order, while u256 limbs are
 * least-significant first. */
#include "evmsail/prelude.h"

#include "evmsail/primitives/word.h"
#include <stdint.h>

u256 hash_to_word(bytes32 bytes)
{
  return (u256){
      .limbs =
          {
              __builtin_bswap64(bytes.lanes[3]),
              __builtin_bswap64(bytes.lanes[2]),
              __builtin_bswap64(bytes.lanes[1]),
              __builtin_bswap64(bytes.lanes[0]),
          },
  };
}

bytes32 word_to_hash(u256 value)
{
  return (bytes32){
      .lanes =
          {
              __builtin_bswap64(value.limbs[3]),
              __builtin_bswap64(value.limbs[2]),
              __builtin_bswap64(value.limbs[1]),
              __builtin_bswap64(value.limbs[0]),
          },
  };
}

uint8_t u64_bit_length(uint64_t value)
{
  return value == UINT64_C(0) ? (uint8_t)0 : (uint8_t)(64 - __builtin_clzll(value));
}

bytes20 word_to_address(u256 value)
{
  return address_from_word(value);
}

u256 address_to_word(bytes20 bytes)
{
  const uint64_t lane0 = bytes.lanes[0];
  const uint64_t lane1 = bytes.lanes[1];
  const uint64_t lane2 = bytes.lanes[2] & UINT64_C(0xffffffff);
  return (u256){
      .limbs =
          {
              __builtin_bswap64((lane1 >> 32) | (lane2 << 32)),
              __builtin_bswap64((lane0 >> 32) | (lane1 << 32)),
              __builtin_bswap32((uint32_t)lane0),
              0,
          },
  };
}
