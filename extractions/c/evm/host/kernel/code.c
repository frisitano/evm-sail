/* Generated from sail/host/kernel/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_fixed_bytes_32 zk_code_key(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE266;
  struct zAccountInfo z2zE2950;
  {
    struct zAccount z2zE2949;
    {
      z2zE2949 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3065;  }
    }
    z2zE2950 = z2zE2949.zinfo;
  }
  z8zE266 = z2zE2950.zcode_hash;
end_function_3064: ;
  return z8zE266;
end_block_exception_3065: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zk_get_codehash(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE267;
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3063;  }
  }
  bool z2zE2947;
  {
    bool z2zE2946;
    z2zE2946 = zacc.zpresent;
    z2zE2947 = not(z2zE2946);
  }
  if (z2zE2947) {  z8zE267 = zZERO_HASH;  } else {
    struct zAccountInfo z2zE2948;
    z2zE2948 = zacc.zinfo;
    z8zE267 = z2zE2948.zcode_hash;
  }
end_function_3062: ;
  return z8zE267;
end_block_exception_3063: ;

  return fixed_bytes_32_zero();
}

unit zk_deploy_code(sail_fixed_bytes_20 za, struct zByteSliceFields zcode)
{
  unit z8zE268;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3061;  }
  }
  sail_fixed_bytes_32 zh;
  zh = zcode_db_insert(zcode, zk_fork);
  struct zAccountInfo z2zE2945;
  {
    struct zAccountInfo z2zE2944;
    z2zE2944 = zcur.zinfo;
    struct zAccountInfo z3zE3036;
    z3zE3036 = z2zE2944;
    z3zE3036.zcode_hash = zh;
    z2zE2945 = z3zE3036;
  }
  z8zE268 = zstore_account_info(za, zcur, z2zE2945);
end_function_3060: ;
  return z8zE268;
end_block_exception_3061: ;

  return UNIT;
}

uint64_t zdelegation_address_index(uint64_t zindex)
{
  uint64_t z8zE269;
  {    z8zE269 = (UINT64_C(19) - zindex);
  }
end_function_3058: ;
  return z8zE269;
end_block_exception_3059: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zdelegation_code_index(uint64_t zindex)
{
  uint64_t z8zE270;
  {    z8zE270 = (UINT64_C(3) + zindex);
  }
end_function_3056: ;
  return z8zE270;
end_block_exception_3057: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zdelegation_jumpdest_chunk(sail_fixed_bytes_20 ztarget)
{
  sail_u256 z8zE271;
  sail_u256 zbits;
  zbits = zEMPTY_JUMPDEST_CHUNK;
  int64_t z3zE3031;
  {    z3zE3031 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3032;
  {    z3zE3032 = (int64_t)(UINT64_C(19));
  }
  int64_t z3zE3033;
  {    z3zE3033 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3031;
    unit z3zE3034;
  for_start_3052: ;
    {
      if ((z3zE3032 < zk)) goto for_end_3053;
      uint64_t zb;
      {
        uint64_t z2zE2943;
        {
          uint64_t z3zE3785;
          z3zE3785 = (uint64_t)(zk);
          z2zE2943 = zdelegation_address_index(z3zE3785);
        }
        zb = fast_unsigned_vector_access_fixed_bytes_20(ztarget, z2zE2943);
      }
      bool z2zE2940;
      z2zE2940 = (zb == UINT64_C(0x5B));
      if (z2zE2940) {
        sail_u256 z2zE2942;
        {
          uint64_t z2zE2941;
          {
            uint64_t z3zE3786;
            z3zE3786 = (uint64_t)(zk);
            z2zE2941 = zdelegation_code_index(z3zE3786);
          }
          z2zE2942 = u256_shiftl_u64(((sail_u256){{UINT64_C(0x0000000000000001), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000)}}), z2zE2941);
        }
        zbits = u256_or(zbits, z2zE2942);
        z3zE3034 = UNIT;
      } else {  z3zE3034 = UNIT;  }
      zk = (zk + z3zE3033);
      goto for_start_3052;
    }
  for_end_3053: ;
  }
  unit z3zE3035;
  z3zE3035 = UNIT;
  z8zE271 = zbits;
end_function_3054: ;
  return z8zE271;
end_block_exception_3055: ;

  return u256_zero();
}

unit zk_set_delegation(sail_fixed_bytes_20 za, sail_fixed_bytes_20 ztarget)
{
  unit z8zE272;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3051;  }
  }
  uint64_t zcode_len;
  zcode_len = UINT64_C(23);
  uint64_t zcode_length;
  zcode_length = zcode_len;
  uint64_t zmodel_code_length;
  zmodel_code_length = zcode_length;
  uint64_t ztable;
  ztable = jumpdest_table_alloc(zmodel_code_length);
  bool z2zE2936;
  z2zE2936 = (ztable != zEMPTY_JUMPDEST_REF);
  unit z3zE3028;
  z3zE3028 = sail_assert(z2zE2936, "delegation JUMPDEST table allocation");
  sail_u256 zchunk;
  zchunk = zdelegation_jumpdest_chunk(ztarget);
  bool z2zE2937;
  z2zE2937 = (!eq_u256(zchunk, zEMPTY_JUMPDEST_CHUNK));
  unit z3zE3029;
  if (z2zE2937) {
    bool zstored;
    zstored = jumpdest_table_store_chunk(ztable, zmodel_code_length, UINT64_C(0), zchunk);
    z3zE3029 = sail_assert(zstored, "delegation JUMPDEST chunk store");
  } else {  z3zE3029 = UNIT;  }
  sail_fixed_bytes_32 zh;
  zh = code_intern_indexed_delegation(ztarget, ztable);
  struct zAccountInfo z2zE2939;
  {
    struct zAccountInfo z2zE2938;
    z2zE2938 = zcur.zinfo;
    struct zAccountInfo z3zE3030;
    z3zE3030 = z2zE2938;
    z3zE3030.zcode_hash = zh;
    z2zE2939 = z3zE3030;
  }
  z8zE272 = zstore_account_info(za, zcur, z2zE2939);
end_function_3050: ;
  return z8zE272;
end_block_exception_3051: ;

  return UNIT;
}

unit zk_clear_code(sail_fixed_bytes_20 za)
{
  unit z8zE273;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3049;  }
  }
  struct zAccountInfo z2zE2935;
  {
    struct zAccountInfo z2zE2934;
    z2zE2934 = zcur.zinfo;
    struct zAccountInfo z3zE3027;
    z3zE3027 = z2zE2934;
    z3zE3027.zcode_hash = zKECCAK_EMPTY;
    z2zE2935 = z3zE3027;
  }
  z8zE273 = zstore_account_info(za, zcur, z2zE2935);
end_function_3048: ;
  return z8zE273;
end_block_exception_3049: ;

  return UNIT;
}

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zk_deleg_target(sail_fixed_bytes_20 za)
{
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE274;
  sail_fixed_bytes_32 zh;
  {
    zh = zk_code_key(za);
    if (have_exception) {  goto end_block_exception_3047;  }
  }
  struct zAddressResult zr;
  zr = code_db_read_delegation(zh);
  bool z2zE2932;
  z2zE2932 = zr.zsuccess;
  sail_fixed_bytes_20 z2zE2933;
  z2zE2933 = zr.zaddress;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3026;
  z3zE3026.ztup0 = z2zE2932;
  z3zE3026.ztup1 = z2zE2933;
  z8zE274 = z3zE3026;
end_function_3046: ;
  return z8zE274;
end_block_exception_3047: ;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE881 = { .ztup0 = false, .ztup1 = fixed_bytes_20_zero() };
  return z8zE881;
}

uint64_t zk_get_code_sizze(sail_fixed_bytes_20 za)
{
  uint64_t z8zE275;
  struct zCode zcode;
  {
    sail_fixed_bytes_32 z2zE2931;
    {
      z2zE2931 = zk_code_key(za);
      if (have_exception) {  goto end_block_exception_3045;  }
    }
    {
      zcode = zcode_db_resolve(z2zE2931);
      if (have_exception) {  goto end_block_exception_3045;  }
    }
  }
  struct zByteSliceFields zbytes;
  zbytes = zcode.zbytes;
  z8zE275 = zbytes.zlen;
end_function_3044: ;
  return z8zE275;
end_block_exception_3045: ;

  return UINT64_C(0xdeadc0de);
}

unit zk_code_copy(sail_fixed_bytes_20 za, uint64_t zdst, sail_u256 zoff, uint64_t zlen)
{
  unit z8zE276;
  struct zCode zcode;
  {
    sail_fixed_bytes_32 z2zE2930;
    {
      z2zE2930 = zk_code_key(za);
      if (have_exception) {  goto end_block_exception_3043;  }
    }
    {
      zcode = zcode_db_resolve(z2zE2930);
      if (have_exception) {  goto end_block_exception_3043;  }
    }
  }
  struct zByteSliceFields z2zE2929;
  z2zE2929 = zcode.zbytes;
  z8zE276 = zslice_copy_word_offset(z2zE2929, zdst, zoff, zlen);
end_function_3042: ;
  return z8zE276;
end_block_exception_3043: ;

  return UNIT;
}

