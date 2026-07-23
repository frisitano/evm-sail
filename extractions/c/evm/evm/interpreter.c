/* Generated from sail/evm/interpreter.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_u256 zread_push(struct zByteSliceFields zcode, uint64_t zoffset, uint64_t zn)
{
  sail_u256 z8zE440;
  z8zE440 = zslice_load_n(zcode, zoffset, zn);
end_function_2294: ;
  return z8zE440;
end_block_exception_2295: ;

  return u256_zero();
}

void zdecode_simple(struct zast *z8zE441, uint64_t zopcode)
{
  bool z2zE2094;
  {
    bool z2zE2093;
    z2zE2093 = (!(zopcode < UINT64_C(128)));
    bool z3zE1906;
    if (z2zE2093) {  z3zE1906 = (!(UINT64_C(143) < zopcode));  } else {  z3zE1906 = false;  }
    z2zE2094 = z3zE1906;
  }
  if (z2zE2094) {
    uint64_t z2zE2095;
    {    z2zE2095 = (zopcode - UINT64_C(127));
    }
    zDUP((*(&z8zE441)), z2zE2095);
  } else {
    bool z2zE2097;
    {
      bool z2zE2096;
      z2zE2096 = (!(zopcode < UINT64_C(144)));
      bool z3zE1907;
      if (z2zE2096) {  z3zE1907 = (!(UINT64_C(159) < zopcode));  } else {  z3zE1907 = false;  }
      z2zE2097 = z3zE1907;
    }
    if (z2zE2097) {
      uint64_t z2zE2098;
      {    z2zE2098 = (zopcode - UINT64_C(143));
      }
      zSWAP((*(&z8zE441)), z2zE2098);
    } else {
      bool z2zE2100;
      {
        bool z2zE2099;
        z2zE2099 = (!(zopcode < UINT64_C(160)));
        bool z3zE1908;
        if (z2zE2099) {  z3zE1908 = (!(UINT64_C(164) < zopcode));  } else {  z3zE1908 = false;  }
        z2zE2100 = z3zE1908;
      }
      if (z2zE2100) {
        uint64_t z2zE2101;
        {    z2zE2101 = (zopcode - UINT64_C(160));
        }
        zLOG((*(&z8zE441)), z2zE2101);
      } else {
        struct zast z3zE1909;
        CREATE(zast)(&z3zE1909);
        {
          uint64_t zp0z3;
          zp0z3 = zopcode;
          bool z3zE1990;
          z3zE1990 = (zp0z3 == UINT64_C(0));
          if (!(z3zE1990)) {  goto case_2291;  }
          zSTOP(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2291: ;
        {
          uint64_t z3zE1991;
          z3zE1991 = zopcode;
          bool z3zE1989;
          z3zE1989 = (z3zE1991 == UINT64_C(1));
          if (!(z3zE1989)) {  goto case_2290;  }
          zADD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2290: ;
        {
          uint64_t z3zE1992;
          z3zE1992 = zopcode;
          bool z3zE1988;
          z3zE1988 = (z3zE1992 == UINT64_C(2));
          if (!(z3zE1988)) {  goto case_2289;  }
          zMUL(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2289: ;
        {
          uint64_t z3zE1993;
          z3zE1993 = zopcode;
          bool z3zE1987;
          z3zE1987 = (z3zE1993 == UINT64_C(3));
          if (!(z3zE1987)) {  goto case_2288;  }
          zSUB(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2288: ;
        {
          uint64_t z3zE1994;
          z3zE1994 = zopcode;
          bool z3zE1986;
          z3zE1986 = (z3zE1994 == UINT64_C(4));
          if (!(z3zE1986)) {  goto case_2287;  }
          zDIV(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2287: ;
        {
          uint64_t z3zE1995;
          z3zE1995 = zopcode;
          bool z3zE1985;
          z3zE1985 = (z3zE1995 == UINT64_C(5));
          if (!(z3zE1985)) {  goto case_2286;  }
          zSDIV(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2286: ;
        {
          uint64_t z3zE1996;
          z3zE1996 = zopcode;
          bool z3zE1984;
          z3zE1984 = (z3zE1996 == UINT64_C(6));
          if (!(z3zE1984)) {  goto case_2285;  }
          zMOD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2285: ;
        {
          uint64_t z3zE1997;
          z3zE1997 = zopcode;
          bool z3zE1983;
          z3zE1983 = (z3zE1997 == UINT64_C(7));
          if (!(z3zE1983)) {  goto case_2284;  }
          zSMOD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2284: ;
        {
          uint64_t z3zE1998;
          z3zE1998 = zopcode;
          bool z3zE1982;
          z3zE1982 = (z3zE1998 == UINT64_C(8));
          if (!(z3zE1982)) {  goto case_2283;  }
          zADDMOD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2283: ;
        {
          uint64_t z3zE1999;
          z3zE1999 = zopcode;
          bool z3zE1981;
          z3zE1981 = (z3zE1999 == UINT64_C(9));
          if (!(z3zE1981)) {  goto case_2282;  }
          zMULMOD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2282: ;
        {
          uint64_t z3zE2000;
          z3zE2000 = zopcode;
          bool z3zE1980;
          z3zE1980 = (z3zE2000 == UINT64_C(10));
          if (!(z3zE1980)) {  goto case_2281;  }
          zEXP(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2281: ;
        {
          uint64_t z3zE2001;
          z3zE2001 = zopcode;
          bool z3zE1979;
          z3zE1979 = (z3zE2001 == UINT64_C(11));
          if (!(z3zE1979)) {  goto case_2280;  }
          zSIGNEXTEND(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2280: ;
        {
          uint64_t z3zE2002;
          z3zE2002 = zopcode;
          bool z3zE1978;
          z3zE1978 = (z3zE2002 == UINT64_C(16));
          if (!(z3zE1978)) {  goto case_2279;  }
          zLT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2279: ;
        {
          uint64_t z3zE2003;
          z3zE2003 = zopcode;
          bool z3zE1977;
          z3zE1977 = (z3zE2003 == UINT64_C(17));
          if (!(z3zE1977)) {  goto case_2278;  }
          zGT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2278: ;
        {
          uint64_t z3zE2004;
          z3zE2004 = zopcode;
          bool z3zE1976;
          z3zE1976 = (z3zE2004 == UINT64_C(18));
          if (!(z3zE1976)) {  goto case_2277;  }
          zSLT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2277: ;
        {
          uint64_t z3zE2005;
          z3zE2005 = zopcode;
          bool z3zE1975;
          z3zE1975 = (z3zE2005 == UINT64_C(19));
          if (!(z3zE1975)) {  goto case_2276;  }
          zSGT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2276: ;
        {
          uint64_t z3zE2006;
          z3zE2006 = zopcode;
          bool z3zE1974;
          z3zE1974 = (z3zE2006 == UINT64_C(20));
          if (!(z3zE1974)) {  goto case_2275;  }
          zEQ(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2275: ;
        {
          uint64_t z3zE2007;
          z3zE2007 = zopcode;
          bool z3zE1973;
          z3zE1973 = (z3zE2007 == UINT64_C(21));
          if (!(z3zE1973)) {  goto case_2274;  }
          zISZERO(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2274: ;
        {
          uint64_t z3zE2008;
          z3zE2008 = zopcode;
          bool z3zE1972;
          z3zE1972 = (z3zE2008 == UINT64_C(22));
          if (!(z3zE1972)) {  goto case_2273;  }
          zAND(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2273: ;
        {
          uint64_t z3zE2009;
          z3zE2009 = zopcode;
          bool z3zE1971;
          z3zE1971 = (z3zE2009 == UINT64_C(23));
          if (!(z3zE1971)) {  goto case_2272;  }
          zOR(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2272: ;
        {
          uint64_t z3zE2010;
          z3zE2010 = zopcode;
          bool z3zE1970;
          z3zE1970 = (z3zE2010 == UINT64_C(24));
          if (!(z3zE1970)) {  goto case_2271;  }
          zXOR(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2271: ;
        {
          uint64_t z3zE2011;
          z3zE2011 = zopcode;
          bool z3zE1969;
          z3zE1969 = (z3zE2011 == UINT64_C(25));
          if (!(z3zE1969)) {  goto case_2270;  }
          zNOT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2270: ;
        {
          uint64_t z3zE2012;
          z3zE2012 = zopcode;
          bool z3zE1968;
          z3zE1968 = (z3zE2012 == UINT64_C(26));
          if (!(z3zE1968)) {  goto case_2269;  }
          zBYTE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2269: ;
        {
          uint64_t z3zE2013;
          z3zE2013 = zopcode;
          bool z3zE1967;
          z3zE1967 = (z3zE2013 == UINT64_C(27));
          if (!(z3zE1967)) {  goto case_2268;  }
          zSHL(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2268: ;
        {
          uint64_t z3zE2014;
          z3zE2014 = zopcode;
          bool z3zE1966;
          z3zE1966 = (z3zE2014 == UINT64_C(28));
          if (!(z3zE1966)) {  goto case_2267;  }
          zSHR(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2267: ;
        {
          uint64_t z3zE2015;
          z3zE2015 = zopcode;
          bool z3zE1965;
          z3zE1965 = (z3zE2015 == UINT64_C(29));
          if (!(z3zE1965)) {  goto case_2266;  }
          zSAR(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2266: ;
        {
          uint64_t z3zE2016;
          z3zE2016 = zopcode;
          bool z3zE1964;
          z3zE1964 = (z3zE2016 == UINT64_C(30));
          if (!(z3zE1964)) {  goto case_2265;  }
          bool z2zE2102;
          z2zE2102 = zfork_gteq(zk_fork, zOsaka);
          if (z2zE2102) {  zCLZ(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2265: ;
        {
          uint64_t z3zE2017;
          z3zE2017 = zopcode;
          bool z3zE1963;
          z3zE1963 = (z3zE2017 == UINT64_C(32));
          if (!(z3zE1963)) {  goto case_2264;  }
          zKECCAK256(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2264: ;
        {
          uint64_t z3zE2018;
          z3zE2018 = zopcode;
          bool z3zE1962;
          z3zE1962 = (z3zE2018 == UINT64_C(48));
          if (!(z3zE1962)) {  goto case_2263;  }
          zADDRESS(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2263: ;
        {
          uint64_t z3zE2019;
          z3zE2019 = zopcode;
          bool z3zE1961;
          z3zE1961 = (z3zE2019 == UINT64_C(49));
          if (!(z3zE1961)) {  goto case_2262;  }
          zBALANCE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2262: ;
        {
          uint64_t z3zE2020;
          z3zE2020 = zopcode;
          bool z3zE1960;
          z3zE1960 = (z3zE2020 == UINT64_C(50));
          if (!(z3zE1960)) {  goto case_2261;  }
          zORIGIN(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2261: ;
        {
          uint64_t z3zE2021;
          z3zE2021 = zopcode;
          bool z3zE1959;
          z3zE1959 = (z3zE2021 == UINT64_C(51));
          if (!(z3zE1959)) {  goto case_2260;  }
          zCALLER(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2260: ;
        {
          uint64_t z3zE2022;
          z3zE2022 = zopcode;
          bool z3zE1958;
          z3zE1958 = (z3zE2022 == UINT64_C(52));
          if (!(z3zE1958)) {  goto case_2259;  }
          zCALLVALUE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2259: ;
        {
          uint64_t z3zE2023;
          z3zE2023 = zopcode;
          bool z3zE1957;
          z3zE1957 = (z3zE2023 == UINT64_C(53));
          if (!(z3zE1957)) {  goto case_2258;  }
          zCALLDATALOAD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2258: ;
        {
          uint64_t z3zE2024;
          z3zE2024 = zopcode;
          bool z3zE1956;
          z3zE1956 = (z3zE2024 == UINT64_C(54));
          if (!(z3zE1956)) {  goto case_2257;  }
          zCALLDATASIZE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2257: ;
        {
          uint64_t z3zE2025;
          z3zE2025 = zopcode;
          bool z3zE1955;
          z3zE1955 = (z3zE2025 == UINT64_C(55));
          if (!(z3zE1955)) {  goto case_2256;  }
          zCALLDATACOPY(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2256: ;
        {
          uint64_t z3zE2026;
          z3zE2026 = zopcode;
          bool z3zE1954;
          z3zE1954 = (z3zE2026 == UINT64_C(56));
          if (!(z3zE1954)) {  goto case_2255;  }
          zCODESIZE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2255: ;
        {
          uint64_t z3zE2027;
          z3zE2027 = zopcode;
          bool z3zE1953;
          z3zE1953 = (z3zE2027 == UINT64_C(57));
          if (!(z3zE1953)) {  goto case_2254;  }
          zCODECOPY(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2254: ;
        {
          uint64_t z3zE2028;
          z3zE2028 = zopcode;
          bool z3zE1952;
          z3zE1952 = (z3zE2028 == UINT64_C(58));
          if (!(z3zE1952)) {  goto case_2253;  }
          zGASPRICE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2253: ;
        {
          uint64_t z3zE2029;
          z3zE2029 = zopcode;
          bool z3zE1951;
          z3zE1951 = (z3zE2029 == UINT64_C(59));
          if (!(z3zE1951)) {  goto case_2252;  }
          zEXTCODESIZE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2252: ;
        {
          uint64_t z3zE2030;
          z3zE2030 = zopcode;
          bool z3zE1950;
          z3zE1950 = (z3zE2030 == UINT64_C(60));
          if (!(z3zE1950)) {  goto case_2251;  }
          zEXTCODECOPY(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2251: ;
        {
          uint64_t z3zE2031;
          z3zE2031 = zopcode;
          bool z3zE1949;
          z3zE1949 = (z3zE2031 == UINT64_C(61));
          if (!(z3zE1949)) {  goto case_2250;  }
          zRETURNDATASIZE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2250: ;
        {
          uint64_t z3zE2032;
          z3zE2032 = zopcode;
          bool z3zE1948;
          z3zE1948 = (z3zE2032 == UINT64_C(62));
          if (!(z3zE1948)) {  goto case_2249;  }
          zRETURNDATACOPY(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2249: ;
        {
          uint64_t z3zE2033;
          z3zE2033 = zopcode;
          bool z3zE1947;
          z3zE1947 = (z3zE2033 == UINT64_C(63));
          if (!(z3zE1947)) {  goto case_2248;  }
          zEXTCODEHASH(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2248: ;
        {
          uint64_t z3zE2034;
          z3zE2034 = zopcode;
          bool z3zE1946;
          z3zE1946 = (z3zE2034 == UINT64_C(64));
          if (!(z3zE1946)) {  goto case_2247;  }
          zBLOCKHASH(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2247: ;
        {
          uint64_t z3zE2035;
          z3zE2035 = zopcode;
          bool z3zE1945;
          z3zE1945 = (z3zE2035 == UINT64_C(65));
          if (!(z3zE1945)) {  goto case_2246;  }
          zCOINBASE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2246: ;
        {
          uint64_t z3zE2036;
          z3zE2036 = zopcode;
          bool z3zE1944;
          z3zE1944 = (z3zE2036 == UINT64_C(66));
          if (!(z3zE1944)) {  goto case_2245;  }
          zTIMESTAMP(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2245: ;
        {
          uint64_t z3zE2037;
          z3zE2037 = zopcode;
          bool z3zE1943;
          z3zE1943 = (z3zE2037 == UINT64_C(67));
          if (!(z3zE1943)) {  goto case_2244;  }
          zNUMBER(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2244: ;
        {
          uint64_t z3zE2038;
          z3zE2038 = zopcode;
          bool z3zE1942;
          z3zE1942 = (z3zE2038 == UINT64_C(68));
          if (!(z3zE1942)) {  goto case_2243;  }
          zPREVRANDAO(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2243: ;
        {
          uint64_t z3zE2039;
          z3zE2039 = zopcode;
          bool z3zE1941;
          z3zE1941 = (z3zE2039 == UINT64_C(69));
          if (!(z3zE1941)) {  goto case_2242;  }
          zGASLIMIT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2242: ;
        {
          uint64_t z3zE2040;
          z3zE2040 = zopcode;
          bool z3zE1940;
          z3zE1940 = (z3zE2040 == UINT64_C(70));
          if (!(z3zE1940)) {  goto case_2241;  }
          zCHAINID(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2241: ;
        {
          uint64_t z3zE2041;
          z3zE2041 = zopcode;
          bool z3zE1939;
          z3zE1939 = (z3zE2041 == UINT64_C(71));
          if (!(z3zE1939)) {  goto case_2240;  }
          zSELFBALANCE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2240: ;
        {
          uint64_t z3zE2042;
          z3zE2042 = zopcode;
          bool z3zE1938;
          z3zE1938 = (z3zE2042 == UINT64_C(72));
          if (!(z3zE1938)) {  goto case_2239;  }
          bool z2zE2103;
          z2zE2103 = zfork_gteq(zk_fork, zLondon);
          if (z2zE2103) {  zBASEFEE(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2239: ;
        {
          uint64_t z3zE2043;
          z3zE2043 = zopcode;
          bool z3zE1937;
          z3zE1937 = (z3zE2043 == UINT64_C(73));
          if (!(z3zE1937)) {  goto case_2238;  }
          bool z2zE2104;
          z2zE2104 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2104) {  zBLOBHASH(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2238: ;
        {
          uint64_t z3zE2044;
          z3zE2044 = zopcode;
          bool z3zE1936;
          z3zE1936 = (z3zE2044 == UINT64_C(74));
          if (!(z3zE1936)) {  goto case_2237;  }
          bool z2zE2105;
          z2zE2105 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2105) {  zBLOBBASEFEE(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2237: ;
        {
          uint64_t z3zE2045;
          z3zE2045 = zopcode;
          bool z3zE1935;
          z3zE1935 = (z3zE2045 == UINT64_C(75));
          if (!(z3zE1935)) {  goto case_2236;  }
          bool z2zE2106;
          z2zE2106 = zfork_gteq(zk_fork, zAmsterdam);
          if (z2zE2106) {  zSLOTNUM(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2236: ;
        {
          uint64_t z3zE2046;
          z3zE2046 = zopcode;
          bool z3zE1934;
          z3zE1934 = (z3zE2046 == UINT64_C(80));
          if (!(z3zE1934)) {  goto case_2235;  }
          zPOP(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2235: ;
        {
          uint64_t z3zE2047;
          z3zE2047 = zopcode;
          bool z3zE1933;
          z3zE1933 = (z3zE2047 == UINT64_C(81));
          if (!(z3zE1933)) {  goto case_2234;  }
          zMLOAD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2234: ;
        {
          uint64_t z3zE2048;
          z3zE2048 = zopcode;
          bool z3zE1932;
          z3zE1932 = (z3zE2048 == UINT64_C(82));
          if (!(z3zE1932)) {  goto case_2233;  }
          zMSTORE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2233: ;
        {
          uint64_t z3zE2049;
          z3zE2049 = zopcode;
          bool z3zE1931;
          z3zE1931 = (z3zE2049 == UINT64_C(83));
          if (!(z3zE1931)) {  goto case_2232;  }
          zMSTORE8(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2232: ;
        {
          uint64_t z3zE2050;
          z3zE2050 = zopcode;
          bool z3zE1930;
          z3zE1930 = (z3zE2050 == UINT64_C(84));
          if (!(z3zE1930)) {  goto case_2231;  }
          zSLOAD(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2231: ;
        {
          uint64_t z3zE2051;
          z3zE2051 = zopcode;
          bool z3zE1929;
          z3zE1929 = (z3zE2051 == UINT64_C(85));
          if (!(z3zE1929)) {  goto case_2230;  }
          zSSTORE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2230: ;
        {
          uint64_t z3zE2052;
          z3zE2052 = zopcode;
          bool z3zE1928;
          z3zE1928 = (z3zE2052 == UINT64_C(86));
          if (!(z3zE1928)) {  goto case_2229;  }
          zJUMP(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2229: ;
        {
          uint64_t z3zE2053;
          z3zE2053 = zopcode;
          bool z3zE1927;
          z3zE1927 = (z3zE2053 == UINT64_C(87));
          if (!(z3zE1927)) {  goto case_2228;  }
          zJUMPI(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2228: ;
        {
          uint64_t z3zE2054;
          z3zE2054 = zopcode;
          bool z3zE1926;
          z3zE1926 = (z3zE2054 == UINT64_C(88));
          if (!(z3zE1926)) {  goto case_2227;  }
          zPC(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2227: ;
        {
          uint64_t z3zE2055;
          z3zE2055 = zopcode;
          bool z3zE1925;
          z3zE1925 = (z3zE2055 == UINT64_C(89));
          if (!(z3zE1925)) {  goto case_2226;  }
          zMSIZE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2226: ;
        {
          uint64_t z3zE2056;
          z3zE2056 = zopcode;
          bool z3zE1924;
          z3zE1924 = (z3zE2056 == UINT64_C(90));
          if (!(z3zE1924)) {  goto case_2225;  }
          zGAS(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2225: ;
        {
          uint64_t z3zE2057;
          z3zE2057 = zopcode;
          bool z3zE1923;
          z3zE1923 = (z3zE2057 == UINT64_C(91));
          if (!(z3zE1923)) {  goto case_2224;  }
          zJUMPDEST(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2224: ;
        {
          uint64_t z3zE2058;
          z3zE2058 = zopcode;
          bool z3zE1922;
          z3zE1922 = (z3zE2058 == UINT64_C(92));
          if (!(z3zE1922)) {  goto case_2223;  }
          bool z2zE2107;
          z2zE2107 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2107) {  zTLOAD(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2223: ;
        {
          uint64_t z3zE2059;
          z3zE2059 = zopcode;
          bool z3zE1921;
          z3zE1921 = (z3zE2059 == UINT64_C(93));
          if (!(z3zE1921)) {  goto case_2222;  }
          bool z2zE2108;
          z2zE2108 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2108) {  zTSTORE(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2222: ;
        {
          uint64_t z3zE2060;
          z3zE2060 = zopcode;
          bool z3zE1920;
          z3zE1920 = (z3zE2060 == UINT64_C(94));
          if (!(z3zE1920)) {  goto case_2221;  }
          bool z2zE2109;
          z2zE2109 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2109) {  zMCOPY(&z3zE1909, UNIT);  } else {  zINVALID(&z3zE1909, UNIT);  }
          goto finish_match_2210;
        }
      case_2221: ;
        {
          uint64_t z3zE2061;
          z3zE2061 = zopcode;
          bool z3zE1919;
          z3zE1919 = (z3zE2061 == UINT64_C(240));
          if (!(z3zE1919)) {  goto case_2220;  }
          zCREATE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2220: ;
        {
          uint64_t z3zE2062;
          z3zE2062 = zopcode;
          bool z3zE1918;
          z3zE1918 = (z3zE2062 == UINT64_C(241));
          if (!(z3zE1918)) {  goto case_2219;  }
          zCALL(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2219: ;
        {
          uint64_t z3zE2063;
          z3zE2063 = zopcode;
          bool z3zE1917;
          z3zE1917 = (z3zE2063 == UINT64_C(242));
          if (!(z3zE1917)) {  goto case_2218;  }
          zCALLCODE(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2218: ;
        {
          uint64_t z3zE2064;
          z3zE2064 = zopcode;
          bool z3zE1916;
          z3zE1916 = (z3zE2064 == UINT64_C(243));
          if (!(z3zE1916)) {  goto case_2217;  }
          zRETURN(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2217: ;
        {
          uint64_t z3zE2065;
          z3zE2065 = zopcode;
          bool z3zE1915;
          z3zE1915 = (z3zE2065 == UINT64_C(244));
          if (!(z3zE1915)) {  goto case_2216;  }
          zDELEGATECALL(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2216: ;
        {
          uint64_t z3zE2066;
          z3zE2066 = zopcode;
          bool z3zE1914;
          z3zE1914 = (z3zE2066 == UINT64_C(245));
          if (!(z3zE1914)) {  goto case_2215;  }
          zCREATE2(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2215: ;
        {
          uint64_t z3zE2067;
          z3zE2067 = zopcode;
          bool z3zE1913;
          z3zE1913 = (z3zE2067 == UINT64_C(250));
          if (!(z3zE1913)) {  goto case_2214;  }
          zSTATICCALL(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2214: ;
        {
          uint64_t z3zE2068;
          z3zE2068 = zopcode;
          bool z3zE1912;
          z3zE1912 = (z3zE2068 == UINT64_C(253));
          if (!(z3zE1912)) {  goto case_2213;  }
          zREVERT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2213: ;
        {
          uint64_t z3zE2069;
          z3zE2069 = zopcode;
          bool z3zE1911;
          z3zE1911 = (z3zE2069 == UINT64_C(255));
          if (!(z3zE1911)) {  goto case_2212;  }
          zSELFDESTRUCT(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2212: ;
        {
          zINVALID(&z3zE1909, UNIT);
          goto finish_match_2210;
        }
      case_2211: ;
      finish_match_2210: ;
        COPY(zast)((*(&z8zE441)), z3zE1909);
        KILL(zast)(&z3zE1909);
      }
    }
  }
end_function_2292: ;
  goto end_function_4066;
end_block_exception_2293: ;
  goto end_function_4066;
end_function_4066: ;
}

void zfetch(struct zast *z8zE442, unit z3zE1877)
{
  uint64_t zcurrent;
  zcurrent = zpc;
  struct zByteSliceFields zcode;
  zcode = zframe_code.zbytes;
  uint64_t zcode_length;
  zcode_length = zcode.zlen;
  bool z2zE2080;
  {
    bool z2zE2079;
    z2zE2079 = (zcurrent < zcode_length);
    z2zE2080 = not(z2zE2079);
  }
  if (z2zE2080) {  zSTOP((*(&z8zE442)), UNIT);  } else {
    uint64_t zopcode;
    {
      uint64_t z2zE2092;
      z2zE2092 = zslice_byte(zcode, zcurrent);
      zopcode = ((uint64_t) z2zE2092);
    }
    uint64_t zimmediate_offset;
    {    zimmediate_offset = (zcurrent + UINT64_C(1));
    }
    struct ztuple_z8z5u64zCz0z5unionz0zzastz9 zdecoded;
    CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded);
    {
      bool z2zE2082;
      {
        bool z2zE2081;
        z2zE2081 = (zopcode == UINT64_C(95));
        bool z3zE1878;
        if (z2zE2081) {  z3zE1878 = zfork_lt(zk_fork, zShanghai);  } else {  z3zE1878 = false;  }
        z2zE2082 = z3zE1878;
      }
      if (z2zE2082) {
        struct zast z2zE2083;
        CREATE(zast)(&z2zE2083);
        zINVALID(&z2zE2083, UNIT);
        struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1896;
        CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1896);
        z3zE1896.ztup0 = zimmediate_offset;
        COPY(zast)(&((&z3zE1896)->ztup1), z2zE2083);
        COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1896);
        KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1896);
        KILL(zast)(&z2zE2083);
      } else {
        bool z2zE2085;
        {
          bool z2zE2084;
          z2zE2084 = (!(zopcode < UINT64_C(95)));
          bool z3zE1879;
          if (z2zE2084) {  z3zE1879 = (!(UINT64_C(127) < zopcode));  } else {  z3zE1879 = false;  }
          z2zE2085 = z3zE1879;
        }
        if (z2zE2085) {
          uint64_t zsizze;
          {    zsizze = (zopcode - UINT64_C(95));
          }
          sail_u256 zvalue;
          zvalue = zread_push(zcode, zimmediate_offset, zsizze);
          uint64_t zafter_immediate;
          {
            uint64_t z2zE2087;
            {    z2zE2087 = (zcurrent + UINT64_C(1));
            }
            {    zafter_immediate = (z2zE2087 + zsizze);
            }
          }
          struct zast z2zE2086;
          CREATE(zast)(&z2zE2086);
          {
            struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 z3zE1894;
            z3zE1894.ztup0 = zsizze;
            z3zE1894.ztup1 = zvalue;
            zPUSH(&z2zE2086, z3zE1894);
          }
          struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1895;
          CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1895);
          z3zE1895.ztup0 = zafter_immediate;
          COPY(zast)(&((&z3zE1895)->ztup1), z2zE2086);
          COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1895);
          KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1895);
          KILL(zast)(&z2zE2086);
        } else {
          bool z2zE2090;
          {
            bool z2zE2089;
            z2zE2089 = zfork_gteq(zk_fork, zAmsterdam);
            bool z3zE1881;
            if (z2zE2089) {
              bool z2zE2088;
              z2zE2088 = (!(zopcode < UINT64_C(230)));
              bool z3zE1880;
              if (z2zE2088) {  z3zE1880 = (!(UINT64_C(232) < zopcode));  } else {  z3zE1880 = false;  }
              z3zE1881 = z3zE1880;
            } else {  z3zE1881 = false;  }
            z2zE2090 = z3zE1881;
          }
          if (z2zE2090) {
            uint64_t zimmediate;
            zimmediate = zslice_byte(zcode, zimmediate_offset);
            bool zimmediate_valid;
            {
              bool z3zE1883;
              {
                uint64_t zp0z3;
                zp0z3 = zopcode;
                bool z3zE1887;
                z3zE1887 = (zp0z3 == UINT64_C(230));
                if (!(z3zE1887)) {  goto case_2200;  }
                z3zE1883 = zdeep_stack_immediate_valid(zimmediate);
                goto finish_match_2196;
              }
            case_2200: ;
              {
                uint64_t z3zE1900;
                z3zE1900 = zopcode;
                bool z3zE1886;
                z3zE1886 = (z3zE1900 == UINT64_C(231));
                if (!(z3zE1886)) {  goto case_2199;  }
                z3zE1883 = zdeep_stack_immediate_valid(zimmediate);
                goto finish_match_2196;
              }
            case_2199: ;
              {
                uint64_t z3zE1901;
                z3zE1901 = zopcode;
                bool z3zE1885;
                z3zE1885 = (z3zE1901 == UINT64_C(232));
                if (!(z3zE1885)) {  goto case_2198;  }
                z3zE1883 = zexchange_immediate_valid(zimmediate);
                goto finish_match_2196;
              }
            case_2198: ;
              {
                z3zE1883 = false;
                goto finish_match_2196;
              }
            case_2197: ;
            finish_match_2196: ;
              zimmediate_valid = z3zE1883;
            }
            uint64_t zafter_instruction;
            if (zimmediate_valid) {
              {    zafter_instruction = (zcurrent + UINT64_C(2));
              }
            } else {
              {    zafter_instruction = (zcurrent + UINT64_C(1));
              }
            }
            struct zast zinstruction;
            CREATE(zast)(&zinstruction);
            {
              {
                uint64_t z3zE1902;
                z3zE1902 = zopcode;
                bool z3zE1892;
                z3zE1892 = (z3zE1902 == UINT64_C(230));
                if (!(z3zE1892)) {  goto case_2205;  }
                zDUPN(&zinstruction, zimmediate);
                goto finish_match_2201;
              }
            case_2205: ;
              {
                uint64_t z3zE1903;
                z3zE1903 = zopcode;
                bool z3zE1891;
                z3zE1891 = (z3zE1903 == UINT64_C(231));
                if (!(z3zE1891)) {  goto case_2204;  }
                zSWAPN(&zinstruction, zimmediate);
                goto finish_match_2201;
              }
            case_2204: ;
              {
                uint64_t z3zE1904;
                z3zE1904 = zopcode;
                bool z3zE1890;
                z3zE1890 = (z3zE1904 == UINT64_C(232));
                if (!(z3zE1890)) {  goto case_2203;  }
                zEXCHANGE(&zinstruction, zimmediate);
                goto finish_match_2201;
              }
            case_2203: ;
              {
                zINVALID(&zinstruction, UNIT);
                goto finish_match_2201;
              }
            case_2202: ;
            finish_match_2201: ;
            }
            struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1893;
            CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1893);
            z3zE1893.ztup0 = zafter_instruction;
            COPY(zast)(&((&z3zE1893)->ztup1), zinstruction);
            COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1893);
            KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1893);
            KILL(zast)(&zinstruction);
          } else {
            struct zast z2zE2091;
            CREATE(zast)(&z2zE2091);
            zdecode_simple(&z2zE2091, zopcode);
            struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1882;
            CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1882);
            z3zE1882.ztup0 = zimmediate_offset;
            COPY(zast)(&((&z3zE1882)->ztup1), z2zE2091);
            COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1882);
            KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1882);
            KILL(zast)(&z2zE2091);
          }
        }
      }
    }
    struct zast z3zE1897;
    CREATE(zast)(&z3zE1897);
    {
      uint64_t znext_pc;
      znext_pc = zdecoded.ztup0;
      COPY(zast)(&z3zE1897, zdecoded.ztup1);
      zpc = znext_pc;
      unit z3zE1898;
      z3zE1898 = UNIT;
      goto finish_match_2206;
    }
  case_2207: ;
    sail_match_failure("fetch");
  finish_match_2206: ;
    COPY(zast)((*(&z8zE442)), z3zE1897);
    KILL(zast)(&z3zE1897);
    KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded);
  }
end_function_2208: ;
  goto end_function_4065;
end_block_exception_2209: ;
  goto end_function_4065;
end_function_4065: ;
}

struct zByteSliceFields zframe_output(unit z3zE1871)
{
  struct zByteSliceFields z8zE443;
  struct zByteSliceFields z3zE1872;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2193;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltReturn) goto case_2193;
    struct zByteSliceFields zoutput;
    zoutput = zframe_status.variants.zHalted.variants.zHaltReturn;
    z3zE1872 = zoutput;
    goto finish_match_2190;
  }
case_2193: ;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2192;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltRevert) goto case_2192;
    struct zByteSliceFields z3zE1876;
    z3zE1876 = zframe_status.variants.zHalted.variants.zHaltRevert;
    z3zE1872 = z3zE1876;
    goto finish_match_2190;
  }
case_2192: ;
  {
    z3zE1872 = zEMPTY_SLICE;
    goto finish_match_2190;
  }
case_2191: ;
finish_match_2190: ;
  z8zE443 = z3zE1872;
end_function_2194: ;
  return z8zE443;
end_block_exception_2195: ;
  struct zByteSliceFields z8zE983 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE983;
}

bool zframe_succeeded(unit z3zE1865)
{
  bool z8zE444;
  bool z3zE1866;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2187;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltRevert) goto case_2187;
    z3zE1866 = false;
    goto finish_match_2183;
  }
case_2187: ;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2186;
    z3zE1866 = true;
    goto finish_match_2183;
  }
case_2186: ;
  {
    if (zframe_status.kind != Kind_zRunning) goto case_2185;
    z3zE1866 = true;
    goto finish_match_2183;
  }
case_2185: ;
  {
    /* complete */
    z3zE1866 = false;
    goto finish_match_2183;
  }
case_2184: ;
finish_match_2183: ;
  z8zE444 = z3zE1866;
end_function_2188: ;
  return z8zE444;
end_block_exception_2189: ;

  return false;
}

unit zresume_call(struct zCallContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE445;
  zreturndata = zoutput;
  unit z3zE1857;
  z3zE1857 = UNIT;
  struct zFrameCheckpoint zcheckpoint;
  CREATE(zFrameCheckpoint)(&zcheckpoint);
  COPY(zFrameCheckpoint)(&zcheckpoint, zcontinuation.zcheckpoint);
  bool zsucceeded;
  zsucceeded = zframe_succeeded(UNIT);
  uint64_t zchild_left;
  zchild_left = zgas_remaining;
  uint64_t zchild_state_left;
  zchild_state_left = zstate_gas_remaining;
  uint64_t zchild_state_spill;
  zchild_state_spill = zstate_gas_spilled;
  __int128 zchild_refund;
  zchild_refund = zframe_refund;
  unit z3zE1861;
  z3zE1861 = zrestore_frame(zcheckpoint);
  unit z3zE1860;
  {
    z3zE1860 = zrefund_gas(zchild_left);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2182;
    }
  }
  unit z3zE1859;
  {
    z3zE1859 = zreturn_child_state_gas(zchild_state_left, zchild_state_spill);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2182;
    }
  }
  uint64_t z2zE2075;
  z2zE2075 = zcontinuation.zreturn_offset;
  uint64_t z2zE2076;
  z2zE2076 = zcontinuation.zreturn_length;
  unit z3zE1858;
  z3zE1858 = zreturndata_copy_prefix(z2zE2075, z2zE2076);
  if (zsucceeded) {
    unit z3zE1864;
    {
      z3zE1864 = zrecord_refund(zchild_refund);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2182;
      }
    }
    {
      z8zE445 = zpush_word(zWORD_ONE);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2182;
      }
    }
  } else {
    uint64_t z2zE2077;
    z2zE2077 = zcheckpoint.zstate;
    unit z3zE1863;
    z3zE1863 = zk_revert(z2zE2077);
    bool z2zE2078;
    z2zE2078 = zcontinuation.znew_account_charged;
    unit z3zE1862;
    if (z2zE2078) {
      {
        z3zE1862 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_2182;
        }
      }
    } else {  z3zE1862 = UNIT;  }
    {
      z8zE445 = zpush_word(zWORD_ZERO);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2182;
      }
    }
  }
  KILL(zFrameCheckpoint)(&zcheckpoint);
end_function_2181: ;
  return z8zE445;
end_block_exception_2182: ;

  return UNIT;
}

unit zresume_create(struct zCreateContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE446;
  zreturndata = zoutput;
  unit z3zE1838;
  z3zE1838 = UNIT;
  struct zFrameCheckpoint zcheckpoint;
  CREATE(zFrameCheckpoint)(&zcheckpoint);
  COPY(zFrameCheckpoint)(&zcheckpoint, zcontinuation.zcheckpoint);
  bool zinitcode_succeeded;
  zinitcode_succeeded = zframe_succeeded(UNIT);
  uint64_t zdeployed_length;
  zdeployed_length = zreturndata_sizze(UNIT);
  uint64_t zdeployed_sizze;
  zdeployed_sizze = zdeployed_length;
  bool zfrontier_empty_deposit;
  zfrontier_empty_deposit = false;
  unit z3zE1839;
  if (zinitcode_succeeded) {
    bool z2zE2066;
    {
      bool z2zE2065;
      {
        bool z2zE2061;
        z2zE2061 = zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdeployed_sizze);
        z2zE2065 = not(z2zE2061);
      }
      bool z3zE1842;
      if (z2zE2065) {  z3zE1842 = true;  } else {
        bool z2zE2064;
        z2zE2064 = zfork_gteq(zk_fork, zLondon);
        bool z3zE1841;
        if (z2zE2064) {
          bool z2zE2063;
          z2zE2063 = (zdeployed_sizze != UINT64_C(0));
          bool z3zE1840;
          if (z2zE2063) {
            uint64_t z2zE2062;
            z2zE2062 = zslice_byte(zreturndata, UINT64_C(0));
            z3zE1840 = (z2zE2062 == UINT64_C(0xEF));
          } else {  z3zE1840 = false;  }
          z3zE1841 = z3zE1840;
        } else {  z3zE1841 = false;  }
        z3zE1842 = z3zE1841;
      }
      z2zE2066 = z3zE1842;
    }
    if (z2zE2066) {
      {
        z3zE1839 = zexc_halt(zOutOfGas);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_2180;
        }
      }
    } else {
      struct zoptionzIU64zK z2zE2067;
      CREATE(zoptionzIU64zK)(&z2zE2067);
      zcode_deployment_execution_cost(&z2zE2067, zdeployed_length, zgas_remaining);
      unit z3zE1843;
      {
        if (z2zE2067.kind != Kind_zSomezIU64zK) goto case_2178;
        uint64_t zexecution_deposit;
        zexecution_deposit = z2zE2067.variants.zSomezIU64zK;
        {
          zgas_remaining = zgas_sub_or_oog(zgas_remaining, zexecution_deposit);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE2067);
            goto end_block_exception_2180;
          }
        }
        unit z3zE1846;
        z3zE1846 = UNIT;
        uint64_t z2zE2068;
        {
          z2zE2068 = zcode_deployment_state_cost(zdeployed_length);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE2067);
            goto end_block_exception_2180;
          }
        }
        {
          z3zE1843 = zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2068);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE2067);
            goto end_block_exception_2180;
          }
        }
        goto finish_match_2176;
      }
    case_2178: ;
      {
        /* complete */
        bool z2zE2069;
        z2zE2069 = zfork_lt(zk_fork, zHomestead);
        if (z2zE2069) {
          zgas_remaining = zGAS_ZERO;
          unit z3zE1845;
          z3zE1845 = UNIT;
          zfrontier_empty_deposit = true;
          z3zE1843 = UNIT;
        } else {
          {
            z3zE1843 = zexc_halt(zOutOfGas);
            if (have_exception) {
              KILL(zFrameCheckpoint)(&zcheckpoint);
              KILL(zoptionzIU64zK)(&z2zE2067);
              goto end_block_exception_2180;
            }
          }
        }
        goto finish_match_2176;
      }
    case_2177: ;
    finish_match_2176: ;
      z3zE1839 = z3zE1843;
      KILL(zoptionzIU64zK)(&z2zE2067);
    }
  } else {  z3zE1839 = UNIT;  }
  bool zdeploy_succeeds;
  {
    bool z3zE1848;
    if (zinitcode_succeeded) {  z3zE1848 = zframe_succeeded(UNIT);  } else {  z3zE1848 = false;  }
    zdeploy_succeeds = z3zE1848;
  }
  uint64_t zchild_left;
  zchild_left = zgas_remaining;
  uint64_t zchild_state_left;
  zchild_state_left = zstate_gas_remaining;
  uint64_t zchild_state_spill;
  zchild_state_spill = zstate_gas_spilled;
  __int128 zchild_refund;
  zchild_refund = zframe_refund;
  unit z3zE1856;
  z3zE1856 = zrestore_frame(zcheckpoint);
  unit z3zE1855;
  {
    z3zE1855 = zrefund_gas(zchild_left);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2180;
    }
  }
  unit z3zE1854;
  {
    z3zE1854 = zreturn_child_state_gas(zchild_state_left, zchild_state_spill);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2180;
    }
  }
  unit z3zE1849;
  if (zdeploy_succeeds) {
    unit z3zE1852;
    {
      z3zE1852 = zrecord_refund(zchild_refund);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2180;
      }
    }
    struct zByteSliceFields zdeployed_bytes;
    if (zfrontier_empty_deposit) {  zdeployed_bytes = zEMPTY_SLICE;  } else {  zdeployed_bytes = zreturndata;  }
    struct zByteSliceFields zdeployed_code;
    zdeployed_code = zvalidated_code_slice(zdeployed_bytes);
    sail_fixed_bytes_20 z2zE2070;
    z2zE2070 = zcontinuation.zaddress;
    unit z3zE1853;
    {
      z3zE1853 = zk_deploy_code(z2zE2070, zdeployed_code);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2180;
      }
    }
    sail_u256 z2zE2072;
    {
      sail_fixed_bytes_20 z2zE2071;
      z2zE2071 = zcontinuation.zaddress;
      z2zE2072 = zaddress_to_word(z2zE2071);
    }
    {
      z3zE1849 = zpush_word(z2zE2072);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2180;
      }
    }
  } else {
    uint64_t z2zE2073;
    z2zE2073 = zcheckpoint.zstate;
    unit z3zE1851;
    z3zE1851 = zk_revert(z2zE2073);
    bool z2zE2074;
    z2zE2074 = zcontinuation.znew_account_charged;
    unit z3zE1850;
    if (z2zE2074) {
      {
        z3zE1850 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_2180;
        }
      }
    } else {  z3zE1850 = UNIT;  }
    {
      z3zE1849 = zpush_word(zWORD_ZERO);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2180;
      }
    }
  }
  if (zinitcode_succeeded) {  z8zE446 = zreturndata_clear(UNIT);  } else {  z8zE446 = UNIT;  }
  KILL(zFrameCheckpoint)(&zcheckpoint);
end_function_2179: ;
  return z8zE446;
end_block_exception_2180: ;

  return UNIT;
}

unit zresume_frame(struct zFrameContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE447;
  unit z3zE1835;
  {
    if (zcontinuation.kind != Kind_zResumeCall) goto case_2173;
    struct zCallContinuation zcall;
    CREATE(zCallContinuation)(&zcall);
    COPY(zCallContinuation)(&zcall, zcontinuation.variants.zResumeCall);
    {
      z3zE1835 = zresume_call(zcall, zoutput);
      if (have_exception) {
        KILL(zCallContinuation)(&zcall);
        goto end_block_exception_2175;
      }
    }
    KILL(zCallContinuation)(&zcall);
    goto finish_match_2171;
  }
case_2173: ;
  {
    /* complete */
    struct zCreateContinuation zcreate;
    CREATE(zCreateContinuation)(&zcreate);
    COPY(zCreateContinuation)(&zcreate, zcontinuation.variants.zResumeCreate);
    {
      z3zE1835 = zresume_create(zcreate, zoutput);
      if (have_exception) {
        KILL(zCreateContinuation)(&zcreate);
        goto end_block_exception_2175;
      }
    }
    KILL(zCreateContinuation)(&zcreate);
    goto finish_match_2171;
  }
case_2172: ;
finish_match_2171: ;
  z8zE447 = z3zE1835;
end_function_2174: ;
  return z8zE447;
end_block_exception_2175: ;

  return UNIT;
}

struct zByteSliceFields zinterpret(unit z3zE1829)
{
  struct zByteSliceFields z8zE448;
  unit z3zE1830;
  z3zE1830 = zframe_stack_reset(UNIT);
  bool zinterpreting;
  zinterpreting = true;
  struct zByteSliceFields zresult;
  zresult = zEMPTY_SLICE;
  bool z3zE1831;
  unit z3zE1832;
while_2167: ;
  {
    z3zE1831 = zinterpreting;
    if (!(z3zE1831)) goto wend_2168;
    bool z2zE2057;
    z2zE2057 = zis_running(UNIT);
    if (z2zE2057) {
      struct zast z2zE2058;
      CREATE(zast)(&z2zE2058);
      zfetch(&z2zE2058, UNIT);
      {
        z3zE1832 = zexecute(z2zE2058);
        if (have_exception) {
          KILL(zast)(&z2zE2058);
          goto end_block_exception_2170;
        }
      }
      KILL(zast)(&z2zE2058);
    } else {
      struct zByteSliceFields zoutput;
      zoutput = zframe_output(UNIT);
      bool z2zE2059;
      z2zE2059 = zframe_stack_is_empty(UNIT);
      if (z2zE2059) {
        zresult = zoutput;
        unit z3zE1833;
        z3zE1833 = UNIT;
        zinterpreting = false;
        z3zE1832 = UNIT;
      } else {
        struct zFrameContinuation z2zE2060;
        CREATE(zFrameContinuation)(&z2zE2060);
        zframe_stack_pop(&z2zE2060, UNIT);
        {
          z3zE1832 = zresume_frame(z2zE2060, zoutput);
          if (have_exception) {
            KILL(zFrameContinuation)(&z2zE2060);
            goto end_block_exception_2170;
          }
        }
        KILL(zFrameContinuation)(&z2zE2060);
      }
    }
    goto while_2167;
  }
wend_2168: ;
  unit z3zE1834;
  z3zE1834 = UNIT;
  z8zE448 = zresult;
end_function_2169: ;
  return z8zE448;
end_block_exception_2170: ;
  struct zByteSliceFields z8zE984 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE984;
}

