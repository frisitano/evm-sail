/* Generated from sail/host/kernel/selfdestruct.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
unit zk_selfdestruct(sail_fixed_bytes_20 za)
{
  unit z8zE277;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3041;  }
  }
  bool z2zE2927;
  {
    bool z2zE2926;
    z2zE2926 = zcur.zselfdestructed;
    z2zE2927 = not(z2zE2926);
  }
  if (z2zE2927) {
    struct zAccount z2zE2928;
    {
      struct zAccount z3zE3025;
      z3zE3025 = zcur;
      z3zE3025.zselfdestructed = true;
      z2zE2928 = z3zE3025;
    }
    z8zE277 = zstore_account(za, z2zE2928);
  } else {  z8zE277 = UNIT;  }
end_function_3040: ;
  return z8zE277;
end_block_exception_3041: ;

  return UNIT;
}

bool zk_is_selfdestructed(sail_fixed_bytes_20 za)
{
  bool z8zE278;
  struct zAccount z2zE2925;
  {
    z2zE2925 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3039;  }
  }
  z8zE278 = z2zE2925.zselfdestructed;
end_function_3038: ;
  return z8zE278;
end_block_exception_3039: ;

  return false;
}

unit zk_mark_created(sail_fixed_bytes_20 za)
{
  unit z8zE279;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3037;  }
  }
  struct zAccount z2zE2924;
  {
    struct zAccount z3zE3024;
    z3zE3024 = zcur;
    z3zE3024.zcreated = true;
    z2zE2924 = z3zE3024;
  }
  z8zE279 = zstore_account(za, z2zE2924);
end_function_3036: ;
  return z8zE279;
end_block_exception_3037: ;

  return UNIT;
}

bool zk_was_created(sail_fixed_bytes_20 za)
{
  bool z8zE280;
  struct zAccount z2zE2923;
  {
    z2zE2923 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3035;  }
  }
  z8zE280 = z2zE2923.zcreated;
end_function_3034: ;
  return z8zE280;
end_block_exception_3035: ;

  return false;
}

unit zk_zzero_balance(sail_fixed_bytes_20 za)
{
  unit z8zE281;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3033;  }
  }
  bool z2zE2920;
  {
    sail_u256 z2zE2919;
    {
      struct zAccountInfo z2zE2918;
      z2zE2918 = zcur.zinfo;
      z2zE2919 = z2zE2918.zbalance;
    }
    z2zE2920 = zword_is_zzero(z2zE2919);
  }
  if (z2zE2920) {  z8zE281 = UNIT;  } else {
    struct zAccountInfo z2zE2922;
    {
      struct zAccountInfo z2zE2921;
      z2zE2921 = zcur.zinfo;
      struct zAccountInfo z3zE3023;
      z3zE3023 = z2zE2921;
      z3zE3023.zbalance = zZERO_WORD;
      z2zE2922 = z3zE3023;
    }
    z8zE281 = zstore_account_info(za, zcur, z2zE2922);
  }
end_function_3032: ;
  return z8zE281;
end_block_exception_3033: ;

  return UNIT;
}

