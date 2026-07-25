/* Generated from sail/evm/precompiles.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_152(void) {

  uint64_t z3zE187;
  z3zE187 = UINT64_C(2097152);
  zACCELERATOR_INPUT_MAX = z3zE187;
let_end_564: ;
}
void kill_letbind_152(void) {
}

void create_letbind_153(void) {

  sail_u256 z3zE188;
  z3zE188 = u256_of_fbits(UINT64_C(4096));
  zFIELD_ELEMENTS_PER_BLOB = z3zE188;
let_end_565: ;
}
void kill_letbind_153(void) {
}

void create_letbind_154(void) {

  sail_u256 z3zE189;
  z3zE189 = ((sail_u256){{UINT64_C(18446744069414584321), UINT64_C(6034159408538082302), UINT64_C(3691218898639771653), UINT64_C(8353516859464449352)}});
  zBLS_MODULUS = z3zE189;
let_end_566: ;
}
void kill_letbind_154(void) {
}

void create_letbind_155(void) {

  uint64_t z3zE190;
  z3zE190 = UINT64_C(213);
  zBLAKE2F_INPUT_LENGTH = z3zE190;
let_end_567: ;
}
void kill_letbind_155(void) {
}

void create_letbind_156(void) {

  uint64_t z3zE191;
  z3zE191 = UINT64_C(212);
  zBLAKE2F_FINAL_BLOCK_OFFSET = z3zE191;
let_end_568: ;
}
void kill_letbind_156(void) {
}

void create_letbind_157(void) {

  uint64_t z3zE192;
  z3zE192 = zDOUBLE_WORD_BYTE_LENGTH;
  zBLAKE2F_OUTPUT_LENGTH = z3zE192;
let_end_569: ;
}
void kill_letbind_157(void) {
}

void create_letbind_158(void) {

  uint64_t z3zE193;
  z3zE193 = UINT64_C(192);
  zKZG_INPUT_LENGTH = z3zE193;
let_end_570: ;
}
void kill_letbind_158(void) {
}

void create_letbind_159(void) {

  uint64_t z3zE194;
  z3zE194 = UINT64_C(96);
  zKZG_COMMITMENT_OFFSET = z3zE194;
let_end_571: ;
}
void kill_letbind_159(void) {
}

void create_letbind_160(void) {

  uint64_t z3zE195;
  z3zE195 = UINT64_C(48);
  zKZG_COMMITMENT_LENGTH = z3zE195;
let_end_572: ;
}
void kill_letbind_160(void) {
}

void create_letbind_161(void) {

  uint64_t z3zE196;
  z3zE196 = UINT64_C(16);
  zBLS_FIELD_PADDING_LENGTH = z3zE196;
let_end_573: ;
}
void kill_letbind_161(void) {
}

void create_letbind_162(void) {

  uint64_t z3zE197;
  z3zE197 = zDOUBLE_WORD_BYTE_LENGTH;
  zBLS_PADDED_FIELD_LENGTH = z3zE197;
let_end_574: ;
}
void kill_letbind_162(void) {
}

void create_letbind_163(void) {

  uint64_t z3zE198;
  z3zE198 = UINT64_C(128);
  zBLS_G1_POINT_LENGTH = z3zE198;
let_end_575: ;
}
void kill_letbind_163(void) {
}

void create_letbind_164(void) {

  uint64_t z3zE199;
  z3zE199 = UINT64_C(256);
  zBLS_G2_POINT_LENGTH = z3zE199;
let_end_576: ;
}
void kill_letbind_164(void) {
}

void create_letbind_165(void) {

  uint64_t z3zE200;
  z3zE200 = UINT64_C(192);
  zBLS_G2_FINAL_FIELD_OFFSET = z3zE200;
let_end_577: ;
}
void kill_letbind_165(void) {
}

void create_letbind_166(void) {

  uint64_t z3zE201;
  z3zE201 = UINT64_C(256);
  zBLS_G1_ADD_INPUT_LENGTH = z3zE201;
let_end_578: ;
}
void kill_letbind_166(void) {
}

void create_letbind_167(void) {

  uint64_t z3zE202;
  z3zE202 = UINT64_C(160);
  zBLS_G1_MSM_ITEM_LENGTH = z3zE202;
let_end_579: ;
}
void kill_letbind_167(void) {
}

void create_letbind_168(void) {

  uint64_t z3zE203;
  z3zE203 = UINT64_C(512);
  zBLS_G2_ADD_INPUT_LENGTH = z3zE203;
let_end_580: ;
}
void kill_letbind_168(void) {
}

void create_letbind_169(void) {

  uint64_t z3zE204;
  z3zE204 = UINT64_C(288);
  zBLS_G2_MSM_ITEM_LENGTH = z3zE204;
let_end_581: ;
}
void kill_letbind_169(void) {
}

void create_letbind_170(void) {

  uint64_t z3zE205;
  z3zE205 = UINT64_C(384);
  zBLS_PAIRING_ITEM_LENGTH = z3zE205;
let_end_582: ;
}
void kill_letbind_170(void) {
}

void create_letbind_171(void) {

  uint64_t z3zE206;
  z3zE206 = UINT64_C(160);
  zP256_INPUT_LENGTH = z3zE206;
let_end_583: ;
}
void kill_letbind_171(void) {
}

void create_letbind_172(void) {

  uint64_t z3zE207;
  z3zE207 = UINT64_C(192);
  zBN254_PAIRING_ITEM_LENGTH = z3zE207;
let_end_584: ;
}
void kill_letbind_172(void) {
}

void create_letbind_173(void) {

  uint64_t z3zE208;
  z3zE208 = zWORD_BYTE_LENGTH;
  zPRECOMPILE_WORD_LENGTH = z3zE208;
let_end_585: ;
}
void kill_letbind_173(void) {
}

void create_letbind_174(void) {

  uint64_t z3zE209;
  z3zE209 = zDOUBLE_WORD_BYTE_LENGTH;
  zPRECOMPILE_DOUBLE_WORD_LENGTH = z3zE209;
let_end_586: ;
}
void kill_letbind_174(void) {
}

void create_letbind_175(void) {

  uint64_t z3zE210;
  z3zE210 = UINT64_C(32);
  zPRECOMPILE_WORD_OFFSET = z3zE210;
let_end_587: ;
}
void kill_letbind_175(void) {
}

void create_letbind_176(void) {

  uint64_t z3zE211;
  z3zE211 = UINT64_C(64);
  zPRECOMPILE_DOUBLE_WORD_OFFSET = z3zE211;
let_end_588: ;
}
void kill_letbind_176(void) {
}

void create_letbind_177(void) {

  uint64_t z3zE212;
  z3zE212 = UINT64_C(96);
  zECRECOVER_S_OFFSET = z3zE212;
let_end_589: ;
}
void kill_letbind_177(void) {
}

void create_letbind_178(void) {

  uint64_t z3zE213;
  z3zE213 = UINT64_C(2);
  zTWO_COMPONENTS = z3zE213;
let_end_590: ;
}
void kill_letbind_178(void) {
}

void create_letbind_179(void) {

  uint64_t z3zE214;
  z3zE214 = UINT64_C(128);
  zBLS_G2_POINT_OFFSET = z3zE214;
let_end_591: ;
}
void kill_letbind_179(void) {
}

void create_letbind_180(void) {

  sail_fixed_bytes_20 z3zE216;
  zz5vecz8z5bv8z9 z3zE215;
  CREATE(zz5vecz8z5bv8z9)(&z3zE215);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE215, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(0), UINT64_C(0x01));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE215, z3zE215, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE908 = 0; z8zE908 < 20; ++z8zE908) {
    z3zE216.bytes[z8zE908] = (uint8_t)(z3zE215.data[z8zE908] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE215);
  zPRECOMPILE_ADDRESS_1 = z3zE216;
let_end_592: ;
}
void kill_letbind_180(void) {
}

void create_letbind_181(void) {

  sail_fixed_bytes_20 z3zE218;
  zz5vecz8z5bv8z9 z3zE217;
  CREATE(zz5vecz8z5bv8z9)(&z3zE217);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE217, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(0), UINT64_C(0x02));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE217, z3zE217, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE909 = 0; z8zE909 < 20; ++z8zE909) {
    z3zE218.bytes[z8zE909] = (uint8_t)(z3zE217.data[z8zE909] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE217);
  zPRECOMPILE_ADDRESS_2 = z3zE218;
let_end_593: ;
}
void kill_letbind_181(void) {
}

void create_letbind_182(void) {

  sail_fixed_bytes_20 z3zE220;
  zz5vecz8z5bv8z9 z3zE219;
  CREATE(zz5vecz8z5bv8z9)(&z3zE219);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE219, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(0), UINT64_C(0x03));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE219, z3zE219, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE910 = 0; z8zE910 < 20; ++z8zE910) {
    z3zE220.bytes[z8zE910] = (uint8_t)(z3zE219.data[z8zE910] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE219);
  zPRECOMPILE_ADDRESS_3 = z3zE220;
let_end_594: ;
}
void kill_letbind_182(void) {
}

void create_letbind_183(void) {

  sail_fixed_bytes_20 z3zE222;
  zz5vecz8z5bv8z9 z3zE221;
  CREATE(zz5vecz8z5bv8z9)(&z3zE221);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE221, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(0), UINT64_C(0x04));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE221, z3zE221, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE911 = 0; z8zE911 < 20; ++z8zE911) {
    z3zE222.bytes[z8zE911] = (uint8_t)(z3zE221.data[z8zE911] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE221);
  zPRECOMPILE_ADDRESS_4 = z3zE222;
let_end_595: ;
}
void kill_letbind_183(void) {
}

void create_letbind_184(void) {

  sail_fixed_bytes_20 z3zE224;
  zz5vecz8z5bv8z9 z3zE223;
  CREATE(zz5vecz8z5bv8z9)(&z3zE223);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE223, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(0), UINT64_C(0x05));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE223, z3zE223, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE912 = 0; z8zE912 < 20; ++z8zE912) {
    z3zE224.bytes[z8zE912] = (uint8_t)(z3zE223.data[z8zE912] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE223);
  zPRECOMPILE_ADDRESS_5 = z3zE224;
let_end_596: ;
}
void kill_letbind_184(void) {
}

void create_letbind_185(void) {

  sail_fixed_bytes_20 z3zE226;
  zz5vecz8z5bv8z9 z3zE225;
  CREATE(zz5vecz8z5bv8z9)(&z3zE225);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE225, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(0), UINT64_C(0x06));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE225, z3zE225, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE913 = 0; z8zE913 < 20; ++z8zE913) {
    z3zE226.bytes[z8zE913] = (uint8_t)(z3zE225.data[z8zE913] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE225);
  zPRECOMPILE_ADDRESS_6 = z3zE226;
let_end_597: ;
}
void kill_letbind_185(void) {
}

void create_letbind_186(void) {

  sail_fixed_bytes_20 z3zE228;
  zz5vecz8z5bv8z9 z3zE227;
  CREATE(zz5vecz8z5bv8z9)(&z3zE227);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE227, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(0), UINT64_C(0x07));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE227, z3zE227, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE914 = 0; z8zE914 < 20; ++z8zE914) {
    z3zE228.bytes[z8zE914] = (uint8_t)(z3zE227.data[z8zE914] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE227);
  zPRECOMPILE_ADDRESS_7 = z3zE228;
let_end_598: ;
}
void kill_letbind_186(void) {
}

void create_letbind_187(void) {

  sail_fixed_bytes_20 z3zE230;
  zz5vecz8z5bv8z9 z3zE229;
  CREATE(zz5vecz8z5bv8z9)(&z3zE229);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE229, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(0), UINT64_C(0x08));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE229, z3zE229, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE915 = 0; z8zE915 < 20; ++z8zE915) {
    z3zE230.bytes[z8zE915] = (uint8_t)(z3zE229.data[z8zE915] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE229);
  zPRECOMPILE_ADDRESS_8 = z3zE230;
let_end_599: ;
}
void kill_letbind_187(void) {
}

void create_letbind_188(void) {

  sail_fixed_bytes_20 z3zE232;
  zz5vecz8z5bv8z9 z3zE231;
  CREATE(zz5vecz8z5bv8z9)(&z3zE231);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE231, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(0), UINT64_C(0x09));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE231, z3zE231, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE916 = 0; z8zE916 < 20; ++z8zE916) {
    z3zE232.bytes[z8zE916] = (uint8_t)(z3zE231.data[z8zE916] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE231);
  zPRECOMPILE_ADDRESS_9 = z3zE232;
let_end_600: ;
}
void kill_letbind_188(void) {
}

void create_letbind_189(void) {

  sail_fixed_bytes_20 z3zE234;
  zz5vecz8z5bv8z9 z3zE233;
  CREATE(zz5vecz8z5bv8z9)(&z3zE233);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE233, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(0), UINT64_C(0x0A));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE233, z3zE233, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE917 = 0; z8zE917 < 20; ++z8zE917) {
    z3zE234.bytes[z8zE917] = (uint8_t)(z3zE233.data[z8zE917] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE233);
  zPRECOMPILE_ADDRESS_10 = z3zE234;
let_end_601: ;
}
void kill_letbind_189(void) {
}

void create_letbind_190(void) {

  sail_fixed_bytes_20 z3zE236;
  zz5vecz8z5bv8z9 z3zE235;
  CREATE(zz5vecz8z5bv8z9)(&z3zE235);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE235, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(0), UINT64_C(0x0B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE235, z3zE235, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE918 = 0; z8zE918 < 20; ++z8zE918) {
    z3zE236.bytes[z8zE918] = (uint8_t)(z3zE235.data[z8zE918] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE235);
  zPRECOMPILE_ADDRESS_11 = z3zE236;
let_end_602: ;
}
void kill_letbind_190(void) {
}

void create_letbind_191(void) {

  sail_fixed_bytes_20 z3zE238;
  zz5vecz8z5bv8z9 z3zE237;
  CREATE(zz5vecz8z5bv8z9)(&z3zE237);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE237, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(0), UINT64_C(0x0C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE237, z3zE237, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE919 = 0; z8zE919 < 20; ++z8zE919) {
    z3zE238.bytes[z8zE919] = (uint8_t)(z3zE237.data[z8zE919] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE237);
  zPRECOMPILE_ADDRESS_12 = z3zE238;
let_end_603: ;
}
void kill_letbind_191(void) {
}

void create_letbind_192(void) {

  sail_fixed_bytes_20 z3zE240;
  zz5vecz8z5bv8z9 z3zE239;
  CREATE(zz5vecz8z5bv8z9)(&z3zE239);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE239, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(0), UINT64_C(0x0D));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE239, z3zE239, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE920 = 0; z8zE920 < 20; ++z8zE920) {
    z3zE240.bytes[z8zE920] = (uint8_t)(z3zE239.data[z8zE920] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE239);
  zPRECOMPILE_ADDRESS_13 = z3zE240;
let_end_604: ;
}
void kill_letbind_192(void) {
}

void create_letbind_193(void) {

  sail_fixed_bytes_20 z3zE242;
  zz5vecz8z5bv8z9 z3zE241;
  CREATE(zz5vecz8z5bv8z9)(&z3zE241);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE241, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(0), UINT64_C(0x0E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE241, z3zE241, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE921 = 0; z8zE921 < 20; ++z8zE921) {
    z3zE242.bytes[z8zE921] = (uint8_t)(z3zE241.data[z8zE921] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE241);
  zPRECOMPILE_ADDRESS_14 = z3zE242;
let_end_605: ;
}
void kill_letbind_193(void) {
}

void create_letbind_194(void) {

  sail_fixed_bytes_20 z3zE244;
  zz5vecz8z5bv8z9 z3zE243;
  CREATE(zz5vecz8z5bv8z9)(&z3zE243);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE243, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(0), UINT64_C(0x0F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE243, z3zE243, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE922 = 0; z8zE922 < 20; ++z8zE922) {
    z3zE244.bytes[z8zE922] = (uint8_t)(z3zE243.data[z8zE922] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE243);
  zPRECOMPILE_ADDRESS_15 = z3zE244;
let_end_606: ;
}
void kill_letbind_194(void) {
}

void create_letbind_195(void) {

  sail_fixed_bytes_20 z3zE246;
  zz5vecz8z5bv8z9 z3zE245;
  CREATE(zz5vecz8z5bv8z9)(&z3zE245);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE245, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(0), UINT64_C(0x10));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE245, z3zE245, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE923 = 0; z8zE923 < 20; ++z8zE923) {
    z3zE246.bytes[z8zE923] = (uint8_t)(z3zE245.data[z8zE923] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE245);
  zPRECOMPILE_ADDRESS_16 = z3zE246;
let_end_607: ;
}
void kill_letbind_195(void) {
}

void create_letbind_196(void) {

  sail_fixed_bytes_20 z3zE248;
  zz5vecz8z5bv8z9 z3zE247;
  CREATE(zz5vecz8z5bv8z9)(&z3zE247);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE247, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(0), UINT64_C(0x11));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE247, z3zE247, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE924 = 0; z8zE924 < 20; ++z8zE924) {
    z3zE248.bytes[z8zE924] = (uint8_t)(z3zE247.data[z8zE924] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE247);
  zPRECOMPILE_ADDRESS_17 = z3zE248;
let_end_608: ;
}
void kill_letbind_196(void) {
}

void create_letbind_197(void) {

  sail_fixed_bytes_20 z3zE250;
  zz5vecz8z5bv8z9 z3zE249;
  CREATE(zz5vecz8z5bv8z9)(&z3zE249);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE249, INT64_C(20));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(0), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(1), UINT64_C(0x01));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE249, z3zE249, INT64_C(19), UINT64_C(0x00));
  for (size_t z8zE925 = 0; z8zE925 < 20; ++z8zE925) {
    z3zE250.bytes[z8zE925] = (uint8_t)(z3zE249.data[z8zE925] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE249);
  zPRECOMPILE_ADDRESS_256 = z3zE250;
let_end_609: ;
}
void kill_letbind_197(void) {
}

struct zPrecompileResult zprecompile_success(struct zByteSliceFields zoutput)
{
  struct zPrecompileResult z8zE387;
  struct zPrecompileResult z3zE2749;
  z3zE2749.zoutput = zoutput;
  z3zE2749.zsuccess = true;
  z8zE387 = z3zE2749;
end_function_2708: ;
  return z8zE387;
end_block_exception_2709: ;
  struct zByteSliceFields z8zE927 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE926 = { .zoutput = z8zE927, .zsuccess = false };
  return z8zE926;
}

struct zPrecompileResult zprecompile_failure(unit z3zE2747)
{
  struct zPrecompileResult z8zE388;
  struct zPrecompileResult z3zE2748;
  z3zE2748.zoutput = zEMPTY_SLICE;
  z3zE2748.zsuccess = false;
  z8zE388 = z3zE2748;
end_function_2706: ;
  return z8zE388;
end_block_exception_2707: ;
  struct zByteSliceFields z8zE929 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE928 = { .zoutput = z8zE929, .zsuccess = false };
  return z8zE928;
}

struct zPrecompileResult zaccelerator_result(bool zsuccess, uint64_t zoutput_len)
{
  struct zPrecompileResult z8zE389;
  if (zsuccess) {
    struct zByteSliceFields z2zE2585;
    z2zE2585 = zoutput_buffer_slice(zoutput_len);
    z8zE389 = zprecompile_success(z2zE2585);
  } else {  z8zE389 = zprecompile_failure(UNIT);  }
end_function_2704: ;
  return z8zE389;
end_block_exception_2705: ;
  struct zByteSliceFields z8zE931 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE930 = { .zoutput = z8zE931, .zsuccess = false };
  return z8zE930;
}

struct zPrecompileResult zcopied_result(struct zByteSliceFields zdata)
{
  struct zPrecompileResult z8zE390;
  struct zByteSliceFields zoutput;
  zoutput = zfreezze_output(zdata);
  bool z2zE2584;
  {
    uint64_t z2zE2582;
    z2zE2582 = zoutput.zlen;
    uint64_t z2zE2583;
    z2zE2583 = zdata.zlen;
    z2zE2584 = (z2zE2582 == z2zE2583);
  }
  if (z2zE2584) {  z8zE390 = zprecompile_success(zoutput);  } else {  z8zE390 = zprecompile_failure(UNIT);  }
end_function_2702: ;
  return z8zE390;
end_block_exception_2703: ;
  struct zByteSliceFields z8zE933 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE932 = { .zoutput = z8zE933, .zsuccess = false };
  return z8zE932;
}

struct zPrecompileResult zboolean_result(bool zvalue)
{
  struct zPrecompileResult z8zE391;
  struct zByteSliceFields z2zE2581;
  {
    sail_u256 z2zE2580;
    if (zvalue) {  z2zE2580 = zWORD_ONE;  } else {  z2zE2580 = zWORD_ZERO;  }
    z2zE2581 = zoutput_buffer_word(z2zE2580);
  }
  z8zE391 = zprecompile_success(z2zE2581);
end_function_2700: ;
  return z8zE391;
end_block_exception_2701: ;
  struct zByteSliceFields z8zE935 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE934 = { .zoutput = z8zE935, .zsuccess = false };
  return z8zE934;
}

bool zprecompile_active_at_fork(uint64_t zn)
{
  bool z8zE392;
  bool zbase;
  zbase = (!(UINT64_C(4) < zn));
  bool zbyzzantium;
  {
    bool z2zE2579;
    z2zE2579 = (!(zn < UINT64_C(5)));
    bool z3zE2736;
    if (z2zE2579) {
      bool z2zE2578;
      z2zE2578 = (!(UINT64_C(8) < zn));
      bool z3zE2735;
      if (z2zE2578) {  z3zE2735 = zfork_gteq(zk_fork, zByzzantium);  } else {  z3zE2735 = false;  }
      z3zE2736 = z3zE2735;
    } else {  z3zE2736 = false;  }
    zbyzzantium = z3zE2736;
  }
  bool zistanbul;
  {
    bool z2zE2577;
    z2zE2577 = (zn == UINT64_C(9));
    bool z3zE2737;
    if (z2zE2577) {  z3zE2737 = zfork_gteq(zk_fork, zIstanbul);  } else {  z3zE2737 = false;  }
    zistanbul = z3zE2737;
  }
  bool zkzzg;
  {
    bool z2zE2576;
    z2zE2576 = (zn == UINT64_C(10));
    bool z3zE2738;
    if (z2zE2576) {  z3zE2738 = zfork_gteq(zk_fork, zCancun);  } else {  z3zE2738 = false;  }
    zkzzg = z3zE2738;
  }
  bool zbls;
  {
    bool z2zE2575;
    z2zE2575 = (!(zn < UINT64_C(11)));
    bool z3zE2740;
    if (z2zE2575) {
      bool z2zE2574;
      z2zE2574 = (!(UINT64_C(17) < zn));
      bool z3zE2739;
      if (z2zE2574) {  z3zE2739 = zfork_gteq(zk_fork, zPrague);  } else {  z3zE2739 = false;  }
      z3zE2740 = z3zE2739;
    } else {  z3zE2740 = false;  }
    zbls = z3zE2740;
  }
  bool zp256;
  {
    bool z2zE2573;
    z2zE2573 = (zn == UINT64_C(256));
    bool z3zE2741;
    if (z2zE2573) {  z3zE2741 = zfork_gteq(zk_fork, zOsaka);  } else {  z3zE2741 = false;  }
    zp256 = z3zE2741;
  }
  bool z3zE2746;
  if (zbase) {  z3zE2746 = true;  } else {
    bool z3zE2745;
    if (zbyzzantium) {  z3zE2745 = true;  } else {
      bool z3zE2744;
      if (zistanbul) {  z3zE2744 = true;  } else {
        bool z3zE2743;
        if (zkzzg) {  z3zE2743 = true;  } else {
          bool z3zE2742;
          if (zbls) {  z3zE2742 = true;  } else {  z3zE2742 = zp256;  }
          z3zE2743 = z3zE2742;
        }
        z3zE2744 = z3zE2743;
      }
      z3zE2745 = z3zE2744;
    }
    z3zE2746 = z3zE2745;
  }
  z8zE392 = z3zE2746;
end_function_2698: ;
  return z8zE392;
end_block_exception_2699: ;

  return false;
}

uint64_t zprecompile_number(sail_fixed_bytes_20 zbytes)
{
  uint64_t z8zE393;
  uint64_t zselector;
  {
    uint64_t z2zE2571;
    z2zE2571 = fast_unsigned_vector_access_fixed_bytes_20(zbytes, UINT64_C(1));
    uint64_t z2zE2572;
    z2zE2572 = fast_unsigned_vector_access_fixed_bytes_20(zbytes, UINT64_C(0));
    zselector = (z2zE2571 << 8) | z2zE2572;
  }
  uint64_t zcandidate;
  {
    uint64_t z3zE2698;
    {
      uint64_t zp0z3;
      zp0z3 = zselector;
      bool z3zE2717;
      z3zE2717 = (zp0z3 == UINT64_C(0x0001));
      if (!(z3zE2717)) {  goto case_2695;  }
      bool z2zE2553;
      z2zE2553 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_1);
      if (z2zE2553) {  z3zE2698 = UINT64_C(1);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2695: ;
    {
      uint64_t z3zE2718;
      z3zE2718 = zselector;
      bool z3zE2716;
      z3zE2716 = (z3zE2718 == UINT64_C(0x0002));
      if (!(z3zE2716)) {  goto case_2694;  }
      bool z2zE2554;
      z2zE2554 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_2);
      if (z2zE2554) {  z3zE2698 = UINT64_C(2);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2694: ;
    {
      uint64_t z3zE2719;
      z3zE2719 = zselector;
      bool z3zE2715;
      z3zE2715 = (z3zE2719 == UINT64_C(0x0003));
      if (!(z3zE2715)) {  goto case_2693;  }
      bool z2zE2555;
      z2zE2555 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_3);
      if (z2zE2555) {  z3zE2698 = UINT64_C(3);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2693: ;
    {
      uint64_t z3zE2720;
      z3zE2720 = zselector;
      bool z3zE2714;
      z3zE2714 = (z3zE2720 == UINT64_C(0x0004));
      if (!(z3zE2714)) {  goto case_2692;  }
      bool z2zE2556;
      z2zE2556 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_4);
      if (z2zE2556) {  z3zE2698 = UINT64_C(4);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2692: ;
    {
      uint64_t z3zE2721;
      z3zE2721 = zselector;
      bool z3zE2713;
      z3zE2713 = (z3zE2721 == UINT64_C(0x0005));
      if (!(z3zE2713)) {  goto case_2691;  }
      bool z2zE2557;
      z2zE2557 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_5);
      if (z2zE2557) {  z3zE2698 = UINT64_C(5);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2691: ;
    {
      uint64_t z3zE2722;
      z3zE2722 = zselector;
      bool z3zE2712;
      z3zE2712 = (z3zE2722 == UINT64_C(0x0006));
      if (!(z3zE2712)) {  goto case_2690;  }
      bool z2zE2558;
      z2zE2558 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_6);
      if (z2zE2558) {  z3zE2698 = UINT64_C(6);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2690: ;
    {
      uint64_t z3zE2723;
      z3zE2723 = zselector;
      bool z3zE2711;
      z3zE2711 = (z3zE2723 == UINT64_C(0x0007));
      if (!(z3zE2711)) {  goto case_2689;  }
      bool z2zE2559;
      z2zE2559 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_7);
      if (z2zE2559) {  z3zE2698 = UINT64_C(7);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2689: ;
    {
      uint64_t z3zE2724;
      z3zE2724 = zselector;
      bool z3zE2710;
      z3zE2710 = (z3zE2724 == UINT64_C(0x0008));
      if (!(z3zE2710)) {  goto case_2688;  }
      bool z2zE2560;
      z2zE2560 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_8);
      if (z2zE2560) {  z3zE2698 = UINT64_C(8);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2688: ;
    {
      uint64_t z3zE2725;
      z3zE2725 = zselector;
      bool z3zE2709;
      z3zE2709 = (z3zE2725 == UINT64_C(0x0009));
      if (!(z3zE2709)) {  goto case_2687;  }
      bool z2zE2561;
      z2zE2561 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_9);
      if (z2zE2561) {  z3zE2698 = UINT64_C(9);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2687: ;
    {
      uint64_t z3zE2726;
      z3zE2726 = zselector;
      bool z3zE2708;
      z3zE2708 = (z3zE2726 == UINT64_C(0x000A));
      if (!(z3zE2708)) {  goto case_2686;  }
      bool z2zE2562;
      z2zE2562 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_10);
      if (z2zE2562) {  z3zE2698 = UINT64_C(10);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2686: ;
    {
      uint64_t z3zE2727;
      z3zE2727 = zselector;
      bool z3zE2707;
      z3zE2707 = (z3zE2727 == UINT64_C(0x000B));
      if (!(z3zE2707)) {  goto case_2685;  }
      bool z2zE2563;
      z2zE2563 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_11);
      if (z2zE2563) {  z3zE2698 = UINT64_C(11);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2685: ;
    {
      uint64_t z3zE2728;
      z3zE2728 = zselector;
      bool z3zE2706;
      z3zE2706 = (z3zE2728 == UINT64_C(0x000C));
      if (!(z3zE2706)) {  goto case_2684;  }
      bool z2zE2564;
      z2zE2564 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_12);
      if (z2zE2564) {  z3zE2698 = UINT64_C(12);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2684: ;
    {
      uint64_t z3zE2729;
      z3zE2729 = zselector;
      bool z3zE2705;
      z3zE2705 = (z3zE2729 == UINT64_C(0x000D));
      if (!(z3zE2705)) {  goto case_2683;  }
      bool z2zE2565;
      z2zE2565 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_13);
      if (z2zE2565) {  z3zE2698 = UINT64_C(13);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2683: ;
    {
      uint64_t z3zE2730;
      z3zE2730 = zselector;
      bool z3zE2704;
      z3zE2704 = (z3zE2730 == UINT64_C(0x000E));
      if (!(z3zE2704)) {  goto case_2682;  }
      bool z2zE2566;
      z2zE2566 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_14);
      if (z2zE2566) {  z3zE2698 = UINT64_C(14);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2682: ;
    {
      uint64_t z3zE2731;
      z3zE2731 = zselector;
      bool z3zE2703;
      z3zE2703 = (z3zE2731 == UINT64_C(0x000F));
      if (!(z3zE2703)) {  goto case_2681;  }
      bool z2zE2567;
      z2zE2567 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_15);
      if (z2zE2567) {  z3zE2698 = UINT64_C(15);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2681: ;
    {
      uint64_t z3zE2732;
      z3zE2732 = zselector;
      bool z3zE2702;
      z3zE2702 = (z3zE2732 == UINT64_C(0x0010));
      if (!(z3zE2702)) {  goto case_2680;  }
      bool z2zE2568;
      z2zE2568 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_16);
      if (z2zE2568) {  z3zE2698 = UINT64_C(16);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2680: ;
    {
      uint64_t z3zE2733;
      z3zE2733 = zselector;
      bool z3zE2701;
      z3zE2701 = (z3zE2733 == UINT64_C(0x0011));
      if (!(z3zE2701)) {  goto case_2679;  }
      bool z2zE2569;
      z2zE2569 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_17);
      if (z2zE2569) {  z3zE2698 = UINT64_C(17);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2679: ;
    {
      uint64_t z3zE2734;
      z3zE2734 = zselector;
      bool z3zE2700;
      z3zE2700 = (z3zE2734 == UINT64_C(0x0100));
      if (!(z3zE2700)) {  goto case_2678;  }
      bool z2zE2570;
      z2zE2570 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_256);
      if (z2zE2570) {  z3zE2698 = UINT64_C(256);  } else {  z3zE2698 = UINT64_C(0);  }
      goto finish_match_2676;
    }
  case_2678: ;
    {
      z3zE2698 = UINT64_C(0);
      goto finish_match_2676;
    }
  case_2677: ;
  finish_match_2676: ;
    zcandidate = z3zE2698;
  }
  bool z2zE2551;
  z2zE2551 = (zcandidate != UINT64_C(0));
  if (z2zE2551) {
    uint64_t zid;
    zid = zcandidate;
    bool z2zE2552;
    z2zE2552 = zprecompile_active_at_fork(zid);
    if (z2zE2552) {  z8zE393 = zid;  } else {  z8zE393 = UINT64_C(0);  }
  } else {  z8zE393 = UINT64_C(0);  }
end_function_2696: ;
  return z8zE393;
end_block_exception_2697: ;

  return UINT64_C(0xdeadc0de);
}

struct zPrecompileResult zrun_ecrecover(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE394;
  sail_u256 zv;
  zv = zslice_load(zinput, zPRECOMPILE_WORD_OFFSET);
  bool zvalid_v;
  {
    bool z2zE2550;
    z2zE2550 = u256_eq_u64(zv, UINT64_C(27));
    bool z3zE2695;
    if (z2zE2550) {  z3zE2695 = true;  } else {  z3zE2695 = u256_eq_u64(zv, UINT64_C(28));  }
    zvalid_v = z3zE2695;
  }
  if (zvalid_v) {
    uint64_t zparity;
    {
      bool z2zE2549;
      z2zE2549 = u256_eq_u64(zv, UINT64_C(27));
      if (z2zE2549) {  zparity = UINT64_C(0);  } else {  zparity = UINT64_C(1);  }
    }
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2546;
    {
      sail_fixed_bytes_32 z2zE2543;
      {
        sail_u256 z2zE2542;
        z2zE2542 = zslice_load(zinput, UINT64_C(0));
        z2zE2543 = zword_to_hash(z2zE2542);
      }
      sail_u256 z2zE2544;
      z2zE2544 = zslice_load(zinput, zPRECOMPILE_DOUBLE_WORD_OFFSET);
      sail_u256 z2zE2545;
      z2zE2545 = zslice_load(zinput, zECRECOVER_S_OFFSET);
      z2zE2546 = zecrecover_addr(z2zE2543, zparity, z2zE2544, z2zE2545);
    }
    struct zPrecompileResult z3zE2696;
    {
      bool zrecovered;
      zrecovered = z2zE2546.ztup0;
      sail_fixed_bytes_20 zaddress;
      zaddress = z2zE2546.ztup1;
      if (zrecovered) {
        struct zByteSliceFields z2zE2548;
        {
          sail_u256 z2zE2547;
          z2zE2547 = zaddress_to_word(zaddress);
          z2zE2548 = zoutput_buffer_word(z2zE2547);
        }
        z3zE2696 = zprecompile_success(z2zE2548);
      } else {  z3zE2696 = zprecompile_success(zEMPTY_SLICE);  }
      goto finish_match_2672;
    }
  case_2673: ;
    sail_match_failure("run_ecrecover");
  finish_match_2672: ;
    z8zE394 = z3zE2696;
  } else {  z8zE394 = zprecompile_success(zEMPTY_SLICE);  }
end_function_2674: ;
  return z8zE394;
end_block_exception_2675: ;
  struct zByteSliceFields z8zE937 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE936 = { .zoutput = z8zE937, .zsuccess = false };
  return z8zE936;
}

struct zPrecompileResult zrun_sha256(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE395;
  struct zByteSliceFields z2zE2541;
  {
    sail_u256 z2zE2540;
    {
      sail_fixed_bytes_32 z2zE2539;
      z2zE2539 = zsha256_slice(zinput);
      z2zE2540 = zhash_to_word(z2zE2539);
    }
    z2zE2541 = zoutput_buffer_word(z2zE2540);
  }
  z8zE395 = zprecompile_success(z2zE2541);
end_function_2670: ;
  return z8zE395;
end_block_exception_2671: ;
  struct zByteSliceFields z8zE939 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE938 = { .zoutput = z8zE939, .zsuccess = false };
  return z8zE938;
}

struct zPrecompileResult zrun_ripemd160(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE396;
  bool z2zE2538;
  z2zE2538 = accelerator_ripemd160(zinput);
  z8zE396 = zaccelerator_result(z2zE2538, zPRECOMPILE_WORD_LENGTH);
end_function_2668: ;
  return z8zE396;
end_block_exception_2669: ;
  struct zByteSliceFields z8zE941 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE940 = { .zoutput = z8zE941, .zsuccess = false };
  return z8zE940;
}

struct zPrecompileResult zrun_modexp(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE397;
  sail_u256 zbase_len;
  zbase_len = zpc_word(zinput, UINT64_C(0), UINT64_C(32));
  sail_u256 zexponent_len;
  zexponent_len = zpc_word(zinput, UINT64_C(32), UINT64_C(32));
  sail_u256 zmodulus_len;
  zmodulus_len = zpc_word(zinput, UINT64_C(64), UINT64_C(32));
  bool z2zE2530;
  z2zE2530 = u256_eq_u64(zmodulus_len, UINT64_C(0));
  if (z2zE2530) {  z8zE397 = zprecompile_success(zEMPTY_SLICE);  } else {
    bool z2zE2533;
    {
      bool z2zE2532;
      z2zE2532 = u64_lt_u256(zACCELERATOR_INPUT_MAX, zbase_len);
      bool z3zE2694;
      if (z2zE2532) {  z3zE2694 = true;  } else {
        bool z2zE2531;
        z2zE2531 = u64_lt_u256(zACCELERATOR_INPUT_MAX, zexponent_len);
        bool z3zE2693;
        if (z2zE2531) {  z3zE2693 = true;  } else {  z3zE2693 = u64_lt_u256(zACCELERATOR_INPUT_MAX, zmodulus_len);  }
        z3zE2694 = z3zE2693;
      }
      z2zE2533 = z3zE2694;
    }
    if (z2zE2533) {  z8zE397 = zprecompile_failure(UNIT);  } else {
      uint64_t zbounded_base;
      zbounded_base = u256_to_u64(zbase_len);
      uint64_t zbounded_exponent;
      zbounded_exponent = u256_to_u64(zexponent_len);
      uint64_t zbounded_modulus;
      zbounded_modulus = u256_to_u64(zmodulus_len);
      uint64_t zinput_end;
      {
        uint64_t z2zE2537;
        {
          uint64_t z2zE2536;
          {    z2zE2536 = (UINT64_C(96) + zbounded_base);
          }
          {    z2zE2537 = (z2zE2536 + zbounded_exponent);
          }
        }
        {    zinput_end = (z2zE2537 + zbounded_modulus);
        }
      }
      bool z2zE2534;
      z2zE2534 = (zACCELERATOR_INPUT_MAX < zinput_end);
      if (z2zE2534) {  z8zE397 = zprecompile_failure(UNIT);  } else {
        bool z2zE2535;
        z2zE2535 = accelerator_modexp(zinput, zbounded_base, zbounded_exponent, zbounded_modulus);
        z8zE397 = zaccelerator_result(z2zE2535, zbounded_modulus);
      }
    }
  }
end_function_2666: ;
  return z8zE397;
end_block_exception_2667: ;
  struct zByteSliceFields z8zE943 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE942 = { .zoutput = z8zE943, .zsuccess = false };
  return z8zE942;
}

struct zPrecompileResult zpairing_result(uint64_t zresult)
{
  struct zPrecompileResult z8zE398;
  bool z2zE2527;
  {
    uint64_t z2zE2526;
    z2zE2526 = (UINT64_C(1) & (zresult >> UINT64_C(1)));
    z2zE2527 = (z2zE2526 == UINT64_C(0b0));
  }
  if (z2zE2527) {  z8zE398 = zprecompile_failure(UNIT);  } else {
    bool z2zE2529;
    {
      uint64_t z2zE2528;
      z2zE2528 = (UINT64_C(1) & (zresult >> UINT64_C(0)));
      z2zE2529 = (z2zE2528 == UINT64_C(0b1));
    }
    z8zE398 = zboolean_result(z2zE2529);
  }
end_function_2664: ;
  return z8zE398;
end_block_exception_2665: ;
  struct zByteSliceFields z8zE945 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE944 = { .zoutput = z8zE945, .zsuccess = false };
  return z8zE944;
}

struct zPrecompileResult zrun_blake2f(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE399;
  uint64_t zfinal_byte;
  zfinal_byte = zslice_byte(zinput, zBLAKE2F_FINAL_BLOCK_OFFSET);
  bool z2zE2522;
  {
    bool z2zE2521;
    {
      uint64_t z2zE2519;
      z2zE2519 = zinput.zlen;
      z2zE2521 = (z2zE2519 != zBLAKE2F_INPUT_LENGTH);
    }
    bool z3zE2692;
    if (z2zE2521) {  z3zE2692 = true;  } else {
      bool z2zE2520;
      z2zE2520 = (zfinal_byte != UINT64_C(0x00));
      bool z3zE2691;
      if (z2zE2520) {  z3zE2691 = (zfinal_byte != UINT64_C(0x01));  } else {  z3zE2691 = false;  }
      z3zE2692 = z3zE2691;
    }
    z2zE2522 = z3zE2692;
  }
  if (z2zE2522) {  z8zE399 = zprecompile_failure(UNIT);  } else {
    uint64_t zfinal_block;
    {
      bool z2zE2525;
      z2zE2525 = (zfinal_byte == UINT64_C(0x00));
      if (z2zE2525) {  zfinal_block = UINT64_C(0);  } else {  zfinal_block = UINT64_C(1);  }
    }
    bool z2zE2524;
    {
      uint64_t z2zE2523;
      z2zE2523 = zpc_blake2_rounds(zinput);
      z2zE2524 = accelerator_blake2f(zinput, z2zE2523, zfinal_block);
    }
    z8zE399 = zaccelerator_result(z2zE2524, zBLAKE2F_OUTPUT_LENGTH);
  }
end_function_2662: ;
  return z8zE399;
end_block_exception_2663: ;
  struct zByteSliceFields z8zE947 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE946 = { .zoutput = z8zE947, .zsuccess = false };
  return z8zE946;
}

bool zkzzg_versioned_hash_matches(struct zByteSliceFields zinput)
{
  bool z8zE400;
  sail_fixed_bytes_32 zcommitment_hash;
  {
    struct zByteSliceFields z2zE2518;
    z2zE2518 = zsub_slice(zinput, zKZG_COMMITMENT_OFFSET, zKZG_COMMITMENT_LENGTH);
    zcommitment_hash = zsha256_slice(z2zE2518);
  }
  sail_fixed_bytes_32 zexpected;
  zexpected = zcommitment_hash;
  zexpected = fast_unsigned_vector_update_fixed_bytes_32(zexpected, UINT64_C(31), UINT64_C(0x01));
  unit z3zE2690;
  z3zE2690 = UNIT;
  sail_fixed_bytes_32 z2zE2517;
  {
    sail_u256 z2zE2516;
    z2zE2516 = zslice_load(zinput, UINT64_C(0));
    z2zE2517 = zword_to_hash(z2zE2516);
  }
  z8zE400 = eq_fixed_bytes_32(z2zE2517, zexpected);
end_function_2660: ;
  return z8zE400;
end_block_exception_2661: ;

  return false;
}

struct zPrecompileResult zrun_kzzg_point_evaluation(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE401;
  bool z2zE2511;
  {
    uint64_t z2zE2510;
    z2zE2510 = zinput.zlen;
    z2zE2511 = (z2zE2510 != zKZG_INPUT_LENGTH);
  }
  if (z2zE2511) {  z8zE401 = zprecompile_failure(UNIT);  } else {
    bool z2zE2513;
    {
      bool z2zE2512;
      z2zE2512 = zkzzg_versioned_hash_matches(zinput);
      z2zE2513 = not(z2zE2512);
    }
    if (z2zE2513) {  z8zE401 = zprecompile_failure(UNIT);  } else {
      bool z2zE2514;
      z2zE2514 = accelerator_kzg_point_evaluation(zinput);
      if (z2zE2514) {
        struct zByteSliceFields z2zE2515;
        z2zE2515 = zoutput_buffer_words(zFIELD_ELEMENTS_PER_BLOB, zBLS_MODULUS);
        z8zE401 = zprecompile_success(z2zE2515);
      } else {  z8zE401 = zprecompile_failure(UNIT);  }
    }
  }
end_function_2658: ;
  return z8zE401;
end_block_exception_2659: ;
  struct zByteSliceFields z8zE949 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE948 = { .zoutput = z8zE949, .zsuccess = false };
  return z8zE948;
}

bool zbls_g1_padding(struct zByteSliceFields zinput, uint64_t zbase, uint64_t zstride, uint64_t zcount)
{
  bool z8zE402;
  bool z2zE2509;
  z2zE2509 = zslice_strided_zzero(zinput, zbase, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
  bool z3zE2689;
  if (z2zE2509) {
    uint64_t z2zE2508;
    {    z2zE2508 = (zbase + zBLS_PADDED_FIELD_LENGTH);
    }
    z3zE2689 = zslice_strided_zzero(zinput, z2zE2508, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
  } else {  z3zE2689 = false;  }
  z8zE402 = z3zE2689;
end_function_2656: ;
  return z8zE402;
end_block_exception_2657: ;

  return false;
}

bool zbls_g2_padding(struct zByteSliceFields zinput, uint64_t zbase, uint64_t zstride, uint64_t zcount)
{
  bool z8zE403;
  bool z2zE2507;
  z2zE2507 = zslice_strided_zzero(zinput, zbase, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
  bool z3zE2688;
  if (z2zE2507) {
    bool z2zE2506;
    {
      uint64_t z2zE2502;
      {    z2zE2502 = (zbase + zBLS_PADDED_FIELD_LENGTH);
      }
      z2zE2506 = zslice_strided_zzero(zinput, z2zE2502, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
    }
    bool z3zE2687;
    if (z2zE2506) {
      bool z2zE2505;
      {
        uint64_t z2zE2503;
        {    z2zE2503 = (zbase + zBLS_G1_POINT_LENGTH);
        }
        z2zE2505 = zslice_strided_zzero(zinput, z2zE2503, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
      }
      bool z3zE2686;
      if (z2zE2505) {
        uint64_t z2zE2504;
        {    z2zE2504 = (zbase + zBLS_G2_FINAL_FIELD_OFFSET);
        }
        z3zE2686 = zslice_strided_zzero(zinput, z2zE2504, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
      } else {  z3zE2686 = false;  }
      z3zE2687 = z3zE2686;
    } else {  z3zE2687 = false;  }
    z3zE2688 = z3zE2687;
  } else {  z3zE2688 = false;  }
  z8zE403 = z3zE2688;
end_function_2654: ;
  return z8zE403;
end_block_exception_2655: ;

  return false;
}

struct zPrecompileResult zrun_bls_g1_add(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE404;
  bool z2zE2498;
  {
    uint64_t z2zE2497;
    z2zE2497 = zinput.zlen;
    z2zE2498 = (z2zE2497 != zBLS_G1_ADD_INPUT_LENGTH);
  }
  if (z2zE2498) {  z8zE404 = zprecompile_failure(UNIT);  } else {
    bool z2zE2500;
    {
      bool z2zE2499;
      z2zE2499 = zbls_g1_padding(zinput, UINT64_C(0), zBLS_G1_POINT_LENGTH, zTWO_COMPONENTS);
      z2zE2500 = not(z2zE2499);
    }
    if (z2zE2500) {  z8zE404 = zprecompile_failure(UNIT);  } else {
      bool z2zE2501;
      z2zE2501 = accelerator_bls_g1_add(zinput);
      z8zE404 = zaccelerator_result(z2zE2501, zBLS_G1_POINT_LENGTH);
    }
  }
end_function_2652: ;
  return z8zE404;
end_block_exception_2653: ;
  struct zByteSliceFields z8zE951 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE950 = { .zoutput = z8zE951, .zsuccess = false };
  return z8zE950;
}

struct zPrecompileResult zrun_bls_g1_msm(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE405;
  uint64_t zlength;
  zlength = zinput.zlen;
  uint64_t zitem_length;
  zitem_length = zBLS_G1_MSM_ITEM_LENGTH;
  uint64_t zpairs;
  zpairs = (zlength / zitem_length);
  bool z2zE2493;
  {
    bool z2zE2492;
    z2zE2492 = (zlength == UINT64_C(0));
    bool z3zE2685;
    if (z2zE2492) {  z3zE2685 = true;  } else {
      uint64_t z2zE2491;
      {    z2zE2491 = (zpairs * zitem_length);
      }
      z3zE2685 = (zlength != z2zE2491);
    }
    z2zE2493 = z3zE2685;
  }
  if (z2zE2493) {  z8zE405 = zprecompile_failure(UNIT);  } else {
    bool z2zE2495;
    {
      bool z2zE2494;
      z2zE2494 = zbls_g1_padding(zinput, UINT64_C(0), zBLS_G1_MSM_ITEM_LENGTH, zpairs);
      z2zE2495 = not(z2zE2494);
    }
    if (z2zE2495) {  z8zE405 = zprecompile_failure(UNIT);  } else {
      bool z2zE2496;
      z2zE2496 = accelerator_bls_g1_msm(zinput);
      z8zE405 = zaccelerator_result(z2zE2496, zBLS_G1_POINT_LENGTH);
    }
  }
end_function_2650: ;
  return z8zE405;
end_block_exception_2651: ;
  struct zByteSliceFields z8zE953 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE952 = { .zoutput = z8zE953, .zsuccess = false };
  return z8zE952;
}

struct zPrecompileResult zrun_bls_g2_add(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE406;
  bool z2zE2487;
  {
    uint64_t z2zE2486;
    z2zE2486 = zinput.zlen;
    z2zE2487 = (z2zE2486 != zBLS_G2_ADD_INPUT_LENGTH);
  }
  if (z2zE2487) {  z8zE406 = zprecompile_failure(UNIT);  } else {
    bool z2zE2489;
    {
      bool z2zE2488;
      z2zE2488 = zbls_g2_padding(zinput, UINT64_C(0), zBLS_G2_POINT_LENGTH, zTWO_COMPONENTS);
      z2zE2489 = not(z2zE2488);
    }
    if (z2zE2489) {  z8zE406 = zprecompile_failure(UNIT);  } else {
      bool z2zE2490;
      z2zE2490 = accelerator_bls_g2_add(zinput);
      z8zE406 = zaccelerator_result(z2zE2490, zBLS_G2_POINT_LENGTH);
    }
  }
end_function_2648: ;
  return z8zE406;
end_block_exception_2649: ;
  struct zByteSliceFields z8zE955 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE954 = { .zoutput = z8zE955, .zsuccess = false };
  return z8zE954;
}

struct zPrecompileResult zrun_bls_g2_msm(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE407;
  uint64_t zlength;
  zlength = zinput.zlen;
  uint64_t zitem_length;
  zitem_length = zBLS_G2_MSM_ITEM_LENGTH;
  uint64_t zpairs;
  zpairs = (zlength / zitem_length);
  bool z2zE2482;
  {
    bool z2zE2481;
    z2zE2481 = (zlength == UINT64_C(0));
    bool z3zE2684;
    if (z2zE2481) {  z3zE2684 = true;  } else {
      uint64_t z2zE2480;
      {    z2zE2480 = (zpairs * zitem_length);
      }
      z3zE2684 = (zlength != z2zE2480);
    }
    z2zE2482 = z3zE2684;
  }
  if (z2zE2482) {  z8zE407 = zprecompile_failure(UNIT);  } else {
    bool z2zE2484;
    {
      bool z2zE2483;
      z2zE2483 = zbls_g2_padding(zinput, UINT64_C(0), zBLS_G2_MSM_ITEM_LENGTH, zpairs);
      z2zE2484 = not(z2zE2483);
    }
    if (z2zE2484) {  z8zE407 = zprecompile_failure(UNIT);  } else {
      bool z2zE2485;
      z2zE2485 = accelerator_bls_g2_msm(zinput);
      z8zE407 = zaccelerator_result(z2zE2485, zBLS_G2_POINT_LENGTH);
    }
  }
end_function_2646: ;
  return z8zE407;
end_block_exception_2647: ;
  struct zByteSliceFields z8zE957 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE956 = { .zoutput = z8zE957, .zsuccess = false };
  return z8zE956;
}

struct zPrecompileResult zrun_bls_pairing(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE408;
  uint64_t zlength;
  zlength = zinput.zlen;
  uint64_t zitem_length;
  zitem_length = zBLS_PAIRING_ITEM_LENGTH;
  uint64_t zpairs;
  zpairs = (zlength / zitem_length);
  bool z2zE2476;
  {
    bool z2zE2475;
    z2zE2475 = (zlength == UINT64_C(0));
    bool z3zE2682;
    if (z2zE2475) {  z3zE2682 = true;  } else {
      uint64_t z2zE2474;
      {    z2zE2474 = (zpairs * zitem_length);
      }
      z3zE2682 = (zlength != z2zE2474);
    }
    z2zE2476 = z3zE2682;
  }
  if (z2zE2476) {  z8zE408 = zprecompile_failure(UNIT);  } else {
    uint64_t zpair_count;
    zpair_count = zpairs;
    bool z2zE2478;
    {
      bool z2zE2477;
      z2zE2477 = zbls_g1_padding(zinput, UINT64_C(0), zBLS_PAIRING_ITEM_LENGTH, zpair_count);
      bool z3zE2683;
      if (z2zE2477) {
        z3zE2683 = zbls_g2_padding(zinput, zBLS_G2_POINT_OFFSET, zBLS_PAIRING_ITEM_LENGTH, zpair_count);
      } else {  z3zE2683 = false;  }
      z2zE2478 = z3zE2683;
    }
    if (z2zE2478) {
      uint64_t z2zE2479;
      z2zE2479 = accelerator_bls_pairing(zinput);
      z8zE408 = zpairing_result(z2zE2479);
    } else {  z8zE408 = zprecompile_failure(UNIT);  }
  }
end_function_2644: ;
  return z8zE408;
end_block_exception_2645: ;
  struct zByteSliceFields z8zE959 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE958 = { .zoutput = z8zE959, .zsuccess = false };
  return z8zE958;
}

struct zPrecompileResult zrun_bls_map_fp_to_g1(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE409;
  bool z2zE2470;
  {
    uint64_t z2zE2469;
    z2zE2469 = zinput.zlen;
    z2zE2470 = (z2zE2469 != zBLS_PADDED_FIELD_LENGTH);
  }
  if (z2zE2470) {  z8zE409 = zprecompile_failure(UNIT);  } else {
    bool z2zE2472;
    {
      bool z2zE2471;
      z2zE2471 = zslice_strided_zzero(zinput, UINT64_C(0), zBLS_PADDED_FIELD_LENGTH, zBLS_FIELD_PADDING_LENGTH, UINT64_C(1));
      z2zE2472 = not(z2zE2471);
    }
    if (z2zE2472) {  z8zE409 = zprecompile_failure(UNIT);  } else {
      bool z2zE2473;
      z2zE2473 = accelerator_bls_map_fp_to_g1(zinput);
      z8zE409 = zaccelerator_result(z2zE2473, zBLS_G1_POINT_LENGTH);
    }
  }
end_function_2642: ;
  return z8zE409;
end_block_exception_2643: ;
  struct zByteSliceFields z8zE961 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE960 = { .zoutput = z8zE961, .zsuccess = false };
  return z8zE960;
}

struct zPrecompileResult zrun_bls_map_fp2_to_g2(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE410;
  bool z2zE2465;
  {
    uint64_t z2zE2464;
    z2zE2464 = zinput.zlen;
    z2zE2465 = (z2zE2464 != zBLS_G1_POINT_LENGTH);
  }
  if (z2zE2465) {  z8zE410 = zprecompile_failure(UNIT);  } else {
    bool z2zE2467;
    {
      bool z2zE2466;
      z2zE2466 = zslice_strided_zzero(zinput, UINT64_C(0), zBLS_PADDED_FIELD_LENGTH, zBLS_FIELD_PADDING_LENGTH, zTWO_COMPONENTS);
      z2zE2467 = not(z2zE2466);
    }
    if (z2zE2467) {  z8zE410 = zprecompile_failure(UNIT);  } else {
      bool z2zE2468;
      z2zE2468 = accelerator_bls_map_fp2_to_g2(zinput);
      z8zE410 = zaccelerator_result(z2zE2468, zBLS_G2_POINT_LENGTH);
    }
  }
end_function_2640: ;
  return z8zE410;
end_block_exception_2641: ;
  struct zByteSliceFields z8zE963 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE962 = { .zoutput = z8zE963, .zsuccess = false };
  return z8zE962;
}

struct zPrecompileResult zrun_p256_verify(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE411;
  bool zverified;
  {
    bool z2zE2463;
    {
      uint64_t z2zE2462;
      z2zE2462 = zinput.zlen;
      z2zE2463 = (z2zE2462 == zP256_INPUT_LENGTH);
    }
    if (z2zE2463) {  zverified = accelerator_p256_verify(zinput);  } else {  zverified = false;  }
  }
  if (zverified) {
    struct zByteSliceFields z2zE2461;
    z2zE2461 = zoutput_buffer_word(zWORD_ONE);
    z8zE411 = zprecompile_success(z2zE2461);
  } else {  z8zE411 = zprecompile_success(zEMPTY_SLICE);  }
end_function_2638: ;
  return z8zE411;
end_block_exception_2639: ;
  struct zByteSliceFields z8zE965 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE964 = { .zoutput = z8zE965, .zsuccess = false };
  return z8zE964;
}

struct zPrecompileResult zrun_precompile_slice(uint64_t znum, struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE412;
  struct zPrecompileResult z3zE2645;
  {
    uint64_t zp0z3;
    zp0z3 = znum;
    bool z3zE2664;
    z3zE2664 = (zp0z3 == UINT64_C(1));
    if (!(z3zE2664)) {  goto case_2635;  }
    z3zE2645 = zrun_ecrecover(zinput);
    goto finish_match_2616;
  }
case_2635: ;
  {
    uint64_t z3zE2665;
    z3zE2665 = znum;
    bool z3zE2663;
    z3zE2663 = (z3zE2665 == UINT64_C(2));
    if (!(z3zE2663)) {  goto case_2634;  }
    z3zE2645 = zrun_sha256(zinput);
    goto finish_match_2616;
  }
case_2634: ;
  {
    uint64_t z3zE2666;
    z3zE2666 = znum;
    bool z3zE2662;
    z3zE2662 = (z3zE2666 == UINT64_C(3));
    if (!(z3zE2662)) {  goto case_2633;  }
    z3zE2645 = zrun_ripemd160(zinput);
    goto finish_match_2616;
  }
case_2633: ;
  {
    uint64_t z3zE2667;
    z3zE2667 = znum;
    bool z3zE2661;
    z3zE2661 = (z3zE2667 == UINT64_C(4));
    if (!(z3zE2661)) {  goto case_2632;  }
    z3zE2645 = zcopied_result(zinput);
    goto finish_match_2616;
  }
case_2632: ;
  {
    uint64_t z3zE2668;
    z3zE2668 = znum;
    bool z3zE2660;
    z3zE2660 = (z3zE2668 == UINT64_C(5));
    if (!(z3zE2660)) {  goto case_2631;  }
    z3zE2645 = zrun_modexp(zinput);
    goto finish_match_2616;
  }
case_2631: ;
  {
    uint64_t z3zE2669;
    z3zE2669 = znum;
    bool z3zE2659;
    z3zE2659 = (z3zE2669 == UINT64_C(6));
    if (!(z3zE2659)) {  goto case_2630;  }
    bool z2zE2455;
    z2zE2455 = accelerator_bn254_add(zinput);
    z3zE2645 = zaccelerator_result(z2zE2455, zPRECOMPILE_DOUBLE_WORD_LENGTH);
    goto finish_match_2616;
  }
case_2630: ;
  {
    uint64_t z3zE2670;
    z3zE2670 = znum;
    bool z3zE2658;
    z3zE2658 = (z3zE2670 == UINT64_C(7));
    if (!(z3zE2658)) {  goto case_2629;  }
    bool z2zE2456;
    z2zE2456 = accelerator_bn254_mul(zinput);
    z3zE2645 = zaccelerator_result(z2zE2456, zPRECOMPILE_DOUBLE_WORD_LENGTH);
    goto finish_match_2616;
  }
case_2629: ;
  {
    uint64_t z3zE2671;
    z3zE2671 = znum;
    bool z3zE2657;
    z3zE2657 = (z3zE2671 == UINT64_C(8));
    if (!(z3zE2657)) {  goto case_2628;  }
    uint64_t zinput_length;
    zinput_length = zinput.zlen;
    uint64_t zitem_length;
    zitem_length = zBN254_PAIRING_ITEM_LENGTH;
    bool z2zE2459;
    {
      uint64_t z2zE2458;
      {
        uint64_t z2zE2457;
        z2zE2457 = (zinput_length / zitem_length);
        {    z2zE2458 = (z2zE2457 * zitem_length);
        }
      }
      z2zE2459 = (zinput_length == z2zE2458);
    }
    if (z2zE2459) {
      uint64_t z2zE2460;
      z2zE2460 = accelerator_bn254_pairing(zinput);
      z3zE2645 = zpairing_result(z2zE2460);
    } else {  z3zE2645 = zprecompile_failure(UNIT);  }
    goto finish_match_2616;
  }
case_2628: ;
  {
    uint64_t z3zE2672;
    z3zE2672 = znum;
    bool z3zE2656;
    z3zE2656 = (z3zE2672 == UINT64_C(9));
    if (!(z3zE2656)) {  goto case_2627;  }
    z3zE2645 = zrun_blake2f(zinput);
    goto finish_match_2616;
  }
case_2627: ;
  {
    uint64_t z3zE2673;
    z3zE2673 = znum;
    bool z3zE2655;
    z3zE2655 = (z3zE2673 == UINT64_C(10));
    if (!(z3zE2655)) {  goto case_2626;  }
    z3zE2645 = zrun_kzzg_point_evaluation(zinput);
    goto finish_match_2616;
  }
case_2626: ;
  {
    uint64_t z3zE2674;
    z3zE2674 = znum;
    bool z3zE2654;
    z3zE2654 = (z3zE2674 == UINT64_C(11));
    if (!(z3zE2654)) {  goto case_2625;  }
    z3zE2645 = zrun_bls_g1_add(zinput);
    goto finish_match_2616;
  }
case_2625: ;
  {
    uint64_t z3zE2675;
    z3zE2675 = znum;
    bool z3zE2653;
    z3zE2653 = (z3zE2675 == UINT64_C(12));
    if (!(z3zE2653)) {  goto case_2624;  }
    z3zE2645 = zrun_bls_g1_msm(zinput);
    goto finish_match_2616;
  }
case_2624: ;
  {
    uint64_t z3zE2676;
    z3zE2676 = znum;
    bool z3zE2652;
    z3zE2652 = (z3zE2676 == UINT64_C(13));
    if (!(z3zE2652)) {  goto case_2623;  }
    z3zE2645 = zrun_bls_g2_add(zinput);
    goto finish_match_2616;
  }
case_2623: ;
  {
    uint64_t z3zE2677;
    z3zE2677 = znum;
    bool z3zE2651;
    z3zE2651 = (z3zE2677 == UINT64_C(14));
    if (!(z3zE2651)) {  goto case_2622;  }
    z3zE2645 = zrun_bls_g2_msm(zinput);
    goto finish_match_2616;
  }
case_2622: ;
  {
    uint64_t z3zE2678;
    z3zE2678 = znum;
    bool z3zE2650;
    z3zE2650 = (z3zE2678 == UINT64_C(15));
    if (!(z3zE2650)) {  goto case_2621;  }
    z3zE2645 = zrun_bls_pairing(zinput);
    goto finish_match_2616;
  }
case_2621: ;
  {
    uint64_t z3zE2679;
    z3zE2679 = znum;
    bool z3zE2649;
    z3zE2649 = (z3zE2679 == UINT64_C(16));
    if (!(z3zE2649)) {  goto case_2620;  }
    z3zE2645 = zrun_bls_map_fp_to_g1(zinput);
    goto finish_match_2616;
  }
case_2620: ;
  {
    uint64_t z3zE2680;
    z3zE2680 = znum;
    bool z3zE2648;
    z3zE2648 = (z3zE2680 == UINT64_C(17));
    if (!(z3zE2648)) {  goto case_2619;  }
    z3zE2645 = zrun_bls_map_fp2_to_g2(zinput);
    goto finish_match_2616;
  }
case_2619: ;
  {
    uint64_t z3zE2681;
    z3zE2681 = znum;
    bool z3zE2647;
    z3zE2647 = (z3zE2681 == UINT64_C(256));
    if (!(z3zE2647)) {  goto case_2618;  }
    z3zE2645 = zrun_p256_verify(zinput);
    goto finish_match_2616;
  }
case_2618: ;
  {
    z3zE2645 = zprecompile_failure(UNIT);
    goto finish_match_2616;
  }
case_2617: ;
finish_match_2616: ;
  z8zE412 = z3zE2645;
end_function_2636: ;
  return z8zE412;
end_block_exception_2637: ;
  struct zByteSliceFields z8zE967 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE966 = { .zoutput = z8zE967, .zsuccess = false };
  return z8zE966;
}

