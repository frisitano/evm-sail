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
              bswap64(bytes.lanes[3]),
              bswap64(bytes.lanes[2]),
              bswap64(bytes.lanes[1]),
              bswap64(bytes.lanes[0]),
          },
  };
}

bytes32 word_to_hash(u256 value)
{
  return (bytes32){
      .lanes =
          {
              bswap64(value.limbs[3]),
              bswap64(value.limbs[2]),
              bswap64(value.limbs[1]),
              bswap64(value.limbs[0]),
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
              bswap64((lane1 >> 32) | (lane2 << 32)),
              bswap64((lane0 >> 32) | (lane1 << 32)),
              bswap32((uint32_t)lane0),
              0,
          },
  };
}

/* 256-bit low product over 64-bit limbs; the standard schoolbook carry
 * bound keeps every partial sum inside 128 bits. */
static u256 word_mul_low(const u256 *a, const u256 *b)
{
  uint64_t out[4] = {0, 0, 0, 0};
  for (unsigned i = 0; i < 4; i++) {
    unsigned __int128 carry = 0;
    for (unsigned j = 0; i + j < 4; j++) {
      carry += (unsigned __int128)a->limbs[i] * b->limbs[j] + out[i + j];
      out[i + j] = (uint64_t)carry;
      carry >>= 64;
    }
  }
  return (u256){.limbs = {out[0], out[1], out[2], out[3]}};
}

/* Square-and-multiply over exactly the exponent's significant bits; the
 * final squaring feeds no later round and is skipped. Matches the canonical
 * alu_exp ladder in sail/prelude.sail. */
u256 word_exp_ladder(u256 base, u256 exponent)
{
  u256 result = {.limbs = {1, 0, 0, 0}};
  u256 b = base;
  int top = -1;
  for (int limb = 3; limb >= 0; limb--) {
    if (exponent.limbs[limb] != 0) {
      top = limb * 64 + 63 - __builtin_clzll(exponent.limbs[limb]);
      break;
    }
  }
  for (int bit = 0; bit <= top; bit++) {
    if ((exponent.limbs[bit >> 6] >> (bit & 63)) & UINT64_C(1)) {
      result = word_mul_low(&result, &b);
    }
    if (bit < top) {
      b = word_mul_low(&b, &b);
    }
  }
  return result;
}
