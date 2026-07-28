/*
 * Optimized EIP-4844 blob-base-fee calculation.
 *
 * The standard Sail model states fake_exponential directly over mathematical
 * integers. Optimized C evaluates the same recurrence in a fixed four-limb
 * accumulator and exposes the u128 block-context representation used by
 * Reth/Revm. The accumulator is capped at 2^128 * denominator, so every
 * multiplication that can still lead to a valid optimized result fits 256
 * bits. Inputs whose exact result exceeds that optimized domain fail closed.
 */
#include EVMSAIL_MODEL_H

#include "blob_fee_glue.h"
#include "optimized_exception.h"
#include "sail_abi.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum { BLOB_BIG_LIMBS = 4 };

struct blob_big {
  uint64_t limbs[BLOB_BIG_LIMBS];
};

struct blob_u128 {
  uint64_t low;
  uint64_t high;
};

static bool blob_big_is_zero(const struct blob_big *value) {
  uint64_t any = 0;
  for (size_t i = 0; i < BLOB_BIG_LIMBS; ++i) any |= value->limbs[i];
  return any == 0;
}

static int blob_big_compare(const struct blob_big *left,
                            const struct blob_big *right) {
  for (size_t i = BLOB_BIG_LIMBS; i-- > 0;) {
    if (left->limbs[i] < right->limbs[i]) return -1;
    if (left->limbs[i] > right->limbs[i]) return 1;
  }
  return 0;
}

static bool blob_big_add(struct blob_big *left,
                         const struct blob_big *right) {
  uint64_t carry = 0;
  for (size_t i = 0; i < BLOB_BIG_LIMBS; ++i) {
    const unsigned __int128 sum =
        (unsigned __int128)left->limbs[i] + right->limbs[i] + carry;
    left->limbs[i] = (uint64_t)sum;
    carry = (uint64_t)(sum >> 64);
  }
  return carry == 0;
}

static bool blob_big_mul_u64(struct blob_big *result,
                             const struct blob_big *value, uint64_t factor) {
  unsigned __int128 carry = 0;
  for (size_t i = 0; i < BLOB_BIG_LIMBS; ++i) {
    const unsigned __int128 product =
        (unsigned __int128)value->limbs[i] * factor + carry;
    result->limbs[i] = (uint64_t)product;
    carry = product >> 64;
  }
  return carry == 0;
}

static bool blob_remainder_ge(const uint64_t remainder[3],
                              struct blob_u128 divisor) {
  return remainder[2] != 0 || remainder[1] > divisor.high ||
         (remainder[1] == divisor.high && remainder[0] >= divisor.low);
}

static void blob_remainder_sub(uint64_t remainder[3],
                               struct blob_u128 divisor) {
  const uint64_t low = remainder[0];
  remainder[0] -= divisor.low;
  const uint64_t low_borrow = low < divisor.low;
  const uint64_t high = remainder[1];
  remainder[1] -= divisor.high + low_borrow;
  const uint64_t high_borrow =
      high < divisor.high || (low_borrow && high == divisor.high);
  remainder[2] -= high_borrow;
}

/*
 * Binary long division needs only shifts, comparisons, and subtraction. This
 * intentionally avoids compiler u128 division libcalls in the freestanding
 * RISC-V guest.
 */
static void blob_big_div_u128(struct blob_big *quotient,
                              const struct blob_big *numerator,
                              struct blob_u128 divisor) {
  uint64_t remainder[3] = {0, 0, 0};
  memset(quotient, 0, sizeof(*quotient));
  for (size_t bit = BLOB_BIG_LIMBS * 64; bit-- > 0;) {
    remainder[2] = (remainder[2] << 1) | (remainder[1] >> 63);
    remainder[1] = (remainder[1] << 1) | (remainder[0] >> 63);
    remainder[0] =
        (remainder[0] << 1) |
        ((numerator->limbs[bit / 64] >> (bit % 64)) & UINT64_C(1));
    if (blob_remainder_ge(remainder, divisor)) {
      blob_remainder_sub(remainder, divisor);
      quotient->limbs[bit / 64] |= UINT64_C(1) << (bit % 64);
    }
  }
}

static struct blob_u128 blob_u128_mul(uint64_t left, uint64_t right) {
  const unsigned __int128 product = (unsigned __int128)left * right;
  return (struct blob_u128){
      .low = (uint64_t)product,
      .high = (uint64_t)(product >> 64),
  };
}

static sail_u128 blob_base_fee_invalid(void) {
  evmsail_throw_invalid_block(zExecutionInvalid, "optimized blob base fee");
  return (sail_u128){{0, 0}};
}

sail_u128 evmsail_blob_base_fee(uint64_t excess_blob_gas) {
  const uint64_t denominator = zk_blob_schedule.zbase_fee_update_fraction;
  struct blob_big output = {{0}};
  struct blob_big term = {{denominator, 0, 0, 0}};
  struct blob_big output_limit = {{0, 0, denominator, 0}};
  uint64_t iteration = 1;

  while (!blob_big_is_zero(&term)) {
    if (!blob_big_add(&output, &term) ||
        blob_big_compare(&output, &output_limit) >= 0) {
      return blob_base_fee_invalid();
    }

    struct blob_big product;
    if (!blob_big_mul_u64(&product, &term, excess_blob_gas)) {
      return blob_base_fee_invalid();
    }
    const struct blob_u128 divisor =
        blob_u128_mul(denominator, iteration);
    blob_big_div_u128(&term, &product, divisor);
    if (iteration == UINT64_MAX) {
      return blob_base_fee_invalid();
    }
    iteration++;
  }

  struct blob_big fee;
  blob_big_div_u128(
      &fee, &output,
      (struct blob_u128){.low = denominator, .high = UINT64_C(0)});
  if ((fee.limbs[2] | fee.limbs[3]) != 0) {
    return blob_base_fee_invalid();
  }
  return (sail_u128){{fee.limbs[0], fee.limbs[1]}};
}
