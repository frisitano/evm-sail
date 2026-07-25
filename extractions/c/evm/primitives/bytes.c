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
    struct zByteSliceFields z3zE3683;
    z3zE3683.zlen = zlen;
    z3zE3683.zoff = zoff;
    z3zE3683.zsource = zsrc;
    zfields = z3zE3683;
  }
  z8zE86 = zfields;
end_function_3789: ;
  return z8zE86;
end_block_exception_3790: ;
  struct zByteSliceFields z8zE808 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE808;
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
  enum zByteSource z2zE3910;
  z2zE3910 = zs.zsource;
  uint64_t z2zE3911;
  {
    uint64_t z2zE3909;
    z2zE3909 = zs.zoff;
    {    z2zE3911 = (z2zE3909 + zoff);
    }
  }
  z8zE87 = zbyte_slice(z2zE3910, z2zE3911, zlen);
end_function_3787: ;
  return z8zE87;
end_block_exception_3788: ;
  struct zByteSliceFields z8zE809 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE809;
}

struct zByteSliceFields zslice_suffix(struct zByteSliceFields zs, uint64_t zoff)
{
  struct zByteSliceFields z8zE88;
  enum zByteSource z2zE3906;
  z2zE3906 = zs.zsource;
  uint64_t z2zE3907;
  {
    uint64_t z2zE3904;
    z2zE3904 = zs.zoff;
    {    z2zE3907 = (z2zE3904 + zoff);
    }
  }
  uint64_t z2zE3908;
  {
    uint64_t z2zE3905;
    z2zE3905 = zs.zlen;
    {    z2zE3908 = (z2zE3905 - zoff);
    }
  }
  z8zE88 = zbyte_slice(z2zE3906, z2zE3907, z2zE3908);
end_function_3785: ;
  return z8zE88;
end_block_exception_3786: ;
  struct zByteSliceFields z8zE810 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE810;
}

void zmaterializzed_bytes(struct zMaterializzedBytes *z8zE89, zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  struct zMaterializzedBytes z3zE3682;
  CREATE(zMaterializzedBytes)(&z3zE3682);
  COPY(zz5listz8z5bv8z9)(&((&z3zE3682)->zdata), zdata);
  z3zE3682.zlen = zlen;
  COPY(zMaterializzedBytes)((*(&z8zE89)), z3zE3682);
  KILL(zMaterializzedBytes)(&z3zE3682);
end_function_3783: ;
  goto end_function_4079;
end_block_exception_3784: ;
  goto end_function_4079;
end_function_4079: ;
}

void zbytes_list(struct zBytes *z8zE90, zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  struct zMaterializzedBytes z2zE3903;
  CREATE(zMaterializzedBytes)(&z2zE3903);
  zmaterializzed_bytes(&z2zE3903, zdata, zlen);
  zBytesList((*(&z8zE90)), z2zE3903);
  KILL(zMaterializzedBytes)(&z2zE3903);
end_function_3781: ;
  goto end_function_4078;
end_block_exception_3782: ;
  goto end_function_4078;
end_function_4078: ;
}

void zbytes_fixed32(struct zBytes *z8zE91, sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  struct zFixedBytes32 z3zE3681;
  z3zE3681.zdata = zdata;
  z3zE3681.zlen = zlen;
  zBytesFixed32((*(&z8zE91)), z3zE3681);
end_function_3779: ;
  goto end_function_4077;
end_block_exception_3780: ;
  goto end_function_4077;
end_function_4077: ;
}

