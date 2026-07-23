/* Generated from sail/evm/execute.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_20 zself_addr(unit z3zE2635)
{
  sail_fixed_bytes_20 z8zE418;
  z8zE418 = zmessage.zaddress;
end_function_2614: ;
  return z8zE418;
end_block_exception_2615: ;

  return fixed_bytes_20_zero();
}

bool zguard_static(unit z3zE2633)
{
  bool z8zE419;
  bool z2zE2466;
  z2zE2466 = zmessage.zis_static;
  if (z2zE2466) {
    unit z3zE2634;
    {
      z3zE2634 = zexc_halt(zWriteProtection);
      if (have_exception) {  goto end_block_exception_2613;  }
    }
    z8zE419 = true;
  } else {  z8zE419 = false;  }
end_function_2612: ;
  return z8zE419;
end_block_exception_2613: ;

  return false;
}

unit zdo_jump(sail_u256 zdestination_value)
{
  unit z8zE420;
  uint64_t zcode_length;
  zcode_length = zframe_code_len(UNIT);
  bool z2zE2464;
  z2zE2464 = u256_lt_u64(zdestination_value, zcode_length);
  if (z2zE2464) {
    uint64_t zdestination;
    zdestination = u256_to_u64(zdestination_value);
    bool z2zE2465;
    z2zE2465 = zframe_jumpdest_valid(zdestination);
    if (z2zE2465) {
      zpc = zdestination;
      z8zE420 = UNIT;
    } else {
      {
        z8zE420 = zexc_halt(zInvalidJump);
        if (have_exception) {  goto end_block_exception_2611;  }
      }
    }
  } else {
    {
      z8zE420 = zexc_halt(zInvalidJump);
      if (have_exception) {  goto end_block_exception_2611;  }
    }
  }
end_function_2610: ;
  return z8zE420;
end_block_exception_2611: ;

  return UNIT;
}

void zpop_log_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9 *z8zE421, uint64_t zcount)
{
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z3zE2611;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
  {
    uint64_t zp0z3;
    zp0z3 = zcount;
    bool z3zE2623;
    z3zE2623 = (zp0z3 == UINT64_C(0));
    if (!(z3zE2623)) {  goto case_2607;  }
    goto finish_match_2601;
  }
case_2607: ;
  {
    uint64_t z3zE2624;
    z3zE2624 = zcount;
    bool z3zE2621;
    z3zE2621 = (z3zE2624 == UINT64_C(1));
    if (!(z3zE2621)) {  goto case_2606;  }
    sail_u256 z2zE2463;
    {
      z2zE2463 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, z2zE2463, z3zE2611);
    goto finish_match_2601;
  }
case_2606: ;
  {
    uint64_t z3zE2625;
    z3zE2625 = zcount;
    bool z3zE2619;
    z3zE2619 = (z3zE2625 == UINT64_C(2));
    if (!(z3zE2619)) {  goto case_2605;  }
    sail_u256 zt0;
    {
      zt0 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    sail_u256 zt1;
    {
      zt1 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, zt1, z3zE2611);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, zt0, z3zE2611);
    goto finish_match_2601;
  }
case_2605: ;
  {
    uint64_t z3zE2626;
    z3zE2626 = zcount;
    bool z3zE2617;
    z3zE2617 = (z3zE2626 == UINT64_C(3));
    if (!(z3zE2617)) {  goto case_2604;  }
    sail_u256 z3zE2627;
    {
      z3zE2627 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    sail_u256 z3zE2628;
    {
      z3zE2628 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    sail_u256 zt2;
    {
      zt2 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, zt2, z3zE2611);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, z3zE2628, z3zE2611);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, z3zE2627, z3zE2611);
    goto finish_match_2601;
  }
case_2604: ;
  {
    uint64_t z3zE2629;
    z3zE2629 = zcount;
    bool z3zE2615;
    z3zE2615 = (z3zE2629 == UINT64_C(4));
    if (!(z3zE2615)) {  goto case_2603;  }
    sail_u256 z3zE2630;
    {
      z3zE2630 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    sail_u256 z3zE2631;
    {
      z3zE2631 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    sail_u256 z3zE2632;
    {
      z3zE2632 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    sail_u256 zt3;
    {
      zt3 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
        goto end_block_exception_2609;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, zt3, z3zE2611);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, z3zE2632, z3zE2611);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, z3zE2631, z3zE2611);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2611, z3zE2630, z3zE2611);
    goto finish_match_2601;
  }
case_2603: ;
  {    goto finish_match_2601;
  }
case_2602: ;
finish_match_2601: ;
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)((*(&z8zE421)), z3zE2611);
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2611);
end_function_2608: ;
  goto end_function_4067;
end_block_exception_2609: ;
  goto end_function_4067;
end_function_4067: ;
}

unit zexecute_arithmetic(struct zast zop)
{
  unit z8zE422;
  unit z3zE2515;
  {
    if (zop.kind != Kind_zADD) goto case_2598;
    unit z3zE2567;
    {
      z3zE2567 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 za;
    {
      za = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zb;
    {
      zb = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2436;
    z2zE2436 = zalu_add(za, zb);
    {
      z3zE2515 = zpush_word(z2zE2436);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2598: ;
  {
    if (zop.kind != Kind_zMUL) goto case_2597;
    unit z3zE2565;
    {
      z3zE2565 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2569;
    {
      z3zE2569 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2570;
    {
      z3zE2570 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2437;
    z2zE2437 = zalu_mul(z3zE2569, z3zE2570);
    {
      z3zE2515 = zpush_word(z2zE2437);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2597: ;
  {
    if (zop.kind != Kind_zSUB) goto case_2596;
    unit z3zE2563;
    {
      z3zE2563 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2571;
    {
      z3zE2571 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2572;
    {
      z3zE2572 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2438;
    z2zE2438 = zalu_sub(z3zE2571, z3zE2572);
    {
      z3zE2515 = zpush_word(z2zE2438);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2596: ;
  {
    if (zop.kind != Kind_zDIV) goto case_2595;
    unit z3zE2561;
    {
      z3zE2561 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2573;
    {
      z3zE2573 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2574;
    {
      z3zE2574 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2439;
    z2zE2439 = zalu_div(z3zE2573, z3zE2574);
    {
      z3zE2515 = zpush_word(z2zE2439);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2595: ;
  {
    if (zop.kind != Kind_zSDIV) goto case_2594;
    unit z3zE2559;
    {
      z3zE2559 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2575;
    {
      z3zE2575 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2576;
    {
      z3zE2576 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2440;
    z2zE2440 = zalu_sdiv(z3zE2575, z3zE2576);
    {
      z3zE2515 = zpush_word(z2zE2440);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2594: ;
  {
    if (zop.kind != Kind_zMOD) goto case_2593;
    unit z3zE2557;
    {
      z3zE2557 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2577;
    {
      z3zE2577 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2578;
    {
      z3zE2578 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2441;
    z2zE2441 = zalu_mod(z3zE2577, z3zE2578);
    {
      z3zE2515 = zpush_word(z2zE2441);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2593: ;
  {
    if (zop.kind != Kind_zSMOD) goto case_2592;
    unit z3zE2555;
    {
      z3zE2555 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2579;
    {
      z3zE2579 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2580;
    {
      z3zE2580 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2442;
    z2zE2442 = zalu_smod(z3zE2579, z3zE2580);
    {
      z3zE2515 = zpush_word(z2zE2442);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2592: ;
  {
    if (zop.kind != Kind_zADDMOD) goto case_2591;
    unit z3zE2553;
    {
      z3zE2553 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2581;
    {
      z3zE2581 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2582;
    {
      z3zE2582 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zn;
    {
      zn = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2443;
    z2zE2443 = zalu_addmod(z3zE2581, z3zE2582, zn);
    {
      z3zE2515 = zpush_word(z2zE2443);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2591: ;
  {
    if (zop.kind != Kind_zMULMOD) goto case_2590;
    unit z3zE2551;
    {
      z3zE2551 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2583;
    {
      z3zE2583 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2584;
    {
      z3zE2584 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2585;
    {
      z3zE2585 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2444;
    z2zE2444 = zalu_mulmod(z3zE2583, z3zE2584, z3zE2585);
    {
      z3zE2515 = zpush_word(z2zE2444);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2590: ;
  {
    if (zop.kind != Kind_zEXP) goto case_2589;
    sail_u256 z3zE2586;
    {
      z3zE2586 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 ze;
    {
      ze = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    uint64_t z2zE2445;
    z2zE2445 = zexp_gas(ze);
    unit z3zE2549;
    {
      z3zE2549 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2445);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2446;
    z2zE2446 = zalu_exp(z3zE2586, ze);
    {
      z3zE2515 = zpush_word(z2zE2446);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2589: ;
  {
    if (zop.kind != Kind_zSIGNEXTEND) goto case_2588;
    unit z3zE2547;
    {
      z3zE2547 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zbi;
    {
      zbi = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zv;
    {
      zv = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2447;
    z2zE2447 = zalu_signextend(zbi, zv);
    {
      z3zE2515 = zpush_word(z2zE2447);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2588: ;
  {
    if (zop.kind != Kind_zLT) goto case_2587;
    unit z3zE2545;
    {
      z3zE2545 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2587;
    {
      z3zE2587 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2588;
    {
      z3zE2588 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2448;
    z2zE2448 = zalu_lt(z3zE2587, z3zE2588);
    {
      z3zE2515 = zpush_word(z2zE2448);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2587: ;
  {
    if (zop.kind != Kind_zGT) goto case_2586;
    unit z3zE2543;
    {
      z3zE2543 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2589;
    {
      z3zE2589 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2590;
    {
      z3zE2590 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2449;
    z2zE2449 = zalu_gt(z3zE2589, z3zE2590);
    {
      z3zE2515 = zpush_word(z2zE2449);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2586: ;
  {
    if (zop.kind != Kind_zSLT) goto case_2585;
    unit z3zE2541;
    {
      z3zE2541 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2591;
    {
      z3zE2591 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2592;
    {
      z3zE2592 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2450;
    z2zE2450 = zalu_slt(z3zE2591, z3zE2592);
    {
      z3zE2515 = zpush_word(z2zE2450);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2585: ;
  {
    if (zop.kind != Kind_zSGT) goto case_2584;
    unit z3zE2539;
    {
      z3zE2539 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2593;
    {
      z3zE2593 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2594;
    {
      z3zE2594 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2451;
    z2zE2451 = zalu_sgt(z3zE2593, z3zE2594);
    {
      z3zE2515 = zpush_word(z2zE2451);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2584: ;
  {
    if (zop.kind != Kind_zEQ) goto case_2583;
    unit z3zE2537;
    {
      z3zE2537 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2595;
    {
      z3zE2595 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2596;
    {
      z3zE2596 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2452;
    z2zE2452 = zalu_eq(z3zE2595, z3zE2596);
    {
      z3zE2515 = zpush_word(z2zE2452);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2583: ;
  {
    if (zop.kind != Kind_zISZERO) goto case_2582;
    unit z3zE2535;
    {
      z3zE2535 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2597;
    {
      z3zE2597 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2453;
    z2zE2453 = zalu_iszzero(z3zE2597);
    {
      z3zE2515 = zpush_word(z2zE2453);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2582: ;
  {
    if (zop.kind != Kind_zAND) goto case_2581;
    unit z3zE2533;
    {
      z3zE2533 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2598;
    {
      z3zE2598 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2599;
    {
      z3zE2599 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2454;
    z2zE2454 = zalu_and(z3zE2598, z3zE2599);
    {
      z3zE2515 = zpush_word(z2zE2454);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2581: ;
  {
    if (zop.kind != Kind_zOR) goto case_2580;
    unit z3zE2531;
    {
      z3zE2531 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2600;
    {
      z3zE2600 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2601;
    {
      z3zE2601 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2455;
    z2zE2455 = zalu_or(z3zE2600, z3zE2601);
    {
      z3zE2515 = zpush_word(z2zE2455);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2580: ;
  {
    if (zop.kind != Kind_zXOR) goto case_2579;
    unit z3zE2529;
    {
      z3zE2529 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2602;
    {
      z3zE2602 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2603;
    {
      z3zE2603 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2456;
    z2zE2456 = zalu_xor(z3zE2602, z3zE2603);
    {
      z3zE2515 = zpush_word(z2zE2456);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2579: ;
  {
    if (zop.kind != Kind_zNOT) goto case_2578;
    unit z3zE2527;
    {
      z3zE2527 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2604;
    {
      z3zE2604 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2457;
    z2zE2457 = zalu_not(z3zE2604);
    {
      z3zE2515 = zpush_word(z2zE2457);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2578: ;
  {
    if (zop.kind != Kind_zBYTE) goto case_2577;
    unit z3zE2525;
    {
      z3zE2525 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zi;
    {
      zi = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zx;
    {
      zx = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2458;
    z2zE2458 = zalu_byte(zi, zx);
    {
      z3zE2515 = zpush_word(z2zE2458);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2577: ;
  {
    if (zop.kind != Kind_zSHL) goto case_2576;
    unit z3zE2523;
    {
      z3zE2523 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 zs;
    {
      zs = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2605;
    {
      z3zE2605 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2459;
    z2zE2459 = zalu_shl(zs, z3zE2605);
    {
      z3zE2515 = zpush_word(z2zE2459);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2576: ;
  {
    if (zop.kind != Kind_zSHR) goto case_2575;
    unit z3zE2521;
    {
      z3zE2521 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2606;
    {
      z3zE2606 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2607;
    {
      z3zE2607 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2460;
    z2zE2460 = zalu_shr(z3zE2606, z3zE2607);
    {
      z3zE2515 = zpush_word(z2zE2460);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2575: ;
  {
    if (zop.kind != Kind_zSAR) goto case_2574;
    unit z3zE2519;
    {
      z3zE2519 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2608;
    {
      z3zE2608 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2609;
    {
      z3zE2609 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2461;
    z2zE2461 = zalu_sar(z3zE2608, z3zE2609);
    {
      z3zE2515 = zpush_word(z2zE2461);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2574: ;
  {
    if (zop.kind != Kind_zCLZ) goto case_2573;
    unit z3zE2517;
    {
      z3zE2517 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z3zE2610;
    {
      z3zE2610 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    sail_u256 z2zE2462;
    z2zE2462 = zalu_clzz(z3zE2610);
    {
      z3zE2515 = zpush_word(z2zE2462);
      if (have_exception) {  goto end_block_exception_2600;  }
    }
    goto finish_match_2571;
  }
case_2573: ;
  {
    z3zE2515 = sail_assert(false, "sail/evm/execute.sail:255.21-255.22");
    goto finish_match_2571;
  }
case_2572: ;
finish_match_2571: ;
  z8zE422 = z3zE2515;
end_function_2599: ;
  return z8zE422;
end_block_exception_2600: ;

  return UNIT;
}

unit zexecute_environment(struct zast zop)
{
  unit z8zE423;
  unit z3zE2452;
  {
    if (zop.kind != Kind_zKECCAK256) goto case_2568;
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    unit z3zE2492;
    {
      z3zE2492 = zcharge_keccak_gas(zlength_word);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_range(zoffset_word, zlength_word);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2385;
    z2zE2385 = zis_running(UNIT);
    if (z2zE2385) {
      sail_u256 z2zE2386;
      z2zE2386 = zmem_keccak(zrange);
      {
        z3zE2452 = zpush_word(z2zE2386);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2568: ;
  {
    if (zop.kind != Kind_zADDRESS) goto case_2567;
    unit z3zE2490;
    {
      z3zE2490 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2388;
    {
      sail_fixed_bytes_20 z2zE2387;
      z2zE2387 = zself_addr(UNIT);
      z2zE2388 = zaddress_to_word(z2zE2387);
    }
    {
      z3zE2452 = zpush_word(z2zE2388);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2567: ;
  {
    if (zop.kind != Kind_zORIGIN) goto case_2566;
    unit z3zE2488;
    {
      z3zE2488 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2389;
    z2zE2389 = zk_env(zF_Origin);
    {
      z3zE2452 = zpush_word(z2zE2389);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2566: ;
  {
    if (zop.kind != Kind_zCALLER) goto case_2565;
    unit z3zE2486;
    {
      z3zE2486 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2391;
    {
      sail_fixed_bytes_20 z2zE2390;
      z2zE2390 = zmessage.zcaller;
      z2zE2391 = zaddress_to_word(z2zE2390);
    }
    {
      z3zE2452 = zpush_word(z2zE2391);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2565: ;
  {
    if (zop.kind != Kind_zCALLVALUE) goto case_2564;
    unit z3zE2484;
    {
      z3zE2484 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2392;
    z2zE2392 = zmessage.zvalue;
    {
      z3zE2452 = zpush_word(z2zE2392);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2564: ;
  {
    if (zop.kind != Kind_zGASPRICE) goto case_2563;
    unit z3zE2482;
    {
      z3zE2482 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2393;
    z2zE2393 = zk_env(zF_GasPrice);
    {
      z3zE2452 = zpush_word(z2zE2393);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2563: ;
  {
    if (zop.kind != Kind_zCALLDATASIZE) goto case_2562;
    unit z3zE2480;
    {
      z3zE2480 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    struct zByteSliceFields zinput;
    zinput = zcalldata;
    sail_u256 z2zE2395;
    {
      uint64_t z2zE2394;
      z2zE2394 = zinput.zlen;
      z2zE2395 = zword_of_source_byte_count(z2zE2394);
    }
    {
      z3zE2452 = zpush_word(z2zE2395);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2562: ;
  {
    if (zop.kind != Kind_zCALLDATALOAD) goto case_2561;
    unit z3zE2478;
    {
      z3zE2478 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2494;
    {
      z3zE2494 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2396;
    z2zE2396 = zis_running(UNIT);
    if (z2zE2396) {
      sail_u256 z2zE2397;
      z2zE2397 = zslice_load_word_offset(zcalldata, z3zE2494);
      {
        z3zE2452 = zpush_word(z2zE2397);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2561: ;
  {
    if (zop.kind != Kind_zCALLDATACOPY) goto case_2560;
    unit z3zE2475;
    {
      z3zE2475 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 zdestination_word;
    {
      zdestination_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 zsource_word;
    {
      zsource_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2495;
    {
      z3zE2495 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    unit z3zE2476;
    {
      z3zE2476 = zcharge_copy_gas(z3zE2495);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    struct zMemoryRangeFields z3zE2496;
    {
      z3zE2496 = zcharge_memory_range(zdestination_word, z3zE2495);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2398;
    z2zE2398 = zis_running(UNIT);
    if (z2zE2398) {
      uint64_t z2zE2399;
      z2zE2399 = z3zE2496.zoff;
      uint64_t z2zE2400;
      z2zE2400 = z3zE2496.zlen;
      z3zE2452 = zslice_copy_word_offset(zcalldata, z2zE2399, zsource_word, z2zE2400);
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2560: ;
  {
    if (zop.kind != Kind_zCODESIZE) goto case_2559;
    unit z3zE2473;
    {
      z3zE2473 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2402;
    {
      uint64_t z2zE2401;
      z2zE2401 = zframe_code_len(UNIT);
      z2zE2402 = zword_of_source_byte_count(z2zE2401);
    }
    {
      z3zE2452 = zpush_word(z2zE2402);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2559: ;
  {
    if (zop.kind != Kind_zCODECOPY) goto case_2558;
    unit z3zE2470;
    {
      z3zE2470 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2497;
    {
      z3zE2497 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2498;
    {
      z3zE2498 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2499;
    {
      z3zE2499 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    unit z3zE2471;
    {
      z3zE2471 = zcharge_copy_gas(z3zE2499);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    struct zMemoryRangeFields z3zE2500;
    {
      z3zE2500 = zcharge_memory_range(z3zE2497, z3zE2499);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2403;
    z2zE2403 = zis_running(UNIT);
    if (z2zE2403) {
      struct zByteSliceFields z2zE2404;
      z2zE2404 = zframe_code.zbytes;
      uint64_t z2zE2405;
      z2zE2405 = z3zE2500.zoff;
      uint64_t z2zE2406;
      z2zE2406 = z3zE2500.zlen;
      z3zE2452 = zslice_copy_word_offset(z2zE2404, z2zE2405, z3zE2498, z2zE2406);
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2558: ;
  {
    if (zop.kind != Kind_zBALANCE) goto case_2557;
    sail_fixed_bytes_20 za;
    {
      sail_u256 z2zE2410;
      {
        z2zE2410 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
      za = zword_to_address(z2zE2410);
    }
    bool zwarm;
    zwarm = zk_access_account(za);
    uint64_t z2zE2407;
    z2zE2407 = zaccount_cost(zwarm);
    unit z3zE2468;
    {
      z3zE2468 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2407);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2408;
    z2zE2408 = zis_running(UNIT);
    if (z2zE2408) {
      sail_u256 z2zE2409;
      {
        z2zE2409 = zk_get_balance(za);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
      {
        z3zE2452 = zpush_word(z2zE2409);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2557: ;
  {
    if (zop.kind != Kind_zSELFBALANCE) goto case_2556;
    unit z3zE2466;
    {
      z3zE2466 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2412;
    {
      sail_fixed_bytes_20 z2zE2411;
      z2zE2411 = zself_addr(UNIT);
      {
        z2zE2412 = zk_get_balance(z2zE2411);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    }
    {
      z3zE2452 = zpush_word(z2zE2412);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2556: ;
  {
    if (zop.kind != Kind_zEXTCODESIZE) goto case_2555;
    sail_fixed_bytes_20 z3zE2501;
    {
      sail_u256 z2zE2419;
      {
        z2zE2419 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
      z3zE2501 = zword_to_address(z2zE2419);
    }
    bool z3zE2502;
    z3zE2502 = zk_access_account(z3zE2501);
    uint64_t z2zE2415;
    {
      uint64_t z2zE2413;
      z2zE2413 = zaccount_cost(z3zE2502);
      uint64_t z2zE2414;
      z2zE2414 = zexternal_code_read_cost(UNIT);
      {    z2zE2415 = (z2zE2413 + z2zE2414);
      }
    }
    unit z3zE2464;
    {
      z3zE2464 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2415);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2416;
    z2zE2416 = zis_running(UNIT);
    if (z2zE2416) {
      sail_u256 z2zE2418;
      {
        uint64_t z2zE2417;
        {
          z2zE2417 = zk_get_code_sizze(z3zE2501);
          if (have_exception) {  goto end_block_exception_2570;  }
        }
        z2zE2418 = zword_of_source_byte_count(z2zE2417);
      }
      {
        z3zE2452 = zpush_word(z2zE2418);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2555: ;
  {
    if (zop.kind != Kind_zEXTCODECOPY) goto case_2554;
    sail_fixed_bytes_20 z3zE2503;
    {
      sail_u256 z2zE2426;
      {
        z2zE2426 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
      z3zE2503 = zword_to_address(z2zE2426);
    }
    sail_u256 z3zE2504;
    {
      z3zE2504 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2505;
    {
      z3zE2505 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2506;
    {
      z3zE2506 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z3zE2507;
    z3zE2507 = zk_access_account(z3zE2503);
    uint64_t z2zE2422;
    {
      uint64_t z2zE2420;
      z2zE2420 = zaccount_cost(z3zE2507);
      uint64_t z2zE2421;
      z2zE2421 = zexternal_code_read_cost(UNIT);
      {    z2zE2422 = (z2zE2420 + z2zE2421);
      }
    }
    unit z3zE2462;
    {
      z3zE2462 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2422);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    unit z3zE2461;
    {
      z3zE2461 = zcharge_copy_gas(z3zE2506);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    struct zMemoryRangeFields z3zE2508;
    {
      z3zE2508 = zcharge_memory_range(z3zE2504, z3zE2506);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2423;
    z2zE2423 = zis_running(UNIT);
    if (z2zE2423) {
      uint64_t z2zE2424;
      z2zE2424 = z3zE2508.zoff;
      uint64_t z2zE2425;
      z2zE2425 = z3zE2508.zlen;
      {
        z3zE2452 = zk_code_copy(z3zE2503, z2zE2424, z3zE2505, z2zE2425);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2554: ;
  {
    if (zop.kind != Kind_zEXTCODEHASH) goto case_2553;
    sail_fixed_bytes_20 z3zE2509;
    {
      sail_u256 z2zE2431;
      {
        z2zE2431 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
      z3zE2509 = zword_to_address(z2zE2431);
    }
    bool z3zE2510;
    z3zE2510 = zk_access_account(z3zE2509);
    uint64_t z2zE2427;
    z2zE2427 = zaccount_cost(z3zE2510);
    unit z3zE2459;
    {
      z3zE2459 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2427);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2428;
    z2zE2428 = zis_running(UNIT);
    if (z2zE2428) {
      sail_u256 z2zE2430;
      {
        sail_fixed_bytes_32 z2zE2429;
        {
          z2zE2429 = zk_get_codehash(z3zE2509);
          if (have_exception) {  goto end_block_exception_2570;  }
        }
        z2zE2430 = zhash_to_word(z2zE2429);
      }
      {
        z3zE2452 = zpush_word(z2zE2430);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2553: ;
  {
    if (zop.kind != Kind_zRETURNDATASIZE) goto case_2552;
    unit z3zE2457;
    {
      z3zE2457 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z2zE2433;
    {
      uint64_t z2zE2432;
      z2zE2432 = zreturndata_sizze(UNIT);
      z2zE2433 = zword_of_source_byte_count(z2zE2432);
    }
    {
      z3zE2452 = zpush_word(z2zE2433);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    goto finish_match_2549;
  }
case_2552: ;
  {
    if (zop.kind != Kind_zRETURNDATACOPY) goto case_2551;
    unit z3zE2454;
    {
      z3zE2454 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2511;
    {
      z3zE2511 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2512;
    {
      z3zE2512 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    sail_u256 z3zE2513;
    {
      z3zE2513 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    unit z3zE2455;
    {
      z3zE2455 = zcharge_copy_gas(z3zE2513);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    struct zMemoryRangeFields z3zE2514;
    {
      z3zE2514 = zcharge_memory_range(z3zE2511, z3zE2513);
      if (have_exception) {  goto end_block_exception_2570;  }
    }
    bool z2zE2434;
    z2zE2434 = zis_running(UNIT);
    if (z2zE2434) {
      uint64_t z2zE2435;
      z2zE2435 = z3zE2514.zoff;
      {
        z3zE2452 = zreturndata_copy_words(z2zE2435, z3zE2512, z3zE2513);
        if (have_exception) {  goto end_block_exception_2570;  }
      }
    } else {  z3zE2452 = UNIT;  }
    goto finish_match_2549;
  }
case_2551: ;
  {
    z3zE2452 = sail_assert(false, "sail/evm/execute.sail:383.21-383.22");
    goto finish_match_2549;
  }
case_2550: ;
finish_match_2549: ;
  z8zE423 = z3zE2452;
end_function_2569: ;
  return z8zE423;
end_block_exception_2570: ;

  return UNIT;
}

unit zexecute_block(struct zast zop)
{
  unit z8zE424;
  unit z3zE2428;
  {
    if (zop.kind != Kind_zBLOCKHASH) goto case_2546;
    unit z3zE2450;
    {
      z3zE2450 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(UINT64_C(20));
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2372;
    {
      sail_fixed_bytes_32 z2zE2371;
      {
        sail_u256 z2zE2370;
        {
          z2zE2370 = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2548;  }
        }
        {
          z2zE2371 = zk_blockhash(z2zE2370);
          if (have_exception) {  goto end_block_exception_2548;  }
        }
      }
      z2zE2372 = zhash_to_word(z2zE2371);
    }
    {
      z3zE2428 = zpush_word(z2zE2372);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2546: ;
  {
    if (zop.kind != Kind_zCOINBASE) goto case_2545;
    unit z3zE2448;
    {
      z3zE2448 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2373;
    z2zE2373 = zk_env(zF_Coinbase);
    {
      z3zE2428 = zpush_word(z2zE2373);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2545: ;
  {
    if (zop.kind != Kind_zTIMESTAMP) goto case_2544;
    unit z3zE2446;
    {
      z3zE2446 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2374;
    z2zE2374 = zk_env(zF_Timestamp);
    {
      z3zE2428 = zpush_word(z2zE2374);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2544: ;
  {
    if (zop.kind != Kind_zNUMBER) goto case_2543;
    unit z3zE2444;
    {
      z3zE2444 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2375;
    z2zE2375 = zk_env(zF_Number);
    {
      z3zE2428 = zpush_word(z2zE2375);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2543: ;
  {
    if (zop.kind != Kind_zSLOTNUM) goto case_2542;
    unit z3zE2442;
    {
      z3zE2442 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2376;
    z2zE2376 = zk_env(zF_SlotNumber);
    {
      z3zE2428 = zpush_word(z2zE2376);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2542: ;
  {
    if (zop.kind != Kind_zPREVRANDAO) goto case_2541;
    unit z3zE2440;
    {
      z3zE2440 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2377;
    z2zE2377 = zk_env(zF_PrevRandao);
    {
      z3zE2428 = zpush_word(z2zE2377);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2541: ;
  {
    if (zop.kind != Kind_zGASLIMIT) goto case_2540;
    unit z3zE2438;
    {
      z3zE2438 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2378;
    z2zE2378 = zk_env(zF_GasLimit);
    {
      z3zE2428 = zpush_word(z2zE2378);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2540: ;
  {
    if (zop.kind != Kind_zCHAINID) goto case_2539;
    unit z3zE2436;
    {
      z3zE2436 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2379;
    z2zE2379 = zk_env(zF_ChainId);
    {
      z3zE2428 = zpush_word(z2zE2379);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2539: ;
  {
    if (zop.kind != Kind_zBASEFEE) goto case_2538;
    unit z3zE2434;
    {
      z3zE2434 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2380;
    z2zE2380 = zk_env(zF_BaseFee);
    {
      z3zE2428 = zpush_word(z2zE2380);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2538: ;
  {
    if (zop.kind != Kind_zBLOBBASEFEE) goto case_2537;
    unit z3zE2432;
    {
      z3zE2432 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2382;
    {
      uint64_t z2zE2381;
      z2zE2381 = zk_header.zexcess_blob_gas;
      {
        z2zE2382 = zblob_base_fee(z2zE2381);
        if (have_exception) {  goto end_block_exception_2548;  }
      }
    }
    {
      z3zE2428 = zpush_word(z2zE2382);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2537: ;
  {
    if (zop.kind != Kind_zBLOBHASH) goto case_2536;
    unit z3zE2430;
    {
      z3zE2430 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    sail_u256 z2zE2384;
    {
      sail_u256 z2zE2383;
      {
        z2zE2383 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2548;  }
      }
      z2zE2384 = zk_blobhash(z2zE2383);
    }
    {
      z3zE2428 = zpush_word(z2zE2384);
      if (have_exception) {  goto end_block_exception_2548;  }
    }
    goto finish_match_2534;
  }
case_2536: ;
  {
    z3zE2428 = sail_assert(false, "sail/evm/execute.sail:432.21-432.22");
    goto finish_match_2534;
  }
case_2535: ;
finish_match_2534: ;
  z8zE424 = z3zE2428;
end_function_2547: ;
  return z8zE424;
end_block_exception_2548: ;

  return UNIT;
}

unit zexecute_memory(struct zast zop)
{
  unit z8zE425;
  unit z3zE2403;
  {
    if (zop.kind != Kind_zPOP) goto case_2529;
    unit z3zE2419;
    {
      z3zE2419 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 z2zE2351;
    {
      z2zE2351 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    unit z3zE2420;
    {
      z3zE2420 = UNIT;
      goto finish_match_2530;
    }
  case_2531: ;
    sail_match_failure("execute_memory");
  finish_match_2530: ;
    z3zE2403 = z3zE2420;
    goto finish_match_2520;
  }
case_2529: ;
  {
    if (zop.kind != Kind_zMLOAD) goto case_2528;
    unit z3zE2417;
    {
      z3zE2417 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zoffset_word, UINT64_C(32));
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    bool z2zE2352;
    z2zE2352 = zis_running(UNIT);
    if (z2zE2352) {
      sail_u256 z2zE2354;
      {
        uint64_t z2zE2353;
        z2zE2353 = zrange.zoff;
        z2zE2354 = zmem_load(z2zE2353);
      }
      {
        z3zE2403 = zpush_word(z2zE2354);
        if (have_exception) {  goto end_block_exception_2533;  }
      }
    } else {  z3zE2403 = UNIT;  }
    goto finish_match_2520;
  }
case_2528: ;
  {
    if (zop.kind != Kind_zMSTORE) goto case_2527;
    unit z3zE2415;
    {
      z3zE2415 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 z3zE2423;
    {
      z3zE2423 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 zv;
    {
      zv = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    struct zMemoryRangeFields z3zE2424;
    {
      z3zE2424 = zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z3zE2423, UINT64_C(32));
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    bool z2zE2355;
    z2zE2355 = zis_running(UNIT);
    if (z2zE2355) {
      uint64_t z2zE2356;
      z2zE2356 = z3zE2424.zoff;
      z3zE2403 = zmem_store(z2zE2356, zv);
    } else {  z3zE2403 = UNIT;  }
    goto finish_match_2520;
  }
case_2527: ;
  {
    if (zop.kind != Kind_zMSTORE8) goto case_2526;
    unit z3zE2413;
    {
      z3zE2413 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 z3zE2425;
    {
      z3zE2425 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 z3zE2426;
    {
      z3zE2426 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    struct zMemoryRangeFields z3zE2427;
    {
      z3zE2427 = zcharge_memory_range(z3zE2425, zWORD_ONE);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    bool z2zE2357;
    z2zE2357 = zis_running(UNIT);
    if (z2zE2357) {
      uint64_t z2zE2358;
      z2zE2358 = z3zE2427.zoff;
      z3zE2403 = zmem_store_byte(z2zE2358, z3zE2426);
    } else {  z3zE2403 = UNIT;  }
    goto finish_match_2520;
  }
case_2526: ;
  {
    if (zop.kind != Kind_zMSIZE) goto case_2525;
    unit z3zE2411;
    {
      z3zE2411 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 z2zE2362;
    {
      sail_u128 z2zE2361;
      {
        uint64_t z2zE2360;
        {
          uint64_t z2zE2359;
          z2zE2359 = zevm_memory_high_water(UNIT);
          z2zE2360 = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2359);
        }
        z2zE2361 = u128_mul_u64_u64(z2zE2360, UINT64_C(32));
      }
      z2zE2362 = zword_of_nat_byte_countzIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2361);
    }
    {
      z3zE2403 = zpush_word(z2zE2362);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    goto finish_match_2520;
  }
case_2525: ;
  {
    if (zop.kind != Kind_zMCOPY) goto case_2522;
    unit z3zE2405;
    {
      z3zE2405 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 zdestination_word;
    {
      zdestination_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 zsource_word;
    {
      zsource_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    unit z3zE2406;
    {
      z3zE2406 = zcharge_copy_gas(zlength_word);
      if (have_exception) {  goto end_block_exception_2533;  }
    }
    bool z2zE2363;
    z2zE2363 = zis_running(UNIT);
    if (z2zE2363) {
      uint64_t zavailable;
      zavailable = zgas_remaining;
      struct zMemoryPairExpansion zexpansion;
      {
        zexpansion = zmemory_pair_expansion(zdestination_word, zlength_word, zsource_word, zlength_word, zavailable);
        if (have_exception) {  goto end_block_exception_2533;  }
      }
      uint64_t z2zE2364;
      z2zE2364 = zexpansion.zcost;
      unit z3zE2408;
      {
        z3zE2408 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2364);
        if (have_exception) {  goto end_block_exception_2533;  }
      }
      bool z2zE2365;
      z2zE2365 = zis_running(UNIT);
      if (z2zE2365) {
        struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z2zE2366;
        z2zE2366 = zapply_memory_pair_expansion(zexpansion);
        unit z3zE2409;
        {
          struct zMemoryRangeFields zdestination;
          zdestination = z2zE2366.ztup0;
          struct zMemoryRangeFields zsource;
          zsource = z2zE2366.ztup1;
          uint64_t z2zE2367;
          z2zE2367 = zdestination.zoff;
          uint64_t z2zE2368;
          z2zE2368 = zsource.zoff;
          uint64_t z2zE2369;
          z2zE2369 = zdestination.zlen;
          z3zE2409 = zmem_mcopy(z2zE2367, z2zE2368, z2zE2369);
          goto finish_match_2523;
        }
      case_2524: ;
        sail_match_failure("execute_memory");
      finish_match_2523: ;
        z3zE2403 = z3zE2409;
      } else {  z3zE2403 = UNIT;  }
    } else {  z3zE2403 = UNIT;  }
    goto finish_match_2520;
  }
case_2522: ;
  {
    z3zE2403 = sail_assert(false, "sail/evm/execute.sail:488.21-488.22");
    goto finish_match_2520;
  }
case_2521: ;
finish_match_2520: ;
  z8zE425 = z3zE2403;
end_function_2532: ;
  return z8zE425;
end_block_exception_2533: ;

  return UNIT;
}

unit zexecute_storage(struct zast zop)
{
  unit z8zE426;
  unit z3zE2380;
  {
    if (zop.kind != Kind_zSLOAD) goto case_2515;
    sail_u256 zs;
    {
      zs = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    bool zwarm;
    {
      sail_fixed_bytes_20 z2zE2319;
      z2zE2319 = zself_addr(UNIT);
      zwarm = zk_slot_is_warm(z2zE2319, zs);
    }
    uint64_t z2zE2314;
    z2zE2314 = zsload_cost(zwarm);
    unit z3zE2396;
    {
      z3zE2396 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2314);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    bool z2zE2315;
    z2zE2315 = zis_running(UNIT);
    if (z2zE2315) {
      sail_u256 z2zE2318;
      {
        struct zStorageValue z2zE2317;
        {
          sail_fixed_bytes_20 z2zE2316;
          z2zE2316 = zself_addr(UNIT);
          {
            z2zE2317 = zk_sload(z2zE2316, zs);
            if (have_exception) {  goto end_block_exception_2519;  }
          }
        }
        z2zE2318 = z2zE2317.zcurr;
      }
      {
        z3zE2380 = zpush_word(z2zE2318);
        if (have_exception) {  goto end_block_exception_2519;  }
      }
    } else {  z3zE2380 = UNIT;  }
    goto finish_match_2510;
  }
case_2515: ;
  {
    if (zop.kind != Kind_zSSTORE) goto case_2514;
    bool z2zE2320;
    {
      z2zE2320 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    if (z2zE2320) {  z3zE2380 = UNIT;  } else {
      bool z2zE2322;
      {
        bool z2zE2321;
        z2zE2321 = zfork_lt(zk_fork, zAmsterdam);
        bool z3zE2387;
        if (z2zE2321) {  z3zE2387 = (!(zG_callstipend < zgas_remaining));  } else {  z3zE2387 = false;  }
        z2zE2322 = z3zE2387;
      }
      if (z2zE2322) {
        {
          z3zE2380 = zexc_halt(zOutOfGas);
          if (have_exception) {  goto end_block_exception_2519;  }
        }
      } else {
        sail_u256 z3zE2398;
        {
          z3zE2398 = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2519;  }
        }
        sail_u256 zv;
        {
          zv = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2519;  }
        }
        bool z2zE2323;
        z2zE2323 = zis_running(UNIT);
        if (z2zE2323) {
          bool z3zE2399;
          {
            sail_fixed_bytes_20 z2zE2346;
            z2zE2346 = zself_addr(UNIT);
            z3zE2399 = zk_slot_is_warm(z2zE2346, z3zE2398);
          }
          bool zcold;
          zcold = not(z3zE2399);
          bool z2zE2324;
          z2zE2324 = zfork_gteq(zk_fork, zAmsterdam);
          unit z3zE2389;
          if (z2zE2324) {
            uint64_t zaccess_cost;
            zaccess_cost = zamsterdam_storage_access_cost(zcold);
            uint64_t zsentry_cost;
            {
              bool z2zE2325;
              z2zE2325 = (zaccess_cost < zG_sstore_sentry);
              if (z2zE2325) {  zsentry_cost = zG_sstore_sentry;  } else {  zsentry_cost = zaccess_cost;  }
            }
            {
              z3zE2389 = zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zsentry_cost);
              if (have_exception) {  goto end_block_exception_2519;  }
            }
          } else {  z3zE2389 = UNIT;  }
          bool z2zE2327;
          {
            bool z2zE2326;
            z2zE2326 = zis_running(UNIT);
            z2zE2327 = not(z2zE2326);
          }
          unit z3zE2388;
          if (z2zE2327) {
            z8zE426 = UNIT;
            goto cleanup_2517;
            /* unreachable after return */
            goto end_cleanup_2518;
          cleanup_2517: ;
            goto end_function_2516;
          end_cleanup_2518: ;
          } else {  z3zE2388 = UNIT;  }
          struct zStorageValue zentry;
          {
            sail_fixed_bytes_20 z2zE2345;
            z2zE2345 = zself_addr(UNIT);
            {
              zentry = zk_sload(z2zE2345, z3zE2398);
              if (have_exception) {  goto end_block_exception_2519;  }
            }
          }
          struct zSstoreCosts zcosts;
          {
            sail_u256 z2zE2343;
            z2zE2343 = zentry.zorig;
            sail_u256 z2zE2344;
            z2zE2344 = zentry.zcurr;
            {
              zcosts = zsstore_costs(z2zE2343, z2zE2344, zv, zcold);
              if (have_exception) {  goto end_block_exception_2519;  }
            }
          }
          bool z2zE2329;
          {
            uint64_t z2zE2328;
            z2zE2328 = zcosts.zstate_credit;
            z2zE2329 = (z2zE2328 != UINT64_C(0));
          }
          unit z3zE2393;
          if (z2zE2329) {
            uint64_t z2zE2330;
            z2zE2330 = zcosts.zstate_credit;
            {
              z3zE2393 = zcredit_state_gas_refund(z2zE2330);
              if (have_exception) {  goto end_block_exception_2519;  }
            }
          } else {  z3zE2393 = UNIT;  }
          uint64_t z2zE2331;
          z2zE2331 = zcosts.zexecution;
          unit z3zE2392;
          {
            z3zE2392 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2331);
            if (have_exception) {  goto end_block_exception_2519;  }
          }
          uint64_t z2zE2332;
          z2zE2332 = zcosts.zstate_charge;
          unit z3zE2391;
          {
            z3zE2391 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2332);
            if (have_exception) {  goto end_block_exception_2519;  }
          }
          bool z2zE2335;
          {
            bool z2zE2334;
            {
              __int128 z2zE2333;
              z2zE2333 = zcosts.zrefund;
              z2zE2334 = (z2zE2333 == zGAS_REFUND_ZERO);
            }
            z2zE2335 = not(z2zE2334);
          }
          unit z3zE2390;
          if (z2zE2335) {
            __int128 z2zE2336;
            z2zE2336 = zcosts.zrefund;
            {
              z3zE2390 = zrecord_refund(z2zE2336);
              if (have_exception) {  goto end_block_exception_2519;  }
            }
          } else {  z3zE2390 = UNIT;  }
          bool z2zE2339;
          {
            bool z2zE2338;
            z2zE2338 = zis_running(UNIT);
            bool z3zE2394;
            if (z2zE2338) {
              sail_u256 z2zE2337;
              z2zE2337 = zentry.zcurr;
              z3zE2394 = (!eq_u256(z2zE2337, zv));
            } else {  z3zE2394 = false;  }
            z2zE2339 = z3zE2394;
          }
          if (z2zE2339) {
            sail_fixed_bytes_20 z2zE2341;
            z2zE2341 = zself_addr(UNIT);
            struct zStorageValue z2zE2342;
            {
              sail_u256 z2zE2340;
              z2zE2340 = zentry.zorig;
              struct zStorageValue z3zE2395;
              z3zE2395.zcurr = zv;
              z3zE2395.zorig = z2zE2340;
              z2zE2342 = z3zE2395;
            }
            z3zE2380 = zk_sstore(z2zE2341, z3zE2398, z2zE2342);
          } else {  z3zE2380 = UNIT;  }
        } else {  z3zE2380 = UNIT;  }
      }
    }
    goto finish_match_2510;
  }
case_2514: ;
  {
    if (zop.kind != Kind_zTLOAD) goto case_2513;
    unit z3zE2384;
    {
      z3zE2384 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_warm_access);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    sail_u256 z3zE2400;
    {
      z3zE2400 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    sail_u256 z2zE2348;
    {
      sail_fixed_bytes_20 z2zE2347;
      z2zE2347 = zself_addr(UNIT);
      z2zE2348 = zk_tload(z2zE2347, z3zE2400);
    }
    {
      z3zE2380 = zpush_word(z2zE2348);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    goto finish_match_2510;
  }
case_2513: ;
  {
    if (zop.kind != Kind_zTSTORE) goto case_2512;
    bool z2zE2349;
    {
      z2zE2349 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2519;  }
    }
    if (z2zE2349) {  z3zE2380 = UNIT;  } else {
      unit z3zE2383;
      {
        z3zE2383 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_warm_access);
        if (have_exception) {  goto end_block_exception_2519;  }
      }
      sail_u256 z3zE2401;
      {
        z3zE2401 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2519;  }
      }
      sail_u256 z3zE2402;
      {
        z3zE2402 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2519;  }
      }
      sail_fixed_bytes_20 z2zE2350;
      z2zE2350 = zself_addr(UNIT);
      z3zE2380 = zk_tstore(z2zE2350, z3zE2401, z3zE2402);
    }
    goto finish_match_2510;
  }
case_2512: ;
  {
    z3zE2380 = sail_assert(false, "sail/evm/execute.sail:556.21-556.22");
    goto finish_match_2510;
  }
case_2511: ;
finish_match_2510: ;
  z8zE426 = z3zE2380;
end_function_2516: ;
  return z8zE426;
end_block_exception_2519: ;

  return UNIT;
}

unit zexecute_control(struct zast zop)
{
  unit z8zE427;
  unit z3zE2342;
  {
    if (zop.kind != Kind_zJUMP) goto case_2507;
    unit z3zE2368;
    {
      z3zE2368 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    sail_u256 zdest;
    {
      zdest = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    {
      z3zE2342 = zdo_jump(zdest);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    goto finish_match_2493;
  }
case_2507: ;
  {
    if (zop.kind != Kind_zJUMPI) goto case_2506;
    unit z3zE2366;
    {
      z3zE2366 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_high);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    sail_u256 z3zE2370;
    {
      z3zE2370 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    sail_u256 zcond;
    {
      zcond = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    bool z2zE2283;
    z2zE2283 = zword_is_zzero(zcond);
    if (z2zE2283) {  z3zE2342 = UNIT;  } else {
      {
        z3zE2342 = zdo_jump(z3zE2370);
        if (have_exception) {  goto end_block_exception_2509;  }
      }
    }
    goto finish_match_2493;
  }
case_2506: ;
  {
    if (zop.kind != Kind_zPC) goto case_2505;
    unit z3zE2364;
    {
      z3zE2364 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    sail_u256 z2zE2285;
    {
      sail_u256 z2zE2284;
      z2zE2284 = zword_of_source_byte_count(zpc);
      z2zE2285 = zalu_sub(z2zE2284, zWORD_ONE);
    }
    {
      z3zE2342 = zpush_word(z2zE2285);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    goto finish_match_2493;
  }
case_2505: ;
  {
    if (zop.kind != Kind_zGAS) goto case_2504;
    unit z3zE2362;
    {
      z3zE2362 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    {
      z3zE2342 = zpush_gas(zgas_remaining);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    goto finish_match_2493;
  }
case_2504: ;
  {
    if (zop.kind != Kind_zJUMPDEST) goto case_2503;
    {
      z3zE2342 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_jumpdest);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    goto finish_match_2493;
  }
case_2503: ;
  {
    if (zop.kind != Kind_zPUSH) goto case_2502;
    uint64_t zn;
    zn = zop.variants.zPUSH.ztup0;
    sail_u256 zv;
    zv = zop.variants.zPUSH.ztup1;
    bool z2zE2286;
    z2zE2286 = (zn == UINT64_C(0));
    unit z3zE2359;
    if (z2zE2286) {
      {
        z3zE2359 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
        if (have_exception) {  goto end_block_exception_2509;  }
      }
    } else {
      {
        z3zE2359 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
        if (have_exception) {  goto end_block_exception_2509;  }
      }
    }
    {
      z3zE2342 = zpush_word(zv);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    goto finish_match_2493;
  }
case_2502: ;
  {
    if (zop.kind != Kind_zDUP) goto case_2501;
    uint64_t z3zE2371;
    z3zE2371 = zop.variants.zDUP;
    unit z3zE2357;
    {
      z3zE2357 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    bool z2zE2288;
    {
      uint64_t z2zE2287;
      z2zE2287 = zstack_height(UNIT);
      z2zE2288 = (z2zE2287 < z3zE2371);
    }
    if (z2zE2288) {
      {
        z3zE2342 = zexc_halt(zStackUnderflow);
        if (have_exception) {  goto end_block_exception_2509;  }
      }
    } else {
      sail_u256 z2zE2290;
      {
        uint64_t z2zE2289;
        {    z2zE2289 = (z3zE2371 - UINT64_C(1));
        }
        z2zE2290 = zpeek(z2zE2289);
      }
      {
        z3zE2342 = zpush_word(z2zE2290);
        if (have_exception) {  goto end_block_exception_2509;  }
      }
    }
    goto finish_match_2493;
  }
case_2501: ;
  {
    if (zop.kind != Kind_zSWAP) goto case_2500;
    uint64_t z3zE2372;
    z3zE2372 = zop.variants.zSWAP;
    unit z3zE2354;
    {
      z3zE2354 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    bool z2zE2293;
    {
      uint64_t z2zE2291;
      z2zE2291 = zstack_height(UNIT);
      uint64_t z2zE2292;
      {    z2zE2292 = (z3zE2372 + UINT64_C(1));
      }
      z2zE2293 = (z2zE2291 < z2zE2292);
    }
    if (z2zE2293) {
      {
        z3zE2342 = zexc_halt(zStackUnderflow);
        if (have_exception) {  goto end_block_exception_2509;  }
      }
    } else {
      sail_u256 ztop;
      ztop = zpeek(UINT64_C(0));
      sail_u256 zother;
      zother = zpeek(z3zE2372);
      unit z3zE2356;
      z3zE2356 = zstack_set(UINT64_C(0), zother);
      z3zE2342 = zstack_set(z3zE2372, ztop);
    }
    goto finish_match_2493;
  }
case_2500: ;
  {
    if (zop.kind != Kind_zDUPN) goto case_2499;
    uint64_t zimmediate;
    zimmediate = zop.variants.zDUPN;
    unit z3zE2352;
    {
      z3zE2352 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    bool z2zE2294;
    z2zE2294 = zis_running(UNIT);
    if (z2zE2294) {
      bool z2zE2296;
      {
        bool z2zE2295;
        z2zE2295 = zdeep_stack_immediate_valid(zimmediate);
        z2zE2296 = not(z2zE2295);
      }
      if (z2zE2296) {
        {
          z3zE2342 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2509;  }
        }
      } else {
        uint64_t z3zE2373;
        z3zE2373 = zdecode_single_stack_index(zimmediate);
        bool z2zE2298;
        {
          uint64_t z2zE2297;
          z2zE2297 = zstack_height(UNIT);
          z2zE2298 = (z2zE2297 < z3zE2373);
        }
        if (z2zE2298) {
          {
            z3zE2342 = zexc_halt(zStackUnderflow);
            if (have_exception) {  goto end_block_exception_2509;  }
          }
        } else {
          sail_u256 z2zE2300;
          {
            uint64_t z2zE2299;
            {    z2zE2299 = (z3zE2373 - UINT64_C(1));
            }
            z2zE2300 = zpeek(z2zE2299);
          }
          {
            z3zE2342 = zpush_word(z2zE2300);
            if (have_exception) {  goto end_block_exception_2509;  }
          }
        }
      }
    } else {  z3zE2342 = UNIT;  }
    goto finish_match_2493;
  }
case_2499: ;
  {
    if (zop.kind != Kind_zSWAPN) goto case_2498;
    uint64_t z3zE2374;
    z3zE2374 = zop.variants.zSWAPN;
    unit z3zE2349;
    {
      z3zE2349 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    bool z2zE2301;
    z2zE2301 = zis_running(UNIT);
    if (z2zE2301) {
      bool z2zE2303;
      {
        bool z2zE2302;
        z2zE2302 = zdeep_stack_immediate_valid(z3zE2374);
        z2zE2303 = not(z2zE2302);
      }
      if (z2zE2303) {
        {
          z3zE2342 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2509;  }
        }
      } else {
        uint64_t z3zE2375;
        z3zE2375 = zdecode_single_stack_index(z3zE2374);
        bool z2zE2306;
        {
          uint64_t z2zE2304;
          z2zE2304 = zstack_height(UNIT);
          uint64_t z2zE2305;
          {    z2zE2305 = (z3zE2375 + UINT64_C(1));
          }
          z2zE2306 = (z2zE2304 < z2zE2305);
        }
        if (z2zE2306) {
          {
            z3zE2342 = zexc_halt(zStackUnderflow);
            if (have_exception) {  goto end_block_exception_2509;  }
          }
        } else {
          sail_u256 z3zE2376;
          z3zE2376 = zpeek(UINT64_C(0));
          sail_u256 z3zE2377;
          z3zE2377 = zpeek(z3zE2375);
          unit z3zE2351;
          z3zE2351 = zstack_set(UINT64_C(0), z3zE2377);
          z3zE2342 = zstack_set(z3zE2375, z3zE2376);
        }
      }
    } else {  z3zE2342 = UNIT;  }
    goto finish_match_2493;
  }
case_2498: ;
  {
    if (zop.kind != Kind_zEXCHANGE) goto case_2495;
    uint64_t z3zE2378;
    z3zE2378 = zop.variants.zEXCHANGE;
    unit z3zE2344;
    {
      z3zE2344 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2509;  }
    }
    bool z2zE2307;
    z2zE2307 = zis_running(UNIT);
    if (z2zE2307) {
      bool z2zE2309;
      {
        bool z2zE2308;
        z2zE2308 = zexchange_immediate_valid(z3zE2378);
        z2zE2309 = not(z2zE2308);
      }
      if (z2zE2309) {
        {
          z3zE2342 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2509;  }
        }
      } else {
        struct ztuple_z8z5u64zCz0z5u64z9 z2zE2310;
        z2zE2310 = zdecode_exchange_stack_indices(z3zE2378);
        unit z3zE2346;
        {
          uint64_t z3zE2379;
          z3zE2379 = z2zE2310.ztup0;
          uint64_t zm;
          zm = z2zE2310.ztup1;
          bool z2zE2313;
          {
            uint64_t z2zE2311;
            z2zE2311 = zstack_height(UNIT);
            uint64_t z2zE2312;
            {    z2zE2312 = (zm + UINT64_C(1));
            }
            z2zE2313 = (z2zE2311 < z2zE2312);
          }
          if (z2zE2313) {
            {
              z3zE2346 = zexc_halt(zStackUnderflow);
              if (have_exception) {  goto end_block_exception_2509;  }
            }
          } else {
            sail_u256 zfirst;
            zfirst = zpeek(z3zE2379);
            sail_u256 zsecond;
            zsecond = zpeek(zm);
            unit z3zE2348;
            z3zE2348 = zstack_set(z3zE2379, zsecond);
            z3zE2346 = zstack_set(zm, zfirst);
          }
          goto finish_match_2496;
        }
      case_2497: ;
        sail_match_failure("execute_control");
      finish_match_2496: ;
        z3zE2342 = z3zE2346;
      }
    } else {  z3zE2342 = UNIT;  }
    goto finish_match_2493;
  }
case_2495: ;
  {
    z3zE2342 = sail_assert(false, "sail/evm/execute.sail:663.21-663.22");
    goto finish_match_2493;
  }
case_2494: ;
finish_match_2493: ;
  z8zE427 = z3zE2342;
end_function_2508: ;
  return z8zE427;
end_block_exception_2509: ;

  return UNIT;
}

unit zexecute_log(struct zast zop)
{
  unit z8zE428;
  unit z3zE2338;
  {
    if (zop.kind != Kind_zLOG) goto case_2490;
    uint64_t zn;
    zn = zop.variants.zLOG;
    bool z2zE2276;
    {
      z2zE2276 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2492;  }
    }
    if (z2zE2276) {  z3zE2338 = UNIT;  } else {
      sail_u256 zoffset_word;
      {
        zoffset_word = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2492;  }
      }
      sail_u256 zlength_word;
      {
        zlength_word = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2492;  }
      }
      zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
      {
        zpop_log_topics(&ztopics, zn);
        if (have_exception) {
          KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
          goto end_block_exception_2492;
        }
      }
      unit z3zE2341;
      {
        z3zE2341 = zcharge_log_gas(zn, zlength_word);
        if (have_exception) {
          KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
          goto end_block_exception_2492;
        }
      }
      struct zMemoryRangeFields zrange;
      {
        zrange = zcharge_memory_range(zoffset_word, zlength_word);
        if (have_exception) {
          KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
          goto end_block_exception_2492;
        }
      }
      bool z2zE2277;
      z2zE2277 = zis_running(UNIT);
      if (z2zE2277) {
        sail_fixed_bytes_20 z2zE2281;
        z2zE2281 = zself_addr(UNIT);
        struct zBytes z2zE2282;
        CREATE(zBytes)(&z2zE2282);
        {
          struct zByteSliceFields z2zE2280;
          {
            uint64_t z2zE2278;
            z2zE2278 = zrange.zoff;
            uint64_t z2zE2279;
            z2zE2279 = zrange.zlen;
            z2zE2280 = zmemory_byte_slice(z2zE2278, z2zE2279);
          }
          zBytesSlice(&z2zE2282, z2zE2280);
        }
        z3zE2338 = zk_log(z2zE2281, ztopics, z2zE2282);
        KILL(zBytes)(&z2zE2282);
      } else {  z3zE2338 = UNIT;  }
      KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
    }
    goto finish_match_2488;
  }
case_2490: ;
  {
    z3zE2338 = sail_assert(false, "sail/evm/execute.sail:682.21-682.22");
    goto finish_match_2488;
  }
case_2489: ;
finish_match_2488: ;
  z8zE428 = z3zE2338;
end_function_2491: ;
  return z8zE428;
end_block_exception_2492: ;

  return UNIT;
}

unit zexecute_halt(struct zast zop)
{
  unit z8zE429;
  unit z3zE2306;
  {
    if (zop.kind != Kind_zSTOP) goto case_2485;
    struct zHaltKind z2zE2228;
    CREATE(zHaltKind)(&z2zE2228);
    zHaltStop(&z2zE2228, UNIT);
    zHalted(&zframe_status, z2zE2228);
    z3zE2306 = UNIT;
    KILL(zHaltKind)(&z2zE2228);
    goto finish_match_2479;
  }
case_2485: ;
  {
    if (zop.kind != Kind_zRETURN) goto case_2484;
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_range(zoffset_word, zlength_word);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    bool z2zE2229;
    z2zE2229 = zis_running(UNIT);
    if (z2zE2229) {
      struct zHaltKind z2zE2234;
      CREATE(zHaltKind)(&z2zE2234);
      {
        struct zByteSliceFields z2zE2233;
        {
          struct zByteSliceFields z2zE2232;
          {
            uint64_t z2zE2230;
            z2zE2230 = zrange.zoff;
            uint64_t z2zE2231;
            z2zE2231 = zrange.zlen;
            z2zE2232 = zmemory_byte_slice(z2zE2230, z2zE2231);
          }
          z2zE2233 = zfreezze_output(z2zE2232);
        }
        zHaltReturn(&z2zE2234, z2zE2233);
      }
      zHalted(&zframe_status, z2zE2234);
      z3zE2306 = UNIT;
      KILL(zHaltKind)(&z2zE2234);
    } else {  z3zE2306 = UNIT;  }
    goto finish_match_2479;
  }
case_2484: ;
  {
    if (zop.kind != Kind_zREVERT) goto case_2483;
    sail_u256 z3zE2333;
    {
      z3zE2333 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    sail_u256 z3zE2334;
    {
      z3zE2334 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    struct zMemoryRangeFields z3zE2335;
    {
      z3zE2335 = zcharge_memory_range(z3zE2333, z3zE2334);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    bool z2zE2235;
    z2zE2235 = zis_running(UNIT);
    if (z2zE2235) {
      unit z3zE2330;
      {
        z3zE2330 = zrefill_frame_state_gas(UNIT);
        if (have_exception) {  goto end_block_exception_2487;  }
      }
      struct zHaltKind z2zE2240;
      CREATE(zHaltKind)(&z2zE2240);
      {
        struct zByteSliceFields z2zE2239;
        {
          struct zByteSliceFields z2zE2238;
          {
            uint64_t z2zE2236;
            z2zE2236 = z3zE2335.zoff;
            uint64_t z2zE2237;
            z2zE2237 = z3zE2335.zlen;
            z2zE2238 = zmemory_byte_slice(z2zE2236, z2zE2237);
          }
          z2zE2239 = zfreezze_output(z2zE2238);
        }
        zHaltRevert(&z2zE2240, z2zE2239);
      }
      zHalted(&zframe_status, z2zE2240);
      z3zE2306 = UNIT;
      KILL(zHaltKind)(&z2zE2240);
    } else {  z3zE2306 = UNIT;  }
    goto finish_match_2479;
  }
case_2483: ;
  {
    if (zop.kind != Kind_zINVALID) goto case_2482;
    zgas_remaining = zGAS_ZERO;
    unit z3zE2327;
    z3zE2327 = UNIT;
    {
      z3zE2306 = zexc_halt(zInvalidOpcode);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    goto finish_match_2479;
  }
case_2482: ;
  {
    if (zop.kind != Kind_zSELFDESTRUCT) goto case_2481;
    bool z2zE2241;
    {
      z2zE2241 = zguard_static(UNIT);
      if (have_exception) {  goto end_block_exception_2487;  }
    }
    if (z2zE2241) {  z3zE2306 = UNIT;  } else {
      sail_fixed_bytes_20 zbeneficiary;
      {
        sail_u256 z2zE2275;
        {
          z2zE2275 = zpop(UNIT);
          if (have_exception) {  goto end_block_exception_2487;  }
        }
        zbeneficiary = zword_to_address(z2zE2275);
      }
      bool z2zE2242;
      z2zE2242 = zfork_gteq(zk_fork, zAmsterdam);
      if (z2zE2242) {
        bool zwarm;
        zwarm = zk_access_account(zbeneficiary);
        uint64_t zaccess_cost;
        {
          uint64_t z2zE2254;
          {    z2zE2254 = (UINT64_C(0) + zG_selfdestruct);
          }
          uint64_t z2zE2255;
          if (zwarm) {  z2zE2255 = zG_zzero;  } else {  z2zE2255 = zG_amsterdam_cold_account_access;  }
          {    zaccess_cost = (z2zE2254 + z2zE2255);
          }
        }
        unit z3zE2320;
        {
          z3zE2320 = zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zaccess_cost);
          if (have_exception) {  goto end_block_exception_2487;  }
        }
        bool z2zE2243;
        z2zE2243 = zis_running(UNIT);
        if (z2zE2243) {
          sail_u256 zbal;
          {
            sail_fixed_bytes_20 z2zE2253;
            z2zE2253 = zself_addr(UNIT);
            {
              zbal = zk_get_balance(z2zE2253);
              if (have_exception) {  goto end_block_exception_2487;  }
            }
          }
          bool zcreates_account;
          {
            bool z2zE2252;
            z2zE2252 = zword_nonzzero(zbal);
            bool z3zE2321;
            if (z2zE2252) {
              {
                z3zE2321 = zk_account_is_empty(zbeneficiary);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            } else {  z3zE2321 = false;  }
            zcreates_account = z3zE2321;
          }
          uint64_t zexecution_cost;
          if (zcreates_account) {
            {    zexecution_cost = (zaccess_cost + zG_amsterdam_account_write);
            }
          } else {  zexecution_cost = zaccess_cost;  }
          unit z3zE2324;
          {
            z3zE2324 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zexecution_cost);
            if (have_exception) {  goto end_block_exception_2487;  }
          }
          bool z2zE2245;
          {
            bool z2zE2244;
            z2zE2244 = zis_running(UNIT);
            bool z3zE2322;
            if (z2zE2244) {  z3zE2322 = zcreates_account;  } else {  z3zE2322 = false;  }
            z2zE2245 = z3zE2322;
          }
          unit z3zE2323;
          if (z2zE2245) {
            {
              z3zE2323 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2487;  }
            }
          } else {  z3zE2323 = UNIT;  }
          bool z2zE2246;
          z2zE2246 = zis_running(UNIT);
          if (z2zE2246) {
            sail_fixed_bytes_20 z2zE2247;
            z2zE2247 = zself_addr(UNIT);
            unit z3zE2326;
            {
              z3zE2326 = zk_transfer(z2zE2247, zbeneficiary, zbal);
              if (have_exception) {  goto end_block_exception_2487;  }
            }
            bool z2zE2249;
            {
              sail_fixed_bytes_20 z2zE2248;
              z2zE2248 = zself_addr(UNIT);
              {
                z2zE2249 = zk_was_created(z2zE2248);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            }
            unit z3zE2325;
            if (z2zE2249) {
              sail_fixed_bytes_20 z2zE2250;
              z2zE2250 = zself_addr(UNIT);
              {
                z3zE2325 = zk_selfdestruct(z2zE2250);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            } else {  z3zE2325 = UNIT;  }
            struct zHaltKind z2zE2251;
            CREATE(zHaltKind)(&z2zE2251);
            zHaltSelfDestruct(&z2zE2251, UNIT);
            zHalted(&zframe_status, z2zE2251);
            z3zE2306 = UNIT;
            KILL(zHaltKind)(&z2zE2251);
          } else {  z3zE2306 = UNIT;  }
        } else {  z3zE2306 = UNIT;  }
      } else {
        sail_u256 z3zE2336;
        {
          sail_fixed_bytes_20 z2zE2274;
          z2zE2274 = zself_addr(UNIT);
          {
            z3zE2336 = zk_get_balance(z2zE2274);
            if (have_exception) {  goto end_block_exception_2487;  }
          }
        }
        bool z3zE2337;
        z3zE2337 = zk_access_account(zbeneficiary);
        unit z3zE2313;
        {
          z3zE2313 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_selfdestruct);
          if (have_exception) {  goto end_block_exception_2487;  }
        }
        bool z2zE2256;
        z2zE2256 = not(z3zE2337);
        unit z3zE2312;
        if (z2zE2256) {
          {
            z3zE2312 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_cold_account);
            if (have_exception) {  goto end_block_exception_2487;  }
          }
        } else {  z3zE2312 = UNIT;  }
        bool z2zE2259;
        {
          bool z2zE2258;
          z2zE2258 = zis_running(UNIT);
          bool z3zE2310;
          if (z2zE2258) {
            bool z2zE2257;
            z2zE2257 = zword_nonzzero(z3zE2336);
            bool z3zE2309;
            if (z2zE2257) {
              {
                z3zE2309 = zk_account_is_empty(zbeneficiary);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            } else {  z3zE2309 = false;  }
            z3zE2310 = z3zE2309;
          } else {  z3zE2310 = false;  }
          z2zE2259 = z3zE2310;
        }
        unit z3zE2311;
        if (z2zE2259) {
          {
            z3zE2311 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_newaccount);
            if (have_exception) {  goto end_block_exception_2487;  }
          }
        } else {  z3zE2311 = UNIT;  }
        bool z2zE2260;
        z2zE2260 = zis_running(UNIT);
        if (z2zE2260) {
          bool zfirst_selfdestruct;
          {
            bool z2zE2273;
            {
              sail_fixed_bytes_20 z2zE2272;
              z2zE2272 = zself_addr(UNIT);
              {
                z2zE2273 = zk_is_selfdestructed(z2zE2272);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            }
            zfirst_selfdestruct = not(z2zE2273);
          }
          bool z2zE2262;
          {
            bool z2zE2261;
            z2zE2261 = zfork_lt(zk_fork, zLondon);
            bool z3zE2314;
            if (z2zE2261) {  z3zE2314 = zfirst_selfdestruct;  } else {  z3zE2314 = false;  }
            z2zE2262 = z3zE2314;
          }
          unit z3zE2319;
          if (z2zE2262) {
            {
              z3zE2319 = zrecord_refundzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zR_selfdestruct_pre_london);
              if (have_exception) {  goto end_block_exception_2487;  }
            }
          } else {  z3zE2319 = UNIT;  }
          sail_fixed_bytes_20 z2zE2263;
          z2zE2263 = zself_addr(UNIT);
          unit z3zE2318;
          {
            z3zE2318 = zk_transfer(z2zE2263, zbeneficiary, z3zE2336);
            if (have_exception) {  goto end_block_exception_2487;  }
          }
          bool z2zE2264;
          z2zE2264 = zfork_lt(zk_fork, zCancun);
          unit z3zE2315;
          if (z2zE2264) {
            sail_fixed_bytes_20 z2zE2265;
            z2zE2265 = zself_addr(UNIT);
            unit z3zE2317;
            {
              z3zE2317 = zk_zzero_balance(z2zE2265);
              if (have_exception) {  goto end_block_exception_2487;  }
            }
            sail_fixed_bytes_20 z2zE2266;
            z2zE2266 = zself_addr(UNIT);
            {
              z3zE2315 = zk_selfdestruct(z2zE2266);
              if (have_exception) {  goto end_block_exception_2487;  }
            }
          } else {
            bool z2zE2268;
            {
              sail_fixed_bytes_20 z2zE2267;
              z2zE2267 = zself_addr(UNIT);
              {
                z2zE2268 = zk_was_created(z2zE2267);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            }
            if (z2zE2268) {
              sail_fixed_bytes_20 z2zE2269;
              z2zE2269 = zself_addr(UNIT);
              unit z3zE2316;
              {
                z3zE2316 = zk_zzero_balance(z2zE2269);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
              sail_fixed_bytes_20 z2zE2270;
              z2zE2270 = zself_addr(UNIT);
              {
                z3zE2315 = zk_selfdestruct(z2zE2270);
                if (have_exception) {  goto end_block_exception_2487;  }
              }
            } else {  z3zE2315 = UNIT;  }
          }
          struct zHaltKind z2zE2271;
          CREATE(zHaltKind)(&z2zE2271);
          zHaltSelfDestruct(&z2zE2271, UNIT);
          zHalted(&zframe_status, z2zE2271);
          z3zE2306 = UNIT;
          KILL(zHaltKind)(&z2zE2271);
        } else {  z3zE2306 = UNIT;  }
      }
    }
    goto finish_match_2479;
  }
case_2481: ;
  {
    z3zE2306 = sail_assert(false, "sail/evm/execute.sail:771.21-771.22");
    goto finish_match_2479;
  }
case_2480: ;
finish_match_2479: ;
  z8zE429 = z3zE2306;
end_function_2486: ;
  return z8zE429;
end_block_exception_2487: ;

  return UNIT;
}

unit zexecute_opcode(struct zast zop)
{
  unit z8zE430;
  unit z3zE2217;
  {
    if (zop.kind != Kind_zSTOP) goto case_2476;
    {
      z3zE2217 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2476: ;
  {
    if (zop.kind != Kind_zADD) goto case_2475;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2475: ;
  {
    if (zop.kind != Kind_zMUL) goto case_2474;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2474: ;
  {
    if (zop.kind != Kind_zSUB) goto case_2473;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2473: ;
  {
    if (zop.kind != Kind_zDIV) goto case_2472;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2472: ;
  {
    if (zop.kind != Kind_zSDIV) goto case_2471;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2471: ;
  {
    if (zop.kind != Kind_zMOD) goto case_2470;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2470: ;
  {
    if (zop.kind != Kind_zSMOD) goto case_2469;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2469: ;
  {
    if (zop.kind != Kind_zADDMOD) goto case_2468;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2468: ;
  {
    if (zop.kind != Kind_zMULMOD) goto case_2467;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2467: ;
  {
    if (zop.kind != Kind_zEXP) goto case_2466;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2466: ;
  {
    if (zop.kind != Kind_zSIGNEXTEND) goto case_2465;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2465: ;
  {
    if (zop.kind != Kind_zLT) goto case_2464;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2464: ;
  {
    if (zop.kind != Kind_zGT) goto case_2463;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2463: ;
  {
    if (zop.kind != Kind_zSLT) goto case_2462;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2462: ;
  {
    if (zop.kind != Kind_zSGT) goto case_2461;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2461: ;
  {
    if (zop.kind != Kind_zEQ) goto case_2460;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2460: ;
  {
    if (zop.kind != Kind_zISZERO) goto case_2459;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2459: ;
  {
    if (zop.kind != Kind_zAND) goto case_2458;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2458: ;
  {
    if (zop.kind != Kind_zOR) goto case_2457;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2457: ;
  {
    if (zop.kind != Kind_zXOR) goto case_2456;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2456: ;
  {
    if (zop.kind != Kind_zNOT) goto case_2455;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2455: ;
  {
    if (zop.kind != Kind_zBYTE) goto case_2454;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2454: ;
  {
    if (zop.kind != Kind_zSHL) goto case_2453;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2453: ;
  {
    if (zop.kind != Kind_zSHR) goto case_2452;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2452: ;
  {
    if (zop.kind != Kind_zSAR) goto case_2451;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2451: ;
  {
    if (zop.kind != Kind_zCLZ) goto case_2450;
    {
      z3zE2217 = zexecute_arithmetic(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2450: ;
  {
    if (zop.kind != Kind_zKECCAK256) goto case_2449;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2449: ;
  {
    if (zop.kind != Kind_zADDRESS) goto case_2448;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2448: ;
  {
    if (zop.kind != Kind_zORIGIN) goto case_2447;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2447: ;
  {
    if (zop.kind != Kind_zCALLER) goto case_2446;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2446: ;
  {
    if (zop.kind != Kind_zCALLVALUE) goto case_2445;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2445: ;
  {
    if (zop.kind != Kind_zGASPRICE) goto case_2444;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2444: ;
  {
    if (zop.kind != Kind_zCALLDATASIZE) goto case_2443;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2443: ;
  {
    if (zop.kind != Kind_zCALLDATALOAD) goto case_2442;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2442: ;
  {
    if (zop.kind != Kind_zCALLDATACOPY) goto case_2441;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2441: ;
  {
    if (zop.kind != Kind_zCODESIZE) goto case_2440;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2440: ;
  {
    if (zop.kind != Kind_zCODECOPY) goto case_2439;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2439: ;
  {
    if (zop.kind != Kind_zBALANCE) goto case_2438;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2438: ;
  {
    if (zop.kind != Kind_zSELFBALANCE) goto case_2437;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2437: ;
  {
    if (zop.kind != Kind_zEXTCODESIZE) goto case_2436;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2436: ;
  {
    if (zop.kind != Kind_zEXTCODECOPY) goto case_2435;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2435: ;
  {
    if (zop.kind != Kind_zEXTCODEHASH) goto case_2434;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2434: ;
  {
    if (zop.kind != Kind_zRETURNDATASIZE) goto case_2433;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2433: ;
  {
    if (zop.kind != Kind_zRETURNDATACOPY) goto case_2432;
    {
      z3zE2217 = zexecute_environment(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2432: ;
  {
    if (zop.kind != Kind_zBLOCKHASH) goto case_2431;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2431: ;
  {
    if (zop.kind != Kind_zCOINBASE) goto case_2430;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2430: ;
  {
    if (zop.kind != Kind_zTIMESTAMP) goto case_2429;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2429: ;
  {
    if (zop.kind != Kind_zNUMBER) goto case_2428;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2428: ;
  {
    if (zop.kind != Kind_zSLOTNUM) goto case_2427;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2427: ;
  {
    if (zop.kind != Kind_zPREVRANDAO) goto case_2426;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2426: ;
  {
    if (zop.kind != Kind_zGASLIMIT) goto case_2425;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2425: ;
  {
    if (zop.kind != Kind_zCHAINID) goto case_2424;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2424: ;
  {
    if (zop.kind != Kind_zBASEFEE) goto case_2423;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2423: ;
  {
    if (zop.kind != Kind_zBLOBBASEFEE) goto case_2422;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2422: ;
  {
    if (zop.kind != Kind_zBLOBHASH) goto case_2421;
    {
      z3zE2217 = zexecute_block(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2421: ;
  {
    if (zop.kind != Kind_zPOP) goto case_2420;
    {
      z3zE2217 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2420: ;
  {
    if (zop.kind != Kind_zMLOAD) goto case_2419;
    {
      z3zE2217 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2419: ;
  {
    if (zop.kind != Kind_zMSTORE) goto case_2418;
    {
      z3zE2217 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2418: ;
  {
    if (zop.kind != Kind_zMSTORE8) goto case_2417;
    {
      z3zE2217 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2417: ;
  {
    if (zop.kind != Kind_zMSIZE) goto case_2416;
    {
      z3zE2217 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2416: ;
  {
    if (zop.kind != Kind_zMCOPY) goto case_2415;
    {
      z3zE2217 = zexecute_memory(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2415: ;
  {
    if (zop.kind != Kind_zSLOAD) goto case_2414;
    {
      z3zE2217 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2414: ;
  {
    if (zop.kind != Kind_zSSTORE) goto case_2413;
    {
      z3zE2217 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2413: ;
  {
    if (zop.kind != Kind_zTLOAD) goto case_2412;
    {
      z3zE2217 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2412: ;
  {
    if (zop.kind != Kind_zTSTORE) goto case_2411;
    {
      z3zE2217 = zexecute_storage(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2411: ;
  {
    if (zop.kind != Kind_zJUMP) goto case_2410;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2410: ;
  {
    if (zop.kind != Kind_zJUMPI) goto case_2409;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2409: ;
  {
    if (zop.kind != Kind_zPC) goto case_2408;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2408: ;
  {
    if (zop.kind != Kind_zGAS) goto case_2407;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2407: ;
  {
    if (zop.kind != Kind_zJUMPDEST) goto case_2406;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2406: ;
  {
    if (zop.kind != Kind_zPUSH) goto case_2405;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2405: ;
  {
    if (zop.kind != Kind_zDUP) goto case_2404;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2404: ;
  {
    if (zop.kind != Kind_zSWAP) goto case_2403;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2403: ;
  {
    if (zop.kind != Kind_zDUPN) goto case_2402;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2402: ;
  {
    if (zop.kind != Kind_zSWAPN) goto case_2401;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2401: ;
  {
    if (zop.kind != Kind_zEXCHANGE) goto case_2400;
    {
      z3zE2217 = zexecute_control(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2400: ;
  {
    if (zop.kind != Kind_zLOG) goto case_2399;
    {
      z3zE2217 = zexecute_log(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2399: ;
  {
    if (zop.kind != Kind_zCREATE) goto case_2398;
    z3zE2217 = sail_assert(false, "sail/evm/execute.sail:867.28-867.29");
    goto finish_match_2388;
  }
case_2398: ;
  {
    if (zop.kind != Kind_zCREATE2) goto case_2397;
    z3zE2217 = sail_assert(false, "sail/evm/execute.sail:868.29-868.30");
    goto finish_match_2388;
  }
case_2397: ;
  {
    if (zop.kind != Kind_zCALL) goto case_2396;
    z3zE2217 = sail_assert(false, "sail/evm/execute.sail:869.26-869.27");
    goto finish_match_2388;
  }
case_2396: ;
  {
    if (zop.kind != Kind_zCALLCODE) goto case_2395;
    z3zE2217 = sail_assert(false, "sail/evm/execute.sail:870.30-870.31");
    goto finish_match_2388;
  }
case_2395: ;
  {
    if (zop.kind != Kind_zDELEGATECALL) goto case_2394;
    z3zE2217 = sail_assert(false, "sail/evm/execute.sail:871.34-871.35");
    goto finish_match_2388;
  }
case_2394: ;
  {
    if (zop.kind != Kind_zSTATICCALL) goto case_2393;
    z3zE2217 = sail_assert(false, "sail/evm/execute.sail:872.32-872.33");
    goto finish_match_2388;
  }
case_2393: ;
  {
    if (zop.kind != Kind_zRETURN) goto case_2392;
    {
      z3zE2217 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2392: ;
  {
    if (zop.kind != Kind_zREVERT) goto case_2391;
    {
      z3zE2217 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2391: ;
  {
    if (zop.kind != Kind_zINVALID) goto case_2390;
    {
      z3zE2217 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2390: ;
  {
    /* complete */
    {
      z3zE2217 = zexecute_halt(zop);
      if (have_exception) {  goto end_block_exception_2478;  }
    }
    goto finish_match_2388;
  }
case_2389: ;
finish_match_2388: ;
  z8zE430 = z3zE2217;
end_function_2477: ;
  return z8zE430;
end_block_exception_2478: ;

  return UNIT;
}

bool zcall_is_delegate(enum zCallKind zkind)
{
  bool z8zE431;
  bool z3zE2214;
  {
    if ((zDelegateCall != zkind)) goto case_2385;
    z3zE2214 = true;
    goto finish_match_2383;
  }
case_2385: ;
  {
    z3zE2214 = false;
    goto finish_match_2383;
  }
case_2384: ;
finish_match_2383: ;
  z8zE431 = z3zE2214;
end_function_2386: ;
  return z8zE431;
end_block_exception_2387: ;

  return false;
}

bool zcall_is_static(enum zCallKind zkind)
{
  bool z8zE432;
  bool z3zE2211;
  {
    if ((zStaticCall != zkind)) goto case_2380;
    z3zE2211 = true;
    goto finish_match_2378;
  }
case_2380: ;
  {
    z3zE2211 = false;
    goto finish_match_2378;
  }
case_2379: ;
finish_match_2378: ;
  z8zE432 = z3zE2211;
end_function_2381: ;
  return z8zE432;
end_block_exception_2382: ;

  return false;
}

bool zcall_takes_value(enum zCallKind zkind)
{
  bool z8zE433;
  bool z3zE2207;
  {
    if ((zCall != zkind)) goto case_2375;
    z3zE2207 = true;
    goto finish_match_2372;
  }
case_2375: ;
  {
    if ((zCallCode != zkind)) goto case_2374;
    z3zE2207 = true;
    goto finish_match_2372;
  }
case_2374: ;
  {
    z3zE2207 = false;
    goto finish_match_2372;
  }
case_2373: ;
finish_match_2372: ;
  z8zE433 = z3zE2207;
end_function_2376: ;
  return z8zE433;
end_block_exception_2377: ;

  return false;
}

bool zcall_transfers_value(enum zCallKind zkind)
{
  bool z8zE434;
  bool z3zE2204;
  {
    if ((zCall != zkind)) goto case_2369;
    z3zE2204 = true;
    goto finish_match_2367;
  }
case_2369: ;
  {
    z3zE2204 = false;
    goto finish_match_2367;
  }
case_2368: ;
finish_match_2367: ;
  z8zE434 = z3zE2204;
end_function_2370: ;
  return z8zE434;
end_block_exception_2371: ;

  return false;
}

bool zcall_uses_target_address(enum zCallKind zkind)
{
  bool z8zE435;
  bool z3zE2200;
  {
    if ((zCall != zkind)) goto case_2364;
    z3zE2200 = true;
    goto finish_match_2361;
  }
case_2364: ;
  {
    if ((zStaticCall != zkind)) goto case_2363;
    z3zE2200 = true;
    goto finish_match_2361;
  }
case_2363: ;
  {
    z3zE2200 = false;
    goto finish_match_2361;
  }
case_2362: ;
finish_match_2361: ;
  z8zE435 = z3zE2200;
end_function_2365: ;
  return z8zE435;
end_block_exception_2366: ;

  return false;
}

struct zCode zexecutable_code(sail_fixed_bytes_20 ztarget, bool zdele, sail_fixed_bytes_20 zdtgt)
{
  struct zCode z8zE436;
  if (zdele) {
    struct zCode zdelegate_code;
    {
      sail_fixed_bytes_32 z2zE2226;
      {
        z2zE2226 = zk_code_key(zdtgt);
        if (have_exception) {  goto end_block_exception_2360;  }
      }
      {
        zdelegate_code = zcode_db_resolve(z2zE2226);
        if (have_exception) {  goto end_block_exception_2360;  }
      }
    }
    bool z2zE2225;
    {
      uint64_t z2zE2224;
      z2zE2224 = zprecompile_number(zdtgt);
      z2zE2225 = (z2zE2224 != UINT64_C(0));
    }
    if (z2zE2225) {  z8zE436 = zEMPTY_CODE;  } else {  z8zE436 = zdelegate_code;  }
  } else {
    sail_fixed_bytes_32 z2zE2227;
    {
      z2zE2227 = zk_code_key(ztarget);
      if (have_exception) {  goto end_block_exception_2360;  }
    }
    {
      z8zE436 = zcode_db_resolve(z2zE2227);
      if (have_exception) {  goto end_block_exception_2360;  }
    }
  }
end_function_2359: ;
  return z8zE436;
end_block_exception_2360: ;
  struct zByteSliceFields z8zE982 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zCode z8zE981 = { .zbytes = z8zE982, .zjumpdests = UINT64_C(0xdeadc0de) };
  return z8zE981;
}

unit zrun_call(enum zCallKind zkind)
{
  unit z8zE437;
  sail_fixed_bytes_20 zcaller;
  zcaller = zself_addr(UNIT);
  sail_u256 zgas_request;
  {
    zgas_request = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  sail_u256 ztarget_word;
  {
    ztarget_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  sail_fixed_bytes_20 ztarget;
  ztarget = zword_to_address(ztarget_word);
  sail_u256 zvalue;
  {
    bool z2zE2223;
    z2zE2223 = zcall_takes_value(zkind);
    if (z2zE2223) {
      {
        zvalue = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
    } else {  zvalue = zWORD_ZERO;  }
  }
  sail_u256 zargs_off_word;
  {
    zargs_off_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  sail_u256 zargs_len_word;
  {
    zargs_len_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  sail_u256 zret_off_word;
  {
    zret_off_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  sail_u256 zret_len_word;
  {
    zret_len_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  bool z2zE2156;
  {
    bool z2zE2155;
    z2zE2155 = zis_running(UNIT);
    z2zE2156 = not(z2zE2155);
  }
  unit z3zE2130;
  if (z2zE2156) {
    z8zE437 = UNIT;
    goto cleanup_2356;
    /* unreachable after return */
    goto end_cleanup_2357;
  cleanup_2356: ;
    goto end_function_2339;
  end_cleanup_2357: ;
  } else {  z3zE2130 = UNIT;  }
  bool z2zE2159;
  {
    bool z2zE2158;
    z2zE2158 = zcall_transfers_value(zkind);
    bool z3zE2127;
    if (z2zE2158) {
      bool z2zE2157;
      z2zE2157 = zword_nonzzero(zvalue);
      bool z3zE2126;
      if (z2zE2157) {  z3zE2126 = zmessage.zis_static;  } else {  z3zE2126 = false;  }
      z3zE2127 = z3zE2126;
    } else {  z3zE2127 = false;  }
    z2zE2159 = z3zE2127;
  }
  unit z3zE2128;
  if (z2zE2159) {
    unit z3zE2129;
    {
      z3zE2129 = zexc_halt(zWriteProtection);
      if (have_exception) {  goto end_block_exception_2358;  }
    }
    z8zE437 = UNIT;
    goto cleanup_2354;
    /* unreachable after return */
    goto end_cleanup_2355;
  cleanup_2354: ;
    goto end_function_2339;
  end_cleanup_2355: ;
  } else {  z3zE2128 = UNIT;  }
  bool zwarm;
  zwarm = zk_access_account(ztarget);
  uint64_t ztarget_cost;
  ztarget_cost = zaccount_cost(zwarm);
  uint64_t ztransfer_cost;
  {
    bool z2zE2222;
    z2zE2222 = zword_nonzzero(zvalue);
    if (z2zE2222) {  ztransfer_cost = zcall_value_cost(UNIT);  } else {  ztransfer_cost = zGAS_CONSTANT_ZERO;  }
  }
  uint64_t zavailable;
  zavailable = zgas_remaining;
  struct zMemoryPairExpansion zmemory;
  {
    zmemory = zmemory_pair_expansion(zargs_off_word, zargs_len_word, zret_off_word, zret_len_word, zavailable);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  bool z2zE2161;
  {
    bool z2zE2160;
    z2zE2160 = zis_running(UNIT);
    z2zE2161 = not(z2zE2160);
  }
  unit z3zE2131;
  if (z2zE2161) {
    z8zE437 = UNIT;
    goto cleanup_2352;
    /* unreachable after return */
    goto end_cleanup_2353;
  cleanup_2352: ;
    goto end_function_2339;
  end_cleanup_2353: ;
  } else {  z3zE2131 = UNIT;  }
  uint64_t zmemory_cost;
  zmemory_cost = zmemory.zcost;
  uint64_t zstatic_base;
  {    zstatic_base = (ztarget_cost + ztransfer_cost);
  }
  uint64_t zbefore_static;
  zbefore_static = zgas_remaining;
  bool z2zE2162;
  z2zE2162 = (zbefore_static < zstatic_base);
  unit z3zE2132;
  if (z2zE2162) {
    unit z3zE2133;
    {
      z3zE2133 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2358;  }
    }
    z8zE437 = UNIT;
    goto cleanup_2350;
    /* unreachable after return */
    goto end_cleanup_2351;
  cleanup_2350: ;
    goto end_function_2339;
  end_cleanup_2351: ;
  } else {  z3zE2132 = UNIT;  }
  uint64_t zafter_static_base;
  {    zafter_static_base = (zbefore_static - zstatic_base);
  }
  bool z2zE2163;
  z2zE2163 = (zafter_static_base < zmemory_cost);
  unit z3zE2134;
  if (z2zE2163) {
    unit z3zE2135;
    {
      z3zE2135 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2358;  }
    }
    z8zE437 = UNIT;
    goto cleanup_2348;
    /* unreachable after return */
    goto end_cleanup_2349;
  cleanup_2348: ;
    goto end_function_2339;
  end_cleanup_2349: ;
  } else {  z3zE2134 = UNIT;  }
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2164;
  {
    z2zE2164 = zk_deleg_target(ztarget);
    if (have_exception) {  goto end_block_exception_2358;  }
  }
  unit z3zE2136;
  {
    bool ztg_deleg;
    ztg_deleg = z2zE2164.ztup0;
    sail_fixed_bytes_20 ztg_target;
    ztg_target = z2zE2164.ztup1;
    uint64_t zdelegation_cost;
    if (ztg_deleg) {
      bool zdw;
      zdw = zk_access_account(ztg_target);
      zdelegation_cost = zaccount_cost(zdw);
    } else {  zdelegation_cost = zGAS_CONSTANT_ZERO;  }
    bool znew_account_charged;
    {
      bool z2zE2221;
      z2zE2221 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE2139;
      if (z2zE2221) {
        bool z2zE2220;
        z2zE2220 = zword_nonzzero(zvalue);
        bool z3zE2138;
        if (z2zE2220) {
          bool z2zE2219;
          z2zE2219 = zcall_transfers_value(zkind);
          bool z3zE2137;
          if (z2zE2219) {
            {
              z3zE2137 = zk_account_is_empty(ztarget);
              if (have_exception) {  goto end_block_exception_2358;  }
            }
          } else {  z3zE2137 = false;  }
          z3zE2138 = z3zE2137;
        } else {  z3zE2138 = false;  }
        z3zE2139 = z3zE2138;
      } else {  z3zE2139 = false;  }
      znew_account_charged = z3zE2139;
    }
    uint64_t zcreate_cost;
    {
      bool z2zE2218;
      {
        bool z2zE2217;
        z2zE2217 = zfork_lt(zk_fork, zAmsterdam);
        bool z3zE2142;
        if (z2zE2217) {
          bool z2zE2216;
          z2zE2216 = zword_nonzzero(zvalue);
          bool z3zE2141;
          if (z2zE2216) {
            bool z2zE2215;
            z2zE2215 = zcall_transfers_value(zkind);
            bool z3zE2140;
            if (z2zE2215) {
              {
                z3zE2140 = zk_account_is_empty(ztarget);
                if (have_exception) {  goto end_block_exception_2358;  }
              }
            } else {  z3zE2140 = false;  }
            z3zE2141 = z3zE2140;
          } else {  z3zE2141 = false;  }
          z3zE2142 = z3zE2141;
        } else {  z3zE2142 = false;  }
        z2zE2218 = z3zE2142;
      }
      if (z2zE2218) {  zcreate_cost = zG_newaccount;  } else {  zcreate_cost = zGAS_CONSTANT_ZERO;  }
    }
    uint64_t zadditional_cost;
    {    zadditional_cost = (zdelegation_cost + zcreate_cost);
    }
    uint64_t zbefore_required;
    zbefore_required = zgas_remaining;
    bool z2zE2165;
    z2zE2165 = (zbefore_required < zstatic_base);
    unit z3zE2143;
    if (z2zE2165) {
      unit z3zE2144;
      {
        z3zE2144 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      z8zE437 = UNIT;
      goto cleanup_2346;
      /* unreachable after return */
      goto end_cleanup_2347;
    cleanup_2346: ;
      goto end_function_2339;
    end_cleanup_2347: ;
    } else {  z3zE2143 = UNIT;  }
    uint64_t zafter_required_base;
    {    zafter_required_base = (zbefore_required - zstatic_base);
    }
    bool z2zE2166;
    z2zE2166 = (zafter_required_base < zmemory_cost);
    unit z3zE2145;
    if (z2zE2166) {
      unit z3zE2146;
      {
        z3zE2146 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      z8zE437 = UNIT;
      goto cleanup_2344;
      /* unreachable after return */
      goto end_cleanup_2345;
    cleanup_2344: ;
      goto end_function_2339;
    end_cleanup_2345: ;
    } else {  z3zE2145 = UNIT;  }
    uint64_t zafter_static_cost;
    {    zafter_static_cost = (zafter_required_base - zmemory_cost);
    }
    bool z2zE2167;
    z2zE2167 = (zafter_static_cost < zadditional_cost);
    unit z3zE2147;
    if (z2zE2167) {
      unit z3zE2148;
      {
        z3zE2148 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      z8zE437 = UNIT;
      goto cleanup_2342;
      /* unreachable after return */
      goto end_cleanup_2343;
    cleanup_2342: ;
      goto end_function_2339;
    end_cleanup_2343: ;
    } else {  z3zE2147 = UNIT;  }
    uint64_t zafter_required;
    {    zafter_required = (zafter_static_cost - zadditional_cost);
    }
    uint64_t zrequired;
    {
      bool z2zE2213;
      z2zE2213 = (!(zbefore_required < zafter_required));
      if (z2zE2213) {
        {    zrequired = (zbefore_required - zafter_required);
        }
      } else {
        struct zexception z2zE2214;
        CREATE(zexception)(&z2zE2214);
        zInvalidBlock(&z2zE2214, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE2214);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/interpreter.sail:472.12-472.48");
        KILL(zexception)(&z2zE2214);
        goto end_block_exception_2358;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2214);
      }
    }
    uint64_t zstipend;
    {
      bool z2zE2212;
      z2zE2212 = zword_nonzzero(zvalue);
      if (z2zE2212) {  zstipend = zG_callstipend;  } else {  zstipend = zGAS_ZERO;  }
    }
    uint64_t zbase_child;
    zbase_child = zGAS_ZERO;
    bool z2zE2168;
    z2zE2168 = zfork_gteq(zk_fork, zAmsterdam);
    unit z3zE2155;
    if (z2zE2168) {
      unit z3zE2159;
      {
        z3zE2159 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      unit z3zE2158;
      if (znew_account_charged) {
        {
          z3zE2158 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
      } else {  z3zE2158 = UNIT;  }
      bool z2zE2169;
      z2zE2169 = zis_running(UNIT);
      if (z2zE2169) {
        {
          zbase_child = zcall_gas_cap_word(zgas_remaining, zgas_request);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
        unit z3zE2160;
        z3zE2160 = UNIT;
        {
          z3zE2155 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase_child);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
      } else {  z3zE2155 = UNIT;  }
    } else {
      uint64_t zavail;
      zavail = zgas_remaining;
      bool z2zE2170;
      z2zE2170 = (zavail < zrequired);
      if (z2zE2170) {  zbase_child = zGAS_ZERO;  } else {
        uint64_t zavailable_after_cost;
        {
          zavailable_after_cost = zgas_sub_or_oog(zavail, zrequired);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
        {
          zbase_child = zcall_gas_cap_word(zavailable_after_cost, zgas_request);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
      }
      unit z3zE2157;
      z3zE2157 = UNIT;
      unit z3zE2156;
      {
        z3zE2156 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      bool z2zE2171;
      z2zE2171 = zis_running(UNIT);
      if (z2zE2171) {
        {
          z3zE2155 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase_child);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
      } else {  z3zE2155 = UNIT;  }
    }
    bool z2zE2173;
    {
      bool z2zE2172;
      z2zE2172 = zis_running(UNIT);
      z2zE2173 = not(z2zE2172);
    }
    unit z3zE2154;
    if (z2zE2173) {
      z8zE437 = UNIT;
      goto cleanup_2340;
      /* unreachable after return */
      goto end_cleanup_2341;
    cleanup_2340: ;
      goto end_function_2339;
    end_cleanup_2341: ;
    } else {  z3zE2154 = UNIT;  }
    unit z3zE2149;
    if (ztg_deleg) {
      struct zCode z2zE2175;
      {
        sail_fixed_bytes_32 z2zE2174;
        {
          z2zE2174 = zk_code_key(ztg_target);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
        {
          z2zE2175 = zcode_db_resolve(z2zE2174);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
      }
      unit z3zE2150;
      {
        struct zAccount z2zE2176;
        {
          z2zE2176 = zk_aload(ztg_target);
          if (have_exception) {  goto end_block_exception_2358;  }
        }
        unit z3zE2151;
        {
          z3zE2151 = UNIT;
          goto finish_match_2330;
        }
      case_2331: ;
        sail_match_failure("run_call");
      finish_match_2330: ;
        z3zE2150 = z3zE2151;
        goto finish_match_2328;
      }
    case_2329: ;
      sail_match_failure("run_call");
    finish_match_2328: ;
      z3zE2149 = z3zE2150;
    } else {  z3zE2149 = UNIT;  }
    struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z2zE2177;
    z2zE2177 = zapply_memory_pair_expansion(zmemory);
    unit z3zE2161;
    {
      struct zMemoryRangeFields zargs;
      zargs = z2zE2177.ztup0;
      struct zMemoryRangeFields zret;
      zret = z2zE2177.ztup1;
      uint64_t zchild_gas;
      {
        zchild_gas = zconserved_gas_add(zbase_child, zstipend);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      struct zAccount z2zE2178;
      {
        z2zE2178 = zk_aload(ztarget);
        if (have_exception) {  goto end_block_exception_2358;  }
      }
      unit z3zE2162;
      {
        bool z2zE2184;
        {
          bool z2zE2183;
          z2zE2183 = (!(zcall_depth < zDEPTH_LIMIT));
          bool z3zE2165;
          if (z2zE2183) {  z3zE2165 = true;  } else {
            bool z2zE2182;
            z2zE2182 = zcall_takes_value(zkind);
            bool z3zE2164;
            if (z2zE2182) {
              bool z2zE2181;
              z2zE2181 = zword_nonzzero(zvalue);
              bool z3zE2163;
              if (z2zE2181) {
                bool z2zE2180;
                {
                  sail_u256 z2zE2179;
                  {
                    z2zE2179 = zk_get_balance(zcaller);
                    if (have_exception) {  goto end_block_exception_2358;  }
                  }
                  z2zE2180 = zword_ule(zvalue, z2zE2179);
                }
                z3zE2163 = not(z2zE2180);
              } else {  z3zE2163 = false;  }
              z3zE2164 = z3zE2163;
            } else {  z3zE2164 = false;  }
            z3zE2165 = z3zE2164;
          }
          z2zE2184 = z3zE2165;
        }
        if (z2zE2184) {
          unit z3zE2197;
          z3zE2197 = zreturndata_clear(UNIT);
          unit z3zE2196;
          {
            z3zE2196 = zrefund_gas(zchild_gas);
            if (have_exception) {  goto end_block_exception_2358;  }
          }
          unit z3zE2195;
          if (znew_account_charged) {
            {
              z3zE2195 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2358;  }
            }
          } else {  z3zE2195 = UNIT;  }
          {
            z3zE2162 = zpush_word(zWORD_ZERO);
            if (have_exception) {  goto end_block_exception_2358;  }
          }
        } else {
          uint64_t zselected_precompile;
          zselected_precompile = zprecompile_number(ztarget);
          bool z2zE2185;
          z2zE2185 = (zselected_precompile != UINT64_C(0));
          if (z2zE2185) {
            uint64_t znumber;
            znumber = zselected_precompile;
            struct zByteSliceFields zinput;
            {
              uint64_t z2zE2193;
              z2zE2193 = zargs.zoff;
              uint64_t z2zE2194;
              z2zE2194 = zargs.zlen;
              zinput = zmemory_byte_slice(z2zE2193, z2zE2194);
            }
            struct zoptionzIU64zK z2zE2186;
            CREATE(zoptionzIU64zK)(&z2zE2186);
            zprecompile_gas(&z2zE2186, znumber, zinput, zchild_gas);
            unit z3zE2183;
            {
              if (z2zE2186.kind != Kind_zSomezIU64zK) goto case_2338;
              uint64_t zused;
              zused = z2zE2186.variants.zSomezIU64zK;
              struct zPrecompileResult zresult;
              zresult = zrun_precompile_slice(znumber, zinput);
              bool z2zE2187;
              z2zE2187 = zresult.zsuccess;
              if (z2zE2187) {
                zreturndata = zresult.zoutput;
                unit z3zE2193;
                z3zE2193 = UNIT;
                bool z2zE2189;
                {
                  bool z2zE2188;
                  z2zE2188 = zcall_transfers_value(zkind);
                  bool z3zE2190;
                  if (z2zE2188) {  z3zE2190 = zword_nonzzero(zvalue);  } else {  z3zE2190 = false;  }
                  z2zE2189 = z3zE2190;
                }
                unit z3zE2192;
                if (z2zE2189) {
                  {
                    z3zE2192 = zk_transfer(zcaller, ztarget, zvalue);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2186);
                      goto end_block_exception_2358;
                    }
                  }
                } else {  z3zE2192 = UNIT;  }
                uint64_t z2zE2190;
                z2zE2190 = zret.zoff;
                uint64_t z2zE2191;
                z2zE2191 = zret.zlen;
                unit z3zE2191;
                z3zE2191 = zreturndata_copy_prefix(z2zE2190, z2zE2191);
                uint64_t zunused;
                {
                  zunused = zgas_sub_or_oog(zchild_gas, zused);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE2186);
                    goto end_block_exception_2358;
                  }
                }
                bool z2zE2192;
                z2zE2192 = zis_running(UNIT);
                if (z2zE2192) {
                  unit z3zE2194;
                  {
                    z3zE2194 = zrefund_gas(zunused);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2186);
                      goto end_block_exception_2358;
                    }
                  }
                  {
                    z3zE2183 = zpush_word(zWORD_ONE);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2186);
                      goto end_block_exception_2358;
                    }
                  }
                } else {  z3zE2183 = UNIT;  }
              } else {
                unit z3zE2189;
                z3zE2189 = zreturndata_clear(UNIT);
                unit z3zE2188;
                if (znew_account_charged) {
                  {
                    z3zE2188 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE2186);
                      goto end_block_exception_2358;
                    }
                  }
                } else {  z3zE2188 = UNIT;  }
                {
                  z3zE2183 = zpush_word(zWORD_ZERO);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE2186);
                    goto end_block_exception_2358;
                  }
                }
              }
              goto finish_match_2336;
            }
          case_2338: ;
            {
              unit z3zE2185;
              z3zE2185 = zreturndata_clear(UNIT);
              unit z3zE2184;
              if (znew_account_charged) {
                {
                  z3zE2184 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE2186);
                    goto end_block_exception_2358;
                  }
                }
              } else {  z3zE2184 = UNIT;  }
              {
                z3zE2183 = zpush_word(zWORD_ZERO);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE2186);
                  goto end_block_exception_2358;
                }
              }
              goto finish_match_2336;
            }
          case_2337: ;
          finish_match_2336: ;
            z3zE2162 = z3zE2183;
            KILL(zoptionzIU64zK)(&z2zE2186);
          } else {
            struct zCode zchild_code;
            {
              zchild_code = zexecutable_code(ztarget, ztg_deleg, ztg_target);
              if (have_exception) {  goto end_block_exception_2358;  }
            }
            sail_fixed_bytes_20 zchild_addr;
            {
              bool z2zE2211;
              z2zE2211 = zcall_uses_target_address(zkind);
              if (z2zE2211) {  zchild_addr = ztarget;  } else {  zchild_addr = zcaller;  }
            }
            sail_fixed_bytes_20 zchild_caller;
            {
              bool z2zE2210;
              z2zE2210 = zcall_is_delegate(zkind);
              if (z2zE2210) {  zchild_caller = zmessage.zcaller;  } else {  zchild_caller = zcaller;  }
            }
            sail_u256 zchild_value;
            {
              bool z2zE2209;
              z2zE2209 = zcall_is_delegate(zkind);
              if (z2zE2209) {  zchild_value = zmessage.zvalue;  } else {  zchild_value = zvalue;  }
            }
            bool zchild_static;
            {
              bool z2zE2208;
              z2zE2208 = zcall_is_static(zkind);
              if (z2zE2208) {  zchild_static = true;  } else {  zchild_static = zmessage.zis_static;  }
            }
            struct zByteSliceFields zchild_calldata;
            {
              bool z2zE2205;
              z2zE2205 = zis_running(UNIT);
              if (z2zE2205) {
                uint64_t z2zE2206;
                z2zE2206 = zargs.zoff;
                uint64_t z2zE2207;
                z2zE2207 = zargs.zlen;
                zchild_calldata = zmemory_byte_slice(z2zE2206, z2zE2207);
              } else {  zchild_calldata = zEMPTY_SLICE;  }
            }
            struct zFrameCheckpoint zcheckpoint;
            CREATE(zFrameCheckpoint)(&zcheckpoint);
            {
              struct zFrameCheckpoint z2zE2204;
              CREATE(zFrameCheckpoint)(&z2zE2204);
              zsuspend_frame(&z2zE2204, UNIT);
              struct zFrameCheckpoint z3zE2167;
              CREATE(zFrameCheckpoint)(&z3zE2167);
              COPY(zFrameCheckpoint)(&z3zE2167, z2zE2204);
              z3zE2167.zstate_gas_remaining = zGAS_ZERO;
              COPY(zFrameCheckpoint)(&zcheckpoint, z3zE2167);
            }
            struct zFrameContinuation z2zE2198;
            CREATE(zFrameContinuation)(&z2zE2198);
            {
              struct zCallContinuation z2zE2197;
              CREATE(zCallContinuation)(&z2zE2197);
              {
                uint64_t z2zE2195;
                z2zE2195 = zret.zoff;
                uint64_t z2zE2196;
                z2zE2196 = zret.zlen;
                COPY(zFrameCheckpoint)(&((&z2zE2197)->zcheckpoint), zcheckpoint);
                z2zE2197.znew_account_charged = znew_account_charged;
                z2zE2197.zreturn_length = z2zE2196;
                z2zE2197.zreturn_offset = z2zE2195;
              }
              zResumeCall(&z2zE2198, z2zE2197);
            }
            unit z3zE2182;
            z3zE2182 = zframe_stack_push(z2zE2198);
            bool z2zE2200;
            {
              bool z2zE2199;
              z2zE2199 = zcall_transfers_value(zkind);
              bool z3zE2169;
              if (z2zE2199) {  z3zE2169 = zword_nonzzero(zvalue);  } else {  z3zE2169 = false;  }
              z2zE2200 = z3zE2169;
            }
            unit z3zE2181;
            if (z2zE2200) {
              {
                z3zE2181 = zk_transfer(zcaller, ztarget, zvalue);
                if (have_exception) {  goto end_block_exception_2358;  }
              }
            } else {  z3zE2181 = UNIT;  }
            uint64_t z2zE2202;
            {
              uint64_t z2zE2201;
              z2zE2201 = zcheckpoint.zcall_depth;
              z2zE2202 = zframe_depth_increment(z2zE2201);
            }
            struct zMessage z3zE2170;
            z3zE2170.zaddress = zchild_addr;
            z3zE2170.zcaller = zchild_caller;
            z3zE2170.zcode_address = ztarget;
            z3zE2170.zdepth = z2zE2202;
            z3zE2170.zis_static = zchild_static;
            z3zE2170.zstate_gas_reservoir = zstate_gas_remaining;
            z3zE2170.zvalue = zchild_value;
            zmessage = z3zE2170;
            unit z3zE2180;
            z3zE2180 = UNIT;
            unit z3zE2179;
            z3zE2179 = zcalldata_install(zchild_calldata);
            zpc = UINT64_C(0);
            unit z3zE2178;
            z3zE2178 = UNIT;
            zgas_remaining = zchild_gas;
            unit z3zE2177;
            z3zE2177 = UNIT;
            unit z3zE2176;
            z3zE2176 = UNIT;
            zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
            unit z3zE2175;
            z3zE2175 = UNIT;
            zRunning(&zframe_status, UNIT);
            unit z3zE2174;
            z3zE2174 = UNIT;
            unit z3zE2173;
            z3zE2173 = zreturndata_clear(UNIT);
            zframe_code = zchild_code;
            unit z3zE2172;
            z3zE2172 = UNIT;
            uint64_t z2zE2203;
            z2zE2203 = zcheckpoint.zcall_depth;
            zcall_depth = zframe_depth_increment(z2zE2203);
            unit z3zE2171;
            z3zE2171 = UNIT;
            zframe_refund = zGAS_REFUND_ZERO;
            z3zE2162 = UNIT;
          }
        }
        goto finish_match_2334;
      }
    case_2335: ;
      sail_match_failure("run_call");
    finish_match_2334: ;
      z3zE2161 = z3zE2162;
      goto finish_match_2332;
    }
  case_2333: ;
    sail_match_failure("run_call");
  finish_match_2332: ;
    z3zE2136 = z3zE2161;
    goto finish_match_2326;
  }
case_2327: ;
  sail_match_failure("run_call");
finish_match_2326: ;
  z8zE437 = z3zE2136;
end_function_2339: ;
  return z8zE437;
end_block_exception_2358: ;

  return UNIT;
}

unit zrun_create(bool zis2)
{
  unit z8zE438;
  sail_fixed_bytes_20 zcreator;
  zcreator = zself_addr(UNIT);
  sail_u256 zvalue;
  {
    zvalue = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2325;  }
  }
  sail_u256 zoff_word;
  {
    zoff_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2325;  }
  }
  sail_u256 zlen_word;
  {
    zlen_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2325;  }
  }
  sail_u256 zsalt;
  if (zis2) {
    {
      zsalt = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2325;  }
    }
  } else {  zsalt = zWORD_ZERO;  }
  bool z2zE2111;
  {
    bool z2zE2110;
    z2zE2110 = zis_running(UNIT);
    z2zE2111 = not(z2zE2110);
  }
  unit z3zE2079;
  if (z2zE2111) {
    z8zE438 = UNIT;
    goto cleanup_2323;
    /* unreachable after return */
    goto end_cleanup_2324;
  cleanup_2323: ;
    goto end_function_2308;
  end_cleanup_2324: ;
  } else {  z3zE2079 = UNIT;  }
  bool z2zE2112;
  {
    z2zE2112 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2325;  }
  }
  unit z3zE2078;
  if (z2zE2112) {
    z8zE438 = UNIT;
    goto cleanup_2321;
    /* unreachable after return */
    goto end_cleanup_2322;
  cleanup_2321: ;
    goto end_function_2308;
  end_cleanup_2322: ;
  } else {  z3zE2078 = UNIT;  }
  struct zMemoryRangeFields zinitcode;
  {
    zinitcode = zcharge_memory_range(zoff_word, zlen_word);
    if (have_exception) {  goto end_block_exception_2325;  }
  }
  bool z2zE2114;
  {
    bool z2zE2113;
    z2zE2113 = zis_running(UNIT);
    z2zE2114 = not(z2zE2113);
  }
  unit z3zE2084;
  if (z2zE2114) {
    z8zE438 = UNIT;
    goto cleanup_2319;
    /* unreachable after return */
    goto end_cleanup_2320;
  cleanup_2319: ;
    goto end_function_2308;
  end_cleanup_2320: ;
  } else {  z3zE2084 = UNIT;  }
  uint64_t z2zE2115;
  z2zE2115 = zcreate_access_cost(UNIT);
  unit z3zE2083;
  {
    z3zE2083 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2115);
    if (have_exception) {  goto end_block_exception_2325;  }
  }
  bool z2zE2116;
  z2zE2116 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE2082;
  if (z2zE2116) {
    sail_u256 z2zE2117;
    z2zE2117 = zmemory_word_count_word(zlen_word);
    {
      z3zE2082 = zcharge_word_scaled_gas(zG_initcode_word, z2zE2117);
      if (have_exception) {  goto end_block_exception_2325;  }
    }
  } else {  z3zE2082 = UNIT;  }
  unit z3zE2081;
  if (zis2) {
    sail_u256 z2zE2118;
    z2zE2118 = zmemory_word_count_word(zlen_word);
    {
      z3zE2081 = zcharge_word_scaled_gas(zG_keccak_word, z2zE2118);
      if (have_exception) {  goto end_block_exception_2325;  }
    }
  } else {  z3zE2081 = UNIT;  }
  bool z2zE2120;
  {
    bool z2zE2119;
    z2zE2119 = zis_running(UNIT);
    z2zE2120 = not(z2zE2119);
  }
  unit z3zE2080;
  if (z2zE2120) {
    z8zE438 = UNIT;
    goto cleanup_2317;
    /* unreachable after return */
    goto end_cleanup_2318;
  cleanup_2317: ;
    goto end_function_2308;
  end_cleanup_2318: ;
  } else {  z3zE2080 = UNIT;  }
  bool z2zE2123;
  {
    bool z2zE2122;
    {
      uint64_t z2zE2121;
      z2zE2121 = zinitcode.zlen;
      z2zE2122 = zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2121);
    }
    z2zE2123 = not(z2zE2122);
  }
  if (z2zE2123) {
    {
      z8zE438 = zexc_halt(zInitCodeTooLarge);
      if (have_exception) {  goto end_block_exception_2325;  }
    }
  } else {
    uint64_t znonce;
    {
      znonce = zk_get_nonce(zcreator);
      if (have_exception) {  goto end_block_exception_2325;  }
    }
    sail_fixed_bytes_20 znew_addr;
    {
      bool z2zE2152;
      {
        bool z3zE2085;
        if (zis2) {  z3zE2085 = zis_running(UNIT);  } else {  z3zE2085 = false;  }
        z2zE2152 = z3zE2085;
      }
      if (z2zE2152) {
        sail_fixed_bytes_32 z2zE2154;
        {
          sail_u256 z2zE2153;
          z2zE2153 = zmem_keccak(zinitcode);
          z2zE2154 = zword_to_hash(z2zE2153);
        }
        znew_addr = zk_create2_addr(zcreator, zsalt, z2zE2154);
      } else {  znew_addr = zk_create_addr(zcreator, znonce);  }
    }
    uint64_t zchild_gas;
    zchild_gas = zGAS_ZERO;
    bool z2zE2124;
    z2zE2124 = zfork_lt(zk_fork, zAmsterdam);
    unit z3zE2089;
    if (z2zE2124) {
      uint64_t zavail;
      zavail = zgas_remaining;
      uint64_t zretained_gas;
      zretained_gas = (zavail / UINT64_C(64));
      {
        zchild_gas = zgas_sub_or_oog(zavail, zretained_gas);
        if (have_exception) {  goto end_block_exception_2325;  }
      }
      unit z3zE2090;
      z3zE2090 = UNIT;
      zgas_remaining = zretained_gas;
      z3zE2089 = UNIT;
    } else {  z3zE2089 = UNIT;  }
    bool z2zE2126;
    {
      bool z2zE2125;
      z2zE2125 = zis_running(UNIT);
      z2zE2126 = not(z2zE2125);
    }
    unit z3zE2088;
    if (z2zE2126) {
      z8zE438 = UNIT;
      goto cleanup_2315;
      /* unreachable after return */
      goto end_cleanup_2316;
    cleanup_2315: ;
      goto end_function_2308;
    end_cleanup_2316: ;
    } else {  z3zE2088 = UNIT;  }
    bool z2zE2127;
    z2zE2127 = zmessage.zis_static;
    unit z3zE2086;
    if (z2zE2127) {
      unit z3zE2087;
      {
        z3zE2087 = zexc_halt(zWriteProtection);
        if (have_exception) {  goto end_block_exception_2325;  }
      }
      z8zE438 = UNIT;
      goto cleanup_2313;
      /* unreachable after return */
      goto end_cleanup_2314;
    cleanup_2313: ;
      goto end_function_2308;
    end_cleanup_2314: ;
    } else {  z3zE2086 = UNIT;  }
    bool z2zE2132;
    {
      bool z2zE2131;
      z2zE2131 = (!(zcall_depth < zDEPTH_LIMIT));
      bool z3zE2092;
      if (z2zE2131) {  z3zE2092 = true;  } else {
        bool z2zE2130;
        {
          bool z2zE2129;
          {
            sail_u256 z2zE2128;
            {
              z2zE2128 = zk_get_balance(zcreator);
              if (have_exception) {  goto end_block_exception_2325;  }
            }
            z2zE2129 = zword_ule(zvalue, z2zE2128);
          }
          z2zE2130 = not(z2zE2129);
        }
        bool z3zE2091;
        if (z2zE2130) {  z3zE2091 = true;  } else {  z3zE2091 = (znonce == UINT64_C(18446744073709551615));  }
        z3zE2092 = z3zE2091;
      }
      z2zE2132 = z3zE2092;
    }
    if (z2zE2132) {
      unit z3zE2123;
      z3zE2123 = zreturndata_clear(UNIT);
      bool z2zE2133;
      z2zE2133 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE2122;
      if (z2zE2133) {
        {
          z3zE2122 = zrefund_gas(zchild_gas);
          if (have_exception) {  goto end_block_exception_2325;  }
        }
      } else {  z3zE2122 = UNIT;  }
      {
        z8zE438 = zpush_word(zWORD_ZERO);
        if (have_exception) {  goto end_block_exception_2325;  }
      }
    } else {
      bool z2zE2134;
      z2zE2134 = zk_access_account(znew_addr);
      unit z3zE2093;
      {
        bool znew_account_charged;
        {
          bool z2zE2151;
          z2zE2151 = zfork_gteq(zk_fork, zAmsterdam);
          bool z3zE2094;
          if (z2zE2151) {
            {
              z3zE2094 = zk_account_is_empty(znew_addr);
              if (have_exception) {  goto end_block_exception_2325;  }
            }
          } else {  z3zE2094 = false;  }
          znew_account_charged = z3zE2094;
        }
        unit z3zE2099;
        if (znew_account_charged) {
          {
            z3zE2099 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
        } else {  z3zE2099 = UNIT;  }
        bool z2zE2136;
        {
          bool z2zE2135;
          z2zE2135 = zis_running(UNIT);
          z2zE2136 = not(z2zE2135);
        }
        unit z3zE2098;
        if (z2zE2136) {
          z8zE438 = UNIT;
          goto cleanup_2311;
          /* unreachable after return */
          goto end_cleanup_2312;
        cleanup_2311: ;
          goto end_function_2308;
        end_cleanup_2312: ;
        } else {  z3zE2098 = UNIT;  }
        bool z2zE2137;
        z2zE2137 = zfork_gteq(zk_fork, zAmsterdam);
        unit z3zE2096;
        if (z2zE2137) {
          uint64_t z3zE2124;
          z3zE2124 = zgas_remaining;
          uint64_t z3zE2125;
          z3zE2125 = (z3zE2124 / UINT64_C(64));
          {
            zchild_gas = zgas_sub_or_oog(z3zE2124, z3zE2125);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
          unit z3zE2097;
          z3zE2097 = UNIT;
          zgas_remaining = z3zE2125;
          z3zE2096 = UNIT;
        } else {  z3zE2096 = UNIT;  }
        bool z2zE2139;
        {
          bool z2zE2138;
          z2zE2138 = zis_running(UNIT);
          z2zE2139 = not(z2zE2138);
        }
        unit z3zE2095;
        if (z2zE2139) {
          z8zE438 = UNIT;
          goto cleanup_2309;
          /* unreachable after return */
          goto end_cleanup_2310;
        cleanup_2309: ;
          goto end_function_2308;
        end_cleanup_2310: ;
        } else {  z3zE2095 = UNIT;  }
        bool zoccupied;
        {
          zoccupied = zk_account_occupied(znew_addr);
          if (have_exception) {  goto end_block_exception_2325;  }
        }
        unit z3zE2101;
        z3zE2101 = zreturndata_clear(UNIT);
        unit z3zE2100;
        {
          z3zE2100 = zk_bump_nonce(zcreator);
          if (have_exception) {  goto end_block_exception_2325;  }
        }
        if (zoccupied) {
          unit z3zE2121;
          if (znew_account_charged) {
            {
              z3zE2121 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2325;  }
            }
          } else {  z3zE2121 = UNIT;  }
          {
            z3zE2093 = zpush_word(zWORD_ZERO);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
        } else {
          struct zByteSliceFields z1zE2;
          {
            bool z2zE2148;
            z2zE2148 = zis_running(UNIT);
            if (z2zE2148) {
              uint64_t z2zE2149;
              z2zE2149 = zinitcode.zoff;
              uint64_t z2zE2150;
              z2zE2150 = zinitcode.zlen;
              z1zE2 = zmemory_code_slice(z2zE2149, z2zE2150);
            } else {  z1zE2 = zEMPTY_CODE_SLICE;  }
          }
          struct zCode zchild_code;
          {
            sail_fixed_bytes_32 z2zE2147;
            z2zE2147 = zcode_db_insert(z1zE2, zk_fork);
            {
              zchild_code = zcode_db_resolve(z2zE2147);
              if (have_exception) {  goto end_block_exception_2325;  }
            }
          }
          struct zFrameCheckpoint zcheckpoint;
          CREATE(zFrameCheckpoint)(&zcheckpoint);
          {
            struct zFrameCheckpoint z2zE2146;
            CREATE(zFrameCheckpoint)(&z2zE2146);
            zsuspend_frame(&z2zE2146, UNIT);
            struct zFrameCheckpoint z3zE2103;
            CREATE(zFrameCheckpoint)(&z3zE2103);
            COPY(zFrameCheckpoint)(&z3zE2103, z2zE2146);
            z3zE2103.zstate_gas_remaining = zGAS_ZERO;
            COPY(zFrameCheckpoint)(&zcheckpoint, z3zE2103);
          }
          struct zFrameContinuation z2zE2140;
          CREATE(zFrameContinuation)(&z2zE2140);
          {
            struct zCreateContinuation z3zE2104;
            CREATE(zCreateContinuation)(&z3zE2104);
            z3zE2104.zaddress = znew_addr;
            COPY(zFrameCheckpoint)(&((&z3zE2104)->zcheckpoint), zcheckpoint);
            z3zE2104.znew_account_charged = znew_account_charged;
            zResumeCreate(&z2zE2140, z3zE2104);
          }
          unit z3zE2120;
          z3zE2120 = zframe_stack_push(z2zE2140);
          unit z3zE2119;
          {
            z3zE2119 = zk_mark_created(znew_addr);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
          unit z3zE2118;
          {
            z3zE2118 = zk_clear_storage(znew_addr);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
          unit z3zE2117;
          {
            z3zE2117 = zk_bump_nonce(znew_addr);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
          unit z3zE2116;
          {
            z3zE2116 = zk_transfer(zcreator, znew_addr, zvalue);
            if (have_exception) {  goto end_block_exception_2325;  }
          }
          bool z2zE2142;
          {
            struct zMessage z2zE2141;
            z2zE2141 = zcheckpoint.zmessage;
            z2zE2142 = z2zE2141.zis_static;
          }
          uint64_t z2zE2144;
          {
            uint64_t z2zE2143;
            z2zE2143 = zcheckpoint.zcall_depth;
            z2zE2144 = zframe_depth_increment(z2zE2143);
          }
          struct zMessage z3zE2105;
          z3zE2105.zaddress = znew_addr;
          z3zE2105.zcaller = zcreator;
          z3zE2105.zcode_address = znew_addr;
          z3zE2105.zdepth = z2zE2144;
          z3zE2105.zis_static = z2zE2142;
          z3zE2105.zstate_gas_reservoir = zstate_gas_remaining;
          z3zE2105.zvalue = zvalue;
          zmessage = z3zE2105;
          unit z3zE2115;
          z3zE2115 = UNIT;
          unit z3zE2114;
          z3zE2114 = zcalldata_install(zEMPTY_SLICE);
          zpc = UINT64_C(0);
          unit z3zE2113;
          z3zE2113 = UNIT;
          zgas_remaining = zchild_gas;
          unit z3zE2112;
          z3zE2112 = UNIT;
          unit z3zE2111;
          z3zE2111 = UNIT;
          zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
          unit z3zE2110;
          z3zE2110 = UNIT;
          zRunning(&zframe_status, UNIT);
          unit z3zE2109;
          z3zE2109 = UNIT;
          unit z3zE2108;
          z3zE2108 = zreturndata_clear(UNIT);
          zframe_code = zchild_code;
          unit z3zE2107;
          z3zE2107 = UNIT;
          uint64_t z2zE2145;
          z2zE2145 = zcheckpoint.zcall_depth;
          zcall_depth = zframe_depth_increment(z2zE2145);
          unit z3zE2106;
          z3zE2106 = UNIT;
          zframe_refund = zGAS_REFUND_ZERO;
          z3zE2093 = UNIT;
        }
        goto finish_match_2306;
      }
    case_2307: ;
      sail_match_failure("run_create");
    finish_match_2306: ;
      z8zE438 = z3zE2093;
    }
  }
end_function_2308: ;
  return z8zE438;
end_block_exception_2325: ;

  return UNIT;
}

unit zexecute(struct zast zop)
{
  unit z8zE439;
  unit z3zE2070;
  {
    if (zop.kind != Kind_zCREATE) goto case_2303;
    {
      z3zE2070 = zrun_create(false);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2303: ;
  {
    if (zop.kind != Kind_zCREATE2) goto case_2302;
    {
      z3zE2070 = zrun_create(true);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2302: ;
  {
    if (zop.kind != Kind_zCALL) goto case_2301;
    {
      z3zE2070 = zrun_call(zCall);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2301: ;
  {
    if (zop.kind != Kind_zCALLCODE) goto case_2300;
    {
      z3zE2070 = zrun_call(zCallCode);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2300: ;
  {
    if (zop.kind != Kind_zDELEGATECALL) goto case_2299;
    {
      z3zE2070 = zrun_call(zDelegateCall);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2299: ;
  {
    if (zop.kind != Kind_zSTATICCALL) goto case_2298;
    {
      z3zE2070 = zrun_call(zStaticCall);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2298: ;
  {
    {
      z3zE2070 = zexecute_opcode(zop);
      if (have_exception) {  goto end_block_exception_2305;  }
    }
    goto finish_match_2296;
  }
case_2297: ;
finish_match_2296: ;
  z8zE439 = z3zE2070;
end_function_2304: ;
  return z8zE439;
end_block_exception_2305: ;

  return UNIT;
}

