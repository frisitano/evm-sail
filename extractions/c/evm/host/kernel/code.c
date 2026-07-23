/* Generated from sail/host/kernel/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_fixed_bytes_32 zk_code_key(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE269;
  struct zAccountInfo z2zE2973;
  {
    struct zAccount z2zE2972;
    {
      z2zE2972 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3069;  }
    }
    z2zE2973 = z2zE2972.zinfo;
  }
  z8zE269 = z2zE2973.zcode_hash;
end_function_3068: ;
  return z8zE269;
end_block_exception_3069: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zk_get_codehash(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE270;
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3067;  }
  }
  bool z2zE2970;
  {
    bool z2zE2969;
    z2zE2969 = zacc.zpresent;
    z2zE2970 = not(z2zE2969);
  }
  if (z2zE2970) {  z8zE270 = zZERO_HASH;  } else {
    struct zAccountInfo z2zE2971;
    z2zE2971 = zacc.zinfo;
    z8zE270 = z2zE2971.zcode_hash;
  }
end_function_3066: ;
  return z8zE270;
end_block_exception_3067: ;

  return fixed_bytes_32_zero();
}

unit zk_deploy_code(sail_fixed_bytes_20 za, struct zByteSliceFields zcode)
{
  unit z8zE271;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3065;  }
  }
  sail_fixed_bytes_32 zh;
  zh = zcode_db_insert(zcode, zk_fork);
  struct zAccountInfo z2zE2968;
  {
    struct zAccountInfo z2zE2967;
    z2zE2967 = zcur.zinfo;
    struct zAccountInfo z3zE3032;
    z3zE3032 = z2zE2967;
    z3zE3032.zcode_hash = zh;
    z2zE2968 = z3zE3032;
  }
  z8zE271 = zstore_account_info(za, zcur, z2zE2968);
end_function_3064: ;
  return z8zE271;
end_block_exception_3065: ;

  return UNIT;
}

uint64_t zdelegation_address_index(uint64_t zindex)
{
  uint64_t z8zE272;
  {    z8zE272 = (UINT64_C(19) - zindex);
  }
end_function_3062: ;
  return z8zE272;
end_block_exception_3063: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zdelegation_code_index(uint64_t zindex)
{
  uint64_t z8zE273;
  {    z8zE273 = (UINT64_C(3) + zindex);
  }
end_function_3060: ;
  return z8zE273;
end_block_exception_3061: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zdelegation_jumpdest_chunk(sail_fixed_bytes_20 ztarget)
{
  sail_u256 z8zE274;
  sail_u256 zbits;
  zbits = zEMPTY_JUMPDEST_CHUNK;
  int64_t z3zE3027;
  {    z3zE3027 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3028;
  {    z3zE3028 = (int64_t)(UINT64_C(19));
  }
  int64_t z3zE3029;
  {    z3zE3029 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3027;
    unit z3zE3030;
  for_start_3056: ;
    {
      if ((z3zE3028 < zk)) goto for_end_3057;
      uint64_t zb;
      {
        uint64_t z2zE2966;
        {
          uint64_t z3zE3856;
          z3zE3856 = (uint64_t)(zk);
          z2zE2966 = zdelegation_address_index(z3zE3856);
        }
        zb = fast_unsigned_vector_access_fixed_bytes_20(ztarget, z2zE2966);
      }
      bool z2zE2963;
      z2zE2963 = (zb == UINT64_C(0x5B));
      if (z2zE2963) {
        sail_u256 z2zE2965;
        {
          uint64_t z2zE2964;
          {
            uint64_t z3zE3857;
            z3zE3857 = (uint64_t)(zk);
            z2zE2964 = zdelegation_code_index(z3zE3857);
          }
          z2zE2965 = u256_shiftl_u64(((sail_u256){{UINT64_C(0x0000000000000001), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000)}}), z2zE2964);
        }
        zbits = u256_or(zbits, z2zE2965);
        z3zE3030 = UNIT;
      } else {  z3zE3030 = UNIT;  }
      zk = (zk + z3zE3029);
      goto for_start_3056;
    }
  for_end_3057: ;
  }
  unit z3zE3031;
  z3zE3031 = UNIT;
  z8zE274 = zbits;
end_function_3058: ;
  return z8zE274;
end_block_exception_3059: ;

  return u256_zero();
}

unit zk_set_delegation(sail_fixed_bytes_20 za, sail_fixed_bytes_20 ztarget)
{
  unit z8zE275;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3055;  }
  }
  uint64_t zcode_len;
  zcode_len = UINT64_C(23);
  uint64_t zcode_length;
  zcode_length = zcode_len;
  uint64_t zmodel_code_length;
  zmodel_code_length = zcode_length;
  uint64_t ztable;
  ztable = jumpdest_table_alloc(zmodel_code_length);
  bool z2zE2959;
  z2zE2959 = (ztable != zEMPTY_JUMPDEST_REF);
  unit z3zE3024;
  z3zE3024 = sail_assert(z2zE2959, "delegation JUMPDEST table allocation");
  sail_u256 zchunk;
  zchunk = zdelegation_jumpdest_chunk(ztarget);
  bool z2zE2960;
  z2zE2960 = (!eq_u256(zchunk, zEMPTY_JUMPDEST_CHUNK));
  unit z3zE3025;
  if (z2zE2960) {
    bool zstored;
    zstored = jumpdest_table_store_chunk(ztable, zmodel_code_length, UINT64_C(0), zchunk);
    z3zE3025 = sail_assert(zstored, "delegation JUMPDEST chunk store");
  } else {  z3zE3025 = UNIT;  }
  sail_fixed_bytes_32 zh;
  zh = code_intern_indexed_delegation(ztarget, ztable);
  struct zAccountInfo z2zE2962;
  {
    struct zAccountInfo z2zE2961;
    z2zE2961 = zcur.zinfo;
    struct zAccountInfo z3zE3026;
    z3zE3026 = z2zE2961;
    z3zE3026.zcode_hash = zh;
    z2zE2962 = z3zE3026;
  }
  z8zE275 = zstore_account_info(za, zcur, z2zE2962);
end_function_3054: ;
  return z8zE275;
end_block_exception_3055: ;

  return UNIT;
}

unit zk_clear_code(sail_fixed_bytes_20 za)
{
  unit z8zE276;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3053;  }
  }
  struct zAccountInfo z2zE2958;
  {
    struct zAccountInfo z2zE2957;
    z2zE2957 = zcur.zinfo;
    struct zAccountInfo z3zE3023;
    z3zE3023 = z2zE2957;
    z3zE3023.zcode_hash = zKECCAK_EMPTY;
    z2zE2958 = z3zE3023;
  }
  z8zE276 = zstore_account_info(za, zcur, z2zE2958);
end_function_3052: ;
  return z8zE276;
end_block_exception_3053: ;

  return UNIT;
}

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zk_deleg_target(sail_fixed_bytes_20 za)
{
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE277;
  sail_fixed_bytes_32 zh;
  {
    zh = zk_code_key(za);
    if (have_exception) {  goto end_block_exception_3051;  }
  }
  struct zAddressResult zr;
  zr = code_db_read_delegation(zh);
  bool z2zE2955;
  z2zE2955 = zr.zsuccess;
  sail_fixed_bytes_20 z2zE2956;
  z2zE2956 = zr.zaddress;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3022;
  z3zE3022.ztup0 = z2zE2955;
  z3zE3022.ztup1 = z2zE2956;
  z8zE277 = z3zE3022;
end_function_3050: ;
  return z8zE277;
end_block_exception_3051: ;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE893 = { .ztup0 = false, .ztup1 = fixed_bytes_20_zero() };
  return z8zE893;
}

uint64_t zk_get_code_sizze(sail_fixed_bytes_20 za)
{
  uint64_t z8zE278;
  struct zCode zcode;
  {
    sail_fixed_bytes_32 z2zE2954;
    {
      z2zE2954 = zk_code_key(za);
      if (have_exception) {  goto end_block_exception_3049;  }
    }
    {
      zcode = zcode_db_resolve(z2zE2954);
      if (have_exception) {  goto end_block_exception_3049;  }
    }
  }
  struct zByteSliceFields zbytes;
  zbytes = zcode.zbytes;
  z8zE278 = zbytes.zlen;
end_function_3048: ;
  return z8zE278;
end_block_exception_3049: ;

  return UINT64_C(0xdeadc0de);
}

unit zk_code_copy(sail_fixed_bytes_20 za, uint64_t zdst, sail_u256 zoff, uint64_t zlen)
{
  unit z8zE279;
  struct zCode zcode;
  {
    sail_fixed_bytes_32 z2zE2953;
    {
      z2zE2953 = zk_code_key(za);
      if (have_exception) {  goto end_block_exception_3047;  }
    }
    {
      zcode = zcode_db_resolve(z2zE2953);
      if (have_exception) {  goto end_block_exception_3047;  }
    }
  }
  struct zByteSliceFields z2zE2952;
  z2zE2952 = zcode.zbytes;
  z8zE279 = zslice_copy_word_offset(z2zE2952, zdst, zoff, zlen);
end_function_3046: ;
  return z8zE279;
end_block_exception_3047: ;

  return UNIT;
}

