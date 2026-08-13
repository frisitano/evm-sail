#pragma once

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } u128;
#endif

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } u256;
#endif


#ifndef SAIL_FIXED_BYTES_U64_LANES_32_DEFINED
#define SAIL_FIXED_BYTES_U64_LANES_32_DEFINED
typedef struct { uint64_t lanes[4]; } bytes32;
#endif

#ifndef SAIL_FIXED_BYTES_U64_LANES_20_DEFINED
#define SAIL_FIXED_BYTES_U64_LANES_20_DEFINED
typedef struct { uint64_t lanes[3]; } bytes20;
#endif

// struct AddressResult
struct AddressResult {
  bytes20 address;
  bool success;
};

#ifndef SAIL_FIXED_VECTOR_VECTOR_32_BITS_8_DEFINED
#define SAIL_FIXED_VECTOR_VECTOR_32_BITS_8_DEFINED
typedef struct vector_32_bits_8 {
  size_t len;
  uint64_t data[32];
} vector_32_bits_8;
#endif

#ifndef SAIL_FIXED_VECTOR_VECTOR_20_BITS_8_DEFINED
#define SAIL_FIXED_VECTOR_VECTOR_20_BITS_8_DEFINED
typedef struct vector_20_bits_8 {
  size_t len;
  uint64_t data[20];
} vector_20_bits_8;
#endif


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } u128;
#endif

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } u256;
#endif

#ifndef SAIL_U320_DEFINED
#define SAIL_U320_DEFINED
typedef struct { uint64_t limbs[5]; } u320;
#endif



#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } u128;
#endif


bool neq_bool(bool x, bool y);

bool neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(bytes32 x, bytes32 y);

bool neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(bytes20 x, bytes20 y);

u256 as_u256(u256 value);

bytes32 B256(bytes32 bytes);

u256 hash_to_word(bytes32 bytes);

bytes32 word_to_hash(u256 value);

bytes20 word_to_address(u256 value);

u256 word_add_word(u256 left, u256 right);

__attribute__((__always_inline__)) u256 word_sub_word(u256 left, u256 right);

u256 word_and(u256 left, u256 right);

u256 word_or(u256 left, u256 right);

u256 word_xor(u256 left, u256 right);

u256 word_not(u256 value);

uint64_t word_bit(u256 value, uint8_t index);

uint64_t word_low_byte(u256 value);

u256 word_of_bool(bool b);

__attribute__((__always_inline__)) bool word_is_zero(u256 w);

bool word_nonzero(u256 w);

bool word_ult(u256 a, u256 b);

bool word_ule(u256 a, u256 b);

uint8_t u64_bit_length(uint64_t /* arg_0 */);

uint8_t u64_bit_length_(uint64_t value);

uint16_t word_bit_length(u256 value);

u256 word_mul_word(u256 a, u256 b);

bool word_greater_than_word(u256 left, u256 right);

u256 word_shift_left(u256 value, uint16_t amount);

u256 word_shift_right(u256 value, uint16_t amount);

__attribute__((__always_inline__)) uint8_t word_byte_length(u256 value);

__attribute__((__always_inline__)) u256 word_arithmetic_shift_right(u256 value, uint16_t amount);

u256 address_to_word(bytes20 bytes);

u256 word_negate(u256 value);

__attribute__((__always_inline__)) u256 word_abs(u256 value);

__attribute__((__always_inline__)) bool word_slt(u256 a, u256 b);

u256 alu_add(u256 a, u256 b);

u256 alu_sub(u256 a, u256 b);

u256 alu_mul(u256 a, u256 b);

u256 word_div_nonzero(u256 /* arg_0 */, u256 /* arg_1 */);

u256 alu_div(u256 a, u256 b);

u256 word_mod_nonzero(u256 /* arg_0 */, u256 /* arg_1 */);

u256 alu_mod(u256 a, u256 b);

u256 alu_sdiv(u256 a, u256 b);

u256 alu_smod(u256 a, u256 b);

u256 word_addmod_nonzero(u256 /* arg_0 */, u256 /* arg_1 */, u256 /* arg_2 */);

u256 alu_addmod(u256 a, u256 b, u256 n);

u256 word_mulmod_nonzero(u256 /* arg_0 */, u256 /* arg_1 */, u256 /* arg_2 */);

u256 alu_mulmod(u256 a, u256 b, u256 n);

u256 word_exp_ladder(u256 /* arg_0 */, u256 /* arg_1 */);

u256 alu_exp(u256 base, u256 exponent);

u256 alu_signextend(u256 byte_index, u256 value);

u256 alu_lt(u256 a, u256 b);

u256 alu_gt(u256 a, u256 b);

u256 alu_slt(u256 a, u256 b);

u256 alu_sgt(u256 a, u256 b);

u256 alu_eq(u256 a, u256 b);

u256 alu_iszero(u256 a);

u256 alu_and(u256 a, u256 b);

u256 alu_or(u256 a, u256 b);

u256 alu_xor(u256 a, u256 b);

u256 alu_not(u256 a);

u256 alu_byte(u256 i, u256 x);

u256 alu_shl(u256 shift_amt, u256 v);

u256 alu_shr(u256 shift_amt, u256 v);

u256 alu_sar(u256 shift_amt, u256 v);

u256 alu_clz(u256 x);

uint64_t rlp_length_byte_native(uint8_t /* arg_0 */);

bytes20 create2_address(bytes20 /* arg_0 */, u256 /* arg_1 */, bytes32 /* arg_2 */);

bool storage_id_is_warm(uint32_t /* arg_0 */);

uint32_t storage_resolve_slot(uint32_t /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, u256 /* arg_3 */);

void storage_update_by_id(uint32_t /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, u256 /* arg_3 */, u256 /* arg_4 */);

void receipt_table_reset(void);

bytes32 receipt_table_root(uint32_t /* arg_0 */);

void initialize_registers(void);

u256 alu_add_u256_u128_to_u256(u256 a, u128 b);

u256 alu_add_u256_uint64_t_to_u256(u256 a, uint64_t b);

u256 u256_u128_to_u256(u128 value);

u256 u256_uint16_t_to_u256(uint16_t value);

u256 u256_uint32_t_to_u256(uint32_t value);

u256 u256_uint64_t_to_u256(uint64_t value);

u256 u256_uint8_t_to_u256(uint8_t value);

u256 word_add_word_u256_u128_to_u256(u256 left, u128 right);

u256 word_add_word_u256_uint64_t_to_u256(u256 left, uint64_t right);

u256 word_add_word_u256_uint8_t_to_u256(u256 left, uint8_t right);

uint16_t word_bit_length_u128_to_uint16_t(u128 value);

uint16_t word_bit_length_uint64_t_to_uint16_t(uint64_t value);

uint16_t word_bit_length_uint8_t_to_uint16_t(uint8_t value);

__attribute__((__always_inline__)) uint8_t word_byte_length_u128_to_uint8_t(u128 value);

__attribute__((__always_inline__)) uint8_t word_byte_length_uint64_t_to_uint8_t(uint64_t value);

__attribute__((__always_inline__)) uint8_t word_byte_length_uint8_t_to_uint8_t(uint8_t value);

u256 word_div_word_u256_u256_to_u256(u256 dividend, u256 divisor);

u256 word_div_word_u256_u256_to_u256_variant_2(u256 dividend, u256 divisor);

u256 word_div_word_u256_uint32_t_to_u256(u256 dividend, uint32_t divisor);

u256 word_div_word_u256_uint32_t_to_u256_variant_2(u256 dividend, uint32_t divisor);

u256 word_div_word_u256_uint64_t_to_u256(u256 dividend, uint64_t divisor);

u256 word_div_word_u256_uint8_t_to_u256(u256 dividend, uint8_t divisor);

__attribute__((__always_inline__)) bool word_is_zero_u128_to_bool(u128 w);

__attribute__((__always_inline__)) bool word_is_zero_uint32_t_to_bool(uint32_t w);

__attribute__((__always_inline__)) bool word_is_zero_uint64_t_to_bool(uint64_t w);

uint64_t word_low_byte_u128_to_uint64_t(u128 value);

uint64_t word_low_byte_uint64_t_to_uint64_t(uint64_t value);

uint64_t word_low_byte_uint8_t_to_uint64_t(uint8_t value);

u256 word_mod_word_u256_u256_to_u256(u256 dividend, u256 divisor);

u256 word_mod_word_u256_u256_to_u256_variant_2(u256 dividend, u256 divisor);

u256 word_mul_word_u256_uint32_t_to_u256(u256 a, uint32_t b);

u256 word_mul_word_u256_uint64_t_to_u256(u256 a, uint64_t b);

u256 word_shift_left_u256_uint8_t_to_u256(u256 value, uint8_t amount);

u256 word_shift_right_u256_uint8_t_to_u256(u256 value, uint8_t amount);

__attribute__((__always_inline__)) u256 word_sub_word_u256_uint8_t_to_u256(u256 left, uint8_t right);

bool word_ule_uint8_t_u256_to_bool(uint8_t a, u256 b);

bool word_ult_u256_uint8_t_to_bool(u256 a, uint8_t b);

extern const u256 ZERO_WORD;


extern const bytes20 ZERO_ADDRESS;


extern const bytes32 ZERO_HASH;


extern const u256 WORD_ZERO;


extern const u256 WORD_ONE;


extern const u256 WORD_ALL_ONES;



#ifdef __cplusplus
}
#endif
