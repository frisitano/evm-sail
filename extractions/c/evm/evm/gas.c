/* Generated from sail/evm/gas.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_120(void) {

  sail_u256 z3zE139;
  z3zE139 = zWORD_ONE;
  zMIN_BLOB_BASE_FEE = z3zE139;
let_end_431: ;
}
void kill_letbind_120(void) {
}

void create_letbind_121(void) {

  uint64_t z3zE140;
  z3zE140 = UINT64_C(131072);
  zGAS_PER_BLOB = z3zE140;
let_end_432: ;
}
void kill_letbind_121(void) {
}

void create_letbind_122(void) {

  sail_u256 z3zE141;
  z3zE141 = ((sail_u256){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(1152921504606846976)}});
  zBLOB_RESERVE_PRICE_SHIFT_LIMIT = z3zE141;
let_end_433: ;
}
void kill_letbind_122(void) {
}

uint64_t zfake_exponential_index_increment(uint64_t zvalue)
{
  uint64_t z8zE298;
  bool z2zE2373;
  z2zE2373 = (zvalue < UINT64_C(18446744073709551615));
  if (z2zE2373) {
    {    z8zE298 = (zvalue + UINT64_C(1));
    }
  } else {
    struct zexception z2zE2374;
    CREATE(zexception)(&z2zE2374);
    zInvalidBlock(&z2zE2374, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2374);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:67.8-67.44");
    KILL(zexception)(&z2zE2374);
    goto end_block_exception_2643;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2374);
  }
end_function_2642: ;
  return z8zE298;
end_block_exception_2643: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zblob_word_add(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE299;
  bool z2zE2371;
  {
    sail_u256 z2zE2370;
    z2zE2370 = zword_sub_word(zWORD_ALL_ONES, zleft);
    z2zE2371 = zword_ule(zright, z2zE2370);
  }
  if (z2zE2371) {  z8zE299 = zword_add_word(zleft, zright);  } else {
    struct zexception z2zE2372;
    CREATE(zexception)(&z2zE2372);
    zInvalidBlock(&z2zE2372, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2372);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:79.8-79.44");
    KILL(zexception)(&z2zE2372);
    goto end_block_exception_2641;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2372);
  }
end_function_2640: ;
  return z8zE299;
end_block_exception_2641: ;

  return u256_zero();
}

struct zScaledBlobValue zscaled_blob_add(struct zScaledBlobValue zleft, struct zScaledBlobValue zright, uint64_t zdenominator)
{
  struct zScaledBlobValue z8zE300;
  uint64_t zcombined;
  {
    uint64_t z2zE2364;
    z2zE2364 = zleft.zremainder;
    uint64_t z2zE2365;
    z2zE2365 = zright.zremainder;
    {    zcombined = (z2zE2364 + z2zE2365);
    }
  }
  bool zcarry;
  zcarry = (!(zcombined < zdenominator));
  uint64_t zremainder_value;
  if (zcarry) {
    {    zremainder_value = (zcombined - zdenominator);
    }
  } else {  zremainder_value = zcombined;  }
  bool z2zE2360;
  z2zE2360 = (zremainder_value < zdenominator);
  if (z2zE2360) {
    uint64_t zremainder;
    zremainder = zremainder_value;
    sail_u256 zwhole;
    {
      sail_u256 z2zE2361;
      z2zE2361 = zleft.zwhole;
      sail_u256 z2zE2362;
      z2zE2362 = zright.zwhole;
      {
        zwhole = zblob_word_add(z2zE2361, z2zE2362);
        if (have_exception) {  goto end_block_exception_2637;  }
      }
    }
    sail_u256 zcarried_whole;
    if (zcarry) {
      {
        zcarried_whole = zblob_word_add(zwhole, zWORD_ONE);
        if (have_exception) {  goto end_block_exception_2637;  }
      }
    } else {  zcarried_whole = zwhole;  }
    struct zScaledBlobValue z3zE2318;
    z3zE2318.zremainder = zremainder;
    z3zE2318.zwhole = zcarried_whole;
    z8zE300 = z3zE2318;
  } else {
    struct zexception z2zE2363;
    CREATE(zexception)(&z2zE2363);
    zInvalidBlock(&z2zE2363, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2363);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:123.8-123.44");
    KILL(zexception)(&z2zE2363);
    goto end_block_exception_2637;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2363);
  }
end_function_2636: ;
  return z8zE300;
end_block_exception_2637: ;
  struct zScaledBlobValue z8zE817 = { .zremainder = UINT64_C(0xdeadc0de), .zwhole = u256_zero() };
  return z8zE817;
}

struct zBlobProductDivMod zblob_product_divmod(sail_u256 zvalue, uint64_t zfactor, sail_u256 zaddend, uint64_t zdenominator, uint64_t ziteration)
{
  struct zBlobProductDivMod z8zE301;
  sail_u128 zdivisor;
  {
    sail_u256 z3zE3022;
    z3zE3022 = u256_of_fbits(zdenominator);
    sail_u256 z3zE3023;
    z3zE3023 = u256_of_fbits(ziteration);
    sail_u256 z3zE3024;
    z3zE3024 = zword_mul_word(z3zE3022, z3zE3023);
    zdivisor = u128_of_u256(z3zE3024);
  }
  bool z2zE2356;
  z2zE2356 = zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdivisor);
  unit z3zE2316;
  if (z2zE2356) {
    struct zexception z2zE2357;
    CREATE(zexception)(&z2zE2357);
    zInvalidBlock(&z2zE2357, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2357);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:142.8-142.44");
    KILL(zexception)(&z2zE2357);
    goto end_block_exception_2635;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2357);
  } else {  z3zE2316 = UNIT;  }
  sail_u256 zvalue_quotient;
  zvalue_quotient = zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue, zdivisor);
  sail_u256 zvalue_remainder;
  zvalue_remainder = zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue, zdivisor);
  sail_u256 zwhole;
  {
    zwhole = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue_quotient, zfactor);
    if (have_exception) {  goto end_block_exception_2635;  }
  }
  sail_u256 zresidual_product;
  zresidual_product = zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue_remainder, zfactor);
  sail_u256 zresidual_sum;
  {
    zresidual_sum = zblob_word_add(zresidual_product, zaddend);
    if (have_exception) {  goto end_block_exception_2635;  }
  }
  sail_u256 zresidual_quotient;
  zresidual_quotient = zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zresidual_sum, zdivisor);
  sail_u256 z2zE2358;
  {
    z2zE2358 = zblob_word_add(zwhole, zresidual_quotient);
    if (have_exception) {  goto end_block_exception_2635;  }
  }
  sail_u256 z2zE2359;
  z2zE2359 = zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zresidual_sum, zdivisor);
  struct zBlobProductDivMod z3zE2317;
  z3zE2317.zquotient = z2zE2358;
  z3zE2317.zremainder = z2zE2359;
  z8zE301 = z3zE2317;
end_function_2634: ;
  return z8zE301;
end_block_exception_2635: ;
  struct zBlobProductDivMod z8zE818 = { .zquotient = u256_zero(), .zremainder = u256_zero() };
  return z8zE818;
}

struct zScaledBlobValue zscaled_blob_next(struct zScaledBlobValue zterm, uint64_t znumerator, uint64_t zdenominator, uint64_t ziteration)
{
  struct zScaledBlobValue z8zE302;
  sail_u128 zremainder_product;
  {
    uint64_t z2zE2355;
    z2zE2355 = zterm.zremainder;
    {
      sail_u256 z3zE3025;
      z3zE3025 = u256_of_fbits(z2zE2355);
      sail_u256 z3zE3026;
      z3zE3026 = u256_of_fbits(znumerator);
      sail_u256 z3zE3027;
      z3zE3027 = zword_mul_word(z3zE3025, z3zE3026);
      zremainder_product = u128_of_u256(z3zE3027);
    }
  }
  sail_u256 zaddend;
  zaddend = zword_div_wordzIreprzGR__sail_c_repr_u128zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zremainder_product, zdenominator);
  struct zBlobProductDivMod zdivided;
  {
    sail_u256 z2zE2354;
    z2zE2354 = zterm.zwhole;
    {
      zdivided = zblob_product_divmod(z2zE2354, znumerator, zaddend, zdenominator, ziteration);
      if (have_exception) {  goto end_block_exception_2633;  }
    }
  }
  sail_u256 zremainder_word;
  {
    sail_u256 z2zE2353;
    z2zE2353 = zdivided.zremainder;
    zremainder_word = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2353, ziteration);
  }
  bool z2zE2350;
  z2zE2350 = u256_lt_u64(zremainder_word, zdenominator);
  if (z2zE2350) {
    uint64_t zremainder;
    zremainder = u256_to_u64(zremainder_word);
    sail_u256 z2zE2351;
    z2zE2351 = zdivided.zquotient;
    struct zScaledBlobValue z3zE2315;
    z3zE2315.zremainder = zremainder;
    z3zE2315.zwhole = z2zE2351;
    z8zE302 = z3zE2315;
  } else {
    struct zexception z2zE2352;
    CREATE(zexception)(&z2zE2352);
    zInvalidBlock(&z2zE2352, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2352);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:170.8-170.44");
    KILL(zexception)(&z2zE2352);
    goto end_block_exception_2633;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2352);
  }
end_function_2632: ;
  return z8zE302;
end_block_exception_2633: ;
  struct zScaledBlobValue z8zE819 = { .zremainder = UINT64_C(0xdeadc0de), .zwhole = u256_zero() };
  return z8zE819;
}

sail_u256 zfake_exponential(sail_u256 zfactor, uint64_t znumerator, uint64_t zdenominator)
{
  sail_u256 z8zE303;
  uint64_t zi;
  zi = UINT64_C(1);
  struct zScaledBlobValue zoutput;
  {
    struct zScaledBlobValue z3zE2307;
    z3zE2307.zremainder = UINT64_C(0);
    z3zE2307.zwhole = zWORD_ZERO;
    zoutput = z3zE2307;
  }
  struct zScaledBlobValue zterm;
  {
    struct zScaledBlobValue z3zE2308;
    z3zE2308.zremainder = UINT64_C(0);
    z3zE2308.zwhole = zfactor;
    zterm = z3zE2308;
  }
  bool z3zE2312;
  unit z3zE2313;
while_2628: ;
  {
    bool z2zE2349;
    {
      sail_u256 z2zE2347;
      z2zE2347 = zterm.zwhole;
      z2zE2349 = zword_nonzzero(z2zE2347);
    }
    bool z3zE2309;
    if (z2zE2349) {  z3zE2309 = true;  } else {
      uint64_t z2zE2348;
      z2zE2348 = zterm.zremainder;
      z3zE2309 = (z2zE2348 != UINT64_C(0));
    }
    z3zE2312 = z3zE2309;
    if (!(z3zE2312)) goto wend_2629;
    {
      zoutput = zscaled_blob_add(zoutput, zterm, zdenominator);
      if (have_exception) {  goto end_block_exception_2631;  }
    }
    unit z3zE2311;
    z3zE2311 = UNIT;
    {
      zterm = zscaled_blob_next(zterm, znumerator, zdenominator, zi);
      if (have_exception) {  goto end_block_exception_2631;  }
    }
    unit z3zE2310;
    z3zE2310 = UNIT;
    {
      zi = zfake_exponential_index_increment(zi);
      if (have_exception) {  goto end_block_exception_2631;  }
    }
    z3zE2313 = UNIT;
    goto while_2628;
  }
wend_2629: ;
  unit z3zE2314;
  z3zE2314 = UNIT;
  z8zE303 = zoutput.zwhole;
end_function_2630: ;
  return z8zE303;
end_block_exception_2631: ;

  return u256_zero();
}

sail_u256 zblob_base_fee(uint64_t zexcess_blob_gas)
{
  sail_u256 z8zE304;
  uint64_t z2zE2346;
  z2zE2346 = zk_blob_schedule.zbase_fee_update_fraction;
  {
    z8zE304 = zfake_exponential(zMIN_BLOB_BASE_FEE, zexcess_blob_gas, z2zE2346);
    if (have_exception) {  goto end_block_exception_2627;  }
  }
end_function_2626: ;
  return z8zE304;
end_block_exception_2627: ;

  return u256_zero();
}

bool zblob_reserve_price_active(sail_u256 zbase_fee, sail_u256 zprice)
{
  bool z8zE305;
  bool z2zE2344;
  z2zE2344 = zword_ult(zprice, zBLOB_RESERVE_PRICE_SHIFT_LIMIT);
  if (z2zE2344) {
    sail_u256 z2zE2345;
    z2zE2345 = zword_shift_left(zprice, UINT64_C(4));
    z8zE305 = zword_ult(z2zE2345, zbase_fee);
  } else {  z8zE305 = false;  }
end_function_2624: ;
  return z8zE305;
end_block_exception_2625: ;

  return false;
}

uint64_t zblob_schedule_target(unit z3zE2306)
{
  uint64_t z8zE306;
  z8zE306 = zk_blob_schedule.ztarget;
end_function_2622: ;
  return z8zE306;
end_block_exception_2623: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_schedule_max(unit z3zE2305)
{
  uint64_t z8zE307;
  z8zE307 = zk_blob_schedule.zmax;
end_function_2620: ;
  return z8zE307;
end_block_exception_2621: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_gas_for_count(uint64_t zcount)
{
  uint64_t z8zE308;
  {    z8zE308 = (zGAS_PER_BLOB * zcount);
  }
end_function_2618: ;
  return z8zE308;
end_block_exception_2619: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_target_gas_per_block(unit z3zE2304)
{
  uint64_t z8zE309;
  uint64_t z2zE2343;
  z2zE2343 = zblob_schedule_target(UNIT);
  z8zE309 = zblob_gas_for_count(z2zE2343);
end_function_2616: ;
  return z8zE309;
end_block_exception_2617: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_max_gas_per_block(unit z3zE2303)
{
  uint64_t z8zE310;
  uint64_t z2zE2342;
  z2zE2342 = zblob_schedule_max(UNIT);
  z8zE310 = zblob_gas_for_count(z2zE2342);
end_function_2614: ;
  return z8zE310;
end_block_exception_2615: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zchecked_block_blob_gas_add(uint64_t zaccumulated, uint64_t ztransaction)
{
  uint64_t z8zE311;
  uint64_t zmaximum;
  zmaximum = zblob_max_gas_per_block(UNIT);
  bool z2zE2337;
  z2zE2337 = (!(zmaximum < zaccumulated));
  if (z2zE2337) {
    bool z2zE2339;
    {
      uint64_t z2zE2338;
      {    z2zE2338 = (zmaximum - zaccumulated);
      }
      z2zE2339 = (!(z2zE2338 < ztransaction));
    }
    if (z2zE2339) {
      {    z8zE311 = (zaccumulated + ztransaction);
      }
    } else {
      struct zexception z2zE2340;
      CREATE(zexception)(&z2zE2340);
      zInvalidBlock(&z2zE2340, zBlobGasLimitExceeded);
      COPY(zexception)(current_exception, z2zE2340);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/gas.sail:227.12-227.52");
      KILL(zexception)(&z2zE2340);
      goto end_block_exception_2613;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2340);
    }
  } else {
    struct zexception z2zE2341;
    CREATE(zexception)(&z2zE2341);
    zInvalidBlock(&z2zE2341, zBlobGasLimitExceeded);
    COPY(zexception)(current_exception, z2zE2341);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:230.8-230.48");
    KILL(zexception)(&z2zE2341);
    goto end_block_exception_2613;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2341);
  }
end_function_2612: ;
  return z8zE311;
end_block_exception_2613: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t znext_excess_blob_gas(uint64_t zparent_excess_blob_gas, uint64_t zparent_blob_gas_used, sail_u256 zparent_base_fee_per_gas)
{
  uint64_t z8zE312;
  sail_u128 zparent_blob_gas;
  zparent_blob_gas = u128_add_u64_u64(zparent_excess_blob_gas, zparent_blob_gas_used);
  uint64_t ztarget_blob_gas;
  ztarget_blob_gas = zblob_target_gas_per_block(UNIT);
  bool z2zE2327;
  z2zE2327 = u128_lt_u64(zparent_blob_gas, ztarget_blob_gas);
  if (z2zE2327) {  z8zE312 = UINT64_C(0);  } else {
    sail_u256 zprice;
    {
      zprice = zblob_base_fee(zparent_excess_blob_gas);
      if (have_exception) {  goto end_block_exception_2611;  }
    }
    bool z2zE2329;
    {
      bool z2zE2328;
      z2zE2328 = zfork_gteq(zk_fork, zOsaka);
      bool z3zE2299;
      if (z2zE2328) {  z3zE2299 = zblob_reserve_price_active(zparent_base_fee_per_gas, zprice);  } else {
        z3zE2299 = false;
      }
      z2zE2329 = z3zE2299;
    }
    if (z2zE2329) {
      uint64_t ztarget;
      ztarget = zblob_schedule_target(UNIT);
      uint64_t zmaximum;
      zmaximum = zblob_schedule_max(UNIT);
      bool z2zE2331;
      {
        bool z2zE2330;
        z2zE2330 = (zmaximum == UINT64_C(0));
        bool z3zE2300;
        if (z2zE2330) {  z3zE2300 = true;  } else {  z3zE2300 = (zmaximum < ztarget);  }
        z2zE2331 = z3zE2300;
      }
      unit z3zE2301;
      if (z2zE2331) {
        struct zexception z2zE2332;
        CREATE(zexception)(&z2zE2332);
        zInvalidBlock(&z2zE2332, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE2332);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/gas.sail:255.16-255.49");
        KILL(zexception)(&z2zE2332);
        goto end_block_exception_2611;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2332);
      } else {  z3zE2301 = UNIT;  }
      uint64_t zschedule_delta;
      {    zschedule_delta = (zmaximum - ztarget);
      }
      uint64_t zscaled;
      {    zscaled = (zparent_blob_gas_used * zschedule_delta);
      }
      uint64_t zshare;
      zshare = (zscaled / zmaximum);
      sail_u128 znext;
      znext = u128_add_u64_u64(zparent_excess_blob_gas, zshare);
      bool z2zE2333;
      z2zE2333 = (!u64_lt_u128(UINT64_C(18446744073709551615), znext));
      if (z2zE2333) {  z8zE312 = u128_to_u64(znext);  } else {
        struct zexception z2zE2334;
        CREATE(zexception)(&z2zE2334);
        zInvalidBlock(&z2zE2334, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE2334);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/gas.sail:264.16-264.49");
        KILL(zexception)(&z2zE2334);
        goto end_block_exception_2611;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2334);
      }
    } else {
      sail_u128 z3zE2302;
      z3zE2302 = u128_sub_u64(zparent_blob_gas, ztarget_blob_gas);
      bool z2zE2335;
      z2zE2335 = (!u64_lt_u128(UINT64_C(18446744073709551615), z3zE2302));
      if (z2zE2335) {  z8zE312 = u128_to_u64(z3zE2302);  } else {
        struct zexception z2zE2336;
        CREATE(zexception)(&z2zE2336);
        zInvalidBlock(&z2zE2336, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE2336);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/gas.sail:271.16-271.49");
        KILL(zexception)(&z2zE2336);
        goto end_block_exception_2611;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2336);
      }
    }
  }
end_function_2610: ;
  return z8zE312;
end_block_exception_2611: ;

  return UINT64_C(0xdeadc0de);
}

void create_letbind_123(void) {

  uint64_t z3zE142;
  z3zE142 = UINT64_C(0);
  zG_zzero = z3zE142;
let_end_450: ;
}
void kill_letbind_123(void) {
}

void create_letbind_124(void) {

  uint64_t z3zE143;
  z3zE143 = UINT64_C(1);
  zG_jumpdest = z3zE143;
let_end_451: ;
}
void kill_letbind_124(void) {
}

void create_letbind_125(void) {

  uint64_t z3zE144;
  z3zE144 = UINT64_C(2);
  zG_base = z3zE144;
let_end_452: ;
}
void kill_letbind_125(void) {
}

void create_letbind_126(void) {

  uint64_t z3zE145;
  z3zE145 = UINT64_C(3);
  zG_verylow = z3zE145;
let_end_453: ;
}
void kill_letbind_126(void) {
}

void create_letbind_127(void) {

  uint64_t z3zE146;
  z3zE146 = UINT64_C(5);
  zG_low = z3zE146;
let_end_454: ;
}
void kill_letbind_127(void) {
}

void create_letbind_128(void) {

  uint64_t z3zE147;
  z3zE147 = UINT64_C(8);
  zG_mid = z3zE147;
let_end_455: ;
}
void kill_letbind_128(void) {
}

void create_letbind_129(void) {

  uint64_t z3zE148;
  z3zE148 = UINT64_C(10);
  zG_high = z3zE148;
let_end_456: ;
}
void kill_letbind_129(void) {
}

void create_letbind_130(void) {

  uint64_t z3zE149;
  z3zE149 = UINT64_C(100);
  zG_warm_access = z3zE149;
let_end_457: ;
}
void kill_letbind_130(void) {
}

void create_letbind_131(void) {

  uint64_t z3zE150;
  z3zE150 = UINT64_C(2100);
  zG_cold_sload = z3zE150;
let_end_458: ;
}
void kill_letbind_131(void) {
}

void create_letbind_132(void) {

  uint64_t z3zE151;
  z3zE151 = UINT64_C(2600);
  zG_cold_account = z3zE151;
let_end_459: ;
}
void kill_letbind_132(void) {
}

void create_letbind_133(void) {

  uint64_t z3zE152;
  z3zE152 = UINT64_C(30);
  zG_keccak = z3zE152;
let_end_460: ;
}
void kill_letbind_133(void) {
}

void create_letbind_134(void) {

  uint64_t z3zE153;
  z3zE153 = UINT64_C(6);
  zG_keccak_word = z3zE153;
let_end_461: ;
}
void kill_letbind_134(void) {
}

void create_letbind_135(void) {

  uint64_t z3zE154;
  z3zE154 = UINT64_C(3);
  zG_copy_word = z3zE154;
let_end_462: ;
}
void kill_letbind_135(void) {
}

void create_letbind_136(void) {

  uint64_t z3zE155;
  z3zE155 = UINT64_C(3);
  zG_memory = z3zE155;
let_end_463: ;
}
void kill_letbind_136(void) {
}

void create_letbind_137(void) {

  uint64_t z3zE156;
  z3zE156 = UINT64_C(375);
  zG_log = z3zE156;
let_end_464: ;
}
void kill_letbind_137(void) {
}

void create_letbind_138(void) {

  uint64_t z3zE157;
  z3zE157 = UINT64_C(375);
  zG_logtopic = z3zE157;
let_end_465: ;
}
void kill_letbind_138(void) {
}

void create_letbind_139(void) {

  uint64_t z3zE158;
  z3zE158 = UINT64_C(8);
  zG_logdata = z3zE158;
let_end_466: ;
}
void kill_letbind_139(void) {
}

void create_letbind_140(void) {

  uint64_t z3zE159;
  z3zE159 = UINT64_C(10);
  zG_exp = z3zE159;
let_end_467: ;
}
void kill_letbind_140(void) {
}

void create_letbind_141(void) {

  uint64_t z3zE160;
  z3zE160 = UINT64_C(50);
  zG_expbyte = z3zE160;
let_end_468: ;
}
void kill_letbind_141(void) {
}

void create_letbind_142(void) {

  uint64_t z3zE161;
  z3zE161 = UINT64_C(20000);
  zG_sset = z3zE161;
let_end_469: ;
}
void kill_letbind_142(void) {
}

void create_letbind_143(void) {

  uint64_t z3zE162;
  z3zE162 = UINT64_C(2900);
  zG_sreset = z3zE162;
let_end_470: ;
}
void kill_letbind_143(void) {
}

void create_letbind_144(void) {

  uint64_t z3zE163;
  z3zE163 = UINT64_C(15000);
  zR_sclear_pre_london = z3zE163;
let_end_471: ;
}
void kill_letbind_144(void) {
}

void create_letbind_145(void) {

  uint64_t z3zE164;
  z3zE164 = UINT64_C(4800);
  zR_sclear_london = z3zE164;
let_end_472: ;
}
void kill_letbind_145(void) {
}

void create_letbind_146(void) {

  uint64_t z3zE165;
  z3zE165 = UINT64_C(24000);
  zR_selfdestruct_pre_london = z3zE165;
let_end_473: ;
}
void kill_letbind_146(void) {
}

void create_letbind_147(void) {

  uint64_t z3zE166;
  z3zE166 = UINT64_C(32000);
  zG_create = z3zE166;
let_end_474: ;
}
void kill_letbind_147(void) {
}

void create_letbind_148(void) {

  uint64_t z3zE167;
  z3zE167 = UINT64_C(200);
  zG_codedeposit = z3zE167;
let_end_475: ;
}
void kill_letbind_148(void) {
}

void create_letbind_149(void) {

  uint64_t z3zE168;
  z3zE168 = UINT64_C(9000);
  zG_callvalue = z3zE168;
let_end_476: ;
}
void kill_letbind_149(void) {
}

void create_letbind_150(void) {

  uint64_t z3zE169;
  z3zE169 = UINT64_C(2300);
  zG_callstipend = z3zE169;
let_end_477: ;
}
void kill_letbind_150(void) {
}

void create_letbind_151(void) {

  uint64_t z3zE170;
  z3zE170 = UINT64_C(25000);
  zG_newaccount = z3zE170;
let_end_478: ;
}
void kill_letbind_151(void) {
}

void create_letbind_152(void) {

  uint64_t z3zE171;
  z3zE171 = UINT64_C(5000);
  zG_selfdestruct = z3zE171;
let_end_479: ;
}
void kill_letbind_152(void) {
}

void create_letbind_153(void) {

  uint64_t z3zE172;
  z3zE172 = UINT64_C(2);
  zG_initcode_word = z3zE172;
let_end_480: ;
}
void kill_letbind_153(void) {
}

void create_letbind_154(void) {

  uint64_t z3zE173;
  z3zE173 = UINT64_C(3000);
  zG_amsterdam_cold_account_access = z3zE173;
let_end_481: ;
}
void kill_letbind_154(void) {
}

void create_letbind_155(void) {

  uint64_t z3zE174;
  z3zE174 = UINT64_C(3000);
  zG_amsterdam_cold_storage_access = z3zE174;
let_end_482: ;
}
void kill_letbind_155(void) {
}

void create_letbind_156(void) {

  uint64_t z3zE175;
  z3zE175 = UINT64_C(10000);
  zG_amsterdam_storage_write = z3zE175;
let_end_483: ;
}
void kill_letbind_156(void) {
}

void create_letbind_157(void) {

  uint64_t z3zE176;
  z3zE176 = UINT64_C(8000);
  zG_amsterdam_account_write = z3zE176;
let_end_484: ;
}
void kill_letbind_157(void) {
}

void create_letbind_158(void) {

  uint64_t z3zE177;
  z3zE177 = UINT64_C(10300);
  zG_amsterdam_call_value = z3zE177;
let_end_485: ;
}
void kill_letbind_158(void) {
}

void create_letbind_159(void) {

  uint64_t z3zE178;
  z3zE178 = UINT64_C(11000);
  zG_amsterdam_create_access = z3zE178;
let_end_486: ;
}
void kill_letbind_159(void) {
}

void create_letbind_160(void) {

  uint64_t z3zE179;
  z3zE179 = UINT64_C(1530);
  zG_amsterdam_state_byte = z3zE179;
let_end_487: ;
}
void kill_letbind_160(void) {
}

void create_letbind_161(void) {

  uint64_t z3zE180;
  z3zE180 = UINT64_C(97920);
  zG_amsterdam_state_storage_set = z3zE180;
let_end_488: ;
}
void kill_letbind_161(void) {
}

void create_letbind_162(void) {

  uint64_t z3zE181;
  z3zE181 = UINT64_C(183600);
  zG_amsterdam_state_new_account = z3zE181;
let_end_489: ;
}
void kill_letbind_162(void) {
}

void create_letbind_163(void) {

  uint64_t z3zE182;
  z3zE182 = UINT64_C(35190);
  zG_amsterdam_state_auth_base = z3zE182;
let_end_490: ;
}
void kill_letbind_163(void) {
}

void create_letbind_164(void) {

  uint64_t z3zE183;
  z3zE183 = UINT64_C(12480);
  zR_amsterdam_storage_clear = z3zE183;
let_end_491: ;
}
void kill_letbind_164(void) {
}

void create_letbind_165(void) {

  uint64_t z3zE184;
  z3zE184 = UINT64_C(2301);
  zG_sstore_sentry = z3zE184;
let_end_492: ;
}
void kill_letbind_165(void) {
}

void create_letbind_166(void) {

  uint64_t z3zE185;
  z3zE185 = UINT64_C(24576);
  zEIP170_DEPLOYED_CODE_SIZE_LIMIT = z3zE185;
let_end_493: ;
}
void kill_letbind_166(void) {
}

void create_letbind_167(void) {

  uint64_t z3zE186;
  z3zE186 = UINT64_C(49152);
  zEIP3860_INITCODE_SIZE_LIMIT = z3zE186;
let_end_494: ;
}
void kill_letbind_167(void) {
}

void create_letbind_168(void) {

  uint64_t z3zE187;
  z3zE187 = UINT64_C(65536);
  zEIP7954_DEPLOYED_CODE_SIZE_LIMIT = z3zE187;
let_end_495: ;
}
void kill_letbind_168(void) {
}

void create_letbind_169(void) {

  uint64_t z3zE188;
  z3zE188 = UINT64_C(131072);
  zEIP7954_INITCODE_SIZE_LIMIT = z3zE188;
let_end_496: ;
}
void kill_letbind_169(void) {
}

uint64_t zsstore_clear_refund(unit z3zE2298)
{
  uint64_t z8zE313;
  bool z2zE2322;
  z2zE2322 = zfork_gteq(zk_fork, zLondon);
  if (z2zE2322) {  z8zE313 = zR_sclear_london;  } else {  z8zE313 = zR_sclear_pre_london;  }
end_function_2604: ;
  return z8zE313;
end_block_exception_2605: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zstate_gas_spill_room(uint64_t zleft)
{
  uint64_t z8zE314;
  {    z8zE314 = (UINT64_C(16777216) - zleft);
  }
end_function_2598: ;
  return z8zE314;
end_block_exception_2599: ;

  return UINT64_C(0xdeadc0de);
}

unit zcredit_state_gas_refund(uint64_t zamount)
{
  unit z8zE315;
  uint64_t zspilled;
  zspilled = zstate_gas_spilled;
  bool z2zE2302;
  z2zE2302 = (!(zspilled < zamount));
  if (z2zE2302) {
    bool z2zE2303;
    z2zE2303 = (zamount != UINT64_C(0));
    if (z2zE2303) {
      {
        zgas_remaining = zconserved_gas_add(zgas_remaining, zamount);
        if (have_exception) {  goto end_block_exception_2587;  }
      }
      unit z3zE2289;
      z3zE2289 = UNIT;
      {    zstate_gas_spilled = (zspilled - zamount);
      }
      z8zE315 = UNIT;
    } else {  z8zE315 = UNIT;  }
  } else {
    bool z2zE2304;
    z2zE2304 = (zspilled != UINT64_C(0));
    unit z3zE2287;
    if (z2zE2304) {
      {
        zgas_remaining = zconserved_gas_add(zgas_remaining, zspilled);
        if (have_exception) {  goto end_block_exception_2587;  }
      }
      unit z3zE2288;
      z3zE2288 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      z3zE2287 = UNIT;
    } else {  z3zE2287 = UNIT;  }
    uint64_t zto_state;
    {    zto_state = (zamount - zspilled);
    }
    {
      zstate_gas_remaining = zconserved_gas_add(zstate_gas_remaining, zto_state);
      if (have_exception) {  goto end_block_exception_2587;  }
    }
    z8zE315 = UNIT;
  }
end_function_2586: ;
  return z8zE315;
end_block_exception_2587: ;

  return UNIT;
}

unit zreturn_child_state_gas(uint64_t zchild_remaining, uint64_t zchild_spilled)
{
  unit z8zE316;
  {
    zstate_gas_remaining = zconserved_gas_add(zstate_gas_remaining, zchild_remaining);
    if (have_exception) {  goto end_block_exception_2585;  }
  }
  unit z3zE2286;
  z3zE2286 = UNIT;
  {
    zstate_gas_spilled = zstate_gas_spill_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zstate_gas_spilled, zchild_spilled);
    if (have_exception) {  goto end_block_exception_2585;  }
  }
  z8zE316 = UNIT;
end_function_2584: ;
  return z8zE316;
end_block_exception_2585: ;

  return UNIT;
}

uint64_t zgas_sub_or_oog(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE317;
  bool z2zE2301;
  z2zE2301 = (!(zleft < zright));
  if (z2zE2301) {
    {    z8zE317 = (zleft - zright);
    }
  } else {
    unit z3zE2285;
    {
      z3zE2285 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2583;  }
    }
    z8zE317 = zGAS_ZERO;
  }
end_function_2582: ;
  return z8zE317;
end_block_exception_2583: ;

  return UINT64_C(0xdeadc0de);
}

unit zrefund_gas(uint64_t zamount)
{
  unit z8zE318;
  {
    zgas_remaining = zconserved_gas_add(zgas_remaining, zamount);
    if (have_exception) {  goto end_block_exception_2581;  }
  }
  z8zE318 = UNIT;
end_function_2580: ;
  return z8zE318;
end_block_exception_2581: ;

  return UNIT;
}

sail_u256 zmemory_word_count_word(sail_u256 zbyte_len)
{
  sail_u256 z8zE319;
  sail_u256 zquotient;
  zquotient = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zbyte_len, UINT64_C(32));
  bool z2zE2299;
  {
    sail_u256 z2zE2298;
    z2zE2298 = zword_mod_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zbyte_len, UINT64_C(32));
    z2zE2299 = eq_u256(z2zE2298, zWORD_ZERO);
  }
  if (z2zE2299) {  z8zE319 = zquotient;  } else {  z8zE319 = zword_add_word(zquotient, zWORD_ONE);  }
end_function_2576: ;
  return z8zE319;
end_block_exception_2577: ;

  return u256_zero();
}

sail_u128 zbounded_mem_cost(uint64_t zwords)
{
  sail_u128 z8zE320;
  sail_u128 z2zE2296;
  z2zE2296 = u128_mul_u64_u64(zG_memory, zwords);
  sail_u128 z2zE2297;
  {
    sail_u128 z2zE2295;
    z2zE2295 = u128_mul_u64_u64(zwords, zwords);
    z2zE2297 = u128_div_u64(z2zE2295, UINT64_C(512));
  }
  z8zE320 = u128_add(z2zE2296, z2zE2297);
end_function_2574: ;
  return z8zE320;
end_block_exception_2575: ;

  return u128_zero();
}

uint64_t zmemory_expansion_cost(uint64_t zrequired_sizze, uint64_t zavailable)
{
  uint64_t z8zE321;
  uint64_t znew_words;
  znew_words = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired_sizze);
  uint64_t zold_words;
  {
    uint64_t z2zE2294;
    z2zE2294 = zevm_memory_high_water(UNIT);
    zold_words = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2294);
  }
  bool z2zE2290;
  z2zE2290 = (!(zold_words < znew_words));
  if (z2zE2290) {  z8zE321 = UINT64_C(0);  } else {
    sail_u128 zold_cost;
    zold_cost = zbounded_mem_cost(zold_words);
    sail_u128 znew_cost;
    znew_cost = zbounded_mem_cost(znew_words);
    bool z2zE2293;
    {
      bool z2zE2292;
      z2zE2292 = (!u128_lt(znew_cost, zold_cost));
      bool z3zE2282;
      if (z2zE2292) {
        sail_u128 z2zE2291;
        z2zE2291 = u128_add_u64(zold_cost, zavailable);
        z3zE2282 = (!u128_lt(z2zE2291, znew_cost));
      } else {  z3zE2282 = false;  }
      z2zE2293 = z3zE2282;
    }
    if (z2zE2293) {
      uint64_t zcost;
      {
        uint64_t z3zE2284;
        z3zE2284 = u128_to_u64(u128_sub(znew_cost, zold_cost));
        zcost = z3zE2284;
      }
      z8zE321 = zcost;
    } else {
      unit z3zE2283;
      {
        z3zE2283 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2573;  }
      }
      z8zE321 = UINT64_C(0);
    }
  }
end_function_2572: ;
  return z8zE321;
end_block_exception_2573: ;

  return UINT64_C(0xdeadc0de);
}

struct zMemoryExpansion zmemory_expansion(sail_u256 zstart, sail_u256 zsizze, uint64_t zavailable)
{
  struct zMemoryExpansion z8zE322;
  bool z2zE2283;
  z2zE2283 = u256_eq_u64(zsizze, UINT64_C(0));
  if (z2zE2283) {
    struct zMemoryExpansion z3zE2281;
    z3zE2281.zcost = UINT64_C(0);
    z3zE2281.zrange = zEMPTY_MEMORY_RANGE;
    z3zE2281.zrequired_sizze = UINT64_C(0);
    z8zE322 = z3zE2281;
  } else {
    bool z2zE2285;
    {
      bool z2zE2284;
      z2zE2284 = (!u64_lt_u256(UINT64_C(18446744073709551615), zstart));
      bool z3zE2267;
      if (z2zE2284) {  z3zE2267 = (!u64_lt_u256(UINT64_C(18446744073709551615), zsizze));  } else {
        z3zE2267 = false;
      }
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
          zbounded_sizze = u256_to_u64(zsizze);
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
            sail_match_failure("memory_expansion");
          finish_match_2568: ;
            z3zE2272 = z3zE2273;
            goto finish_match_2566;
          }
        case_2567: ;
          sail_match_failure("memory_expansion");
        finish_match_2566: ;
          z3zE2271 = z3zE2272;
          goto finish_match_2564;
        }
      case_2565: ;
        sail_match_failure("memory_expansion");
      finish_match_2564: ;
        z3zE2270 = z3zE2271;
        goto finish_match_2562;
      }
    case_2563: ;
      sail_match_failure("memory_expansion");
    finish_match_2562: ;
      z8zE322 = z3zE2270;
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
      z8zE322 = z3zE2269;
    }
  }
end_function_2570: ;
  return z8zE322;
end_block_exception_2571: ;
  struct zMemoryRangeFields z8zE821 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryExpansion z8zE820 = { .zcost = UINT64_C(0xdeadc0de), .zrange = z8zE821, .zrequired_sizze = UINT64_C(0xdeadc0de) };
  return z8zE820;
}

struct zMemoryPairExpansion zmemory_pair_expansion(sail_u256 zleft_start, sail_u256 zleft_sizze, sail_u256 zright_start, sail_u256 zright_sizze, uint64_t zavailable)
{
  struct zMemoryPairExpansion z8zE323;
  bool zleft_live;
  zleft_live = (!u256_eq_u64(zleft_sizze, UINT64_C(0)));
  bool zright_live;
  zright_live = (!u256_eq_u64(zright_sizze, UINT64_C(0)));
  bool zleft_fits;
  {
    bool z2zE2282;
    z2zE2282 = not(zleft_live);
    bool z3zE2254;
    if (z2zE2282) {  z3zE2254 = true;  } else {
      bool z2zE2281;
      z2zE2281 = (!u64_lt_u256(UINT64_C(18446744073709551615), zleft_start));
      bool z3zE2253;
      if (z2zE2281) {  z3zE2253 = (!u64_lt_u256(UINT64_C(18446744073709551615), zleft_sizze));  } else {
        z3zE2253 = false;
      }
      z3zE2254 = z3zE2253;
    }
    zleft_fits = z3zE2254;
  }
  bool zright_fits;
  {
    bool z2zE2280;
    z2zE2280 = not(zright_live);
    bool z3zE2256;
    if (z2zE2280) {  z3zE2256 = true;  } else {
      bool z2zE2279;
      z2zE2279 = (!u64_lt_u256(UINT64_C(18446744073709551615), zright_start));
      bool z3zE2255;
      if (z2zE2279) {  z3zE2255 = (!u64_lt_u256(UINT64_C(18446744073709551615), zright_sizze));  } else {
        z3zE2255 = false;
      }
      z3zE2256 = z3zE2255;
    }
    zright_fits = z3zE2256;
  }
  bool z2zE2272;
  {
    bool z3zE2257;
    if (zleft_fits) {  z3zE2257 = zright_fits;  } else {  z3zE2257 = false;  }
    z2zE2272 = z3zE2257;
  }
  if (z2zE2272) {
    uint64_t zbounded_left_start;
    if (zleft_live) {  zbounded_left_start = u256_to_u64(zleft_start);  } else {  zbounded_left_start = UINT64_C(0);  }
    uint64_t zbounded_left_sizze;
    if (zleft_live) {  zbounded_left_sizze = u256_to_u64(zleft_sizze);  } else {  zbounded_left_sizze = UINT64_C(0);  }
    uint64_t zbounded_right_start;
    if (zright_live) {  zbounded_right_start = u256_to_u64(zright_start);  } else {
      zbounded_right_start = UINT64_C(0);
    }
    uint64_t zbounded_right_sizze;
    if (zright_live) {  zbounded_right_sizze = u256_to_u64(zright_sizze);  } else {
      zbounded_right_sizze = UINT64_C(0);
    }
    sail_u128 zleft_end;
    zleft_end = u128_add_u64_u64(zbounded_left_start, zbounded_left_sizze);
    sail_u128 zright_end;
    zright_end = u128_add_u64_u64(zbounded_right_start, zbounded_right_sizze);
    sail_u128 zrequired;
    {
      bool z2zE2278;
      z2zE2278 = u128_lt(zleft_end, zright_end);
      if (z2zE2278) {  zrequired = zright_end;  } else {  zrequired = zleft_end;  }
    }
    uint64_t z2zE2273;
    z2zE2273 = zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
    struct zMemoryPairExpansion z3zE2260;
    {
      uint64_t zwords;
      zwords = z2zE2273;
      struct zMemoryPairExpansion z3zE2261;
      {
        uint64_t zbounded_words;
        zbounded_words = zwords;
        sail_u128 zexpansion_cost;
        zexpansion_cost = zbounded_mem_cost(zbounded_words);
        bool z2zE2274;
        z2zE2274 = (!u64_lt_u128(UINT64_C(18446744073709551615), zexpansion_cost));
        if (z2zE2274) {
          uint64_t zrequired_sizze;
          zrequired_sizze = u128_to_u64(zrequired);
          struct zMemoryRangeFields z2zE2275;
          if (zleft_live) {  z2zE2275 = zmemory_range(zbounded_left_start, zbounded_left_sizze);  } else {
            z2zE2275 = zEMPTY_MEMORY_RANGE;
          }
          struct zMemoryRangeFields z2zE2276;
          if (zright_live) {  z2zE2276 = zmemory_range(zbounded_right_start, zbounded_right_sizze);  } else {
            z2zE2276 = zEMPTY_MEMORY_RANGE;
          }
          uint64_t z2zE2277;
          {
            z2zE2277 = zmemory_expansion_cost(zrequired_sizze, zavailable);
            if (have_exception) {  goto end_block_exception_2561;  }
          }
          struct zMemoryPairExpansion z3zE2265;
          z3zE2265.zcost = z2zE2277;
          z3zE2265.zleft = z2zE2275;
          z3zE2265.zrequired_sizze = zrequired_sizze;
          z3zE2265.zright = z2zE2276;
          z3zE2261 = z3zE2265;
        } else {
          unit z3zE2263;
          {
            z3zE2263 = zexc_halt(zOutOfGas);
            if (have_exception) {  goto end_block_exception_2561;  }
          }
          struct zMemoryPairExpansion z3zE2264;
          z3zE2264.zcost = UINT64_C(0);
          z3zE2264.zleft = zEMPTY_MEMORY_RANGE;
          z3zE2264.zrequired_sizze = UINT64_C(0);
          z3zE2264.zright = zEMPTY_MEMORY_RANGE;
          z3zE2261 = z3zE2264;
        }
        goto finish_match_2558;
      }
    case_2559: ;
      sail_match_failure("memory_pair_expansion");
    finish_match_2558: ;
      z3zE2260 = z3zE2261;
      goto finish_match_2556;
    }
  case_2557: ;
    sail_match_failure("memory_pair_expansion");
  finish_match_2556: ;
    z8zE323 = z3zE2260;
  } else {
    unit z3zE2258;
    {
      z3zE2258 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2561;  }
    }
    struct zMemoryPairExpansion z3zE2259;
    z3zE2259.zcost = UINT64_C(0);
    z3zE2259.zleft = zEMPTY_MEMORY_RANGE;
    z3zE2259.zrequired_sizze = UINT64_C(0);
    z3zE2259.zright = zEMPTY_MEMORY_RANGE;
    z8zE323 = z3zE2259;
  }
end_function_2560: ;
  return z8zE323;
end_block_exception_2561: ;
  struct zMemoryRangeFields z8zE824 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryRangeFields z8zE823 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryPairExpansion z8zE822 = { .zcost = UINT64_C(0xdeadc0de), .zleft = z8zE824, .zrequired_sizze = UINT64_C(0xdeadc0de), .zright = z8zE823 };
  return z8zE822;
}

unit zexpand_memory(uint64_t zrequired_sizze)
{
  unit z8zE324;
  struct zByteSliceFields z2zE2271;
  z2zE2271 = zmemory_expand_to(zrequired_sizze);
  unit z3zE2251;
  {
    z3zE2251 = UNIT;
    goto finish_match_2552;
  }
case_2553: ;
  sail_match_failure("expand_memory");
finish_match_2552: ;
  z8zE324 = z3zE2251;
end_function_2554: ;
  return z8zE324;
end_block_exception_2555: ;

  return UNIT;
}

struct zMemoryRangeFields zapply_memory_expansion(struct zMemoryExpansion zplan)
{
  struct zMemoryRangeFields z8zE325;
  uint64_t z2zE2270;
  z2zE2270 = zplan.zrequired_sizze;
  unit z3zE2250;
  z3zE2250 = zexpand_memory(z2zE2270);
  z8zE325 = zplan.zrange;
end_function_2550: ;
  return z8zE325;
end_block_exception_2551: ;
  struct zMemoryRangeFields z8zE825 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE825;
}

struct zMemoryRangeFields zcharge_memory_range(sail_u256 zstart, sail_u256 zsizze)
{
  struct zMemoryRangeFields z8zE326;
  bool z2zE2267;
  z2zE2267 = zis_running(UNIT);
  if (z2zE2267) {
    uint64_t zavailable;
    zavailable = zgas_remaining;
    struct zMemoryExpansion zplan;
    {
      zplan = zmemory_expansion(zstart, zsizze, zavailable);
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
    if (z2zE2269) {  z8zE326 = zapply_memory_expansion(zplan);  } else {  z8zE326 = zEMPTY_MEMORY_RANGE;  }
  } else {  z8zE326 = zEMPTY_MEMORY_RANGE;  }
end_function_2548: ;
  return z8zE326;
end_block_exception_2549: ;
  struct zMemoryRangeFields z8zE826 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE826;
}

struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 zapply_memory_pair_expansion(struct zMemoryPairExpansion zplan)
{
  struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z8zE327;
  uint64_t z2zE2264;
  z2zE2264 = zplan.zrequired_sizze;
  unit z3zE2247;
  z3zE2247 = zexpand_memory(z2zE2264);
  struct zMemoryRangeFields z2zE2265;
  z2zE2265 = zplan.zleft;
  struct zMemoryRangeFields z2zE2266;
  z2zE2266 = zplan.zright;
  struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z3zE2248;
  z3zE2248.ztup0 = z2zE2265;
  z3zE2248.ztup1 = z2zE2266;
  z8zE327 = z3zE2248;
end_function_2546: ;
  return z8zE327;
end_block_exception_2547: ;
  struct zMemoryRangeFields z8zE829 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryRangeFields z8zE828 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z8zE827 = { .ztup0 = z8zE829, .ztup1 = z8zE828 };
  return z8zE827;
}

uint64_t zaccount_cost(bool zwarm)
{
  uint64_t z8zE328;
  if (zwarm) {  z8zE328 = zG_warm_access;  } else {
    bool z2zE2263;
    z2zE2263 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE2263) {  z8zE328 = zG_amsterdam_cold_account_access;  } else {  z8zE328 = zG_cold_account;  }
  }
end_function_2544: ;
  return z8zE328;
end_block_exception_2545: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zexternal_code_read_cost(unit z3zE2246)
{
  uint64_t z8zE329;
  bool z2zE2262;
  z2zE2262 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2262) {  z8zE329 = zG_warm_access;  } else {  z8zE329 = zG_zzero;  }
end_function_2542: ;
  return z8zE329;
end_block_exception_2543: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsload_cost(bool zwarm)
{
  uint64_t z8zE330;
  if (zwarm) {  z8zE330 = zG_warm_access;  } else {
    bool z2zE2261;
    z2zE2261 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE2261) {  z8zE330 = zG_amsterdam_cold_storage_access;  } else {  z8zE330 = zG_cold_sload;  }
  }
end_function_2540: ;
  return z8zE330;
end_block_exception_2541: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zcall_value_cost(unit z3zE2245)
{
  uint64_t z8zE331;
  bool z2zE2260;
  z2zE2260 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2260) {  z8zE331 = zG_amsterdam_call_value;  } else {  z8zE331 = zG_callvalue;  }
end_function_2538: ;
  return z8zE331;
end_block_exception_2539: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zcreate_access_cost(unit z3zE2244)
{
  uint64_t z8zE332;
  bool z2zE2259;
  z2zE2259 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2259) {  z8zE332 = zG_amsterdam_create_access;  } else {  z8zE332 = zG_create;  }
end_function_2536: ;
  return z8zE332;
end_block_exception_2537: ;

  return UINT64_C(0xdeadc0de);
}

void zcode_deployment_execution_cost(struct zoptionzIU64zK *z8zE333, uint64_t zbyte_len, uint64_t zavailable)
{
  bool z2zE2253;
  z2zE2253 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2253) {
    bool z2zE2254;
    z2zE2254 = (!(zEIP7954_DEPLOYED_CODE_SIZE_LIMIT < zbyte_len));
    if (z2zE2254) {
      uint64_t zwords;
      zwords = zmemory_word_countzIreprzGU64zCU64zKzIboundszG07d9444e226eec3cde1a1e781c91ddf4zK(zbyte_len);
      bool z2zE2256;
      {
        uint64_t z2zE2255;
        z2zE2255 = (zavailable / zG_keccak_word);
        z2zE2256 = (!(z2zE2255 < zwords));
      }
      if (z2zE2256) {
        uint64_t zcost;
        {    zcost = (zG_keccak_word * zwords);
        }
        zSomezIU64zK((*(&z8zE333)), zcost);
      } else {  zNonezIU64zK((*(&z8zE333)), UNIT);  }
    } else {  zNonezIU64zK((*(&z8zE333)), UNIT);  }
  } else {
    bool z2zE2258;
    {
      uint64_t z2zE2257;
      z2zE2257 = (zavailable / zG_codedeposit);
      z2zE2258 = (!(z2zE2257 < zbyte_len));
    }
    if (z2zE2258) {
      uint64_t z3zE2243;
      {    z3zE2243 = (zG_codedeposit * zbyte_len);
      }
      zSomezIU64zK((*(&z8zE333)), z3zE2243);
    } else {  zNonezIU64zK((*(&z8zE333)), UNIT);  }
  }
end_function_2534: ;
  goto end_function_3640;
end_block_exception_2535: ;
  goto end_function_3640;
end_function_3640: ;
}

uint64_t zcode_deployment_state_cost(uint64_t zbyte_len)
{
  uint64_t z8zE334;
  bool z2zE2250;
  z2zE2250 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2250) {
    bool z2zE2251;
    z2zE2251 = (!(zEIP7954_DEPLOYED_CODE_SIZE_LIMIT < zbyte_len));
    if (z2zE2251) {
      {    z8zE334 = (zG_amsterdam_state_byte * zbyte_len);
      }
    } else {
      struct zexception z2zE2252;
      CREATE(zexception)(&z2zE2252);
      zInvalidBlock(&z2zE2252, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE2252);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/gas.sail:811.12-811.48");
      KILL(zexception)(&z2zE2252);
      goto end_block_exception_2533;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2252);
    }
  } else {  z8zE334 = zGAS_COST_ZERO;  }
end_function_2532: ;
  return z8zE334;
end_block_exception_2533: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zpc_word(struct zByteSliceFields zinput, uint64_t zstart, uint64_t zbyte_count)
{
  sail_u256 z8zE335;
  sail_u256 zvalue;
  zvalue = zZERO_WORD;
  uint64_t zstart_offset;
  zstart_offset = zstart;
  uint64_t zcount;
  zcount = zbyte_count;
  uint64_t zinput_length;
  zinput_length = zinput.zlen;
  int64_t z3zE2238;
  {    z3zE2238 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2239;
  {    z3zE2239 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2240;
  {    z3zE2240 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE2238;
    unit z3zE2241;
  for_start_2528: ;
    {
      if ((z3zE2239 < zbyte_index)) goto for_end_2529;
      uint64_t zoffset;
      zoffset = (uint64_t)(zbyte_index);
      bool z2zE2245;
      z2zE2245 = (zoffset < zcount);
      if (z2zE2245) {
        uint64_t znext_byte;
        {
          bool z2zE2248;
          z2zE2248 = (zstart_offset < zinput_length);
          if (z2zE2248) {
            uint64_t zremaining;
            {    zremaining = (zinput_length - zstart_offset);
            }
            bool z2zE2249;
            z2zE2249 = (zoffset < zremaining);
            if (z2zE2249) {
              uint64_t zcursor;
              {    zcursor = (zstart_offset + zoffset);
              }
              znext_byte = zslice_byte(zinput, zcursor);
            } else {  znext_byte = UINT64_C(0x00);  }
          } else {  znext_byte = UINT64_C(0x00);  }
        }
        sail_u256 z2zE2246;
        z2zE2246 = zword_shift_left(zvalue, UINT64_C(8));
        uint64_t z2zE2247;
        z2zE2247 = ((uint64_t) znext_byte);
        zvalue = zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2246, z2zE2247);
        z3zE2241 = UNIT;
      } else {  z3zE2241 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE2240);
      goto for_start_2528;
    }
  for_end_2529: ;
  }
  unit z3zE2242;
  z3zE2242 = UNIT;
  z8zE335 = zvalue;
end_function_2530: ;
  return z8zE335;
end_block_exception_2531: ;

  return u256_zero();
}

sail_u256 zpc_word_after_declared_field(struct zByteSliceFields zinput, uint64_t zprefix, sail_u256 zdeclared_length, uint64_t zbyte_count)
{
  sail_u256 z8zE336;
  uint64_t zinput_length;
  zinput_length = zinput.zlen;
  sail_u256 z3zE2232;
  {
    uint64_t zprefix_offset;
    zprefix_offset = zprefix;
    bool z2zE2242;
    z2zE2242 = (zprefix_offset < zinput_length);
    if (z2zE2242) {
      uint64_t z2zE2243;
      {    z2zE2243 = (zinput_length - zprefix_offset);
      }
      sail_u256 z3zE2234;
      {
        uint64_t zsuffix_length;
        zsuffix_length = z2zE2243;
        bool z2zE2244;
        z2zE2244 = u256_lt_u64(zdeclared_length, zsuffix_length);
        if (z2zE2244) {
          sail_u256 z3zE2236;
          {
            uint64_t zfield_length;
            zfield_length = u256_to_u64(zdeclared_length);
            uint64_t zstart;
            {    zstart = (zprefix_offset + zfield_length);
            }
            z3zE2236 = zpc_word(zinput, zstart, zbyte_count);
            goto finish_match_2524;
          }
        case_2525: ;
          sail_match_failure("pc_word_after_declared_field");
        finish_match_2524: ;
          z3zE2234 = z3zE2236;
        } else {  z3zE2234 = zZERO_WORD;  }
        goto finish_match_2522;
      }
    case_2523: ;
      sail_match_failure("pc_word_after_declared_field");
    finish_match_2522: ;
      z3zE2232 = z3zE2234;
    } else {  z3zE2232 = zZERO_WORD;  }
    goto finish_match_2520;
  }
case_2521: ;
  sail_match_failure("pc_word_after_declared_field");
finish_match_2520: ;
  z8zE336 = z3zE2232;
end_function_2526: ;
  return z8zE336;
end_block_exception_2527: ;

  return u256_zero();
}

uint64_t zpc_blake2_rounds(struct zByteSliceFields zinput)
{
  uint64_t z8zE337;
  sail_u256 z2zE2241;
  z2zE2241 = zpc_word(zinput, UINT64_C(0), UINT64_C(4));
  uint64_t z3zE2231;
  z3zE2231 = u256_to_u64(u256_mod_u64(z2zE2241, UINT64_C(4294967296)));
  z8zE337 = z3zE2231;
end_function_2518: ;
  return z8zE337;
end_block_exception_2519: ;

  return UINT64_C(0xdeadc0de);
}

void zmodexp_gas(struct zoptionzIU64zK *z8zE338, struct zByteSliceFields zinput, uint64_t zavailable)
{
  sail_u256 zbl_word;
  zbl_word = zpc_word(zinput, UINT64_C(0), UINT64_C(32));
  sail_u256 zel_word;
  zel_word = zpc_word(zinput, UINT64_C(32), UINT64_C(32));
  sail_u256 zml_word;
  zml_word = zpc_word(zinput, UINT64_C(64), UINT64_C(32));
  sail_u256 zbl;
  zbl = zbl_word;
  sail_u256 zel;
  zel = zel_word;
  sail_u256 zml;
  zml = zml_word;
  bool zosaka;
  zosaka = zfork_gteq(zk_fork, zOsaka);
  bool z2zE2206;
  {
    bool z3zE2206;
    if (zosaka) {
      bool z2zE2205;
      z2zE2205 = u64_lt_u256(UINT64_C(1024), zbl);
      bool z3zE2205;
      if (z2zE2205) {  z3zE2205 = true;  } else {
        bool z2zE2204;
        z2zE2204 = u64_lt_u256(UINT64_C(1024), zel);
        bool z3zE2204;
        if (z2zE2204) {  z3zE2204 = true;  } else {  z3zE2204 = u64_lt_u256(UINT64_C(1024), zml);  }
        z3zE2205 = z3zE2204;
      }
      z3zE2206 = z3zE2205;
    } else {  z3zE2206 = false;  }
    z2zE2206 = z3zE2206;
  }
  unit z3zE2207;
  if (z2zE2206) {
    struct zoptionzIU64zK z2zE2207;
    CREATE(zoptionzIU64zK)(&z2zE2207);
    zNonezIU64zK(&z2zE2207, UNIT);
    (*(z8zE338)) = z2zE2207;
    goto cleanup_2515;
    /* unreachable after return */
    KILL(zoptionzIU64zK)(&z2zE2207);
    goto end_cleanup_2516;
  cleanup_2515: ;
    KILL(zoptionzIU64zK)(&z2zE2207);
    goto end_function_2498;
  end_cleanup_2516: ;
  } else {  z3zE2207 = UNIT;  }
  uint64_t zminimum;
  if (zosaka) {  zminimum = UINT64_C(500);  } else {  zminimum = UINT64_C(200);  }
  bool z2zE2208;
  z2zE2208 = (zavailable < zminimum);
  unit z3zE2211;
  if (z2zE2208) {
    struct zoptionzIU64zK z2zE2209;
    CREATE(zoptionzIU64zK)(&z2zE2209);
    zNonezIU64zK(&z2zE2209, UNIT);
    (*(z8zE338)) = z2zE2209;
    goto cleanup_2513;
    /* unreachable after return */
    KILL(zoptionzIU64zK)(&z2zE2209);
    goto end_cleanup_2514;
  cleanup_2513: ;
    KILL(zoptionzIU64zK)(&z2zE2209);
    goto end_function_2498;
  end_cleanup_2514: ;
  } else {  z3zE2211 = UNIT;  }
  bool z2zE2212;
  {
    bool z2zE2211;
    z2zE2211 = not(zosaka);
    bool z3zE2209;
    if (z2zE2211) {
      bool z2zE2210;
      z2zE2210 = zword_is_zzero(zbl_word);
      bool z3zE2208;
      if (z2zE2210) {  z3zE2208 = zword_is_zzero(zml_word);  } else {  z3zE2208 = false;  }
      z3zE2209 = z3zE2208;
    } else {  z3zE2209 = false;  }
    z2zE2212 = z3zE2209;
  }
  unit z3zE2210;
  if (z2zE2212) {
    struct zoptionzIU64zK z2zE2213;
    CREATE(zoptionzIU64zK)(&z2zE2213);
    zSomezIU64zK(&z2zE2213, UINT64_C(200));
    (*(z8zE338)) = z2zE2213;
    goto cleanup_2511;
    /* unreachable after return */
    KILL(zoptionzIU64zK)(&z2zE2213);
    goto end_cleanup_2512;
  cleanup_2511: ;
    KILL(zoptionzIU64zK)(&z2zE2213);
    goto end_function_2498;
  end_cleanup_2512: ;
  } else {  z3zE2210 = UNIT;  }
  sail_u256 zmaxlen;
  {
    bool z2zE2240;
    z2zE2240 = u256_lt(zml, zbl);
    if (z2zE2240) {  zmaxlen = zbl;  } else {  zmaxlen = zml;  }
  }
  sail_u256 zwhole_words;
  zwhole_words = u256_div_u64(zmaxlen, UINT64_C(8));
  sail_u256 zwords;
  {
    uint64_t z2zE2239;
    {
      bool z2zE2238;
      {
        uint64_t z2zE2237;
        {
          uint64_t z3zE2212;
          z3zE2212 = u256_to_u64(u256_mod_u64(zmaxlen, UINT64_C(8)));
          z2zE2237 = z3zE2212;
        }
        z2zE2238 = (z2zE2237 == UINT64_C(0));
      }
      if (z2zE2238) {  z2zE2239 = UINT64_C(0);  } else {  z2zE2239 = UINT64_C(1);  }
    }
    zwords = u256_add_u64(zwhole_words, z2zE2239);
  }
  sail_u128 zproduct_limit_value;
  if (zosaka) {  zproduct_limit_value = u128_of_u64(zavailable);  } else {
    sail_u128 z2zE2236;
    z2zE2236 = u128_mul_u64_u64(UINT64_C(3), zavailable);
    zproduct_limit_value = u128_add_u64(z2zE2236, UINT64_C(2));
  }
  struct zoptionzIU64zK z3zE2213;
  CREATE(zoptionzIU64zK)(&z3zE2213);
  {
    sail_u128 zproduct_limit;
    zproduct_limit = zproduct_limit_value;
    bool zwords_unaffordable;
    zwords_unaffordable = u128_lt_u256(zproduct_limit, zwords);
    sail_u128 zbounded_words;
    if (zwords_unaffordable) {  zbounded_words = zproduct_limit;  } else {  zbounded_words = u128_of_u256(zwords);  }
    unit z3zE2214;
    if (zwords_unaffordable) {
      struct zoptionzIU64zK z2zE2214;
      CREATE(zoptionzIU64zK)(&z2zE2214);
      zNonezIU64zK(&z2zE2214, UNIT);
      (*(z8zE338)) = z2zE2214;
      goto cleanup_2509;
      /* unreachable after return */
      KILL(zoptionzIU64zK)(&z2zE2214);
      goto end_cleanup_2510;
    cleanup_2509: ;
      KILL(zoptionzIU64zK)(&z3zE2213);
      KILL(zoptionzIU64zK)(&z2zE2214);
      goto end_function_2498;
    end_cleanup_2510: ;
    } else {  z3zE2214 = UNIT;  }
    sail_u256 zwords_squared;
    zwords_squared = u256_mul_u128_u128(zbounded_words, zbounded_words);
    sail_u128 zwide_product_limit;
    zwide_product_limit = zproduct_limit;
    bool zsquare_unaffordable;
    zsquare_unaffordable = u128_lt_u256(zwide_product_limit, zwords_squared);
    sail_u128 zaffordable_square;
    if (zsquare_unaffordable) {  zaffordable_square = zproduct_limit;  } else {
      zaffordable_square = u128_of_u256(zwords_squared);
    }
    unit z3zE2215;
    if (zsquare_unaffordable) {
      struct zoptionzIU64zK z2zE2215;
      CREATE(zoptionzIU64zK)(&z2zE2215);
      zNonezIU64zK(&z2zE2215, UNIT);
      (*(z8zE338)) = z2zE2215;
      goto cleanup_2507;
      /* unreachable after return */
      KILL(zoptionzIU64zK)(&z2zE2215);
      goto end_cleanup_2508;
    cleanup_2507: ;
      KILL(zoptionzIU64zK)(&z3zE2213);
      KILL(zoptionzIU64zK)(&z2zE2215);
      goto end_function_2498;
    end_cleanup_2508: ;
    } else {  z3zE2215 = UNIT;  }
    sail_u128 zmult_value;
    {
      bool z2zE2234;
      {
        bool z3zE2216;
        if (zosaka) {  z3zE2216 = (!u64_lt_u256(UINT64_C(32), zmaxlen));  } else {  z3zE2216 = false;  }
        z2zE2234 = z3zE2216;
      }
      if (z2zE2234) {  zmult_value = u128_of_u64(UINT64_C(16));  } else if (zosaka) {
        uint64_t zdoubled_limit_value;
        zdoubled_limit_value = (zavailable / UINT64_C(2));
        sail_u128 z3zE2217;
        {
          uint64_t zdoubled_limit;
          zdoubled_limit = zdoubled_limit_value;
          uint64_t zwide_doubled_limit;
          zwide_doubled_limit = zdoubled_limit;
          bool zdoubled_unaffordable;
          zdoubled_unaffordable = u64_lt_u256(zwide_doubled_limit, zwords_squared);
          uint64_t zbounded_square;
          if (zdoubled_unaffordable) {  zbounded_square = UINT64_C(0);  } else {
            zbounded_square = u256_to_u64(zwords_squared);
          }
          unit z3zE2218;
          if (zdoubled_unaffordable) {
            struct zoptionzIU64zK z2zE2235;
            CREATE(zoptionzIU64zK)(&z2zE2235);
            zNonezIU64zK(&z2zE2235, UNIT);
            (*(z8zE338)) = z2zE2235;
            goto cleanup_2505;
            /* unreachable after return */
            KILL(zoptionzIU64zK)(&z2zE2235);
            goto end_cleanup_2506;
          cleanup_2505: ;
            KILL(zoptionzIU64zK)(&z3zE2213);
            KILL(zoptionzIU64zK)(&z2zE2235);
            goto end_function_2498;
          end_cleanup_2506: ;
          } else {  z3zE2218 = UNIT;  }
          z3zE2217 = u128_mul_u64_u64(UINT64_C(2), zbounded_square);
          goto finish_match_2494;
        }
      case_2495: ;
        sail_match_failure("modexp_gas");
      finish_match_2494: ;
        zmult_value = z3zE2217;
      } else {  zmult_value = zaffordable_square;  }
    }
    struct zoptionzIU64zK z3zE2220;
    CREATE(zoptionzIU64zK)(&z3zE2220);
    {
      sail_u128 zmult;
      zmult = zmult_value;
      bool z2zE2216;
      z2zE2216 = u128_lt(zproduct_limit, zmult);
      unit z3zE2221;
      if (z2zE2216) {
        struct zoptionzIU64zK z2zE2217;
        CREATE(zoptionzIU64zK)(&z2zE2217);
        zNonezIU64zK(&z2zE2217, UNIT);
        (*(z8zE338)) = z2zE2217;
        goto cleanup_2503;
        /* unreachable after return */
        KILL(zoptionzIU64zK)(&z2zE2217);
        goto end_cleanup_2504;
      cleanup_2503: ;
        KILL(zoptionzIU64zK)(&z3zE2213);
        KILL(zoptionzIU64zK)(&z3zE2220);
        KILL(zoptionzIU64zK)(&z2zE2217);
        goto end_function_2498;
      end_cleanup_2504: ;
      } else {  z3zE2221 = UNIT;  }
      sail_u128 ziterations;
      {
        bool z2zE2221;
        z2zE2221 = (!u64_lt_u256(UINT64_C(32), zel));
        if (z2zE2221) {
          uint64_t zexponent_bits;
          {
            sail_u256 z2zE2224;
            {
              uint64_t z3zE3028;
              z3zE3028 = u256_to_u64(zel);
              z2zE2224 = zpc_word_after_declared_field(zinput, UINT64_C(96), zbl, z3zE3028);
            }
            zexponent_bits = zword_bit_length(z2zE2224);
          }
          bool z2zE2222;
          z2zE2222 = (zexponent_bits == UINT64_C(0));
          if (z2zE2222) {  ziterations = u128_of_u64(UINT64_C(1));  } else {
            uint64_t zcount;
            {    zcount = (zexponent_bits - UINT64_C(1));
            }
            bool z2zE2223;
            z2zE2223 = (zcount == UINT64_C(0));
            if (z2zE2223) {  ziterations = u128_of_u64(UINT64_C(1));  } else {  ziterations = u128_of_u64(zcount);  }
          }
        } else {
          uint64_t zhead_bits;
          {
            sail_u256 z2zE2233;
            z2zE2233 = zpc_word_after_declared_field(zinput, UINT64_C(96), zbl, UINT64_C(32));
            zhead_bits = zword_bit_length(z2zE2233);
          }
          uint64_t zhigh_bits;
          {
            bool z2zE2232;
            z2zE2232 = (zhead_bits != UINT64_C(0));
            if (z2zE2232) {
              {    zhigh_bits = (zhead_bits - UINT64_C(1));
              }
            } else {  zhigh_bits = UINT64_C(0);  }
          }
          sail_u256 zextra;
          zextra = u256_sub_u64(zel, UINT64_C(32));
          sail_u128 z3zE2227;
          if (zosaka) {
            uint64_t zextra_limit;
            zextra_limit = (zavailable / UINT64_C(16));
            uint64_t zosaka_extra;
            {
              bool z2zE2228;
              z2zE2228 = zword_greater_than_wordzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zextra, UINT64_C(992));
              if (z2zE2228) {  zosaka_extra = UINT64_C(992);  } else {  zosaka_extra = u256_to_u64(zextra);  }
            }
            bool zextra_unaffordable;
            zextra_unaffordable = (zextra_limit < zosaka_extra);
            uint64_t zbounded_extra;
            if (zextra_unaffordable) {  zbounded_extra = zextra_limit;  } else {  zbounded_extra = zosaka_extra;  }
            unit z3zE2224;
            if (zextra_unaffordable) {
              struct zoptionzIU64zK z2zE2226;
              CREATE(zoptionzIU64zK)(&z2zE2226);
              zNonezIU64zK(&z2zE2226, UNIT);
              (*(z8zE338)) = z2zE2226;
              goto cleanup_2501;
              /* unreachable after return */
              KILL(zoptionzIU64zK)(&z2zE2226);
              goto end_cleanup_2502;
            cleanup_2501: ;
              KILL(zoptionzIU64zK)(&z3zE2213);
              KILL(zoptionzIU64zK)(&z3zE2220);
              KILL(zoptionzIU64zK)(&z2zE2226);
              goto end_function_2498;
            end_cleanup_2502: ;
            } else {  z3zE2224 = UNIT;  }
            uint64_t z2zE2227;
            {    z2zE2227 = (UINT64_C(16) * zbounded_extra);
            }
            z3zE2227 = u128_add_u64_u64(z2zE2227, zhigh_bits);
          } else {
            sail_u128 zpre_osaka_limit;
            {
              sail_u128 z2zE2231;
              z2zE2231 = u128_mul_u64_u64(UINT64_C(3), zavailable);
              zpre_osaka_limit = u128_add_u64(z2zE2231, UINT64_C(2));
            }
            uint64_t z3zE2228;
            {
              uint64_t z3zE2222;
              z3zE2222 = u128_to_u64(u128_div_u64(zpre_osaka_limit, UINT64_C(8)));
              z3zE2228 = z3zE2222;
            }
            bool z3zE2229;
            z3zE2229 = u64_lt_u256(z3zE2228, zextra);
            uint64_t z3zE2230;
            if (z3zE2229) {  z3zE2230 = z3zE2228;  } else {  z3zE2230 = u256_to_u64(zextra);  }
            unit z3zE2223;
            if (z3zE2229) {
              struct zoptionzIU64zK z2zE2229;
              CREATE(zoptionzIU64zK)(&z2zE2229);
              zNonezIU64zK(&z2zE2229, UNIT);
              (*(z8zE338)) = z2zE2229;
              goto cleanup_2499;
              /* unreachable after return */
              KILL(zoptionzIU64zK)(&z2zE2229);
              goto end_cleanup_2500;
            cleanup_2499: ;
              KILL(zoptionzIU64zK)(&z3zE2213);
              KILL(zoptionzIU64zK)(&z3zE2220);
              KILL(zoptionzIU64zK)(&z2zE2229);
              goto end_function_2498;
            end_cleanup_2500: ;
            } else {  z3zE2223 = UNIT;  }
            sail_u128 z2zE2230;
            z2zE2230 = u128_mul_u64_u64(UINT64_C(8), z3zE2230);
            z3zE2227 = u128_add_u64(z2zE2230, zhigh_bits);
          }
          bool z2zE2225;
          z2zE2225 = u128_eq_u64(z3zE2227, UINT64_C(0));
          if (z2zE2225) {  ziterations = u128_of_u64(UINT64_C(1));  } else {  ziterations = z3zE2227;  }
        }
      }
      bool z2zE2218;
      z2zE2218 = u128_eq_u64(ziterations, UINT64_C(0));
      if (z2zE2218) {  zNonezIU64zK(&z3zE2220, UNIT);  } else {
        sail_u256 zproduct;
        zproduct = u256_mul_u128_u128(zmult, ziterations);
        bool zproduct_unaffordable;
        zproduct_unaffordable = u128_lt_u256(zwide_product_limit, zproduct);
        sail_u128 zaffordable_product;
        if (zproduct_unaffordable) {  zaffordable_product = zproduct_limit;  } else {
          zaffordable_product = u128_of_u256(zproduct);
        }
        if (zproduct_unaffordable) {  zNonezIU64zK(&z3zE2220, UNIT);  } else {
          sail_u128 zcalculated;
          if (zosaka) {  zcalculated = zaffordable_product;  } else {
            zcalculated = u128_div_u64(zaffordable_product, UINT64_C(3));
          }
          sail_u128 zcost;
          {
            bool z2zE2220;
            z2zE2220 = u128_lt_u64(zcalculated, zminimum);
            if (z2zE2220) {  zcost = u128_of_u64(zminimum);  } else {  zcost = zcalculated;  }
          }
          bool z2zE2219;
          z2zE2219 = (!u64_lt_u128(zavailable, zcost));
          if (z2zE2219) {
            uint64_t zaffordable;
            zaffordable = u128_to_u64(zcost);
            zSomezIU64zK(&z3zE2220, zaffordable);
          } else {  zNonezIU64zK(&z3zE2220, UNIT);  }
        }
      }
      goto finish_match_2496;
    }
  case_2497: ;
    sail_match_failure("modexp_gas");
  finish_match_2496: ;
    z3zE2213 = z3zE2220;
    KILL(zoptionzIU64zK)(&z3zE2220);
    goto finish_match_2492;
  }
case_2493: ;
  sail_match_failure("modexp_gas");
finish_match_2492: ;
  (*(z8zE338)) = z3zE2213;
  KILL(zoptionzIU64zK)(&z3zE2213);
end_function_2498: ;
  goto end_function_3639;
end_block_exception_2517: ;
  goto end_function_3639;
end_function_3639: ;
}

void create_letbind_170(void) {    CREATE(zz5vecz8z5bv16z9)(&zBLS_G1_DISCOUNT);

  zz5vecz8z5bv16z9 z3zE190;
  CREATE(zz5vecz8z5bv16z9)(&z3zE190);
  zz5vecz8z5bv16z9 z3zE189;
  CREATE(zz5vecz8z5bv16z9)(&z3zE189);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE189, INT64_C(128));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(0), UINT64_C(0x0207));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(1), UINT64_C(0x0208));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(2), UINT64_C(0x0208));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(3), UINT64_C(0x0209));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(4), UINT64_C(0x020A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(5), UINT64_C(0x020A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(6), UINT64_C(0x020B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(7), UINT64_C(0x020C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(8), UINT64_C(0x020D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(9), UINT64_C(0x020D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(10), UINT64_C(0x020E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(11), UINT64_C(0x020F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(12), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(13), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(14), UINT64_C(0x0211));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(15), UINT64_C(0x0212));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(16), UINT64_C(0x0213));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(17), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(18), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(19), UINT64_C(0x0215));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(20), UINT64_C(0x0216));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(21), UINT64_C(0x0217));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(22), UINT64_C(0x0218));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(23), UINT64_C(0x0218));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(24), UINT64_C(0x0219));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(25), UINT64_C(0x021A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(26), UINT64_C(0x021B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(27), UINT64_C(0x021C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(28), UINT64_C(0x021C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(29), UINT64_C(0x021D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(30), UINT64_C(0x021E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(31), UINT64_C(0x021F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(32), UINT64_C(0x0220));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(33), UINT64_C(0x0221));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(34), UINT64_C(0x0222));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(35), UINT64_C(0x0223));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(36), UINT64_C(0x0223));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(37), UINT64_C(0x0224));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(38), UINT64_C(0x0225));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(39), UINT64_C(0x0226));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(40), UINT64_C(0x0227));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(41), UINT64_C(0x0228));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(42), UINT64_C(0x0229));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(43), UINT64_C(0x022A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(44), UINT64_C(0x022B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(45), UINT64_C(0x022C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(46), UINT64_C(0x022D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(47), UINT64_C(0x022E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(48), UINT64_C(0x022F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(49), UINT64_C(0x0230));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(50), UINT64_C(0x0231));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(51), UINT64_C(0x0232));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(52), UINT64_C(0x0233));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(53), UINT64_C(0x0234));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(54), UINT64_C(0x0235));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(55), UINT64_C(0x0236));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(56), UINT64_C(0x0237));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(57), UINT64_C(0x0238));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(58), UINT64_C(0x0239));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(59), UINT64_C(0x023A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(60), UINT64_C(0x023C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(61), UINT64_C(0x023D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(62), UINT64_C(0x023E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(63), UINT64_C(0x023F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(64), UINT64_C(0x0240));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(65), UINT64_C(0x0241));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(66), UINT64_C(0x0243));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(67), UINT64_C(0x0244));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(68), UINT64_C(0x0245));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(69), UINT64_C(0x0246));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(70), UINT64_C(0x0248));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(71), UINT64_C(0x0249));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(72), UINT64_C(0x024A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(73), UINT64_C(0x024C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(74), UINT64_C(0x024D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(75), UINT64_C(0x024F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(76), UINT64_C(0x0250));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(77), UINT64_C(0x0251));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(78), UINT64_C(0x0253));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(79), UINT64_C(0x0254));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(80), UINT64_C(0x0256));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(81), UINT64_C(0x0257));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(82), UINT64_C(0x0259));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(83), UINT64_C(0x025B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(84), UINT64_C(0x025C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(85), UINT64_C(0x025E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(86), UINT64_C(0x0260));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(87), UINT64_C(0x0261));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(88), UINT64_C(0x0263));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(89), UINT64_C(0x0265));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(90), UINT64_C(0x0267));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(91), UINT64_C(0x0269));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(92), UINT64_C(0x026B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(93), UINT64_C(0x026D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(94), UINT64_C(0x026F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(95), UINT64_C(0x0271));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(96), UINT64_C(0x0273));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(97), UINT64_C(0x0276));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(98), UINT64_C(0x0278));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(99), UINT64_C(0x027B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(100), UINT64_C(0x027D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(101), UINT64_C(0x0280));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(102), UINT64_C(0x0282));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(103), UINT64_C(0x0285));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(104), UINT64_C(0x0288));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(105), UINT64_C(0x028B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(106), UINT64_C(0x028E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(107), UINT64_C(0x0292));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(108), UINT64_C(0x0295));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(109), UINT64_C(0x0299));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(110), UINT64_C(0x029D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(111), UINT64_C(0x02A1));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(112), UINT64_C(0x02A5));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(113), UINT64_C(0x02AA));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(114), UINT64_C(0x02AF));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(115), UINT64_C(0x02B4));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(116), UINT64_C(0x02BA));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(117), UINT64_C(0x02C1));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(118), UINT64_C(0x02C8));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(119), UINT64_C(0x02CF));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(120), UINT64_C(0x02D8));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(121), UINT64_C(0x02E2));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(122), UINT64_C(0x02EE));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(123), UINT64_C(0x02FC));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(124), UINT64_C(0x031D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(125), UINT64_C(0x0350));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(126), UINT64_C(0x03B5));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE189, z3zE189, INT64_C(127), UINT64_C(0x03E8));
  KILL(zz5vecz8z5bv16z9)(&z3zE190);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE190, z3zE189.len);
  for (int z8zE830 = 0; z8zE830 < z3zE189.len; z8zE830++) {
    uint64_t z8zE831 = z3zE189.data[z8zE830];
    uint64_t z8zE832;
    z8zE832 = z8zE831;
    z3zE190.data[z8zE830] = z8zE832;
  }
  KILL(zz5vecz8z5bv16z9)(&z3zE189);
  COPY(zz5vecz8z5bv16z9)(&zBLS_G1_DISCOUNT, z3zE190);
  KILL(zz5vecz8z5bv16z9)(&z3zE190);
let_end_532: ;
}
void kill_letbind_170(void) {    KILL(zz5vecz8z5bv16z9)(&zBLS_G1_DISCOUNT);
}

void create_letbind_171(void) {    CREATE(zz5vecz8z5bv16z9)(&zBLS_G2_DISCOUNT);

  zz5vecz8z5bv16z9 z3zE192;
  CREATE(zz5vecz8z5bv16z9)(&z3zE192);
  zz5vecz8z5bv16z9 z3zE191;
  CREATE(zz5vecz8z5bv16z9)(&z3zE191);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE191, INT64_C(128));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(0), UINT64_C(0x020C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(1), UINT64_C(0x020C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(2), UINT64_C(0x020D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(3), UINT64_C(0x020E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(4), UINT64_C(0x020E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(5), UINT64_C(0x020F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(6), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(7), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(8), UINT64_C(0x0211));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(9), UINT64_C(0x0212));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(10), UINT64_C(0x0212));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(11), UINT64_C(0x0213));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(12), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(13), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(14), UINT64_C(0x0215));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(15), UINT64_C(0x0216));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(16), UINT64_C(0x0217));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(17), UINT64_C(0x0217));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(18), UINT64_C(0x0218));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(19), UINT64_C(0x0219));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(20), UINT64_C(0x0219));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(21), UINT64_C(0x021A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(22), UINT64_C(0x021B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(23), UINT64_C(0x021C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(24), UINT64_C(0x021D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(25), UINT64_C(0x021D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(26), UINT64_C(0x021E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(27), UINT64_C(0x021F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(28), UINT64_C(0x0220));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(29), UINT64_C(0x0221));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(30), UINT64_C(0x0221));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(31), UINT64_C(0x0222));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(32), UINT64_C(0x0223));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(33), UINT64_C(0x0224));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(34), UINT64_C(0x0225));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(35), UINT64_C(0x0226));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(36), UINT64_C(0x0227));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(37), UINT64_C(0x0228));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(38), UINT64_C(0x0228));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(39), UINT64_C(0x0229));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(40), UINT64_C(0x022A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(41), UINT64_C(0x022B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(42), UINT64_C(0x022C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(43), UINT64_C(0x022D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(44), UINT64_C(0x022E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(45), UINT64_C(0x022F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(46), UINT64_C(0x0230));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(47), UINT64_C(0x0231));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(48), UINT64_C(0x0232));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(49), UINT64_C(0x0233));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(50), UINT64_C(0x0235));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(51), UINT64_C(0x0236));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(52), UINT64_C(0x0237));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(53), UINT64_C(0x0238));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(54), UINT64_C(0x0239));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(55), UINT64_C(0x023A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(56), UINT64_C(0x023B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(57), UINT64_C(0x023D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(58), UINT64_C(0x023E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(59), UINT64_C(0x023F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(60), UINT64_C(0x0240));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(61), UINT64_C(0x0242));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(62), UINT64_C(0x0243));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(63), UINT64_C(0x0244));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(64), UINT64_C(0x0246));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(65), UINT64_C(0x0247));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(66), UINT64_C(0x0248));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(67), UINT64_C(0x024A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(68), UINT64_C(0x024B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(69), UINT64_C(0x024D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(70), UINT64_C(0x024E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(71), UINT64_C(0x0250));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(72), UINT64_C(0x0251));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(73), UINT64_C(0x0253));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(74), UINT64_C(0x0255));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(75), UINT64_C(0x0256));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(76), UINT64_C(0x0258));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(77), UINT64_C(0x025A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(78), UINT64_C(0x025C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(79), UINT64_C(0x025E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(80), UINT64_C(0x025F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(81), UINT64_C(0x0261));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(82), UINT64_C(0x0263));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(83), UINT64_C(0x0265));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(84), UINT64_C(0x0267));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(85), UINT64_C(0x026A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(86), UINT64_C(0x026C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(87), UINT64_C(0x026E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(88), UINT64_C(0x0270));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(89), UINT64_C(0x0273));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(90), UINT64_C(0x0275));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(91), UINT64_C(0x0278));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(92), UINT64_C(0x027A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(93), UINT64_C(0x027D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(94), UINT64_C(0x0280));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(95), UINT64_C(0x0283));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(96), UINT64_C(0x0286));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(97), UINT64_C(0x0289));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(98), UINT64_C(0x028C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(99), UINT64_C(0x028F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(100), UINT64_C(0x0293));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(101), UINT64_C(0x0297));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(102), UINT64_C(0x029A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(103), UINT64_C(0x029E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(104), UINT64_C(0x02A2));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(105), UINT64_C(0x02A7));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(106), UINT64_C(0x02AB));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(107), UINT64_C(0x02B0));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(108), UINT64_C(0x02B5));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(109), UINT64_C(0x02BB));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(110), UINT64_C(0x02C0));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(111), UINT64_C(0x02C7));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(112), UINT64_C(0x02CD));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(113), UINT64_C(0x02D4));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(114), UINT64_C(0x02DC));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(115), UINT64_C(0x02E4));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(116), UINT64_C(0x02ED));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(117), UINT64_C(0x02F7));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(118), UINT64_C(0x0302));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(119), UINT64_C(0x030E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(120), UINT64_C(0x031C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(121), UINT64_C(0x032C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(122), UINT64_C(0x0340));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(123), UINT64_C(0x0357));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(124), UINT64_C(0x0374));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(125), UINT64_C(0x039B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(126), UINT64_C(0x03E8));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE191, z3zE191, INT64_C(127), UINT64_C(0x03E8));
  KILL(zz5vecz8z5bv16z9)(&z3zE192);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE192, z3zE191.len);
  for (int z8zE833 = 0; z8zE833 < z3zE191.len; z8zE833++) {
    uint64_t z8zE834 = z3zE191.data[z8zE833];
    uint64_t z8zE835;
    z8zE835 = z8zE834;
    z3zE192.data[z8zE833] = z8zE835;
  }
  KILL(zz5vecz8z5bv16z9)(&z3zE191);
  COPY(zz5vecz8z5bv16z9)(&zBLS_G2_DISCOUNT, z3zE192);
  KILL(zz5vecz8z5bv16z9)(&z3zE192);
let_end_533: ;
}
void kill_letbind_171(void) {    KILL(zz5vecz8z5bv16z9)(&zBLS_G2_DISCOUNT);
}

void zbls_msm_gas(struct zoptionzIU64zK *z8zE339, zz5vecz8z5bv16z9 ztable, uint64_t zbase, uint64_t zmaxd, uint64_t zk, uint64_t zavailable)
{
  bool z2zE2199;
  z2zE2199 = (zk == UINT64_C(0));
  if (z2zE2199) {  zSomezIU64zK((*(&z8zE339)), zGAS_COST_ZERO);  } else {
    uint64_t zdiscount;
    zdiscount = zmaxd;
    bool z2zE2200;
    z2zE2200 = (zk < UINT64_C(128));
    unit z3zE2203;
    if (z2zE2200) {
      uint64_t zindex;
      {    zindex = (UINT64_C(128) - zk);
      }
      uint64_t z2zE2201;
      z2zE2201 = fast_unsigned_vector_access_zz5vecz8z5bv16z9(ztable, zindex);
      zdiscount = ((uint64_t) z2zE2201);
      z3zE2203 = UNIT;
    } else {  z3zE2203 = UNIT;  }
    uint64_t zcoefficient;
    {    zcoefficient = (zbase * zdiscount);
    }
    uint64_t zwhole;
    zwhole = (zk / UINT64_C(1000));
    uint64_t zremainder;
    {    zremainder = (zk % UINT64_C(1000));
    }
    sail_u128 zwhole_product;
    zwhole_product = u128_mul_u64_u64(zcoefficient, zwhole);
    bool z2zE2202;
    z2zE2202 = u64_lt_u128(zavailable, zwhole_product);
    if (z2zE2202) {  zNonezIU64zK((*(&z8zE339)), UNIT);  } else {
      uint64_t zmajor;
      zmajor = u128_to_u64(zwhole_product);
      uint64_t ztail_product;
      {    ztail_product = (zcoefficient * zremainder);
      }
      uint64_t ztail;
      ztail = (ztail_product / UINT64_C(1000));
      sail_u128 ztotal;
      ztotal = u128_add_u64_u64(zmajor, ztail);
      bool z2zE2203;
      z2zE2203 = (!u64_lt_u128(zavailable, ztotal));
      if (z2zE2203) {
        uint64_t zaffordable;
        zaffordable = u128_to_u64(ztotal);
        zSomezIU64zK((*(&z8zE339)), zaffordable);
      } else {  zNonezIU64zK((*(&z8zE339)), UNIT);  }
    }
  }
end_function_2490: ;
  goto end_function_3638;
end_block_exception_2491: ;
  goto end_function_3638;
end_function_3638: ;
}

void zlinear_gas(struct zoptionzIU64zK *z8zE340, uint64_t zbase, uint64_t zper_unit, uint64_t zunits, uint64_t zavailable)
{
  sail_u128 zvariable_cost;
  zvariable_cost = u128_mul_u64_u64(zper_unit, zunits);
  sail_u128 zexact_cost;
  zexact_cost = u128_add_u64(zvariable_cost, zbase);
  bool z2zE2198;
  z2zE2198 = u64_lt_u128(zavailable, zexact_cost);
  if (z2zE2198) {  zNonezIU64zK((*(&z8zE340)), UNIT);  } else {
    uint64_t zaffordable;
    zaffordable = u128_to_u64(zexact_cost);
    zSomezIU64zK((*(&z8zE340)), zaffordable);
  }
end_function_2488: ;
  goto end_function_3637;
end_block_exception_2489: ;
  goto end_function_3637;
end_function_3637: ;
}

void zprecompile_gas(struct zoptionzIU64zK *z8zE341, uint64_t znum, struct zByteSliceFields zinput, uint64_t zavailable)
{
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  uint64_t zinput_length;
  zinput_length = zinput_len;
  uint64_t zwords;
  zwords = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zinput_len);
  struct zoptionzIU64zK z3zE2166;
  CREATE(zoptionzIU64zK)(&z3zE2166);
  {
    uint64_t zp0z3;
    zp0z3 = znum;
    bool z3zE2185;
    z3zE2185 = (zp0z3 == UINT64_C(1));
    if (!(z3zE2185)) {  goto case_2483;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(3000), zavailable);
    goto finish_match_2464;
  }
case_2483: ;
  {
    uint64_t z3zE2186;
    z3zE2186 = znum;
    bool z3zE2184;
    z3zE2184 = (z3zE2186 == UINT64_C(2));
    if (!(z3zE2184)) {  goto case_2482;  }
    zlinear_gas(&z3zE2166, UINT64_C(60), UINT64_C(12), zwords, zavailable);
    goto finish_match_2464;
  }
case_2482: ;
  {
    uint64_t z3zE2187;
    z3zE2187 = znum;
    bool z3zE2183;
    z3zE2183 = (z3zE2187 == UINT64_C(3));
    if (!(z3zE2183)) {  goto case_2481;  }
    zlinear_gas(&z3zE2166, UINT64_C(600), UINT64_C(120), zwords, zavailable);
    goto finish_match_2464;
  }
case_2481: ;
  {
    uint64_t z3zE2188;
    z3zE2188 = znum;
    bool z3zE2182;
    z3zE2182 = (z3zE2188 == UINT64_C(4));
    if (!(z3zE2182)) {  goto case_2480;  }
    zlinear_gas(&z3zE2166, UINT64_C(15), UINT64_C(3), zwords, zavailable);
    goto finish_match_2464;
  }
case_2480: ;
  {
    uint64_t z3zE2189;
    z3zE2189 = znum;
    bool z3zE2181;
    z3zE2181 = (z3zE2189 == UINT64_C(5));
    if (!(z3zE2181)) {  goto case_2479;  }
    zmodexp_gas(&z3zE2166, zinput, zavailable);
    goto finish_match_2464;
  }
case_2479: ;
  {
    uint64_t z3zE2190;
    z3zE2190 = znum;
    bool z3zE2180;
    z3zE2180 = (z3zE2190 == UINT64_C(6));
    if (!(z3zE2180)) {  goto case_2478;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(150), zavailable);
    goto finish_match_2464;
  }
case_2478: ;
  {
    uint64_t z3zE2191;
    z3zE2191 = znum;
    bool z3zE2179;
    z3zE2179 = (z3zE2191 == UINT64_C(7));
    if (!(z3zE2179)) {  goto case_2477;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(6000), zavailable);
    goto finish_match_2464;
  }
case_2477: ;
  {
    uint64_t z3zE2192;
    z3zE2192 = znum;
    bool z3zE2178;
    z3zE2178 = (z3zE2192 == UINT64_C(8));
    if (!(z3zE2178)) {  goto case_2476;  }
    uint64_t z2zE2192;
    z2zE2192 = (zinput_length / UINT64_C(192));
    zlinear_gas(&z3zE2166, UINT64_C(45000), UINT64_C(34000), z2zE2192, zavailable);
    goto finish_match_2464;
  }
case_2476: ;
  {
    uint64_t z3zE2193;
    z3zE2193 = znum;
    bool z3zE2177;
    z3zE2177 = (z3zE2193 == UINT64_C(9));
    if (!(z3zE2177)) {  goto case_2475;  }
    uint64_t z2zE2193;
    z2zE2193 = zpc_blake2_rounds(zinput);
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, z2zE2193, zavailable);
    goto finish_match_2464;
  }
case_2475: ;
  {
    uint64_t z3zE2194;
    z3zE2194 = znum;
    bool z3zE2176;
    z3zE2176 = (z3zE2194 == UINT64_C(10));
    if (!(z3zE2176)) {  goto case_2474;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(50000), zavailable);
    goto finish_match_2464;
  }
case_2474: ;
  {
    uint64_t z3zE2195;
    z3zE2195 = znum;
    bool z3zE2175;
    z3zE2175 = (z3zE2195 == UINT64_C(11));
    if (!(z3zE2175)) {  goto case_2473;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(375), zavailable);
    goto finish_match_2464;
  }
case_2473: ;
  {
    uint64_t z3zE2196;
    z3zE2196 = znum;
    bool z3zE2174;
    z3zE2174 = (z3zE2196 == UINT64_C(12));
    if (!(z3zE2174)) {  goto case_2472;  }
    uint64_t z2zE2194;
    z2zE2194 = (zinput_length / UINT64_C(160));
    zbls_msm_gas(&z3zE2166, zBLS_G1_DISCOUNT, UINT64_C(12000), UINT64_C(519), z2zE2194, zavailable);
    goto finish_match_2464;
  }
case_2472: ;
  {
    uint64_t z3zE2197;
    z3zE2197 = znum;
    bool z3zE2173;
    z3zE2173 = (z3zE2197 == UINT64_C(13));
    if (!(z3zE2173)) {  goto case_2471;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(600), zavailable);
    goto finish_match_2464;
  }
case_2471: ;
  {
    uint64_t z3zE2198;
    z3zE2198 = znum;
    bool z3zE2172;
    z3zE2172 = (z3zE2198 == UINT64_C(14));
    if (!(z3zE2172)) {  goto case_2470;  }
    uint64_t z2zE2195;
    z2zE2195 = (zinput_length / UINT64_C(288));
    zbls_msm_gas(&z3zE2166, zBLS_G2_DISCOUNT, UINT64_C(22500), UINT64_C(524), z2zE2195, zavailable);
    goto finish_match_2464;
  }
case_2470: ;
  {
    uint64_t z3zE2199;
    z3zE2199 = znum;
    bool z3zE2171;
    z3zE2171 = (z3zE2199 == UINT64_C(15));
    if (!(z3zE2171)) {  goto case_2469;  }
    uint64_t z2zE2196;
    z2zE2196 = (zinput_length / UINT64_C(384));
    zlinear_gas(&z3zE2166, UINT64_C(37700), UINT64_C(32600), z2zE2196, zavailable);
    goto finish_match_2464;
  }
case_2469: ;
  {
    uint64_t z3zE2200;
    z3zE2200 = znum;
    bool z3zE2170;
    z3zE2170 = (z3zE2200 == UINT64_C(16));
    if (!(z3zE2170)) {  goto case_2468;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(5500), zavailable);
    goto finish_match_2464;
  }
case_2468: ;
  {
    uint64_t z3zE2201;
    z3zE2201 = znum;
    bool z3zE2169;
    z3zE2169 = (z3zE2201 == UINT64_C(17));
    if (!(z3zE2169)) {  goto case_2467;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(23800), zavailable);
    goto finish_match_2464;
  }
case_2467: ;
  {
    uint64_t z3zE2202;
    z3zE2202 = znum;
    bool z3zE2168;
    z3zE2168 = (z3zE2202 == UINT64_C(256));
    if (!(z3zE2168)) {  goto case_2466;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2166, UINT64_C(6900), zavailable);
    goto finish_match_2464;
  }
case_2466: ;
  {
    zSomezIU64zK(&z3zE2166, zGAS_COST_ZERO);
    goto finish_match_2464;
  }
case_2465: ;
finish_match_2464: ;
  (*(z8zE341)) = z3zE2166;
  KILL(zoptionzIU64zK)(&z3zE2166);
end_function_2484: ;
  goto end_function_3636;
end_block_exception_2485: ;
  goto end_function_3636;
end_function_3636: ;
}

uint64_t zamsterdam_storage_access_cost(bool zcold)
{
  uint64_t z8zE342;
  if (zcold) {  z8zE342 = zG_amsterdam_cold_storage_access;  } else {  z8zE342 = zG_warm_access;  }
end_function_2462: ;
  return z8zE342;
end_block_exception_2463: ;

  return UINT64_C(0xdeadc0de);
}

struct zSstoreCosts zlegacy_sstore_costs(sail_u256 zoriginal, sail_u256 zcurrent, sail_u256 znew, bool zcold)
{
  struct zSstoreCosts z8zE343;
  uint64_t zcold_cost;
  if (zcold) {  zcold_cost = zG_cold_sload;  } else {  zcold_cost = zGAS_CONSTANT_ZERO;  }
  uint64_t zclear_refund;
  zclear_refund = zsstore_clear_refund(UNIT);
  uint64_t zbase;
  {
    bool z2zE2189;
    z2zE2189 = eq_u256(zcurrent, znew);
    if (z2zE2189) {  zbase = zG_warm_access;  } else {
      bool z2zE2190;
      z2zE2190 = eq_u256(zoriginal, zcurrent);
      if (z2zE2190) {
        bool z2zE2191;
        z2zE2191 = zword_is_zzero(zoriginal);
        if (z2zE2191) {  zbase = zG_sset;  } else {  zbase = zG_sreset;  }
      } else {  zbase = zG_warm_access;  }
    }
  }
  __int128 zrefund;
  zrefund = zGAS_REFUND_ZERO;
  bool z2zE2174;
  z2zE2174 = (!eq_u256(zcurrent, znew));
  unit z3zE2155;
  if (z2zE2174) {
    bool z2zE2175;
    z2zE2175 = eq_u256(zoriginal, zcurrent);
    if (z2zE2175) {
      bool z2zE2178;
      {
        bool z2zE2177;
        {
          bool z2zE2176;
          z2zE2176 = zword_is_zzero(zoriginal);
          z2zE2177 = not(z2zE2176);
        }
        bool z3zE2164;
        if (z2zE2177) {  z3zE2164 = zword_is_zzero(znew);  } else {  z3zE2164 = false;  }
        z2zE2178 = z3zE2164;
      }
      if (z2zE2178) {
        {
          zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zclear_refund);
          if (have_exception) {  goto end_block_exception_2461;  }
        }
        z3zE2155 = UNIT;
      } else {  z3zE2155 = UNIT;  }
    } else {
      bool z2zE2180;
      {
        bool z2zE2179;
        z2zE2179 = zword_is_zzero(zoriginal);
        z2zE2180 = not(z2zE2179);
      }
      unit z3zE2156;
      if (z2zE2180) {
        bool z2zE2181;
        z2zE2181 = zword_is_zzero(zcurrent);
        unit z3zE2157;
        if (z2zE2181) {
          int64_t z2zE2182;
          {
            int64_t z3zE2158;
            z3zE2158 = (int64_t)(zGAS_CONSTANT_ZERO);
            int64_t z3zE2159;
            z3zE2159 = (int64_t)(zclear_refund);
            z2zE2182 = (z3zE2158 - z3zE2159);
          }
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2182);
            if (have_exception) {  goto end_block_exception_2461;  }
          }
          z3zE2157 = UNIT;
        } else {  z3zE2157 = UNIT;  }
        bool z2zE2183;
        z2zE2183 = zword_is_zzero(znew);
        if (z2zE2183) {
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zclear_refund);
            if (have_exception) {  goto end_block_exception_2461;  }
          }
          z3zE2156 = UNIT;
        } else {  z3zE2156 = UNIT;  }
      } else {  z3zE2156 = UNIT;  }
      bool z2zE2184;
      z2zE2184 = eq_u256(zoriginal, znew);
      if (z2zE2184) {
        bool z2zE2185;
        z2zE2185 = zword_is_zzero(zoriginal);
        if (z2zE2185) {
          int64_t z2zE2186;
          {
            int64_t z3zE2162;
            z3zE2162 = (int64_t)(zG_sset);
            int64_t z3zE2163;
            z3zE2163 = (int64_t)(zG_warm_access);
            z2zE2186 = (z3zE2162 - z3zE2163);
          }
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2186);
            if (have_exception) {  goto end_block_exception_2461;  }
          }
          z3zE2155 = UNIT;
        } else {
          int64_t z2zE2187;
          {
            int64_t z3zE2160;
            z3zE2160 = (int64_t)(zG_sreset);
            int64_t z3zE2161;
            z3zE2161 = (int64_t)(zG_warm_access);
            z2zE2187 = (z3zE2160 - z3zE2161);
          }
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2187);
            if (have_exception) {  goto end_block_exception_2461;  }
          }
          z3zE2155 = UNIT;
        }
      } else {  z3zE2155 = UNIT;  }
    }
  } else {  z3zE2155 = UNIT;  }
  uint64_t z2zE2188;
  {    z2zE2188 = (zbase + zcold_cost);
  }
  struct zSstoreCosts z3zE2165;
  z3zE2165.zexecution = z2zE2188;
  z3zE2165.zrefund = zrefund;
  z3zE2165.zstate_charge = UINT64_C(0);
  z3zE2165.zstate_credit = UINT64_C(0);
  z8zE343 = z3zE2165;
end_function_2460: ;
  return z8zE343;
end_block_exception_2461: ;
  struct zSstoreCosts z8zE836 = { .zexecution = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_charge = UINT64_C(0xdeadc0de), .zstate_credit = UINT64_C(0xdeadc0de) };
  return z8zE836;
}

struct zSstoreCosts zamsterdam_sstore_costs(sail_u256 zoriginal, sail_u256 zcurrent, sail_u256 znew, bool zcold)
{
  struct zSstoreCosts z8zE344;
  bool zchanged;
  zchanged = (!eq_u256(zcurrent, znew));
  bool zclean_change;
  {
    bool z2zE2173;
    z2zE2173 = eq_u256(zoriginal, zcurrent);
    bool z3zE2143;
    if (z2zE2173) {  z3zE2143 = zchanged;  } else {  z3zE2143 = false;  }
    zclean_change = z3zE2143;
  }
  uint64_t zaccess;
  zaccess = zamsterdam_storage_access_cost(zcold);
  uint64_t zexecution;
  if (zclean_change) {
    {    zexecution = (zaccess + zG_amsterdam_storage_write);
    }
  } else {  zexecution = zaccess;  }
  __int128 zrefund;
  zrefund = zGAS_REFUND_ZERO;
  uint64_t zstate_charge;
  {
    bool z2zE2172;
    {
      bool z3zE2144;
      if (zclean_change) {  z3zE2144 = zword_is_zzero(zoriginal);  } else {  z3zE2144 = false;  }
      z2zE2172 = z3zE2144;
    }
    if (z2zE2172) {  zstate_charge = zG_amsterdam_state_storage_set;  } else {  zstate_charge = zGAS_COST_ZERO;  }
  }
  uint64_t zstate_credit;
  zstate_credit = UINT64_C(0);
  unit z3zE2145;
  if (zchanged) {
    bool z2zE2165;
    {
      bool z2zE2164;
      {
        bool z2zE2161;
        z2zE2161 = zword_is_zzero(zoriginal);
        z2zE2164 = not(z2zE2161);
      }
      bool z3zE2147;
      if (z2zE2164) {
        bool z2zE2163;
        {
          bool z2zE2162;
          z2zE2162 = zword_is_zzero(zcurrent);
          z2zE2163 = not(z2zE2162);
        }
        bool z3zE2146;
        if (z2zE2163) {  z3zE2146 = zword_is_zzero(znew);  } else {  z3zE2146 = false;  }
        z3zE2147 = z3zE2146;
      } else {  z3zE2147 = false;  }
      z2zE2165 = z3zE2147;
    }
    unit z3zE2152;
    if (z2zE2165) {
      {
        zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zR_amsterdam_storage_clear);
        if (have_exception) {  goto end_block_exception_2459;  }
      }
      z3zE2152 = UNIT;
    } else {  z3zE2152 = UNIT;  }
    bool z2zE2168;
    {
      bool z2zE2167;
      {
        bool z2zE2166;
        z2zE2166 = zword_is_zzero(zoriginal);
        z2zE2167 = not(z2zE2166);
      }
      bool z3zE2148;
      if (z2zE2167) {  z3zE2148 = zword_is_zzero(zcurrent);  } else {  z3zE2148 = false;  }
      z2zE2168 = z3zE2148;
    }
    unit z3zE2149;
    if (z2zE2168) {
      int64_t z2zE2169;
      {
        int64_t z3zE2150;
        z3zE2150 = (int64_t)(zGAS_CONSTANT_ZERO);
        int64_t z3zE2151;
        z3zE2151 = (int64_t)(zR_amsterdam_storage_clear);
        z2zE2169 = (z3zE2150 - z3zE2151);
      }
      {
        zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2169);
        if (have_exception) {  goto end_block_exception_2459;  }
      }
      z3zE2149 = UNIT;
    } else {  z3zE2149 = UNIT;  }
    bool z2zE2170;
    z2zE2170 = eq_u256(zoriginal, znew);
    if (z2zE2170) {
      {
        zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zG_amsterdam_storage_write);
        if (have_exception) {  goto end_block_exception_2459;  }
      }
      unit z3zE2153;
      z3zE2153 = UNIT;
      bool z2zE2171;
      z2zE2171 = zword_is_zzero(zoriginal);
      if (z2zE2171) {
        zstate_credit = zG_amsterdam_state_storage_set;
        z3zE2145 = UNIT;
      } else {  z3zE2145 = UNIT;  }
    } else {  z3zE2145 = UNIT;  }
  } else {  z3zE2145 = UNIT;  }
  struct zSstoreCosts z3zE2154;
  z3zE2154.zexecution = zexecution;
  z3zE2154.zrefund = zrefund;
  z3zE2154.zstate_charge = zstate_charge;
  z3zE2154.zstate_credit = zstate_credit;
  z8zE344 = z3zE2154;
end_function_2458: ;
  return z8zE344;
end_block_exception_2459: ;
  struct zSstoreCosts z8zE837 = { .zexecution = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_charge = UINT64_C(0xdeadc0de), .zstate_credit = UINT64_C(0xdeadc0de) };
  return z8zE837;
}

struct zSstoreCosts zsstore_costs(sail_u256 zoriginal, sail_u256 zcurrent, sail_u256 znew, bool zcold)
{
  struct zSstoreCosts z8zE345;
  bool z2zE2160;
  z2zE2160 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2160) {
    {
      z8zE345 = zamsterdam_sstore_costs(zoriginal, zcurrent, znew, zcold);
      if (have_exception) {  goto end_block_exception_2457;  }
    }
  } else {
    {
      z8zE345 = zlegacy_sstore_costs(zoriginal, zcurrent, znew, zcold);
      if (have_exception) {  goto end_block_exception_2457;  }
    }
  }
end_function_2456: ;
  return z8zE345;
end_block_exception_2457: ;
  struct zSstoreCosts z8zE838 = { .zexecution = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_charge = UINT64_C(0xdeadc0de), .zstate_credit = UINT64_C(0xdeadc0de) };
  return z8zE838;
}

unit zcharge_word_scaled_gas(uint64_t zper_unit, sail_u256 zunits)
{
  unit z8zE346;
  bool z2zE2157;
  {
    bool z2zE2156;
    {
      bool z2zE2154;
      z2zE2154 = zis_running(UNIT);
      z2zE2156 = not(z2zE2154);
    }
    bool z3zE2141;
    if (z2zE2156) {  z3zE2141 = true;  } else {
      bool z2zE2155;
      z2zE2155 = (zper_unit == UINT64_C(0));
      bool z3zE2140;
      if (z2zE2155) {  z3zE2140 = true;  } else {  z3zE2140 = u256_eq_u64(zunits, UINT64_C(0));  }
      z3zE2141 = z3zE2140;
    }
    z2zE2157 = z3zE2141;
  }
  unit z3zE2142;
  if (z2zE2157) {
    z8zE346 = UNIT;
    goto cleanup_2453;
    /* unreachable after return */
    goto end_cleanup_2454;
  cleanup_2453: ;
    goto end_function_2452;
  end_cleanup_2454: ;
  } else {  z3zE2142 = UNIT;  }
  uint64_t zremaining;
  zremaining = zgas_remaining;
  bool z2zE2158;
  z2zE2158 = (!u64_lt_u256(zremaining, zunits));
  if (z2zE2158) {
    uint64_t zaffordable_units;
    zaffordable_units = u256_to_u64(zunits);
    sail_u128 zexact_cost;
    zexact_cost = u128_mul_u64_u64(zper_unit, zaffordable_units);
    bool z2zE2159;
    z2zE2159 = (!u64_lt_u128(zremaining, zexact_cost));
    if (z2zE2159) {
      uint64_t zcost;
      zcost = u128_to_u64(zexact_cost);
      {
        z8zE346 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcost);
        if (have_exception) {  goto end_block_exception_2455;  }
      }
    } else {
      {
        z8zE346 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2455;  }
      }
    }
  } else {
    {
      z8zE346 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2455;  }
    }
  }
end_function_2452: ;
  return z8zE346;
end_block_exception_2455: ;

  return UNIT;
}

unit zcharge_memory_word_gas(uint64_t zbase, uint64_t zper_word, sail_u256 zsizze)
{
  unit z8zE347;
  unit z3zE2139;
  {
    z3zE2139 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase);
    if (have_exception) {  goto end_block_exception_2451;  }
  }
  sail_u256 z2zE2153;
  z2zE2153 = zmemory_word_count_word(zsizze);
  {
    z8zE347 = zcharge_word_scaled_gas(zper_word, z2zE2153);
    if (have_exception) {  goto end_block_exception_2451;  }
  }
end_function_2450: ;
  return z8zE347;
end_block_exception_2451: ;

  return UNIT;
}

unit zcharge_keccak_gas(sail_u256 zsizze)
{
  unit z8zE348;
  {
    z8zE348 = zcharge_memory_word_gas(zG_keccak, zG_keccak_word, zsizze);
    if (have_exception) {  goto end_block_exception_2449;  }
  }
end_function_2448: ;
  return z8zE348;
end_block_exception_2449: ;

  return UNIT;
}

unit zcharge_copy_gas(sail_u256 zsizze)
{
  unit z8zE349;
  {
    z8zE349 = zcharge_memory_word_gas(zGAS_CONSTANT_ZERO, zG_copy_word, zsizze);
    if (have_exception) {  goto end_block_exception_2447;  }
  }
end_function_2446: ;
  return z8zE349;
end_block_exception_2447: ;

  return UNIT;
}

unit zcharge_log_gas(uint64_t znum_topics, sail_u256 zsizze)
{
  unit z8zE350;
  unit z3zE2137;
  {
    z3zE2137 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_log);
    if (have_exception) {  goto end_block_exception_2445;  }
  }
  uint64_t ztopic_cost;
  {    ztopic_cost = (zG_logtopic * znum_topics);
  }
  unit z3zE2138;
  {
    z3zE2138 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(ztopic_cost);
    if (have_exception) {  goto end_block_exception_2445;  }
  }
  {
    z8zE350 = zcharge_word_scaled_gas(zG_logdata, zsizze);
    if (have_exception) {  goto end_block_exception_2445;  }
  }
end_function_2444: ;
  return z8zE350;
end_block_exception_2445: ;

  return UNIT;
}

uint64_t zexp_gas(sail_u256 zexponent)
{
  uint64_t z8zE351;
  uint64_t z2zE2152;
  {
    uint64_t z2zE2151;
    z2zE2151 = zword_byte_length(zexponent);
    {    z2zE2152 = (zG_expbyte * z2zE2151);
    }
  }
  {    z8zE351 = (z2zE2152 + zG_exp);
  }
end_function_2442: ;
  return z8zE351;
end_block_exception_2443: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_initcode_gas(uint64_t zbyte_len)
{
  uint64_t z8zE352;
  bool z2zE2150;
  z2zE2150 = zfork_gteq(zk_fork, zShanghai);
  if (z2zE2150) {
    uint64_t zwords;
    zwords = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbyte_len);
    {    z8zE352 = (zwords + zwords);
    }
  } else {  z8zE352 = UINT64_C(0);  }
end_function_2440: ;
  return z8zE352;
end_block_exception_2441: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zcall_gas_cap_word(uint64_t zavailable, sail_u256 zrequested)
{
  uint64_t z8zE353;
  uint64_t zretained;
  zretained = (zavailable / UINT64_C(64));
  uint64_t zall_but_64th;
  {
    zall_but_64th = zgas_sub_or_oog(zavailable, zretained);
    if (have_exception) {  goto end_block_exception_2439;  }
  }
  bool z2zE2149;
  z2zE2149 = u256_lt_u64(zrequested, zall_but_64th);
  if (z2zE2149) {  z8zE353 = u256_to_u64(zrequested);  } else {  z8zE353 = zall_but_64th;  }
end_function_2438: ;
  return z8zE353;
end_block_exception_2439: ;

  return UINT64_C(0xdeadc0de);
}

