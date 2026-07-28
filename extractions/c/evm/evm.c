#ifndef EVMSAIL_C_UNITY_BUILD
#define EVMSAIL_C_UNITY_BUILD 1
#endif
#include "evm_internal.h"
#include "prelude.c"
#include "primitives/cycle_scopes.c"
#include "host/cycle_scopes_disabled.c"
#include "primitives/quantities.c"
#include "primitives/gas.c"
#include "primitives/bytes.c"
#include "exceptions.c"
#include "evm/halt.c"
#include "primitives/ssz.c"
#include "primitives/code.c"
#include "lib/bytes.c"
#include "host/accelerators.c"
#include "primitives/rlp.c"
#include "primitives/crypto.c"
#include "primitives/fork.c"
#include "primitives/chain_config.c"
#include "primitives/system.c"
#include "primitives/account.c"
#include "primitives/tx.c"
#include "primitives/block.c"
#include "primitives/evm.c"
#include "primitives/stateless_input.c"
#include "host/byte_slice.c"
#include "host/scratch.c"
#include "kernel/scratch.c"
#include "host/code.c"
#include "host/nodes.c"
#include "lib/ssz/ssz.c"
#include "lib/rlp/rlp.c"
#include "lib/address.c"
#include "lib/tx.c"
#include "lib/rlp/tx.c"
#include "host/state.c"
#include "host/environment.c"
#include "host/stack.c"
#include "host/frame_stack.c"
#include "host/memory.c"
#include "host/output.c"
#include "kernel/environment.c"
#include "kernel/storage.c"
#include "kernel/logs.c"
#include "kernel/accounts.c"
#include "kernel/code.c"
#include "kernel/selfdestruct.c"
#include "kernel/lifecycle.c"
#include "evm/machine.c"
#include "evm/gas.c"
#include "evm/precompiles.c"
#include "evm/instructions.c"
#include "evm/execute.c"
#include "evm/interpreter.c"
#include "evm/transaction.c"
#include "lib/mpt/primitives.c"
#include "lib/mpt/nodes.c"
#include "lib/mpt/updates.c"
#include "lib/mpt/indexed.c"
#include "lib/mpt/trie.c"
#include "lib/state_trie.c"
#include "host/debug_disabled.c"
#include "lib/ssz/stateless_input.c"
#include "executor/receipts.c"
#include "executor/system_calls.c"
#include "executor/block_access_list.c"
#include "executor/block.c"
#include "executor/payload.c"
#include "executor/stateless.c"
#include "lib/htr.c"
#include "executor/result.c"
#include "main.c"

unit zinitializze_registers(unit z3zE409)
{
  unit z8zE699;
  z8zE699 = UNIT;
end_function_1111: ;
  return z8zE699;
end_block_exception_1112: ;

  return UNIT;
}

sail_u256 zU256zIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zvalue)
{
  sail_u256 z8zE700;
  z8zE700 = u256_of_u128(zvalue);
end_function_3545: ;
  return z8zE700;
end_block_exception_3546: ;

  return u256_zero();
}

sail_u256 zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  sail_u256 z8zE701;
  z8zE701 = u256_of_fbits(zvalue);
end_function_3545: ;
  return z8zE701;
end_block_exception_3546: ;

  return u256_zero();
}

sail_u256 zalu_addzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, sail_u128 zb)
{
  sail_u256 z8zE702;
  z8zE702 = zword_add_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(za, zb);
end_function_3459: ;
  return z8zE702;
end_block_exception_3460: ;

  return u256_zero();
}

sail_u256 zalu_addzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, uint64_t zb)
{
  sail_u256 z8zE703;
  z8zE703 = zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(za, zb);
end_function_3459: ;
  return z8zE703;
end_block_exception_3460: ;

  return u256_zero();
}

sail_u256 zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  sail_u256 z8zE704;
  bool z2zE2366;
  z2zE2366 = zword_is_zzero(zleft);
  if (z2zE2366) {  z8zE704 = zWORD_ZERO;  } else {
    bool z2zE2368;
    {
      sail_u256 z2zE2367;
      z2zE2367 = zword_div_word(zWORD_ALL_ONES, zleft);
      z2zE2368 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zright, z2zE2367);
    }
    if (z2zE2368) {
      z8zE704 = zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zleft, zright);
    } else {
      struct zexception z2zE2369;
      CREATE(zexception)(&z2zE2369);
      zInvalidBlock(&z2zE2369, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE2369);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/gas.sail:94.12-94.48");
      KILL(zexception)(&z2zE2369);
      goto end_block_exception_2639;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2369);
    }
  }
end_function_2638: ;
  return z8zE704;
end_block_exception_2639: ;

  return u256_zero();
}

uint64_t zcapped_transaction_refundzIreprzGI128zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128 ztotal, uint64_t zcap)
{
  uint64_t z8zE705;
  bool z2zE1225;
  z2zE1225 = (!(UINT64_C(0) < ztotal));
  if (z2zE1225) {  z8zE705 = UINT64_C(0);  } else {
    bool z2zE1226;
    z2zE1226 = (!(UINT64_C(18446744073709551615) < ztotal));
    if (z2zE1226) {
      uint64_t zadmitted_total;
      zadmitted_total = (uint64_t)(ztotal);
      bool z2zE1227;
      {
        __int128 z3zE2983;
        z3zE2983 = (__int128)(zadmitted_total);
        __int128 z3zE2984;
        z3zE2984 = (__int128)(zcap);
        z2zE1227 = (!(z3zE2984 < z3zE2983));
      }
      if (z2zE1227) {  z8zE705 = zadmitted_total;  } else {  z8zE705 = zcap;  }
    } else {  z8zE705 = zcap;  }
  }
end_function_1803: ;
  return z8zE705;
end_block_exception_1804: ;

  return UINT64_C(0xdeadc0de);
}

unit zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE706;
  bool z2zE2320;
  {
    bool z2zE2319;
    {
      bool z2zE2318;
      z2zE2318 = zis_running(UNIT);
      z2zE2319 = not(z2zE2318);
    }
    bool z3zE2297;
    if (z2zE2319) {  z3zE2297 = true;  } else {
      {    z3zE2297 = (zamount == UINT64_C(0));
      }
    }
    z2zE2320 = z3zE2297;
  }
  if (z2zE2320) {  z8zE706 = UNIT;  } else {
    uint64_t zremaining;
    zremaining = zgas_remaining;
    bool z2zE2321;
    {    z2zE2321 = (!(zremaining < zamount));
    }
    if (z2zE2321) {
      {
        __int128 z3zE2977;
        z3zE2977 = (__int128)(zremaining);
        __int128 z3zE2978;
        z3zE2978 = (__int128)(zamount);
        __int128 z3zE2979;
        z3zE2979 = (z3zE2977 - z3zE2978);
        zgas_remaining = (uint64_t)(z3zE2979);
      }
      z8zE706 = UNIT;
    } else {
      {
        z8zE706 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2603;  }
      }
    }
  }
end_function_2602: ;
  return z8zE706;
end_block_exception_2603: ;

  return UNIT;
}

unit zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE707;
  bool z2zE2306;
  {
    bool z2zE2305;
    {
      z2zE2305 = zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(zamount);
      if (have_exception) {  goto end_block_exception_2589;  }
    }
    z2zE2306 = not(z2zE2305);
  }
  if (z2zE2306) {
    {
      z8zE707 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2589;  }
    }
  } else {  z8zE707 = UNIT;  }
end_function_2588: ;
  return z8zE707;
end_block_exception_2589: ;

  return UNIT;
}

struct zMemoryRangeFields zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zstart, uint64_t zsizze)
{
  struct zMemoryRangeFields z8zE708;
  bool z2zE2267;
  z2zE2267 = zis_running(UNIT);
  if (z2zE2267) {
    uint64_t zavailable;
    zavailable = zgas_remaining;
    struct zMemoryExpansion zplan;
    {
      zplan = zmemory_expansionzIreprzGR__sail_c_repr_u256zCU64zCU64zCRMemoryExpansionzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(zstart, zsizze, zavailable);
      if (have_exception) {  goto end_block_exception_2549;  }
    }
    uint64_t z2zE2268;
    z2zE2268 = zplan.zcost;
    unit z3zE2249;
    {
      z3zE2249 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2268);
      if (have_exception) {  goto end_block_exception_2549;  }
    }
    bool z2zE2269;
    z2zE2269 = zis_running(UNIT);
    if (z2zE2269) {  z8zE708 = zapply_memory_expansion(zplan);  } else {  z8zE708 = zEMPTY_MEMORY_RANGE;  }
  } else {  z8zE708 = zEMPTY_MEMORY_RANGE;  }
end_function_2548: ;
  return z8zE708;
end_block_exception_2549: ;
  struct zMemoryRangeFields z8zE1022 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE1022;
}

unit zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE709;
  bool z2zE2309;
  {
    bool z2zE2308;
    z2zE2308 = zis_running(UNIT);
    bool z3zE2290;
    if (z2zE2308) {
      bool z2zE2307;
      {
        z2zE2307 = zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(zamount);
        if (have_exception) {  goto end_block_exception_2591;  }
      }
      z3zE2290 = not(z2zE2307);
    } else {  z3zE2290 = false;  }
    z2zE2309 = z3zE2290;
  }
  if (z2zE2309) {
    {
      z8zE709 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2591;  }
    }
  } else {  z8zE709 = UNIT;  }
end_function_2590: ;
  return z8zE709;
end_block_exception_2591: ;

  return UNIT;
}

unit zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE710;
  uint64_t zremaining;
  zremaining = zgas_remaining;
  bool z2zE2317;
  {
    bool z2zE2316;
    z2zE2316 = zis_running(UNIT);
    bool z3zE2296;
    if (z2zE2316) {
      {    z3zE2296 = (zremaining < zamount);
      }
    } else {  z3zE2296 = false;  }
    z2zE2317 = z3zE2296;
  }
  if (z2zE2317) {
    {
      z8zE710 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2601;  }
    }
  } else {  z8zE710 = UNIT;  }
end_function_2600: ;
  return z8zE710;
end_block_exception_2601: ;

  return UNIT;
}

bool zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(uint64_t zamount)
{
  bool z8zE711;
  bool z2zE2310;
  {    z2zE2310 = (zamount == UINT64_C(0));
  }
  unit z3zE2291;
  if (z2zE2310) {
    z8zE711 = true;
    goto cleanup_2593;
    /* unreachable after return */
    goto end_cleanup_2594;
  cleanup_2593: ;
    goto end_function_2592;
  end_cleanup_2594: ;
  } else {  z3zE2291 = UNIT;  }
  uint64_t zstate_left;
  zstate_left = zstate_gas_remaining;
  uint64_t zexecution_left;
  zexecution_left = zgas_remaining;
  bool z2zE2311;
  {    z2zE2311 = (!(zstate_left < zamount));
  }
  if (z2zE2311) {
    {
      __int128 z3zE2985;
      z3zE2985 = (__int128)(zstate_left);
      __int128 z3zE2986;
      z3zE2986 = (__int128)(zamount);
      __int128 z3zE2987;
      z3zE2987 = (z3zE2985 - z3zE2986);
      zstate_gas_remaining = (uint64_t)(z3zE2987);
    }
    unit z3zE2295;
    z3zE2295 = UNIT;
    z8zE711 = true;
  } else {
    __int128 zremainder;
    {
      __int128 z3zE2988;
      z3zE2988 = (__int128)(zamount);
      __int128 z3zE2989;
      z3zE2989 = (__int128)(zstate_left);
      zremainder = (z3zE2988 - z3zE2989);
    }
    bool z2zE2312;
    z2zE2312 = (!(zexecution_left < zremainder));
    if (z2zE2312) {
      uint64_t zspilled;
      zspilled = zstate_gas_spilled;
      zstate_gas_remaining = zGAS_ZERO;
      unit z3zE2294;
      z3zE2294 = UNIT;
      {
        __int128 z3zE2990;
        z3zE2990 = (__int128)(zexecution_left);
        __int128 z3zE2991;
        z3zE2991 = (z3zE2990 - zremainder);
        zgas_remaining = (uint64_t)(z3zE2991);
      }
      unit z3zE2293;
      z3zE2293 = UNIT;
      {
        zstate_gas_spilled = zstate_gas_spill_addzIreprzGU64zCI128zCU64zKzIboundszG1c0bd00f9b9bd1c00190715feb145f1dzK(zspilled, zremainder);
        if (have_exception) {  goto end_block_exception_2595;  }
      }
      unit z3zE2292;
      z3zE2292 = UNIT;
      z8zE711 = true;
    } else {  z8zE711 = false;  }
  }
end_function_2592: ;
  return z8zE711;
end_block_exception_2595: ;

  return false;
}

bool zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zsizze)
{
  bool z8zE712;
  bool z2zE2325;
  z2zE2325 = zfork_lt(zk_fork, zByzzantium);
  if (z2zE2325) {  z8zE712 = true;  } else {
    bool z2zE2326;
    z2zE2326 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE2326) {
      {    z8zE712 = (!(zEIP170_DEPLOYED_CODE_SIZE_LIMIT < zsizze));
      }
    } else {
      {    z8zE712 = (!(zEIP7954_DEPLOYED_CODE_SIZE_LIMIT < zsizze));
      }
    }
  }
end_function_2608: ;
  return z8zE712;
end_block_exception_2609: ;

  return false;
}

void zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(struct zoptionzIU64zK *z8zE713, uint64_t zcost, uint64_t zavailable)
{
  bool z2zE2197;
  {    z2zE2197 = (!(zavailable < zcost));
  }
  if (z2zE2197) {
    uint64_t zaffordable;
    zaffordable = zcost;
    zSomezIU64zK((*(&z8zE713)), zaffordable);
  } else {  zNonezIU64zK((*(&z8zE713)), UNIT);  }
end_function_2486: ;
  goto end_function_3586;
end_block_exception_2487: ;
  goto end_function_3586;
end_function_3586: ;
}

sail_u256 zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u256zKzIboundszG83613fe416f377787be94d88c23f4a78zK(uint64_t zn, sail_fixed_bytes_20 zv)
{
  sail_u256 z8zE714;
  z8zE714 = u256_from_fixed_bytes_20(zv);
  return z8zE714;
}

sail_u256 zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC32zKzCR__sail_c_repr_u256zKzIboundszG78c09f65c90525db636e344fe928bc0bzK(uint64_t zn, sail_fixed_bytes_32 zv)
{
  sail_u256 z8zE715;
  z8zE715 = u256_from_fixed_bytes_32(zv);
  return z8zE715;
}

bool zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zsizze)
{
  bool z8zE716;
  bool z2zE2323;
  z2zE2323 = zfork_lt(zk_fork, zShanghai);
  if (z2zE2323) {  z8zE716 = true;  } else {
    bool z2zE2324;
    z2zE2324 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE2324) {
      {    z8zE716 = (!(zEIP3860_INITCODE_SIZE_LIMIT < zsizze));
      }
    } else {
      {    z8zE716 = (!(zEIP7954_INITCODE_SIZE_LIMIT < zsizze));
      }
    }
  }
end_function_2606: ;
  return z8zE716;
end_block_exception_2607: ;

  return false;
}

unit zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u128zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_fixed_bytes_20 za, sail_u128 zv)
{
  unit z8zE717;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2774;  }
  }
  bool z2zE2456;
  {
    bool z2zE2455;
    z2zE2455 = zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zv);
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
        z2zE2460 = zalu_addzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2459, zv);
      }
      struct zAccountInfo z3zE2389;
      z3zE2389 = z2zE2457;
      z3zE2389.zbalance = z2zE2460;
      z2zE2461 = z3zE2389;
    }
    z8zE717 = zstore_account_info(za, zcur, z2zE2461);
  } else {  z8zE717 = UNIT;  }
end_function_2773: ;
  return z8zE717;
end_block_exception_2774: ;

  return UNIT;
}

unit zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_fixed_bytes_20 za, uint64_t zv)
{
  unit z8zE718;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_2774;  }
  }
  bool z2zE2456;
  {
    bool z2zE2455;
    z2zE2455 = zword_is_zzerozIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zv);
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
        z2zE2460 = zalu_addzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2459, zv);
      }
      struct zAccountInfo z3zE2389;
      z3zE2389 = z2zE2457;
      z3zE2389.zbalance = z2zE2460;
      z2zE2461 = z3zE2389;
    }
    z8zE718 = zstore_account_info(za, zcur, z2zE2461);
  } else {  z8zE718 = UNIT;  }
end_function_2773: ;
  return z8zE718;
end_block_exception_2774: ;

  return UNIT;
}

struct zMemoryExpansion zmemory_expansionzIreprzGR__sail_c_repr_u256zCU64zCU64zCRMemoryExpansionzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(sail_u256 zstart, uint64_t zsizze, uint64_t zavailable)
{
  struct zMemoryExpansion z8zE719;
  bool z2zE2283;
  z2zE2283 = (zsizze == UINT64_C(0));
  if (z2zE2283) {
    struct zMemoryExpansion z3zE2281;
    z3zE2281.zcost = UINT64_C(0);
    z3zE2281.zrange = zEMPTY_MEMORY_RANGE;
    z3zE2281.zrequired_sizze = UINT64_C(0);
    z8zE719 = z3zE2281;
  } else {
    bool z2zE2285;
    {
      bool z2zE2284;
      z2zE2284 = (!u64_lt_u256(UINT64_C(18446744073709551615), zstart));
      bool z3zE2267;
      if (z2zE2284) {  z3zE2267 = (!(UINT64_C(18446744073709551615) < zsizze));  } else {  z3zE2267 = false;  }
      z2zE2285 = z3zE2267;
    }
    if (z2zE2285) {
      struct zMemoryExpansion z3zE2270;
      {
        uint64_t zbounded_start;
        zbounded_start = u256_to_u64(zstart);
        struct zMemoryExpansion z3zE2271;
        {
          uint64_t zbounded_sizze;
          zbounded_sizze = zsizze;
          sail_u128 zrequired;
          zrequired = u128_add_u64_u64(zbounded_start, zbounded_sizze);
          uint64_t z2zE2286;
          z2zE2286 = zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
          struct zMemoryExpansion z3zE2272;
          {
            uint64_t zwords;
            zwords = z2zE2286;
            struct zMemoryExpansion z3zE2273;
            {
              uint64_t zbounded_words;
              zbounded_words = zwords;
              sail_u128 zexpansion_cost;
              zexpansion_cost = zbounded_mem_cost(zbounded_words);
              bool z2zE2287;
              z2zE2287 = (!u64_lt_u128(UINT64_C(18446744073709551615), zexpansion_cost));
              if (z2zE2287) {
                uint64_t zrequired_sizze;
                zrequired_sizze = u128_to_u64(zrequired);
                struct zMemoryRangeFields z2zE2288;
                z2zE2288 = zmemory_range(zbounded_start, zbounded_sizze);
                uint64_t z2zE2289;
                {
                  z2zE2289 = zmemory_expansion_cost(zrequired_sizze, zavailable);
                  if (have_exception) {  goto end_block_exception_2571;  }
                }
                struct zMemoryExpansion z3zE2277;
                z3zE2277.zcost = z2zE2289;
                z3zE2277.zrange = z2zE2288;
                z3zE2277.zrequired_sizze = zrequired_sizze;
                z3zE2273 = z3zE2277;
              } else {
                unit z3zE2275;
                {
                  z3zE2275 = zexc_halt(zOutOfGas);
                  if (have_exception) {  goto end_block_exception_2571;  }
                }
                struct zMemoryExpansion z3zE2276;
                z3zE2276.zcost = UINT64_C(0);
                z3zE2276.zrange = zEMPTY_MEMORY_RANGE;
                z3zE2276.zrequired_sizze = UINT64_C(0);
                z3zE2273 = z3zE2276;
              }
              goto finish_match_2568;
            }
          case_2569: ;
            sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
          finish_match_2568: ;
            z3zE2272 = z3zE2273;
            goto finish_match_2566;
          }
        case_2567: ;
          sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
        finish_match_2566: ;
          z3zE2271 = z3zE2272;
          goto finish_match_2564;
        }
      case_2565: ;
        sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
      finish_match_2564: ;
        z3zE2270 = z3zE2271;
        goto finish_match_2562;
      }
    case_2563: ;
      sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
    finish_match_2562: ;
      z8zE719 = z3zE2270;
    } else {
      unit z3zE2268;
      {
        z3zE2268 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2571;  }
      }
      struct zMemoryExpansion z3zE2269;
      z3zE2269.zcost = UINT64_C(0);
      z3zE2269.zrange = zEMPTY_MEMORY_RANGE;
      z3zE2269.zrequired_sizze = UINT64_C(0);
      z8zE719 = z3zE2269;
    }
  }
end_function_2570: ;
  return z8zE719;
end_block_exception_2571: ;
  struct zMemoryRangeFields z8zE1024 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryExpansion z8zE1023 = { .zcost = UINT64_C(0xdeadc0de), .zrange = z8zE1024, .zrequired_sizze = UINT64_C(0xdeadc0de) };
  return z8zE1023;
}

uint64_t zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zbyte_len)
{
  uint64_t z8zE720;
  sail_u256 z2zE2300;
  {
    sail_u256 z3zE2975;
    z3zE2975 = u256_of_u128(zbyte_len);
    z2zE2300 = u256_add_u64(z3zE2975, UINT64_C(31));
  }
  {
    sail_u256 z3zE2976;
    z3zE2976 = u256_div_u64(z2zE2300, UINT64_C(32));
    z8zE720 = u256_to_u64(z3zE2976);
  }
end_function_2578: ;
  return z8zE720;
end_block_exception_2579: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zmemory_word_countzIreprzGU64zCU64zKzIboundszG07d9444e226eec3cde1a1e781c91ddf4zK(uint64_t zbyte_len)
{
  uint64_t z8zE721;
  uint64_t z2zE2300;
  {    z2zE2300 = (UINT64_C(31) + zbyte_len);
  }
  {    z8zE721 = (z2zE2300 / UINT64_C(32));
  }
end_function_2578: ;
  return z8zE721;
end_block_exception_2579: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zbyte_len)
{
  uint64_t z8zE722;
  sail_u128 z2zE2300;
  {
    sail_u128 z3zE2973;
    z3zE2973 = u128_of_u64(zbyte_len);
    z2zE2300 = u128_add_u64(z3zE2973, UINT64_C(31));
  }
  {
    sail_u128 z3zE2974;
    z3zE2974 = u128_div_u64(z2zE2300, UINT64_C(32));
    z8zE722 = u128_to_u64(z3zE2974);
  }
end_function_2578: ;
  return z8zE722;
end_block_exception_2579: ;

  return UINT64_C(0xdeadc0de);
}

void zminimal_word_byteszIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE723, uint64_t zw)
{
  sail_u256 zremaining;
  zremaining = u256_of_fbits(zw);
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  uint64_t zlen;
  zlen = UINT64_C(0);
  int64_t z3zE2775;
  {    z3zE2775 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2776;
  {    z3zE2776 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2777;
  {    z3zE2777 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE2775;
    unit z3zE2778;
  for_start_3203: ;
    {
      if ((z3zE2776 < zbyte_index)) goto for_end_3204;
      bool z2zE3036;
      z2zE3036 = zword_nonzzero(zremaining);
      if (z2zE3036) {
        uint64_t z2zE3037;
        z2zE3037 = zword_low_byte(zremaining);
        zconsz3z5bv8(&zout, z2zE3037, zout);
        unit z3zE2780;
        z3zE2780 = UNIT;
        zremaining = zword_shift_right(zremaining, UINT64_C(8));
        unit z3zE2779;
        z3zE2779 = UNIT;
        {
          uint64_t z3zE3051;
          z3zE3051 = (uint64_t)(zbyte_index);
          zlen = zword_byte_count(z3zE3051);
        }
        z3zE2778 = UNIT;
      } else {  z3zE2778 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE2777);
      goto for_start_3203;
    }
  for_end_3204: ;
  }
  unit z3zE2781;
  z3zE2781 = UNIT;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2782;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2782);
  COPY(zz5listz8z5bv8z9)(&((&z3zE2782)->ztup0), zout);
  z3zE2782.ztup1 = zlen;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE723)), z3zE2782);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2782);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3205: ;
  goto end_function_3585;
end_block_exception_3206: ;
  goto end_function_3585;
end_function_3585: ;
}

unit zrecord_refundzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zdelta)
{
  unit z8zE724;
  {
    zframe_refund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zframe_refund, zdelta);
    if (have_exception) {  goto end_block_exception_2722;  }
  }
  z8zE724 = UNIT;
end_function_2721: ;
  return z8zE724;
end_block_exception_2722: ;

  return UNIT;
}

uint64_t zrlp_bytes_sizzezIreprzGLB8zCU64zCU64zKzIboundszGed754718030b429454cf327fc49bd29dzK(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  uint64_t z8zE725;
  uint64_t zfirst;
  {
    uint64_t z3zE2757;
    {
      if ((zdata == NULL)) goto case_3182;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE2757 = zb;
      goto finish_match_3180;
    }
  case_3182: ;
    {
      /* complete */
      z3zE2757 = UINT64_C(0x00);
      goto finish_match_3180;
    }
  case_3181: ;
  finish_match_3180: ;
    zfirst = z3zE2757;
  }
  z8zE725 = zrlp_string_sizzezIreprzGU64zCB8zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(zlen, zfirst);
end_function_3183: ;
  return z8zE725;
end_block_exception_3184: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE726;
  uint64_t z3zE2638;
  z3zE2638 = (zvalue + ((__int128)((((unsigned __int128)UINT64_C(0)) << 64) | UINT64_C(1))));
  z8zE726 = z3zE2638;
end_function_3058: ;
  return z8zE726;
end_block_exception_3059: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zlen)
{
  uint64_t z8zE727;
  bool z2zE3010;
  {    z2zE3010 = (!(zRLP_SHORT_LENGTH_LIMIT < zlen));
  }
  if (z2zE3010) {  z8zE727 = UINT64_C(1);  } else {
    uint64_t z2zE3012;
    {
      sail_u256 z2zE3011;
      z2zE3011 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
      z2zE3012 = zrlp_minimal_word_len(z2zE3011);
    }
    {    z8zE727 = (UINT64_C(1) + z2zE3012);
    }
  }
end_function_3187: ;
  return z8zE727;
end_block_exception_3188: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  sail_u256 z8zE728;
  z8zE728 = zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
end_function_3199: ;
  return z8zE728;
end_block_exception_3200: ;

  return u256_zero();
}

void zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE729, uint64_t zlen)
{
  bool z2zE3013;
  {    z2zE3013 = (!(zRLP_SHORT_LENGTH_LIMIT < zlen));
  }
  if (z2zE3013) {
    zz5listz8z5bv8z9 z2zE3016;
    CREATE(zz5listz8z5bv8z9)(&z2zE3016);
    {
      uint64_t z2zE3015;
      {
        uint64_t z2zE3014;
        z2zE3014 = zrlp_nat_length_byte(zlen);
        z2zE3015 = ((UINT64_C(0xC0) + z2zE3014) & UINT64_C(0xFF));
      }
      zconsz3z5bv8(&z2zE3016, z2zE3015, z2zE3016);
    }
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2765;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2765);
    COPY(zz5listz8z5bv8z9)(&((&z3zE2765)->ztup0), z2zE3016);
    z3zE2765.ztup1 = UINT64_C(1);
    COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE729)), z3zE2765);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2765);
    KILL(zz5listz8z5bv8z9)(&z2zE3016);
  } else {
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3018;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3018);
    {
      sail_u256 z2zE3017;
      z2zE3017 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
      zminimal_word_bytes(&z2zE3018, z2zE3017);
    }
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2761;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2761);
    {
      zz5listz8z5bv8z9 zlength_bytes;
      CREATE(zz5listz8z5bv8z9)(&zlength_bytes);
      COPY(zz5listz8z5bv8z9)(&zlength_bytes, z2zE3018.ztup0);
      uint64_t zlength_len;
      zlength_len = z2zE3018.ztup1;
      zz5listz8z5bv8z9 z2zE3021;
      CREATE(zz5listz8z5bv8z9)(&z2zE3021);
      {
        uint64_t z2zE3020;
        {
          uint64_t z2zE3019;
          z2zE3019 = zrlp_nat_length_byte(zlength_len);
          z2zE3020 = ((UINT64_C(0xF7) + z2zE3019) & UINT64_C(0xFF));
        }
        zconsz3z5bv8(&z2zE3021, z2zE3020, zlength_bytes);
      }
      uint64_t z2zE3022;
      {    z2zE3022 = (UINT64_C(1) + zlength_len);
      }
      COPY(zz5listz8z5bv8z9)(&((&z3zE2761)->ztup0), z2zE3021);
      z3zE2761.ztup1 = z2zE3022;
      KILL(zz5listz8z5bv8z9)(&z2zE3021);
      KILL(zz5listz8z5bv8z9)(&zlength_bytes);
      goto finish_match_3191;
    }
  case_3192: ;
    sail_match_failure("rlp_list_prefix<repr:U64,(LB8,U64)><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
  finish_match_3191: ;
    COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE729)), z3zE2761);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2761);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3018);
  }
end_function_3193: ;
  goto end_function_3584;
end_block_exception_3194: ;
  goto end_function_3584;
end_function_3584: ;
}

uint64_t zrlp_minimal_word_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  uint64_t z8zE730;
  z8zE730 = zword_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zw);
end_function_3189: ;
  return z8zE730;
end_block_exception_3190: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE731;
  bool z2zE3039;
  {
    uint64_t z2zE3038;
    {    z2zE3038 = (UINT64_C(18446744073709551615) - zleft);
    }
    {    z2zE3039 = (!(z2zE3038 < zright));
    }
  }
  if (z2zE3039) {
    {
      sail_u128 z3zE2969;
      z3zE2969 = u128_of_u64(zleft);
      sail_u128 z3zE2970;
      z3zE2970 = u128_add_u64(z3zE2969, zright);
      z8zE731 = u128_to_u64(z3zE2970);
    }
  } else {
    struct zexception z2zE3040;
    CREATE(zexception)(&z2zE3040);
    zInvalidBlock(&z2zE3040, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3040);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:173.8-173.37");
    KILL(zexception)(&z2zE3040);
    goto end_block_exception_3210;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3040);
  }
end_function_3209: ;
  return z8zE731;
end_block_exception_3210: ;

  return UINT64_C(0xdeadc0de);
}

void zrlp_string_prefixzIreprzGU64zCB8zCz8LB8zCU64z9zKzIboundszG7d1a1d23d11135c1c1bff78d34e8a73dzK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE732, uint64_t zlen, uint64_t zfirst)
{
  bool z2zE3025;
  {
    bool z2zE3024;
    {    z2zE3024 = (zlen == UINT64_C(1));
    }
    bool z3zE2766;
    if (z2zE3024) {
      uint64_t z2zE3023;
      z2zE3023 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE2766 = (z2zE3023 == UINT64_C(0b0));
    } else {  z3zE2766 = false;  }
    z2zE3025 = z3zE2766;
  }
  if (z2zE3025) {
    zz5listz8z5bv8z9 z3zE2772;
    CREATE(zz5listz8z5bv8z9)(&z3zE2772);
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2773;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2773);
    COPY(zz5listz8z5bv8z9)(&((&z3zE2773)->ztup0), z3zE2772);
    z3zE2773.ztup1 = UINT64_C(0);
    COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE732)), z3zE2773);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2773);
    KILL(zz5listz8z5bv8z9)(&z3zE2772);
  } else {
    bool z2zE3026;
    {    z2zE3026 = (!(zRLP_SHORT_LENGTH_LIMIT < zlen));
    }
    if (z2zE3026) {
      zz5listz8z5bv8z9 z2zE3029;
      CREATE(zz5listz8z5bv8z9)(&z2zE3029);
      {
        uint64_t z2zE3028;
        {
          uint64_t z2zE3027;
          z2zE3027 = zrlp_nat_length_byte(zlen);
          z2zE3028 = ((UINT64_C(0x80) + z2zE3027) & UINT64_C(0xFF));
        }
        zconsz3z5bv8(&z2zE3029, z2zE3028, z2zE3029);
      }
      struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2771;
      CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2771);
      COPY(zz5listz8z5bv8z9)(&((&z3zE2771)->ztup0), z2zE3029);
      z3zE2771.ztup1 = UINT64_C(1);
      COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE732)), z3zE2771);
      KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2771);
      KILL(zz5listz8z5bv8z9)(&z2zE3029);
    } else {
      struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3031;
      CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3031);
      {
        sail_u256 z2zE3030;
        z2zE3030 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
        zminimal_word_bytes(&z2zE3031, z2zE3030);
      }
      struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2767;
      CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2767);
      {
        zz5listz8z5bv8z9 zlength_bytes;
        CREATE(zz5listz8z5bv8z9)(&zlength_bytes);
        COPY(zz5listz8z5bv8z9)(&zlength_bytes, z2zE3031.ztup0);
        uint64_t zlength_len;
        zlength_len = z2zE3031.ztup1;
        zz5listz8z5bv8z9 z2zE3034;
        CREATE(zz5listz8z5bv8z9)(&z2zE3034);
        {
          uint64_t z2zE3033;
          {
            uint64_t z2zE3032;
            z2zE3032 = zrlp_nat_length_byte(zlength_len);
            z2zE3033 = ((UINT64_C(0xB7) + z2zE3032) & UINT64_C(0xFF));
          }
          zconsz3z5bv8(&z2zE3034, z2zE3033, zlength_bytes);
        }
        uint64_t z2zE3035;
        {    z2zE3035 = (UINT64_C(1) + zlength_len);
        }
        COPY(zz5listz8z5bv8z9)(&((&z3zE2767)->ztup0), z2zE3034);
        z3zE2767.ztup1 = z2zE3035;
        KILL(zz5listz8z5bv8z9)(&z2zE3034);
        KILL(zz5listz8z5bv8z9)(&zlength_bytes);
        goto finish_match_3195;
      }
    case_3196: ;
      sail_match_failure("rlp_string_prefix<repr:U64,B8,(LB8,U64)><bounds:7d1a1d23d11135c1c1bff78d34e8a73d>");
    finish_match_3195: ;
      COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE732)), z3zE2767);
      KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2767);
      KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3031);
    }
  }
end_function_3197: ;
  goto end_function_3583;
end_block_exception_3198: ;
  goto end_function_3583;
end_function_3583: ;
}

uint64_t zrlp_string_sizzezIreprzGU64zCB8zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(uint64_t zlen, uint64_t zfirst)
{
  uint64_t z8zE733;
  bool z2zE3008;
  {
    bool z2zE3007;
    {    z2zE3007 = (zlen == UINT64_C(1));
    }
    bool z3zE2760;
    if (z2zE3007) {
      uint64_t z2zE3006;
      z2zE3006 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE2760 = (z2zE3006 == UINT64_C(0b0));
    } else {  z3zE2760 = false;  }
    z2zE3008 = z3zE2760;
  }
  if (z2zE3008) {  z8zE733 = UINT64_C(1);  } else {
    uint64_t z2zE3009;
    z2zE3009 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
    {    z8zE733 = (zlen + z2zE3009);
    }
  }
end_function_3185: ;
  return z8zE733;
end_block_exception_3186: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  uint64_t z8zE734;
  uint64_t zlen;
  zlen = zrlp_minimal_word_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zw);
  uint64_t zfirst;
  {
    bool z2zE3005;
    z2zE3005 = (zlen == UINT64_C(1));
    if (z2zE3005) {  zfirst = zword_low_bytezIreprzGU64zCB8zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zw);  } else {
      zfirst = UINT64_C(0x00);
    }
  }
  bool z2zE3004;
  {
    bool z2zE3003;
    z2zE3003 = (zlen == UINT64_C(1));
    bool z3zE2756;
    if (z2zE3003) {
      uint64_t z2zE3002;
      z2zE3002 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE2756 = (z2zE3002 == UINT64_C(0b0));
    } else {  z3zE2756 = false;  }
    z2zE3004 = z3zE2756;
  }
  if (z2zE3004) {  z8zE734 = UINT64_C(1);  } else {
    {    z8zE734 = (UINT64_C(1) + zlen);
    }
  }
end_function_3178: ;
  return z8zE734;
end_block_exception_3179: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zcontent_len)
{
  unit z8zE735;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2986;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2986);
  zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE2986, zcontent_len);
  unit z3zE2747;
  {
    zz5listz8z5bv8z9 zencoded_prefix;
    CREATE(zz5listz8z5bv8z9)(&zencoded_prefix);
    COPY(zz5listz8z5bv8z9)(&zencoded_prefix, z2zE2986.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE2986.ztup1;
    z3zE2747 = zscratch_push_bytes(zencoded_prefix, zprefix_len);
    KILL(zz5listz8z5bv8z9)(&zencoded_prefix);
    goto finish_match_3159;
  }
case_3160: ;
  sail_match_failure("rlp_write_list_prefix<repr:U64,u><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
finish_match_3159: ;
  z8zE735 = z3zE2747;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2986);
end_function_3161: ;
  return z8zE735;
end_block_exception_3162: ;

  return UNIT;
}

unit zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zlen, uint64_t zfirst)
{
  unit z8zE736;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2987;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2987);
  zrlp_string_prefixzIreprzGU64zCB8zCz8LB8zCU64z9zKzIboundszG7d1a1d23d11135c1c1bff78d34e8a73dzK(&z2zE2987, zlen, zfirst);
  unit z3zE2749;
  {
    zz5listz8z5bv8z9 zencoded_prefix;
    CREATE(zz5listz8z5bv8z9)(&zencoded_prefix);
    COPY(zz5listz8z5bv8z9)(&zencoded_prefix, z2zE2987.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE2987.ztup1;
    z3zE2749 = zscratch_push_bytes(zencoded_prefix, zprefix_len);
    KILL(zz5listz8z5bv8z9)(&zencoded_prefix);
    goto finish_match_3163;
  }
case_3164: ;
  sail_match_failure("rlp_write_string_prefix<repr:U64,B8,u><bounds:491a746de554142e7d65e0bb42a9e751>");
finish_match_3163: ;
  z8zE736 = z3zE2749;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2987);
end_function_3165: ;
  return z8zE736;
end_block_exception_3166: ;

  return UNIT;
}

unit zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  unit z8zE737;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2981;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2981);
  zminimal_word_byteszIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE2981, zw);
  unit z3zE2740;
  {
    zz5listz8z5bv8z9 zbytes;
    CREATE(zz5listz8z5bv8z9)(&zbytes);
    COPY(zz5listz8z5bv8z9)(&zbytes, z2zE2981.ztup0);
    uint64_t zlen;
    zlen = z2zE2981.ztup1;
    z3zE2740 = zrlp_write_bytes(zbytes, zlen);
    KILL(zz5listz8z5bv8z9)(&zbytes);
    goto finish_match_3148;
  }
case_3149: ;
  sail_match_failure("rlp_write_uint_word<repr:U64,u><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
finish_match_3148: ;
  z8zE737 = z3zE2740;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2981);
end_function_3150: ;
  return z8zE737;
end_block_exception_3151: ;

  return UNIT;
}

struct zByteSliceFields zsszz_container_byteszIreprzGRByteSliceFieldszCU64zCRByteSliceFieldszKzIboundszG718308cac13a033149f52c385b3cc002zK(struct zByteSliceFields zbytes, uint64_t zminimum)
{
  struct zByteSliceFields z8zE738;
  struct zByteSliceFields zfields;
  zfields = zbytes;
  bool z2zE689;
  {
    uint64_t z2zE688;
    z2zE688 = zfields.zlen;
    {    z2zE689 = (!(z2zE688 < zminimum));
    }
  }
  if (z2zE689) {  z8zE738 = zfields;  } else {
    struct zexception z2zE690;
    CREATE(zexception)(&z2zE690);
    zInvalidBlock(&z2zE690, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE690);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:301.8-301.41");
    KILL(zexception)(&z2zE690);
    goto end_block_exception_1438;
    /* unreachable after throw */
    KILL(zexception)(&z2zE690);
  }
end_function_1437: ;
  return z8zE738;
end_block_exception_1438: ;
  struct zByteSliceFields z8zE1025 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1025;
}

struct zByteSliceFields zsszz_container_byteszIreprzGRByteSliceFieldszCU64zCRByteSliceFieldszKzIboundszGd430ac7621e2b62bc50ae9cc272dba03zK(struct zByteSliceFields zbytes, uint64_t zminimum)
{
  struct zByteSliceFields z8zE739;
  struct zByteSliceFields zfields;
  zfields = zbytes;
  bool z2zE689;
  {
    uint64_t z2zE688;
    z2zE688 = zfields.zlen;
    {    z2zE689 = (!(z2zE688 < zminimum));
    }
  }
  if (z2zE689) {  z8zE739 = zfields;  } else {
    struct zexception z2zE690;
    CREATE(zexception)(&z2zE690);
    zInvalidBlock(&z2zE690, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE690);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:301.8-301.41");
    KILL(zexception)(&z2zE690);
    goto end_block_exception_1438;
    /* unreachable after throw */
    KILL(zexception)(&z2zE690);
  }
end_function_1437: ;
  return z8zE739;
end_block_exception_1438: ;
  struct zByteSliceFields z8zE1026 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1026;
}

uint64_t zstate_gas_spill_addzIreprzGU64zCI128zCU64zKzIboundszG1c0bd00f9b9bd1c00190715feb145f1dzK(uint64_t zleft, __int128 zright)
{
  uint64_t z8zE740;
  bool z2zE2314;
  {
    uint64_t z2zE2313;
    z2zE2313 = zstate_gas_spill_room(zleft);
    z2zE2314 = (!(z2zE2313 < zright));
  }
  if (z2zE2314) {
    {
      __int128 z3zE2992;
      z3zE2992 = (__int128)(zleft);
      __int128 z3zE2993;
      z3zE2993 = (zright + z3zE2992);
      z8zE740 = (uint64_t)(z3zE2993);
    }
  } else {
    struct zexception z2zE2315;
    CREATE(zexception)(&z2zE2315);
    zInvalidBlock(&z2zE2315, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2315);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:454.8-454.44");
    KILL(zexception)(&z2zE2315);
    goto end_block_exception_2597;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2315);
  }
end_function_2596: ;
  return z8zE740;
end_block_exception_2597: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zstate_gas_spill_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE741;
  bool z2zE2314;
  {
    uint64_t z2zE2313;
    z2zE2313 = zstate_gas_spill_room(zleft);
    {    z2zE2314 = (!(z2zE2313 < zright));
    }
  }
  if (z2zE2314) {
    {
      sail_u128 z3zE2971;
      z3zE2971 = u128_of_u64(zleft);
      sail_u128 z3zE2972;
      z3zE2972 = u128_add_u64(z3zE2971, zright);
      z8zE741 = u128_to_u64(z3zE2972);
    }
  } else {
    struct zexception z2zE2315;
    CREATE(zexception)(&z2zE2315);
    zInvalidBlock(&z2zE2315, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2315);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:454.8-454.44");
    KILL(zexception)(&z2zE2315);
    goto end_block_exception_2597;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2315);
  }
end_function_2596: ;
  return z8zE741;
end_block_exception_2597: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC20zKzKzIboundszG83613fe416f377787be94d88c23f4a78zK(uint64_t zn, sail_u256 zb)
{
  sail_fixed_bytes_20 z8zE742;
  z8zE742 = fixed_bytes_20_from_u256(zb);
  return z8zE742;
}

sail_fixed_bytes_32 zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC32zKzKzIboundszG78c09f65c90525db636e344fe928bc0bzK(uint64_t zn, sail_u256 zb)
{
  sail_fixed_bytes_32 z8zE743;
  z8zE743 = fixed_bytes_32_from_u256(zb);
  return z8zE743;
}

uint64_t zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE744;
  bool z2zE1555;
  {    z2zE1555 = (!(zleft < zright));
  }
  if (z2zE1555) {
    {
      __int128 z3zE2980;
      z3zE2980 = (__int128)(zleft);
      __int128 z3zE2981;
      z3zE2981 = (__int128)(zright);
      __int128 z3zE2982;
      z3zE2982 = (z3zE2980 - z3zE2981);
      z8zE744 = (uint64_t)(z3zE2982);
    }
  } else {
    struct zexception z2zE1556;
    CREATE(zexception)(&z2zE1556);
    zInvalidBlock(&z2zE1556, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1556);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:316.8-316.44");
    KILL(zexception)(&z2zE1556);
    goto end_block_exception_1917;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1556);
  }
end_function_1916: ;
  return z8zE744;
end_block_exception_1917: ;

  return UINT64_C(0xdeadc0de);
}

__int128 zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128 zleft, int64_t zright)
{
  __int128 z8zE745;
  __int128 ztotal;
  {    ztotal = (zleft + zright);
  }
  bool z2zE2411;
  {
    bool z2zE2410;
    z2zE2410 = (!(ztotal < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1)));
    bool z3zE2362;
    if (z2zE2410) {
      z3zE2362 = (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < ztotal));
    } else {  z3zE2362 = false;  }
    z2zE2411 = z3zE2362;
  }
  if (z2zE2411) {  z8zE745 = ztotal;  } else {
    struct zexception z2zE2412;
    CREATE(zexception)(&z2zE2412);
    zInvalidBlock(&z2zE2412, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2412);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/machine.sail:58.8-58.44");
    KILL(zexception)(&z2zE2412);
    goto end_block_exception_2724;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2412);
  }
end_function_2723: ;
  return z8zE745;
end_block_exception_2724: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

__int128 zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128 zleft, uint64_t zright)
{
  __int128 z8zE746;
  __int128 ztotal;
  {    ztotal = (zleft + zright);
  }
  bool z2zE2411;
  {
    bool z2zE2410;
    z2zE2410 = (!(ztotal < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1)));
    bool z3zE2362;
    if (z2zE2410) {
      z3zE2362 = (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < ztotal));
    } else {  z3zE2362 = false;  }
    z2zE2411 = z3zE2362;
  }
  if (z2zE2411) {  z8zE746 = ztotal;  } else {
    struct zexception z2zE2412;
    CREATE(zexception)(&z2zE2412);
    zInvalidBlock(&z2zE2412, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2412);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/machine.sail:58.8-58.44");
    KILL(zexception)(&z2zE2412);
    goto end_block_exception_2724;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2412);
  }
end_function_2723: ;
  return z8zE746;
end_block_exception_2724: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

sail_u256 zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zvalue, uint64_t zfactor)
{
  sail_u256 z8zE747;
  {
    z8zE747 = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue, zfactor);
    if (have_exception) {  goto end_block_exception_1921;  }
  }
end_function_1920: ;
  return z8zE747;
end_block_exception_1921: ;

  return u256_zero();
}

sail_u256 zword_add_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, sail_u128 zright)
{
  sail_u256 z8zE748;
  sail_u256 z2zE3328;
  {    z2zE3328 = u256_add_u128(zleft, zright);
  }
  z8zE748 = zU256(z2zE3328);
end_function_3523: ;
  return z8zE748;
end_block_exception_3524: ;

  return u256_zero();
}

sail_u256 zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  sail_u256 z8zE749;
  sail_u256 z2zE3328;
  {    z2zE3328 = u256_add_u64(zleft, zright);
  }
  z8zE749 = zU256(z2zE3328);
end_function_3523: ;
  return z8zE749;
end_block_exception_3524: ;

  return u256_zero();
}

uint64_t zword_bit_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE750;
  uint64_t zlimb3;
  {
    uint64_t z2zE3289;
    z2zE3289 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(192)));
    zlimb3 = ((uint64_t) z2zE3289);
  }
  bool z2zE3279;
  z2zE3279 = (zlimb3 != UINT64_C(0));
  if (z2zE3279) {
    uint64_t z2zE3280;
    z2zE3280 = zu64_bit_length(zlimb3);
    {    z8zE750 = (UINT64_C(192) + z2zE3280);
    }
  } else {
    uint64_t zlimb2;
    {
      uint64_t z2zE3288;
      z2zE3288 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(128)));
      zlimb2 = ((uint64_t) z2zE3288);
    }
    bool z2zE3281;
    z2zE3281 = (zlimb2 != UINT64_C(0));
    if (z2zE3281) {
      uint64_t z2zE3282;
      z2zE3282 = zu64_bit_length(zlimb2);
      {    z8zE750 = (UINT64_C(128) + z2zE3282);
      }
    } else {
      uint64_t zlimb1;
      {
        uint64_t z2zE3287;
        z2zE3287 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(64)));
        zlimb1 = ((uint64_t) z2zE3287);
      }
      bool z2zE3283;
      z2zE3283 = (zlimb1 != UINT64_C(0));
      if (z2zE3283) {
        uint64_t z2zE3284;
        z2zE3284 = zu64_bit_length(zlimb1);
        {    z8zE750 = (UINT64_C(64) + z2zE3284);
        }
      } else {
        uint64_t z2zE3286;
        {
          uint64_t z2zE3285;
          z2zE3285 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(0)));
          z2zE3286 = ((uint64_t) z2zE3285);
        }
        z8zE750 = zu64_bit_length(z2zE3286);
      }
    }
  }
end_function_3487: ;
  return z8zE750;
end_block_exception_3488: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE751;
  uint64_t zbit_length;
  zbit_length = zword_bit_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  bool z2zE3267;
  z2zE3267 = (zbit_length == UINT64_C(0));
  if (z2zE3267) {  z8zE751 = UINT64_C(0);  } else {
    uint64_t z2zE3268;
    {    z2zE3268 = (zbit_length + UINT64_C(7));
    }
    {    z8zE751 = (z2zE3268 / UINT64_C(8));
    }
  }
end_function_3473: ;
  return z8zE751;
end_block_exception_3474: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u128zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u128 zdividend, uint64_t zdivisor)
{
  sail_u256 z8zE752;
  bool z2zE3276;
  z2zE3276 = (zdivisor == UINT64_C(0));
  if (z2zE3276) {  z8zE752 = zWORD_ZERO;  } else {
    sail_u256 z2zE3277;
    z2zE3277 = u256_of_u128(u128_div_u64(zdividend, zdivisor));
    z8zE752 = zU256(z2zE3277);
  }
end_function_3483: ;
  return z8zE752;
end_block_exception_3484: ;

  return u256_zero();
}

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, sail_u128 zdivisor)
{
  sail_u256 z8zE753;
  bool z2zE3276;
  z2zE3276 = u128_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE3276) {  z8zE753 = zWORD_ZERO;  } else {
    sail_u256 z2zE3277;
    z2zE3277 = u256_div_u128(zdividend, zdivisor);
    z8zE753 = zU256(z2zE3277);
  }
end_function_3483: ;
  return z8zE753;
end_block_exception_3484: ;

  return u256_zero();
}

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, uint64_t zdivisor)
{
  sail_u256 z8zE754;
  bool z2zE3276;
  z2zE3276 = (zdivisor == UINT64_C(0));
  if (z2zE3276) {  z8zE754 = zWORD_ZERO;  } else {
    sail_u256 z2zE3277;
    z2zE3277 = u256_div_u64(zdividend, zdivisor);
    z8zE754 = zU256(z2zE3277);
  }
end_function_3483: ;
  return z8zE754;
end_block_exception_3484: ;

  return u256_zero();
}

bool zword_greater_than_wordzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  bool z8zE755;
  z8zE755 = u64_lt_u256(zright, zleft);
end_function_3479: ;
  return z8zE755;
end_block_exception_3480: ;

  return false;
}

bool zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zw)
{
  bool z8zE756;
  z8zE756 = u256_eq_u128(zWORD_ZERO, zw);
end_function_3503: ;
  return z8zE756;
end_block_exception_3504: ;

  return false;
}

bool zword_is_zzerozIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  bool z8zE757;
  z8zE757 = u256_eq_u64(zWORD_ZERO, zw);
end_function_3503: ;
  return z8zE757;
end_block_exception_3504: ;

  return false;
}

uint64_t zword_low_bytezIreprzGU64zCB8zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE758;
  z8zE758 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
end_function_3509: ;
  return z8zE758;
end_block_exception_3510: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, sail_u128 zdivisor)
{
  sail_u256 z8zE759;
  bool z2zE3274;
  z2zE3274 = u128_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE3274) {  z8zE759 = zWORD_ZERO;  } else {
    sail_u256 z2zE3275;
    z2zE3275 = u256_mod_u128(zdividend, zdivisor);
    z8zE759 = zU256(z2zE3275);
  }
end_function_3481: ;
  return z8zE759;
end_block_exception_3482: ;

  return u256_zero();
}

sail_u256 zword_mod_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, uint64_t zdivisor)
{
  sail_u256 z8zE760;
  bool z2zE3274;
  z2zE3274 = (zdivisor == UINT64_C(0));
  if (z2zE3274) {  z8zE760 = zWORD_ZERO;  } else {
    sail_u256 z2zE3275;
    z2zE3275 = u256_mod_u64(zdividend, zdivisor);
    z8zE760 = zU256(z2zE3275);
  }
end_function_3481: ;
  return z8zE760;
end_block_exception_3482: ;

  return u256_zero();
}

sail_u256 zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, uint64_t zb)
{
  sail_u256 z8zE761;
  z8zE761 = u256_mul_u64(za, zb);
end_function_3485: ;
  return z8zE761;
end_block_exception_3486: ;

  return u256_zero();
}

sail_u256 zword_of_nat_byte_countzIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zvalue)
{
  sail_u256 z8zE762;
  bool z2zE3204;
  {    z2zE3204 = true;
  }
  if (z2zE3204) {
    z8zE762 = zU256zIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  } else {
    unit z3zE2905;
    z3zE2905 = sail_assert(false, "sail/primitives/quantities.sail:351.20-351.21");
    sail_match_failure("word_of_nat_byte_count<repr:R__sail_c_repr_u128,R__sail_c_repr_u256><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
    /* unreachable after exit */
  }
end_function_3377: ;
  return z8zE762;
end_block_exception_3378: ;

  return u256_zero();
}

sail_u256 zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  sail_u256 z8zE763;
  bool z2zE3204;
  {    z2zE3204 = true;
  }
  if (z2zE3204) {
    z8zE763 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  } else {
    unit z3zE2905;
    z3zE2905 = sail_assert(false, "sail/primitives/quantities.sail:351.20-351.21");
    sail_match_failure("word_of_nat_byte_count<repr:U64,R__sail_c_repr_u256><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
    /* unreachable after exit */
  }
end_function_3377: ;
  return z8zE763;
end_block_exception_3378: ;

  return u256_zero();
}

sail_u256 zword_sub_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  sail_u256 z8zE764;
  sail_u256 z2zE3326;
  {    z2zE3326 = u256_sub_u64(zleft, zright);
  }
  z8zE764 = zU256(z2zE3326);
end_function_3521: ;
  return z8zE764;
end_block_exception_3522: ;

  return u256_zero();
}

bool zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t za, sail_u256 zb)
{
  bool z8zE765;
  bool z2zE3306;
  z2zE3306 = zword_ultzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zb, za);
  z8zE765 = not(z2zE3306);
end_function_3497: ;
  return z8zE765;
end_block_exception_3498: ;

  return false;
}

bool zword_ultzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, uint64_t zb)
{
  bool z8zE766;
  z8zE766 = u256_lt_u64(za, zb);
end_function_3499: ;
  return z8zE766;
end_block_exception_3500: ;

  return false;
}

void model_init(void)
{
  setup_rts();
  current_exception = sail_new(struct zexception);
  CREATE(zexception)(current_exception);
  throw_location = sail_new(sail_string);
  CREATE(sail_string)(throw_location);
  create_letbind_0();
  create_letbind_1();
  create_letbind_2();
  create_letbind_3();
  create_letbind_4();
  create_letbind_5();
  create_letbind_6();
  create_letbind_7();
  create_letbind_8();
  create_letbind_9();
  create_letbind_10();
  create_letbind_11();
  create_letbind_12();
  create_letbind_13();
  create_letbind_14();
  create_letbind_15();
  create_letbind_16();
  create_letbind_17();
  create_letbind_18();
  create_letbind_19();
  create_letbind_20();
  create_letbind_21();
  create_letbind_22();
  create_letbind_23();
  create_letbind_24();
  create_letbind_25();
  create_letbind_26();
  create_letbind_27();
  create_letbind_28();
  create_letbind_29();
  create_letbind_30();
  create_letbind_31();
  create_letbind_32();
  create_letbind_33();
  create_letbind_34();
  create_letbind_35();
  create_letbind_36();
  create_letbind_37();
  create_letbind_38();
  create_letbind_39();
  create_letbind_40();
  create_letbind_41();
  create_letbind_42();
  create_letbind_43();
  create_letbind_44();
  create_letbind_45();
  create_letbind_46();
  create_letbind_47();
  create_letbind_48();
  create_letbind_49();
  create_letbind_50();
  create_letbind_51();
  create_letbind_52();
  create_letbind_53();
  create_letbind_54();
  create_letbind_55();
  create_letbind_56();
  create_letbind_57();
  create_letbind_58();
  create_letbind_59();
  create_letbind_60();
  create_letbind_61();
  create_letbind_62();
  create_letbind_63();
  create_letbind_64();
  create_letbind_65();
  create_letbind_66();
  create_letbind_67();
  create_letbind_68();
  create_letbind_69();
  create_letbind_70();
  create_letbind_71();
  create_letbind_72();
  create_letbind_73();
  create_letbind_74();
  create_letbind_75();
  create_letbind_76();
  create_letbind_77();
  create_letbind_78();
  create_letbind_79();
  create_letbind_80();
  create_letbind_81();
  create_letbind_82();
  create_letbind_83();
  create_letbind_84();
  create_letbind_85();
  create_letbind_86();
  create_letbind_87();
  create_letbind_88();
  create_letbind_89();
  create_letbind_90();
  create_letbind_91();
  create_letbind_92();
  create_letbind_93();
  create_letbind_94();
  create_letbind_95();
  create_letbind_96();
  create_letbind_97();
  create_letbind_98();
  create_letbind_99();
  create_letbind_100();
  create_letbind_101();
  create_letbind_102();
  create_letbind_103();
  create_letbind_104();
  create_letbind_105();
  create_letbind_106();
  create_letbind_107();
  create_letbind_108();
  create_letbind_109();
  create_letbind_110();
  create_letbind_111();
  create_letbind_112();
  create_letbind_113();
  create_letbind_114();
  create_letbind_115();
  create_letbind_116();
  create_letbind_117();
  create_letbind_118();
  create_letbind_119();
  create_letbind_120();
  create_letbind_121();
  create_letbind_122();
  create_letbind_123();
  create_letbind_124();
  create_letbind_125();
  create_letbind_126();
  create_letbind_127();
  create_letbind_128();
  create_letbind_129();
  create_letbind_130();
  create_letbind_131();
  create_letbind_132();
  create_letbind_133();
  create_letbind_134();
  create_letbind_135();
  create_letbind_136();
  create_letbind_137();
  create_letbind_138();
  create_letbind_139();
  create_letbind_140();
  create_letbind_141();
  create_letbind_142();
  create_letbind_143();
  create_letbind_144();
  create_letbind_145();
  create_letbind_146();
  create_letbind_147();
  create_letbind_148();
  create_letbind_149();
  create_letbind_150();
  create_letbind_151();
  create_letbind_152();
  create_letbind_153();
  create_letbind_154();
  create_letbind_155();
  create_letbind_156();
  create_letbind_157();
  create_letbind_158();
  create_letbind_159();
  create_letbind_160();
  create_letbind_161();
  create_letbind_162();
  create_letbind_163();
  create_letbind_164();
  create_letbind_165();
  create_letbind_166();
  create_letbind_167();
  create_letbind_168();
  create_letbind_169();
  create_letbind_170();
  create_letbind_171();
  create_letbind_172();
  create_letbind_173();
  create_letbind_174();
  create_letbind_175();
  create_letbind_176();
  create_letbind_177();
  create_letbind_178();
  create_letbind_179();
  create_letbind_180();
  create_letbind_181();
  create_letbind_182();
  create_letbind_183();
  create_letbind_184();
  create_letbind_185();
  create_letbind_186();
  create_letbind_187();
  create_letbind_188();
  create_letbind_189();
  create_letbind_190();
  create_letbind_191();
  create_letbind_192();
  create_letbind_193();
  create_letbind_194();
  create_letbind_195();
  create_letbind_196();
  create_letbind_197();
  create_letbind_198();
  create_letbind_199();
  create_letbind_200();
  create_letbind_201();
  create_letbind_202();
  create_letbind_203();
  create_letbind_204();
  create_letbind_205();
  create_letbind_206();
  create_letbind_207();
  create_letbind_208();
  create_letbind_209();
  create_letbind_210();
  create_letbind_211();
  create_letbind_212();
  create_letbind_213();
  create_letbind_214();
  create_letbind_215();
  create_letbind_216();
  create_letbind_217();
  create_letbind_218();
  create_letbind_219();
  create_letbind_220();
  create_letbind_221();
  create_letbind_222();
  create_letbind_223();
  create_letbind_224();
  create_letbind_225();
  create_letbind_226();
  create_letbind_227();
  create_letbind_228();
  create_letbind_229();
  create_letbind_230();
  create_letbind_231();
  create_letbind_232();
  create_letbind_233();
  create_letbind_234();
  create_letbind_235();
  create_letbind_236();
  create_letbind_237();
  create_letbind_238();
  create_letbind_239();
  create_letbind_240();
  create_letbind_241();
  create_letbind_242();
  create_letbind_243();
  create_letbind_244();
  create_letbind_245();
  create_letbind_246();
  create_letbind_247();
  create_letbind_248();
  create_letbind_249();
  create_letbind_250();
  create_letbind_251();
  create_letbind_252();
  create_letbind_253();
  create_letbind_254();
  create_letbind_255();
  create_letbind_256();
  create_letbind_257();
  create_letbind_258();
  create_letbind_259();
  create_letbind_260();
  create_letbind_261();
  create_letbind_262();
  create_letbind_263();
  create_letbind_264();
  create_letbind_265();
  create_letbind_266();
  create_letbind_267();
  create_letbind_268();
  create_letbind_269();
  create_letbind_270();
  create_letbind_271();
  create_letbind_272();
  create_letbind_273();
  create_letbind_274();
  create_letbind_275();
  create_letbind_276();
  create_letbind_277();
  create_letbind_278();
  create_letbind_279();
  create_letbind_280();
  create_letbind_281();
  create_letbind_282();
  create_letbind_283();
  create_letbind_284();
  create_letbind_285();
  create_letbind_286();
  create_letbind_287();
  create_letbind_288();
  create_letbind_289();
  create_letbind_290();
  create_letbind_291();
  create_letbind_292();
  create_letbind_293();
  create_letbind_294();
  create_letbind_295();
  create_letbind_296();
  create_letbind_297();
  create_letbind_298();
  create_letbind_299();
  create_letbind_300();
  create_letbind_301();
  create_letbind_302();
  create_letbind_303();
  create_letbind_304();
  create_letbind_305();
  create_letbind_306();
  create_letbind_307();
  create_letbind_308();
  create_letbind_309();
  create_letbind_310();
  create_letbind_311();
  create_letbind_312();
  create_letbind_313();
  create_letbind_314();
  create_letbind_315();
  create_letbind_316();
  create_letbind_317();
  create_letbind_318();
  create_letbind_319();
  create_letbind_320();
  create_letbind_321();
  create_letbind_322();
  create_letbind_323();
  create_letbind_324();
  create_letbind_325();
  create_letbind_326();
  create_letbind_327();
  create_letbind_328();
  create_letbind_329();
  create_letbind_330();
  create_letbind_331();
  create_letbind_332();
  create_letbind_333();
  create_letbind_334();
  create_letbind_335();
  create_letbind_336();
  create_letbind_337();
  create_letbind_338();
  create_letbind_339();
  create_letbind_340();
  create_letbind_341();
  create_letbind_342();
  create_letbind_343();
  create_letbind_344();
  create_letbind_345();
  create_letbind_346();
  create_letbind_347();
  create_letbind_348();
  create_letbind_349();
  create_letbind_350();
  create_letbind_351();
  create_letbind_352();
  create_letbind_353();
  create_letbind_354();
  create_letbind_355();
  create_letbind_356();
  create_letbind_357();
  create_letbind_358();
  create_letbind_359();
  create_letbind_360();
  create_letbind_361();
  create_letbind_362();
  create_letbind_363();
  create_letbind_364();
  create_letbind_365();
  create_letbind_366();
  create_letbind_367();
  create_letbind_368();
  create_letbind_369();
  create_letbind_370();
  create_letbind_371();
  create_letbind_372();
  create_letbind_373();
  create_letbind_374();
  create_letbind_375();
  create_letbind_376();
  create_letbind_377();
  create_letbind_378();
  create_letbind_379();
  create_letbind_380();
  create_letbind_381();
  create_letbind_382();
  zscratch_arena = zbyte_slice(zScratchSource, UINT64_C(0), UINT64_C(0));
  zk_parent_state_root = zZERO_HASH;
  zk_n_headers = UINT64_C(0);
  zk_chain_id = UINT64_C(1);
  zk_fork = zAmsterdam;
  struct zBlobSchedule z3zE134;
  z3zE134.zbase_fee_update_fraction = UINT64_C(11684671);
  z3zE134.zmax = UINT64_C(21);
  z3zE134.ztarget = UINT64_C(14);
  zk_blob_schedule = z3zE134;
  struct zBlockHeader z3zE135;
  z3zE135.zbase_fee = zZERO_WORD;
  z3zE135.zblob_gas_used = UINT64_C(0);
  z3zE135.zexcess_blob_gas = UINT64_C(0);
  z3zE135.zextra_data = zEMPTY_SLICE;
  z3zE135.zfee_recipient = zZERO_ADDRESS;
  z3zE135.zgas_limit = UINT64_C(0);
  z3zE135.zgas_used = UINT64_C(0);
  z3zE135.zlogs_bloom = zEMPTY_LOGS_BLOOM;
  z3zE135.znumber = UINT64_C(0);
  z3zE135.zparent_beacon_block_root = zZERO_HASH;
  z3zE135.zparent_hash = zZERO_HASH;
  z3zE135.zprev_randao = zZERO_WORD;
  z3zE135.zreceipts_root = zZERO_HASH;
  z3zE135.zslot_number = UINT64_C(0);
  z3zE135.zstate_root = zZERO_HASH;
  z3zE135.ztimestamp = UINT64_C(0);
  zk_header = z3zE135;
  struct zTxEnv z3zE136;
  z3zE136.zblob_hashes = zEMPTY_BLOB_HASHES;
  z3zE136.zgas_price = zZERO_WORD;
  z3zE136.zorigin = zZERO_ADDRESS;
  zk_tx = z3zE136;
  zk_block_access_index = UINT64_C(0);
  zpc = UINT64_C(0);
  zgas_remaining = zGAS_ZERO;
  zstate_gas_remaining = zGAS_ZERO;
  zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
  zframe_refund = zGAS_REFUND_ZERO;
  CREATE(zFrameStatus)(&zframe_status);
  zRunning(&zframe_status, UNIT);
  zmessage = zDEFAULT_MESSAGE;
  zcall_depth = UINT64_C(0);
  zframe_code = zEMPTY_CODE;
  zcalldata = zEMPTY_SLICE;
  zreturndata = zEMPTY_SLICE;
  zevm_memory = zbyte_slice(zEvmMemorySource, UINT64_C(0), UINT64_C(0));
  zinitializze_registers(UNIT);
}

void model_fini(void)
{
  KILL(zFrameStatus)(&zframe_status);
  kill_letbind_382();
  kill_letbind_381();
  kill_letbind_380();
  kill_letbind_379();
  kill_letbind_378();
  kill_letbind_377();
  kill_letbind_376();
  kill_letbind_375();
  kill_letbind_374();
  kill_letbind_373();
  kill_letbind_372();
  kill_letbind_371();
  kill_letbind_370();
  kill_letbind_369();
  kill_letbind_368();
  kill_letbind_367();
  kill_letbind_366();
  kill_letbind_365();
  kill_letbind_364();
  kill_letbind_363();
  kill_letbind_362();
  kill_letbind_361();
  kill_letbind_360();
  kill_letbind_359();
  kill_letbind_358();
  kill_letbind_357();
  kill_letbind_356();
  kill_letbind_355();
  kill_letbind_354();
  kill_letbind_353();
  kill_letbind_352();
  kill_letbind_351();
  kill_letbind_350();
  kill_letbind_349();
  kill_letbind_348();
  kill_letbind_347();
  kill_letbind_346();
  kill_letbind_345();
  kill_letbind_344();
  kill_letbind_343();
  kill_letbind_342();
  kill_letbind_341();
  kill_letbind_340();
  kill_letbind_339();
  kill_letbind_338();
  kill_letbind_337();
  kill_letbind_336();
  kill_letbind_335();
  kill_letbind_334();
  kill_letbind_333();
  kill_letbind_332();
  kill_letbind_331();
  kill_letbind_330();
  kill_letbind_329();
  kill_letbind_328();
  kill_letbind_327();
  kill_letbind_326();
  kill_letbind_325();
  kill_letbind_324();
  kill_letbind_323();
  kill_letbind_322();
  kill_letbind_321();
  kill_letbind_320();
  kill_letbind_319();
  kill_letbind_318();
  kill_letbind_317();
  kill_letbind_316();
  kill_letbind_315();
  kill_letbind_314();
  kill_letbind_313();
  kill_letbind_312();
  kill_letbind_311();
  kill_letbind_310();
  kill_letbind_309();
  kill_letbind_308();
  kill_letbind_307();
  kill_letbind_306();
  kill_letbind_305();
  kill_letbind_304();
  kill_letbind_303();
  kill_letbind_302();
  kill_letbind_301();
  kill_letbind_300();
  kill_letbind_299();
  kill_letbind_298();
  kill_letbind_297();
  kill_letbind_296();
  kill_letbind_295();
  kill_letbind_294();
  kill_letbind_293();
  kill_letbind_292();
  kill_letbind_291();
  kill_letbind_290();
  kill_letbind_289();
  kill_letbind_288();
  kill_letbind_287();
  kill_letbind_286();
  kill_letbind_285();
  kill_letbind_284();
  kill_letbind_283();
  kill_letbind_282();
  kill_letbind_281();
  kill_letbind_280();
  kill_letbind_279();
  kill_letbind_278();
  kill_letbind_277();
  kill_letbind_276();
  kill_letbind_275();
  kill_letbind_274();
  kill_letbind_273();
  kill_letbind_272();
  kill_letbind_271();
  kill_letbind_270();
  kill_letbind_269();
  kill_letbind_268();
  kill_letbind_267();
  kill_letbind_266();
  kill_letbind_265();
  kill_letbind_264();
  kill_letbind_263();
  kill_letbind_262();
  kill_letbind_261();
  kill_letbind_260();
  kill_letbind_259();
  kill_letbind_258();
  kill_letbind_257();
  kill_letbind_256();
  kill_letbind_255();
  kill_letbind_254();
  kill_letbind_253();
  kill_letbind_252();
  kill_letbind_251();
  kill_letbind_250();
  kill_letbind_249();
  kill_letbind_248();
  kill_letbind_247();
  kill_letbind_246();
  kill_letbind_245();
  kill_letbind_244();
  kill_letbind_243();
  kill_letbind_242();
  kill_letbind_241();
  kill_letbind_240();
  kill_letbind_239();
  kill_letbind_238();
  kill_letbind_237();
  kill_letbind_236();
  kill_letbind_235();
  kill_letbind_234();
  kill_letbind_233();
  kill_letbind_232();
  kill_letbind_231();
  kill_letbind_230();
  kill_letbind_229();
  kill_letbind_228();
  kill_letbind_227();
  kill_letbind_226();
  kill_letbind_225();
  kill_letbind_224();
  kill_letbind_223();
  kill_letbind_222();
  kill_letbind_221();
  kill_letbind_220();
  kill_letbind_219();
  kill_letbind_218();
  kill_letbind_217();
  kill_letbind_216();
  kill_letbind_215();
  kill_letbind_214();
  kill_letbind_213();
  kill_letbind_212();
  kill_letbind_211();
  kill_letbind_210();
  kill_letbind_209();
  kill_letbind_208();
  kill_letbind_207();
  kill_letbind_206();
  kill_letbind_205();
  kill_letbind_204();
  kill_letbind_203();
  kill_letbind_202();
  kill_letbind_201();
  kill_letbind_200();
  kill_letbind_199();
  kill_letbind_198();
  kill_letbind_197();
  kill_letbind_196();
  kill_letbind_195();
  kill_letbind_194();
  kill_letbind_193();
  kill_letbind_192();
  kill_letbind_191();
  kill_letbind_190();
  kill_letbind_189();
  kill_letbind_188();
  kill_letbind_187();
  kill_letbind_186();
  kill_letbind_185();
  kill_letbind_184();
  kill_letbind_183();
  kill_letbind_182();
  kill_letbind_181();
  kill_letbind_180();
  kill_letbind_179();
  kill_letbind_178();
  kill_letbind_177();
  kill_letbind_176();
  kill_letbind_175();
  kill_letbind_174();
  kill_letbind_173();
  kill_letbind_172();
  kill_letbind_171();
  kill_letbind_170();
  kill_letbind_169();
  kill_letbind_168();
  kill_letbind_167();
  kill_letbind_166();
  kill_letbind_165();
  kill_letbind_164();
  kill_letbind_163();
  kill_letbind_162();
  kill_letbind_161();
  kill_letbind_160();
  kill_letbind_159();
  kill_letbind_158();
  kill_letbind_157();
  kill_letbind_156();
  kill_letbind_155();
  kill_letbind_154();
  kill_letbind_153();
  kill_letbind_152();
  kill_letbind_151();
  kill_letbind_150();
  kill_letbind_149();
  kill_letbind_148();
  kill_letbind_147();
  kill_letbind_146();
  kill_letbind_145();
  kill_letbind_144();
  kill_letbind_143();
  kill_letbind_142();
  kill_letbind_141();
  kill_letbind_140();
  kill_letbind_139();
  kill_letbind_138();
  kill_letbind_137();
  kill_letbind_136();
  kill_letbind_135();
  kill_letbind_134();
  kill_letbind_133();
  kill_letbind_132();
  kill_letbind_131();
  kill_letbind_130();
  kill_letbind_129();
  kill_letbind_128();
  kill_letbind_127();
  kill_letbind_126();
  kill_letbind_125();
  kill_letbind_124();
  kill_letbind_123();
  kill_letbind_122();
  kill_letbind_121();
  kill_letbind_120();
  kill_letbind_119();
  kill_letbind_118();
  kill_letbind_117();
  kill_letbind_116();
  kill_letbind_115();
  kill_letbind_114();
  kill_letbind_113();
  kill_letbind_112();
  kill_letbind_111();
  kill_letbind_110();
  kill_letbind_109();
  kill_letbind_108();
  kill_letbind_107();
  kill_letbind_106();
  kill_letbind_105();
  kill_letbind_104();
  kill_letbind_103();
  kill_letbind_102();
  kill_letbind_101();
  kill_letbind_100();
  kill_letbind_99();
  kill_letbind_98();
  kill_letbind_97();
  kill_letbind_96();
  kill_letbind_95();
  kill_letbind_94();
  kill_letbind_93();
  kill_letbind_92();
  kill_letbind_91();
  kill_letbind_90();
  kill_letbind_89();
  kill_letbind_88();
  kill_letbind_87();
  kill_letbind_86();
  kill_letbind_85();
  kill_letbind_84();
  kill_letbind_83();
  kill_letbind_82();
  kill_letbind_81();
  kill_letbind_80();
  kill_letbind_79();
  kill_letbind_78();
  kill_letbind_77();
  kill_letbind_76();
  kill_letbind_75();
  kill_letbind_74();
  kill_letbind_73();
  kill_letbind_72();
  kill_letbind_71();
  kill_letbind_70();
  kill_letbind_69();
  kill_letbind_68();
  kill_letbind_67();
  kill_letbind_66();
  kill_letbind_65();
  kill_letbind_64();
  kill_letbind_63();
  kill_letbind_62();
  kill_letbind_61();
  kill_letbind_60();
  kill_letbind_59();
  kill_letbind_58();
  kill_letbind_57();
  kill_letbind_56();
  kill_letbind_55();
  kill_letbind_54();
  kill_letbind_53();
  kill_letbind_52();
  kill_letbind_51();
  kill_letbind_50();
  kill_letbind_49();
  kill_letbind_48();
  kill_letbind_47();
  kill_letbind_46();
  kill_letbind_45();
  kill_letbind_44();
  kill_letbind_43();
  kill_letbind_42();
  kill_letbind_41();
  kill_letbind_40();
  kill_letbind_39();
  kill_letbind_38();
  kill_letbind_37();
  kill_letbind_36();
  kill_letbind_35();
  kill_letbind_34();
  kill_letbind_33();
  kill_letbind_32();
  kill_letbind_31();
  kill_letbind_30();
  kill_letbind_29();
  kill_letbind_28();
  kill_letbind_27();
  kill_letbind_26();
  kill_letbind_25();
  kill_letbind_24();
  kill_letbind_23();
  kill_letbind_22();
  kill_letbind_21();
  kill_letbind_20();
  kill_letbind_19();
  kill_letbind_18();
  kill_letbind_17();
  kill_letbind_16();
  kill_letbind_15();
  kill_letbind_14();
  kill_letbind_13();
  kill_letbind_12();
  kill_letbind_11();
  kill_letbind_10();
  kill_letbind_9();
  kill_letbind_8();
  kill_letbind_7();
  kill_letbind_6();
  kill_letbind_5();
  kill_letbind_4();
  kill_letbind_3();
  kill_letbind_2();
  kill_letbind_1();
  kill_letbind_0();
  cleanup_rts();
  if (have_exception) {fprintf(stderr, "Exiting due to uncaught exception: %s\n", *throw_location);}
  KILL(zexception)(current_exception);
  sail_free(current_exception);
  KILL(sail_string)(throw_location);
  sail_free(throw_location);
  if (have_exception) {exit(EXIT_FAILURE);}
}



void model_test(void)
{
  for (size_t i = 0; i < SAIL_TEST_COUNT; ++i) {
    model_init();
    printf("Testing %s\n", SAIL_TEST_NAMES[i]);
    SAIL_TESTS[i](UNIT);
    printf("Pass\n");
    model_fini();
  }
}



#ifdef __cplusplus
}
#endif
