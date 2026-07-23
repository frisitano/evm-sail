/* Generated from sail/prelude.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "evm_internal.h"
#endif
bool zneq_bool(bool zx, bool zy)
{
  bool z8zE1;
  bool z2zE4096;
  z2zE4096 = eq_bool(zx, zy);
  z8zE1 = not(z2zE4096);
end_function_4004: ;
  return z8zE1;
end_block_exception_4005: ;

  return false;
}

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(sail_fixed_bytes_32 zx, sail_fixed_bytes_32 zy)
{
  bool z8zE2;
  bool z2zE4095;
  z2zE4095 = eq_fixed_bytes_32(zy, zx);
  z8zE2 = not(z2zE4095);
end_function_4002: ;
  return z8zE2;
end_block_exception_4003: ;

  return false;
}

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(sail_fixed_bytes_20 zx, sail_fixed_bytes_20 zy)
{
  bool z8zE3;
  bool z2zE4095;
  z2zE4095 = eq_fixed_bytes_20(zy, zx);
  z8zE3 = not(z2zE4095);
end_function_4002: ;
  return z8zE3;
end_block_exception_4003: ;

  return false;
}

uint64_t zfork_index(enum zFork zf)
{
  uint64_t z8zE5;
  uint64_t z3zE3782;
  {
    if ((zFrontier != zf)) goto case_3997;
    z3zE3782 = UINT64_C(0);
    goto finish_match_3984;
  }
case_3997: ;
  {
    if ((zHomestead != zf)) goto case_3996;
    z3zE3782 = UINT64_C(1);
    goto finish_match_3984;
  }
case_3996: ;
  {
    if ((zByzzantium != zf)) goto case_3995;
    z3zE3782 = UINT64_C(2);
    goto finish_match_3984;
  }
case_3995: ;
  {
    if ((zConstantinople != zf)) goto case_3994;
    z3zE3782 = UINT64_C(3);
    goto finish_match_3984;
  }
case_3994: ;
  {
    if ((zIstanbul != zf)) goto case_3993;
    z3zE3782 = UINT64_C(4);
    goto finish_match_3984;
  }
case_3993: ;
  {
    if ((zBerlin != zf)) goto case_3992;
    z3zE3782 = UINT64_C(5);
    goto finish_match_3984;
  }
case_3992: ;
  {
    if ((zLondon != zf)) goto case_3991;
    z3zE3782 = UINT64_C(6);
    goto finish_match_3984;
  }
case_3991: ;
  {
    if ((zParis != zf)) goto case_3990;
    z3zE3782 = UINT64_C(7);
    goto finish_match_3984;
  }
case_3990: ;
  {
    if ((zShanghai != zf)) goto case_3989;
    z3zE3782 = UINT64_C(8);
    goto finish_match_3984;
  }
case_3989: ;
  {
    if ((zCancun != zf)) goto case_3988;
    z3zE3782 = UINT64_C(9);
    goto finish_match_3984;
  }
case_3988: ;
  {
    if ((zPrague != zf)) goto case_3987;
    z3zE3782 = UINT64_C(10);
    goto finish_match_3984;
  }
case_3987: ;
  {
    if ((zOsaka != zf)) goto case_3986;
    z3zE3782 = UINT64_C(11);
    goto finish_match_3984;
  }
case_3986: ;
  {
    /* complete */
    z3zE3782 = UINT64_C(12);
    goto finish_match_3984;
  }
case_3985: ;
finish_match_3984: ;
  z8zE5 = z3zE3782;
end_function_3998: ;
  return z8zE5;
end_block_exception_3999: ;

  return UINT64_C(0xdeadc0de);
}

bool zfork_lt(enum zFork za, enum zFork zb)
{
  bool z8zE6;
  uint64_t z2zE4092;
  z2zE4092 = zfork_index(za);
  uint64_t z2zE4093;
  z2zE4093 = zfork_index(zb);
  z8zE6 = (z2zE4092 < z2zE4093);
end_function_3982: ;
  return z8zE6;
end_block_exception_3983: ;

  return false;
}

bool zfork_gteq(enum zFork za, enum zFork zb)
{
  bool z8zE7;
  uint64_t z2zE4090;
  z2zE4090 = zfork_index(zb);
  uint64_t z2zE4091;
  z2zE4091 = zfork_index(za);
  z8zE7 = (!(z2zE4091 < z2zE4090));
end_function_3980: ;
  return z8zE7;
end_block_exception_3981: ;

  return false;
}

sail_u256 zU256(sail_u256 zvalue)
{
  sail_u256 z8zE8;
  z8zE8 = zvalue;
end_function_3970: ;
  return z8zE8;
end_block_exception_3971: ;

  return u256_zero();
}

sail_fixed_bytes_20 zAddress(sail_fixed_bytes_20 zbytes)
{
  sail_fixed_bytes_20 z8zE9;
  z8zE9 = zbytes;
end_function_3968: ;
  return z8zE9;
end_block_exception_3969: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_32 zB256(sail_fixed_bytes_32 zbytes)
{
  sail_fixed_bytes_32 z8zE10;
  z8zE10 = zbytes;
end_function_3966: ;
  return z8zE10;
end_block_exception_3967: ;

  return fixed_bytes_32_zero();
}

sail_u256 zhash_to_word(sail_fixed_bytes_32 zbytes)
{
  sail_u256 z8zE11;
  sail_u256 z2zE4080;
  z2zE4080 = zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC32zKzCR__sail_c_repr_u256zKzIboundszG78c09f65c90525db636e344fe928bc0bzK(UINT64_C(32), zbytes);
  z8zE11 = z2zE4080;
end_function_3964: ;
  return z8zE11;
end_block_exception_3965: ;

  return u256_zero();
}

sail_fixed_bytes_32 zword_to_hash(sail_u256 zvalue)
{
  sail_fixed_bytes_32 z8zE12;
  sail_fixed_bytes_32 z2zE4079;
  {
    sail_u256 z2zE4078;
    z2zE4078 = zvalue;
    z2zE4079 = zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC32zKzKzIboundszG78c09f65c90525db636e344fe928bc0bzK(UINT64_C(32), z2zE4078);
  }
  z8zE12 = zB256(z2zE4079);
end_function_3962: ;
  return z8zE12;
end_block_exception_3963: ;

  return fixed_bytes_32_zero();
}

bool zhash_lt(sail_fixed_bytes_32 zleft, sail_fixed_bytes_32 zright)
{
  bool z8zE13;
  sail_u256 z2zE4076;
  z2zE4076 = zhash_to_word(zleft);
  sail_u256 z2zE4077;
  z2zE4077 = zhash_to_word(zright);
  z8zE13 = u256_lt(z2zE4076, z2zE4077);
end_function_3960: ;
  return z8zE13;
end_block_exception_3961: ;

  return false;
}

sail_fixed_bytes_20 zword_to_address(sail_u256 zvalue)
{
  sail_fixed_bytes_20 z8zE14;
  sail_fixed_bytes_20 z2zE4075;
  {
    z2zE4075 = zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC20zKzKzIboundszG83613fe416f377787be94d88c23f4a78zK(UINT64_C(20), zvalue);
  }
  z8zE14 = zAddress(z2zE4075);
end_function_3958: ;
  return z8zE14;
end_block_exception_3959: ;

  return fixed_bytes_20_zero();
}

void create_letbind_0(void) {

  sail_u256 z3zE0;
  z3zE0 = u256_of_fbits(UINT64_C(0));
  zZERO_WORD = z3zE0;
let_end_16: ;
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
  for (size_t z8zE806 = 0; z8zE806 < 20; ++z8zE806) {
    z3zE2.bytes[z8zE806] = (uint8_t)(z3zE1.data[z8zE806] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE1);
  zZERO_ADDRESS = z3zE2;
let_end_17: ;
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
  for (size_t z8zE807 = 0; z8zE807 < 32; ++z8zE807) {
    z3zE4.bytes[z8zE807] = (uint8_t)(z3zE3.data[z8zE807] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE3);
  zZERO_HASH = z3zE4;
let_end_18: ;
}
void kill_letbind_2(void) {
}

void create_letbind_3(void) {

  sail_u256 z3zE5;
  z3zE5 = u256_of_fbits(UINT64_C(0));
  zWORD_ZERO = z3zE5;
let_end_19: ;
}
void kill_letbind_3(void) {
}

void create_letbind_4(void) {

  sail_u256 z3zE6;
  z3zE6 = u256_of_fbits(UINT64_C(1));
  zWORD_ONE = z3zE6;
let_end_20: ;
}
void kill_letbind_4(void) {
}

void create_letbind_5(void) {

  sail_u256 z3zE7;
  z3zE7 = ((sail_u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}});
  zWORD_ALL_ONES = z3zE7;
let_end_21: ;
}
void kill_letbind_5(void) {
}

void create_letbind_6(void) {

  sail_u256 z3zE8;
  z3zE8 = ((sail_u256){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(9223372036854775808)}});
  zWORD_SIGN_BIT = z3zE8;
let_end_22: ;
}
void kill_letbind_6(void) {
}

sail_u256 zword_add_word(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE15;
  sail_u256 z2zE4073;
  {    z2zE4073 = u256_add(zleft, zright);
  }
  z8zE15 = zU256(z2zE4073);
end_function_3956: ;
  return z8zE15;
end_block_exception_3957: ;

  return u256_zero();
}

sail_u256 zword_sub_word(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE16;
  sail_u256 z2zE4071;
  {    z2zE4071 = u256_sub(zleft, zright);
  }
  z8zE16 = zU256(z2zE4071);
end_function_3954: ;
  return z8zE16;
end_block_exception_3955: ;

  return u256_zero();
}

sail_u256 zword_and(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE17;
  sail_u256 z2zE4069;
  {
    sail_u256 z2zE4068;
    {
      sail_u256 z2zE4066;
      z2zE4066 = zleft;
      sail_u256 z2zE4067;
      z2zE4067 = zright;
      z2zE4068 = u256_and(z2zE4066, z2zE4067);
    }
    z2zE4069 = z2zE4068;
  }
  z8zE17 = zU256(z2zE4069);
end_function_3952: ;
  return z8zE17;
end_block_exception_3953: ;

  return u256_zero();
}

sail_u256 zword_or(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE18;
  sail_u256 z2zE4065;
  {
    sail_u256 z2zE4064;
    {
      sail_u256 z2zE4062;
      z2zE4062 = zleft;
      sail_u256 z2zE4063;
      z2zE4063 = zright;
      z2zE4064 = u256_or(z2zE4062, z2zE4063);
    }
    z2zE4065 = z2zE4064;
  }
  z8zE18 = zU256(z2zE4065);
end_function_3950: ;
  return z8zE18;
end_block_exception_3951: ;

  return u256_zero();
}

sail_u256 zword_xor(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE19;
  sail_u256 z2zE4061;
  {
    sail_u256 z2zE4060;
    {
      sail_u256 z2zE4058;
      z2zE4058 = zleft;
      sail_u256 z2zE4059;
      z2zE4059 = zright;
      z2zE4060 = u256_xor(z2zE4058, z2zE4059);
    }
    z2zE4061 = z2zE4060;
  }
  z8zE19 = zU256(z2zE4061);
end_function_3948: ;
  return z8zE19;
end_block_exception_3949: ;

  return u256_zero();
}

sail_u256 zword_not(sail_u256 zvalue)
{
  sail_u256 z8zE20;
  sail_u256 z2zE4057;
  {
    sail_u256 z2zE4056;
    {
      sail_u256 z2zE4055;
      z2zE4055 = zvalue;
      z2zE4056 = u256_not(z2zE4055);
    }
    z2zE4057 = z2zE4056;
  }
  z8zE20 = zU256(z2zE4057);
end_function_3946: ;
  return z8zE20;
end_block_exception_3947: ;

  return u256_zero();
}

uint64_t zword_bit(sail_u256 zvalue, uint64_t zindex)
{
  uint64_t z8zE21;
  sail_u256 z2zE4054;
  z2zE4054 = zvalue;
  z8zE21 = u256_bit(z2zE4054, zindex);
end_function_3944: ;
  return z8zE21;
end_block_exception_3945: ;

  return UINT64_C(0);
}

uint64_t zword_low_byte(sail_u256 zvalue)
{
  uint64_t z8zE22;
  z8zE22 = (UINT64_C(0xFF) & u256_extract_u64(zvalue, (uint64_t)(UINT64_C(0))));
end_function_3942: ;
  return z8zE22;
end_block_exception_3943: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_shift_right_one(sail_u256 zvalue)
{
  sail_u256 z8zE23;
  sail_u256 z2zE4053;
  z2zE4053 = u256_div_u64(zvalue, UINT64_C(2));
  z8zE23 = zU256(z2zE4053);
end_function_3940: ;
  return z8zE23;
end_block_exception_3941: ;

  return u256_zero();
}

sail_u256 zword_of_bool(bool zb)
{
  sail_u256 z8zE24;
  if (zb) {  z8zE24 = zWORD_ONE;  } else {  z8zE24 = zWORD_ZERO;  }
end_function_3938: ;
  return z8zE24;
end_block_exception_3939: ;

  return u256_zero();
}

bool zword_is_zzero(sail_u256 zw)
{
  bool z8zE25;
  z8zE25 = eq_u256(zw, zWORD_ZERO);
end_function_3936: ;
  return z8zE25;
end_block_exception_3937: ;

  return false;
}

bool zword_nonzzero(sail_u256 zw)
{
  bool z8zE26;
  bool z2zE4052;
  z2zE4052 = zword_is_zzero(zw);
  z8zE26 = not(z2zE4052);
end_function_3934: ;
  return z8zE26;
end_block_exception_3935: ;

  return false;
}

bool zword_ult(sail_u256 za, sail_u256 zb)
{
  bool z8zE27;
  z8zE27 = u256_lt(za, zb);
end_function_3932: ;
  return z8zE27;
end_block_exception_3933: ;

  return false;
}

bool zword_ule(sail_u256 za, sail_u256 zb)
{
  bool z8zE28;
  bool z2zE4051;
  z2zE4051 = zword_ult(zb, za);
  z8zE28 = not(z2zE4051);
end_function_3930: ;
  return z8zE28;
end_block_exception_3931: ;

  return false;
}

uint64_t zbyte_bit_length(uint64_t zvalue)
{
  uint64_t z8zE29;
  bool z2zE4044;
  z2zE4044 = (zvalue < UINT64_C(16));
  if (z2zE4044) {
    bool z2zE4045;
    z2zE4045 = (zvalue < UINT64_C(4));
    if (z2zE4045) {
      bool z2zE4046;
      z2zE4046 = (zvalue < UINT64_C(2));
      if (z2zE4046) {  z8zE29 = zvalue;  } else {  z8zE29 = UINT64_C(2);  }
    } else {
      bool z2zE4047;
      z2zE4047 = (zvalue < UINT64_C(8));
      if (z2zE4047) {  z8zE29 = UINT64_C(3);  } else {  z8zE29 = UINT64_C(4);  }
    }
  } else {
    bool z2zE4048;
    z2zE4048 = (zvalue < UINT64_C(64));
    if (z2zE4048) {
      bool z2zE4049;
      z2zE4049 = (zvalue < UINT64_C(32));
      if (z2zE4049) {  z8zE29 = UINT64_C(5);  } else {  z8zE29 = UINT64_C(6);  }
    } else {
      bool z2zE4050;
      z2zE4050 = (zvalue < UINT64_C(128));
      if (z2zE4050) {  z8zE29 = UINT64_C(7);  } else {  z8zE29 = UINT64_C(8);  }
    }
  }
end_function_3928: ;
  return z8zE29;
end_block_exception_3929: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zu16_bit_length(uint64_t zvalue)
{
  uint64_t z8zE30;
  bool z2zE4041;
  z2zE4041 = (zvalue < UINT64_C(256));
  if (z2zE4041) {  z8zE30 = zbyte_bit_length(zvalue);  } else {
    uint64_t z2zE4043;
    {
      uint64_t z2zE4042;
      {    z2zE4042 = (zvalue / UINT64_C(256));
      }
      z2zE4043 = zbyte_bit_length(z2zE4042);
    }
    {    z8zE30 = (UINT64_C(8) + z2zE4043);
    }
  }
end_function_3926: ;
  return z8zE30;
end_block_exception_3927: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zu32_bit_length(uint64_t zvalue)
{
  uint64_t z8zE31;
  bool z2zE4038;
  z2zE4038 = (zvalue < UINT64_C(65536));
  if (z2zE4038) {  z8zE31 = zu16_bit_length(zvalue);  } else {
    uint64_t z2zE4040;
    {
      uint64_t z2zE4039;
      {    z2zE4039 = (zvalue / UINT64_C(65536));
      }
      z2zE4040 = zu16_bit_length(z2zE4039);
    }
    {    z8zE31 = (UINT64_C(16) + z2zE4040);
    }
  }
end_function_3924: ;
  return z8zE31;
end_block_exception_3925: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zu64_bit_length(uint64_t zvalue)
{
  uint64_t z8zE32;
  bool z2zE4035;
  z2zE4035 = (zvalue < UINT64_C(4294967296));
  if (z2zE4035) {  z8zE32 = zu32_bit_length(zvalue);  } else {
    uint64_t z2zE4037;
    {
      uint64_t z2zE4036;
      {    z2zE4036 = (zvalue / UINT64_C(4294967296));
      }
      z2zE4037 = zu32_bit_length(z2zE4036);
    }
    {    z8zE32 = (UINT64_C(32) + z2zE4037);
    }
  }
end_function_3922: ;
  return z8zE32;
end_block_exception_3923: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_bit_length(sail_u256 zvalue)
{
  uint64_t z8zE33;
  uint64_t zlimb3;
  {
    uint64_t z2zE4034;
    z2zE4034 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(192)));
    zlimb3 = ((uint64_t) z2zE4034);
  }
  bool z2zE4024;
  z2zE4024 = (zlimb3 != UINT64_C(0));
  if (z2zE4024) {
    uint64_t z2zE4025;
    z2zE4025 = zu64_bit_length(zlimb3);
    {    z8zE33 = (UINT64_C(192) + z2zE4025);
    }
  } else {
    uint64_t zlimb2;
    {
      uint64_t z2zE4033;
      z2zE4033 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(128)));
      zlimb2 = ((uint64_t) z2zE4033);
    }
    bool z2zE4026;
    z2zE4026 = (zlimb2 != UINT64_C(0));
    if (z2zE4026) {
      uint64_t z2zE4027;
      z2zE4027 = zu64_bit_length(zlimb2);
      {    z8zE33 = (UINT64_C(128) + z2zE4027);
      }
    } else {
      uint64_t zlimb1;
      {
        uint64_t z2zE4032;
        z2zE4032 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(64)));
        zlimb1 = ((uint64_t) z2zE4032);
      }
      bool z2zE4028;
      z2zE4028 = (zlimb1 != UINT64_C(0));
      if (z2zE4028) {
        uint64_t z2zE4029;
        z2zE4029 = zu64_bit_length(zlimb1);
        {    z8zE33 = (UINT64_C(64) + z2zE4029);
        }
      } else {
        uint64_t z2zE4031;
        {
          uint64_t z2zE4030;
          z2zE4030 = u256_extract_u64(zvalue, (uint64_t)(UINT64_C(0)));
          z2zE4031 = ((uint64_t) z2zE4030);
        }
        z8zE33 = zu64_bit_length(z2zE4031);
      }
    }
  }
end_function_3920: ;
  return z8zE33;
end_block_exception_3921: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_mul_word(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE34;
  z8zE34 = u256_mul(za, zb);
end_function_3918: ;
  return z8zE34;
end_block_exception_3919: ;

  return u256_zero();
}

sail_u256 zword_div_word(sail_u256 zdividend, sail_u256 zdivisor)
{
  sail_u256 z8zE35;
  bool z2zE4021;
  z2zE4021 = u256_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE4021) {  z8zE35 = zWORD_ZERO;  } else {
    sail_u256 z2zE4022;
    z2zE4022 = u256_div(zdividend, zdivisor);
    z8zE35 = zU256(z2zE4022);
  }
end_function_3916: ;
  return z8zE35;
end_block_exception_3917: ;

  return u256_zero();
}

sail_u256 zword_mod_word(sail_u256 zdividend, sail_u256 zdivisor)
{
  sail_u256 z8zE36;
  bool z2zE4019;
  z2zE4019 = u256_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE4019) {  z8zE36 = zWORD_ZERO;  } else {
    sail_u256 z2zE4020;
    z2zE4020 = u256_mod(zdividend, zdivisor);
    z8zE36 = zU256(z2zE4020);
  }
end_function_3914: ;
  return z8zE36;
end_block_exception_3915: ;

  return u256_zero();
}

sail_u256 zword_shift_left(sail_u256 zvalue, uint64_t zamount)
{
  sail_u256 z8zE37;
  sail_u256 z2zE4018;
  {
    sail_u256 z2zE4017;
    {
      sail_u256 z2zE4016;
      z2zE4016 = zvalue;
      z2zE4017 = u256_shiftl_u64(z2zE4016, zamount);
    }
    z2zE4018 = z2zE4017;
  }
  z8zE37 = zU256(z2zE4018);
end_function_3910: ;
  return z8zE37;
end_block_exception_3911: ;

  return u256_zero();
}

sail_u256 zword_shift_right(sail_u256 zvalue, uint64_t zamount)
{
  sail_u256 z8zE38;
  sail_u256 z2zE4015;
  {
    sail_u256 z2zE4014;
    z2zE4014 = zvalue;
    z2zE4015 = u256_shiftr_u64(z2zE4014, zamount);
  }
  z8zE38 = z2zE4015;
end_function_3908: ;
  return z8zE38;
end_block_exception_3909: ;

  return u256_zero();
}

uint64_t zword_byte_length(sail_u256 zvalue)
{
  uint64_t z8zE39;
  uint64_t zbit_length;
  zbit_length = zword_bit_length(zvalue);
  bool z2zE4012;
  z2zE4012 = (zbit_length == UINT64_C(0));
  if (z2zE4012) {  z8zE39 = UINT64_C(0);  } else {
    uint64_t z2zE4013;
    {    z2zE4013 = (zbit_length + UINT64_C(7));
    }
    {    z8zE39 = (z2zE4013 / UINT64_C(8));
    }
  }
end_function_3906: ;
  return z8zE39;
end_block_exception_3907: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_arithmetic_shift_right(sail_u256 zvalue, uint64_t zamount)
{
  sail_u256 z8zE40;
  sail_u256 zshifted;
  zshifted = zword_shift_right(zvalue, zamount);
  bool z2zE4010;
  {
    uint64_t z2zE4009;
    z2zE4009 = zword_bit(zvalue, UINT64_C(255));
    z2zE4010 = (z2zE4009 == UINT64_C(0b1));
  }
  if (z2zE4010) {
    sail_u256 zsign_fill;
    {
      uint64_t z2zE4011;
      {    z2zE4011 = (UINT64_C(256) - zamount);
      }
      zsign_fill = zword_shift_left(zWORD_ALL_ONES, z2zE4011);
    }
    z8zE40 = zword_or(zshifted, zsign_fill);
  } else {  z8zE40 = zshifted;  }
end_function_3904: ;
  return z8zE40;
end_block_exception_3905: ;

  return u256_zero();
}

sail_u256 zaddress_to_word(sail_fixed_bytes_20 zbytes)
{
  sail_u256 z8zE41;
  sail_u256 z2zE4008;
  z2zE4008 = zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u256zKzIboundszG83613fe416f377787be94d88c23f4a78zK(UINT64_C(20), zbytes);
  z8zE41 = z2zE4008;
end_function_3902: ;
  return z8zE41;
end_block_exception_3903: ;

  return u256_zero();
}

sail_u256 zword_negate(sail_u256 zvalue)
{
  sail_u256 z8zE42;
  z8zE42 = zword_sub_word(zWORD_ZERO, zvalue);
end_function_3900: ;
  return z8zE42;
end_block_exception_3901: ;

  return u256_zero();
}

sail_u256 zword_abs(sail_u256 zvalue)
{
  sail_u256 z8zE43;
  bool z2zE4007;
  {
    uint64_t z2zE4006;
    z2zE4006 = zword_bit(zvalue, UINT64_C(255));
    z2zE4007 = (z2zE4006 == UINT64_C(0b1));
  }
  if (z2zE4007) {  z8zE43 = zword_negate(zvalue);  } else {  z8zE43 = zvalue;  }
end_function_3898: ;
  return z8zE43;
end_block_exception_3899: ;

  return u256_zero();
}

bool zword_slt(sail_u256 za, sail_u256 zb)
{
  bool z8zE44;
  bool za_neg;
  {
    uint64_t z2zE4005;
    z2zE4005 = zword_bit(za, UINT64_C(255));
    za_neg = (z2zE4005 == UINT64_C(0b1));
  }
  bool zb_neg;
  {
    uint64_t z2zE4004;
    z2zE4004 = zword_bit(zb, UINT64_C(255));
    zb_neg = (z2zE4004 == UINT64_C(0b1));
  }
  if (za_neg) {  if (zb_neg) {  z8zE44 = zword_ult(za, zb);  } else {  z8zE44 = true;  }  } else if (zb_neg) {
    z8zE44 = false;
  } else {  z8zE44 = zword_ult(za, zb);  }
end_function_3896: ;
  return z8zE44;
end_block_exception_3897: ;

  return false;
}

sail_u256 zalu_add(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE45;
  z8zE45 = zword_add_word(za, zb);
end_function_3894: ;
  return z8zE45;
end_block_exception_3895: ;

  return u256_zero();
}

sail_u256 zalu_sub(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE46;
  z8zE46 = zword_sub_word(za, zb);
end_function_3892: ;
  return z8zE46;
end_block_exception_3893: ;

  return u256_zero();
}

sail_u256 zalu_mul(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE47;
  z8zE47 = zword_mul_word(za, zb);
end_function_3890: ;
  return z8zE47;
end_block_exception_3891: ;

  return u256_zero();
}

sail_u256 zalu_div(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE48;
  z8zE48 = zword_div_word(za, zb);
end_function_3888: ;
  return z8zE48;
end_block_exception_3889: ;

  return u256_zero();
}

sail_u256 zalu_mod(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE49;
  z8zE49 = zword_mod_word(za, zb);
end_function_3886: ;
  return z8zE49;
end_block_exception_3887: ;

  return u256_zero();
}

sail_u256 zalu_sdiv(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE50;
  bool z2zE3996;
  z2zE3996 = zword_is_zzero(zb);
  if (z2zE3996) {  z8zE50 = zWORD_ZERO;  } else {
    sail_u256 zquotient;
    {
      sail_u256 z2zE4002;
      z2zE4002 = zword_abs(za);
      sail_u256 z2zE4003;
      z2zE4003 = zword_abs(zb);
      zquotient = zword_div_word(z2zE4002, z2zE4003);
    }
    bool z2zE4001;
    {
      bool z2zE3999;
      {
        uint64_t z2zE3997;
        z2zE3997 = zword_bit(za, UINT64_C(255));
        z2zE3999 = (z2zE3997 == UINT64_C(0b1));
      }
      bool z2zE4000;
      {
        uint64_t z2zE3998;
        z2zE3998 = zword_bit(zb, UINT64_C(255));
        z2zE4000 = (z2zE3998 == UINT64_C(0b1));
      }
      z2zE4001 = zneq_bool(z2zE3999, z2zE4000);
    }
    if (z2zE4001) {  z8zE50 = zword_negate(zquotient);  } else {  z8zE50 = zquotient;  }
  }
end_function_3884: ;
  return z8zE50;
end_block_exception_3885: ;

  return u256_zero();
}

sail_u256 zalu_smod(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE51;
  bool z2zE3991;
  z2zE3991 = zword_is_zzero(zb);
  if (z2zE3991) {  z8zE51 = zWORD_ZERO;  } else {
    sail_u256 zremainder;
    {
      sail_u256 z2zE3994;
      z2zE3994 = zword_abs(za);
      sail_u256 z2zE3995;
      z2zE3995 = zword_abs(zb);
      zremainder = zword_mod_word(z2zE3994, z2zE3995);
    }
    bool z2zE3993;
    {
      uint64_t z2zE3992;
      z2zE3992 = zword_bit(za, UINT64_C(255));
      z2zE3993 = (z2zE3992 == UINT64_C(0b1));
    }
    if (z2zE3993) {  z8zE51 = zword_negate(zremainder);  } else {  z8zE51 = zremainder;  }
  }
end_function_3882: ;
  return z8zE51;
end_block_exception_3883: ;

  return u256_zero();
}

sail_u256 zalu_addmod(sail_u256 za, sail_u256 zb, sail_u256 zn)
{
  sail_u256 z8zE52;
  z8zE52 = u256_addmod(za, zb, zn);
  return z8zE52;
}

sail_u256 zalu_mulmod(sail_u256 za, sail_u256 zb, sail_u256 zn)
{
  sail_u256 z8zE53;
  z8zE53 = u256_mulmod(za, zb, zn);
  return z8zE53;
}

sail_u256 zalu_exp(sail_u256 zbase, sail_u256 zexponent)
{
  sail_u256 z8zE54;
  sail_u256 zresult;
  zresult = zWORD_ONE;
  sail_u256 zb;
  zb = zbase;
  sail_u256 ze;
  ze = zexponent;
  int64_t z3zE3762;
  {    z3zE3762 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3763;
  {    z3zE3763 = (int64_t)(UINT64_C(255));
  }
  int64_t z3zE3764;
  {    z3zE3764 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z_step;
    z_step = z3zE3762;
    unit z3zE3767;
  for_start_3874: ;
    {
      if ((z3zE3763 < z_step)) goto for_end_3875;
      bool z2zE3984;
      {
        uint64_t z2zE3983;
        z2zE3983 = zword_bit(ze, UINT64_C(0));
        z2zE3984 = (z2zE3983 == UINT64_C(0b1));
      }
      unit z3zE3766;
      if (z2zE3984) {
        zresult = zword_mul_word(zresult, zb);
        z3zE3766 = UNIT;
      } else {  z3zE3766 = UNIT;  }
      zb = zword_mul_word(zb, zb);
      unit z3zE3765;
      z3zE3765 = UNIT;
      ze = zword_shift_right_one(ze);
      z3zE3767 = UNIT;
      z_step = (z_step + z3zE3764);
      goto for_start_3874;
    }
  for_end_3875: ;
  }
  unit z3zE3768;
  z3zE3768 = UNIT;
  z8zE54 = zresult;
end_function_3876: ;
  return z8zE54;
end_block_exception_3877: ;

  return u256_zero();
}

sail_u256 zalu_signextend(sail_u256 zbyte_index, sail_u256 zvalue)
{
  sail_u256 z8zE55;
  bool z2zE3975;
  z2zE3975 = u256_lt_u64(zbyte_index, UINT64_C(32));
  if (z2zE3975) {
    uint64_t zindex;
    zindex = u256_to_u64(zbyte_index);
    uint64_t zwidth;
    {
      uint64_t z2zE3982;
      {    z2zE3982 = (zindex * UINT64_C(8));
      }
      {    zwidth = (z2zE3982 + UINT64_C(8));
      }
    }
    uint64_t zsign_shift;
    {
      uint64_t z2zE3981;
      {    z2zE3981 = (zindex * UINT64_C(8));
      }
      {    zsign_shift = (z2zE3981 + UINT64_C(7));
      }
    }
    bool zsign_set;
    {
      sail_u256 z2zE3980;
      {
        sail_u256 z2zE3979;
        z2zE3979 = zword_shift_right(zvalue, zsign_shift);
        z2zE3980 = zword_and(z2zE3979, zWORD_ONE);
      }
      zsign_set = eq_u256(z2zE3980, zWORD_ONE);
    }
    sail_u256 zlow_mask;
    {
      sail_u256 z2zE3978;
      z2zE3978 = zword_shift_left(zWORD_ONE, zwidth);
      zlow_mask = zword_sub_word(z2zE3978, zWORD_ONE);
    }
    if (zsign_set) {
      sail_u256 z2zE3976;
      z2zE3976 = zword_and(zvalue, zlow_mask);
      sail_u256 z2zE3977;
      z2zE3977 = zword_not(zlow_mask);
      z8zE55 = zword_or(z2zE3976, z2zE3977);
    } else {  z8zE55 = zword_and(zvalue, zlow_mask);  }
  } else {  z8zE55 = zvalue;  }
end_function_3872: ;
  return z8zE55;
end_block_exception_3873: ;

  return u256_zero();
}

sail_u256 zalu_lt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE56;
  bool z2zE3974;
  z2zE3974 = zword_ult(za, zb);
  z8zE56 = zword_of_bool(z2zE3974);
end_function_3870: ;
  return z8zE56;
end_block_exception_3871: ;

  return u256_zero();
}

sail_u256 zalu_gt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE57;
  bool z2zE3973;
  z2zE3973 = zword_ult(zb, za);
  z8zE57 = zword_of_bool(z2zE3973);
end_function_3868: ;
  return z8zE57;
end_block_exception_3869: ;

  return u256_zero();
}

sail_u256 zalu_slt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE58;
  bool z2zE3972;
  z2zE3972 = zword_slt(za, zb);
  z8zE58 = zword_of_bool(z2zE3972);
end_function_3866: ;
  return z8zE58;
end_block_exception_3867: ;

  return u256_zero();
}

sail_u256 zalu_sgt(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE59;
  bool z2zE3971;
  z2zE3971 = zword_slt(zb, za);
  z8zE59 = zword_of_bool(z2zE3971);
end_function_3864: ;
  return z8zE59;
end_block_exception_3865: ;

  return u256_zero();
}

sail_u256 zalu_eq(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE60;
  bool z2zE3970;
  z2zE3970 = eq_u256(za, zb);
  z8zE60 = zword_of_bool(z2zE3970);
end_function_3862: ;
  return z8zE60;
end_block_exception_3863: ;

  return u256_zero();
}

sail_u256 zalu_iszzero(sail_u256 za)
{
  sail_u256 z8zE61;
  bool z2zE3969;
  z2zE3969 = zword_is_zzero(za);
  z8zE61 = zword_of_bool(z2zE3969);
end_function_3860: ;
  return z8zE61;
end_block_exception_3861: ;

  return u256_zero();
}

sail_u256 zalu_and(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE62;
  z8zE62 = zword_and(za, zb);
end_function_3858: ;
  return z8zE62;
end_block_exception_3859: ;

  return u256_zero();
}

sail_u256 zalu_or(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE63;
  z8zE63 = zword_or(za, zb);
end_function_3856: ;
  return z8zE63;
end_block_exception_3857: ;

  return u256_zero();
}

sail_u256 zalu_xor(sail_u256 za, sail_u256 zb)
{
  sail_u256 z8zE64;
  z8zE64 = zword_xor(za, zb);
end_function_3854: ;
  return z8zE64;
end_block_exception_3855: ;

  return u256_zero();
}

sail_u256 zalu_not(sail_u256 za)
{
  sail_u256 z8zE65;
  z8zE65 = zword_not(za);
end_function_3852: ;
  return z8zE65;
end_block_exception_3853: ;

  return u256_zero();
}

sail_u256 zalu_byte(sail_u256 zi, sail_u256 zx)
{
  sail_u256 z8zE66;
  bool z2zE3965;
  z2zE3965 = u256_lt_u64(zi, UINT64_C(32));
  if (z2zE3965) {
    uint64_t zindex;
    zindex = u256_to_u64(zi);
    uint64_t zshift;
    {
      uint64_t z2zE3968;
      {    z2zE3968 = (UINT64_C(31) - zindex);
      }
      {    zshift = (z2zE3968 * UINT64_C(8));
      }
    }
    uint64_t z2zE3967;
    {
      sail_u256 z2zE3966;
      z2zE3966 = zword_shift_right(zx, zshift);
      z2zE3967 = zword_low_byte(z2zE3966);
    }
    z8zE66 = u256_of_fbits(((uint64_t) z2zE3967));
  } else {  z8zE66 = zWORD_ZERO;  }
end_function_3850: ;
  return z8zE66;
end_block_exception_3851: ;

  return u256_zero();
}

sail_u256 zalu_shl(sail_u256 zshift_amt, sail_u256 zv)
{
  sail_u256 z8zE67;
  bool z2zE3964;
  z2zE3964 = u256_lt_u64(zshift_amt, UINT64_C(256));
  if (z2zE3964) {
    {
      uint64_t z3zE3846;
      z3zE3846 = u256_to_u64(zshift_amt);
      z8zE67 = zword_shift_left(zv, z3zE3846);
    }
  } else {  z8zE67 = zWORD_ZERO;  }
end_function_3848: ;
  return z8zE67;
end_block_exception_3849: ;

  return u256_zero();
}

sail_u256 zalu_shr(sail_u256 zshift_amt, sail_u256 zv)
{
  sail_u256 z8zE68;
  bool z2zE3963;
  z2zE3963 = u256_lt_u64(zshift_amt, UINT64_C(256));
  if (z2zE3963) {
    {
      uint64_t z3zE3847;
      z3zE3847 = u256_to_u64(zshift_amt);
      z8zE68 = zword_shift_right(zv, z3zE3847);
    }
  } else {  z8zE68 = zWORD_ZERO;  }
end_function_3846: ;
  return z8zE68;
end_block_exception_3847: ;

  return u256_zero();
}

sail_u256 zalu_sar(sail_u256 zshift_amt, sail_u256 zv)
{
  sail_u256 z8zE69;
  bool z2zE3960;
  z2zE3960 = u256_lt_u64(zshift_amt, UINT64_C(256));
  if (z2zE3960) {
    {
      uint64_t z3zE3848;
      z3zE3848 = u256_to_u64(zshift_amt);
      z8zE69 = zword_arithmetic_shift_right(zv, z3zE3848);
    }
  } else {
    bool z2zE3962;
    {
      uint64_t z2zE3961;
      z2zE3961 = zword_bit(zv, UINT64_C(255));
      z2zE3962 = (z2zE3961 == UINT64_C(0b1));
    }
    if (z2zE3962) {  z8zE69 = zWORD_ALL_ONES;  } else {  z8zE69 = zWORD_ZERO;  }
  }
end_function_3844: ;
  return z8zE69;
end_block_exception_3845: ;

  return u256_zero();
}

sail_u256 zalu_clzz(sail_u256 zx)
{
  sail_u256 z8zE70;
  uint64_t z2zE3959;
  {
    uint64_t z2zE3958;
    z2zE3958 = zword_bit_length(zx);
    {    z2zE3959 = (UINT64_C(256) - z2zE3958);
    }
  }
  z8zE70 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3959);
end_function_3842: ;
  return z8zE70;
end_block_exception_3843: ;

  return u256_zero();
}

