/* GMP-free runtime for the EVM model's generated C.
 *
 * bits(256) is an inline four-limb value.  Sail int/nat retains its exact
 * mathematical semantics through a sign-magnitude value with twelve 64-bit
 * limbs.  The model-wide bound ledger puts the largest runtime intermediate
 * below 2^765, so 768 bits is sufficient.  Every operation traps on a bound
 * violation: this implementation never silently wraps or saturates an int. */

#include "sail.h"

#include <limits.h>
#include <string.h>

typedef sail_int_struct integer;

enum { INTEGER_LIMBS = SAIL_INT_LIMBS };

static __attribute__((noreturn)) void integer_failure(void) { abort(); }

unit undefined_unit(const unit value) {
  (void)value;
  return UNIT;
}

bool eq_unit(const unit lhs, const unit rhs) {
  (void)lhs;
  (void)rhs;
  return true;
}

unit skip(const unit value) {
  (void)value;
  return UNIT;
}

bool eq_bool(const bool lhs, const bool rhs) { return lhs == rhs; }

bool undefined_bool(const unit value) {
  (void)value;
  return false;
}

bool eq_mach_int(const mach_int lhs, const mach_int rhs) {
  return lhs == rhs;
}

bool eq_mach_uint(const mach_uint lhs, const mach_uint rhs) {
  return lhs == rhs;
}

bool eq_bit(const fbits lhs, const fbits rhs) { return lhs == rhs; }

bool eq_fbits(const fbits lhs, const fbits rhs) { return lhs == rhs; }

bool eq_ref_fbits(const fbits *lhs, const fbits *rhs) {
  return *lhs == *rhs;
}

fbits undefined_fbits(const unit value) {
  (void)value;
  return UINT64_C(0);
}

static int magnitude_normalize(const uint64_t *limbs, int length) {
  while (length > 0 && limbs[length - 1] == 0) length--;
  return length;
}

static int magnitude_compare(const uint64_t *lhs, int lhs_length,
                             const uint64_t *rhs, int rhs_length) {
  if (lhs_length != rhs_length) return lhs_length < rhs_length ? -1 : 1;
  for (int i = lhs_length - 1; i >= 0; i--) {
    if (lhs[i] != rhs[i]) return lhs[i] < rhs[i] ? -1 : 1;
  }
  return 0;
}

static int magnitude_add(uint64_t *result, const uint64_t *lhs, int lhs_length,
                         const uint64_t *rhs, int rhs_length) {
  int length = lhs_length > rhs_length ? lhs_length : rhs_length;
  unsigned __int128 carry = 0;
  for (int i = 0; i < length; i++) {
    unsigned __int128 sum =
        (unsigned __int128)(i < lhs_length ? lhs[i] : 0) +
        (i < rhs_length ? rhs[i] : 0) + carry;
    result[i] = (uint64_t)sum;
    carry = sum >> 64;
  }
  if (carry != 0) {
    if (length == INTEGER_LIMBS) integer_failure();
    result[length++] = (uint64_t)carry;
  }
  return magnitude_normalize(result, length);
}

/* Requires lhs >= rhs.  result may alias either operand. */
static int magnitude_subtract(uint64_t *result, const uint64_t *lhs,
                              int lhs_length, const uint64_t *rhs,
                              int rhs_length) {
  unsigned __int128 borrow = 0;
  for (int i = 0; i < lhs_length; i++) {
    unsigned __int128 difference =
        (unsigned __int128)lhs[i] -
        (i < rhs_length ? rhs[i] : 0) - borrow;
    result[i] = (uint64_t)difference;
    borrow = (difference >> 64) & 1;
  }
  if (borrow != 0) integer_failure();
  return magnitude_normalize(result, lhs_length);
}

/* Compute the full product first, then reject a non-zero upper half.  This is
 * the same fixed-limb schoolbook shape used by monomorphic uint libraries and
 * makes truncation impossible to hide in the carry path. */
static int magnitude_multiply(uint64_t *result, const uint64_t *lhs,
                              int lhs_length, const uint64_t *rhs,
                              int rhs_length) {
  uint64_t product[INTEGER_LIMBS * 2] = {0};
  for (int i = 0; i < lhs_length; i++) {
    unsigned __int128 carry = 0;
    for (int j = 0; j < rhs_length; j++) {
      int index = i + j;
      unsigned __int128 value = (unsigned __int128)lhs[i] * rhs[j] +
                                product[index] + carry;
      product[index] = (uint64_t)value;
      carry = value >> 64;
    }
    int index = i + rhs_length;
    while (carry != 0) {
      if (index >= INTEGER_LIMBS * 2) integer_failure();
      unsigned __int128 value = (unsigned __int128)product[index] + carry;
      product[index++] = (uint64_t)value;
      carry = value >> 64;
    }
  }
  for (int i = INTEGER_LIMBS; i < INTEGER_LIMBS * 2; i++) {
    if (product[i] != 0) integer_failure();
  }
  for (int i = 0; i < INTEGER_LIMBS; i++) result[i] = product[i];
  return magnitude_normalize(result, INTEGER_LIMBS);
}

static void magnitude_full(uint64_t *result, const uint64_t *value,
                           int length) {
  for (int i = 0; i < INTEGER_LIMBS; i++) {
    result[i] = i < length ? value[i] : 0;
  }
}

static int magnitude_full_compare(const uint64_t *lhs, const uint64_t *rhs) {
  for (int i = INTEGER_LIMBS - 1; i >= 0; i--) {
    if (lhs[i] != rhs[i]) return lhs[i] < rhs[i] ? -1 : 1;
  }
  return 0;
}

/* Full-width subtraction modulo 2^768.  In division, an incoming carry is an
 * implicit 769th bit, so the wrapped low half is the exact post-subtraction
 * remainder. */
static void magnitude_full_subtract(uint64_t *result, const uint64_t *lhs,
                                    const uint64_t *rhs) {
  unsigned __int128 borrow = 0;
  for (int i = 0; i < INTEGER_LIMBS; i++) {
    unsigned __int128 difference =
        (unsigned __int128)lhs[i] - rhs[i] - borrow;
    result[i] = (uint64_t)difference;
    borrow = (difference >> 64) & 1;
  }
}

/* Divide (high << 64 | low) by a 64-bit divisor without relying on the
 * compiler's 128-bit division runtime.  The caller maintains high < divisor,
 * so both the quotient and remainder fit in one limb. */
static uint64_t magnitude_divide_limb(uint64_t high, uint64_t low,
                                      uint64_t divisor,
                                      uint64_t *remainder) {
  uint64_t quotient = 0;
  uint64_t rem = high;
  for (int bit = 63; bit >= 0; bit--) {
    bool carry = (rem >> 63) != 0;
    rem = (rem << 1) | ((low >> bit) & UINT64_C(1));
    if (carry || rem >= divisor) {
      rem -= divisor;
      quotient |= UINT64_C(1) << bit;
    }
  }
  *remainder = rem;
  return quotient;
}

static void magnitude_divide(uint64_t *quotient, int *quotient_length,
                             uint64_t *remainder, int *remainder_length,
                             const uint64_t *dividend, int dividend_length,
                             const uint64_t *divisor, int divisor_length) {
  if (divisor_length == 0) integer_failure();
  for (int i = 0; i < INTEGER_LIMBS; i++) {
    quotient[i] = 0;
    remainder[i] = 0;
  }
  if (dividend_length == 0) {
    *quotient_length = 0;
    *remainder_length = 0;
    return;
  }

  if (divisor_length == 1) {
    uint64_t carry = 0;
    for (int i = dividend_length - 1; i >= 0; i--) {
      quotient[i] = magnitude_divide_limb(
          carry, dividend[i], divisor[0], &carry);
    }
    if (carry != 0) remainder[0] = carry;
    *quotient_length = magnitude_normalize(quotient, dividend_length);
    *remainder_length = carry == 0 ? 0 : 1;
    return;
  }

  uint64_t divisor_full[INTEGER_LIMBS];
  magnitude_full(divisor_full, divisor, divisor_length);
  int top_bit = dividend_length * 64 - 1;
  while (top_bit > 0 &&
         ((dividend[top_bit >> 6] >> (top_bit & 63)) & 1) == 0) {
    top_bit--;
  }
  for (int bit = top_bit; bit >= 0; bit--) {
    uint64_t carry = 0;
    for (int i = 0; i < INTEGER_LIMBS; i++) {
      uint64_t next = remainder[i] >> 63;
      remainder[i] = (remainder[i] << 1) | carry;
      carry = next;
    }
    remainder[0] |= (dividend[bit >> 6] >> (bit & 63)) & 1;
    if (carry != 0 ||
        magnitude_full_compare(remainder, divisor_full) >= 0) {
      magnitude_full_subtract(remainder, remainder, divisor_full);
      quotient[bit >> 6] |= UINT64_C(1) << (bit & 63);
    }
  }
  *quotient_length = magnitude_normalize(quotient, INTEGER_LIMBS);
  *remainder_length = magnitude_normalize(remainder, INTEGER_LIMBS);
}

static void integer_zero(integer *value) {
  value->negative = 0;
  value->length = 0;
}

static void integer_one(integer *value) {
  value->negative = 0;
  value->length = 1;
  value->limbs[0] = 1;
}

static uint64_t integer_small_unsigned(const sail_int value) {
  if (value->negative || value->length > 1) integer_failure();
  return value->length == 0 ? UINT64_C(0) : value->limbs[0];
}

static int integer_compare(const sail_int lhs, const sail_int rhs) {
  if (lhs->negative != rhs->negative) return lhs->negative ? -1 : 1;
  int comparison = magnitude_compare(lhs->limbs, lhs->length,
                                     rhs->limbs, rhs->length);
  return lhs->negative ? -comparison : comparison;
}

void create_sail_int(sail_int *result) { integer_zero(*result); }
void recreate_sail_int(sail_int *result) { integer_zero(*result); }
void kill_sail_int(sail_int *result) { (void)result; }
void copy_sail_int(sail_int *result, const sail_int value) {
  (*result)->negative = value->negative;
  (*result)->length = value->length;
  for (int i = 0; i < value->length; i++) {
    (*result)->limbs[i] = value->limbs[i];
  }
}

uint64_t sail_int_get_ui(const sail_int value) { return mpz_get_ui(value); }

void sail_int_to_u64_array(uint64_t *limbs, size_t limb_count,
                           const sail_int value) {
  for (size_t i = 0; i < limb_count; i++) {
    uint64_t limb = i < value->length ? value->limbs[i] : UINT64_C(0);
    limbs[i] = value->negative ? ~limb : limb;
  }
  if (value->negative) {
    uint64_t carry = UINT64_C(1);
    for (size_t i = 0; i < limb_count && carry != 0; i++) {
      uint64_t before = limbs[i];
      limbs[i] = before + carry;
      carry = limbs[i] < before;
    }
  }
}

void sail_int_from_u64_array(sail_int *result, const uint64_t *limbs,
                             size_t limb_count) {
  if (limb_count > SAIL_INT_LIMBS) integer_failure();
  while (limb_count != 0 && limbs[limb_count - 1] == 0) limb_count--;
  integer *out = *result;
  out->negative = 0;
  out->length = (uint8_t)limb_count;
  for (size_t i = 0; i < limb_count; i++) out->limbs[i] = limbs[i];
}

void sail_int_from_twos_complement_u64_array(sail_int *result,
                                             const uint64_t *limbs,
                                             size_t limb_count) {
  if (limb_count > SAIL_INT_LIMBS) integer_failure();
  if (limb_count == 0 || (limbs[limb_count - 1] >> 63) == 0) {
    sail_int_from_u64_array(result, limbs, limb_count);
    return;
  }

  integer *out = *result;
  uint64_t carry = UINT64_C(1);
  for (size_t i = 0; i < limb_count; i++) {
    uint64_t complemented = ~limbs[i];
    out->limbs[i] = complemented + carry;
    carry = out->limbs[i] < complemented;
  }
  while (limb_count != 0 && out->limbs[limb_count - 1] == 0) limb_count--;
  out->length = (uint8_t)limb_count;
  out->negative = limb_count == 0 ? 0 : 1;
}

mach_uint convert_mach_uint_of_mach_int(const mach_int value) {
  if (value < 0) integer_failure();
  return (mach_uint)value;
}

mach_int convert_mach_int_of_mach_uint(const mach_uint value) {
  if (value > INT64_MAX) integer_failure();
  return (mach_int)value;
}

mach_uint convert_mach_uint_of_sail_int(const sail_int value) {
  if (value->negative || value->length > 1) integer_failure();
  return value->length == 0 ? UINT64_C(0) : value->limbs[0];
}

void convert_sail_int_of_mach_uint(sail_int *result, const mach_uint value) {
  integer *out = *result;
  out->negative = 0;
  out->length = value == 0 ? 0 : 1;
  if (value != 0) out->limbs[0] = value;
}

mach_int convert_mach_int_of_sail_int(const sail_int value) {
  if (value->length > 1) integer_failure();
  uint64_t magnitude = value->length == 0 ? 0 : value->limbs[0];
  if (!value->negative) {
    if (magnitude > INT64_MAX) integer_failure();
    return (mach_int)magnitude;
  }
  if (magnitude > (UINT64_C(1) << 63)) integer_failure();
  if (magnitude == (UINT64_C(1) << 63)) return INT64_MIN;
  return -(mach_int)magnitude;
}

void convert_sail_int_of_mach_int(sail_int *result, const mach_int value) {
  integer *out = *result;
  if (value == 0) {
    integer_zero(out);
    return;
  }
  out->negative = value < 0;
  out->length = 1;
  out->limbs[0] = value < 0
      ? (uint64_t)(-(value + 1)) + UINT64_C(1)
      : (uint64_t)value;
}

void create_sail_int_of_mach_int(sail_int *result, const mach_int value) {
  convert_sail_int_of_mach_int(result, value);
}
void recreate_sail_int_of_mach_int(sail_int *result, const mach_int value) {
  convert_sail_int_of_mach_int(result, value);
}
mach_int create_mach_int_of_sail_int(const sail_int value) {
  return convert_mach_int_of_sail_int(value);
}

void add_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  integer *out = *result;
  if (lhs->negative == rhs->negative) {
    out->length = magnitude_add(out->limbs, lhs->limbs, lhs->length,
                                rhs->limbs, rhs->length);
    out->negative = out->length == 0 ? 0 : lhs->negative;
    return;
  }
  int comparison = magnitude_compare(lhs->limbs, lhs->length,
                                     rhs->limbs, rhs->length);
  if (comparison == 0) {
    integer_zero(out);
  } else if (comparison > 0) {
    out->length = magnitude_subtract(out->limbs, lhs->limbs, lhs->length,
                                     rhs->limbs, rhs->length);
    out->negative = lhs->negative;
  } else {
    out->length = magnitude_subtract(out->limbs, rhs->limbs, rhs->length,
                                     lhs->limbs, lhs->length);
    out->negative = rhs->negative;
  }
}

void neg_int(sail_int *result, const sail_int value) {
  copy_sail_int(result, value);
  if ((*result)->length != 0) (*result)->negative = !(*result)->negative;
}

void abs_int(sail_int *result, const sail_int value) {
  copy_sail_int(result, value);
  (*result)->negative = 0;
}

void sub_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  sail_int negative_rhs;
  copy_sail_int(&negative_rhs, rhs);
  if (negative_rhs->length != 0) negative_rhs->negative = !negative_rhs->negative;
  add_int(result, lhs, negative_rhs);
}

void sub_nat(sail_int *result, const sail_int lhs, const sail_int rhs) {
  sub_int(result, lhs, rhs);
  if ((*result)->negative) integer_zero(*result);
}

void mult_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  integer *out = *result;
  out->length = magnitude_multiply(out->limbs, lhs->limbs, lhs->length,
                                   rhs->limbs, rhs->length);
  out->negative = out->length == 0 ? 0 : lhs->negative ^ rhs->negative;
}

static void integer_divide_magnitudes(uint64_t *quotient, int *quotient_length,
                                      uint64_t *remainder,
                                      int *remainder_length,
                                      const sail_int lhs,
                                      const sail_int rhs) {
  uint64_t dividend[INTEGER_LIMBS], divisor[INTEGER_LIMBS];
  int dividend_length = lhs->length;
  int divisor_length = rhs->length;
  for (int i = 0; i < dividend_length; i++) dividend[i] = lhs->limbs[i];
  for (int i = 0; i < divisor_length; i++) divisor[i] = rhs->limbs[i];
  magnitude_divide(quotient, quotient_length, remainder, remainder_length,
                   dividend, dividend_length, divisor, divisor_length);
}

void tdiv_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  uint64_t remainder[INTEGER_LIMBS];
  int quotient_length, remainder_length;
  integer *out = *result;
  integer_divide_magnitudes(out->limbs, &quotient_length,
                            remainder, &remainder_length, lhs, rhs);
  out->length = (uint8_t)quotient_length;
  out->negative = out->length == 0 ? 0 : lhs->negative ^ rhs->negative;
}

void tmod_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  uint64_t quotient[INTEGER_LIMBS];
  int quotient_length, remainder_length;
  integer *out = *result;
  integer_divide_magnitudes(quotient, &quotient_length, out->limbs,
                            &remainder_length, lhs, rhs);
  out->length = (uint8_t)remainder_length;
  out->negative = out->length == 0 ? 0 : lhs->negative;
}

/* Euclidean division: remainder is always in [0, |rhs|). */
void ediv_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  uint64_t quotient[INTEGER_LIMBS], remainder[INTEGER_LIMBS];
  int quotient_length, remainder_length;
  integer_divide_magnitudes(quotient, &quotient_length,
                            remainder, &remainder_length, lhs, rhs);
  if (lhs->negative && remainder_length != 0) {
    const uint64_t one[1] = {1};
    quotient_length = magnitude_add(quotient, quotient, quotient_length,
                                    one, 1);
  }
  integer *out = *result;
  for (int i = 0; i < quotient_length; i++) out->limbs[i] = quotient[i];
  out->length = quotient_length;
  out->negative = out->length == 0 ? 0 : lhs->negative ^ rhs->negative;
}

void emod_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  uint64_t quotient[INTEGER_LIMBS], remainder[INTEGER_LIMBS];
  int quotient_length, remainder_length;
  integer_divide_magnitudes(quotient, &quotient_length,
                            remainder, &remainder_length, lhs, rhs);
  integer *out = *result;
  if (lhs->negative && remainder_length != 0) {
    out->length = magnitude_subtract(out->limbs, rhs->limbs, rhs->length,
                                     remainder, remainder_length);
  } else {
    for (int i = 0; i < remainder_length; i++) out->limbs[i] = remainder[i];
    out->length = remainder_length;
  }
  out->negative = 0;
}

/* Flooring division: remainder has the divisor's sign. */
void fdiv_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  uint64_t quotient[INTEGER_LIMBS], remainder[INTEGER_LIMBS];
  int quotient_length, remainder_length;
  integer_divide_magnitudes(quotient, &quotient_length,
                            remainder, &remainder_length, lhs, rhs);
  bool negative = lhs->negative ^ rhs->negative;
  if (negative && remainder_length != 0) {
    const uint64_t one[1] = {1};
    quotient_length = magnitude_add(quotient, quotient, quotient_length,
                                    one, 1);
  }
  integer *out = *result;
  for (int i = 0; i < quotient_length; i++) out->limbs[i] = quotient[i];
  out->length = quotient_length;
  out->negative = out->length == 0 ? 0 : negative;
}

void fmod_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  uint64_t quotient[INTEGER_LIMBS], remainder[INTEGER_LIMBS];
  int quotient_length, remainder_length;
  integer_divide_magnitudes(quotient, &quotient_length,
                            remainder, &remainder_length, lhs, rhs);
  integer *out = *result;
  if ((lhs->negative ^ rhs->negative) && remainder_length != 0) {
    out->length = magnitude_subtract(out->limbs, rhs->limbs, rhs->length,
                                     remainder, remainder_length);
  } else {
    for (int i = 0; i < remainder_length; i++) out->limbs[i] = remainder[i];
    out->length = remainder_length;
  }
  out->negative = out->length == 0 ? 0 : rhs->negative;
}

void max_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  copy_sail_int(result, integer_compare(lhs, rhs) < 0 ? rhs : lhs);
}
void min_int(sail_int *result, const sail_int lhs, const sail_int rhs) {
  copy_sail_int(result, integer_compare(lhs, rhs) > 0 ? rhs : lhs);
}

bool eq_int(const sail_int lhs, const sail_int rhs) {
  return integer_compare(lhs, rhs) == 0;
}
bool eq_sail_int(const sail_int lhs, const sail_int rhs) {
  return eq_int(lhs, rhs);
}
bool lt(const sail_int lhs, const sail_int rhs) {
  return integer_compare(lhs, rhs) < 0;
}
bool gt(const sail_int lhs, const sail_int rhs) {
  return integer_compare(lhs, rhs) > 0;
}
bool lteq(const sail_int lhs, const sail_int rhs) {
  return integer_compare(lhs, rhs) <= 0;
}
bool gteq(const sail_int lhs, const sail_int rhs) {
  return integer_compare(lhs, rhs) >= 0;
}

void shl_int(sail_int *result, const sail_int value, const sail_int amount) {
  uint64_t shift = integer_small_unsigned(amount);
  if (value->length == 0) {
    integer_zero(*result);
    return;
  }
  if (shift >= INTEGER_LIMBS * 64) integer_failure();
  int word_shift = (int)(shift >> 6);
  int bit_shift = (int)(shift & 63);
  uint64_t shifted[INTEGER_LIMBS] = {0};
  for (int i = 0; i < value->length; i++) {
    int target = i + word_shift;
    if (target >= INTEGER_LIMBS) integer_failure();
    shifted[target] |= value->limbs[i] << bit_shift;
    if (bit_shift != 0) {
      uint64_t high = value->limbs[i] >> (64 - bit_shift);
      if (high != 0) {
        if (target + 1 >= INTEGER_LIMBS) integer_failure();
        shifted[target + 1] |= high;
      }
    }
  }
  integer *out = *result;
  for (int i = 0; i < INTEGER_LIMBS; i++) out->limbs[i] = shifted[i];
  out->length = magnitude_normalize(out->limbs, INTEGER_LIMBS);
  out->negative = value->negative;
}

void shr_int(sail_int *result, const sail_int value, const sail_int amount) {
  uint64_t shift = integer_small_unsigned(amount);
  bool negative = value->negative;
  if (value->length == 0) {
    integer_zero(*result);
    return;
  }
  bool discarded = false;
  if (shift >= INTEGER_LIMBS * 64) {
    discarded = true;
    integer_zero(*result);
  } else {
    int word_shift = (int)(shift >> 6);
    int bit_shift = (int)(shift & 63);
    uint64_t shifted[INTEGER_LIMBS] = {0};
    for (int i = 0; i < word_shift && i < value->length; i++) {
      discarded |= value->limbs[i] != 0;
    }
    if (bit_shift != 0 && word_shift < value->length) {
      discarded |= (value->limbs[word_shift] &
                    ((UINT64_C(1) << bit_shift) - 1)) != 0;
    }
    for (int source = word_shift; source < value->length; source++) {
      int target = source - word_shift;
      shifted[target] |= value->limbs[source] >> bit_shift;
      if (bit_shift != 0 && source + 1 < value->length) {
        shifted[target] |= value->limbs[source + 1] << (64 - bit_shift);
      }
    }
    integer *out = *result;
    for (int i = 0; i < INTEGER_LIMBS; i++) out->limbs[i] = shifted[i];
    out->length = magnitude_normalize(out->limbs, INTEGER_LIMBS);
    out->negative = negative && out->length != 0;
  }
  if (negative && discarded) {
    const uint64_t one[1] = {1};
    integer *out = *result;
    out->length = magnitude_add(out->limbs, out->limbs, out->length, one, 1);
    out->negative = 1;
  }
}

mach_int shl_mach_int(const mach_int value, const mach_int amount) {
  return value << amount;
}
mach_int shr_mach_int(const mach_int value, const mach_int amount) {
  return value >> amount;
}

void pow_int(sail_int *result, const sail_int base, const sail_int exponent) {
  uint64_t power = integer_small_unsigned(exponent);
  sail_int accumulated, factor, temporary;
  integer_one(accumulated);
  copy_sail_int(&factor, base);
  while (power != 0) {
    if ((power & 1) != 0) {
      mult_int(&temporary, accumulated, factor);
      copy_sail_int(&accumulated, temporary);
    }
    power >>= 1;
    if (power != 0) {
      mult_int(&temporary, factor, factor);
      copy_sail_int(&factor, temporary);
    }
  }
  copy_sail_int(result, accumulated);
}

void pow2(sail_int *result, const sail_int exponent) {
  uint64_t power = integer_small_unsigned(exponent);
  if (power >= INTEGER_LIMBS * 64) integer_failure();
  integer *out = *result;
  int limb = (int)(power >> 6);
  for (int i = 0; i <= limb; i++) out->limbs[i] = 0;
  out->limbs[limb] = UINT64_C(1) << (power & 63);
  out->length = limb + 1;
  out->negative = 0;
}

void undefined_int(sail_int *result, const int value) {
  convert_sail_int_of_mach_int(result, value);
}
void undefined_nat(sail_int *result, const unit value) {
  (void)value;
  integer_zero(*result);
}
void undefined_range(sail_int *result, const sail_int lower,
                     const sail_int upper) {
  (void)upper;
  copy_sail_int(result, lower);
}
void make_the_value(sail_int *result, const sail_int value) {
  copy_sail_int(result, value);
}
void size_itself_int(sail_int *result, const sail_int value) {
  copy_sail_int(result, value);
}

int mpz_set_str(sail_int result, const char *string, int base) {
  if (base != 0 && base != 10) return -1;
  const char *cursor = string;
  while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n') cursor++;
  bool negative = false;
  if (*cursor == '-' || *cursor == '+') negative = *cursor++ == '-';
  integer_zero(result);
  bool digit_seen = false;
  const uint64_t ten[1] = {10};
  while (*cursor >= '0' && *cursor <= '9') {
    digit_seen = true;
    uint64_t product[INTEGER_LIMBS];
    int product_length = magnitude_multiply(product, result->limbs,
                                             result->length, ten, 1);
    const uint64_t digit[1] = {(uint64_t)(*cursor++ - '0')};
    result->length = magnitude_add(result->limbs, product, product_length,
                                   digit, digit[0] == 0 ? 0 : 1);
  }
  if (!digit_seen || *cursor != '\0') return -1;
  result->negative = negative && result->length != 0;
  return 0;
}

void convert_sail_int_of_sail_string(sail_int *result,
                                     const_sail_string string) {
  if (mpz_set_str(*result, string, 10) != 0) integer_failure();
}
void create_sail_int_of_sail_string(sail_int *result,
                                    const_sail_string string) {
  convert_sail_int_of_sail_string(result, string);
}
void recreate_sail_int_of_sail_string(sail_int *result,
                                      const_sail_string string) {
  convert_sail_int_of_sail_string(result, string);
}

/* Generated code uses this helper for sub-64-bit extracts.  Keep it in the
 * freestanding core rather than sail_native.c: the optimized RISC-V model
 * references it too, and an over-wide C shift would otherwise be undefined. */
fbits safe_rshift(const fbits value, const fbits amount) {
  return amount >= UINT64_C(64) ? UINT64_C(0) : value >> amount;
}

void create_sail_string(sail_string *string) {
  char *empty = (char *)malloc(1);
  empty[0] = 0;
  *string = empty;
}
void kill_sail_string(sail_string *string) { free((void *)*string); }
void copy_sail_string(sail_string *result, const_sail_string value) {
  size_t length = strlen(value);
  *result = (char *)realloc((void *)*result, length + 1);
  memcpy(*result, value, length + 1);
}
