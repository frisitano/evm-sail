/* Generated from sail/evm/interpreter.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_u256 zread_push(struct zByteSliceFields zcode, uint64_t zoffset, uint64_t zn)
{
  sail_u256 z8zE437;
  z8zE437 = zslice_load_n(zcode, zoffset, zn);
end_function_2290: ;
  return z8zE437;
end_block_exception_2291: ;

  return u256_zero();
}

void zdecode_simple(struct zast *z8zE438, uint64_t zopcode)
{
  bool z2zE2071;
  {
    bool z2zE2070;
    z2zE2070 = (!(zopcode < UINT64_C(128)));
    bool z3zE1910;
    if (z2zE2070) {  z3zE1910 = (!(UINT64_C(143) < zopcode));  } else {  z3zE1910 = false;  }
    z2zE2071 = z3zE1910;
  }
  if (z2zE2071) {
    uint64_t z2zE2072;
    {    z2zE2072 = (zopcode - UINT64_C(127));
    }
    zDUP((*(&z8zE438)), z2zE2072);
  } else {
    bool z2zE2074;
    {
      bool z2zE2073;
      z2zE2073 = (!(zopcode < UINT64_C(144)));
      bool z3zE1911;
      if (z2zE2073) {  z3zE1911 = (!(UINT64_C(159) < zopcode));  } else {  z3zE1911 = false;  }
      z2zE2074 = z3zE1911;
    }
    if (z2zE2074) {
      uint64_t z2zE2075;
      {    z2zE2075 = (zopcode - UINT64_C(143));
      }
      zSWAP((*(&z8zE438)), z2zE2075);
    } else {
      bool z2zE2077;
      {
        bool z2zE2076;
        z2zE2076 = (!(zopcode < UINT64_C(160)));
        bool z3zE1912;
        if (z2zE2076) {  z3zE1912 = (!(UINT64_C(164) < zopcode));  } else {  z3zE1912 = false;  }
        z2zE2077 = z3zE1912;
      }
      if (z2zE2077) {
        uint64_t z2zE2078;
        {    z2zE2078 = (zopcode - UINT64_C(160));
        }
        zLOG((*(&z8zE438)), z2zE2078);
      } else {
        struct zast z3zE1913;
        CREATE(zast)(&z3zE1913);
        {
          uint64_t zp0z3;
          zp0z3 = zopcode;
          bool z3zE1994;
          z3zE1994 = (zp0z3 == UINT64_C(0));
          if (!(z3zE1994)) {  goto case_2287;  }
          zSTOP(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2287: ;
        {
          uint64_t z3zE1995;
          z3zE1995 = zopcode;
          bool z3zE1993;
          z3zE1993 = (z3zE1995 == UINT64_C(1));
          if (!(z3zE1993)) {  goto case_2286;  }
          zADD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2286: ;
        {
          uint64_t z3zE1996;
          z3zE1996 = zopcode;
          bool z3zE1992;
          z3zE1992 = (z3zE1996 == UINT64_C(2));
          if (!(z3zE1992)) {  goto case_2285;  }
          zMUL(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2285: ;
        {
          uint64_t z3zE1997;
          z3zE1997 = zopcode;
          bool z3zE1991;
          z3zE1991 = (z3zE1997 == UINT64_C(3));
          if (!(z3zE1991)) {  goto case_2284;  }
          zSUB(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2284: ;
        {
          uint64_t z3zE1998;
          z3zE1998 = zopcode;
          bool z3zE1990;
          z3zE1990 = (z3zE1998 == UINT64_C(4));
          if (!(z3zE1990)) {  goto case_2283;  }
          zDIV(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2283: ;
        {
          uint64_t z3zE1999;
          z3zE1999 = zopcode;
          bool z3zE1989;
          z3zE1989 = (z3zE1999 == UINT64_C(5));
          if (!(z3zE1989)) {  goto case_2282;  }
          zSDIV(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2282: ;
        {
          uint64_t z3zE2000;
          z3zE2000 = zopcode;
          bool z3zE1988;
          z3zE1988 = (z3zE2000 == UINT64_C(6));
          if (!(z3zE1988)) {  goto case_2281;  }
          zMOD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2281: ;
        {
          uint64_t z3zE2001;
          z3zE2001 = zopcode;
          bool z3zE1987;
          z3zE1987 = (z3zE2001 == UINT64_C(7));
          if (!(z3zE1987)) {  goto case_2280;  }
          zSMOD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2280: ;
        {
          uint64_t z3zE2002;
          z3zE2002 = zopcode;
          bool z3zE1986;
          z3zE1986 = (z3zE2002 == UINT64_C(8));
          if (!(z3zE1986)) {  goto case_2279;  }
          zADDMOD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2279: ;
        {
          uint64_t z3zE2003;
          z3zE2003 = zopcode;
          bool z3zE1985;
          z3zE1985 = (z3zE2003 == UINT64_C(9));
          if (!(z3zE1985)) {  goto case_2278;  }
          zMULMOD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2278: ;
        {
          uint64_t z3zE2004;
          z3zE2004 = zopcode;
          bool z3zE1984;
          z3zE1984 = (z3zE2004 == UINT64_C(10));
          if (!(z3zE1984)) {  goto case_2277;  }
          zEXP(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2277: ;
        {
          uint64_t z3zE2005;
          z3zE2005 = zopcode;
          bool z3zE1983;
          z3zE1983 = (z3zE2005 == UINT64_C(11));
          if (!(z3zE1983)) {  goto case_2276;  }
          zSIGNEXTEND(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2276: ;
        {
          uint64_t z3zE2006;
          z3zE2006 = zopcode;
          bool z3zE1982;
          z3zE1982 = (z3zE2006 == UINT64_C(16));
          if (!(z3zE1982)) {  goto case_2275;  }
          zLT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2275: ;
        {
          uint64_t z3zE2007;
          z3zE2007 = zopcode;
          bool z3zE1981;
          z3zE1981 = (z3zE2007 == UINT64_C(17));
          if (!(z3zE1981)) {  goto case_2274;  }
          zGT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2274: ;
        {
          uint64_t z3zE2008;
          z3zE2008 = zopcode;
          bool z3zE1980;
          z3zE1980 = (z3zE2008 == UINT64_C(18));
          if (!(z3zE1980)) {  goto case_2273;  }
          zSLT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2273: ;
        {
          uint64_t z3zE2009;
          z3zE2009 = zopcode;
          bool z3zE1979;
          z3zE1979 = (z3zE2009 == UINT64_C(19));
          if (!(z3zE1979)) {  goto case_2272;  }
          zSGT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2272: ;
        {
          uint64_t z3zE2010;
          z3zE2010 = zopcode;
          bool z3zE1978;
          z3zE1978 = (z3zE2010 == UINT64_C(20));
          if (!(z3zE1978)) {  goto case_2271;  }
          zEQ(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2271: ;
        {
          uint64_t z3zE2011;
          z3zE2011 = zopcode;
          bool z3zE1977;
          z3zE1977 = (z3zE2011 == UINT64_C(21));
          if (!(z3zE1977)) {  goto case_2270;  }
          zISZERO(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2270: ;
        {
          uint64_t z3zE2012;
          z3zE2012 = zopcode;
          bool z3zE1976;
          z3zE1976 = (z3zE2012 == UINT64_C(22));
          if (!(z3zE1976)) {  goto case_2269;  }
          zAND(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2269: ;
        {
          uint64_t z3zE2013;
          z3zE2013 = zopcode;
          bool z3zE1975;
          z3zE1975 = (z3zE2013 == UINT64_C(23));
          if (!(z3zE1975)) {  goto case_2268;  }
          zOR(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2268: ;
        {
          uint64_t z3zE2014;
          z3zE2014 = zopcode;
          bool z3zE1974;
          z3zE1974 = (z3zE2014 == UINT64_C(24));
          if (!(z3zE1974)) {  goto case_2267;  }
          zXOR(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2267: ;
        {
          uint64_t z3zE2015;
          z3zE2015 = zopcode;
          bool z3zE1973;
          z3zE1973 = (z3zE2015 == UINT64_C(25));
          if (!(z3zE1973)) {  goto case_2266;  }
          zNOT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2266: ;
        {
          uint64_t z3zE2016;
          z3zE2016 = zopcode;
          bool z3zE1972;
          z3zE1972 = (z3zE2016 == UINT64_C(26));
          if (!(z3zE1972)) {  goto case_2265;  }
          zBYTE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2265: ;
        {
          uint64_t z3zE2017;
          z3zE2017 = zopcode;
          bool z3zE1971;
          z3zE1971 = (z3zE2017 == UINT64_C(27));
          if (!(z3zE1971)) {  goto case_2264;  }
          zSHL(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2264: ;
        {
          uint64_t z3zE2018;
          z3zE2018 = zopcode;
          bool z3zE1970;
          z3zE1970 = (z3zE2018 == UINT64_C(28));
          if (!(z3zE1970)) {  goto case_2263;  }
          zSHR(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2263: ;
        {
          uint64_t z3zE2019;
          z3zE2019 = zopcode;
          bool z3zE1969;
          z3zE1969 = (z3zE2019 == UINT64_C(29));
          if (!(z3zE1969)) {  goto case_2262;  }
          zSAR(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2262: ;
        {
          uint64_t z3zE2020;
          z3zE2020 = zopcode;
          bool z3zE1968;
          z3zE1968 = (z3zE2020 == UINT64_C(30));
          if (!(z3zE1968)) {  goto case_2261;  }
          bool z2zE2079;
          z2zE2079 = zfork_gteq(zk_fork, zOsaka);
          if (z2zE2079) {  zCLZ(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2261: ;
        {
          uint64_t z3zE2021;
          z3zE2021 = zopcode;
          bool z3zE1967;
          z3zE1967 = (z3zE2021 == UINT64_C(32));
          if (!(z3zE1967)) {  goto case_2260;  }
          zKECCAK256(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2260: ;
        {
          uint64_t z3zE2022;
          z3zE2022 = zopcode;
          bool z3zE1966;
          z3zE1966 = (z3zE2022 == UINT64_C(48));
          if (!(z3zE1966)) {  goto case_2259;  }
          zADDRESS(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2259: ;
        {
          uint64_t z3zE2023;
          z3zE2023 = zopcode;
          bool z3zE1965;
          z3zE1965 = (z3zE2023 == UINT64_C(49));
          if (!(z3zE1965)) {  goto case_2258;  }
          zBALANCE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2258: ;
        {
          uint64_t z3zE2024;
          z3zE2024 = zopcode;
          bool z3zE1964;
          z3zE1964 = (z3zE2024 == UINT64_C(50));
          if (!(z3zE1964)) {  goto case_2257;  }
          zORIGIN(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2257: ;
        {
          uint64_t z3zE2025;
          z3zE2025 = zopcode;
          bool z3zE1963;
          z3zE1963 = (z3zE2025 == UINT64_C(51));
          if (!(z3zE1963)) {  goto case_2256;  }
          zCALLER(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2256: ;
        {
          uint64_t z3zE2026;
          z3zE2026 = zopcode;
          bool z3zE1962;
          z3zE1962 = (z3zE2026 == UINT64_C(52));
          if (!(z3zE1962)) {  goto case_2255;  }
          zCALLVALUE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2255: ;
        {
          uint64_t z3zE2027;
          z3zE2027 = zopcode;
          bool z3zE1961;
          z3zE1961 = (z3zE2027 == UINT64_C(53));
          if (!(z3zE1961)) {  goto case_2254;  }
          zCALLDATALOAD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2254: ;
        {
          uint64_t z3zE2028;
          z3zE2028 = zopcode;
          bool z3zE1960;
          z3zE1960 = (z3zE2028 == UINT64_C(54));
          if (!(z3zE1960)) {  goto case_2253;  }
          zCALLDATASIZE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2253: ;
        {
          uint64_t z3zE2029;
          z3zE2029 = zopcode;
          bool z3zE1959;
          z3zE1959 = (z3zE2029 == UINT64_C(55));
          if (!(z3zE1959)) {  goto case_2252;  }
          zCALLDATACOPY(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2252: ;
        {
          uint64_t z3zE2030;
          z3zE2030 = zopcode;
          bool z3zE1958;
          z3zE1958 = (z3zE2030 == UINT64_C(56));
          if (!(z3zE1958)) {  goto case_2251;  }
          zCODESIZE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2251: ;
        {
          uint64_t z3zE2031;
          z3zE2031 = zopcode;
          bool z3zE1957;
          z3zE1957 = (z3zE2031 == UINT64_C(57));
          if (!(z3zE1957)) {  goto case_2250;  }
          zCODECOPY(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2250: ;
        {
          uint64_t z3zE2032;
          z3zE2032 = zopcode;
          bool z3zE1956;
          z3zE1956 = (z3zE2032 == UINT64_C(58));
          if (!(z3zE1956)) {  goto case_2249;  }
          zGASPRICE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2249: ;
        {
          uint64_t z3zE2033;
          z3zE2033 = zopcode;
          bool z3zE1955;
          z3zE1955 = (z3zE2033 == UINT64_C(59));
          if (!(z3zE1955)) {  goto case_2248;  }
          zEXTCODESIZE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2248: ;
        {
          uint64_t z3zE2034;
          z3zE2034 = zopcode;
          bool z3zE1954;
          z3zE1954 = (z3zE2034 == UINT64_C(60));
          if (!(z3zE1954)) {  goto case_2247;  }
          zEXTCODECOPY(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2247: ;
        {
          uint64_t z3zE2035;
          z3zE2035 = zopcode;
          bool z3zE1953;
          z3zE1953 = (z3zE2035 == UINT64_C(61));
          if (!(z3zE1953)) {  goto case_2246;  }
          zRETURNDATASIZE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2246: ;
        {
          uint64_t z3zE2036;
          z3zE2036 = zopcode;
          bool z3zE1952;
          z3zE1952 = (z3zE2036 == UINT64_C(62));
          if (!(z3zE1952)) {  goto case_2245;  }
          zRETURNDATACOPY(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2245: ;
        {
          uint64_t z3zE2037;
          z3zE2037 = zopcode;
          bool z3zE1951;
          z3zE1951 = (z3zE2037 == UINT64_C(63));
          if (!(z3zE1951)) {  goto case_2244;  }
          zEXTCODEHASH(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2244: ;
        {
          uint64_t z3zE2038;
          z3zE2038 = zopcode;
          bool z3zE1950;
          z3zE1950 = (z3zE2038 == UINT64_C(64));
          if (!(z3zE1950)) {  goto case_2243;  }
          zBLOCKHASH(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2243: ;
        {
          uint64_t z3zE2039;
          z3zE2039 = zopcode;
          bool z3zE1949;
          z3zE1949 = (z3zE2039 == UINT64_C(65));
          if (!(z3zE1949)) {  goto case_2242;  }
          zCOINBASE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2242: ;
        {
          uint64_t z3zE2040;
          z3zE2040 = zopcode;
          bool z3zE1948;
          z3zE1948 = (z3zE2040 == UINT64_C(66));
          if (!(z3zE1948)) {  goto case_2241;  }
          zTIMESTAMP(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2241: ;
        {
          uint64_t z3zE2041;
          z3zE2041 = zopcode;
          bool z3zE1947;
          z3zE1947 = (z3zE2041 == UINT64_C(67));
          if (!(z3zE1947)) {  goto case_2240;  }
          zNUMBER(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2240: ;
        {
          uint64_t z3zE2042;
          z3zE2042 = zopcode;
          bool z3zE1946;
          z3zE1946 = (z3zE2042 == UINT64_C(68));
          if (!(z3zE1946)) {  goto case_2239;  }
          zPREVRANDAO(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2239: ;
        {
          uint64_t z3zE2043;
          z3zE2043 = zopcode;
          bool z3zE1945;
          z3zE1945 = (z3zE2043 == UINT64_C(69));
          if (!(z3zE1945)) {  goto case_2238;  }
          zGASLIMIT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2238: ;
        {
          uint64_t z3zE2044;
          z3zE2044 = zopcode;
          bool z3zE1944;
          z3zE1944 = (z3zE2044 == UINT64_C(70));
          if (!(z3zE1944)) {  goto case_2237;  }
          zCHAINID(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2237: ;
        {
          uint64_t z3zE2045;
          z3zE2045 = zopcode;
          bool z3zE1943;
          z3zE1943 = (z3zE2045 == UINT64_C(71));
          if (!(z3zE1943)) {  goto case_2236;  }
          zSELFBALANCE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2236: ;
        {
          uint64_t z3zE2046;
          z3zE2046 = zopcode;
          bool z3zE1942;
          z3zE1942 = (z3zE2046 == UINT64_C(72));
          if (!(z3zE1942)) {  goto case_2235;  }
          bool z2zE2080;
          z2zE2080 = zfork_gteq(zk_fork, zLondon);
          if (z2zE2080) {  zBASEFEE(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2235: ;
        {
          uint64_t z3zE2047;
          z3zE2047 = zopcode;
          bool z3zE1941;
          z3zE1941 = (z3zE2047 == UINT64_C(73));
          if (!(z3zE1941)) {  goto case_2234;  }
          bool z2zE2081;
          z2zE2081 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2081) {  zBLOBHASH(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2234: ;
        {
          uint64_t z3zE2048;
          z3zE2048 = zopcode;
          bool z3zE1940;
          z3zE1940 = (z3zE2048 == UINT64_C(74));
          if (!(z3zE1940)) {  goto case_2233;  }
          bool z2zE2082;
          z2zE2082 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2082) {  zBLOBBASEFEE(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2233: ;
        {
          uint64_t z3zE2049;
          z3zE2049 = zopcode;
          bool z3zE1939;
          z3zE1939 = (z3zE2049 == UINT64_C(75));
          if (!(z3zE1939)) {  goto case_2232;  }
          bool z2zE2083;
          z2zE2083 = zfork_gteq(zk_fork, zAmsterdam);
          if (z2zE2083) {  zSLOTNUM(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2232: ;
        {
          uint64_t z3zE2050;
          z3zE2050 = zopcode;
          bool z3zE1938;
          z3zE1938 = (z3zE2050 == UINT64_C(80));
          if (!(z3zE1938)) {  goto case_2231;  }
          zPOP(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2231: ;
        {
          uint64_t z3zE2051;
          z3zE2051 = zopcode;
          bool z3zE1937;
          z3zE1937 = (z3zE2051 == UINT64_C(81));
          if (!(z3zE1937)) {  goto case_2230;  }
          zMLOAD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2230: ;
        {
          uint64_t z3zE2052;
          z3zE2052 = zopcode;
          bool z3zE1936;
          z3zE1936 = (z3zE2052 == UINT64_C(82));
          if (!(z3zE1936)) {  goto case_2229;  }
          zMSTORE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2229: ;
        {
          uint64_t z3zE2053;
          z3zE2053 = zopcode;
          bool z3zE1935;
          z3zE1935 = (z3zE2053 == UINT64_C(83));
          if (!(z3zE1935)) {  goto case_2228;  }
          zMSTORE8(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2228: ;
        {
          uint64_t z3zE2054;
          z3zE2054 = zopcode;
          bool z3zE1934;
          z3zE1934 = (z3zE2054 == UINT64_C(84));
          if (!(z3zE1934)) {  goto case_2227;  }
          zSLOAD(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2227: ;
        {
          uint64_t z3zE2055;
          z3zE2055 = zopcode;
          bool z3zE1933;
          z3zE1933 = (z3zE2055 == UINT64_C(85));
          if (!(z3zE1933)) {  goto case_2226;  }
          zSSTORE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2226: ;
        {
          uint64_t z3zE2056;
          z3zE2056 = zopcode;
          bool z3zE1932;
          z3zE1932 = (z3zE2056 == UINT64_C(86));
          if (!(z3zE1932)) {  goto case_2225;  }
          zJUMP(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2225: ;
        {
          uint64_t z3zE2057;
          z3zE2057 = zopcode;
          bool z3zE1931;
          z3zE1931 = (z3zE2057 == UINT64_C(87));
          if (!(z3zE1931)) {  goto case_2224;  }
          zJUMPI(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2224: ;
        {
          uint64_t z3zE2058;
          z3zE2058 = zopcode;
          bool z3zE1930;
          z3zE1930 = (z3zE2058 == UINT64_C(88));
          if (!(z3zE1930)) {  goto case_2223;  }
          zPC(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2223: ;
        {
          uint64_t z3zE2059;
          z3zE2059 = zopcode;
          bool z3zE1929;
          z3zE1929 = (z3zE2059 == UINT64_C(89));
          if (!(z3zE1929)) {  goto case_2222;  }
          zMSIZE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2222: ;
        {
          uint64_t z3zE2060;
          z3zE2060 = zopcode;
          bool z3zE1928;
          z3zE1928 = (z3zE2060 == UINT64_C(90));
          if (!(z3zE1928)) {  goto case_2221;  }
          zGAS(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2221: ;
        {
          uint64_t z3zE2061;
          z3zE2061 = zopcode;
          bool z3zE1927;
          z3zE1927 = (z3zE2061 == UINT64_C(91));
          if (!(z3zE1927)) {  goto case_2220;  }
          zJUMPDEST(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2220: ;
        {
          uint64_t z3zE2062;
          z3zE2062 = zopcode;
          bool z3zE1926;
          z3zE1926 = (z3zE2062 == UINT64_C(92));
          if (!(z3zE1926)) {  goto case_2219;  }
          bool z2zE2084;
          z2zE2084 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2084) {  zTLOAD(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2219: ;
        {
          uint64_t z3zE2063;
          z3zE2063 = zopcode;
          bool z3zE1925;
          z3zE1925 = (z3zE2063 == UINT64_C(93));
          if (!(z3zE1925)) {  goto case_2218;  }
          bool z2zE2085;
          z2zE2085 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2085) {  zTSTORE(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2218: ;
        {
          uint64_t z3zE2064;
          z3zE2064 = zopcode;
          bool z3zE1924;
          z3zE1924 = (z3zE2064 == UINT64_C(94));
          if (!(z3zE1924)) {  goto case_2217;  }
          bool z2zE2086;
          z2zE2086 = zfork_gteq(zk_fork, zCancun);
          if (z2zE2086) {  zMCOPY(&z3zE1913, UNIT);  } else {  zINVALID(&z3zE1913, UNIT);  }
          goto finish_match_2206;
        }
      case_2217: ;
        {
          uint64_t z3zE2065;
          z3zE2065 = zopcode;
          bool z3zE1923;
          z3zE1923 = (z3zE2065 == UINT64_C(240));
          if (!(z3zE1923)) {  goto case_2216;  }
          zCREATE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2216: ;
        {
          uint64_t z3zE2066;
          z3zE2066 = zopcode;
          bool z3zE1922;
          z3zE1922 = (z3zE2066 == UINT64_C(241));
          if (!(z3zE1922)) {  goto case_2215;  }
          zCALL(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2215: ;
        {
          uint64_t z3zE2067;
          z3zE2067 = zopcode;
          bool z3zE1921;
          z3zE1921 = (z3zE2067 == UINT64_C(242));
          if (!(z3zE1921)) {  goto case_2214;  }
          zCALLCODE(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2214: ;
        {
          uint64_t z3zE2068;
          z3zE2068 = zopcode;
          bool z3zE1920;
          z3zE1920 = (z3zE2068 == UINT64_C(243));
          if (!(z3zE1920)) {  goto case_2213;  }
          zRETURN(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2213: ;
        {
          uint64_t z3zE2069;
          z3zE2069 = zopcode;
          bool z3zE1919;
          z3zE1919 = (z3zE2069 == UINT64_C(244));
          if (!(z3zE1919)) {  goto case_2212;  }
          zDELEGATECALL(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2212: ;
        {
          uint64_t z3zE2070;
          z3zE2070 = zopcode;
          bool z3zE1918;
          z3zE1918 = (z3zE2070 == UINT64_C(245));
          if (!(z3zE1918)) {  goto case_2211;  }
          zCREATE2(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2211: ;
        {
          uint64_t z3zE2071;
          z3zE2071 = zopcode;
          bool z3zE1917;
          z3zE1917 = (z3zE2071 == UINT64_C(250));
          if (!(z3zE1917)) {  goto case_2210;  }
          zSTATICCALL(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2210: ;
        {
          uint64_t z3zE2072;
          z3zE2072 = zopcode;
          bool z3zE1916;
          z3zE1916 = (z3zE2072 == UINT64_C(253));
          if (!(z3zE1916)) {  goto case_2209;  }
          zREVERT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2209: ;
        {
          uint64_t z3zE2073;
          z3zE2073 = zopcode;
          bool z3zE1915;
          z3zE1915 = (z3zE2073 == UINT64_C(255));
          if (!(z3zE1915)) {  goto case_2208;  }
          zSELFDESTRUCT(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2208: ;
        {
          zINVALID(&z3zE1913, UNIT);
          goto finish_match_2206;
        }
      case_2207: ;
      finish_match_2206: ;
        COPY(zast)((*(&z8zE438)), z3zE1913);
        KILL(zast)(&z3zE1913);
      }
    }
  }
end_function_2288: ;
  goto end_function_4047;
end_block_exception_2289: ;
  goto end_function_4047;
end_function_4047: ;
}

void zfetch(struct zast *z8zE439, unit z3zE1881)
{
  uint64_t zcurrent;
  zcurrent = zpc;
  struct zByteSliceFields zcode;
  zcode = zframe_code.zbytes;
  uint64_t zcode_length;
  zcode_length = zcode.zlen;
  bool z2zE2057;
  {
    bool z2zE2056;
    z2zE2056 = (zcurrent < zcode_length);
    z2zE2057 = not(z2zE2056);
  }
  if (z2zE2057) {  zSTOP((*(&z8zE439)), UNIT);  } else {
    uint64_t zopcode;
    {
      uint64_t z2zE2069;
      z2zE2069 = zslice_byte(zcode, zcurrent);
      zopcode = ((uint64_t) z2zE2069);
    }
    uint64_t zimmediate_offset;
    {    zimmediate_offset = (zcurrent + UINT64_C(1));
    }
    struct ztuple_z8z5u64zCz0z5unionz0zzastz9 zdecoded;
    CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded);
    {
      bool z2zE2059;
      {
        bool z2zE2058;
        z2zE2058 = (zopcode == UINT64_C(95));
        bool z3zE1882;
        if (z2zE2058) {  z3zE1882 = zfork_lt(zk_fork, zShanghai);  } else {  z3zE1882 = false;  }
        z2zE2059 = z3zE1882;
      }
      if (z2zE2059) {
        struct zast z2zE2060;
        CREATE(zast)(&z2zE2060);
        zINVALID(&z2zE2060, UNIT);
        struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1900;
        CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1900);
        z3zE1900.ztup0 = zimmediate_offset;
        COPY(zast)(&((&z3zE1900)->ztup1), z2zE2060);
        COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1900);
        KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1900);
        KILL(zast)(&z2zE2060);
      } else {
        bool z2zE2062;
        {
          bool z2zE2061;
          z2zE2061 = (!(zopcode < UINT64_C(95)));
          bool z3zE1883;
          if (z2zE2061) {  z3zE1883 = (!(UINT64_C(127) < zopcode));  } else {  z3zE1883 = false;  }
          z2zE2062 = z3zE1883;
        }
        if (z2zE2062) {
          uint64_t zsizze;
          {    zsizze = (zopcode - UINT64_C(95));
          }
          sail_u256 zvalue;
          zvalue = zread_push(zcode, zimmediate_offset, zsizze);
          uint64_t zafter_immediate;
          {
            uint64_t z2zE2064;
            {    z2zE2064 = (zcurrent + UINT64_C(1));
            }
            {    zafter_immediate = (z2zE2064 + zsizze);
            }
          }
          struct zast z2zE2063;
          CREATE(zast)(&z2zE2063);
          {
            struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 z3zE1898;
            z3zE1898.ztup0 = zsizze;
            z3zE1898.ztup1 = zvalue;
            zPUSH(&z2zE2063, z3zE1898);
          }
          struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1899;
          CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1899);
          z3zE1899.ztup0 = zafter_immediate;
          COPY(zast)(&((&z3zE1899)->ztup1), z2zE2063);
          COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1899);
          KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1899);
          KILL(zast)(&z2zE2063);
        } else {
          bool z2zE2067;
          {
            bool z2zE2066;
            z2zE2066 = zfork_gteq(zk_fork, zAmsterdam);
            bool z3zE1885;
            if (z2zE2066) {
              bool z2zE2065;
              z2zE2065 = (!(zopcode < UINT64_C(230)));
              bool z3zE1884;
              if (z2zE2065) {  z3zE1884 = (!(UINT64_C(232) < zopcode));  } else {  z3zE1884 = false;  }
              z3zE1885 = z3zE1884;
            } else {  z3zE1885 = false;  }
            z2zE2067 = z3zE1885;
          }
          if (z2zE2067) {
            uint64_t zimmediate;
            zimmediate = zslice_byte(zcode, zimmediate_offset);
            bool zimmediate_valid;
            {
              bool z3zE1887;
              {
                uint64_t zp0z3;
                zp0z3 = zopcode;
                bool z3zE1891;
                z3zE1891 = (zp0z3 == UINT64_C(230));
                if (!(z3zE1891)) {  goto case_2196;  }
                z3zE1887 = zdeep_stack_immediate_valid(zimmediate);
                goto finish_match_2192;
              }
            case_2196: ;
              {
                uint64_t z3zE1904;
                z3zE1904 = zopcode;
                bool z3zE1890;
                z3zE1890 = (z3zE1904 == UINT64_C(231));
                if (!(z3zE1890)) {  goto case_2195;  }
                z3zE1887 = zdeep_stack_immediate_valid(zimmediate);
                goto finish_match_2192;
              }
            case_2195: ;
              {
                uint64_t z3zE1905;
                z3zE1905 = zopcode;
                bool z3zE1889;
                z3zE1889 = (z3zE1905 == UINT64_C(232));
                if (!(z3zE1889)) {  goto case_2194;  }
                z3zE1887 = zexchange_immediate_valid(zimmediate);
                goto finish_match_2192;
              }
            case_2194: ;
              {
                z3zE1887 = false;
                goto finish_match_2192;
              }
            case_2193: ;
            finish_match_2192: ;
              zimmediate_valid = z3zE1887;
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
                uint64_t z3zE1906;
                z3zE1906 = zopcode;
                bool z3zE1896;
                z3zE1896 = (z3zE1906 == UINT64_C(230));
                if (!(z3zE1896)) {  goto case_2201;  }
                zDUPN(&zinstruction, zimmediate);
                goto finish_match_2197;
              }
            case_2201: ;
              {
                uint64_t z3zE1907;
                z3zE1907 = zopcode;
                bool z3zE1895;
                z3zE1895 = (z3zE1907 == UINT64_C(231));
                if (!(z3zE1895)) {  goto case_2200;  }
                zSWAPN(&zinstruction, zimmediate);
                goto finish_match_2197;
              }
            case_2200: ;
              {
                uint64_t z3zE1908;
                z3zE1908 = zopcode;
                bool z3zE1894;
                z3zE1894 = (z3zE1908 == UINT64_C(232));
                if (!(z3zE1894)) {  goto case_2199;  }
                zEXCHANGE(&zinstruction, zimmediate);
                goto finish_match_2197;
              }
            case_2199: ;
              {
                zINVALID(&zinstruction, UNIT);
                goto finish_match_2197;
              }
            case_2198: ;
            finish_match_2197: ;
            }
            struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1897;
            CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1897);
            z3zE1897.ztup0 = zafter_instruction;
            COPY(zast)(&((&z3zE1897)->ztup1), zinstruction);
            COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1897);
            KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1897);
            KILL(zast)(&zinstruction);
          } else {
            struct zast z2zE2068;
            CREATE(zast)(&z2zE2068);
            zdecode_simple(&z2zE2068, zopcode);
            struct ztuple_z8z5u64zCz0z5unionz0zzastz9 z3zE1886;
            CREATE(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1886);
            z3zE1886.ztup0 = zimmediate_offset;
            COPY(zast)(&((&z3zE1886)->ztup1), z2zE2068);
            COPY(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded, z3zE1886);
            KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&z3zE1886);
            KILL(zast)(&z2zE2068);
          }
        }
      }
    }
    struct zast z3zE1901;
    CREATE(zast)(&z3zE1901);
    {
      uint64_t znext_pc;
      znext_pc = zdecoded.ztup0;
      COPY(zast)(&z3zE1901, zdecoded.ztup1);
      zpc = znext_pc;
      unit z3zE1902;
      z3zE1902 = UNIT;
      goto finish_match_2202;
    }
  case_2203: ;
    sail_match_failure("fetch");
  finish_match_2202: ;
    COPY(zast)((*(&z8zE439)), z3zE1901);
    KILL(zast)(&z3zE1901);
    KILL(ztuple_z8z5u64zCz0z5unionz0zzastz9)(&zdecoded);
  }
end_function_2204: ;
  goto end_function_4046;
end_block_exception_2205: ;
  goto end_function_4046;
end_function_4046: ;
}

struct zByteSliceFields zframe_output(unit z3zE1875)
{
  struct zByteSliceFields z8zE440;
  struct zByteSliceFields z3zE1876;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2189;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltReturn) goto case_2189;
    struct zByteSliceFields zoutput;
    zoutput = zframe_status.variants.zHalted.variants.zHaltReturn;
    z3zE1876 = zoutput;
    goto finish_match_2186;
  }
case_2189: ;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2188;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltRevert) goto case_2188;
    struct zByteSliceFields z3zE1880;
    z3zE1880 = zframe_status.variants.zHalted.variants.zHaltRevert;
    z3zE1876 = z3zE1880;
    goto finish_match_2186;
  }
case_2188: ;
  {
    z3zE1876 = zEMPTY_SLICE;
    goto finish_match_2186;
  }
case_2187: ;
finish_match_2186: ;
  z8zE440 = z3zE1876;
end_function_2190: ;
  return z8zE440;
end_block_exception_2191: ;
  struct zByteSliceFields z8zE971 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE971;
}

bool zframe_succeeded(unit z3zE1869)
{
  bool z8zE441;
  bool z3zE1870;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2183;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltRevert) goto case_2183;
    z3zE1870 = false;
    goto finish_match_2179;
  }
case_2183: ;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_2182;
    z3zE1870 = true;
    goto finish_match_2179;
  }
case_2182: ;
  {
    if (zframe_status.kind != Kind_zRunning) goto case_2181;
    z3zE1870 = true;
    goto finish_match_2179;
  }
case_2181: ;
  {
    /* complete */
    z3zE1870 = false;
    goto finish_match_2179;
  }
case_2180: ;
finish_match_2179: ;
  z8zE441 = z3zE1870;
end_function_2184: ;
  return z8zE441;
end_block_exception_2185: ;

  return false;
}

unit zresume_call(struct zCallContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE442;
  zreturndata = zoutput;
  unit z3zE1861;
  z3zE1861 = UNIT;
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
  unit z3zE1865;
  z3zE1865 = zrestore_frame(zcheckpoint);
  unit z3zE1864;
  {
    z3zE1864 = zrefund_gas(zchild_left);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2178;
    }
  }
  unit z3zE1863;
  {
    z3zE1863 = zreturn_child_state_gas(zchild_state_left, zchild_state_spill);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2178;
    }
  }
  uint64_t z2zE2052;
  z2zE2052 = zcontinuation.zreturn_offset;
  uint64_t z2zE2053;
  z2zE2053 = zcontinuation.zreturn_length;
  unit z3zE1862;
  z3zE1862 = zreturndata_copy_prefix(z2zE2052, z2zE2053);
  if (zsucceeded) {
    unit z3zE1868;
    {
      z3zE1868 = zrecord_refund(zchild_refund);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2178;
      }
    }
    {
      z8zE442 = zpush_word(zWORD_ONE);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2178;
      }
    }
  } else {
    uint64_t z2zE2054;
    z2zE2054 = zcheckpoint.zstate;
    unit z3zE1867;
    z3zE1867 = zk_revert(z2zE2054);
    bool z2zE2055;
    z2zE2055 = zcontinuation.znew_account_charged;
    unit z3zE1866;
    if (z2zE2055) {
      {
        z3zE1866 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_2178;
        }
      }
    } else {  z3zE1866 = UNIT;  }
    {
      z8zE442 = zpush_word(zWORD_ZERO);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2178;
      }
    }
  }
  KILL(zFrameCheckpoint)(&zcheckpoint);
end_function_2177: ;
  return z8zE442;
end_block_exception_2178: ;

  return UNIT;
}

unit zresume_create(struct zCreateContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE443;
  zreturndata = zoutput;
  unit z3zE1842;
  z3zE1842 = UNIT;
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
  unit z3zE1843;
  if (zinitcode_succeeded) {
    bool z2zE2043;
    {
      bool z2zE2042;
      {
        bool z2zE2038;
        z2zE2038 = zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdeployed_sizze);
        z2zE2042 = not(z2zE2038);
      }
      bool z3zE1846;
      if (z2zE2042) {  z3zE1846 = true;  } else {
        bool z2zE2041;
        z2zE2041 = zfork_gteq(zk_fork, zLondon);
        bool z3zE1845;
        if (z2zE2041) {
          bool z2zE2040;
          z2zE2040 = (zdeployed_sizze != UINT64_C(0));
          bool z3zE1844;
          if (z2zE2040) {
            uint64_t z2zE2039;
            z2zE2039 = zslice_byte(zreturndata, UINT64_C(0));
            z3zE1844 = (z2zE2039 == UINT64_C(0xEF));
          } else {  z3zE1844 = false;  }
          z3zE1845 = z3zE1844;
        } else {  z3zE1845 = false;  }
        z3zE1846 = z3zE1845;
      }
      z2zE2043 = z3zE1846;
    }
    if (z2zE2043) {
      {
        z3zE1843 = zexc_halt(zOutOfGas);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_2176;
        }
      }
    } else {
      struct zoptionzIU64zK z2zE2044;
      CREATE(zoptionzIU64zK)(&z2zE2044);
      zcode_deployment_execution_cost(&z2zE2044, zdeployed_length, zgas_remaining);
      unit z3zE1847;
      {
        if (z2zE2044.kind != Kind_zSomezIU64zK) goto case_2174;
        uint64_t zexecution_deposit;
        zexecution_deposit = z2zE2044.variants.zSomezIU64zK;
        {
          zgas_remaining = zgas_sub_or_oog(zgas_remaining, zexecution_deposit);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE2044);
            goto end_block_exception_2176;
          }
        }
        unit z3zE1850;
        z3zE1850 = UNIT;
        uint64_t z2zE2045;
        {
          z2zE2045 = zcode_deployment_state_cost(zdeployed_length);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE2044);
            goto end_block_exception_2176;
          }
        }
        {
          z3zE1847 = zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2045);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE2044);
            goto end_block_exception_2176;
          }
        }
        goto finish_match_2172;
      }
    case_2174: ;
      {
        /* complete */
        bool z2zE2046;
        z2zE2046 = zfork_lt(zk_fork, zHomestead);
        if (z2zE2046) {
          zgas_remaining = zGAS_ZERO;
          unit z3zE1849;
          z3zE1849 = UNIT;
          zfrontier_empty_deposit = true;
          z3zE1847 = UNIT;
        } else {
          {
            z3zE1847 = zexc_halt(zOutOfGas);
            if (have_exception) {
              KILL(zFrameCheckpoint)(&zcheckpoint);
              KILL(zoptionzIU64zK)(&z2zE2044);
              goto end_block_exception_2176;
            }
          }
        }
        goto finish_match_2172;
      }
    case_2173: ;
    finish_match_2172: ;
      z3zE1843 = z3zE1847;
      KILL(zoptionzIU64zK)(&z2zE2044);
    }
  } else {  z3zE1843 = UNIT;  }
  bool zdeploy_succeeds;
  {
    bool z3zE1852;
    if (zinitcode_succeeded) {  z3zE1852 = zframe_succeeded(UNIT);  } else {  z3zE1852 = false;  }
    zdeploy_succeeds = z3zE1852;
  }
  uint64_t zchild_left;
  zchild_left = zgas_remaining;
  uint64_t zchild_state_left;
  zchild_state_left = zstate_gas_remaining;
  uint64_t zchild_state_spill;
  zchild_state_spill = zstate_gas_spilled;
  __int128 zchild_refund;
  zchild_refund = zframe_refund;
  unit z3zE1860;
  z3zE1860 = zrestore_frame(zcheckpoint);
  unit z3zE1859;
  {
    z3zE1859 = zrefund_gas(zchild_left);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2176;
    }
  }
  unit z3zE1858;
  {
    z3zE1858 = zreturn_child_state_gas(zchild_state_left, zchild_state_spill);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_2176;
    }
  }
  unit z3zE1853;
  if (zdeploy_succeeds) {
    unit z3zE1856;
    {
      z3zE1856 = zrecord_refund(zchild_refund);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2176;
      }
    }
    struct zByteSliceFields zdeployed_bytes;
    if (zfrontier_empty_deposit) {  zdeployed_bytes = zEMPTY_SLICE;  } else {  zdeployed_bytes = zreturndata;  }
    struct zByteSliceFields zdeployed_code;
    zdeployed_code = zvalidated_code_slice(zdeployed_bytes);
    sail_fixed_bytes_20 z2zE2047;
    z2zE2047 = zcontinuation.zaddress;
    unit z3zE1857;
    {
      z3zE1857 = zk_deploy_code(z2zE2047, zdeployed_code);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2176;
      }
    }
    sail_u256 z2zE2049;
    {
      sail_fixed_bytes_20 z2zE2048;
      z2zE2048 = zcontinuation.zaddress;
      z2zE2049 = zaddress_to_word(z2zE2048);
    }
    {
      z3zE1853 = zpush_word(z2zE2049);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2176;
      }
    }
  } else {
    uint64_t z2zE2050;
    z2zE2050 = zcheckpoint.zstate;
    unit z3zE1855;
    z3zE1855 = zk_revert(z2zE2050);
    bool z2zE2051;
    z2zE2051 = zcontinuation.znew_account_charged;
    unit z3zE1854;
    if (z2zE2051) {
      {
        z3zE1854 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_2176;
        }
      }
    } else {  z3zE1854 = UNIT;  }
    {
      z3zE1853 = zpush_word(zWORD_ZERO);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_2176;
      }
    }
  }
  if (zinitcode_succeeded) {  z8zE443 = zreturndata_clear(UNIT);  } else {  z8zE443 = UNIT;  }
  KILL(zFrameCheckpoint)(&zcheckpoint);
end_function_2175: ;
  return z8zE443;
end_block_exception_2176: ;

  return UNIT;
}

unit zresume_frame(struct zFrameContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE444;
  unit z3zE1839;
  {
    if (zcontinuation.kind != Kind_zResumeCall) goto case_2169;
    struct zCallContinuation zcall;
    CREATE(zCallContinuation)(&zcall);
    COPY(zCallContinuation)(&zcall, zcontinuation.variants.zResumeCall);
    {
      z3zE1839 = zresume_call(zcall, zoutput);
      if (have_exception) {
        KILL(zCallContinuation)(&zcall);
        goto end_block_exception_2171;
      }
    }
    KILL(zCallContinuation)(&zcall);
    goto finish_match_2167;
  }
case_2169: ;
  {
    /* complete */
    struct zCreateContinuation zcreate;
    CREATE(zCreateContinuation)(&zcreate);
    COPY(zCreateContinuation)(&zcreate, zcontinuation.variants.zResumeCreate);
    {
      z3zE1839 = zresume_create(zcreate, zoutput);
      if (have_exception) {
        KILL(zCreateContinuation)(&zcreate);
        goto end_block_exception_2171;
      }
    }
    KILL(zCreateContinuation)(&zcreate);
    goto finish_match_2167;
  }
case_2168: ;
finish_match_2167: ;
  z8zE444 = z3zE1839;
end_function_2170: ;
  return z8zE444;
end_block_exception_2171: ;

  return UNIT;
}

struct zByteSliceFields zinterpret(unit z3zE1833)
{
  struct zByteSliceFields z8zE445;
  unit z3zE1834;
  z3zE1834 = zframe_stack_reset(UNIT);
  bool zinterpreting;
  zinterpreting = true;
  struct zByteSliceFields zresult;
  zresult = zEMPTY_SLICE;
  bool z3zE1835;
  unit z3zE1836;
while_2163: ;
  {
    z3zE1835 = zinterpreting;
    if (!(z3zE1835)) goto wend_2164;
    bool z2zE2034;
    z2zE2034 = zis_running(UNIT);
    if (z2zE2034) {
      struct zast z2zE2035;
      CREATE(zast)(&z2zE2035);
      zfetch(&z2zE2035, UNIT);
      {
        z3zE1836 = zexecute(z2zE2035);
        if (have_exception) {
          KILL(zast)(&z2zE2035);
          goto end_block_exception_2166;
        }
      }
      KILL(zast)(&z2zE2035);
    } else {
      struct zByteSliceFields zoutput;
      zoutput = zframe_output(UNIT);
      bool z2zE2036;
      z2zE2036 = zframe_stack_is_empty(UNIT);
      if (z2zE2036) {
        zresult = zoutput;
        unit z3zE1837;
        z3zE1837 = UNIT;
        zinterpreting = false;
        z3zE1836 = UNIT;
      } else {
        struct zFrameContinuation z2zE2037;
        CREATE(zFrameContinuation)(&z2zE2037);
        zframe_stack_pop(&z2zE2037, UNIT);
        {
          z3zE1836 = zresume_frame(z2zE2037, zoutput);
          if (have_exception) {
            KILL(zFrameContinuation)(&z2zE2037);
            goto end_block_exception_2166;
          }
        }
        KILL(zFrameContinuation)(&z2zE2037);
      }
    }
    goto while_2163;
  }
wend_2164: ;
  unit z3zE1838;
  z3zE1838 = UNIT;
  z8zE445 = zresult;
end_function_2165: ;
  return z8zE445;
end_block_exception_2166: ;
  struct zByteSliceFields z8zE972 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE972;
}

