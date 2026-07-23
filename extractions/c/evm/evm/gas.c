/* Generated from sail/evm/gas.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_100(void) {

  sail_u256 z3zE133;
  z3zE133 = zWORD_ONE;
  zMIN_BLOB_BASE_FEE = z3zE133;
let_end_448: ;
}
void kill_letbind_100(void) {
}

void create_letbind_101(void) {

  uint64_t z3zE134;
  z3zE134 = UINT64_C(131072);
  zGAS_PER_BLOB = z3zE134;
let_end_449: ;
}
void kill_letbind_101(void) {
}

void create_letbind_102(void) {

  sail_u256 z3zE135;
  z3zE135 = ((sail_u256){{UINT64_C(0), UINT64_C(0), UINT64_C(0), UINT64_C(1152921504606846976)}});
  zBLOB_RESERVE_PRICE_SHIFT_LIMIT = z3zE135;
let_end_450: ;
}
void kill_letbind_102(void) {
}

uint64_t zfake_exponential_index_increment(uint64_t zvalue)
{
  uint64_t z8zE334;
  bool z2zE2833;
  z2zE2833 = (zvalue < UINT64_C(18446744073709551615));
  if (z2zE2833) {
    {    z8zE334 = (zvalue + UINT64_C(1));
    }
  } else {
    struct zexception z2zE2834;
    CREATE(zexception)(&z2zE2834);
    zInvalidBlock(&z2zE2834, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2834);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:67.8-67.44");
    KILL(zexception)(&z2zE2834);
    goto end_block_exception_2919;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2834);
  }
end_function_2918: ;
  return z8zE334;
end_block_exception_2919: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zblob_word_add(sail_u256 zleft, sail_u256 zright)
{
  sail_u256 z8zE335;
  bool z2zE2831;
  {
    sail_u256 z2zE2830;
    z2zE2830 = zword_sub_word(zWORD_ALL_ONES, zleft);
    z2zE2831 = zword_ule(zright, z2zE2830);
  }
  if (z2zE2831) {  z8zE335 = zword_add_word(zleft, zright);  } else {
    struct zexception z2zE2832;
    CREATE(zexception)(&z2zE2832);
    zInvalidBlock(&z2zE2832, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2832);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:79.8-79.44");
    KILL(zexception)(&z2zE2832);
    goto end_block_exception_2917;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2832);
  }
end_function_2916: ;
  return z8zE335;
end_block_exception_2917: ;

  return u256_zero();
}

struct zScaledBlobValue zscaled_blob_add(struct zScaledBlobValue zleft, struct zScaledBlobValue zright, uint64_t zdenominator)
{
  struct zScaledBlobValue z8zE336;
  uint64_t zcombined;
  {
    uint64_t z2zE2824;
    z2zE2824 = zleft.zremainder;
    uint64_t z2zE2825;
    z2zE2825 = zright.zremainder;
    {    zcombined = (z2zE2824 + z2zE2825);
    }
  }
  bool zcarry;
  zcarry = (!(zcombined < zdenominator));
  uint64_t zremainder_value;
  if (zcarry) {
    {    zremainder_value = (zcombined - zdenominator);
    }
  } else {  zremainder_value = zcombined;  }
  bool z2zE2820;
  z2zE2820 = (zremainder_value < zdenominator);
  if (z2zE2820) {
    uint64_t zremainder;
    zremainder = zremainder_value;
    sail_u256 zwhole;
    {
      sail_u256 z2zE2821;
      z2zE2821 = zleft.zwhole;
      sail_u256 z2zE2822;
      z2zE2822 = zright.zwhole;
      {
        zwhole = zblob_word_add(z2zE2821, z2zE2822);
        if (have_exception) {  goto end_block_exception_2913;  }
      }
    }
    sail_u256 zcarried_whole;
    if (zcarry) {
      {
        zcarried_whole = zblob_word_add(zwhole, zWORD_ONE);
        if (have_exception) {  goto end_block_exception_2913;  }
      }
    } else {  zcarried_whole = zwhole;  }
    struct zScaledBlobValue z3zE2927;
    z3zE2927.zremainder = zremainder;
    z3zE2927.zwhole = zcarried_whole;
    z8zE336 = z3zE2927;
  } else {
    struct zexception z2zE2823;
    CREATE(zexception)(&z2zE2823);
    zInvalidBlock(&z2zE2823, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2823);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:123.8-123.44");
    KILL(zexception)(&z2zE2823);
    goto end_block_exception_2913;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2823);
  }
end_function_2912: ;
  return z8zE336;
end_block_exception_2913: ;
  struct zScaledBlobValue z8zE898 = { .zremainder = UINT64_C(0xdeadc0de), .zwhole = u256_zero() };
  return z8zE898;
}

struct zBlobProductDivMod zblob_product_divmod(sail_u256 zvalue, uint64_t zfactor, sail_u256 zaddend, uint64_t zdenominator, uint64_t ziteration)
{
  struct zBlobProductDivMod z8zE337;
  sail_u128 zdivisor;
  {
    sail_u256 z3zE3859;
    z3zE3859 = u256_of_fbits(zdenominator);
    sail_u256 z3zE3860;
    z3zE3860 = u256_of_fbits(ziteration);
    sail_u256 z3zE3861;
    z3zE3861 = zword_mul_word(z3zE3859, z3zE3860);
    zdivisor = u128_of_u256(z3zE3861);
  }
  bool z2zE2816;
  z2zE2816 = zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdivisor);
  unit z3zE2925;
  if (z2zE2816) {
    struct zexception z2zE2817;
    CREATE(zexception)(&z2zE2817);
    zInvalidBlock(&z2zE2817, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2817);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:142.8-142.44");
    KILL(zexception)(&z2zE2817);
    goto end_block_exception_2911;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2817);
  } else {  z3zE2925 = UNIT;  }
  sail_u256 zvalue_quotient;
  zvalue_quotient = zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue, zdivisor);
  sail_u256 zvalue_remainder;
  zvalue_remainder = zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue, zdivisor);
  sail_u256 zwhole;
  {
    zwhole = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue_quotient, zfactor);
    if (have_exception) {  goto end_block_exception_2911;  }
  }
  sail_u256 zresidual_product;
  zresidual_product = zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue_remainder, zfactor);
  sail_u256 zresidual_sum;
  {
    zresidual_sum = zblob_word_add(zresidual_product, zaddend);
    if (have_exception) {  goto end_block_exception_2911;  }
  }
  sail_u256 zresidual_quotient;
  zresidual_quotient = zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zresidual_sum, zdivisor);
  sail_u256 z2zE2818;
  {
    z2zE2818 = zblob_word_add(zwhole, zresidual_quotient);
    if (have_exception) {  goto end_block_exception_2911;  }
  }
  sail_u256 z2zE2819;
  z2zE2819 = zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zresidual_sum, zdivisor);
  struct zBlobProductDivMod z3zE2926;
  z3zE2926.zquotient = z2zE2818;
  z3zE2926.zremainder = z2zE2819;
  z8zE337 = z3zE2926;
end_function_2910: ;
  return z8zE337;
end_block_exception_2911: ;
  struct zBlobProductDivMod z8zE899 = { .zquotient = u256_zero(), .zremainder = u256_zero() };
  return z8zE899;
}

struct zScaledBlobValue zscaled_blob_next(struct zScaledBlobValue zterm, uint64_t znumerator, uint64_t zdenominator, uint64_t ziteration)
{
  struct zScaledBlobValue z8zE338;
  sail_u128 zremainder_product;
  {
    uint64_t z2zE2815;
    z2zE2815 = zterm.zremainder;
    {
      sail_u256 z3zE3862;
      z3zE3862 = u256_of_fbits(z2zE2815);
      sail_u256 z3zE3863;
      z3zE3863 = u256_of_fbits(znumerator);
      sail_u256 z3zE3864;
      z3zE3864 = zword_mul_word(z3zE3862, z3zE3863);
      zremainder_product = u128_of_u256(z3zE3864);
    }
  }
  sail_u256 zaddend;
  zaddend = zword_div_wordzIreprzGR__sail_c_repr_u128zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zremainder_product, zdenominator);
  struct zBlobProductDivMod zdivided;
  {
    sail_u256 z2zE2814;
    z2zE2814 = zterm.zwhole;
    {
      zdivided = zblob_product_divmod(z2zE2814, znumerator, zaddend, zdenominator, ziteration);
      if (have_exception) {  goto end_block_exception_2909;  }
    }
  }
  sail_u256 zremainder_word;
  {
    sail_u256 z2zE2813;
    z2zE2813 = zdivided.zremainder;
    zremainder_word = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2813, ziteration);
  }
  bool z2zE2810;
  z2zE2810 = u256_lt_u64(zremainder_word, zdenominator);
  if (z2zE2810) {
    uint64_t zremainder;
    zremainder = u256_to_u64(zremainder_word);
    sail_u256 z2zE2811;
    z2zE2811 = zdivided.zquotient;
    struct zScaledBlobValue z3zE2924;
    z3zE2924.zremainder = zremainder;
    z3zE2924.zwhole = z2zE2811;
    z8zE338 = z3zE2924;
  } else {
    struct zexception z2zE2812;
    CREATE(zexception)(&z2zE2812);
    zInvalidBlock(&z2zE2812, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2812);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:170.8-170.44");
    KILL(zexception)(&z2zE2812);
    goto end_block_exception_2909;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2812);
  }
end_function_2908: ;
  return z8zE338;
end_block_exception_2909: ;
  struct zScaledBlobValue z8zE900 = { .zremainder = UINT64_C(0xdeadc0de), .zwhole = u256_zero() };
  return z8zE900;
}

sail_u256 zfake_exponential(sail_u256 zfactor, uint64_t znumerator, uint64_t zdenominator)
{
  sail_u256 z8zE339;
  uint64_t zi;
  zi = UINT64_C(1);
  struct zScaledBlobValue zoutput;
  {
    struct zScaledBlobValue z3zE2916;
    z3zE2916.zremainder = UINT64_C(0);
    z3zE2916.zwhole = zWORD_ZERO;
    zoutput = z3zE2916;
  }
  struct zScaledBlobValue zterm;
  {
    struct zScaledBlobValue z3zE2917;
    z3zE2917.zremainder = UINT64_C(0);
    z3zE2917.zwhole = zfactor;
    zterm = z3zE2917;
  }
  bool z3zE2921;
  unit z3zE2922;
while_2904: ;
  {
    bool z2zE2809;
    {
      sail_u256 z2zE2807;
      z2zE2807 = zterm.zwhole;
      z2zE2809 = zword_nonzzero(z2zE2807);
    }
    bool z3zE2918;
    if (z2zE2809) {  z3zE2918 = true;  } else {
      uint64_t z2zE2808;
      z2zE2808 = zterm.zremainder;
      z3zE2918 = (z2zE2808 != UINT64_C(0));
    }
    z3zE2921 = z3zE2918;
    if (!(z3zE2921)) goto wend_2905;
    {
      zoutput = zscaled_blob_add(zoutput, zterm, zdenominator);
      if (have_exception) {  goto end_block_exception_2907;  }
    }
    unit z3zE2920;
    z3zE2920 = UNIT;
    {
      zterm = zscaled_blob_next(zterm, znumerator, zdenominator, zi);
      if (have_exception) {  goto end_block_exception_2907;  }
    }
    unit z3zE2919;
    z3zE2919 = UNIT;
    {
      zi = zfake_exponential_index_increment(zi);
      if (have_exception) {  goto end_block_exception_2907;  }
    }
    z3zE2922 = UNIT;
    goto while_2904;
  }
wend_2905: ;
  unit z3zE2923;
  z3zE2923 = UNIT;
  z8zE339 = zoutput.zwhole;
end_function_2906: ;
  return z8zE339;
end_block_exception_2907: ;

  return u256_zero();
}

sail_u256 zblob_base_fee(uint64_t zexcess_blob_gas)
{
  sail_u256 z8zE340;
  uint64_t z2zE2806;
  z2zE2806 = zk_blob_schedule.zbase_fee_update_fraction;
  {
    z8zE340 = zfake_exponential(zMIN_BLOB_BASE_FEE, zexcess_blob_gas, z2zE2806);
    if (have_exception) {  goto end_block_exception_2903;  }
  }
end_function_2902: ;
  return z8zE340;
end_block_exception_2903: ;

  return u256_zero();
}

bool zblob_reserve_price_active(sail_u256 zbase_fee, sail_u256 zprice)
{
  bool z8zE341;
  bool z2zE2804;
  z2zE2804 = zword_ult(zprice, zBLOB_RESERVE_PRICE_SHIFT_LIMIT);
  if (z2zE2804) {
    sail_u256 z2zE2805;
    z2zE2805 = zword_shift_left(zprice, UINT64_C(4));
    z8zE341 = zword_ult(z2zE2805, zbase_fee);
  } else {  z8zE341 = false;  }
end_function_2900: ;
  return z8zE341;
end_block_exception_2901: ;

  return false;
}

uint64_t zblob_schedule_target(unit z3zE2915)
{
  uint64_t z8zE342;
  z8zE342 = zk_blob_schedule.ztarget;
end_function_2898: ;
  return z8zE342;
end_block_exception_2899: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_schedule_max(unit z3zE2914)
{
  uint64_t z8zE343;
  z8zE343 = zk_blob_schedule.zmax;
end_function_2896: ;
  return z8zE343;
end_block_exception_2897: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_gas_for_count(uint64_t zcount)
{
  uint64_t z8zE344;
  {    z8zE344 = (zGAS_PER_BLOB * zcount);
  }
end_function_2894: ;
  return z8zE344;
end_block_exception_2895: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_target_gas_per_block(unit z3zE2913)
{
  uint64_t z8zE345;
  uint64_t z2zE2803;
  z2zE2803 = zblob_schedule_target(UNIT);
  z8zE345 = zblob_gas_for_count(z2zE2803);
end_function_2892: ;
  return z8zE345;
end_block_exception_2893: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zblob_max_gas_per_block(unit z3zE2912)
{
  uint64_t z8zE346;
  uint64_t z2zE2802;
  z2zE2802 = zblob_schedule_max(UNIT);
  z8zE346 = zblob_gas_for_count(z2zE2802);
end_function_2890: ;
  return z8zE346;
end_block_exception_2891: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zchecked_block_blob_gas_add(uint64_t zaccumulated, uint64_t ztransaction)
{
  uint64_t z8zE347;
  uint64_t zmaximum;
  zmaximum = zblob_max_gas_per_block(UNIT);
  bool z2zE2797;
  z2zE2797 = (!(zmaximum < zaccumulated));
  if (z2zE2797) {
    bool z2zE2799;
    {
      uint64_t z2zE2798;
      {    z2zE2798 = (zmaximum - zaccumulated);
      }
      z2zE2799 = (!(z2zE2798 < ztransaction));
    }
    if (z2zE2799) {
      {    z8zE347 = (zaccumulated + ztransaction);
      }
    } else {
      struct zexception z2zE2800;
      CREATE(zexception)(&z2zE2800);
      zInvalidBlock(&z2zE2800, zBlobGasLimitExceeded);
      COPY(zexception)(current_exception, z2zE2800);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/gas.sail:227.12-227.52");
      KILL(zexception)(&z2zE2800);
      goto end_block_exception_2889;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2800);
    }
  } else {
    struct zexception z2zE2801;
    CREATE(zexception)(&z2zE2801);
    zInvalidBlock(&z2zE2801, zBlobGasLimitExceeded);
    COPY(zexception)(current_exception, z2zE2801);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:230.8-230.48");
    KILL(zexception)(&z2zE2801);
    goto end_block_exception_2889;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2801);
  }
end_function_2888: ;
  return z8zE347;
end_block_exception_2889: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t znext_excess_blob_gas(uint64_t zparent_excess_blob_gas, uint64_t zparent_blob_gas_used, sail_u256 zparent_base_fee_per_gas)
{
  uint64_t z8zE348;
  sail_u128 zparent_blob_gas;
  zparent_blob_gas = u128_add_u64_u64(zparent_excess_blob_gas, zparent_blob_gas_used);
  uint64_t ztarget_blob_gas;
  ztarget_blob_gas = zblob_target_gas_per_block(UNIT);
  bool z2zE2787;
  z2zE2787 = u128_lt_u64(zparent_blob_gas, ztarget_blob_gas);
  if (z2zE2787) {  z8zE348 = UINT64_C(0);  } else {
    sail_u256 zprice;
    {
      zprice = zblob_base_fee(zparent_excess_blob_gas);
      if (have_exception) {  goto end_block_exception_2887;  }
    }
    bool z2zE2789;
    {
      bool z2zE2788;
      z2zE2788 = zfork_gteq(zk_fork, zOsaka);
      bool z3zE2908;
      if (z2zE2788) {  z3zE2908 = zblob_reserve_price_active(zparent_base_fee_per_gas, zprice);  } else {
        z3zE2908 = false;
      }
      z2zE2789 = z3zE2908;
    }
    if (z2zE2789) {
      uint64_t ztarget;
      ztarget = zblob_schedule_target(UNIT);
      uint64_t zmaximum;
      zmaximum = zblob_schedule_max(UNIT);
      bool z2zE2791;
      {
        bool z2zE2790;
        z2zE2790 = (zmaximum == UINT64_C(0));
        bool z3zE2909;
        if (z2zE2790) {  z3zE2909 = true;  } else {  z3zE2909 = (zmaximum < ztarget);  }
        z2zE2791 = z3zE2909;
      }
      unit z3zE2910;
      if (z2zE2791) {
        struct zexception z2zE2792;
        CREATE(zexception)(&z2zE2792);
        zInvalidBlock(&z2zE2792, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE2792);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/gas.sail:255.16-255.49");
        KILL(zexception)(&z2zE2792);
        goto end_block_exception_2887;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2792);
      } else {  z3zE2910 = UNIT;  }
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
      bool z2zE2793;
      z2zE2793 = (!u64_lt_u128(UINT64_C(18446744073709551615), znext));
      if (z2zE2793) {  z8zE348 = u128_to_u64(znext);  } else {
        struct zexception z2zE2794;
        CREATE(zexception)(&z2zE2794);
        zInvalidBlock(&z2zE2794, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE2794);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/gas.sail:264.16-264.49");
        KILL(zexception)(&z2zE2794);
        goto end_block_exception_2887;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2794);
      }
    } else {
      sail_u128 z3zE2911;
      z3zE2911 = u128_sub_u64(zparent_blob_gas, ztarget_blob_gas);
      bool z2zE2795;
      z2zE2795 = (!u64_lt_u128(UINT64_C(18446744073709551615), z3zE2911));
      if (z2zE2795) {  z8zE348 = u128_to_u64(z3zE2911);  } else {
        struct zexception z2zE2796;
        CREATE(zexception)(&z2zE2796);
        zInvalidBlock(&z2zE2796, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE2796);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/gas.sail:271.16-271.49");
        KILL(zexception)(&z2zE2796);
        goto end_block_exception_2887;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2796);
      }
    }
  }
end_function_2886: ;
  return z8zE348;
end_block_exception_2887: ;

  return UINT64_C(0xdeadc0de);
}

void create_letbind_103(void) {

  uint64_t z3zE136;
  z3zE136 = UINT64_C(0);
  zG_zzero = z3zE136;
let_end_467: ;
}
void kill_letbind_103(void) {
}

void create_letbind_104(void) {

  uint64_t z3zE137;
  z3zE137 = UINT64_C(1);
  zG_jumpdest = z3zE137;
let_end_468: ;
}
void kill_letbind_104(void) {
}

void create_letbind_105(void) {

  uint64_t z3zE138;
  z3zE138 = UINT64_C(2);
  zG_base = z3zE138;
let_end_469: ;
}
void kill_letbind_105(void) {
}

void create_letbind_106(void) {

  uint64_t z3zE139;
  z3zE139 = UINT64_C(3);
  zG_verylow = z3zE139;
let_end_470: ;
}
void kill_letbind_106(void) {
}

void create_letbind_107(void) {

  uint64_t z3zE140;
  z3zE140 = UINT64_C(5);
  zG_low = z3zE140;
let_end_471: ;
}
void kill_letbind_107(void) {
}

void create_letbind_108(void) {

  uint64_t z3zE141;
  z3zE141 = UINT64_C(8);
  zG_mid = z3zE141;
let_end_472: ;
}
void kill_letbind_108(void) {
}

void create_letbind_109(void) {

  uint64_t z3zE142;
  z3zE142 = UINT64_C(10);
  zG_high = z3zE142;
let_end_473: ;
}
void kill_letbind_109(void) {
}

void create_letbind_110(void) {

  uint64_t z3zE143;
  z3zE143 = UINT64_C(100);
  zG_warm_access = z3zE143;
let_end_474: ;
}
void kill_letbind_110(void) {
}

void create_letbind_111(void) {

  uint64_t z3zE144;
  z3zE144 = UINT64_C(2100);
  zG_cold_sload = z3zE144;
let_end_475: ;
}
void kill_letbind_111(void) {
}

void create_letbind_112(void) {

  uint64_t z3zE145;
  z3zE145 = UINT64_C(2600);
  zG_cold_account = z3zE145;
let_end_476: ;
}
void kill_letbind_112(void) {
}

void create_letbind_113(void) {

  uint64_t z3zE146;
  z3zE146 = UINT64_C(30);
  zG_keccak = z3zE146;
let_end_477: ;
}
void kill_letbind_113(void) {
}

void create_letbind_114(void) {

  uint64_t z3zE147;
  z3zE147 = UINT64_C(6);
  zG_keccak_word = z3zE147;
let_end_478: ;
}
void kill_letbind_114(void) {
}

void create_letbind_115(void) {

  uint64_t z3zE148;
  z3zE148 = UINT64_C(3);
  zG_copy_word = z3zE148;
let_end_479: ;
}
void kill_letbind_115(void) {
}

void create_letbind_116(void) {

  uint64_t z3zE149;
  z3zE149 = UINT64_C(3);
  zG_memory = z3zE149;
let_end_480: ;
}
void kill_letbind_116(void) {
}

void create_letbind_117(void) {

  uint64_t z3zE150;
  z3zE150 = UINT64_C(375);
  zG_log = z3zE150;
let_end_481: ;
}
void kill_letbind_117(void) {
}

void create_letbind_118(void) {

  uint64_t z3zE151;
  z3zE151 = UINT64_C(375);
  zG_logtopic = z3zE151;
let_end_482: ;
}
void kill_letbind_118(void) {
}

void create_letbind_119(void) {

  uint64_t z3zE152;
  z3zE152 = UINT64_C(8);
  zG_logdata = z3zE152;
let_end_483: ;
}
void kill_letbind_119(void) {
}

void create_letbind_120(void) {

  uint64_t z3zE153;
  z3zE153 = UINT64_C(10);
  zG_exp = z3zE153;
let_end_484: ;
}
void kill_letbind_120(void) {
}

void create_letbind_121(void) {

  uint64_t z3zE154;
  z3zE154 = UINT64_C(50);
  zG_expbyte = z3zE154;
let_end_485: ;
}
void kill_letbind_121(void) {
}

void create_letbind_122(void) {

  uint64_t z3zE155;
  z3zE155 = UINT64_C(20000);
  zG_sset = z3zE155;
let_end_486: ;
}
void kill_letbind_122(void) {
}

void create_letbind_123(void) {

  uint64_t z3zE156;
  z3zE156 = UINT64_C(2900);
  zG_sreset = z3zE156;
let_end_487: ;
}
void kill_letbind_123(void) {
}

void create_letbind_124(void) {

  uint64_t z3zE157;
  z3zE157 = UINT64_C(15000);
  zR_sclear_pre_london = z3zE157;
let_end_488: ;
}
void kill_letbind_124(void) {
}

void create_letbind_125(void) {

  uint64_t z3zE158;
  z3zE158 = UINT64_C(4800);
  zR_sclear_london = z3zE158;
let_end_489: ;
}
void kill_letbind_125(void) {
}

void create_letbind_126(void) {

  uint64_t z3zE159;
  z3zE159 = UINT64_C(24000);
  zR_selfdestruct_pre_london = z3zE159;
let_end_490: ;
}
void kill_letbind_126(void) {
}

void create_letbind_127(void) {

  uint64_t z3zE160;
  z3zE160 = UINT64_C(32000);
  zG_create = z3zE160;
let_end_491: ;
}
void kill_letbind_127(void) {
}

void create_letbind_128(void) {

  uint64_t z3zE161;
  z3zE161 = UINT64_C(200);
  zG_codedeposit = z3zE161;
let_end_492: ;
}
void kill_letbind_128(void) {
}

void create_letbind_129(void) {

  uint64_t z3zE162;
  z3zE162 = UINT64_C(9000);
  zG_callvalue = z3zE162;
let_end_493: ;
}
void kill_letbind_129(void) {
}

void create_letbind_130(void) {

  uint64_t z3zE163;
  z3zE163 = UINT64_C(2300);
  zG_callstipend = z3zE163;
let_end_494: ;
}
void kill_letbind_130(void) {
}

void create_letbind_131(void) {

  uint64_t z3zE164;
  z3zE164 = UINT64_C(25000);
  zG_newaccount = z3zE164;
let_end_495: ;
}
void kill_letbind_131(void) {
}

void create_letbind_132(void) {

  uint64_t z3zE165;
  z3zE165 = UINT64_C(5000);
  zG_selfdestruct = z3zE165;
let_end_496: ;
}
void kill_letbind_132(void) {
}

void create_letbind_133(void) {

  uint64_t z3zE166;
  z3zE166 = UINT64_C(2);
  zG_initcode_word = z3zE166;
let_end_497: ;
}
void kill_letbind_133(void) {
}

void create_letbind_134(void) {

  uint64_t z3zE167;
  z3zE167 = UINT64_C(3000);
  zG_amsterdam_cold_account_access = z3zE167;
let_end_498: ;
}
void kill_letbind_134(void) {
}

void create_letbind_135(void) {

  uint64_t z3zE168;
  z3zE168 = UINT64_C(3000);
  zG_amsterdam_cold_storage_access = z3zE168;
let_end_499: ;
}
void kill_letbind_135(void) {
}

void create_letbind_136(void) {

  uint64_t z3zE169;
  z3zE169 = UINT64_C(10000);
  zG_amsterdam_storage_write = z3zE169;
let_end_500: ;
}
void kill_letbind_136(void) {
}

void create_letbind_137(void) {

  uint64_t z3zE170;
  z3zE170 = UINT64_C(8000);
  zG_amsterdam_account_write = z3zE170;
let_end_501: ;
}
void kill_letbind_137(void) {
}

void create_letbind_138(void) {

  uint64_t z3zE171;
  z3zE171 = UINT64_C(10300);
  zG_amsterdam_call_value = z3zE171;
let_end_502: ;
}
void kill_letbind_138(void) {
}

void create_letbind_139(void) {

  uint64_t z3zE172;
  z3zE172 = UINT64_C(11000);
  zG_amsterdam_create_access = z3zE172;
let_end_503: ;
}
void kill_letbind_139(void) {
}

void create_letbind_140(void) {

  uint64_t z3zE173;
  z3zE173 = UINT64_C(1530);
  zG_amsterdam_state_byte = z3zE173;
let_end_504: ;
}
void kill_letbind_140(void) {
}

void create_letbind_141(void) {

  uint64_t z3zE174;
  z3zE174 = UINT64_C(97920);
  zG_amsterdam_state_storage_set = z3zE174;
let_end_505: ;
}
void kill_letbind_141(void) {
}

void create_letbind_142(void) {

  uint64_t z3zE175;
  z3zE175 = UINT64_C(183600);
  zG_amsterdam_state_new_account = z3zE175;
let_end_506: ;
}
void kill_letbind_142(void) {
}

void create_letbind_143(void) {

  uint64_t z3zE176;
  z3zE176 = UINT64_C(35190);
  zG_amsterdam_state_auth_base = z3zE176;
let_end_507: ;
}
void kill_letbind_143(void) {
}

void create_letbind_144(void) {

  uint64_t z3zE177;
  z3zE177 = UINT64_C(12480);
  zR_amsterdam_storage_clear = z3zE177;
let_end_508: ;
}
void kill_letbind_144(void) {
}

void create_letbind_145(void) {

  uint64_t z3zE178;
  z3zE178 = UINT64_C(2301);
  zG_sstore_sentry = z3zE178;
let_end_509: ;
}
void kill_letbind_145(void) {
}

void create_letbind_146(void) {

  uint64_t z3zE179;
  z3zE179 = UINT64_C(24576);
  zEIP170_DEPLOYED_CODE_SIZE_LIMIT = z3zE179;
let_end_510: ;
}
void kill_letbind_146(void) {
}

void create_letbind_147(void) {

  uint64_t z3zE180;
  z3zE180 = UINT64_C(49152);
  zEIP3860_INITCODE_SIZE_LIMIT = z3zE180;
let_end_511: ;
}
void kill_letbind_147(void) {
}

void create_letbind_148(void) {

  uint64_t z3zE181;
  z3zE181 = UINT64_C(65536);
  zEIP7954_DEPLOYED_CODE_SIZE_LIMIT = z3zE181;
let_end_512: ;
}
void kill_letbind_148(void) {
}

void create_letbind_149(void) {

  uint64_t z3zE182;
  z3zE182 = UINT64_C(131072);
  zEIP7954_INITCODE_SIZE_LIMIT = z3zE182;
let_end_513: ;
}
void kill_letbind_149(void) {
}

uint64_t zsstore_clear_refund(unit z3zE2907)
{
  uint64_t z8zE349;
  bool z2zE2782;
  z2zE2782 = zfork_gteq(zk_fork, zLondon);
  if (z2zE2782) {  z8zE349 = zR_sclear_london;  } else {  z8zE349 = zR_sclear_pre_london;  }
end_function_2880: ;
  return z8zE349;
end_block_exception_2881: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zstate_gas_spill_room(uint64_t zleft)
{
  uint64_t z8zE350;
  {    z8zE350 = (UINT64_C(16777216) - zleft);
  }
end_function_2874: ;
  return z8zE350;
end_block_exception_2875: ;

  return UINT64_C(0xdeadc0de);
}

unit zcredit_state_gas_refund(uint64_t zamount)
{
  unit z8zE351;
  uint64_t zspilled;
  zspilled = zstate_gas_spilled;
  bool z2zE2762;
  z2zE2762 = (!(zspilled < zamount));
  if (z2zE2762) {
    bool z2zE2763;
    z2zE2763 = (zamount != UINT64_C(0));
    if (z2zE2763) {
      {
        zgas_remaining = zconserved_gas_add(zgas_remaining, zamount);
        if (have_exception) {  goto end_block_exception_2863;  }
      }
      unit z3zE2898;
      z3zE2898 = UNIT;
      {    zstate_gas_spilled = (zspilled - zamount);
      }
      z8zE351 = UNIT;
    } else {  z8zE351 = UNIT;  }
  } else {
    bool z2zE2764;
    z2zE2764 = (zspilled != UINT64_C(0));
    unit z3zE2896;
    if (z2zE2764) {
      {
        zgas_remaining = zconserved_gas_add(zgas_remaining, zspilled);
        if (have_exception) {  goto end_block_exception_2863;  }
      }
      unit z3zE2897;
      z3zE2897 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      z3zE2896 = UNIT;
    } else {  z3zE2896 = UNIT;  }
    uint64_t zto_state;
    {    zto_state = (zamount - zspilled);
    }
    {
      zstate_gas_remaining = zconserved_gas_add(zstate_gas_remaining, zto_state);
      if (have_exception) {  goto end_block_exception_2863;  }
    }
    z8zE351 = UNIT;
  }
end_function_2862: ;
  return z8zE351;
end_block_exception_2863: ;

  return UNIT;
}

unit zreturn_child_state_gas(uint64_t zchild_remaining, uint64_t zchild_spilled)
{
  unit z8zE352;
  {
    zstate_gas_remaining = zconserved_gas_add(zstate_gas_remaining, zchild_remaining);
    if (have_exception) {  goto end_block_exception_2861;  }
  }
  unit z3zE2895;
  z3zE2895 = UNIT;
  {
    zstate_gas_spilled = zstate_gas_spill_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zstate_gas_spilled, zchild_spilled);
    if (have_exception) {  goto end_block_exception_2861;  }
  }
  z8zE352 = UNIT;
end_function_2860: ;
  return z8zE352;
end_block_exception_2861: ;

  return UNIT;
}

uint64_t zgas_sub_or_oog(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE353;
  bool z2zE2761;
  z2zE2761 = (!(zleft < zright));
  if (z2zE2761) {
    {    z8zE353 = (zleft - zright);
    }
  } else {
    unit z3zE2894;
    {
      z3zE2894 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2859;  }
    }
    z8zE353 = zGAS_ZERO;
  }
end_function_2858: ;
  return z8zE353;
end_block_exception_2859: ;

  return UINT64_C(0xdeadc0de);
}

unit zrefund_gas(uint64_t zamount)
{
  unit z8zE354;
  {
    zgas_remaining = zconserved_gas_add(zgas_remaining, zamount);
    if (have_exception) {  goto end_block_exception_2857;  }
  }
  z8zE354 = UNIT;
end_function_2856: ;
  return z8zE354;
end_block_exception_2857: ;

  return UNIT;
}

sail_u256 zmemory_word_count_word(sail_u256 zbyte_len)
{
  sail_u256 z8zE355;
  sail_u256 zquotient;
  zquotient = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zbyte_len, UINT64_C(32));
  bool z2zE2759;
  {
    sail_u256 z2zE2758;
    z2zE2758 = zword_mod_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zbyte_len, UINT64_C(32));
    z2zE2759 = eq_u256(z2zE2758, zWORD_ZERO);
  }
  if (z2zE2759) {  z8zE355 = zquotient;  } else {  z8zE355 = zword_add_word(zquotient, zWORD_ONE);  }
end_function_2852: ;
  return z8zE355;
end_block_exception_2853: ;

  return u256_zero();
}

sail_u128 zbounded_mem_cost(uint64_t zwords)
{
  sail_u128 z8zE356;
  sail_u128 z2zE2756;
  z2zE2756 = u128_mul_u64_u64(zG_memory, zwords);
  sail_u128 z2zE2757;
  {
    sail_u128 z2zE2755;
    z2zE2755 = u128_mul_u64_u64(zwords, zwords);
    z2zE2757 = u128_div_u64(z2zE2755, UINT64_C(512));
  }
  z8zE356 = u128_add(z2zE2756, z2zE2757);
end_function_2850: ;
  return z8zE356;
end_block_exception_2851: ;

  return u128_zero();
}

uint64_t zmemory_expansion_cost(uint64_t zrequired_sizze, uint64_t zavailable)
{
  uint64_t z8zE357;
  uint64_t znew_words;
  znew_words = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired_sizze);
  uint64_t zold_words;
  {
    uint64_t z2zE2754;
    z2zE2754 = zevm_memory_high_water(UNIT);
    zold_words = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2754);
  }
  bool z2zE2750;
  z2zE2750 = (!(zold_words < znew_words));
  if (z2zE2750) {  z8zE357 = UINT64_C(0);  } else {
    sail_u128 zold_cost;
    zold_cost = zbounded_mem_cost(zold_words);
    sail_u128 znew_cost;
    znew_cost = zbounded_mem_cost(znew_words);
    bool z2zE2753;
    {
      bool z2zE2752;
      z2zE2752 = (!u128_lt(znew_cost, zold_cost));
      bool z3zE2891;
      if (z2zE2752) {
        sail_u128 z2zE2751;
        z2zE2751 = u128_add_u64(zold_cost, zavailable);
        z3zE2891 = (!u128_lt(z2zE2751, znew_cost));
      } else {  z3zE2891 = false;  }
      z2zE2753 = z3zE2891;
    }
    if (z2zE2753) {
      uint64_t zcost;
      {
        uint64_t z3zE2893;
        z3zE2893 = u128_to_u64(u128_sub(znew_cost, zold_cost));
        zcost = z3zE2893;
      }
      z8zE357 = zcost;
    } else {
      unit z3zE2892;
      {
        z3zE2892 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2849;  }
      }
      z8zE357 = UINT64_C(0);
    }
  }
end_function_2848: ;
  return z8zE357;
end_block_exception_2849: ;

  return UINT64_C(0xdeadc0de);
}

struct zMemoryExpansion zmemory_expansion(sail_u256 zstart, sail_u256 zsizze, uint64_t zavailable)
{
  struct zMemoryExpansion z8zE358;
  bool z2zE2743;
  z2zE2743 = u256_eq_u64(zsizze, UINT64_C(0));
  if (z2zE2743) {
    struct zMemoryExpansion z3zE2890;
    z3zE2890.zcost = UINT64_C(0);
    z3zE2890.zrange = zEMPTY_MEMORY_RANGE;
    z3zE2890.zrequired_sizze = UINT64_C(0);
    z8zE358 = z3zE2890;
  } else {
    bool z2zE2745;
    {
      bool z2zE2744;
      z2zE2744 = (!u64_lt_u256(UINT64_C(18446744073709551615), zstart));
      bool z3zE2876;
      if (z2zE2744) {  z3zE2876 = (!u64_lt_u256(UINT64_C(18446744073709551615), zsizze));  } else {
        z3zE2876 = false;
      }
      z2zE2745 = z3zE2876;
    }
    if (z2zE2745) {
      struct zMemoryExpansion z3zE2879;
      {
        uint64_t zbounded_start;
        zbounded_start = u256_to_u64(zstart);
        struct zMemoryExpansion z3zE2880;
        {
          uint64_t zbounded_sizze;
          zbounded_sizze = u256_to_u64(zsizze);
          sail_u128 zrequired;
          zrequired = u128_add_u64_u64(zbounded_start, zbounded_sizze);
          uint64_t z2zE2746;
          z2zE2746 = zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
          struct zMemoryExpansion z3zE2881;
          {
            uint64_t zwords;
            zwords = z2zE2746;
            struct zMemoryExpansion z3zE2882;
            {
              uint64_t zbounded_words;
              zbounded_words = zwords;
              sail_u128 zexpansion_cost;
              zexpansion_cost = zbounded_mem_cost(zbounded_words);
              bool z2zE2747;
              z2zE2747 = (!u64_lt_u128(UINT64_C(18446744073709551615), zexpansion_cost));
              if (z2zE2747) {
                uint64_t zrequired_sizze;
                zrequired_sizze = u128_to_u64(zrequired);
                struct zMemoryRangeFields z2zE2748;
                z2zE2748 = zmemory_range(zbounded_start, zbounded_sizze);
                uint64_t z2zE2749;
                {
                  z2zE2749 = zmemory_expansion_cost(zrequired_sizze, zavailable);
                  if (have_exception) {  goto end_block_exception_2847;  }
                }
                struct zMemoryExpansion z3zE2886;
                z3zE2886.zcost = z2zE2749;
                z3zE2886.zrange = z2zE2748;
                z3zE2886.zrequired_sizze = zrequired_sizze;
                z3zE2882 = z3zE2886;
              } else {
                unit z3zE2884;
                {
                  z3zE2884 = zexc_halt(zOutOfGas);
                  if (have_exception) {  goto end_block_exception_2847;  }
                }
                struct zMemoryExpansion z3zE2885;
                z3zE2885.zcost = UINT64_C(0);
                z3zE2885.zrange = zEMPTY_MEMORY_RANGE;
                z3zE2885.zrequired_sizze = UINT64_C(0);
                z3zE2882 = z3zE2885;
              }
              goto finish_match_2844;
            }
          case_2845: ;
            sail_match_failure("memory_expansion");
          finish_match_2844: ;
            z3zE2881 = z3zE2882;
            goto finish_match_2842;
          }
        case_2843: ;
          sail_match_failure("memory_expansion");
        finish_match_2842: ;
          z3zE2880 = z3zE2881;
          goto finish_match_2840;
        }
      case_2841: ;
        sail_match_failure("memory_expansion");
      finish_match_2840: ;
        z3zE2879 = z3zE2880;
        goto finish_match_2838;
      }
    case_2839: ;
      sail_match_failure("memory_expansion");
    finish_match_2838: ;
      z8zE358 = z3zE2879;
    } else {
      unit z3zE2877;
      {
        z3zE2877 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2847;  }
      }
      struct zMemoryExpansion z3zE2878;
      z3zE2878.zcost = UINT64_C(0);
      z3zE2878.zrange = zEMPTY_MEMORY_RANGE;
      z3zE2878.zrequired_sizze = UINT64_C(0);
      z8zE358 = z3zE2878;
    }
  }
end_function_2846: ;
  return z8zE358;
end_block_exception_2847: ;
  struct zMemoryRangeFields z8zE902 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryExpansion z8zE901 = { .zcost = UINT64_C(0xdeadc0de), .zrange = z8zE902, .zrequired_sizze = UINT64_C(0xdeadc0de) };
  return z8zE901;
}

struct zMemoryPairExpansion zmemory_pair_expansion(sail_u256 zleft_start, sail_u256 zleft_sizze, sail_u256 zright_start, sail_u256 zright_sizze, uint64_t zavailable)
{
  struct zMemoryPairExpansion z8zE359;
  bool zleft_live;
  zleft_live = (!u256_eq_u64(zleft_sizze, UINT64_C(0)));
  bool zright_live;
  zright_live = (!u256_eq_u64(zright_sizze, UINT64_C(0)));
  bool zleft_fits;
  {
    bool z2zE2742;
    z2zE2742 = not(zleft_live);
    bool z3zE2863;
    if (z2zE2742) {  z3zE2863 = true;  } else {
      bool z2zE2741;
      z2zE2741 = (!u64_lt_u256(UINT64_C(18446744073709551615), zleft_start));
      bool z3zE2862;
      if (z2zE2741) {  z3zE2862 = (!u64_lt_u256(UINT64_C(18446744073709551615), zleft_sizze));  } else {
        z3zE2862 = false;
      }
      z3zE2863 = z3zE2862;
    }
    zleft_fits = z3zE2863;
  }
  bool zright_fits;
  {
    bool z2zE2740;
    z2zE2740 = not(zright_live);
    bool z3zE2865;
    if (z2zE2740) {  z3zE2865 = true;  } else {
      bool z2zE2739;
      z2zE2739 = (!u64_lt_u256(UINT64_C(18446744073709551615), zright_start));
      bool z3zE2864;
      if (z2zE2739) {  z3zE2864 = (!u64_lt_u256(UINT64_C(18446744073709551615), zright_sizze));  } else {
        z3zE2864 = false;
      }
      z3zE2865 = z3zE2864;
    }
    zright_fits = z3zE2865;
  }
  bool z2zE2732;
  {
    bool z3zE2866;
    if (zleft_fits) {  z3zE2866 = zright_fits;  } else {  z3zE2866 = false;  }
    z2zE2732 = z3zE2866;
  }
  if (z2zE2732) {
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
      bool z2zE2738;
      z2zE2738 = u128_lt(zleft_end, zright_end);
      if (z2zE2738) {  zrequired = zright_end;  } else {  zrequired = zleft_end;  }
    }
    uint64_t z2zE2733;
    z2zE2733 = zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
    struct zMemoryPairExpansion z3zE2869;
    {
      uint64_t zwords;
      zwords = z2zE2733;
      struct zMemoryPairExpansion z3zE2870;
      {
        uint64_t zbounded_words;
        zbounded_words = zwords;
        sail_u128 zexpansion_cost;
        zexpansion_cost = zbounded_mem_cost(zbounded_words);
        bool z2zE2734;
        z2zE2734 = (!u64_lt_u128(UINT64_C(18446744073709551615), zexpansion_cost));
        if (z2zE2734) {
          uint64_t zrequired_sizze;
          zrequired_sizze = u128_to_u64(zrequired);
          struct zMemoryRangeFields z2zE2735;
          if (zleft_live) {  z2zE2735 = zmemory_range(zbounded_left_start, zbounded_left_sizze);  } else {
            z2zE2735 = zEMPTY_MEMORY_RANGE;
          }
          struct zMemoryRangeFields z2zE2736;
          if (zright_live) {  z2zE2736 = zmemory_range(zbounded_right_start, zbounded_right_sizze);  } else {
            z2zE2736 = zEMPTY_MEMORY_RANGE;
          }
          uint64_t z2zE2737;
          {
            z2zE2737 = zmemory_expansion_cost(zrequired_sizze, zavailable);
            if (have_exception) {  goto end_block_exception_2837;  }
          }
          struct zMemoryPairExpansion z3zE2874;
          z3zE2874.zcost = z2zE2737;
          z3zE2874.zleft = z2zE2735;
          z3zE2874.zrequired_sizze = zrequired_sizze;
          z3zE2874.zright = z2zE2736;
          z3zE2870 = z3zE2874;
        } else {
          unit z3zE2872;
          {
            z3zE2872 = zexc_halt(zOutOfGas);
            if (have_exception) {  goto end_block_exception_2837;  }
          }
          struct zMemoryPairExpansion z3zE2873;
          z3zE2873.zcost = UINT64_C(0);
          z3zE2873.zleft = zEMPTY_MEMORY_RANGE;
          z3zE2873.zrequired_sizze = UINT64_C(0);
          z3zE2873.zright = zEMPTY_MEMORY_RANGE;
          z3zE2870 = z3zE2873;
        }
        goto finish_match_2834;
      }
    case_2835: ;
      sail_match_failure("memory_pair_expansion");
    finish_match_2834: ;
      z3zE2869 = z3zE2870;
      goto finish_match_2832;
    }
  case_2833: ;
    sail_match_failure("memory_pair_expansion");
  finish_match_2832: ;
    z8zE359 = z3zE2869;
  } else {
    unit z3zE2867;
    {
      z3zE2867 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2837;  }
    }
    struct zMemoryPairExpansion z3zE2868;
    z3zE2868.zcost = UINT64_C(0);
    z3zE2868.zleft = zEMPTY_MEMORY_RANGE;
    z3zE2868.zrequired_sizze = UINT64_C(0);
    z3zE2868.zright = zEMPTY_MEMORY_RANGE;
    z8zE359 = z3zE2868;
  }
end_function_2836: ;
  return z8zE359;
end_block_exception_2837: ;
  struct zMemoryRangeFields z8zE905 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryRangeFields z8zE904 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryPairExpansion z8zE903 = { .zcost = UINT64_C(0xdeadc0de), .zleft = z8zE905, .zrequired_sizze = UINT64_C(0xdeadc0de), .zright = z8zE904 };
  return z8zE903;
}

unit zexpand_memory(uint64_t zrequired_sizze)
{
  unit z8zE360;
  struct zByteSliceFields z2zE2731;
  z2zE2731 = zmemory_expand_to(zrequired_sizze);
  unit z3zE2860;
  {
    z3zE2860 = UNIT;
    goto finish_match_2828;
  }
case_2829: ;
  sail_match_failure("expand_memory");
finish_match_2828: ;
  z8zE360 = z3zE2860;
end_function_2830: ;
  return z8zE360;
end_block_exception_2831: ;

  return UNIT;
}

struct zMemoryRangeFields zapply_memory_expansion(struct zMemoryExpansion zplan)
{
  struct zMemoryRangeFields z8zE361;
  uint64_t z2zE2730;
  z2zE2730 = zplan.zrequired_sizze;
  unit z3zE2859;
  z3zE2859 = zexpand_memory(z2zE2730);
  z8zE361 = zplan.zrange;
end_function_2826: ;
  return z8zE361;
end_block_exception_2827: ;
  struct zMemoryRangeFields z8zE906 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE906;
}

struct zMemoryRangeFields zcharge_memory_range(sail_u256 zstart, sail_u256 zsizze)
{
  struct zMemoryRangeFields z8zE362;
  bool z2zE2727;
  z2zE2727 = zis_running(UNIT);
  if (z2zE2727) {
    uint64_t zavailable;
    zavailable = zgas_remaining;
    struct zMemoryExpansion zplan;
    {
      zplan = zmemory_expansion(zstart, zsizze, zavailable);
      if (have_exception) {  goto end_block_exception_2825;  }
    }
    uint64_t z2zE2728;
    z2zE2728 = zplan.zcost;
    unit z3zE2858;
    {
      z3zE2858 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2728);
      if (have_exception) {  goto end_block_exception_2825;  }
    }
    bool z2zE2729;
    z2zE2729 = zis_running(UNIT);
    if (z2zE2729) {  z8zE362 = zapply_memory_expansion(zplan);  } else {  z8zE362 = zEMPTY_MEMORY_RANGE;  }
  } else {  z8zE362 = zEMPTY_MEMORY_RANGE;  }
end_function_2824: ;
  return z8zE362;
end_block_exception_2825: ;
  struct zMemoryRangeFields z8zE907 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE907;
}

struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 zapply_memory_pair_expansion(struct zMemoryPairExpansion zplan)
{
  struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z8zE363;
  uint64_t z2zE2724;
  z2zE2724 = zplan.zrequired_sizze;
  unit z3zE2856;
  z3zE2856 = zexpand_memory(z2zE2724);
  struct zMemoryRangeFields z2zE2725;
  z2zE2725 = zplan.zleft;
  struct zMemoryRangeFields z2zE2726;
  z2zE2726 = zplan.zright;
  struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z3zE2857;
  z3zE2857.ztup0 = z2zE2725;
  z3zE2857.ztup1 = z2zE2726;
  z8zE363 = z3zE2857;
end_function_2822: ;
  return z8zE363;
end_block_exception_2823: ;
  struct zMemoryRangeFields z8zE910 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryRangeFields z8zE909 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z8zE908 = { .ztup0 = z8zE910, .ztup1 = z8zE909 };
  return z8zE908;
}

uint64_t zaccount_cost(bool zwarm)
{
  uint64_t z8zE364;
  if (zwarm) {  z8zE364 = zG_warm_access;  } else {
    bool z2zE2723;
    z2zE2723 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE2723) {  z8zE364 = zG_amsterdam_cold_account_access;  } else {  z8zE364 = zG_cold_account;  }
  }
end_function_2820: ;
  return z8zE364;
end_block_exception_2821: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zexternal_code_read_cost(unit z3zE2855)
{
  uint64_t z8zE365;
  bool z2zE2722;
  z2zE2722 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2722) {  z8zE365 = zG_warm_access;  } else {  z8zE365 = zG_zzero;  }
end_function_2818: ;
  return z8zE365;
end_block_exception_2819: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsload_cost(bool zwarm)
{
  uint64_t z8zE366;
  if (zwarm) {  z8zE366 = zG_warm_access;  } else {
    bool z2zE2721;
    z2zE2721 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE2721) {  z8zE366 = zG_amsterdam_cold_storage_access;  } else {  z8zE366 = zG_cold_sload;  }
  }
end_function_2816: ;
  return z8zE366;
end_block_exception_2817: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zcall_value_cost(unit z3zE2854)
{
  uint64_t z8zE367;
  bool z2zE2720;
  z2zE2720 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2720) {  z8zE367 = zG_amsterdam_call_value;  } else {  z8zE367 = zG_callvalue;  }
end_function_2814: ;
  return z8zE367;
end_block_exception_2815: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zcreate_access_cost(unit z3zE2853)
{
  uint64_t z8zE368;
  bool z2zE2719;
  z2zE2719 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2719) {  z8zE368 = zG_amsterdam_create_access;  } else {  z8zE368 = zG_create;  }
end_function_2812: ;
  return z8zE368;
end_block_exception_2813: ;

  return UINT64_C(0xdeadc0de);
}

void zcode_deployment_execution_cost(struct zoptionzIU64zK *z8zE369, uint64_t zbyte_len, uint64_t zavailable)
{
  bool z2zE2713;
  z2zE2713 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2713) {
    bool z2zE2714;
    z2zE2714 = (!(zEIP7954_DEPLOYED_CODE_SIZE_LIMIT < zbyte_len));
    if (z2zE2714) {
      uint64_t zwords;
      zwords = zmemory_word_countzIreprzGU64zCU64zKzIboundszG07d9444e226eec3cde1a1e781c91ddf4zK(zbyte_len);
      bool z2zE2716;
      {
        uint64_t z2zE2715;
        z2zE2715 = (zavailable / zG_keccak_word);
        z2zE2716 = (!(z2zE2715 < zwords));
      }
      if (z2zE2716) {
        uint64_t zcost;
        {    zcost = (zG_keccak_word * zwords);
        }
        zSomezIU64zK((*(&z8zE369)), zcost);
      } else {  zNonezIU64zK((*(&z8zE369)), UNIT);  }
    } else {  zNonezIU64zK((*(&z8zE369)), UNIT);  }
  } else {
    bool z2zE2718;
    {
      uint64_t z2zE2717;
      z2zE2717 = (zavailable / zG_codedeposit);
      z2zE2718 = (!(z2zE2717 < zbyte_len));
    }
    if (z2zE2718) {
      uint64_t z3zE2852;
      {    z3zE2852 = (zG_codedeposit * zbyte_len);
      }
      zSomezIU64zK((*(&z8zE369)), z3zE2852);
    } else {  zNonezIU64zK((*(&z8zE369)), UNIT);  }
  }
end_function_2810: ;
  goto end_function_4072;
end_block_exception_2811: ;
  goto end_function_4072;
end_function_4072: ;
}

uint64_t zcode_deployment_state_cost(uint64_t zbyte_len)
{
  uint64_t z8zE370;
  bool z2zE2710;
  z2zE2710 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2710) {
    bool z2zE2711;
    z2zE2711 = (!(zEIP7954_DEPLOYED_CODE_SIZE_LIMIT < zbyte_len));
    if (z2zE2711) {
      {    z8zE370 = (zG_amsterdam_state_byte * zbyte_len);
      }
    } else {
      struct zexception z2zE2712;
      CREATE(zexception)(&z2zE2712);
      zInvalidBlock(&z2zE2712, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE2712);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/gas.sail:811.12-811.48");
      KILL(zexception)(&z2zE2712);
      goto end_block_exception_2809;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2712);
    }
  } else {  z8zE370 = zGAS_COST_ZERO;  }
end_function_2808: ;
  return z8zE370;
end_block_exception_2809: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zpc_word(struct zByteSliceFields zinput, uint64_t zstart, uint64_t zbyte_count)
{
  sail_u256 z8zE371;
  sail_u256 zvalue;
  zvalue = zZERO_WORD;
  uint64_t zstart_offset;
  zstart_offset = zstart;
  uint64_t zcount;
  zcount = zbyte_count;
  uint64_t zinput_length;
  zinput_length = zinput.zlen;
  int64_t z3zE2847;
  {    z3zE2847 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2848;
  {    z3zE2848 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2849;
  {    z3zE2849 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE2847;
    unit z3zE2850;
  for_start_2804: ;
    {
      if ((z3zE2848 < zbyte_index)) goto for_end_2805;
      uint64_t zoffset;
      zoffset = (uint64_t)(zbyte_index);
      bool z2zE2705;
      z2zE2705 = (zoffset < zcount);
      if (z2zE2705) {
        uint64_t znext_byte;
        {
          bool z2zE2708;
          z2zE2708 = (zstart_offset < zinput_length);
          if (z2zE2708) {
            uint64_t zremaining;
            {    zremaining = (zinput_length - zstart_offset);
            }
            bool z2zE2709;
            z2zE2709 = (zoffset < zremaining);
            if (z2zE2709) {
              uint64_t zcursor;
              {    zcursor = (zstart_offset + zoffset);
              }
              znext_byte = zslice_byte(zinput, zcursor);
            } else {  znext_byte = UINT64_C(0x00);  }
          } else {  znext_byte = UINT64_C(0x00);  }
        }
        sail_u256 z2zE2706;
        z2zE2706 = zword_shift_left(zvalue, UINT64_C(8));
        uint64_t z2zE2707;
        z2zE2707 = ((uint64_t) znext_byte);
        zvalue = zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2706, z2zE2707);
        z3zE2850 = UNIT;
      } else {  z3zE2850 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE2849);
      goto for_start_2804;
    }
  for_end_2805: ;
  }
  unit z3zE2851;
  z3zE2851 = UNIT;
  z8zE371 = zvalue;
end_function_2806: ;
  return z8zE371;
end_block_exception_2807: ;

  return u256_zero();
}

sail_u256 zpc_word_after_declared_field(struct zByteSliceFields zinput, uint64_t zprefix, sail_u256 zdeclared_length, uint64_t zbyte_count)
{
  sail_u256 z8zE372;
  uint64_t zinput_length;
  zinput_length = zinput.zlen;
  sail_u256 z3zE2841;
  {
    uint64_t zprefix_offset;
    zprefix_offset = zprefix;
    bool z2zE2702;
    z2zE2702 = (zprefix_offset < zinput_length);
    if (z2zE2702) {
      uint64_t z2zE2703;
      {    z2zE2703 = (zinput_length - zprefix_offset);
      }
      sail_u256 z3zE2843;
      {
        uint64_t zsuffix_length;
        zsuffix_length = z2zE2703;
        bool z2zE2704;
        z2zE2704 = u256_lt_u64(zdeclared_length, zsuffix_length);
        if (z2zE2704) {
          sail_u256 z3zE2845;
          {
            uint64_t zfield_length;
            zfield_length = u256_to_u64(zdeclared_length);
            uint64_t zstart;
            {    zstart = (zprefix_offset + zfield_length);
            }
            z3zE2845 = zpc_word(zinput, zstart, zbyte_count);
            goto finish_match_2800;
          }
        case_2801: ;
          sail_match_failure("pc_word_after_declared_field");
        finish_match_2800: ;
          z3zE2843 = z3zE2845;
        } else {  z3zE2843 = zZERO_WORD;  }
        goto finish_match_2798;
      }
    case_2799: ;
      sail_match_failure("pc_word_after_declared_field");
    finish_match_2798: ;
      z3zE2841 = z3zE2843;
    } else {  z3zE2841 = zZERO_WORD;  }
    goto finish_match_2796;
  }
case_2797: ;
  sail_match_failure("pc_word_after_declared_field");
finish_match_2796: ;
  z8zE372 = z3zE2841;
end_function_2802: ;
  return z8zE372;
end_block_exception_2803: ;

  return u256_zero();
}

uint64_t zpc_blake2_rounds(struct zByteSliceFields zinput)
{
  uint64_t z8zE373;
  sail_u256 z2zE2701;
  z2zE2701 = zpc_word(zinput, UINT64_C(0), UINT64_C(4));
  uint64_t z3zE2840;
  z3zE2840 = u256_to_u64(u256_mod_u64(z2zE2701, UINT64_C(4294967296)));
  z8zE373 = z3zE2840;
end_function_2794: ;
  return z8zE373;
end_block_exception_2795: ;

  return UINT64_C(0xdeadc0de);
}

void zmodexp_gas(struct zoptionzIU64zK *z8zE374, struct zByteSliceFields zinput, uint64_t zavailable)
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
  bool z2zE2666;
  {
    bool z3zE2815;
    if (zosaka) {
      bool z2zE2665;
      z2zE2665 = u64_lt_u256(UINT64_C(1024), zbl);
      bool z3zE2814;
      if (z2zE2665) {  z3zE2814 = true;  } else {
        bool z2zE2664;
        z2zE2664 = u64_lt_u256(UINT64_C(1024), zel);
        bool z3zE2813;
        if (z2zE2664) {  z3zE2813 = true;  } else {  z3zE2813 = u64_lt_u256(UINT64_C(1024), zml);  }
        z3zE2814 = z3zE2813;
      }
      z3zE2815 = z3zE2814;
    } else {  z3zE2815 = false;  }
    z2zE2666 = z3zE2815;
  }
  unit z3zE2816;
  if (z2zE2666) {
    struct zoptionzIU64zK z2zE2667;
    CREATE(zoptionzIU64zK)(&z2zE2667);
    zNonezIU64zK(&z2zE2667, UNIT);
    COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2667);
    goto cleanup_2791;
    /* unreachable after return */
    KILL(zoptionzIU64zK)(&z2zE2667);
    goto end_cleanup_2792;
  cleanup_2791: ;
    KILL(zoptionzIU64zK)(&z2zE2667);
    goto end_function_2774;
  end_cleanup_2792: ;
  } else {  z3zE2816 = UNIT;  }
  uint64_t zminimum;
  if (zosaka) {  zminimum = UINT64_C(500);  } else {  zminimum = UINT64_C(200);  }
  bool z2zE2668;
  z2zE2668 = (zavailable < zminimum);
  unit z3zE2820;
  if (z2zE2668) {
    struct zoptionzIU64zK z2zE2669;
    CREATE(zoptionzIU64zK)(&z2zE2669);
    zNonezIU64zK(&z2zE2669, UNIT);
    COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2669);
    goto cleanup_2789;
    /* unreachable after return */
    KILL(zoptionzIU64zK)(&z2zE2669);
    goto end_cleanup_2790;
  cleanup_2789: ;
    KILL(zoptionzIU64zK)(&z2zE2669);
    goto end_function_2774;
  end_cleanup_2790: ;
  } else {  z3zE2820 = UNIT;  }
  bool z2zE2672;
  {
    bool z2zE2671;
    z2zE2671 = not(zosaka);
    bool z3zE2818;
    if (z2zE2671) {
      bool z2zE2670;
      z2zE2670 = zword_is_zzero(zbl_word);
      bool z3zE2817;
      if (z2zE2670) {  z3zE2817 = zword_is_zzero(zml_word);  } else {  z3zE2817 = false;  }
      z3zE2818 = z3zE2817;
    } else {  z3zE2818 = false;  }
    z2zE2672 = z3zE2818;
  }
  unit z3zE2819;
  if (z2zE2672) {
    struct zoptionzIU64zK z2zE2673;
    CREATE(zoptionzIU64zK)(&z2zE2673);
    zSomezIU64zK(&z2zE2673, UINT64_C(200));
    COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2673);
    goto cleanup_2787;
    /* unreachable after return */
    KILL(zoptionzIU64zK)(&z2zE2673);
    goto end_cleanup_2788;
  cleanup_2787: ;
    KILL(zoptionzIU64zK)(&z2zE2673);
    goto end_function_2774;
  end_cleanup_2788: ;
  } else {  z3zE2819 = UNIT;  }
  sail_u256 zmaxlen;
  {
    bool z2zE2700;
    z2zE2700 = u256_lt(zml, zbl);
    if (z2zE2700) {  zmaxlen = zbl;  } else {  zmaxlen = zml;  }
  }
  sail_u256 zwhole_words;
  zwhole_words = u256_div_u64(zmaxlen, UINT64_C(8));
  sail_u256 zwords;
  {
    uint64_t z2zE2699;
    {
      bool z2zE2698;
      {
        uint64_t z2zE2697;
        {
          uint64_t z3zE2821;
          z3zE2821 = u256_to_u64(u256_mod_u64(zmaxlen, UINT64_C(8)));
          z2zE2697 = z3zE2821;
        }
        z2zE2698 = (z2zE2697 == UINT64_C(0));
      }
      if (z2zE2698) {  z2zE2699 = UINT64_C(0);  } else {  z2zE2699 = UINT64_C(1);  }
    }
    zwords = u256_add_u64(zwhole_words, z2zE2699);
  }
  sail_u128 zproduct_limit_value;
  if (zosaka) {  zproduct_limit_value = u128_of_u64(zavailable);  } else {
    sail_u128 z2zE2696;
    z2zE2696 = u128_mul_u64_u64(UINT64_C(3), zavailable);
    zproduct_limit_value = u128_add_u64(z2zE2696, UINT64_C(2));
  }
  struct zoptionzIU64zK z3zE2822;
  CREATE(zoptionzIU64zK)(&z3zE2822);
  {
    sail_u128 zproduct_limit;
    zproduct_limit = zproduct_limit_value;
    bool zwords_unaffordable;
    zwords_unaffordable = u128_lt_u256(zproduct_limit, zwords);
    sail_u128 zbounded_words;
    if (zwords_unaffordable) {  zbounded_words = zproduct_limit;  } else {  zbounded_words = u128_of_u256(zwords);  }
    unit z3zE2823;
    if (zwords_unaffordable) {
      struct zoptionzIU64zK z2zE2674;
      CREATE(zoptionzIU64zK)(&z2zE2674);
      zNonezIU64zK(&z2zE2674, UNIT);
      COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2674);
      goto cleanup_2785;
      /* unreachable after return */
      KILL(zoptionzIU64zK)(&z2zE2674);
      goto end_cleanup_2786;
    cleanup_2785: ;
      KILL(zoptionzIU64zK)(&z3zE2822);
      KILL(zoptionzIU64zK)(&z2zE2674);
      goto end_function_2774;
    end_cleanup_2786: ;
    } else {  z3zE2823 = UNIT;  }
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
    unit z3zE2824;
    if (zsquare_unaffordable) {
      struct zoptionzIU64zK z2zE2675;
      CREATE(zoptionzIU64zK)(&z2zE2675);
      zNonezIU64zK(&z2zE2675, UNIT);
      COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2675);
      goto cleanup_2783;
      /* unreachable after return */
      KILL(zoptionzIU64zK)(&z2zE2675);
      goto end_cleanup_2784;
    cleanup_2783: ;
      KILL(zoptionzIU64zK)(&z3zE2822);
      KILL(zoptionzIU64zK)(&z2zE2675);
      goto end_function_2774;
    end_cleanup_2784: ;
    } else {  z3zE2824 = UNIT;  }
    sail_u128 zmult_value;
    {
      bool z2zE2694;
      {
        bool z3zE2825;
        if (zosaka) {  z3zE2825 = (!u64_lt_u256(UINT64_C(32), zmaxlen));  } else {  z3zE2825 = false;  }
        z2zE2694 = z3zE2825;
      }
      if (z2zE2694) {  zmult_value = u128_of_u64(UINT64_C(16));  } else if (zosaka) {
        uint64_t zdoubled_limit_value;
        zdoubled_limit_value = (zavailable / UINT64_C(2));
        sail_u128 z3zE2826;
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
          unit z3zE2827;
          if (zdoubled_unaffordable) {
            struct zoptionzIU64zK z2zE2695;
            CREATE(zoptionzIU64zK)(&z2zE2695);
            zNonezIU64zK(&z2zE2695, UNIT);
            COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2695);
            goto cleanup_2781;
            /* unreachable after return */
            KILL(zoptionzIU64zK)(&z2zE2695);
            goto end_cleanup_2782;
          cleanup_2781: ;
            KILL(zoptionzIU64zK)(&z3zE2822);
            KILL(zoptionzIU64zK)(&z2zE2695);
            goto end_function_2774;
          end_cleanup_2782: ;
          } else {  z3zE2827 = UNIT;  }
          z3zE2826 = u128_mul_u64_u64(UINT64_C(2), zbounded_square);
          goto finish_match_2770;
        }
      case_2771: ;
        sail_match_failure("modexp_gas");
      finish_match_2770: ;
        zmult_value = z3zE2826;
      } else {  zmult_value = zaffordable_square;  }
    }
    struct zoptionzIU64zK z3zE2829;
    CREATE(zoptionzIU64zK)(&z3zE2829);
    {
      sail_u128 zmult;
      zmult = zmult_value;
      bool z2zE2676;
      z2zE2676 = u128_lt(zproduct_limit, zmult);
      unit z3zE2830;
      if (z2zE2676) {
        struct zoptionzIU64zK z2zE2677;
        CREATE(zoptionzIU64zK)(&z2zE2677);
        zNonezIU64zK(&z2zE2677, UNIT);
        COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2677);
        goto cleanup_2779;
        /* unreachable after return */
        KILL(zoptionzIU64zK)(&z2zE2677);
        goto end_cleanup_2780;
      cleanup_2779: ;
        KILL(zoptionzIU64zK)(&z3zE2822);
        KILL(zoptionzIU64zK)(&z3zE2829);
        KILL(zoptionzIU64zK)(&z2zE2677);
        goto end_function_2774;
      end_cleanup_2780: ;
      } else {  z3zE2830 = UNIT;  }
      sail_u128 ziterations;
      {
        bool z2zE2681;
        z2zE2681 = (!u64_lt_u256(UINT64_C(32), zel));
        if (z2zE2681) {
          uint64_t zexponent_bits;
          {
            sail_u256 z2zE2684;
            {
              uint64_t z3zE3865;
              z3zE3865 = u256_to_u64(zel);
              z2zE2684 = zpc_word_after_declared_field(zinput, UINT64_C(96), zbl, z3zE3865);
            }
            zexponent_bits = zword_bit_length(z2zE2684);
          }
          bool z2zE2682;
          z2zE2682 = (zexponent_bits == UINT64_C(0));
          if (z2zE2682) {  ziterations = u128_of_u64(UINT64_C(1));  } else {
            uint64_t zcount;
            {    zcount = (zexponent_bits - UINT64_C(1));
            }
            bool z2zE2683;
            z2zE2683 = (zcount == UINT64_C(0));
            if (z2zE2683) {  ziterations = u128_of_u64(UINT64_C(1));  } else {  ziterations = u128_of_u64(zcount);  }
          }
        } else {
          uint64_t zhead_bits;
          {
            sail_u256 z2zE2693;
            z2zE2693 = zpc_word_after_declared_field(zinput, UINT64_C(96), zbl, UINT64_C(32));
            zhead_bits = zword_bit_length(z2zE2693);
          }
          uint64_t zhigh_bits;
          {
            bool z2zE2692;
            z2zE2692 = (zhead_bits != UINT64_C(0));
            if (z2zE2692) {
              {    zhigh_bits = (zhead_bits - UINT64_C(1));
              }
            } else {  zhigh_bits = UINT64_C(0);  }
          }
          sail_u256 zextra;
          zextra = u256_sub_u64(zel, UINT64_C(32));
          sail_u128 z3zE2836;
          if (zosaka) {
            uint64_t zextra_limit;
            zextra_limit = (zavailable / UINT64_C(16));
            uint64_t zosaka_extra;
            {
              bool z2zE2688;
              z2zE2688 = zword_greater_than_wordzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zextra, UINT64_C(992));
              if (z2zE2688) {  zosaka_extra = UINT64_C(992);  } else {  zosaka_extra = u256_to_u64(zextra);  }
            }
            bool zextra_unaffordable;
            zextra_unaffordable = (zextra_limit < zosaka_extra);
            uint64_t zbounded_extra;
            if (zextra_unaffordable) {  zbounded_extra = zextra_limit;  } else {  zbounded_extra = zosaka_extra;  }
            unit z3zE2833;
            if (zextra_unaffordable) {
              struct zoptionzIU64zK z2zE2686;
              CREATE(zoptionzIU64zK)(&z2zE2686);
              zNonezIU64zK(&z2zE2686, UNIT);
              COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2686);
              goto cleanup_2777;
              /* unreachable after return */
              KILL(zoptionzIU64zK)(&z2zE2686);
              goto end_cleanup_2778;
            cleanup_2777: ;
              KILL(zoptionzIU64zK)(&z3zE2822);
              KILL(zoptionzIU64zK)(&z3zE2829);
              KILL(zoptionzIU64zK)(&z2zE2686);
              goto end_function_2774;
            end_cleanup_2778: ;
            } else {  z3zE2833 = UNIT;  }
            uint64_t z2zE2687;
            {    z2zE2687 = (UINT64_C(16) * zbounded_extra);
            }
            z3zE2836 = u128_add_u64_u64(z2zE2687, zhigh_bits);
          } else {
            sail_u128 zpre_osaka_limit;
            {
              sail_u128 z2zE2691;
              z2zE2691 = u128_mul_u64_u64(UINT64_C(3), zavailable);
              zpre_osaka_limit = u128_add_u64(z2zE2691, UINT64_C(2));
            }
            uint64_t z3zE2837;
            {
              uint64_t z3zE2831;
              z3zE2831 = u128_to_u64(u128_div_u64(zpre_osaka_limit, UINT64_C(8)));
              z3zE2837 = z3zE2831;
            }
            bool z3zE2838;
            z3zE2838 = u64_lt_u256(z3zE2837, zextra);
            uint64_t z3zE2839;
            if (z3zE2838) {  z3zE2839 = z3zE2837;  } else {  z3zE2839 = u256_to_u64(zextra);  }
            unit z3zE2832;
            if (z3zE2838) {
              struct zoptionzIU64zK z2zE2689;
              CREATE(zoptionzIU64zK)(&z2zE2689);
              zNonezIU64zK(&z2zE2689, UNIT);
              COPY(zoptionzIU64zK)((*(&z8zE374)), z2zE2689);
              goto cleanup_2775;
              /* unreachable after return */
              KILL(zoptionzIU64zK)(&z2zE2689);
              goto end_cleanup_2776;
            cleanup_2775: ;
              KILL(zoptionzIU64zK)(&z3zE2822);
              KILL(zoptionzIU64zK)(&z3zE2829);
              KILL(zoptionzIU64zK)(&z2zE2689);
              goto end_function_2774;
            end_cleanup_2776: ;
            } else {  z3zE2832 = UNIT;  }
            sail_u128 z2zE2690;
            z2zE2690 = u128_mul_u64_u64(UINT64_C(8), z3zE2839);
            z3zE2836 = u128_add_u64(z2zE2690, zhigh_bits);
          }
          bool z2zE2685;
          z2zE2685 = u128_eq_u64(z3zE2836, UINT64_C(0));
          if (z2zE2685) {  ziterations = u128_of_u64(UINT64_C(1));  } else {  ziterations = z3zE2836;  }
        }
      }
      bool z2zE2678;
      z2zE2678 = u128_eq_u64(ziterations, UINT64_C(0));
      if (z2zE2678) {  zNonezIU64zK(&z3zE2829, UNIT);  } else {
        sail_u256 zproduct;
        zproduct = u256_mul_u128_u128(zmult, ziterations);
        bool zproduct_unaffordable;
        zproduct_unaffordable = u128_lt_u256(zwide_product_limit, zproduct);
        sail_u128 zaffordable_product;
        if (zproduct_unaffordable) {  zaffordable_product = zproduct_limit;  } else {
          zaffordable_product = u128_of_u256(zproduct);
        }
        if (zproduct_unaffordable) {  zNonezIU64zK(&z3zE2829, UNIT);  } else {
          sail_u128 zcalculated;
          if (zosaka) {  zcalculated = zaffordable_product;  } else {
            zcalculated = u128_div_u64(zaffordable_product, UINT64_C(3));
          }
          sail_u128 zcost;
          {
            bool z2zE2680;
            z2zE2680 = u128_lt_u64(zcalculated, zminimum);
            if (z2zE2680) {  zcost = u128_of_u64(zminimum);  } else {  zcost = zcalculated;  }
          }
          bool z2zE2679;
          z2zE2679 = (!u64_lt_u128(zavailable, zcost));
          if (z2zE2679) {
            uint64_t zaffordable;
            zaffordable = u128_to_u64(zcost);
            zSomezIU64zK(&z3zE2829, zaffordable);
          } else {  zNonezIU64zK(&z3zE2829, UNIT);  }
        }
      }
      goto finish_match_2772;
    }
  case_2773: ;
    sail_match_failure("modexp_gas");
  finish_match_2772: ;
    COPY(zoptionzIU64zK)(&z3zE2822, z3zE2829);
    KILL(zoptionzIU64zK)(&z3zE2829);
    goto finish_match_2768;
  }
case_2769: ;
  sail_match_failure("modexp_gas");
finish_match_2768: ;
  COPY(zoptionzIU64zK)((*(&z8zE374)), z3zE2822);
  KILL(zoptionzIU64zK)(&z3zE2822);
end_function_2774: ;
  goto end_function_4071;
end_block_exception_2793: ;
  goto end_function_4071;
end_function_4071: ;
}

void create_letbind_150(void) {    CREATE(zz5vecz8z5bv16z9)(&zBLS_G1_DISCOUNT);

  zz5vecz8z5bv16z9 z3zE184;
  CREATE(zz5vecz8z5bv16z9)(&z3zE184);
  zz5vecz8z5bv16z9 z3zE183;
  CREATE(zz5vecz8z5bv16z9)(&z3zE183);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE183, INT64_C(128));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(0), UINT64_C(0x0207));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(1), UINT64_C(0x0208));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(2), UINT64_C(0x0208));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(3), UINT64_C(0x0209));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(4), UINT64_C(0x020A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(5), UINT64_C(0x020A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(6), UINT64_C(0x020B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(7), UINT64_C(0x020C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(8), UINT64_C(0x020D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(9), UINT64_C(0x020D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(10), UINT64_C(0x020E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(11), UINT64_C(0x020F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(12), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(13), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(14), UINT64_C(0x0211));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(15), UINT64_C(0x0212));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(16), UINT64_C(0x0213));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(17), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(18), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(19), UINT64_C(0x0215));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(20), UINT64_C(0x0216));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(21), UINT64_C(0x0217));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(22), UINT64_C(0x0218));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(23), UINT64_C(0x0218));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(24), UINT64_C(0x0219));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(25), UINT64_C(0x021A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(26), UINT64_C(0x021B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(27), UINT64_C(0x021C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(28), UINT64_C(0x021C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(29), UINT64_C(0x021D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(30), UINT64_C(0x021E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(31), UINT64_C(0x021F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(32), UINT64_C(0x0220));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(33), UINT64_C(0x0221));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(34), UINT64_C(0x0222));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(35), UINT64_C(0x0223));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(36), UINT64_C(0x0223));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(37), UINT64_C(0x0224));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(38), UINT64_C(0x0225));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(39), UINT64_C(0x0226));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(40), UINT64_C(0x0227));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(41), UINT64_C(0x0228));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(42), UINT64_C(0x0229));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(43), UINT64_C(0x022A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(44), UINT64_C(0x022B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(45), UINT64_C(0x022C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(46), UINT64_C(0x022D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(47), UINT64_C(0x022E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(48), UINT64_C(0x022F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(49), UINT64_C(0x0230));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(50), UINT64_C(0x0231));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(51), UINT64_C(0x0232));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(52), UINT64_C(0x0233));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(53), UINT64_C(0x0234));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(54), UINT64_C(0x0235));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(55), UINT64_C(0x0236));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(56), UINT64_C(0x0237));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(57), UINT64_C(0x0238));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(58), UINT64_C(0x0239));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(59), UINT64_C(0x023A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(60), UINT64_C(0x023C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(61), UINT64_C(0x023D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(62), UINT64_C(0x023E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(63), UINT64_C(0x023F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(64), UINT64_C(0x0240));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(65), UINT64_C(0x0241));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(66), UINT64_C(0x0243));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(67), UINT64_C(0x0244));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(68), UINT64_C(0x0245));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(69), UINT64_C(0x0246));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(70), UINT64_C(0x0248));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(71), UINT64_C(0x0249));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(72), UINT64_C(0x024A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(73), UINT64_C(0x024C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(74), UINT64_C(0x024D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(75), UINT64_C(0x024F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(76), UINT64_C(0x0250));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(77), UINT64_C(0x0251));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(78), UINT64_C(0x0253));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(79), UINT64_C(0x0254));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(80), UINT64_C(0x0256));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(81), UINT64_C(0x0257));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(82), UINT64_C(0x0259));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(83), UINT64_C(0x025B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(84), UINT64_C(0x025C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(85), UINT64_C(0x025E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(86), UINT64_C(0x0260));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(87), UINT64_C(0x0261));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(88), UINT64_C(0x0263));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(89), UINT64_C(0x0265));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(90), UINT64_C(0x0267));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(91), UINT64_C(0x0269));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(92), UINT64_C(0x026B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(93), UINT64_C(0x026D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(94), UINT64_C(0x026F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(95), UINT64_C(0x0271));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(96), UINT64_C(0x0273));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(97), UINT64_C(0x0276));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(98), UINT64_C(0x0278));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(99), UINT64_C(0x027B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(100), UINT64_C(0x027D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(101), UINT64_C(0x0280));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(102), UINT64_C(0x0282));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(103), UINT64_C(0x0285));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(104), UINT64_C(0x0288));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(105), UINT64_C(0x028B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(106), UINT64_C(0x028E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(107), UINT64_C(0x0292));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(108), UINT64_C(0x0295));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(109), UINT64_C(0x0299));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(110), UINT64_C(0x029D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(111), UINT64_C(0x02A1));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(112), UINT64_C(0x02A5));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(113), UINT64_C(0x02AA));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(114), UINT64_C(0x02AF));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(115), UINT64_C(0x02B4));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(116), UINT64_C(0x02BA));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(117), UINT64_C(0x02C1));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(118), UINT64_C(0x02C8));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(119), UINT64_C(0x02CF));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(120), UINT64_C(0x02D8));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(121), UINT64_C(0x02E2));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(122), UINT64_C(0x02EE));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(123), UINT64_C(0x02FC));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(124), UINT64_C(0x031D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(125), UINT64_C(0x0350));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(126), UINT64_C(0x03B5));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE183, z3zE183, INT64_C(127), UINT64_C(0x03E8));
  KILL(zz5vecz8z5bv16z9)(&z3zE184);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE184, z3zE183.len);
  for (int z8zE911 = 0; z8zE911 < z3zE183.len; z8zE911++) {
    uint64_t z8zE912 = z3zE183.data[z8zE911];
    uint64_t z8zE913;
    z8zE913 = z8zE912;
    z3zE184.data[z8zE911] = z8zE913;
  }
  KILL(zz5vecz8z5bv16z9)(&z3zE183);
  COPY(zz5vecz8z5bv16z9)(&zBLS_G1_DISCOUNT, z3zE184);
  KILL(zz5vecz8z5bv16z9)(&z3zE184);
let_end_549: ;
}
void kill_letbind_150(void) {    KILL(zz5vecz8z5bv16z9)(&zBLS_G1_DISCOUNT);
}

void create_letbind_151(void) {    CREATE(zz5vecz8z5bv16z9)(&zBLS_G2_DISCOUNT);

  zz5vecz8z5bv16z9 z3zE186;
  CREATE(zz5vecz8z5bv16z9)(&z3zE186);
  zz5vecz8z5bv16z9 z3zE185;
  CREATE(zz5vecz8z5bv16z9)(&z3zE185);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE185, INT64_C(128));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(0), UINT64_C(0x020C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(1), UINT64_C(0x020C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(2), UINT64_C(0x020D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(3), UINT64_C(0x020E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(4), UINT64_C(0x020E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(5), UINT64_C(0x020F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(6), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(7), UINT64_C(0x0210));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(8), UINT64_C(0x0211));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(9), UINT64_C(0x0212));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(10), UINT64_C(0x0212));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(11), UINT64_C(0x0213));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(12), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(13), UINT64_C(0x0214));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(14), UINT64_C(0x0215));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(15), UINT64_C(0x0216));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(16), UINT64_C(0x0217));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(17), UINT64_C(0x0217));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(18), UINT64_C(0x0218));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(19), UINT64_C(0x0219));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(20), UINT64_C(0x0219));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(21), UINT64_C(0x021A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(22), UINT64_C(0x021B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(23), UINT64_C(0x021C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(24), UINT64_C(0x021D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(25), UINT64_C(0x021D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(26), UINT64_C(0x021E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(27), UINT64_C(0x021F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(28), UINT64_C(0x0220));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(29), UINT64_C(0x0221));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(30), UINT64_C(0x0221));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(31), UINT64_C(0x0222));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(32), UINT64_C(0x0223));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(33), UINT64_C(0x0224));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(34), UINT64_C(0x0225));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(35), UINT64_C(0x0226));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(36), UINT64_C(0x0227));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(37), UINT64_C(0x0228));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(38), UINT64_C(0x0228));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(39), UINT64_C(0x0229));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(40), UINT64_C(0x022A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(41), UINT64_C(0x022B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(42), UINT64_C(0x022C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(43), UINT64_C(0x022D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(44), UINT64_C(0x022E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(45), UINT64_C(0x022F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(46), UINT64_C(0x0230));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(47), UINT64_C(0x0231));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(48), UINT64_C(0x0232));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(49), UINT64_C(0x0233));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(50), UINT64_C(0x0235));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(51), UINT64_C(0x0236));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(52), UINT64_C(0x0237));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(53), UINT64_C(0x0238));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(54), UINT64_C(0x0239));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(55), UINT64_C(0x023A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(56), UINT64_C(0x023B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(57), UINT64_C(0x023D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(58), UINT64_C(0x023E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(59), UINT64_C(0x023F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(60), UINT64_C(0x0240));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(61), UINT64_C(0x0242));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(62), UINT64_C(0x0243));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(63), UINT64_C(0x0244));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(64), UINT64_C(0x0246));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(65), UINT64_C(0x0247));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(66), UINT64_C(0x0248));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(67), UINT64_C(0x024A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(68), UINT64_C(0x024B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(69), UINT64_C(0x024D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(70), UINT64_C(0x024E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(71), UINT64_C(0x0250));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(72), UINT64_C(0x0251));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(73), UINT64_C(0x0253));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(74), UINT64_C(0x0255));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(75), UINT64_C(0x0256));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(76), UINT64_C(0x0258));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(77), UINT64_C(0x025A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(78), UINT64_C(0x025C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(79), UINT64_C(0x025E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(80), UINT64_C(0x025F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(81), UINT64_C(0x0261));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(82), UINT64_C(0x0263));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(83), UINT64_C(0x0265));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(84), UINT64_C(0x0267));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(85), UINT64_C(0x026A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(86), UINT64_C(0x026C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(87), UINT64_C(0x026E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(88), UINT64_C(0x0270));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(89), UINT64_C(0x0273));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(90), UINT64_C(0x0275));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(91), UINT64_C(0x0278));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(92), UINT64_C(0x027A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(93), UINT64_C(0x027D));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(94), UINT64_C(0x0280));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(95), UINT64_C(0x0283));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(96), UINT64_C(0x0286));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(97), UINT64_C(0x0289));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(98), UINT64_C(0x028C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(99), UINT64_C(0x028F));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(100), UINT64_C(0x0293));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(101), UINT64_C(0x0297));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(102), UINT64_C(0x029A));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(103), UINT64_C(0x029E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(104), UINT64_C(0x02A2));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(105), UINT64_C(0x02A7));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(106), UINT64_C(0x02AB));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(107), UINT64_C(0x02B0));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(108), UINT64_C(0x02B5));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(109), UINT64_C(0x02BB));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(110), UINT64_C(0x02C0));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(111), UINT64_C(0x02C7));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(112), UINT64_C(0x02CD));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(113), UINT64_C(0x02D4));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(114), UINT64_C(0x02DC));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(115), UINT64_C(0x02E4));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(116), UINT64_C(0x02ED));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(117), UINT64_C(0x02F7));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(118), UINT64_C(0x0302));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(119), UINT64_C(0x030E));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(120), UINT64_C(0x031C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(121), UINT64_C(0x032C));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(122), UINT64_C(0x0340));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(123), UINT64_C(0x0357));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(124), UINT64_C(0x0374));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(125), UINT64_C(0x039B));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(126), UINT64_C(0x03E8));
  internal_vector_update_zz5vecz8z5bv16z9(&z3zE185, z3zE185, INT64_C(127), UINT64_C(0x03E8));
  KILL(zz5vecz8z5bv16z9)(&z3zE186);
  internal_vector_init_zz5vecz8z5bv16z9(&z3zE186, z3zE185.len);
  for (int z8zE914 = 0; z8zE914 < z3zE185.len; z8zE914++) {
    uint64_t z8zE915 = z3zE185.data[z8zE914];
    uint64_t z8zE916;
    z8zE916 = z8zE915;
    z3zE186.data[z8zE914] = z8zE916;
  }
  KILL(zz5vecz8z5bv16z9)(&z3zE185);
  COPY(zz5vecz8z5bv16z9)(&zBLS_G2_DISCOUNT, z3zE186);
  KILL(zz5vecz8z5bv16z9)(&z3zE186);
let_end_550: ;
}
void kill_letbind_151(void) {    KILL(zz5vecz8z5bv16z9)(&zBLS_G2_DISCOUNT);
}

void zbls_msm_gas(struct zoptionzIU64zK *z8zE375, zz5vecz8z5bv16z9 ztable, uint64_t zbase, uint64_t zmaxd, uint64_t zk, uint64_t zavailable)
{
  bool z2zE2659;
  z2zE2659 = (zk == UINT64_C(0));
  if (z2zE2659) {  zSomezIU64zK((*(&z8zE375)), zGAS_COST_ZERO);  } else {
    uint64_t zdiscount;
    zdiscount = zmaxd;
    bool z2zE2660;
    z2zE2660 = (zk < UINT64_C(128));
    unit z3zE2812;
    if (z2zE2660) {
      uint64_t zindex;
      {    zindex = (UINT64_C(128) - zk);
      }
      uint64_t z2zE2661;
      z2zE2661 = fast_unsigned_vector_access_zz5vecz8z5bv16z9(ztable, zindex);
      zdiscount = ((uint64_t) z2zE2661);
      z3zE2812 = UNIT;
    } else {  z3zE2812 = UNIT;  }
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
    bool z2zE2662;
    z2zE2662 = u64_lt_u128(zavailable, zwhole_product);
    if (z2zE2662) {  zNonezIU64zK((*(&z8zE375)), UNIT);  } else {
      uint64_t zmajor;
      zmajor = u128_to_u64(zwhole_product);
      uint64_t ztail_product;
      {    ztail_product = (zcoefficient * zremainder);
      }
      uint64_t ztail;
      ztail = (ztail_product / UINT64_C(1000));
      sail_u128 ztotal;
      ztotal = u128_add_u64_u64(zmajor, ztail);
      bool z2zE2663;
      z2zE2663 = (!u64_lt_u128(zavailable, ztotal));
      if (z2zE2663) {
        uint64_t zaffordable;
        zaffordable = u128_to_u64(ztotal);
        zSomezIU64zK((*(&z8zE375)), zaffordable);
      } else {  zNonezIU64zK((*(&z8zE375)), UNIT);  }
    }
  }
end_function_2766: ;
  goto end_function_4070;
end_block_exception_2767: ;
  goto end_function_4070;
end_function_4070: ;
}

void zlinear_gas(struct zoptionzIU64zK *z8zE376, uint64_t zbase, uint64_t zper_unit, uint64_t zunits, uint64_t zavailable)
{
  sail_u128 zvariable_cost;
  zvariable_cost = u128_mul_u64_u64(zper_unit, zunits);
  sail_u128 zexact_cost;
  zexact_cost = u128_add_u64(zvariable_cost, zbase);
  bool z2zE2658;
  z2zE2658 = u64_lt_u128(zavailable, zexact_cost);
  if (z2zE2658) {  zNonezIU64zK((*(&z8zE376)), UNIT);  } else {
    uint64_t zaffordable;
    zaffordable = u128_to_u64(zexact_cost);
    zSomezIU64zK((*(&z8zE376)), zaffordable);
  }
end_function_2764: ;
  goto end_function_4069;
end_block_exception_2765: ;
  goto end_function_4069;
end_function_4069: ;
}

void zprecompile_gas(struct zoptionzIU64zK *z8zE377, uint64_t znum, struct zByteSliceFields zinput, uint64_t zavailable)
{
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  uint64_t zinput_length;
  zinput_length = zinput_len;
  uint64_t zwords;
  zwords = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zinput_len);
  struct zoptionzIU64zK z3zE2775;
  CREATE(zoptionzIU64zK)(&z3zE2775);
  {
    uint64_t zp0z3;
    zp0z3 = znum;
    bool z3zE2794;
    z3zE2794 = (zp0z3 == UINT64_C(1));
    if (!(z3zE2794)) {  goto case_2759;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(3000), zavailable);
    goto finish_match_2740;
  }
case_2759: ;
  {
    uint64_t z3zE2795;
    z3zE2795 = znum;
    bool z3zE2793;
    z3zE2793 = (z3zE2795 == UINT64_C(2));
    if (!(z3zE2793)) {  goto case_2758;  }
    zlinear_gas(&z3zE2775, UINT64_C(60), UINT64_C(12), zwords, zavailable);
    goto finish_match_2740;
  }
case_2758: ;
  {
    uint64_t z3zE2796;
    z3zE2796 = znum;
    bool z3zE2792;
    z3zE2792 = (z3zE2796 == UINT64_C(3));
    if (!(z3zE2792)) {  goto case_2757;  }
    zlinear_gas(&z3zE2775, UINT64_C(600), UINT64_C(120), zwords, zavailable);
    goto finish_match_2740;
  }
case_2757: ;
  {
    uint64_t z3zE2797;
    z3zE2797 = znum;
    bool z3zE2791;
    z3zE2791 = (z3zE2797 == UINT64_C(4));
    if (!(z3zE2791)) {  goto case_2756;  }
    zlinear_gas(&z3zE2775, UINT64_C(15), UINT64_C(3), zwords, zavailable);
    goto finish_match_2740;
  }
case_2756: ;
  {
    uint64_t z3zE2798;
    z3zE2798 = znum;
    bool z3zE2790;
    z3zE2790 = (z3zE2798 == UINT64_C(5));
    if (!(z3zE2790)) {  goto case_2755;  }
    zmodexp_gas(&z3zE2775, zinput, zavailable);
    goto finish_match_2740;
  }
case_2755: ;
  {
    uint64_t z3zE2799;
    z3zE2799 = znum;
    bool z3zE2789;
    z3zE2789 = (z3zE2799 == UINT64_C(6));
    if (!(z3zE2789)) {  goto case_2754;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(150), zavailable);
    goto finish_match_2740;
  }
case_2754: ;
  {
    uint64_t z3zE2800;
    z3zE2800 = znum;
    bool z3zE2788;
    z3zE2788 = (z3zE2800 == UINT64_C(7));
    if (!(z3zE2788)) {  goto case_2753;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(6000), zavailable);
    goto finish_match_2740;
  }
case_2753: ;
  {
    uint64_t z3zE2801;
    z3zE2801 = znum;
    bool z3zE2787;
    z3zE2787 = (z3zE2801 == UINT64_C(8));
    if (!(z3zE2787)) {  goto case_2752;  }
    uint64_t z2zE2652;
    z2zE2652 = (zinput_length / UINT64_C(192));
    zlinear_gas(&z3zE2775, UINT64_C(45000), UINT64_C(34000), z2zE2652, zavailable);
    goto finish_match_2740;
  }
case_2752: ;
  {
    uint64_t z3zE2802;
    z3zE2802 = znum;
    bool z3zE2786;
    z3zE2786 = (z3zE2802 == UINT64_C(9));
    if (!(z3zE2786)) {  goto case_2751;  }
    uint64_t z2zE2653;
    z2zE2653 = zpc_blake2_rounds(zinput);
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, z2zE2653, zavailable);
    goto finish_match_2740;
  }
case_2751: ;
  {
    uint64_t z3zE2803;
    z3zE2803 = znum;
    bool z3zE2785;
    z3zE2785 = (z3zE2803 == UINT64_C(10));
    if (!(z3zE2785)) {  goto case_2750;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(50000), zavailable);
    goto finish_match_2740;
  }
case_2750: ;
  {
    uint64_t z3zE2804;
    z3zE2804 = znum;
    bool z3zE2784;
    z3zE2784 = (z3zE2804 == UINT64_C(11));
    if (!(z3zE2784)) {  goto case_2749;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(375), zavailable);
    goto finish_match_2740;
  }
case_2749: ;
  {
    uint64_t z3zE2805;
    z3zE2805 = znum;
    bool z3zE2783;
    z3zE2783 = (z3zE2805 == UINT64_C(12));
    if (!(z3zE2783)) {  goto case_2748;  }
    uint64_t z2zE2654;
    z2zE2654 = (zinput_length / UINT64_C(160));
    zbls_msm_gas(&z3zE2775, zBLS_G1_DISCOUNT, UINT64_C(12000), UINT64_C(519), z2zE2654, zavailable);
    goto finish_match_2740;
  }
case_2748: ;
  {
    uint64_t z3zE2806;
    z3zE2806 = znum;
    bool z3zE2782;
    z3zE2782 = (z3zE2806 == UINT64_C(13));
    if (!(z3zE2782)) {  goto case_2747;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(600), zavailable);
    goto finish_match_2740;
  }
case_2747: ;
  {
    uint64_t z3zE2807;
    z3zE2807 = znum;
    bool z3zE2781;
    z3zE2781 = (z3zE2807 == UINT64_C(14));
    if (!(z3zE2781)) {  goto case_2746;  }
    uint64_t z2zE2655;
    z2zE2655 = (zinput_length / UINT64_C(288));
    zbls_msm_gas(&z3zE2775, zBLS_G2_DISCOUNT, UINT64_C(22500), UINT64_C(524), z2zE2655, zavailable);
    goto finish_match_2740;
  }
case_2746: ;
  {
    uint64_t z3zE2808;
    z3zE2808 = znum;
    bool z3zE2780;
    z3zE2780 = (z3zE2808 == UINT64_C(15));
    if (!(z3zE2780)) {  goto case_2745;  }
    uint64_t z2zE2656;
    z2zE2656 = (zinput_length / UINT64_C(384));
    zlinear_gas(&z3zE2775, UINT64_C(37700), UINT64_C(32600), z2zE2656, zavailable);
    goto finish_match_2740;
  }
case_2745: ;
  {
    uint64_t z3zE2809;
    z3zE2809 = znum;
    bool z3zE2779;
    z3zE2779 = (z3zE2809 == UINT64_C(16));
    if (!(z3zE2779)) {  goto case_2744;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(5500), zavailable);
    goto finish_match_2740;
  }
case_2744: ;
  {
    uint64_t z3zE2810;
    z3zE2810 = znum;
    bool z3zE2778;
    z3zE2778 = (z3zE2810 == UINT64_C(17));
    if (!(z3zE2778)) {  goto case_2743;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(23800), zavailable);
    goto finish_match_2740;
  }
case_2743: ;
  {
    uint64_t z3zE2811;
    z3zE2811 = znum;
    bool z3zE2777;
    z3zE2777 = (z3zE2811 == UINT64_C(256));
    if (!(z3zE2777)) {  goto case_2742;  }
    zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(&z3zE2775, UINT64_C(6900), zavailable);
    goto finish_match_2740;
  }
case_2742: ;
  {
    zSomezIU64zK(&z3zE2775, zGAS_COST_ZERO);
    goto finish_match_2740;
  }
case_2741: ;
finish_match_2740: ;
  COPY(zoptionzIU64zK)((*(&z8zE377)), z3zE2775);
  KILL(zoptionzIU64zK)(&z3zE2775);
end_function_2760: ;
  goto end_function_4068;
end_block_exception_2761: ;
  goto end_function_4068;
end_function_4068: ;
}

uint64_t zamsterdam_storage_access_cost(bool zcold)
{
  uint64_t z8zE378;
  if (zcold) {  z8zE378 = zG_amsterdam_cold_storage_access;  } else {  z8zE378 = zG_warm_access;  }
end_function_2738: ;
  return z8zE378;
end_block_exception_2739: ;

  return UINT64_C(0xdeadc0de);
}

struct zSstoreCosts zlegacy_sstore_costs(sail_u256 zoriginal, sail_u256 zcurrent, sail_u256 znew, bool zcold)
{
  struct zSstoreCosts z8zE379;
  uint64_t zcold_cost;
  if (zcold) {  zcold_cost = zG_cold_sload;  } else {  zcold_cost = zGAS_CONSTANT_ZERO;  }
  uint64_t zclear_refund;
  zclear_refund = zsstore_clear_refund(UNIT);
  uint64_t zbase;
  {
    bool z2zE2649;
    z2zE2649 = eq_u256(zcurrent, znew);
    if (z2zE2649) {  zbase = zG_warm_access;  } else {
      bool z2zE2650;
      z2zE2650 = eq_u256(zoriginal, zcurrent);
      if (z2zE2650) {
        bool z2zE2651;
        z2zE2651 = zword_is_zzero(zoriginal);
        if (z2zE2651) {  zbase = zG_sset;  } else {  zbase = zG_sreset;  }
      } else {  zbase = zG_warm_access;  }
    }
  }
  __int128 zrefund;
  zrefund = zGAS_REFUND_ZERO;
  bool z2zE2634;
  z2zE2634 = (!eq_u256(zcurrent, znew));
  unit z3zE2764;
  if (z2zE2634) {
    bool z2zE2635;
    z2zE2635 = eq_u256(zoriginal, zcurrent);
    if (z2zE2635) {
      bool z2zE2638;
      {
        bool z2zE2637;
        {
          bool z2zE2636;
          z2zE2636 = zword_is_zzero(zoriginal);
          z2zE2637 = not(z2zE2636);
        }
        bool z3zE2773;
        if (z2zE2637) {  z3zE2773 = zword_is_zzero(znew);  } else {  z3zE2773 = false;  }
        z2zE2638 = z3zE2773;
      }
      if (z2zE2638) {
        {
          zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zclear_refund);
          if (have_exception) {  goto end_block_exception_2737;  }
        }
        z3zE2764 = UNIT;
      } else {  z3zE2764 = UNIT;  }
    } else {
      bool z2zE2640;
      {
        bool z2zE2639;
        z2zE2639 = zword_is_zzero(zoriginal);
        z2zE2640 = not(z2zE2639);
      }
      unit z3zE2765;
      if (z2zE2640) {
        bool z2zE2641;
        z2zE2641 = zword_is_zzero(zcurrent);
        unit z3zE2766;
        if (z2zE2641) {
          int64_t z2zE2642;
          {
            int64_t z3zE2767;
            z3zE2767 = (int64_t)(zGAS_CONSTANT_ZERO);
            int64_t z3zE2768;
            z3zE2768 = (int64_t)(zclear_refund);
            z2zE2642 = (z3zE2767 - z3zE2768);
          }
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2642);
            if (have_exception) {  goto end_block_exception_2737;  }
          }
          z3zE2766 = UNIT;
        } else {  z3zE2766 = UNIT;  }
        bool z2zE2643;
        z2zE2643 = zword_is_zzero(znew);
        if (z2zE2643) {
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zclear_refund);
            if (have_exception) {  goto end_block_exception_2737;  }
          }
          z3zE2765 = UNIT;
        } else {  z3zE2765 = UNIT;  }
      } else {  z3zE2765 = UNIT;  }
      bool z2zE2644;
      z2zE2644 = eq_u256(zoriginal, znew);
      if (z2zE2644) {
        bool z2zE2645;
        z2zE2645 = zword_is_zzero(zoriginal);
        if (z2zE2645) {
          int64_t z2zE2646;
          {
            int64_t z3zE2771;
            z3zE2771 = (int64_t)(zG_sset);
            int64_t z3zE2772;
            z3zE2772 = (int64_t)(zG_warm_access);
            z2zE2646 = (z3zE2771 - z3zE2772);
          }
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2646);
            if (have_exception) {  goto end_block_exception_2737;  }
          }
          z3zE2764 = UNIT;
        } else {
          int64_t z2zE2647;
          {
            int64_t z3zE2769;
            z3zE2769 = (int64_t)(zG_sreset);
            int64_t z3zE2770;
            z3zE2770 = (int64_t)(zG_warm_access);
            z2zE2647 = (z3zE2769 - z3zE2770);
          }
          {
            zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2647);
            if (have_exception) {  goto end_block_exception_2737;  }
          }
          z3zE2764 = UNIT;
        }
      } else {  z3zE2764 = UNIT;  }
    }
  } else {  z3zE2764 = UNIT;  }
  uint64_t z2zE2648;
  {    z2zE2648 = (zbase + zcold_cost);
  }
  struct zSstoreCosts z3zE2774;
  z3zE2774.zexecution = z2zE2648;
  z3zE2774.zrefund = zrefund;
  z3zE2774.zstate_charge = UINT64_C(0);
  z3zE2774.zstate_credit = UINT64_C(0);
  z8zE379 = z3zE2774;
end_function_2736: ;
  return z8zE379;
end_block_exception_2737: ;
  struct zSstoreCosts z8zE917 = { .zexecution = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_charge = UINT64_C(0xdeadc0de), .zstate_credit = UINT64_C(0xdeadc0de) };
  return z8zE917;
}

struct zSstoreCosts zamsterdam_sstore_costs(sail_u256 zoriginal, sail_u256 zcurrent, sail_u256 znew, bool zcold)
{
  struct zSstoreCosts z8zE380;
  bool zchanged;
  zchanged = (!eq_u256(zcurrent, znew));
  bool zclean_change;
  {
    bool z2zE2633;
    z2zE2633 = eq_u256(zoriginal, zcurrent);
    bool z3zE2752;
    if (z2zE2633) {  z3zE2752 = zchanged;  } else {  z3zE2752 = false;  }
    zclean_change = z3zE2752;
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
    bool z2zE2632;
    {
      bool z3zE2753;
      if (zclean_change) {  z3zE2753 = zword_is_zzero(zoriginal);  } else {  z3zE2753 = false;  }
      z2zE2632 = z3zE2753;
    }
    if (z2zE2632) {  zstate_charge = zG_amsterdam_state_storage_set;  } else {  zstate_charge = zGAS_COST_ZERO;  }
  }
  uint64_t zstate_credit;
  zstate_credit = UINT64_C(0);
  unit z3zE2754;
  if (zchanged) {
    bool z2zE2625;
    {
      bool z2zE2624;
      {
        bool z2zE2621;
        z2zE2621 = zword_is_zzero(zoriginal);
        z2zE2624 = not(z2zE2621);
      }
      bool z3zE2756;
      if (z2zE2624) {
        bool z2zE2623;
        {
          bool z2zE2622;
          z2zE2622 = zword_is_zzero(zcurrent);
          z2zE2623 = not(z2zE2622);
        }
        bool z3zE2755;
        if (z2zE2623) {  z3zE2755 = zword_is_zzero(znew);  } else {  z3zE2755 = false;  }
        z3zE2756 = z3zE2755;
      } else {  z3zE2756 = false;  }
      z2zE2625 = z3zE2756;
    }
    unit z3zE2761;
    if (z2zE2625) {
      {
        zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zR_amsterdam_storage_clear);
        if (have_exception) {  goto end_block_exception_2735;  }
      }
      z3zE2761 = UNIT;
    } else {  z3zE2761 = UNIT;  }
    bool z2zE2628;
    {
      bool z2zE2627;
      {
        bool z2zE2626;
        z2zE2626 = zword_is_zzero(zoriginal);
        z2zE2627 = not(z2zE2626);
      }
      bool z3zE2757;
      if (z2zE2627) {  z3zE2757 = zword_is_zzero(zcurrent);  } else {  z3zE2757 = false;  }
      z2zE2628 = z3zE2757;
    }
    unit z3zE2758;
    if (z2zE2628) {
      int64_t z2zE2629;
      {
        int64_t z3zE2759;
        z3zE2759 = (int64_t)(zGAS_CONSTANT_ZERO);
        int64_t z3zE2760;
        z3zE2760 = (int64_t)(zR_amsterdam_storage_clear);
        z2zE2629 = (z3zE2759 - z3zE2760);
      }
      {
        zrefund = zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, z2zE2629);
        if (have_exception) {  goto end_block_exception_2735;  }
      }
      z3zE2758 = UNIT;
    } else {  z3zE2758 = UNIT;  }
    bool z2zE2630;
    z2zE2630 = eq_u256(zoriginal, znew);
    if (z2zE2630) {
      {
        zrefund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zrefund, zG_amsterdam_storage_write);
        if (have_exception) {  goto end_block_exception_2735;  }
      }
      unit z3zE2762;
      z3zE2762 = UNIT;
      bool z2zE2631;
      z2zE2631 = zword_is_zzero(zoriginal);
      if (z2zE2631) {
        zstate_credit = zG_amsterdam_state_storage_set;
        z3zE2754 = UNIT;
      } else {  z3zE2754 = UNIT;  }
    } else {  z3zE2754 = UNIT;  }
  } else {  z3zE2754 = UNIT;  }
  struct zSstoreCosts z3zE2763;
  z3zE2763.zexecution = zexecution;
  z3zE2763.zrefund = zrefund;
  z3zE2763.zstate_charge = zstate_charge;
  z3zE2763.zstate_credit = zstate_credit;
  z8zE380 = z3zE2763;
end_function_2734: ;
  return z8zE380;
end_block_exception_2735: ;
  struct zSstoreCosts z8zE918 = { .zexecution = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_charge = UINT64_C(0xdeadc0de), .zstate_credit = UINT64_C(0xdeadc0de) };
  return z8zE918;
}

struct zSstoreCosts zsstore_costs(sail_u256 zoriginal, sail_u256 zcurrent, sail_u256 znew, bool zcold)
{
  struct zSstoreCosts z8zE381;
  bool z2zE2620;
  z2zE2620 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2620) {
    {
      z8zE381 = zamsterdam_sstore_costs(zoriginal, zcurrent, znew, zcold);
      if (have_exception) {  goto end_block_exception_2733;  }
    }
  } else {
    {
      z8zE381 = zlegacy_sstore_costs(zoriginal, zcurrent, znew, zcold);
      if (have_exception) {  goto end_block_exception_2733;  }
    }
  }
end_function_2732: ;
  return z8zE381;
end_block_exception_2733: ;
  struct zSstoreCosts z8zE919 = { .zexecution = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_charge = UINT64_C(0xdeadc0de), .zstate_credit = UINT64_C(0xdeadc0de) };
  return z8zE919;
}

unit zcharge_word_scaled_gas(uint64_t zper_unit, sail_u256 zunits)
{
  unit z8zE382;
  bool z2zE2617;
  {
    bool z2zE2616;
    {
      bool z2zE2614;
      z2zE2614 = zis_running(UNIT);
      z2zE2616 = not(z2zE2614);
    }
    bool z3zE2750;
    if (z2zE2616) {  z3zE2750 = true;  } else {
      bool z2zE2615;
      z2zE2615 = (zper_unit == UINT64_C(0));
      bool z3zE2749;
      if (z2zE2615) {  z3zE2749 = true;  } else {  z3zE2749 = u256_eq_u64(zunits, UINT64_C(0));  }
      z3zE2750 = z3zE2749;
    }
    z2zE2617 = z3zE2750;
  }
  unit z3zE2751;
  if (z2zE2617) {
    z8zE382 = UNIT;
    goto cleanup_2729;
    /* unreachable after return */
    goto end_cleanup_2730;
  cleanup_2729: ;
    goto end_function_2728;
  end_cleanup_2730: ;
  } else {  z3zE2751 = UNIT;  }
  uint64_t zremaining;
  zremaining = zgas_remaining;
  bool z2zE2618;
  z2zE2618 = (!u64_lt_u256(zremaining, zunits));
  if (z2zE2618) {
    uint64_t zaffordable_units;
    zaffordable_units = u256_to_u64(zunits);
    sail_u128 zexact_cost;
    zexact_cost = u128_mul_u64_u64(zper_unit, zaffordable_units);
    bool z2zE2619;
    z2zE2619 = (!u64_lt_u128(zremaining, zexact_cost));
    if (z2zE2619) {
      uint64_t zcost;
      zcost = u128_to_u64(zexact_cost);
      {
        z8zE382 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcost);
        if (have_exception) {  goto end_block_exception_2731;  }
      }
    } else {
      {
        z8zE382 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2731;  }
      }
    }
  } else {
    {
      z8zE382 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2731;  }
    }
  }
end_function_2728: ;
  return z8zE382;
end_block_exception_2731: ;

  return UNIT;
}

unit zcharge_memory_word_gas(uint64_t zbase, uint64_t zper_word, sail_u256 zsizze)
{
  unit z8zE383;
  unit z3zE2748;
  {
    z3zE2748 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase);
    if (have_exception) {  goto end_block_exception_2727;  }
  }
  sail_u256 z2zE2613;
  z2zE2613 = zmemory_word_count_word(zsizze);
  {
    z8zE383 = zcharge_word_scaled_gas(zper_word, z2zE2613);
    if (have_exception) {  goto end_block_exception_2727;  }
  }
end_function_2726: ;
  return z8zE383;
end_block_exception_2727: ;

  return UNIT;
}

unit zcharge_keccak_gas(sail_u256 zsizze)
{
  unit z8zE384;
  {
    z8zE384 = zcharge_memory_word_gas(zG_keccak, zG_keccak_word, zsizze);
    if (have_exception) {  goto end_block_exception_2725;  }
  }
end_function_2724: ;
  return z8zE384;
end_block_exception_2725: ;

  return UNIT;
}

unit zcharge_copy_gas(sail_u256 zsizze)
{
  unit z8zE385;
  {
    z8zE385 = zcharge_memory_word_gas(zGAS_CONSTANT_ZERO, zG_copy_word, zsizze);
    if (have_exception) {  goto end_block_exception_2723;  }
  }
end_function_2722: ;
  return z8zE385;
end_block_exception_2723: ;

  return UNIT;
}

unit zcharge_log_gas(uint64_t znum_topics, sail_u256 zsizze)
{
  unit z8zE386;
  unit z3zE2746;
  {
    z3zE2746 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_log);
    if (have_exception) {  goto end_block_exception_2721;  }
  }
  uint64_t ztopic_cost;
  {    ztopic_cost = (zG_logtopic * znum_topics);
  }
  unit z3zE2747;
  {
    z3zE2747 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(ztopic_cost);
    if (have_exception) {  goto end_block_exception_2721;  }
  }
  {
    z8zE386 = zcharge_word_scaled_gas(zG_logdata, zsizze);
    if (have_exception) {  goto end_block_exception_2721;  }
  }
end_function_2720: ;
  return z8zE386;
end_block_exception_2721: ;

  return UNIT;
}

uint64_t zexp_gas(sail_u256 zexponent)
{
  uint64_t z8zE387;
  uint64_t z2zE2612;
  {
    uint64_t z2zE2611;
    z2zE2611 = zword_byte_length(zexponent);
    {    z2zE2612 = (zG_expbyte * z2zE2611);
    }
  }
  {    z8zE387 = (z2zE2612 + zG_exp);
  }
end_function_2718: ;
  return z8zE387;
end_block_exception_2719: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_initcode_gas(uint64_t zbyte_len)
{
  uint64_t z8zE388;
  bool z2zE2610;
  z2zE2610 = zfork_gteq(zk_fork, zShanghai);
  if (z2zE2610) {
    uint64_t zwords;
    zwords = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbyte_len);
    {    z8zE388 = (zwords + zwords);
    }
  } else {  z8zE388 = UINT64_C(0);  }
end_function_2716: ;
  return z8zE388;
end_block_exception_2717: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zcall_gas_cap_word(uint64_t zavailable, sail_u256 zrequested)
{
  uint64_t z8zE389;
  uint64_t zretained;
  zretained = (zavailable / UINT64_C(64));
  uint64_t zall_but_64th;
  {
    zall_but_64th = zgas_sub_or_oog(zavailable, zretained);
    if (have_exception) {  goto end_block_exception_2715;  }
  }
  bool z2zE2609;
  z2zE2609 = u256_lt_u64(zrequested, zall_but_64th);
  if (z2zE2609) {  z8zE389 = u256_to_u64(zrequested);  } else {  z8zE389 = zall_but_64th;  }
end_function_2714: ;
  return z8zE389;
end_block_exception_2715: ;

  return UINT64_C(0xdeadc0de);
}

