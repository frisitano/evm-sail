/* Generated from sail/evm/execute.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_20 zself_addr(unit z3zE2026)
{
  sail_fixed_bytes_20 z8zE382;
  z8zE382 = zmessage.zaddress;
end_function_2338: ;
  return z8zE382;
end_block_exception_2339: ;

  return fixed_bytes_20_zero();
}

bool zguard_static(unit z3zE2024)
{
  bool z8zE383;
  bool z2zE2006;
  z2zE2006 = zmessage.zis_static;
  if (z2zE2006) {
    unit z3zE2025;
    {
      z3zE2025 = zexc_halt(zWriteProtection);
      if (have_exception) {  goto end_block_exception_2337;  }
    }
    z8zE383 = true;
  } else {  z8zE383 = false;  }
end_function_2336: ;
  return z8zE383;
end_block_exception_2337: ;

  return false;
}

unit zdo_jump(sail_u256 zdestination_value)
{
  unit z8zE384;
  uint64_t zcode_length;
  zcode_length = zframe_code_len(UNIT);
  bool z2zE2004;
  z2zE2004 = u256_lt_u64(zdestination_value, zcode_length);
  if (z2zE2004) {
    uint64_t zdestination;
    zdestination = u256_to_u64(zdestination_value);
    bool z2zE2005;
    z2zE2005 = zframe_jumpdest_valid(zdestination);
    if (z2zE2005) {
      zpc = zdestination;
      z8zE384 = UNIT;
    } else {
      {
        z8zE384 = zexc_halt(zInvalidJump);
        if (have_exception) {  goto end_block_exception_2335;  }
      }
    }
  } else {
    {
      z8zE384 = zexc_halt(zInvalidJump);
      if (have_exception) {  goto end_block_exception_2335;  }
    }
  }
end_function_2334: ;
  return z8zE384;
end_block_exception_2335: ;

  return UNIT;
}

void zpop_log_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9 *z8zE385, uint64_t zcount)
{
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z3zE2002;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
  {
    uint64_t zp0z3;
    zp0z3 = zcount;
    bool z3zE2014;
    z3zE2014 = (zp0z3 == UINT64_C(0));
    if (!(z3zE2014)) {  goto case_2331;  }
    goto finish_match_2325;
  }
case_2331: ;
  {
    uint64_t z3zE2015;
    z3zE2015 = zcount;
    bool z3zE2012;
    z3zE2012 = (z3zE2015 == UINT64_C(1));
    if (!(z3zE2012)) {  goto case_2330;  }
    sail_u256 z2zE2003;
    {
      z2zE2003 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, z2zE2003, z3zE2002);
    goto finish_match_2325;
  }
case_2330: ;
  {
    uint64_t z3zE2016;
    z3zE2016 = zcount;
    bool z3zE2010;
    z3zE2010 = (z3zE2016 == UINT64_C(2));
    if (!(z3zE2010)) {  goto case_2329;  }
    sail_u256 zt0;
    {
      zt0 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    sail_u256 zt1;
    {
      zt1 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, zt1, z3zE2002);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, zt0, z3zE2002);
    goto finish_match_2325;
  }
case_2329: ;
  {
    uint64_t z3zE2017;
    z3zE2017 = zcount;
    bool z3zE2008;
    z3zE2008 = (z3zE2017 == UINT64_C(3));
    if (!(z3zE2008)) {  goto case_2328;  }
    sail_u256 z3zE2018;
    {
      z3zE2018 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    sail_u256 z3zE2019;
    {
      z3zE2019 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    sail_u256 zt2;
    {
      zt2 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, zt2, z3zE2002);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, z3zE2019, z3zE2002);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, z3zE2018, z3zE2002);
    goto finish_match_2325;
  }
case_2328: ;
  {
    uint64_t z3zE2020;
    z3zE2020 = zcount;
    bool z3zE2006;
    z3zE2006 = (z3zE2020 == UINT64_C(4));
    if (!(z3zE2006)) {  goto case_2327;  }
    sail_u256 z3zE2021;
    {
      z3zE2021 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    sail_u256 z3zE2022;
    {
      z3zE2022 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    sail_u256 z3zE2023;
    {
      z3zE2023 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    sail_u256 zt3;
    {
      zt3 = zpop(UNIT);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
        goto end_block_exception_2333;
      }
    }
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, zt3, z3zE2002);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, z3zE2023, z3zE2002);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, z3zE2022, z3zE2002);
    zconsz3z5structz0zz__sail_c_repr_u256(&z3zE2002, z3zE2021, z3zE2002);
    goto finish_match_2325;
  }
case_2327: ;
  {    goto finish_match_2325;
  }
case_2326: ;
finish_match_2325: ;
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)((*(&z8zE385)), z3zE2002);
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z3zE2002);
end_function_2332: ;
  goto end_function_3635;
end_block_exception_2333: ;
  goto end_function_3635;
end_function_3635: ;
}

unit zexecute_add(unit z3zE2000)
{
  unit z8zE386;
  unit z3zE2001;
  {
    z3zE2001 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2324;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2324;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2324;  }
  }
  sail_u256 z2zE2002;
  z2zE2002 = zalu_add(za, zb);
  {
    z8zE386 = zpush_word(z2zE2002);
    if (have_exception) {  goto end_block_exception_2324;  }
  }
end_function_2323: ;
  return z8zE386;
end_block_exception_2324: ;

  return UNIT;
}

unit zexecute_mul(unit z3zE1998)
{
  unit z8zE387;
  unit z3zE1999;
  {
    z3zE1999 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2322;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2322;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2322;  }
  }
  sail_u256 z2zE2001;
  z2zE2001 = zalu_mul(za, zb);
  {
    z8zE387 = zpush_word(z2zE2001);
    if (have_exception) {  goto end_block_exception_2322;  }
  }
end_function_2321: ;
  return z8zE387;
end_block_exception_2322: ;

  return UNIT;
}

unit zexecute_sub(unit z3zE1996)
{
  unit z8zE388;
  unit z3zE1997;
  {
    z3zE1997 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2320;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2320;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2320;  }
  }
  sail_u256 z2zE2000;
  z2zE2000 = zalu_sub(za, zb);
  {
    z8zE388 = zpush_word(z2zE2000);
    if (have_exception) {  goto end_block_exception_2320;  }
  }
end_function_2319: ;
  return z8zE388;
end_block_exception_2320: ;

  return UNIT;
}

unit zexecute_div(unit z3zE1994)
{
  unit z8zE389;
  unit z3zE1995;
  {
    z3zE1995 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2318;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2318;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2318;  }
  }
  sail_u256 z2zE1999;
  z2zE1999 = zalu_div(za, zb);
  {
    z8zE389 = zpush_word(z2zE1999);
    if (have_exception) {  goto end_block_exception_2318;  }
  }
end_function_2317: ;
  return z8zE389;
end_block_exception_2318: ;

  return UNIT;
}

unit zexecute_sdiv(unit z3zE1992)
{
  unit z8zE390;
  unit z3zE1993;
  {
    z3zE1993 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2316;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2316;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2316;  }
  }
  sail_u256 z2zE1998;
  z2zE1998 = zalu_sdiv(za, zb);
  {
    z8zE390 = zpush_word(z2zE1998);
    if (have_exception) {  goto end_block_exception_2316;  }
  }
end_function_2315: ;
  return z8zE390;
end_block_exception_2316: ;

  return UNIT;
}

unit zexecute_mod(unit z3zE1990)
{
  unit z8zE391;
  unit z3zE1991;
  {
    z3zE1991 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2314;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2314;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2314;  }
  }
  sail_u256 z2zE1997;
  z2zE1997 = zalu_mod(za, zb);
  {
    z8zE391 = zpush_word(z2zE1997);
    if (have_exception) {  goto end_block_exception_2314;  }
  }
end_function_2313: ;
  return z8zE391;
end_block_exception_2314: ;

  return UNIT;
}

unit zexecute_smod(unit z3zE1988)
{
  unit z8zE392;
  unit z3zE1989;
  {
    z3zE1989 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2312;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2312;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2312;  }
  }
  sail_u256 z2zE1996;
  z2zE1996 = zalu_smod(za, zb);
  {
    z8zE392 = zpush_word(z2zE1996);
    if (have_exception) {  goto end_block_exception_2312;  }
  }
end_function_2311: ;
  return z8zE392;
end_block_exception_2312: ;

  return UNIT;
}

unit zexecute_addmod(unit z3zE1986)
{
  unit z8zE393;
  unit z3zE1987;
  {
    z3zE1987 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
    if (have_exception) {  goto end_block_exception_2310;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2310;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2310;  }
  }
  sail_u256 zn;
  {
    zn = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2310;  }
  }
  sail_u256 z2zE1995;
  z2zE1995 = zalu_addmod(za, zb, zn);
  {
    z8zE393 = zpush_word(z2zE1995);
    if (have_exception) {  goto end_block_exception_2310;  }
  }
end_function_2309: ;
  return z8zE393;
end_block_exception_2310: ;

  return UNIT;
}

unit zexecute_mulmod(unit z3zE1984)
{
  unit z8zE394;
  unit z3zE1985;
  {
    z3zE1985 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
    if (have_exception) {  goto end_block_exception_2308;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2308;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2308;  }
  }
  sail_u256 zn;
  {
    zn = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2308;  }
  }
  sail_u256 z2zE1994;
  z2zE1994 = zalu_mulmod(za, zb, zn);
  {
    z8zE394 = zpush_word(z2zE1994);
    if (have_exception) {  goto end_block_exception_2308;  }
  }
end_function_2307: ;
  return z8zE394;
end_block_exception_2308: ;

  return UNIT;
}

unit zexecute_exp(unit z3zE1982)
{
  unit z8zE395;
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2306;  }
  }
  sail_u256 ze;
  {
    ze = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2306;  }
  }
  uint64_t z2zE1992;
  z2zE1992 = zexp_gas(ze);
  unit z3zE1983;
  {
    z3zE1983 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1992);
    if (have_exception) {  goto end_block_exception_2306;  }
  }
  sail_u256 z2zE1993;
  z2zE1993 = zalu_exp(za, ze);
  {
    z8zE395 = zpush_word(z2zE1993);
    if (have_exception) {  goto end_block_exception_2306;  }
  }
end_function_2305: ;
  return z8zE395;
end_block_exception_2306: ;

  return UNIT;
}

unit zexecute_signextend(unit z3zE1980)
{
  unit z8zE396;
  unit z3zE1981;
  {
    z3zE1981 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2304;  }
  }
  sail_u256 zbi;
  {
    zbi = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2304;  }
  }
  sail_u256 zv;
  {
    zv = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2304;  }
  }
  sail_u256 z2zE1991;
  z2zE1991 = zalu_signextend(zbi, zv);
  {
    z8zE396 = zpush_word(z2zE1991);
    if (have_exception) {  goto end_block_exception_2304;  }
  }
end_function_2303: ;
  return z8zE396;
end_block_exception_2304: ;

  return UNIT;
}

unit zexecute_lt(unit z3zE1978)
{
  unit z8zE397;
  unit z3zE1979;
  {
    z3zE1979 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2302;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2302;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2302;  }
  }
  sail_u256 z2zE1990;
  z2zE1990 = zalu_lt(za, zb);
  {
    z8zE397 = zpush_word(z2zE1990);
    if (have_exception) {  goto end_block_exception_2302;  }
  }
end_function_2301: ;
  return z8zE397;
end_block_exception_2302: ;

  return UNIT;
}

unit zexecute_gt(unit z3zE1976)
{
  unit z8zE398;
  unit z3zE1977;
  {
    z3zE1977 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2300;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2300;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2300;  }
  }
  sail_u256 z2zE1989;
  z2zE1989 = zalu_gt(za, zb);
  {
    z8zE398 = zpush_word(z2zE1989);
    if (have_exception) {  goto end_block_exception_2300;  }
  }
end_function_2299: ;
  return z8zE398;
end_block_exception_2300: ;

  return UNIT;
}

unit zexecute_slt(unit z3zE1974)
{
  unit z8zE399;
  unit z3zE1975;
  {
    z3zE1975 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2298;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2298;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2298;  }
  }
  sail_u256 z2zE1988;
  z2zE1988 = zalu_slt(za, zb);
  {
    z8zE399 = zpush_word(z2zE1988);
    if (have_exception) {  goto end_block_exception_2298;  }
  }
end_function_2297: ;
  return z8zE399;
end_block_exception_2298: ;

  return UNIT;
}

unit zexecute_sgt(unit z3zE1972)
{
  unit z8zE400;
  unit z3zE1973;
  {
    z3zE1973 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2296;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2296;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2296;  }
  }
  sail_u256 z2zE1987;
  z2zE1987 = zalu_sgt(za, zb);
  {
    z8zE400 = zpush_word(z2zE1987);
    if (have_exception) {  goto end_block_exception_2296;  }
  }
end_function_2295: ;
  return z8zE400;
end_block_exception_2296: ;

  return UNIT;
}

unit zexecute_eq(unit z3zE1970)
{
  unit z8zE401;
  unit z3zE1971;
  {
    z3zE1971 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2294;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2294;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2294;  }
  }
  sail_u256 z2zE1986;
  z2zE1986 = zalu_eq(za, zb);
  {
    z8zE401 = zpush_word(z2zE1986);
    if (have_exception) {  goto end_block_exception_2294;  }
  }
end_function_2293: ;
  return z8zE401;
end_block_exception_2294: ;

  return UNIT;
}

unit zexecute_iszzero(unit z3zE1968)
{
  unit z8zE402;
  unit z3zE1969;
  {
    z3zE1969 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2292;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2292;  }
  }
  sail_u256 z2zE1985;
  z2zE1985 = zalu_iszzero(za);
  {
    z8zE402 = zpush_word(z2zE1985);
    if (have_exception) {  goto end_block_exception_2292;  }
  }
end_function_2291: ;
  return z8zE402;
end_block_exception_2292: ;

  return UNIT;
}

unit zexecute_and(unit z3zE1966)
{
  unit z8zE403;
  unit z3zE1967;
  {
    z3zE1967 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2290;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2290;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2290;  }
  }
  sail_u256 z2zE1984;
  z2zE1984 = zalu_and(za, zb);
  {
    z8zE403 = zpush_word(z2zE1984);
    if (have_exception) {  goto end_block_exception_2290;  }
  }
end_function_2289: ;
  return z8zE403;
end_block_exception_2290: ;

  return UNIT;
}

unit zexecute_or(unit z3zE1964)
{
  unit z8zE404;
  unit z3zE1965;
  {
    z3zE1965 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2288;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2288;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2288;  }
  }
  sail_u256 z2zE1983;
  z2zE1983 = zalu_or(za, zb);
  {
    z8zE404 = zpush_word(z2zE1983);
    if (have_exception) {  goto end_block_exception_2288;  }
  }
end_function_2287: ;
  return z8zE404;
end_block_exception_2288: ;

  return UNIT;
}

unit zexecute_xor(unit z3zE1962)
{
  unit z8zE405;
  unit z3zE1963;
  {
    z3zE1963 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2286;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2286;  }
  }
  sail_u256 zb;
  {
    zb = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2286;  }
  }
  sail_u256 z2zE1982;
  z2zE1982 = zalu_xor(za, zb);
  {
    z8zE405 = zpush_word(z2zE1982);
    if (have_exception) {  goto end_block_exception_2286;  }
  }
end_function_2285: ;
  return z8zE405;
end_block_exception_2286: ;

  return UNIT;
}

unit zexecute_not(unit z3zE1960)
{
  unit z8zE406;
  unit z3zE1961;
  {
    z3zE1961 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2284;  }
  }
  sail_u256 za;
  {
    za = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2284;  }
  }
  sail_u256 z2zE1981;
  z2zE1981 = zalu_not(za);
  {
    z8zE406 = zpush_word(z2zE1981);
    if (have_exception) {  goto end_block_exception_2284;  }
  }
end_function_2283: ;
  return z8zE406;
end_block_exception_2284: ;

  return UNIT;
}

unit zexecute_byte(unit z3zE1958)
{
  unit z8zE407;
  unit z3zE1959;
  {
    z3zE1959 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2282;  }
  }
  sail_u256 zi;
  {
    zi = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2282;  }
  }
  sail_u256 zx;
  {
    zx = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2282;  }
  }
  sail_u256 z2zE1980;
  z2zE1980 = zalu_byte(zi, zx);
  {
    z8zE407 = zpush_word(z2zE1980);
    if (have_exception) {  goto end_block_exception_2282;  }
  }
end_function_2281: ;
  return z8zE407;
end_block_exception_2282: ;

  return UNIT;
}

unit zexecute_shl(unit z3zE1956)
{
  unit z8zE408;
  unit z3zE1957;
  {
    z3zE1957 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2280;  }
  }
  sail_u256 zs;
  {
    zs = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2280;  }
  }
  sail_u256 zv;
  {
    zv = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2280;  }
  }
  sail_u256 z2zE1979;
  z2zE1979 = zalu_shl(zs, zv);
  {
    z8zE408 = zpush_word(z2zE1979);
    if (have_exception) {  goto end_block_exception_2280;  }
  }
end_function_2279: ;
  return z8zE408;
end_block_exception_2280: ;

  return UNIT;
}

unit zexecute_shr(unit z3zE1954)
{
  unit z8zE409;
  unit z3zE1955;
  {
    z3zE1955 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2278;  }
  }
  sail_u256 zs;
  {
    zs = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2278;  }
  }
  sail_u256 zv;
  {
    zv = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2278;  }
  }
  sail_u256 z2zE1978;
  z2zE1978 = zalu_shr(zs, zv);
  {
    z8zE409 = zpush_word(z2zE1978);
    if (have_exception) {  goto end_block_exception_2278;  }
  }
end_function_2277: ;
  return z8zE409;
end_block_exception_2278: ;

  return UNIT;
}

unit zexecute_sar(unit z3zE1952)
{
  unit z8zE410;
  unit z3zE1953;
  {
    z3zE1953 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2276;  }
  }
  sail_u256 zs;
  {
    zs = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2276;  }
  }
  sail_u256 zv;
  {
    zv = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2276;  }
  }
  sail_u256 z2zE1977;
  z2zE1977 = zalu_sar(zs, zv);
  {
    z8zE410 = zpush_word(z2zE1977);
    if (have_exception) {  goto end_block_exception_2276;  }
  }
end_function_2275: ;
  return z8zE410;
end_block_exception_2276: ;

  return UNIT;
}

unit zexecute_clzz(unit z3zE1950)
{
  unit z8zE411;
  unit z3zE1951;
  {
    z3zE1951 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2274;  }
  }
  sail_u256 zx;
  {
    zx = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2274;  }
  }
  sail_u256 z2zE1976;
  z2zE1976 = zalu_clzz(zx);
  {
    z8zE411 = zpush_word(z2zE1976);
    if (have_exception) {  goto end_block_exception_2274;  }
  }
end_function_2273: ;
  return z8zE411;
end_block_exception_2274: ;

  return UNIT;
}

unit zexecute_keccak256(unit z3zE1948)
{
  unit z8zE412;
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2272;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2272;  }
  }
  unit z3zE1949;
  {
    z3zE1949 = zcharge_keccak_gas(zlength_word);
    if (have_exception) {  goto end_block_exception_2272;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zoffset_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2272;  }
  }
  bool z2zE1974;
  z2zE1974 = zis_running(UNIT);
  if (z2zE1974) {
    sail_u256 z2zE1975;
    z2zE1975 = zmem_keccak(zrange);
    {
      z8zE412 = zpush_word(z2zE1975);
      if (have_exception) {  goto end_block_exception_2272;  }
    }
  } else {  z8zE412 = UNIT;  }
end_function_2271: ;
  return z8zE412;
end_block_exception_2272: ;

  return UNIT;
}

unit zexecute_address(unit z3zE1946)
{
  unit z8zE413;
  unit z3zE1947;
  {
    z3zE1947 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2270;  }
  }
  sail_u256 z2zE1973;
  {
    sail_fixed_bytes_20 z2zE1972;
    z2zE1972 = zself_addr(UNIT);
    z2zE1973 = evmsail_address_to_word(z2zE1972);
  }
  {
    z8zE413 = zpush_word(z2zE1973);
    if (have_exception) {  goto end_block_exception_2270;  }
  }
end_function_2269: ;
  return z8zE413;
end_block_exception_2270: ;

  return UNIT;
}

unit zexecute_origin(unit z3zE1944)
{
  unit z8zE414;
  unit z3zE1945;
  {
    z3zE1945 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2268;  }
  }
  sail_u256 z2zE1971;
  z2zE1971 = zk_env(zF_Origin);
  {
    z8zE414 = zpush_word(z2zE1971);
    if (have_exception) {  goto end_block_exception_2268;  }
  }
end_function_2267: ;
  return z8zE414;
end_block_exception_2268: ;

  return UNIT;
}

unit zexecute_caller(unit z3zE1942)
{
  unit z8zE415;
  unit z3zE1943;
  {
    z3zE1943 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2266;  }
  }
  sail_u256 z2zE1970;
  {
    sail_fixed_bytes_20 z2zE1969;
    z2zE1969 = zmessage.zcaller;
    z2zE1970 = evmsail_address_to_word(z2zE1969);
  }
  {
    z8zE415 = zpush_word(z2zE1970);
    if (have_exception) {  goto end_block_exception_2266;  }
  }
end_function_2265: ;
  return z8zE415;
end_block_exception_2266: ;

  return UNIT;
}

unit zexecute_callvalue(unit z3zE1940)
{
  unit z8zE416;
  unit z3zE1941;
  {
    z3zE1941 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2264;  }
  }
  sail_u256 z2zE1968;
  z2zE1968 = zmessage.zvalue;
  {
    z8zE416 = zpush_word(z2zE1968);
    if (have_exception) {  goto end_block_exception_2264;  }
  }
end_function_2263: ;
  return z8zE416;
end_block_exception_2264: ;

  return UNIT;
}

unit zexecute_gasprice(unit z3zE1938)
{
  unit z8zE417;
  unit z3zE1939;
  {
    z3zE1939 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2262;  }
  }
  sail_u256 z2zE1967;
  z2zE1967 = zk_env(zF_GasPrice);
  {
    z8zE417 = zpush_word(z2zE1967);
    if (have_exception) {  goto end_block_exception_2262;  }
  }
end_function_2261: ;
  return z8zE417;
end_block_exception_2262: ;

  return UNIT;
}

unit zexecute_calldatasizze(unit z3zE1936)
{
  unit z8zE418;
  unit z3zE1937;
  {
    z3zE1937 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2260;  }
  }
  struct zByteSliceFields zinput;
  zinput = zcalldata;
  sail_u256 z2zE1966;
  {
    uint64_t z2zE1965;
    z2zE1965 = zinput.zlen;
    z2zE1966 = zword_of_source_byte_count(z2zE1965);
  }
  {
    z8zE418 = zpush_word(z2zE1966);
    if (have_exception) {  goto end_block_exception_2260;  }
  }
end_function_2259: ;
  return z8zE418;
end_block_exception_2260: ;

  return UNIT;
}

unit zexecute_calldataload(unit z3zE1934)
{
  unit z8zE419;
  unit z3zE1935;
  {
    z3zE1935 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2258;  }
  }
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2258;  }
  }
  bool z2zE1963;
  z2zE1963 = zis_running(UNIT);
  if (z2zE1963) {
    sail_u256 z2zE1964;
    z2zE1964 = zslice_load_word_offset(zcalldata, zoffset_word);
    {
      z8zE419 = zpush_word(z2zE1964);
      if (have_exception) {  goto end_block_exception_2258;  }
    }
  } else {  z8zE419 = UNIT;  }
end_function_2257: ;
  return z8zE419;
end_block_exception_2258: ;

  return UNIT;
}

unit zexecute_calldatacopy(unit z3zE1931)
{
  unit z8zE420;
  unit z3zE1932;
  {
    z3zE1932 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2256;  }
  }
  sail_u256 zdestination_word;
  {
    zdestination_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2256;  }
  }
  sail_u256 zsource_word;
  {
    zsource_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2256;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2256;  }
  }
  unit z3zE1933;
  {
    z3zE1933 = zcharge_copy_gas(zlength_word);
    if (have_exception) {  goto end_block_exception_2256;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zdestination_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2256;  }
  }
  bool z2zE1960;
  z2zE1960 = zis_running(UNIT);
  if (z2zE1960) {
    uint64_t z2zE1961;
    z2zE1961 = zrange.zoff;
    uint64_t z2zE1962;
    z2zE1962 = zrange.zlen;
    z8zE420 = zslice_copy_word_offset(zcalldata, z2zE1961, zsource_word, z2zE1962);
  } else {  z8zE420 = UNIT;  }
end_function_2255: ;
  return z8zE420;
end_block_exception_2256: ;

  return UNIT;
}

unit zexecute_codesizze(unit z3zE1929)
{
  unit z8zE421;
  unit z3zE1930;
  {
    z3zE1930 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2254;  }
  }
  sail_u256 z2zE1959;
  {
    uint64_t z2zE1958;
    z2zE1958 = zframe_code_len(UNIT);
    z2zE1959 = zword_of_source_byte_count(z2zE1958);
  }
  {
    z8zE421 = zpush_word(z2zE1959);
    if (have_exception) {  goto end_block_exception_2254;  }
  }
end_function_2253: ;
  return z8zE421;
end_block_exception_2254: ;

  return UNIT;
}

unit zexecute_codecopy(unit z3zE1926)
{
  unit z8zE422;
  unit z3zE1927;
  {
    z3zE1927 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2252;  }
  }
  sail_u256 zdestination_word;
  {
    zdestination_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2252;  }
  }
  sail_u256 zsource_word;
  {
    zsource_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2252;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2252;  }
  }
  unit z3zE1928;
  {
    z3zE1928 = zcharge_copy_gas(zlength_word);
    if (have_exception) {  goto end_block_exception_2252;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zdestination_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2252;  }
  }
  bool z2zE1954;
  z2zE1954 = zis_running(UNIT);
  if (z2zE1954) {
    struct zByteSliceFields z2zE1955;
    z2zE1955 = zframe_code.zbytes;
    uint64_t z2zE1956;
    z2zE1956 = zrange.zoff;
    uint64_t z2zE1957;
    z2zE1957 = zrange.zlen;
    z8zE422 = zslice_copy_word_offset(z2zE1955, z2zE1956, zsource_word, z2zE1957);
  } else {  z8zE422 = UNIT;  }
end_function_2251: ;
  return z8zE422;
end_block_exception_2252: ;

  return UNIT;
}

unit zexecute_balance(unit z3zE1924)
{
  unit z8zE423;
  sail_fixed_bytes_20 za;
  {
    sail_u256 z2zE1953;
    {
      z2zE1953 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2250;  }
    }
    za = evmsail_word_to_address(z2zE1953);
  }
  bool zwarm;
  zwarm = zk_access_account(za);
  uint64_t z2zE1950;
  z2zE1950 = zaccount_cost(zwarm);
  unit z3zE1925;
  {
    z3zE1925 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1950);
    if (have_exception) {  goto end_block_exception_2250;  }
  }
  bool z2zE1951;
  z2zE1951 = zis_running(UNIT);
  if (z2zE1951) {
    sail_u256 z2zE1952;
    {
      z2zE1952 = zk_get_balance(za);
      if (have_exception) {  goto end_block_exception_2250;  }
    }
    {
      z8zE423 = zpush_word(z2zE1952);
      if (have_exception) {  goto end_block_exception_2250;  }
    }
  } else {  z8zE423 = UNIT;  }
end_function_2249: ;
  return z8zE423;
end_block_exception_2250: ;

  return UNIT;
}

unit zexecute_selfbalance(unit z3zE1922)
{
  unit z8zE424;
  unit z3zE1923;
  {
    z3zE1923 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_low);
    if (have_exception) {  goto end_block_exception_2248;  }
  }
  sail_u256 z2zE1949;
  {
    sail_fixed_bytes_20 z2zE1948;
    z2zE1948 = zself_addr(UNIT);
    {
      z2zE1949 = zk_get_balance(z2zE1948);
      if (have_exception) {  goto end_block_exception_2248;  }
    }
  }
  {
    z8zE424 = zpush_word(z2zE1949);
    if (have_exception) {  goto end_block_exception_2248;  }
  }
end_function_2247: ;
  return z8zE424;
end_block_exception_2248: ;

  return UNIT;
}

unit zexecute_extcodesizze(unit z3zE1920)
{
  unit z8zE425;
  sail_fixed_bytes_20 za;
  {
    sail_u256 z2zE1947;
    {
      z2zE1947 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2246;  }
    }
    za = evmsail_word_to_address(z2zE1947);
  }
  bool zwarm;
  zwarm = zk_access_account(za);
  uint64_t z2zE1943;
  {
    uint64_t z2zE1941;
    z2zE1941 = zaccount_cost(zwarm);
    uint64_t z2zE1942;
    z2zE1942 = zexternal_code_read_cost(UNIT);
    {    z2zE1943 = (z2zE1941 + z2zE1942);
    }
  }
  unit z3zE1921;
  {
    z3zE1921 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1943);
    if (have_exception) {  goto end_block_exception_2246;  }
  }
  bool z2zE1944;
  z2zE1944 = zis_running(UNIT);
  if (z2zE1944) {
    sail_u256 z2zE1946;
    {
      uint64_t z2zE1945;
      {
        z2zE1945 = zk_get_code_sizze(za);
        if (have_exception) {  goto end_block_exception_2246;  }
      }
      z2zE1946 = zword_of_source_byte_count(z2zE1945);
    }
    {
      z8zE425 = zpush_word(z2zE1946);
      if (have_exception) {  goto end_block_exception_2246;  }
    }
  } else {  z8zE425 = UNIT;  }
end_function_2245: ;
  return z8zE425;
end_block_exception_2246: ;

  return UNIT;
}

unit zexecute_extcodecopy(unit z3zE1917)
{
  unit z8zE426;
  sail_fixed_bytes_20 za;
  {
    sail_u256 z2zE1940;
    {
      z2zE1940 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2244;  }
    }
    za = evmsail_word_to_address(z2zE1940);
  }
  sail_u256 zdestination_word;
  {
    zdestination_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2244;  }
  }
  sail_u256 zsource_word;
  {
    zsource_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2244;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2244;  }
  }
  bool zwarm;
  zwarm = zk_access_account(za);
  uint64_t z2zE1936;
  {
    uint64_t z2zE1934;
    z2zE1934 = zaccount_cost(zwarm);
    uint64_t z2zE1935;
    z2zE1935 = zexternal_code_read_cost(UNIT);
    {    z2zE1936 = (z2zE1934 + z2zE1935);
    }
  }
  unit z3zE1919;
  {
    z3zE1919 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1936);
    if (have_exception) {  goto end_block_exception_2244;  }
  }
  unit z3zE1918;
  {
    z3zE1918 = zcharge_copy_gas(zlength_word);
    if (have_exception) {  goto end_block_exception_2244;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zdestination_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2244;  }
  }
  bool z2zE1937;
  z2zE1937 = zis_running(UNIT);
  if (z2zE1937) {
    uint64_t z2zE1938;
    z2zE1938 = zrange.zoff;
    uint64_t z2zE1939;
    z2zE1939 = zrange.zlen;
    {
      z8zE426 = zk_code_copy(za, z2zE1938, zsource_word, z2zE1939);
      if (have_exception) {  goto end_block_exception_2244;  }
    }
  } else {  z8zE426 = UNIT;  }
end_function_2243: ;
  return z8zE426;
end_block_exception_2244: ;

  return UNIT;
}

unit zexecute_extcodehash(unit z3zE1915)
{
  unit z8zE427;
  sail_fixed_bytes_20 za;
  {
    sail_u256 z2zE1933;
    {
      z2zE1933 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2242;  }
    }
    za = evmsail_word_to_address(z2zE1933);
  }
  bool zwarm;
  zwarm = zk_access_account(za);
  uint64_t z2zE1929;
  z2zE1929 = zaccount_cost(zwarm);
  unit z3zE1916;
  {
    z3zE1916 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1929);
    if (have_exception) {  goto end_block_exception_2242;  }
  }
  bool z2zE1930;
  z2zE1930 = zis_running(UNIT);
  if (z2zE1930) {
    sail_u256 z2zE1932;
    {
      sail_fixed_bytes_32 z2zE1931;
      {
        z2zE1931 = zk_get_codehash(za);
        if (have_exception) {  goto end_block_exception_2242;  }
      }
      z2zE1932 = evmsail_hash_to_word(z2zE1931);
    }
    {
      z8zE427 = zpush_word(z2zE1932);
      if (have_exception) {  goto end_block_exception_2242;  }
    }
  } else {  z8zE427 = UNIT;  }
end_function_2241: ;
  return z8zE427;
end_block_exception_2242: ;

  return UNIT;
}

unit zexecute_returndatasizze(unit z3zE1913)
{
  unit z8zE428;
  unit z3zE1914;
  {
    z3zE1914 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2240;  }
  }
  sail_u256 z2zE1928;
  {
    uint64_t z2zE1927;
    z2zE1927 = zreturndata_sizze(UNIT);
    z2zE1928 = zword_of_source_byte_count(z2zE1927);
  }
  {
    z8zE428 = zpush_word(z2zE1928);
    if (have_exception) {  goto end_block_exception_2240;  }
  }
end_function_2239: ;
  return z8zE428;
end_block_exception_2240: ;

  return UNIT;
}

unit zexecute_returndatacopy(unit z3zE1910)
{
  unit z8zE429;
  unit z3zE1911;
  {
    z3zE1911 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2238;  }
  }
  sail_u256 zdestination_word;
  {
    zdestination_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2238;  }
  }
  sail_u256 zsource_word;
  {
    zsource_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2238;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2238;  }
  }
  unit z3zE1912;
  {
    z3zE1912 = zcharge_copy_gas(zlength_word);
    if (have_exception) {  goto end_block_exception_2238;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zdestination_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2238;  }
  }
  bool z2zE1925;
  z2zE1925 = zis_running(UNIT);
  if (z2zE1925) {
    uint64_t z2zE1926;
    z2zE1926 = zrange.zoff;
    {
      z8zE429 = zreturndata_copy_words(z2zE1926, zsource_word, zlength_word);
      if (have_exception) {  goto end_block_exception_2238;  }
    }
  } else {  z8zE429 = UNIT;  }
end_function_2237: ;
  return z8zE429;
end_block_exception_2238: ;

  return UNIT;
}

unit zexecute_blockhash(unit z3zE1908)
{
  unit z8zE430;
  unit z3zE1909;
  {
    z3zE1909 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(UINT64_C(20));
    if (have_exception) {  goto end_block_exception_2236;  }
  }
  sail_u256 z2zE1924;
  {
    sail_fixed_bytes_32 z2zE1923;
    {
      sail_u256 z2zE1922;
      {
        z2zE1922 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2236;  }
      }
      {
        z2zE1923 = zk_blockhash(z2zE1922);
        if (have_exception) {  goto end_block_exception_2236;  }
      }
    }
    z2zE1924 = evmsail_hash_to_word(z2zE1923);
  }
  {
    z8zE430 = zpush_word(z2zE1924);
    if (have_exception) {  goto end_block_exception_2236;  }
  }
end_function_2235: ;
  return z8zE430;
end_block_exception_2236: ;

  return UNIT;
}

unit zexecute_coinbase(unit z3zE1906)
{
  unit z8zE431;
  unit z3zE1907;
  {
    z3zE1907 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2234;  }
  }
  sail_u256 z2zE1921;
  z2zE1921 = zk_env(zF_Coinbase);
  {
    z8zE431 = zpush_word(z2zE1921);
    if (have_exception) {  goto end_block_exception_2234;  }
  }
end_function_2233: ;
  return z8zE431;
end_block_exception_2234: ;

  return UNIT;
}

unit zexecute_timestamp(unit z3zE1904)
{
  unit z8zE432;
  unit z3zE1905;
  {
    z3zE1905 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2232;  }
  }
  sail_u256 z2zE1920;
  z2zE1920 = zk_env(zF_Timestamp);
  {
    z8zE432 = zpush_word(z2zE1920);
    if (have_exception) {  goto end_block_exception_2232;  }
  }
end_function_2231: ;
  return z8zE432;
end_block_exception_2232: ;

  return UNIT;
}

unit zexecute_number(unit z3zE1902)
{
  unit z8zE433;
  unit z3zE1903;
  {
    z3zE1903 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2230;  }
  }
  sail_u256 z2zE1919;
  z2zE1919 = zk_env(zF_Number);
  {
    z8zE433 = zpush_word(z2zE1919);
    if (have_exception) {  goto end_block_exception_2230;  }
  }
end_function_2229: ;
  return z8zE433;
end_block_exception_2230: ;

  return UNIT;
}

unit zexecute_slotnum(unit z3zE1900)
{
  unit z8zE434;
  unit z3zE1901;
  {
    z3zE1901 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2228;  }
  }
  sail_u256 z2zE1918;
  z2zE1918 = zk_env(zF_SlotNumber);
  {
    z8zE434 = zpush_word(z2zE1918);
    if (have_exception) {  goto end_block_exception_2228;  }
  }
end_function_2227: ;
  return z8zE434;
end_block_exception_2228: ;

  return UNIT;
}

unit zexecute_prevrandao(unit z3zE1898)
{
  unit z8zE435;
  unit z3zE1899;
  {
    z3zE1899 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2226;  }
  }
  sail_u256 z2zE1917;
  z2zE1917 = zk_env(zF_PrevRandao);
  {
    z8zE435 = zpush_word(z2zE1917);
    if (have_exception) {  goto end_block_exception_2226;  }
  }
end_function_2225: ;
  return z8zE435;
end_block_exception_2226: ;

  return UNIT;
}

unit zexecute_gaslimit(unit z3zE1896)
{
  unit z8zE436;
  unit z3zE1897;
  {
    z3zE1897 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2224;  }
  }
  sail_u256 z2zE1916;
  z2zE1916 = zk_env(zF_GasLimit);
  {
    z8zE436 = zpush_word(z2zE1916);
    if (have_exception) {  goto end_block_exception_2224;  }
  }
end_function_2223: ;
  return z8zE436;
end_block_exception_2224: ;

  return UNIT;
}

unit zexecute_chainid(unit z3zE1894)
{
  unit z8zE437;
  unit z3zE1895;
  {
    z3zE1895 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2222;  }
  }
  sail_u256 z2zE1915;
  z2zE1915 = zk_env(zF_ChainId);
  {
    z8zE437 = zpush_word(z2zE1915);
    if (have_exception) {  goto end_block_exception_2222;  }
  }
end_function_2221: ;
  return z8zE437;
end_block_exception_2222: ;

  return UNIT;
}

unit zexecute_basefee(unit z3zE1892)
{
  unit z8zE438;
  unit z3zE1893;
  {
    z3zE1893 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2220;  }
  }
  sail_u256 z2zE1914;
  z2zE1914 = zk_env(zF_BaseFee);
  {
    z8zE438 = zpush_word(z2zE1914);
    if (have_exception) {  goto end_block_exception_2220;  }
  }
end_function_2219: ;
  return z8zE438;
end_block_exception_2220: ;

  return UNIT;
}

unit zexecute_blobbasefee(unit z3zE1890)
{
  unit z8zE439;
  unit z3zE1891;
  {
    z3zE1891 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2218;  }
  }
  sail_u256 z2zE1913;
  {
    uint64_t z2zE1912;
    z2zE1912 = zk_header.zexcess_blob_gas;
    {
      z2zE1913 = zblob_base_fee(z2zE1912);
      if (have_exception) {  goto end_block_exception_2218;  }
    }
  }
  {
    z8zE439 = zpush_word(z2zE1913);
    if (have_exception) {  goto end_block_exception_2218;  }
  }
end_function_2217: ;
  return z8zE439;
end_block_exception_2218: ;

  return UNIT;
}

unit zexecute_blobhash(unit z3zE1888)
{
  unit z8zE440;
  unit z3zE1889;
  {
    z3zE1889 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2216;  }
  }
  sail_u256 z2zE1911;
  {
    sail_u256 z2zE1910;
    {
      z2zE1910 = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2216;  }
    }
    z2zE1911 = zk_blobhash(z2zE1910);
  }
  {
    z8zE440 = zpush_word(z2zE1911);
    if (have_exception) {  goto end_block_exception_2216;  }
  }
end_function_2215: ;
  return z8zE440;
end_block_exception_2216: ;

  return UNIT;
}

unit zexecute_pop(unit z3zE1884)
{
  unit z8zE441;
  unit z3zE1885;
  {
    z3zE1885 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2214;  }
  }
  sail_u256 z2zE1909;
  {
    z2zE1909 = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2214;  }
  }
  unit z3zE1886;
  {
    z3zE1886 = UNIT;
    goto finish_match_2211;
  }
case_2212: ;
  sail_match_failure("execute_pop");
finish_match_2211: ;
  z8zE441 = z3zE1886;
end_function_2213: ;
  return z8zE441;
end_block_exception_2214: ;

  return UNIT;
}

unit zexecute_mload(unit z3zE1882)
{
  unit z8zE442;
  unit z3zE1883;
  {
    z3zE1883 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2210;  }
  }
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2210;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zoffset_word, UINT64_C(32));
    if (have_exception) {  goto end_block_exception_2210;  }
  }
  bool z2zE1906;
  z2zE1906 = zis_running(UNIT);
  if (z2zE1906) {
    sail_u256 z2zE1908;
    {
      uint64_t z2zE1907;
      z2zE1907 = zrange.zoff;
      z2zE1908 = zmem_load(z2zE1907);
    }
    {
      z8zE442 = zpush_word(z2zE1908);
      if (have_exception) {  goto end_block_exception_2210;  }
    }
  } else {  z8zE442 = UNIT;  }
end_function_2209: ;
  return z8zE442;
end_block_exception_2210: ;

  return UNIT;
}

unit zexecute_mstore(unit z3zE1880)
{
  unit z8zE443;
  unit z3zE1881;
  {
    z3zE1881 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2208;  }
  }
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2208;  }
  }
  sail_u256 zv;
  {
    zv = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2208;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zoffset_word, UINT64_C(32));
    if (have_exception) {  goto end_block_exception_2208;  }
  }
  bool z2zE1904;
  z2zE1904 = zis_running(UNIT);
  if (z2zE1904) {
    uint64_t z2zE1905;
    z2zE1905 = zrange.zoff;
    z8zE443 = zmem_store(z2zE1905, zv);
  } else {  z8zE443 = UNIT;  }
end_function_2207: ;
  return z8zE443;
end_block_exception_2208: ;

  return UNIT;
}

unit zexecute_mstore8(unit z3zE1878)
{
  unit z8zE444;
  unit z3zE1879;
  {
    z3zE1879 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2206;  }
  }
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2206;  }
  }
  sail_u256 zv;
  {
    zv = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2206;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zoffset_word, zWORD_ONE);
    if (have_exception) {  goto end_block_exception_2206;  }
  }
  bool z2zE1902;
  z2zE1902 = zis_running(UNIT);
  if (z2zE1902) {
    uint64_t z2zE1903;
    z2zE1903 = zrange.zoff;
    z8zE444 = zmem_store_byte(z2zE1903, zv);
  } else {  z8zE444 = UNIT;  }
end_function_2205: ;
  return z8zE444;
end_block_exception_2206: ;

  return UNIT;
}

unit zexecute_msizze(unit z3zE1876)
{
  unit z8zE445;
  unit z3zE1877;
  {
    z3zE1877 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2204;  }
  }
  sail_u256 z2zE1901;
  {
    sail_u128 z2zE1900;
    {
      uint64_t z2zE1899;
      {
        uint64_t z2zE1898;
        z2zE1898 = zevm_memory_high_water(UNIT);
        z2zE1899 = zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1898);
      }
      z2zE1900 = u128_mul_u64_u64(z2zE1899, UINT64_C(32));
    }
    z2zE1901 = zword_of_nat_byte_countzIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1900);
  }
  {
    z8zE445 = zpush_word(z2zE1901);
    if (have_exception) {  goto end_block_exception_2204;  }
  }
end_function_2203: ;
  return z8zE445;
end_block_exception_2204: ;

  return UNIT;
}

unit zexecute_mcopy(unit z3zE1870)
{
  unit z8zE446;
  unit z3zE1871;
  {
    z3zE1871 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2202;  }
  }
  sail_u256 zdestination_word;
  {
    zdestination_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2202;  }
  }
  sail_u256 zsource_word;
  {
    zsource_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2202;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2202;  }
  }
  unit z3zE1872;
  {
    z3zE1872 = zcharge_copy_gas(zlength_word);
    if (have_exception) {  goto end_block_exception_2202;  }
  }
  bool z2zE1891;
  z2zE1891 = zis_running(UNIT);
  if (z2zE1891) {
    uint64_t zavailable;
    zavailable = zgas_remaining;
    struct zMemoryPairExpansion zexpansion;
    {
      zexpansion = zmemory_pair_expansion(zdestination_word, zlength_word, zsource_word, zlength_word, zavailable);
      if (have_exception) {  goto end_block_exception_2202;  }
    }
    uint64_t z2zE1892;
    z2zE1892 = zexpansion.zcost;
    unit z3zE1873;
    {
      z3zE1873 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1892);
      if (have_exception) {  goto end_block_exception_2202;  }
    }
    bool z2zE1893;
    z2zE1893 = zis_running(UNIT);
    if (z2zE1893) {
      struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z2zE1894;
      z2zE1894 = zapply_memory_pair_expansion(zexpansion);
      unit z3zE1874;
      {
        struct zMemoryRangeFields zdestination;
        zdestination = z2zE1894.ztup0;
        struct zMemoryRangeFields zsource;
        zsource = z2zE1894.ztup1;
        uint64_t z2zE1895;
        z2zE1895 = zdestination.zoff;
        uint64_t z2zE1896;
        z2zE1896 = zsource.zoff;
        uint64_t z2zE1897;
        z2zE1897 = zdestination.zlen;
        z3zE1874 = zmem_mcopy(z2zE1895, z2zE1896, z2zE1897);
        goto finish_match_2199;
      }
    case_2200: ;
      sail_match_failure("execute_mcopy");
    finish_match_2199: ;
      z8zE446 = z3zE1874;
    } else {  z8zE446 = UNIT;  }
  } else {  z8zE446 = UNIT;  }
end_function_2201: ;
  return z8zE446;
end_block_exception_2202: ;

  return UNIT;
}

unit zexecute_sload(unit z3zE1868)
{
  unit z8zE447;
  sail_u256 zs;
  {
    zs = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2198;  }
  }
  bool zwarm;
  {
    sail_fixed_bytes_20 z2zE1890;
    z2zE1890 = zself_addr(UNIT);
    zwarm = zk_slot_is_warm(z2zE1890, zs);
  }
  uint64_t z2zE1885;
  z2zE1885 = zsload_cost(zwarm);
  unit z3zE1869;
  {
    z3zE1869 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1885);
    if (have_exception) {  goto end_block_exception_2198;  }
  }
  bool z2zE1886;
  z2zE1886 = zis_running(UNIT);
  if (z2zE1886) {
    sail_u256 z2zE1889;
    {
      struct zStorageValue z2zE1888;
      {
        sail_fixed_bytes_20 z2zE1887;
        z2zE1887 = zself_addr(UNIT);
        {
          z2zE1888 = zk_sload(z2zE1887, zs);
          if (have_exception) {  goto end_block_exception_2198;  }
        }
      }
      z2zE1889 = z2zE1888.zcurr;
    }
    {
      z8zE447 = zpush_word(z2zE1889);
      if (have_exception) {  goto end_block_exception_2198;  }
    }
  } else {  z8zE447 = UNIT;  }
end_function_2197: ;
  return z8zE447;
end_block_exception_2198: ;

  return UNIT;
}

unit zexecute_sstore(unit z3zE1858)
{
  unit z8zE448;
  bool z2zE1858;
  {
    z2zE1858 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2196;  }
  }
  if (z2zE1858) {  z8zE448 = UNIT;  } else {
    bool z2zE1860;
    {
      bool z2zE1859;
      z2zE1859 = zfork_lt(zk_fork, zAmsterdam);
      bool z3zE1859;
      if (z2zE1859) {  z3zE1859 = (!(zG_callstipend < zgas_remaining));  } else {  z3zE1859 = false;  }
      z2zE1860 = z3zE1859;
    }
    if (z2zE1860) {
      {
        z8zE448 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2196;  }
      }
    } else {
      sail_u256 zs;
      {
        zs = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2196;  }
      }
      sail_u256 zv;
      {
        zv = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2196;  }
      }
      bool z2zE1861;
      z2zE1861 = zis_running(UNIT);
      if (z2zE1861) {
        bool zwarm;
        {
          sail_fixed_bytes_20 z2zE1884;
          z2zE1884 = zself_addr(UNIT);
          zwarm = zk_slot_is_warm(z2zE1884, zs);
        }
        bool zcold;
        zcold = not(zwarm);
        bool z2zE1862;
        z2zE1862 = zfork_gteq(zk_fork, zAmsterdam);
        unit z3zE1861;
        if (z2zE1862) {
          uint64_t zaccess_cost;
          zaccess_cost = zamsterdam_storage_access_cost(zcold);
          uint64_t zsentry_cost;
          {
            bool z2zE1863;
            z2zE1863 = (zaccess_cost < zG_sstore_sentry);
            if (z2zE1863) {  zsentry_cost = zG_sstore_sentry;  } else {  zsentry_cost = zaccess_cost;  }
          }
          {
            z3zE1861 = zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zsentry_cost);
            if (have_exception) {  goto end_block_exception_2196;  }
          }
        } else {  z3zE1861 = UNIT;  }
        bool z2zE1865;
        {
          bool z2zE1864;
          z2zE1864 = zis_running(UNIT);
          z2zE1865 = not(z2zE1864);
        }
        unit z3zE1860;
        if (z2zE1865) {
          z8zE448 = UNIT;
          goto cleanup_2194;
          /* unreachable after return */
          goto end_cleanup_2195;
        cleanup_2194: ;
          goto end_function_2193;
        end_cleanup_2195: ;
        } else {  z3zE1860 = UNIT;  }
        struct zStorageValue zentry;
        {
          sail_fixed_bytes_20 z2zE1883;
          z2zE1883 = zself_addr(UNIT);
          {
            zentry = zk_sload(z2zE1883, zs);
            if (have_exception) {  goto end_block_exception_2196;  }
          }
        }
        struct zSstoreCosts zcosts;
        {
          sail_u256 z2zE1881;
          z2zE1881 = zentry.zorig;
          sail_u256 z2zE1882;
          z2zE1882 = zentry.zcurr;
          {
            zcosts = zsstore_costs(z2zE1881, z2zE1882, zv, zcold);
            if (have_exception) {  goto end_block_exception_2196;  }
          }
        }
        bool z2zE1867;
        {
          uint64_t z2zE1866;
          z2zE1866 = zcosts.zstate_credit;
          z2zE1867 = (z2zE1866 != UINT64_C(0));
        }
        unit z3zE1865;
        if (z2zE1867) {
          uint64_t z2zE1868;
          z2zE1868 = zcosts.zstate_credit;
          {
            z3zE1865 = zcredit_state_gas_refund(z2zE1868);
            if (have_exception) {  goto end_block_exception_2196;  }
          }
        } else {  z3zE1865 = UNIT;  }
        uint64_t z2zE1869;
        z2zE1869 = zcosts.zexecution;
        unit z3zE1864;
        {
          z3zE1864 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1869);
          if (have_exception) {  goto end_block_exception_2196;  }
        }
        uint64_t z2zE1870;
        z2zE1870 = zcosts.zstate_charge;
        unit z3zE1863;
        {
          z3zE1863 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1870);
          if (have_exception) {  goto end_block_exception_2196;  }
        }
        bool z2zE1873;
        {
          bool z2zE1872;
          {
            __int128 z2zE1871;
            z2zE1871 = zcosts.zrefund;
            z2zE1872 = (z2zE1871 == zGAS_REFUND_ZERO);
          }
          z2zE1873 = not(z2zE1872);
        }
        unit z3zE1862;
        if (z2zE1873) {
          __int128 z2zE1874;
          z2zE1874 = zcosts.zrefund;
          {
            z3zE1862 = zrecord_refund(z2zE1874);
            if (have_exception) {  goto end_block_exception_2196;  }
          }
        } else {  z3zE1862 = UNIT;  }
        bool z2zE1877;
        {
          bool z2zE1876;
          z2zE1876 = zis_running(UNIT);
          bool z3zE1866;
          if (z2zE1876) {
            sail_u256 z2zE1875;
            z2zE1875 = zentry.zcurr;
            z3zE1866 = (!eq_u256(z2zE1875, zv));
          } else {  z3zE1866 = false;  }
          z2zE1877 = z3zE1866;
        }
        if (z2zE1877) {
          sail_fixed_bytes_20 z2zE1879;
          z2zE1879 = zself_addr(UNIT);
          struct zStorageValue z2zE1880;
          {
            sail_u256 z2zE1878;
            z2zE1878 = zentry.zorig;
            struct zStorageValue z3zE1867;
            z3zE1867.zcurr = zv;
            z3zE1867.zorig = z2zE1878;
            z2zE1880 = z3zE1867;
          }
          z8zE448 = zk_sstore(z2zE1879, zs, z2zE1880);
        } else {  z8zE448 = UNIT;  }
      } else {  z8zE448 = UNIT;  }
    }
  }
end_function_2193: ;
  return z8zE448;
end_block_exception_2196: ;

  return UNIT;
}

unit zexecute_tload(unit z3zE1856)
{
  unit z8zE449;
  unit z3zE1857;
  {
    z3zE1857 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_warm_access);
    if (have_exception) {  goto end_block_exception_2192;  }
  }
  sail_u256 zs;
  {
    zs = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2192;  }
  }
  sail_u256 z2zE1857;
  {
    sail_fixed_bytes_20 z2zE1856;
    z2zE1856 = zself_addr(UNIT);
    z2zE1857 = zk_tload(z2zE1856, zs);
  }
  {
    z8zE449 = zpush_word(z2zE1857);
    if (have_exception) {  goto end_block_exception_2192;  }
  }
end_function_2191: ;
  return z8zE449;
end_block_exception_2192: ;

  return UNIT;
}

unit zexecute_tstore(unit z3zE1854)
{
  unit z8zE450;
  bool z2zE1854;
  {
    z2zE1854 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2190;  }
  }
  if (z2zE1854) {  z8zE450 = UNIT;  } else {
    unit z3zE1855;
    {
      z3zE1855 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_warm_access);
      if (have_exception) {  goto end_block_exception_2190;  }
    }
    sail_u256 zs;
    {
      zs = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2190;  }
    }
    sail_u256 zv;
    {
      zv = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2190;  }
    }
    sail_fixed_bytes_20 z2zE1855;
    z2zE1855 = zself_addr(UNIT);
    z8zE450 = zk_tstore(z2zE1855, zs, zv);
  }
end_function_2189: ;
  return z8zE450;
end_block_exception_2190: ;

  return UNIT;
}

unit zexecute_jump(unit z3zE1852)
{
  unit z8zE451;
  unit z3zE1853;
  {
    z3zE1853 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_mid);
    if (have_exception) {  goto end_block_exception_2188;  }
  }
  sail_u256 zdest;
  {
    zdest = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2188;  }
  }
  {
    z8zE451 = zdo_jump(zdest);
    if (have_exception) {  goto end_block_exception_2188;  }
  }
end_function_2187: ;
  return z8zE451;
end_block_exception_2188: ;

  return UNIT;
}

unit zexecute_jumpi(unit z3zE1850)
{
  unit z8zE452;
  unit z3zE1851;
  {
    z3zE1851 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_high);
    if (have_exception) {  goto end_block_exception_2186;  }
  }
  sail_u256 zdest;
  {
    zdest = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2186;  }
  }
  sail_u256 zcond;
  {
    zcond = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2186;  }
  }
  bool z2zE1853;
  z2zE1853 = zword_is_zzero(zcond);
  if (z2zE1853) {  z8zE452 = UNIT;  } else {
    {
      z8zE452 = zdo_jump(zdest);
      if (have_exception) {  goto end_block_exception_2186;  }
    }
  }
end_function_2185: ;
  return z8zE452;
end_block_exception_2186: ;

  return UNIT;
}

unit zexecute_pc(unit z3zE1848)
{
  unit z8zE453;
  unit z3zE1849;
  {
    z3zE1849 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2184;  }
  }
  sail_u256 z2zE1852;
  {
    sail_u256 z2zE1851;
    z2zE1851 = zword_of_source_byte_count(zpc);
    z2zE1852 = zalu_sub(z2zE1851, zWORD_ONE);
  }
  {
    z8zE453 = zpush_word(z2zE1852);
    if (have_exception) {  goto end_block_exception_2184;  }
  }
end_function_2183: ;
  return z8zE453;
end_block_exception_2184: ;

  return UNIT;
}

unit zexecute_gas(unit z3zE1846)
{
  unit z8zE454;
  unit z3zE1847;
  {
    z3zE1847 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
    if (have_exception) {  goto end_block_exception_2182;  }
  }
  {
    z8zE454 = zpush_gas(zgas_remaining);
    if (have_exception) {  goto end_block_exception_2182;  }
  }
end_function_2181: ;
  return z8zE454;
end_block_exception_2182: ;

  return UNIT;
}

unit zexecute_jumpdest(unit z3zE1845)
{
  unit z8zE455;
  {
    z8zE455 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_jumpdest);
    if (have_exception) {  goto end_block_exception_2180;  }
  }
end_function_2179: ;
  return z8zE455;
end_block_exception_2180: ;

  return UNIT;
}

unit zexecute_push(uint64_t zn, sail_u256 zv)
{
  unit z8zE456;
  bool z2zE1850;
  z2zE1850 = (zn == UINT64_C(0));
  unit z3zE1844;
  if (z2zE1850) {
    {
      z3zE1844 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_base);
      if (have_exception) {  goto end_block_exception_2178;  }
    }
  } else {
    {
      z3zE1844 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
      if (have_exception) {  goto end_block_exception_2178;  }
    }
  }
  {
    z8zE456 = zpush_word(zv);
    if (have_exception) {  goto end_block_exception_2178;  }
  }
end_function_2177: ;
  return z8zE456;
end_block_exception_2178: ;

  return UNIT;
}

unit zexecute_dup(uint64_t zn)
{
  unit z8zE457;
  unit z3zE1843;
  {
    z3zE1843 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2176;  }
  }
  bool z2zE1847;
  {
    uint64_t z2zE1846;
    z2zE1846 = zstack_height(UNIT);
    z2zE1847 = (z2zE1846 < zn);
  }
  if (z2zE1847) {
    {
      z8zE457 = zexc_halt(zStackUnderflow);
      if (have_exception) {  goto end_block_exception_2176;  }
    }
  } else {
    sail_u256 z2zE1849;
    {
      uint64_t z2zE1848;
      {    z2zE1848 = (zn - UINT64_C(1));
      }
      z2zE1849 = zpeek(z2zE1848);
    }
    {
      z8zE457 = zpush_word(z2zE1849);
      if (have_exception) {  goto end_block_exception_2176;  }
    }
  }
end_function_2175: ;
  return z8zE457;
end_block_exception_2176: ;

  return UNIT;
}

unit zexecute_swap(uint64_t zn)
{
  unit z8zE458;
  unit z3zE1841;
  {
    z3zE1841 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2174;  }
  }
  bool z2zE1845;
  {
    uint64_t z2zE1843;
    z2zE1843 = zstack_height(UNIT);
    uint64_t z2zE1844;
    {    z2zE1844 = (zn + UINT64_C(1));
    }
    z2zE1845 = (z2zE1843 < z2zE1844);
  }
  if (z2zE1845) {
    {
      z8zE458 = zexc_halt(zStackUnderflow);
      if (have_exception) {  goto end_block_exception_2174;  }
    }
  } else {
    sail_u256 ztop;
    ztop = zpeek(UINT64_C(0));
    sail_u256 zother;
    zother = zpeek(zn);
    unit z3zE1842;
    z3zE1842 = zstack_set(UINT64_C(0), zother);
    z8zE458 = zstack_set(zn, ztop);
  }
end_function_2173: ;
  return z8zE458;
end_block_exception_2174: ;

  return UNIT;
}

unit zexecute_dupn(uint64_t zimmediate)
{
  unit z8zE459;
  unit z3zE1840;
  {
    z3zE1840 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2172;  }
  }
  bool z2zE1836;
  z2zE1836 = zis_running(UNIT);
  if (z2zE1836) {
    bool z2zE1838;
    {
      bool z2zE1837;
      z2zE1837 = zdeep_stack_immediate_valid(zimmediate);
      z2zE1838 = not(z2zE1837);
    }
    if (z2zE1838) {
      {
        z8zE459 = zexc_halt(zInvalidOpcode);
        if (have_exception) {  goto end_block_exception_2172;  }
      }
    } else {
      uint64_t zn;
      zn = zdecode_single_stack_index(zimmediate);
      bool z2zE1840;
      {
        uint64_t z2zE1839;
        z2zE1839 = zstack_height(UNIT);
        z2zE1840 = (z2zE1839 < zn);
      }
      if (z2zE1840) {
        {
          z8zE459 = zexc_halt(zStackUnderflow);
          if (have_exception) {  goto end_block_exception_2172;  }
        }
      } else {
        sail_u256 z2zE1842;
        {
          uint64_t z2zE1841;
          {    z2zE1841 = (zn - UINT64_C(1));
          }
          z2zE1842 = zpeek(z2zE1841);
        }
        {
          z8zE459 = zpush_word(z2zE1842);
          if (have_exception) {  goto end_block_exception_2172;  }
        }
      }
    }
  } else {  z8zE459 = UNIT;  }
end_function_2171: ;
  return z8zE459;
end_block_exception_2172: ;

  return UNIT;
}

unit zexecute_swapn(uint64_t zimmediate)
{
  unit z8zE460;
  unit z3zE1838;
  {
    z3zE1838 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2170;  }
  }
  bool z2zE1830;
  z2zE1830 = zis_running(UNIT);
  if (z2zE1830) {
    bool z2zE1832;
    {
      bool z2zE1831;
      z2zE1831 = zdeep_stack_immediate_valid(zimmediate);
      z2zE1832 = not(z2zE1831);
    }
    if (z2zE1832) {
      {
        z8zE460 = zexc_halt(zInvalidOpcode);
        if (have_exception) {  goto end_block_exception_2170;  }
      }
    } else {
      uint64_t zn;
      zn = zdecode_single_stack_index(zimmediate);
      bool z2zE1835;
      {
        uint64_t z2zE1833;
        z2zE1833 = zstack_height(UNIT);
        uint64_t z2zE1834;
        {    z2zE1834 = (zn + UINT64_C(1));
        }
        z2zE1835 = (z2zE1833 < z2zE1834);
      }
      if (z2zE1835) {
        {
          z8zE460 = zexc_halt(zStackUnderflow);
          if (have_exception) {  goto end_block_exception_2170;  }
        }
      } else {
        sail_u256 ztop;
        ztop = zpeek(UINT64_C(0));
        sail_u256 zother;
        zother = zpeek(zn);
        unit z3zE1839;
        z3zE1839 = zstack_set(UINT64_C(0), zother);
        z8zE460 = zstack_set(zn, ztop);
      }
    }
  } else {  z8zE460 = UNIT;  }
end_function_2169: ;
  return z8zE460;
end_block_exception_2170: ;

  return UNIT;
}

unit zexecute_exchange(uint64_t zimmediate)
{
  unit z8zE461;
  unit z3zE1834;
  {
    z3zE1834 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_verylow);
    if (have_exception) {  goto end_block_exception_2168;  }
  }
  bool z2zE1823;
  z2zE1823 = zis_running(UNIT);
  if (z2zE1823) {
    bool z2zE1825;
    {
      bool z2zE1824;
      z2zE1824 = zexchange_immediate_valid(zimmediate);
      z2zE1825 = not(z2zE1824);
    }
    if (z2zE1825) {
      {
        z8zE461 = zexc_halt(zInvalidOpcode);
        if (have_exception) {  goto end_block_exception_2168;  }
      }
    } else {
      struct ztuple_z8z5u64zCz0z5u64z9 z2zE1826;
      z2zE1826 = zdecode_exchange_stack_indices(zimmediate);
      unit z3zE1835;
      {
        uint64_t zn;
        zn = z2zE1826.ztup0;
        uint64_t zm;
        zm = z2zE1826.ztup1;
        bool z2zE1829;
        {
          uint64_t z2zE1827;
          z2zE1827 = zstack_height(UNIT);
          uint64_t z2zE1828;
          {    z2zE1828 = (zm + UINT64_C(1));
          }
          z2zE1829 = (z2zE1827 < z2zE1828);
        }
        if (z2zE1829) {
          {
            z3zE1835 = zexc_halt(zStackUnderflow);
            if (have_exception) {  goto end_block_exception_2168;  }
          }
        } else {
          sail_u256 zfirst;
          zfirst = zpeek(zn);
          sail_u256 zsecond;
          zsecond = zpeek(zm);
          unit z3zE1837;
          z3zE1837 = zstack_set(zn, zsecond);
          z3zE1835 = zstack_set(zm, zfirst);
        }
        goto finish_match_2165;
      }
    case_2166: ;
      sail_match_failure("execute_exchange");
    finish_match_2165: ;
      z8zE461 = z3zE1835;
    }
  } else {  z8zE461 = UNIT;  }
end_function_2167: ;
  return z8zE461;
end_block_exception_2168: ;

  return UNIT;
}

unit zexecute_log(uint64_t zn)
{
  unit z8zE462;
  bool z2zE1816;
  {
    z2zE1816 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2164;  }
  }
  if (z2zE1816) {  z8zE462 = UNIT;  } else {
    sail_u256 zoffset_word;
    {
      zoffset_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2164;  }
    }
    sail_u256 zlength_word;
    {
      zlength_word = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2164;  }
    }
    zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
    {
      zpop_log_topics(&ztopics, zn);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
        goto end_block_exception_2164;
      }
    }
    unit z3zE1833;
    {
      z3zE1833 = zcharge_log_gas(zn, zlength_word);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
        goto end_block_exception_2164;
      }
    }
    struct zMemoryRangeFields zrange;
    {
      zrange = zcharge_memory_range(zoffset_word, zlength_word);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
        goto end_block_exception_2164;
      }
    }
    bool z2zE1817;
    z2zE1817 = zis_running(UNIT);
    if (z2zE1817) {
      sail_fixed_bytes_20 z2zE1821;
      z2zE1821 = zself_addr(UNIT);
      struct zBytes z2zE1822;
      CREATE(zBytes)(&z2zE1822);
      {
        struct zByteSliceFields z2zE1820;
        {
          uint64_t z2zE1818;
          z2zE1818 = zrange.zoff;
          uint64_t z2zE1819;
          z2zE1819 = zrange.zlen;
          z2zE1820 = zmemory_byte_slice(z2zE1818, z2zE1819);
        }
        zBytesSlice(&z2zE1822, z2zE1820);
      }
      z8zE462 = zk_log(z2zE1821, ztopics, z2zE1822);
      KILL(zBytes)(&z2zE1822);
    } else {  z8zE462 = UNIT;  }
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&ztopics);
  }
end_function_2163: ;
  return z8zE462;
end_block_exception_2164: ;

  return UNIT;
}

unit zexecute_stop(unit z3zE1832)
{
  unit z8zE463;
  struct zHaltKind z2zE1815;
  CREATE(zHaltKind)(&z2zE1815);
  zHaltStop(&z2zE1815, UNIT);
  zHalted(&zframe_status, z2zE1815);
  z8zE463 = UNIT;
  KILL(zHaltKind)(&z2zE1815);
end_function_2161: ;
  return z8zE463;
end_block_exception_2162: ;

  return UNIT;
}

unit zexecute_return(unit z3zE1831)
{
  unit z8zE464;
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zoffset_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  bool z2zE1809;
  z2zE1809 = zis_running(UNIT);
  if (z2zE1809) {
    struct zHaltKind z2zE1814;
    CREATE(zHaltKind)(&z2zE1814);
    {
      struct zByteSliceFields z2zE1813;
      {
        struct zByteSliceFields z2zE1812;
        {
          uint64_t z2zE1810;
          z2zE1810 = zrange.zoff;
          uint64_t z2zE1811;
          z2zE1811 = zrange.zlen;
          z2zE1812 = zmemory_byte_slice(z2zE1810, z2zE1811);
        }
        z2zE1813 = zfreezze_output(z2zE1812);
      }
      zHaltReturn(&z2zE1814, z2zE1813);
    }
    zHalted(&zframe_status, z2zE1814);
    z8zE464 = UNIT;
    KILL(zHaltKind)(&z2zE1814);
  } else {  z8zE464 = UNIT;  }
end_function_2159: ;
  return z8zE464;
end_block_exception_2160: ;

  return UNIT;
}

unit zexecute_revert(unit z3zE1829)
{
  unit z8zE465;
  sail_u256 zoffset_word;
  {
    zoffset_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2158;  }
  }
  sail_u256 zlength_word;
  {
    zlength_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2158;  }
  }
  struct zMemoryRangeFields zrange;
  {
    zrange = zcharge_memory_range(zoffset_word, zlength_word);
    if (have_exception) {  goto end_block_exception_2158;  }
  }
  bool z2zE1803;
  z2zE1803 = zis_running(UNIT);
  if (z2zE1803) {
    unit z3zE1830;
    {
      z3zE1830 = zrefill_frame_state_gas(UNIT);
      if (have_exception) {  goto end_block_exception_2158;  }
    }
    struct zHaltKind z2zE1808;
    CREATE(zHaltKind)(&z2zE1808);
    {
      struct zByteSliceFields z2zE1807;
      {
        struct zByteSliceFields z2zE1806;
        {
          uint64_t z2zE1804;
          z2zE1804 = zrange.zoff;
          uint64_t z2zE1805;
          z2zE1805 = zrange.zlen;
          z2zE1806 = zmemory_byte_slice(z2zE1804, z2zE1805);
        }
        z2zE1807 = zfreezze_output(z2zE1806);
      }
      zHaltRevert(&z2zE1808, z2zE1807);
    }
    zHalted(&zframe_status, z2zE1808);
    z8zE465 = UNIT;
    KILL(zHaltKind)(&z2zE1808);
  } else {  z8zE465 = UNIT;  }
end_function_2157: ;
  return z8zE465;
end_block_exception_2158: ;

  return UNIT;
}

unit zexecute_invalid(unit z3zE1827)
{
  unit z8zE466;
  zgas_remaining = zGAS_ZERO;
  unit z3zE1828;
  z3zE1828 = UNIT;
  {
    z8zE466 = zexc_halt(zInvalidOpcode);
    if (have_exception) {  goto end_block_exception_2156;  }
  }
end_function_2155: ;
  return z8zE466;
end_block_exception_2156: ;

  return UNIT;
}

unit zexecute_selfdestruct(unit z3zE1806)
{
  unit z8zE467;
  bool z2zE1768;
  {
    z2zE1768 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2154;  }
  }
  if (z2zE1768) {  z8zE467 = UNIT;  } else {
    sail_fixed_bytes_20 zbeneficiary;
    {
      sail_u256 z2zE1802;
      {
        z2zE1802 = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2154;  }
      }
      zbeneficiary = evmsail_word_to_address(z2zE1802);
    }
    bool z2zE1769;
    z2zE1769 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1769) {
      bool zwarm;
      zwarm = zk_access_account(zbeneficiary);
      uint64_t zaccess_cost;
      {
        uint64_t z2zE1781;
        {    z2zE1781 = (UINT64_C(0) + zG_selfdestruct);
        }
        uint64_t z2zE1782;
        if (zwarm) {  z2zE1782 = zG_zzero;  } else {  z2zE1782 = zG_amsterdam_cold_account_access;  }
        {    zaccess_cost = (z2zE1781 + z2zE1782);
        }
      }
      unit z3zE1818;
      {
        z3zE1818 = zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zaccess_cost);
        if (have_exception) {  goto end_block_exception_2154;  }
      }
      bool z2zE1770;
      z2zE1770 = zis_running(UNIT);
      if (z2zE1770) {
        sail_u256 zbal;
        {
          sail_fixed_bytes_20 z2zE1780;
          z2zE1780 = zself_addr(UNIT);
          {
            zbal = zk_get_balance(z2zE1780);
            if (have_exception) {  goto end_block_exception_2154;  }
          }
        }
        bool zcreates_account;
        {
          bool z2zE1779;
          z2zE1779 = zword_nonzzero(zbal);
          bool z3zE1819;
          if (z2zE1779) {
            {
              z3zE1819 = zk_account_is_empty(zbeneficiary);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          } else {  z3zE1819 = false;  }
          zcreates_account = z3zE1819;
        }
        uint64_t zexecution_cost;
        if (zcreates_account) {
          {    zexecution_cost = (zaccess_cost + zG_amsterdam_account_write);
          }
        } else {  zexecution_cost = zaccess_cost;  }
        unit z3zE1822;
        {
          z3zE1822 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zexecution_cost);
          if (have_exception) {  goto end_block_exception_2154;  }
        }
        bool z2zE1772;
        {
          bool z2zE1771;
          z2zE1771 = zis_running(UNIT);
          bool z3zE1820;
          if (z2zE1771) {  z3zE1820 = zcreates_account;  } else {  z3zE1820 = false;  }
          z2zE1772 = z3zE1820;
        }
        unit z3zE1821;
        if (z2zE1772) {
          {
            z3zE1821 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
            if (have_exception) {  goto end_block_exception_2154;  }
          }
        } else {  z3zE1821 = UNIT;  }
        bool z2zE1773;
        z2zE1773 = zis_running(UNIT);
        if (z2zE1773) {
          sail_fixed_bytes_20 z2zE1774;
          z2zE1774 = zself_addr(UNIT);
          unit z3zE1824;
          {
            z3zE1824 = zk_transfer(z2zE1774, zbeneficiary, zbal);
            if (have_exception) {  goto end_block_exception_2154;  }
          }
          bool z2zE1776;
          {
            sail_fixed_bytes_20 z2zE1775;
            z2zE1775 = zself_addr(UNIT);
            {
              z2zE1776 = zk_was_created(z2zE1775);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          }
          unit z3zE1823;
          if (z2zE1776) {
            sail_fixed_bytes_20 z2zE1777;
            z2zE1777 = zself_addr(UNIT);
            {
              z3zE1823 = zk_selfdestruct(z2zE1777);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          } else {  z3zE1823 = UNIT;  }
          struct zHaltKind z2zE1778;
          CREATE(zHaltKind)(&z2zE1778);
          zHaltSelfDestruct(&z2zE1778, UNIT);
          zHalted(&zframe_status, z2zE1778);
          z8zE467 = UNIT;
          KILL(zHaltKind)(&z2zE1778);
        } else {  z8zE467 = UNIT;  }
      } else {  z8zE467 = UNIT;  }
    } else {
      sail_u256 z3zE1825;
      {
        sail_fixed_bytes_20 z2zE1801;
        z2zE1801 = zself_addr(UNIT);
        {
          z3zE1825 = zk_get_balance(z2zE1801);
          if (have_exception) {  goto end_block_exception_2154;  }
        }
      }
      bool z3zE1826;
      z3zE1826 = zk_access_account(zbeneficiary);
      unit z3zE1811;
      {
        z3zE1811 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_selfdestruct);
        if (have_exception) {  goto end_block_exception_2154;  }
      }
      bool z2zE1783;
      z2zE1783 = not(z3zE1826);
      unit z3zE1810;
      if (z2zE1783) {
        {
          z3zE1810 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_cold_account);
          if (have_exception) {  goto end_block_exception_2154;  }
        }
      } else {  z3zE1810 = UNIT;  }
      bool z2zE1786;
      {
        bool z2zE1785;
        z2zE1785 = zis_running(UNIT);
        bool z3zE1808;
        if (z2zE1785) {
          bool z2zE1784;
          z2zE1784 = zword_nonzzero(z3zE1825);
          bool z3zE1807;
          if (z2zE1784) {
            {
              z3zE1807 = zk_account_is_empty(zbeneficiary);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          } else {  z3zE1807 = false;  }
          z3zE1808 = z3zE1807;
        } else {  z3zE1808 = false;  }
        z2zE1786 = z3zE1808;
      }
      unit z3zE1809;
      if (z2zE1786) {
        {
          z3zE1809 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_newaccount);
          if (have_exception) {  goto end_block_exception_2154;  }
        }
      } else {  z3zE1809 = UNIT;  }
      bool z2zE1787;
      z2zE1787 = zis_running(UNIT);
      if (z2zE1787) {
        bool zfirst_selfdestruct;
        {
          bool z2zE1800;
          {
            sail_fixed_bytes_20 z2zE1799;
            z2zE1799 = zself_addr(UNIT);
            {
              z2zE1800 = zk_is_selfdestructed(z2zE1799);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          }
          zfirst_selfdestruct = not(z2zE1800);
        }
        bool z2zE1789;
        {
          bool z2zE1788;
          z2zE1788 = zfork_lt(zk_fork, zLondon);
          bool z3zE1812;
          if (z2zE1788) {  z3zE1812 = zfirst_selfdestruct;  } else {  z3zE1812 = false;  }
          z2zE1789 = z3zE1812;
        }
        unit z3zE1817;
        if (z2zE1789) {
          {
            z3zE1817 = zrecord_refundzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zR_selfdestruct_pre_london);
            if (have_exception) {  goto end_block_exception_2154;  }
          }
        } else {  z3zE1817 = UNIT;  }
        sail_fixed_bytes_20 z2zE1790;
        z2zE1790 = zself_addr(UNIT);
        unit z3zE1816;
        {
          z3zE1816 = zk_transfer(z2zE1790, zbeneficiary, z3zE1825);
          if (have_exception) {  goto end_block_exception_2154;  }
        }
        bool z2zE1791;
        z2zE1791 = zfork_lt(zk_fork, zCancun);
        unit z3zE1813;
        if (z2zE1791) {
          sail_fixed_bytes_20 z2zE1792;
          z2zE1792 = zself_addr(UNIT);
          unit z3zE1815;
          {
            z3zE1815 = zk_zzero_balance(z2zE1792);
            if (have_exception) {  goto end_block_exception_2154;  }
          }
          sail_fixed_bytes_20 z2zE1793;
          z2zE1793 = zself_addr(UNIT);
          {
            z3zE1813 = zk_selfdestruct(z2zE1793);
            if (have_exception) {  goto end_block_exception_2154;  }
          }
        } else {
          bool z2zE1795;
          {
            sail_fixed_bytes_20 z2zE1794;
            z2zE1794 = zself_addr(UNIT);
            {
              z2zE1795 = zk_was_created(z2zE1794);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          }
          if (z2zE1795) {
            sail_fixed_bytes_20 z2zE1796;
            z2zE1796 = zself_addr(UNIT);
            unit z3zE1814;
            {
              z3zE1814 = zk_zzero_balance(z2zE1796);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
            sail_fixed_bytes_20 z2zE1797;
            z2zE1797 = zself_addr(UNIT);
            {
              z3zE1813 = zk_selfdestruct(z2zE1797);
              if (have_exception) {  goto end_block_exception_2154;  }
            }
          } else {  z3zE1813 = UNIT;  }
        }
        struct zHaltKind z2zE1798;
        CREATE(zHaltKind)(&z2zE1798);
        zHaltSelfDestruct(&z2zE1798, UNIT);
        zHalted(&zframe_status, z2zE1798);
        z8zE467 = UNIT;
        KILL(zHaltKind)(&z2zE1798);
      } else {  z8zE467 = UNIT;  }
    }
  }
end_function_2153: ;
  return z8zE467;
end_block_exception_2154: ;

  return UNIT;
}

unit zrun_create(bool zis2)
{
  unit z8zE468;
  sail_fixed_bytes_20 zcreator;
  zcreator = zself_addr(UNIT);
  sail_u256 zvalue;
  {
    zvalue = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2152;  }
  }
  sail_u256 zoff_word;
  {
    zoff_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2152;  }
  }
  sail_u256 zlen_word;
  {
    zlen_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2152;  }
  }
  sail_u256 zsalt;
  if (zis2) {
    {
      zsalt = zpop(UNIT);
      if (have_exception) {  goto end_block_exception_2152;  }
    }
  } else {  zsalt = zWORD_ZERO;  }
  bool z2zE1724;
  {
    bool z2zE1723;
    z2zE1723 = zis_running(UNIT);
    z2zE1724 = not(z2zE1723);
  }
  unit z3zE1759;
  if (z2zE1724) {
    z8zE468 = UNIT;
    goto cleanup_2150;
    /* unreachable after return */
    goto end_cleanup_2151;
  cleanup_2150: ;
    goto end_function_2135;
  end_cleanup_2151: ;
  } else {  z3zE1759 = UNIT;  }
  bool z2zE1725;
  {
    z2zE1725 = zguard_static(UNIT);
    if (have_exception) {  goto end_block_exception_2152;  }
  }
  unit z3zE1758;
  if (z2zE1725) {
    z8zE468 = UNIT;
    goto cleanup_2148;
    /* unreachable after return */
    goto end_cleanup_2149;
  cleanup_2148: ;
    goto end_function_2135;
  end_cleanup_2149: ;
  } else {  z3zE1758 = UNIT;  }
  struct zMemoryRangeFields zinitcode;
  {
    zinitcode = zcharge_memory_range(zoff_word, zlen_word);
    if (have_exception) {  goto end_block_exception_2152;  }
  }
  bool z2zE1727;
  {
    bool z2zE1726;
    z2zE1726 = zis_running(UNIT);
    z2zE1727 = not(z2zE1726);
  }
  unit z3zE1764;
  if (z2zE1727) {
    z8zE468 = UNIT;
    goto cleanup_2146;
    /* unreachable after return */
    goto end_cleanup_2147;
  cleanup_2146: ;
    goto end_function_2135;
  end_cleanup_2147: ;
  } else {  z3zE1764 = UNIT;  }
  uint64_t z2zE1728;
  z2zE1728 = zcreate_access_cost(UNIT);
  unit z3zE1763;
  {
    z3zE1763 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1728);
    if (have_exception) {  goto end_block_exception_2152;  }
  }
  bool z2zE1729;
  z2zE1729 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE1762;
  if (z2zE1729) {
    sail_u256 z2zE1730;
    z2zE1730 = zmemory_word_count_word(zlen_word);
    {
      z3zE1762 = zcharge_word_scaled_gas(zG_initcode_word, z2zE1730);
      if (have_exception) {  goto end_block_exception_2152;  }
    }
  } else {  z3zE1762 = UNIT;  }
  unit z3zE1761;
  if (zis2) {
    sail_u256 z2zE1731;
    z2zE1731 = zmemory_word_count_word(zlen_word);
    {
      z3zE1761 = zcharge_word_scaled_gas(zG_keccak_word, z2zE1731);
      if (have_exception) {  goto end_block_exception_2152;  }
    }
  } else {  z3zE1761 = UNIT;  }
  bool z2zE1733;
  {
    bool z2zE1732;
    z2zE1732 = zis_running(UNIT);
    z2zE1733 = not(z2zE1732);
  }
  unit z3zE1760;
  if (z2zE1733) {
    z8zE468 = UNIT;
    goto cleanup_2144;
    /* unreachable after return */
    goto end_cleanup_2145;
  cleanup_2144: ;
    goto end_function_2135;
  end_cleanup_2145: ;
  } else {  z3zE1760 = UNIT;  }
  bool z2zE1736;
  {
    bool z2zE1735;
    {
      uint64_t z2zE1734;
      z2zE1734 = zinitcode.zlen;
      z2zE1735 = zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1734);
    }
    z2zE1736 = not(z2zE1735);
  }
  if (z2zE1736) {
    {
      z8zE468 = zexc_halt(zInitCodeTooLarge);
      if (have_exception) {  goto end_block_exception_2152;  }
    }
  } else {
    uint64_t znonce;
    {
      znonce = zk_get_nonce(zcreator);
      if (have_exception) {  goto end_block_exception_2152;  }
    }
    sail_fixed_bytes_20 znew_addr;
    {
      bool z2zE1765;
      {
        bool z3zE1765;
        if (zis2) {  z3zE1765 = zis_running(UNIT);  } else {  z3zE1765 = false;  }
        z2zE1765 = z3zE1765;
      }
      if (z2zE1765) {
        sail_fixed_bytes_32 z2zE1767;
        {
          sail_u256 z2zE1766;
          z2zE1766 = zmem_keccak(zinitcode);
          z2zE1767 = evmsail_word_to_hash(z2zE1766);
        }
        znew_addr = zk_create2_addr(zcreator, zsalt, z2zE1767);
      } else {  znew_addr = zk_create_addr(zcreator, znonce);  }
    }
    uint64_t zchild_gas;
    zchild_gas = zGAS_ZERO;
    bool z2zE1737;
    z2zE1737 = zfork_lt(zk_fork, zAmsterdam);
    unit z3zE1769;
    if (z2zE1737) {
      uint64_t zavail;
      zavail = zgas_remaining;
      uint64_t zretained_gas;
      zretained_gas = (zavail / UINT64_C(64));
      {
        zchild_gas = zgas_sub_or_oog(zavail, zretained_gas);
        if (have_exception) {  goto end_block_exception_2152;  }
      }
      unit z3zE1770;
      z3zE1770 = UNIT;
      zgas_remaining = zretained_gas;
      z3zE1769 = UNIT;
    } else {  z3zE1769 = UNIT;  }
    bool z2zE1739;
    {
      bool z2zE1738;
      z2zE1738 = zis_running(UNIT);
      z2zE1739 = not(z2zE1738);
    }
    unit z3zE1768;
    if (z2zE1739) {
      z8zE468 = UNIT;
      goto cleanup_2142;
      /* unreachable after return */
      goto end_cleanup_2143;
    cleanup_2142: ;
      goto end_function_2135;
    end_cleanup_2143: ;
    } else {  z3zE1768 = UNIT;  }
    bool z2zE1740;
    z2zE1740 = zmessage.zis_static;
    unit z3zE1766;
    if (z2zE1740) {
      unit z3zE1767;
      {
        z3zE1767 = zexc_halt(zWriteProtection);
        if (have_exception) {  goto end_block_exception_2152;  }
      }
      z8zE468 = UNIT;
      goto cleanup_2140;
      /* unreachable after return */
      goto end_cleanup_2141;
    cleanup_2140: ;
      goto end_function_2135;
    end_cleanup_2141: ;
    } else {  z3zE1766 = UNIT;  }
    bool z2zE1745;
    {
      bool z2zE1744;
      z2zE1744 = (!(zcall_depth < zDEPTH_LIMIT));
      bool z3zE1772;
      if (z2zE1744) {  z3zE1772 = true;  } else {
        bool z2zE1743;
        {
          bool z2zE1742;
          {
            sail_u256 z2zE1741;
            {
              z2zE1741 = zk_get_balance(zcreator);
              if (have_exception) {  goto end_block_exception_2152;  }
            }
            z2zE1742 = zword_ule(zvalue, z2zE1741);
          }
          z2zE1743 = not(z2zE1742);
        }
        bool z3zE1771;
        if (z2zE1743) {  z3zE1771 = true;  } else {  z3zE1771 = (znonce == UINT64_C(18446744073709551615));  }
        z3zE1772 = z3zE1771;
      }
      z2zE1745 = z3zE1772;
    }
    if (z2zE1745) {
      unit z3zE1803;
      z3zE1803 = zreturndata_clear(UNIT);
      bool z2zE1746;
      z2zE1746 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1802;
      if (z2zE1746) {
        {
          z3zE1802 = zrefund_gas(zchild_gas);
          if (have_exception) {  goto end_block_exception_2152;  }
        }
      } else {  z3zE1802 = UNIT;  }
      {
        z8zE468 = zpush_word(zWORD_ZERO);
        if (have_exception) {  goto end_block_exception_2152;  }
      }
    } else {
      bool z2zE1747;
      z2zE1747 = zk_access_account(znew_addr);
      unit z3zE1773;
      {
        bool znew_account_charged;
        {
          bool z2zE1764;
          z2zE1764 = zfork_gteq(zk_fork, zAmsterdam);
          bool z3zE1774;
          if (z2zE1764) {
            {
              z3zE1774 = zk_account_is_empty(znew_addr);
              if (have_exception) {  goto end_block_exception_2152;  }
            }
          } else {  z3zE1774 = false;  }
          znew_account_charged = z3zE1774;
        }
        unit z3zE1779;
        if (znew_account_charged) {
          {
            z3zE1779 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
        } else {  z3zE1779 = UNIT;  }
        bool z2zE1749;
        {
          bool z2zE1748;
          z2zE1748 = zis_running(UNIT);
          z2zE1749 = not(z2zE1748);
        }
        unit z3zE1778;
        if (z2zE1749) {
          z8zE468 = UNIT;
          goto cleanup_2138;
          /* unreachable after return */
          goto end_cleanup_2139;
        cleanup_2138: ;
          goto end_function_2135;
        end_cleanup_2139: ;
        } else {  z3zE1778 = UNIT;  }
        bool z2zE1750;
        z2zE1750 = zfork_gteq(zk_fork, zAmsterdam);
        unit z3zE1776;
        if (z2zE1750) {
          uint64_t z3zE1804;
          z3zE1804 = zgas_remaining;
          uint64_t z3zE1805;
          z3zE1805 = (z3zE1804 / UINT64_C(64));
          {
            zchild_gas = zgas_sub_or_oog(z3zE1804, z3zE1805);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
          unit z3zE1777;
          z3zE1777 = UNIT;
          zgas_remaining = z3zE1805;
          z3zE1776 = UNIT;
        } else {  z3zE1776 = UNIT;  }
        bool z2zE1752;
        {
          bool z2zE1751;
          z2zE1751 = zis_running(UNIT);
          z2zE1752 = not(z2zE1751);
        }
        unit z3zE1775;
        if (z2zE1752) {
          z8zE468 = UNIT;
          goto cleanup_2136;
          /* unreachable after return */
          goto end_cleanup_2137;
        cleanup_2136: ;
          goto end_function_2135;
        end_cleanup_2137: ;
        } else {  z3zE1775 = UNIT;  }
        bool zoccupied;
        {
          zoccupied = zk_account_occupied(znew_addr);
          if (have_exception) {  goto end_block_exception_2152;  }
        }
        unit z3zE1781;
        z3zE1781 = zreturndata_clear(UNIT);
        unit z3zE1780;
        {
          z3zE1780 = zk_bump_nonce(zcreator);
          if (have_exception) {  goto end_block_exception_2152;  }
        }
        if (zoccupied) {
          unit z3zE1801;
          if (znew_account_charged) {
            {
              z3zE1801 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2152;  }
            }
          } else {  z3zE1801 = UNIT;  }
          {
            z3zE1773 = zpush_word(zWORD_ZERO);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
        } else {
          struct zByteSliceFields z1zE24;
          {
            bool z2zE1761;
            z2zE1761 = zis_running(UNIT);
            if (z2zE1761) {
              uint64_t z2zE1762;
              z2zE1762 = zinitcode.zoff;
              uint64_t z2zE1763;
              z2zE1763 = zinitcode.zlen;
              z1zE24 = zmemory_code_slice(z2zE1762, z2zE1763);
            } else {  z1zE24 = zEMPTY_CODE_SLICE;  }
          }
          struct zCode zchild_code;
          {
            sail_fixed_bytes_32 z2zE1760;
            z2zE1760 = zcode_db_insert(z1zE24, zk_fork);
            {
              zchild_code = zcode_db_resolve(z2zE1760);
              if (have_exception) {  goto end_block_exception_2152;  }
            }
          }
          struct zFrameCheckpoint zcheckpoint;
          CREATE(zFrameCheckpoint)(&zcheckpoint);
          {
            struct zFrameCheckpoint z2zE1759;
            CREATE(zFrameCheckpoint)(&z2zE1759);
            zsuspend_frame(&z2zE1759, UNIT);
            struct zFrameCheckpoint z3zE1783;
            CREATE(zFrameCheckpoint)(&z3zE1783);
            z3zE1783 = z2zE1759;
            z3zE1783.zstate_gas_remaining = zGAS_ZERO;
            zcheckpoint = z3zE1783;
          }
          struct zFrameContinuation z2zE1753;
          CREATE(zFrameContinuation)(&z2zE1753);
          {
            struct zCreateContinuation z3zE1784;
            CREATE(zCreateContinuation)(&z3zE1784);
            z3zE1784.zaddress = znew_addr;
            z3zE1784.zcheckpoint = zcheckpoint;
            z3zE1784.znew_account_charged = znew_account_charged;
            zResumeCreate(&z2zE1753, z3zE1784);
          }
          unit z3zE1800;
          z3zE1800 = frame_stack_push(z2zE1753);
          unit z3zE1799;
          {
            z3zE1799 = zk_mark_created(znew_addr);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
          unit z3zE1798;
          {
            z3zE1798 = zk_clear_storage(znew_addr);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
          unit z3zE1797;
          {
            z3zE1797 = zk_bump_nonce(znew_addr);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
          unit z3zE1796;
          {
            z3zE1796 = zk_transfer(zcreator, znew_addr, zvalue);
            if (have_exception) {  goto end_block_exception_2152;  }
          }
          bool z2zE1755;
          {
            struct zMessage z2zE1754;
            z2zE1754 = zcheckpoint.zmessage;
            z2zE1755 = z2zE1754.zis_static;
          }
          uint64_t z2zE1757;
          {
            uint64_t z2zE1756;
            z2zE1756 = zcheckpoint.zcall_depth;
            z2zE1757 = zframe_depth_increment(z2zE1756);
          }
          struct zMessage z3zE1785;
          z3zE1785.zaddress = znew_addr;
          z3zE1785.zcaller = zcreator;
          z3zE1785.zcode_address = znew_addr;
          z3zE1785.zdepth = z2zE1757;
          z3zE1785.zis_static = z2zE1755;
          z3zE1785.zstate_gas_reservoir = zstate_gas_remaining;
          z3zE1785.zvalue = zvalue;
          zmessage = z3zE1785;
          unit z3zE1795;
          z3zE1795 = UNIT;
          unit z3zE1794;
          z3zE1794 = zcalldata_install(zEMPTY_SLICE);
          zpc = UINT64_C(0);
          unit z3zE1793;
          z3zE1793 = UNIT;
          zgas_remaining = zchild_gas;
          unit z3zE1792;
          z3zE1792 = UNIT;
          unit z3zE1791;
          z3zE1791 = UNIT;
          zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
          unit z3zE1790;
          z3zE1790 = UNIT;
          zRunning(&zframe_status, UNIT);
          unit z3zE1789;
          z3zE1789 = UNIT;
          unit z3zE1788;
          z3zE1788 = zreturndata_clear(UNIT);
          zframe_code = zchild_code;
          unit z3zE1787;
          z3zE1787 = UNIT;
          uint64_t z2zE1758;
          z2zE1758 = zcheckpoint.zcall_depth;
          zcall_depth = zframe_depth_increment(z2zE1758);
          unit z3zE1786;
          z3zE1786 = UNIT;
          zframe_refund = zGAS_REFUND_ZERO;
          z3zE1773 = UNIT;
        }
        goto finish_match_2133;
      }
    case_2134: ;
      sail_match_failure("run_create");
    finish_match_2133: ;
      z8zE468 = z3zE1773;
    }
  }
end_function_2135: ;
  return z8zE468;
end_block_exception_2152: ;

  return UNIT;
}

unit zexecute_create(unit z3zE1757)
{
  unit z8zE469;
  {
    z8zE469 = zrun_create(false);
    if (have_exception) {  goto end_block_exception_2132;  }
  }
end_function_2131: ;
  return z8zE469;
end_block_exception_2132: ;

  return UNIT;
}

unit zexecute_create2(unit z3zE1756)
{
  unit z8zE470;
  {
    z8zE470 = zrun_create(true);
    if (have_exception) {  goto end_block_exception_2130;  }
  }
end_function_2129: ;
  return z8zE470;
end_block_exception_2130: ;

  return UNIT;
}

bool zcall_is_delegate(enum zCallKind zkind)
{
  bool z8zE471;
  bool z3zE1753;
  {
    if ((zDelegateCall != zkind)) goto case_2126;
    z3zE1753 = true;
    goto finish_match_2124;
  }
case_2126: ;
  {
    z3zE1753 = false;
    goto finish_match_2124;
  }
case_2125: ;
finish_match_2124: ;
  z8zE471 = z3zE1753;
end_function_2127: ;
  return z8zE471;
end_block_exception_2128: ;

  return false;
}

bool zcall_is_static(enum zCallKind zkind)
{
  bool z8zE472;
  bool z3zE1750;
  {
    if ((zStaticCall != zkind)) goto case_2121;
    z3zE1750 = true;
    goto finish_match_2119;
  }
case_2121: ;
  {
    z3zE1750 = false;
    goto finish_match_2119;
  }
case_2120: ;
finish_match_2119: ;
  z8zE472 = z3zE1750;
end_function_2122: ;
  return z8zE472;
end_block_exception_2123: ;

  return false;
}

bool zcall_takes_value(enum zCallKind zkind)
{
  bool z8zE473;
  bool z3zE1746;
  {
    if ((zCall != zkind)) goto case_2116;
    z3zE1746 = true;
    goto finish_match_2113;
  }
case_2116: ;
  {
    if ((zCallCode != zkind)) goto case_2115;
    z3zE1746 = true;
    goto finish_match_2113;
  }
case_2115: ;
  {
    z3zE1746 = false;
    goto finish_match_2113;
  }
case_2114: ;
finish_match_2113: ;
  z8zE473 = z3zE1746;
end_function_2117: ;
  return z8zE473;
end_block_exception_2118: ;

  return false;
}

bool zcall_transfers_value(enum zCallKind zkind)
{
  bool z8zE474;
  bool z3zE1743;
  {
    if ((zCall != zkind)) goto case_2110;
    z3zE1743 = true;
    goto finish_match_2108;
  }
case_2110: ;
  {
    z3zE1743 = false;
    goto finish_match_2108;
  }
case_2109: ;
finish_match_2108: ;
  z8zE474 = z3zE1743;
end_function_2111: ;
  return z8zE474;
end_block_exception_2112: ;

  return false;
}

bool zcall_uses_target_address(enum zCallKind zkind)
{
  bool z8zE475;
  bool z3zE1739;
  {
    if ((zCall != zkind)) goto case_2105;
    z3zE1739 = true;
    goto finish_match_2102;
  }
case_2105: ;
  {
    if ((zStaticCall != zkind)) goto case_2104;
    z3zE1739 = true;
    goto finish_match_2102;
  }
case_2104: ;
  {
    z3zE1739 = false;
    goto finish_match_2102;
  }
case_2103: ;
finish_match_2102: ;
  z8zE475 = z3zE1739;
end_function_2106: ;
  return z8zE475;
end_block_exception_2107: ;

  return false;
}

struct zCode zexecutable_code(sail_fixed_bytes_20 ztarget, bool zdele, sail_fixed_bytes_20 zdtgt)
{
  struct zCode z8zE476;
  if (zdele) {
    struct zCode zdelegate_code;
    {
      sail_fixed_bytes_32 z2zE1721;
      {
        z2zE1721 = zk_code_key(zdtgt);
        if (have_exception) {  goto end_block_exception_2101;  }
      }
      {
        zdelegate_code = zcode_db_resolve(z2zE1721);
        if (have_exception) {  goto end_block_exception_2101;  }
      }
    }
    bool z2zE1720;
    {
      uint64_t z2zE1719;
      z2zE1719 = zprecompile_number(zdtgt);
      z2zE1720 = (z2zE1719 != UINT64_C(0));
    }
    if (z2zE1720) {  z8zE476 = zEMPTY_CODE;  } else {  z8zE476 = zdelegate_code;  }
  } else {
    sail_fixed_bytes_32 z2zE1722;
    {
      z2zE1722 = zk_code_key(ztarget);
      if (have_exception) {  goto end_block_exception_2101;  }
    }
    {
      z8zE476 = zcode_db_resolve(z2zE1722);
      if (have_exception) {  goto end_block_exception_2101;  }
    }
  }
end_function_2100: ;
  return z8zE476;
end_block_exception_2101: ;
  struct zByteSliceFields z8zE883 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zCode z8zE882 = { .zbytes = z8zE883, .zjumpdests = UINT64_C(0xdeadc0de) };
  return z8zE882;
}

unit zrun_call(enum zCallKind zkind)
{
  unit z8zE477;
  sail_fixed_bytes_20 zcaller;
  zcaller = zself_addr(UNIT);
  sail_u256 zgas_request;
  {
    zgas_request = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  sail_u256 ztarget_word;
  {
    ztarget_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  sail_fixed_bytes_20 ztarget;
  ztarget = evmsail_word_to_address(ztarget_word);
  sail_u256 zvalue;
  {
    bool z2zE1718;
    z2zE1718 = zcall_takes_value(zkind);
    if (z2zE1718) {
      {
        zvalue = zpop(UNIT);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
    } else {  zvalue = zWORD_ZERO;  }
  }
  sail_u256 zargs_off_word;
  {
    zargs_off_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  sail_u256 zargs_len_word;
  {
    zargs_len_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  sail_u256 zret_off_word;
  {
    zret_off_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  sail_u256 zret_len_word;
  {
    zret_len_word = zpop(UNIT);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  bool z2zE1651;
  {
    bool z2zE1650;
    z2zE1650 = zis_running(UNIT);
    z2zE1651 = not(z2zE1650);
  }
  unit z3zE1669;
  if (z2zE1651) {
    z8zE477 = UNIT;
    goto cleanup_2097;
    /* unreachable after return */
    goto end_cleanup_2098;
  cleanup_2097: ;
    goto end_function_2080;
  end_cleanup_2098: ;
  } else {  z3zE1669 = UNIT;  }
  bool z2zE1654;
  {
    bool z2zE1653;
    z2zE1653 = zcall_transfers_value(zkind);
    bool z3zE1666;
    if (z2zE1653) {
      bool z2zE1652;
      z2zE1652 = zword_nonzzero(zvalue);
      bool z3zE1665;
      if (z2zE1652) {  z3zE1665 = zmessage.zis_static;  } else {  z3zE1665 = false;  }
      z3zE1666 = z3zE1665;
    } else {  z3zE1666 = false;  }
    z2zE1654 = z3zE1666;
  }
  unit z3zE1667;
  if (z2zE1654) {
    unit z3zE1668;
    {
      z3zE1668 = zexc_halt(zWriteProtection);
      if (have_exception) {  goto end_block_exception_2099;  }
    }
    z8zE477 = UNIT;
    goto cleanup_2095;
    /* unreachable after return */
    goto end_cleanup_2096;
  cleanup_2095: ;
    goto end_function_2080;
  end_cleanup_2096: ;
  } else {  z3zE1667 = UNIT;  }
  bool zwarm;
  zwarm = zk_access_account(ztarget);
  uint64_t ztarget_cost;
  ztarget_cost = zaccount_cost(zwarm);
  uint64_t ztransfer_cost;
  {
    bool z2zE1717;
    z2zE1717 = zword_nonzzero(zvalue);
    if (z2zE1717) {  ztransfer_cost = zcall_value_cost(UNIT);  } else {  ztransfer_cost = zGAS_CONSTANT_ZERO;  }
  }
  uint64_t zavailable;
  zavailable = zgas_remaining;
  struct zMemoryPairExpansion zmemory;
  {
    zmemory = zmemory_pair_expansion(zargs_off_word, zargs_len_word, zret_off_word, zret_len_word, zavailable);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  bool z2zE1656;
  {
    bool z2zE1655;
    z2zE1655 = zis_running(UNIT);
    z2zE1656 = not(z2zE1655);
  }
  unit z3zE1670;
  if (z2zE1656) {
    z8zE477 = UNIT;
    goto cleanup_2093;
    /* unreachable after return */
    goto end_cleanup_2094;
  cleanup_2093: ;
    goto end_function_2080;
  end_cleanup_2094: ;
  } else {  z3zE1670 = UNIT;  }
  uint64_t zmemory_cost;
  zmemory_cost = zmemory.zcost;
  uint64_t zstatic_base;
  {    zstatic_base = (ztarget_cost + ztransfer_cost);
  }
  uint64_t zbefore_static;
  zbefore_static = zgas_remaining;
  bool z2zE1657;
  z2zE1657 = (zbefore_static < zstatic_base);
  unit z3zE1671;
  if (z2zE1657) {
    unit z3zE1672;
    {
      z3zE1672 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2099;  }
    }
    z8zE477 = UNIT;
    goto cleanup_2091;
    /* unreachable after return */
    goto end_cleanup_2092;
  cleanup_2091: ;
    goto end_function_2080;
  end_cleanup_2092: ;
  } else {  z3zE1671 = UNIT;  }
  uint64_t zafter_static_base;
  {    zafter_static_base = (zbefore_static - zstatic_base);
  }
  bool z2zE1658;
  z2zE1658 = (zafter_static_base < zmemory_cost);
  unit z3zE1673;
  if (z2zE1658) {
    unit z3zE1674;
    {
      z3zE1674 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2099;  }
    }
    z8zE477 = UNIT;
    goto cleanup_2089;
    /* unreachable after return */
    goto end_cleanup_2090;
  cleanup_2089: ;
    goto end_function_2080;
  end_cleanup_2090: ;
  } else {  z3zE1673 = UNIT;  }
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1659;
  {
    z2zE1659 = zk_deleg_target(ztarget);
    if (have_exception) {  goto end_block_exception_2099;  }
  }
  unit z3zE1675;
  {
    bool ztg_deleg;
    ztg_deleg = z2zE1659.ztup0;
    sail_fixed_bytes_20 ztg_target;
    ztg_target = z2zE1659.ztup1;
    uint64_t zdelegation_cost;
    if (ztg_deleg) {
      bool zdw;
      zdw = zk_access_account(ztg_target);
      zdelegation_cost = zaccount_cost(zdw);
    } else {  zdelegation_cost = zGAS_CONSTANT_ZERO;  }
    bool znew_account_charged;
    {
      bool z2zE1716;
      z2zE1716 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE1678;
      if (z2zE1716) {
        bool z2zE1715;
        z2zE1715 = zword_nonzzero(zvalue);
        bool z3zE1677;
        if (z2zE1715) {
          bool z2zE1714;
          z2zE1714 = zcall_transfers_value(zkind);
          bool z3zE1676;
          if (z2zE1714) {
            {
              z3zE1676 = zk_account_is_empty(ztarget);
              if (have_exception) {  goto end_block_exception_2099;  }
            }
          } else {  z3zE1676 = false;  }
          z3zE1677 = z3zE1676;
        } else {  z3zE1677 = false;  }
        z3zE1678 = z3zE1677;
      } else {  z3zE1678 = false;  }
      znew_account_charged = z3zE1678;
    }
    uint64_t zcreate_cost;
    {
      bool z2zE1713;
      {
        bool z2zE1712;
        z2zE1712 = zfork_lt(zk_fork, zAmsterdam);
        bool z3zE1681;
        if (z2zE1712) {
          bool z2zE1711;
          z2zE1711 = zword_nonzzero(zvalue);
          bool z3zE1680;
          if (z2zE1711) {
            bool z2zE1710;
            z2zE1710 = zcall_transfers_value(zkind);
            bool z3zE1679;
            if (z2zE1710) {
              {
                z3zE1679 = zk_account_is_empty(ztarget);
                if (have_exception) {  goto end_block_exception_2099;  }
              }
            } else {  z3zE1679 = false;  }
            z3zE1680 = z3zE1679;
          } else {  z3zE1680 = false;  }
          z3zE1681 = z3zE1680;
        } else {  z3zE1681 = false;  }
        z2zE1713 = z3zE1681;
      }
      if (z2zE1713) {  zcreate_cost = zG_newaccount;  } else {  zcreate_cost = zGAS_CONSTANT_ZERO;  }
    }
    uint64_t zadditional_cost;
    {    zadditional_cost = (zdelegation_cost + zcreate_cost);
    }
    uint64_t zbefore_required;
    zbefore_required = zgas_remaining;
    bool z2zE1660;
    z2zE1660 = (zbefore_required < zstatic_base);
    unit z3zE1682;
    if (z2zE1660) {
      unit z3zE1683;
      {
        z3zE1683 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      z8zE477 = UNIT;
      goto cleanup_2087;
      /* unreachable after return */
      goto end_cleanup_2088;
    cleanup_2087: ;
      goto end_function_2080;
    end_cleanup_2088: ;
    } else {  z3zE1682 = UNIT;  }
    uint64_t zafter_required_base;
    {    zafter_required_base = (zbefore_required - zstatic_base);
    }
    bool z2zE1661;
    z2zE1661 = (zafter_required_base < zmemory_cost);
    unit z3zE1684;
    if (z2zE1661) {
      unit z3zE1685;
      {
        z3zE1685 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      z8zE477 = UNIT;
      goto cleanup_2085;
      /* unreachable after return */
      goto end_cleanup_2086;
    cleanup_2085: ;
      goto end_function_2080;
    end_cleanup_2086: ;
    } else {  z3zE1684 = UNIT;  }
    uint64_t zafter_static_cost;
    {    zafter_static_cost = (zafter_required_base - zmemory_cost);
    }
    bool z2zE1662;
    z2zE1662 = (zafter_static_cost < zadditional_cost);
    unit z3zE1686;
    if (z2zE1662) {
      unit z3zE1687;
      {
        z3zE1687 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      z8zE477 = UNIT;
      goto cleanup_2083;
      /* unreachable after return */
      goto end_cleanup_2084;
    cleanup_2083: ;
      goto end_function_2080;
    end_cleanup_2084: ;
    } else {  z3zE1686 = UNIT;  }
    uint64_t zafter_required;
    {    zafter_required = (zafter_static_cost - zadditional_cost);
    }
    uint64_t zrequired;
    {
      bool z2zE1708;
      z2zE1708 = (!(zbefore_required < zafter_required));
      if (z2zE1708) {
        {    zrequired = (zbefore_required - zafter_required);
        }
      } else {
        struct zexception z2zE1709;
        CREATE(zexception)(&z2zE1709);
        zInvalidBlock(&z2zE1709, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1709);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/interpreter.sail:473.12-473.48");
        KILL(zexception)(&z2zE1709);
        goto end_block_exception_2099;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1709);
      }
    }
    uint64_t zstipend;
    {
      bool z2zE1707;
      z2zE1707 = zword_nonzzero(zvalue);
      if (z2zE1707) {  zstipend = zG_callstipend;  } else {  zstipend = zGAS_ZERO;  }
    }
    uint64_t zbase_child;
    zbase_child = zGAS_ZERO;
    bool z2zE1663;
    z2zE1663 = zfork_gteq(zk_fork, zAmsterdam);
    unit z3zE1694;
    if (z2zE1663) {
      unit z3zE1698;
      {
        z3zE1698 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      unit z3zE1697;
      if (znew_account_charged) {
        {
          z3zE1697 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
      } else {  z3zE1697 = UNIT;  }
      bool z2zE1664;
      z2zE1664 = zis_running(UNIT);
      if (z2zE1664) {
        {
          zbase_child = zcall_gas_cap_word(zgas_remaining, zgas_request);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
        unit z3zE1699;
        z3zE1699 = UNIT;
        {
          z3zE1694 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase_child);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
      } else {  z3zE1694 = UNIT;  }
    } else {
      uint64_t zavail;
      zavail = zgas_remaining;
      bool z2zE1665;
      z2zE1665 = (zavail < zrequired);
      if (z2zE1665) {  zbase_child = zGAS_ZERO;  } else {
        uint64_t zavailable_after_cost;
        {
          zavailable_after_cost = zgas_sub_or_oog(zavail, zrequired);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
        {
          zbase_child = zcall_gas_cap_word(zavailable_after_cost, zgas_request);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
      }
      unit z3zE1696;
      z3zE1696 = UNIT;
      unit z3zE1695;
      {
        z3zE1695 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      bool z2zE1666;
      z2zE1666 = zis_running(UNIT);
      if (z2zE1666) {
        {
          z3zE1694 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zbase_child);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
      } else {  z3zE1694 = UNIT;  }
    }
    bool z2zE1668;
    {
      bool z2zE1667;
      z2zE1667 = zis_running(UNIT);
      z2zE1668 = not(z2zE1667);
    }
    unit z3zE1693;
    if (z2zE1668) {
      z8zE477 = UNIT;
      goto cleanup_2081;
      /* unreachable after return */
      goto end_cleanup_2082;
    cleanup_2081: ;
      goto end_function_2080;
    end_cleanup_2082: ;
    } else {  z3zE1693 = UNIT;  }
    unit z3zE1688;
    if (ztg_deleg) {
      struct zCode z2zE1670;
      {
        sail_fixed_bytes_32 z2zE1669;
        {
          z2zE1669 = zk_code_key(ztg_target);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
        {
          z2zE1670 = zcode_db_resolve(z2zE1669);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
      }
      unit z3zE1689;
      {
        struct zAccount z2zE1671;
        {
          z2zE1671 = zk_aload(ztg_target);
          if (have_exception) {  goto end_block_exception_2099;  }
        }
        unit z3zE1690;
        {
          z3zE1690 = UNIT;
          goto finish_match_2071;
        }
      case_2072: ;
        sail_match_failure("run_call");
      finish_match_2071: ;
        z3zE1689 = z3zE1690;
        goto finish_match_2069;
      }
    case_2070: ;
      sail_match_failure("run_call");
    finish_match_2069: ;
      z3zE1688 = z3zE1689;
    } else {  z3zE1688 = UNIT;  }
    struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 z2zE1672;
    z2zE1672 = zapply_memory_pair_expansion(zmemory);
    unit z3zE1700;
    {
      struct zMemoryRangeFields zargs;
      zargs = z2zE1672.ztup0;
      struct zMemoryRangeFields zret;
      zret = z2zE1672.ztup1;
      uint64_t zchild_gas;
      {
        zchild_gas = zconserved_gas_add(zbase_child, zstipend);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      struct zAccount z2zE1673;
      {
        z2zE1673 = zk_aload(ztarget);
        if (have_exception) {  goto end_block_exception_2099;  }
      }
      unit z3zE1701;
      {
        bool z2zE1679;
        {
          bool z2zE1678;
          z2zE1678 = (!(zcall_depth < zDEPTH_LIMIT));
          bool z3zE1704;
          if (z2zE1678) {  z3zE1704 = true;  } else {
            bool z2zE1677;
            z2zE1677 = zcall_takes_value(zkind);
            bool z3zE1703;
            if (z2zE1677) {
              bool z2zE1676;
              z2zE1676 = zword_nonzzero(zvalue);
              bool z3zE1702;
              if (z2zE1676) {
                bool z2zE1675;
                {
                  sail_u256 z2zE1674;
                  {
                    z2zE1674 = zk_get_balance(zcaller);
                    if (have_exception) {  goto end_block_exception_2099;  }
                  }
                  z2zE1675 = zword_ule(zvalue, z2zE1674);
                }
                z3zE1702 = not(z2zE1675);
              } else {  z3zE1702 = false;  }
              z3zE1703 = z3zE1702;
            } else {  z3zE1703 = false;  }
            z3zE1704 = z3zE1703;
          }
          z2zE1679 = z3zE1704;
        }
        if (z2zE1679) {
          unit z3zE1736;
          z3zE1736 = zreturndata_clear(UNIT);
          unit z3zE1735;
          {
            z3zE1735 = zrefund_gas(zchild_gas);
            if (have_exception) {  goto end_block_exception_2099;  }
          }
          unit z3zE1734;
          if (znew_account_charged) {
            {
              z3zE1734 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
              if (have_exception) {  goto end_block_exception_2099;  }
            }
          } else {  z3zE1734 = UNIT;  }
          {
            z3zE1701 = zpush_word(zWORD_ZERO);
            if (have_exception) {  goto end_block_exception_2099;  }
          }
        } else {
          uint64_t zselected_precompile;
          zselected_precompile = zprecompile_number(ztarget);
          bool z2zE1680;
          z2zE1680 = (zselected_precompile != UINT64_C(0));
          if (z2zE1680) {
            uint64_t znumber;
            znumber = zselected_precompile;
            struct zByteSliceFields zinput;
            {
              uint64_t z2zE1688;
              z2zE1688 = zargs.zoff;
              uint64_t z2zE1689;
              z2zE1689 = zargs.zlen;
              zinput = zmemory_byte_slice(z2zE1688, z2zE1689);
            }
            struct zoptionzIU64zK z2zE1681;
            CREATE(zoptionzIU64zK)(&z2zE1681);
            zprecompile_gas(&z2zE1681, znumber, zinput, zchild_gas);
            unit z3zE1722;
            {
              if (z2zE1681.kind != Kind_zSomezIU64zK) goto case_2079;
              uint64_t zused;
              zused = z2zE1681.variants.zSomezIU64zK;
              struct zPrecompileResult zresult;
              zresult = zrun_precompile_slice(znumber, zinput);
              bool z2zE1682;
              z2zE1682 = zresult.zsuccess;
              if (z2zE1682) {
                zreturndata = zresult.zoutput;
                unit z3zE1732;
                z3zE1732 = UNIT;
                bool z2zE1684;
                {
                  bool z2zE1683;
                  z2zE1683 = zcall_transfers_value(zkind);
                  bool z3zE1729;
                  if (z2zE1683) {  z3zE1729 = zword_nonzzero(zvalue);  } else {  z3zE1729 = false;  }
                  z2zE1684 = z3zE1729;
                }
                unit z3zE1731;
                if (z2zE1684) {
                  {
                    z3zE1731 = zk_transfer(zcaller, ztarget, zvalue);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE1681);
                      goto end_block_exception_2099;
                    }
                  }
                } else {  z3zE1731 = UNIT;  }
                uint64_t z2zE1685;
                z2zE1685 = zret.zoff;
                uint64_t z2zE1686;
                z2zE1686 = zret.zlen;
                unit z3zE1730;
                z3zE1730 = zreturndata_copy_prefix(z2zE1685, z2zE1686);
                uint64_t zunused;
                {
                  zunused = zgas_sub_or_oog(zchild_gas, zused);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE1681);
                    goto end_block_exception_2099;
                  }
                }
                bool z2zE1687;
                z2zE1687 = zis_running(UNIT);
                if (z2zE1687) {
                  unit z3zE1733;
                  {
                    z3zE1733 = zrefund_gas(zunused);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE1681);
                      goto end_block_exception_2099;
                    }
                  }
                  {
                    z3zE1722 = zpush_word(zWORD_ONE);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE1681);
                      goto end_block_exception_2099;
                    }
                  }
                } else {  z3zE1722 = UNIT;  }
              } else {
                unit z3zE1728;
                z3zE1728 = zreturndata_clear(UNIT);
                unit z3zE1727;
                if (znew_account_charged) {
                  {
                    z3zE1727 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
                    if (have_exception) {
                      KILL(zoptionzIU64zK)(&z2zE1681);
                      goto end_block_exception_2099;
                    }
                  }
                } else {  z3zE1727 = UNIT;  }
                {
                  z3zE1722 = zpush_word(zWORD_ZERO);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE1681);
                    goto end_block_exception_2099;
                  }
                }
              }
              goto finish_match_2077;
            }
          case_2079: ;
            {
              unit z3zE1724;
              z3zE1724 = zreturndata_clear(UNIT);
              unit z3zE1723;
              if (znew_account_charged) {
                {
                  z3zE1723 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
                  if (have_exception) {
                    KILL(zoptionzIU64zK)(&z2zE1681);
                    goto end_block_exception_2099;
                  }
                }
              } else {  z3zE1723 = UNIT;  }
              {
                z3zE1722 = zpush_word(zWORD_ZERO);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1681);
                  goto end_block_exception_2099;
                }
              }
              goto finish_match_2077;
            }
          case_2078: ;
          finish_match_2077: ;
            z3zE1701 = z3zE1722;
            KILL(zoptionzIU64zK)(&z2zE1681);
          } else {
            struct zCode zchild_code;
            {
              zchild_code = zexecutable_code(ztarget, ztg_deleg, ztg_target);
              if (have_exception) {  goto end_block_exception_2099;  }
            }
            sail_fixed_bytes_20 zchild_addr;
            {
              bool z2zE1706;
              z2zE1706 = zcall_uses_target_address(zkind);
              if (z2zE1706) {  zchild_addr = ztarget;  } else {  zchild_addr = zcaller;  }
            }
            sail_fixed_bytes_20 zchild_caller;
            {
              bool z2zE1705;
              z2zE1705 = zcall_is_delegate(zkind);
              if (z2zE1705) {  zchild_caller = zmessage.zcaller;  } else {  zchild_caller = zcaller;  }
            }
            sail_u256 zchild_value;
            {
              bool z2zE1704;
              z2zE1704 = zcall_is_delegate(zkind);
              if (z2zE1704) {  zchild_value = zmessage.zvalue;  } else {  zchild_value = zvalue;  }
            }
            bool zchild_static;
            {
              bool z2zE1703;
              z2zE1703 = zcall_is_static(zkind);
              if (z2zE1703) {  zchild_static = true;  } else {  zchild_static = zmessage.zis_static;  }
            }
            struct zByteSliceFields zchild_calldata;
            {
              bool z2zE1700;
              z2zE1700 = zis_running(UNIT);
              if (z2zE1700) {
                uint64_t z2zE1701;
                z2zE1701 = zargs.zoff;
                uint64_t z2zE1702;
                z2zE1702 = zargs.zlen;
                zchild_calldata = zmemory_byte_slice(z2zE1701, z2zE1702);
              } else {  zchild_calldata = zEMPTY_SLICE;  }
            }
            struct zFrameCheckpoint zcheckpoint;
            CREATE(zFrameCheckpoint)(&zcheckpoint);
            {
              struct zFrameCheckpoint z2zE1699;
              CREATE(zFrameCheckpoint)(&z2zE1699);
              zsuspend_frame(&z2zE1699, UNIT);
              struct zFrameCheckpoint z3zE1706;
              CREATE(zFrameCheckpoint)(&z3zE1706);
              z3zE1706 = z2zE1699;
              z3zE1706.zstate_gas_remaining = zGAS_ZERO;
              zcheckpoint = z3zE1706;
            }
            struct zFrameContinuation z2zE1693;
            CREATE(zFrameContinuation)(&z2zE1693);
            {
              struct zCallContinuation z2zE1692;
              CREATE(zCallContinuation)(&z2zE1692);
              {
                uint64_t z2zE1690;
                z2zE1690 = zret.zoff;
                uint64_t z2zE1691;
                z2zE1691 = zret.zlen;
                z2zE1692.zcheckpoint = zcheckpoint;
                z2zE1692.znew_account_charged = znew_account_charged;
                z2zE1692.zreturn_length = z2zE1691;
                z2zE1692.zreturn_offset = z2zE1690;
              }
              zResumeCall(&z2zE1693, z2zE1692);
            }
            unit z3zE1721;
            z3zE1721 = frame_stack_push(z2zE1693);
            bool z2zE1695;
            {
              bool z2zE1694;
              z2zE1694 = zcall_transfers_value(zkind);
              bool z3zE1708;
              if (z2zE1694) {  z3zE1708 = zword_nonzzero(zvalue);  } else {  z3zE1708 = false;  }
              z2zE1695 = z3zE1708;
            }
            unit z3zE1720;
            if (z2zE1695) {
              {
                z3zE1720 = zk_transfer(zcaller, ztarget, zvalue);
                if (have_exception) {  goto end_block_exception_2099;  }
              }
            } else {  z3zE1720 = UNIT;  }
            uint64_t z2zE1697;
            {
              uint64_t z2zE1696;
              z2zE1696 = zcheckpoint.zcall_depth;
              z2zE1697 = zframe_depth_increment(z2zE1696);
            }
            struct zMessage z3zE1709;
            z3zE1709.zaddress = zchild_addr;
            z3zE1709.zcaller = zchild_caller;
            z3zE1709.zcode_address = ztarget;
            z3zE1709.zdepth = z2zE1697;
            z3zE1709.zis_static = zchild_static;
            z3zE1709.zstate_gas_reservoir = zstate_gas_remaining;
            z3zE1709.zvalue = zchild_value;
            zmessage = z3zE1709;
            unit z3zE1719;
            z3zE1719 = UNIT;
            unit z3zE1718;
            z3zE1718 = zcalldata_install(zchild_calldata);
            zpc = UINT64_C(0);
            unit z3zE1717;
            z3zE1717 = UNIT;
            zgas_remaining = zchild_gas;
            unit z3zE1716;
            z3zE1716 = UNIT;
            unit z3zE1715;
            z3zE1715 = UNIT;
            zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
            unit z3zE1714;
            z3zE1714 = UNIT;
            zRunning(&zframe_status, UNIT);
            unit z3zE1713;
            z3zE1713 = UNIT;
            unit z3zE1712;
            z3zE1712 = zreturndata_clear(UNIT);
            zframe_code = zchild_code;
            unit z3zE1711;
            z3zE1711 = UNIT;
            uint64_t z2zE1698;
            z2zE1698 = zcheckpoint.zcall_depth;
            zcall_depth = zframe_depth_increment(z2zE1698);
            unit z3zE1710;
            z3zE1710 = UNIT;
            zframe_refund = zGAS_REFUND_ZERO;
            z3zE1701 = UNIT;
          }
        }
        goto finish_match_2075;
      }
    case_2076: ;
      sail_match_failure("run_call");
    finish_match_2075: ;
      z3zE1700 = z3zE1701;
      goto finish_match_2073;
    }
  case_2074: ;
    sail_match_failure("run_call");
  finish_match_2073: ;
    z3zE1675 = z3zE1700;
    goto finish_match_2067;
  }
case_2068: ;
  sail_match_failure("run_call");
finish_match_2067: ;
  z8zE477 = z3zE1675;
end_function_2080: ;
  return z8zE477;
end_block_exception_2099: ;

  return UNIT;
}

unit zexecute_call(unit z3zE1664)
{
  unit z8zE478;
  {
    z8zE478 = zrun_call(zCall);
    if (have_exception) {  goto end_block_exception_2066;  }
  }
end_function_2065: ;
  return z8zE478;
end_block_exception_2066: ;

  return UNIT;
}

unit zexecute_callcode(unit z3zE1663)
{
  unit z8zE479;
  {
    z8zE479 = zrun_call(zCallCode);
    if (have_exception) {  goto end_block_exception_2064;  }
  }
end_function_2063: ;
  return z8zE479;
end_block_exception_2064: ;

  return UNIT;
}

unit zexecute_delegatecall(unit z3zE1662)
{
  unit z8zE480;
  {
    z8zE480 = zrun_call(zDelegateCall);
    if (have_exception) {  goto end_block_exception_2062;  }
  }
end_function_2061: ;
  return z8zE480;
end_block_exception_2062: ;

  return UNIT;
}

unit zexecute_staticcall(unit z3zE1661)
{
  unit z8zE481;
  {
    z8zE481 = zrun_call(zStaticCall);
    if (have_exception) {  goto end_block_exception_2060;  }
  }
end_function_2059: ;
  return z8zE481;
end_block_exception_2060: ;

  return UNIT;
}

unit zexecute_opcode(struct zast zop)
{
  unit z8zE482;
  unit z3zE1567;
  {
    if (zop.kind != Kind_zSTOP) goto case_2056;
    z3zE1567 = zexecute_stop(UNIT);
    goto finish_match_1968;
  }
case_2056: ;
  {
    if (zop.kind != Kind_zADD) goto case_2055;
    {
      z3zE1567 = zexecute_add(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2055: ;
  {
    if (zop.kind != Kind_zMUL) goto case_2054;
    {
      z3zE1567 = zexecute_mul(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2054: ;
  {
    if (zop.kind != Kind_zSUB) goto case_2053;
    {
      z3zE1567 = zexecute_sub(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2053: ;
  {
    if (zop.kind != Kind_zDIV) goto case_2052;
    {
      z3zE1567 = zexecute_div(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2052: ;
  {
    if (zop.kind != Kind_zSDIV) goto case_2051;
    {
      z3zE1567 = zexecute_sdiv(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2051: ;
  {
    if (zop.kind != Kind_zMOD) goto case_2050;
    {
      z3zE1567 = zexecute_mod(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2050: ;
  {
    if (zop.kind != Kind_zSMOD) goto case_2049;
    {
      z3zE1567 = zexecute_smod(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2049: ;
  {
    if (zop.kind != Kind_zADDMOD) goto case_2048;
    {
      z3zE1567 = zexecute_addmod(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2048: ;
  {
    if (zop.kind != Kind_zMULMOD) goto case_2047;
    {
      z3zE1567 = zexecute_mulmod(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2047: ;
  {
    if (zop.kind != Kind_zEXP) goto case_2046;
    {
      z3zE1567 = zexecute_exp(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2046: ;
  {
    if (zop.kind != Kind_zSIGNEXTEND) goto case_2045;
    {
      z3zE1567 = zexecute_signextend(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2045: ;
  {
    if (zop.kind != Kind_zLT) goto case_2044;
    {
      z3zE1567 = zexecute_lt(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2044: ;
  {
    if (zop.kind != Kind_zGT) goto case_2043;
    {
      z3zE1567 = zexecute_gt(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2043: ;
  {
    if (zop.kind != Kind_zSLT) goto case_2042;
    {
      z3zE1567 = zexecute_slt(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2042: ;
  {
    if (zop.kind != Kind_zSGT) goto case_2041;
    {
      z3zE1567 = zexecute_sgt(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2041: ;
  {
    if (zop.kind != Kind_zEQ) goto case_2040;
    {
      z3zE1567 = zexecute_eq(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2040: ;
  {
    if (zop.kind != Kind_zISZERO) goto case_2039;
    {
      z3zE1567 = zexecute_iszzero(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2039: ;
  {
    if (zop.kind != Kind_zAND) goto case_2038;
    {
      z3zE1567 = zexecute_and(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2038: ;
  {
    if (zop.kind != Kind_zOR) goto case_2037;
    {
      z3zE1567 = zexecute_or(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2037: ;
  {
    if (zop.kind != Kind_zXOR) goto case_2036;
    {
      z3zE1567 = zexecute_xor(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2036: ;
  {
    if (zop.kind != Kind_zNOT) goto case_2035;
    {
      z3zE1567 = zexecute_not(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2035: ;
  {
    if (zop.kind != Kind_zBYTE) goto case_2034;
    {
      z3zE1567 = zexecute_byte(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2034: ;
  {
    if (zop.kind != Kind_zSHL) goto case_2033;
    {
      z3zE1567 = zexecute_shl(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2033: ;
  {
    if (zop.kind != Kind_zSHR) goto case_2032;
    {
      z3zE1567 = zexecute_shr(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2032: ;
  {
    if (zop.kind != Kind_zSAR) goto case_2031;
    {
      z3zE1567 = zexecute_sar(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2031: ;
  {
    if (zop.kind != Kind_zCLZ) goto case_2030;
    {
      z3zE1567 = zexecute_clzz(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2030: ;
  {
    if (zop.kind != Kind_zKECCAK256) goto case_2029;
    {
      z3zE1567 = zexecute_keccak256(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2029: ;
  {
    if (zop.kind != Kind_zADDRESS) goto case_2028;
    {
      z3zE1567 = zexecute_address(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2028: ;
  {
    if (zop.kind != Kind_zORIGIN) goto case_2027;
    {
      z3zE1567 = zexecute_origin(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2027: ;
  {
    if (zop.kind != Kind_zCALLER) goto case_2026;
    {
      z3zE1567 = zexecute_caller(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2026: ;
  {
    if (zop.kind != Kind_zCALLVALUE) goto case_2025;
    {
      z3zE1567 = zexecute_callvalue(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2025: ;
  {
    if (zop.kind != Kind_zGASPRICE) goto case_2024;
    {
      z3zE1567 = zexecute_gasprice(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2024: ;
  {
    if (zop.kind != Kind_zCALLDATASIZE) goto case_2023;
    {
      z3zE1567 = zexecute_calldatasizze(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2023: ;
  {
    if (zop.kind != Kind_zCALLDATALOAD) goto case_2022;
    {
      z3zE1567 = zexecute_calldataload(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2022: ;
  {
    if (zop.kind != Kind_zCALLDATACOPY) goto case_2021;
    {
      z3zE1567 = zexecute_calldatacopy(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2021: ;
  {
    if (zop.kind != Kind_zCODESIZE) goto case_2020;
    {
      z3zE1567 = zexecute_codesizze(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2020: ;
  {
    if (zop.kind != Kind_zCODECOPY) goto case_2019;
    {
      z3zE1567 = zexecute_codecopy(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2019: ;
  {
    if (zop.kind != Kind_zBALANCE) goto case_2018;
    {
      z3zE1567 = zexecute_balance(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2018: ;
  {
    if (zop.kind != Kind_zSELFBALANCE) goto case_2017;
    {
      z3zE1567 = zexecute_selfbalance(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2017: ;
  {
    if (zop.kind != Kind_zEXTCODESIZE) goto case_2016;
    {
      z3zE1567 = zexecute_extcodesizze(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2016: ;
  {
    if (zop.kind != Kind_zEXTCODECOPY) goto case_2015;
    {
      z3zE1567 = zexecute_extcodecopy(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2015: ;
  {
    if (zop.kind != Kind_zEXTCODEHASH) goto case_2014;
    {
      z3zE1567 = zexecute_extcodehash(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2014: ;
  {
    if (zop.kind != Kind_zRETURNDATASIZE) goto case_2013;
    {
      z3zE1567 = zexecute_returndatasizze(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2013: ;
  {
    if (zop.kind != Kind_zRETURNDATACOPY) goto case_2012;
    {
      z3zE1567 = zexecute_returndatacopy(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2012: ;
  {
    if (zop.kind != Kind_zBLOCKHASH) goto case_2011;
    {
      z3zE1567 = zexecute_blockhash(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2011: ;
  {
    if (zop.kind != Kind_zCOINBASE) goto case_2010;
    {
      z3zE1567 = zexecute_coinbase(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2010: ;
  {
    if (zop.kind != Kind_zTIMESTAMP) goto case_2009;
    {
      z3zE1567 = zexecute_timestamp(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2009: ;
  {
    if (zop.kind != Kind_zNUMBER) goto case_2008;
    {
      z3zE1567 = zexecute_number(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2008: ;
  {
    if (zop.kind != Kind_zSLOTNUM) goto case_2007;
    {
      z3zE1567 = zexecute_slotnum(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2007: ;
  {
    if (zop.kind != Kind_zPREVRANDAO) goto case_2006;
    {
      z3zE1567 = zexecute_prevrandao(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2006: ;
  {
    if (zop.kind != Kind_zGASLIMIT) goto case_2005;
    {
      z3zE1567 = zexecute_gaslimit(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2005: ;
  {
    if (zop.kind != Kind_zCHAINID) goto case_2004;
    {
      z3zE1567 = zexecute_chainid(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2004: ;
  {
    if (zop.kind != Kind_zBASEFEE) goto case_2003;
    {
      z3zE1567 = zexecute_basefee(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2003: ;
  {
    if (zop.kind != Kind_zBLOBBASEFEE) goto case_2002;
    {
      z3zE1567 = zexecute_blobbasefee(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2002: ;
  {
    if (zop.kind != Kind_zBLOBHASH) goto case_2001;
    {
      z3zE1567 = zexecute_blobhash(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2001: ;
  {
    if (zop.kind != Kind_zPOP) goto case_2000;
    {
      z3zE1567 = zexecute_pop(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_2000: ;
  {
    if (zop.kind != Kind_zMLOAD) goto case_1999;
    {
      z3zE1567 = zexecute_mload(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1999: ;
  {
    if (zop.kind != Kind_zMSTORE) goto case_1998;
    {
      z3zE1567 = zexecute_mstore(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1998: ;
  {
    if (zop.kind != Kind_zMSTORE8) goto case_1997;
    {
      z3zE1567 = zexecute_mstore8(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1997: ;
  {
    if (zop.kind != Kind_zMSIZE) goto case_1996;
    {
      z3zE1567 = zexecute_msizze(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1996: ;
  {
    if (zop.kind != Kind_zMCOPY) goto case_1995;
    {
      z3zE1567 = zexecute_mcopy(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1995: ;
  {
    if (zop.kind != Kind_zSLOAD) goto case_1994;
    {
      z3zE1567 = zexecute_sload(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1994: ;
  {
    if (zop.kind != Kind_zSSTORE) goto case_1993;
    {
      z3zE1567 = zexecute_sstore(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1993: ;
  {
    if (zop.kind != Kind_zTLOAD) goto case_1992;
    {
      z3zE1567 = zexecute_tload(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1992: ;
  {
    if (zop.kind != Kind_zTSTORE) goto case_1991;
    {
      z3zE1567 = zexecute_tstore(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1991: ;
  {
    if (zop.kind != Kind_zJUMP) goto case_1990;
    {
      z3zE1567 = zexecute_jump(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1990: ;
  {
    if (zop.kind != Kind_zJUMPI) goto case_1989;
    {
      z3zE1567 = zexecute_jumpi(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1989: ;
  {
    if (zop.kind != Kind_zPC) goto case_1988;
    {
      z3zE1567 = zexecute_pc(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1988: ;
  {
    if (zop.kind != Kind_zGAS) goto case_1987;
    {
      z3zE1567 = zexecute_gas(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1987: ;
  {
    if (zop.kind != Kind_zJUMPDEST) goto case_1986;
    {
      z3zE1567 = zexecute_jumpdest(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1986: ;
  {
    if (zop.kind != Kind_zPUSH) goto case_1985;
    uint64_t zn;
    zn = zop.variants.zPUSH.ztup0;
    sail_u256 zv;
    zv = zop.variants.zPUSH.ztup1;
    {
      z3zE1567 = zexecute_push(zn, zv);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1985: ;
  {
    if (zop.kind != Kind_zDUP) goto case_1984;
    uint64_t z3zE1656;
    z3zE1656 = zop.variants.zDUP;
    {
      z3zE1567 = zexecute_dup(z3zE1656);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1984: ;
  {
    if (zop.kind != Kind_zSWAP) goto case_1983;
    uint64_t z3zE1657;
    z3zE1657 = zop.variants.zSWAP;
    {
      z3zE1567 = zexecute_swap(z3zE1657);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1983: ;
  {
    if (zop.kind != Kind_zDUPN) goto case_1982;
    uint64_t zimmediate;
    zimmediate = zop.variants.zDUPN;
    {
      z3zE1567 = zexecute_dupn(zimmediate);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1982: ;
  {
    if (zop.kind != Kind_zSWAPN) goto case_1981;
    uint64_t z3zE1658;
    z3zE1658 = zop.variants.zSWAPN;
    {
      z3zE1567 = zexecute_swapn(z3zE1658);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1981: ;
  {
    if (zop.kind != Kind_zEXCHANGE) goto case_1980;
    uint64_t z3zE1659;
    z3zE1659 = zop.variants.zEXCHANGE;
    {
      z3zE1567 = zexecute_exchange(z3zE1659);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1980: ;
  {
    if (zop.kind != Kind_zLOG) goto case_1979;
    uint64_t z3zE1660;
    z3zE1660 = zop.variants.zLOG;
    {
      z3zE1567 = zexecute_log(z3zE1660);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1979: ;
  {
    if (zop.kind != Kind_zCREATE) goto case_1978;
    {
      z3zE1567 = zexecute_create(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1978: ;
  {
    if (zop.kind != Kind_zCREATE2) goto case_1977;
    {
      z3zE1567 = zexecute_create2(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1977: ;
  {
    if (zop.kind != Kind_zCALL) goto case_1976;
    {
      z3zE1567 = zexecute_call(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1976: ;
  {
    if (zop.kind != Kind_zCALLCODE) goto case_1975;
    {
      z3zE1567 = zexecute_callcode(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1975: ;
  {
    if (zop.kind != Kind_zDELEGATECALL) goto case_1974;
    {
      z3zE1567 = zexecute_delegatecall(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1974: ;
  {
    if (zop.kind != Kind_zSTATICCALL) goto case_1973;
    {
      z3zE1567 = zexecute_staticcall(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1973: ;
  {
    if (zop.kind != Kind_zRETURN) goto case_1972;
    {
      z3zE1567 = zexecute_return(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1972: ;
  {
    if (zop.kind != Kind_zREVERT) goto case_1971;
    {
      z3zE1567 = zexecute_revert(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1971: ;
  {
    if (zop.kind != Kind_zINVALID) goto case_1970;
    {
      z3zE1567 = zexecute_invalid(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1970: ;
  {
    /* complete */
    {
      z3zE1567 = zexecute_selfdestruct(UNIT);
      if (have_exception) {  goto end_block_exception_2058;  }
    }
    goto finish_match_1968;
  }
case_1969: ;
finish_match_1968: ;
  z8zE482 = z3zE1567;
end_function_2057: ;
  return z8zE482;
end_block_exception_2058: ;

  return UNIT;
}

unit zexecute(struct zast zop)
{
  unit z8zE483;
  {
    z8zE483 = zexecute_opcode(zop);
    if (have_exception) {  goto end_block_exception_1967;  }
  }
end_function_1966: ;
  return z8zE483;
end_block_exception_1967: ;

  return UNIT;
}

