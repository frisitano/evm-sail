/* Generated from sail/prelude.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "evm_internal.h"
#endif
bool zneq_bool(bool zx, bool zy)
{
  bool z8zE1;
  bool z2zE3355;
  z2zE3355 = eq_bool(zx, zy);
  z8zE1 = not(z2zE3355);
end_function_3579: ;
  return z8zE1;
end_block_exception_3580: ;

  return false;
}

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(sail_fixed_bytes_32 zx, sail_fixed_bytes_32 zy)
{
  bool z8zE2;
  bool z2zE3354;
  z2zE3354 = eq_fixed_bytes_32(zy, zx);
  z8zE2 = not(z2zE3354);
end_function_3577: ;
  return z8zE2;
end_block_exception_3578: ;

  return false;
}

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(sail_fixed_bytes_20 zx, sail_fixed_bytes_20 zy)
{
  bool z8zE3;
  bool z2zE3354;
  z2zE3354 = eq_fixed_bytes_20(zy, zx);
  z8zE3 = not(z2zE3354);
end_function_3577: ;
  return z8zE3;
end_block_exception_3578: ;

  return false;
}

uint64_t zfork_index(enum zFork zf)
{
  uint64_t z8zE5;
  uint64_t z3zE2955;
  {
    if ((zFrontier != zf)) goto case_3572;
    z3zE2955 = UINT64_C(0);
    goto finish_match_3559;
  }
case_3572: ;
  {
    if ((zHomestead != zf)) goto case_3571;
    z3zE2955 = UINT64_C(1);
    goto finish_match_3559;
  }
case_3571: ;
  {
    if ((zByzzantium != zf)) goto case_3570;
    z3zE2955 = UINT64_C(2);
    goto finish_match_3559;
  }
case_3570: ;
  {
    if ((zConstantinople != zf)) goto case_3569;
    z3zE2955 = UINT64_C(3);
    goto finish_match_3559;
  }
case_3569: ;
  {
    if ((zIstanbul != zf)) goto case_3568;
    z3zE2955 = UINT64_C(4);
    goto finish_match_3559;
  }
case_3568: ;
  {
    if ((zBerlin != zf)) goto case_3567;
    z3zE2955 = UINT64_C(5);
    goto finish_match_3559;
  }
case_3567: ;
  {
    if ((zLondon != zf)) goto case_3566;
    z3zE2955 = UINT64_C(6);
    goto finish_match_3559;
  }
case_3566: ;
  {
    if ((zParis != zf)) goto case_3565;
    z3zE2955 = UINT64_C(7);
    goto finish_match_3559;
  }
case_3565: ;
  {
    if ((zShanghai != zf)) goto case_3564;
    z3zE2955 = UINT64_C(8);
    goto finish_match_3559;
  }
case_3564: ;
  {
    if ((zCancun != zf)) goto case_3563;
    z3zE2955 = UINT64_C(9);
    goto finish_match_3559;
  }
case_3563: ;
  {
    if ((zPrague != zf)) goto case_3562;
    z3zE2955 = UINT64_C(10);
    goto finish_match_3559;
  }
case_3562: ;
  {
    if ((zOsaka != zf)) goto case_3561;
    z3zE2955 = UINT64_C(11);
    goto finish_match_3559;
  }
case_3561: ;
  {
    /* complete */
    z3zE2955 = UINT64_C(12);
    goto finish_match_3559;
  }
case_3560: ;
finish_match_3559: ;
  z8zE5 = z3zE2955;
end_function_3573: ;
  return z8zE5;
end_block_exception_3574: ;

  return UINT64_C(0xdeadc0de);
}

bool zfork_lt(enum zFork za, enum zFork zb)
{
  bool z8zE6;
  uint64_t z2zE3351;
  z2zE3351 = zfork_index(za);
  uint64_t z2zE3352;
  z2zE3352 = zfork_index(zb);
  z8zE6 = (z2zE3351 < z2zE3352);
end_function_3557: ;
  return z8zE6;
end_block_exception_3558: ;

  return false;
}

bool zfork_gteq(enum zFork za, enum zFork zb)
{
  bool z8zE7;
  uint64_t z2zE3349;
  z2zE3349 = zfork_index(zb);
  uint64_t z2zE3350;
  z2zE3350 = zfork_index(za);
  z8zE7 = (!(z2zE3350 < z2zE3349));
end_function_3555: ;
  return z8zE7;
end_block_exception_3556: ;

  return false;
}

sail_u256 zU256(sail_u256 zvalue)
{
  sail_u256 z8zE8;
  z8zE8 = zvalue;
end_function_3545: ;
  return z8zE8;
end_block_exception_3546: ;

  return u256_zero();
}

sail_fixed_bytes_32 zB256(sail_fixed_bytes_32 zbytes)
{
  sail_fixed_bytes_32 z8zE9;
  z8zE9 = zbytes;
end_function_3543: ;
  return z8zE9;
end_block_exception_3544: ;

  return fixed_bytes_32_zero();
}

uint64_t zhash_little_endian_index(uint64_t zindex)
{
  uint64_t z8zE10;
  {    z8zE10 = (UINT64_C(31) - zindex);
  }
end_function_3541: ;
  return z8zE10;
end_block_exception_3542: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zaddress_little_endian_index(uint64_t zindex)
{
  uint64_t z8zE14;
  {    z8zE14 = (UINT64_C(19) - zindex);
  }
end_function_3529: ;
  return z8zE14;
end_block_exception_3530: ;

  return UINT64_C(0xdeadc0de);
}

void create_letbind_0(void) {

  sail_u256 z3zE0;
  z3zE0 = u256_of_fbits(UINT64_C(0));
  zZERO_WORD = z3zE0;
let_end_17: ;
}
void kill_letbind_0(void) {
}

void create_letbind_1(void) {

  sail_fixed_bytes_20 z3zE2;
  zz5vecz8z5bv8z9 z3zE1;
  CREATE(zz5vecz8z5bv8z9)(&z3zE1);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE1, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(0), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1, z3zE1, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE767 = 0; z8zE767 < 20; ++z8zE767) {
    z3zE2.bytes[z8zE767] = (uint8_t)(z3zE1.data[z8zE767] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE1);
  zZERO_ADDRESS = z3zE2;
let_end_18: ;
}
void kill_letbind_1(void) {
}

void create_letbind_2(void) {

  sail_fixed_bytes_32 z3zE4;
  zz5vecz8z5bv8z9 z3zE3;
  CREATE(zz5vecz8z5bv8z9)(&z3zE3);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE3, INT64_C(32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(0), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(19), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(20), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(21), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(22), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(23), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(24), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(25), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(26), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(27), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(28), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(29), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(30), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE3, z3zE3, INT64_C(31), UINT64_C(0x00));
  for (size_t z8zE768 = 0; z8zE768 < 32; ++z8zE768) {
    z3zE4.bytes[z8zE768] = (uint8_t)(z3zE3.data[z8zE768] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE3);
  zZERO_HASH = z3zE4;
let_end_19: ;
}
void kill_letbind_2(void) {
}

void create_letbind_3(void) {

  sail_u256 z3zE5;
  z3zE5 = u256_of_fbits(UINT64_C(0));
  zWORD_ZERO = z3zE5;
let_end_20: ;
}
void kill_letbind_3(void) {
}

void create_letbind_4(void) {

  sail_u256 z3zE6;
  z3zE6 = u256_of_fbits(UINT64_C(1));
  zWORD_ONE = z3zE6;
let_end_21: ;
}
void kill_letbind_4(void) {
}

void create_letbind_5(void) {

  sail_u256 z3zE7;
  z3zE7 = ((sail_u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}});
  zWORD_ALL_ONES = z3zE7;
let_end_22: ;
}
void kill_letbind_5(void) {
}

void create_letbind_6(void) {

  sail_u256 z3zE8;
  z3zE8 = ((sail_u256){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(9223372036854775808)}});
  zWORD_SIGN_BIT = z3zE8;
let_end_23: ;
}
void kill_letbind_6(void) {
}

sail_u256 zword_add_word(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE16;
  sail_u256 z2zE3328;
  {    z2zE3328 = u256_add(zleft, zright);
  }
  z8zE16 = zU256(z2zE3328);
end_function_3523: ;
  return z8zE16;
end_block_exception_3524: ;

  return u256_zero();
}

sail_u256 zword_sub_word(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE17;
  sail_u256 z2zE3326;
  {    z2zE3326 = u256_sub(zleft, zright);
  }
  z8zE17 = zU256(z2zE3326);
end_function_3521: ;
  return z8zE17;
end_block_exception_3522: ;

  return u256_zero();
}

sail_u256 zword_and(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE18;
  sail_u256 z2zE3324;
  {
    sail_u256 z2zE3323;
    {
      sail_u256 z2zE3321;
      z2zE3321 = zleft;
      sail_u256 z2zE3322;
      z2zE3322 = zright;
      z2zE3323 = u256_and(z2zE3321, z2zE3322);
    }
    z2zE3324 = z2zE3323;
  }
  z8zE18 = zU256(z2zE3324);
end_function_3519: ;
  return z8zE18;
end_block_exception_3520: ;

  return u256_zero();
}

sail_u256 zword_or(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE19;
  sail_u256 z2zE3320;
  {
    sail_u256 z2zE3319;
    {
      sail_u256 z2zE3317;
      z2zE3317 = zleft;
      sail_u256 z2zE3318;
      z2zE3318 = zright;
      z2zE3319 = u256_or(z2zE3317, z2zE3318);
    }
    z2zE3320 = z2zE3319;
  }
  z8zE19 = zU256(z2zE3320);
end_function_3517: ;
  return z8zE19;
end_block_exception_3518: ;

  return u256_zero();
}

sail_u256 zword_xor(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE20;
  sail_u256 z2zE3316;
  {
    sail_u256 z2zE3315;
    {
      sail_u256 z2zE3313;
      z2zE3313 = zleft;
      sail_u256 z2zE3314;
      z2zE3314 = zright;
      z2zE3315 = u256_xor(z2zE3313, z2zE3314);
    }
    z2zE3316 = z2zE3315;
  }
  z8zE20 = zU256(z2zE3316);
end_function_3515: ;
  return z8zE20;
end_block_exception_3516: ;

  return u256_zero();
}

sail_u256 zword_not(sail_u256 zvalue)
{
  sail_u256 z8zE21;
  sail_u256 z2zE3312;
  {
    sail_u256 z2zE3311;
    {
      sail_u256 z2zE3310;
      z2zE3310 = zvalue;
      z2zE3311 = u256_not(z2zE3310);
    }
    z2zE3312 = z2zE3311;
  }
  z8zE21 = zU256(z2zE3312);
end_function_3513: ;
  return z8zE21;
end_block_exception_3514: ;

  return u256_zero();
}

uint64_t zword_bit(sail_u256 zvalue, uint64_t zindex)
{
  uint64_t z8zE22;
  sail_u256 z2zE3309;
  z2zE3309 = zvalue;
  z8zE22 = u256_bit(z2zE3309, zindex);
end_function_3511: ;
  return z8zE22;
end_block_exception_3512: ;

  return UINT64_C(0);
}

uint64_t zword_low_byte(sail_u256 zvalue)
{
  uint64_t z8zE23;
  z8zE23 = (UINT64_C(0xFF) & u256_extract_u64(zvalue, (uint64_t)(UINT64_C(0))));
end_function_3509: ;
  return z8zE23;
end_block_exception_3510: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_shift_right_one(sail_u256 zvalue)
{
  sail_u256 z8zE24;
  sail_u256 z2zE3308;
  z2zE3308 = u256_div_u64(zvalue, UINT64_C(2));
  z8zE24 = zU256(z2zE3308);
end_function_3507: ;
  return z8zE24;
end_block_exception_3508: ;

  return u256_zero();
}

sail_u256 zword_of_bool(bool zb)
{
  sail_u256 z8zE25;
  if (zb) {  z8zE25 = zWORD_ONE;  } else {  z8zE25 = zWORD_ZERO;  }
end_function_3505: ;
  return z8zE25;
end_block_exception_3506: ;

  return u256_zero();
}

bool zword_is_zzero(sail_u256 zw)
{
  bool z8zE26;
  z8zE26 = eq_u256(zw, zWORD_ZERO);
end_function_3503: ;
  return z8zE26;
end_block_exception_3504: ;

  return false;
}

bool zword_nonzzero(sail_u256 zw)
{
  bool z8zE27;
  bool z2zE3307;
  z2zE3307 = zword_is_zzero(zw);
  z8zE27 = not(z2zE3307);
end_function_3501: ;
  return z8zE27;
end_block_exception_3502: ;

  return false;
}

bool zword_ult(sail_u256 za, sail_u256 zb)
{
  bool z8zE28;
  z8zE28 = u256_lt(za, zb);
end_function_3499: ;
  return z8zE28;
end_block_exception_3500: ;

  return false;
}

bool zword_ule(sail_u256 za, sail_u256 zb)
{
  bool z8zE29;
  bool z2zE3306;
  z2zE3306 = zword_ult(zb, za);
  z8zE29 = not(z2zE3306);
end_function_3497: ;
  return z8zE29;
end_block_exception_3498: ;

  return false;
}

uint64_t zbyte_bit_length(uint64_t zvalue)
{
  uint64_t z8zE30;
  bool z2zE3299;
  z2zE3299 = (zvalue < UINT64_C(16));
  if (z2zE3299) {
    bool z2zE3300;
    z2zE3300 = (zvalue < UINT64_C(4));
    if (z2zE3300) {
      bool z2zE3301;
      z2zE3301 = (zvalue < UINT64_C(2));
      if (z2zE3301) {  z8zE30 = zvalue;  } else {  z8zE30 = UINT64_C(2);  }
    } else {
      bool z2zE3302;
      z2zE3302 = (zvalue < UINT64_C(8));
      if (z2zE3302) {  z8zE30 = UINT64_C(3);  } else {  z8zE30 = UINT64_C(4);  }
    }
  } else {
    bool z2zE3303;
    z2zE3303 = (zvalue < UINT64_C(64));
    if (z2zE3303) {
      bool z2zE3304;
      z2zE3304 = (zvalue < UINT64_C(32));
      if (z2zE3304) {  z8zE30 = UINT64_C(5);  } else {  z8zE30 = UINT64_C(6);  }
    } else {
      bool z2zE3305;
      z2zE3305 = (zvalue < UINT64_C(128));
      if (z2zE3305) {  z8zE30 = UINT64_C(7);  } else {  z8zE30 = UINT64_C(8);  }
    }
  }
end_function_3495: ;
  return z8zE30;
end_block_exception_3496: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zu16_bit_length(uint64_t zvalue)
{
  uint64_t z8zE31;
  bool z2zE3296;
  z2zE3296 = (zvalue < UINT64_C(256));
  if (z2zE3296) {  z8zE31 = zbyte_bit_length(zvalue);  } else {
    uint64_t z2zE3298;
    {
      uint64_t z2zE3297;
      {    z2zE3297 = (zvalue / UINT64_C(256));
      }
      z2zE3298 = zbyte_bit_length(z2zE3297);
    }
    {    z8zE31 = (UINT64_C(8) + z2zE3298);
    }
  }
end_function_3493: ;
  return z8zE31;
end_block_exception_3494: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zu32_bit_length(uint64_t zvalue)
{
  uint64_t z8zE32;
  bool z2zE3293;
  z2zE3293 = (zvalue < UINT64_C(65536));
  if (z2zE3293) {  z8zE32 = zu16_bit_length(zvalue);  } else {
    uint64_t z2zE3295;
    {
      uint64_t z2zE3294;
      {    z2zE3294 = (zvalue / UINT64_C(65536));
      }
      z2zE3295 = zu16_bit_length(z2zE3294);
    }
    {    z8zE32 = (UINT64_C(16) + z2zE3295);
    }
  }
end_function_3491: ;
  return z8zE32;
end_block_exception_3492: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zu64_bit_length(uint64_t zvalue)
{
  uint64_t z8zE33;
  bool z2zE3290;
  z2zE3290 = (zvalue < UINT64_C(4294967296));
  if (z2zE3290) {  z8zE33 = zu32_bit_length(zvalue);  } else {
    uint64_t z2zE3292;
    {
      uint64_t z2zE3291;
      {    z2zE3291 = (zvalue / UINT64_C(4294967296));
      }
      z2zE3292 = zu32_bit_length(z2zE3291);
    }
    {    z8zE33 = (UINT64_C(32) + z2zE3292);
    }
  }
end_function_3489: ;
  return z8zE33;
end_block_exception_3490: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_bit_length(sail_u256 zvalue)
{
  uint64_t z8zE34;
  uint64_t zlimb3;
  {
    uint64_t z2zE3289;
    z2zE3289 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(192)));
    zlimb3 = ((uint64_t) z2zE3289);
  }
  bool z2zE3279;
  z2zE3279 = (zlimb3 != UINT64_C(0));
  if (z2zE3279) {
    uint64_t z2zE3280;
    z2zE3280 = zu64_bit_length(zlimb3);
    {    z8zE34 = (UINT64_C(192) + z2zE3280);
    }
  } else {
    uint64_t zlimb2;
    {
      uint64_t z2zE3288;
      z2zE3288 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(128)));
      zlimb2 = ((uint64_t) z2zE3288);
    }
    bool z2zE3281;
    z2zE3281 = (zlimb2 != UINT64_C(0));
    if (z2zE3281) {
      uint64_t z2zE3282;
      z2zE3282 = zu64_bit_length(zlimb2);
      {    z8zE34 = (UINT64_C(128) + z2zE3282);
      }
    } else {
      uint64_t zlimb1;
      {
        uint64_t z2zE3287;
        z2zE3287 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(64)));
        zlimb1 = ((uint64_t) z2zE3287);
      }
      bool z2zE3283;
      z2zE3283 = (zlimb1 != UINT64_C(0));
      if (z2zE3283) {
        uint64_t z2zE3284;
        z2zE3284 = zu64_bit_length(zlimb1);
        {    z8zE34 = (UINT64_C(64) + z2zE3284);
        }
      } else {
        uint64_t z2zE3286;
        {
          uint64_t z2zE3285;
          z2zE3285 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(0)));
          z2zE3286 = ((uint64_t) z2zE3285);
        }
        z8zE34 = zu64_bit_length(z2zE3286);
      }
    }
  }
end_function_3487: ;
  return z8zE34;
end_block_exception_3488: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_mul_word(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE35;
  z8zE35 = u256_mul(za, zb);
end_function_3485: ;
  return z8zE35;
end_block_exception_3486: ;

  return u256_zero();
}

sail_u256 zword_div_word(sail_u256 zdividend, sail_u256 zdivisor)
{
  sail_u256 z8zE36;
  bool z2zE3276;
  z2zE3276 = u256_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE3276) {  z8zE36 = zWORD_ZERO;  } else {
    sail_u256 z2zE3277;
    z2zE3277 = u256_div(zdividend, zdivisor);
    z8zE36 = zU256(z2zE3277);
  }
end_function_3483: ;
  return z8zE36;
end_block_exception_3484: ;

  return u256_zero();
}

sail_u256 zword_mod_word(sail_u256 zdividend, sail_u256 zdivisor)
{
  sail_u256 z8zE37;
  bool z2zE3274;
  z2zE3274 = u256_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE3274) {  z8zE37 = zWORD_ZERO;  } else {
    sail_u256 z2zE3275;
    z2zE3275 = u256_mod(zdividend, zdivisor);
    z8zE37 = zU256(z2zE3275);
  }
end_function_3481: ;
  return z8zE37;
end_block_exception_3482: ;

  return u256_zero();
}

sail_u256 zword_shift_left(sail_u256 zvalue, uint64_t zamount)
{
  sail_u256 z8zE38;
  sail_u256 z2zE3273;
  {
    sail_u256 z2zE3272;
    {
      sail_u256 z2zE3271;
      z2zE3271 = zvalue;
      z2zE3272 = u256_shiftl_u64(z2zE3271, zamount);
    }
    z2zE3273 = z2zE3272;
  }
  z8zE38 = zU256(z2zE3273);
end_function_3477: ;
  return z8zE38;
end_block_exception_3478: ;

  return u256_zero();
}

sail_u256 zword_shift_right(sail_u256 zvalue, uint64_t zamount)
{
  sail_u256 z8zE39;
  sail_u256 z2zE3270;
  {
    sail_u256 z2zE3269;
    z2zE3269 = zvalue;
    z2zE3270 = u256_shiftr_u64(z2zE3269, zamount);
  }
  z8zE39 = z2zE3270;
end_function_3475: ;
  return z8zE39;
end_block_exception_3476: ;

  return u256_zero();
}

uint64_t zword_byte_length(sail_u256 zvalue)
{
  uint64_t z8zE40;
  uint64_t zbit_length;
  zbit_length = zword_bit_length(zvalue);
  bool z2zE3267;
  z2zE3267 = (zbit_length == UINT64_C(0));
  if (z2zE3267) {  z8zE40 = UINT64_C(0);  } else {
    uint64_t z2zE3268;
    {    z2zE3268 = (zbit_length + UINT64_C(7));
    }
    {    z8zE40 = (z2zE3268 / UINT64_C(8));
    }
  }
end_function_3473: ;
  return z8zE40;
end_block_exception_3474: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_arithmetic_shift_right(sail_u256 zvalue, uint64_t zamount)
{
  sail_u256 z8zE41;
  sail_u256 zshifted;
  zshifted = zword_shift_right(zvalue, zamount);
  bool z2zE3265;
  {
    uint64_t z2zE3264;
    z2zE3264 = zword_bit(zvalue, UINT64_C(255));
    z2zE3265 = (z2zE3264 == UINT64_C(0b1));
  }
  if (z2zE3265) {
    sail_u256 zsign_fill;
    {
      uint64_t z2zE3266;
      {    z2zE3266 = (UINT64_C(256) - zamount);
      }
      zsign_fill = zword_shift_left(zWORD_ALL_ONES, z2zE3266);
    }
    z8zE41 = zword_or(zshifted, zsign_fill);
  } else {  z8zE41 = zshifted;  }
end_function_3471: ;
  return z8zE41;
end_block_exception_3472: ;

  return u256_zero();
}

sail_u256 zword_negate(sail_u256 zvalue)
{
  sail_u256 z8zE43;
  z8zE43 = zword_sub_word(zWORD_ZERO, zvalue);
end_function_3465: ;
  return z8zE43;
end_block_exception_3466: ;

  return u256_zero();
}

sail_u256 zword_abs(sail_u256 zvalue)
{
  sail_u256 z8zE44;
  bool z2zE3260;
  {
    uint64_t z2zE3259;
    z2zE3259 = zword_bit(zvalue, UINT64_C(255));
    z2zE3260 = (z2zE3259 == UINT64_C(0b1));
  }
  if (z2zE3260) {  z8zE44 = zword_negate(zvalue);  } else {  z8zE44 = zvalue;  }
end_function_3463: ;
  return z8zE44;
end_block_exception_3464: ;

  return u256_zero();
}

bool zword_slt(sail_u256 za, sail_u256 zb)
{
  bool z8zE45;
  bool za_neg;
  {
    uint64_t z2zE3258;
    z2zE3258 = zword_bit(za, UINT64_C(255));
    za_neg = (z2zE3258 == UINT64_C(0b1));
  }
  bool zb_neg;
  {
    uint64_t z2zE3257;
    z2zE3257 = zword_bit(zb, UINT64_C(255));
    zb_neg = (z2zE3257 == UINT64_C(0b1));
  }
  if (za_neg) {  if (zb_neg) {  z8zE45 = zword_ult(za, zb);  } else {  z8zE45 = true;  }  } else if (zb_neg) {
    z8zE45 = false;
  } else {  z8zE45 = zword_ult(za, zb);  }
end_function_3461: ;
  return z8zE45;
end_block_exception_3462: ;

  return false;
}

sail_u256 zalu_add(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE46;
  z8zE46 = zword_add_word(za, zb);
end_function_3459: ;
  return z8zE46;
end_block_exception_3460: ;

  return u256_zero();
}

sail_u256 zalu_sub(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE47;
  z8zE47 = zword_sub_word(za, zb);
end_function_3457: ;
  return z8zE47;
end_block_exception_3458: ;

  return u256_zero();
}

sail_u256 zalu_mul(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE48;
  z8zE48 = zword_mul_word(za, zb);
end_function_3455: ;
  return z8zE48;
end_block_exception_3456: ;

  return u256_zero();
}

sail_u256 zalu_div(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE49;
  z8zE49 = zword_div_word(za, zb);
end_function_3453: ;
  return z8zE49;
end_block_exception_3454: ;

  return u256_zero();
}

sail_u256 zalu_mod(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE50;
  z8zE50 = zword_mod_word(za, zb);
end_function_3451: ;
  return z8zE50;
end_block_exception_3452: ;

  return u256_zero();
}

sail_u256 zalu_sdiv(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE51;
  bool z2zE3249;
  z2zE3249 = zword_is_zzero(zb);
  if (z2zE3249) {  z8zE51 = zWORD_ZERO;  } else {
    sail_u256 zquotient;
    {
      sail_u256 z2zE3255;
      z2zE3255 = zword_abs(za);
      sail_u256 z2zE3256;
      z2zE3256 = zword_abs(zb);
      zquotient = zword_div_word(z2zE3255, z2zE3256);
    }
    bool z2zE3254;
    {
      bool z2zE3252;
      {
        uint64_t z2zE3250;
        z2zE3250 = zword_bit(za, UINT64_C(255));
        z2zE3252 = (z2zE3250 == UINT64_C(0b1));
      }
      bool z2zE3253;
      {
        uint64_t z2zE3251;
        z2zE3251 = zword_bit(zb, UINT64_C(255));
        z2zE3253 = (z2zE3251 == UINT64_C(0b1));
      }
      z2zE3254 = zneq_bool(z2zE3252, z2zE3253);
    }
    if (z2zE3254) {  z8zE51 = zword_negate(zquotient);  } else {  z8zE51 = zquotient;  }
  }
end_function_3449: ;
  return z8zE51;
end_block_exception_3450: ;

  return u256_zero();
}

sail_u256 zalu_smod(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE52;
  bool z2zE3244;
  z2zE3244 = zword_is_zzero(zb);
  if (z2zE3244) {  z8zE52 = zWORD_ZERO;  } else {
    sail_u256 zremainder;
    {
      sail_u256 z2zE3247;
      z2zE3247 = zword_abs(za);
      sail_u256 z2zE3248;
      z2zE3248 = zword_abs(zb);
      zremainder = zword_mod_word(z2zE3247, z2zE3248);
    }
    bool z2zE3246;
    {
      uint64_t z2zE3245;
      z2zE3245 = zword_bit(za, UINT64_C(255));
      z2zE3246 = (z2zE3245 == UINT64_C(0b1));
    }
    if (z2zE3246) {  z8zE52 = zword_negate(zremainder);  } else {  z8zE52 = zremainder;  }
  }
end_function_3447: ;
  return z8zE52;
end_block_exception_3448: ;

  return u256_zero();
}

sail_u256 zalu_addmod(sail_u256 za, sail_u256 zb, sail_u256 zn)
{
  sail_u256 z8zE53;
  z8zE53 = u256_addmod(za, zb, zn);
  return z8zE53;
}

sail_u256 zalu_mulmod(sail_u256 za, sail_u256 zb, sail_u256 zn)
{
  sail_u256 z8zE54;
  z8zE54 = u256_mulmod(za, zb, zn);
  return z8zE54;
}

sail_u256 zalu_exp(sail_u256 zbase, sail_u256 zexponent)
{
  sail_u256 z8zE55;
  sail_u256 zresult;
  zresult = zWORD_ONE;
  sail_u256 zb;
  zb = zbase;
  sail_u256 ze;
  ze = zexponent;
  int64_t z3zE2911;
  {    z3zE2911 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2912;
  {    z3zE2912 = (int64_t)(UINT64_C(255));
  }
  int64_t z3zE2913;
  {    z3zE2913 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z_step;
    z_step = z3zE2911;
    unit z3zE2916;
  for_start_3439: ;
    {
      if ((z3zE2912 < z_step)) goto for_end_3440;
      bool z2zE3237;
      {
        uint64_t z2zE3236;
        z2zE3236 = zword_bit(ze, UINT64_C(0));
        z2zE3237 = (z2zE3236 == UINT64_C(0b1));
      }
      unit z3zE2915;
      if (z2zE3237) {
        zresult = zword_mul_word(zresult, zb);
        z3zE2915 = UNIT;
      } else {  z3zE2915 = UNIT;  }
      zb = zword_mul_word(zb, zb);
      unit z3zE2914;
      z3zE2914 = UNIT;
      ze = zword_shift_right_one(ze);
      z3zE2916 = UNIT;
      z_step = (z_step + z3zE2913);
      goto for_start_3439;
    }
  for_end_3440: ;
  }
  unit z3zE2917;
  z3zE2917 = UNIT;
  z8zE55 = zresult;
end_function_3441: ;
  return z8zE55;
end_block_exception_3442: ;

  return u256_zero();
}

sail_u256 zalu_signextend(sail_u256 zbyte_index, sail_u256 zvalue)
{
  sail_u256 z8zE56;
  bool z2zE3228;
  z2zE3228 = u256_lt_u64(zbyte_index, UINT64_C(32));
  if (z2zE3228) {
    uint64_t zindex;
    zindex = u256_to_u64(zbyte_index);
    uint64_t zwidth;
    {
      uint64_t z2zE3235;
      {    z2zE3235 = (zindex * UINT64_C(8));
      }
      {    zwidth = (z2zE3235 + UINT64_C(8));
      }
    }
    uint64_t zsign_shift;
    {
      uint64_t z2zE3234;
      {    z2zE3234 = (zindex * UINT64_C(8));
      }
      {    zsign_shift = (z2zE3234 + UINT64_C(7));
      }
    }
    bool zsign_set;
    {
      sail_u256 z2zE3233;
      {
        sail_u256 z2zE3232;
        z2zE3232 = zword_shift_right(zvalue, zsign_shift);
        z2zE3233 = zword_and(z2zE3232, zWORD_ONE);
      }
      zsign_set = eq_u256(z2zE3233, zWORD_ONE);
    }
    sail_u256 zlow_mask;
    {
      sail_u256 z2zE3231;
      z2zE3231 = zword_shift_left(zWORD_ONE, zwidth);
      zlow_mask = zword_sub_word(z2zE3231, zWORD_ONE);
    }
    if (zsign_set) {
      sail_u256 z2zE3229;
      z2zE3229 = zword_and(zvalue, zlow_mask);
      sail_u256 z2zE3230;
      z2zE3230 = zword_not(zlow_mask);
      z8zE56 = zword_or(z2zE3229, z2zE3230);
    } else {  z8zE56 = zword_and(zvalue, zlow_mask);  }
  } else {  z8zE56 = zvalue;  }
end_function_3437: ;
  return z8zE56;
end_block_exception_3438: ;

  return u256_zero();
}

sail_u256 zalu_lt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE57;
  bool z2zE3227;
  z2zE3227 = zword_ult(za, zb);
  z8zE57 = zword_of_bool(z2zE3227);
end_function_3435: ;
  return z8zE57;
end_block_exception_3436: ;

  return u256_zero();
}

sail_u256 zalu_gt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE58;
  bool z2zE3226;
  z2zE3226 = zword_ult(zb, za);
  z8zE58 = zword_of_bool(z2zE3226);
end_function_3433: ;
  return z8zE58;
end_block_exception_3434: ;

  return u256_zero();
}

sail_u256 zalu_slt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE59;
  bool z2zE3225;
  z2zE3225 = zword_slt(za, zb);
  z8zE59 = zword_of_bool(z2zE3225);
end_function_3431: ;
  return z8zE59;
end_block_exception_3432: ;

  return u256_zero();
}

sail_u256 zalu_sgt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE60;
  bool z2zE3224;
  z2zE3224 = zword_slt(zb, za);
  z8zE60 = zword_of_bool(z2zE3224);
end_function_3429: ;
  return z8zE60;
end_block_exception_3430: ;

  return u256_zero();
}

sail_u256 zalu_eq(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE61;
  bool z2zE3223;
  z2zE3223 = eq_u256(za, zb);
  z8zE61 = zword_of_bool(z2zE3223);
end_function_3427: ;
  return z8zE61;
end_block_exception_3428: ;

  return u256_zero();
}

sail_u256 zalu_iszzero(sail_u256 za)
{
  sail_u256 z8zE62;
  bool z2zE3222;
  z2zE3222 = zword_is_zzero(za);
  z8zE62 = zword_of_bool(z2zE3222);
end_function_3425: ;
  return z8zE62;
end_block_exception_3426: ;

  return u256_zero();
}

sail_u256 zalu_and(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE63;
  z8zE63 = zword_and(za, zb);
end_function_3423: ;
  return z8zE63;
end_block_exception_3424: ;

  return u256_zero();
}

sail_u256 zalu_or(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE64;
  z8zE64 = zword_or(za, zb);
end_function_3421: ;
  return z8zE64;
end_block_exception_3422: ;

  return u256_zero();
}

sail_u256 zalu_xor(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE65;
  z8zE65 = zword_xor(za, zb);
end_function_3419: ;
  return z8zE65;
end_block_exception_3420: ;

  return u256_zero();
}

sail_u256 zalu_not(sail_u256 za)
{
  sail_u256 z8zE66;
  z8zE66 = zword_not(za);
end_function_3417: ;
  return z8zE66;
end_block_exception_3418: ;

  return u256_zero();
}

sail_u256 zalu_byte(sail_u256 zi, sail_u256 zx)
{
  sail_u256 z8zE67;
  bool z2zE3218;
  z2zE3218 = u256_lt_u64(zi, UINT64_C(32));
  if (z2zE3218) {
    uint64_t zindex;
    zindex = u256_to_u64(zi);
    uint64_t zshift;
    {
      uint64_t z2zE3221;
      {    z2zE3221 = (UINT64_C(31) - zindex);
      }
      {    zshift = (z2zE3221 * UINT64_C(8));
      }
    }
    uint64_t z2zE3220;
    {
      sail_u256 z2zE3219;
      z2zE3219 = zword_shift_right(zx, zshift);
      z2zE3220 = zword_low_byte(z2zE3219);
    }
    z8zE67 = u256_of_fbits(((uint64_t) z2zE3220));
  } else {  z8zE67 = zWORD_ZERO;  }
end_function_3415: ;
  return z8zE67;
end_block_exception_3416: ;

  return u256_zero();
}

sail_u256 zalu_shl(sail_u256 zshift_amt, sail_u256 zv)
{
  sail_u256 z8zE68;
  bool z2zE3217;
  z2zE3217 = u256_lt_u64(zshift_amt, UINT64_C(256));
  if (z2zE3217) {
    {
      uint64_t z3zE3007;
      z3zE3007 = u256_to_u64(zshift_amt);
      z8zE68 = zword_shift_left(zv, z3zE3007);
    }
  } else {  z8zE68 = zWORD_ZERO;  }
end_function_3413: ;
  return z8zE68;
end_block_exception_3414: ;

  return u256_zero();
}

sail_u256 zalu_shr(sail_u256 zshift_amt, sail_u256 zv)
{
  sail_u256 z8zE69;
  bool z2zE3216;
  z2zE3216 = u256_lt_u64(zshift_amt, UINT64_C(256));
  if (z2zE3216) {
    {
      uint64_t z3zE3008;
      z3zE3008 = u256_to_u64(zshift_amt);
      z8zE69 = zword_shift_right(zv, z3zE3008);
    }
  } else {  z8zE69 = zWORD_ZERO;  }
end_function_3411: ;
  return z8zE69;
end_block_exception_3412: ;

  return u256_zero();
}

sail_u256 zalu_sar(sail_u256 zshift_amt, sail_u256 zv)
{
  sail_u256 z8zE70;
  bool z2zE3213;
  z2zE3213 = u256_lt_u64(zshift_amt, UINT64_C(256));
  if (z2zE3213) {
    {
      uint64_t z3zE3009;
      z3zE3009 = u256_to_u64(zshift_amt);
      z8zE70 = zword_arithmetic_shift_right(zv, z3zE3009);
    }
  } else {
    bool z2zE3215;
    {
      uint64_t z2zE3214;
      z2zE3214 = zword_bit(zv, UINT64_C(255));
      z2zE3215 = (z2zE3214 == UINT64_C(0b1));
    }
    if (z2zE3215) {  z8zE70 = zWORD_ALL_ONES;  } else {  z8zE70 = zWORD_ZERO;  }
  }
end_function_3409: ;
  return z8zE70;
end_block_exception_3410: ;

  return u256_zero();
}

sail_u256 zalu_clzz(sail_u256 zx)
{
  sail_u256 z8zE71;
  uint64_t z2zE3212;
  {
    uint64_t z2zE3211;
    z2zE3211 = zword_bit_length(zx);
    {    z2zE3212 = (UINT64_C(256) - z2zE3211);
    }
  }
  z8zE71 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3212);
end_function_3407: ;
  return z8zE71;
end_block_exception_3408: ;

  return u256_zero();
}

