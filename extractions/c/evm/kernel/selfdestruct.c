/* Generated from sail/kernel/selfdestruct.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
unit zk_selfdestruct(sail_fixed_bytes_20 za)
{
  unit z8zE249;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2746;  }
  }
  bool z2zE2424;
  {
    bool z2zE2423;
    z2zE2423 = zcur.zselfdestructed;
    z2zE2424 = not(z2zE2423);
  }
  if (z2zE2424) {
    struct zAccount z2zE2425;
    {
      struct zAccount z3zE2375;
      z3zE2375 = zcur;
      z3zE2375.zselfdestructed = true;
      z2zE2425 = z3zE2375;
    }
    z8zE249 = zstore_account(za, z2zE2425);
  } else {  z8zE249 = UNIT;  }
end_function_2745: ;
  return z8zE249;
end_block_exception_2746: ;

  return UNIT;
}

bool zk_is_selfdestructed(sail_fixed_bytes_20 za)
{
  bool z8zE250;
  struct zAccount z2zE2422;
  {
    z2zE2422 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2744;  }
  }
  z8zE250 = z2zE2422.zselfdestructed;
end_function_2743: ;
  return z8zE250;
end_block_exception_2744: ;

  return false;
}

unit zk_mark_created(sail_fixed_bytes_20 za)
{
  unit z8zE251;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2742;  }
  }
  struct zAccount z2zE2421;
  {
    struct zAccount z3zE2374;
    z3zE2374 = zcur;
    z3zE2374.zcreated = true;
    z2zE2421 = z3zE2374;
  }
  z8zE251 = zstore_account(za, z2zE2421);
end_function_2741: ;
  return z8zE251;
end_block_exception_2742: ;

  return UNIT;
}

bool zk_was_created(sail_fixed_bytes_20 za)
{
  bool z8zE252;
  struct zAccount z2zE2420;
  {
    z2zE2420 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2740;  }
  }
  z8zE252 = z2zE2420.zcreated;
end_function_2739: ;
  return z8zE252;
end_block_exception_2740: ;

  return false;
}

unit zk_zzero_balance(sail_fixed_bytes_20 za)
{
  unit z8zE253;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2738;  }
  }
  bool z2zE2417;
  {
    sail_u256 z2zE2416;
    {
      struct zAccountInfo z2zE2415;
      z2zE2415 = zcur.zinfo;
      z2zE2416 = z2zE2415.zbalance;
    }
    z2zE2417 = zword_is_zzero(z2zE2416);
  }
  if (z2zE2417) {  z8zE253 = UNIT;  } else {
    struct zAccountInfo z2zE2419;
    {
      struct zAccountInfo z2zE2418;
      z2zE2418 = zcur.zinfo;
      struct zAccountInfo z3zE2373;
      z3zE2373 = z2zE2418;
      z3zE2373.zbalance = zZERO_WORD;
      z2zE2419 = z3zE2373;
    }
    z8zE253 = zstore_account_info(za, zcur, z2zE2419);
  }
end_function_2737: ;
  return z8zE253;
end_block_exception_2738: ;

  return UNIT;
}
