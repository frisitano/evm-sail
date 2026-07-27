/* Generated from sail/evm/precompiles.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_172(void) {

  uint64_t z3zE193;
  z3zE193 = UINT64_C(2097152);
  zACCELERATOR_INPUT_MAX = z3zE193;
let_end_550: ;
}
void kill_letbind_172(void) {
}

void create_letbind_173(void) {

  sail_u256 z3zE194;
  z3zE194 = u256_of_fbits(UINT64_C(4096));
  zFIELD_ELEMENTS_PER_BLOB = z3zE194;
let_end_551: ;
}
void kill_letbind_173(void) {
}

void create_letbind_174(void) {

  sail_u256 z3zE195;
  z3zE195 = ((sail_u256){{UINT64_C(18446744069414584321), UINT64_C(6034159408538082302), UINT64_C(3691218898639771653), UINT64_C(8353516859464449352)}});
  zBLS_MODULUS = z3zE195;
let_end_552: ;
}
void kill_letbind_174(void) {
}

void create_letbind_175(void) {

  uint64_t z3zE196;
  z3zE196 = UINT64_C(213);
  zBLAKE2F_INPUT_LENGTH = z3zE196;
let_end_553: ;
}
void kill_letbind_175(void) {
}

void create_letbind_176(void) {

  uint64_t z3zE197;
  z3zE197 = UINT64_C(212);
  zBLAKE2F_FINAL_BLOCK_OFFSET = z3zE197;
let_end_554: ;
}
void kill_letbind_176(void) {
}

void create_letbind_177(void) {

  uint64_t z3zE198;
  z3zE198 = zDOUBLE_WORD_BYTE_LENGTH;
  zBLAKE2F_OUTPUT_LENGTH = z3zE198;
let_end_555: ;
}
void kill_letbind_177(void) {
}

void create_letbind_178(void) {

  uint64_t z3zE199;
  z3zE199 = UINT64_C(192);
  zKZG_INPUT_LENGTH = z3zE199;
let_end_556: ;
}
void kill_letbind_178(void) {
}

void create_letbind_179(void) {

  uint64_t z3zE200;
  z3zE200 = UINT64_C(96);
  zKZG_COMMITMENT_OFFSET = z3zE200;
let_end_557: ;
}
void kill_letbind_179(void) {
}

void create_letbind_180(void) {

  uint64_t z3zE201;
  z3zE201 = UINT64_C(48);
  zKZG_COMMITMENT_LENGTH = z3zE201;
let_end_558: ;
}
void kill_letbind_180(void) {
}

void create_letbind_181(void) {

  uint64_t z3zE202;
  z3zE202 = UINT64_C(16);
  zBLS_FIELD_PADDING_LENGTH = z3zE202;
let_end_559: ;
}
void kill_letbind_181(void) {
}

void create_letbind_182(void) {

  uint64_t z3zE203;
  z3zE203 = zDOUBLE_WORD_BYTE_LENGTH;
  zBLS_PADDED_FIELD_LENGTH = z3zE203;
let_end_560: ;
}
void kill_letbind_182(void) {
}

void create_letbind_183(void) {

  uint64_t z3zE204;
  z3zE204 = UINT64_C(128);
  zBLS_G1_POINT_LENGTH = z3zE204;
let_end_561: ;
}
void kill_letbind_183(void) {
}

void create_letbind_184(void) {

  uint64_t z3zE205;
  z3zE205 = UINT64_C(256);
  zBLS_G2_POINT_LENGTH = z3zE205;
let_end_562: ;
}
void kill_letbind_184(void) {
}

void create_letbind_185(void) {

  uint64_t z3zE206;
  z3zE206 = UINT64_C(192);
  zBLS_G2_FINAL_FIELD_OFFSET = z3zE206;
let_end_563: ;
}
void kill_letbind_185(void) {
}

void create_letbind_186(void) {

  uint64_t z3zE207;
  z3zE207 = UINT64_C(256);
  zBLS_G1_ADD_INPUT_LENGTH = z3zE207;
let_end_564: ;
}
void kill_letbind_186(void) {
}

void create_letbind_187(void) {

  uint64_t z3zE208;
  z3zE208 = UINT64_C(160);
  zBLS_G1_MSM_ITEM_LENGTH = z3zE208;
let_end_565: ;
}
void kill_letbind_187(void) {
}

void create_letbind_188(void) {

  uint64_t z3zE209;
  z3zE209 = UINT64_C(512);
  zBLS_G2_ADD_INPUT_LENGTH = z3zE209;
let_end_566: ;
}
void kill_letbind_188(void) {
}

void create_letbind_189(void) {

  uint64_t z3zE210;
  z3zE210 = UINT64_C(288);
  zBLS_G2_MSM_ITEM_LENGTH = z3zE210;
let_end_567: ;
}
void kill_letbind_189(void) {
}

void create_letbind_190(void) {

  uint64_t z3zE211;
  z3zE211 = UINT64_C(384);
  zBLS_PAIRING_ITEM_LENGTH = z3zE211;
let_end_568: ;
}
void kill_letbind_190(void) {
}

void create_letbind_191(void) {

  uint64_t z3zE212;
  z3zE212 = UINT64_C(160);
  zP256_INPUT_LENGTH = z3zE212;
let_end_569: ;
}
void kill_letbind_191(void) {
}

void create_letbind_192(void) {

  uint64_t z3zE213;
  z3zE213 = UINT64_C(192);
  zBN254_PAIRING_ITEM_LENGTH = z3zE213;
let_end_570: ;
}
void kill_letbind_192(void) {
}

void create_letbind_193(void) {

  uint64_t z3zE214;
  z3zE214 = zWORD_BYTE_LENGTH;
  zPRECOMPILE_WORD_LENGTH = z3zE214;
let_end_571: ;
}
void kill_letbind_193(void) {
}

void create_letbind_194(void) {

  uint64_t z3zE215;
  z3zE215 = zDOUBLE_WORD_BYTE_LENGTH;
  zPRECOMPILE_DOUBLE_WORD_LENGTH = z3zE215;
let_end_572: ;
}
void kill_letbind_194(void) {
}

void create_letbind_195(void) {

  uint64_t z3zE216;
  z3zE216 = UINT64_C(32);
  zPRECOMPILE_WORD_OFFSET = z3zE216;
let_end_573: ;
}
void kill_letbind_195(void) {
}

void create_letbind_196(void) {

  uint64_t z3zE217;
  z3zE217 = UINT64_C(64);
  zPRECOMPILE_DOUBLE_WORD_OFFSET = z3zE217;
let_end_574: ;
}
void kill_letbind_196(void) {
}

void create_letbind_197(void) {

  uint64_t z3zE218;
  z3zE218 = UINT64_C(96);
  zECRECOVER_S_OFFSET = z3zE218;
let_end_575: ;
}
void kill_letbind_197(void) {
}

void create_letbind_198(void) {

  uint64_t z3zE219;
  z3zE219 = UINT64_C(2);
  zTWO_COMPONENTS = z3zE219;
let_end_576: ;
}
void kill_letbind_198(void) {
}

void create_letbind_199(void) {

  uint64_t z3zE220;
  z3zE220 = UINT64_C(128);
  zBLS_G2_POINT_OFFSET = z3zE220;
let_end_577: ;
}
void kill_letbind_199(void) {
}

void create_letbind_200(void) {

  sail_fixed_bytes_20 z3zE221;
  {
    sail_u256 z3zE3029;
    z3zE3029 = u256_of_fbits(UINT64_C(1));
    z3zE221 = evmsail_word_to_address(z3zE3029);
  }
  zPRECOMPILE_ADDRESS_1 = z3zE221;
let_end_578: ;
}
void kill_letbind_200(void) {
}

void create_letbind_201(void) {

  sail_fixed_bytes_20 z3zE222;
  {
    sail_u256 z3zE3030;
    z3zE3030 = u256_of_fbits(UINT64_C(2));
    z3zE222 = evmsail_word_to_address(z3zE3030);
  }
  zPRECOMPILE_ADDRESS_2 = z3zE222;
let_end_579: ;
}
void kill_letbind_201(void) {
}

void create_letbind_202(void) {

  sail_fixed_bytes_20 z3zE223;
  {
    sail_u256 z3zE3031;
    z3zE3031 = u256_of_fbits(UINT64_C(3));
    z3zE223 = evmsail_word_to_address(z3zE3031);
  }
  zPRECOMPILE_ADDRESS_3 = z3zE223;
let_end_580: ;
}
void kill_letbind_202(void) {
}

void create_letbind_203(void) {

  sail_fixed_bytes_20 z3zE224;
  {
    sail_u256 z3zE3032;
    z3zE3032 = u256_of_fbits(UINT64_C(4));
    z3zE224 = evmsail_word_to_address(z3zE3032);
  }
  zPRECOMPILE_ADDRESS_4 = z3zE224;
let_end_581: ;
}
void kill_letbind_203(void) {
}

void create_letbind_204(void) {

  sail_fixed_bytes_20 z3zE225;
  {
    sail_u256 z3zE3033;
    z3zE3033 = u256_of_fbits(UINT64_C(5));
    z3zE225 = evmsail_word_to_address(z3zE3033);
  }
  zPRECOMPILE_ADDRESS_5 = z3zE225;
let_end_582: ;
}
void kill_letbind_204(void) {
}

void create_letbind_205(void) {

  sail_fixed_bytes_20 z3zE226;
  {
    sail_u256 z3zE3034;
    z3zE3034 = u256_of_fbits(UINT64_C(6));
    z3zE226 = evmsail_word_to_address(z3zE3034);
  }
  zPRECOMPILE_ADDRESS_6 = z3zE226;
let_end_583: ;
}
void kill_letbind_205(void) {
}

void create_letbind_206(void) {

  sail_fixed_bytes_20 z3zE227;
  {
    sail_u256 z3zE3035;
    z3zE3035 = u256_of_fbits(UINT64_C(7));
    z3zE227 = evmsail_word_to_address(z3zE3035);
  }
  zPRECOMPILE_ADDRESS_7 = z3zE227;
let_end_584: ;
}
void kill_letbind_206(void) {
}

void create_letbind_207(void) {

  sail_fixed_bytes_20 z3zE228;
  {
    sail_u256 z3zE3036;
    z3zE3036 = u256_of_fbits(UINT64_C(8));
    z3zE228 = evmsail_word_to_address(z3zE3036);
  }
  zPRECOMPILE_ADDRESS_8 = z3zE228;
let_end_585: ;
}
void kill_letbind_207(void) {
}

void create_letbind_208(void) {

  sail_fixed_bytes_20 z3zE229;
  {
    sail_u256 z3zE3037;
    z3zE3037 = u256_of_fbits(UINT64_C(9));
    z3zE229 = evmsail_word_to_address(z3zE3037);
  }
  zPRECOMPILE_ADDRESS_9 = z3zE229;
let_end_586: ;
}
void kill_letbind_208(void) {
}

void create_letbind_209(void) {

  sail_fixed_bytes_20 z3zE230;
  {
    sail_u256 z3zE3038;
    z3zE3038 = u256_of_fbits(UINT64_C(10));
    z3zE230 = evmsail_word_to_address(z3zE3038);
  }
  zPRECOMPILE_ADDRESS_10 = z3zE230;
let_end_587: ;
}
void kill_letbind_209(void) {
}

void create_letbind_210(void) {

  sail_fixed_bytes_20 z3zE231;
  {
    sail_u256 z3zE3039;
    z3zE3039 = u256_of_fbits(UINT64_C(11));
    z3zE231 = evmsail_word_to_address(z3zE3039);
  }
  zPRECOMPILE_ADDRESS_11 = z3zE231;
let_end_588: ;
}
void kill_letbind_210(void) {
}

void create_letbind_211(void) {

  sail_fixed_bytes_20 z3zE232;
  {
    sail_u256 z3zE3040;
    z3zE3040 = u256_of_fbits(UINT64_C(12));
    z3zE232 = evmsail_word_to_address(z3zE3040);
  }
  zPRECOMPILE_ADDRESS_12 = z3zE232;
let_end_589: ;
}
void kill_letbind_211(void) {
}

void create_letbind_212(void) {

  sail_fixed_bytes_20 z3zE233;
  {
    sail_u256 z3zE3041;
    z3zE3041 = u256_of_fbits(UINT64_C(13));
    z3zE233 = evmsail_word_to_address(z3zE3041);
  }
  zPRECOMPILE_ADDRESS_13 = z3zE233;
let_end_590: ;
}
void kill_letbind_212(void) {
}

void create_letbind_213(void) {

  sail_fixed_bytes_20 z3zE234;
  {
    sail_u256 z3zE3042;
    z3zE3042 = u256_of_fbits(UINT64_C(14));
    z3zE234 = evmsail_word_to_address(z3zE3042);
  }
  zPRECOMPILE_ADDRESS_14 = z3zE234;
let_end_591: ;
}
void kill_letbind_213(void) {
}

void create_letbind_214(void) {

  sail_fixed_bytes_20 z3zE235;
  {
    sail_u256 z3zE3043;
    z3zE3043 = u256_of_fbits(UINT64_C(15));
    z3zE235 = evmsail_word_to_address(z3zE3043);
  }
  zPRECOMPILE_ADDRESS_15 = z3zE235;
let_end_592: ;
}
void kill_letbind_214(void) {
}

void create_letbind_215(void) {

  sail_fixed_bytes_20 z3zE236;
  {
    sail_u256 z3zE3044;
    z3zE3044 = u256_of_fbits(UINT64_C(16));
    z3zE236 = evmsail_word_to_address(z3zE3044);
  }
  zPRECOMPILE_ADDRESS_16 = z3zE236;
let_end_593: ;
}
void kill_letbind_215(void) {
}

void create_letbind_216(void) {

  sail_fixed_bytes_20 z3zE237;
  {
    sail_u256 z3zE3045;
    z3zE3045 = u256_of_fbits(UINT64_C(17));
    z3zE237 = evmsail_word_to_address(z3zE3045);
  }
  zPRECOMPILE_ADDRESS_17 = z3zE237;
let_end_594: ;
}
void kill_letbind_216(void) {
}

void create_letbind_217(void) {

  sail_fixed_bytes_20 z3zE238;
  {
    sail_u256 z3zE3046;
    z3zE3046 = u256_of_fbits(UINT64_C(256));
    z3zE238 = evmsail_word_to_address(z3zE3046);
  }
  zPRECOMPILE_ADDRESS_256 = z3zE238;
let_end_595: ;
}
void kill_letbind_217(void) {
}

struct zPrecompileResult zprecompile_success(struct zByteSliceFields zoutput)
{
  struct zPrecompileResult z8zE354;
  struct zPrecompileResult z3zE2136;
  z3zE2136.zoutput = zoutput;
  z3zE2136.zsuccess = true;
  z8zE354 = z3zE2136;
end_function_2436: ;
  return z8zE354;
end_block_exception_2437: ;
  struct zByteSliceFields z8zE840 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE839 = { .zoutput = z8zE840, .zsuccess = false };
  return z8zE839;
}

struct zPrecompileResult zprecompile_failure(unit z3zE2134)
{
  struct zPrecompileResult z8zE355;
  struct zPrecompileResult z3zE2135;
  z3zE2135.zoutput = zEMPTY_SLICE;
  z3zE2135.zsuccess = false;
  z8zE355 = z3zE2135;
end_function_2434: ;
  return z8zE355;
end_block_exception_2435: ;
  struct zByteSliceFields z8zE842 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE841 = { .zoutput = z8zE842, .zsuccess = false };
  return z8zE841;
}

struct zPrecompileResult zaccelerator_result(bool zsuccess, uint64_t zoutput_len)
{
  struct zPrecompileResult z8zE356;
  if (zsuccess) {
    struct zByteSliceFields z2zE2148;
    z2zE2148 = zoutput_buffer_slice(zoutput_len);
    z8zE356 = zprecompile_success(z2zE2148);
  } else {  z8zE356 = zprecompile_failure(UNIT);  }
end_function_2432: ;
  return z8zE356;
end_block_exception_2433: ;
  struct zByteSliceFields z8zE844 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE843 = { .zoutput = z8zE844, .zsuccess = false };
  return z8zE843;
}

struct zPrecompileResult zcopied_result(struct zByteSliceFields zdata)
{
  struct zPrecompileResult z8zE357;
  struct zByteSliceFields zoutput;
  zoutput = zfreezze_output(zdata);
  bool z2zE2147;
  {
    uint64_t z2zE2145;
    z2zE2145 = zoutput.zlen;
    uint64_t z2zE2146;
    z2zE2146 = zdata.zlen;
    z2zE2147 = (z2zE2145 == z2zE2146);
  }
  if (z2zE2147) {  z8zE357 = zprecompile_success(zoutput);  } else {  z8zE357 = zprecompile_failure(UNIT);  }
end_function_2430: ;
  return z8zE357;
end_block_exception_2431: ;
  struct zByteSliceFields z8zE846 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE845 = { .zoutput = z8zE846, .zsuccess = false };
  return z8zE845;
}

struct zPrecompileResult zboolean_result(bool zvalue)
{
  struct zPrecompileResult z8zE358;
  struct zByteSliceFields z2zE2144;
  {
    sail_u256 z2zE2143;
    if (zvalue) {  z2zE2143 = zWORD_ONE;  } else {  z2zE2143 = zWORD_ZERO;  }
    z2zE2144 = zoutput_buffer_word(z2zE2143);
  }
  z8zE358 = zprecompile_success(z2zE2144);
end_function_2428: ;
  return z8zE358;
end_block_exception_2429: ;
  struct zByteSliceFields z8zE848 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE847 = { .zoutput = z8zE848, .zsuccess = false };
  return z8zE847;
}

bool zprecompile_active_at_fork(uint64_t zn)
{
  bool z8zE359;
  bool zbase;
  zbase = (!(UINT64_C(4) < zn));
  bool zbyzzantium;
  {
    bool z2zE2142;
    z2zE2142 = (!(zn < UINT64_C(5)));
    bool z3zE2123;
    if (z2zE2142) {
      bool z2zE2141;
      z2zE2141 = (!(UINT64_C(8) < zn));
      bool z3zE2122;
      if (z2zE2141) {  z3zE2122 = zfork_gteq(zk_fork, zByzzantium);  } else {  z3zE2122 = false;  }
      z3zE2123 = z3zE2122;
    } else {  z3zE2123 = false;  }
    zbyzzantium = z3zE2123;
  }
  bool zistanbul;
  {
    bool z2zE2140;
    z2zE2140 = (zn == UINT64_C(9));
    bool z3zE2124;
    if (z2zE2140) {  z3zE2124 = zfork_gteq(zk_fork, zIstanbul);  } else {  z3zE2124 = false;  }
    zistanbul = z3zE2124;
  }
  bool zkzzg;
  {
    bool z2zE2139;
    z2zE2139 = (zn == UINT64_C(10));
    bool z3zE2125;
    if (z2zE2139) {  z3zE2125 = zfork_gteq(zk_fork, zCancun);  } else {  z3zE2125 = false;  }
    zkzzg = z3zE2125;
  }
  bool zbls;
  {
    bool z2zE2138;
    z2zE2138 = (!(zn < UINT64_C(11)));
    bool z3zE2127;
    if (z2zE2138) {
      bool z2zE2137;
      z2zE2137 = (!(UINT64_C(17) < zn));
      bool z3zE2126;
      if (z2zE2137) {  z3zE2126 = zfork_gteq(zk_fork, zPrague);  } else {  z3zE2126 = false;  }
      z3zE2127 = z3zE2126;
    } else {  z3zE2127 = false;  }
    zbls = z3zE2127;
  }
  bool zp256;
  {
    bool z2zE2136;
    z2zE2136 = (zn == UINT64_C(256));
    bool z3zE2128;
    if (z2zE2136) {  z3zE2128 = zfork_gteq(zk_fork, zOsaka);  } else {  z3zE2128 = false;  }
    zp256 = z3zE2128;
  }
  bool z3zE2133;
  if (zbase) {  z3zE2133 = true;  } else {
    bool z3zE2132;
    if (zbyzzantium) {  z3zE2132 = true;  } else {
      bool z3zE2131;
      if (zistanbul) {  z3zE2131 = true;  } else {
        bool z3zE2130;
        if (zkzzg) {  z3zE2130 = true;  } else {
          bool z3zE2129;
          if (zbls) {  z3zE2129 = true;  } else {  z3zE2129 = zp256;  }
          z3zE2130 = z3zE2129;
        }
        z3zE2131 = z3zE2130;
      }
      z3zE2132 = z3zE2131;
    }
    z3zE2133 = z3zE2132;
  }
  z8zE359 = z3zE2133;
end_function_2426: ;
  return z8zE359;
end_block_exception_2427: ;

  return false;
}

uint64_t zprecompile_number(sail_fixed_bytes_20 zbytes)
{
  uint64_t z8zE360;
  uint64_t zselector;
  {
    uint64_t z2zE2134;
    z2zE2134 = fast_unsigned_vector_access_fixed_bytes_20(zbytes, UINT64_C(18));
    uint64_t z2zE2135;
    z2zE2135 = fast_unsigned_vector_access_fixed_bytes_20(zbytes, UINT64_C(19));
    zselector = (z2zE2134 << 8) | z2zE2135;
  }
  uint64_t zcandidate;
  {
    uint64_t z3zE2085;
    {
      uint64_t zp0z3;
      zp0z3 = zselector;
      bool z3zE2104;
      z3zE2104 = (zp0z3 == UINT64_C(0x0001));
      if (!(z3zE2104)) {  goto case_2423;  }
      bool z2zE2116;
      z2zE2116 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_1);
      if (z2zE2116) {  z3zE2085 = UINT64_C(1);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2423: ;
    {
      uint64_t z3zE2105;
      z3zE2105 = zselector;
      bool z3zE2103;
      z3zE2103 = (z3zE2105 == UINT64_C(0x0002));
      if (!(z3zE2103)) {  goto case_2422;  }
      bool z2zE2117;
      z2zE2117 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_2);
      if (z2zE2117) {  z3zE2085 = UINT64_C(2);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2422: ;
    {
      uint64_t z3zE2106;
      z3zE2106 = zselector;
      bool z3zE2102;
      z3zE2102 = (z3zE2106 == UINT64_C(0x0003));
      if (!(z3zE2102)) {  goto case_2421;  }
      bool z2zE2118;
      z2zE2118 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_3);
      if (z2zE2118) {  z3zE2085 = UINT64_C(3);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2421: ;
    {
      uint64_t z3zE2107;
      z3zE2107 = zselector;
      bool z3zE2101;
      z3zE2101 = (z3zE2107 == UINT64_C(0x0004));
      if (!(z3zE2101)) {  goto case_2420;  }
      bool z2zE2119;
      z2zE2119 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_4);
      if (z2zE2119) {  z3zE2085 = UINT64_C(4);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2420: ;
    {
      uint64_t z3zE2108;
      z3zE2108 = zselector;
      bool z3zE2100;
      z3zE2100 = (z3zE2108 == UINT64_C(0x0005));
      if (!(z3zE2100)) {  goto case_2419;  }
      bool z2zE2120;
      z2zE2120 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_5);
      if (z2zE2120) {  z3zE2085 = UINT64_C(5);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2419: ;
    {
      uint64_t z3zE2109;
      z3zE2109 = zselector;
      bool z3zE2099;
      z3zE2099 = (z3zE2109 == UINT64_C(0x0006));
      if (!(z3zE2099)) {  goto case_2418;  }
      bool z2zE2121;
      z2zE2121 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_6);
      if (z2zE2121) {  z3zE2085 = UINT64_C(6);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2418: ;
    {
      uint64_t z3zE2110;
      z3zE2110 = zselector;
      bool z3zE2098;
      z3zE2098 = (z3zE2110 == UINT64_C(0x0007));
      if (!(z3zE2098)) {  goto case_2417;  }
      bool z2zE2122;
      z2zE2122 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_7);
      if (z2zE2122) {  z3zE2085 = UINT64_C(7);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2417: ;
    {
      uint64_t z3zE2111;
      z3zE2111 = zselector;
      bool z3zE2097;
      z3zE2097 = (z3zE2111 == UINT64_C(0x0008));
      if (!(z3zE2097)) {  goto case_2416;  }
      bool z2zE2123;
      z2zE2123 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_8);
      if (z2zE2123) {  z3zE2085 = UINT64_C(8);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2416: ;
    {
      uint64_t z3zE2112;
      z3zE2112 = zselector;
      bool z3zE2096;
      z3zE2096 = (z3zE2112 == UINT64_C(0x0009));
      if (!(z3zE2096)) {  goto case_2415;  }
      bool z2zE2124;
      z2zE2124 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_9);
      if (z2zE2124) {  z3zE2085 = UINT64_C(9);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2415: ;
    {
      uint64_t z3zE2113;
      z3zE2113 = zselector;
      bool z3zE2095;
      z3zE2095 = (z3zE2113 == UINT64_C(0x000A));
      if (!(z3zE2095)) {  goto case_2414;  }
      bool z2zE2125;
      z2zE2125 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_10);
      if (z2zE2125) {  z3zE2085 = UINT64_C(10);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2414: ;
    {
      uint64_t z3zE2114;
      z3zE2114 = zselector;
      bool z3zE2094;
      z3zE2094 = (z3zE2114 == UINT64_C(0x000B));
      if (!(z3zE2094)) {  goto case_2413;  }
      bool z2zE2126;
      z2zE2126 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_11);
      if (z2zE2126) {  z3zE2085 = UINT64_C(11);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2413: ;
    {
      uint64_t z3zE2115;
      z3zE2115 = zselector;
      bool z3zE2093;
      z3zE2093 = (z3zE2115 == UINT64_C(0x000C));
      if (!(z3zE2093)) {  goto case_2412;  }
      bool z2zE2127;
      z2zE2127 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_12);
      if (z2zE2127) {  z3zE2085 = UINT64_C(12);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2412: ;
    {
      uint64_t z3zE2116;
      z3zE2116 = zselector;
      bool z3zE2092;
      z3zE2092 = (z3zE2116 == UINT64_C(0x000D));
      if (!(z3zE2092)) {  goto case_2411;  }
      bool z2zE2128;
      z2zE2128 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_13);
      if (z2zE2128) {  z3zE2085 = UINT64_C(13);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2411: ;
    {
      uint64_t z3zE2117;
      z3zE2117 = zselector;
      bool z3zE2091;
      z3zE2091 = (z3zE2117 == UINT64_C(0x000E));
      if (!(z3zE2091)) {  goto case_2410;  }
      bool z2zE2129;
      z2zE2129 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_14);
      if (z2zE2129) {  z3zE2085 = UINT64_C(14);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2410: ;
    {
      uint64_t z3zE2118;
      z3zE2118 = zselector;
      bool z3zE2090;
      z3zE2090 = (z3zE2118 == UINT64_C(0x000F));
      if (!(z3zE2090)) {  goto case_2409;  }
      bool z2zE2130;
      z2zE2130 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_15);
      if (z2zE2130) {  z3zE2085 = UINT64_C(15);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2409: ;
    {
      uint64_t z3zE2119;
      z3zE2119 = zselector;
      bool z3zE2089;
      z3zE2089 = (z3zE2119 == UINT64_C(0x0010));
      if (!(z3zE2089)) {  goto case_2408;  }
      bool z2zE2131;
      z2zE2131 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_16);
      if (z2zE2131) {  z3zE2085 = UINT64_C(16);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2408: ;
    {
      uint64_t z3zE2120;
      z3zE2120 = zselector;
      bool z3zE2088;
      z3zE2088 = (z3zE2120 == UINT64_C(0x0011));
      if (!(z3zE2088)) {  goto case_2407;  }
      bool z2zE2132;
      z2zE2132 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_17);
      if (z2zE2132) {  z3zE2085 = UINT64_C(17);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2407: ;
    {
      uint64_t z3zE2121;
      z3zE2121 = zselector;
      bool z3zE2087;
      z3zE2087 = (z3zE2121 == UINT64_C(0x0100));
      if (!(z3zE2087)) {  goto case_2406;  }
      bool z2zE2133;
      z2zE2133 = eq_fixed_bytes_20(zbytes, zPRECOMPILE_ADDRESS_256);
      if (z2zE2133) {  z3zE2085 = UINT64_C(256);  } else {  z3zE2085 = UINT64_C(0);  }
      goto finish_match_2404;
    }
  case_2406: ;
    {
      z3zE2085 = UINT64_C(0);
      goto finish_match_2404;
    }
  case_2405: ;
  finish_match_2404: ;
    zcandidate = z3zE2085;
  }
  bool z2zE2114;
  z2zE2114 = (zcandidate != UINT64_C(0));
  if (z2zE2114) {
    uint64_t zid;
    zid = zcandidate;
    bool z2zE2115;
    z2zE2115 = zprecompile_active_at_fork(zid);
    if (z2zE2115) {  z8zE360 = zid;  } else {  z8zE360 = UINT64_C(0);  }
  } else {  z8zE360 = UINT64_C(0);  }
end_function_2424: ;
  return z8zE360;
end_block_exception_2425: ;

  return UINT64_C(0xdeadc0de);
}

struct zPrecompileResult zrun_ecrecover(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE361;
  sail_u256 zv;
  zv = zslice_load(zinput, zPRECOMPILE_WORD_OFFSET);
  bool zvalid_v;
  {
    bool z2zE2113;
    z2zE2113 = u256_eq_u64(zv, UINT64_C(27));
    bool z3zE2082;
    if (z2zE2113) {  z3zE2082 = true;  } else {  z3zE2082 = u256_eq_u64(zv, UINT64_C(28));  }
    zvalid_v = z3zE2082;
  }
  if (zvalid_v) {
    uint64_t zparity;
    {
      bool z2zE2112;
      z2zE2112 = u256_eq_u64(zv, UINT64_C(27));
      if (z2zE2112) {  zparity = UINT64_C(0);  } else {  zparity = UINT64_C(1);  }
    }
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2109;
    {
      sail_fixed_bytes_32 z2zE2106;
      {
        sail_u256 z2zE2105;
        z2zE2105 = zslice_load(zinput, UINT64_C(0));
        z2zE2106 = evmsail_word_to_hash(z2zE2105);
      }
      sail_u256 z2zE2107;
      z2zE2107 = zslice_load(zinput, zPRECOMPILE_DOUBLE_WORD_OFFSET);
      sail_u256 z2zE2108;
      z2zE2108 = zslice_load(zinput, zECRECOVER_S_OFFSET);
      z2zE2109 = zecrecover_addr(z2zE2106, zparity, z2zE2107, z2zE2108);
    }
    struct zPrecompileResult z3zE2083;
    {
      bool zrecovered;
      zrecovered = z2zE2109.ztup0;
      sail_fixed_bytes_20 zaddress;
      zaddress = z2zE2109.ztup1;
      if (zrecovered) {
        struct zByteSliceFields z2zE2111;
        {
          sail_u256 z2zE2110;
          z2zE2110 = evmsail_address_to_word(zaddress);
          z2zE2111 = zoutput_buffer_word(z2zE2110);
        }
        z3zE2083 = zprecompile_success(z2zE2111);
      } else {  z3zE2083 = zprecompile_success(zEMPTY_SLICE);  }
      goto finish_match_2400;
    }
  case_2401: ;
    sail_match_failure("run_ecrecover");
  finish_match_2400: ;
    z8zE361 = z3zE2083;
  } else {  z8zE361 = zprecompile_success(zEMPTY_SLICE);  }
end_function_2402: ;
  return z8zE361;
end_block_exception_2403: ;
  struct zByteSliceFields z8zE850 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE849 = { .zoutput = z8zE850, .zsuccess = false };
  return z8zE849;
}

struct zPrecompileResult zrun_sha256(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE362;
  struct zByteSliceFields z2zE2104;
  {
    sail_u256 z2zE2103;
    {
      sail_fixed_bytes_32 z2zE2102;
      z2zE2102 = zsha256_slice(zinput);
      z2zE2103 = evmsail_hash_to_word(z2zE2102);
    }
    z2zE2104 = zoutput_buffer_word(z2zE2103);
  }
  z8zE362 = zprecompile_success(z2zE2104);
end_function_2398: ;
  return z8zE362;
end_block_exception_2399: ;
  struct zByteSliceFields z8zE852 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE851 = { .zoutput = z8zE852, .zsuccess = false };
  return z8zE851;
}

struct zPrecompileResult zrun_ripemd160(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE363;
  bool z2zE2101;
  z2zE2101 = accelerator_ripemd160(zinput);
  z8zE363 = zaccelerator_result(z2zE2101, zPRECOMPILE_WORD_LENGTH);
end_function_2396: ;
  return z8zE363;
end_block_exception_2397: ;
  struct zByteSliceFields z8zE854 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE853 = { .zoutput = z8zE854, .zsuccess = false };
  return z8zE853;
}

struct zPrecompileResult zrun_modexp(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE364;
  sail_u256 zbase_len;
  zbase_len = zpc_word(zinput, UINT64_C(0), UINT64_C(32));
  sail_u256 zexponent_len;
  zexponent_len = zpc_word(zinput, UINT64_C(32), UINT64_C(32));
  sail_u256 zmodulus_len;
  zmodulus_len = zpc_word(zinput, UINT64_C(64), UINT64_C(32));
  bool z2zE2093;
  z2zE2093 = u256_eq_u64(zmodulus_len, UINT64_C(0));
  if (z2zE2093) {  z8zE364 = zprecompile_success(zEMPTY_SLICE);  } else {
    bool z2zE2096;
    {
      bool z2zE2095;
      z2zE2095 = u64_lt_u256(zACCELERATOR_INPUT_MAX, zbase_len);
      bool z3zE2081;
      if (z2zE2095) {  z3zE2081 = true;  } else {
        bool z2zE2094;
        z2zE2094 = u64_lt_u256(zACCELERATOR_INPUT_MAX, zexponent_len);
        bool z3zE2080;
        if (z2zE2094) {  z3zE2080 = true;  } else {  z3zE2080 = u64_lt_u256(zACCELERATOR_INPUT_MAX, zmodulus_len);  }
        z3zE2081 = z3zE2080;
      }
      z2zE2096 = z3zE2081;
    }
    if (z2zE2096) {  z8zE364 = zprecompile_failure(UNIT);  } else {
      uint64_t zbounded_base;
      zbounded_base = u256_to_u64(zbase_len);
      uint64_t zbounded_exponent;
      zbounded_exponent = u256_to_u64(zexponent_len);
      uint64_t zbounded_modulus;
      zbounded_modulus = u256_to_u64(zmodulus_len);
      uint64_t zinput_end;
      {
        uint64_t z2zE2100;
        {
          uint64_t z2zE2099;
          {    z2zE2099 = (UINT64_C(96) + zbounded_base);
          }
          {    z2zE2100 = (z2zE2099 + zbounded_exponent);
          }
        }
        {    zinput_end = (z2zE2100 + zbounded_modulus);
        }
      }
      bool z2zE2097;
      z2zE2097 = (zACCELERATOR_INPUT_MAX < zinput_end);
      if (z2zE2097) {  z8zE364 = zprecompile_failure(UNIT);  } else {
        bool z2zE2098;
        z2zE2098 = accelerator_modexp(zinput, zbounded_base, zbounded_exponent, zbounded_modulus);
        z8zE364 = zaccelerator_result(z2zE2098, zbounded_modulus);
      }
    }
  }
end_function_2394: ;
  return z8zE364;
end_block_exception_2395: ;
  struct zByteSliceFields z8zE856 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE855 = { .zoutput = z8zE856, .zsuccess = false };
  return z8zE855;
}

struct zPrecompileResult zpairing_result(uint64_t zresult)
{
  struct zPrecompileResult z8zE365;
  bool z2zE2090;
  {
    uint64_t z2zE2089;
    z2zE2089 = (UINT64_C(1) & (zresult >> UINT64_C(1)));
    z2zE2090 = (z2zE2089 == UINT64_C(0b0));
  }
  if (z2zE2090) {  z8zE365 = zprecompile_failure(UNIT);  } else {
    bool z2zE2092;
    {
      uint64_t z2zE2091;
      z2zE2091 = (UINT64_C(1) & (zresult >> UINT64_C(0)));
      z2zE2092 = (z2zE2091 == UINT64_C(0b1));
    }
    z8zE365 = zboolean_result(z2zE2092);
  }
end_function_2392: ;
  return z8zE365;
end_block_exception_2393: ;
  struct zByteSliceFields z8zE858 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE857 = { .zoutput = z8zE858, .zsuccess = false };
  return z8zE857;
}

struct zPrecompileResult zrun_blake2f(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE366;
  uint64_t zfinal_byte;
  zfinal_byte = zslice_byte(zinput, zBLAKE2F_FINAL_BLOCK_OFFSET);
  bool z2zE2085;
  {
    bool z2zE2084;
    {
      uint64_t z2zE2082;
      z2zE2082 = zinput.zlen;
      z2zE2084 = (z2zE2082 != zBLAKE2F_INPUT_LENGTH);
    }
    bool z3zE2079;
    if (z2zE2084) {  z3zE2079 = true;  } else {
      bool z2zE2083;
      z2zE2083 = (zfinal_byte != UINT64_C(0x00));
      bool z3zE2078;
      if (z2zE2083) {  z3zE2078 = (zfinal_byte != UINT64_C(0x01));  } else {  z3zE2078 = false;  }
      z3zE2079 = z3zE2078;
    }
    z2zE2085 = z3zE2079;
  }
  if (z2zE2085) {  z8zE366 = zprecompile_failure(UNIT);  } else {
    uint64_t zfinal_block;
    {
      bool z2zE2088;
      z2zE2088 = (zfinal_byte == UINT64_C(0x00));
      if (z2zE2088) {  zfinal_block = UINT64_C(0);  } else {  zfinal_block = UINT64_C(1);  }
    }
    bool z2zE2087;
    {
      uint64_t z2zE2086;
      z2zE2086 = zpc_blake2_rounds(zinput);
      z2zE2087 = accelerator_blake2f(zinput, z2zE2086, zfinal_block);
    }
    z8zE366 = zaccelerator_result(z2zE2087, zBLAKE2F_OUTPUT_LENGTH);
  }
end_function_2390: ;
  return z8zE366;
end_block_exception_2391: ;
  struct zByteSliceFields z8zE860 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE859 = { .zoutput = z8zE860, .zsuccess = false };
  return z8zE859;
}

bool zkzzg_versioned_hash_matches(struct zByteSliceFields zinput)
{
  bool z8zE367;
  sail_fixed_bytes_32 zcommitment_hash;
  {
    struct zByteSliceFields z2zE2081;
    z2zE2081 = zsub_slice(zinput, zKZG_COMMITMENT_OFFSET, zKZG_COMMITMENT_LENGTH);
    zcommitment_hash = zsha256_slice(z2zE2081);
  }
  sail_fixed_bytes_32 zexpected;
  zexpected = zcommitment_hash;
  zexpected = fast_unsigned_vector_update_fixed_bytes_32(zexpected, UINT64_C(0), UINT64_C(0x01));
  unit z3zE2077;
  z3zE2077 = UNIT;
  sail_fixed_bytes_32 z2zE2080;
  {
    sail_u256 z2zE2079;
    z2zE2079 = zslice_load(zinput, UINT64_C(0));
    z2zE2080 = evmsail_word_to_hash(z2zE2079);
  }
  z8zE367 = eq_fixed_bytes_32(z2zE2080, zexpected);
end_function_2388: ;
  return z8zE367;
end_block_exception_2389: ;

  return false;
}

struct zPrecompileResult zrun_kzzg_point_evaluation(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE368;
  bool z2zE2074;
  {
    uint64_t z2zE2073;
    z2zE2073 = zinput.zlen;
    z2zE2074 = (z2zE2073 != zKZG_INPUT_LENGTH);
  }
  if (z2zE2074) {  z8zE368 = zprecompile_failure(UNIT);  } else {
    bool z2zE2076;
    {
      bool z2zE2075;
      z2zE2075 = zkzzg_versioned_hash_matches(zinput);
      z2zE2076 = not(z2zE2075);
    }
    if (z2zE2076) {  z8zE368 = zprecompile_failure(UNIT);  } else {
      bool z2zE2077;
      z2zE2077 = accelerator_kzg_point_evaluation(zinput);
      if (z2zE2077) {
        struct zByteSliceFields z2zE2078;
        z2zE2078 = zoutput_buffer_words(zFIELD_ELEMENTS_PER_BLOB, zBLS_MODULUS);
        z8zE368 = zprecompile_success(z2zE2078);
      } else {  z8zE368 = zprecompile_failure(UNIT);  }
    }
  }
end_function_2386: ;
  return z8zE368;
end_block_exception_2387: ;
  struct zByteSliceFields z8zE862 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE861 = { .zoutput = z8zE862, .zsuccess = false };
  return z8zE861;
}

bool zbls_g1_padding(struct zByteSliceFields zinput, uint64_t zbase, uint64_t zstride, uint64_t zcount)
{
  bool z8zE369;
  bool z2zE2072;
  z2zE2072 = zslice_strided_zzero(zinput, zbase, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
  bool z3zE2076;
  if (z2zE2072) {
    uint64_t z2zE2071;
    {    z2zE2071 = (zbase + zBLS_PADDED_FIELD_LENGTH);
    }
    z3zE2076 = zslice_strided_zzero(zinput, z2zE2071, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
  } else {  z3zE2076 = false;  }
  z8zE369 = z3zE2076;
end_function_2384: ;
  return z8zE369;
end_block_exception_2385: ;

  return false;
}

bool zbls_g2_padding(struct zByteSliceFields zinput, uint64_t zbase, uint64_t zstride, uint64_t zcount)
{
  bool z8zE370;
  bool z2zE2070;
  z2zE2070 = zslice_strided_zzero(zinput, zbase, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
  bool z3zE2075;
  if (z2zE2070) {
    bool z2zE2069;
    {
      uint64_t z2zE2065;
      {    z2zE2065 = (zbase + zBLS_PADDED_FIELD_LENGTH);
      }
      z2zE2069 = zslice_strided_zzero(zinput, z2zE2065, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
    }
    bool z3zE2074;
    if (z2zE2069) {
      bool z2zE2068;
      {
        uint64_t z2zE2066;
        {    z2zE2066 = (zbase + zBLS_G1_POINT_LENGTH);
        }
        z2zE2068 = zslice_strided_zzero(zinput, z2zE2066, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
      }
      bool z3zE2073;
      if (z2zE2068) {
        uint64_t z2zE2067;
        {    z2zE2067 = (zbase + zBLS_G2_FINAL_FIELD_OFFSET);
        }
        z3zE2073 = zslice_strided_zzero(zinput, z2zE2067, zstride, zBLS_FIELD_PADDING_LENGTH, zcount);
      } else {  z3zE2073 = false;  }
      z3zE2074 = z3zE2073;
    } else {  z3zE2074 = false;  }
    z3zE2075 = z3zE2074;
  } else {  z3zE2075 = false;  }
  z8zE370 = z3zE2075;
end_function_2382: ;
  return z8zE370;
end_block_exception_2383: ;

  return false;
}

struct zPrecompileResult zrun_bls_g1_add(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE371;
  bool z2zE2061;
  {
    uint64_t z2zE2060;
    z2zE2060 = zinput.zlen;
    z2zE2061 = (z2zE2060 != zBLS_G1_ADD_INPUT_LENGTH);
  }
  if (z2zE2061) {  z8zE371 = zprecompile_failure(UNIT);  } else {
    bool z2zE2063;
    {
      bool z2zE2062;
      z2zE2062 = zbls_g1_padding(zinput, UINT64_C(0), zBLS_G1_POINT_LENGTH, zTWO_COMPONENTS);
      z2zE2063 = not(z2zE2062);
    }
    if (z2zE2063) {  z8zE371 = zprecompile_failure(UNIT);  } else {
      bool z2zE2064;
      z2zE2064 = accelerator_bls_g1_add(zinput);
      z8zE371 = zaccelerator_result(z2zE2064, zBLS_G1_POINT_LENGTH);
    }
  }
end_function_2380: ;
  return z8zE371;
end_block_exception_2381: ;
  struct zByteSliceFields z8zE864 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE863 = { .zoutput = z8zE864, .zsuccess = false };
  return z8zE863;
}

struct zPrecompileResult zrun_bls_g1_msm(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE372;
  uint64_t zlength;
  zlength = zinput.zlen;
  uint64_t zitem_length;
  zitem_length = zBLS_G1_MSM_ITEM_LENGTH;
  uint64_t zpairs;
  zpairs = (zlength / zitem_length);
  bool z2zE2056;
  {
    bool z2zE2055;
    z2zE2055 = (zlength == UINT64_C(0));
    bool z3zE2072;
    if (z2zE2055) {  z3zE2072 = true;  } else {
      uint64_t z2zE2054;
      {    z2zE2054 = (zpairs * zitem_length);
      }
      z3zE2072 = (zlength != z2zE2054);
    }
    z2zE2056 = z3zE2072;
  }
  if (z2zE2056) {  z8zE372 = zprecompile_failure(UNIT);  } else {
    bool z2zE2058;
    {
      bool z2zE2057;
      z2zE2057 = zbls_g1_padding(zinput, UINT64_C(0), zBLS_G1_MSM_ITEM_LENGTH, zpairs);
      z2zE2058 = not(z2zE2057);
    }
    if (z2zE2058) {  z8zE372 = zprecompile_failure(UNIT);  } else {
      bool z2zE2059;
      z2zE2059 = accelerator_bls_g1_msm(zinput);
      z8zE372 = zaccelerator_result(z2zE2059, zBLS_G1_POINT_LENGTH);
    }
  }
end_function_2378: ;
  return z8zE372;
end_block_exception_2379: ;
  struct zByteSliceFields z8zE866 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE865 = { .zoutput = z8zE866, .zsuccess = false };
  return z8zE865;
}

struct zPrecompileResult zrun_bls_g2_add(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE373;
  bool z2zE2050;
  {
    uint64_t z2zE2049;
    z2zE2049 = zinput.zlen;
    z2zE2050 = (z2zE2049 != zBLS_G2_ADD_INPUT_LENGTH);
  }
  if (z2zE2050) {  z8zE373 = zprecompile_failure(UNIT);  } else {
    bool z2zE2052;
    {
      bool z2zE2051;
      z2zE2051 = zbls_g2_padding(zinput, UINT64_C(0), zBLS_G2_POINT_LENGTH, zTWO_COMPONENTS);
      z2zE2052 = not(z2zE2051);
    }
    if (z2zE2052) {  z8zE373 = zprecompile_failure(UNIT);  } else {
      bool z2zE2053;
      z2zE2053 = accelerator_bls_g2_add(zinput);
      z8zE373 = zaccelerator_result(z2zE2053, zBLS_G2_POINT_LENGTH);
    }
  }
end_function_2376: ;
  return z8zE373;
end_block_exception_2377: ;
  struct zByteSliceFields z8zE868 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE867 = { .zoutput = z8zE868, .zsuccess = false };
  return z8zE867;
}

struct zPrecompileResult zrun_bls_g2_msm(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE374;
  uint64_t zlength;
  zlength = zinput.zlen;
  uint64_t zitem_length;
  zitem_length = zBLS_G2_MSM_ITEM_LENGTH;
  uint64_t zpairs;
  zpairs = (zlength / zitem_length);
  bool z2zE2045;
  {
    bool z2zE2044;
    z2zE2044 = (zlength == UINT64_C(0));
    bool z3zE2071;
    if (z2zE2044) {  z3zE2071 = true;  } else {
      uint64_t z2zE2043;
      {    z2zE2043 = (zpairs * zitem_length);
      }
      z3zE2071 = (zlength != z2zE2043);
    }
    z2zE2045 = z3zE2071;
  }
  if (z2zE2045) {  z8zE374 = zprecompile_failure(UNIT);  } else {
    bool z2zE2047;
    {
      bool z2zE2046;
      z2zE2046 = zbls_g2_padding(zinput, UINT64_C(0), zBLS_G2_MSM_ITEM_LENGTH, zpairs);
      z2zE2047 = not(z2zE2046);
    }
    if (z2zE2047) {  z8zE374 = zprecompile_failure(UNIT);  } else {
      bool z2zE2048;
      z2zE2048 = accelerator_bls_g2_msm(zinput);
      z8zE374 = zaccelerator_result(z2zE2048, zBLS_G2_POINT_LENGTH);
    }
  }
end_function_2374: ;
  return z8zE374;
end_block_exception_2375: ;
  struct zByteSliceFields z8zE870 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE869 = { .zoutput = z8zE870, .zsuccess = false };
  return z8zE869;
}

struct zPrecompileResult zrun_bls_pairing(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE375;
  uint64_t zlength;
  zlength = zinput.zlen;
  uint64_t zitem_length;
  zitem_length = zBLS_PAIRING_ITEM_LENGTH;
  uint64_t zpairs;
  zpairs = (zlength / zitem_length);
  bool z2zE2039;
  {
    bool z2zE2038;
    z2zE2038 = (zlength == UINT64_C(0));
    bool z3zE2069;
    if (z2zE2038) {  z3zE2069 = true;  } else {
      uint64_t z2zE2037;
      {    z2zE2037 = (zpairs * zitem_length);
      }
      z3zE2069 = (zlength != z2zE2037);
    }
    z2zE2039 = z3zE2069;
  }
  if (z2zE2039) {  z8zE375 = zprecompile_failure(UNIT);  } else {
    uint64_t zpair_count;
    zpair_count = zpairs;
    bool z2zE2041;
    {
      bool z2zE2040;
      z2zE2040 = zbls_g1_padding(zinput, UINT64_C(0), zBLS_PAIRING_ITEM_LENGTH, zpair_count);
      bool z3zE2070;
      if (z2zE2040) {
        z3zE2070 = zbls_g2_padding(zinput, zBLS_G2_POINT_OFFSET, zBLS_PAIRING_ITEM_LENGTH, zpair_count);
      } else {  z3zE2070 = false;  }
      z2zE2041 = z3zE2070;
    }
    if (z2zE2041) {
      uint64_t z2zE2042;
      z2zE2042 = accelerator_bls_pairing(zinput);
      z8zE375 = zpairing_result(z2zE2042);
    } else {  z8zE375 = zprecompile_failure(UNIT);  }
  }
end_function_2372: ;
  return z8zE375;
end_block_exception_2373: ;
  struct zByteSliceFields z8zE872 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE871 = { .zoutput = z8zE872, .zsuccess = false };
  return z8zE871;
}

struct zPrecompileResult zrun_bls_map_fp_to_g1(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE376;
  bool z2zE2033;
  {
    uint64_t z2zE2032;
    z2zE2032 = zinput.zlen;
    z2zE2033 = (z2zE2032 != zBLS_PADDED_FIELD_LENGTH);
  }
  if (z2zE2033) {  z8zE376 = zprecompile_failure(UNIT);  } else {
    bool z2zE2035;
    {
      bool z2zE2034;
      z2zE2034 = zslice_strided_zzero(zinput, UINT64_C(0), zBLS_PADDED_FIELD_LENGTH, zBLS_FIELD_PADDING_LENGTH, UINT64_C(1));
      z2zE2035 = not(z2zE2034);
    }
    if (z2zE2035) {  z8zE376 = zprecompile_failure(UNIT);  } else {
      bool z2zE2036;
      z2zE2036 = accelerator_bls_map_fp_to_g1(zinput);
      z8zE376 = zaccelerator_result(z2zE2036, zBLS_G1_POINT_LENGTH);
    }
  }
end_function_2370: ;
  return z8zE376;
end_block_exception_2371: ;
  struct zByteSliceFields z8zE874 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE873 = { .zoutput = z8zE874, .zsuccess = false };
  return z8zE873;
}

struct zPrecompileResult zrun_bls_map_fp2_to_g2(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE377;
  bool z2zE2028;
  {
    uint64_t z2zE2027;
    z2zE2027 = zinput.zlen;
    z2zE2028 = (z2zE2027 != zBLS_G1_POINT_LENGTH);
  }
  if (z2zE2028) {  z8zE377 = zprecompile_failure(UNIT);  } else {
    bool z2zE2030;
    {
      bool z2zE2029;
      z2zE2029 = zslice_strided_zzero(zinput, UINT64_C(0), zBLS_PADDED_FIELD_LENGTH, zBLS_FIELD_PADDING_LENGTH, zTWO_COMPONENTS);
      z2zE2030 = not(z2zE2029);
    }
    if (z2zE2030) {  z8zE377 = zprecompile_failure(UNIT);  } else {
      bool z2zE2031;
      z2zE2031 = accelerator_bls_map_fp2_to_g2(zinput);
      z8zE377 = zaccelerator_result(z2zE2031, zBLS_G2_POINT_LENGTH);
    }
  }
end_function_2368: ;
  return z8zE377;
end_block_exception_2369: ;
  struct zByteSliceFields z8zE876 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE875 = { .zoutput = z8zE876, .zsuccess = false };
  return z8zE875;
}

struct zPrecompileResult zrun_p256_verify(struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE378;
  bool zverified;
  {
    bool z2zE2026;
    {
      uint64_t z2zE2025;
      z2zE2025 = zinput.zlen;
      z2zE2026 = (z2zE2025 == zP256_INPUT_LENGTH);
    }
    if (z2zE2026) {  zverified = accelerator_p256_verify(zinput);  } else {  zverified = false;  }
  }
  if (zverified) {
    struct zByteSliceFields z2zE2024;
    z2zE2024 = zoutput_buffer_word(zWORD_ONE);
    z8zE378 = zprecompile_success(z2zE2024);
  } else {  z8zE378 = zprecompile_success(zEMPTY_SLICE);  }
end_function_2366: ;
  return z8zE378;
end_block_exception_2367: ;
  struct zByteSliceFields z8zE878 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE877 = { .zoutput = z8zE878, .zsuccess = false };
  return z8zE877;
}

struct zPrecompileResult zrun_precompile_slice(uint64_t znum, struct zByteSliceFields zinput)
{
  struct zPrecompileResult z8zE379;
  struct zPrecompileResult z3zE2032;
  {
    uint64_t zp0z3;
    zp0z3 = znum;
    bool z3zE2051;
    z3zE2051 = (zp0z3 == UINT64_C(1));
    if (!(z3zE2051)) {  goto case_2363;  }
    z3zE2032 = zrun_ecrecover(zinput);
    goto finish_match_2344;
  }
case_2363: ;
  {
    uint64_t z3zE2052;
    z3zE2052 = znum;
    bool z3zE2050;
    z3zE2050 = (z3zE2052 == UINT64_C(2));
    if (!(z3zE2050)) {  goto case_2362;  }
    z3zE2032 = zrun_sha256(zinput);
    goto finish_match_2344;
  }
case_2362: ;
  {
    uint64_t z3zE2053;
    z3zE2053 = znum;
    bool z3zE2049;
    z3zE2049 = (z3zE2053 == UINT64_C(3));
    if (!(z3zE2049)) {  goto case_2361;  }
    z3zE2032 = zrun_ripemd160(zinput);
    goto finish_match_2344;
  }
case_2361: ;
  {
    uint64_t z3zE2054;
    z3zE2054 = znum;
    bool z3zE2048;
    z3zE2048 = (z3zE2054 == UINT64_C(4));
    if (!(z3zE2048)) {  goto case_2360;  }
    z3zE2032 = zcopied_result(zinput);
    goto finish_match_2344;
  }
case_2360: ;
  {
    uint64_t z3zE2055;
    z3zE2055 = znum;
    bool z3zE2047;
    z3zE2047 = (z3zE2055 == UINT64_C(5));
    if (!(z3zE2047)) {  goto case_2359;  }
    z3zE2032 = zrun_modexp(zinput);
    goto finish_match_2344;
  }
case_2359: ;
  {
    uint64_t z3zE2056;
    z3zE2056 = znum;
    bool z3zE2046;
    z3zE2046 = (z3zE2056 == UINT64_C(6));
    if (!(z3zE2046)) {  goto case_2358;  }
    bool z2zE2018;
    z2zE2018 = accelerator_bn254_add(zinput);
    z3zE2032 = zaccelerator_result(z2zE2018, zPRECOMPILE_DOUBLE_WORD_LENGTH);
    goto finish_match_2344;
  }
case_2358: ;
  {
    uint64_t z3zE2057;
    z3zE2057 = znum;
    bool z3zE2045;
    z3zE2045 = (z3zE2057 == UINT64_C(7));
    if (!(z3zE2045)) {  goto case_2357;  }
    bool z2zE2019;
    z2zE2019 = accelerator_bn254_mul(zinput);
    z3zE2032 = zaccelerator_result(z2zE2019, zPRECOMPILE_DOUBLE_WORD_LENGTH);
    goto finish_match_2344;
  }
case_2357: ;
  {
    uint64_t z3zE2058;
    z3zE2058 = znum;
    bool z3zE2044;
    z3zE2044 = (z3zE2058 == UINT64_C(8));
    if (!(z3zE2044)) {  goto case_2356;  }
    uint64_t zinput_length;
    zinput_length = zinput.zlen;
    uint64_t zitem_length;
    zitem_length = zBN254_PAIRING_ITEM_LENGTH;
    bool z2zE2022;
    {
      uint64_t z2zE2021;
      {
        uint64_t z2zE2020;
        z2zE2020 = (zinput_length / zitem_length);
        {    z2zE2021 = (z2zE2020 * zitem_length);
        }
      }
      z2zE2022 = (zinput_length == z2zE2021);
    }
    if (z2zE2022) {
      uint64_t z2zE2023;
      z2zE2023 = accelerator_bn254_pairing(zinput);
      z3zE2032 = zpairing_result(z2zE2023);
    } else {  z3zE2032 = zprecompile_failure(UNIT);  }
    goto finish_match_2344;
  }
case_2356: ;
  {
    uint64_t z3zE2059;
    z3zE2059 = znum;
    bool z3zE2043;
    z3zE2043 = (z3zE2059 == UINT64_C(9));
    if (!(z3zE2043)) {  goto case_2355;  }
    z3zE2032 = zrun_blake2f(zinput);
    goto finish_match_2344;
  }
case_2355: ;
  {
    uint64_t z3zE2060;
    z3zE2060 = znum;
    bool z3zE2042;
    z3zE2042 = (z3zE2060 == UINT64_C(10));
    if (!(z3zE2042)) {  goto case_2354;  }
    z3zE2032 = zrun_kzzg_point_evaluation(zinput);
    goto finish_match_2344;
  }
case_2354: ;
  {
    uint64_t z3zE2061;
    z3zE2061 = znum;
    bool z3zE2041;
    z3zE2041 = (z3zE2061 == UINT64_C(11));
    if (!(z3zE2041)) {  goto case_2353;  }
    z3zE2032 = zrun_bls_g1_add(zinput);
    goto finish_match_2344;
  }
case_2353: ;
  {
    uint64_t z3zE2062;
    z3zE2062 = znum;
    bool z3zE2040;
    z3zE2040 = (z3zE2062 == UINT64_C(12));
    if (!(z3zE2040)) {  goto case_2352;  }
    z3zE2032 = zrun_bls_g1_msm(zinput);
    goto finish_match_2344;
  }
case_2352: ;
  {
    uint64_t z3zE2063;
    z3zE2063 = znum;
    bool z3zE2039;
    z3zE2039 = (z3zE2063 == UINT64_C(13));
    if (!(z3zE2039)) {  goto case_2351;  }
    z3zE2032 = zrun_bls_g2_add(zinput);
    goto finish_match_2344;
  }
case_2351: ;
  {
    uint64_t z3zE2064;
    z3zE2064 = znum;
    bool z3zE2038;
    z3zE2038 = (z3zE2064 == UINT64_C(14));
    if (!(z3zE2038)) {  goto case_2350;  }
    z3zE2032 = zrun_bls_g2_msm(zinput);
    goto finish_match_2344;
  }
case_2350: ;
  {
    uint64_t z3zE2065;
    z3zE2065 = znum;
    bool z3zE2037;
    z3zE2037 = (z3zE2065 == UINT64_C(15));
    if (!(z3zE2037)) {  goto case_2349;  }
    z3zE2032 = zrun_bls_pairing(zinput);
    goto finish_match_2344;
  }
case_2349: ;
  {
    uint64_t z3zE2066;
    z3zE2066 = znum;
    bool z3zE2036;
    z3zE2036 = (z3zE2066 == UINT64_C(16));
    if (!(z3zE2036)) {  goto case_2348;  }
    z3zE2032 = zrun_bls_map_fp_to_g1(zinput);
    goto finish_match_2344;
  }
case_2348: ;
  {
    uint64_t z3zE2067;
    z3zE2067 = znum;
    bool z3zE2035;
    z3zE2035 = (z3zE2067 == UINT64_C(17));
    if (!(z3zE2035)) {  goto case_2347;  }
    z3zE2032 = zrun_bls_map_fp2_to_g2(zinput);
    goto finish_match_2344;
  }
case_2347: ;
  {
    uint64_t z3zE2068;
    z3zE2068 = znum;
    bool z3zE2034;
    z3zE2034 = (z3zE2068 == UINT64_C(256));
    if (!(z3zE2034)) {  goto case_2346;  }
    z3zE2032 = zrun_p256_verify(zinput);
    goto finish_match_2344;
  }
case_2346: ;
  {
    z3zE2032 = zprecompile_failure(UNIT);
    goto finish_match_2344;
  }
case_2345: ;
finish_match_2344: ;
  z8zE379 = z3zE2032;
end_function_2364: ;
  return z8zE379;
end_block_exception_2365: ;
  struct zByteSliceFields z8zE880 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zPrecompileResult z8zE879 = { .zoutput = z8zE880, .zsuccess = false };
  return z8zE879;
}

