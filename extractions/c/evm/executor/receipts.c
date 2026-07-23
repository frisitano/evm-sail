/* Generated from sail/executor/receipts.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_287(void) {

  uint64_t z3zE345;
  z3zE345 = UINT64_C(256);
  zLOGS_BLOOM_BYTE_LENGTH = z3zE345;
let_end_907: ;
}
void kill_letbind_287(void) {
}

uint64_t zbloom_bit_mask(uint64_t zbit_to_set)
{
  uint64_t z8zE589;
  z8zE589 = ((zbit_to_set >= UINT64_C(64)) ? UINT64_C(0) : ((UINT64_C(0x01) << zbit_to_set) & UINT64_C(0xFF)));
end_function_1589: ;
  return z8zE589;
end_block_exception_1590: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_256 zbloom_set_bit(sail_fixed_bytes_256 zbloom, uint64_t zbit_to_set)
{
  sail_fixed_bytes_256 z8zE590;
  sail_fixed_bytes_256 zout;
  zout = zbloom;
  uint64_t zquotient;
  zquotient = (zbit_to_set / UINT64_C(8));
  uint64_t znatural_byte;
  {
    bool z2zE862;
    z2zE862 = (!(UINT64_C(255) < zquotient));
    znatural_byte = zquotient;
  }
  uint64_t zremainder;
  {    zremainder = (zbit_to_set % UINT64_C(8));
  }
  uint64_t zbit_in_byte;
  {
    bool z2zE861;
    z2zE861 = (!(UINT64_C(7) < zremainder));
    zbit_in_byte = zremainder;
  }
  uint64_t z2zE860;
  {
    uint64_t z2zE858;
    z2zE858 = fast_unsigned_vector_access_fixed_bytes_256(zout, znatural_byte);
    uint64_t z2zE859;
    z2zE859 = zbloom_bit_mask(zbit_in_byte);
    z2zE860 = (z2zE858 | z2zE859);
  }
  zout = fast_unsigned_vector_update_fixed_bytes_256(zout, znatural_byte, z2zE860);
  unit z3zE1092;
  z3zE1092 = UNIT;
  z8zE590 = zout;
end_function_1587: ;
  return z8zE590;
end_block_exception_1588: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_entry_hash(sail_fixed_bytes_256 zbloom, sail_fixed_bytes_32 zh)
{
  sail_fixed_bytes_256 z8zE591;
  sail_fixed_bytes_32 zbytes;
  zbytes = zh;
  sail_fixed_bytes_256 zout;
  {
    uint64_t z2zE857;
    {
      uint64_t z2zE856;
      {
        uint64_t z2zE854;
        {
          uint64_t z2zE853;
          z2zE853 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(31));
          z2zE854 = (safe_rshift(UINT64_MAX, 64 - 3) & (z2zE853 >> UINT64_C(0)));
        }
        uint64_t z2zE855;
        z2zE855 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(30));
        z2zE856 = (z2zE854 << 8) | z2zE855;
      }
      z2zE857 = ((uint64_t) z2zE856);
    }
    zout = zbloom_set_bit(zbloom, z2zE857);
  }
  uint64_t z2zE847;
  {
    uint64_t z2zE846;
    {
      uint64_t z2zE844;
      {
        uint64_t z2zE843;
        z2zE843 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(29));
        z2zE844 = (safe_rshift(UINT64_MAX, 64 - 3) & (z2zE843 >> UINT64_C(0)));
      }
      uint64_t z2zE845;
      z2zE845 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(28));
      z2zE846 = (z2zE844 << 8) | z2zE845;
    }
    z2zE847 = ((uint64_t) z2zE846);
  }
  zout = zbloom_set_bit(zout, z2zE847);
  unit z3zE1091;
  z3zE1091 = UNIT;
  uint64_t z2zE852;
  {
    uint64_t z2zE851;
    {
      uint64_t z2zE849;
      {
        uint64_t z2zE848;
        z2zE848 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(27));
        z2zE849 = (safe_rshift(UINT64_MAX, 64 - 3) & (z2zE848 >> UINT64_C(0)));
      }
      uint64_t z2zE850;
      z2zE850 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(26));
      z2zE851 = (z2zE849 << 8) | z2zE850;
    }
    z2zE852 = ((uint64_t) z2zE851);
  }
  zout = zbloom_set_bit(zout, z2zE852);
  unit z3zE1090;
  z3zE1090 = UNIT;
  z8zE591 = zout;
end_function_1585: ;
  return z8zE591;
end_block_exception_1586: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_topics(sail_fixed_bytes_256 zbloom, zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  sail_fixed_bytes_256 z8zE592;
  sail_fixed_bytes_256 z3zE1087;
  {
    if (!((ztopics == NULL))) goto case_1582;
    z3zE1087 = zbloom;
    goto finish_match_1580;
  }
case_1582: ;
  {
    /* complete */
    sail_u256 ztopic;
    ztopic = (*ztopics).hd;
    zz5listz8z5structz0zz__sail_c_repr_u256z9 zrest;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest, (*ztopics).tl);
    sail_fixed_bytes_256 z2zE842;
    {
      sail_fixed_bytes_32 z2zE841;
      z2zE841 = zkeccak256_word(ztopic);
      z2zE842 = zbloom_add_entry_hash(zbloom, z2zE841);
    }
    z3zE1087 = zbloom_add_topics(z2zE842, zrest);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    goto finish_match_1580;
  }
case_1581: ;
finish_match_1580: ;
  z8zE592 = z3zE1087;
end_function_1583: ;
  return z8zE592;
end_block_exception_1584: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_log(sail_fixed_bytes_256 zbloom, struct zLogEntry zlog)
{
  sail_fixed_bytes_256 z8zE593;
  sail_fixed_bytes_256 zwith_address;
  {
    sail_fixed_bytes_32 z2zE840;
    {
      sail_fixed_bytes_20 z2zE839;
      z2zE839 = zlog.zaddress;
      z2zE840 = zkeccak256_address(z2zE839);
    }
    zwith_address = zbloom_add_entry_hash(zbloom, z2zE840);
  }
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE838;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE838);
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE838, zlog.ztopics);
  z8zE593 = zbloom_add_topics(zwith_address, z2zE838);
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE838);
end_function_1578: ;
  return z8zE593;
end_block_exception_1579: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_logs(sail_fixed_bytes_256 zbloom, zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  sail_fixed_bytes_256 z8zE594;
  sail_fixed_bytes_256 z3zE1084;
  {
    if (!((zlogs == NULL))) goto case_1575;
    z3zE1084 = zbloom;
    goto finish_match_1573;
  }
case_1575: ;
  {
    /* complete */
    struct zLogEntry zlog;
    CREATE(zLogEntry)(&zlog);
    COPY(zLogEntry)(&zlog, (*zlogs).hd);
    zz5listz8z5structz0zzLogEntryz9 zrest;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&zrest, (*zlogs).tl);
    sail_fixed_bytes_256 z2zE837;
    z2zE837 = zbloom_add_log(zbloom, zlog);
    z3zE1084 = zbloom_add_logs(z2zE837, zrest);
    KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    KILL(zLogEntry)(&zlog);
    goto finish_match_1573;
  }
case_1574: ;
finish_match_1573: ;
  z8zE594 = z3zE1084;
end_function_1576: ;
  return z8zE594;
end_block_exception_1577: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zlogs_bloom_for_logs(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  sail_fixed_bytes_256 z8zE595;
  z8zE595 = zbloom_add_logs(zEMPTY_LOGS_BLOOM, zlogs);
end_function_1571: ;
  return z8zE595;
end_block_exception_1572: ;

  return fixed_bytes_256_zero();
}

uint64_t ztopics_rlp_content_sizze(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  uint64_t z8zE596;
  uint64_t z3zE1081;
  {
    if (!((ztopics == NULL))) goto case_1568;
    z3zE1081 = UINT64_C(0);
    goto finish_match_1566;
  }
case_1568: ;
  {
    /* complete */
    zz5listz8z5structz0zz__sail_c_repr_u256z9 zrest;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest, (*ztopics).tl);
    uint64_t zrest_length;
    {
      zrest_length = ztopics_rlp_content_sizze(zrest);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
        goto end_block_exception_1570;
      }
    }
    {
      z3zE1081 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(33), zrest_length);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
        goto end_block_exception_1570;
      }
    }
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    goto finish_match_1566;
  }
case_1567: ;
finish_match_1566: ;
  z8zE596 = z3zE1081;
end_function_1569: ;
  return z8zE596;
end_block_exception_1570: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztopics_rlp_sizze(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  uint64_t z8zE597;
  uint64_t z2zE836;
  {
    z2zE836 = ztopics_rlp_content_sizze(ztopics);
    if (have_exception) {  goto end_block_exception_1565;  }
  }
  {
    z8zE597 = zrlp_scratch_list_sizze(z2zE836);
    if (have_exception) {  goto end_block_exception_1565;  }
  }
end_function_1564: ;
  return z8zE597;
end_block_exception_1565: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlog_entry_rlp_content_sizze(struct zLogEntry zlog)
{
  uint64_t z8zE598;
  uint64_t ztopics_length;
  {
    zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE835;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE835);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE835, zlog.ztopics);
    {
      ztopics_length = ztopics_rlp_sizze(z2zE835);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE835);
        goto end_block_exception_1563;
      }
    }
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE835);
  }
  uint64_t zdata_length;
  {
    struct zByteSliceFields z2zE834;
    z2zE834 = zlog.zdata;
    {
      zdata_length = zrlp_scratch_slice_sizze(z2zE834);
      if (have_exception) {  goto end_block_exception_1563;  }
    }
  }
  uint64_t z2zE833;
  {
    z2zE833 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(21), ztopics_length);
    if (have_exception) {  goto end_block_exception_1563;  }
  }
  {
    z8zE598 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE833, zdata_length);
    if (have_exception) {  goto end_block_exception_1563;  }
  }
end_function_1562: ;
  return z8zE598;
end_block_exception_1563: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlog_entry_rlp_sizze(struct zLogEntry zlog)
{
  uint64_t z8zE599;
  uint64_t z2zE832;
  {
    z2zE832 = zlog_entry_rlp_content_sizze(zlog);
    if (have_exception) {  goto end_block_exception_1561;  }
  }
  {
    z8zE599 = zrlp_scratch_list_sizze(z2zE832);
    if (have_exception) {  goto end_block_exception_1561;  }
  }
end_function_1560: ;
  return z8zE599;
end_block_exception_1561: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlogs_rlp_content_sizze(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  uint64_t z8zE600;
  uint64_t z3zE1078;
  {
    if (!((zlogs == NULL))) goto case_1557;
    z3zE1078 = UINT64_C(0);
    goto finish_match_1555;
  }
case_1557: ;
  {
    /* complete */
    struct zLogEntry zlog;
    CREATE(zLogEntry)(&zlog);
    COPY(zLogEntry)(&zlog, (*zlogs).hd);
    zz5listz8z5structz0zzLogEntryz9 zrest;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&zrest, (*zlogs).tl);
    uint64_t zlog_length;
    {
      zlog_length = zlog_entry_rlp_sizze(zlog);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1559;
      }
    }
    uint64_t zrest_length;
    {
      zrest_length = zlogs_rlp_content_sizze(zrest);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1559;
      }
    }
    {
      z3zE1078 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlog_length, zrest_length);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1559;
      }
    }
    KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    KILL(zLogEntry)(&zlog);
    goto finish_match_1555;
  }
case_1556: ;
finish_match_1555: ;
  z8zE600 = z3zE1078;
end_function_1558: ;
  return z8zE600;
end_block_exception_1559: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlogs_rlp_sizze(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  uint64_t z8zE601;
  uint64_t z2zE831;
  {
    z2zE831 = zlogs_rlp_content_sizze(zlogs);
    if (have_exception) {  goto end_block_exception_1554;  }
  }
  {
    z8zE601 = zrlp_scratch_list_sizze(z2zE831);
    if (have_exception) {  goto end_block_exception_1554;  }
  }
end_function_1553: ;
  return z8zE601;
end_block_exception_1554: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_topics_content(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  unit z8zE602;
  unit z3zE1074;
  {
    if (!((ztopics == NULL))) goto case_1550;
    z3zE1074 = UNIT;
    goto finish_match_1548;
  }
case_1550: ;
  {
    /* complete */
    sail_u256 ztopic;
    ztopic = (*ztopics).hd;
    zz5listz8z5structz0zz__sail_c_repr_u256z9 zrest;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest, (*ztopics).tl);
    unit z3zE1075;
    z3zE1075 = zrlp_write_word(ztopic);
    z3zE1074 = zrlp_write_topics_content(zrest);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    goto finish_match_1548;
  }
case_1549: ;
finish_match_1548: ;
  z8zE602 = z3zE1074;
end_function_1551: ;
  return z8zE602;
end_block_exception_1552: ;

  return UNIT;
}

unit zrlp_write_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  unit z8zE603;
  uint64_t z2zE830;
  {
    z2zE830 = ztopics_rlp_content_sizze(ztopics);
    if (have_exception) {  goto end_block_exception_1547;  }
  }
  unit z3zE1073;
  z3zE1073 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE830);
  z8zE603 = zrlp_write_topics_content(ztopics);
end_function_1546: ;
  return z8zE603;
end_block_exception_1547: ;

  return UNIT;
}

unit zrlp_write_log_entry(struct zLogEntry zlog)
{
  unit z8zE604;
  uint64_t z2zE826;
  {
    z2zE826 = zlog_entry_rlp_content_sizze(zlog);
    if (have_exception) {  goto end_block_exception_1545;  }
  }
  unit z3zE1072;
  z3zE1072 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE826);
  sail_fixed_bytes_20 z2zE827;
  z2zE827 = zlog.zaddress;
  unit z3zE1071;
  z3zE1071 = zrlp_write_addr(z2zE827);
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE828;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE828);
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE828, zlog.ztopics);
  unit z3zE1070;
  {
    z3zE1070 = zrlp_write_topics(z2zE828);
    if (have_exception) {
      KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE828);
      goto end_block_exception_1545;
    }
  }
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE828);
  struct zByteSliceFields z2zE829;
  z2zE829 = zlog.zdata;
  z8zE604 = zrlp_write_slice(z2zE829);
end_function_1544: ;
  return z8zE604;
end_block_exception_1545: ;

  return UNIT;
}

unit zrlp_write_logs_content(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  unit z8zE605;
  unit z3zE1066;
  {
    if (!((zlogs == NULL))) goto case_1541;
    z3zE1066 = UNIT;
    goto finish_match_1539;
  }
case_1541: ;
  {
    /* complete */
    struct zLogEntry zlog;
    CREATE(zLogEntry)(&zlog);
    COPY(zLogEntry)(&zlog, (*zlogs).hd);
    zz5listz8z5structz0zzLogEntryz9 zrest;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&zrest, (*zlogs).tl);
    unit z3zE1067;
    {
      z3zE1067 = zrlp_write_log_entry(zlog);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1543;
      }
    }
    {
      z3zE1066 = zrlp_write_logs_content(zrest);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1543;
      }
    }
    KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    KILL(zLogEntry)(&zlog);
    goto finish_match_1539;
  }
case_1540: ;
finish_match_1539: ;
  z8zE605 = z3zE1066;
end_function_1542: ;
  return z8zE605;
end_block_exception_1543: ;

  return UNIT;
}

unit zrlp_write_logs(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  unit z8zE606;
  uint64_t z2zE825;
  {
    z2zE825 = zlogs_rlp_content_sizze(zlogs);
    if (have_exception) {  goto end_block_exception_1538;  }
  }
  unit z3zE1065;
  z3zE1065 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE825);
  {
    z8zE606 = zrlp_write_logs_content(zlogs);
    if (have_exception) {  goto end_block_exception_1538;  }
  }
end_function_1537: ;
  return z8zE606;
end_block_exception_1538: ;

  return UNIT;
}

uint64_t zreceipt_payload_content_sizze(struct zReceipt zr, uint64_t zcumulative_gas_used)
{
  uint64_t z8zE607;
  uint64_t zstatus;
  {
    bool z2zE824;
    z2zE824 = zr.zsuccess;
    if (z2zE824) {  zstatus = UINT64_C(1);  } else {  zstatus = UINT64_C(0);  }
  }
  uint64_t zstatus_length;
  {
    uint64_t z2zE823;
    z2zE823 = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zstatus);
    zstatus_length = zrlp_scratch_small_length(z2zE823);
  }
  sail_u256 zgas_word;
  zgas_word = zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcumulative_gas_used);
  uint64_t zgas_length;
  {
    uint64_t z2zE822;
    z2zE822 = zrlp_uint_word_sizze(zgas_word);
    zgas_length = zrlp_scratch_small_length(z2zE822);
  }
  uint64_t zbloom_length;
  {
    uint64_t z2zE821;
    {
      uint64_t z2zE820;
      z2zE820 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zLOGS_BLOOM_BYTE_LENGTH);
      z2zE821 = zrlp_scratch_small_length(z2zE820);
    }
    {
      zbloom_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zLOGS_BLOOM_BYTE_LENGTH, z2zE821);
      if (have_exception) {  goto end_block_exception_1536;  }
    }
  }
  uint64_t zlogs_length;
  {
    zz5listz8z5structz0zzLogEntryz9 z2zE819;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE819);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE819, zr.zlogs);
    {
      zlogs_length = zlogs_rlp_sizze(z2zE819);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE819);
        goto end_block_exception_1536;
      }
    }
    KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE819);
  }
  uint64_t zfixed_length;
  {
    zfixed_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zstatus_length, zgas_length);
    if (have_exception) {  goto end_block_exception_1536;  }
  }
  uint64_t z2zE818;
  {
    z2zE818 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zfixed_length, zbloom_length);
    if (have_exception) {  goto end_block_exception_1536;  }
  }
  {
    z8zE607 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE818, zlogs_length);
    if (have_exception) {  goto end_block_exception_1536;  }
  }
end_function_1535: ;
  return z8zE607;
end_block_exception_1536: ;

  return UINT64_C(0xdeadc0de);
}

struct zByteSliceFields zreceipt_encoded(struct zReceipt zr, uint64_t zcumulative_gas_used)
{
  struct zByteSliceFields z8zE608;
  uint64_t zstatus;
  {
    bool z2zE817;
    z2zE817 = zr.zsuccess;
    if (z2zE817) {  zstatus = UINT64_C(1);  } else {  zstatus = UINT64_C(0);  }
  }
  zz5listz8z5bv8z9 zbloom;
  CREATE(zz5listz8z5bv8z9)(&zbloom);
  {
    sail_fixed_bytes_256 z2zE816;
    {
      zz5listz8z5structz0zzLogEntryz9 z2zE815;
      CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE815);
      COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE815, zr.zlogs);
      z2zE816 = zlogs_bloom_for_logs(z2zE815);
      KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE815);
    }
    zlogs_bloom_bytes(&zbloom, z2zE816);
  }
  uint64_t zcontent_len;
  {
    zcontent_len = zreceipt_payload_content_sizze(zr, zcumulative_gas_used);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      goto end_block_exception_1534;
    }
  }
  bool ztyped;
  {
    uint64_t z2zE814;
    {
      enum zTxType z2zE813;
      z2zE813 = zr.ztx_type;
      z2zE814 = ztx_type_byte(z2zE813);
    }
    ztyped = (z2zE814 != UINT64_C(0x00));
  }
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1063;
  if (ztyped) {
    zz5listz8z5bv8z9 z2zE811;
    CREATE(zz5listz8z5bv8z9)(&z2zE811);
    {
      uint64_t z2zE810;
      {
        enum zTxType z2zE809;
        z2zE809 = zr.ztx_type;
        z2zE810 = ztx_type_byte(z2zE809);
      }
      zconsz3z5bv8(&z2zE811, z2zE810, z2zE811);
    }
    z3zE1063 = zscratch_push_bytes(z2zE811, UINT64_C(1));
    KILL(zz5listz8z5bv8z9)(&z2zE811);
  } else {  z3zE1063 = UNIT;  }
  unit z3zE1062;
  z3zE1062 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE1061;
  z3zE1061 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zstatus);
  unit z3zE1060;
  z3zE1060 = zrlp_write_uint_nat(zcumulative_gas_used);
  unit z3zE1059;
  z3zE1059 = zrlp_write_bytes(zbloom, zLOGS_BLOOM_BYTE_LENGTH);
  zz5listz8z5structz0zzLogEntryz9 z2zE812;
  CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE812);
  COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE812, zr.zlogs);
  unit z3zE1058;
  {
    z3zE1058 = zrlp_write_logs(z2zE812);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE812);
      goto end_block_exception_1534;
    }
  }
  KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE812);
  z8zE608 = zrlp_finish(zstart);
  KILL(zz5listz8z5bv8z9)(&zbloom);
end_function_1533: ;
  return z8zE608;
end_block_exception_1534: ;
  struct zByteSliceFields z8zE1091 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1091;
}

void zreceipt_accumulator_empty(struct zReceiptAccumulator *z8zE609, unit z3zE1056)
{
  struct zTrieBuilder z2zE806;
  CREATE(zTrieBuilder)(&z2zE806);
  ztrie_builder_empty(&z2zE806, UNIT);
  struct zoptionzIRPendingReceiptzK z2zE807;
  CREATE(zoptionzIRPendingReceiptzK)(&z2zE807);
  zNonezIRPendingReceiptzK(&z2zE807, UNIT);
  struct zoptionzIRPendingReceiptzK z2zE808;
  CREATE(zoptionzIRPendingReceiptzK)(&z2zE808);
  zNonezIRPendingReceiptzK(&z2zE808, UNIT);
  struct zReceiptAccumulator z3zE1057;
  CREATE(zReceiptAccumulator)(&z3zE1057);
  z3zE1057.zbloom = zEMPTY_LOGS_BLOOM;
  COPY(zTrieBuilder)(&((&z3zE1057)->zbuilder), z2zE806);
  z3zE1057.zcount = UINT64_C(0);
  z3zE1057.zcumulative_gas_used = UINT64_C(0);
  COPY(zoptionzIRPendingReceiptzK)(&((&z3zE1057)->zfirst), z2zE807);
  COPY(zoptionzIRPendingReceiptzK)(&((&z3zE1057)->zpending), z2zE808);
  COPY(zReceiptAccumulator)((*(&z8zE609)), z3zE1057);
  KILL(zReceiptAccumulator)(&z3zE1057);
  KILL(zoptionzIRPendingReceiptzK)(&z2zE808);
  KILL(zoptionzIRPendingReceiptzK)(&z2zE807);
  KILL(zTrieBuilder)(&z2zE806);
end_function_1531: ;
  goto end_function_4019;
end_block_exception_1532: ;
  goto end_function_4019;
end_function_4019: ;
}

void zreceipt_insert(struct zTrieBuilder *z8zE610, struct zTrieBuilder zbuilder, struct zPendingReceipt zpending, struct zoptionzIRTriePathzK znext_key)
{
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  struct zByteSliceFields zvalue;
  {
    struct zReceipt z2zE804;
    CREATE(zReceipt)(&z2zE804);
    COPY(zReceipt)(&z2zE804, zpending.zreceipt);
    uint64_t z2zE805;
    z2zE805 = zpending.zcumulative_gas_used;
    {
      zvalue = zreceipt_encoded(z2zE804, z2zE805);
      if (have_exception) {
        KILL(zReceipt)(&z2zE804);
        goto end_block_exception_1530;
      }
    }
    KILL(zReceipt)(&z2zE804);
  }
  struct zTrieBuilder zinserted;
  CREATE(zTrieBuilder)(&zinserted);
  {
    struct zTrieItem z2zE803;
    CREATE(zTrieItem)(&z2zE803);
    {
      struct zTriePath z2zE802;
      {
        uint64_t z2zE801;
        z2zE801 = zpending.zindex;
        {
          z2zE802 = ztrie_index_key(z2zE801);
          if (have_exception) {
            KILL(zTrieBuilder)(&zinserted);
            KILL(zTrieItem)(&z2zE803);
            goto end_block_exception_1530;
          }
        }
      }
      zitem_leaf(&z2zE803, z2zE802, zvalue);
    }
    {
      ztrie_insert_item(&zinserted, zbuilder, z2zE803, znext_key);
      if (have_exception) {
        KILL(zTrieBuilder)(&zinserted);
        KILL(zTrieItem)(&z2zE803);
        goto end_block_exception_1530;
      }
    }
    KILL(zTrieItem)(&z2zE803);
  }
  unit z3zE1055;
  z3zE1055 = zscratch_rewind(zmark);
  COPY(zTrieBuilder)((*(&z8zE610)), zinserted);
  KILL(zTrieBuilder)(&zinserted);
end_function_1529: ;
  goto end_function_4018;
end_block_exception_1530: ;
  goto end_function_4018;
end_function_4018: ;
}

void zreceipt_accumulator_push(struct zReceiptAccumulator *z8zE611, struct zReceiptAccumulator zacc, struct zReceipt zreceipt, uint64_t znext_count)
{
  uint64_t zcumulative;
  {
    uint64_t z2zE799;
    z2zE799 = zacc.zcumulative_gas_used;
    uint64_t z2zE800;
    z2zE800 = zreceipt.zgas_used;
    {
      zcumulative = zconserved_gas_add(z2zE799, z2zE800);
      if (have_exception) {  goto end_block_exception_1528;  }
    }
  }
  struct zPendingReceipt zcurrent;
  CREATE(zPendingReceipt)(&zcurrent);
  {
    uint64_t z2zE798;
    z2zE798 = zacc.zcount;
    zcurrent.zcumulative_gas_used = zcumulative;
    zcurrent.zindex = z2zE798;
    COPY(zReceipt)(&((&zcurrent)->zreceipt), zreceipt);
  }
  struct zTrieBuilder zbuilder;
  CREATE(zTrieBuilder)(&zbuilder);
  COPY(zTrieBuilder)(&zbuilder, zacc.zbuilder);
  struct zoptionzIRPendingReceiptzK zfirst;
  CREATE(zoptionzIRPendingReceiptzK)(&zfirst);
  COPY(zoptionzIRPendingReceiptzK)(&zfirst, zacc.zfirst);
  struct zoptionzIRPendingReceiptzK zpending;
  CREATE(zoptionzIRPendingReceiptzK)(&zpending);
  COPY(zoptionzIRPendingReceiptzK)(&zpending, zacc.zpending);
  bool z2zE780;
  {
    uint64_t z2zE779;
    z2zE779 = zacc.zcount;
    z2zE780 = (z2zE779 == UINT64_C(0));
  }
  unit z3zE1037;
  if (z2zE780) {
    zSomezIRPendingReceiptzK(&zfirst, zcurrent);
    z3zE1037 = UNIT;
  } else {
    bool z2zE782;
    {
      uint64_t z2zE781;
      z2zE781 = zacc.zcount;
      z2zE782 = (z2zE781 == UINT64_C(128));
    }
    if (z2zE782) {
      unit z3zE1042;
      {
        if (zpending.kind != Kind_zSomezIRPendingReceiptzK) goto case_1523;
        struct zPendingReceipt zprevious;
        CREATE(zPendingReceipt)(&zprevious);
        COPY(zPendingReceipt)(&zprevious, zpending.variants.zSomezIRPendingReceiptzK);
        struct zoptionzIRTriePathzK z2zE783;
        CREATE(zoptionzIRTriePathzK)(&z2zE783);
        {
          struct zTriePath z3zE1044;
          zz5vecz8z5bv8z9 z3zE1045;
          CREATE(zz5vecz8z5bv8z9)(&z3zE1045);
          internal_vector_init_zz5vecz8z5bv8z9(&z3zE1045, INT64_C(32));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(0), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(1), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(2), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(3), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(4), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(5), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(6), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(7), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(8), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(9), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(10), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(11), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(12), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(13), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(14), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(15), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(16), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(17), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(18), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(19), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(20), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(21), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(22), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(23), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(24), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(25), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(26), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(27), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(28), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(29), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(30), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1045, z3zE1045, INT64_C(31), UINT64_C(0x80));
          for (size_t z8zE1092 = 0; z8zE1092 < 32; ++z8zE1092) {
            z3zE1044.zdata.bytes[z8zE1092] = (uint8_t)(z3zE1045.data[z8zE1092] & UINT64_C(0xff));
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE1045);
          z3zE1044.zlen = UINT64_C(2);
          zSomezIRTriePathzK(&z2zE783, z3zE1044);
        }
        {
          zreceipt_insert(&zbuilder, zbuilder, zprevious, z2zE783);
          if (have_exception) {
            KILL(zPendingReceipt)(&zcurrent);
            KILL(zTrieBuilder)(&zbuilder);
            KILL(zoptionzIRPendingReceiptzK)(&zfirst);
            KILL(zoptionzIRPendingReceiptzK)(&zpending);
            KILL(zPendingReceipt)(&zprevious);
            KILL(zoptionzIRTriePathzK)(&z2zE783);
            goto end_block_exception_1528;
          }
        }
        z3zE1042 = UNIT;
        KILL(zoptionzIRTriePathzK)(&z2zE783);
        KILL(zPendingReceipt)(&zprevious);
        goto finish_match_1521;
      }
    case_1523: ;
      {
        /* complete */
        struct zexception z2zE784;
        CREATE(zexception)(&z2zE784);
        zInvalidBlock(&z2zE784, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE784);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/receipts.sail:250.26-250.62");
        KILL(zPendingReceipt)(&zcurrent);
        KILL(zTrieBuilder)(&zbuilder);
        KILL(zoptionzIRPendingReceiptzK)(&zfirst);
        KILL(zoptionzIRPendingReceiptzK)(&zpending);
        KILL(zexception)(&z2zE784);
        goto end_block_exception_1528;
        /* unreachable after throw */
        KILL(zexception)(&z2zE784);
        goto finish_match_1521;
      }
    case_1522: ;
    finish_match_1521: ;
      unit z3zE1052;
      z3zE1052 = z3zE1042;
      unit z3zE1047;
      {
        if (zfirst.kind != Kind_zSomezIRPendingReceiptzK) goto case_1526;
        struct zPendingReceipt zzzero;
        CREATE(zPendingReceipt)(&zzzero);
        COPY(zPendingReceipt)(&zzzero, zfirst.variants.zSomezIRPendingReceiptzK);
        struct zoptionzIRTriePathzK z2zE787;
        CREATE(zoptionzIRTriePathzK)(&z2zE787);
        {
          struct zTriePath z2zE786;
          {
            uint64_t z2zE785;
            z2zE785 = zacc.zcount;
            {
              z2zE786 = ztrie_index_key(z2zE785);
              if (have_exception) {
                KILL(zPendingReceipt)(&zcurrent);
                KILL(zTrieBuilder)(&zbuilder);
                KILL(zoptionzIRPendingReceiptzK)(&zfirst);
                KILL(zoptionzIRPendingReceiptzK)(&zpending);
                KILL(zPendingReceipt)(&zzzero);
                KILL(zoptionzIRTriePathzK)(&z2zE787);
                goto end_block_exception_1528;
              }
            }
          }
          zSomezIRTriePathzK(&z2zE787, z2zE786);
        }
        {
          zreceipt_insert(&zbuilder, zbuilder, zzzero, z2zE787);
          if (have_exception) {
            KILL(zPendingReceipt)(&zcurrent);
            KILL(zTrieBuilder)(&zbuilder);
            KILL(zoptionzIRPendingReceiptzK)(&zfirst);
            KILL(zoptionzIRPendingReceiptzK)(&zpending);
            KILL(zPendingReceipt)(&zzzero);
            KILL(zoptionzIRTriePathzK)(&z2zE787);
            goto end_block_exception_1528;
          }
        }
        z3zE1047 = UNIT;
        KILL(zoptionzIRTriePathzK)(&z2zE787);
        KILL(zPendingReceipt)(&zzzero);
        goto finish_match_1524;
      }
    case_1526: ;
      {
        /* complete */
        struct zexception z2zE788;
        CREATE(zexception)(&z2zE788);
        zInvalidBlock(&z2zE788, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE788);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/receipts.sail:254.26-254.62");
        KILL(zPendingReceipt)(&zcurrent);
        KILL(zTrieBuilder)(&zbuilder);
        KILL(zoptionzIRPendingReceiptzK)(&zfirst);
        KILL(zoptionzIRPendingReceiptzK)(&zpending);
        KILL(zexception)(&z2zE788);
        goto end_block_exception_1528;
        /* unreachable after throw */
        KILL(zexception)(&z2zE788);
        goto finish_match_1524;
      }
    case_1525: ;
    finish_match_1524: ;
      unit z3zE1051;
      z3zE1051 = z3zE1047;
      zNonezIRPendingReceiptzK(&zfirst, UNIT);
      unit z3zE1050;
      z3zE1050 = UNIT;
      zSomezIRPendingReceiptzK(&zpending, zcurrent);
      z3zE1037 = UNIT;
    } else {
      unit z3zE1038;
      {
        if (zpending.kind != Kind_zSomezIRPendingReceiptzK) goto case_1520;
        struct zPendingReceipt z3zE1054;
        CREATE(zPendingReceipt)(&z3zE1054);
        COPY(zPendingReceipt)(&z3zE1054, zpending.variants.zSomezIRPendingReceiptzK);
        struct zoptionzIRTriePathzK z2zE791;
        CREATE(zoptionzIRTriePathzK)(&z2zE791);
        {
          struct zTriePath z2zE790;
          {
            uint64_t z2zE789;
            z2zE789 = zacc.zcount;
            {
              z2zE790 = ztrie_index_key(z2zE789);
              if (have_exception) {
                KILL(zPendingReceipt)(&zcurrent);
                KILL(zTrieBuilder)(&zbuilder);
                KILL(zoptionzIRPendingReceiptzK)(&zfirst);
                KILL(zoptionzIRPendingReceiptzK)(&zpending);
                KILL(zPendingReceipt)(&z3zE1054);
                KILL(zoptionzIRTriePathzK)(&z2zE791);
                goto end_block_exception_1528;
              }
            }
          }
          zSomezIRTriePathzK(&z2zE791, z2zE790);
        }
        {
          zreceipt_insert(&zbuilder, zbuilder, z3zE1054, z2zE791);
          if (have_exception) {
            KILL(zPendingReceipt)(&zcurrent);
            KILL(zTrieBuilder)(&zbuilder);
            KILL(zoptionzIRPendingReceiptzK)(&zfirst);
            KILL(zoptionzIRPendingReceiptzK)(&zpending);
            KILL(zPendingReceipt)(&z3zE1054);
            KILL(zoptionzIRTriePathzK)(&z2zE791);
            goto end_block_exception_1528;
          }
        }
        unit z3zE1040;
        z3zE1040 = UNIT;
        KILL(zoptionzIRTriePathzK)(&z2zE791);
        zSomezIRPendingReceiptzK(&zpending, zcurrent);
        z3zE1038 = UNIT;
        KILL(zPendingReceipt)(&z3zE1054);
        goto finish_match_1518;
      }
    case_1520: ;
      {
        /* complete */
        bool z2zE793;
        {
          uint64_t z2zE792;
          z2zE792 = zacc.zcount;
          z2zE793 = (z2zE792 == UINT64_C(1));
        }
        if (z2zE793) {
          zSomezIRPendingReceiptzK(&zpending, zcurrent);
          z3zE1038 = UNIT;
        } else {
          struct zexception z2zE794;
          CREATE(zexception)(&z2zE794);
          zInvalidBlock(&z2zE794, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE794);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/receipts.sail:267.20-267.56");
          KILL(zPendingReceipt)(&zcurrent);
          KILL(zTrieBuilder)(&zbuilder);
          KILL(zoptionzIRPendingReceiptzK)(&zfirst);
          KILL(zoptionzIRPendingReceiptzK)(&zpending);
          KILL(zexception)(&z2zE794);
          goto end_block_exception_1528;
          /* unreachable after throw */
          KILL(zexception)(&z2zE794);
        }
        goto finish_match_1518;
      }
    case_1519: ;
    finish_match_1518: ;
      z3zE1037 = z3zE1038;
    }
  }
  sail_fixed_bytes_256 z2zE797;
  {
    sail_fixed_bytes_256 z2zE795;
    z2zE795 = zacc.zbloom;
    zz5listz8z5structz0zzLogEntryz9 z2zE796;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE796);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE796, zreceipt.zlogs);
    z2zE797 = zbloom_add_logs(z2zE795, z2zE796);
    KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE796);
  }
  struct zReceiptAccumulator z3zE1053;
  CREATE(zReceiptAccumulator)(&z3zE1053);
  z3zE1053.zbloom = z2zE797;
  COPY(zTrieBuilder)(&((&z3zE1053)->zbuilder), zbuilder);
  z3zE1053.zcount = znext_count;
  z3zE1053.zcumulative_gas_used = zcumulative;
  COPY(zoptionzIRPendingReceiptzK)(&((&z3zE1053)->zfirst), zfirst);
  COPY(zoptionzIRPendingReceiptzK)(&((&z3zE1053)->zpending), zpending);
  COPY(zReceiptAccumulator)((*(&z8zE611)), z3zE1053);
  KILL(zReceiptAccumulator)(&z3zE1053);
  KILL(zoptionzIRPendingReceiptzK)(&zpending);
  KILL(zoptionzIRPendingReceiptzK)(&zfirst);
  KILL(zTrieBuilder)(&zbuilder);
  KILL(zPendingReceipt)(&zcurrent);
end_function_1527: ;
  goto end_function_4017;
end_block_exception_1528: ;
  goto end_function_4017;
end_function_4017: ;
}

sail_fixed_bytes_32 zreceipt_accumulator_root(struct zReceiptAccumulator zacc)
{
  sail_fixed_bytes_32 z8zE612;
  struct zTrieBuilder zbuilder;
  CREATE(zTrieBuilder)(&zbuilder);
  COPY(zTrieBuilder)(&zbuilder, zacc.zbuilder);
  bool z2zE760;
  {
    uint64_t z2zE759;
    z2zE759 = zacc.zcount;
    z2zE760 = (z2zE759 == UINT64_C(0));
  }
  unit z3zE1014;
  if (z2zE760) {  z3zE1014 = UNIT;  } else {
    bool z2zE762;
    {
      uint64_t z2zE761;
      z2zE761 = zacc.zcount;
      z2zE762 = (z2zE761 == UINT64_C(1));
    }
    if (z2zE762) {
      struct zoptionzIRPendingReceiptzK z2zE763;
      CREATE(zoptionzIRPendingReceiptzK)(&z2zE763);
      COPY(zoptionzIRPendingReceiptzK)(&z2zE763, zacc.zfirst);
      unit z3zE1031;
      {
        if (z2zE763.kind != Kind_zSomezIRPendingReceiptzK) goto case_1515;
        struct zPendingReceipt zzzero;
        CREATE(zPendingReceipt)(&zzzero);
        COPY(zPendingReceipt)(&zzzero, z2zE763.variants.zSomezIRPendingReceiptzK);
        struct zoptionzIRTriePathzK z2zE764;
        CREATE(zoptionzIRTriePathzK)(&z2zE764);
        zNonezIRTriePathzK(&z2zE764, UNIT);
        {
          zreceipt_insert(&zbuilder, zbuilder, zzzero, z2zE764);
          if (have_exception) {
            KILL(zTrieBuilder)(&zbuilder);
            KILL(zoptionzIRPendingReceiptzK)(&z2zE763);
            KILL(zPendingReceipt)(&zzzero);
            KILL(zoptionzIRTriePathzK)(&z2zE764);
            goto end_block_exception_1517;
          }
        }
        z3zE1031 = UNIT;
        KILL(zoptionzIRTriePathzK)(&z2zE764);
        KILL(zPendingReceipt)(&zzzero);
        goto finish_match_1513;
      }
    case_1515: ;
      {
        /* complete */
        struct zexception z2zE765;
        CREATE(zexception)(&z2zE765);
        zInvalidBlock(&z2zE765, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE765);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/receipts.sail:291.26-291.62");
        KILL(zTrieBuilder)(&zbuilder);
        KILL(zoptionzIRPendingReceiptzK)(&z2zE763);
        KILL(zexception)(&z2zE765);
        goto end_block_exception_1517;
        /* unreachable after throw */
        KILL(zexception)(&z2zE765);
        goto finish_match_1513;
      }
    case_1514: ;
    finish_match_1513: ;
      z3zE1014 = z3zE1031;
      KILL(zoptionzIRPendingReceiptzK)(&z2zE763);
    } else {
      bool z2zE767;
      {
        uint64_t z2zE766;
        z2zE766 = zacc.zcount;
        z2zE767 = (!(UINT64_C(128) < z2zE766));
      }
      if (z2zE767) {
        struct zoptionzIRPendingReceiptzK z2zE768;
        CREATE(zoptionzIRPendingReceiptzK)(&z2zE768);
        COPY(zoptionzIRPendingReceiptzK)(&z2zE768, zacc.zpending);
        unit z3zE1022;
        {
          if (z2zE768.kind != Kind_zSomezIRPendingReceiptzK) goto case_1509;
          struct zPendingReceipt zprevious;
          CREATE(zPendingReceipt)(&zprevious);
          COPY(zPendingReceipt)(&zprevious, z2zE768.variants.zSomezIRPendingReceiptzK);
          struct zoptionzIRTriePathzK z2zE769;
          CREATE(zoptionzIRTriePathzK)(&z2zE769);
          {
            struct zTriePath z3zE1024;
            zz5vecz8z5bv8z9 z3zE1025;
            CREATE(zz5vecz8z5bv8z9)(&z3zE1025);
            internal_vector_init_zz5vecz8z5bv8z9(&z3zE1025, INT64_C(32));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(0), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(1), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(2), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(3), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(4), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(5), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(6), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(7), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(8), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(9), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(10), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(11), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(12), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(13), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(14), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(15), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(16), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(17), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(18), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(19), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(20), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(21), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(22), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(23), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(24), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(25), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(26), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(27), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(28), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(29), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(30), UINT64_C(0x00));
            internal_vector_update_zz5vecz8z5bv8z9(&z3zE1025, z3zE1025, INT64_C(31), UINT64_C(0x80));
            for (size_t z8zE1093 = 0; z8zE1093 < 32; ++z8zE1093) {
              z3zE1024.zdata.bytes[z8zE1093] = (uint8_t)(z3zE1025.data[z8zE1093] & UINT64_C(0xff));
            }
            KILL(zz5vecz8z5bv8z9)(&z3zE1025);
            z3zE1024.zlen = UINT64_C(2);
            zSomezIRTriePathzK(&z2zE769, z3zE1024);
          }
          {
            zreceipt_insert(&zbuilder, zbuilder, zprevious, z2zE769);
            if (have_exception) {
              KILL(zTrieBuilder)(&zbuilder);
              KILL(zoptionzIRPendingReceiptzK)(&z2zE768);
              KILL(zPendingReceipt)(&zprevious);
              KILL(zoptionzIRTriePathzK)(&z2zE769);
              goto end_block_exception_1517;
            }
          }
          z3zE1022 = UNIT;
          KILL(zoptionzIRTriePathzK)(&z2zE769);
          KILL(zPendingReceipt)(&zprevious);
          goto finish_match_1507;
        }
      case_1509: ;
        {
          /* complete */
          struct zexception z2zE770;
          CREATE(zexception)(&z2zE770);
          zInvalidBlock(&z2zE770, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE770);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/receipts.sail:297.30-297.66");
          KILL(zTrieBuilder)(&zbuilder);
          KILL(zoptionzIRPendingReceiptzK)(&z2zE768);
          KILL(zexception)(&z2zE770);
          goto end_block_exception_1517;
          /* unreachable after throw */
          KILL(zexception)(&z2zE770);
          goto finish_match_1507;
        }
      case_1508: ;
      finish_match_1507: ;
        unit z3zE1027;
        z3zE1027 = z3zE1022;
        KILL(zoptionzIRPendingReceiptzK)(&z2zE768);
        struct zoptionzIRPendingReceiptzK z2zE771;
        CREATE(zoptionzIRPendingReceiptzK)(&z2zE771);
        COPY(zoptionzIRPendingReceiptzK)(&z2zE771, zacc.zfirst);
        unit z3zE1028;
        {
          if (z2zE771.kind != Kind_zSomezIRPendingReceiptzK) goto case_1512;
          struct zPendingReceipt z3zE1034;
          CREATE(zPendingReceipt)(&z3zE1034);
          COPY(zPendingReceipt)(&z3zE1034, z2zE771.variants.zSomezIRPendingReceiptzK);
          struct zoptionzIRTriePathzK z2zE772;
          CREATE(zoptionzIRTriePathzK)(&z2zE772);
          zNonezIRTriePathzK(&z2zE772, UNIT);
          {
            zreceipt_insert(&zbuilder, zbuilder, z3zE1034, z2zE772);
            if (have_exception) {
              KILL(zTrieBuilder)(&zbuilder);
              KILL(zoptionzIRPendingReceiptzK)(&z2zE771);
              KILL(zPendingReceipt)(&z3zE1034);
              KILL(zoptionzIRTriePathzK)(&z2zE772);
              goto end_block_exception_1517;
            }
          }
          z3zE1028 = UNIT;
          KILL(zoptionzIRTriePathzK)(&z2zE772);
          KILL(zPendingReceipt)(&z3zE1034);
          goto finish_match_1510;
        }
      case_1512: ;
        {
          /* complete */
          struct zexception z2zE773;
          CREATE(zexception)(&z2zE773);
          zInvalidBlock(&z2zE773, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE773);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/receipts.sail:301.30-301.66");
          KILL(zTrieBuilder)(&zbuilder);
          KILL(zoptionzIRPendingReceiptzK)(&z2zE771);
          KILL(zexception)(&z2zE773);
          goto end_block_exception_1517;
          /* unreachable after throw */
          KILL(zexception)(&z2zE773);
          goto finish_match_1510;
        }
      case_1511: ;
      finish_match_1510: ;
        z3zE1014 = z3zE1028;
        KILL(zoptionzIRPendingReceiptzK)(&z2zE771);
      } else {
        struct zoptionzIRPendingReceiptzK z2zE774;
        CREATE(zoptionzIRPendingReceiptzK)(&z2zE774);
        COPY(zoptionzIRPendingReceiptzK)(&z2zE774, zacc.zpending);
        unit z3zE1015;
        {
          if (z2zE774.kind != Kind_zSomezIRPendingReceiptzK) goto case_1503;
          struct zPendingReceipt z3zE1035;
          CREATE(zPendingReceipt)(&z3zE1035);
          COPY(zPendingReceipt)(&z3zE1035, z2zE774.variants.zSomezIRPendingReceiptzK);
          struct zoptionzIRTriePathzK z2zE775;
          CREATE(zoptionzIRTriePathzK)(&z2zE775);
          zNonezIRTriePathzK(&z2zE775, UNIT);
          {
            zreceipt_insert(&zbuilder, zbuilder, z3zE1035, z2zE775);
            if (have_exception) {
              KILL(zTrieBuilder)(&zbuilder);
              KILL(zoptionzIRPendingReceiptzK)(&z2zE774);
              KILL(zPendingReceipt)(&z3zE1035);
              KILL(zoptionzIRTriePathzK)(&z2zE775);
              goto end_block_exception_1517;
            }
          }
          z3zE1015 = UNIT;
          KILL(zoptionzIRTriePathzK)(&z2zE775);
          KILL(zPendingReceipt)(&z3zE1035);
          goto finish_match_1501;
        }
      case_1503: ;
        {
          /* complete */
          struct zexception z2zE776;
          CREATE(zexception)(&z2zE776);
          zInvalidBlock(&z2zE776, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE776);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/receipts.sail:306.30-306.66");
          KILL(zTrieBuilder)(&zbuilder);
          KILL(zoptionzIRPendingReceiptzK)(&z2zE774);
          KILL(zexception)(&z2zE776);
          goto end_block_exception_1517;
          /* unreachable after throw */
          KILL(zexception)(&z2zE776);
          goto finish_match_1501;
        }
      case_1502: ;
      finish_match_1501: ;
        unit z3zE1018;
        z3zE1018 = z3zE1015;
        KILL(zoptionzIRPendingReceiptzK)(&z2zE774);
        struct zoptionzIRPendingReceiptzK z2zE777;
        CREATE(zoptionzIRPendingReceiptzK)(&z2zE777);
        COPY(zoptionzIRPendingReceiptzK)(&z2zE777, zacc.zfirst);
        unit z3zE1019;
        {
          if (z2zE777.kind != Kind_zSomezIRPendingReceiptzK) goto case_1506;
          struct zexception z2zE778;
          CREATE(zexception)(&z2zE778);
          zInvalidBlock(&z2zE778, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE778);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/receipts.sail:309.31-309.67");
          KILL(zTrieBuilder)(&zbuilder);
          KILL(zoptionzIRPendingReceiptzK)(&z2zE777);
          KILL(zexception)(&z2zE778);
          goto end_block_exception_1517;
          /* unreachable after throw */
          KILL(zexception)(&z2zE778);
          goto finish_match_1504;
        }
      case_1506: ;
        {
          /* complete */
          z3zE1019 = UNIT;
          goto finish_match_1504;
        }
      case_1505: ;
      finish_match_1504: ;
        z3zE1014 = z3zE1019;
        KILL(zoptionzIRPendingReceiptzK)(&z2zE777);
      }
    }
  }
  {
    z8zE612 = ztrie_builder_root(zbuilder);
    if (have_exception) {
      KILL(zTrieBuilder)(&zbuilder);
      goto end_block_exception_1517;
    }
  }
  KILL(zTrieBuilder)(&zbuilder);
end_function_1516: ;
  return z8zE612;
end_block_exception_1517: ;

  return fixed_bytes_32_zero();
}

