/* Generated from sail/primitives/bytes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_46(void) {

  uint64_t z3zE49;
  z3zE49 = UINT64_C(20);
  zADDRESS_BYTE_LENGTH = z3zE49;
let_end_135: ;
}
void kill_letbind_46(void) {
}

void create_letbind_47(void) {

  uint64_t z3zE50;
  z3zE50 = UINT64_C(32);
  zWORD_BYTE_LENGTH = z3zE50;
let_end_136: ;
}
void kill_letbind_47(void) {
}

void create_letbind_48(void) {

  uint64_t z3zE51;
  z3zE51 = UINT64_C(8);
  zEIGHT_BYTE_LENGTH = z3zE51;
let_end_137: ;
}
void kill_letbind_48(void) {
}

void create_letbind_49(void) {

  uint64_t z3zE52;
  z3zE52 = UINT64_C(64);
  zDOUBLE_WORD_BYTE_LENGTH = z3zE52;
let_end_138: ;
}
void kill_letbind_49(void) {
}

struct zByteSliceFields zbyte_slice(enum zByteSource zsrc, uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE86;
  struct zByteSliceFields zfields;
  {
    struct zByteSliceFields z3zE3754;
    z3zE3754.zlen = zlen;
    z3zE3754.zoff = zoff;
    z3zE3754.zsource = zsrc;
    zfields = z3zE3754;
  }
  z8zE86 = zfields;
end_function_3808: ;
  return z8zE86;
end_block_exception_3809: ;
  struct zByteSliceFields z8zE811 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE811;
}

void create_letbind_50(void) {

  struct zByteSliceFields z3zE53;
  z3zE53 = zbyte_slice(zStatelessInputSource, UINT64_C(0), UINT64_C(0));
  zEMPTY_SLICE = z3zE53;
let_end_140: ;
}
void kill_letbind_50(void) {
}

struct zByteSliceFields zsub_slice(struct zByteSliceFields zs, uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE87;
  enum zByteSource z2zE3949;
  z2zE3949 = zs.zsource;
  uint64_t z2zE3950;
  {
    uint64_t z2zE3948;
    z2zE3948 = zs.zoff;
    {    z2zE3950 = (z2zE3948 + zoff);
    }
  }
  z8zE87 = zbyte_slice(z2zE3949, z2zE3950, zlen);
end_function_3806: ;
  return z8zE87;
end_block_exception_3807: ;
  struct zByteSliceFields z8zE812 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE812;
}

struct zByteSliceFields zslice_suffix(struct zByteSliceFields zs, uint64_t zoff)
{
  struct zByteSliceFields z8zE88;
  enum zByteSource z2zE3945;
  z2zE3945 = zs.zsource;
  uint64_t z2zE3946;
  {
    uint64_t z2zE3943;
    z2zE3943 = zs.zoff;
    {    z2zE3946 = (z2zE3943 + zoff);
    }
  }
  uint64_t z2zE3947;
  {
    uint64_t z2zE3944;
    z2zE3944 = zs.zlen;
    {    z2zE3947 = (z2zE3944 - zoff);
    }
  }
  z8zE88 = zbyte_slice(z2zE3945, z2zE3946, z2zE3947);
end_function_3804: ;
  return z8zE88;
end_block_exception_3805: ;
  struct zByteSliceFields z8zE813 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE813;
}

void zmaterializzed_bytes(struct zMaterializzedBytes *z8zE89, zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  struct zMaterializzedBytes z3zE3753;
  CREATE(zMaterializzedBytes)(&z3zE3753);
  COPY(zz5listz8z5bv8z9)(&((&z3zE3753)->zdata), zdata);
  z3zE3753.zlen = zlen;
  COPY(zMaterializzedBytes)((*(&z8zE89)), z3zE3753);
  KILL(zMaterializzedBytes)(&z3zE3753);
end_function_3802: ;
  goto end_function_4091;
end_block_exception_3803: ;
  goto end_function_4091;
end_function_4091: ;
}

void zbytes_list(struct zBytes *z8zE90, zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  struct zMaterializzedBytes z2zE3942;
  CREATE(zMaterializzedBytes)(&z2zE3942);
  zmaterializzed_bytes(&z2zE3942, zdata, zlen);
  zBytesList((*(&z8zE90)), z2zE3942);
  KILL(zMaterializzedBytes)(&z2zE3942);
end_function_3800: ;
  goto end_function_4090;
end_block_exception_3801: ;
  goto end_function_4090;
end_function_4090: ;
}

void zbytes_fixed32(struct zBytes *z8zE91, sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  struct zFixedBytes32 z3zE3752;
  z3zE3752.zdata = zdata;
  z3zE3752.zlen = zlen;
  zBytesFixed32((*(&z8zE91)), z3zE3752);
end_function_3798: ;
  goto end_function_4089;
end_block_exception_3799: ;
  goto end_function_4089;
end_function_4089: ;
}

