/* Generated from sail/evm/execute.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_20 zself_addr(unit z3zE2639)
{
  sail_fixed_bytes_20 z8zE415;
  z8zE415 = zmessage.zaddress;
end_function_2610: ;
  return z8zE415;
end_block_exception_2611: ;

  return fixed_bytes_20_zero();
}

bool zguard_static(unit z3zE2637)
{
  bool z8zE416;
  bool z2zE2443;
  z2zE2443 = zmessage.zis_static;
  if (z2zE2443) {
    unit z3zE2638;
    {
      z3zE2638 = zexc_halt(zWriteProtection);
      if (have_exception) {  goto end_block_exception_2609;  }
    }
    z8zE416 = true;
  } else {  z8zE416 = false;  }
end_function_2608: ;
  return z8zE416;
end_block_exception_2609: ;

  return false;
}

unit zdo_jump(sail_u256 zdestination_value)
{
  unit z8zE417;
  uint64_t zcode_length;
  zcode_length = zframe_code_len(UNIT);
  bool z2zE2441;
  z2zE2441 = u256_lt_u64(zdestination_value, zcode_length);
  if (z2zE2441) {
    uint64_t zdestination;
    zdestination = u256_to_u64(zdestination_value);
    bool z2zE2442;
    z2zE2442 = zframe_jumpdest_valid(zdestination);
    if (z2zE2442) {
      zpc = zdestination;
      z8zE417 = UNIT;
    } else {
      {
        z8zE417 = zexc_halt(zInvalidJump);
        if (have_exception) {  goto end_block_exception_2607;  }
      }
    }
  } else {
    {
      z8zE417 = zexc_halt(zInvalidJump);
      if (have_exception) {  goto end_block_exception_2607;  }
    }
  }
end_function_2606: ;
  return z8zE417;
end_block_exception_2607: ;

  return UNIT;
}

void zpop_log_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9 *z8zE418, uint64_t zcount)
{
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z3zE2615;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
  {
    uint64_t zp0z3;
    zp0z3 = zcount;
    bool z3zE2627;
    z3zE2627 = (zp0z3 == UINT64_C(0));
    if (!(z3zE2627)) {  goto case_2603;  }
    goto finish_match_2597;
  }
case_2603: ;
  {
    uint64_t z3zE2628;
    z3zE2628 = zcount;
    bool z3zE2625;
    z3zE2625 = (z3zE2628 == UINT64_C(1));
    if (!(z3zE2625)) {  goto case_2602;  }
    sail_u256 z2zE2440;
    {
      z2zE2440 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, z2zE2440, z3zE2615);
    goto finish_match_2597;
  }
case_2602: ;
  {
    uint64_t z3zE2629;
    z3zE2629 = zcount;
    bool z3zE2623;
    z3zE2623 = (z3zE2629 == UINT64_C(2));
    if (!(z3zE2623)) {  goto case_2601;  }
    sail_u256 zt0;
    {
      zt0 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    sail_u256 zt1;
    {
      zt1 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, zt1, z3zE2615);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, zt0, z3zE2615);
    goto finish_match_2597;
  }
case_2601: ;
  {
    uint64_t z3zE2630;
    z3zE2630 = zcount;
    bool z3zE2621;
    z3zE2621 = (z3zE2630 == UINT64_C(3));
    if (!(z3zE2621)) {  goto case_2600;  }
    sail_u256 z3zE2631;
    {
      z3zE2631 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    sail_u256 z3zE2632;
    {
      z3zE2632 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    sail_u256 zt2;
    {
      zt2 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, zt2, z3zE2615);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, z3zE2632, z3zE2615);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, z3zE2631, z3zE2615);
    goto finish_match_2597;
  }
case_2600: ;
  {
    uint64_t z3zE2633;
    z3zE2633 = zcount;
    bool z3zE2619;
    z3zE2619 = (z3zE2633 == UINT64_C(4));
    if (!(z3zE2619)) {  goto case_2599;  }
    sail_u256 z3zE2634;
    {
      z3zE2634 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    sail_u256 z3zE2635;
    {
      z3zE2635 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    sail_u256 z3zE2636;
    {
      z3zE2636 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    sail_u256 zt3;
    {
      zt3 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
        goto end_block_exception_2605;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, zt3, z3zE2615);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, z3zE2636, z3zE2615);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, z3zE2635, z3zE2615);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2615, z3zE2634, z3zE2615);
    goto finish_match_2597;
  }
case_2599: ;
  {    goto finish_match_2597;
  }
case_2598: ;
finish_match_2597: ;
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)((*(&z8zE418)), z3zE2615);
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2615);
end_function_2604: ;
  goto end_function_4048;
end_block_exception_2605: ;
  goto end_function_4048;
end_function_4048: ;
}

unit zexecute_arithmetic(struct zast zop)
{
  unit z8zE419;
  unit z3zE2519;
  {
    if (zop.kind != Kind_zADD) goto case_2594;
    unit z3zE2571;
    {
      z3zE2571 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 za;
    {
      za = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zb;
    {
      zb = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2413;
    z2zE2413 = zalu_add(za, zb);
    {
      z3zE2519 = zpush_word(z2zE2413);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2594: ;
  {
    if (zop.kind != Kind_zMUL) goto case_2593;
    unit z3zE2569;
    {
      z3zE2569 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2573;
    {
      z3zE2573 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2574;
    {
      z3zE2574 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2414;
    z2zE2414 = zalu_mul(z3zE2573, z3zE2574);
    {
      z3zE2519 = zpush_word(z2zE2414);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2593: ;
  {
    if (zop.kind != Kind_zSUB) goto case_2592;
    unit z3zE2567;
    {
      z3zE2567 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2575;
    {
      z3zE2575 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2576;
    {
      z3zE2576 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2415;
    z2zE2415 = zalu_sub(z3zE2575, z3zE2576);
    {
      z3zE2519 = zpush_word(z2zE2415);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2592: ;
  {
    if (zop.kind != Kind_zDIV) goto case_2591;
    unit z3zE2565;
    {
      z3zE2565 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2577;
    {
      z3zE2577 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2578;
    {
      z3zE2578 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2416;
    z2zE2416 = zalu_div(z3zE2577, z3zE2578);
    {
      z3zE2519 = zpush_word(z2zE2416);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2591: ;
  {
    if (zop.kind != Kind_zSDIV) goto case_2590;
    unit z3zE2563;
    {
      z3zE2563 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2579;
    {
      z3zE2579 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2580;
    {
      z3zE2580 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2417;
    z2zE2417 = zalu_sdiv(z3zE2579, z3zE2580);
    {
      z3zE2519 = zpush_word(z2zE2417);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2590: ;
  {
    if (zop.kind != Kind_zMOD) goto case_2589;
    unit z3zE2561;
    {
      z3zE2561 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2581;
    {
      z3zE2581 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2582;
    {
      z3zE2582 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2418;
    z2zE2418 = zalu_mod(z3zE2581, z3zE2582);
    {
      z3zE2519 = zpush_word(z2zE2418);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2589: ;
  {
    if (zop.kind != Kind_zSMOD) goto case_2588;
    unit z3zE2559;
    {
      z3zE2559 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2583;
    {
      z3zE2583 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2584;
    {
      z3zE2584 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2419;
    z2zE2419 = zalu_smod(z3zE2583, z3zE2584);
    {
      z3zE2519 = zpush_word(z2zE2419);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2588: ;
  {
    if (zop.kind != Kind_zADDMOD) goto case_2587;
    unit z3zE2557;
    {
      z3zE2557 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2585;
    {
      z3zE2585 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2586;
    {
      z3zE2586 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zn;
    {
      zn = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2420;
    z2zE2420 = zalu_addmod(z3zE2585, z3zE2586, zn);
    {
      z3zE2519 = zpush_word(z2zE2420);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2587: ;
  {
    if (zop.kind != Kind_zMULMOD) goto case_2586;
    unit z3zE2555;
    {
      z3zE2555 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2587;
    {
      z3zE2587 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2588;
    {
      z3zE2588 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2589;
    {
      z3zE2589 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2421;
    z2zE2421 = zalu_mulmod(z3zE2587, z3zE2588, z3zE2589);
    {
      z3zE2519 = zpush_word(z2zE2421);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2586: ;
  {
    if (zop.kind != Kind_zEXP) goto case_2585;
    sail_u256 z3zE2590;
    {
      z3zE2590 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 ze;
    {
      ze = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    uint64_t z2zE2422;
    z2zE2422 = zexp_gas(ze);
    unit z3zE2553;
    {
      z3zE2553 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2422);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2423;
    z2zE2423 = zalu_exp(z3zE2590, ze);
    {
      z3zE2519 = zpush_word(z2zE2423);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2585: ;
  {
    if (zop.kind != Kind_zSIGNEXTEND) goto case_2584;
    unit z3zE2551;
    {
      z3zE2551 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zbi;
    {
      zbi = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zv;
    {
      zv = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2424;
    z2zE2424 = zalu_signextend(zbi, zv);
    {
      z3zE2519 = zpush_word(z2zE2424);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2584: ;
  {
    if (zop.kind != Kind_zLT) goto case_2583;
    unit z3zE2549;
    {
      z3zE2549 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2591;
    {
      z3zE2591 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2592;
    {
      z3zE2592 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2425;
    z2zE2425 = zalu_lt(z3zE2591, z3zE2592);
    {
      z3zE2519 = zpush_word(z2zE2425);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2583: ;
  {
    if (zop.kind != Kind_zGT) goto case_2582;
    unit z3zE2547;
    {
      z3zE2547 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2593;
    {
      z3zE2593 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2594;
    {
      z3zE2594 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2426;
    z2zE2426 = zalu_gt(z3zE2593, z3zE2594);
    {
      z3zE2519 = zpush_word(z2zE2426);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2582: ;
  {
    if (zop.kind != Kind_zSLT) goto case_2581;
    unit z3zE2545;
    {
      z3zE2545 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2595;
    {
      z3zE2595 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2596;
    {
      z3zE2596 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2427;
    z2zE2427 = zalu_slt(z3zE2595, z3zE2596);
    {
      z3zE2519 = zpush_word(z2zE2427);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2581: ;
  {
    if (zop.kind != Kind_zSGT) goto case_2580;
    unit z3zE2543;
    {
      z3zE2543 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2597;
    {
      z3zE2597 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2598;
    {
      z3zE2598 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2428;
    z2zE2428 = zalu_sgt(z3zE2597, z3zE2598);
    {
      z3zE2519 = zpush_word(z2zE2428);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2580: ;
  {
    if (zop.kind != Kind_zEQ) goto case_2579;
    unit z3zE2541;
    {
      z3zE2541 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2599;
    {
      z3zE2599 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2600;
    {
      z3zE2600 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2429;
    z2zE2429 = zalu_eq(z3zE2599, z3zE2600);
    {
      z3zE2519 = zpush_word(z2zE2429);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2579: ;
  {
    if (zop.kind != Kind_zISZERO) goto case_2578;
    unit z3zE2539;
    {
      z3zE2539 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2601;
    {
      z3zE2601 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2430;
    z2zE2430 = zalu_iszzero(z3zE2601);
    {
      z3zE2519 = zpush_word(z2zE2430);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2578: ;
  {
    if (zop.kind != Kind_zAND) goto case_2577;
    unit z3zE2537;
    {
      z3zE2537 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2602;
    {
      z3zE2602 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2603;
    {
      z3zE2603 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2431;
    z2zE2431 = zalu_and(z3zE2602, z3zE2603);
    {
      z3zE2519 = zpush_word(z2zE2431);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2577: ;
  {
    if (zop.kind != Kind_zOR) goto case_2576;
    unit z3zE2535;
    {
      z3zE2535 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2604;
    {
      z3zE2604 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2605;
    {
      z3zE2605 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2432;
    z2zE2432 = zalu_or(z3zE2604, z3zE2605);
    {
      z3zE2519 = zpush_word(z2zE2432);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2576: ;
  {
    if (zop.kind != Kind_zXOR) goto case_2575;
    unit z3zE2533;
    {
      z3zE2533 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2606;
    {
      z3zE2606 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2607;
    {
      z3zE2607 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2433;
    z2zE2433 = zalu_xor(z3zE2606, z3zE2607);
    {
      z3zE2519 = zpush_word(z2zE2433);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2575: ;
  {
    if (zop.kind != Kind_zNOT) goto case_2574;
    unit z3zE2531;
    {
      z3zE2531 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2608;
    {
      z3zE2608 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2434;
    z2zE2434 = zalu_not(z3zE2608);
    {
      z3zE2519 = zpush_word(z2zE2434);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2574: ;
  {
    if (zop.kind != Kind_zBYTE) goto case_2573;
    unit z3zE2529;
    {
      z3zE2529 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zi;
    {
      zi = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zx;
    {
      zx = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2435;
    z2zE2435 = zalu_byte(zi, zx);
    {
      z3zE2519 = zpush_word(z2zE2435);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2573: ;
  {
    if (zop.kind != Kind_zSHL) goto case_2572;
    unit z3zE2527;
    {
      z3zE2527 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 zs;
    {
      zs = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2609;
    {
      z3zE2609 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2436;
    z2zE2436 = zalu_shl(zs, z3zE2609);
    {
      z3zE2519 = zpush_word(z2zE2436);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2572: ;
  {
    if (zop.kind != Kind_zSHR) goto case_2571;
    unit z3zE2525;
    {
      z3zE2525 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2610;
    {
      z3zE2610 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2611;
    {
      z3zE2611 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2437;
    z2zE2437 = zalu_shr(z3zE2610, z3zE2611);
    {
      z3zE2519 = zpush_word(z2zE2437);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2571: ;
  {
    if (zop.kind != Kind_zSAR) goto case_2570;
    unit z3zE2523;
    {
      z3zE2523 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2612;
    {
      z3zE2612 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2613;
    {
      z3zE2613 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2438;
    z2zE2438 = zalu_sar(z3zE2612, z3zE2613);
    {
      z3zE2519 = zpush_word(z2zE2438);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2570: ;
  {
    if (zop.kind != Kind_zCLZ) goto case_2569;
    unit z3zE2521;
    {
      z3zE2521 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z3zE2614;
    {
      z3zE2614 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    sail_u256 z2zE2439;
    z2zE2439 = zalu_clzz(z3zE2614);
    {
      z3zE2519 = zpush_word(z2zE2439);
      if (have_exception) {  goto end_block_exception_2596;  }
    }
    goto finish_match_2567;
  }
case_2569: ;
  {
    z3zE2519 = sail_assert(false, "sail/evm/execute.sail:255.21-255.22");
    goto finish_match_2567;
  }
case_2568: ;
finish_match_2567: ;
  z8zE419 = z3zE2519;
end_function_2595: ;
  return z8zE419;
end_block_exception_2596: ;

  return UNIT;
}

unit zexecute_environment(struct zast zop)
{
  unit z8zE420;
  unit z3zE2456;
  {
    if (zop.kind != Kind_zKECCAK256) goto case_2564;
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    unit z3zE2496;
    {
      z3zE2496 = zcharge_keccak_gas(zlength_word);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_range(zoffset_word, zlength_word);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2362;
    z2zE2362 = zis_running(UNIT);
    if (z2zE2362) {
      sail_u256 z2zE2363;
      z2zE2363 = zmem_keccak(zrange);
      {
        z3zE2456 = zpush_word(z2zE2363);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2564: ;
  {
    if (zop.kind != Kind_zADDRESS) goto case_2563;
    unit z3zE2494;
    {
      z3zE2494 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2365;
    {
      sail_fixed_bytes_20 z2zE2364;
      z2zE2364 = zself_addr(UNIT);
      z2zE2365 = zaddress_to_word(z2zE2364);
    }
    {
      z3zE2456 = zpush_word(z2zE2365);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2563: ;
  {
    if (zop.kind != Kind_zORIGIN) goto case_2562;
    unit z3zE2492;
    {
      z3zE2492 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2366;
    z2zE2366 = zk_env(zF_Origin);
    {
      z3zE2456 = zpush_word(z2zE2366);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2562: ;
  {
    if (zop.kind != Kind_zCALLER) goto case_2561;
    unit z3zE2490;
    {
      z3zE2490 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2368;
    {
      sail_fixed_bytes_20 z2zE2367;
      z2zE2367 = zmessage.zcaller;
      z2zE2368 = zaddress_to_word(z2zE2367);
    }
    {
      z3zE2456 = zpush_word(z2zE2368);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2561: ;
  {
    if (zop.kind != Kind_zCALLVALUE) goto case_2560;
    unit z3zE2488;
    {
      z3zE2488 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2369;
    z2zE2369 = zmessage.zvalue;
    {
      z3zE2456 = zpush_word(z2zE2369);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2560: ;
  {
    if (zop.kind != Kind_zGASPRICE) goto case_2559;
    unit z3zE2486;
    {
      z3zE2486 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2370;
    z2zE2370 = zk_env(zF_GasPrice);
    {
      z3zE2456 = zpush_word(z2zE2370);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2559: ;
  {
    if (zop.kind != Kind_zCALLDATASIZE) goto case_2558;
    unit z3zE2484;
    {
      z3zE2484 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    struct zByteSliceFields zinput;
    zinput = zcalldata;
    sail_u256 z2zE2372;
    {
      uint64_t z2zE2371;
      z2zE2371 = zinput.zlen;
      z2zE2372 = zword_of_source_byte_count(z2zE2371);
    }
    {
      z3zE2456 = zpush_word(z2zE2372);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2558: ;
  {
    if (zop.kind != Kind_zCALLDATALOAD) goto case_2557;
    unit z3zE2482;
    {
      z3zE2482 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2498;
    {
      z3zE2498 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2373;
    z2zE2373 = zis_running(UNIT);
    if (z2zE2373) {
      sail_u256 z2zE2374;
      z2zE2374 = zslice_load_word_offset(zcalldata, z3zE2498);
      {
        z3zE2456 = zpush_word(z2zE2374);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2557: ;
  {
    if (zop.kind != Kind_zCALLDATACOPY) goto case_2556;
    unit z3zE2479;
    {
      z3zE2479 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 zdestination_word;
    {
      zdestination_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 zsource_word;
    {
      zsource_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2499;
    {
      z3zE2499 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    unit z3zE2480;
    {
      z3zE2480 = zcharge_copy_gas(z3zE2499);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    struct zMemoryRangeFields z3zE2500;
    {
      z3zE2500 = zcharge_memory_range(zdestination_word, z3zE2499);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2375;
    z2zE2375 = zis_running(UNIT);
    if (z2zE2375) {
      uint64_t z2zE2376;
      z2zE2376 = z3zE2500.zoff;
      uint64_t z2zE2377;
      z2zE2377 = z3zE2500.zlen;
      z3zE2456 = zslice_copy_word_offset(zcalldata, z2zE2376, zsource_word, z2zE2377);
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2556: ;
  {
    if (zop.kind != Kind_zCODESIZE) goto case_2555;
    unit z3zE2477;
    {
      z3zE2477 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2379;
    {
      uint64_t z2zE2378;
      z2zE2378 = zframe_code_len(UNIT);
      z2zE2379 = zword_of_source_byte_count(z2zE2378);
    }
    {
      z3zE2456 = zpush_word(z2zE2379);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2555: ;
  {
    if (zop.kind != Kind_zCODECOPY) goto case_2554;
    unit z3zE2474;
    {
      z3zE2474 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2501;
    {
      z3zE2501 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2502;
    {
      z3zE2502 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2503;
    {
      z3zE2503 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    unit z3zE2475;
    {
      z3zE2475 = zcharge_copy_gas(z3zE2503);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    struct zMemoryRangeFields z3zE2504;
    {
      z3zE2504 = zcharge_memory_range(z3zE2501, z3zE2503);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2380;
    z2zE2380 = zis_running(UNIT);
    if (z2zE2380) {
      struct zByteSliceFields z2zE2381;
      z2zE2381 = zframe_code.zbytes;
      uint64_t z2zE2382;
      z2zE2382 = z3zE2504.zoff;
      uint64_t z2zE2383;
      z2zE2383 = z3zE2504.zlen;
      z3zE2456 = zslice_copy_word_offset(z2zE2381, z2zE2382, z3zE2502, z2zE2383);
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2554: ;
  {
    if (zop.kind != Kind_zBALANCE) goto case_2553;
    sail_fixed_bytes_20 za;
    {
      sail_u256 z2zE2387;
      {
        z2zE2387 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
      za = zword_to_address(z2zE2387);
    }
    bool zwarm;
    zwarm = zk_access_account(za);
    uint64_t z2zE2384;
    z2zE2384 = zaccount_cost(zwarm);
    unit z3zE2472;
    {
      z3zE2472 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2384);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2385;
    z2zE2385 = zis_running(UNIT);
    if (z2zE2385) {
      sail_u256 z2zE2386;
      {
        z2zE2386 = zk_get_balance(za);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
      {
        z3zE2456 = zpush_word(z2zE2386);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2553: ;
  {
    if (zop.kind != Kind_zSELFBALANCE) goto case_2552;
    unit z3zE2470;
    {
      z3zE2470 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2389;
    {
      sail_fixed_bytes_20 z2zE2388;
      z2zE2388 = zself_addr(UNIT);
      {
        z2zE2389 = zk_get_balance(z2zE2388);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    }
    {
      z3zE2456 = zpush_word(z2zE2389);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2552: ;
  {
    if (zop.kind != Kind_zEXTCODESIZE) goto case_2551;
    sail_fixed_bytes_20 z3zE2505;
    {
      sail_u256 z2zE2396;
      {
        z2zE2396 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
      z3zE2505 = zword_to_address(z2zE2396);
    }
    bool z3zE2506;
    z3zE2506 = zk_access_account(z3zE2505);
    uint64_t z2zE2392;
    {
      uint64_t z2zE2390;
      z2zE2390 = zaccount_cost(z3zE2506);
      uint64_t z2zE2391;
      z2zE2391 = zexternal_code_read_cost(UNIT);
      {    z2zE2392 = (z2zE2390 + z2zE2391);
      }
    }
    unit z3zE2468;
    {
      z3zE2468 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2392);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2393;
    z2zE2393 = zis_running(UNIT);
    if (z2zE2393) {
      sail_u256 z2zE2395;
      {
        uint64_t z2zE2394;
        {
          z2zE2394 = zk_get_code_sizze(z3zE2505);
          if (have_exception) {  goto end_block_exception_2566;  }
        }
        z2zE2395 = zword_of_source_byte_count(z2zE2394);
      }
      {
        z3zE2456 = zpush_word(z2zE2395);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2551: ;
  {
    if (zop.kind != Kind_zEXTCODECOPY) goto case_2550;
    sail_fixed_bytes_20 z3zE2507;
    {
      sail_u256 z2zE2403;
      {
        z2zE2403 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
      z3zE2507 = zword_to_address(z2zE2403);
    }
    sail_u256 z3zE2508;
    {
      z3zE2508 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2509;
    {
      z3zE2509 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2510;
    {
      z3zE2510 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z3zE2511;
    z3zE2511 = zk_access_account(z3zE2507);
    uint64_t z2zE2399;
    {
      uint64_t z2zE2397;
      z2zE2397 = zaccount_cost(z3zE2511);
      uint64_t z2zE2398;
      z2zE2398 = zexternal_code_read_cost(UNIT);
      {    z2zE2399 = (z2zE2397 + z2zE2398);
      }
    }
    unit z3zE2466;
    {
      z3zE2466 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2399);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    unit z3zE2465;
    {
      z3zE2465 = zcharge_copy_gas(z3zE2510);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    struct zMemoryRangeFields z3zE2512;
    {
      z3zE2512 = zcharge_memory_range(z3zE2508, z3zE2510);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2400;
    z2zE2400 = zis_running(UNIT);
    if (z2zE2400) {
      uint64_t z2zE2401;
      z2zE2401 = z3zE2512.zoff;
      uint64_t z2zE2402;
      z2zE2402 = z3zE2512.zlen;
      {
        z3zE2456 = zk_code_copy(z3zE2507, z2zE2401, z3zE2509, z2zE2402);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2550: ;
  {
    if (zop.kind != Kind_zEXTCODEHASH) goto case_2549;
    sail_fixed_bytes_20 z3zE2513;
    {
      sail_u256 z2zE2408;
      {
        z2zE2408 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
      z3zE2513 = zword_to_address(z2zE2408);
    }
    bool z3zE2514;
    z3zE2514 = zk_access_account(z3zE2513);
    uint64_t z2zE2404;
    z2zE2404 = zaccount_cost(z3zE2514);
    unit z3zE2463;
    {
      z3zE2463 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2404);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2405;
    z2zE2405 = zis_running(UNIT);
    if (z2zE2405) {
      sail_u256 z2zE2407;
      {
        sail_fixed_bytes_32 z2zE2406;
        {
          z2zE2406 = zk_get_codehash(z3zE2513);
          if (have_exception) {  goto end_block_exception_2566;  }
        }
        z2zE2407 = zhash_to_word(z2zE2406);
      }
      {
        z3zE2456 = zpush_word(z2zE2407);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2549: ;
  {
    if (zop.kind != Kind_zRETURNDATASIZE) goto case_2548;
    unit z3zE2461;
    {
      z3zE2461 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z2zE2410;
    {
      uint64_t z2zE2409;
      z2zE2409 = zreturndata_sizze(UNIT);
      z2zE2410 = zword_of_source_byte_count(z2zE2409);
    }
    {
      z3zE2456 = zpush_word(z2zE2410);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    goto finish_match_2545;
  }
case_2548: ;
  {
    if (zop.kind != Kind_zRETURNDATACOPY) goto case_2547;
    unit z3zE2458;
    {
      z3zE2458 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2515;
    {
      z3zE2515 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2516;
    {
      z3zE2516 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    sail_u256 z3zE2517;
    {
      z3zE2517 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    unit z3zE2459;
    {
      z3zE2459 = zcharge_copy_gas(z3zE2517);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    struct zMemoryRangeFields z3zE2518;
    {
      z3zE2518 = zcharge_memory_range(z3zE2515, z3zE2517);
      if (have_exception) {  goto end_block_exception_2566;  }
    }
    bool z2zE2411;
    z2zE2411 = zis_running(UNIT);
    if (z2zE2411) {
      uint64_t z2zE2412;
      z2zE2412 = z3zE2518.zoff;
      {
        z3zE2456 = zreturndata_copy_words(z2zE2412, z3zE2516, z3zE2517);
        if (have_exception) {  goto end_block_exception_2566;  }
      }
    } else {  z3zE2456 = UNIT;  }
    goto finish_match_2545;
  }
case_2547: ;
  {
    z3zE2456 = sail_assert(false, "sail/evm/execute.sail:383.21-383.22");
    goto finish_match_2545;
  }
case_2546: ;
finish_match_2545: ;
  z8zE420 = z3zE2456;
end_function_2565: ;
  return z8zE420;
end_block_exception_2566: ;

  return UNIT;
}

unit zexecute_block(struct zast zop)
{
  unit z8zE421;
  unit z3zE2432;
  {
    if (zop.kind != Kind_zBLOCKHASH) goto case_2542;
    unit z3zE2454;
    {
      z3zE2454 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(UINT64_C(20));
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2349;
    {
      sail_fixed_bytes_32 z2zE2348;
      {
        sail_u256 z2zE2347;
        {
          z2zE2347 = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2544;  }
        }
        {
          z2zE2348 = zk_blockhash(z2zE2347);
          if (have_exception) {  goto end_block_exception_2544;  }
        }
      }
      z2zE2349 = zhash_to_word(z2zE2348);
    }
    {
      z3zE2432 = zpush_word(z2zE2349);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2542: ;
  {
    if (zop.kind != Kind_zCOINBASE) goto case_2541;
    unit z3zE2452;
    {
      z3zE2452 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2350;
    z2zE2350 = zk_env(zF_Coinbase);
    {
      z3zE2432 = zpush_word(z2zE2350);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2541: ;
  {
    if (zop.kind != Kind_zTIMESTAMP) goto case_2540;
    unit z3zE2450;
    {
      z3zE2450 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2351;
    z2zE2351 = zk_env(zF_Timestamp);
    {
      z3zE2432 = zpush_word(z2zE2351);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2540: ;
  {
    if (zop.kind != Kind_zNUMBER) goto case_2539;
    unit z3zE2448;
    {
      z3zE2448 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2352;
    z2zE2352 = zk_env(zF_Number);
    {
      z3zE2432 = zpush_word(z2zE2352);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2539: ;
  {
    if (zop.kind != Kind_zSLOTNUM) goto case_2538;
    unit z3zE2446;
    {
      z3zE2446 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2353;
    z2zE2353 = zk_env(zF_SlotNumber);
    {
      z3zE2432 = zpush_word(z2zE2353);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2538: ;
  {
    if (zop.kind != Kind_zPREVRANDAO) goto case_2537;
    unit z3zE2444;
    {
      z3zE2444 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2354;
    z2zE2354 = zk_env(zF_PrevRandao);
    {
      z3zE2432 = zpush_word(z2zE2354);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2537: ;
  {
    if (zop.kind != Kind_zGASLIMIT) goto case_2536;
    unit z3zE2442;
    {
      z3zE2442 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2355;
    z2zE2355 = zk_env(zF_GasLimit);
    {
      z3zE2432 = zpush_word(z2zE2355);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2536: ;
  {
    if (zop.kind != Kind_zCHAINID) goto case_2535;
    unit z3zE2440;
    {
      z3zE2440 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2356;
    z2zE2356 = zk_env(zF_ChainId);
    {
      z3zE2432 = zpush_word(z2zE2356);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2535: ;
  {
    if (zop.kind != Kind_zBASEFEE) goto case_2534;
    unit z3zE2438;
    {
      z3zE2438 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2357;
    z2zE2357 = zk_env(zF_BaseFee);
    {
      z3zE2432 = zpush_word(z2zE2357);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2534: ;
  {
    if (zop.kind != Kind_zBLOBBASEFEE) goto case_2533;
    unit z3zE2436;
    {
      z3zE2436 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2359;
    {
      uint64_t z2zE2358;
      z2zE2358 = zk_header.zexcess_blob_gas;
      {
        z2zE2359 = zblob_base_fee(z2zE2358);
        if (have_exception) {  goto end_block_exception_2544;  }
      }
    }
    {
      z3zE2432 = zpush_word(z2zE2359);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2533: ;
  {
    if (zop.kind != Kind_zBLOBHASH) goto case_2532;
    unit z3zE2434;
    {
      z3zE2434 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    sail_u256 z2zE2361;
    {
      sail_u256 z2zE2360;
      {
        z2zE2360 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2544;  }
      }
      z2zE2361 = zk_blobhash(z2zE2360);
    }
    {
      z3zE2432 = zpush_word(z2zE2361);
      if (have_exception) {  goto end_block_exception_2544;  }
    }
    goto finish_match_2530;
  }
case_2532: ;
  {
    z3zE2432 = sail_assert(false, "sail/evm/execute.sail:432.21-432.22");
    goto finish_match_2530;
  }
case_2531: ;
finish_match_2530: ;
  z8zE421 = z3zE2432;
end_function_2543: ;
  return z8zE421;
end_block_exception_2544: ;

  return UNIT;
}

unit zexecute_memory(struct zast zop)
{
  unit z8zE422;
  unit z3zE2407;
  {
    if (zop.kind != Kind_zPOP) goto case_2525;
    unit z3zE2423;
    {
      z3zE2423 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 z2zE2328;
    {
      z2zE2328 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    unit z3zE2424;
    {
      z3zE2424 = UNIT;
      goto finish_match_2526;
    }
  case_2527: ;
    sail_match_failure("execute_memory");
  finish_match_2526: ;
    z3zE2407 = z3zE2424;
    goto finish_match_2516;
  }
case_2525: ;
  {
    if (zop.kind != Kind_zMLOAD) goto case_2524;
    unit z3zE2421;
    {
      z3zE2421 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zoffset_word, UINT64_C(32));
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    bool z2zE2329;
    z2zE2329 = zis_running(UNIT);
    if (z2zE2329) {
      sail_u256 z2zE2331;
      {
        uint64_t z2zE2330;
        z2zE2330 = zrange.zoff;
        z2zE2331 = zmem_load(z2zE2330);
      }
      {
        z3zE2407 = zpush_word(z2zE2331);
        if (have_exception) {  goto end_block_exception_2529;  }
      }
    } else {  z3zE2407 = UNIT;  }
    goto finish_match_2516;
  }
case_2524: ;
  {
    if (zop.kind != Kind_zMSTORE) goto case_2523;
    unit z3zE2419;
    {
      z3zE2419 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 z3zE2427;
    {
      z3zE2427 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 zv;
    {
      zv = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    struct zMemoryRangeFields z3zE2428;
    {
      z3zE2428 = zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z3zE2427, UINT64_C(32));
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    bool z2zE2332;
    z2zE2332 = zis_running(UNIT);
    if (z2zE2332) {
      uint64_t z2zE2333;
      z2zE2333 = z3zE2428.zoff;
      z3zE2407 = zmem_store(z2zE2333, zv);
    } else {  z3zE2407 = UNIT;  }
    goto finish_match_2516;
  }
case_2523: ;
  {
    if (zop.kind != Kind_zMSTORE8) goto case_2522;
    unit z3zE2417;
    {
      z3zE2417 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 z3zE2429;
    {
      z3zE2429 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 z3zE2430;
    {
      z3zE2430 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    struct zMemoryRangeFields z3zE2431;
    {
      z3zE2431 = zcharge_memory_range(z3zE2429, zWORD_ONE);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    bool z2zE2334;
    z2zE2334 = zis_running(UNIT);
    if (z2zE2334) {
      uint64_t z2zE2335;
      z2zE2335 = z3zE2431.zoff;
      z3zE2407 = zmem_store_byte(z2zE2335, z3zE2430);
    } else {  z3zE2407 = UNIT;  }
    goto finish_match_2516;
  }
case_2522: ;
  {
    if (zop.kind != Kind_zMSIZE) goto case_2521;
    unit z3zE2415;
    {
      z3zE2415 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 z2zE2339;
    {
      sail_u128 z2zE2338;
      {
        uint64_t z2zE2337;
        {
          uint64_t z2zE2336;
          z2zE2336 = zevm_memory_high_water(UNIT);
          z2zE2337 = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2336);
        }
        z2zE2338 = u128_mul_u64_u64(z2zE2337, UINT64_C(32));
      }
      z2zE2339 = zword_of_nat_byte_countzIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2338);
    }
    {
      z3zE2407 = zpush_word(z2zE2339);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    goto finish_match_2516;
  }
case_2521: ;
  {
    if (zop.kind != Kind_zMCOPY) goto case_2518;
    unit z3zE2409;
    {
      z3zE2409 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 zdestination_word;
    {
      zdestination_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 zsource_word;
    {
      zsource_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    unit z3zE2410;
    {
      z3zE2410 = zcharge_copy_gas(zlength_word);
      if (have_exception) {  goto end_block_exception_2529;  }
    }
    bool z2zE2340;
    z2zE2340 = zis_running(UNIT);
    if (z2zE2340) {
      uint64_t zavailable;
      zavailable = zgas_remaining;
      struct zMemoryPairExpansion zexpansion;
      {
        zexpansion = zmemory_pair_expansion(zdestination_word, zlength_word, zsource_word, zlength_word, zavailable);
        if (have_exception) {  goto end_block_exception_2529;  }
      }
      uint64_t z2zE2341;
      z2zE2341 = zexpansion.zcost;
      unit z3zE2412;
      {
        z3zE2412 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2341);
        if (have_exception) {  goto end_block_exception_2529;  }
      }
      bool z2zE2342;
      z2zE2342 = zis_running(UNIT);
      if (z2zE2342) {
        struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z2zE2343;
        z2zE2343 = zapply_memory_pair_expansion(zexpansion);
        unit z3zE2413;
        {
          struct zMemoryRangeFields zdestination;
          zdestination = z2zE2343.ztup0;
          struct zMemoryRangeFields zsource;
          zsource = z2zE2343.ztup1;
          uint64_t z2zE2344;
          z2zE2344 = zdestination.zoff;
          uint64_t z2zE2345;
          z2zE2345 = zsource.zoff;
          uint64_t z2zE2346;
          z2zE2346 = zdestination.zlen;
          z3zE2413 = zmem_mcopy(z2zE2344, z2zE2345, z2zE2346);
          goto finish_match_2519;
        }
      case_2520: ;
        sail_match_failure("execute_memory");
      finish_match_2519: ;
        z3zE2407 = z3zE2413;
      } else {  z3zE2407 = UNIT;  }
    } else {  z3zE2407 = UNIT;  }
    goto finish_match_2516;
  }
case_2518: ;
  {
    z3zE2407 = sail_assert(false, "sail/evm/execute.sail:488.21-488.22");
    goto finish_match_2516;
  }
case_2517: ;
finish_match_2516: ;
  z8zE422 = z3zE2407;
end_function_2528: ;
  return z8zE422;
end_block_exception_2529: ;

  return UNIT;
}

unit zexecute_storage(struct zast zop)
{
  unit z8zE423;
  unit z3zE2384;
  {
    if (zop.kind != Kind_zSLOAD) goto case_2511;
    sail_u256 zs;
    {
      zs = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    bool zwarm;
    {
      sail_fixed_bytes_20 z2zE2296;
      z2zE2296 = zself_addr(UNIT);
      zwarm = zk_slot_is_warm(z2zE2296, zs);
    }
    uint64_t z2zE2291;
    z2zE2291 = zsload_cost(zwarm);
    unit z3zE2400;
    {
      z3zE2400 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2291);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    bool z2zE2292;
    z2zE2292 = zis_running(UNIT);
    if (z2zE2292) {
      sail_u256 z2zE2295;
      {
        struct zStorageValue z2zE2294;
        {
          sail_fixed_bytes_20 z2zE2293;
          z2zE2293 = zself_addr(UNIT);
          {
            z2zE2294 = zk_sload(z2zE2293, zs);
            if (have_exception) {  goto end_block_exception_2515;  }
          }
        }
        z2zE2295 = z2zE2294.zcurr;
      }
      {
        z3zE2384 = zpush_word(z2zE2295);
        if (have_exception) {  goto end_block_exception_2515;  }
      }
    } else {  z3zE2384 = UNIT;  }
    goto finish_match_2506;
  }
case_2511: ;
  {
    if (zop.kind != Kind_zSSTORE) goto case_2510;
    bool z2zE2297;
    {
      z2zE2297 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    if (z2zE2297) {  z3zE2384 = UNIT;  } else {
      bool z2zE2299;
      {
        bool z2zE2298;
        z2zE2298 = zfork_lt(zk_fork, zAmsterdam);
        bool z3zE2391;
        if (z2zE2298) {  z3zE2391 = (!(zG_callstipend < zgas_remaining));  } else {  z3zE2391 = false;  }
        z2zE2299 = z3zE2391;
      }
      if (z2zE2299) {
        {
          z3zE2384 = zexc_halt(zOutOfGas);
          if (have_exception) {  goto end_block_exception_2515;  }
        }
      } else {
        sail_u256 z3zE2402;
        {
          z3zE2402 = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2515;  }
        }
        sail_u256 zv;
        {
          zv = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2515;  }
        }
        bool z2zE2300;
        z2zE2300 = zis_running(UNIT);
        if (z2zE2300) {
          bool z3zE2403;
          {
            sail_fixed_bytes_20 z2zE2323;
            z2zE2323 = zself_addr(UNIT);
            z3zE2403 = zk_slot_is_warm(z2zE2323, z3zE2402);
          }
          bool zcold;
          zcold = not(z3zE2403);
          bool z2zE2301;
          z2zE2301 = zfork_gteq(zk_fork, zAmsterdam);
          unit z3zE2393;
          if (z2zE2301) {
            uint64_t zaccess_cost;
            zaccess_cost = zamsterdam_storage_access_cost(zcold);
            uint64_t zsentry_cost;
            {
              bool z2zE2302;
              z2zE2302 = (zaccess_cost < zG_sstore_sentry);
              if (z2zE2302) {  zsentry_cost = zG_sstore_sentry;  } else {  zsentry_cost = zaccess_cost;  }
            }
            {
              z3zE2393 = zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zsentry_cost);
              if (have_exception) {  goto end_block_exception_2515;  }
            }
          } else {  z3zE2393 = UNIT;  }
          bool z2zE2304;
          {
            bool z2zE2303;
            z2zE2303 = zis_running(UNIT);
            z2zE2304 = not(z2zE2303);
          }
          unit z3zE2392;
          if (z2zE2304) {
            z8zE423 = UNIT;
            goto cleanup_2513;
            /* unreachable after return */
            goto end_cleanup_2514;
          cleanup_2513: ;
            goto end_function_2512;
          end_cleanup_2514: ;
          } else {  z3zE2392 = UNIT;  }
          struct zStorageValue zentry;
          {
            sail_fixed_bytes_20 z2zE2322;
            z2zE2322 = zself_addr(UNIT);
            {
              zentry = zk_sload(z2zE2322, z3zE2402);
              if (have_exception) {  goto end_block_exception_2515;  }
            }
          }
          struct zSstoreCosts zcosts;
          {
            sail_u256 z2zE2320;
            z2zE2320 = zentry.zorig;
            sail_u256 z2zE2321;
            z2zE2321 = zentry.zcurr;
            {
              zcosts = zsstore_costs(z2zE2320, z2zE2321, zv, zcold);
              if (have_exception) {  goto end_block_exception_2515;  }
            }
          }
          bool z2zE2306;
          {
            uint64_t z2zE2305;
            z2zE2305 = zcosts.zstate_credit;
            z2zE2306 = (z2zE2305 != UINT64_C(0));
          }
          unit z3zE2397;
          if (z2zE2306) {
            uint64_t z2zE2307;
            z2zE2307 = zcosts.zstate_credit;
            {
              z3zE2397 = zcredit_state_gas_refund(z2zE2307);
              if (have_exception) {  goto end_block_exception_2515;  }
            }
          } else {  z3zE2397 = UNIT;  }
          uint64_t z2zE2308;
          z2zE2308 = zcosts.zexecution;
          unit z3zE2396;
          {
            z3zE2396 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2308);
            if (have_exception) {  goto end_block_exception_2515;  }
          }
          uint64_t z2zE2309;
          z2zE2309 = zcosts.zstate_charge;
          unit z3zE2395;
          {
            z3zE2395 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2309);
            if (have_exception) {  goto end_block_exception_2515;  }
          }
          bool z2zE2312;
          {
            bool z2zE2311;
            {
              __int128 z2zE2310;
              z2zE2310 = zcosts.zrefund;
              z2zE2311 = (z2zE2310 == zGAS_REFUND_ZERO);
            }
            z2zE2312 = not(z2zE2311);
          }
          unit z3zE2394;
          if (z2zE2312) {
            __int128 z2zE2313;
            z2zE2313 = zcosts.zrefund;
            {
              z3zE2394 = zrecord_refund(z2zE2313);
              if (have_exception) {  goto end_block_exception_2515;  }
            }
          } else {  z3zE2394 = UNIT;  }
          bool z2zE2316;
          {
            bool z2zE2315;
            z2zE2315 = zis_running(UNIT);
            bool z3zE2398;
            if (z2zE2315) {
              sail_u256 z2zE2314;
              z2zE2314 = zentry.zcurr;
              z3zE2398 = (!eq_u256(z2zE2314, zv));
            } else {  z3zE2398 = false;  }
            z2zE2316 = z3zE2398;
          }
          if (z2zE2316) {
            sail_fixed_bytes_20 z2zE2318;
            z2zE2318 = zself_addr(UNIT);
            struct zStorageValue z2zE2319;
            {
              sail_u256 z2zE2317;
              z2zE2317 = zentry.zorig;
              struct zStorageValue z3zE2399;
              z3zE2399.zcurr = zv;
              z3zE2399.zorig = z2zE2317;
              z2zE2319 = z3zE2399;
            }
            z3zE2384 = zk_sstore(z2zE2318, z3zE2402, z2zE2319);
          } else {  z3zE2384 = UNIT;  }
        } else {  z3zE2384 = UNIT;  }
      }
    }
    goto finish_match_2506;
  }
case_2510: ;
  {
    if (zop.kind != Kind_zTLOAD) goto case_2509;
    unit z3zE2388;
    {
      z3zE2388 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_warm_access);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    sail_u256 z3zE2404;
    {
      z3zE2404 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    sail_u256 z2zE2325;
    {
      sail_fixed_bytes_20 z2zE2324;
      z2zE2324 = zself_addr(UNIT);
      z2zE2325 = zk_tload(z2zE2324, z3zE2404);
    }
    {
      z3zE2384 = zpush_word(z2zE2325);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    goto finish_match_2506;
  }
case_2509: ;
  {
    if (zop.kind != Kind_zTSTORE) goto case_2508;
    bool z2zE2326;
    {
      z2zE2326 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2515;  }
    }
    if (z2zE2326) {  z3zE2384 = UNIT;  } else {
      unit z3zE2387;
      {
        z3zE2387 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_warm_access);
        if (have_exception) {  goto end_block_exception_2515;  }
      }
      sail_u256 z3zE2405;
      {
        z3zE2405 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2515;  }
      }
      sail_u256 z3zE2406;
      {
        z3zE2406 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2515;  }
      }
      sail_fixed_bytes_20 z2zE2327;
      z2zE2327 = zself_addr(UNIT);
      z3zE2384 = zk_tstore(z2zE2327, z3zE2405, z3zE2406);
    }
    goto finish_match_2506;
  }
case_2508: ;
  {
    z3zE2384 = sail_assert(false, "sail/evm/execute.sail:556.21-556.22");
    goto finish_match_2506;
  }
case_2507: ;
finish_match_2506: ;
  z8zE423 = z3zE2384;
end_function_2512: ;
  return z8zE423;
end_block_exception_2515: ;

  return UNIT;
}

unit zexecute_control(struct zast zop)
{
  unit z8zE424;
  unit z3zE2346;
  {
    if (zop.kind != Kind_zJUMP) goto case_2503;
    unit z3zE2372;
    {
      z3zE2372 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    sail_u256 zdest;
    {
      zdest = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    {
      z3zE2346 = zdo_jump(zdest);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    goto finish_match_2489;
  }
case_2503: ;
  {
    if (zop.kind != Kind_zJUMPI) goto case_2502;
    unit z3zE2370;
    {
      z3zE2370 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_high);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    sail_u256 z3zE2374;
    {
      z3zE2374 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    sail_u256 zcond;
    {
      zcond = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    bool z2zE2260;
    z2zE2260 = zword_is_zzero(zcond);
    if (z2zE2260) {  z3zE2346 = UNIT;  } else {
      {
        z3zE2346 = zdo_jump(z3zE2374);
        if (have_exception) {  goto end_block_exception_2505;  }
      }
    }
    goto finish_match_2489;
  }
case_2502: ;
  {
    if (zop.kind != Kind_zPC) goto case_2501;
    unit z3zE2368;
    {
      z3zE2368 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    sail_u256 z2zE2262;
    {
      sail_u256 z2zE2261;
      z2zE2261 = zword_of_source_byte_count(zpc);
      z2zE2262 = zalu_sub(z2zE2261, zWORD_ONE);
    }
    {
      z3zE2346 = zpush_word(z2zE2262);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    goto finish_match_2489;
  }
case_2501: ;
  {
    if (zop.kind != Kind_zGAS) goto case_2500;
    unit z3zE2366;
    {
      z3zE2366 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    {
      z3zE2346 = zpush_gas(zgas_remaining);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    goto finish_match_2489;
  }
case_2500: ;
  {
    if (zop.kind != Kind_zJUMPDEST) goto case_2499;
    {
      z3zE2346 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_jumpdest);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    goto finish_match_2489;
  }
case_2499: ;
  {
    if (zop.kind != Kind_zPUSH) goto case_2498;
    uint64_t zn;
    zn = zop.variants.zPUSH.ztup0;
    sail_u256 zv;
    zv = zop.variants.zPUSH.ztup1;
    bool z2zE2263;
    z2zE2263 = (zn == UINT64_C(0));
    unit z3zE2363;
    if (z2zE2263) {
      {
        z3zE2363 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
        if (have_exception) {  goto end_block_exception_2505;  }
      }
    } else {
      {
        z3zE2363 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
        if (have_exception) {  goto end_block_exception_2505;  }
      }
    }
    {
      z3zE2346 = zpush_word(zv);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    goto finish_match_2489;
  }
case_2498: ;
  {
    if (zop.kind != Kind_zDUP) goto case_2497;
    uint64_t z3zE2375;
    z3zE2375 = zop.variants.zDUP;
    unit z3zE2361;
    {
      z3zE2361 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    bool z2zE2265;
    {
      uint64_t z2zE2264;
      z2zE2264 = zstack_height(UNIT);
      z2zE2265 = (z2zE2264 < z3zE2375);
    }
    if (z2zE2265) {
      {
        z3zE2346 = zexc_halt(zStackUnderflow);
        if (have_exception) {  goto end_block_exception_2505;  }
      }
    } else {
      sail_u256 z2zE2267;
      {
        uint64_t z2zE2266;
        {    z2zE2266 = (z3zE2375 - UINT64_C(1));
        }
        z2zE2267 = zpeek(z2zE2266);
      }
      {
        z3zE2346 = zpush_word(z2zE2267);
        if (have_exception) {  goto end_block_exception_2505;  }
      }
    }
    goto finish_match_2489;
  }
case_2497: ;
  {
    if (zop.kind != Kind_zSWAP) goto case_2496;
    uint64_t z3zE2376;
    z3zE2376 = zop.variants.zSWAP;
    unit z3zE2358;
    {
      z3zE2358 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    bool z2zE2270;
    {
      uint64_t z2zE2268;
      z2zE2268 = zstack_height(UNIT);
      uint64_t z2zE2269;
      {    z2zE2269 = (z3zE2376 + UINT64_C(1));
      }
      z2zE2270 = (z2zE2268 < z2zE2269);
    }
    if (z2zE2270) {
      {
        z3zE2346 = zexc_halt(zStackUnderflow);
        if (have_exception) {  goto end_block_exception_2505;  }
      }
    } else {
      sail_u256 ztop;
      ztop = zpeek(UINT64_C(0));
      sail_u256 zother;
      zother = zpeek(z3zE2376);
      unit z3zE2360;
      z3zE2360 = zstack_set(UINT64_C(0), zother);
      z3zE2346 = zstack_set(z3zE2376, ztop);
    }
    goto finish_match_2489;
  }
case_2496: ;
  {
    if (zop.kind != Kind_zDUPN) goto case_2495;
    uint64_t zimmediate;
    zimmediate = zop.variants.zDUPN;
    unit z3zE2356;
    {
      z3zE2356 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    bool z2zE2271;
    z2zE2271 = zis_running(UNIT);
    if (z2zE2271) {
      bool z2zE2273;
      {
        bool z2zE2272;
        z2zE2272 = zdeep_stack_immediate_valid(zimmediate);
        z2zE2273 = not(z2zE2272);
      }
      if (z2zE2273) {
        {
          z3zE2346 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2505;  }
        }
      } else {
        uint64_t z3zE2377;
        z3zE2377 = zdecode_single_stack_index(zimmediate);
        bool z2zE2275;
        {
          uint64_t z2zE2274;
          z2zE2274 = zstack_height(UNIT);
          z2zE2275 = (z2zE2274 < z3zE2377);
        }
        if (z2zE2275) {
          {
            z3zE2346 = zexc_halt(zStackUnderflow);
            if (have_exception) {  goto end_block_exception_2505;  }
          }
        } else {
          sail_u256 z2zE2277;
          {
            uint64_t z2zE2276;
            {    z2zE2276 = (z3zE2377 - UINT64_C(1));
            }
            z2zE2277 = zpeek(z2zE2276);
          }
          {
            z3zE2346 = zpush_word(z2zE2277);
            if (have_exception) {  goto end_block_exception_2505;  }
          }
        }
      }
    } else {  z3zE2346 = UNIT;  }
    goto finish_match_2489;
  }
case_2495: ;
  {
    if (zop.kind != Kind_zSWAPN) goto case_2494;
    uint64_t z3zE2378;
    z3zE2378 = zop.variants.zSWAPN;
    unit z3zE2353;
    {
      z3zE2353 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    bool z2zE2278;
    z2zE2278 = zis_running(UNIT);
    if (z2zE2278) {
      bool z2zE2280;
      {
        bool z2zE2279;
        z2zE2279 = zdeep_stack_immediate_valid(z3zE2378);
        z2zE2280 = not(z2zE2279);
      }
      if (z2zE2280) {
        {
          z3zE2346 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2505;  }
        }
      } else {
        uint64_t z3zE2379;
        z3zE2379 = zdecode_single_stack_index(z3zE2378);
        bool z2zE2283;
        {
          uint64_t z2zE2281;
          z2zE2281 = zstack_height(UNIT);
          uint64_t z2zE2282;
          {    z2zE2282 = (z3zE2379 + UINT64_C(1));
          }
          z2zE2283 = (z2zE2281 < z2zE2282);
        }
        if (z2zE2283) {
          {
            z3zE2346 = zexc_halt(zStackUnderflow);
            if (have_exception) {  goto end_block_exception_2505;  }
          }
        } else {
          sail_u256 z3zE2380;
          z3zE2380 = zpeek(UINT64_C(0));
          sail_u256 z3zE2381;
          z3zE2381 = zpeek(z3zE2379);
          unit z3zE2355;
          z3zE2355 = zstack_set(UINT64_C(0), z3zE2381);
          z3zE2346 = zstack_set(z3zE2379, z3zE2380);
        }
      }
    } else {  z3zE2346 = UNIT;  }
    goto finish_match_2489;
  }
case_2494: ;
  {
    if (zop.kind != Kind_zEXCHANGE) goto case_2491;
    uint64_t z3zE2382;
    z3zE2382 = zop.variants.zEXCHANGE;
    unit z3zE2348;
    {
      z3zE2348 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2505;  }
    }
    bool z2zE2284;
    z2zE2284 = zis_running(UNIT);
    if (z2zE2284) {
      bool z2zE2286;
      {
        bool z2zE2285;
        z2zE2285 = zexchange_immediate_valid(z3zE2382);
        z2zE2286 = not(z2zE2285);
      }
      if (z2zE2286) {
        {
          z3zE2346 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2505;  }
        }
      } else {
        struct ztuple_z8z5u64zCz0z5u64z9 z2zE2287;
        z2zE2287 = zdecode_exchange_stack_indices(z3zE2382);
        unit z3zE2350;
        {
          uint64_t z3zE2383;
          z3zE2383 = z2zE2287.ztup0;
          uint64_t zm;
          zm = z2zE2287.ztup1;
          bool z2zE2290;
          {
            uint64_t z2zE2288;
            z2zE2288 = zstack_height(UNIT);
            uint64_t z2zE2289;
            {    z2zE2289 = (zm + UINT64_C(1));
            }
            z2zE2290 = (z2zE2288 < z2zE2289);
          }
          if (z2zE2290) {
            {
              z3zE2350 = zexc_halt(zStackUnderflow);
              if (have_exception) {  goto end_block_exception_2505;  }
            }
          } else {
            sail_u256 zfirst;
            zfirst = zpeek(z3zE2383);
            sail_u256 zsecond;
            zsecond = zpeek(zm);
            unit z3zE2352;
            z3zE2352 = zstack_set(z3zE2383, zsecond);
            z3zE2350 = zstack_set(zm, zfirst);
          }
          goto finish_match_2492;
        }
      case_2493: ;
        sail_match_failure("execute_control");
      finish_match_2492: ;
        z3zE2346 = z3zE2350;
      }
    } else {  z3zE2346 = UNIT;  }
    goto finish_match_2489;
  }
case_2491: ;
  {
    z3zE2346 = sail_assert(false, "sail/evm/execute.sail:663.21-663.22");
    goto finish_match_2489;
  }
case_2490: ;
finish_match_2489: ;
  z8zE424 = z3zE2346;
end_function_2504: ;
  return z8zE424;
end_block_exception_2505: ;

  return UNIT;
}

unit zexecute_log(struct zast zop)
{
  unit z8zE425;
  unit z3zE2342;
  {
    if (zop.kind != Kind_zLOG) goto case_2486;
    uint64_t zn;
    zn = zop.variants.zLOG;
    bool z2zE2253;
    {
      z2zE2253 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2488;  }
    }
    if (z2zE2253) {  z3zE2342 = UNIT;  } else {
      sail_u256 zoffset_word;
      {
        zoffset_word = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2488;  }
      }
      sail_u256 zlength_word;
      {
        zlength_word = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2488;  }
      }
      zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
      {
        zpop_log_topics(&ztopics, zn);
        if (have_exception) {
          KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
          goto end_block_exception_2488;
        }
      }
      unit z3zE2345;
      {
        z3zE2345 = zcharge_log_gas(zn, zlength_word);
        if (have_exception) {
          KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
          goto end_block_exception_2488;
        }
      }
      struct zMemoryRangeFields zrange;
      {
        zrange = zcharge_memory_range(zoffset_word, zlength_word);
        if (have_exception) {
          KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
          goto end_block_exception_2488;
        }
      }
      bool z2zE2254;
      z2zE2254 = zis_running(UNIT);
      if (z2zE2254) {
        sail_fixed_bytes_20 z2zE2258;
        z2zE2258 = zself_addr(UNIT);
        struct zBytes z2zE2259;
        CREATE(zBytes)(&z2zE2259);
        {
          struct zByteSliceFields z2zE2257;
          {
            uint64_t z2zE2255;
            z2zE2255 = zrange.zoff;
            uint64_t z2zE2256;
            z2zE2256 = zrange.zlen;
            z2zE2257 = zmemory_byte_slice(z2zE2255, z2zE2256);
          }
          zBytesSlice(&z2zE2259, z2zE2257);
        }
        z3zE2342 = zk_log(z2zE2258, ztopics, z2zE2259);
        KILL(zBytes)(&z2zE2259);
      } else {  z3zE2342 = UNIT;  }
      KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
    }
    goto finish_match_2484;
  }
case_2486: ;
  {
    z3zE2342 = sail_assert(false, "sail/evm/execute.sail:682.21-682.22");
    goto finish_match_2484;
  }
case_2485: ;
finish_match_2484: ;
  z8zE425 = z3zE2342;
end_function_2487: ;
  return z8zE425;
end_block_exception_2488: ;

  return UNIT;
}

unit zexecute_halt(struct zast zop)
{
  unit z8zE426;
  unit z3zE2310;
  {
    if (zop.kind != Kind_zSTOP) goto case_2481;
    struct zHaltKind z2zE2205;
    CREATE(zHaltKind)(&z2zE2205);
    zHaltStop(&z2zE2205, UNIT);
    zHalted(&zframe_status, z2zE2205);
    z3zE2310 = UNIT;
    KILL(zHaltKind)(&z2zE2205);
    goto finish_match_2475;
  }
case_2481: ;
  {
    if (zop.kind != Kind_zRETURN) goto case_2480;
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_range(zoffset_word, zlength_word);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    bool z2zE2206;
    z2zE2206 = zis_running(UNIT);
    if (z2zE2206) {
      struct zHaltKind z2zE2211;
      CREATE(zHaltKind)(&z2zE2211);
      {
        struct zByteSliceFields z2zE2210;
        {
          struct zByteSliceFields z2zE2209;
          {
            uint64_t z2zE2207;
            z2zE2207 = zrange.zoff;
            uint64_t z2zE2208;
            z2zE2208 = zrange.zlen;
            z2zE2209 = zmemory_byte_slice(z2zE2207, z2zE2208);
          }
          z2zE2210 = zfreezze_output(z2zE2209);
        }
        zHaltReturn(&z2zE2211, z2zE2210);
      }
      zHalted(&zframe_status, z2zE2211);
      z3zE2310 = UNIT;
      KILL(zHaltKind)(&z2zE2211);
    } else {  z3zE2310 = UNIT;  }
    goto finish_match_2475;
  }
case_2480: ;
  {
    if (zop.kind != Kind_zREVERT) goto case_2479;
    sail_u256 z3zE2337;
    {
      z3zE2337 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    sail_u256 z3zE2338;
    {
      z3zE2338 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    struct zMemoryRangeFields z3zE2339;
    {
      z3zE2339 = zcharge_memory_range(z3zE2337, z3zE2338);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    bool z2zE2212;
    z2zE2212 = zis_running(UNIT);
    if (z2zE2212) {
      unit z3zE2334;
      {
        z3zE2334 = zrefill_frame_state_gas(UNIT);
        if (have_exception) {  goto end_block_exception_2483;  }
      }
      struct zHaltKind z2zE2217;
      CREATE(zHaltKind)(&z2zE2217);
      {
        struct zByteSliceFields z2zE2216;
        {
          struct zByteSliceFields z2zE2215;
          {
            uint64_t z2zE2213;
            z2zE2213 = z3zE2339.zoff;
            uint64_t z2zE2214;
            z2zE2214 = z3zE2339.zlen;
            z2zE2215 = zmemory_byte_slice(z2zE2213, z2zE2214);
          }
          z2zE2216 = zfreezze_output(z2zE2215);
        }
        zHaltRevert(&z2zE2217, z2zE2216);
      }
      zHalted(&zframe_status, z2zE2217);
      z3zE2310 = UNIT;
      KILL(zHaltKind)(&z2zE2217);
    } else {  z3zE2310 = UNIT;  }
    goto finish_match_2475;
  }
case_2479: ;
  {
    if (zop.kind != Kind_zINVALID) goto case_2478;
    zgas_remaining = zGAS_ZERO;
    unit z3zE2331;
    z3zE2331 = UNIT;
    {
      z3zE2310 = zexc_halt(zInvalidOpcode);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    goto finish_match_2475;
  }
case_2478: ;
  {
    if (zop.kind != Kind_zSELFDESTRUCT) goto case_2477;
    bool z2zE2218;
    {
      z2zE2218 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2483;  }
    }
    if (z2zE2218) {  z3zE2310 = UNIT;  } else {
      sail_fixed_bytes_20 zbeneficiary;
      {
        sail_u256 z2zE2252;
        {
          z2zE2252 = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2483;  }
        }
        zbeneficiary = zword_to_address(z2zE2252);
      }
      bool z2zE2219;
      z2zE2219 = zfork_gteq(zk_fork, zAmsterdam);
      if (z2zE2219) {
        bool zwarm;
        zwarm = zk_access_account(zbeneficiary);
        uint64_t zaccess_cost;
        {
          uint64_t z2zE2231;
          {    z2zE2231 = (UINT64_C(0) + zG_selfdestruct);
          }
          uint64_t z2zE2232;
          if (zwarm) {  z2zE2232 = zG_zzero;  } else {  z2zE2232 = zG_amsterdam_cold_account_access;  }
          {    zaccess_cost = (z2zE2231 + z2zE2232);
          }
        }
        unit z3zE2324;
        {
          z3zE2324 = zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zaccess_cost);
          if (have_exception) {  goto end_block_exception_2483;  }
        }
        bool z2zE2220;
        z2zE2220 = zis_running(UNIT);
        if (z2zE2220) {
          sail_u256 zbal;
          {
            sail_fixed_bytes_20 z2zE2230;
            z2zE2230 = zself_addr(UNIT);
            {
              zbal = zk_get_balance(z2zE2230);
              if (have_exception) {  goto end_block_exception_2483;  }
            }
          }
          bool zcreates_account;
          {
            bool z2zE2229;
            z2zE2229 = zword_nonzzero(zbal);
            bool z3zE2325;
            if (z2zE2229) {
              {
                z3zE2325 = zk_account_is_empty(zbeneficiary);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            } else {  z3zE2325 = false;  }
            zcreates_account = z3zE2325;
          }
          uint64_t zexecution_cost;
          if (zcreates_account) {
            {    zexecution_cost = (zaccess_cost + zG_amsterdam_account_write);
            }
          } else {  zexecution_cost = zaccess_cost;  }
          unit z3zE2328;
          {
            z3zE2328 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zexecution_cost);
            if (have_exception) {  goto end_block_exception_2483;  }
          }
          bool z2zE2222;
          {
            bool z2zE2221;
            z2zE2221 = zis_running(UNIT);
            bool z3zE2326;
            if (z2zE2221) {  z3zE2326 = zcreates_account;  } else {  z3zE2326 = false;  }
            z2zE2222 = z3zE2326;
          }
          unit z3zE2327;
          if (z2zE2222) {
            {
              z3zE2327 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2483;  }
            }
          } else {  z3zE2327 = UNIT;  }
          bool z2zE2223;
          z2zE2223 = zis_running(UNIT);
          if (z2zE2223) {
            sail_fixed_bytes_20 z2zE2224;
            z2zE2224 = zself_addr(UNIT);
            unit z3zE2330;
            {
              z3zE2330 = zk_transfer(z2zE2224, zbeneficiary, zbal);
              if (have_exception) {  goto end_block_exception_2483;  }
            }
            bool z2zE2226;
            {
              sail_fixed_bytes_20 z2zE2225;
              z2zE2225 = zself_addr(UNIT);
              {
                z2zE2226 = zk_was_created(z2zE2225);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            }
            unit z3zE2329;
            if (z2zE2226) {
              sail_fixed_bytes_20 z2zE2227;
              z2zE2227 = zself_addr(UNIT);
              {
                z3zE2329 = zk_selfdestruct(z2zE2227);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            } else {  z3zE2329 = UNIT;  }
            struct zHaltKind z2zE2228;
            CREATE(zHaltKind)(&z2zE2228);
            zHaltSelfDestruct(&z2zE2228, UNIT);
            zHalted(&zframe_status, z2zE2228);
            z3zE2310 = UNIT;
            KILL(zHaltKind)(&z2zE2228);
          } else {  z3zE2310 = UNIT;  }
        } else {  z3zE2310 = UNIT;  }
      } else {
        sail_u256 z3zE2340;
        {
          sail_fixed_bytes_20 z2zE2251;
          z2zE2251 = zself_addr(UNIT);
          {
            z3zE2340 = zk_get_balance(z2zE2251);
            if (have_exception) {  goto end_block_exception_2483;  }
          }
        }
        bool z3zE2341;
        z3zE2341 = zk_access_account(zbeneficiary);
        unit z3zE2317;
        {
          z3zE2317 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_selfdestruct);
          if (have_exception) {  goto end_block_exception_2483;  }
        }
        bool z2zE2233;
        z2zE2233 = not(z3zE2341);
        unit z3zE2316;
        if (z2zE2233) {
          {
            z3zE2316 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_cold_account);
            if (have_exception) {  goto end_block_exception_2483;  }
          }
        } else {  z3zE2316 = UNIT;  }
        bool z2zE2236;
        {
          bool z2zE2235;
          z2zE2235 = zis_running(UNIT);
          bool z3zE2314;
          if (z2zE2235) {
            bool z2zE2234;
            z2zE2234 = zword_nonzzero(z3zE2340);
            bool z3zE2313;
            if (z2zE2234) {
              {
                z3zE2313 = zk_account_is_empty(zbeneficiary);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            } else {  z3zE2313 = false;  }
            z3zE2314 = z3zE2313;
          } else {  z3zE2314 = false;  }
          z2zE2236 = z3zE2314;
        }
        unit z3zE2315;
        if (z2zE2236) {
          {
            z3zE2315 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_newaccount);
            if (have_exception) {  goto end_block_exception_2483;  }
          }
        } else {  z3zE2315 = UNIT;  }
        bool z2zE2237;
        z2zE2237 = zis_running(UNIT);
        if (z2zE2237) {
          bool zfirst_selfdestruct;
          {
            bool z2zE2250;
            {
              sail_fixed_bytes_20 z2zE2249;
              z2zE2249 = zself_addr(UNIT);
              {
                z2zE2250 = zk_is_selfdestructed(z2zE2249);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            }
            zfirst_selfdestruct = not(z2zE2250);
          }
          bool z2zE2239;
          {
            bool z2zE2238;
            z2zE2238 = zfork_lt(zk_fork, zLondon);
            bool z3zE2318;
            if (z2zE2238) {  z3zE2318 = zfirst_selfdestruct;  } else {  z3zE2318 = false;  }
            z2zE2239 = z3zE2318;
          }
          unit z3zE2323;
          if (z2zE2239) {
            {
              z3zE2323 = zrecord_refundzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zR_selfdestruct_pre_london);
              if (have_exception) {  goto end_block_exception_2483;  }
            }
          } else {  z3zE2323 = UNIT;  }
          sail_fixed_bytes_20 z2zE2240;
          z2zE2240 = zself_addr(UNIT);
          unit z3zE2322;
          {
            z3zE2322 = zk_transfer(z2zE2240, zbeneficiary, z3zE2340);
            if (have_exception) {  goto end_block_exception_2483;  }
          }
          bool z2zE2241;
          z2zE2241 = zfork_lt(zk_fork, zCancun);
          unit z3zE2319;
          if (z2zE2241) {
            sail_fixed_bytes_20 z2zE2242;
            z2zE2242 = zself_addr(UNIT);
            unit z3zE2321;
            {
              z3zE2321 = zk_zzero_balance(z2zE2242);
              if (have_exception) {  goto end_block_exception_2483;  }
            }
            sail_fixed_bytes_20 z2zE2243;
            z2zE2243 = zself_addr(UNIT);
            {
              z3zE2319 = zk_selfdestruct(z2zE2243);
              if (have_exception) {  goto end_block_exception_2483;  }
            }
          } else {
            bool z2zE2245;
            {
              sail_fixed_bytes_20 z2zE2244;
              z2zE2244 = zself_addr(UNIT);
              {
                z2zE2245 = zk_was_created(z2zE2244);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            }
            if (z2zE2245) {
              sail_fixed_bytes_20 z2zE2246;
              z2zE2246 = zself_addr(UNIT);
              unit z3zE2320;
              {
                z3zE2320 = zk_zzero_balance(z2zE2246);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
              sail_fixed_bytes_20 z2zE2247;
              z2zE2247 = zself_addr(UNIT);
              {
                z3zE2319 = zk_selfdestruct(z2zE2247);
                if (have_exception) {  goto end_block_exception_2483;  }
              }
            } else {  z3zE2319 = UNIT;  }
          }
          struct zHaltKind z2zE2248;
          CREATE(zHaltKind)(&z2zE2248);
          zHaltSelfDestruct(&z2zE2248, UNIT);
          zHalted(&zframe_status, z2zE2248);
          z3zE2310 = UNIT;
          KILL(zHaltKind)(&z2zE2248);
        } else {  z3zE2310 = UNIT;  }
      }
    }
    goto finish_match_2475;
  }
case_2477: ;
  {
    z3zE2310 = sail_assert(false, "sail/evm/execute.sail:771.21-771.22");
    goto finish_match_2475;
  }
case_2476: ;
finish_match_2475: ;
  z8zE426 = z3zE2310;
end_function_2482: ;
  return z8zE426;
end_block_exception_2483: ;

  return UNIT;
}

unit zexecute_opcode(struct zast zop)
{
  unit z8zE427;
  unit z3zE2221;
  {
    if (zop.kind != Kind_zSTOP) goto case_2472;
    {
      z3zE2221 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2472: ;
  {
    if (zop.kind != Kind_zADD) goto case_2471;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2471: ;
  {
    if (zop.kind != Kind_zMUL) goto case_2470;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2470: ;
  {
    if (zop.kind != Kind_zSUB) goto case_2469;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2469: ;
  {
    if (zop.kind != Kind_zDIV) goto case_2468;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2468: ;
  {
    if (zop.kind != Kind_zSDIV) goto case_2467;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2467: ;
  {
    if (zop.kind != Kind_zMOD) goto case_2466;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2466: ;
  {
    if (zop.kind != Kind_zSMOD) goto case_2465;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2465: ;
  {
    if (zop.kind != Kind_zADDMOD) goto case_2464;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2464: ;
  {
    if (zop.kind != Kind_zMULMOD) goto case_2463;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2463: ;
  {
    if (zop.kind != Kind_zEXP) goto case_2462;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2462: ;
  {
    if (zop.kind != Kind_zSIGNEXTEND) goto case_2461;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2461: ;
  {
    if (zop.kind != Kind_zLT) goto case_2460;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2460: ;
  {
    if (zop.kind != Kind_zGT) goto case_2459;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2459: ;
  {
    if (zop.kind != Kind_zSLT) goto case_2458;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2458: ;
  {
    if (zop.kind != Kind_zSGT) goto case_2457;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2457: ;
  {
    if (zop.kind != Kind_zEQ) goto case_2456;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2456: ;
  {
    if (zop.kind != Kind_zISZERO) goto case_2455;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2455: ;
  {
    if (zop.kind != Kind_zAND) goto case_2454;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2454: ;
  {
    if (zop.kind != Kind_zOR) goto case_2453;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2453: ;
  {
    if (zop.kind != Kind_zXOR) goto case_2452;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2452: ;
  {
    if (zop.kind != Kind_zNOT) goto case_2451;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2451: ;
  {
    if (zop.kind != Kind_zBYTE) goto case_2450;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2450: ;
  {
    if (zop.kind != Kind_zSHL) goto case_2449;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2449: ;
  {
    if (zop.kind != Kind_zSHR) goto case_2448;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2448: ;
  {
    if (zop.kind != Kind_zSAR) goto case_2447;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2447: ;
  {
    if (zop.kind != Kind_zCLZ) goto case_2446;
    {
      z3zE2221 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2446: ;
  {
    if (zop.kind != Kind_zKECCAK256) goto case_2445;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2445: ;
  {
    if (zop.kind != Kind_zADDRESS) goto case_2444;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2444: ;
  {
    if (zop.kind != Kind_zORIGIN) goto case_2443;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2443: ;
  {
    if (zop.kind != Kind_zCALLER) goto case_2442;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2442: ;
  {
    if (zop.kind != Kind_zCALLVALUE) goto case_2441;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2441: ;
  {
    if (zop.kind != Kind_zGASPRICE) goto case_2440;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2440: ;
  {
    if (zop.kind != Kind_zCALLDATASIZE) goto case_2439;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2439: ;
  {
    if (zop.kind != Kind_zCALLDATALOAD) goto case_2438;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2438: ;
  {
    if (zop.kind != Kind_zCALLDATACOPY) goto case_2437;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2437: ;
  {
    if (zop.kind != Kind_zCODESIZE) goto case_2436;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2436: ;
  {
    if (zop.kind != Kind_zCODECOPY) goto case_2435;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2435: ;
  {
    if (zop.kind != Kind_zBALANCE) goto case_2434;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2434: ;
  {
    if (zop.kind != Kind_zSELFBALANCE) goto case_2433;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2433: ;
  {
    if (zop.kind != Kind_zEXTCODESIZE) goto case_2432;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2432: ;
  {
    if (zop.kind != Kind_zEXTCODECOPY) goto case_2431;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2431: ;
  {
    if (zop.kind != Kind_zEXTCODEHASH) goto case_2430;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2430: ;
  {
    if (zop.kind != Kind_zRETURNDATASIZE) goto case_2429;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2429: ;
  {
    if (zop.kind != Kind_zRETURNDATACOPY) goto case_2428;
    {
      z3zE2221 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2428: ;
  {
    if (zop.kind != Kind_zBLOCKHASH) goto case_2427;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2427: ;
  {
    if (zop.kind != Kind_zCOINBASE) goto case_2426;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2426: ;
  {
    if (zop.kind != Kind_zTIMESTAMP) goto case_2425;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2425: ;
  {
    if (zop.kind != Kind_zNUMBER) goto case_2424;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2424: ;
  {
    if (zop.kind != Kind_zSLOTNUM) goto case_2423;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2423: ;
  {
    if (zop.kind != Kind_zPREVRANDAO) goto case_2422;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2422: ;
  {
    if (zop.kind != Kind_zGASLIMIT) goto case_2421;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2421: ;
  {
    if (zop.kind != Kind_zCHAINID) goto case_2420;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2420: ;
  {
    if (zop.kind != Kind_zBASEFEE) goto case_2419;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2419: ;
  {
    if (zop.kind != Kind_zBLOBBASEFEE) goto case_2418;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2418: ;
  {
    if (zop.kind != Kind_zBLOBHASH) goto case_2417;
    {
      z3zE2221 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2417: ;
  {
    if (zop.kind != Kind_zPOP) goto case_2416;
    {
      z3zE2221 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2416: ;
  {
    if (zop.kind != Kind_zMLOAD) goto case_2415;
    {
      z3zE2221 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2415: ;
  {
    if (zop.kind != Kind_zMSTORE) goto case_2414;
    {
      z3zE2221 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2414: ;
  {
    if (zop.kind != Kind_zMSTORE8) goto case_2413;
    {
      z3zE2221 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2413: ;
  {
    if (zop.kind != Kind_zMSIZE) goto case_2412;
    {
      z3zE2221 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2412: ;
  {
    if (zop.kind != Kind_zMCOPY) goto case_2411;
    {
      z3zE2221 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2411: ;
  {
    if (zop.kind != Kind_zSLOAD) goto case_2410;
    {
      z3zE2221 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2410: ;
  {
    if (zop.kind != Kind_zSSTORE) goto case_2409;
    {
      z3zE2221 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2409: ;
  {
    if (zop.kind != Kind_zTLOAD) goto case_2408;
    {
      z3zE2221 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2408: ;
  {
    if (zop.kind != Kind_zTSTORE) goto case_2407;
    {
      z3zE2221 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2407: ;
  {
    if (zop.kind != Kind_zJUMP) goto case_2406;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2406: ;
  {
    if (zop.kind != Kind_zJUMPI) goto case_2405;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2405: ;
  {
    if (zop.kind != Kind_zPC) goto case_2404;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2404: ;
  {
    if (zop.kind != Kind_zGAS) goto case_2403;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2403: ;
  {
    if (zop.kind != Kind_zJUMPDEST) goto case_2402;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2402: ;
  {
    if (zop.kind != Kind_zPUSH) goto case_2401;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2401: ;
  {
    if (zop.kind != Kind_zDUP) goto case_2400;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2400: ;
  {
    if (zop.kind != Kind_zSWAP) goto case_2399;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2399: ;
  {
    if (zop.kind != Kind_zDUPN) goto case_2398;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2398: ;
  {
    if (zop.kind != Kind_zSWAPN) goto case_2397;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2397: ;
  {
    if (zop.kind != Kind_zEXCHANGE) goto case_2396;
    {
      z3zE2221 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2396: ;
  {
    if (zop.kind != Kind_zLOG) goto case_2395;
    {
      z3zE2221 = zexecute_log(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2395: ;
  {
    if (zop.kind != Kind_zCREATE) goto case_2394;
    z3zE2221 = sail_assert(false, "sail/evm/execute.sail:867.28-867.29");
    goto finish_match_2384;
  }
case_2394: ;
  {
    if (zop.kind != Kind_zCREATE2) goto case_2393;
    z3zE2221 = sail_assert(false, "sail/evm/execute.sail:868.29-868.30");
    goto finish_match_2384;
  }
case_2393: ;
  {
    if (zop.kind != Kind_zCALL) goto case_2392;
    z3zE2221 = sail_assert(false, "sail/evm/execute.sail:869.26-869.27");
    goto finish_match_2384;
  }
case_2392: ;
  {
    if (zop.kind != Kind_zCALLCODE) goto case_2391;
    z3zE2221 = sail_assert(false, "sail/evm/execute.sail:870.30-870.31");
    goto finish_match_2384;
  }
case_2391: ;
  {
    if (zop.kind != Kind_zDELEGATECALL) goto case_2390;
    z3zE2221 = sail_assert(false, "sail/evm/execute.sail:871.34-871.35");
    goto finish_match_2384;
  }
case_2390: ;
  {
    if (zop.kind != Kind_zSTATICCALL) goto case_2389;
    z3zE2221 = sail_assert(false, "sail/evm/execute.sail:872.32-872.33");
    goto finish_match_2384;
  }
case_2389: ;
  {
    if (zop.kind != Kind_zRETURN) goto case_2388;
    {
      z3zE2221 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2388: ;
  {
    if (zop.kind != Kind_zREVERT) goto case_2387;
    {
      z3zE2221 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2387: ;
  {
    if (zop.kind != Kind_zINVALID) goto case_2386;
    {
      z3zE2221 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2386: ;
  {
    /* complete */
    {
      z3zE2221 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2474;  }
    }
    goto finish_match_2384;
  }
case_2385: ;
finish_match_2384: ;
  z8zE427 = z3zE2221;
end_function_2473: ;
  return z8zE427;
end_block_exception_2474: ;

  return UNIT;
}

bool zcall_is_delegate(enum zCallKind zkind)
{
  bool z8zE428;
  bool z3zE2218;
  {
    if ((zDelegateCall != zkind)) goto case_2381;
    z3zE2218 = true;
    goto finish_match_2379;
  }
case_2381: ;
  {
    z3zE2218 = false;
    goto finish_match_2379;
  }
case_2380: ;
finish_match_2379: ;
  z8zE428 = z3zE2218;
end_function_2382: ;
  return z8zE428;
end_block_exception_2383: ;

  return false;
}

bool zcall_is_static(enum zCallKind zkind)
{
  bool z8zE429;
  bool z3zE2215;
  {
    if ((zStaticCall != zkind)) goto case_2376;
    z3zE2215 = true;
    goto finish_match_2374;
  }
case_2376: ;
  {
    z3zE2215 = false;
    goto finish_match_2374;
  }
case_2375: ;
finish_match_2374: ;
  z8zE429 = z3zE2215;
end_function_2377: ;
  return z8zE429;
end_block_exception_2378: ;

  return false;
}

bool zcall_takes_value(enum zCallKind zkind)
{
  bool z8zE430;
  bool z3zE2211;
  {
    if ((zCall != zkind)) goto case_2371;
    z3zE2211 = true;
    goto finish_match_2368;
  }
case_2371: ;
  {
    if ((zCallCode != zkind)) goto case_2370;
    z3zE2211 = true;
    goto finish_match_2368;
  }
case_2370: ;
  {
    z3zE2211 = false;
    goto finish_match_2368;
  }
case_2369: ;
finish_match_2368: ;
  z8zE430 = z3zE2211;
end_function_2372: ;
  return z8zE430;
end_block_exception_2373: ;

  return false;
}

bool zcall_transfers_value(enum zCallKind zkind)
{
  bool z8zE431;
  bool z3zE2208;
  {
    if ((zCall != zkind)) goto case_2365;
    z3zE2208 = true;
    goto finish_match_2363;
  }
case_2365: ;
  {
    z3zE2208 = false;
    goto finish_match_2363;
  }
case_2364: ;
finish_match_2363: ;
  z8zE431 = z3zE2208;
end_function_2366: ;
  return z8zE431;
end_block_exception_2367: ;

  return false;
}

bool zcall_uses_target_address(enum zCallKind zkind)
{
  bool z8zE432;
  bool z3zE2204;
  {
    if ((zCall != zkind)) goto case_2360;
    z3zE2204 = true;
    goto finish_match_2357;
  }
case_2360: ;
  {
    if ((zStaticCall != zkind)) goto case_2359;
    z3zE2204 = true;
    goto finish_match_2357;
  }
case_2359: ;
  {
    z3zE2204 = false;
    goto finish_match_2357;
  }
case_2358: ;
finish_match_2357: ;
  z8zE432 = z3zE2204;
end_function_2361: ;
  return z8zE432;
end_block_exception_2362: ;

  return false;
}

struct zCode zexecutable_code(sail_fixed_bytes_20 ztarget, bool zdele, sail_fixed_bytes_20 zdtgt)
{
  struct zCode z8zE433;
  if (zdele) {
    struct zCode zdelegate_code;
    {
      sail_fixed_bytes_32 z2zE2203;
      {
        z2zE2203 = zk_code_key(zdtgt);
        if (have_exception) {  goto end_block_exception_2356;  }
      }
      {
        zdelegate_code = zcode_db_resolve(z2zE2203);
        if (have_exception) {  goto end_block_exception_2356;  }
      }
    }
    bool z2zE2202;
    {
      uint64_t z2zE2201;
      z2zE2201 = zprecompile_number(zdtgt);
      z2zE2202 = (z2zE2201 != UINT64_C(0));
    }
    if (z2zE2202) {  z8zE433 = zEMPTY_CODE;  } else {  z8zE433 = zdelegate_code;  }
  } else {
    sail_fixed_bytes_32 z2zE2204;
    {
      z2zE2204 = zk_code_key(ztarget);
      if (have_exception) {  goto end_block_exception_2356;  }
    }
    {
      z8zE433 = zcode_db_resolve(z2zE2204);
      if (have_exception) {  goto end_block_exception_2356;  }
    }
  }
end_function_2355: ;
  return z8zE433;
end_block_exception_2356: ;
  struct zByteSliceFields z8zE970 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zCode z8zE969 = { .zbytes = z8zE970, .zjumpdests = UINT64_C(0xdeadc0de) };
  return z8zE969;
}

unit zrun_call(enum zCallKind zkind)
{
  unit z8zE434;
  sail_fixed_bytes_20 zcaller;
  zcaller = zself_addr(UNIT);
  sail_u256 zgas_request;
  {
    zgas_request = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  sail_u256 ztarget_word;
  {
    ztarget_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  sail_fixed_bytes_20 ztarget;
  ztarget = zword_to_address(ztarget_word);
  sail_u256 zvalue;
  {
    bool z2zE2200;
    z2zE2200 = zcall_takes_value(zkind);
    if (z2zE2200) {
      {
        zvalue = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
    } else {  zvalue = zWORD_ZERO;  }
  }
  sail_u256 zargs_off_word;
  {
    zargs_off_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  sail_u256 zargs_len_word;
  {
    zargs_len_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  sail_u256 zret_off_word;
  {
    zret_off_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  sail_u256 zret_len_word;
  {
    zret_len_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  bool z2zE2133;
  {
    bool z2zE2132;
    z2zE2132 = zis_running(UNIT);
    z2zE2133 = not(z2zE2132);
  }
  unit z3zE2134;
  if (z2zE2133) {
    z8zE434 = UNIT;
    goto cleanup_2352;
    /* unreachable after return */
    goto end_cleanup_2353;
  cleanup_2352: ;
    goto end_function_2335;
  end_cleanup_2353: ;
  } else {  z3zE2134 = UNIT;  }
  bool z2zE2136;
  {
    bool z2zE2135;
    z2zE2135 = zcall_transfers_value(zkind);
    bool z3zE2131;
    if (z2zE2135) {
      bool z2zE2134;
      z2zE2134 = zword_nonzzero(zvalue);
      bool z3zE2130;
      if (z2zE2134) {  z3zE2130 = zmessage.zis_static;  } else {  z3zE2130 = false;  }
      z3zE2131 = z3zE2130;
    } else {  z3zE2131 = false;  }
    z2zE2136 = z3zE2131;
  }
  unit z3zE2132;
  if (z2zE2136) {
    unit z3zE2133;
    {
      z3zE2133 = zexc_halt(zWriteProtection);
      if (have_exception) {  goto end_block_exception_2354;  }
    }
    z8zE434 = UNIT;
    goto cleanup_2350;
    /* unreachable after return */
    goto end_cleanup_2351;
  cleanup_2350: ;
    goto end_function_2335;
  end_cleanup_2351: ;
  } else {  z3zE2132 = UNIT;  }
  bool zwarm;
  zwarm = zk_access_account(ztarget);
  uint64_t ztarget_cost;
  ztarget_cost = zaccount_cost(zwarm);
  uint64_t ztransfer_cost;
  {
    bool z2zE2199;
    z2zE2199 = zword_nonzzero(zvalue);
    if (z2zE2199) {  ztransfer_cost = zcall_value_cost(UNIT);  } else {  ztransfer_cost = zGAS_CONSTANT_ZERO;  }
  }
  uint64_t zavailable;
  zavailable = zgas_remaining;
  struct zMemoryPairExpansion zmemory;
  {
    zmemory = zmemory_pair_expansion(zargs_off_word, zargs_len_word, zret_off_word, zret_len_word, zavailable);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  bool z2zE2138;
  {
    bool z2zE2137;
    z2zE2137 = zis_running(UNIT);
    z2zE2138 = not(z2zE2137);
  }
  unit z3zE2135;
  if (z2zE2138) {
    z8zE434 = UNIT;
    goto cleanup_2348;
    /* unreachable after return */
    goto end_cleanup_2349;
  cleanup_2348: ;
    goto end_function_2335;
  end_cleanup_2349: ;
  } else {  z3zE2135 = UNIT;  }
  uint64_t zmemory_cost;
  zmemory_cost = zmemory.zcost;
  uint64_t zstatic_base;
  {    zstatic_base = (ztarget_cost + ztransfer_cost);
  }
  uint64_t zbefore_static;
  zbefore_static = zgas_remaining;
  bool z2zE2139;
  z2zE2139 = (zbefore_static < zstatic_base);
  unit z3zE2136;
  if (z2zE2139) {
    unit z3zE2137;
    {
      z3zE2137 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2354;  }
    }
    z8zE434 = UNIT;
    goto cleanup_2346;
    /* unreachable after return */
    goto end_cleanup_2347;
  cleanup_2346: ;
    goto end_function_2335;
  end_cleanup_2347: ;
  } else {  z3zE2136 = UNIT;  }
  uint64_t zafter_static_base;
  {    zafter_static_base = (zbefore_static - zstatic_base);
  }
  bool z2zE2140;
  z2zE2140 = (zafter_static_base < zmemory_cost);
  unit z3zE2138;
  if (z2zE2140) {
    unit z3zE2139;
    {
      z3zE2139 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2354;  }
    }
    z8zE434 = UNIT;
    goto cleanup_2344;
    /* unreachable after return */
    goto end_cleanup_2345;
  cleanup_2344: ;
    goto end_function_2335;
  end_cleanup_2345: ;
  } else {  z3zE2138 = UNIT;  }
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2141;
  {
    z2zE2141 = zk_deleg_target(ztarget);
    if (have_exception) {  goto end_block_exception_2354;  }
  }
  unit z3zE2140;
  {
    bool ztg_deleg;
    ztg_deleg = z2zE2141.ztup0;
    sail_fixed_bytes_20 ztg_target;
    ztg_target = z2zE2141.ztup1;
    uint64_t zdelegation_cost;
    if (ztg_deleg) {
      bool zdw;
      zdw = zk_access_account(ztg_target);
      zdelegation_cost = zaccount_cost(zdw);
    } else {  zdelegation_cost = zGAS_CONSTANT_ZERO;  }
    bool znew_account_charged;
    {
      bool z2zE2198;
      z2zE2198 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE2143;
      if (z2zE2198) {
        bool z2zE2197;
        z2zE2197 = zword_nonzzero(zvalue);
        bool z3zE2142;
        if (z2zE2197) {
          bool z2zE2196;
          z2zE2196 = zcall_transfers_value(zkind);
          bool z3zE2141;
          if (z2zE2196) {
            {
              z3zE2141 = zk_account_is_empty(ztarget);
              if (have_exception) {  goto end_block_exception_2354;  }
            }
          } else {  z3zE2141 = false;  }
          z3zE2142 = z3zE2141;
        } else {  z3zE2142 = false;  }
        z3zE2143 = z3zE2142;
      } else {  z3zE2143 = false;  }
      znew_account_charged = z3zE2143;
    }
    uint64_t zcreate_cost;
    {
      bool z2zE2195;
      {
        bool z2zE2194;
        z2zE2194 = zfork_lt(zk_fork, zAmsterdam);
        bool z3zE2146;
        if (z2zE2194) {
          bool z2zE2193;
          z2zE2193 = zword_nonzzero(zvalue);
          bool z3zE2145;
          if (z2zE2193) {
            bool z2zE2192;
            z2zE2192 = zcall_transfers_value(zkind);
            bool z3zE2144;
            if (z2zE2192) {
              {
                z3zE2144 = zk_account_is_empty(ztarget);
                if (have_exception) {  goto end_block_exception_2354;  }
              }
            } else {  z3zE2144 = false;  }
            z3zE2145 = z3zE2144;
          } else {  z3zE2145 = false;  }
          z3zE2146 = z3zE2145;
        } else {  z3zE2146 = false;  }
        z2zE2195 = z3zE2146;
      }
      if (z2zE2195) {  zcreate_cost = zG_newaccount;  } else {  zcreate_cost = zGAS_CONSTANT_ZERO;  }
    }
    uint64_t zadditional_cost;
    {    zadditional_cost = (zdelegation_cost + zcreate_cost);
    }
    uint64_t zbefore_required;
    zbefore_required = zgas_remaining;
    bool z2zE2142;
    z2zE2142 = (zbefore_required < zstatic_base);
    unit z3zE2147;
    if (z2zE2142) {
      unit z3zE2148;
      {
        z3zE2148 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      z8zE434 = UNIT;
      goto cleanup_2342;
      /* unreachable after return */
      goto end_cleanup_2343;
    cleanup_2342: ;
      goto end_function_2335;
    end_cleanup_2343: ;
    } else {  z3zE2147 = UNIT;  }
    uint64_t zafter_required_base;
    {    zafter_required_base = (zbefore_required - zstatic_base);
    }
    bool z2zE2143;
    z2zE2143 = (zafter_required_base < zmemory_cost);
    unit z3zE2149;
    if (z2zE2143) {
      unit z3zE2150;
      {
        z3zE2150 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      z8zE434 = UNIT;
      goto cleanup_2340;
      /* unreachable after return */
      goto end_cleanup_2341;
    cleanup_2340: ;
      goto end_function_2335;
    end_cleanup_2341: ;
    } else {  z3zE2149 = UNIT;  }
    uint64_t zafter_static_cost;
    {    zafter_static_cost = (zafter_required_base - zmemory_cost);
    }
    bool z2zE2144;
    z2zE2144 = (zafter_static_cost < zadditional_cost);
    unit z3zE2151;
    if (z2zE2144) {
      unit z3zE2152;
      {
        z3zE2152 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      z8zE434 = UNIT;
      goto cleanup_2338;
      /* unreachable after return */
      goto end_cleanup_2339;
    cleanup_2338: ;
      goto end_function_2335;
    end_cleanup_2339: ;
    } else {  z3zE2151 = UNIT;  }
    uint64_t zafter_required;
    {    zafter_required = (zafter_static_cost - zadditional_cost);
    }
    uint64_t zrequired;
    {
      bool z2zE2190;
      z2zE2190 = (!(zbefore_required < zafter_required));
      if (z2zE2190) {
        {    zrequired = (zbefore_required - zafter_required);
        }
      } else {
        struct zexception z2zE2191;
        CREATE(zexception)(&z2zE2191);
        zInvalidBlock(&z2zE2191, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE2191);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/interpreter.sail:472.12-472.48");
        KILL(zexception)(&z2zE2191);
        goto end_block_exception_2354;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2191);
      }
    }
    uint64_t zstipend;
    {
      bool z2zE2189;
      z2zE2189 = zword_nonzzero(zvalue);
      if (z2zE2189) {  zstipend = zG_callstipend;  } else {  zstipend = zGAS_ZERO;  }
    }
    uint64_t zbase_child;
    zbase_child = zGAS_ZERO;
    bool z2zE2145;
    z2zE2145 = zfork_gteq(zk_fork, zAmsterdam);
    unit z3zE2159;
    if (z2zE2145) {
      unit z3zE2163;
      {
        z3zE2163 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      unit z3zE2162;
      if (znew_account_charged) {
        {
          z3zE2162 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
      } else {  z3zE2162 = UNIT;  }
      bool z2zE2146;
      z2zE2146 = zis_running(UNIT);
      if (z2zE2146) {
        {
          zbase_child = zcall_gas_cap_word(zgas_remaining, zgas_request);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
        unit z3zE2164;
        z3zE2164 = UNIT;
        {
          z3zE2159 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase_child);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
      } else {  z3zE2159 = UNIT;  }
    } else {
      uint64_t zavail;
      zavail = zgas_remaining;
      bool z2zE2147;
      z2zE2147 = (zavail < zrequired);
      if (z2zE2147) {  zbase_child = zGAS_ZERO;  } else {
        uint64_t zavailable_after_cost;
        {
          zavailable_after_cost = zgas_sub_or_oog(zavail, zrequired);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
        {
          zbase_child = zcall_gas_cap_word(zavailable_after_cost, zgas_request);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
      }
      unit z3zE2161;
      z3zE2161 = UNIT;
      unit z3zE2160;
      {
        z3zE2160 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      bool z2zE2148;
      z2zE2148 = zis_running(UNIT);
      if (z2zE2148) {
        {
          z3zE2159 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase_child);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
      } else {  z3zE2159 = UNIT;  }
    }
    bool z2zE2150;
    {
      bool z2zE2149;
      z2zE2149 = zis_running(UNIT);
      z2zE2150 = not(z2zE2149);
    }
    unit z3zE2158;
    if (z2zE2150) {
      z8zE434 = UNIT;
      goto cleanup_2336;
      /* unreachable after return */
      goto end_cleanup_2337;
    cleanup_2336: ;
      goto end_function_2335;
    end_cleanup_2337: ;
    } else {  z3zE2158 = UNIT;  }
    unit z3zE2153;
    if (ztg_deleg) {
      struct zCode z2zE2152;
      {
        sail_fixed_bytes_32 z2zE2151;
        {
          z2zE2151 = zk_code_key(ztg_target);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
        {
          z2zE2152 = zcode_db_resolve(z2zE2151);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
      }
      unit z3zE2154;
      {
        struct zAccount z2zE2153;
        {
          z2zE2153 = zk_aload(ztg_target);
          if (have_exception) {  goto end_block_exception_2354;  }
        }
        unit z3zE2155;
        {
          z3zE2155 = UNIT;
          goto finish_match_2326;
        }
      case_2327: ;
        sail_match_failure("run_call");
      finish_match_2326: ;
        z3zE2154 = z3zE2155;
        goto finish_match_2324;
      }
    case_2325: ;
      sail_match_failure("run_call");
    finish_match_2324: ;
      z3zE2153 = z3zE2154;
    } else {  z3zE2153 = UNIT;  }
    struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z2zE2154;
    z2zE2154 = zapply_memory_pair_expansion(zmemory);
    unit z3zE2165;
    {
      struct zMemoryRangeFields zargs;
      zargs = z2zE2154.ztup0;
      struct zMemoryRangeFields zret;
      zret = z2zE2154.ztup1;
      uint64_t zchild_gas;
      {
        zchild_gas = zconserved_gas_add(zbase_child, zstipend);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      struct zAccount z2zE2155;
      {
        z2zE2155 = zk_aload(ztarget);
        if (have_exception) {  goto end_block_exception_2354;  }
      }
      unit z3zE2166;
      {
        bool z2zE2161;
        {
          bool z2zE2160;
          z2zE2160 = (!(zcall_depth < zDEPTH_LIMIT));
          bool z3zE2169;
          if (z2zE2160) {  z3zE2169 = true;  } else {
            bool z2zE2159;
            z2zE2159 = zcall_takes_value(zkind);
            bool z3zE2168;
            if (z2zE2159) {
              bool z2zE2158;
              z2zE2158 = zword_nonzzero(zvalue);
              bool z3zE2167;
              if (z2zE2158) {
                bool z2zE2157;
                {
                  sail_u256 z2zE2156;
                  {
                    z2zE2156 = zk_get_balance(zcaller);
                    if (have_exception) {  goto end_block_exception_2354;  }
                  }
                  z2zE2157 = zword_ule(zvalue, z2zE2156);
                }
                z3zE2167 = not(z2zE2157);
              } else {  z3zE2167 = false;  }
              z3zE2168 = z3zE2167;
            } else {  z3zE2168 = false;  }
            z3zE2169 = z3zE2168;
          }
          z2zE2161 = z3zE2169;
        }
        if (z2zE2161) {
          unit z3zE2201;
          z3zE2201 = zreturndata_clear(UNIT);
          unit z3zE2200;
          {
            z3zE2200 = zrefund_gas(zchild_gas);
            if (have_exception) {  goto end_block_exception_2354;  }
          }
          unit z3zE2199;
          if (znew_account_charged) {
            {
              z3zE2199 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2354;  }
            }
          } else {  z3zE2199 = UNIT;  }
          {
            z3zE2166 = zpush_word(zWORD_ZERO);
            if (have_exception) {  goto end_block_exception_2354;  }
          }
        } else {
          uint64_t zselected_precompile;
          zselected_precompile = zprecompile_number(ztarget);
          bool z2zE2162;
          z2zE2162 = (zselected_precompile != UINT64_C(0));
          if (z2zE2162) {
            uint64_t znumber;
            znumber = zselected_precompile;
            struct zByteSliceFields zinput;
            {
              uint64_t z2zE2170;
              z2zE2170 = zargs.zoff;
              uint64_t z2zE2171;
              z2zE2171 = zargs.zlen;
              zinput = zmemory_byte_slice(z2zE2170, z2zE2171);
            }
            struct zoptionzIU64zK z2zE2163;
            CREATE(zoptionzIU64zK)(&z2zE2163);
            zprecompile_gas(&z2zE2163, znumber, zinput, zchild_gas);
            unit z3zE2187;
            {
              if (z2zE2163.kind != Kind_zSomezIU64zK) goto case_2334;
              uint64_t zused;
              zused = z2zE2163.variants.zSomezIU64zK;
              struct zPrecompileResult zresult;
              zresult = zrun_precompile_slice(znumber, zinput);
              bool z2zE2164;
              z2zE2164 = zresult.zsuccess;
              if (z2zE2164) {
                zreturndata = zresult.zoutput;
                unit z3zE2197;
                z3zE2197 = UNIT;
                bool z2zE2166;
                {
                  bool z2zE2165;
                  z2zE2165 = zcall_transfers_value(zkind);
                  bool z3zE2194;
                  if (z2zE2165) {  z3zE2194 = zword_nonzzero(zvalue);  } else {  z3zE2194 = false;  }
                  z2zE2166 = z3zE2194;
                }
                unit z3zE2196;
                if (z2zE2166) {
                  {
                    z3zE2196 = zk_transfer(zcaller, ztarget, zvalue);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2163);
                      goto end_block_exception_2354;
                    }
                  }
                } else {  z3zE2196 = UNIT;  }
                uint64_t z2zE2167;
                z2zE2167 = zret.zoff;
                uint64_t z2zE2168;
                z2zE2168 = zret.zlen;
                unit z3zE2195;
                z3zE2195 = zreturndata_copy_prefix(z2zE2167, z2zE2168);
                uint64_t zunused;
                {
                  zunused = zgas_sub_or_oog(zchild_gas, zused);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE2163);
                    goto end_block_exception_2354;
                  }
                }
                bool z2zE2169;
                z2zE2169 = zis_running(UNIT);
                if (z2zE2169) {
                  unit z3zE2198;
                  {
                    z3zE2198 = zrefund_gas(zunused);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2163);
                      goto end_block_exception_2354;
                    }
                  }
                  {
                    z3zE2187 = zpush_word(zWORD_ONE);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2163);
                      goto end_block_exception_2354;
                    }
                  }
                } else {  z3zE2187 = UNIT;  }
              } else {
                unit z3zE2193;
                z3zE2193 = zreturndata_clear(UNIT);
                unit z3zE2192;
                if (znew_account_charged) {
                  {
                    z3zE2192 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2163);
                      goto end_block_exception_2354;
                    }
                  }
                } else {  z3zE2192 = UNIT;  }
                {
                  z3zE2187 = zpush_word(zWORD_ZERO);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE2163);
                    goto end_block_exception_2354;
                  }
                }
              }
              goto finish_match_2332;
            }
          case_2334: ;
            {
              unit z3zE2189;
              z3zE2189 = zreturndata_clear(UNIT);
              unit z3zE2188;
              if (znew_account_charged) {
                {
                  z3zE2188 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE2163);
                    goto end_block_exception_2354;
                  }
                }
              } else {  z3zE2188 = UNIT;  }
              {
                z3zE2187 = zpush_word(zWORD_ZERO);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE2163);
                  goto end_block_exception_2354;
                }
              }
              goto finish_match_2332;
            }
          case_2333: ;
          finish_match_2332: ;
            z3zE2166 = z3zE2187;
            KILL(zoptionzIU64zK)(&z2zE2163);
          } else {
            struct zCode zchild_code;
            {
              zchild_code = zexecutable_code(ztarget, ztg_deleg, ztg_target);
              if (have_exception) {  goto end_block_exception_2354;  }
            }
            sail_fixed_bytes_20 zchild_addr;
            {
              bool z2zE2188;
              z2zE2188 = zcall_uses_target_address(zkind);
              if (z2zE2188) {  zchild_addr = ztarget;  } else {  zchild_addr = zcaller;  }
            }
            sail_fixed_bytes_20 zchild_caller;
            {
              bool z2zE2187;
              z2zE2187 = zcall_is_delegate(zkind);
              if (z2zE2187) {  zchild_caller = zmessage.zcaller;  } else {  zchild_caller = zcaller;  }
            }
            sail_u256 zchild_value;
            {
              bool z2zE2186;
              z2zE2186 = zcall_is_delegate(zkind);
              if (z2zE2186) {  zchild_value = zmessage.zvalue;  } else {  zchild_value = zvalue;  }
            }
            bool zchild_static;
            {
              bool z2zE2185;
              z2zE2185 = zcall_is_static(zkind);
              if (z2zE2185) {  zchild_static = true;  } else {  zchild_static = zmessage.zis_static;  }
            }
            struct zByteSliceFields zchild_calldata;
            {
              bool z2zE2182;
              z2zE2182 = zis_running(UNIT);
              if (z2zE2182) {
                uint64_t z2zE2183;
                z2zE2183 = zargs.zoff;
                uint64_t z2zE2184;
                z2zE2184 = zargs.zlen;
                zchild_calldata = zmemory_byte_slice(z2zE2183, z2zE2184);
              } else {  zchild_calldata = zEMPTY_SLICE;  }
            }
            struct zFrameCheckpoint zcheckpoint;
            CREATE(zFrameCheckpoint)(&zcheckpoint);
            {
              struct zFrameCheckpoint z2zE2181;
              CREATE(zFrameCheckpoint)(&z2zE2181);
              zsuspend_frame(&z2zE2181, UNIT);
              struct zFrameCheckpoint z3zE2171;
              CREATE(zFrameCheckpoint)(&z3zE2171);
              COPY(zFrameCheckpoint)(&z3zE2171, z2zE2181);
              z3zE2171.zstate_gas_remaining = zGAS_ZERO;
              COPY(zFrameCheckpoint)(&zcheckpoint, z3zE2171);
            }
            struct zFrameContinuation z2zE2175;
            CREATE(zFrameContinuation)(&z2zE2175);
            {
              struct zCallContinuation z2zE2174;
              CREATE(zCallContinuation)(&z2zE2174);
              {
                uint64_t z2zE2172;
                z2zE2172 = zret.zoff;
                uint64_t z2zE2173;
                z2zE2173 = zret.zlen;
                COPY(zFrameCheckpoint)(&((&z2zE2174)->zcheckpoint), zcheckpoint);
                z2zE2174.znew_account_charged = znew_account_charged;
                z2zE2174.zreturn_length = z2zE2173;
                z2zE2174.zreturn_offset = z2zE2172;
              }
              zResumeCall(&z2zE2175, z2zE2174);
            }
            unit z3zE2186;
            z3zE2186 = zframe_stack_push(z2zE2175);
            bool z2zE2177;
            {
              bool z2zE2176;
              z2zE2176 = zcall_transfers_value(zkind);
              bool z3zE2173;
              if (z2zE2176) {  z3zE2173 = zword_nonzzero(zvalue);  } else {  z3zE2173 = false;  }
              z2zE2177 = z3zE2173;
            }
            unit z3zE2185;
            if (z2zE2177) {
              {
                z3zE2185 = zk_transfer(zcaller, ztarget, zvalue);
                if (have_exception) {  goto end_block_exception_2354;  }
              }
            } else {  z3zE2185 = UNIT;  }
            uint64_t z2zE2179;
            {
              uint64_t z2zE2178;
              z2zE2178 = zcheckpoint.zcall_depth;
              z2zE2179 = zframe_depth_increment(z2zE2178);
            }
            struct zMessage z3zE2174;
            z3zE2174.zaddress = zchild_addr;
            z3zE2174.zcaller = zchild_caller;
            z3zE2174.zcode_address = ztarget;
            z3zE2174.zdepth = z2zE2179;
            z3zE2174.zis_static = zchild_static;
            z3zE2174.zstate_gas_reservoir = zstate_gas_remaining;
            z3zE2174.zvalue = zchild_value;
            zmessage = z3zE2174;
            unit z3zE2184;
            z3zE2184 = UNIT;
            unit z3zE2183;
            z3zE2183 = zcalldata_install(zchild_calldata);
            zpc = UINT64_C(0);
            unit z3zE2182;
            z3zE2182 = UNIT;
            zgas_remaining = zchild_gas;
            unit z3zE2181;
            z3zE2181 = UNIT;
            unit z3zE2180;
            z3zE2180 = UNIT;
            zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
            unit z3zE2179;
            z3zE2179 = UNIT;
            zRunning(&zframe_status, UNIT);
            unit z3zE2178;
            z3zE2178 = UNIT;
            unit z3zE2177;
            z3zE2177 = zreturndata_clear(UNIT);
            zframe_code = zchild_code;
            unit z3zE2176;
            z3zE2176 = UNIT;
            uint64_t z2zE2180;
            z2zE2180 = zcheckpoint.zcall_depth;
            zcall_depth = zframe_depth_increment(z2zE2180);
            unit z3zE2175;
            z3zE2175 = UNIT;
            zframe_refund = zGAS_REFUND_ZERO;
            z3zE2166 = UNIT;
          }
        }
        goto finish_match_2330;
      }
    case_2331: ;
      sail_match_failure("run_call");
    finish_match_2330: ;
      z3zE2165 = z3zE2166;
      goto finish_match_2328;
    }
  case_2329: ;
    sail_match_failure("run_call");
  finish_match_2328: ;
    z3zE2140 = z3zE2165;
    goto finish_match_2322;
  }
case_2323: ;
  sail_match_failure("run_call");
finish_match_2322: ;
  z8zE434 = z3zE2140;
end_function_2335: ;
  return z8zE434;
end_block_exception_2354: ;

  return UNIT;
}

unit zrun_create(bool zis2)
{
  unit z8zE435;
  sail_fixed_bytes_20 zcreator;
  zcreator = zself_addr(UNIT);
  sail_u256 zvalue;
  {
    zvalue = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2321;  }
  }
  sail_u256 zoff_word;
  {
    zoff_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2321;  }
  }
  sail_u256 zlen_word;
  {
    zlen_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2321;  }
  }
  sail_u256 zsalt;
  if (zis2) {
    {
      zsalt = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2321;  }
    }
  } else {  zsalt = zWORD_ZERO;  }
  bool z2zE2088;
  {
    bool z2zE2087;
    z2zE2087 = zis_running(UNIT);
    z2zE2088 = not(z2zE2087);
  }
  unit z3zE2083;
  if (z2zE2088) {
    z8zE435 = UNIT;
    goto cleanup_2319;
    /* unreachable after return */
    goto end_cleanup_2320;
  cleanup_2319: ;
    goto end_function_2304;
  end_cleanup_2320: ;
  } else {  z3zE2083 = UNIT;  }
  bool z2zE2089;
  {
    z2zE2089 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2321;  }
  }
  unit z3zE2082;
  if (z2zE2089) {
    z8zE435 = UNIT;
    goto cleanup_2317;
    /* unreachable after return */
    goto end_cleanup_2318;
  cleanup_2317: ;
    goto end_function_2304;
  end_cleanup_2318: ;
  } else {  z3zE2082 = UNIT;  }
  struct zMemoryRangeFields zinitcode;
  {
    zinitcode = zcharge_memory_range(zoff_word, zlen_word);
    if (have_exception) {  goto end_block_exception_2321;  }
  }
  bool z2zE2091;
  {
    bool z2zE2090;
    z2zE2090 = zis_running(UNIT);
    z2zE2091 = not(z2zE2090);
  }
  unit z3zE2088;
  if (z2zE2091) {
    z8zE435 = UNIT;
    goto cleanup_2315;
    /* unreachable after return */
    goto end_cleanup_2316;
  cleanup_2315: ;
    goto end_function_2304;
  end_cleanup_2316: ;
  } else {  z3zE2088 = UNIT;  }
  uint64_t z2zE2092;
  z2zE2092 = zcreate_access_cost(UNIT);
  unit z3zE2087;
  {
    z3zE2087 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2092);
    if (have_exception) {  goto end_block_exception_2321;  }
  }
  bool z2zE2093;
  z2zE2093 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE2086;
  if (z2zE2093) {
    sail_u256 z2zE2094;
    z2zE2094 = zmemory_word_count_word(zlen_word);
    {
      z3zE2086 = zcharge_word_scaled_gas(zG_initcode_word, z2zE2094);
      if (have_exception) {  goto end_block_exception_2321;  }
    }
  } else {  z3zE2086 = UNIT;  }
  unit z3zE2085;
  if (zis2) {
    sail_u256 z2zE2095;
    z2zE2095 = zmemory_word_count_word(zlen_word);
    {
      z3zE2085 = zcharge_word_scaled_gas(zG_keccak_word, z2zE2095);
      if (have_exception) {  goto end_block_exception_2321;  }
    }
  } else {  z3zE2085 = UNIT;  }
  bool z2zE2097;
  {
    bool z2zE2096;
    z2zE2096 = zis_running(UNIT);
    z2zE2097 = not(z2zE2096);
  }
  unit z3zE2084;
  if (z2zE2097) {
    z8zE435 = UNIT;
    goto cleanup_2313;
    /* unreachable after return */
    goto end_cleanup_2314;
  cleanup_2313: ;
    goto end_function_2304;
  end_cleanup_2314: ;
  } else {  z3zE2084 = UNIT;  }
  bool z2zE2100;
  {
    bool z2zE2099;
    {
      uint64_t z2zE2098;
      z2zE2098 = zinitcode.zlen;
      z2zE2099 = zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2098);
    }
    z2zE2100 = not(z2zE2099);
  }
  if (z2zE2100) {
    {
      z8zE435 = zexc_halt(zInitCodeTooLarge);
      if (have_exception) {  goto end_block_exception_2321;  }
    }
  } else {
    uint64_t znonce;
    {
      znonce = zk_get_nonce(zcreator);
      if (have_exception) {  goto end_block_exception_2321;  }
    }
    sail_fixed_bytes_20 znew_addr;
    {
      bool z2zE2129;
      {
        bool z3zE2089;
        if (zis2) {  z3zE2089 = zis_running(UNIT);  } else {  z3zE2089 = false;  }
        z2zE2129 = z3zE2089;
      }
      if (z2zE2129) {
        sail_fixed_bytes_32 z2zE2131;
        {
          sail_u256 z2zE2130;
          z2zE2130 = zmem_keccak(zinitcode);
          z2zE2131 = zword_to_hash(z2zE2130);
        }
        znew_addr = zk_create2_addr(zcreator, zsalt, z2zE2131);
      } else {  znew_addr = zk_create_addr(zcreator, znonce);  }
    }
    uint64_t zchild_gas;
    zchild_gas = zGAS_ZERO;
    bool z2zE2101;
    z2zE2101 = zfork_lt(zk_fork, zAmsterdam);
    unit z3zE2093;
    if (z2zE2101) {
      uint64_t zavail;
      zavail = zgas_remaining;
      uint64_t zretained_gas;
      zretained_gas = (zavail / UINT64_C(64));
      {
        zchild_gas = zgas_sub_or_oog(zavail, zretained_gas);
        if (have_exception) {  goto end_block_exception_2321;  }
      }
      unit z3zE2094;
      z3zE2094 = UNIT;
      zgas_remaining = zretained_gas;
      z3zE2093 = UNIT;
    } else {  z3zE2093 = UNIT;  }
    bool z2zE2103;
    {
      bool z2zE2102;
      z2zE2102 = zis_running(UNIT);
      z2zE2103 = not(z2zE2102);
    }
    unit z3zE2092;
    if (z2zE2103) {
      z8zE435 = UNIT;
      goto cleanup_2311;
      /* unreachable after return */
      goto end_cleanup_2312;
    cleanup_2311: ;
      goto end_function_2304;
    end_cleanup_2312: ;
    } else {  z3zE2092 = UNIT;  }
    bool z2zE2104;
    z2zE2104 = zmessage.zis_static;
    unit z3zE2090;
    if (z2zE2104) {
      unit z3zE2091;
      {
        z3zE2091 = zexc_halt(zWriteProtection);
        if (have_exception) {  goto end_block_exception_2321;  }
      }
      z8zE435 = UNIT;
      goto cleanup_2309;
      /* unreachable after return */
      goto end_cleanup_2310;
    cleanup_2309: ;
      goto end_function_2304;
    end_cleanup_2310: ;
    } else {  z3zE2090 = UNIT;  }
    bool z2zE2109;
    {
      bool z2zE2108;
      z2zE2108 = (!(zcall_depth < zDEPTH_LIMIT));
      bool z3zE2096;
      if (z2zE2108) {  z3zE2096 = true;  } else {
        bool z2zE2107;
        {
          bool z2zE2106;
          {
            sail_u256 z2zE2105;
            {
              z2zE2105 = zk_get_balance(zcreator);
              if (have_exception) {  goto end_block_exception_2321;  }
            }
            z2zE2106 = zword_ule(zvalue, z2zE2105);
          }
          z2zE2107 = not(z2zE2106);
        }
        bool z3zE2095;
        if (z2zE2107) {  z3zE2095 = true;  } else {  z3zE2095 = (znonce == UINT64_C(18446744073709551615));  }
        z3zE2096 = z3zE2095;
      }
      z2zE2109 = z3zE2096;
    }
    if (z2zE2109) {
      unit z3zE2127;
      z3zE2127 = zreturndata_clear(UNIT);
      bool z2zE2110;
      z2zE2110 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE2126;
      if (z2zE2110) {
        {
          z3zE2126 = zrefund_gas(zchild_gas);
          if (have_exception) {  goto end_block_exception_2321;  }
        }
      } else {  z3zE2126 = UNIT;  }
      {
        z8zE435 = zpush_word(zWORD_ZERO);
        if (have_exception) {  goto end_block_exception_2321;  }
      }
    } else {
      bool z2zE2111;
      z2zE2111 = zk_access_account(znew_addr);
      unit z3zE2097;
      {
        bool znew_account_charged;
        {
          bool z2zE2128;
          z2zE2128 = zfork_gteq(zk_fork, zAmsterdam);
          bool z3zE2098;
          if (z2zE2128) {
            {
              z3zE2098 = zk_account_is_empty(znew_addr);
              if (have_exception) {  goto end_block_exception_2321;  }
            }
          } else {  z3zE2098 = false;  }
          znew_account_charged = z3zE2098;
        }
        unit z3zE2103;
        if (znew_account_charged) {
          {
            z3zE2103 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
        } else {  z3zE2103 = UNIT;  }
        bool z2zE2113;
        {
          bool z2zE2112;
          z2zE2112 = zis_running(UNIT);
          z2zE2113 = not(z2zE2112);
        }
        unit z3zE2102;
        if (z2zE2113) {
          z8zE435 = UNIT;
          goto cleanup_2307;
          /* unreachable after return */
          goto end_cleanup_2308;
        cleanup_2307: ;
          goto end_function_2304;
        end_cleanup_2308: ;
        } else {  z3zE2102 = UNIT;  }
        bool z2zE2114;
        z2zE2114 = zfork_gteq(zk_fork, zAmsterdam);
        unit z3zE2100;
        if (z2zE2114) {
          uint64_t z3zE2128;
          z3zE2128 = zgas_remaining;
          uint64_t z3zE2129;
          z3zE2129 = (z3zE2128 / UINT64_C(64));
          {
            zchild_gas = zgas_sub_or_oog(z3zE2128, z3zE2129);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
          unit z3zE2101;
          z3zE2101 = UNIT;
          zgas_remaining = z3zE2129;
          z3zE2100 = UNIT;
        } else {  z3zE2100 = UNIT;  }
        bool z2zE2116;
        {
          bool z2zE2115;
          z2zE2115 = zis_running(UNIT);
          z2zE2116 = not(z2zE2115);
        }
        unit z3zE2099;
        if (z2zE2116) {
          z8zE435 = UNIT;
          goto cleanup_2305;
          /* unreachable after return */
          goto end_cleanup_2306;
        cleanup_2305: ;
          goto end_function_2304;
        end_cleanup_2306: ;
        } else {  z3zE2099 = UNIT;  }
        bool zoccupied;
        {
          zoccupied = zk_account_occupied(znew_addr);
          if (have_exception) {  goto end_block_exception_2321;  }
        }
        unit z3zE2105;
        z3zE2105 = zreturndata_clear(UNIT);
        unit z3zE2104;
        {
          z3zE2104 = zk_bump_nonce(zcreator);
          if (have_exception) {  goto end_block_exception_2321;  }
        }
        if (zoccupied) {
          unit z3zE2125;
          if (znew_account_charged) {
            {
              z3zE2125 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2321;  }
            }
          } else {  z3zE2125 = UNIT;  }
          {
            z3zE2097 = zpush_word(zWORD_ZERO);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
        } else {
          struct zByteSliceFields z1zE2;
          {
            bool z2zE2125;
            z2zE2125 = zis_running(UNIT);
            if (z2zE2125) {
              uint64_t z2zE2126;
              z2zE2126 = zinitcode.zoff;
              uint64_t z2zE2127;
              z2zE2127 = zinitcode.zlen;
              z1zE2 = zmemory_code_slice(z2zE2126, z2zE2127);
            } else {  z1zE2 = zEMPTY_CODE_SLICE;  }
          }
          struct zCode zchild_code;
          {
            sail_fixed_bytes_32 z2zE2124;
            z2zE2124 = zcode_db_insert(z1zE2, zk_fork);
            {
              zchild_code = zcode_db_resolve(z2zE2124);
              if (have_exception) {  goto end_block_exception_2321;  }
            }
          }
          struct zFrameCheckpoint zcheckpoint;
          CREATE(zFrameCheckpoint)(&zcheckpoint);
          {
            struct zFrameCheckpoint z2zE2123;
            CREATE(zFrameCheckpoint)(&z2zE2123);
            zsuspend_frame(&z2zE2123, UNIT);
            struct zFrameCheckpoint z3zE2107;
            CREATE(zFrameCheckpoint)(&z3zE2107);
            COPY(zFrameCheckpoint)(&z3zE2107, z2zE2123);
            z3zE2107.zstate_gas_remaining = zGAS_ZERO;
            COPY(zFrameCheckpoint)(&zcheckpoint, z3zE2107);
          }
          struct zFrameContinuation z2zE2117;
          CREATE(zFrameContinuation)(&z2zE2117);
          {
            struct zCreateContinuation z3zE2108;
            CREATE(zCreateContinuation)(&z3zE2108);
            z3zE2108.zaddress = znew_addr;
            COPY(zFrameCheckpoint)(&((&z3zE2108)->zcheckpoint), zcheckpoint);
            z3zE2108.znew_account_charged = znew_account_charged;
            zResumeCreate(&z2zE2117, z3zE2108);
          }
          unit z3zE2124;
          z3zE2124 = zframe_stack_push(z2zE2117);
          unit z3zE2123;
          {
            z3zE2123 = zk_mark_created(znew_addr);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
          unit z3zE2122;
          {
            z3zE2122 = zk_clear_storage(znew_addr);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
          unit z3zE2121;
          {
            z3zE2121 = zk_bump_nonce(znew_addr);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
          unit z3zE2120;
          {
            z3zE2120 = zk_transfer(zcreator, znew_addr, zvalue);
            if (have_exception) {  goto end_block_exception_2321;  }
          }
          bool z2zE2119;
          {
            struct zMessage z2zE2118;
            z2zE2118 = zcheckpoint.zmessage;
            z2zE2119 = z2zE2118.zis_static;
          }
          uint64_t z2zE2121;
          {
            uint64_t z2zE2120;
            z2zE2120 = zcheckpoint.zcall_depth;
            z2zE2121 = zframe_depth_increment(z2zE2120);
          }
          struct zMessage z3zE2109;
          z3zE2109.zaddress = znew_addr;
          z3zE2109.zcaller = zcreator;
          z3zE2109.zcode_address = znew_addr;
          z3zE2109.zdepth = z2zE2121;
          z3zE2109.zis_static = z2zE2119;
          z3zE2109.zstate_gas_reservoir = zstate_gas_remaining;
          z3zE2109.zvalue = zvalue;
          zmessage = z3zE2109;
          unit z3zE2119;
          z3zE2119 = UNIT;
          unit z3zE2118;
          z3zE2118 = zcalldata_install(zEMPTY_SLICE);
          zpc = UINT64_C(0);
          unit z3zE2117;
          z3zE2117 = UNIT;
          zgas_remaining = zchild_gas;
          unit z3zE2116;
          z3zE2116 = UNIT;
          unit z3zE2115;
          z3zE2115 = UNIT;
          zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
          unit z3zE2114;
          z3zE2114 = UNIT;
          zRunning(&zframe_status, UNIT);
          unit z3zE2113;
          z3zE2113 = UNIT;
          unit z3zE2112;
          z3zE2112 = zreturndata_clear(UNIT);
          zframe_code = zchild_code;
          unit z3zE2111;
          z3zE2111 = UNIT;
          uint64_t z2zE2122;
          z2zE2122 = zcheckpoint.zcall_depth;
          zcall_depth = zframe_depth_increment(z2zE2122);
          unit z3zE2110;
          z3zE2110 = UNIT;
          zframe_refund = zGAS_REFUND_ZERO;
          z3zE2097 = UNIT;
        }
        goto finish_match_2302;
      }
    case_2303: ;
      sail_match_failure("run_create");
    finish_match_2302: ;
      z8zE435 = z3zE2097;
    }
  }
end_function_2304: ;
  return z8zE435;
end_block_exception_2321: ;

  return UNIT;
}

unit zexecute(struct zast zop)
{
  unit z8zE436;
  unit z3zE2074;
  {
    if (zop.kind != Kind_zCREATE) goto case_2299;
    {
      z3zE2074 = zrun_create(false);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2299: ;
  {
    if (zop.kind != Kind_zCREATE2) goto case_2298;
    {
      z3zE2074 = zrun_create(true);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2298: ;
  {
    if (zop.kind != Kind_zCALL) goto case_2297;
    {
      z3zE2074 = zrun_call(zCall);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2297: ;
  {
    if (zop.kind != Kind_zCALLCODE) goto case_2296;
    {
      z3zE2074 = zrun_call(zCallCode);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2296: ;
  {
    if (zop.kind != Kind_zDELEGATECALL) goto case_2295;
    {
      z3zE2074 = zrun_call(zDelegateCall);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2295: ;
  {
    if (zop.kind != Kind_zSTATICCALL) goto case_2294;
    {
      z3zE2074 = zrun_call(zStaticCall);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2294: ;
  {
    {
      z3zE2074 = zexecute_opcode(zop);
      if (have_exception) {  goto end_block_exception_2301;  }
    }
    goto finish_match_2292;
  }
case_2293: ;
finish_match_2292: ;
  z8zE436 = z3zE2074;
end_function_2300: ;
  return z8zE436;
end_block_exception_2301: ;

  return UNIT;
}

