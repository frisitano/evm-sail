/* Generated from sail/primitives/bytes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_70(void) {

  uint64_t z3zE73;
  z3zE73 = UINT64_C(20);
  zADDRESS_BYTE_LENGTH = z3zE73;
let_end_160: ;
}
void kill_letbind_70(void) {
}

void create_letbind_71(void) {

  uint64_t z3zE74;
  z3zE74 = UINT64_C(32);
  zWORD_BYTE_LENGTH = z3zE74;
let_end_161: ;
}
void kill_letbind_71(void) {
}

void create_letbind_72(void) {

  uint64_t z3zE75;
  z3zE75 = UINT64_C(8);
  zEIGHT_BYTE_LENGTH = z3zE75;
let_end_162: ;
}
void kill_letbind_72(void) {
}

void create_letbind_73(void) {

  uint64_t z3zE76;
  z3zE76 = UINT64_C(64);
  zDOUBLE_WORD_BYTE_LENGTH = z3zE76;
let_end_163: ;
}
void kill_letbind_73(void) {
}

struct zByteSliceFields zbyte_slice(enum zByteSource zsrc, uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE87;
  struct zByteSliceFields zfields;
  {
    struct zByteSliceFields z3zE2903;
    z3zE2903.zlen = zlen;
    z3zE2903.zoff = zoff;
    z3zE2903.zsource = zsrc;
    zfields = z3zE2903;
  }
  z8zE87 = zfields;
end_function_3373: ;
  return z8zE87;
end_block_exception_3374: ;
  struct zByteSliceFields z8zE772 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE772;
}

void create_letbind_74(void) {

  struct zByteSliceFields z3zE77;
  z3zE77 = zbyte_slice(zStatelessInputSource, UINT64_C(0), UINT64_C(0));
  zEMPTY_SLICE = z3zE77;
let_end_165: ;
}
void kill_letbind_74(void) {
}

struct zByteSliceFields zsub_slice(struct zByteSliceFields zs, uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE88;
  enum zByteSource z2zE3202;
  z2zE3202 = zs.zsource;
  uint64_t z2zE3203;
  {
    uint64_t z2zE3201;
    z2zE3201 = zs.zoff;
    {    z2zE3203 = (z2zE3201 + zoff);
    }
  }
  z8zE88 = zbyte_slice(z2zE3202, z2zE3203, zlen);
end_function_3371: ;
  return z8zE88;
end_block_exception_3372: ;
  struct zByteSliceFields z8zE773 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE773;
}

struct zByteSliceFields zslice_suffix(struct zByteSliceFields zs, uint64_t zoff)
{
  struct zByteSliceFields z8zE89;
  enum zByteSource z2zE3198;
  z2zE3198 = zs.zsource;
  uint64_t z2zE3199;
  {
    uint64_t z2zE3196;
    z2zE3196 = zs.zoff;
    {    z2zE3199 = (z2zE3196 + zoff);
    }
  }
  uint64_t z2zE3200;
  {
    uint64_t z2zE3197;
    z2zE3197 = zs.zlen;
    {    z2zE3200 = (z2zE3197 - zoff);
    }
  }
  z8zE89 = zbyte_slice(z2zE3198, z2zE3199, z2zE3200);
end_function_3369: ;
  return z8zE89;
end_block_exception_3370: ;
  struct zByteSliceFields z8zE774 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE774;
}

void zmaterializzed_bytes(struct zMaterializzedBytes *z8zE90, zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  struct zMaterializzedBytes z3zE2902;
  CREATE(zMaterializzedBytes)(&z3zE2902);
  COPY(zz5listz8z5bv8z9)(&((&z3zE2902)->zdata), zdata);
  z3zE2902.zlen = zlen;
  COPY(zMaterializzedBytes)((*(&z8zE90)), z3zE2902);
  KILL(zMaterializzedBytes)(&z3zE2902);
end_function_3367: ;
  goto end_function_3660;
end_block_exception_3368: ;
  goto end_function_3660;
end_function_3660: ;
}

void zbytes_list(struct zBytes *z8zE91, zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  struct zMaterializzedBytes z2zE3195;
  CREATE(zMaterializzedBytes)(&z2zE3195);
  zmaterializzed_bytes(&z2zE3195, zdata, zlen);
  zBytesList((*(&z8zE91)), z2zE3195);
  KILL(zMaterializzedBytes)(&z2zE3195);
end_function_3365: ;
  goto end_function_3659;
end_block_exception_3366: ;
  goto end_function_3659;
end_function_3659: ;
}

void zbytes_fixed32(struct zBytes *z8zE92, sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  struct zFixedBytes32 z3zE2901;
  z3zE2901.zdata = zdata;
  z3zE2901.zlen = zlen;
  zBytesFixed32((*(&z8zE92)), z3zE2901);
end_function_3363: ;
  goto end_function_3658;
end_block_exception_3364: ;
  goto end_function_3658;
end_function_3658: ;
}

