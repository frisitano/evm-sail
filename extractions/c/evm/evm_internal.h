/* Generated unity-build context. Public consumers should include evm.h. */
#pragma once
#include "sail.h"
#include <string.h>
#include "cycle_scopes.h"
#include "state_db.h"
#include "trie_node_db.h"
#include "kernel_state.h"
#include "code_db.h"
#include "stack.h"
#include "transient_storage.h"
#include "memory.h"
#include "scratch.h"
#include "output.h"
#include "precompiles.h"
#include "host_crypto.h"
#include "byte_slice_glue.h"
#include "sail_failure.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "evm.h"


static inline void sail_native_conversion_failure(const char *operation) {
  fprintf(stderr, "Sail C backend: %s\n", operation);
  exit(EXIT_FAILURE);
}



static inline sail_u256 u256_zero(void) {
  sail_u256 result = {{0}};
  return result;
}

static inline sail_u256 u256_of_u128(const sail_u128 value) {
  sail_u256 result = {{value.limbs[0], value.limbs[1], UINT64_C(0), UINT64_C(0)}};
  return result;
}

static inline sail_u128 u128_of_u256(const sail_u256 value) {
  if (value.limbs[2] != UINT64_C(0) || value.limbs[3] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint128_t domain");
  }
  sail_u128 result = {{value.limbs[0], value.limbs[1]}};
  return result;
}

static inline uint64_t u256_to_u64(const sail_u256 value) {
  if (value.limbs[1] != UINT64_C(0)
      || value.limbs[2] != UINT64_C(0)
      || value.limbs[3] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint64_t domain");
  }
  return value.limbs[0];
}

static inline bool eq_u256(const sail_u256 lhs, const sail_u256 rhs) {
  return lhs.limbs[0] == rhs.limbs[0]
      && lhs.limbs[1] == rhs.limbs[1]
      && lhs.limbs[2] == rhs.limbs[2]
      && lhs.limbs[3] == rhs.limbs[3];
}

static inline uint64_t u256_bit(const sail_u256 value, const int64_t index) {
  if (index < 0 || index >= 256) return UINT64_C(0);
  return (value.limbs[(uint64_t)index >> 6] >> ((uint64_t)index & UINT64_C(63))) & UINT64_C(1);
}

static inline uint64_t u256_extract_u64(const sail_u256 value, const uint64_t start) {
  if (start >= UINT64_C(256)) return UINT64_C(0);
  const size_t limb = (size_t)(start >> 6);
  const unsigned offset = (unsigned)(start & UINT64_C(63));
  uint64_t result = value.limbs[limb] >> offset;
  if (offset != 0 && limb + 1 < 4) result |= value.limbs[limb + 1] << (64 - offset);
  return result;
}

static inline sail_u256 u256_update_u64(sail_u256 value, const uint64_t index,
                                        const uint64_t bit) {
  if (index >= UINT64_C(256)) return value;
  const size_t limb = (size_t)(index >> 6);
  const uint64_t mask = UINT64_C(1) << (index & UINT64_C(63));
  if ((bit & UINT64_C(1)) != 0) value.limbs[limb] |= mask;
  else value.limbs[limb] &= ~mask;
  return value;
}

static inline sail_u256 u256_update_i64(sail_u256 value, const int64_t index,
                                        const uint64_t bit) {
  if (index < INT64_C(0)) return value;
  return u256_update_u64(value, (uint64_t)index, bit);
}

static inline uint64_t fast_vector_access_u256(const sail_u256 value, const int64_t index) {
  return u256_bit(value, index);
}

static inline sail_u256 u256_not(const sail_u256 value) {
  sail_u256 result;
  for (size_t i = 0; i < 4; ++i) result.limbs[i] = ~value.limbs[i];
  return result;
}

static inline sail_u256 u256_and(const sail_u256 lhs, const sail_u256 rhs) {
  sail_u256 result;
  for (size_t i = 0; i < 4; ++i) result.limbs[i] = lhs.limbs[i] & rhs.limbs[i];
  return result;
}

static inline sail_u256 u256_or(const sail_u256 lhs, const sail_u256 rhs) {
  sail_u256 result;
  for (size_t i = 0; i < 4; ++i) result.limbs[i] = lhs.limbs[i] | rhs.limbs[i];
  return result;
}

static inline sail_u256 u256_xor(const sail_u256 lhs, const sail_u256 rhs) {
  sail_u256 result;
  for (size_t i = 0; i < 4; ++i) result.limbs[i] = lhs.limbs[i] ^ rhs.limbs[i];
  return result;
}

static inline sail_u256 u256_add(const sail_u256 lhs, const sail_u256 rhs) {
  sail_u256 result;
  uint64_t carry = UINT64_C(0);
  for (size_t i = 0; i < 4; ++i) {
    const uint64_t partial = lhs.limbs[i] + rhs.limbs[i];
    const uint64_t carry1 = partial < lhs.limbs[i];
    result.limbs[i] = partial + carry;
    const uint64_t carry2 = result.limbs[i] < partial;
    carry = carry1 | carry2;
  }
  return result;
}

static inline sail_u256 u256_add_u128_u128(const sail_u128 lhs,
                                           const sail_u128 rhs) {
  sail_u256 result = {{0}};
  const unsigned __int128 low =
      (unsigned __int128)lhs.limbs[0] + rhs.limbs[0];
  result.limbs[0] = (uint64_t)low;
  const unsigned __int128 high =
      (unsigned __int128)lhs.limbs[1] + rhs.limbs[1] + (low >> 64);
  result.limbs[1] = (uint64_t)high;
  result.limbs[2] = (uint64_t)(high >> 64);
  return result;
}

static inline sail_u256 u256_add_u128_u64(const sail_u128 lhs,
                                          const uint64_t rhs) {
  sail_u256 result = {{0}};
  const unsigned __int128 low = (unsigned __int128)lhs.limbs[0] + rhs;
  result.limbs[0] = (uint64_t)low;
  const unsigned __int128 high = (unsigned __int128)lhs.limbs[1] + (low >> 64);
  result.limbs[1] = (uint64_t)high;
  result.limbs[2] = (uint64_t)(high >> 64);
  return result;
}

static inline sail_u256 u256_add_u64_u128(const uint64_t lhs,
                                          const sail_u128 rhs) {
  return u256_add_u128_u64(rhs, lhs);
}

static inline sail_u256 u256_sub(const sail_u256 lhs, const sail_u256 rhs) {
  sail_u256 result;
  uint64_t borrow = UINT64_C(0);
  for (size_t i = 0; i < 4; ++i) {
    const uint64_t partial = lhs.limbs[i] - rhs.limbs[i];
    const uint64_t borrow1 = lhs.limbs[i] < rhs.limbs[i];
    result.limbs[i] = partial - borrow;
    const uint64_t borrow2 = partial < borrow;
    borrow = borrow1 | borrow2;
  }
  return result;
}

static inline sail_u256 u256_mul(const sail_u256 lhs, const sail_u256 rhs) {
  sail_u256 result = {{0}};
  for (size_t i = 0; i < 4; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; i + j < 4; ++j) {
      const size_t k = i + j;
      const unsigned __int128 product = (unsigned __int128)lhs.limbs[i] * rhs.limbs[j];
      const unsigned __int128 sum = product + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
  }
  return result;
}

static inline sail_u256 u256_mul_u128_u128(const sail_u128 lhs,
                                           const sail_u128 rhs) {
  sail_u256 result = {{0}};
  for (size_t i = 0; i < 2; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; j < 2; ++j) {
      const size_t k = i + j;
      const unsigned __int128 sum =
          (unsigned __int128)lhs.limbs[i] * rhs.limbs[j]
          + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
    result.limbs[i + 2] = (uint64_t)carry;
  }
  return result;
}

static inline sail_u256 u256_mul_u128_u64(const sail_u128 lhs,
                                          const uint64_t rhs) {
  sail_u256 result = {{0}};
  unsigned __int128 carry = 0;
  for (size_t i = 0; i < 2; ++i) {
    const unsigned __int128 product =
        (unsigned __int128)lhs.limbs[i] * rhs + carry;
    result.limbs[i] = (uint64_t)product;
    carry = product >> 64;
  }
  result.limbs[2] = (uint64_t)carry;
  return result;
}

static inline sail_u256 u256_mul_u64_u128(const uint64_t lhs,
                                          const sail_u128 rhs) {
  return u256_mul_u128_u64(rhs, lhs);
}

static inline bool u256_is_zero(const sail_u256 value) {
  return (value.limbs[0] | value.limbs[1] | value.limbs[2] | value.limbs[3])
      == UINT64_C(0);
}

static inline bool u256_lt(const sail_u256 lhs, const sail_u256 rhs) {
  for (size_t i = 4; i-- > 0;) {
    if (lhs.limbs[i] != rhs.limbs[i]) return lhs.limbs[i] < rhs.limbs[i];
  }
  return false;
}

static inline bool u256_eq_u128(const sail_u256 lhs, const sail_u128 rhs) {
  return lhs.limbs[0] == rhs.limbs[0]
      && lhs.limbs[1] == rhs.limbs[1]
      && lhs.limbs[2] == UINT64_C(0)
      && lhs.limbs[3] == UINT64_C(0);
}

static inline bool u256_lt_u128(const sail_u256 lhs, const sail_u128 rhs) {
  if ((lhs.limbs[2] | lhs.limbs[3]) != UINT64_C(0)) return false;
  if (lhs.limbs[1] != rhs.limbs[1]) return lhs.limbs[1] < rhs.limbs[1];
  return lhs.limbs[0] < rhs.limbs[0];
}

static inline bool u128_lt_u256(const sail_u128 lhs, const sail_u256 rhs) {
  if ((rhs.limbs[2] | rhs.limbs[3]) != UINT64_C(0)) return true;
  if (lhs.limbs[1] != rhs.limbs[1]) return lhs.limbs[1] < rhs.limbs[1];
  return lhs.limbs[0] < rhs.limbs[0];
}

static inline bool u256_eq_u64(const sail_u256 lhs, const uint64_t rhs) {
  return lhs.limbs[0] == rhs
      && lhs.limbs[1] == UINT64_C(0)
      && lhs.limbs[2] == UINT64_C(0)
      && lhs.limbs[3] == UINT64_C(0);
}

static inline bool u256_lt_u64(const sail_u256 lhs, const uint64_t rhs) {
  return lhs.limbs[1] == UINT64_C(0)
      && lhs.limbs[2] == UINT64_C(0)
      && lhs.limbs[3] == UINT64_C(0)
      && lhs.limbs[0] < rhs;
}

static inline bool u64_lt_u256(const uint64_t lhs, const sail_u256 rhs) {
  return rhs.limbs[1] != UINT64_C(0)
      || rhs.limbs[2] != UINT64_C(0)
      || rhs.limbs[3] != UINT64_C(0)
      || lhs < rhs.limbs[0];
}

static inline sail_u256 u256_add_u64(const sail_u256 lhs,
                                     const uint64_t rhs) {
  sail_u256 result = lhs;
  result.limbs[0] += rhs;
  uint64_t carry = result.limbs[0] < lhs.limbs[0];
  for (size_t i = 1; i < 4 && carry != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]++;
    carry = result.limbs[i] < previous;
  }
  return result;
}

static inline sail_u256 u256_add_u128(const sail_u256 lhs,
                                      const sail_u128 rhs) {
  sail_u256 result = lhs;
  unsigned __int128 sum =
      (unsigned __int128)lhs.limbs[0] + rhs.limbs[0];
  result.limbs[0] = (uint64_t)sum;
  sum = (unsigned __int128)lhs.limbs[1] + rhs.limbs[1] + (sum >> 64);
  result.limbs[1] = (uint64_t)sum;
  uint64_t carry = (uint64_t)(sum >> 64);
  for (size_t i = 2; i < 4 && carry != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]++;
    carry = result.limbs[i] < previous;
  }
  return result;
}

static inline sail_u256 u256_sub_u64(const sail_u256 lhs,
                                     const uint64_t rhs) {
  sail_u256 result = lhs;
  result.limbs[0] -= rhs;
  uint64_t borrow = lhs.limbs[0] < rhs;
  for (size_t i = 1; i < 4 && borrow != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]--;
    borrow = previous == UINT64_C(0);
  }
  return result;
}

static inline sail_u256 u256_sub_u128(const sail_u256 lhs,
                                      const sail_u128 rhs) {
  sail_u256 result = lhs;
  const uint64_t low = lhs.limbs[0] - rhs.limbs[0];
  uint64_t borrow = lhs.limbs[0] < rhs.limbs[0];
  const uint64_t high = lhs.limbs[1] - rhs.limbs[1];
  const uint64_t borrow1 = lhs.limbs[1] < rhs.limbs[1];
  result.limbs[0] = low;
  result.limbs[1] = high - borrow;
  const uint64_t borrow2 = high < borrow;
  borrow = borrow1 | borrow2;
  for (size_t i = 2; i < 4 && borrow != UINT64_C(0); ++i) {
    const uint64_t previous = result.limbs[i];
    result.limbs[i]--;
    borrow = previous == UINT64_C(0);
  }
  return result;
}

/* A valid natural subtraction with a u128 minuend proves that the u256
 * subtrahend's upper limbs are zero. */
static inline sail_u128 u128_sub_u256(const sail_u128 lhs,
                                      const sail_u256 rhs) {
  sail_u128 result;
  result.limbs[0] = lhs.limbs[0] - rhs.limbs[0];
  const uint64_t borrow = lhs.limbs[0] < rhs.limbs[0];
  result.limbs[1] = lhs.limbs[1] - rhs.limbs[1] - borrow;
  return result;
}

/* The Sail range checker proves that rhs fits and rhs <= lhs at each emitted
 * u64 - u256 call site.  The helper only expresses the selected C
 * representation; it does not add a second runtime semantics. */
static inline uint64_t u64_sub_u256(const uint64_t lhs,
                                    const sail_u256 rhs) {
  return lhs - rhs.limbs[0];
}

static inline sail_u256 u256_mul_u64(const sail_u256 lhs,
                                     const uint64_t rhs) {
  sail_u256 result = {{0}};
  unsigned __int128 carry = 0;
  for (size_t i = 0; i < 4; ++i) {
    const unsigned __int128 product =
        (unsigned __int128)lhs.limbs[i] * rhs + carry;
    result.limbs[i] = (uint64_t)product;
    carry = product >> 64;
  }
  return result;
}

static inline sail_u256 u256_mul_u128(const sail_u256 lhs,
                                      const sail_u128 rhs) {
  sail_u256 result = {{0}};
  for (size_t i = 0; i < 4; ++i) {
    unsigned __int128 carry = 0;
    for (size_t j = 0; j < 2 && i + j < 4; ++j) {
      const size_t k = i + j;
      const unsigned __int128 sum =
          (unsigned __int128)lhs.limbs[i] * rhs.limbs[j]
          + result.limbs[k] + carry;
      result.limbs[k] = (uint64_t)sum;
      carry = sum >> 64;
    }
    if (i + 2 < 4) result.limbs[i + 2] = (uint64_t)carry;
  }
  return result;
}

static inline sail_u256 u256_div_u64(const sail_u256 dividend,
                                     const uint64_t divisor) {
  sail_u256 quotient = {{0}};
  uint64_t remainder = UINT64_C(0);
  for (size_t i = 4; i-- > 0;) {
    const unsigned __int128 partial =
        ((unsigned __int128)remainder << 64) | dividend.limbs[i];
    quotient.limbs[i] = (uint64_t)(partial / divisor);
    remainder = (uint64_t)(partial % divisor);
  }
  return quotient;
}

static inline sail_u256 u256_mod_u64(const sail_u256 dividend,
                                     const uint64_t divisor) {
  sail_u256 result = {{0}};
  uint64_t remainder = UINT64_C(0);
  for (size_t i = 4; i-- > 0;) {
    const unsigned __int128 partial =
        ((unsigned __int128)remainder << 64) | dividend.limbs[i];
    remainder = (uint64_t)(partial % divisor);
  }
  result.limbs[0] = remainder;
  return result;
}

static inline size_t u256_significant_words(const uint64_t *value,
                                            size_t count) {
  while (count != 0 && value[count - 1] == UINT64_C(0)) count--;
  return count;
}

static inline unsigned u256_leading_zeros(const uint64_t value) {
  return value == UINT64_C(0) ? 64U : (unsigned)__builtin_clzll(value);
}

/* Knuth division, Algorithm D, in base 2^64.  The numerator has at most
 * eight limbs and the divisor at most four.  Callers either carry the JIB
 * nonzero proof or implement an explicit source-level zero-modulus branch. */
static inline void u256_divrem_words(const uint64_t *numerator,
                                     size_t numerator_count,
                                     const uint64_t divisor[4],
                                     uint64_t quotient[8],
                                     uint64_t remainder[4]) {
  uint64_t u[9] = {0};
  uint64_t v[4] = {0};
  const size_t un = u256_significant_words(numerator, numerator_count);
  const size_t vn = u256_significant_words(divisor, 4);

  if (quotient != NULL)
    for (size_t i = 0; i < 8; ++i) quotient[i] = UINT64_C(0);
  if (remainder != NULL)
    for (size_t i = 0; i < 4; ++i) remainder[i] = UINT64_C(0);
  if (un == 0) return;
  if (un < vn) {
    if (remainder != NULL)
      for (size_t i = 0; i < un; ++i) remainder[i] = numerator[i];
    return;
  }

  if (vn == 1) {
    uint64_t rem = UINT64_C(0);
    for (size_t i = un; i-- > 0;) {
      const unsigned __int128 partial =
          ((unsigned __int128)rem << 64) | numerator[i];
      if (quotient != NULL)
        quotient[i] = (uint64_t)(partial / divisor[0]);
      rem = (uint64_t)(partial % divisor[0]);
    }
    if (remainder != NULL) remainder[0] = rem;
    return;
  }

  const unsigned shift = u256_leading_zeros(divisor[vn - 1]);
  if (shift == 0) {
    for (size_t i = 0; i < vn; ++i) v[i] = divisor[i];
    for (size_t i = 0; i < un; ++i) u[i] = numerator[i];
  } else {
    uint64_t carry = UINT64_C(0);
    for (size_t i = 0; i < vn; ++i) {
      const uint64_t next = divisor[i] >> (64 - shift);
      v[i] = (divisor[i] << shift) | carry;
      carry = next;
    }
    carry = UINT64_C(0);
    for (size_t i = 0; i < un; ++i) {
      const uint64_t next = numerator[i] >> (64 - shift);
      u[i] = (numerator[i] << shift) | carry;
      carry = next;
    }
    u[un] = carry;
  }

  const size_t qn = un - vn + 1;
  for (size_t jj = qn; jj-- > 0;) {
    const size_t j = jj;
    uint64_t qhat;
    uint64_t rhat;
    bool rhat_overflow = false;
    if (u[j + vn] == v[vn - 1]) {
      qhat = UINT64_MAX;
      rhat = u[j + vn - 1] + v[vn - 1];
      rhat_overflow = rhat < u[j + vn - 1];
    } else {
      const unsigned __int128 top =
          ((unsigned __int128)u[j + vn] << 64) | u[j + vn - 1];
      qhat = (uint64_t)(top / v[vn - 1]);
      rhat = (uint64_t)(top % v[vn - 1]);
    }

    while (!rhat_overflow
           && (unsigned __int128)qhat * v[vn - 2]
                  > (((unsigned __int128)rhat << 64) | u[j + vn - 2])) {
      qhat--;
      const uint64_t next = rhat + v[vn - 1];
      rhat_overflow = next < rhat;
      rhat = next;
    }

    uint64_t borrow = UINT64_C(0);
    for (size_t i = 0; i < vn; ++i) {
      const unsigned __int128 product =
          (unsigned __int128)qhat * v[i] + borrow;
      const uint64_t low = (uint64_t)product;
      borrow = (uint64_t)(product >> 64) + (u[j + i] < low);
      u[j + i] -= low;
    }
    const bool negative = u[j + vn] < borrow;
    u[j + vn] -= borrow;

    if (negative) {
      qhat--;
      uint64_t carry = UINT64_C(0);
      for (size_t i = 0; i < vn; ++i) {
        const unsigned __int128 sum =
            (unsigned __int128)u[j + i] + v[i] + carry;
        u[j + i] = (uint64_t)sum;
        carry = (uint64_t)(sum >> 64);
      }
      u[j + vn] += carry;
    }
    if (quotient != NULL) quotient[j] = qhat;
  }

  if (remainder != NULL) {
    if (shift == 0) {
      for (size_t i = 0; i < vn; ++i) remainder[i] = u[i];
    } else {
      for (size_t i = 0; i < vn; ++i) {
        remainder[i] = u[i] >> shift;
        if (i + 1 < vn) remainder[i] |= u[i + 1] << (64 - shift);
      }
    }
  }
}

static inline sail_u256 u256_div_u128(const sail_u256 dividend,
                                      const sail_u128 divisor) {
  sail_u256 result = {{0}};
  const uint64_t words[4] = {
      divisor.limbs[0], divisor.limbs[1], UINT64_C(0), UINT64_C(0)};
  uint64_t quotient[8] = {0};
  u256_divrem_words(dividend.limbs, 4, words, quotient, NULL);
  for (size_t i = 0; i < 4; ++i) result.limbs[i] = quotient[i];
  return result;
}

static inline sail_u256 u256_mod_u128(const sail_u256 dividend,
                                      const sail_u128 divisor) {
  sail_u256 result = {{0}};
  const uint64_t words[4] = {
      divisor.limbs[0], divisor.limbs[1], UINT64_C(0), UINT64_C(0)};
  u256_divrem_words(dividend.limbs, 4, words, NULL, result.limbs);
  return result;
}

static inline sail_u128 u128_div_u256(const sail_u128 dividend,
                                      const sail_u256 divisor) {
  sail_u128 result = {{0}};
  if ((divisor.limbs[2] | divisor.limbs[3]) != UINT64_C(0)) return result;
  uint64_t quotient[8] = {0};
  u256_divrem_words(dividend.limbs, 2, divisor.limbs, quotient, NULL);
  result.limbs[0] = quotient[0];
  result.limbs[1] = quotient[1];
  return result;
}

static inline sail_u128 u128_mod_u256(const sail_u128 dividend,
                                      const sail_u256 divisor) {
  if ((divisor.limbs[2] | divisor.limbs[3]) != UINT64_C(0)) return dividend;
  uint64_t remainder[4] = {0};
  u256_divrem_words(dividend.limbs, 2, divisor.limbs, NULL, remainder);
  sail_u128 result = {{remainder[0], remainder[1]}};
  return result;
}

static inline sail_u256 u256_div(const sail_u256 dividend,
                                 const sail_u256 divisor) {
  sail_u256 result = {{0}};
  uint64_t quotient[8] = {0};
  u256_divrem_words(dividend.limbs, 4, divisor.limbs, quotient, NULL);
  for (size_t i = 0; i < 4; ++i) result.limbs[i] = quotient[i];
  return result;
}

static inline sail_u256 u256_mod(const sail_u256 dividend,
                                 const sail_u256 divisor) {
  sail_u256 result = {{0}};
  u256_divrem_words(dividend.limbs, 4, divisor.limbs, NULL, result.limbs);
  return result;
}

static inline sail_u256 u256_addmod(const sail_u256 lhs,
                                    const sail_u256 rhs,
                                    const sail_u256 modulus) {
  sail_u256 result = {{0}};
  uint64_t sum[5] = {0};
  uint64_t carry = UINT64_C(0);
  for (size_t i = 0; i < 4; ++i) {
    const unsigned __int128 wide =
        (unsigned __int128)lhs.limbs[i] + rhs.limbs[i] + carry;
    sum[i] = (uint64_t)wide;
    carry = (uint64_t)(wide >> 64);
  }
  sum[4] = carry;
  if (!u256_is_zero(modulus))
    u256_divrem_words(sum, 5, modulus.limbs, NULL, result.limbs);
  return result;
}

static inline sail_u256 u256_mulmod(const sail_u256 lhs,
                                    const sail_u256 rhs,
                                    const sail_u256 modulus) {
  sail_u256 result = {{0}};
  uint64_t product[8] = {0};
  for (size_t i = 0; i < 4; ++i) {
    uint64_t carry = UINT64_C(0);
    for (size_t j = 0; j < 4; ++j) {
      const size_t k = i + j;
      const unsigned __int128 wide =
          (unsigned __int128)lhs.limbs[i] * rhs.limbs[j]
          + product[k] + carry;
      product[k] = (uint64_t)wide;
      carry = (uint64_t)(wide >> 64);
    }
    product[i + 4] = carry;
  }
  if (!u256_is_zero(modulus))
    u256_divrem_words(product, 8, modulus.limbs, NULL, result.limbs);
  return result;
}

static inline sail_u256 u256_shiftl_u64(const sail_u256 value, const uint64_t amount) {
  sail_u256 result = {{0}};
  if (amount >= UINT64_C(256)) return result;
  const size_t words = (size_t)(amount >> 6);
  const unsigned bits = (unsigned)(amount & UINT64_C(63));
  for (size_t dst = 4; dst-- > words;) {
    result.limbs[dst] = value.limbs[dst - words] << bits;
    if (bits != 0 && dst > words) {
      result.limbs[dst] |= value.limbs[dst - words - 1] >> (64 - bits);
    }
  }
  return result;
}

static inline sail_u256 u256_shiftr_u64(const sail_u256 value, const uint64_t amount) {
  sail_u256 result = {{0}};
  if (amount >= UINT64_C(256)) return result;
  const size_t words = (size_t)(amount >> 6);
  const unsigned bits = (unsigned)(amount & UINT64_C(63));
  for (size_t dst = 0; dst + words < 4; ++dst) {
    result.limbs[dst] = value.limbs[dst + words] >> bits;
    if (bits != 0 && dst + words + 1 < 4) {
      result.limbs[dst] |= value.limbs[dst + words + 1] << (64 - bits);
    }
  }
  return result;
}

static inline sail_u256 u256_arith_shiftr_u64(const sail_u256 value, const uint64_t amount) {
  if ((value.limbs[3] >> 63) == 0) return u256_shiftr_u64(value, amount);
  return u256_not(u256_shiftr_u64(u256_not(value), amount));
}

static inline uint64_t u256_abs_i64(const int64_t amount) {
  return amount < 0 ? (uint64_t)(-(amount + 1)) + UINT64_C(1) : (uint64_t)amount;
}

static inline sail_u256 u256_shiftl_i64(const sail_u256 value, const int64_t amount) {
  return u256_shiftl_u64(value, u256_abs_i64(amount));
}

static inline sail_u256 u256_shiftr_i64(const sail_u256 value, const int64_t amount) {
  return u256_shiftr_u64(value, u256_abs_i64(amount));
}

static inline sail_u256 u256_arith_shiftr_i64(const sail_u256 value, const int64_t amount) {
  return u256_arith_shiftr_u64(value, u256_abs_i64(amount));
}

static inline sail_u256 u256_of_fbits(const uint64_t value) {
  sail_u256 result = {{0}};
  result.limbs[0] = value;
  return result;
}

static inline void string_of_u256(sail_string *result, const sail_u256 value) {
  sail_free(*result);
  const int bytes = asprintf(
      result,
      "0x%016" PRIx64 "%016" PRIx64 "%016" PRIx64 "%016" PRIx64,
      value.limbs[3], value.limbs[2], value.limbs[1], value.limbs[0]);
  if (bytes == -1) {
    fprintf(stderr, "Could not print a 256-bit value\n");
  }
}


static void CREATE(zoptionzIU64zK)(struct zoptionzIU64zK *op) {
  op->kind = Kind_zNonezIU64zK;
}

static void RECREATE(zoptionzIU64zK)(struct zoptionzIU64zK *op) {

}

static void KILL(zoptionzIU64zK)(struct zoptionzIU64zK *op) {

}

static void COPY(zoptionzIU64zK)(struct zoptionzIU64zK *rop, struct zoptionzIU64zK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIU64zK: {
    rop->variants.zNonezIU64zK = op.variants.zNonezIU64zK;
    break;
  }
  case Kind_zSomezIU64zK: {
    rop->variants.zSomezIU64zK = op.variants.zSomezIU64zK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIU64zK)(struct zoptionzIU64zK op1, struct zoptionzIU64zK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIU64zK: {
    return EQUAL(unit)(op1.variants.zNonezIU64zK, op2.variants.zNonezIU64zK);
    break;
  }
  case Kind_zSomezIU64zK: {
    return (op1.variants.zSomezIU64zK == op2.variants.zSomezIU64zK);
    break;
  }
  }
  return false;
}

static void zNonezIU64zK(struct zoptionzIU64zK *rop, unit op) {

  rop->kind = Kind_zNonezIU64zK;
  rop->variants.zNonezIU64zK = op;
}

static void zSomezIU64zK(struct zoptionzIU64zK *rop, uint64_t op) {

  rop->kind = Kind_zSomezIU64zK;
  rop->variants.zSomezIU64zK = op;
}

static void CREATE(zoptionzIR__sail_c_repr_u256zK)(struct zoptionzIR__sail_c_repr_u256zK *op) {
  op->kind = Kind_zNonezIR__sail_c_repr_u256zK;
}

static void RECREATE(zoptionzIR__sail_c_repr_u256zK)(struct zoptionzIR__sail_c_repr_u256zK *op) {

}

static void KILL(zoptionzIR__sail_c_repr_u256zK)(struct zoptionzIR__sail_c_repr_u256zK *op) {

}

static void COPY(zoptionzIR__sail_c_repr_u256zK)(struct zoptionzIR__sail_c_repr_u256zK *rop, struct zoptionzIR__sail_c_repr_u256zK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIR__sail_c_repr_u256zK: {
    rop->variants.zNonezIR__sail_c_repr_u256zK = op.variants.zNonezIR__sail_c_repr_u256zK;
    break;
  }
  case Kind_zSomezIR__sail_c_repr_u256zK: {
    rop->variants.zSomezIR__sail_c_repr_u256zK = op.variants.zSomezIR__sail_c_repr_u256zK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIR__sail_c_repr_u256zK)(struct zoptionzIR__sail_c_repr_u256zK op1, struct zoptionzIR__sail_c_repr_u256zK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIR__sail_c_repr_u256zK: {
    return EQUAL(unit)(op1.variants.zNonezIR__sail_c_repr_u256zK, op2.variants.zNonezIR__sail_c_repr_u256zK);
    break;
  }
  case Kind_zSomezIR__sail_c_repr_u256zK: {
    return EQUAL(u256)(op1.variants.zSomezIR__sail_c_repr_u256zK, op2.variants.zSomezIR__sail_c_repr_u256zK);
    break;
  }
  }
  return false;
}

static void zNonezIR__sail_c_repr_u256zK(struct zoptionzIR__sail_c_repr_u256zK *rop, unit op) {

  rop->kind = Kind_zNonezIR__sail_c_repr_u256zK;
  rop->variants.zNonezIR__sail_c_repr_u256zK = op;
}

static void zSomezIR__sail_c_repr_u256zK(struct zoptionzIR__sail_c_repr_u256zK *rop, sail_u256 op) {

  rop->kind = Kind_zSomezIR__sail_c_repr_u256zK;
  rop->variants.zSomezIR__sail_c_repr_u256zK = op;
}



static inline sail_u128 u128_zero(void) {
  sail_u128 result = {{0}};
  return result;
}

static inline sail_u128 u128_of_u64(const uint64_t value) {
  sail_u128 result = {{value, UINT64_C(0)}};
  return result;
}

static inline uint64_t u128_to_u64(const sail_u128 value) {
  if (value.limbs[1] != UINT64_C(0)) {
    sail_native_conversion_failure("integer value is outside the uint64_t domain");
  }
  return value.limbs[0];
}

static inline bool eq_u128(const sail_u128 lhs, const sail_u128 rhs) {
  return lhs.limbs[0] == rhs.limbs[0] && lhs.limbs[1] == rhs.limbs[1];
}

static inline bool u128_eq_u64(const sail_u128 lhs, const uint64_t rhs) {
  return lhs.limbs[0] == rhs && lhs.limbs[1] == UINT64_C(0);
}

static inline sail_u128 u128_not(const sail_u128 value) {
  sail_u128 result = {{~value.limbs[0], ~value.limbs[1]}};
  return result;
}

static inline sail_u128 u128_and(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result = {{lhs.limbs[0] & rhs.limbs[0], lhs.limbs[1] & rhs.limbs[1]}};
  return result;
}

static inline sail_u128 u128_or(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result = {{lhs.limbs[0] | rhs.limbs[0], lhs.limbs[1] | rhs.limbs[1]}};
  return result;
}

static inline sail_u128 u128_xor(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result = {{lhs.limbs[0] ^ rhs.limbs[0], lhs.limbs[1] ^ rhs.limbs[1]}};
  return result;
}

static inline bool u128_lt(const sail_u128 lhs, const sail_u128 rhs) {
  return lhs.limbs[1] != rhs.limbs[1]
      ? lhs.limbs[1] < rhs.limbs[1]
      : lhs.limbs[0] < rhs.limbs[0];
}

static inline bool u128_lt_u64(const sail_u128 lhs, const uint64_t rhs) {
  return lhs.limbs[1] == UINT64_C(0) && lhs.limbs[0] < rhs;
}

static inline bool u64_lt_u128(const uint64_t lhs, const sail_u128 rhs) {
  return rhs.limbs[1] != UINT64_C(0) || lhs < rhs.limbs[0];
}

static inline sail_u128 u128_add(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result;
  result.limbs[0] = lhs.limbs[0] + rhs.limbs[0];
  result.limbs[1] = lhs.limbs[1] + rhs.limbs[1]
                  + (result.limbs[0] < lhs.limbs[0]);
  return result;
}

static inline sail_u128 u128_add_u64(const sail_u128 lhs, const uint64_t rhs) {
  sail_u128 result = lhs;
  result.limbs[0] += rhs;
  result.limbs[1] += result.limbs[0] < lhs.limbs[0];
  return result;
}

static inline sail_u128 u128_add_u64_u64(const uint64_t lhs,
                                         const uint64_t rhs) {
  sail_u128 result = {{lhs + rhs, lhs + rhs < lhs}};
  return result;
}

static inline sail_u128 u128_sub(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result;
  result.limbs[0] = lhs.limbs[0] - rhs.limbs[0];
  result.limbs[1] = lhs.limbs[1] - rhs.limbs[1]
                  - (lhs.limbs[0] < rhs.limbs[0]);
  return result;
}

static inline sail_u128 u128_sub_u64(const sail_u128 lhs, const uint64_t rhs) {
  sail_u128 result = lhs;
  result.limbs[0] -= rhs;
  result.limbs[1] -= lhs.limbs[0] < rhs;
  return result;
}

static inline uint64_t u64_sub_u128(const uint64_t lhs, const sail_u128 rhs) {
  return lhs - rhs.limbs[0];
}

static inline sail_u128 u128_sub_u64_u64(const uint64_t lhs,
                                         const uint64_t rhs) {
  sail_u128 result = {{lhs - rhs, UINT64_C(0)}};
  return result;
}

static inline sail_u128 u128_mul(const sail_u128 lhs, const sail_u128 rhs) {
  const unsigned __int128 low = (unsigned __int128)lhs.limbs[0] * rhs.limbs[0];
  sail_u128 result;
  result.limbs[0] = (uint64_t)low;
  result.limbs[1] = (uint64_t)(low >> 64)
                  + lhs.limbs[0] * rhs.limbs[1]
                  + lhs.limbs[1] * rhs.limbs[0];
  return result;
}

static inline sail_u128 u128_mul_u64(const sail_u128 lhs, const uint64_t rhs) {
  const unsigned __int128 low = (unsigned __int128)lhs.limbs[0] * rhs;
  sail_u128 result;
  result.limbs[0] = (uint64_t)low;
  result.limbs[1] = (uint64_t)(low >> 64) + lhs.limbs[1] * rhs;
  return result;
}

static inline sail_u128 u128_mul_u64_u64(const uint64_t lhs,
                                         const uint64_t rhs) {
  const unsigned __int128 product = (unsigned __int128)lhs * rhs;
  sail_u128 result = {{(uint64_t)product, (uint64_t)(product >> 64)}};
  return result;
}

static inline bool u128_is_zero(const sail_u128 value) {
  return (value.limbs[0] | value.limbs[1]) == UINT64_C(0);
}

/* Integer division helpers are emitted only for JIB operations carrying the
 * nonzero-divisor proof marker.  Do not re-check that source invariant here:
 * an unproved division remains in Sail's mathematical integer runtime. */
static inline void u128_divrem_u64(const sail_u128 dividend,
                                   const uint64_t divisor,
  sail_u128 *quotient,
                                   uint64_t *remainder) {
  sail_u128 q = {{0}};
  uint64_t r = UINT64_C(0);
  q.limbs[1] = dividend.limbs[1] / divisor;
  r = dividend.limbs[1] % divisor;
  const unsigned __int128 partial =
      ((unsigned __int128)r << 64) | dividend.limbs[0];
  q.limbs[0] = (uint64_t)(partial / divisor);
  r = (uint64_t)(partial % divisor);
  if (quotient != NULL) *quotient = q;
  if (remainder != NULL) *remainder = r;
}

/* Two-limb specialization of normalized Knuth division.  Like ruint, it
 * dispatches the one-limb divisor case separately and computes at most one
 * quotient limb for a normalized two-limb divisor. */
static inline void u128_divrem(const sail_u128 dividend,
                               const sail_u128 divisor,
                               sail_u128 *quotient,
  sail_u128 *remainder) {
  sail_u128 q = {{0}};
  sail_u128 r = {{0}};
  if (divisor.limbs[1] == UINT64_C(0)) {
    uint64_t rem = UINT64_C(0);
    u128_divrem_u64(dividend, divisor.limbs[0], &q, &rem);
    r.limbs[0] = rem;
    goto done;
  }

  if (u128_lt(dividend, divisor)) {
    r = dividend;
    goto done;
  }

  const unsigned shift = (unsigned)__builtin_clzll(divisor.limbs[1]);
  uint64_t v0;
  uint64_t v1;
  uint64_t u0;
  uint64_t u1;
  uint64_t u2;
  if (shift == 0) {
    v0 = divisor.limbs[0];
    v1 = divisor.limbs[1];
    u0 = dividend.limbs[0];
    u1 = dividend.limbs[1];
    u2 = UINT64_C(0);
  } else {
    v0 = divisor.limbs[0] << shift;
    v1 = (divisor.limbs[1] << shift)
       | (divisor.limbs[0] >> (64 - shift));
    u0 = dividend.limbs[0] << shift;
    u1 = (dividend.limbs[1] << shift)
       | (dividend.limbs[0] >> (64 - shift));
    u2 = dividend.limbs[1] >> (64 - shift);
  }

  uint64_t qhat;
  uint64_t rhat;
  bool rhat_overflow = false;
  if (u2 == v1) {
    qhat = UINT64_MAX;
    rhat = u1 + v1;
    rhat_overflow = rhat < u1;
  } else {
    const unsigned __int128 top = ((unsigned __int128)u2 << 64) | u1;
    qhat = (uint64_t)(top / v1);
    rhat = (uint64_t)(top % v1);
  }
  while (!rhat_overflow
         && (unsigned __int128)qhat * v0
                > (((unsigned __int128)rhat << 64) | u0)) {
    qhat--;
    const uint64_t next = rhat + v1;
    rhat_overflow = next < rhat;
    rhat = next;
  }

  const unsigned __int128 p0 = (unsigned __int128)qhat * v0;
  const uint64_t p0_low = (uint64_t)p0;
  const uint64_t p0_high = (uint64_t)(p0 >> 64);
  const uint64_t borrow0 = u0 < p0_low;
  u0 -= p0_low;

  const unsigned __int128 p1 = (unsigned __int128)qhat * v1
                              + p0_high + borrow0;
  const uint64_t p1_low = (uint64_t)p1;
  const uint64_t p1_high = (uint64_t)(p1 >> 64);
  const uint64_t borrow1 = u1 < p1_low;
  u1 -= p1_low;
  const uint64_t top_subtrahend = p1_high + borrow1;
  const bool top_overflow = top_subtrahend < p1_high;
  const bool negative = top_overflow || u2 < top_subtrahend;
  u2 -= top_subtrahend;

  if (negative) {
    qhat--;
    const uint64_t previous = u0;
    u0 += v0;
    const uint64_t carry = u0 < previous;
    u1 += v1 + carry;
  }
  q.limbs[0] = qhat;
  if (shift == 0) {
    r.limbs[0] = u0;
    r.limbs[1] = u1;
  } else {
    r.limbs[0] = (u0 >> shift) | (u1 << (64 - shift));
    r.limbs[1] = u1 >> shift;
  }

done:
  if (quotient != NULL) *quotient = q;
  if (remainder != NULL) *remainder = r;
}

static inline sail_u128 u128_div(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result;
  u128_divrem(lhs, rhs, &result, NULL);
  return result;
}

static inline sail_u128 u128_mod(const sail_u128 lhs, const sail_u128 rhs) {
  sail_u128 result;
  u128_divrem(lhs, rhs, NULL, &result);
  return result;
}

static inline sail_u128 u128_div_u64(const sail_u128 lhs, const uint64_t rhs) {
  sail_u128 result;
  u128_divrem_u64(lhs, rhs, &result, NULL);
  return result;
}

static inline sail_u128 u128_mod_u64(const sail_u128 lhs, const uint64_t rhs) {
  uint64_t remainder;
  u128_divrem_u64(lhs, rhs, NULL, &remainder);
  return u128_of_u64(remainder);
}



static inline sail_fixed_bytes_32 fixed_bytes_32_zero(void) {
  sail_fixed_bytes_32 result = {{0}};
  return result;
}

static inline bool eq_fixed_bytes_32(const sail_fixed_bytes_32 lhs, const sail_fixed_bytes_32 rhs) {
  return memcmp(lhs.bytes, rhs.bytes, 32) == 0;
}


#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } sail_u256;
#endif

static inline sail_u256 u256_from_fixed_bytes_32(const sail_fixed_bytes_32 value) {
  sail_u256 result = {{0}};
  for (size_t i = 0; i < 32; ++i) {
    result.limbs[i >> 3] |= ((uint64_t)value.bytes[i]) << ((i & 7) * 8);
  }
  return result;
}

static inline sail_fixed_bytes_32 fixed_bytes_32_from_u256(const sail_u256 value) {
  sail_fixed_bytes_32 result;
  for (size_t i = 0; i < 32; ++i) {
    result.bytes[i] = (uint8_t)(value.limbs[i >> 3] >> ((i & 7) * 8));
  }
  return result;
}

static inline sail_fixed_bytes_32 internal_vector_init_fixed_bytes_32(const int64_t length_arg) {
  (void)length_arg;
  return fixed_bytes_32_zero();
}

static inline sail_fixed_bytes_32 internal_vector_update_fixed_bytes_32(
    sail_fixed_bytes_32 value, const int64_t index, const uint64_t elem) {
  if (index >= 0 && index < 32) value.bytes[index] = (uint8_t)elem;
  return value;
}

static inline uint64_t fast_vector_access_fixed_bytes_32(const sail_fixed_bytes_32 value, const int64_t index) {
  return index >= 0 && index < 32 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline sail_fixed_bytes_32 fast_unsigned_vector_update_fixed_bytes_32(
    sail_fixed_bytes_32 value, const uint64_t index, const uint64_t elem) {
  if (index < 32) value.bytes[index] = (uint8_t)elem;
  return value;
}

static inline uint64_t fast_unsigned_vector_access_fixed_bytes_32(
    const sail_fixed_bytes_32 value, const uint64_t index) {
  return index < 32 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline sail_fixed_bytes_32 fast_vector_init_fixed_bytes_32(const int64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  sail_fixed_bytes_32 result;
  for (size_t i = 0; i < 32; ++i) result.bytes[i] = (uint8_t)elem;
  return result;
}

static inline sail_fixed_bytes_32 fast_unsigned_vector_init_fixed_bytes_32(const uint64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  sail_fixed_bytes_32 result;
  for (size_t i = 0; i < 32; ++i) result.bytes[i] = (uint8_t)elem;
  return result;
}


static void COPY(ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9)(struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 *rop, const struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9)(struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 op1, struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 op2) {
  return (op1.ztup0 == op2.ztup0) && EQUAL(u256)(op1.ztup1, op2.ztup1);
}

static void CREATE(zast)(struct zast *op) {
  op->kind = Kind_zADD;
}

static void RECREATE(zast)(struct zast *op) {

}

static void KILL(zast)(struct zast *op) {

}

static void COPY(zast)(struct zast *rop, struct zast op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zADD: {
    rop->variants.zADD = op.variants.zADD;
    break;
  }
  case Kind_zADDMOD: {
    rop->variants.zADDMOD = op.variants.zADDMOD;
    break;
  }
  case Kind_zADDRESS: {
    rop->variants.zADDRESS = op.variants.zADDRESS;
    break;
  }
  case Kind_zAND: {
    rop->variants.zAND = op.variants.zAND;
    break;
  }
  case Kind_zBALANCE: {
    rop->variants.zBALANCE = op.variants.zBALANCE;
    break;
  }
  case Kind_zBASEFEE: {
    rop->variants.zBASEFEE = op.variants.zBASEFEE;
    break;
  }
  case Kind_zBLOBBASEFEE: {
    rop->variants.zBLOBBASEFEE = op.variants.zBLOBBASEFEE;
    break;
  }
  case Kind_zBLOBHASH: {
    rop->variants.zBLOBHASH = op.variants.zBLOBHASH;
    break;
  }
  case Kind_zBLOCKHASH: {
    rop->variants.zBLOCKHASH = op.variants.zBLOCKHASH;
    break;
  }
  case Kind_zBYTE: {
    rop->variants.zBYTE = op.variants.zBYTE;
    break;
  }
  case Kind_zCALL: {
    rop->variants.zCALL = op.variants.zCALL;
    break;
  }
  case Kind_zCALLCODE: {
    rop->variants.zCALLCODE = op.variants.zCALLCODE;
    break;
  }
  case Kind_zCALLDATACOPY: {
    rop->variants.zCALLDATACOPY = op.variants.zCALLDATACOPY;
    break;
  }
  case Kind_zCALLDATALOAD: {
    rop->variants.zCALLDATALOAD = op.variants.zCALLDATALOAD;
    break;
  }
  case Kind_zCALLDATASIZE: {
    rop->variants.zCALLDATASIZE = op.variants.zCALLDATASIZE;
    break;
  }
  case Kind_zCALLER: {
    rop->variants.zCALLER = op.variants.zCALLER;
    break;
  }
  case Kind_zCALLVALUE: {
    rop->variants.zCALLVALUE = op.variants.zCALLVALUE;
    break;
  }
  case Kind_zCHAINID: {
    rop->variants.zCHAINID = op.variants.zCHAINID;
    break;
  }
  case Kind_zCLZ: {
    rop->variants.zCLZ = op.variants.zCLZ;
    break;
  }
  case Kind_zCODECOPY: {
    rop->variants.zCODECOPY = op.variants.zCODECOPY;
    break;
  }
  case Kind_zCODESIZE: {
    rop->variants.zCODESIZE = op.variants.zCODESIZE;
    break;
  }
  case Kind_zCOINBASE: {
    rop->variants.zCOINBASE = op.variants.zCOINBASE;
    break;
  }
  case Kind_zCREATE: {
    rop->variants.zCREATE = op.variants.zCREATE;
    break;
  }
  case Kind_zCREATE2: {
    rop->variants.zCREATE2 = op.variants.zCREATE2;
    break;
  }
  case Kind_zDELEGATECALL: {
    rop->variants.zDELEGATECALL = op.variants.zDELEGATECALL;
    break;
  }
  case Kind_zDIV: {
    rop->variants.zDIV = op.variants.zDIV;
    break;
  }
  case Kind_zDUP: {
    rop->variants.zDUP = op.variants.zDUP;
    break;
  }
  case Kind_zDUPN: {
    rop->variants.zDUPN = op.variants.zDUPN;
    break;
  }
  case Kind_zEQ: {
    rop->variants.zEQ = op.variants.zEQ;
    break;
  }
  case Kind_zEXCHANGE: {
    rop->variants.zEXCHANGE = op.variants.zEXCHANGE;
    break;
  }
  case Kind_zEXP: {
    rop->variants.zEXP = op.variants.zEXP;
    break;
  }
  case Kind_zEXTCODECOPY: {
    rop->variants.zEXTCODECOPY = op.variants.zEXTCODECOPY;
    break;
  }
  case Kind_zEXTCODEHASH: {
    rop->variants.zEXTCODEHASH = op.variants.zEXTCODEHASH;
    break;
  }
  case Kind_zEXTCODESIZE: {
    rop->variants.zEXTCODESIZE = op.variants.zEXTCODESIZE;
    break;
  }
  case Kind_zGAS: {
    rop->variants.zGAS = op.variants.zGAS;
    break;
  }
  case Kind_zGASLIMIT: {
    rop->variants.zGASLIMIT = op.variants.zGASLIMIT;
    break;
  }
  case Kind_zGASPRICE: {
    rop->variants.zGASPRICE = op.variants.zGASPRICE;
    break;
  }
  case Kind_zGT: {
    rop->variants.zGT = op.variants.zGT;
    break;
  }
  case Kind_zINVALID: {
    rop->variants.zINVALID = op.variants.zINVALID;
    break;
  }
  case Kind_zISZERO: {
    rop->variants.zISZERO = op.variants.zISZERO;
    break;
  }
  case Kind_zJUMP: {
    rop->variants.zJUMP = op.variants.zJUMP;
    break;
  }
  case Kind_zJUMPDEST: {
    rop->variants.zJUMPDEST = op.variants.zJUMPDEST;
    break;
  }
  case Kind_zJUMPI: {
    rop->variants.zJUMPI = op.variants.zJUMPI;
    break;
  }
  case Kind_zKECCAK256: {
    rop->variants.zKECCAK256 = op.variants.zKECCAK256;
    break;
  }
  case Kind_zLOG: {
    rop->variants.zLOG = op.variants.zLOG;
    break;
  }
  case Kind_zLT: {
    rop->variants.zLT = op.variants.zLT;
    break;
  }
  case Kind_zMCOPY: {
    rop->variants.zMCOPY = op.variants.zMCOPY;
    break;
  }
  case Kind_zMLOAD: {
    rop->variants.zMLOAD = op.variants.zMLOAD;
    break;
  }
  case Kind_zMOD: {
    rop->variants.zMOD = op.variants.zMOD;
    break;
  }
  case Kind_zMSIZE: {
    rop->variants.zMSIZE = op.variants.zMSIZE;
    break;
  }
  case Kind_zMSTORE: {
    rop->variants.zMSTORE = op.variants.zMSTORE;
    break;
  }
  case Kind_zMSTORE8: {
    rop->variants.zMSTORE8 = op.variants.zMSTORE8;
    break;
  }
  case Kind_zMUL: {
    rop->variants.zMUL = op.variants.zMUL;
    break;
  }
  case Kind_zMULMOD: {
    rop->variants.zMULMOD = op.variants.zMULMOD;
    break;
  }
  case Kind_zNOT: {
    rop->variants.zNOT = op.variants.zNOT;
    break;
  }
  case Kind_zNUMBER: {
    rop->variants.zNUMBER = op.variants.zNUMBER;
    break;
  }
  case Kind_zOR: {
    rop->variants.zOR = op.variants.zOR;
    break;
  }
  case Kind_zORIGIN: {
    rop->variants.zORIGIN = op.variants.zORIGIN;
    break;
  }
  case Kind_zPC: {
    rop->variants.zPC = op.variants.zPC;
    break;
  }
  case Kind_zPOP: {
    rop->variants.zPOP = op.variants.zPOP;
    break;
  }
  case Kind_zPREVRANDAO: {
    rop->variants.zPREVRANDAO = op.variants.zPREVRANDAO;
    break;
  }
  case Kind_zPUSH: {
    rop->variants.zPUSH = op.variants.zPUSH;
    break;
  }
  case Kind_zRETURN: {
    rop->variants.zRETURN = op.variants.zRETURN;
    break;
  }
  case Kind_zRETURNDATACOPY: {
    rop->variants.zRETURNDATACOPY = op.variants.zRETURNDATACOPY;
    break;
  }
  case Kind_zRETURNDATASIZE: {
    rop->variants.zRETURNDATASIZE = op.variants.zRETURNDATASIZE;
    break;
  }
  case Kind_zREVERT: {
    rop->variants.zREVERT = op.variants.zREVERT;
    break;
  }
  case Kind_zSAR: {
    rop->variants.zSAR = op.variants.zSAR;
    break;
  }
  case Kind_zSDIV: {
    rop->variants.zSDIV = op.variants.zSDIV;
    break;
  }
  case Kind_zSELFBALANCE: {
    rop->variants.zSELFBALANCE = op.variants.zSELFBALANCE;
    break;
  }
  case Kind_zSELFDESTRUCT: {
    rop->variants.zSELFDESTRUCT = op.variants.zSELFDESTRUCT;
    break;
  }
  case Kind_zSGT: {
    rop->variants.zSGT = op.variants.zSGT;
    break;
  }
  case Kind_zSHL: {
    rop->variants.zSHL = op.variants.zSHL;
    break;
  }
  case Kind_zSHR: {
    rop->variants.zSHR = op.variants.zSHR;
    break;
  }
  case Kind_zSIGNEXTEND: {
    rop->variants.zSIGNEXTEND = op.variants.zSIGNEXTEND;
    break;
  }
  case Kind_zSLOAD: {
    rop->variants.zSLOAD = op.variants.zSLOAD;
    break;
  }
  case Kind_zSLOTNUM: {
    rop->variants.zSLOTNUM = op.variants.zSLOTNUM;
    break;
  }
  case Kind_zSLT: {
    rop->variants.zSLT = op.variants.zSLT;
    break;
  }
  case Kind_zSMOD: {
    rop->variants.zSMOD = op.variants.zSMOD;
    break;
  }
  case Kind_zSSTORE: {
    rop->variants.zSSTORE = op.variants.zSSTORE;
    break;
  }
  case Kind_zSTATICCALL: {
    rop->variants.zSTATICCALL = op.variants.zSTATICCALL;
    break;
  }
  case Kind_zSTOP: {
    rop->variants.zSTOP = op.variants.zSTOP;
    break;
  }
  case Kind_zSUB: {
    rop->variants.zSUB = op.variants.zSUB;
    break;
  }
  case Kind_zSWAP: {
    rop->variants.zSWAP = op.variants.zSWAP;
    break;
  }
  case Kind_zSWAPN: {
    rop->variants.zSWAPN = op.variants.zSWAPN;
    break;
  }
  case Kind_zTIMESTAMP: {
    rop->variants.zTIMESTAMP = op.variants.zTIMESTAMP;
    break;
  }
  case Kind_zTLOAD: {
    rop->variants.zTLOAD = op.variants.zTLOAD;
    break;
  }
  case Kind_zTSTORE: {
    rop->variants.zTSTORE = op.variants.zTSTORE;
    break;
  }
  case Kind_zXOR: {
    rop->variants.zXOR = op.variants.zXOR;
    break;
  }
  }
}

static bool EQUAL(zast)(struct zast op1, struct zast op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zADD: {
    return EQUAL(unit)(op1.variants.zADD, op2.variants.zADD);
    break;
  }
  case Kind_zADDMOD: {
    return EQUAL(unit)(op1.variants.zADDMOD, op2.variants.zADDMOD);
    break;
  }
  case Kind_zADDRESS: {
    return EQUAL(unit)(op1.variants.zADDRESS, op2.variants.zADDRESS);
    break;
  }
  case Kind_zAND: {
    return EQUAL(unit)(op1.variants.zAND, op2.variants.zAND);
    break;
  }
  case Kind_zBALANCE: {
    return EQUAL(unit)(op1.variants.zBALANCE, op2.variants.zBALANCE);
    break;
  }
  case Kind_zBASEFEE: {
    return EQUAL(unit)(op1.variants.zBASEFEE, op2.variants.zBASEFEE);
    break;
  }
  case Kind_zBLOBBASEFEE: {
    return EQUAL(unit)(op1.variants.zBLOBBASEFEE, op2.variants.zBLOBBASEFEE);
    break;
  }
  case Kind_zBLOBHASH: {
    return EQUAL(unit)(op1.variants.zBLOBHASH, op2.variants.zBLOBHASH);
    break;
  }
  case Kind_zBLOCKHASH: {
    return EQUAL(unit)(op1.variants.zBLOCKHASH, op2.variants.zBLOCKHASH);
    break;
  }
  case Kind_zBYTE: {
    return EQUAL(unit)(op1.variants.zBYTE, op2.variants.zBYTE);
    break;
  }
  case Kind_zCALL: {
    return EQUAL(unit)(op1.variants.zCALL, op2.variants.zCALL);
    break;
  }
  case Kind_zCALLCODE: {
    return EQUAL(unit)(op1.variants.zCALLCODE, op2.variants.zCALLCODE);
    break;
  }
  case Kind_zCALLDATACOPY: {
    return EQUAL(unit)(op1.variants.zCALLDATACOPY, op2.variants.zCALLDATACOPY);
    break;
  }
  case Kind_zCALLDATALOAD: {
    return EQUAL(unit)(op1.variants.zCALLDATALOAD, op2.variants.zCALLDATALOAD);
    break;
  }
  case Kind_zCALLDATASIZE: {
    return EQUAL(unit)(op1.variants.zCALLDATASIZE, op2.variants.zCALLDATASIZE);
    break;
  }
  case Kind_zCALLER: {
    return EQUAL(unit)(op1.variants.zCALLER, op2.variants.zCALLER);
    break;
  }
  case Kind_zCALLVALUE: {
    return EQUAL(unit)(op1.variants.zCALLVALUE, op2.variants.zCALLVALUE);
    break;
  }
  case Kind_zCHAINID: {
    return EQUAL(unit)(op1.variants.zCHAINID, op2.variants.zCHAINID);
    break;
  }
  case Kind_zCLZ: {
    return EQUAL(unit)(op1.variants.zCLZ, op2.variants.zCLZ);
    break;
  }
  case Kind_zCODECOPY: {
    return EQUAL(unit)(op1.variants.zCODECOPY, op2.variants.zCODECOPY);
    break;
  }
  case Kind_zCODESIZE: {
    return EQUAL(unit)(op1.variants.zCODESIZE, op2.variants.zCODESIZE);
    break;
  }
  case Kind_zCOINBASE: {
    return EQUAL(unit)(op1.variants.zCOINBASE, op2.variants.zCOINBASE);
    break;
  }
  case Kind_zCREATE: {
    return EQUAL(unit)(op1.variants.zCREATE, op2.variants.zCREATE);
    break;
  }
  case Kind_zCREATE2: {
    return EQUAL(unit)(op1.variants.zCREATE2, op2.variants.zCREATE2);
    break;
  }
  case Kind_zDELEGATECALL: {
    return EQUAL(unit)(op1.variants.zDELEGATECALL, op2.variants.zDELEGATECALL);
    break;
  }
  case Kind_zDIV: {
    return EQUAL(unit)(op1.variants.zDIV, op2.variants.zDIV);
    break;
  }
  case Kind_zDUP: {
    return (op1.variants.zDUP == op2.variants.zDUP);
    break;
  }
  case Kind_zDUPN: {
    return EQUAL(fbits)(op1.variants.zDUPN, op2.variants.zDUPN);
    break;
  }
  case Kind_zEQ: {
    return EQUAL(unit)(op1.variants.zEQ, op2.variants.zEQ);
    break;
  }
  case Kind_zEXCHANGE: {
    return EQUAL(fbits)(op1.variants.zEXCHANGE, op2.variants.zEXCHANGE);
    break;
  }
  case Kind_zEXP: {
    return EQUAL(unit)(op1.variants.zEXP, op2.variants.zEXP);
    break;
  }
  case Kind_zEXTCODECOPY: {
    return EQUAL(unit)(op1.variants.zEXTCODECOPY, op2.variants.zEXTCODECOPY);
    break;
  }
  case Kind_zEXTCODEHASH: {
    return EQUAL(unit)(op1.variants.zEXTCODEHASH, op2.variants.zEXTCODEHASH);
    break;
  }
  case Kind_zEXTCODESIZE: {
    return EQUAL(unit)(op1.variants.zEXTCODESIZE, op2.variants.zEXTCODESIZE);
    break;
  }
  case Kind_zGAS: {
    return EQUAL(unit)(op1.variants.zGAS, op2.variants.zGAS);
    break;
  }
  case Kind_zGASLIMIT: {
    return EQUAL(unit)(op1.variants.zGASLIMIT, op2.variants.zGASLIMIT);
    break;
  }
  case Kind_zGASPRICE: {
    return EQUAL(unit)(op1.variants.zGASPRICE, op2.variants.zGASPRICE);
    break;
  }
  case Kind_zGT: {
    return EQUAL(unit)(op1.variants.zGT, op2.variants.zGT);
    break;
  }
  case Kind_zINVALID: {
    return EQUAL(unit)(op1.variants.zINVALID, op2.variants.zINVALID);
    break;
  }
  case Kind_zISZERO: {
    return EQUAL(unit)(op1.variants.zISZERO, op2.variants.zISZERO);
    break;
  }
  case Kind_zJUMP: {
    return EQUAL(unit)(op1.variants.zJUMP, op2.variants.zJUMP);
    break;
  }
  case Kind_zJUMPDEST: {
    return EQUAL(unit)(op1.variants.zJUMPDEST, op2.variants.zJUMPDEST);
    break;
  }
  case Kind_zJUMPI: {
    return EQUAL(unit)(op1.variants.zJUMPI, op2.variants.zJUMPI);
    break;
  }
  case Kind_zKECCAK256: {
    return EQUAL(unit)(op1.variants.zKECCAK256, op2.variants.zKECCAK256);
    break;
  }
  case Kind_zLOG: {
    return (op1.variants.zLOG == op2.variants.zLOG);
    break;
  }
  case Kind_zLT: {
    return EQUAL(unit)(op1.variants.zLT, op2.variants.zLT);
    break;
  }
  case Kind_zMCOPY: {
    return EQUAL(unit)(op1.variants.zMCOPY, op2.variants.zMCOPY);
    break;
  }
  case Kind_zMLOAD: {
    return EQUAL(unit)(op1.variants.zMLOAD, op2.variants.zMLOAD);
    break;
  }
  case Kind_zMOD: {
    return EQUAL(unit)(op1.variants.zMOD, op2.variants.zMOD);
    break;
  }
  case Kind_zMSIZE: {
    return EQUAL(unit)(op1.variants.zMSIZE, op2.variants.zMSIZE);
    break;
  }
  case Kind_zMSTORE: {
    return EQUAL(unit)(op1.variants.zMSTORE, op2.variants.zMSTORE);
    break;
  }
  case Kind_zMSTORE8: {
    return EQUAL(unit)(op1.variants.zMSTORE8, op2.variants.zMSTORE8);
    break;
  }
  case Kind_zMUL: {
    return EQUAL(unit)(op1.variants.zMUL, op2.variants.zMUL);
    break;
  }
  case Kind_zMULMOD: {
    return EQUAL(unit)(op1.variants.zMULMOD, op2.variants.zMULMOD);
    break;
  }
  case Kind_zNOT: {
    return EQUAL(unit)(op1.variants.zNOT, op2.variants.zNOT);
    break;
  }
  case Kind_zNUMBER: {
    return EQUAL(unit)(op1.variants.zNUMBER, op2.variants.zNUMBER);
    break;
  }
  case Kind_zOR: {
    return EQUAL(unit)(op1.variants.zOR, op2.variants.zOR);
    break;
  }
  case Kind_zORIGIN: {
    return EQUAL(unit)(op1.variants.zORIGIN, op2.variants.zORIGIN);
    break;
  }
  case Kind_zPC: {
    return EQUAL(unit)(op1.variants.zPC, op2.variants.zPC);
    break;
  }
  case Kind_zPOP: {
    return EQUAL(unit)(op1.variants.zPOP, op2.variants.zPOP);
    break;
  }
  case Kind_zPREVRANDAO: {
    return EQUAL(unit)(op1.variants.zPREVRANDAO, op2.variants.zPREVRANDAO);
    break;
  }
  case Kind_zPUSH: {
    return EQUAL(ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9)(op1.variants.zPUSH, op2.variants.zPUSH);
    break;
  }
  case Kind_zRETURN: {
    return EQUAL(unit)(op1.variants.zRETURN, op2.variants.zRETURN);
    break;
  }
  case Kind_zRETURNDATACOPY: {
    return EQUAL(unit)(op1.variants.zRETURNDATACOPY, op2.variants.zRETURNDATACOPY);
    break;
  }
  case Kind_zRETURNDATASIZE: {
    return EQUAL(unit)(op1.variants.zRETURNDATASIZE, op2.variants.zRETURNDATASIZE);
    break;
  }
  case Kind_zREVERT: {
    return EQUAL(unit)(op1.variants.zREVERT, op2.variants.zREVERT);
    break;
  }
  case Kind_zSAR: {
    return EQUAL(unit)(op1.variants.zSAR, op2.variants.zSAR);
    break;
  }
  case Kind_zSDIV: {
    return EQUAL(unit)(op1.variants.zSDIV, op2.variants.zSDIV);
    break;
  }
  case Kind_zSELFBALANCE: {
    return EQUAL(unit)(op1.variants.zSELFBALANCE, op2.variants.zSELFBALANCE);
    break;
  }
  case Kind_zSELFDESTRUCT: {
    return EQUAL(unit)(op1.variants.zSELFDESTRUCT, op2.variants.zSELFDESTRUCT);
    break;
  }
  case Kind_zSGT: {
    return EQUAL(unit)(op1.variants.zSGT, op2.variants.zSGT);
    break;
  }
  case Kind_zSHL: {
    return EQUAL(unit)(op1.variants.zSHL, op2.variants.zSHL);
    break;
  }
  case Kind_zSHR: {
    return EQUAL(unit)(op1.variants.zSHR, op2.variants.zSHR);
    break;
  }
  case Kind_zSIGNEXTEND: {
    return EQUAL(unit)(op1.variants.zSIGNEXTEND, op2.variants.zSIGNEXTEND);
    break;
  }
  case Kind_zSLOAD: {
    return EQUAL(unit)(op1.variants.zSLOAD, op2.variants.zSLOAD);
    break;
  }
  case Kind_zSLOTNUM: {
    return EQUAL(unit)(op1.variants.zSLOTNUM, op2.variants.zSLOTNUM);
    break;
  }
  case Kind_zSLT: {
    return EQUAL(unit)(op1.variants.zSLT, op2.variants.zSLT);
    break;
  }
  case Kind_zSMOD: {
    return EQUAL(unit)(op1.variants.zSMOD, op2.variants.zSMOD);
    break;
  }
  case Kind_zSSTORE: {
    return EQUAL(unit)(op1.variants.zSSTORE, op2.variants.zSSTORE);
    break;
  }
  case Kind_zSTATICCALL: {
    return EQUAL(unit)(op1.variants.zSTATICCALL, op2.variants.zSTATICCALL);
    break;
  }
  case Kind_zSTOP: {
    return EQUAL(unit)(op1.variants.zSTOP, op2.variants.zSTOP);
    break;
  }
  case Kind_zSUB: {
    return EQUAL(unit)(op1.variants.zSUB, op2.variants.zSUB);
    break;
  }
  case Kind_zSWAP: {
    return (op1.variants.zSWAP == op2.variants.zSWAP);
    break;
  }
  case Kind_zSWAPN: {
    return EQUAL(fbits)(op1.variants.zSWAPN, op2.variants.zSWAPN);
    break;
  }
  case Kind_zTIMESTAMP: {
    return EQUAL(unit)(op1.variants.zTIMESTAMP, op2.variants.zTIMESTAMP);
    break;
  }
  case Kind_zTLOAD: {
    return EQUAL(unit)(op1.variants.zTLOAD, op2.variants.zTLOAD);
    break;
  }
  case Kind_zTSTORE: {
    return EQUAL(unit)(op1.variants.zTSTORE, op2.variants.zTSTORE);
    break;
  }
  case Kind_zXOR: {
    return EQUAL(unit)(op1.variants.zXOR, op2.variants.zXOR);
    break;
  }
  }
  return false;
}

static void zADD(struct zast *rop, unit op) {

  rop->kind = Kind_zADD;
  rop->variants.zADD = op;
}

static void zADDMOD(struct zast *rop, unit op) {

  rop->kind = Kind_zADDMOD;
  rop->variants.zADDMOD = op;
}

static void zADDRESS(struct zast *rop, unit op) {

  rop->kind = Kind_zADDRESS;
  rop->variants.zADDRESS = op;
}

static void zAND(struct zast *rop, unit op) {

  rop->kind = Kind_zAND;
  rop->variants.zAND = op;
}

static void zBALANCE(struct zast *rop, unit op) {

  rop->kind = Kind_zBALANCE;
  rop->variants.zBALANCE = op;
}

static void zBASEFEE(struct zast *rop, unit op) {

  rop->kind = Kind_zBASEFEE;
  rop->variants.zBASEFEE = op;
}

static void zBLOBBASEFEE(struct zast *rop, unit op) {

  rop->kind = Kind_zBLOBBASEFEE;
  rop->variants.zBLOBBASEFEE = op;
}

static void zBLOBHASH(struct zast *rop, unit op) {

  rop->kind = Kind_zBLOBHASH;
  rop->variants.zBLOBHASH = op;
}

static void zBLOCKHASH(struct zast *rop, unit op) {

  rop->kind = Kind_zBLOCKHASH;
  rop->variants.zBLOCKHASH = op;
}

static void zBYTE(struct zast *rop, unit op) {

  rop->kind = Kind_zBYTE;
  rop->variants.zBYTE = op;
}

static void zCALL(struct zast *rop, unit op) {

  rop->kind = Kind_zCALL;
  rop->variants.zCALL = op;
}

static void zCALLCODE(struct zast *rop, unit op) {

  rop->kind = Kind_zCALLCODE;
  rop->variants.zCALLCODE = op;
}

static void zCALLDATACOPY(struct zast *rop, unit op) {

  rop->kind = Kind_zCALLDATACOPY;
  rop->variants.zCALLDATACOPY = op;
}

static void zCALLDATALOAD(struct zast *rop, unit op) {

  rop->kind = Kind_zCALLDATALOAD;
  rop->variants.zCALLDATALOAD = op;
}

static void zCALLDATASIZE(struct zast *rop, unit op) {

  rop->kind = Kind_zCALLDATASIZE;
  rop->variants.zCALLDATASIZE = op;
}

static void zCALLER(struct zast *rop, unit op) {

  rop->kind = Kind_zCALLER;
  rop->variants.zCALLER = op;
}

static void zCALLVALUE(struct zast *rop, unit op) {

  rop->kind = Kind_zCALLVALUE;
  rop->variants.zCALLVALUE = op;
}

static void zCHAINID(struct zast *rop, unit op) {

  rop->kind = Kind_zCHAINID;
  rop->variants.zCHAINID = op;
}

static void zCLZ(struct zast *rop, unit op) {

  rop->kind = Kind_zCLZ;
  rop->variants.zCLZ = op;
}

static void zCODECOPY(struct zast *rop, unit op) {

  rop->kind = Kind_zCODECOPY;
  rop->variants.zCODECOPY = op;
}

static void zCODESIZE(struct zast *rop, unit op) {

  rop->kind = Kind_zCODESIZE;
  rop->variants.zCODESIZE = op;
}

static void zCOINBASE(struct zast *rop, unit op) {

  rop->kind = Kind_zCOINBASE;
  rop->variants.zCOINBASE = op;
}

static void zCREATE(struct zast *rop, unit op) {

  rop->kind = Kind_zCREATE;
  rop->variants.zCREATE = op;
}

static void zCREATE2(struct zast *rop, unit op) {

  rop->kind = Kind_zCREATE2;
  rop->variants.zCREATE2 = op;
}

static void zDELEGATECALL(struct zast *rop, unit op) {

  rop->kind = Kind_zDELEGATECALL;
  rop->variants.zDELEGATECALL = op;
}

static void zDIV(struct zast *rop, unit op) {

  rop->kind = Kind_zDIV;
  rop->variants.zDIV = op;
}

static void zDUP(struct zast *rop, uint64_t op) {

  rop->kind = Kind_zDUP;
  rop->variants.zDUP = op;
}

static void zDUPN(struct zast *rop, uint64_t op) {

  rop->kind = Kind_zDUPN;
  rop->variants.zDUPN = op;
}

static void zEQ(struct zast *rop, unit op) {

  rop->kind = Kind_zEQ;
  rop->variants.zEQ = op;
}

static void zEXCHANGE(struct zast *rop, uint64_t op) {

  rop->kind = Kind_zEXCHANGE;
  rop->variants.zEXCHANGE = op;
}

static void zEXP(struct zast *rop, unit op) {

  rop->kind = Kind_zEXP;
  rop->variants.zEXP = op;
}

static void zEXTCODECOPY(struct zast *rop, unit op) {

  rop->kind = Kind_zEXTCODECOPY;
  rop->variants.zEXTCODECOPY = op;
}

static void zEXTCODEHASH(struct zast *rop, unit op) {

  rop->kind = Kind_zEXTCODEHASH;
  rop->variants.zEXTCODEHASH = op;
}

static void zEXTCODESIZE(struct zast *rop, unit op) {

  rop->kind = Kind_zEXTCODESIZE;
  rop->variants.zEXTCODESIZE = op;
}

static void zGAS(struct zast *rop, unit op) {

  rop->kind = Kind_zGAS;
  rop->variants.zGAS = op;
}

static void zGASLIMIT(struct zast *rop, unit op) {

  rop->kind = Kind_zGASLIMIT;
  rop->variants.zGASLIMIT = op;
}

static void zGASPRICE(struct zast *rop, unit op) {

  rop->kind = Kind_zGASPRICE;
  rop->variants.zGASPRICE = op;
}

static void zGT(struct zast *rop, unit op) {

  rop->kind = Kind_zGT;
  rop->variants.zGT = op;
}

static void zINVALID(struct zast *rop, unit op) {

  rop->kind = Kind_zINVALID;
  rop->variants.zINVALID = op;
}

static void zISZERO(struct zast *rop, unit op) {

  rop->kind = Kind_zISZERO;
  rop->variants.zISZERO = op;
}

static void zJUMP(struct zast *rop, unit op) {

  rop->kind = Kind_zJUMP;
  rop->variants.zJUMP = op;
}

static void zJUMPDEST(struct zast *rop, unit op) {

  rop->kind = Kind_zJUMPDEST;
  rop->variants.zJUMPDEST = op;
}

static void zJUMPI(struct zast *rop, unit op) {

  rop->kind = Kind_zJUMPI;
  rop->variants.zJUMPI = op;
}

static void zKECCAK256(struct zast *rop, unit op) {

  rop->kind = Kind_zKECCAK256;
  rop->variants.zKECCAK256 = op;
}

static void zLOG(struct zast *rop, uint64_t op) {

  rop->kind = Kind_zLOG;
  rop->variants.zLOG = op;
}

static void zLT(struct zast *rop, unit op) {

  rop->kind = Kind_zLT;
  rop->variants.zLT = op;
}

static void zMCOPY(struct zast *rop, unit op) {

  rop->kind = Kind_zMCOPY;
  rop->variants.zMCOPY = op;
}

static void zMLOAD(struct zast *rop, unit op) {

  rop->kind = Kind_zMLOAD;
  rop->variants.zMLOAD = op;
}

static void zMOD(struct zast *rop, unit op) {

  rop->kind = Kind_zMOD;
  rop->variants.zMOD = op;
}

static void zMSIZE(struct zast *rop, unit op) {

  rop->kind = Kind_zMSIZE;
  rop->variants.zMSIZE = op;
}

static void zMSTORE(struct zast *rop, unit op) {

  rop->kind = Kind_zMSTORE;
  rop->variants.zMSTORE = op;
}

static void zMSTORE8(struct zast *rop, unit op) {

  rop->kind = Kind_zMSTORE8;
  rop->variants.zMSTORE8 = op;
}

static void zMUL(struct zast *rop, unit op) {

  rop->kind = Kind_zMUL;
  rop->variants.zMUL = op;
}

static void zMULMOD(struct zast *rop, unit op) {

  rop->kind = Kind_zMULMOD;
  rop->variants.zMULMOD = op;
}

static void zNOT(struct zast *rop, unit op) {

  rop->kind = Kind_zNOT;
  rop->variants.zNOT = op;
}

static void zNUMBER(struct zast *rop, unit op) {

  rop->kind = Kind_zNUMBER;
  rop->variants.zNUMBER = op;
}

static void zOR(struct zast *rop, unit op) {

  rop->kind = Kind_zOR;
  rop->variants.zOR = op;
}

static void zORIGIN(struct zast *rop, unit op) {

  rop->kind = Kind_zORIGIN;
  rop->variants.zORIGIN = op;
}

static void zPC(struct zast *rop, unit op) {

  rop->kind = Kind_zPC;
  rop->variants.zPC = op;
}

static void zPOP(struct zast *rop, unit op) {

  rop->kind = Kind_zPOP;
  rop->variants.zPOP = op;
}

static void zPREVRANDAO(struct zast *rop, unit op) {

  rop->kind = Kind_zPREVRANDAO;
  rop->variants.zPREVRANDAO = op;
}

static void zPUSH(struct zast *rop, struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 op) {

  rop->kind = Kind_zPUSH;
  rop->variants.zPUSH = op;
}

static void zRETURN(struct zast *rop, unit op) {

  rop->kind = Kind_zRETURN;
  rop->variants.zRETURN = op;
}

static void zRETURNDATACOPY(struct zast *rop, unit op) {

  rop->kind = Kind_zRETURNDATACOPY;
  rop->variants.zRETURNDATACOPY = op;
}

static void zRETURNDATASIZE(struct zast *rop, unit op) {

  rop->kind = Kind_zRETURNDATASIZE;
  rop->variants.zRETURNDATASIZE = op;
}

static void zREVERT(struct zast *rop, unit op) {

  rop->kind = Kind_zREVERT;
  rop->variants.zREVERT = op;
}

static void zSAR(struct zast *rop, unit op) {

  rop->kind = Kind_zSAR;
  rop->variants.zSAR = op;
}

static void zSDIV(struct zast *rop, unit op) {

  rop->kind = Kind_zSDIV;
  rop->variants.zSDIV = op;
}

static void zSELFBALANCE(struct zast *rop, unit op) {

  rop->kind = Kind_zSELFBALANCE;
  rop->variants.zSELFBALANCE = op;
}

static void zSELFDESTRUCT(struct zast *rop, unit op) {

  rop->kind = Kind_zSELFDESTRUCT;
  rop->variants.zSELFDESTRUCT = op;
}

static void zSGT(struct zast *rop, unit op) {

  rop->kind = Kind_zSGT;
  rop->variants.zSGT = op;
}

static void zSHL(struct zast *rop, unit op) {

  rop->kind = Kind_zSHL;
  rop->variants.zSHL = op;
}

static void zSHR(struct zast *rop, unit op) {

  rop->kind = Kind_zSHR;
  rop->variants.zSHR = op;
}

static void zSIGNEXTEND(struct zast *rop, unit op) {

  rop->kind = Kind_zSIGNEXTEND;
  rop->variants.zSIGNEXTEND = op;
}

static void zSLOAD(struct zast *rop, unit op) {

  rop->kind = Kind_zSLOAD;
  rop->variants.zSLOAD = op;
}

static void zSLOTNUM(struct zast *rop, unit op) {

  rop->kind = Kind_zSLOTNUM;
  rop->variants.zSLOTNUM = op;
}

static void zSLT(struct zast *rop, unit op) {

  rop->kind = Kind_zSLT;
  rop->variants.zSLT = op;
}

static void zSMOD(struct zast *rop, unit op) {

  rop->kind = Kind_zSMOD;
  rop->variants.zSMOD = op;
}

static void zSSTORE(struct zast *rop, unit op) {

  rop->kind = Kind_zSSTORE;
  rop->variants.zSSTORE = op;
}

static void zSTATICCALL(struct zast *rop, unit op) {

  rop->kind = Kind_zSTATICCALL;
  rop->variants.zSTATICCALL = op;
}

static void zSTOP(struct zast *rop, unit op) {

  rop->kind = Kind_zSTOP;
  rop->variants.zSTOP = op;
}

static void zSUB(struct zast *rop, unit op) {

  rop->kind = Kind_zSUB;
  rop->variants.zSUB = op;
}

static void zSWAP(struct zast *rop, uint64_t op) {

  rop->kind = Kind_zSWAP;
  rop->variants.zSWAP = op;
}

static void zSWAPN(struct zast *rop, uint64_t op) {

  rop->kind = Kind_zSWAPN;
  rop->variants.zSWAPN = op;
}

static void zTIMESTAMP(struct zast *rop, unit op) {

  rop->kind = Kind_zTIMESTAMP;
  rop->variants.zTIMESTAMP = op;
}

static void zTLOAD(struct zast *rop, unit op) {

  rop->kind = Kind_zTLOAD;
  rop->variants.zTLOAD = op;
}

static void zTSTORE(struct zast *rop, unit op) {

  rop->kind = Kind_zTSTORE;
  rop->variants.zTSTORE = op;
}

static void zXOR(struct zast *rop, unit op) {

  rop->kind = Kind_zXOR;
  rop->variants.zXOR = op;
}


static inline sail_fixed_bytes_20 fixed_bytes_20_zero(void) {
  sail_fixed_bytes_20 result = {{0}};
  return result;
}

static inline bool eq_fixed_bytes_20(const sail_fixed_bytes_20 lhs, const sail_fixed_bytes_20 rhs) {
  return memcmp(lhs.bytes, rhs.bytes, 20) == 0;
}


#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } sail_u256;
#endif

static inline sail_u256 u256_from_fixed_bytes_20(const sail_fixed_bytes_20 value) {
  sail_u256 result = {{0}};
  for (size_t i = 0; i < 20; ++i) {
    result.limbs[i >> 3] |= ((uint64_t)value.bytes[i]) << ((i & 7) * 8);
  }
  return result;
}

static inline sail_fixed_bytes_20 fixed_bytes_20_from_u256(const sail_u256 value) {
  sail_fixed_bytes_20 result;
  for (size_t i = 0; i < 20; ++i) {
    result.bytes[i] = (uint8_t)(value.limbs[i >> 3] >> ((i & 7) * 8));
  }
  return result;
}

static inline sail_fixed_bytes_20 internal_vector_init_fixed_bytes_20(const int64_t length_arg) {
  (void)length_arg;
  return fixed_bytes_20_zero();
}

static inline sail_fixed_bytes_20 internal_vector_update_fixed_bytes_20(
    sail_fixed_bytes_20 value, const int64_t index, const uint64_t elem) {
  if (index >= 0 && index < 20) value.bytes[index] = (uint8_t)elem;
  return value;
}

static inline uint64_t fast_vector_access_fixed_bytes_20(const sail_fixed_bytes_20 value, const int64_t index) {
  return index >= 0 && index < 20 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline sail_fixed_bytes_20 fast_unsigned_vector_update_fixed_bytes_20(
    sail_fixed_bytes_20 value, const uint64_t index, const uint64_t elem) {
  if (index < 20) value.bytes[index] = (uint8_t)elem;
  return value;
}

static inline uint64_t fast_unsigned_vector_access_fixed_bytes_20(
    const sail_fixed_bytes_20 value, const uint64_t index) {
  return index < 20 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline sail_fixed_bytes_20 fast_vector_init_fixed_bytes_20(const int64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  sail_fixed_bytes_20 result;
  for (size_t i = 0; i < 20; ++i) result.bytes[i] = (uint8_t)elem;
  return result;
}

static inline sail_fixed_bytes_20 fast_unsigned_vector_init_fixed_bytes_20(const uint64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  sail_fixed_bytes_20 result;
  for (size_t i = 0; i < 20; ++i) result.bytes[i] = (uint8_t)elem;
  return result;
}


static void COPY(zWitnessContext)(struct zWitnessContext *rop, const struct zWitnessContext op) {
  rop->zparent_base_fee_per_gas = op.zparent_base_fee_per_gas;
  rop->zparent_blob_gas_used = op.zparent_blob_gas_used;
  rop->zparent_excess_blob_gas = op.zparent_excess_blob_gas;
  rop->zparent_hash = op.zparent_hash;
  rop->zparent_state_root = op.zparent_state_root;
}

static bool EQUAL(zWitnessContext)(struct zWitnessContext op1, struct zWitnessContext op2) {
  return EQUAL(u256)(op1.zparent_base_fee_per_gas, op2.zparent_base_fee_per_gas) && (op1.zparent_blob_gas_used == op2.zparent_blob_gas_used) && (op1.zparent_excess_blob_gas == op2.zparent_excess_blob_gas) && EQUAL(fixed_bytes_32)(op1.zparent_hash, op2.zparent_hash) && EQUAL(fixed_bytes_32)(op1.zparent_state_root, op2.zparent_state_root);
}

static void COPY(zWithdrawal)(struct zWithdrawal *rop, const struct zWithdrawal op) {
  rop->zaddress = op.zaddress;
  rop->zamount = op.zamount;
  rop->zindex = op.zindex;
  rop->zvalidator_index = op.zvalidator_index;
}

static bool EQUAL(zWithdrawal)(struct zWithdrawal op1, struct zWithdrawal op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddress, op2.zaddress) && (op1.zamount == op2.zamount) && (op1.zindex == op2.zindex) && (op1.zvalidator_index == op2.zvalidator_index);
}

static void COPY(zTxValidity)(struct zTxValidity *rop, const struct zTxValidity op) {
  rop->zblob_fee = op.zblob_fee;
  rop->zcalldata_floor = op.zcalldata_floor;
  rop->zgas_limit = op.zgas_limit;
  rop->zgas_price = op.zgas_price;
  rop->zintrinsic_execution_gas = op.zintrinsic_execution_gas;
  rop->zintrinsic_state_gas = op.zintrinsic_state_gas;
  rop->znonce_before = op.znonce_before;
  rop->zpriority_fee = op.zpriority_fee;
  rop->zsender = op.zsender;
}

static bool EQUAL(zTxValidity)(struct zTxValidity op1, struct zTxValidity op2) {
  return EQUAL(u256)(op1.zblob_fee, op2.zblob_fee) && (op1.zcalldata_floor == op2.zcalldata_floor) && (op1.zgas_limit == op2.zgas_limit) && EQUAL(u256)(op1.zgas_price, op2.zgas_price) && (op1.zintrinsic_execution_gas == op2.zintrinsic_execution_gas) && (op1.zintrinsic_state_gas == op2.zintrinsic_state_gas) && (op1.znonce_before == op2.znonce_before) && EQUAL(u256)(op1.zpriority_fee, op2.zpriority_fee) && EQUAL(fixed_bytes_20)(op1.zsender, op2.zsender);
}

static void COPY(zTxUpfrontResult)(struct zTxUpfrontResult *rop, const struct zTxUpfrontResult op) {
  rop->zauthorizzation_refund = op.zauthorizzation_refund;
  rop->zcreate_target_prestate_empty = op.zcreate_target_prestate_empty;
}

static bool EQUAL(zTxUpfrontResult)(struct zTxUpfrontResult op1, struct zTxUpfrontResult op2) {
  return (op1.zauthorizzation_refund == op2.zauthorizzation_refund) && EQUAL(bool)(op1.zcreate_target_prestate_empty, op2.zcreate_target_prestate_empty);
}

static bool EQUAL(zTxType)(enum zTxType op1, enum zTxType op2) {
  return op1 == op2;
}

static enum zTxType UNDEFINED(zTxType)(unit u) { return zLegacyTx; }

static void COPY(zTxFrameResult)(struct zTxFrameResult *rop, const struct zTxFrameResult op) {
  rop->zexecution_gas_remaining = op.zexecution_gas_remaining;
  rop->zrefund = op.zrefund;
  rop->zstate_gas_remaining = op.zstate_gas_remaining;
  rop->zstate_gas_used = op.zstate_gas_used;
  rop->zsuccess = op.zsuccess;
}

static bool EQUAL(zTxFrameResult)(struct zTxFrameResult op1, struct zTxFrameResult op2) {
  return (op1.zexecution_gas_remaining == op2.zexecution_gas_remaining) && (op1.zrefund == op2.zrefund) && (op1.zstate_gas_remaining == op2.zstate_gas_remaining) && (op1.zstate_gas_used == op2.zstate_gas_used) && EQUAL(bool)(op1.zsuccess, op2.zsuccess);
}

static void CREATE(zTrieUpdateSource)(struct zTrieUpdateSource *op) {
  op->kind = Kind_zCachedAccountTrieUpdates;
}

static void RECREATE(zTrieUpdateSource)(struct zTrieUpdateSource *op) {

}

static void KILL(zTrieUpdateSource)(struct zTrieUpdateSource *op) {

}

static void COPY(zTrieUpdateSource)(struct zTrieUpdateSource *rop, struct zTrieUpdateSource op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zCachedAccountTrieUpdates: {
    rop->variants.zCachedAccountTrieUpdates = op.variants.zCachedAccountTrieUpdates;
    break;
  }
  case Kind_zChangedAccountTrieUpdates: {
    rop->variants.zChangedAccountTrieUpdates = op.variants.zChangedAccountTrieUpdates;
    break;
  }
  case Kind_zStorageTrieUpdates: {
    rop->variants.zStorageTrieUpdates = op.variants.zStorageTrieUpdates;
    break;
  }
  }
}

static bool EQUAL(zTrieUpdateSource)(struct zTrieUpdateSource op1, struct zTrieUpdateSource op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zCachedAccountTrieUpdates: {
    return EQUAL(unit)(op1.variants.zCachedAccountTrieUpdates, op2.variants.zCachedAccountTrieUpdates);
    break;
  }
  case Kind_zChangedAccountTrieUpdates: {
    return EQUAL(unit)(op1.variants.zChangedAccountTrieUpdates, op2.variants.zChangedAccountTrieUpdates);
    break;
  }
  case Kind_zStorageTrieUpdates: {
    return EQUAL(fixed_bytes_20)(op1.variants.zStorageTrieUpdates, op2.variants.zStorageTrieUpdates);
    break;
  }
  }
  return false;
}

static void zCachedAccountTrieUpdates(struct zTrieUpdateSource *rop, unit op) {

  rop->kind = Kind_zCachedAccountTrieUpdates;
  rop->variants.zCachedAccountTrieUpdates = op;
}

static void zChangedAccountTrieUpdates(struct zTrieUpdateSource *rop, unit op) {

  rop->kind = Kind_zChangedAccountTrieUpdates;
  rop->variants.zChangedAccountTrieUpdates = op;
}

static void zStorageTrieUpdates(struct zTrieUpdateSource *rop, sail_fixed_bytes_20 op) {

  rop->kind = Kind_zStorageTrieUpdates;
  rop->variants.zStorageTrieUpdates = op;
}

static void COPY(zTrieRootResult)(struct zTrieRootResult *rop, const struct zTrieRootResult op) {
  rop->zchanged = op.zchanged;
  rop->zroot = op.zroot;
}

static bool EQUAL(zTrieRootResult)(struct zTrieRootResult op1, struct zTrieRootResult op2) {
  return EQUAL(bool)(op1.zchanged, op2.zchanged) && EQUAL(fixed_bytes_32)(op1.zroot, op2.zroot);
}

static void COPY(zTriePath)(struct zTriePath *rop, const struct zTriePath op) {
  rop->zdata = op.zdata;
  rop->zlen = op.zlen;
}

static bool EQUAL(zTriePath)(struct zTriePath op1, struct zTriePath op2) {
  return EQUAL(fixed_bytes_32)(op1.zdata, op2.zdata) && (op1.zlen == op2.zlen);
}

static void CREATE(zoptionzIRTriePathzK)(struct zoptionzIRTriePathzK *op) {
  op->kind = Kind_zNonezIRTriePathzK;
}

static void RECREATE(zoptionzIRTriePathzK)(struct zoptionzIRTriePathzK *op) {

}

static void KILL(zoptionzIRTriePathzK)(struct zoptionzIRTriePathzK *op) {

}

static void COPY(zoptionzIRTriePathzK)(struct zoptionzIRTriePathzK *rop, struct zoptionzIRTriePathzK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRTriePathzK: {
    rop->variants.zNonezIRTriePathzK = op.variants.zNonezIRTriePathzK;
    break;
  }
  case Kind_zSomezIRTriePathzK: {
    rop->variants.zSomezIRTriePathzK = op.variants.zSomezIRTriePathzK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRTriePathzK)(struct zoptionzIRTriePathzK op1, struct zoptionzIRTriePathzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRTriePathzK: {
    return EQUAL(unit)(op1.variants.zNonezIRTriePathzK, op2.variants.zNonezIRTriePathzK);
    break;
  }
  case Kind_zSomezIRTriePathzK: {
    return EQUAL(zTriePath)(op1.variants.zSomezIRTriePathzK, op2.variants.zSomezIRTriePathzK);
    break;
  }
  }
  return false;
}

static void zNonezIRTriePathzK(struct zoptionzIRTriePathzK *rop, unit op) {

  rop->kind = Kind_zNonezIRTriePathzK;
  rop->variants.zNonezIRTriePathzK = op;
}

static void zSomezIRTriePathzK(struct zoptionzIRTriePathzK *rop, struct zTriePath op) {

  rop->kind = Kind_zSomezIRTriePathzK;
  rop->variants.zSomezIRTriePathzK = op;
}

static void COPY(zTransactionCosts)(struct zTransactionCosts *rop, const struct zTransactionCosts op) {
  rop->zblob_fee = op.zblob_fee;
  rop->zblob_gas = op.zblob_gas;
  rop->zcalldata_floor = op.zcalldata_floor;
  rop->zintrinsic_execution = op.zintrinsic_execution;
  rop->zintrinsic_state = op.zintrinsic_state;
  rop->zupfront = op.zupfront;
}

static bool EQUAL(zTransactionCosts)(struct zTransactionCosts op1, struct zTransactionCosts op2) {
  return EQUAL(u256)(op1.zblob_fee, op2.zblob_fee) && (op1.zblob_gas == op2.zblob_gas) && (op1.zcalldata_floor == op2.zcalldata_floor) && (op1.zintrinsic_execution == op2.zintrinsic_execution) && (op1.zintrinsic_state == op2.zintrinsic_state) && EQUAL(u256)(op1.zupfront, op2.zupfront);
}

static void COPY(zStorageValue)(struct zStorageValue *rop, const struct zStorageValue op) {
  rop->zcurr = op.zcurr;
  rop->zorig = op.zorig;
}

static bool EQUAL(zStorageValue)(struct zStorageValue op1, struct zStorageValue op2) {
  return EQUAL(u256)(op1.zcurr, op2.zcurr) && EQUAL(u256)(op1.zorig, op2.zorig);
}

static void CREATE(zoptionzIRStorageValuezK)(struct zoptionzIRStorageValuezK *op) {
  op->kind = Kind_zNonezIRStorageValuezK;
}

static void RECREATE(zoptionzIRStorageValuezK)(struct zoptionzIRStorageValuezK *op) {

}

static void KILL(zoptionzIRStorageValuezK)(struct zoptionzIRStorageValuezK *op) {

}

static void COPY(zoptionzIRStorageValuezK)(struct zoptionzIRStorageValuezK *rop, struct zoptionzIRStorageValuezK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRStorageValuezK: {
    rop->variants.zNonezIRStorageValuezK = op.variants.zNonezIRStorageValuezK;
    break;
  }
  case Kind_zSomezIRStorageValuezK: {
    rop->variants.zSomezIRStorageValuezK = op.variants.zSomezIRStorageValuezK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRStorageValuezK)(struct zoptionzIRStorageValuezK op1, struct zoptionzIRStorageValuezK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRStorageValuezK: {
    return EQUAL(unit)(op1.variants.zNonezIRStorageValuezK, op2.variants.zNonezIRStorageValuezK);
    break;
  }
  case Kind_zSomezIRStorageValuezK: {
    return EQUAL(zStorageValue)(op1.variants.zSomezIRStorageValuezK, op2.variants.zSomezIRStorageValuezK);
    break;
  }
  }
  return false;
}

static void zNonezIRStorageValuezK(struct zoptionzIRStorageValuezK *rop, unit op) {

  rop->kind = Kind_zNonezIRStorageValuezK;
  rop->variants.zNonezIRStorageValuezK = op;
}

static void zSomezIRStorageValuezK(struct zoptionzIRStorageValuezK *rop, struct zStorageValue op) {

  rop->kind = Kind_zSomezIRStorageValuezK;
  rop->variants.zSomezIRStorageValuezK = op;
}

static void COPY(zStorageKey)(struct zStorageKey *rop, const struct zStorageKey op) {
  rop->zaddr = op.zaddr;
  rop->zslot = op.zslot;
}

static bool EQUAL(zStorageKey)(struct zStorageKey op1, struct zStorageKey op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddr, op2.zaddr) && EQUAL(u256)(op1.zslot, op2.zslot);
}

static void COPY(zStorageEntry)(struct zStorageEntry *rop, const struct zStorageEntry op) {
  rop->zkey = op.zkey;
  rop->zvalue = op.zvalue;
}

static bool EQUAL(zStorageEntry)(struct zStorageEntry op1, struct zStorageEntry op2) {
  return EQUAL(zStorageKey)(op1.zkey, op2.zkey) && EQUAL(zStorageValue)(op1.zvalue, op2.zvalue);
}

static void CREATE(zoptionzIRStorageEntryzK)(struct zoptionzIRStorageEntryzK *op) {
  op->kind = Kind_zNonezIRStorageEntryzK;
}

static void RECREATE(zoptionzIRStorageEntryzK)(struct zoptionzIRStorageEntryzK *op) {

}

static void KILL(zoptionzIRStorageEntryzK)(struct zoptionzIRStorageEntryzK *op) {

}

static void COPY(zoptionzIRStorageEntryzK)(struct zoptionzIRStorageEntryzK *rop, struct zoptionzIRStorageEntryzK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRStorageEntryzK: {
    rop->variants.zNonezIRStorageEntryzK = op.variants.zNonezIRStorageEntryzK;
    break;
  }
  case Kind_zSomezIRStorageEntryzK: {
    rop->variants.zSomezIRStorageEntryzK = op.variants.zSomezIRStorageEntryzK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRStorageEntryzK)(struct zoptionzIRStorageEntryzK op1, struct zoptionzIRStorageEntryzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRStorageEntryzK: {
    return EQUAL(unit)(op1.variants.zNonezIRStorageEntryzK, op2.variants.zNonezIRStorageEntryzK);
    break;
  }
  case Kind_zSomezIRStorageEntryzK: {
    return EQUAL(zStorageEntry)(op1.variants.zSomezIRStorageEntryzK, op2.variants.zSomezIRStorageEntryzK);
    break;
  }
  }
  return false;
}

static void zNonezIRStorageEntryzK(struct zoptionzIRStorageEntryzK *rop, unit op) {

  rop->kind = Kind_zNonezIRStorageEntryzK;
  rop->variants.zNonezIRStorageEntryzK = op;
}

static void zSomezIRStorageEntryzK(struct zoptionzIRStorageEntryzK *rop, struct zStorageEntry op) {

  rop->kind = Kind_zSomezIRStorageEntryzK;
  rop->variants.zSomezIRStorageEntryzK = op;
}

static void COPY(zSstoreCosts)(struct zSstoreCosts *rop, const struct zSstoreCosts op) {
  rop->zexecution = op.zexecution;
  rop->zrefund = op.zrefund;
  rop->zstate_charge = op.zstate_charge;
  rop->zstate_credit = op.zstate_credit;
}

static bool EQUAL(zSstoreCosts)(struct zSstoreCosts op1, struct zSstoreCosts op2) {
  return (op1.zexecution == op2.zexecution) && (op1.zrefund == op2.zrefund) && (op1.zstate_charge == op2.zstate_charge) && (op1.zstate_credit == op2.zstate_credit);
}

static void COPY(zScaledBlobValue)(struct zScaledBlobValue *rop, const struct zScaledBlobValue op) {
  rop->zremainder = op.zremainder;
  rop->zwhole = op.zwhole;
}

static bool EQUAL(zScaledBlobValue)(struct zScaledBlobValue op1, struct zScaledBlobValue op2) {
  return (op1.zremainder == op2.zremainder) && EQUAL(u256)(op1.zwhole, op2.zwhole);
}

static void COPY(zRlpIndexItem)(struct zRlpIndexItem *rop, const struct zRlpIndexItem op) {
  rop->zindex = op.zindex;
  rop->zkey = op.zkey;
  COPY(zoptionzIRTriePathzK)(&rop->znext_key, op.znext_key);
}

static void CREATE(zRlpIndexItem)(struct zRlpIndexItem *op) {
  CREATE(zoptionzIRTriePathzK)(&op->znext_key);
}

static void RECREATE(zRlpIndexItem)(struct zRlpIndexItem *op) {
  RECREATE(zoptionzIRTriePathzK)(&op->znext_key);
}

static void KILL(zRlpIndexItem)(struct zRlpIndexItem *op) {
  KILL(zoptionzIRTriePathzK)(&op->znext_key);
}

static bool EQUAL(zRlpIndexItem)(struct zRlpIndexItem op1, struct zRlpIndexItem op2) {
  return (op1.zindex == op2.zindex) && EQUAL(zTriePath)(op1.zkey, op2.zkey) && EQUAL(zoptionzIRTriePathzK)(op1.znext_key, op2.znext_key);
}

static void COPY(zRlpIndexCursor)(struct zRlpIndexCursor *rop, const struct zRlpIndexCursor op) {
  rop->zcount = op.zcount;
  rop->zposition = op.zposition;
}

static bool EQUAL(zRlpIndexCursor)(struct zRlpIndexCursor op1, struct zRlpIndexCursor op2) {
  return (op1.zcount == op2.zcount) && (op1.zposition == op2.zposition);
}

static void COPY(zParentHeaderFields)(struct zParentHeaderFields *rop, const struct zParentHeaderFields op) {
  rop->zbase_fee = op.zbase_fee;
  rop->zblob_gas_used = op.zblob_gas_used;
  rop->zexcess_blob_gas = op.zexcess_blob_gas;
  rop->zhave_base_fee = op.zhave_base_fee;
  rop->zhave_blob_gas = op.zhave_blob_gas;
  rop->zhave_excess_blob_gas = op.zhave_excess_blob_gas;
  rop->zhave_parent = op.zhave_parent;
  rop->zhave_state = op.zhave_state;
  rop->zparent_hash = op.zparent_hash;
  rop->zstate_root = op.zstate_root;
}

static bool EQUAL(zParentHeaderFields)(struct zParentHeaderFields op1, struct zParentHeaderFields op2) {
  return EQUAL(u256)(op1.zbase_fee, op2.zbase_fee) && (op1.zblob_gas_used == op2.zblob_gas_used) && (op1.zexcess_blob_gas == op2.zexcess_blob_gas) && EQUAL(bool)(op1.zhave_base_fee, op2.zhave_base_fee) && EQUAL(bool)(op1.zhave_blob_gas, op2.zhave_blob_gas) && EQUAL(bool)(op1.zhave_excess_blob_gas, op2.zhave_excess_blob_gas) && EQUAL(bool)(op1.zhave_parent, op2.zhave_parent) && EQUAL(bool)(op1.zhave_state, op2.zhave_state) && EQUAL(fixed_bytes_32)(op1.zparent_hash, op2.zparent_hash) && EQUAL(fixed_bytes_32)(op1.zstate_root, op2.zstate_root);
}

static void COPY(zMessage)(struct zMessage *rop, const struct zMessage op) {
  rop->zaddress = op.zaddress;
  rop->zcaller = op.zcaller;
  rop->zcode_address = op.zcode_address;
  rop->zdepth = op.zdepth;
  rop->zis_static = op.zis_static;
  rop->zstate_gas_reservoir = op.zstate_gas_reservoir;
  rop->zvalue = op.zvalue;
}

static bool EQUAL(zMessage)(struct zMessage op1, struct zMessage op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddress, op2.zaddress) && EQUAL(fixed_bytes_20)(op1.zcaller, op2.zcaller) && EQUAL(fixed_bytes_20)(op1.zcode_address, op2.zcode_address) && (op1.zdepth == op2.zdepth) && EQUAL(bool)(op1.zis_static, op2.zis_static) && (op1.zstate_gas_reservoir == op2.zstate_gas_reservoir) && EQUAL(u256)(op1.zvalue, op2.zvalue);
}

static void CREATE(zMerkleSlot)(struct zMerkleSlot *op) {
  op->kind = Kind_zEmptyMerkleSlot;
}

static void RECREATE(zMerkleSlot)(struct zMerkleSlot *op) {

}

static void KILL(zMerkleSlot)(struct zMerkleSlot *op) {

}

static void COPY(zMerkleSlot)(struct zMerkleSlot *rop, struct zMerkleSlot op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zEmptyMerkleSlot: {
    rop->variants.zEmptyMerkleSlot = op.variants.zEmptyMerkleSlot;
    break;
  }
  case Kind_zOccupiedMerkleSlot: {
    rop->variants.zOccupiedMerkleSlot = op.variants.zOccupiedMerkleSlot;
    break;
  }
  }
}

static bool EQUAL(zMerkleSlot)(struct zMerkleSlot op1, struct zMerkleSlot op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zEmptyMerkleSlot: {
    return EQUAL(unit)(op1.variants.zEmptyMerkleSlot, op2.variants.zEmptyMerkleSlot);
    break;
  }
  case Kind_zOccupiedMerkleSlot: {
    return EQUAL(fixed_bytes_32)(op1.variants.zOccupiedMerkleSlot, op2.variants.zOccupiedMerkleSlot);
    break;
  }
  }
  return false;
}

static void zEmptyMerkleSlot(struct zMerkleSlot *rop, unit op) {

  rop->kind = Kind_zEmptyMerkleSlot;
  rop->variants.zEmptyMerkleSlot = op;
}

static void zOccupiedMerkleSlot(struct zMerkleSlot *rop, sail_fixed_bytes_32 op) {

  rop->kind = Kind_zOccupiedMerkleSlot;
  rop->variants.zOccupiedMerkleSlot = op;
}

static void CREATE(zz5listz8z5unionz0zzMerkleSlotz9)(zz5listz8z5unionz0zzMerkleSlotz9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5unionz0zzMerkleSlotz9(zz5listz8z5unionz0zzMerkleSlotz9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5unionz0zzMerkleSlotz9(zz5listz8z5unionz0zzMerkleSlotz9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5unionz0zzMerkleSlotz9)(zz5listz8z5unionz0zzMerkleSlotz9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5unionz0zzMerkleSlotz9 node = *rop;
  while (node != NULL && node->rc == 0) {
    KILL(zMerkleSlot)(&node->hd);
    zz5listz8z5unionz0zzMerkleSlotz9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5unionz0zzMerkleSlotz9(node);
  }
}

static void RECREATE(zz5listz8z5unionz0zzMerkleSlotz9)(zz5listz8z5unionz0zzMerkleSlotz9 *rop) {
  KILL(zz5listz8z5unionz0zzMerkleSlotz9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5unionz0zzMerkleSlotz9)(zz5listz8z5unionz0zzMerkleSlotz9 *rop, zz5listz8z5unionz0zzMerkleSlotz9 op) {
  internal_inc_zz5listz8z5unionz0zzMerkleSlotz9(op);
  KILL(zz5listz8z5unionz0zzMerkleSlotz9)(rop);
  *rop = op;
}

static void zconsz3z5unionz0zzMerkleSlot(zz5listz8z5unionz0zzMerkleSlotz9 *rop, struct zMerkleSlot x, zz5listz8z5unionz0zzMerkleSlotz9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5unionz0zzMerkleSlotz9);
  (*rop)->rc = 1;
  CREATE(zMerkleSlot)(&(*rop)->hd);
  COPY(zMerkleSlot)(&(*rop)->hd, x);
  if (!same) internal_inc_zz5listz8z5unionz0zzMerkleSlotz9(xs);
  (*rop)->tl = xs;
}

static void pick_zMerkleSlot(struct zMerkleSlot *x, const zz5listz8z5unionz0zzMerkleSlotz9 xs) {
  COPY(zMerkleSlot)(x, xs->hd);
}

static bool EQUAL(zz5listz8z5unionz0zzMerkleSlotz9)(const zz5listz8z5unionz0zzMerkleSlotz9 op1, const zz5listz8z5unionz0zzMerkleSlotz9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(zMerkleSlot)(op1->hd, op2->hd) && EQUAL(zz5listz8z5unionz0zzMerkleSlotz9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5unionz0zzMerkleSlotz9)(zz5listz8z5unionz0zzMerkleSlotz9 *rop, struct zMerkleSlot u) {
  *rop = NULL;
}

static void COPY(zMerkleAccumulator)(struct zMerkleAccumulator *rop, const struct zMerkleAccumulator op) {
  rop->zcount = op.zcount;
  rop->zdepth = op.zdepth;
  COPY(zz5listz8z5unionz0zzMerkleSlotz9)(&rop->zfrontier, op.zfrontier);
}

static void CREATE(zMerkleAccumulator)(struct zMerkleAccumulator *op) {
  CREATE(zz5listz8z5unionz0zzMerkleSlotz9)(&op->zfrontier);
}

static void RECREATE(zMerkleAccumulator)(struct zMerkleAccumulator *op) {
  RECREATE(zz5listz8z5unionz0zzMerkleSlotz9)(&op->zfrontier);
}

static void KILL(zMerkleAccumulator)(struct zMerkleAccumulator *op) {
  KILL(zz5listz8z5unionz0zzMerkleSlotz9)(&op->zfrontier);
}

static bool EQUAL(zMerkleAccumulator)(struct zMerkleAccumulator op1, struct zMerkleAccumulator op2) {
  return (op1.zcount == op2.zcount) && (op1.zdepth == op2.zdepth) && EQUAL(zz5listz8z5unionz0zzMerkleSlotz9)(op1.zfrontier, op2.zfrontier);
}

static void COPY(zMemoryRangeFields)(struct zMemoryRangeFields *rop, const struct zMemoryRangeFields op) {
  rop->zlen = op.zlen;
  rop->zoff = op.zoff;
}

static bool EQUAL(zMemoryRangeFields)(struct zMemoryRangeFields op1, struct zMemoryRangeFields op2) {
  return (op1.zlen == op2.zlen) && (op1.zoff == op2.zoff);
}

static void COPY(zMemoryPairExpansion)(struct zMemoryPairExpansion *rop, const struct zMemoryPairExpansion op) {
  rop->zcost = op.zcost;
  rop->zleft = op.zleft;
  rop->zrequired_sizze = op.zrequired_sizze;
  rop->zright = op.zright;
}

static bool EQUAL(zMemoryPairExpansion)(struct zMemoryPairExpansion op1, struct zMemoryPairExpansion op2) {
  return (op1.zcost == op2.zcost) && EQUAL(zMemoryRangeFields)(op1.zleft, op2.zleft) && (op1.zrequired_sizze == op2.zrequired_sizze) && EQUAL(zMemoryRangeFields)(op1.zright, op2.zright);
}

static void COPY(zMemoryExpansion)(struct zMemoryExpansion *rop, const struct zMemoryExpansion op) {
  rop->zcost = op.zcost;
  rop->zrange = op.zrange;
  rop->zrequired_sizze = op.zrequired_sizze;
}

static bool EQUAL(zMemoryExpansion)(struct zMemoryExpansion op1, struct zMemoryExpansion op2) {
  return (op1.zcost == op2.zcost) && EQUAL(zMemoryRangeFields)(op1.zrange, op2.zrange) && (op1.zrequired_sizze == op2.zrequired_sizze);
}

static void CREATE(zz5listz8z5bv8z9)(zz5listz8z5bv8z9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5bv8z9(zz5listz8z5bv8z9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5bv8z9(zz5listz8z5bv8z9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5bv8z9)(zz5listz8z5bv8z9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5bv8z9 node = *rop;
  while (node != NULL && node->rc == 0) {
    zz5listz8z5bv8z9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5bv8z9(node);
  }
}

static void RECREATE(zz5listz8z5bv8z9)(zz5listz8z5bv8z9 *rop) {
  KILL(zz5listz8z5bv8z9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5bv8z9)(zz5listz8z5bv8z9 *rop, zz5listz8z5bv8z9 op) {
  internal_inc_zz5listz8z5bv8z9(op);
  KILL(zz5listz8z5bv8z9)(rop);
  *rop = op;
}

static void zconsz3z5bv8(zz5listz8z5bv8z9 *rop, uint64_t x, zz5listz8z5bv8z9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5bv8z9);
  (*rop)->rc = 1;
  (*rop)->hd = x;
  if (!same) internal_inc_zz5listz8z5bv8z9(xs);
  (*rop)->tl = xs;
}

static uint64_t pick_fbits(const zz5listz8z5bv8z9 xs) {
  return xs->hd;
}

static bool EQUAL(zz5listz8z5bv8z9)(const zz5listz8z5bv8z9 op1, const zz5listz8z5bv8z9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(fbits)(op1->hd, op2->hd) && EQUAL(zz5listz8z5bv8z9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5bv8z9)(zz5listz8z5bv8z9 *rop, uint64_t u) {
  *rop = NULL;
}

static void COPY(zMaterializzedBytes)(struct zMaterializzedBytes *rop, const struct zMaterializzedBytes op) {
  COPY(zz5listz8z5bv8z9)(&rop->zdata, op.zdata);
  rop->zlen = op.zlen;
}

static void CREATE(zMaterializzedBytes)(struct zMaterializzedBytes *op) {
  CREATE(zz5listz8z5bv8z9)(&op->zdata);
}

static void RECREATE(zMaterializzedBytes)(struct zMaterializzedBytes *op) {
  RECREATE(zz5listz8z5bv8z9)(&op->zdata);
}

static void KILL(zMaterializzedBytes)(struct zMaterializzedBytes *op) {
  KILL(zz5listz8z5bv8z9)(&op->zdata);
}

static bool EQUAL(zMaterializzedBytes)(struct zMaterializzedBytes op1, struct zMaterializzedBytes op2) {
  return EQUAL(zz5listz8z5bv8z9)(op1.zdata, op2.zdata) && (op1.zlen == op2.zlen);
}


static inline sail_fixed_bytes_256 fixed_bytes_256_zero(void) {
  sail_fixed_bytes_256 result = {{0}};
  return result;
}

static inline bool eq_fixed_bytes_256(const sail_fixed_bytes_256 lhs, const sail_fixed_bytes_256 rhs) {
  return memcmp(lhs.bytes, rhs.bytes, 256) == 0;
}

static inline sail_fixed_bytes_256 internal_vector_init_fixed_bytes_256(const int64_t length_arg) {
  (void)length_arg;
  return fixed_bytes_256_zero();
}

static inline sail_fixed_bytes_256 internal_vector_update_fixed_bytes_256(
    sail_fixed_bytes_256 value, const int64_t index, const uint64_t elem) {
  if (index >= 0 && index < 256) value.bytes[index] = (uint8_t)elem;
  return value;
}

static inline uint64_t fast_vector_access_fixed_bytes_256(const sail_fixed_bytes_256 value, const int64_t index) {
  return index >= 0 && index < 256 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline sail_fixed_bytes_256 fast_unsigned_vector_update_fixed_bytes_256(
    sail_fixed_bytes_256 value, const uint64_t index, const uint64_t elem) {
  if (index < 256) value.bytes[index] = (uint8_t)elem;
  return value;
}

static inline uint64_t fast_unsigned_vector_access_fixed_bytes_256(
    const sail_fixed_bytes_256 value, const uint64_t index) {
  return index < 256 ? (uint64_t)value.bytes[index] : UINT64_C(0);
}

static inline sail_fixed_bytes_256 fast_vector_init_fixed_bytes_256(const int64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  sail_fixed_bytes_256 result;
  for (size_t i = 0; i < 256; ++i) result.bytes[i] = (uint8_t)elem;
  return result;
}

static inline sail_fixed_bytes_256 fast_unsigned_vector_init_fixed_bytes_256(const uint64_t length_arg, const uint64_t elem) {
  (void)length_arg;
  sail_fixed_bytes_256 result;
  for (size_t i = 0; i < 256; ++i) result.bytes[i] = (uint8_t)elem;
  return result;
}


static void COPY(zIntrinsicGasCost)(struct zIntrinsicGasCost *rop, const struct zIntrinsicGasCost op) {
  rop->zcalldata_floor = op.zcalldata_floor;
  rop->zexecution = op.zexecution;
  rop->zstate = op.zstate;
}

static bool EQUAL(zIntrinsicGasCost)(struct zIntrinsicGasCost op1, struct zIntrinsicGasCost op2) {
  return (op1.zcalldata_floor == op2.zcalldata_floor) && (op1.zexecution == op2.zexecution) && (op1.zstate == op2.zstate);
}

static void COPY(zInlineNode)(struct zInlineNode *rop, const struct zInlineNode op) {
  rop->zdata = op.zdata;
  rop->zlen = op.zlen;
}

static bool EQUAL(zInlineNode)(struct zInlineNode op1, struct zInlineNode op2) {
  return EQUAL(fixed_bytes_32)(op1.zdata, op2.zdata) && (op1.zlen == op2.zlen);
}

static void CREATE(zNodeRef)(struct zNodeRef *op) {
  op->kind = Kind_zEmptyRef;
}

static void RECREATE(zNodeRef)(struct zNodeRef *op) {

}

static void KILL(zNodeRef)(struct zNodeRef *op) {

}

static void COPY(zNodeRef)(struct zNodeRef *rop, struct zNodeRef op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zEmptyRef: {
    rop->variants.zEmptyRef = op.variants.zEmptyRef;
    break;
  }
  case Kind_zHashRef: {
    rop->variants.zHashRef = op.variants.zHashRef;
    break;
  }
  case Kind_zInlineRef: {
    rop->variants.zInlineRef = op.variants.zInlineRef;
    break;
  }
  }
}

static bool EQUAL(zNodeRef)(struct zNodeRef op1, struct zNodeRef op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zEmptyRef: {
    return EQUAL(unit)(op1.variants.zEmptyRef, op2.variants.zEmptyRef);
    break;
  }
  case Kind_zHashRef: {
    return EQUAL(fixed_bytes_32)(op1.variants.zHashRef, op2.variants.zHashRef);
    break;
  }
  case Kind_zInlineRef: {
    return EQUAL(zInlineNode)(op1.variants.zInlineRef, op2.variants.zInlineRef);
    break;
  }
  }
  return false;
}

static void zEmptyRef(struct zNodeRef *rop, unit op) {

  rop->kind = Kind_zEmptyRef;
  rop->variants.zEmptyRef = op;
}

static void zHashRef(struct zNodeRef *rop, sail_fixed_bytes_32 op) {

  rop->kind = Kind_zHashRef;
  rop->variants.zHashRef = op;
}

static void zInlineRef(struct zNodeRef *rop, struct zInlineNode op) {

  rop->kind = Kind_zInlineRef;
  rop->variants.zInlineRef = op;
}

static void CREATE(zz5vecz8z5unionz0zzNodeRefz9)(zz5vecz8z5unionz0zzNodeRefz9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5unionz0zzNodeRefz9)(zz5vecz8z5unionz0zzNodeRefz9 *rop) {
  for (int i = 0; i < (rop->len); i++) {
    KILL(zNodeRef)((rop->data) + i);
  }
  if (rop->data != NULL) sail_free(rop->data);
}

static void fast_vector_init_zz5vecz8z5unionz0zzNodeRefz9(zz5vecz8z5unionz0zzNodeRefz9 *vec, const int64_t n, struct zNodeRef elem) {
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(struct zNodeRef, m);
  for (size_t i = 0; i < m; i++) {
    CREATE(zNodeRef)((vec->data) + i);
    COPY(zNodeRef)((vec->data) + i, elem);
  }
}

static void fast_unsigned_vector_init_zz5vecz8z5unionz0zzNodeRefz9(zz5vecz8z5unionz0zzNodeRefz9 *vec, const uint64_t n, struct zNodeRef elem) {
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(struct zNodeRef, m);
  for (size_t i = 0; i < m; i++) {
    CREATE(zNodeRef)((vec->data) + i);
    COPY(zNodeRef)((vec->data) + i, elem);
  }
}

static void RECREATE(zz5vecz8z5unionz0zzNodeRefz9)(zz5vecz8z5unionz0zzNodeRefz9 *rop) {
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(rop);
  CREATE(zz5vecz8z5unionz0zzNodeRefz9)(rop);
}

static void fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(struct zNodeRef *rop, zz5vecz8z5unionz0zzNodeRefz9 op, int64_t n) {
  COPY(zNodeRef)(rop, op.data[n]);
}

static void fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(struct zNodeRef *rop, zz5vecz8z5unionz0zzNodeRefz9 op, uint64_t n) {
  COPY(zNodeRef)(rop, op.data[n]);
}

static void COPY(zz5vecz8z5unionz0zzNodeRefz9)(zz5vecz8z5unionz0zzNodeRefz9 *rop, zz5vecz8z5unionz0zzNodeRefz9 op) {
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(rop);
  rop->len = op.len;
  rop->data = sail_new_array(struct zNodeRef, rop->len);
  for (int i = 0; i < op.len; i++) {
    CREATE(zNodeRef)((rop->data) + i);
    COPY(zNodeRef)((rop->data) + i, op.data[i]);
  }
}

static void fast_vector_update_zz5vecz8z5unionz0zzNodeRefz9(zz5vecz8z5unionz0zzNodeRefz9 *rop, zz5vecz8z5unionz0zzNodeRefz9 op, const int64_t n, struct zNodeRef elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    COPY(zNodeRef)((rop->data) + m, elem);
  } else {
    COPY(zz5vecz8z5unionz0zzNodeRefz9)(rop, op);
    COPY(zNodeRef)((rop->data) + m, elem);
  }
}

static void fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(zz5vecz8z5unionz0zzNodeRefz9 *rop, zz5vecz8z5unionz0zzNodeRefz9 op, const uint64_t n, struct zNodeRef elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    COPY(zNodeRef)((rop->data) + m, elem);
  } else {
    COPY(zz5vecz8z5unionz0zzNodeRefz9)(rop, op);
    COPY(zNodeRef)((rop->data) + m, elem);
  }
}

static bool EQUAL(zz5vecz8z5unionz0zzNodeRefz9)(const zz5vecz8z5unionz0zzNodeRefz9 op1, const zz5vecz8z5unionz0zzNodeRefz9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;
  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(zNodeRef)(op1.data[i], op2.data[i]);
  }
  return result;
}

static void internal_vector_update_zz5vecz8z5unionz0zzNodeRefz9(zz5vecz8z5unionz0zzNodeRefz9 *rop, zz5vecz8z5unionz0zzNodeRefz9 op, const int64_t n, struct zNodeRef elem) {
  COPY(zNodeRef)((rop->data) + n, elem);
}

static void internal_vector_init_zz5vecz8z5unionz0zzNodeRefz9(zz5vecz8z5unionz0zzNodeRefz9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_new_array(struct zNodeRef, len);
  for (int i = 0; i < len; i++) {
    CREATE(zNodeRef)((rop->data) + i);
  }
}

static void COPY(zTrieBranchFrame)(struct zTrieBranchFrame *rop, const struct zTrieBranchFrame op) {
  COPY(zz5vecz8z5unionz0zzNodeRefz9)(&rop->zchildren, op.zchildren);
  rop->zdepth = op.zdepth;
  rop->zmask = op.zmask;
}

static void CREATE(zTrieBranchFrame)(struct zTrieBranchFrame *op) {
  CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&op->zchildren);
}

static void RECREATE(zTrieBranchFrame)(struct zTrieBranchFrame *op) {
  RECREATE(zz5vecz8z5unionz0zzNodeRefz9)(&op->zchildren);
}

static void KILL(zTrieBranchFrame)(struct zTrieBranchFrame *op) {
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&op->zchildren);
}

static bool EQUAL(zTrieBranchFrame)(struct zTrieBranchFrame op1, struct zTrieBranchFrame op2) {
  return EQUAL(zz5vecz8z5unionz0zzNodeRefz9)(op1.zchildren, op2.zchildren) && (op1.zdepth == op2.zdepth) && EQUAL(fbits)(op1.zmask, op2.zmask);
}

static void CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(zz5listz8z5structz0zzTrieBranchFramez9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zzTrieBranchFramez9(zz5listz8z5structz0zzTrieBranchFramez9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zzTrieBranchFramez9(zz5listz8z5structz0zzTrieBranchFramez9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zzTrieBranchFramez9)(zz5listz8z5structz0zzTrieBranchFramez9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zzTrieBranchFramez9 node = *rop;
  while (node != NULL && node->rc == 0) {
    KILL(zTrieBranchFrame)(&node->hd);
    zz5listz8z5structz0zzTrieBranchFramez9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zzTrieBranchFramez9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zzTrieBranchFramez9)(zz5listz8z5structz0zzTrieBranchFramez9 *rop) {
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zzTrieBranchFramez9)(zz5listz8z5structz0zzTrieBranchFramez9 *rop, zz5listz8z5structz0zzTrieBranchFramez9 op) {
  internal_inc_zz5listz8z5structz0zzTrieBranchFramez9(op);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zzTrieBranchFrame(zz5listz8z5structz0zzTrieBranchFramez9 *rop, struct zTrieBranchFrame x, zz5listz8z5structz0zzTrieBranchFramez9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zzTrieBranchFramez9);
  (*rop)->rc = 1;
  CREATE(zTrieBranchFrame)(&(*rop)->hd);
  COPY(zTrieBranchFrame)(&(*rop)->hd, x);
  if (!same) internal_inc_zz5listz8z5structz0zzTrieBranchFramez9(xs);
  (*rop)->tl = xs;
}

static void pick_zTrieBranchFrame(struct zTrieBranchFrame *x, const zz5listz8z5structz0zzTrieBranchFramez9 xs) {
  COPY(zTrieBranchFrame)(x, xs->hd);
}

static bool EQUAL(zz5listz8z5structz0zzTrieBranchFramez9)(const zz5listz8z5structz0zzTrieBranchFramez9 op1, const zz5listz8z5structz0zzTrieBranchFramez9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(zTrieBranchFrame)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zzTrieBranchFramez9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zzTrieBranchFramez9)(zz5listz8z5structz0zzTrieBranchFramez9 *rop, struct zTrieBranchFrame u) {
  *rop = NULL;
}

static void COPY(zTrieBuilder)(struct zTrieBuilder *rop, const struct zTrieBuilder op) {
  rop->zcomplete = op.zcomplete;
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&rop->zframes, op.zframes);
  COPY(zNodeRef)(&rop->zroot, op.zroot);
}

static void CREATE(zTrieBuilder)(struct zTrieBuilder *op) {
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&op->zframes);
  CREATE(zNodeRef)(&op->zroot);
}

static void RECREATE(zTrieBuilder)(struct zTrieBuilder *op) {
  RECREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&op->zframes);
  RECREATE(zNodeRef)(&op->zroot);
}

static void KILL(zTrieBuilder)(struct zTrieBuilder *op) {
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&op->zframes);
  KILL(zNodeRef)(&op->zroot);
}

static bool EQUAL(zTrieBuilder)(struct zTrieBuilder op1, struct zTrieBuilder op2) {
  return EQUAL(bool)(op1.zcomplete, op2.zcomplete) && EQUAL(zz5listz8z5structz0zzTrieBranchFramez9)(op1.zframes, op2.zframes) && EQUAL(zNodeRef)(op1.zroot, op2.zroot);
}

static bool EQUAL(zFork)(enum zFork op1, enum zFork op2) {
  return op1 == op2;
}

static enum zFork UNDEFINED(zFork)(unit u) { return zFrontier; }

static void COPY(zFixedBytes32)(struct zFixedBytes32 *rop, const struct zFixedBytes32 op) {
  rop->zdata = op.zdata;
  rop->zlen = op.zlen;
}

static bool EQUAL(zFixedBytes32)(struct zFixedBytes32 op1, struct zFixedBytes32 op2) {
  return EQUAL(fixed_bytes_32)(op1.zdata, op2.zdata) && (op1.zlen == op2.zlen);
}

static bool EQUAL(zExceptionKind)(enum zExceptionKind op1, enum zExceptionKind op2) {
  return op1 == op2;
}

static enum zExceptionKind UNDEFINED(zExceptionKind)(unit u) { return zStackUnderflow; }

static bool EQUAL(zEnvField)(enum zEnvField op1, enum zEnvField op2) {
  return op1 == op2;
}

static enum zEnvField UNDEFINED(zEnvField)(unit u) { return zF_Number; }

static void COPY(zCodeAnalysis)(struct zCodeAnalysis *rop, const struct zCodeAnalysis op) {
  rop->zchunk = op.zchunk;
  rop->zchunk_index = op.zchunk_index;
  rop->zchunk_offset = op.zchunk_offset;
}

static bool EQUAL(zCodeAnalysis)(struct zCodeAnalysis op1, struct zCodeAnalysis op2) {
  return EQUAL(u256)(op1.zchunk, op2.zchunk) && (op1.zchunk_index == op2.zchunk_index) && (op1.zchunk_offset == op2.zchunk_offset);
}

static void COPY(zChainConfig)(struct zChainConfig *rop, const struct zChainConfig op) {
  rop->zchain_id = op.zchain_id;
}

static bool EQUAL(zChainConfig)(struct zChainConfig op1, struct zChainConfig op2) {
  return (op1.zchain_id == op2.zchain_id);
}

static bool EQUAL(zCallKind)(enum zCallKind op1, enum zCallKind op2) {
  return op1 == op2;
}

static enum zCallKind UNDEFINED(zCallKind)(unit u) { return zCall; }

static bool EQUAL(zByteSource)(enum zByteSource op1, enum zByteSource op2) {
  return op1 == op2;
}

static enum zByteSource UNDEFINED(zByteSource)(unit u) { return zStatelessInputSource; }

static void COPY(zByteSliceFields)(struct zByteSliceFields *rop, const struct zByteSliceFields op) {
  rop->zlen = op.zlen;
  rop->zoff = op.zoff;
  rop->zsource = op.zsource;
}

static bool EQUAL(zByteSliceFields)(struct zByteSliceFields op1, struct zByteSliceFields op2) {
  return (op1.zlen == op2.zlen) && (op1.zoff == op2.zoff) && EQUAL(zByteSource)(op1.zsource, op2.zsource);
}

static void CREATE(zTrieItemValue)(struct zTrieItemValue *op) {
  op->kind = Kind_zBranchItem;
  CREATE(zNodeRef)(&op->variants.zBranchItem);
}

static void RECREATE(zTrieItemValue)(struct zTrieItemValue *op) {

}

static void KILL(zTrieItemValue)(struct zTrieItemValue *op) {
  switch (op->kind) {
  case Kind_zBranchItem: {
    KILL(zNodeRef)(&op->variants.zBranchItem);
    break;
  }
  case Kind_zSubtreeItem: {
    KILL(zNodeRef)(&op->variants.zSubtreeItem);
    break;
  }
  default: break;
  }
}

static void COPY(zTrieItemValue)(struct zTrieItemValue *rop, struct zTrieItemValue op) {
  switch (rop->kind) {
  case Kind_zBranchItem: {
    KILL(zNodeRef)(&rop->variants.zBranchItem);
    break;
  }
  case Kind_zSubtreeItem: {
    KILL(zNodeRef)(&rop->variants.zSubtreeItem);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zBranchItem: {
    CREATE(zNodeRef)(&rop->variants.zBranchItem); COPY(zNodeRef)(&rop->variants.zBranchItem, op.variants.zBranchItem);
    break;
  }
  case Kind_zLeafItem: {
    rop->variants.zLeafItem = op.variants.zLeafItem;
    break;
  }
  case Kind_zSubtreeItem: {
    CREATE(zNodeRef)(&rop->variants.zSubtreeItem); COPY(zNodeRef)(&rop->variants.zSubtreeItem, op.variants.zSubtreeItem);
    break;
  }
  }
}

static bool EQUAL(zTrieItemValue)(struct zTrieItemValue op1, struct zTrieItemValue op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zBranchItem: {
    return EQUAL(zNodeRef)(op1.variants.zBranchItem, op2.variants.zBranchItem);
    break;
  }
  case Kind_zLeafItem: {
    return EQUAL(zByteSliceFields)(op1.variants.zLeafItem, op2.variants.zLeafItem);
    break;
  }
  case Kind_zSubtreeItem: {
    return EQUAL(zNodeRef)(op1.variants.zSubtreeItem, op2.variants.zSubtreeItem);
    break;
  }
  }
  return false;
}

static void zBranchItem(struct zTrieItemValue *rop, struct zNodeRef op) {
  switch (rop->kind) {
  case Kind_zBranchItem: {
    KILL(zNodeRef)(&rop->variants.zBranchItem);
    break;
  }
  case Kind_zSubtreeItem: {
    KILL(zNodeRef)(&rop->variants.zSubtreeItem);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zBranchItem;
  CREATE(zNodeRef)(&rop->variants.zBranchItem);
  COPY(zNodeRef)(&rop->variants.zBranchItem, op);
}

static void zLeafItem(struct zTrieItemValue *rop, struct zByteSliceFields op) {
  switch (rop->kind) {
  case Kind_zBranchItem: {
    KILL(zNodeRef)(&rop->variants.zBranchItem);
    break;
  }
  case Kind_zSubtreeItem: {
    KILL(zNodeRef)(&rop->variants.zSubtreeItem);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zLeafItem;
  rop->variants.zLeafItem = op;
}

static void zSubtreeItem(struct zTrieItemValue *rop, struct zNodeRef op) {
  switch (rop->kind) {
  case Kind_zBranchItem: {
    KILL(zNodeRef)(&rop->variants.zBranchItem);
    break;
  }
  case Kind_zSubtreeItem: {
    KILL(zNodeRef)(&rop->variants.zSubtreeItem);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zSubtreeItem;
  CREATE(zNodeRef)(&rop->variants.zSubtreeItem);
  COPY(zNodeRef)(&rop->variants.zSubtreeItem, op);
}

static void COPY(zTrieItem)(struct zTrieItem *rop, const struct zTrieItem op) {
  rop->zpath = op.zpath;
  COPY(zTrieItemValue)(&rop->zvalue, op.zvalue);
}

static void CREATE(zTrieItem)(struct zTrieItem *op) {
  CREATE(zTrieItemValue)(&op->zvalue);
}

static void RECREATE(zTrieItem)(struct zTrieItem *op) {
  RECREATE(zTrieItemValue)(&op->zvalue);
}

static void KILL(zTrieItem)(struct zTrieItem *op) {
  KILL(zTrieItemValue)(&op->zvalue);
}

static bool EQUAL(zTrieItem)(struct zTrieItem op1, struct zTrieItem op2) {
  return EQUAL(zTriePath)(op1.zpath, op2.zpath) && EQUAL(zTrieItemValue)(op1.zvalue, op2.zvalue);
}

static void CREATE(zoptionzIRTrieItemzK)(struct zoptionzIRTrieItemzK *op) {
  op->kind = Kind_zNonezIRTrieItemzK;
}

static void RECREATE(zoptionzIRTrieItemzK)(struct zoptionzIRTrieItemzK *op) {

}

static void KILL(zoptionzIRTrieItemzK)(struct zoptionzIRTrieItemzK *op) {
  switch (op->kind) {
  case Kind_zSomezIRTrieItemzK: {
    KILL(zTrieItem)(&op->variants.zSomezIRTrieItemzK);
    break;
  }
  default: break;
  }
}

static void COPY(zoptionzIRTrieItemzK)(struct zoptionzIRTrieItemzK *rop, struct zoptionzIRTrieItemzK op) {
  switch (rop->kind) {
  case Kind_zSomezIRTrieItemzK: {
    KILL(zTrieItem)(&rop->variants.zSomezIRTrieItemzK);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRTrieItemzK: {
    rop->variants.zNonezIRTrieItemzK = op.variants.zNonezIRTrieItemzK;
    break;
  }
  case Kind_zSomezIRTrieItemzK: {
    CREATE(zTrieItem)(&rop->variants.zSomezIRTrieItemzK); COPY(zTrieItem)(&rop->variants.zSomezIRTrieItemzK, op.variants.zSomezIRTrieItemzK);
    break;
  }
  }
}

static bool EQUAL(zoptionzIRTrieItemzK)(struct zoptionzIRTrieItemzK op1, struct zoptionzIRTrieItemzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRTrieItemzK: {
    return EQUAL(unit)(op1.variants.zNonezIRTrieItemzK, op2.variants.zNonezIRTrieItemzK);
    break;
  }
  case Kind_zSomezIRTrieItemzK: {
    return EQUAL(zTrieItem)(op1.variants.zSomezIRTrieItemzK, op2.variants.zSomezIRTrieItemzK);
    break;
  }
  }
  return false;
}

static void zNonezIRTrieItemzK(struct zoptionzIRTrieItemzK *rop, unit op) {
  switch (rop->kind) {
  case Kind_zSomezIRTrieItemzK: {
    KILL(zTrieItem)(&rop->variants.zSomezIRTrieItemzK);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zNonezIRTrieItemzK;
  rop->variants.zNonezIRTrieItemzK = op;
}

static void zSomezIRTrieItemzK(struct zoptionzIRTrieItemzK *rop, struct zTrieItem op) {
  switch (rop->kind) {
  case Kind_zSomezIRTrieItemzK: {
    KILL(zTrieItem)(&rop->variants.zSomezIRTrieItemzK);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zSomezIRTrieItemzK;
  CREATE(zTrieItem)(&rop->variants.zSomezIRTrieItemzK);
  COPY(zTrieItem)(&rop->variants.zSomezIRTrieItemzK, op);
}

static void COPY(zTrieItemSink)(struct zTrieItemSink *rop, const struct zTrieItemSink op) {
  COPY(zTrieBuilder)(&rop->zbuilder, op.zbuilder);
  COPY(zoptionzIRTrieItemzK)(&rop->zpending, op.zpending);
}

static void CREATE(zTrieItemSink)(struct zTrieItemSink *op) {
  CREATE(zTrieBuilder)(&op->zbuilder);
  CREATE(zoptionzIRTrieItemzK)(&op->zpending);
}

static void RECREATE(zTrieItemSink)(struct zTrieItemSink *op) {
  RECREATE(zTrieBuilder)(&op->zbuilder);
  RECREATE(zoptionzIRTrieItemzK)(&op->zpending);
}

static void KILL(zTrieItemSink)(struct zTrieItemSink *op) {
  KILL(zTrieBuilder)(&op->zbuilder);
  KILL(zoptionzIRTrieItemzK)(&op->zpending);
}

static bool EQUAL(zTrieItemSink)(struct zTrieItemSink op1, struct zTrieItemSink op2) {
  return EQUAL(zTrieBuilder)(op1.zbuilder, op2.zbuilder) && EQUAL(zoptionzIRTrieItemzK)(op1.zpending, op2.zpending);
}

static void CREATE(zTrieChange)(struct zTrieChange *op) {
  op->kind = Kind_zTrieDelete;
}

static void RECREATE(zTrieChange)(struct zTrieChange *op) {

}

static void KILL(zTrieChange)(struct zTrieChange *op) {

}

static void COPY(zTrieChange)(struct zTrieChange *rop, struct zTrieChange op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zTrieDelete: {
    rop->variants.zTrieDelete = op.variants.zTrieDelete;
    break;
  }
  case Kind_zTriePut: {
    rop->variants.zTriePut = op.variants.zTriePut;
    break;
  }
  }
}

static bool EQUAL(zTrieChange)(struct zTrieChange op1, struct zTrieChange op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zTrieDelete: {
    return EQUAL(unit)(op1.variants.zTrieDelete, op2.variants.zTrieDelete);
    break;
  }
  case Kind_zTriePut: {
    return EQUAL(zByteSliceFields)(op1.variants.zTriePut, op2.variants.zTriePut);
    break;
  }
  }
  return false;
}

static void zTrieDelete(struct zTrieChange *rop, unit op) {

  rop->kind = Kind_zTrieDelete;
  rop->variants.zTrieDelete = op;
}

static void zTriePut(struct zTrieChange *rop, struct zByteSliceFields op) {

  rop->kind = Kind_zTriePut;
  rop->variants.zTriePut = op;
}

static void COPY(zTrieUpdate)(struct zTrieUpdate *rop, const struct zTrieUpdate op) {
  COPY(zTrieChange)(&rop->zchange, op.zchange);
  rop->zkey = op.zkey;
}

static void CREATE(zTrieUpdate)(struct zTrieUpdate *op) {
  CREATE(zTrieChange)(&op->zchange);
}

static void RECREATE(zTrieUpdate)(struct zTrieUpdate *op) {
  RECREATE(zTrieChange)(&op->zchange);
}

static void KILL(zTrieUpdate)(struct zTrieUpdate *op) {
  KILL(zTrieChange)(&op->zchange);
}

static bool EQUAL(zTrieUpdate)(struct zTrieUpdate op1, struct zTrieUpdate op2) {
  return EQUAL(zTrieChange)(op1.zchange, op2.zchange) && EQUAL(zTriePath)(op1.zkey, op2.zkey);
}

static void CREATE(zoptionzIRTrieUpdatezK)(struct zoptionzIRTrieUpdatezK *op) {
  op->kind = Kind_zNonezIRTrieUpdatezK;
}

static void RECREATE(zoptionzIRTrieUpdatezK)(struct zoptionzIRTrieUpdatezK *op) {

}

static void KILL(zoptionzIRTrieUpdatezK)(struct zoptionzIRTrieUpdatezK *op) {
  switch (op->kind) {
  case Kind_zSomezIRTrieUpdatezK: {
    KILL(zTrieUpdate)(&op->variants.zSomezIRTrieUpdatezK);
    break;
  }
  default: break;
  }
}

static void COPY(zoptionzIRTrieUpdatezK)(struct zoptionzIRTrieUpdatezK *rop, struct zoptionzIRTrieUpdatezK op) {
  switch (rop->kind) {
  case Kind_zSomezIRTrieUpdatezK: {
    KILL(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRTrieUpdatezK: {
    rop->variants.zNonezIRTrieUpdatezK = op.variants.zNonezIRTrieUpdatezK;
    break;
  }
  case Kind_zSomezIRTrieUpdatezK: {
    CREATE(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK); COPY(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK, op.variants.zSomezIRTrieUpdatezK);
    break;
  }
  }
}

static bool EQUAL(zoptionzIRTrieUpdatezK)(struct zoptionzIRTrieUpdatezK op1, struct zoptionzIRTrieUpdatezK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRTrieUpdatezK: {
    return EQUAL(unit)(op1.variants.zNonezIRTrieUpdatezK, op2.variants.zNonezIRTrieUpdatezK);
    break;
  }
  case Kind_zSomezIRTrieUpdatezK: {
    return EQUAL(zTrieUpdate)(op1.variants.zSomezIRTrieUpdatezK, op2.variants.zSomezIRTrieUpdatezK);
    break;
  }
  }
  return false;
}

static void zNonezIRTrieUpdatezK(struct zoptionzIRTrieUpdatezK *rop, unit op) {
  switch (rop->kind) {
  case Kind_zSomezIRTrieUpdatezK: {
    KILL(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zNonezIRTrieUpdatezK;
  rop->variants.zNonezIRTrieUpdatezK = op;
}

static void zSomezIRTrieUpdatezK(struct zoptionzIRTrieUpdatezK *rop, struct zTrieUpdate op) {
  switch (rop->kind) {
  case Kind_zSomezIRTrieUpdatezK: {
    KILL(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zSomezIRTrieUpdatezK;
  CREATE(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK);
  COPY(zTrieUpdate)(&rop->variants.zSomezIRTrieUpdatezK, op);
}

static void COPY(zTrieUpdateCursor)(struct zTrieUpdateCursor *rop, const struct zTrieUpdateCursor op) {
  COPY(zoptionzIRTrieUpdatezK)(&rop->zpending, op.zpending);
  COPY(zTrieUpdateSource)(&rop->zsource, op.zsource);
}

static void CREATE(zTrieUpdateCursor)(struct zTrieUpdateCursor *op) {
  CREATE(zoptionzIRTrieUpdatezK)(&op->zpending);
  CREATE(zTrieUpdateSource)(&op->zsource);
}

static void RECREATE(zTrieUpdateCursor)(struct zTrieUpdateCursor *op) {
  RECREATE(zoptionzIRTrieUpdatezK)(&op->zpending);
  RECREATE(zTrieUpdateSource)(&op->zsource);
}

static void KILL(zTrieUpdateCursor)(struct zTrieUpdateCursor *op) {
  KILL(zoptionzIRTrieUpdatezK)(&op->zpending);
  KILL(zTrieUpdateSource)(&op->zsource);
}

static bool EQUAL(zTrieUpdateCursor)(struct zTrieUpdateCursor op1, struct zTrieUpdateCursor op2) {
  return EQUAL(zoptionzIRTrieUpdatezK)(op1.zpending, op2.zpending) && EQUAL(zTrieUpdateSource)(op1.zsource, op2.zsource);
}

static void COPY(zSszzContainerCursor)(struct zSszzContainerCursor *rop, const struct zSszzContainerCursor op) {
  rop->zbytes = op.zbytes;
  rop->zcurrent = op.zcurrent;
}

static bool EQUAL(zSszzContainerCursor)(struct zSszzContainerCursor op1, struct zSszzContainerCursor op2) {
  return EQUAL(zByteSliceFields)(op1.zbytes, op2.zbytes) && (op1.zcurrent == op2.zcurrent);
}

static void COPY(zRlpFieldRefFields)(struct zRlpFieldRefFields *rop, const struct zRlpFieldRefFields op) {
  rop->zcontent_len = op.zcontent_len;
  rop->zcontent_off = op.zcontent_off;
  rop->zfull_len = op.zfull_len;
  rop->zfull_off = op.zfull_off;
  rop->zis_list = op.zis_list;
  rop->zsource = op.zsource;
}

static bool EQUAL(zRlpFieldRefFields)(struct zRlpFieldRefFields op1, struct zRlpFieldRefFields op2) {
  return (op1.zcontent_len == op2.zcontent_len) && (op1.zcontent_off == op2.zcontent_off) && (op1.zfull_len == op2.zfull_len) && (op1.zfull_off == op2.zfull_off) && EQUAL(bool)(op1.zis_list, op2.zis_list) && EQUAL(zByteSliceFields)(op1.zsource, op2.zsource);
}

static void COPY(zLeafNodeData)(struct zLeafNodeData *rop, const struct zLeafNodeData op) {
  rop->zpath = op.zpath;
  rop->zvalue = op.zvalue;
}

static bool EQUAL(zLeafNodeData)(struct zLeafNodeData op1, struct zLeafNodeData op2) {
  return EQUAL(zTriePath)(op1.zpath, op2.zpath) && EQUAL(zRlpFieldRefFields)(op1.zvalue, op2.zvalue);
}

static void COPY(zExtensionNodeData)(struct zExtensionNodeData *rop, const struct zExtensionNodeData op) {
  rop->zchild = op.zchild;
  rop->zpath = op.zpath;
}

static bool EQUAL(zExtensionNodeData)(struct zExtensionNodeData op1, struct zExtensionNodeData op2) {
  return EQUAL(zRlpFieldRefFields)(op1.zchild, op2.zchild) && EQUAL(zTriePath)(op1.zpath, op2.zpath);
}

static void COPY(zRlpCursorFields)(struct zRlpCursorFields *rop, const struct zRlpCursorFields op) {
  rop->zcurrent = op.zcurrent;
  rop->zsource = op.zsource;
  rop->zstop = op.zstop;
  rop->zvalid = op.zvalid;
}

static bool EQUAL(zRlpCursorFields)(struct zRlpCursorFields op1, struct zRlpCursorFields op2) {
  return (op1.zcurrent == op2.zcurrent) && EQUAL(zByteSliceFields)(op1.zsource, op2.zsource) && (op1.zstop == op2.zstop) && EQUAL(bool)(op1.zvalid, op2.zvalid);
}

static void COPY(zPrecompileResult)(struct zPrecompileResult *rop, const struct zPrecompileResult op) {
  rop->zoutput = op.zoutput;
  rop->zsuccess = op.zsuccess;
}

static bool EQUAL(zPrecompileResult)(struct zPrecompileResult op1, struct zPrecompileResult op2) {
  return EQUAL(zByteSliceFields)(op1.zoutput, op2.zoutput) && EQUAL(bool)(op1.zsuccess, op2.zsuccess);
}

static void CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zz__sail_c_repr_u256z9(zz5listz8z5structz0zz__sail_c_repr_u256z9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zz__sail_c_repr_u256z9(zz5listz8z5structz0zz__sail_c_repr_u256z9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zz__sail_c_repr_u256z9 node = *rop;
  while (node != NULL && node->rc == 0) {
    zz5listz8z5structz0zz__sail_c_repr_u256z9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zz__sail_c_repr_u256z9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop, zz5listz8z5structz0zz__sail_c_repr_u256z9 op) {
  internal_inc_zz5listz8z5structz0zz__sail_c_repr_u256z9(op);
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zz__sail_c_repr_u256(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop, sail_u256 x, zz5listz8z5structz0zz__sail_c_repr_u256z9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9);
  (*rop)->rc = 1;
  (*rop)->hd = x;
  if (!same) internal_inc_zz5listz8z5structz0zz__sail_c_repr_u256z9(xs);
  (*rop)->tl = xs;
}

static sail_u256 pick_u256(const zz5listz8z5structz0zz__sail_c_repr_u256z9 xs) {
  return xs->hd;
}

static bool EQUAL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(const zz5listz8z5structz0zz__sail_c_repr_u256z9 op1, const zz5listz8z5structz0zz__sail_c_repr_u256z9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(u256)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zz__sail_c_repr_u256z9)(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop, sail_u256 u) {
  *rop = NULL;
}

static void COPY(zLogEntry)(struct zLogEntry *rop, const struct zLogEntry op) {
  rop->zaddress = op.zaddress;
  rop->zdata = op.zdata;
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&rop->ztopics, op.ztopics);
}

static void CREATE(zLogEntry)(struct zLogEntry *op) {
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&op->ztopics);
}

static void RECREATE(zLogEntry)(struct zLogEntry *op) {
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&op->ztopics);
}

static void KILL(zLogEntry)(struct zLogEntry *op) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&op->ztopics);
}

static bool EQUAL(zLogEntry)(struct zLogEntry op1, struct zLogEntry op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddress, op2.zaddress) && EQUAL(zByteSliceFields)(op1.zdata, op2.zdata) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(op1.ztopics, op2.ztopics);
}

static void CREATE(zz5listz8z5structz0zzLogEntryz9)(zz5listz8z5structz0zzLogEntryz9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zzLogEntryz9(zz5listz8z5structz0zzLogEntryz9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zzLogEntryz9(zz5listz8z5structz0zzLogEntryz9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zzLogEntryz9)(zz5listz8z5structz0zzLogEntryz9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zzLogEntryz9 node = *rop;
  while (node != NULL && node->rc == 0) {
    KILL(zLogEntry)(&node->hd);
    zz5listz8z5structz0zzLogEntryz9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zzLogEntryz9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zzLogEntryz9)(zz5listz8z5structz0zzLogEntryz9 *rop) {
  KILL(zz5listz8z5structz0zzLogEntryz9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zzLogEntryz9)(zz5listz8z5structz0zzLogEntryz9 *rop, zz5listz8z5structz0zzLogEntryz9 op) {
  internal_inc_zz5listz8z5structz0zzLogEntryz9(op);
  KILL(zz5listz8z5structz0zzLogEntryz9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zzLogEntry(zz5listz8z5structz0zzLogEntryz9 *rop, struct zLogEntry x, zz5listz8z5structz0zzLogEntryz9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zzLogEntryz9);
  (*rop)->rc = 1;
  CREATE(zLogEntry)(&(*rop)->hd);
  COPY(zLogEntry)(&(*rop)->hd, x);
  if (!same) internal_inc_zz5listz8z5structz0zzLogEntryz9(xs);
  (*rop)->tl = xs;
}

static void pick_zLogEntry(struct zLogEntry *x, const zz5listz8z5structz0zzLogEntryz9 xs) {
  COPY(zLogEntry)(x, xs->hd);
}

static bool EQUAL(zz5listz8z5structz0zzLogEntryz9)(const zz5listz8z5structz0zzLogEntryz9 op1, const zz5listz8z5structz0zzLogEntryz9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(zLogEntry)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zzLogEntryz9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zzLogEntryz9)(zz5listz8z5structz0zzLogEntryz9 *rop, struct zLogEntry u) {
  *rop = NULL;
}

static void COPY(zReceipt)(struct zReceipt *rop, const struct zReceipt op) {
  rop->zexecution_gas = op.zexecution_gas;
  rop->zgas_used = op.zgas_used;
  COPY(zz5listz8z5structz0zzLogEntryz9)(&rop->zlogs, op.zlogs);
  rop->zstate_gas = op.zstate_gas;
  rop->zsuccess = op.zsuccess;
  rop->ztx_type = op.ztx_type;
}

static void CREATE(zReceipt)(struct zReceipt *op) {
  CREATE(zz5listz8z5structz0zzLogEntryz9)(&op->zlogs);
}

static void RECREATE(zReceipt)(struct zReceipt *op) {
  RECREATE(zz5listz8z5structz0zzLogEntryz9)(&op->zlogs);
}

static void KILL(zReceipt)(struct zReceipt *op) {
  KILL(zz5listz8z5structz0zzLogEntryz9)(&op->zlogs);
}

static bool EQUAL(zReceipt)(struct zReceipt op1, struct zReceipt op2) {
  return (op1.zexecution_gas == op2.zexecution_gas) && (op1.zgas_used == op2.zgas_used) && EQUAL(zz5listz8z5structz0zzLogEntryz9)(op1.zlogs, op2.zlogs) && (op1.zstate_gas == op2.zstate_gas) && EQUAL(bool)(op1.zsuccess, op2.zsuccess) && EQUAL(zTxType)(op1.ztx_type, op2.ztx_type);
}

static void COPY(zPendingReceipt)(struct zPendingReceipt *rop, const struct zPendingReceipt op) {
  rop->zcumulative_gas_used = op.zcumulative_gas_used;
  rop->zindex = op.zindex;
  COPY(zReceipt)(&rop->zreceipt, op.zreceipt);
}

static void CREATE(zPendingReceipt)(struct zPendingReceipt *op) {
  CREATE(zReceipt)(&op->zreceipt);
}

static void RECREATE(zPendingReceipt)(struct zPendingReceipt *op) {
  RECREATE(zReceipt)(&op->zreceipt);
}

static void KILL(zPendingReceipt)(struct zPendingReceipt *op) {
  KILL(zReceipt)(&op->zreceipt);
}

static bool EQUAL(zPendingReceipt)(struct zPendingReceipt op1, struct zPendingReceipt op2) {
  return (op1.zcumulative_gas_used == op2.zcumulative_gas_used) && (op1.zindex == op2.zindex) && EQUAL(zReceipt)(op1.zreceipt, op2.zreceipt);
}

static void CREATE(zoptionzIRPendingReceiptzK)(struct zoptionzIRPendingReceiptzK *op) {
  op->kind = Kind_zNonezIRPendingReceiptzK;
}

static void RECREATE(zoptionzIRPendingReceiptzK)(struct zoptionzIRPendingReceiptzK *op) {

}

static void KILL(zoptionzIRPendingReceiptzK)(struct zoptionzIRPendingReceiptzK *op) {
  switch (op->kind) {
  case Kind_zSomezIRPendingReceiptzK: {
    KILL(zPendingReceipt)(&op->variants.zSomezIRPendingReceiptzK);
    break;
  }
  default: break;
  }
}

static void COPY(zoptionzIRPendingReceiptzK)(struct zoptionzIRPendingReceiptzK *rop, struct zoptionzIRPendingReceiptzK op) {
  switch (rop->kind) {
  case Kind_zSomezIRPendingReceiptzK: {
    KILL(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRPendingReceiptzK: {
    rop->variants.zNonezIRPendingReceiptzK = op.variants.zNonezIRPendingReceiptzK;
    break;
  }
  case Kind_zSomezIRPendingReceiptzK: {
    CREATE(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK); COPY(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK, op.variants.zSomezIRPendingReceiptzK);
    break;
  }
  }
}

static bool EQUAL(zoptionzIRPendingReceiptzK)(struct zoptionzIRPendingReceiptzK op1, struct zoptionzIRPendingReceiptzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRPendingReceiptzK: {
    return EQUAL(unit)(op1.variants.zNonezIRPendingReceiptzK, op2.variants.zNonezIRPendingReceiptzK);
    break;
  }
  case Kind_zSomezIRPendingReceiptzK: {
    return EQUAL(zPendingReceipt)(op1.variants.zSomezIRPendingReceiptzK, op2.variants.zSomezIRPendingReceiptzK);
    break;
  }
  }
  return false;
}

static void zNonezIRPendingReceiptzK(struct zoptionzIRPendingReceiptzK *rop, unit op) {
  switch (rop->kind) {
  case Kind_zSomezIRPendingReceiptzK: {
    KILL(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zNonezIRPendingReceiptzK;
  rop->variants.zNonezIRPendingReceiptzK = op;
}

static void zSomezIRPendingReceiptzK(struct zoptionzIRPendingReceiptzK *rop, struct zPendingReceipt op) {
  switch (rop->kind) {
  case Kind_zSomezIRPendingReceiptzK: {
    KILL(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zSomezIRPendingReceiptzK;
  CREATE(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK);
  COPY(zPendingReceipt)(&rop->variants.zSomezIRPendingReceiptzK, op);
}

static void COPY(zReceiptAccumulator)(struct zReceiptAccumulator *rop, const struct zReceiptAccumulator op) {
  rop->zbloom = op.zbloom;
  COPY(zTrieBuilder)(&rop->zbuilder, op.zbuilder);
  rop->zcount = op.zcount;
  rop->zcumulative_gas_used = op.zcumulative_gas_used;
  COPY(zoptionzIRPendingReceiptzK)(&rop->zfirst, op.zfirst);
  COPY(zoptionzIRPendingReceiptzK)(&rop->zpending, op.zpending);
}

static void CREATE(zReceiptAccumulator)(struct zReceiptAccumulator *op) {
  CREATE(zTrieBuilder)(&op->zbuilder);
  CREATE(zoptionzIRPendingReceiptzK)(&op->zfirst);
  CREATE(zoptionzIRPendingReceiptzK)(&op->zpending);
}

static void RECREATE(zReceiptAccumulator)(struct zReceiptAccumulator *op) {
  RECREATE(zTrieBuilder)(&op->zbuilder);
  RECREATE(zoptionzIRPendingReceiptzK)(&op->zfirst);
  RECREATE(zoptionzIRPendingReceiptzK)(&op->zpending);
}

static void KILL(zReceiptAccumulator)(struct zReceiptAccumulator *op) {
  KILL(zTrieBuilder)(&op->zbuilder);
  KILL(zoptionzIRPendingReceiptzK)(&op->zfirst);
  KILL(zoptionzIRPendingReceiptzK)(&op->zpending);
}

static bool EQUAL(zReceiptAccumulator)(struct zReceiptAccumulator op1, struct zReceiptAccumulator op2) {
  return EQUAL(fixed_bytes_256)(op1.zbloom, op2.zbloom) && EQUAL(zTrieBuilder)(op1.zbuilder, op2.zbuilder) && (op1.zcount == op2.zcount) && (op1.zcumulative_gas_used == op2.zcumulative_gas_used) && EQUAL(zoptionzIRPendingReceiptzK)(op1.zfirst, op2.zfirst) && EQUAL(zoptionzIRPendingReceiptzK)(op1.zpending, op2.zpending);
}

static void CREATE(zHaltKind)(struct zHaltKind *op) {
  op->kind = Kind_zHaltReturn;
}

static void RECREATE(zHaltKind)(struct zHaltKind *op) {

}

static void KILL(zHaltKind)(struct zHaltKind *op) {

}

static void COPY(zHaltKind)(struct zHaltKind *rop, struct zHaltKind op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zHaltReturn: {
    rop->variants.zHaltReturn = op.variants.zHaltReturn;
    break;
  }
  case Kind_zHaltRevert: {
    rop->variants.zHaltRevert = op.variants.zHaltRevert;
    break;
  }
  case Kind_zHaltSelfDestruct: {
    rop->variants.zHaltSelfDestruct = op.variants.zHaltSelfDestruct;
    break;
  }
  case Kind_zHaltStop: {
    rop->variants.zHaltStop = op.variants.zHaltStop;
    break;
  }
  }
}

static bool EQUAL(zHaltKind)(struct zHaltKind op1, struct zHaltKind op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zHaltReturn: {
    return EQUAL(zByteSliceFields)(op1.variants.zHaltReturn, op2.variants.zHaltReturn);
    break;
  }
  case Kind_zHaltRevert: {
    return EQUAL(zByteSliceFields)(op1.variants.zHaltRevert, op2.variants.zHaltRevert);
    break;
  }
  case Kind_zHaltSelfDestruct: {
    return EQUAL(unit)(op1.variants.zHaltSelfDestruct, op2.variants.zHaltSelfDestruct);
    break;
  }
  case Kind_zHaltStop: {
    return EQUAL(unit)(op1.variants.zHaltStop, op2.variants.zHaltStop);
    break;
  }
  }
  return false;
}

static void zHaltReturn(struct zHaltKind *rop, struct zByteSliceFields op) {

  rop->kind = Kind_zHaltReturn;
  rop->variants.zHaltReturn = op;
}

static void zHaltRevert(struct zHaltKind *rop, struct zByteSliceFields op) {

  rop->kind = Kind_zHaltRevert;
  rop->variants.zHaltRevert = op;
}

static void zHaltSelfDestruct(struct zHaltKind *rop, unit op) {

  rop->kind = Kind_zHaltSelfDestruct;
  rop->variants.zHaltSelfDestruct = op;
}

static void zHaltStop(struct zHaltKind *rop, unit op) {

  rop->kind = Kind_zHaltStop;
  rop->variants.zHaltStop = op;
}

static void CREATE(zFrameStatus)(struct zFrameStatus *op) {
  op->kind = Kind_zExceptional;
}

static void RECREATE(zFrameStatus)(struct zFrameStatus *op) {

}

static void KILL(zFrameStatus)(struct zFrameStatus *op) {
  switch (op->kind) {
  case Kind_zHalted: {
    KILL(zHaltKind)(&op->variants.zHalted);
    break;
  }
  default: break;
  }
}

static void COPY(zFrameStatus)(struct zFrameStatus *rop, struct zFrameStatus op) {
  switch (rop->kind) {
  case Kind_zHalted: {
    KILL(zHaltKind)(&rop->variants.zHalted);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zExceptional: {
    rop->variants.zExceptional = op.variants.zExceptional;
    break;
  }
  case Kind_zHalted: {
    CREATE(zHaltKind)(&rop->variants.zHalted); COPY(zHaltKind)(&rop->variants.zHalted, op.variants.zHalted);
    break;
  }
  case Kind_zRunning: {
    rop->variants.zRunning = op.variants.zRunning;
    break;
  }
  }
}

static bool EQUAL(zFrameStatus)(struct zFrameStatus op1, struct zFrameStatus op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zExceptional: {
    return EQUAL(zExceptionKind)(op1.variants.zExceptional, op2.variants.zExceptional);
    break;
  }
  case Kind_zHalted: {
    return EQUAL(zHaltKind)(op1.variants.zHalted, op2.variants.zHalted);
    break;
  }
  case Kind_zRunning: {
    return EQUAL(unit)(op1.variants.zRunning, op2.variants.zRunning);
    break;
  }
  }
  return false;
}

static void zExceptional(struct zFrameStatus *rop, enum zExceptionKind op) {
  switch (rop->kind) {
  case Kind_zHalted: {
    KILL(zHaltKind)(&rop->variants.zHalted);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zExceptional;
  rop->variants.zExceptional = op;
}

static void zHalted(struct zFrameStatus *rop, struct zHaltKind op) {
  switch (rop->kind) {
  case Kind_zHalted: {
    KILL(zHaltKind)(&rop->variants.zHalted);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zHalted;
  CREATE(zHaltKind)(&rop->variants.zHalted);
  COPY(zHaltKind)(&rop->variants.zHalted, op);
}

static void zRunning(struct zFrameStatus *rop, unit op) {
  switch (rop->kind) {
  case Kind_zHalted: {
    KILL(zHaltKind)(&rop->variants.zHalted);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zRunning;
  rop->variants.zRunning = op;
}

static void COPY(zExecutionRequests)(struct zExecutionRequests *rop, const struct zExecutionRequests op) {
  rop->zbuilder_deposits = op.zbuilder_deposits;
  rop->zbuilder_exits = op.zbuilder_exits;
  rop->zconsolidations = op.zconsolidations;
  rop->zdeposits = op.zdeposits;
  rop->zwithdrawals = op.zwithdrawals;
}

static bool EQUAL(zExecutionRequests)(struct zExecutionRequests op1, struct zExecutionRequests op2) {
  return EQUAL(zByteSliceFields)(op1.zbuilder_deposits, op2.zbuilder_deposits) && EQUAL(zByteSliceFields)(op1.zbuilder_exits, op2.zbuilder_exits) && EQUAL(zByteSliceFields)(op1.zconsolidations, op2.zconsolidations) && EQUAL(zByteSliceFields)(op1.zdeposits, op2.zdeposits) && EQUAL(zByteSliceFields)(op1.zwithdrawals, op2.zwithdrawals);
}

static void COPY(zEncodedBlockAccessList)(struct zEncodedBlockAccessList *rop, const struct zEncodedBlockAccessList op) {
  rop->zbytes = op.zbytes;
  rop->zitem_count = op.zitem_count;
}

static bool EQUAL(zEncodedBlockAccessList)(struct zEncodedBlockAccessList op1, struct zEncodedBlockAccessList op2) {
  return EQUAL(zByteSliceFields)(op1.zbytes, op2.zbytes) && (op1.zitem_count == op2.zitem_count);
}

static void COPY(zCode)(struct zCode *rop, const struct zCode op) {
  rop->zbytes = op.zbytes;
  rop->zjumpdests = op.zjumpdests;
}

static bool EQUAL(zCode)(struct zCode op1, struct zCode op2) {
  return EQUAL(zByteSliceFields)(op1.zbytes, op2.zbytes) && EQUAL(fbits)(op1.zjumpdests, op2.zjumpdests);
}

static void CREATE(zoptionzIRCodezK)(struct zoptionzIRCodezK *op) {
  op->kind = Kind_zNonezIRCodezK;
}

static void RECREATE(zoptionzIRCodezK)(struct zoptionzIRCodezK *op) {

}

static void KILL(zoptionzIRCodezK)(struct zoptionzIRCodezK *op) {

}

static void COPY(zoptionzIRCodezK)(struct zoptionzIRCodezK *rop, struct zoptionzIRCodezK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRCodezK: {
    rop->variants.zNonezIRCodezK = op.variants.zNonezIRCodezK;
    break;
  }
  case Kind_zSomezIRCodezK: {
    rop->variants.zSomezIRCodezK = op.variants.zSomezIRCodezK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRCodezK)(struct zoptionzIRCodezK op1, struct zoptionzIRCodezK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRCodezK: {
    return EQUAL(unit)(op1.variants.zNonezIRCodezK, op2.variants.zNonezIRCodezK);
    break;
  }
  case Kind_zSomezIRCodezK: {
    return EQUAL(zCode)(op1.variants.zSomezIRCodezK, op2.variants.zSomezIRCodezK);
    break;
  }
  }
  return false;
}

static void zNonezIRCodezK(struct zoptionzIRCodezK *rop, unit op) {

  rop->kind = Kind_zNonezIRCodezK;
  rop->variants.zNonezIRCodezK = op;
}

static void zSomezIRCodezK(struct zoptionzIRCodezK *rop, struct zCode op) {

  rop->kind = Kind_zSomezIRCodezK;
  rop->variants.zSomezIRCodezK = op;
}

static void COPY(zFrameCheckpoint)(struct zFrameCheckpoint *rop, const struct zFrameCheckpoint op) {
  rop->zcall_depth = op.zcall_depth;
  rop->zcalldata = op.zcalldata;
  rop->zcode = op.zcode;
  rop->zgas_remaining = op.zgas_remaining;
  rop->zmemory = op.zmemory;
  rop->zmessage = op.zmessage;
  rop->zpc = op.zpc;
  rop->zrefund = op.zrefund;
  rop->zstate = op.zstate;
  rop->zstate_gas_remaining = op.zstate_gas_remaining;
  rop->zstate_gas_spilled = op.zstate_gas_spilled;
  COPY(zFrameStatus)(&rop->zstatus, op.zstatus);
}

static void CREATE(zFrameCheckpoint)(struct zFrameCheckpoint *op) {
  CREATE(zFrameStatus)(&op->zstatus);
}

static void RECREATE(zFrameCheckpoint)(struct zFrameCheckpoint *op) {
  RECREATE(zFrameStatus)(&op->zstatus);
}

static void KILL(zFrameCheckpoint)(struct zFrameCheckpoint *op) {
  KILL(zFrameStatus)(&op->zstatus);
}

static bool EQUAL(zFrameCheckpoint)(struct zFrameCheckpoint op1, struct zFrameCheckpoint op2) {
  return (op1.zcall_depth == op2.zcall_depth) && EQUAL(zByteSliceFields)(op1.zcalldata, op2.zcalldata) && EQUAL(zCode)(op1.zcode, op2.zcode) && (op1.zgas_remaining == op2.zgas_remaining) && EQUAL(zByteSliceFields)(op1.zmemory, op2.zmemory) && EQUAL(zMessage)(op1.zmessage, op2.zmessage) && (op1.zpc == op2.zpc) && (op1.zrefund == op2.zrefund) && (op1.zstate == op2.zstate) && (op1.zstate_gas_remaining == op2.zstate_gas_remaining) && (op1.zstate_gas_spilled == op2.zstate_gas_spilled) && EQUAL(zFrameStatus)(op1.zstatus, op2.zstatus);
}

static void COPY(zCreateContinuation)(struct zCreateContinuation *rop, const struct zCreateContinuation op) {
  rop->zaddress = op.zaddress;
  COPY(zFrameCheckpoint)(&rop->zcheckpoint, op.zcheckpoint);
  rop->znew_account_charged = op.znew_account_charged;
}

static void CREATE(zCreateContinuation)(struct zCreateContinuation *op) {
  CREATE(zFrameCheckpoint)(&op->zcheckpoint);
}

static void RECREATE(zCreateContinuation)(struct zCreateContinuation *op) {
  RECREATE(zFrameCheckpoint)(&op->zcheckpoint);
}

static void KILL(zCreateContinuation)(struct zCreateContinuation *op) {
  KILL(zFrameCheckpoint)(&op->zcheckpoint);
}

static bool EQUAL(zCreateContinuation)(struct zCreateContinuation op1, struct zCreateContinuation op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddress, op2.zaddress) && EQUAL(zFrameCheckpoint)(op1.zcheckpoint, op2.zcheckpoint) && EQUAL(bool)(op1.znew_account_charged, op2.znew_account_charged);
}

static void COPY(zCallContinuation)(struct zCallContinuation *rop, const struct zCallContinuation op) {
  COPY(zFrameCheckpoint)(&rop->zcheckpoint, op.zcheckpoint);
  rop->znew_account_charged = op.znew_account_charged;
  rop->zreturn_length = op.zreturn_length;
  rop->zreturn_offset = op.zreturn_offset;
}

static void CREATE(zCallContinuation)(struct zCallContinuation *op) {
  CREATE(zFrameCheckpoint)(&op->zcheckpoint);
}

static void RECREATE(zCallContinuation)(struct zCallContinuation *op) {
  RECREATE(zFrameCheckpoint)(&op->zcheckpoint);
}

static void KILL(zCallContinuation)(struct zCallContinuation *op) {
  KILL(zFrameCheckpoint)(&op->zcheckpoint);
}

static bool EQUAL(zCallContinuation)(struct zCallContinuation op1, struct zCallContinuation op2) {
  return EQUAL(zFrameCheckpoint)(op1.zcheckpoint, op2.zcheckpoint) && EQUAL(bool)(op1.znew_account_charged, op2.znew_account_charged) && (op1.zreturn_length == op2.zreturn_length) && (op1.zreturn_offset == op2.zreturn_offset);
}

static void CREATE(zFrameContinuation)(struct zFrameContinuation *op) {
  op->kind = Kind_zResumeCall;
  CREATE(zCallContinuation)(&op->variants.zResumeCall);
}

static void RECREATE(zFrameContinuation)(struct zFrameContinuation *op) {

}

static void KILL(zFrameContinuation)(struct zFrameContinuation *op) {
  switch (op->kind) {
  case Kind_zResumeCall: {
    KILL(zCallContinuation)(&op->variants.zResumeCall);
    break;
  }
  case Kind_zResumeCreate: {
    KILL(zCreateContinuation)(&op->variants.zResumeCreate);
    break;
  }
  }
}

static void COPY(zFrameContinuation)(struct zFrameContinuation *rop, struct zFrameContinuation op) {
  switch (rop->kind) {
  case Kind_zResumeCall: {
    KILL(zCallContinuation)(&rop->variants.zResumeCall);
    break;
  }
  case Kind_zResumeCreate: {
    KILL(zCreateContinuation)(&rop->variants.zResumeCreate);
    break;
  }
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zResumeCall: {
    CREATE(zCallContinuation)(&rop->variants.zResumeCall); COPY(zCallContinuation)(&rop->variants.zResumeCall, op.variants.zResumeCall);
    break;
  }
  case Kind_zResumeCreate: {
    CREATE(zCreateContinuation)(&rop->variants.zResumeCreate); COPY(zCreateContinuation)(&rop->variants.zResumeCreate, op.variants.zResumeCreate);
    break;
  }
  }
}

static bool EQUAL(zFrameContinuation)(struct zFrameContinuation op1, struct zFrameContinuation op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zResumeCall: {
    return EQUAL(zCallContinuation)(op1.variants.zResumeCall, op2.variants.zResumeCall);
    break;
  }
  case Kind_zResumeCreate: {
    return EQUAL(zCreateContinuation)(op1.variants.zResumeCreate, op2.variants.zResumeCreate);
    break;
  }
  }
  return false;
}

static void zResumeCall(struct zFrameContinuation *rop, struct zCallContinuation op) {
  switch (rop->kind) {
  case Kind_zResumeCall: {
    KILL(zCallContinuation)(&rop->variants.zResumeCall);
    break;
  }
  case Kind_zResumeCreate: {
    KILL(zCreateContinuation)(&rop->variants.zResumeCreate);
    break;
  }
  }
  rop->kind = Kind_zResumeCall;
  CREATE(zCallContinuation)(&rop->variants.zResumeCall);
  COPY(zCallContinuation)(&rop->variants.zResumeCall, op);
}

static void zResumeCreate(struct zFrameContinuation *rop, struct zCreateContinuation op) {
  switch (rop->kind) {
  case Kind_zResumeCall: {
    KILL(zCallContinuation)(&rop->variants.zResumeCall);
    break;
  }
  case Kind_zResumeCreate: {
    KILL(zCreateContinuation)(&rop->variants.zResumeCreate);
    break;
  }
  }
  rop->kind = Kind_zResumeCreate;
  CREATE(zCreateContinuation)(&rop->variants.zResumeCreate);
  COPY(zCreateContinuation)(&rop->variants.zResumeCreate, op);
}

static void CREATE(zz5vecz8z5unionz0zzFrameContinuationz9)(zz5vecz8z5unionz0zzFrameContinuationz9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5unionz0zzFrameContinuationz9)(zz5vecz8z5unionz0zzFrameContinuationz9 *rop) {
  for (int i = 0; i < (rop->len); i++) {
    KILL(zFrameContinuation)((rop->data) + i);
  }
  if (rop->data != NULL) sail_free(rop->data);
}

static void fast_vector_init_zz5vecz8z5unionz0zzFrameContinuationz9(zz5vecz8z5unionz0zzFrameContinuationz9 *vec, const int64_t n, struct zFrameContinuation elem) {
  KILL(zz5vecz8z5unionz0zzFrameContinuationz9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(struct zFrameContinuation, m);
  for (size_t i = 0; i < m; i++) {
    CREATE(zFrameContinuation)((vec->data) + i);
    COPY(zFrameContinuation)((vec->data) + i, elem);
  }
}

static void fast_unsigned_vector_init_zz5vecz8z5unionz0zzFrameContinuationz9(zz5vecz8z5unionz0zzFrameContinuationz9 *vec, const uint64_t n, struct zFrameContinuation elem) {
  KILL(zz5vecz8z5unionz0zzFrameContinuationz9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(struct zFrameContinuation, m);
  for (size_t i = 0; i < m; i++) {
    CREATE(zFrameContinuation)((vec->data) + i);
    COPY(zFrameContinuation)((vec->data) + i, elem);
  }
}

static void RECREATE(zz5vecz8z5unionz0zzFrameContinuationz9)(zz5vecz8z5unionz0zzFrameContinuationz9 *rop) {
  KILL(zz5vecz8z5unionz0zzFrameContinuationz9)(rop);
  CREATE(zz5vecz8z5unionz0zzFrameContinuationz9)(rop);
}

static void fast_vector_access_zz5vecz8z5unionz0zzFrameContinuationz9(struct zFrameContinuation *rop, zz5vecz8z5unionz0zzFrameContinuationz9 op, int64_t n) {
  COPY(zFrameContinuation)(rop, op.data[n]);
}

static void fast_unsigned_vector_access_zz5vecz8z5unionz0zzFrameContinuationz9(struct zFrameContinuation *rop, zz5vecz8z5unionz0zzFrameContinuationz9 op, uint64_t n) {
  COPY(zFrameContinuation)(rop, op.data[n]);
}

static void COPY(zz5vecz8z5unionz0zzFrameContinuationz9)(zz5vecz8z5unionz0zzFrameContinuationz9 *rop, zz5vecz8z5unionz0zzFrameContinuationz9 op) {
  KILL(zz5vecz8z5unionz0zzFrameContinuationz9)(rop);
  rop->len = op.len;
  rop->data = sail_new_array(struct zFrameContinuation, rop->len);
  for (int i = 0; i < op.len; i++) {
    CREATE(zFrameContinuation)((rop->data) + i);
    COPY(zFrameContinuation)((rop->data) + i, op.data[i]);
  }
}

static void fast_vector_update_zz5vecz8z5unionz0zzFrameContinuationz9(zz5vecz8z5unionz0zzFrameContinuationz9 *rop, zz5vecz8z5unionz0zzFrameContinuationz9 op, const int64_t n, struct zFrameContinuation elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    COPY(zFrameContinuation)((rop->data) + m, elem);
  } else {
    COPY(zz5vecz8z5unionz0zzFrameContinuationz9)(rop, op);
    COPY(zFrameContinuation)((rop->data) + m, elem);
  }
}

static void fast_unsigned_vector_update_zz5vecz8z5unionz0zzFrameContinuationz9(zz5vecz8z5unionz0zzFrameContinuationz9 *rop, zz5vecz8z5unionz0zzFrameContinuationz9 op, const uint64_t n, struct zFrameContinuation elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    COPY(zFrameContinuation)((rop->data) + m, elem);
  } else {
    COPY(zz5vecz8z5unionz0zzFrameContinuationz9)(rop, op);
    COPY(zFrameContinuation)((rop->data) + m, elem);
  }
}

static bool EQUAL(zz5vecz8z5unionz0zzFrameContinuationz9)(const zz5vecz8z5unionz0zzFrameContinuationz9 op1, const zz5vecz8z5unionz0zzFrameContinuationz9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;
  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(zFrameContinuation)(op1.data[i], op2.data[i]);
  }
  return result;
}

static void internal_vector_update_zz5vecz8z5unionz0zzFrameContinuationz9(zz5vecz8z5unionz0zzFrameContinuationz9 *rop, zz5vecz8z5unionz0zzFrameContinuationz9 op, const int64_t n, struct zFrameContinuation elem) {
  COPY(zFrameContinuation)((rop->data) + n, elem);
}

static void internal_vector_init_zz5vecz8z5unionz0zzFrameContinuationz9(zz5vecz8z5unionz0zzFrameContinuationz9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_new_array(struct zFrameContinuation, len);
  for (int i = 0; i < len; i++) {
    CREATE(zFrameContinuation)((rop->data) + i);
  }
}

static void CREATE(zBytes)(struct zBytes *op) {
  op->kind = Kind_zBytesFixed32;
}

static void RECREATE(zBytes)(struct zBytes *op) {

}

static void KILL(zBytes)(struct zBytes *op) {
  switch (op->kind) {
  case Kind_zBytesList: {
    KILL(zMaterializzedBytes)(&op->variants.zBytesList);
    break;
  }
  default: break;
  }
}

static void COPY(zBytes)(struct zBytes *rop, struct zBytes op) {
  switch (rop->kind) {
  case Kind_zBytesList: {
    KILL(zMaterializzedBytes)(&rop->variants.zBytesList);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zBytesFixed32: {
    rop->variants.zBytesFixed32 = op.variants.zBytesFixed32;
    break;
  }
  case Kind_zBytesList: {
    CREATE(zMaterializzedBytes)(&rop->variants.zBytesList); COPY(zMaterializzedBytes)(&rop->variants.zBytesList, op.variants.zBytesList);
    break;
  }
  case Kind_zBytesSlice: {
    rop->variants.zBytesSlice = op.variants.zBytesSlice;
    break;
  }
  }
}

static bool EQUAL(zBytes)(struct zBytes op1, struct zBytes op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zBytesFixed32: {
    return EQUAL(zFixedBytes32)(op1.variants.zBytesFixed32, op2.variants.zBytesFixed32);
    break;
  }
  case Kind_zBytesList: {
    return EQUAL(zMaterializzedBytes)(op1.variants.zBytesList, op2.variants.zBytesList);
    break;
  }
  case Kind_zBytesSlice: {
    return EQUAL(zByteSliceFields)(op1.variants.zBytesSlice, op2.variants.zBytesSlice);
    break;
  }
  }
  return false;
}

static void zBytesFixed32(struct zBytes *rop, struct zFixedBytes32 op) {
  switch (rop->kind) {
  case Kind_zBytesList: {
    KILL(zMaterializzedBytes)(&rop->variants.zBytesList);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zBytesFixed32;
  rop->variants.zBytesFixed32 = op;
}

static void zBytesList(struct zBytes *rop, struct zMaterializzedBytes op) {
  switch (rop->kind) {
  case Kind_zBytesList: {
    KILL(zMaterializzedBytes)(&rop->variants.zBytesList);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zBytesList;
  CREATE(zMaterializzedBytes)(&rop->variants.zBytesList);
  COPY(zMaterializzedBytes)(&rop->variants.zBytesList, op);
}

static void zBytesSlice(struct zBytes *rop, struct zByteSliceFields op) {
  switch (rop->kind) {
  case Kind_zBytesList: {
    KILL(zMaterializzedBytes)(&rop->variants.zBytesList);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zBytesSlice;
  rop->variants.zBytesSlice = op;
}

static void CREATE(zByteRegionResult)(struct zByteRegionResult *op) {
  op->kind = Kind_zByteRegionFailed;
}

static void RECREATE(zByteRegionResult)(struct zByteRegionResult *op) {

}

static void KILL(zByteRegionResult)(struct zByteRegionResult *op) {

}

static void COPY(zByteRegionResult)(struct zByteRegionResult *rop, struct zByteRegionResult op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zByteRegionFailed: {
    rop->variants.zByteRegionFailed = op.variants.zByteRegionFailed;
    break;
  }
  case Kind_zByteRegionReady: {
    rop->variants.zByteRegionReady = op.variants.zByteRegionReady;
    break;
  }
  }
}

static bool EQUAL(zByteRegionResult)(struct zByteRegionResult op1, struct zByteRegionResult op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zByteRegionFailed: {
    return EQUAL(unit)(op1.variants.zByteRegionFailed, op2.variants.zByteRegionFailed);
    break;
  }
  case Kind_zByteRegionReady: {
    return EQUAL(zByteSliceFields)(op1.variants.zByteRegionReady, op2.variants.zByteRegionReady);
    break;
  }
  }
  return false;
}

static void zByteRegionFailed(struct zByteRegionResult *rop, unit op) {

  rop->kind = Kind_zByteRegionFailed;
  rop->variants.zByteRegionFailed = op;
}

static void zByteRegionReady(struct zByteRegionResult *rop, struct zByteSliceFields op) {

  rop->kind = Kind_zByteRegionReady;
  rop->variants.zByteRegionReady = op;
}

static void CREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop) {
  if (rop->data != NULL) sail_free(rop->data);
}

static void fast_vector_init_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *vec, const int64_t n, struct zRlpFieldRefFields elem) {
  KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(struct zRlpFieldRefFields, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void fast_unsigned_vector_init_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *vec, const uint64_t n, struct zRlpFieldRefFields elem) {
  KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(struct zRlpFieldRefFields, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void RECREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop) {
  KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(rop);
  CREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(rop);
}

static struct zRlpFieldRefFields fast_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op, int64_t n) {
  return op.data[n];
}

static struct zRlpFieldRefFields fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op, uint64_t n) {
  return op.data[n];
}

static void COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop, zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op) {
  KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(rop);
  rop->len = op.len;
  rop->data = sail_new_array(struct zRlpFieldRefFields, rop->len);
  for (int i = 0; i < op.len; i++) {
    (rop->data)[i] = op.data[i];
  }
}

static void fast_vector_update_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop, zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op, const int64_t n, struct zRlpFieldRefFields elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(rop, op);
    rop->data[m] = elem;
  }
}

static void fast_unsigned_vector_update_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop, zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op, const uint64_t n, struct zRlpFieldRefFields elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(rop, op);
    rop->data[m] = elem;
  }
}

static bool EQUAL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(const zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op1, const zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;
  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(zRlpFieldRefFields)(op1.data[i], op2.data[i]);
  }
  return result;
}

static void internal_vector_update_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop, zz5vecz8z5structz0zzRlpFieldRefFieldsz9 op, const int64_t n, struct zRlpFieldRefFields elem) {
  rop->data[n] = elem;
}

static void internal_vector_init_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_new_array(struct zRlpFieldRefFields, len);
}

static void COPY(zBranchNodeData)(struct zBranchNodeData *rop, const struct zBranchNodeData op) {
  COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&rop->zchildren, op.zchildren);
  rop->zvalue = op.zvalue;
}

static void CREATE(zBranchNodeData)(struct zBranchNodeData *op) {
  CREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&op->zchildren);
}

static void RECREATE(zBranchNodeData)(struct zBranchNodeData *op) {
  RECREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&op->zchildren);
}

static void KILL(zBranchNodeData)(struct zBranchNodeData *op) {
  KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&op->zchildren);
}

static bool EQUAL(zBranchNodeData)(struct zBranchNodeData op1, struct zBranchNodeData op2) {
  return EQUAL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(op1.zchildren, op2.zchildren) && EQUAL(zRlpFieldRefFields)(op1.zvalue, op2.zvalue);
}

static void CREATE(zTrieNode)(struct zTrieNode *op) {
  op->kind = Kind_zBranchNode;
  CREATE(zBranchNodeData)(&op->variants.zBranchNode);
}

static void RECREATE(zTrieNode)(struct zTrieNode *op) {

}

static void KILL(zTrieNode)(struct zTrieNode *op) {
  switch (op->kind) {
  case Kind_zBranchNode: {
    KILL(zBranchNodeData)(&op->variants.zBranchNode);
    break;
  }
  default: break;
  }
}

static void COPY(zTrieNode)(struct zTrieNode *rop, struct zTrieNode op) {
  switch (rop->kind) {
  case Kind_zBranchNode: {
    KILL(zBranchNodeData)(&rop->variants.zBranchNode);
    break;
  }
  default: break;
  };
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zBranchNode: {
    CREATE(zBranchNodeData)(&rop->variants.zBranchNode); COPY(zBranchNodeData)(&rop->variants.zBranchNode, op.variants.zBranchNode);
    break;
  }
  case Kind_zExtensionNode: {
    rop->variants.zExtensionNode = op.variants.zExtensionNode;
    break;
  }
  case Kind_zInvalidNode: {
    rop->variants.zInvalidNode = op.variants.zInvalidNode;
    break;
  }
  case Kind_zLeafNode: {
    rop->variants.zLeafNode = op.variants.zLeafNode;
    break;
  }
  }
}

static bool EQUAL(zTrieNode)(struct zTrieNode op1, struct zTrieNode op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zBranchNode: {
    return EQUAL(zBranchNodeData)(op1.variants.zBranchNode, op2.variants.zBranchNode);
    break;
  }
  case Kind_zExtensionNode: {
    return EQUAL(zExtensionNodeData)(op1.variants.zExtensionNode, op2.variants.zExtensionNode);
    break;
  }
  case Kind_zInvalidNode: {
    return EQUAL(unit)(op1.variants.zInvalidNode, op2.variants.zInvalidNode);
    break;
  }
  case Kind_zLeafNode: {
    return EQUAL(zLeafNodeData)(op1.variants.zLeafNode, op2.variants.zLeafNode);
    break;
  }
  }
  return false;
}

static void zBranchNode(struct zTrieNode *rop, struct zBranchNodeData op) {
  switch (rop->kind) {
  case Kind_zBranchNode: {
    KILL(zBranchNodeData)(&rop->variants.zBranchNode);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zBranchNode;
  CREATE(zBranchNodeData)(&rop->variants.zBranchNode);
  COPY(zBranchNodeData)(&rop->variants.zBranchNode, op);
}

static void zExtensionNode(struct zTrieNode *rop, struct zExtensionNodeData op) {
  switch (rop->kind) {
  case Kind_zBranchNode: {
    KILL(zBranchNodeData)(&rop->variants.zBranchNode);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zExtensionNode;
  rop->variants.zExtensionNode = op;
}

static void zInvalidNode(struct zTrieNode *rop, unit op) {
  switch (rop->kind) {
  case Kind_zBranchNode: {
    KILL(zBranchNodeData)(&rop->variants.zBranchNode);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zInvalidNode;
  rop->variants.zInvalidNode = op;
}

static void zLeafNode(struct zTrieNode *rop, struct zLeafNodeData op) {
  switch (rop->kind) {
  case Kind_zBranchNode: {
    KILL(zBranchNodeData)(&rop->variants.zBranchNode);
    break;
  }
  default: break;
  }
  rop->kind = Kind_zLeafNode;
  rop->variants.zLeafNode = op;
}

static void COPY(zBoundedSszzListRef)(struct zBoundedSszzListRef *rop, const struct zBoundedSszzListRef op) {
  rop->zbytes = op.zbytes;
  rop->zcount = op.zcount;
  rop->zmax_item_length = op.zmax_item_length;
}

static bool EQUAL(zBoundedSszzListRef)(struct zBoundedSszzListRef op1, struct zBoundedSszzListRef op2) {
  return EQUAL(zByteSliceFields)(op1.zbytes, op2.zbytes) && (op1.zcount == op2.zcount) && (op1.zmax_item_length == op2.zmax_item_length);
}

static void COPY(zBoundedSszzListCursor)(struct zBoundedSszzListCursor *rop, const struct zBoundedSszzListCursor op) {
  rop->zcurrent = op.zcurrent;
  rop->zindex = op.zindex;
  rop->zitems = op.zitems;
}

static bool EQUAL(zBoundedSszzListCursor)(struct zBoundedSszzListCursor op1, struct zBoundedSszzListCursor op2) {
  return (op1.zcurrent == op2.zcurrent) && (op1.zindex == op2.zindex) && EQUAL(zBoundedSszzListRef)(op1.zitems, op2.zitems);
}

static void COPY(zWitnessHeaderIndex)(struct zWitnessHeaderIndex *rop, const struct zWitnessHeaderIndex op) {
  rop->zcursor = op.zcursor;
  rop->zparent_base_fee_per_gas = op.zparent_base_fee_per_gas;
  rop->zparent_blob_gas_used = op.zparent_blob_gas_used;
  rop->zparent_excess_blob_gas = op.zparent_excess_blob_gas;
  rop->zparent_fields_valid = op.zparent_fields_valid;
  rop->zparent_state_root = op.zparent_state_root;
  rop->zprevious_hash = op.zprevious_hash;
  rop->zvalid = op.zvalid;
}

static bool EQUAL(zWitnessHeaderIndex)(struct zWitnessHeaderIndex op1, struct zWitnessHeaderIndex op2) {
  return EQUAL(zBoundedSszzListCursor)(op1.zcursor, op2.zcursor) && EQUAL(u256)(op1.zparent_base_fee_per_gas, op2.zparent_base_fee_per_gas) && (op1.zparent_blob_gas_used == op2.zparent_blob_gas_used) && (op1.zparent_excess_blob_gas == op2.zparent_excess_blob_gas) && EQUAL(bool)(op1.zparent_fields_valid, op2.zparent_fields_valid) && EQUAL(fixed_bytes_32)(op1.zparent_state_root, op2.zparent_state_root) && EQUAL(fixed_bytes_32)(op1.zprevious_hash, op2.zprevious_hash) && EQUAL(bool)(op1.zvalid, op2.zvalid);
}

static void COPY(zBlockHeader)(struct zBlockHeader *rop, const struct zBlockHeader op) {
  rop->zbase_fee = op.zbase_fee;
  rop->zblob_gas_used = op.zblob_gas_used;
  rop->zexcess_blob_gas = op.zexcess_blob_gas;
  rop->zextra_data = op.zextra_data;
  rop->zfee_recipient = op.zfee_recipient;
  rop->zgas_limit = op.zgas_limit;
  rop->zgas_used = op.zgas_used;
  rop->zlogs_bloom = op.zlogs_bloom;
  rop->znumber = op.znumber;
  rop->zparent_beacon_block_root = op.zparent_beacon_block_root;
  rop->zparent_hash = op.zparent_hash;
  rop->zprev_randao = op.zprev_randao;
  rop->zreceipts_root = op.zreceipts_root;
  rop->zslot_number = op.zslot_number;
  rop->zstate_root = op.zstate_root;
  rop->ztimestamp = op.ztimestamp;
}

static bool EQUAL(zBlockHeader)(struct zBlockHeader op1, struct zBlockHeader op2) {
  return EQUAL(u256)(op1.zbase_fee, op2.zbase_fee) && (op1.zblob_gas_used == op2.zblob_gas_used) && (op1.zexcess_blob_gas == op2.zexcess_blob_gas) && EQUAL(zByteSliceFields)(op1.zextra_data, op2.zextra_data) && EQUAL(fixed_bytes_20)(op1.zfee_recipient, op2.zfee_recipient) && (op1.zgas_limit == op2.zgas_limit) && (op1.zgas_used == op2.zgas_used) && EQUAL(fixed_bytes_256)(op1.zlogs_bloom, op2.zlogs_bloom) && (op1.znumber == op2.znumber) && EQUAL(fixed_bytes_32)(op1.zparent_beacon_block_root, op2.zparent_beacon_block_root) && EQUAL(fixed_bytes_32)(op1.zparent_hash, op2.zparent_hash) && EQUAL(u256)(op1.zprev_randao, op2.zprev_randao) && EQUAL(fixed_bytes_32)(op1.zreceipts_root, op2.zreceipts_root) && (op1.zslot_number == op2.zslot_number) && EQUAL(fixed_bytes_32)(op1.zstate_root, op2.zstate_root) && (op1.ztimestamp == op2.ztimestamp);
}

static void COPY(zBlockExecutionResult)(struct zBlockExecutionResult *rop, const struct zBlockExecutionResult op) {
  rop->zblob_gas_used = op.zblob_gas_used;
  rop->zdeposits = op.zdeposits;
  rop->zexecution_gas_used = op.zexecution_gas_used;
  rop->zfirst_tx_recipient = op.zfirst_tx_recipient;
  rop->zheader_gas_used = op.zheader_gas_used;
  rop->zlogs_bloom = op.zlogs_bloom;
  rop->zreceipts_root = op.zreceipts_root;
  rop->zrequests = op.zrequests;
  rop->zstate_gas_used = op.zstate_gas_used;
}

static bool EQUAL(zBlockExecutionResult)(struct zBlockExecutionResult op1, struct zBlockExecutionResult op2) {
  return (op1.zblob_gas_used == op2.zblob_gas_used) && EQUAL(zByteSliceFields)(op1.zdeposits, op2.zdeposits) && (op1.zexecution_gas_used == op2.zexecution_gas_used) && EQUAL(fixed_bytes_20)(op1.zfirst_tx_recipient, op2.zfirst_tx_recipient) && (op1.zheader_gas_used == op2.zheader_gas_used) && EQUAL(fixed_bytes_256)(op1.zlogs_bloom, op2.zlogs_bloom) && EQUAL(fixed_bytes_32)(op1.zreceipts_root, op2.zreceipts_root) && EQUAL(zExecutionRequests)(op1.zrequests, op2.zrequests) && (op1.zstate_gas_used == op2.zstate_gas_used);
}

static bool EQUAL(zBlockError)(enum zBlockError op1, enum zBlockError op2) {
  return op1 == op2;
}

static enum zBlockError UNDEFINED(zBlockError)(unit u) { return zInvalidConfig; }

static void CREATE(zexception)(struct zexception *op) {
  op->kind = Kind_zInvalidBlock;
}

static void RECREATE(zexception)(struct zexception *op) {

}

static void KILL(zexception)(struct zexception *op) {

}

static void COPY(zexception)(struct zexception *rop, struct zexception op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zInvalidBlock: {
    rop->variants.zInvalidBlock = op.variants.zInvalidBlock;
    break;
  }
  }
}

static bool EQUAL(zexception)(struct zexception op1, struct zexception op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zInvalidBlock: {
    return EQUAL(zBlockError)(op1.variants.zInvalidBlock, op2.variants.zInvalidBlock);
    break;
  }
  }
  return false;
}

static void zInvalidBlock(struct zexception *rop, enum zBlockError op) {

  rop->kind = Kind_zInvalidBlock;
  rop->variants.zInvalidBlock = op;
}

static void COPY(zStatelessValidationFailure)(struct zStatelessValidationFailure *rop, const struct zStatelessValidationFailure op) {
  rop->zreason = op.zreason;
  rop->zscope = op.zscope;
}

static bool EQUAL(zStatelessValidationFailure)(struct zStatelessValidationFailure op1, struct zStatelessValidationFailure op2) {
  return EQUAL(zBlockError)(op1.zreason, op2.zreason) && EQUAL(fbits)(op1.zscope, op2.zscope);
}

static void CREATE(zStatelessValidationResult)(struct zStatelessValidationResult *op) {
  op->kind = Kind_zStatelessPayloadInvalid;
}

static void RECREATE(zStatelessValidationResult)(struct zStatelessValidationResult *op) {

}

static void KILL(zStatelessValidationResult)(struct zStatelessValidationResult *op) {

}

static void COPY(zStatelessValidationResult)(struct zStatelessValidationResult *rop, struct zStatelessValidationResult op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zStatelessPayloadInvalid: {
    rop->variants.zStatelessPayloadInvalid = op.variants.zStatelessPayloadInvalid;
    break;
  }
  case Kind_zStatelessPayloadValid: {
    rop->variants.zStatelessPayloadValid = op.variants.zStatelessPayloadValid;
    break;
  }
  }
}

static bool EQUAL(zStatelessValidationResult)(struct zStatelessValidationResult op1, struct zStatelessValidationResult op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zStatelessPayloadInvalid: {
    return EQUAL(zStatelessValidationFailure)(op1.variants.zStatelessPayloadInvalid, op2.variants.zStatelessPayloadInvalid);
    break;
  }
  case Kind_zStatelessPayloadValid: {
    return EQUAL(unit)(op1.variants.zStatelessPayloadValid, op2.variants.zStatelessPayloadValid);
    break;
  }
  }
  return false;
}

static void zStatelessPayloadInvalid(struct zStatelessValidationResult *rop, struct zStatelessValidationFailure op) {

  rop->kind = Kind_zStatelessPayloadInvalid;
  rop->variants.zStatelessPayloadInvalid = op;
}

static void zStatelessPayloadValid(struct zStatelessValidationResult *rop, unit op) {

  rop->kind = Kind_zStatelessPayloadValid;
  rop->variants.zStatelessPayloadValid = op;
}

static void COPY(zBlockBody)(struct zBlockBody *rop, const struct zBlockBody op) {
  rop->zblock_access_list = op.zblock_access_list;
  rop->ztransactions = op.ztransactions;
  rop->zwithdrawals = op.zwithdrawals;
}

static bool EQUAL(zBlockBody)(struct zBlockBody op1, struct zBlockBody op2) {
  return EQUAL(zByteSliceFields)(op1.zblock_access_list, op2.zblock_access_list) && EQUAL(zBoundedSszzListRef)(op1.ztransactions, op2.ztransactions) && EQUAL(zBoundedSszzListRef)(op1.zwithdrawals, op2.zwithdrawals);
}

static void COPY(zBlock)(struct zBlock *rop, const struct zBlock op) {
  rop->zbody = op.zbody;
  rop->zheader = op.zheader;
}

static bool EQUAL(zBlock)(struct zBlock op1, struct zBlock op2) {
  return EQUAL(zBlockBody)(op1.zbody, op2.zbody) && EQUAL(zBlockHeader)(op1.zheader, op2.zheader);
}

static void COPY(zExecutionPayload)(struct zExecutionPayload *rop, const struct zExecutionPayload op) {
  rop->zblock = op.zblock;
  rop->zexpected_block_hash = op.zexpected_block_hash;
}

static bool EQUAL(zExecutionPayload)(struct zExecutionPayload op1, struct zExecutionPayload op2) {
  return EQUAL(zBlock)(op1.zblock, op2.zblock) && EQUAL(fixed_bytes_32)(op1.zexpected_block_hash, op2.zexpected_block_hash);
}

static void COPY(zStatelessInput)(struct zStatelessInput *rop, const struct zStatelessInput op) {
  rop->zchain_config = op.zchain_config;
  rop->zpayload = op.zpayload;
}

static bool EQUAL(zStatelessInput)(struct zStatelessInput op1, struct zStatelessInput op2) {
  return EQUAL(zChainConfig)(op1.zchain_config, op2.zchain_config) && EQUAL(zExecutionPayload)(op1.zpayload, op2.zpayload);
}

static void COPY(zBlobSchedule)(struct zBlobSchedule *rop, const struct zBlobSchedule op) {
  rop->zbase_fee_update_fraction = op.zbase_fee_update_fraction;
  rop->zmax = op.zmax;
  rop->ztarget = op.ztarget;
}

static bool EQUAL(zBlobSchedule)(struct zBlobSchedule op1, struct zBlobSchedule op2) {
  return (op1.zbase_fee_update_fraction == op2.zbase_fee_update_fraction) && (op1.zmax == op2.zmax) && (op1.ztarget == op2.ztarget);
}

static void COPY(zProtocolProfile)(struct zProtocolProfile *rop, const struct zProtocolProfile op) {
  rop->zblob_schedule = op.zblob_schedule;
  rop->zfork = op.zfork;
}

static bool EQUAL(zProtocolProfile)(struct zProtocolProfile op1, struct zProtocolProfile op2) {
  return EQUAL(zBlobSchedule)(op1.zblob_schedule, op2.zblob_schedule) && EQUAL(zFork)(op1.zfork, op2.zfork);
}

static void COPY(zStatelessInputRef)(struct zStatelessInputRef *rop, const struct zStatelessInputRef op) {
  rop->zblock_access_list = op.zblock_access_list;
  rop->zbuilder_deposit_requests = op.zbuilder_deposit_requests;
  rop->zbuilder_exit_requests = op.zbuilder_exit_requests;
  rop->zchain_config = op.zchain_config;
  rop->zconsolidation_requests = op.zconsolidation_requests;
  rop->zdeposits = op.zdeposits;
  rop->zexecution_payload = op.zexecution_payload;
  rop->zextra_data = op.zextra_data;
  rop->znew_payload_request = op.znew_payload_request;
  rop->zprotocol = op.zprotocol;
  rop->zpublic_keys = op.zpublic_keys;
  rop->ztransactions = op.ztransactions;
  rop->zversioned_hashes = op.zversioned_hashes;
  rop->zwithdrawal_requests = op.zwithdrawal_requests;
  rop->zwithdrawals = op.zwithdrawals;
  rop->zwitness_codes = op.zwitness_codes;
  rop->zwitness_headers = op.zwitness_headers;
  rop->zwitness_state = op.zwitness_state;
}

static bool EQUAL(zStatelessInputRef)(struct zStatelessInputRef op1, struct zStatelessInputRef op2) {
  return EQUAL(zByteSliceFields)(op1.zblock_access_list, op2.zblock_access_list) && EQUAL(zByteSliceFields)(op1.zbuilder_deposit_requests, op2.zbuilder_deposit_requests) && EQUAL(zByteSliceFields)(op1.zbuilder_exit_requests, op2.zbuilder_exit_requests) && EQUAL(zByteSliceFields)(op1.zchain_config, op2.zchain_config) && EQUAL(zByteSliceFields)(op1.zconsolidation_requests, op2.zconsolidation_requests) && EQUAL(zByteSliceFields)(op1.zdeposits, op2.zdeposits) && EQUAL(zByteSliceFields)(op1.zexecution_payload, op2.zexecution_payload) && EQUAL(zByteSliceFields)(op1.zextra_data, op2.zextra_data) && EQUAL(zByteSliceFields)(op1.znew_payload_request, op2.znew_payload_request) && EQUAL(zProtocolProfile)(op1.zprotocol, op2.zprotocol) && EQUAL(zByteSliceFields)(op1.zpublic_keys, op2.zpublic_keys) && EQUAL(zBoundedSszzListRef)(op1.ztransactions, op2.ztransactions) && EQUAL(zByteSliceFields)(op1.zversioned_hashes, op2.zversioned_hashes) && EQUAL(zByteSliceFields)(op1.zwithdrawal_requests, op2.zwithdrawal_requests) && EQUAL(zBoundedSszzListRef)(op1.zwithdrawals, op2.zwithdrawals) && EQUAL(zBoundedSszzListRef)(op1.zwitness_codes, op2.zwitness_codes) && EQUAL(zBoundedSszzListRef)(op1.zwitness_headers, op2.zwitness_headers) && EQUAL(zBoundedSszzListRef)(op1.zwitness_state, op2.zwitness_state);
}

static void COPY(zGuestValidation)(struct zGuestValidation *rop, const struct zGuestValidation op) {
  rop->zinput_ref = op.zinput_ref;
  rop->zvalid = op.zvalid;
}

static bool EQUAL(zGuestValidation)(struct zGuestValidation op1, struct zGuestValidation op2) {
  return EQUAL(zStatelessInputRef)(op1.zinput_ref, op2.zinput_ref) && EQUAL(bool)(op1.zvalid, op2.zvalid);
}

static void CREATE(zoptionzIRGuestValidationzK)(struct zoptionzIRGuestValidationzK *op) {
  op->kind = Kind_zNonezIRGuestValidationzK;
}

static void RECREATE(zoptionzIRGuestValidationzK)(struct zoptionzIRGuestValidationzK *op) {

}

static void KILL(zoptionzIRGuestValidationzK)(struct zoptionzIRGuestValidationzK *op) {

}

static void COPY(zoptionzIRGuestValidationzK)(struct zoptionzIRGuestValidationzK *rop, struct zoptionzIRGuestValidationzK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRGuestValidationzK: {
    rop->variants.zNonezIRGuestValidationzK = op.variants.zNonezIRGuestValidationzK;
    break;
  }
  case Kind_zSomezIRGuestValidationzK: {
    rop->variants.zSomezIRGuestValidationzK = op.variants.zSomezIRGuestValidationzK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRGuestValidationzK)(struct zoptionzIRGuestValidationzK op1, struct zoptionzIRGuestValidationzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRGuestValidationzK: {
    return EQUAL(unit)(op1.variants.zNonezIRGuestValidationzK, op2.variants.zNonezIRGuestValidationzK);
    break;
  }
  case Kind_zSomezIRGuestValidationzK: {
    return EQUAL(zGuestValidation)(op1.variants.zSomezIRGuestValidationzK, op2.variants.zSomezIRGuestValidationzK);
    break;
  }
  }
  return false;
}

static void zNonezIRGuestValidationzK(struct zoptionzIRGuestValidationzK *rop, unit op) {

  rop->kind = Kind_zNonezIRGuestValidationzK;
  rop->variants.zNonezIRGuestValidationzK = op;
}

static void zSomezIRGuestValidationzK(struct zoptionzIRGuestValidationzK *rop, struct zGuestValidation op) {

  rop->kind = Kind_zSomezIRGuestValidationzK;
  rop->variants.zSomezIRGuestValidationzK = op;
}

static void COPY(zBlobProductDivMod)(struct zBlobProductDivMod *rop, const struct zBlobProductDivMod op) {
  rop->zquotient = op.zquotient;
  rop->zremainder = op.zremainder;
}

static bool EQUAL(zBlobProductDivMod)(struct zBlobProductDivMod op1, struct zBlobProductDivMod op2) {
  return EQUAL(u256)(op1.zquotient, op2.zquotient) && EQUAL(u256)(op1.zremainder, op2.zremainder);
}

static void COPY(zBlobHashes)(struct zBlobHashes *rop, const struct zBlobHashes op) {
  rop->zbytes = op.zbytes;
  rop->zcount = op.zcount;
}

static bool EQUAL(zBlobHashes)(struct zBlobHashes op1, struct zBlobHashes op2) {
  return EQUAL(zByteSliceFields)(op1.zbytes, op2.zbytes) && (op1.zcount == op2.zcount);
}

static void COPY(zTxEnv)(struct zTxEnv *rop, const struct zTxEnv op) {
  rop->zblob_hashes = op.zblob_hashes;
  rop->zgas_price = op.zgas_price;
  rop->zorigin = op.zorigin;
}

static bool EQUAL(zTxEnv)(struct zTxEnv op1, struct zTxEnv op2) {
  return EQUAL(zBlobHashes)(op1.zblob_hashes, op2.zblob_hashes) && EQUAL(u256)(op1.zgas_price, op2.zgas_price) && EQUAL(fixed_bytes_20)(op1.zorigin, op2.zorigin);
}

static void COPY(zBalNonceRun)(struct zBalNonceRun *rop, const struct zBalNonceRun op) {
  rop->zcursor = op.zcursor;
  rop->zmaximum = op.zmaximum;
}

static bool EQUAL(zBalNonceRun)(struct zBalNonceRun op1, struct zBalNonceRun op2) {
  return (op1.zcursor == op2.zcursor) && (op1.zmaximum == op2.zmaximum);
}

static void COPY(zBalContentCursor)(struct zBalContentCursor *rop, const struct zBalContentCursor op) {
  rop->zcontent_len = op.zcontent_len;
  rop->zcursor = op.zcursor;
}

static bool EQUAL(zBalContentCursor)(struct zBalContentCursor op1, struct zBalContentCursor op2) {
  return (op1.zcontent_len == op2.zcontent_len) && (op1.zcursor == op2.zcursor);
}

static void COPY(zBalContentCount)(struct zBalContentCount *rop, const struct zBalContentCount op) {
  rop->zcontent_len = op.zcontent_len;
  rop->zcount = op.zcount;
}

static bool EQUAL(zBalContentCount)(struct zBalContentCount op1, struct zBalContentCount op2) {
  return (op1.zcontent_len == op2.zcontent_len) && (op1.zcount == op2.zcount);
}

static void COPY(zBalAccountSizze)(struct zBalAccountSizze *rop, const struct zBalAccountSizze op) {
  rop->zencoded_len = op.zencoded_len;
  rop->zitem_count = op.zitem_count;
}

static bool EQUAL(zBalAccountSizze)(struct zBalAccountSizze op1, struct zBalAccountSizze op2) {
  return (op1.zencoded_len == op2.zencoded_len) && (op1.zitem_count == op2.zitem_count);
}

static void COPY(zAuthorizzation)(struct zAuthorizzation *rop, const struct zAuthorizzation op) {
  rop->zaddress = op.zaddress;
  rop->zauthority = op.zauthority;
  rop->zchain_id = op.zchain_id;
  rop->znonce = op.znonce;
  rop->zvalid_sig = op.zvalid_sig;
}

static bool EQUAL(zAuthorizzation)(struct zAuthorizzation op1, struct zAuthorizzation op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddress, op2.zaddress) && EQUAL(fixed_bytes_20)(op1.zauthority, op2.zauthority) && EQUAL(u256)(op1.zchain_id, op2.zchain_id) && (op1.znonce == op2.znonce) && EQUAL(bool)(op1.zvalid_sig, op2.zvalid_sig);
}

static void CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(zz5listz8z5structz0zzAuthorizzzzationz9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zzAuthorizzzzationz9(zz5listz8z5structz0zzAuthorizzzzationz9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zzAuthorizzzzationz9(zz5listz8z5structz0zzAuthorizzzzationz9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(zz5listz8z5structz0zzAuthorizzzzationz9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zzAuthorizzzzationz9 node = *rop;
  while (node != NULL && node->rc == 0) {
    zz5listz8z5structz0zzAuthorizzzzationz9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zzAuthorizzzzationz9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(zz5listz8z5structz0zzAuthorizzzzationz9 *rop) {
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(zz5listz8z5structz0zzAuthorizzzzationz9 *rop, zz5listz8z5structz0zzAuthorizzzzationz9 op) {
  internal_inc_zz5listz8z5structz0zzAuthorizzzzationz9(op);
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zzAuthorizzzzation(zz5listz8z5structz0zzAuthorizzzzationz9 *rop, struct zAuthorizzation x, zz5listz8z5structz0zzAuthorizzzzationz9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zzAuthorizzzzationz9);
  (*rop)->rc = 1;
  (*rop)->hd = x;
  if (!same) internal_inc_zz5listz8z5structz0zzAuthorizzzzationz9(xs);
  (*rop)->tl = xs;
}

static struct zAuthorizzation pick_zAuthorizzation(const zz5listz8z5structz0zzAuthorizzzzationz9 xs) {
  return xs->hd;
}

static bool EQUAL(zz5listz8z5structz0zzAuthorizzzzationz9)(const zz5listz8z5structz0zzAuthorizzzzationz9 op1, const zz5listz8z5structz0zzAuthorizzzzationz9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(zAuthorizzation)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zzAuthorizzzzationz9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zzAuthorizzzzationz9)(zz5listz8z5structz0zzAuthorizzzzationz9 *rop, struct zAuthorizzation u) {
  *rop = NULL;
}

static void CREATE(zz5listz8z5structz0zzStorageKeyz9)(zz5listz8z5structz0zzStorageKeyz9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zzStorageKeyz9(zz5listz8z5structz0zzStorageKeyz9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zzStorageKeyz9(zz5listz8z5structz0zzStorageKeyz9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zzStorageKeyz9)(zz5listz8z5structz0zzStorageKeyz9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zzStorageKeyz9 node = *rop;
  while (node != NULL && node->rc == 0) {
    zz5listz8z5structz0zzStorageKeyz9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zzStorageKeyz9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zzStorageKeyz9)(zz5listz8z5structz0zzStorageKeyz9 *rop) {
  KILL(zz5listz8z5structz0zzStorageKeyz9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zzStorageKeyz9)(zz5listz8z5structz0zzStorageKeyz9 *rop, zz5listz8z5structz0zzStorageKeyz9 op) {
  internal_inc_zz5listz8z5structz0zzStorageKeyz9(op);
  KILL(zz5listz8z5structz0zzStorageKeyz9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zzStorageKey(zz5listz8z5structz0zzStorageKeyz9 *rop, struct zStorageKey x, zz5listz8z5structz0zzStorageKeyz9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zzStorageKeyz9);
  (*rop)->rc = 1;
  (*rop)->hd = x;
  if (!same) internal_inc_zz5listz8z5structz0zzStorageKeyz9(xs);
  (*rop)->tl = xs;
}

static struct zStorageKey pick_zStorageKey(const zz5listz8z5structz0zzStorageKeyz9 xs) {
  return xs->hd;
}

static bool EQUAL(zz5listz8z5structz0zzStorageKeyz9)(const zz5listz8z5structz0zzStorageKeyz9 op1, const zz5listz8z5structz0zzStorageKeyz9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(zStorageKey)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zzStorageKeyz9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zzStorageKeyz9)(zz5listz8z5structz0zzStorageKeyz9 *rop, struct zStorageKey u) {
  *rop = NULL;
}

static void CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 node = *rop;
  while (node != NULL && node->rc == 0) {
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 op) {
  internal_inc_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9(op);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop, sail_fixed_bytes_20 x, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9);
  (*rop)->rc = 1;
  (*rop)->hd = x;
  if (!same) internal_inc_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9(xs);
  (*rop)->tl = xs;
}

static sail_fixed_bytes_20 pick_fixed_bytes_20(const zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 xs) {
  return xs->hd;
}

static bool EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(const zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 op1, const zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(fixed_bytes_20)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop, sail_fixed_bytes_20 u) {
  *rop = NULL;
}

static void COPY(zTransaction)(struct zTransaction *rop, const struct zTransaction op) {
  rop->zaccess_list_address_count = op.zaccess_list_address_count;
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&rop->zaccess_list_addresses, op.zaccess_list_addresses);
  rop->zaccess_list_slot_count = op.zaccess_list_slot_count;
  COPY(zz5listz8z5structz0zzStorageKeyz9)(&rop->zaccess_list_slots, op.zaccess_list_slots);
  rop->zauthorizzation_count = op.zauthorizzation_count;
  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&rop->zauthorizzations, op.zauthorizzations);
  rop->zblob_hashes = op.zblob_hashes;
  rop->zchain_id = op.zchain_id;
  rop->zgas_limit = op.zgas_limit;
  rop->zinput_src = op.zinput_src;
  rop->zis_create = op.zis_create;
  rop->zmax_blob_fee = op.zmax_blob_fee;
  rop->zmax_fee = op.zmax_fee;
  rop->zmax_priority_fee = op.zmax_priority_fee;
  rop->znonce = op.znonce;
  rop->zpubkey = op.zpubkey;
  rop->zraw = op.zraw;
  rop->zrecipient = op.zrecipient;
  rop->zsender = op.zsender;
  rop->zsig_r = op.zsig_r;
  rop->zsig_s = op.zsig_s;
  rop->zsig_v = op.zsig_v;
  rop->zsigning_hash = op.zsigning_hash;
  rop->ztx_type = op.ztx_type;
  rop->zvalue = op.zvalue;
}

static void CREATE(zTransaction)(struct zTransaction *op) {
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zaccess_list_addresses);
  CREATE(zz5listz8z5structz0zzStorageKeyz9)(&op->zaccess_list_slots);
  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->zauthorizzations);
}

static void RECREATE(zTransaction)(struct zTransaction *op) {
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zaccess_list_addresses);
  RECREATE(zz5listz8z5structz0zzStorageKeyz9)(&op->zaccess_list_slots);
  RECREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->zauthorizzations);
}

static void KILL(zTransaction)(struct zTransaction *op) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zaccess_list_addresses);
  KILL(zz5listz8z5structz0zzStorageKeyz9)(&op->zaccess_list_slots);
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->zauthorizzations);
}

static bool EQUAL(zTransaction)(struct zTransaction op1, struct zTransaction op2) {
  return (op1.zaccess_list_address_count == op2.zaccess_list_address_count) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1.zaccess_list_addresses, op2.zaccess_list_addresses) && (op1.zaccess_list_slot_count == op2.zaccess_list_slot_count) && EQUAL(zz5listz8z5structz0zzStorageKeyz9)(op1.zaccess_list_slots, op2.zaccess_list_slots) && (op1.zauthorizzation_count == op2.zauthorizzation_count) && EQUAL(zz5listz8z5structz0zzAuthorizzzzationz9)(op1.zauthorizzations, op2.zauthorizzations) && EQUAL(zBlobHashes)(op1.zblob_hashes, op2.zblob_hashes) && (op1.zchain_id == op2.zchain_id) && (op1.zgas_limit == op2.zgas_limit) && EQUAL(zByteSliceFields)(op1.zinput_src, op2.zinput_src) && EQUAL(bool)(op1.zis_create, op2.zis_create) && EQUAL(u256)(op1.zmax_blob_fee, op2.zmax_blob_fee) && EQUAL(u256)(op1.zmax_fee, op2.zmax_fee) && EQUAL(u256)(op1.zmax_priority_fee, op2.zmax_priority_fee) && EQUAL(u256)(op1.znonce, op2.znonce) && EQUAL(zByteSliceFields)(op1.zpubkey, op2.zpubkey) && EQUAL(zByteSliceFields)(op1.zraw, op2.zraw) && EQUAL(fixed_bytes_20)(op1.zrecipient, op2.zrecipient) && EQUAL(fixed_bytes_20)(op1.zsender, op2.zsender) && EQUAL(u256)(op1.zsig_r, op2.zsig_r) && EQUAL(u256)(op1.zsig_s, op2.zsig_s) && EQUAL(u256)(op1.zsig_v, op2.zsig_v) && EQUAL(fixed_bytes_32)(op1.zsigning_hash, op2.zsigning_hash) && EQUAL(zTxType)(op1.ztx_type, op2.ztx_type) && EQUAL(u256)(op1.zvalue, op2.zvalue);
}

static void COPY(zAuthorizzationDecode)(struct zAuthorizzationDecode *rop, const struct zAuthorizzationDecode op) {
  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&rop->zauthorizzations, op.zauthorizzations);
  rop->zcount = op.zcount;
}

static void CREATE(zAuthorizzationDecode)(struct zAuthorizzationDecode *op) {
  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->zauthorizzations);
}

static void RECREATE(zAuthorizzationDecode)(struct zAuthorizzationDecode *op) {
  RECREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->zauthorizzations);
}

static void KILL(zAuthorizzationDecode)(struct zAuthorizzationDecode *op) {
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->zauthorizzations);
}

static bool EQUAL(zAuthorizzationDecode)(struct zAuthorizzationDecode op1, struct zAuthorizzationDecode op2) {
  return EQUAL(zz5listz8z5structz0zzAuthorizzzzationz9)(op1.zauthorizzations, op2.zauthorizzations) && (op1.zcount == op2.zcount);
}

static void COPY(zAmsterdamAuthorizzationState)(struct zAmsterdamAuthorizzationState *rop, const struct zAmsterdamAuthorizzationState op) {
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&rop->zdelegation_set_for, op.zdelegation_set_for);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&rop->zoriginally_delegated, op.zoriginally_delegated);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&rop->zseen_valid_authorities, op.zseen_valid_authorities);
}

static void CREATE(zAmsterdamAuthorizzationState)(struct zAmsterdamAuthorizzationState *op) {
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zdelegation_set_for);
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zoriginally_delegated);
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zseen_valid_authorities);
}

static void RECREATE(zAmsterdamAuthorizzationState)(struct zAmsterdamAuthorizzationState *op) {
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zdelegation_set_for);
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zoriginally_delegated);
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zseen_valid_authorities);
}

static void KILL(zAmsterdamAuthorizzationState)(struct zAmsterdamAuthorizzationState *op) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zdelegation_set_for);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zoriginally_delegated);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zseen_valid_authorities);
}

static bool EQUAL(zAmsterdamAuthorizzationState)(struct zAmsterdamAuthorizzationState op1, struct zAmsterdamAuthorizzationState op2) {
  return EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1.zdelegation_set_for, op2.zdelegation_set_for) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1.zoriginally_delegated, op2.zoriginally_delegated) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1.zseen_valid_authorities, op2.zseen_valid_authorities);
}

static void COPY(zAddressResult)(struct zAddressResult *rop, const struct zAddressResult op) {
  rop->zaddress = op.zaddress;
  rop->zsuccess = op.zsuccess;
}

static bool EQUAL(zAddressResult)(struct zAddressResult op1, struct zAddressResult op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddress, op2.zaddress) && EQUAL(bool)(op1.zsuccess, op2.zsuccess);
}

static void COPY(zAccountInfo)(struct zAccountInfo *rop, const struct zAccountInfo op) {
  rop->zbalance = op.zbalance;
  rop->zcode_hash = op.zcode_hash;
  rop->znonce = op.znonce;
  rop->zstorage_root = op.zstorage_root;
}

static bool EQUAL(zAccountInfo)(struct zAccountInfo op1, struct zAccountInfo op2) {
  return EQUAL(u256)(op1.zbalance, op2.zbalance) && EQUAL(fixed_bytes_32)(op1.zcode_hash, op2.zcode_hash) && (op1.znonce == op2.znonce) && EQUAL(fixed_bytes_32)(op1.zstorage_root, op2.zstorage_root);
}

static void CREATE(zoptionzIRAccountInfozK)(struct zoptionzIRAccountInfozK *op) {
  op->kind = Kind_zNonezIRAccountInfozK;
}

static void RECREATE(zoptionzIRAccountInfozK)(struct zoptionzIRAccountInfozK *op) {

}

static void KILL(zoptionzIRAccountInfozK)(struct zoptionzIRAccountInfozK *op) {

}

static void COPY(zoptionzIRAccountInfozK)(struct zoptionzIRAccountInfozK *rop, struct zoptionzIRAccountInfozK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRAccountInfozK: {
    rop->variants.zNonezIRAccountInfozK = op.variants.zNonezIRAccountInfozK;
    break;
  }
  case Kind_zSomezIRAccountInfozK: {
    rop->variants.zSomezIRAccountInfozK = op.variants.zSomezIRAccountInfozK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRAccountInfozK)(struct zoptionzIRAccountInfozK op1, struct zoptionzIRAccountInfozK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRAccountInfozK: {
    return EQUAL(unit)(op1.variants.zNonezIRAccountInfozK, op2.variants.zNonezIRAccountInfozK);
    break;
  }
  case Kind_zSomezIRAccountInfozK: {
    return EQUAL(zAccountInfo)(op1.variants.zSomezIRAccountInfozK, op2.variants.zSomezIRAccountInfozK);
    break;
  }
  }
  return false;
}

static void zNonezIRAccountInfozK(struct zoptionzIRAccountInfozK *rop, unit op) {

  rop->kind = Kind_zNonezIRAccountInfozK;
  rop->variants.zNonezIRAccountInfozK = op;
}

static void zSomezIRAccountInfozK(struct zoptionzIRAccountInfozK *rop, struct zAccountInfo op) {

  rop->kind = Kind_zSomezIRAccountInfozK;
  rop->variants.zSomezIRAccountInfozK = op;
}

static void COPY(zAccount)(struct zAccount *rop, const struct zAccount op) {
  rop->zcreated = op.zcreated;
  rop->zinfo = op.zinfo;
  rop->zpresent = op.zpresent;
  rop->zselfdestructed = op.zselfdestructed;
  rop->zstorage_cleared = op.zstorage_cleared;
}

static bool EQUAL(zAccount)(struct zAccount op1, struct zAccount op2) {
  return EQUAL(bool)(op1.zcreated, op2.zcreated) && EQUAL(zAccountInfo)(op1.zinfo, op2.zinfo) && EQUAL(bool)(op1.zpresent, op2.zpresent) && EQUAL(bool)(op1.zselfdestructed, op2.zselfdestructed) && EQUAL(bool)(op1.zstorage_cleared, op2.zstorage_cleared);
}

static void CREATE(zoptionzIRAccountzK)(struct zoptionzIRAccountzK *op) {
  op->kind = Kind_zNonezIRAccountzK;
}

static void RECREATE(zoptionzIRAccountzK)(struct zoptionzIRAccountzK *op) {

}

static void KILL(zoptionzIRAccountzK)(struct zoptionzIRAccountzK *op) {

}

static void COPY(zoptionzIRAccountzK)(struct zoptionzIRAccountzK *rop, struct zoptionzIRAccountzK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRAccountzK: {
    rop->variants.zNonezIRAccountzK = op.variants.zNonezIRAccountzK;
    break;
  }
  case Kind_zSomezIRAccountzK: {
    rop->variants.zSomezIRAccountzK = op.variants.zSomezIRAccountzK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRAccountzK)(struct zoptionzIRAccountzK op1, struct zoptionzIRAccountzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRAccountzK: {
    return EQUAL(unit)(op1.variants.zNonezIRAccountzK, op2.variants.zNonezIRAccountzK);
    break;
  }
  case Kind_zSomezIRAccountzK: {
    return EQUAL(zAccount)(op1.variants.zSomezIRAccountzK, op2.variants.zSomezIRAccountzK);
    break;
  }
  }
  return false;
}

static void zNonezIRAccountzK(struct zoptionzIRAccountzK *rop, unit op) {

  rop->kind = Kind_zNonezIRAccountzK;
  rop->variants.zNonezIRAccountzK = op;
}

static void zSomezIRAccountzK(struct zoptionzIRAccountzK *rop, struct zAccount op) {

  rop->kind = Kind_zSomezIRAccountzK;
  rop->variants.zSomezIRAccountzK = op;
}

static void COPY(zAcctValue)(struct zAcctValue *rop, const struct zAcctValue op) {
  rop->zcurr = op.zcurr;
  rop->zorig = op.zorig;
}

static bool EQUAL(zAcctValue)(struct zAcctValue op1, struct zAcctValue op2) {
  return EQUAL(zAccount)(op1.zcurr, op2.zcurr) && EQUAL(zAccount)(op1.zorig, op2.zorig);
}

static void COPY(zAcctEntry)(struct zAcctEntry *rop, const struct zAcctEntry op) {
  rop->zaddr = op.zaddr;
  rop->zvalue = op.zvalue;
}

static bool EQUAL(zAcctEntry)(struct zAcctEntry op1, struct zAcctEntry op2) {
  return EQUAL(fixed_bytes_20)(op1.zaddr, op2.zaddr) && EQUAL(zAcctValue)(op1.zvalue, op2.zvalue);
}

static void CREATE(zoptionzIRAcctEntryzK)(struct zoptionzIRAcctEntryzK *op) {
  op->kind = Kind_zNonezIRAcctEntryzK;
}

static void RECREATE(zoptionzIRAcctEntryzK)(struct zoptionzIRAcctEntryzK *op) {

}

static void KILL(zoptionzIRAcctEntryzK)(struct zoptionzIRAcctEntryzK *op) {

}

static void COPY(zoptionzIRAcctEntryzK)(struct zoptionzIRAcctEntryzK *rop, struct zoptionzIRAcctEntryzK op) {
  ;
  rop->kind = op.kind;
  switch (op.kind) {
  case Kind_zNonezIRAcctEntryzK: {
    rop->variants.zNonezIRAcctEntryzK = op.variants.zNonezIRAcctEntryzK;
    break;
  }
  case Kind_zSomezIRAcctEntryzK: {
    rop->variants.zSomezIRAcctEntryzK = op.variants.zSomezIRAcctEntryzK;
    break;
  }
  }
}

static bool EQUAL(zoptionzIRAcctEntryzK)(struct zoptionzIRAcctEntryzK op1, struct zoptionzIRAcctEntryzK op2) {
  if (op1.kind != op2.kind) {
    return false;
  }
  switch (op1.kind) {
  case Kind_zNonezIRAcctEntryzK: {
    return EQUAL(unit)(op1.variants.zNonezIRAcctEntryzK, op2.variants.zNonezIRAcctEntryzK);
    break;
  }
  case Kind_zSomezIRAcctEntryzK: {
    return EQUAL(zAcctEntry)(op1.variants.zSomezIRAcctEntryzK, op2.variants.zSomezIRAcctEntryzK);
    break;
  }
  }
  return false;
}

static void zNonezIRAcctEntryzK(struct zoptionzIRAcctEntryzK *rop, unit op) {

  rop->kind = Kind_zNonezIRAcctEntryzK;
  rop->variants.zNonezIRAcctEntryzK = op;
}

static void zSomezIRAcctEntryzK(struct zoptionzIRAcctEntryzK *rop, struct zAcctEntry op) {

  rop->kind = Kind_zSomezIRAcctEntryzK;
  rop->variants.zSomezIRAcctEntryzK = op;
}

static void COPY(zAccessListDecode)(struct zAccessListDecode *rop, const struct zAccessListDecode op) {
  rop->zaddress_count = op.zaddress_count;
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&rop->zaddresses, op.zaddresses);
  rop->zslot_count = op.zslot_count;
  COPY(zz5listz8z5structz0zzStorageKeyz9)(&rop->zstorage_slots, op.zstorage_slots);
}

static void CREATE(zAccessListDecode)(struct zAccessListDecode *op) {
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zaddresses);
  CREATE(zz5listz8z5structz0zzStorageKeyz9)(&op->zstorage_slots);
}

static void RECREATE(zAccessListDecode)(struct zAccessListDecode *op) {
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zaddresses);
  RECREATE(zz5listz8z5structz0zzStorageKeyz9)(&op->zstorage_slots);
}

static void KILL(zAccessListDecode)(struct zAccessListDecode *op) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->zaddresses);
  KILL(zz5listz8z5structz0zzStorageKeyz9)(&op->zstorage_slots);
}

static bool EQUAL(zAccessListDecode)(struct zAccessListDecode op1, struct zAccessListDecode op2) {
  return (op1.zaddress_count == op2.zaddress_count) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1.zaddresses, op2.zaddresses) && (op1.zslot_count == op2.zslot_count) && EQUAL(zz5listz8z5structz0zzStorageKeyz9)(op1.zstorage_slots, op2.zstorage_slots);
}

static void CREATE(zz5vecz8z5bv8z9)(zz5vecz8z5bv8z9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5bv8z9)(zz5vecz8z5bv8z9 *rop) {
  if (rop->data != NULL) sail_free(rop->data);
}

static void fast_vector_init_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 *vec, const int64_t n, uint64_t elem) {
  KILL(zz5vecz8z5bv8z9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(uint64_t, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void fast_unsigned_vector_init_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 *vec, const uint64_t n, uint64_t elem) {
  KILL(zz5vecz8z5bv8z9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(uint64_t, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void RECREATE(zz5vecz8z5bv8z9)(zz5vecz8z5bv8z9 *rop) {
  KILL(zz5vecz8z5bv8z9)(rop);
  CREATE(zz5vecz8z5bv8z9)(rop);
}

static uint64_t fast_vector_access_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 op, int64_t n) {
  return op.data[n];
}

static uint64_t fast_unsigned_vector_access_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 op, uint64_t n) {
  return op.data[n];
}

static void COPY(zz5vecz8z5bv8z9)(zz5vecz8z5bv8z9 *rop, zz5vecz8z5bv8z9 op) {
  KILL(zz5vecz8z5bv8z9)(rop);
  rop->len = op.len;
  rop->data = sail_new_array(uint64_t, rop->len);
  for (int i = 0; i < op.len; i++) {
    (rop->data)[i] = op.data[i];
  }
}

static void fast_vector_update_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 *rop, zz5vecz8z5bv8z9 op, const int64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5bv8z9)(rop, op);
    rop->data[m] = elem;
  }
}

static void fast_unsigned_vector_update_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 *rop, zz5vecz8z5bv8z9 op, const uint64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5bv8z9)(rop, op);
    rop->data[m] = elem;
  }
}

static bool EQUAL(zz5vecz8z5bv8z9)(const zz5vecz8z5bv8z9 op1, const zz5vecz8z5bv8z9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;
  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(fbits)(op1.data[i], op2.data[i]);
  }
  return result;
}

static void internal_vector_update_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 *rop, zz5vecz8z5bv8z9 op, const int64_t n, uint64_t elem) {
  rop->data[n] = elem;
}

static void internal_vector_init_zz5vecz8z5bv8z9(zz5vecz8z5bv8z9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_new_array(uint64_t, len);
}

static void CREATE(zz5listz8z5unionz0zzBytesz9)(zz5listz8z5unionz0zzBytesz9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5unionz0zzBytesz9(zz5listz8z5unionz0zzBytesz9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5unionz0zzBytesz9(zz5listz8z5unionz0zzBytesz9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5unionz0zzBytesz9)(zz5listz8z5unionz0zzBytesz9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5unionz0zzBytesz9 node = *rop;
  while (node != NULL && node->rc == 0) {
    KILL(zBytes)(&node->hd);
    zz5listz8z5unionz0zzBytesz9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5unionz0zzBytesz9(node);
  }
}

static void RECREATE(zz5listz8z5unionz0zzBytesz9)(zz5listz8z5unionz0zzBytesz9 *rop) {
  KILL(zz5listz8z5unionz0zzBytesz9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5unionz0zzBytesz9)(zz5listz8z5unionz0zzBytesz9 *rop, zz5listz8z5unionz0zzBytesz9 op) {
  internal_inc_zz5listz8z5unionz0zzBytesz9(op);
  KILL(zz5listz8z5unionz0zzBytesz9)(rop);
  *rop = op;
}

static void zconsz3z5unionz0zzBytes(zz5listz8z5unionz0zzBytesz9 *rop, struct zBytes x, zz5listz8z5unionz0zzBytesz9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5unionz0zzBytesz9);
  (*rop)->rc = 1;
  CREATE(zBytes)(&(*rop)->hd);
  COPY(zBytes)(&(*rop)->hd, x);
  if (!same) internal_inc_zz5listz8z5unionz0zzBytesz9(xs);
  (*rop)->tl = xs;
}

static void pick_zBytes(struct zBytes *x, const zz5listz8z5unionz0zzBytesz9 xs) {
  COPY(zBytes)(x, xs->hd);
}

static bool EQUAL(zz5listz8z5unionz0zzBytesz9)(const zz5listz8z5unionz0zzBytesz9 op1, const zz5listz8z5unionz0zzBytesz9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(zBytes)(op1->hd, op2->hd) && EQUAL(zz5listz8z5unionz0zzBytesz9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5unionz0zzBytesz9)(zz5listz8z5unionz0zzBytesz9 *rop, struct zBytes u) {
  *rop = NULL;
}

static void COPY(ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *rop, const struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 op1, struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 op2) {
  return EQUAL(bool)(op1.ztup0, op2.ztup0) && EQUAL(fixed_bytes_20)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *rop, const struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 op) {
  COPY(zz5listz8z5bv8z9)(&rop->ztup0, op.ztup0);
  rop->ztup1 = op.ztup1;
}

static void CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *op) {
  CREATE(zz5listz8z5bv8z9)(&op->ztup0);
}

static void RECREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *op) {
  RECREATE(zz5listz8z5bv8z9)(&op->ztup0);
}

static void KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *op) {
  KILL(zz5listz8z5bv8z9)(&op->ztup0);
}

static bool EQUAL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 op1, struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 op2) {
  return EQUAL(zz5listz8z5bv8z9)(op1.ztup0, op2.ztup0) && (op1.ztup1 == op2.ztup1);
}

static void COPY(ztuple_z8z5boolzCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 *rop, const struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
}

static bool EQUAL(ztuple_z8z5boolzCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 op1, struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 op2) {
  return EQUAL(bool)(op1.ztup0, op2.ztup0) && (op1.ztup1 == op2.ztup1) && (op1.ztup2 == op2.ztup2);
}

static void COPY(ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9)(struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 *rop, const struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9)(struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 op1, struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 op2) {
  return EQUAL(zRlpFieldRefFields)(op1.ztup0, op2.ztup0) && EQUAL(zRlpCursorFields)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *rop, const struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 op) {
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&rop->ztup0, op.ztup0);
  COPY(zz5listz8z5structz0zzStorageKeyz9)(&rop->ztup1, op.ztup1);
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
}

static void CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *op) {
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->ztup0);
  CREATE(zz5listz8z5structz0zzStorageKeyz9)(&op->ztup1);
}

static void RECREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *op) {
  RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->ztup0);
  RECREATE(zz5listz8z5structz0zzStorageKeyz9)(&op->ztup1);
}

static void KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *op) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&op->ztup0);
  KILL(zz5listz8z5structz0zzStorageKeyz9)(&op->ztup1);
}

static bool EQUAL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 op1, struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 op2) {
  return EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(op1.ztup0, op2.ztup0) && EQUAL(zz5listz8z5structz0zzStorageKeyz9)(op1.ztup1, op2.ztup1) && (op1.ztup2 == op2.ztup2) && (op1.ztup3 == op2.ztup3);
}

static void COPY(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *rop, const struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 op) {
  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&rop->ztup0, op.ztup0);
  rop->ztup1 = op.ztup1;
}

static void CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *op) {
  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->ztup0);
}

static void RECREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *op) {
  RECREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->ztup0);
}

static void KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *op) {
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&op->ztup0);
}

static bool EQUAL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 op1, struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 op2) {
  return EQUAL(zz5listz8z5structz0zzAuthorizzzzationz9)(op1.ztup0, op2.ztup0) && (op1.ztup1 == op2.ztup1);
}

static void COPY(ztuple_z8z5boolzCz0z5structz0zzTriePathz9)(struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 *rop, const struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5boolzCz0z5structz0zzTriePathz9)(struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 op1, struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 op2) {
  return EQUAL(bool)(op1.ztup0, op2.ztup0) && EQUAL(zTriePath)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9)(struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 *rop, const struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9)(struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 op1, struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 op2) {
  return EQUAL(zMemoryRangeFields)(op1.ztup0, op2.ztup0) && EQUAL(zMemoryRangeFields)(op1.ztup1, op2.ztup1);
}

static void CREATE(zz5vecz8z5bv16z9)(zz5vecz8z5bv16z9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5bv16z9)(zz5vecz8z5bv16z9 *rop) {
  if (rop->data != NULL) sail_free(rop->data);
}

static void fast_vector_init_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 *vec, const int64_t n, uint64_t elem) {
  KILL(zz5vecz8z5bv16z9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(uint64_t, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void fast_unsigned_vector_init_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 *vec, const uint64_t n, uint64_t elem) {
  KILL(zz5vecz8z5bv16z9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(uint64_t, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void RECREATE(zz5vecz8z5bv16z9)(zz5vecz8z5bv16z9 *rop) {
  KILL(zz5vecz8z5bv16z9)(rop);
  CREATE(zz5vecz8z5bv16z9)(rop);
}

static uint64_t fast_vector_access_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 op, int64_t n) {
  return op.data[n];
}

static uint64_t fast_unsigned_vector_access_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 op, uint64_t n) {
  return op.data[n];
}

static void COPY(zz5vecz8z5bv16z9)(zz5vecz8z5bv16z9 *rop, zz5vecz8z5bv16z9 op) {
  KILL(zz5vecz8z5bv16z9)(rop);
  rop->len = op.len;
  rop->data = sail_new_array(uint64_t, rop->len);
  for (int i = 0; i < op.len; i++) {
    (rop->data)[i] = op.data[i];
  }
}

static void fast_vector_update_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 *rop, zz5vecz8z5bv16z9 op, const int64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5bv16z9)(rop, op);
    rop->data[m] = elem;
  }
}

static void fast_unsigned_vector_update_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 *rop, zz5vecz8z5bv16z9 op, const uint64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5bv16z9)(rop, op);
    rop->data[m] = elem;
  }
}

static bool EQUAL(zz5vecz8z5bv16z9)(const zz5vecz8z5bv16z9 op1, const zz5vecz8z5bv16z9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;
  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(fbits)(op1.data[i], op2.data[i]);
  }
  return result;
}

static void internal_vector_update_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 *rop, zz5vecz8z5bv16z9 op, const int64_t n, uint64_t elem) {
  rop->data[n] = elem;
}

static void internal_vector_init_zz5vecz8z5bv16z9(zz5vecz8z5bv16z9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_new_array(uint64_t, len);
}

static void COPY(ztuple_z8z5u64zCz0z5u64z9)(struct ztuple_z8z5u64zCz0z5u64z9 *rop, const struct ztuple_z8z5u64zCz0z5u64z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5u64zCz0z5u64z9)(struct ztuple_z8z5u64zCz0z5u64z9 op1, struct ztuple_z8z5u64zCz0z5u64z9 op2) {
  return (op1.ztup0 == op2.ztup0) && (op1.ztup1 == op2.ztup1);
}

static void COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(struct ztuple_z8z5u64zCz0z5unionz0zzastz9 *rop, const struct ztuple_z8z5u64zCz0z5unionz0zzastz9 op) {
  rop->ztup0 = op.ztup0;
  COPY(zast)(&rop->ztup1, op.ztup1);
}

static void CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(struct ztuple_z8z5u64zCz0z5unionz0zzastz9 *op) {
  CREATE(zast)(&op->ztup1);
}

static void RECREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(struct ztuple_z8z5u64zCz0z5unionz0zzastz9 *op) {
  RECREATE(zast)(&op->ztup1);
}

static void KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(struct ztuple_z8z5u64zCz0z5unionz0zzastz9 *op) {
  KILL(zast)(&op->ztup1);
}

static bool EQUAL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(struct ztuple_z8z5u64zCz0z5unionz0zzastz9 op1, struct ztuple_z8z5u64zCz0z5unionz0zzastz9 op2) {
  return (op1.ztup0 == op2.ztup0) && EQUAL(zast)(op1.ztup1, op2.ztup1);
}

static void CREATE(zz5vecz8z5u64z9)(zz5vecz8z5u64z9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5u64z9)(zz5vecz8z5u64z9 *rop) {
  if (rop->data != NULL) sail_free(rop->data);
}

static void fast_vector_init_zz5vecz8z5u64z9(zz5vecz8z5u64z9 *vec, const int64_t n, uint64_t elem) {
  KILL(zz5vecz8z5u64z9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(uint64_t, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void fast_unsigned_vector_init_zz5vecz8z5u64z9(zz5vecz8z5u64z9 *vec, const uint64_t n, uint64_t elem) {
  KILL(zz5vecz8z5u64z9)(vec);
  size_t m = (size_t)n;
  vec->len = m;
  vec->data = sail_new_array(uint64_t, m);
  for (size_t i = 0; i < m; i++) {
    (vec->data)[i] = elem;
  }
}

static void RECREATE(zz5vecz8z5u64z9)(zz5vecz8z5u64z9 *rop) {
  KILL(zz5vecz8z5u64z9)(rop);
  CREATE(zz5vecz8z5u64z9)(rop);
}

static uint64_t fast_vector_access_zz5vecz8z5u64z9(zz5vecz8z5u64z9 op, int64_t n) {
  return op.data[n];
}

static uint64_t fast_unsigned_vector_access_zz5vecz8z5u64z9(zz5vecz8z5u64z9 op, uint64_t n) {
  return op.data[n];
}

static void COPY(zz5vecz8z5u64z9)(zz5vecz8z5u64z9 *rop, zz5vecz8z5u64z9 op) {
  KILL(zz5vecz8z5u64z9)(rop);
  rop->len = op.len;
  rop->data = sail_new_array(uint64_t, rop->len);
  for (int i = 0; i < op.len; i++) {
    (rop->data)[i] = op.data[i];
  }
}

static void fast_vector_update_zz5vecz8z5u64z9(zz5vecz8z5u64z9 *rop, zz5vecz8z5u64z9 op, const int64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5u64z9)(rop, op);
    rop->data[m] = elem;
  }
}

static void fast_unsigned_vector_update_zz5vecz8z5u64z9(zz5vecz8z5u64z9 *rop, zz5vecz8z5u64z9 op, const uint64_t n, uint64_t elem) {
  size_t m = (size_t)n;
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5u64z9)(rop, op);
    rop->data[m] = elem;
  }
}

static bool EQUAL(zz5vecz8z5u64z9)(const zz5vecz8z5u64z9 op1, const zz5vecz8z5u64z9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;
  for (int i = 0; i < op1.len; i++) {
    result &= (op1.data[i] == op2.data[i]);
  }
  return result;
}

static void internal_vector_update_zz5vecz8z5u64z9(zz5vecz8z5u64z9 *rop, zz5vecz8z5u64z9 op, const int64_t n, uint64_t elem) {
  rop->data[n] = elem;
}

static void internal_vector_init_zz5vecz8z5u64z9(zz5vecz8z5u64z9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_new_array(uint64_t, len);
}

static void COPY(ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9)(struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 *rop, const struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9)(struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 op1, struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 op2) {
  return EQUAL(u256)(op1.ztup0, op2.ztup0) && EQUAL(u256)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *rop, const struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 op) {
  COPY(zTrieUpdate)(&rop->ztup0, op.ztup0);
  rop->ztup1 = op.ztup1;
}

static void CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *op) {
  CREATE(zTrieUpdate)(&op->ztup0);
}

static void RECREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *op) {
  RECREATE(zTrieUpdate)(&op->ztup0);
}

static void KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *op) {
  KILL(zTrieUpdate)(&op->ztup0);
}

static bool EQUAL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 op1, struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 op2) {
  return EQUAL(zTrieUpdate)(op1.ztup0, op2.ztup0) && EQUAL(bool)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *rop, const struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 op) {
  COPY(zTrieBranchFrame)(&rop->ztup0, op.ztup0);
  COPY(zTrieBuilder)(&rop->ztup1, op.ztup1);
}

static void CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *op) {
  CREATE(zTrieBranchFrame)(&op->ztup0);
  CREATE(zTrieBuilder)(&op->ztup1);
}

static void RECREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *op) {
  RECREATE(zTrieBranchFrame)(&op->ztup0);
  RECREATE(zTrieBuilder)(&op->ztup1);
}

static void KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *op) {
  KILL(zTrieBranchFrame)(&op->ztup0);
  KILL(zTrieBuilder)(&op->ztup1);
}

static bool EQUAL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 op1, struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 op2) {
  return EQUAL(zTrieBranchFrame)(op1.ztup0, op2.ztup0) && EQUAL(zTrieBuilder)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *rop, const struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 op) {
  COPY(zRlpIndexItem)(&rop->ztup0, op.ztup0);
  rop->ztup1 = op.ztup1;
}

static void CREATE(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *op) {
  CREATE(zRlpIndexItem)(&op->ztup0);
}

static void RECREATE(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *op) {
  RECREATE(zRlpIndexItem)(&op->ztup0);
}

static void KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *op) {
  KILL(zRlpIndexItem)(&op->ztup0);
}

static bool EQUAL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 op1, struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 op2) {
  return EQUAL(zRlpIndexItem)(op1.ztup0, op2.ztup0) && EQUAL(zRlpIndexCursor)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *rop, const struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 op) {
  COPY(zTrieItemSink)(&rop->ztup0, op.ztup0);
  COPY(zTrieUpdateCursor)(&rop->ztup1, op.ztup1);
}

static void CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *op) {
  CREATE(zTrieItemSink)(&op->ztup0);
  CREATE(zTrieUpdateCursor)(&op->ztup1);
}

static void RECREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *op) {
  RECREATE(zTrieItemSink)(&op->ztup0);
  RECREATE(zTrieUpdateCursor)(&op->ztup1);
}

static void KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *op) {
  KILL(zTrieItemSink)(&op->ztup0);
  KILL(zTrieUpdateCursor)(&op->ztup1);
}

static bool EQUAL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 op1, struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 op2) {
  return EQUAL(zTrieItemSink)(op1.ztup0, op2.ztup0) && EQUAL(zTrieUpdateCursor)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9)(struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 *rop, const struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9)(struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 op1, struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 op2) {
  return EQUAL(zByteSliceFields)(op1.ztup0, op2.ztup0) && EQUAL(zBoundedSszzListCursor)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9)(struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 *rop, const struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9)(struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 op1, struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 op2) {
  return EQUAL(zByteSliceFields)(op1.ztup0, op2.ztup0) && EQUAL(zBoundedSszzListRef)(op1.ztup1, op2.ztup1);
}

static void COPY(ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9)(struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 *rop, const struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9)(struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 op1, struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 op2) {
  return EQUAL(zByteSliceFields)(op1.ztup0, op2.ztup0) && EQUAL(zSszzContainerCursor)(op1.ztup1, op2.ztup1);
}

static void CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *rop) { *rop = NULL; }

static void internal_inc_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 l) {
  if (l == NULL) return;
  l->rc += 1;
}

static void internal_dec_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 l) {
  if (l == NULL) return;
  l->rc -= 1;
}

static void KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *rop) {
  if (*rop == NULL) return;
  if ((*rop)->rc >= 1) {
    (*rop)->rc -= 1;
  }
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 node = *rop;
  while (node != NULL && node->rc == 0) {
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 next = node->tl;
    sail_free(node);
    node = next;
    internal_dec_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9(node);
  }
}

static void RECREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *rop) {
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(rop);
  *rop = NULL;
}

static void COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *rop, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 op) {
  internal_inc_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9(op);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(rop);
  *rop = op;
}

static void zconsz3z5structz0zz__sail_c_repr_fixed_bytesz832z9(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *rop, sail_fixed_bytes_32 x, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 xs) {
  bool same = *rop == xs;
  *rop = sail_new(struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9);
  (*rop)->rc = 1;
  (*rop)->hd = x;
  if (!same) internal_inc_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9(xs);
  (*rop)->tl = xs;
}

static sail_fixed_bytes_32 pick_fixed_bytes_32(const zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 xs) {
  return xs->hd;
}

static bool EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(const zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 op1, const zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 op2) {
  if (op1 == NULL && op2 == NULL) { return true; };
  if (op1 == NULL || op2 == NULL) { return false; };
  return EQUAL(fixed_bytes_32)(op1->hd, op2->hd) && EQUAL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(op1->tl, op2->tl);
}

static void UNDEFINED(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9)(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *rop, sail_fixed_bytes_32 u) {
  *rop = NULL;
}

struct zexception *current_exception = NULL;

bool have_exception = false;

sail_string *throw_location = NULL;

sail_u256 zZERO_WORD;


sail_fixed_bytes_20 zZERO_ADDRESS;


sail_fixed_bytes_32 zZERO_HASH;


sail_u256 zWORD_ZERO;


sail_u256 zWORD_ONE;


sail_u256 zWORD_ALL_ONES;


sail_u256 zWORD_SIGN_BIT;


uint64_t zSCOPE_STATELESS_VALIDATION;


uint64_t zSCOPE_DECODE_INPUT;


uint64_t zSCOPE_INDEX_WITNESS;


uint64_t zSCOPE_VALIDATE_PAYLOAD;


uint64_t zSCOPE_EXECUTE_BLOCK;


uint64_t zSCOPE_VALIDATE_RESULT;


uint64_t zSCOPE_COMPUTE_OUTPUT_ROOT;


uint64_t zSCOPE_SERIALIZE_OUTPUT;


uint64_t zSCOPE_BLOCK_START;


uint64_t zSCOPE_BLOCK_TRANSACTIONS;


uint64_t zSCOPE_TX_DECODE;


uint64_t zSCOPE_TX_RESET;


uint64_t zSCOPE_TX_VALIDATE;


uint64_t zSCOPE_TX_UPFRONT;


uint64_t zSCOPE_TX_FRAME;


uint64_t zSCOPE_TX_SETTLE;


uint64_t zSCOPE_RECEIPTS_ROOT;


uint64_t zSCOPE_BLOCK_END_STATE;


uint64_t zSCOPE_BLOCK_END_REQUESTS;


uint64_t zSCOPE_STATE_ROOT;


uint64_t zSCOPE_BLOCK_ACCESS_LIST;


uint64_t zSCOPE_HTR_EXECUTION_PAYLOAD;


uint64_t zSCOPE_HTR_TRANSACTIONS;


uint64_t zSCOPE_HTR_WITHDRAWALS;


uint64_t zSCOPE_HTR_VERSIONED_HASHES;


uint64_t zSCOPE_HTR_EXECUTION_REQUESTS;


uint64_t zSCOPE_HTR_BYTES_ROOT;


uint64_t zSCOPE_HTR_MERKLE_PADDING;


struct zMemoryRangeFields zEMPTY_MEMORY_RANGE;


uint64_t zGAS_COST_ZERO;


uint64_t zTRANSACTION_EXECUTION_GAS_LIMIT;


uint64_t zSTATE_GAS_SPILL_ZERO;


uint64_t zSTATE_GAS_SPILL_LIMIT;


uint64_t zGAS_ZERO;


uint64_t zSYSTEM_CALL_GAS_LIMIT;


uint64_t zGAS_CONSTANT_ZERO;


__int128 zGAS_REFUND_ZERO;


__int128 zFRAME_STATE_GAS_DELTA_ZERO;


__int128 zSTATE_GAS_DELTA_ZERO;


uint64_t zADDRESS_BYTE_LENGTH;


uint64_t zWORD_BYTE_LENGTH;


uint64_t zEIGHT_BYTE_LENGTH;


uint64_t zDOUBLE_WORD_BYTE_LENGTH;


struct zByteSliceFields zEMPTY_SLICE;


sail_u256 zEMPTY_JUMPDEST_CHUNK;


uint64_t zEMPTY_JUMPDEST_REF;


struct zByteSliceFields zEMPTY_CODE_SLICE;


struct zCode zEMPTY_CODE;


sail_fixed_bytes_32 zKECCAK_EMPTY;


sail_fixed_bytes_32 zEMPTY_TRIE_ROOT;


sail_u256 zSECP_N_FULL;


sail_u256 zSECP_N_HALF;


struct zBlobSchedule zNO_BLOB_SCHEDULE;


sail_fixed_bytes_20 zSYSTEM_ADDRESS;


sail_fixed_bytes_20 zBEACON_ROOTS_ADDR;


sail_fixed_bytes_20 zHISTORY_STORAGE_ADDR;


sail_fixed_bytes_20 zWITHDRAWAL_REQUEST_ADDR;


sail_fixed_bytes_20 zCONSOLIDATION_REQUEST_ADDR;


sail_fixed_bytes_20 zBUILDER_DEPOSIT_REQUEST_ADDR;


sail_fixed_bytes_20 zBUILDER_EXIT_REQUEST_ADDR;


sail_fixed_bytes_20 zDEPOSIT_CONTRACT_ADDR;


sail_u256 zDEPOSIT_EVENT_TOPIC;


sail_fixed_bytes_20 zEIP7708_SYSTEM_ADDRESS;


sail_u256 zEIP7708_TRANSFER_TOPIC;


sail_u256 zEIP7708_BURN_TOPIC;


struct zAccountInfo zEMPTY_ACCOUNT_INFO;


struct zAccount zEMPTY_ACCOUNT;


uint64_t zOSAKA_TRANSACTION_GAS_LIMIT_VALUE;


uint64_t zOSAKA_TRANSACTION_GAS_LIMIT;


struct zBlobHashes zEMPTY_BLOB_HASHES;


sail_fixed_bytes_256 zEMPTY_LOGS_BLOOM;


sail_fixed_bytes_32 zEMPTY_OMMER_HASH;


struct zExecutionRequests zEMPTY_EXECUTION_REQUESTS;


struct zMessage zDEFAULT_MESSAGE;


struct zFrameCheckpoint zDEFAULT_FRAME_CHECKPOINT;


struct zFrameContinuation zDEFAULT_FRAME_CONTINUATION;


// register zscratch_arena
struct zByteSliceFields zscratch_arena;

uint64_t zSSZ_OFF_BYTES;


uint64_t zSSZ_UINT_BYTES;


uint64_t zRLP_SHORT_LENGTH_LIMIT;


uint64_t zRLP_WORD_LENGTH_LIMIT;


uint64_t zRLP_UINT64_LENGTH_LIMIT;


uint64_t zRLP_ENCODED_WORD_LENGTH;


uint64_t zRLP_ENCODED_ADDRESS_LENGTH;


uint64_t zLEGACY_SIGNATURE_SUFFIX_LENGTH;


uint64_t zPUBLIC_KEY_BODY_LENGTH;


struct zAccessListDecode zEMPTY_ACCESS_LIST_DECODE;


uint64_t zBLOB_HASH_RLP_LENGTH;


uint64_t zBLOB_HASH_LENGTH;


struct zAuthorizzationDecode zEMPTY_AUTHORIZATION_DECODE;


// register zk_parent_state_root
sail_fixed_bytes_32 zk_parent_state_root;

// register zk_n_headers
uint64_t zk_n_headers;

// register zk_chain_id
uint64_t zk_chain_id;

// register zk_fork
enum zFork zk_fork;

// register zk_blob_schedule
struct zBlobSchedule zk_blob_schedule;

// register zk_header
struct zBlockHeader zk_header;

// register zk_tx
struct zTxEnv zk_tx;

uint64_t zHEX_PREFIX_MAX_LENGTH;


uint64_t zMPT_HASH_LENGTH;


// register zpc
uint64_t zpc;

// register zgas_remaining
uint64_t zgas_remaining;

// register zstate_gas_remaining
uint64_t zstate_gas_remaining;

// register zstate_gas_spilled
uint64_t zstate_gas_spilled;

// register zframe_refund
__int128 zframe_refund;

// register zframe_status
struct zFrameStatus zframe_status;

// register zmessage
struct zMessage zmessage;

// register zcall_depth
uint64_t zcall_depth;

uint64_t zDEPTH_LIMIT;


// register zframe_stack
zz5vecz8z5unionz0zzFrameContinuationz9 zframe_stack;

// register zframe_stack_top
uint64_t zframe_stack_top;

// register zframe_code
struct zCode zframe_code;

uint64_t zSTACK_LIMIT;


// register zcalldata
struct zByteSliceFields zcalldata;

// register zreturndata
struct zByteSliceFields zreturndata;

// register zevm_memory
struct zByteSliceFields zevm_memory;

sail_u256 zMIN_BLOB_BASE_FEE;


uint64_t zGAS_PER_BLOB;


sail_u256 zBLOB_RESERVE_PRICE_SHIFT_LIMIT;


uint64_t zG_zzero;


uint64_t zG_jumpdest;


uint64_t zG_base;


uint64_t zG_verylow;


uint64_t zG_low;


uint64_t zG_mid;


uint64_t zG_high;


uint64_t zG_warm_access;


uint64_t zG_cold_sload;


uint64_t zG_cold_account;


uint64_t zG_keccak;


uint64_t zG_keccak_word;


uint64_t zG_copy_word;


uint64_t zG_memory;


uint64_t zG_log;


uint64_t zG_logtopic;


uint64_t zG_logdata;


uint64_t zG_exp;


uint64_t zG_expbyte;


uint64_t zG_sset;


uint64_t zG_sreset;


uint64_t zR_sclear_pre_london;


uint64_t zR_sclear_london;


uint64_t zR_selfdestruct_pre_london;


uint64_t zG_create;


uint64_t zG_codedeposit;


uint64_t zG_callvalue;


uint64_t zG_callstipend;


uint64_t zG_newaccount;


uint64_t zG_selfdestruct;


uint64_t zG_initcode_word;


uint64_t zG_amsterdam_cold_account_access;


uint64_t zG_amsterdam_cold_storage_access;


uint64_t zG_amsterdam_storage_write;


uint64_t zG_amsterdam_account_write;


uint64_t zG_amsterdam_call_value;


uint64_t zG_amsterdam_create_access;


uint64_t zG_amsterdam_state_byte;


uint64_t zG_amsterdam_state_storage_set;


uint64_t zG_amsterdam_state_new_account;


uint64_t zG_amsterdam_state_auth_base;


uint64_t zR_amsterdam_storage_clear;


uint64_t zG_sstore_sentry;


uint64_t zEIP170_DEPLOYED_CODE_SIZE_LIMIT;


uint64_t zEIP3860_INITCODE_SIZE_LIMIT;


uint64_t zEIP7954_DEPLOYED_CODE_SIZE_LIMIT;


uint64_t zEIP7954_INITCODE_SIZE_LIMIT;


zz5vecz8z5bv16z9 zBLS_G1_DISCOUNT;


zz5vecz8z5bv16z9 zBLS_G2_DISCOUNT;


uint64_t zACCELERATOR_INPUT_MAX;


sail_u256 zFIELD_ELEMENTS_PER_BLOB;


sail_u256 zBLS_MODULUS;


uint64_t zBLAKE2F_INPUT_LENGTH;


uint64_t zBLAKE2F_FINAL_BLOCK_OFFSET;


uint64_t zBLAKE2F_OUTPUT_LENGTH;


uint64_t zKZG_INPUT_LENGTH;


uint64_t zKZG_COMMITMENT_OFFSET;


uint64_t zKZG_COMMITMENT_LENGTH;


uint64_t zBLS_FIELD_PADDING_LENGTH;


uint64_t zBLS_PADDED_FIELD_LENGTH;


uint64_t zBLS_G1_POINT_LENGTH;


uint64_t zBLS_G2_POINT_LENGTH;


uint64_t zBLS_G2_FINAL_FIELD_OFFSET;


uint64_t zBLS_G1_ADD_INPUT_LENGTH;


uint64_t zBLS_G1_MSM_ITEM_LENGTH;


uint64_t zBLS_G2_ADD_INPUT_LENGTH;


uint64_t zBLS_G2_MSM_ITEM_LENGTH;


uint64_t zBLS_PAIRING_ITEM_LENGTH;


uint64_t zP256_INPUT_LENGTH;


uint64_t zBN254_PAIRING_ITEM_LENGTH;


uint64_t zPRECOMPILE_WORD_LENGTH;


uint64_t zPRECOMPILE_DOUBLE_WORD_LENGTH;


uint64_t zPRECOMPILE_WORD_OFFSET;


uint64_t zPRECOMPILE_DOUBLE_WORD_OFFSET;


uint64_t zECRECOVER_S_OFFSET;


uint64_t zTWO_COMPONENTS;


uint64_t zBLS_G2_POINT_OFFSET;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_1;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_2;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_3;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_4;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_5;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_6;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_7;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_8;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_9;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_10;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_11;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_12;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_13;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_14;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_15;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_16;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_17;


sail_fixed_bytes_20 zPRECOMPILE_ADDRESS_256;


uint64_t zG_transaction;


uint64_t zG_txcreate;


uint64_t zG_txdatazzero;


uint64_t zG_txdatanonzzero;


uint64_t zG_access_list_address;


uint64_t zG_access_list_storage_key;


uint64_t zPER_AUTH_BASE;


uint64_t zPER_EMPTY_ACCOUNT;


uint64_t zAMSTERDAM_TX_BASE;


uint64_t zAMSTERDAM_CREATE_ACCESS;


uint64_t zAMSTERDAM_COLD_ACCOUNT_ACCESS;


uint64_t zAMSTERDAM_TX_VALUE_COST;


uint64_t zAMSTERDAM_TRANSFER_LOG_COST;


uint64_t zAMSTERDAM_ACCESS_LIST_ADDRESS;


uint64_t zAMSTERDAM_ACCESS_LIST_SLOT;


uint64_t zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR;


uint64_t zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR;


uint64_t zAMSTERDAM_AUTH_BASE;


uint64_t zAMSTERDAM_CALLDATA_FLOOR_BYTE;


uint64_t zAMSTERDAM_TX_MAX_GAS;


struct zAmsterdamAuthorizzationState zEMPTY_AMSTERDAM_AUTHORIZATION_STATE;


uint64_t zSSZ_BODY;


uint64_t zIN_NPR_OFF;


uint64_t zIN_WITNESS_OFF;


uint64_t zIN_CHAIN_CONFIG_OFF;


uint64_t zIN_PUBLIC_KEYS_OFF;


uint64_t zNPR_PAYLOAD_OFF;


uint64_t zNPR_VHASHES_OFF;


uint64_t zNPR_BEACON_ROOT;


uint64_t zNPR_REQUESTS_OFF;


uint64_t zREQ_DEPOSITS_OFF;


uint64_t zREQ_WITHDRAWALS_OFF;


uint64_t zREQ_CONSOLIDATIONS_OFF;


uint64_t zREQ_BUILDER_DEPOSITS_OFF;


uint64_t zREQ_BUILDER_EXITS_OFF;


uint64_t zPL_FEE_RECIPIENT;


uint64_t zPL_STATE_ROOT;


uint64_t zPL_RECEIPTS_ROOT;


uint64_t zPL_LOGS_BLOOM;


uint64_t zPL_PREV_RANDAO;


uint64_t zPL_BLOCK_NUMBER;


uint64_t zPL_GAS_LIMIT;


uint64_t zPL_GAS_USED;


uint64_t zPL_TIMESTAMP;


uint64_t zPL_EXTRA_OFF;


uint64_t zPL_BASE_FEE;


uint64_t zPL_BLOCK_HASH;


uint64_t zPL_TXS_OFF;


uint64_t zPL_WDS_OFF;


uint64_t zPL_BLOB_GAS_USED;


uint64_t zPL_EXCESS_BLOB_GAS;


uint64_t zPL_BAL_OFF;


uint64_t zPL_SLOT_NUMBER;


uint64_t zWD_SIZE;


uint64_t zWD_INDEX;


uint64_t zWD_VALIDATOR_INDEX;


uint64_t zWD_ADDRESS;


uint64_t zWD_AMOUNT;


uint64_t zCC_CHAIN_ID;


uint64_t zCC_ACTIVE_FORK_OFF;


uint64_t zFC_ACTIVATION_OFF;


uint64_t zFA_BLOCK_NUMBER_OFF;


uint64_t zFA_TIMESTAMP_OFF;


uint64_t zWIT_STATE_OFF;


uint64_t zWIT_CODES_OFF;


uint64_t zWIT_HEADERS_OFF;


uint64_t zSTATELESS_INPUT_FIXED_LENGTH;


uint64_t zSTATELESS_INPUT_BODY_FIXED_LENGTH;


uint64_t zNEW_PAYLOAD_REQUEST_FIXED_LENGTH;


uint64_t zEXECUTION_PAYLOAD_FIXED_LENGTH;


uint64_t zEXECUTION_REQUESTS_FIXED_LENGTH;


uint64_t zEXECUTION_WITNESS_FIXED_LENGTH;


uint64_t zPUBLIC_KEY_LENGTH;


uint64_t zCHAIN_CONFIG_HEADER_LENGTH;


uint64_t zCHAIN_CONFIG_MIN_LENGTH;


uint64_t zMAX_EXTRA_DATA_LENGTH;


uint64_t zMAX_TRANSACTION_LENGTH;


uint64_t zMAX_TRANSACTIONS_PER_PAYLOAD;


uint64_t zMAX_WITHDRAWALS_PER_PAYLOAD;


uint64_t zMAX_BLOCK_ACCESS_LIST_LENGTH;


uint64_t zMAX_BLOB_COMMITMENTS_PER_BLOCK;


uint64_t zMAX_WITNESS_NODES;


uint64_t zMAX_WITNESS_NODE_LENGTH;


uint64_t zMAX_WITNESS_CODES;


uint64_t zMAX_WITNESS_CODE_LENGTH;


uint64_t zMAX_WITNESS_HEADERS;


uint64_t zMAX_WITNESS_HEADER_LENGTH;


uint64_t zMAX_PUBLIC_KEYS;


struct zParentHeaderFields zEMPTY_PARENT_HEADER_FIELDS;


uint64_t zLOGS_BLOOM_BYTE_LENGTH;


uint64_t zSYSTEM_CALL_INPUT_LENGTH;


uint64_t zDEPOSIT_EVENT_DATA_LENGTH;


uint64_t zDEPOSIT_PUBKEY_HEAD;


uint64_t zDEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD;


uint64_t zDEPOSIT_AMOUNT_HEAD;


uint64_t zDEPOSIT_SIGNATURE_HEAD;


uint64_t zDEPOSIT_INDEX_HEAD;


uint64_t zDEPOSIT_PUBKEY_LENGTH_WORD;


uint64_t zDEPOSIT_PUBKEY_DATA;


uint64_t zDEPOSIT_PUBKEY_LENGTH;


uint64_t zDEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD;


uint64_t zDEPOSIT_WITHDRAWAL_CREDENTIALS_DATA;


uint64_t zDEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH;


uint64_t zDEPOSIT_AMOUNT_LENGTH_WORD;


uint64_t zDEPOSIT_AMOUNT_DATA;


uint64_t zDEPOSIT_AMOUNT_LENGTH;


uint64_t zDEPOSIT_SIGNATURE_LENGTH_WORD;


uint64_t zDEPOSIT_SIGNATURE_DATA;


uint64_t zDEPOSIT_SIGNATURE_LENGTH;


uint64_t zDEPOSIT_INDEX_LENGTH_WORD;


uint64_t zDEPOSIT_INDEX_DATA;


uint64_t zDEPOSIT_INDEX_LENGTH;


uint64_t zBAL_RLP_ZERO;


uint64_t zPRE_MERGE_BLOCK_REWARD;


uint64_t zHTR_BYTE_LIST_LIMIT;


uint64_t zHTR_DEPOSIT_LENGTH;


uint64_t zHTR_DEPOSIT_PUBKEY;


uint64_t zHTR_DEPOSIT_PUBKEY_LENGTH;


uint64_t zHTR_DEPOSIT_WITHDRAWAL_CREDENTIALS;


uint64_t zHTR_DEPOSIT_AMOUNT;


uint64_t zHTR_DEPOSIT_SIGNATURE;


uint64_t zHTR_DEPOSIT_SIGNATURE_LENGTH;


uint64_t zHTR_DEPOSIT_INDEX;


uint64_t zHTR_WITHDRAWAL_REQUEST_LENGTH;


uint64_t zHTR_REQUEST_SOURCE_ADDRESS;


uint64_t zHTR_REQUEST_SOURCE_PUBKEY;


uint64_t zHTR_REQUEST_PUBKEY_LENGTH;


uint64_t zHTR_WITHDRAWAL_REQUEST_AMOUNT;


uint64_t zHTR_CONSOLIDATION_REQUEST_LENGTH;


uint64_t zHTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY;


uint64_t zHTR_BUILDER_DEPOSIT_LENGTH;


uint64_t zHTR_BUILDER_DEPOSIT_PUBKEY;


uint64_t zHTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS;


uint64_t zHTR_BUILDER_DEPOSIT_AMOUNT;


uint64_t zHTR_BUILDER_DEPOSIT_SIGNATURE;


uint64_t zHTR_BUILDER_EXIT_LENGTH;


uint64_t zHTR_BUILDER_EXIT_SOURCE_ADDRESS;


uint64_t zHTR_BUILDER_EXIT_PUBKEY;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_01;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_02;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_03;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_04;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_05;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_06;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_07;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_08;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_09;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_10;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_11;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_12;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_13;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_14;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_15;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_16;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_17;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_18;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_19;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_20;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_21;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_22;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_23;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_24;


sail_fixed_bytes_32 zSSZ_ZERO_HASH_25;


uint64_t zRESULT_METADATA_LENGTH;


static const size_t SAIL_TEST_COUNT = 0;
static unit (*const SAIL_TESTS[1])(unit) = {
  NULL
};
static const char* const SAIL_TEST_NAMES[1] = {
  NULL
};

