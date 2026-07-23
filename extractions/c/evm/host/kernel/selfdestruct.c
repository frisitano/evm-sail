/* Generated from sail/host/kernel/selfdestruct.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
unit zk_selfdestruct(sail_fixed_bytes_20 za)
{
  unit z8zE280;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3045;  }
  }
  bool z2zE2950;
  {
    bool z2zE2949;
    z2zE2949 = zcur.zselfdestructed;
    z2zE2950 = not(z2zE2949);
  }
  if (z2zE2950) {
    struct zAccount z2zE2951;
    {
      struct zAccount z3zE3021;
      z3zE3021 = zcur;
      z3zE3021.zselfdestructed = true;
      z2zE2951 = z3zE3021;
    }
    z8zE280 = zstore_account(za, z2zE2951);
  } else {  z8zE280 = UNIT;  }
end_function_3044: ;
  return z8zE280;
end_block_exception_3045: ;

  return UNIT;
}

bool zk_is_selfdestructed(sail_fixed_bytes_20 za)
{
  bool z8zE281;
  struct zAccount z2zE2948;
  {
    z2zE2948 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3043;  }
  }
  z8zE281 = z2zE2948.zselfdestructed;
end_function_3042: ;
  return z8zE281;
end_block_exception_3043: ;

  return false;
}

unit zk_mark_created(sail_fixed_bytes_20 za)
{
  unit z8zE282;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3041;  }
  }
  struct zAccount z2zE2947;
  {
    struct zAccount z3zE3020;
    z3zE3020 = zcur;
    z3zE3020.zcreated = true;
    z2zE2947 = z3zE3020;
  }
  z8zE282 = zstore_account(za, z2zE2947);
end_function_3040: ;
  return z8zE282;
end_block_exception_3041: ;

  return UNIT;
}

bool zk_was_created(sail_fixed_bytes_20 za)
{
  bool z8zE283;
  struct zAccount z2zE2946;
  {
    z2zE2946 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3039;  }
  }
  z8zE283 = z2zE2946.zcreated;
end_function_3038: ;
  return z8zE283;
end_block_exception_3039: ;

  return false;
}

unit zk_zzero_balance(sail_fixed_bytes_20 za)
{
  unit z8zE284;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3037;  }
  }
  bool z2zE2943;
  {
    sail_u256 z2zE2942;
    {
      struct zAccountInfo z2zE2941;
      z2zE2941 = zcur.zinfo;
      z2zE2942 = z2zE2941.zbalance;
    }
    z2zE2943 = zword_is_zzero(z2zE2942);
  }
  if (z2zE2943) {  z8zE284 = UNIT;  } else {
    struct zAccountInfo z2zE2945;
    {
      struct zAccountInfo z2zE2944;
      z2zE2944 = zcur.zinfo;
      struct zAccountInfo z3zE3019;
      z3zE3019 = z2zE2944;
      z3zE3019.zbalance = zZERO_WORD;
      z2zE2945 = z3zE3019;
    }
    z8zE284 = zstore_account_info(za, zcur, z2zE2945);
  }
end_function_3036: ;
  return z8zE284;
end_block_exception_3037: ;

  return UNIT;
}

