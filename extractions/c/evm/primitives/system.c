/* Generated from sail/primitives/system.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_60(void) {

  sail_fixed_bytes_20 z3zE68;
  zz5vecz8z5bv8z9 z3zE67;
  CREATE(zz5vecz8z5bv8z9)(&z3zE67);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE67, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(0), UINT64_C(0xFE));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(1), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(2), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(3), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(4), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(5), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(6), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(7), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(8), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(9), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(10), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(11), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(12), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(13), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(14), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(15), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(16), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(17), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(18), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE67, z3zE67, INT64_C(19), UINT64_C(0xFF));
  for (size_t z8zE818 = 0; z8zE818 < 20; ++z8zE818) {
    z3zE68.bytes[z8zE818] = (uint8_t)(z3zE67.data[z8zE818] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE67);
  zSYSTEM_ADDRESS = z3zE68;
let_end_169: ;
}
void kill_letbind_60(void) {
}

void create_letbind_61(void) {

  sail_fixed_bytes_20 z3zE70;
  zz5vecz8z5bv8z9 z3zE69;
  CREATE(zz5vecz8z5bv8z9)(&z3zE69);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE69, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(0), UINT64_C(0x02));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(1), UINT64_C(0xAC));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(2), UINT64_C(0xBE));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(3), UINT64_C(0xD0));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(4), UINT64_C(0x22));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(5), UINT64_C(0x85));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(6), UINT64_C(0xBB));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(7), UINT64_C(0xB8));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(8), UINT64_C(0xB7));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(9), UINT64_C(0x9F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(10), UINT64_C(0x31));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(11), UINT64_C(0xF1));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(12), UINT64_C(0x7E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(13), UINT64_C(0x80));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(14), UINT64_C(0x32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(15), UINT64_C(0xD7));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(16), UINT64_C(0xF6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(17), UINT64_C(0x3D));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(18), UINT64_C(0x0F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE69, z3zE69, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE819 = 0; z8zE819 < 20; ++z8zE819) {
    z3zE70.bytes[z8zE819] = (uint8_t)(z3zE69.data[z8zE819] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE69);
  zBEACON_ROOTS_ADDR = z3zE70;
let_end_170: ;
}
void kill_letbind_61(void) {
}

void create_letbind_62(void) {

  sail_fixed_bytes_20 z3zE72;
  zz5vecz8z5bv8z9 z3zE71;
  CREATE(zz5vecz8z5bv8z9)(&z3zE71);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE71, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(0), UINT64_C(0x35));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(1), UINT64_C(0x29));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(3), UINT64_C(0x20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(4), UINT64_C(0x53));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(5), UINT64_C(0x17));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(6), UINT64_C(0x5B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(7), UINT64_C(0x33));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(8), UINT64_C(0x02));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(9), UINT64_C(0x7A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(10), UINT64_C(0xCB));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(11), UINT64_C(0x10));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(12), UINT64_C(0x3A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(13), UINT64_C(0xC5));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(14), UINT64_C(0xF1));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(15), UINT64_C(0x27));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(16), UINT64_C(0x08));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(17), UINT64_C(0xF9));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE71, z3zE71, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE820 = 0; z8zE820 < 20; ++z8zE820) {
    z3zE72.bytes[z8zE820] = (uint8_t)(z3zE71.data[z8zE820] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE71);
  zHISTORY_STORAGE_ADDR = z3zE72;
let_end_171: ;
}
void kill_letbind_62(void) {
}

void create_letbind_63(void) {

  sail_fixed_bytes_20 z3zE74;
  zz5vecz8z5bv8z9 z3zE73;
  CREATE(zz5vecz8z5bv8z9)(&z3zE73);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE73, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(0), UINT64_C(0x02));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(1), UINT64_C(0x70));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(3), UINT64_C(0x4C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(4), UINT64_C(0xA6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(5), UINT64_C(0x79));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(6), UINT64_C(0x35));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(7), UINT64_C(0xD8));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(8), UINT64_C(0x9A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(9), UINT64_C(0xD1));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(10), UINT64_C(0x80));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(11), UINT64_C(0x5E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(12), UINT64_C(0xB5));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(13), UINT64_C(0x0E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(14), UINT64_C(0x48));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(15), UINT64_C(0xEF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(16), UINT64_C(0x61));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(17), UINT64_C(0x09));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE73, z3zE73, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE821 = 0; z8zE821 < 20; ++z8zE821) {
    z3zE74.bytes[z8zE821] = (uint8_t)(z3zE73.data[z8zE821] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE73);
  zWITHDRAWAL_REQUEST_ADDR = z3zE74;
let_end_172: ;
}
void kill_letbind_63(void) {
}

void create_letbind_64(void) {

  sail_fixed_bytes_20 z3zE76;
  zz5vecz8z5bv8z9 z3zE75;
  CREATE(zz5vecz8z5bv8z9)(&z3zE75);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE75, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(0), UINT64_C(0x51));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(1), UINT64_C(0x72));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(3), UINT64_C(0x90));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(4), UINT64_C(0xA5));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(5), UINT64_C(0xF3));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(7), UINT64_C(0x8B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(8), UINT64_C(0x9F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(9), UINT64_C(0x57));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(10), UINT64_C(0xFB));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(11), UINT64_C(0x42));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(12), UINT64_C(0x86));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(13), UINT64_C(0x48));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(14), UINT64_C(0xCE));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(15), UINT64_C(0xC7));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(16), UINT64_C(0xDD));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(17), UINT64_C(0xBB));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE75, z3zE75, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE822 = 0; z8zE822 < 20; ++z8zE822) {
    z3zE76.bytes[z8zE822] = (uint8_t)(z3zE75.data[z8zE822] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE75);
  zCONSOLIDATION_REQUEST_ADDR = z3zE76;
let_end_173: ;
}
void kill_letbind_64(void) {
}

void create_letbind_65(void) {

  sail_fixed_bytes_20 z3zE78;
  zz5vecz8z5bv8z9 z3zE77;
  CREATE(zz5vecz8z5bv8z9)(&z3zE77);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE77, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(0), UINT64_C(0x82));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(1), UINT64_C(0x82));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(2), UINT64_C(0x0D));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(3), UINT64_C(0x30));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(4), UINT64_C(0x89));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(5), UINT64_C(0x75));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(6), UINT64_C(0x8C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(7), UINT64_C(0x0A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(8), UINT64_C(0x54));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(9), UINT64_C(0xFA));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(10), UINT64_C(0x91));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(11), UINT64_C(0x56));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(12), UINT64_C(0x72));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(13), UINT64_C(0xE3));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(14), UINT64_C(0x84));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(15), UINT64_C(0x69));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(16), UINT64_C(0xF4));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(17), UINT64_C(0xBF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE77, z3zE77, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE823 = 0; z8zE823 < 20; ++z8zE823) {
    z3zE78.bytes[z8zE823] = (uint8_t)(z3zE77.data[z8zE823] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE77);
  zBUILDER_DEPOSIT_REQUEST_ADDR = z3zE78;
let_end_174: ;
}
void kill_letbind_65(void) {
}

void create_letbind_66(void) {

  sail_fixed_bytes_20 z3zE80;
  zz5vecz8z5bv8z9 z3zE79;
  CREATE(zz5vecz8z5bv8z9)(&z3zE79);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE79, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(0), UINT64_C(0x82));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(1), UINT64_C(0x82));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(2), UINT64_C(0x0E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(3), UINT64_C(0x80));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(4), UINT64_C(0x13));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(5), UINT64_C(0x56));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(6), UINT64_C(0xC6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(7), UINT64_C(0x3B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(8), UINT64_C(0x09));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(9), UINT64_C(0xCB));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(10), UINT64_C(0x8C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(11), UINT64_C(0x8F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(12), UINT64_C(0xD4));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(13), UINT64_C(0x5A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(14), UINT64_C(0x50));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(15), UINT64_C(0x78));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(16), UINT64_C(0xD6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(17), UINT64_C(0x64));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE79, z3zE79, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE824 = 0; z8zE824 < 20; ++z8zE824) {
    z3zE80.bytes[z8zE824] = (uint8_t)(z3zE79.data[z8zE824] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE79);
  zBUILDER_EXIT_REQUEST_ADDR = z3zE80;
let_end_175: ;
}
void kill_letbind_66(void) {
}

void create_letbind_67(void) {

  sail_fixed_bytes_20 z3zE82;
  zz5vecz8z5bv8z9 z3zE81;
  CREATE(zz5vecz8z5bv8z9)(&z3zE81);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE81, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(0), UINT64_C(0xFA));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(1), UINT64_C(0x05));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(2), UINT64_C(0x77));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(3), UINT64_C(0x3D));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(4), UINT64_C(0x30));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(5), UINT64_C(0x05));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(6), UINT64_C(0xBE));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(7), UINT64_C(0x9C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(8), UINT64_C(0x83));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(9), UINT64_C(0xBB));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(10), UINT64_C(0x6C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(11), UINT64_C(0x35));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(12), UINT64_C(0x40));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(13), UINT64_C(0xB5));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(14), UINT64_C(0x9A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(15), UINT64_C(0x21));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE81, z3zE81, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE825 = 0; z8zE825 < 20; ++z8zE825) {
    z3zE82.bytes[z8zE825] = (uint8_t)(z3zE81.data[z8zE825] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE81);
  zDEPOSIT_CONTRACT_ADDR = z3zE82;
let_end_176: ;
}
void kill_letbind_67(void) {
}

void create_letbind_68(void) {

  sail_u256 z3zE83;
  z3zE83 = ((sail_u256){{UINT64_C(12074291595689605317), UINT64_C(16708898399860066458), UINT64_C(12676030261858484297), UINT64_C(7249595157780304706)}});
  zDEPOSIT_EVENT_TOPIC = z3zE83;
let_end_177: ;
}
void kill_letbind_68(void) {
}

void create_letbind_69(void) {

  sail_fixed_bytes_20 z3zE85;
  zz5vecz8z5bv8z9 z3zE84;
  CREATE(zz5vecz8z5bv8z9)(&z3zE84);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE84, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(0), UINT64_C(0xFE));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(1), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(2), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(3), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(4), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(5), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(6), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(7), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(8), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(9), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(10), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(11), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(12), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(13), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(14), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(15), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(16), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(17), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(18), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE84, z3zE84, INT64_C(19), UINT64_C(0xFF));
  for (size_t z8zE826 = 0; z8zE826 < 20; ++z8zE826) {
    z3zE85.bytes[z8zE826] = (uint8_t)(z3zE84.data[z8zE826] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE84);
  zEIP7708_SYSTEM_ADDRESS = z3zE85;
let_end_178: ;
}
void kill_letbind_69(void) {
}

void create_letbind_70(void) {

  sail_u256 z3zE86;
  z3zE86 = ((sail_u256){{UINT64_C(2951364421682967535), UINT64_C(10748869590852608278), UINT64_C(7620847484418887082), UINT64_C(15992936130196719771)}});
  zEIP7708_TRANSFER_TOPIC = z3zE86;
let_end_179: ;
}
void kill_letbind_70(void) {
}

void create_letbind_71(void) {

  sail_u256 z3zE87;
  z3zE87 = ((sail_u256){{UINT64_C(8187823086544452773), UINT64_C(7854220679750799226), UINT64_C(9321359278818002547), UINT64_C(14706211957200335488)}});
  zEIP7708_BURN_TOPIC = z3zE87;
let_end_180: ;
}
void kill_letbind_71(void) {
}

