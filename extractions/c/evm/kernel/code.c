/* Generated from sail/kernel/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_fixed_bytes_32 zk_code_key(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE239;
  struct zAccountInfo z2zE2446;
  {
    struct zAccount z2zE2445;
    {
      z2zE2445 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_2768;  }
    }
    z2zE2446 = z2zE2445.zinfo;
  }
  z8zE239 = z2zE2446.zcode_hash;
end_function_2767: ;
  return z8zE239;
end_block_exception_2768: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zk_get_codehash(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE240;
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2766;  }
  }
  bool z2zE2443;
  {
    bool z2zE2442;
    z2zE2442 = zacc.zpresent;
    z2zE2443 = not(z2zE2442);
  }
  if (z2zE2443) {  z8zE240 = zZERO_HASH;  } else {
    struct zAccountInfo z2zE2444;
    z2zE2444 = zacc.zinfo;
    z8zE240 = z2zE2444.zcode_hash;
  }
end_function_2765: ;
  return z8zE240;
end_block_exception_2766: ;

  return fixed_bytes_32_zero();
}

unit zk_deploy_code(sail_fixed_bytes_20 za, struct zByteSliceFields zcode)
{
  unit z8zE241;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2764;  }
  }
  sail_fixed_bytes_32 zh;
  zh = zcode_db_insert(zcode, zk_fork);
  struct zAccountInfo z2zE2441;
  {
    struct zAccountInfo z2zE2440;
    z2zE2440 = zcur.zinfo;
    struct zAccountInfo z3zE2386;
    z3zE2386 = z2zE2440;
    z3zE2386.zcode_hash = zh;
    z2zE2441 = z3zE2386;
  }
  z8zE241 = zstore_account_info(za, zcur, z2zE2441);
end_function_2763: ;
  return z8zE241;
end_block_exception_2764: ;

  return UNIT;
}

uint64_t zdelegation_code_index(uint64_t zindex)
{
  uint64_t z8zE242;
  {    z8zE242 = (UINT64_C(3) + zindex);
  }
end_function_2761: ;
  return z8zE242;
end_block_exception_2762: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zdelegation_jumpdest_chunk(sail_fixed_bytes_20 ztarget)
{
  sail_u256 z8zE243;
  sail_u256 zbits;
  zbits = zEMPTY_JUMPDEST_CHUNK;
  int64_t z3zE2381;
  {    z3zE2381 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2382;
  {    z3zE2382 = (int64_t)(UINT64_C(19));
  }
  int64_t z3zE2383;
  {    z3zE2383 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE2381;
    unit z3zE2384;
  for_start_2757: ;
    {
      if ((z3zE2382 < zk)) goto for_end_2758;
      uint64_t zb;
      zb = fast_vector_access_fixed_bytes_20(ztarget, zk);
      bool z2zE2437;
      z2zE2437 = (zb == UINT64_C(0x5B));
      if (z2zE2437) {
        sail_u256 z2zE2439;
        {
          uint64_t z2zE2438;
          {
            uint64_t z3zE3020;
            z3zE3020 = (uint64_t)(zk);
            z2zE2438 = zdelegation_code_index(z3zE3020);
          }
          z2zE2439 = u256_shiftl_u64(((sail_u256){{UINT64_C(0x0000000000000001), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000)}}), z2zE2438);
        }
        zbits = u256_or(zbits, z2zE2439);
        z3zE2384 = UNIT;
      } else {  z3zE2384 = UNIT;  }
      zk = (zk + z3zE2383);
      goto for_start_2757;
    }
  for_end_2758: ;
  }
  unit z3zE2385;
  z3zE2385 = UNIT;
  z8zE243 = zbits;
end_function_2759: ;
  return z8zE243;
end_block_exception_2760: ;

  return u256_zero();
}

unit zk_set_delegation(sail_fixed_bytes_20 za, sail_fixed_bytes_20 ztarget)
{
  unit z8zE244;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2756;  }
  }
  uint64_t zcode_len;
  zcode_len = UINT64_C(23);
  uint64_t zcode_length;
  zcode_length = zcode_len;
  uint64_t zmodel_code_length;
  zmodel_code_length = zcode_length;
  uint64_t ztable;
  ztable = jumpdest_table_alloc(zmodel_code_length);
  bool z2zE2433;
  z2zE2433 = (ztable != zEMPTY_JUMPDEST_REF);
  unit z3zE2378;
  z3zE2378 = sail_assert(z2zE2433, "delegation JUMPDEST table allocation");
  sail_u256 zchunk;
  zchunk = zdelegation_jumpdest_chunk(ztarget);
  bool z2zE2434;
  z2zE2434 = (!eq_u256(zchunk, zEMPTY_JUMPDEST_CHUNK));
  unit z3zE2379;
  if (z2zE2434) {
    bool zstored;
    zstored = jumpdest_table_store_chunk(ztable, zmodel_code_length, UINT64_C(0), zchunk);
    z3zE2379 = sail_assert(zstored, "delegation JUMPDEST chunk store");
  } else {  z3zE2379 = UNIT;  }
  sail_fixed_bytes_32 zh;
  zh = code_intern_indexed_delegation(ztarget, ztable);
  struct zAccountInfo z2zE2436;
  {
    struct zAccountInfo z2zE2435;
    z2zE2435 = zcur.zinfo;
    struct zAccountInfo z3zE2380;
    z3zE2380 = z2zE2435;
    z3zE2380.zcode_hash = zh;
    z2zE2436 = z3zE2380;
  }
  z8zE244 = zstore_account_info(za, zcur, z2zE2436);
end_function_2755: ;
  return z8zE244;
end_block_exception_2756: ;

  return UNIT;
}

unit zk_clear_code(sail_fixed_bytes_20 za)
{
  unit z8zE245;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2754;  }
  }
  struct zAccountInfo z2zE2432;
  {
    struct zAccountInfo z2zE2431;
    z2zE2431 = zcur.zinfo;
    struct zAccountInfo z3zE2377;
    z3zE2377 = z2zE2431;
    z3zE2377.zcode_hash = zKECCAK_EMPTY;
    z2zE2432 = z3zE2377;
  }
  z8zE245 = zstore_account_info(za, zcur, z2zE2432);
end_function_2753: ;
  return z8zE245;
end_block_exception_2754: ;

  return UNIT;
}

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zk_deleg_target(sail_fixed_bytes_20 za)
{
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE246;
  sail_fixed_bytes_32 zh;
  {
    zh = zk_code_key(za);
    if (have_exception) {  goto end_block_exception_2752;  }
  }
  struct zAddressResult zr;
  zr = code_db_read_delegation(zh);
  bool z2zE2429;
  z2zE2429 = zr.zsuccess;
  sail_fixed_bytes_20 z2zE2430;
  z2zE2430 = zr.zaddress;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE2376;
  z3zE2376.ztup0 = z2zE2429;
  z3zE2376.ztup1 = z2zE2430;
  z8zE246 = z3zE2376;
end_function_2751: ;
  return z8zE246;
end_block_exception_2752: ;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE812 = { .ztup0 = false, .ztup1 = fixed_bytes_20_zero() };
  return z8zE812;
}

uint64_t zk_get_code_sizze(sail_fixed_bytes_20 za)
{
  uint64_t z8zE247;
  struct zCode zcode;
  {
    sail_fixed_bytes_32 z2zE2428;
    {
      z2zE2428 = zk_code_key(za);
      if (have_exception) {  goto end_block_exception_2750;  }
    }
    {
      zcode = zcode_db_resolve(z2zE2428);
      if (have_exception) {  goto end_block_exception_2750;  }
    }
  }
  struct zByteSliceFields zbytes;
  zbytes = zcode.zbytes;
  z8zE247 = zbytes.zlen;
end_function_2749: ;
  return z8zE247;
end_block_exception_2750: ;

  return UINT64_C(0xdeadc0de);
}

unit zk_code_copy(sail_fixed_bytes_20 za, uint64_t zdst, sail_u256 zoff, uint64_t zlen)
{
  unit z8zE248;
  struct zCode zcode;
  {
    sail_fixed_bytes_32 z2zE2427;
    {
      z2zE2427 = zk_code_key(za);
      if (have_exception) {  goto end_block_exception_2748;  }
    }
    {
      zcode = zcode_db_resolve(z2zE2427);
      if (have_exception) {  goto end_block_exception_2748;  }
    }
  }
  struct zByteSliceFields z2zE2426;
  z2zE2426 = zcode.zbytes;
  z8zE248 = zslice_copy_word_offset(z2zE2426, zdst, zoff, zlen);
end_function_2747: ;
  return z8zE248;
end_block_exception_2748: ;

  return UNIT;
}
