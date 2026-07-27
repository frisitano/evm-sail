/* Generated from sail/host/kernel/accounts.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
bool zaccount_info_empty(struct zAccountInfo zinfo)
{
  bool z8zE225;
  bool z2zE2498;
  {
    sail_fixed_bytes_32 z2zE2494;
    z2zE2494 = zinfo.zcode_hash;
    z2zE2498 = eq_fixed_bytes_32(z2zE2494, zKECCAK_EMPTY);
  }
  bool z3zE2401;
  if (z2zE2498) {
    bool z2zE2497;
    {
      uint64_t z2zE2495;
      z2zE2495 = zinfo.znonce;
      z2zE2497 = (z2zE2495 == UINT64_C(0));
    }
    bool z3zE2400;
    if (z2zE2497) {
      sail_u256 z2zE2496;
      z2zE2496 = zinfo.zbalance;
      z3zE2400 = zword_is_zzero(z2zE2496);
    } else {  z3zE2400 = false;  }
    z3zE2401 = z3zE2400;
  } else {  z3zE2401 = false;  }
  z8zE225 = z3zE2401;
end_function_2795: ;
  return z8zE225;
end_block_exception_2796: ;

  return false;
}

struct zAccount zaccount_clear_storage(struct zAccount zacc)
{
  struct zAccount z8zE226;
  struct zAccount z3zE2399;
  z3zE2399 = zacc;
  z3zE2399.zstorage_cleared = true;
  z8zE226 = z3zE2399;
end_function_2793: ;
  return z8zE226;
end_block_exception_2794: ;
  struct zAccountInfo z8zE811 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE810 = { .zcreated = false, .zinfo = z8zE811, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE810;
}

unit zstore_account(sail_fixed_bytes_20 za, struct zAccount zaccount)
{
  unit z8zE227;
  z8zE227 = evmsail_store_account(za, zaccount);
end_function_2791: ;
  return z8zE227;
end_block_exception_2792: ;

  return UNIT;
}

unit zstore_account_info(sail_fixed_bytes_20 za, struct zAccount zaccount, struct zAccountInfo zinfo)
{
  unit z8zE228;
  z8zE228 = evmsail_store_account_info(za, zaccount, zinfo);
end_function_2789: ;
  return z8zE228;
end_block_exception_2790: ;

  return UNIT;
}

sail_u256 zk_get_balance(sail_fixed_bytes_20 za)
{
  sail_u256 z8zE229;
  struct zAccountInfo z2zE2493;
  {
    struct zAccount z2zE2492;
    {
      z2zE2492 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_2788;  }
    }
    z2zE2493 = z2zE2492.zinfo;
  }
  z8zE229 = z2zE2493.zbalance;
end_function_2787: ;
  return z8zE229;
end_block_exception_2788: ;

  return u256_zero();
}

uint64_t zk_get_nonce(sail_fixed_bytes_20 za)
{
  uint64_t z8zE230;
  struct zAccountInfo z2zE2491;
  {
    struct zAccount z2zE2490;
    {
      z2zE2490 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_2786;  }
    }
    z2zE2491 = z2zE2490.zinfo;
  }
  z8zE230 = z2zE2491.znonce;
end_function_2785: ;
  return z8zE230;
end_block_exception_2786: ;

  return UINT64_C(0xdeadc0de);
}

bool zk_account_exists(sail_fixed_bytes_20 za)
{
  bool z8zE231;
  struct zAccount z2zE2489;
  {
    z2zE2489 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2784;  }
  }
  z8zE231 = z2zE2489.zpresent;
end_function_2783: ;
  return z8zE231;
end_block_exception_2784: ;

  return false;
}

bool zk_account_is_empty(sail_fixed_bytes_20 za)
{
  bool z8zE232;
  struct zAccountInfo z2zE2488;
  {
    struct zAccount z2zE2487;
    {
      z2zE2487 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_2782;  }
    }
    z2zE2488 = z2zE2487.zinfo;
  }
  z8zE232 = zaccount_info_empty(z2zE2488);
end_function_2781: ;
  return z8zE232;
end_block_exception_2782: ;

  return false;
}

bool zk_account_occupied(sail_fixed_bytes_20 za)
{
  bool z8zE233;
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2780;  }
  }
  struct zAccountInfo zinfo;
  zinfo = zacc.zinfo;
  bool zanchored_storage;
  {
    bool z2zE2486;
    {
      bool z2zE2484;
      z2zE2484 = zacc.zstorage_cleared;
      z2zE2486 = not(z2zE2484);
    }
    bool z3zE2396;
    if (z2zE2486) {
      sail_fixed_bytes_32 z2zE2485;
      z2zE2485 = zinfo.zstorage_root;
      z3zE2396 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE2485, zEMPTY_TRIE_ROOT);
    } else {  z3zE2396 = false;  }
    zanchored_storage = z3zE2396;
  }
  bool z2zE2483;
  {
    bool z2zE2482;
    {
      sail_fixed_bytes_32 z2zE2479;
      z2zE2479 = zinfo.zcode_hash;
      z2zE2482 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE2479, zKECCAK_EMPTY);
    }
    bool z3zE2398;
    if (z2zE2482) {  z3zE2398 = true;  } else {
      bool z2zE2481;
      {
        uint64_t z2zE2480;
        z2zE2480 = zinfo.znonce;
        z2zE2481 = (z2zE2480 != UINT64_C(0));
      }
      bool z3zE2397;
      if (z2zE2481) {  z3zE2397 = true;  } else {  z3zE2397 = zanchored_storage;  }
      z3zE2398 = z3zE2397;
    }
    z2zE2483 = z3zE2398;
  }
  if (z2zE2483) {  z8zE233 = true;  } else {  z8zE233 = storage_has_writes(za);  }
end_function_2779: ;
  return z8zE233;
end_block_exception_2780: ;

  return false;
}

unit zk_transfer(sail_fixed_bytes_20 zsrc, sail_fixed_bytes_20 zdst, sail_u256 zv)
{
  unit z8zE234;
  struct zAccount zsrc_acc;
  {
    zsrc_acc = zk_aload(zsrc);
    if (have_exception) {  goto end_block_exception_2778;  }
  }
  struct zAccount zdst_acc;
  {
    zdst_acc = zk_aload(zdst);
    if (have_exception) {  goto end_block_exception_2778;  }
  }
  bool z2zE2468;
  {
    bool z2zE2467;
    z2zE2467 = zword_is_zzero(zv);
    bool z3zE2391;
    if (z2zE2467) {  z3zE2391 = true;  } else {  z3zE2391 = eq_fixed_bytes_20(zsrc, zdst);  }
    z2zE2468 = z3zE2391;
  }
  if (z2zE2468) {  z8zE234 = UNIT;  } else {
    struct zAccountInfo z2zE2473;
    {
      struct zAccountInfo z2zE2469;
      z2zE2469 = zsrc_acc.zinfo;
      sail_u256 z2zE2472;
      {
        sail_u256 z2zE2471;
        {
          struct zAccountInfo z2zE2470;
          z2zE2470 = zsrc_acc.zinfo;
          z2zE2471 = z2zE2470.zbalance;
        }
        z2zE2472 = zalu_sub(z2zE2471, zv);
      }
      struct zAccountInfo z3zE2392;
      z3zE2392 = z2zE2469;
      z3zE2392.zbalance = z2zE2472;
      z2zE2473 = z3zE2392;
    }
    unit z3zE2395;
    z3zE2395 = zstore_account_info(zsrc, zsrc_acc, z2zE2473);
    struct zAccountInfo z2zE2478;
    {
      struct zAccountInfo z2zE2474;
      z2zE2474 = zdst_acc.zinfo;
      sail_u256 z2zE2477;
      {
        sail_u256 z2zE2476;
        {
          struct zAccountInfo z2zE2475;
          z2zE2475 = zdst_acc.zinfo;
          z2zE2476 = z2zE2475.zbalance;
        }
        z2zE2477 = zalu_add(z2zE2476, zv);
      }
      struct zAccountInfo z3zE2393;
      z3zE2393 = z2zE2474;
      z3zE2393.zbalance = z2zE2477;
      z2zE2478 = z3zE2393;
    }
    unit z3zE2394;
    z3zE2394 = zstore_account_info(zdst, zdst_acc, z2zE2478);
    z8zE234 = zk_emit_transfer_log(zsrc, zdst, zv);
  }
end_function_2777: ;
  return z8zE234;
end_block_exception_2778: ;

  return UNIT;
}

unit zk_bump_nonce(sail_fixed_bytes_20 za)
{
  unit z8zE235;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2776;  }
  }
  struct zAccountInfo z2zE2466;
  {
    struct zAccountInfo z2zE2462;
    z2zE2462 = zcur.zinfo;
    uint64_t z2zE2465;
    {
      uint64_t z2zE2464;
      {
        struct zAccountInfo z2zE2463;
        z2zE2463 = zcur.zinfo;
        z2zE2464 = z2zE2463.znonce;
      }
      z2zE2465 = zaccount_nonce_increment(z2zE2464);
    }
    struct zAccountInfo z3zE2390;
    z3zE2390 = z2zE2462;
    z3zE2390.znonce = z2zE2465;
    z2zE2466 = z3zE2390;
  }
  z8zE235 = zstore_account_info(za, zcur, z2zE2466);
end_function_2775: ;
  return z8zE235;
end_block_exception_2776: ;

  return UNIT;
}

unit zk_add_balance(sail_fixed_bytes_20 za, sail_u256 zv)
{
  unit z8zE236;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2774;  }
  }
  bool z2zE2456;
  {
    bool z2zE2455;
    z2zE2455 = zword_is_zzero(zv);
    z2zE2456 = not(z2zE2455);
  }
  if (z2zE2456) {
    struct zAccountInfo z2zE2461;
    {
      struct zAccountInfo z2zE2457;
      z2zE2457 = zcur.zinfo;
      sail_u256 z2zE2460;
      {
        sail_u256 z2zE2459;
        {
          struct zAccountInfo z2zE2458;
          z2zE2458 = zcur.zinfo;
          z2zE2459 = z2zE2458.zbalance;
        }
        z2zE2460 = zalu_add(z2zE2459, zv);
      }
      struct zAccountInfo z3zE2389;
      z3zE2389 = z2zE2457;
      z3zE2389.zbalance = z2zE2460;
      z2zE2461 = z3zE2389;
    }
    z8zE236 = zstore_account_info(za, zcur, z2zE2461);
  } else {  z8zE236 = UNIT;  }
end_function_2773: ;
  return z8zE236;
end_block_exception_2774: ;

  return UNIT;
}

unit zk_sub_balance(sail_fixed_bytes_20 za, sail_u256 zv)
{
  unit z8zE237;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2772;  }
  }
  bool z2zE2449;
  {
    bool z2zE2448;
    z2zE2448 = zword_is_zzero(zv);
    z2zE2449 = not(z2zE2448);
  }
  if (z2zE2449) {
    struct zAccountInfo z2zE2454;
    {
      struct zAccountInfo z2zE2450;
      z2zE2450 = zcur.zinfo;
      sail_u256 z2zE2453;
      {
        sail_u256 z2zE2452;
        {
          struct zAccountInfo z2zE2451;
          z2zE2451 = zcur.zinfo;
          z2zE2452 = z2zE2451.zbalance;
        }
        z2zE2453 = zalu_sub(z2zE2452, zv);
      }
      struct zAccountInfo z3zE2388;
      z3zE2388 = z2zE2450;
      z3zE2388.zbalance = z2zE2453;
      z2zE2454 = z3zE2388;
    }
    z8zE237 = zstore_account_info(za, zcur, z2zE2454);
  } else {  z8zE237 = UNIT;  }
end_function_2771: ;
  return z8zE237;
end_block_exception_2772: ;

  return UNIT;
}

unit zk_clear_storage(sail_fixed_bytes_20 za)
{
  unit z8zE238;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2770;  }
  }
  unit z3zE2387;
  z3zE2387 = storage_tx_clear(za);
  struct zAccount z2zE2447;
  z2zE2447 = zaccount_clear_storage(zcur);
  z8zE238 = zstore_account(za, z2zE2447);
end_function_2769: ;
  return z8zE238;
end_block_exception_2770: ;

  return UNIT;
}

