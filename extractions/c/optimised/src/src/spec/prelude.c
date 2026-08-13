#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const u256 ZERO_WORD = (u256){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(0)}};


const bytes20 ZERO_ADDRESS = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(0)
  },
};


const bytes32 ZERO_HASH = {
  .lanes = {
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(0),
      UINT64_C(0)
  },
};


const u256 WORD_ZERO = (u256){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(0)}};


const u256 WORD_ONE = (u256){{UINT64_C(1), UINT64_C(0), UINT64_C(0), UINT64_C(0)}};


const u256 WORD_ALL_ONES = (u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}};


bool neq_bool(bool x, bool y)
{
  return (bool)(x != y);
}

bool neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(bytes32 x, bytes32 y)
{
  bool eq_anything_result_2_2778 = eq_bytes32(y, x);
  return (bool)(!eq_anything_result_2_2778);
}

bool neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(bytes20 x, bytes20 y)
{
  bool eq_anything_result_2_2778 = eq_bytes20(y, x);
  return (bool)(!eq_anything_result_2_2778);
}

u256 as_u256(u256 value)
{
  return value;
}

bytes32 B256(bytes32 bytes)
{
  return bytes;
}

u256 word_add_word(u256 left, u256 right)
{
  return as_u256(u256_add(left, right));
}

__attribute__((__always_inline__)) u256 word_sub_word(u256 left, u256 right)
{
  return !u256_lt(left, right) ? u256_sub(left, right) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(right, left))), u320_of_u64(UINT8_C(1))));
}

u256 word_and(u256 left, u256 right)
{
  return as_u256(u256_and(left, right));
}

u256 word_or(u256 left, u256 right)
{
  return as_u256(u256_or(left, right));
}

u256 word_xor(u256 left, u256 right)
{
  return as_u256(u256_xor(left, right));
}

u256 word_not(u256 value)
{
  return as_u256(u256_not(value));
}

uint64_t word_bit(u256 value, uint8_t index)
{
  return u256_bit(value, index);
}

uint64_t word_low_byte(u256 value)
{
  return (UINT64_C(0xFF) & u256_extract_u64(value, (uint64_t)(UINT64_C(0))));
}

u256 word_of_bool(bool b)
{
  if (b) {
    return WORD_ONE;
  }
  return WORD_ZERO;
}

__attribute__((__always_inline__)) bool word_is_zero(u256 w)
{
  return eq_u256(w, WORD_ZERO);
}

bool word_nonzero(u256 w)
{
  bool is_zero = eq_u256(w, WORD_ZERO);
  return (bool)(!is_zero);
}

bool word_ult(u256 a, u256 b)
{
  return u256_lt(a, b);
}

bool word_ule(u256 a, u256 b)
{
  bool greater = word_ult(b, a);
  return (bool)(!greater);
}

uint8_t u64_bit_length_(uint64_t value)
{
  return u64_bit_length(value);
}

uint16_t word_bit_length(u256 value)
{
  uint16_t result_8_442;
  if (u256_extract_u64(value, (uint64_t)(UINT64_C(192))) != UINT8_C(0)) {
    uint8_t u64_bit_length_result_2_2652 = u64_bit_length_(u256_extract_u64(value, (uint64_t)(UINT64_C(192))));
    result_8_442 = ((uint16_t)((uint32_t)UINT16_C(192) + (uint32_t)(uint16_t)u64_bit_length_result_2_2652));
  } else if (u256_extract_u64(value, (uint64_t)(UINT64_C(128))) != UINT8_C(0)) {
    uint8_t u64_bit_length_result_2_2654 = u64_bit_length_(u256_extract_u64(value, (uint64_t)(UINT64_C(128))));
    result_8_442 = ((uint16_t)u64_bit_length_result_2_2654 + (uint16_t)UINT8_C(128));
  } else if (u256_extract_u64(value, (uint64_t)(UINT64_C(64))) != UINT8_C(0)) {
    uint8_t u64_bit_length_result_2_2656 = u64_bit_length_(u256_extract_u64(value, (uint64_t)(UINT64_C(64))));
    result_8_442 = ((uint16_t)u64_bit_length_result_2_2656 + (uint16_t)UINT8_C(64));
  } else {
    uint8_t tmp_3_3998 = u64_bit_length_(u256_extract_u64(value, (uint64_t)(UINT64_C(0))));
    result_8_442 = (uint16_t)tmp_3_3998;
  }
  return result_8_442;
}

u256 word_mul_word(u256 a, u256 b)
{
  return u256_mul(a, b);
}

bool word_greater_than_word(u256 left, u256 right)
{
  return u256_lt(right, left);
}

u256 word_shift_left(u256 value, uint16_t amount)
{
  u256 shifted_bits = u256_shiftl_u64(value, amount);
  return as_u256(shifted_bits);
}

u256 word_shift_right(u256 value, uint16_t amount)
{
  return u256_shiftr_u64(value, amount);
}

__attribute__((__always_inline__)) uint8_t word_byte_length(u256 value)
{
  uint16_t bit_length = word_bit_length(value);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

__attribute__((__always_inline__)) u256 word_arithmetic_shift_right(u256 value, uint16_t amount)
{
  u256 shifted = word_shift_right(value, amount);
  uint64_t sign_bit = word_bit(value, UINT8_C(255));
  if (sign_bit == UINT64_C(0x1)) {
    u256 sign_fill = word_shift_left(WORD_ALL_ONES, ((uint16_t)((uint32_t)UINT16_C(256) - (uint32_t)amount)));
    shifted = word_or(shifted, sign_fill);
  }
  return shifted;
}

u256 word_negate(u256 value)
{
  u256 result_8_450;
  bool lteq_int_result_2_2658 = (bool)(!u256_lt(WORD_ZERO, value));
  if (lteq_int_result_2_2658) {
    result_8_450 = u256_sub(WORD_ZERO, value);
  } else {
    u256 sub_atom_result_2_2659 = u256_sub(value, WORD_ZERO);
    result_8_450 = u256_of_u320_unchecked(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, sub_atom_result_2_2659)), u320_of_u64(UINT8_C(1))));
  }
  return result_8_450;
}

__attribute__((__always_inline__)) u256 word_abs(u256 value)
{
  uint64_t sign_bit = word_bit(value, UINT8_C(255));
  if (sign_bit == UINT64_C(0x1)) {
    return word_negate(value);
  }
  return value;
}

__attribute__((__always_inline__)) bool word_slt(u256 a, u256 b)
{
  uint64_t word_bit_result_2_2603 = word_bit(a, UINT8_C(255));
  uint64_t word_bit_result_2_2602 = word_bit(b, UINT8_C(255));
  if (word_bit_result_2_2603 == UINT64_C(0x1)) {
    if (word_bit_result_2_2602 == UINT64_C(0x1)) {
      return word_ult(a, b);
    }
    return true;
  }
  if (word_bit_result_2_2602 == UINT64_C(0x1)) {
    return false;
  }
  return word_ult(a, b);
}

u256 alu_add(u256 a, u256 b)
{
  return word_add_word(a, b);
}

u256 alu_sub(u256 a, u256 b)
{
  return !u256_lt(a, b) ? u256_sub(a, b) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(b, a))), u320_of_u64(UINT8_C(1))));
}

u256 alu_mul(u256 a, u256 b)
{
  return word_mul_word(a, b);
}

u256 alu_div(u256 a, u256 b)
{
  bool divisor_is_zero = eq_u256(b, WORD_ZERO);
  if (divisor_is_zero) {
    return WORD_ZERO;
  }
  return word_div_nonzero(a, b);
}

u256 alu_mod(u256 a, u256 b)
{
  bool modulus_is_zero = eq_u256(b, WORD_ZERO);
  if (modulus_is_zero) {
    return WORD_ZERO;
  }
  return word_mod_nonzero(a, b);
}

u256 alu_sdiv(u256 a, u256 b)
{
  bool divisor_is_zero = eq_u256(b, WORD_ZERO);
  if (divisor_is_zero) {
    return WORD_ZERO;
  }
  u256 dividend_magnitude;
  uint64_t sign_bit = word_bit(a, UINT8_C(255));
  if (sign_bit == UINT64_C(0x1)) {
    dividend_magnitude = word_negate(a);
  } else {
    dividend_magnitude = a;
  }
  u256 divisor_magnitude;
  uint64_t sign_bit_8_0 = word_bit(b, UINT8_C(255));
  if (sign_bit_8_0 == UINT64_C(0x1)) {
    divisor_magnitude = word_negate(b);
  } else {
    divisor_magnitude = b;
  }
  u256 quotient = word_div_nonzero(dividend_magnitude, divisor_magnitude);
  uint64_t dividend_sign = word_bit(a, UINT8_C(255));
  uint64_t divisor_sign = word_bit(b, UINT8_C(255));
  bool result_2_2601 = neq_bool((bool)(dividend_sign == UINT64_C(0x1)), (bool)(divisor_sign == UINT64_C(0x1)));
  if (result_2_2601) {
    quotient = word_negate(quotient);
  }
  return quotient;
}

u256 alu_smod(u256 a, u256 b)
{
  bool modulus_is_zero = eq_u256(b, WORD_ZERO);
  if (modulus_is_zero) {
    return WORD_ZERO;
  }
  u256 dividend_magnitude;
  uint64_t sign_bit = word_bit(a, UINT8_C(255));
  if (sign_bit == UINT64_C(0x1)) {
    dividend_magnitude = word_negate(a);
  } else {
    dividend_magnitude = a;
  }
  u256 modulus_magnitude;
  uint64_t sign_bit_8_2 = word_bit(b, UINT8_C(255));
  if (sign_bit_8_2 == UINT64_C(0x1)) {
    modulus_magnitude = word_negate(b);
  } else {
    modulus_magnitude = b;
  }
  u256 remainder = word_mod_nonzero(dividend_magnitude, modulus_magnitude);
  uint64_t dividend_sign = word_bit(a, UINT8_C(255));
  if (dividend_sign == UINT64_C(0x1)) {
    remainder = word_negate(remainder);
  }
  return remainder;
}

u256 alu_addmod(u256 a, u256 b, u256 n)
{
  if (u256_eq_u64(n, UINT8_C(0))) {
    return WORD_ZERO;
  }
  return word_addmod_nonzero(a, b, n);
}

u256 alu_mulmod(u256 a, u256 b, u256 n)
{
  if (u256_eq_u64(n, UINT8_C(0))) {
    return WORD_ZERO;
  }
  return word_mulmod_nonzero(a, b, n);
}

u256 alu_exp(u256 base, u256 exponent)
{
  return word_exp_ladder(base, exponent);
}

u256 alu_signextend(u256 byte_index, u256 value)
{
  if (u256_lt_u64(byte_index, UINT8_C(32))) {
    uint8_t index = (uint8_t)u256_to_u64_unchecked(byte_index);
    u256 shifted_sign = word_shift_right(value, ((uint16_t)UINT8_C(7) + ((uint16_t)UINT8_C(8) * (uint16_t)index)));
    u256 isolated_sign = word_and(shifted_sign, WORD_ONE);
    bool sign_set = eq_u256(isolated_sign, WORD_ONE);
    u256 low_mask_end = word_shift_left(WORD_ONE, ((uint16_t)((uint32_t)((uint16_t)UINT8_C(8) * (uint16_t)index) + (uint32_t)UINT16_C(8))));
    u256 low_mask;
    bool lteq_int_result_2_2658 = (bool)(!u256_lt(low_mask_end, WORD_ONE));
    if (lteq_int_result_2_2658) {
      low_mask = u256_sub(low_mask_end, WORD_ONE);
    } else {
      u256 sub_atom_result_2_2659 = u256_sub(WORD_ONE, low_mask_end);
      low_mask = u256_of_u320_unchecked(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, sub_atom_result_2_2659)), u320_of_u64(UINT8_C(1))));
    }
    if (sign_set) {
      u256 low_value = word_and(value, low_mask);
      u256 high_mask = word_not(low_mask);
      return word_or(low_value, high_mask);
    }
    return word_and(value, low_mask);
  }
  return value;
}

u256 alu_lt(u256 a, u256 b)
{
  bool result = word_ult(a, b);
  return word_of_bool(result);
}

u256 alu_gt(u256 a, u256 b)
{
  bool result = word_ult(b, a);
  return word_of_bool(result);
}

u256 alu_slt(u256 a, u256 b)
{
  bool result;
  uint64_t word_bit_result_2_2603 = word_bit(a, UINT8_C(255));
  uint64_t word_bit_result_2_2602 = word_bit(b, UINT8_C(255));
  if (word_bit_result_2_2603 == UINT64_C(0x1)) {
    if (word_bit_result_2_2602 == UINT64_C(0x1)) {
      result = word_ult(a, b);
    } else {
      result = true;
    }
  } else if (word_bit_result_2_2602 == UINT64_C(0x1)) {
    result = false;
  } else {
    result = word_ult(a, b);
  }
  return word_of_bool(result);
}

u256 alu_sgt(u256 a, u256 b)
{
  bool result;
  uint64_t word_bit_result_2_2603 = word_bit(b, UINT8_C(255));
  uint64_t word_bit_result_2_2602 = word_bit(a, UINT8_C(255));
  if (word_bit_result_2_2603 == UINT64_C(0x1)) {
    if (word_bit_result_2_2602 == UINT64_C(0x1)) {
      result = word_ult(b, a);
    } else {
      result = true;
    }
  } else if (word_bit_result_2_2602 == UINT64_C(0x1)) {
    result = false;
  } else {
    result = word_ult(b, a);
  }
  return word_of_bool(result);
}

u256 alu_eq(u256 a, u256 b)
{
  return word_of_bool(eq_u256(a, b));
}

u256 alu_iszero(u256 a)
{
  bool result = eq_u256(a, WORD_ZERO);
  return word_of_bool(result);
}

u256 alu_and(u256 a, u256 b)
{
  return word_and(a, b);
}

u256 alu_or(u256 a, u256 b)
{
  return word_or(a, b);
}

u256 alu_xor(u256 a, u256 b)
{
  return word_xor(a, b);
}

u256 alu_not(u256 a)
{
  return word_not(a);
}

u256 alu_byte(u256 i, u256 x)
{
  if (u256_lt_u64(i, UINT8_C(32))) {
    u256 shifted = word_shift_right(x, ((uint16_t)UINT8_C(8) * ((uint16_t)UINT8_C(31) - (uint16_t)(uint8_t)u256_to_u64_unchecked(i))));
    uint64_t result_byte = word_low_byte(shifted);
    uint8_t tmp_3_3786 = (uint8_t)result_byte;
    return u256_of_fbits(tmp_3_3786);
  }
  return WORD_ZERO;
}

u256 alu_shl(u256 shift_amt, u256 v)
{
  if (u256_lt_u64(shift_amt, UINT16_C(256))) {
    return word_shift_left(v, (uint16_t)u256_to_u64_unchecked(shift_amt));
  }
  return WORD_ZERO;
}

u256 alu_shr(u256 shift_amt, u256 v)
{
  if (u256_lt_u64(shift_amt, UINT16_C(256))) {
    return word_shift_right(v, (uint16_t)u256_to_u64_unchecked(shift_amt));
  }
  return WORD_ZERO;
}

u256 alu_sar(u256 shift_amt, u256 v)
{
  if (u256_lt_u64(shift_amt, UINT16_C(256))) {
    u256 shifted = word_shift_right(v, (uint16_t)u256_to_u64_unchecked(shift_amt));
    uint64_t sign_bit = word_bit(v, UINT8_C(255));
    if (sign_bit == UINT64_C(0x1)) {
      u256 sign_fill = word_shift_left(WORD_ALL_ONES, ((uint16_t)((uint32_t)UINT16_C(256) - (uint32_t)(uint16_t)u256_to_u64_unchecked(shift_amt))));
      shifted = word_or(shifted, sign_fill);
    }
    return shifted;
  }
  uint64_t sign_bit_8_4 = word_bit(v, UINT8_C(255));
  if (sign_bit_8_4 == UINT64_C(0x1)) {
    return WORD_ALL_ONES;
  }
  return WORD_ZERO;
}

u256 alu_clz(u256 x)
{
  uint16_t bit_length = word_bit_length(x);
  return u256_uint16_t_to_u256(((uint16_t)((uint32_t)UINT16_C(256) - (uint32_t)bit_length)));
}

void initialize_registers(void)
{

}

u256 alu_add_u256_u128_to_u256(u256 a, u128 b)
{
  return word_add_word_u256_u128_to_u256(a, b);
}

u256 alu_add_u256_uint64_t_to_u256(u256 a, uint64_t b)
{
  return word_add_word_u256_uint64_t_to_u256(a, b);
}

u256 u256_u128_to_u256(u128 value)
{
  return u256_of_u128(value);
}

u256 u256_uint16_t_to_u256(uint16_t value)
{
  return u256_of_fbits(value);
}

u256 u256_uint32_t_to_u256(uint32_t value)
{
  return u256_of_fbits(value);
}

u256 u256_uint64_t_to_u256(uint64_t value)
{
  return u256_of_fbits(value);
}

u256 u256_uint8_t_to_u256(uint8_t value)
{
  return u256_of_fbits(value);
}

u256 word_add_word_u256_u128_to_u256(u256 left, u128 right)
{
  return as_u256(u256_add_u128(left, right));
}

u256 word_add_word_u256_uint64_t_to_u256(u256 left, uint64_t right)
{
  return as_u256(u256_add_u64(left, right));
}

u256 word_add_word_u256_uint8_t_to_u256(u256 left, uint8_t right)
{
  return as_u256(u256_add_u64(left, right));
}

uint16_t word_bit_length_u128_to_uint16_t(u128 value)
{
  uint16_t result_8_1431;
  if (u128_extract_u64(value, (uint64_t)(UINT64_C(64))) != UINT8_C(0)) {
    uint8_t u64_bit_length_result_2_2656 = u64_bit_length_(u128_extract_u64(value, (uint64_t)(UINT64_C(64))));
    result_8_1431 = ((uint16_t)u64_bit_length_result_2_2656 + (uint16_t)UINT8_C(64));
  } else {
    uint8_t tmp_3_4098 = u64_bit_length_(u128_extract_u64(value, (uint64_t)(UINT64_C(0))));
    result_8_1431 = (uint16_t)tmp_3_4098;
  }
  return result_8_1431;
}

uint16_t word_bit_length_uint64_t_to_uint16_t(uint64_t value)
{
  uint8_t tmp_3_4099 = u64_bit_length_((value >> UINT64_C(0)));
  return (uint16_t)tmp_3_4099;
}

uint16_t word_bit_length_uint8_t_to_uint16_t(uint8_t value)
{
  uint8_t tmp_3_4100 = u64_bit_length_((value >> UINT64_C(0)));
  return (uint16_t)tmp_3_4100;
}

__attribute__((__always_inline__)) uint8_t word_byte_length_u128_to_uint8_t(u128 value)
{
  uint16_t bit_length = word_bit_length_u128_to_uint16_t(value);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

__attribute__((__always_inline__)) uint8_t word_byte_length_uint64_t_to_uint8_t(uint64_t value)
{
  uint16_t bit_length = word_bit_length_uint64_t_to_uint16_t(value);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

__attribute__((__always_inline__)) uint8_t word_byte_length_uint8_t_to_uint8_t(uint8_t value)
{
  uint16_t bit_length = word_bit_length_uint8_t_to_uint16_t(value);
  if (bit_length == UINT8_C(0)) {
    return UINT8_C(0);
  }
  return (uint8_t)((uint16_t)(((uint32_t)bit_length + (uint32_t)UINT16_C(7)) / (uint32_t)UINT16_C(8)));
}

u256 word_div_word_u256_u256_to_u256(u256 dividend, u256 divisor)
{
  return as_u256(u256_div(dividend, divisor));
}

u256 word_div_word_u256_u256_to_u256_variant_2(u256 dividend, u256 divisor)
{
  return as_u256(u256_of_fbits(((uint32_t)u256_to_u64_unchecked(dividend) / (uint32_t)u256_to_u64_unchecked(divisor))));
}

u256 word_div_word_u256_uint32_t_to_u256(u256 dividend, uint32_t divisor)
{
  if (divisor == UINT8_C(0)) {
    return WORD_ZERO;
  }
  return as_u256(u256_div_u64(dividend, divisor));
}

u256 word_div_word_u256_uint32_t_to_u256_variant_2(u256 dividend, uint32_t divisor)
{
  return as_u256(u256_div_u64(dividend, divisor));
}

u256 word_div_word_u256_uint64_t_to_u256(u256 dividend, uint64_t divisor)
{
  if (divisor == UINT8_C(0)) {
    return WORD_ZERO;
  }
  return as_u256(u256_div_u64(dividend, divisor));
}

u256 word_div_word_u256_uint8_t_to_u256(u256 dividend, uint8_t divisor)
{
  return as_u256(u256_div_u64(dividend, divisor));
}

__attribute__((__always_inline__)) bool word_is_zero_u128_to_bool(u128 w)
{
  return u256_eq_u128(WORD_ZERO, w);
}

__attribute__((__always_inline__)) bool word_is_zero_uint32_t_to_bool(uint32_t w)
{
  return u256_eq_u64(WORD_ZERO, w);
}

__attribute__((__always_inline__)) bool word_is_zero_uint64_t_to_bool(uint64_t w)
{
  return u256_eq_u64(WORD_ZERO, w);
}

uint64_t word_low_byte_u128_to_uint64_t(u128 value)
{
  return (UINT64_C(0xFF) & u128_extract_u64(value, (uint64_t)(UINT64_C(0))));
}

uint64_t word_low_byte_uint64_t_to_uint64_t(uint64_t value)
{
  return (UINT64_C(0xFF) & (value >> UINT64_C(0)));
}

uint64_t word_low_byte_uint8_t_to_uint64_t(uint8_t value)
{
  return (UINT64_C(0xFF) & (value >> UINT64_C(0)));
}

u256 word_mod_word_u256_u256_to_u256(u256 dividend, u256 divisor)
{
  return as_u256(u256_mod(dividend, divisor));
}

u256 word_mod_word_u256_u256_to_u256_variant_2(u256 dividend, u256 divisor)
{
  return as_u256(u256_of_fbits(((uint32_t)u256_to_u64_unchecked(dividend) % (uint32_t)u256_to_u64_unchecked(divisor))));
}

u256 word_mul_word_u256_uint32_t_to_u256(u256 a, uint32_t b)
{
  return u256_mul_u64(a, b);
}

u256 word_mul_word_u256_uint64_t_to_u256(u256 a, uint64_t b)
{
  return u256_mul_u64(a, b);
}

u256 word_shift_left_u256_uint8_t_to_u256(u256 value, uint8_t amount)
{
  u256 shifted_bits = u256_shiftl_u64(value, amount);
  return as_u256(shifted_bits);
}

u256 word_shift_right_u256_uint8_t_to_u256(u256 value, uint8_t amount)
{
  return u256_shiftr_u64(value, amount);
}

__attribute__((__always_inline__)) u256 word_sub_word_u256_uint8_t_to_u256(u256 left, uint8_t right)
{
  if (!u256_lt_u64(left, right)) {
    return u256_sub_u64(left, right);
  }
  return u256_add_u64(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_of_fbits(((uint8_t)((uint32_t)right - (uint32_t)(uint8_t)u256_to_u64_unchecked(left))))), UINT8_C(1));
}

bool word_ule_uint8_t_u256_to_bool(uint8_t a, u256 b)
{
  bool greater = word_ult_u256_uint8_t_to_bool(b, a);
  return (bool)(!greater);
}

bool word_ult_u256_uint8_t_to_bool(u256 a, uint8_t b)
{
  return u256_lt_u64(a, b);
}

