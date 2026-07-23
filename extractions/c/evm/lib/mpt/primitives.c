/* Generated from sail/lib/mpt/primitives.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zto_trie_depth(uint64_t zvalue)
{
  uint64_t z8zE483;
  bool z2zE1615;
  z2zE1615 = (!(zvalue < UINT64_C(64)));
  if (z2zE1615) {
    struct zexception z2zE1616;
    CREATE(zexception)(&z2zE1616);
    zInvalidBlock(&z2zE1616, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1616);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:45.8-45.44");
    KILL(zexception)(&z2zE1616);
    goto end_block_exception_2019;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1616);
  } else {  z8zE483 = zvalue;  }
end_function_2018: ;
  return z8zE483;
end_block_exception_2019: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath zpath_append_nibble(struct zTriePath zpath, uint64_t zvalue)
{
  struct zTriePath z8zE484;
  uint64_t zlength;
  zlength = zpath_len(zpath);
  bool z2zE1605;
  z2zE1605 = (!(zlength < UINT64_C(64)));
  unit z3zE1570;
  if (z2zE1605) {
    struct zexception z2zE1606;
    CREATE(zexception)(&z2zE1606);
    zInvalidBlock(&z2zE1606, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1606);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:88.8-88.44");
    KILL(zexception)(&z2zE1606);
    goto end_block_exception_2017;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1606);
  } else {  z3zE1570 = UNIT;  }
  sail_fixed_bytes_32 zoriginal;
  zoriginal = zpath.zdata;
  sail_fixed_bytes_32 zbytes;
  zbytes = zoriginal;
  uint64_t zbyte_index;
  zbyte_index = zpath_byte_index(zlength);
  bool z2zE1608;
  {
    uint64_t z2zE1607;
    {    z2zE1607 = (zlength % UINT64_C(2));
    }
    z2zE1608 = (z2zE1607 == UINT64_C(0));
  }
  unit z3zE1571;
  if (z2zE1608) {
    uint64_t z2zE1609;
    z2zE1609 = (zvalue << 4) | UINT64_C(0x0);
    zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, zbyte_index, z2zE1609);
    z3zE1571 = UNIT;
  } else {
    uint64_t z2zE1612;
    {
      uint64_t z2zE1611;
      {
        uint64_t z2zE1610;
        z2zE1610 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
        z2zE1611 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE1610 >> UINT64_C(4)));
      }
      z2zE1612 = (z2zE1611 << 4) | zvalue;
    }
    zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, zbyte_index, z2zE1612);
    z3zE1571 = UNIT;
  }
  sail_fixed_bytes_32 z2zE1613;
  z2zE1613 = zB256(zbytes);
  uint64_t z2zE1614;
  {
    z2zE1614 = ztrie_path_len_increment(zlength);
    if (have_exception) {  goto end_block_exception_2017;  }
  }
  z8zE484 = zpath_new(z2zE1613, z2zE1614);
end_function_2016: ;
  return z8zE484;
end_block_exception_2017: ;
  struct zTriePath z8zE998 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE998;
}

struct zTriePath zpath_append_byte(struct zTriePath zpath, uint64_t zvalue)
{
  struct zTriePath z8zE485;
  struct zTriePath z2zE1603;
  {
    uint64_t z2zE1602;
    z2zE1602 = (safe_rshift(UINT64_MAX, 64 - 4) & (zvalue >> UINT64_C(4)));
    {
      z2zE1603 = zpath_append_nibble(zpath, z2zE1602);
      if (have_exception) {  goto end_block_exception_2015;  }
    }
  }
  uint64_t z2zE1604;
  z2zE1604 = (safe_rshift(UINT64_MAX, 64 - 4) & (zvalue >> UINT64_C(0)));
  {
    z8zE485 = zpath_append_nibble(z2zE1603, z2zE1604);
    if (have_exception) {  goto end_block_exception_2015;  }
  }
end_function_2014: ;
  return z8zE485;
end_block_exception_2015: ;
  struct zTriePath z8zE999 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE999;
}

struct zTriePath zpath_single(uint64_t zn)
{
  struct zTriePath z8zE486;
  struct zTriePath z3zE1568;
  zz5vecz8z5bv8z9 z3zE1569;
  CREATE(zz5vecz8z5bv8z9)(&z3zE1569);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE1569, INT64_C(32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(0), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(19), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(20), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(21), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(22), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(23), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(24), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(25), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(26), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(27), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(28), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(29), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(30), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1569, z3zE1569, INT64_C(31), UINT64_C(0x00));
  for (size_t z8zE1000 = 0; z8zE1000 < 32; ++z8zE1000) {
    z3zE1568.zdata.bytes[z8zE1000] = (uint8_t)(z3zE1569.data[z8zE1000] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE1569);
  z3zE1568.zlen = UINT64_C(0);
  {
    z8zE486 = zpath_append_nibble(z3zE1568, zn);
    if (have_exception) {  goto end_block_exception_2013;  }
  }
end_function_2012: ;
  return z8zE486;
end_block_exception_2013: ;
  struct zTriePath z8zE1001 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE1001;
}

struct zTriePath zpath_concat(struct zTriePath za, struct zTriePath zb)
{
  struct zTriePath z8zE487;
  uint64_t zalen;
  zalen = zpath_len(za);
  uint64_t zblen;
  zblen = zpath_len(zb);
  uint64_t zcombined;
  {    zcombined = (zalen + zblen);
  }
  bool z2zE1598;
  z2zE1598 = (!(UINT64_C(64) < zcombined));
  if (z2zE1598) {
    struct zTriePath zresult;
    zresult = za;
    uint64_t zindex;
    zindex = UINT64_C(0);
    int64_t z3zE1562;
    {    z3zE1562 = (int64_t)(UINT64_C(0));
    }
    int64_t z3zE1563;
    {    z3zE1563 = (int64_t)(UINT64_C(63));
    }
    int64_t z3zE1564;
    {    z3zE1564 = (int64_t)(UINT64_C(1));
    }
    {
      int64_t z_step;
      z_step = z3zE1562;
      unit z3zE1565;
    for_start_2008: ;
      {
        if ((z3zE1563 < z_step)) goto for_end_2009;
        bool z2zE1599;
        z2zE1599 = (zindex < zblen);
        if (z2zE1599) {
          uint64_t z2zE1600;
          z2zE1600 = zpath_nibble(zb, zindex);
          {
            zresult = zpath_append_nibble(zresult, z2zE1600);
            if (have_exception) {  goto end_block_exception_2011;  }
          }
          unit z3zE1566;
          z3zE1566 = UNIT;
          {
            zindex = ztrie_path_len_increment(zindex);
            if (have_exception) {  goto end_block_exception_2011;  }
          }
          z3zE1565 = UNIT;
        } else {  z3zE1565 = UNIT;  }
        z_step = (z_step + z3zE1564);
        goto for_start_2008;
      }
    for_end_2009: ;
    }
    unit z3zE1567;
    z3zE1567 = UNIT;
    z8zE487 = zresult;
  } else {
    struct zexception z2zE1601;
    CREATE(zexception)(&z2zE1601);
    zInvalidBlock(&z2zE1601, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1601);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:124.8-124.44");
    KILL(zexception)(&z2zE1601);
    goto end_block_exception_2011;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1601);
  }
end_function_2010: ;
  return z8zE487;
end_block_exception_2011: ;
  struct zTriePath z8zE1002 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE1002;
}

struct zTriePath zpath_take(struct zTriePath zpath, uint64_t zn)
{
  struct zTriePath z8zE488;
  bool z2zE1593;
  z2zE1593 = (zn == UINT64_C(0));
  if (z2zE1593) {
    struct zTriePath z3zE1560;
    zz5vecz8z5bv8z9 z3zE1561;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1561);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1561, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1561, z3zE1561, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE1004 = 0; z8zE1004 < 32; ++z8zE1004) {
      z3zE1560.zdata.bytes[z8zE1004] = (uint8_t)(z3zE1561.data[z8zE1004] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1561);
    z3zE1560.zlen = UINT64_C(0);
    z8zE488 = z3zE1560;
  } else {
    bool z2zE1595;
    {
      uint64_t z2zE1594;
      z2zE1594 = zpath_len(zpath);
      z2zE1595 = (!(zn < z2zE1594));
    }
    if (z2zE1595) {  z8zE488 = zpath;  } else {
      struct zTriePath zresult;
      {
        struct zTriePath z3zE1552;
        zz5vecz8z5bv8z9 z3zE1553;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1553);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1553, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1553, z3zE1553, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE1003 = 0; z8zE1003 < 32; ++z8zE1003) {
          z3zE1552.zdata.bytes[z8zE1003] = (uint8_t)(z3zE1553.data[z8zE1003] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1553);
        z3zE1552.zlen = UINT64_C(0);
        zresult = z3zE1552;
      }
      uint64_t zindex;
      zindex = UINT64_C(0);
      int64_t z3zE1554;
      {    z3zE1554 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1555;
      {    z3zE1555 = (int64_t)(UINT64_C(63));
      }
      int64_t z3zE1556;
      {    z3zE1556 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_step;
        z_step = z3zE1554;
        unit z3zE1557;
      for_start_2004: ;
        {
          if ((z3zE1555 < z_step)) goto for_end_2005;
          bool z2zE1596;
          z2zE1596 = (zindex < zn);
          if (z2zE1596) {
            uint64_t z2zE1597;
            z2zE1597 = zpath_nibble(zpath, zindex);
            {
              zresult = zpath_append_nibble(zresult, z2zE1597);
              if (have_exception) {  goto end_block_exception_2007;  }
            }
            unit z3zE1558;
            z3zE1558 = UNIT;
            {
              zindex = ztrie_path_len_increment(zindex);
              if (have_exception) {  goto end_block_exception_2007;  }
            }
            z3zE1557 = UNIT;
          } else {  z3zE1557 = UNIT;  }
          z_step = (z_step + z3zE1556);
          goto for_start_2004;
        }
      for_end_2005: ;
      }
      unit z3zE1559;
      z3zE1559 = UNIT;
      z8zE488 = zresult;
    }
  }
end_function_2006: ;
  return z8zE488;
end_block_exception_2007: ;
  struct zTriePath z8zE1005 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE1005;
}

struct zTriePath zpath_drop(struct zTriePath zpath, uint64_t zn)
{
  struct zTriePath z8zE489;
  uint64_t zlength;
  zlength = zpath_len(zpath);
  bool z2zE1587;
  z2zE1587 = (!(zn < zlength));
  if (z2zE1587) {
    struct zTriePath z3zE1550;
    zz5vecz8z5bv8z9 z3zE1551;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1551);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1551, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1551, z3zE1551, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE1007 = 0; z8zE1007 < 32; ++z8zE1007) {
      z3zE1550.zdata.bytes[z8zE1007] = (uint8_t)(z3zE1551.data[z8zE1007] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1551);
    z3zE1550.zlen = UINT64_C(0);
    z8zE489 = z3zE1550;
  } else {
    bool z2zE1588;
    z2zE1588 = (zn == UINT64_C(0));
    if (z2zE1588) {  z8zE489 = zpath;  } else {
      uint64_t zremain;
      {    zremain = (zlength - zn);
      }
      struct zTriePath zresult;
      {
        struct zTriePath z3zE1540;
        zz5vecz8z5bv8z9 z3zE1541;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1541);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1541, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1541, z3zE1541, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE1006 = 0; z8zE1006 < 32; ++z8zE1006) {
          z3zE1540.zdata.bytes[z8zE1006] = (uint8_t)(z3zE1541.data[z8zE1006] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1541);
        z3zE1540.zlen = UINT64_C(0);
        zresult = z3zE1540;
      }
      uint64_t zoffset;
      zoffset = UINT64_C(0);
      int64_t z3zE1542;
      {    z3zE1542 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1543;
      {    z3zE1543 = (int64_t)(UINT64_C(63));
      }
      int64_t z3zE1544;
      {    z3zE1544 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_step;
        z_step = z3zE1542;
        unit z3zE1545;
      for_start_2000: ;
        {
          if ((z3zE1543 < z_step)) goto for_end_2001;
          bool z2zE1589;
          z2zE1589 = (zoffset < zremain);
          if (z2zE1589) {
            uint64_t zcandidate;
            {    zcandidate = (zn + zoffset);
            }
            uint64_t zsource_index;
            {
              bool z2zE1592;
              {
                bool z2zE1591;
                z2zE1591 = (!(zcandidate < UINT64_C(0)));
                bool z3zE1546;
                if (z2zE1591) {  z3zE1546 = (!(UINT64_C(64) < zcandidate));  } else {  z3zE1546 = false;  }
                z2zE1592 = z3zE1546;
              }
              if (z2zE1592) {  zsource_index = zcandidate;  } else {
                unit z3zE1547;
                z3zE1547 = sail_assert(false, "sail/lib/mpt/primitives.sail:166.40-166.41");
                sail_match_failure("path_drop");
                /* unreachable after exit */
              }
            }
            uint64_t z2zE1590;
            z2zE1590 = zpath_nibble(zpath, zsource_index);
            {
              zresult = zpath_append_nibble(zresult, z2zE1590);
              if (have_exception) {  goto end_block_exception_2003;  }
            }
            unit z3zE1548;
            z3zE1548 = UNIT;
            {
              zoffset = ztrie_path_len_increment(zoffset);
              if (have_exception) {  goto end_block_exception_2003;  }
            }
            z3zE1545 = UNIT;
          } else {  z3zE1545 = UNIT;  }
          z_step = (z_step + z3zE1544);
          goto for_start_2000;
        }
      for_end_2001: ;
      }
      unit z3zE1549;
      z3zE1549 = UNIT;
      z8zE489 = zresult;
    }
  }
end_function_2002: ;
  return z8zE489;
end_block_exception_2003: ;
  struct zTriePath z8zE1008 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE1008;
}

bool zpath_eq(struct zTriePath za, struct zTriePath zb)
{
  bool z8zE490;
  bool z2zE1586;
  {
    uint64_t z2zE1582;
    z2zE1582 = za.zlen;
    uint64_t z2zE1583;
    z2zE1583 = zb.zlen;
    z2zE1586 = (z2zE1582 == z2zE1583);
  }
  bool z3zE1539;
  if (z2zE1586) {
    sail_fixed_bytes_32 z2zE1584;
    z2zE1584 = za.zdata;
    sail_fixed_bytes_32 z2zE1585;
    z2zE1585 = zb.zdata;
    z3zE1539 = eq_fixed_bytes_32(z2zE1584, z2zE1585);
  } else {  z3zE1539 = false;  }
  z8zE490 = z3zE1539;
end_function_1998: ;
  return z8zE490;
end_block_exception_1999: ;

  return false;
}

bool zpath_lt(struct zTriePath za, struct zTriePath zb)
{
  bool z8zE491;
  bool z2zE1577;
  {
    sail_fixed_bytes_32 z2zE1575;
    z2zE1575 = za.zdata;
    sail_fixed_bytes_32 z2zE1576;
    z2zE1576 = zb.zdata;
    z2zE1577 = eq_fixed_bytes_32(z2zE1575, z2zE1576);
  }
  if (z2zE1577) {
    uint64_t z2zE1578;
    z2zE1578 = zpath_len(za);
    uint64_t z2zE1579;
    z2zE1579 = zpath_len(zb);
    z8zE491 = (z2zE1578 < z2zE1579);
  } else {
    sail_fixed_bytes_32 z2zE1580;
    z2zE1580 = za.zdata;
    sail_fixed_bytes_32 z2zE1581;
    z2zE1581 = zb.zdata;
    z8zE491 = zhash_lt(z2zE1580, z2zE1581);
  }
end_function_1996: ;
  return z8zE491;
end_block_exception_1997: ;

  return false;
}

bool zpath_prefix_of(struct zTriePath zprefix, struct zTriePath zpath)
{
  bool z8zE492;
  bool z2zE1572;
  {
    uint64_t z2zE1570;
    z2zE1570 = zpath_len(zpath);
    uint64_t z2zE1571;
    z2zE1571 = zpath_len(zprefix);
    z2zE1572 = (z2zE1570 < z2zE1571);
  }
  if (z2zE1572) {  z8zE492 = false;  } else {
    struct zTriePath z2zE1574;
    {
      uint64_t z2zE1573;
      z2zE1573 = zpath_len(zprefix);
      {
        z2zE1574 = zpath_take(zpath, z2zE1573);
        if (have_exception) {  goto end_block_exception_1995;  }
      }
    }
    z8zE492 = zpath_eq(zprefix, z2zE1574);
  }
end_function_1994: ;
  return z8zE492;
end_block_exception_1995: ;

  return false;
}

uint64_t zcommon_prefix_from(struct zTriePath za, struct zTriePath zb, uint64_t zstart)
{
  uint64_t z8zE493;
  uint64_t zalen;
  zalen = zpath_len(za);
  uint64_t zblen;
  zblen = zpath_len(zb);
  uint64_t zstop;
  {
    bool z2zE1569;
    z2zE1569 = (zalen < zblen);
    if (z2zE1569) {  zstop = zalen;  } else {  zstop = zblen;  }
  }
  uint64_t zindex;
  zindex = zstart;
  uint64_t zcount;
  zcount = UINT64_C(0);
  bool zmatching;
  zmatching = true;
  int64_t z3zE1532;
  {    z3zE1532 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1533;
  {    z3zE1533 = (int64_t)(UINT64_C(63));
  }
  int64_t z3zE1534;
  {    z3zE1534 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z_step;
    z_step = z3zE1532;
    unit z3zE1536;
  for_start_1990: ;
    {
      if ((z3zE1533 < z_step)) goto for_end_1991;
      bool z2zE1565;
      {
        bool z3zE1535;
        if (zmatching) {  z3zE1535 = (zindex < zstop);  } else {  z3zE1535 = false;  }
        z2zE1565 = z3zE1535;
      }
      if (z2zE1565) {
        bool z2zE1568;
        {
          uint64_t z2zE1566;
          z2zE1566 = zpath_nibble(za, zindex);
          uint64_t z2zE1567;
          z2zE1567 = zpath_nibble(zb, zindex);
          z2zE1568 = (z2zE1566 == z2zE1567);
        }
        if (z2zE1568) {
          {
            zcount = ztrie_path_len_increment(zcount);
            if (have_exception) {  goto end_block_exception_1993;  }
          }
          unit z3zE1537;
          z3zE1537 = UNIT;
          {
            zindex = ztrie_path_len_increment(zindex);
            if (have_exception) {  goto end_block_exception_1993;  }
          }
          z3zE1536 = UNIT;
        } else {
          zmatching = false;
          z3zE1536 = UNIT;
        }
      } else {  z3zE1536 = UNIT;  }
      z_step = (z_step + z3zE1534);
      goto for_start_1990;
    }
  for_end_1991: ;
  }
  unit z3zE1538;
  z3zE1538 = UNIT;
  z8zE493 = zcount;
end_function_1992: ;
  return z8zE493;
end_block_exception_1993: ;

  return UINT64_C(0xdeadc0de);
}

void zhex_prefix_pairs(zz5listz8z5bv8z9 *z8zE494, struct zTriePath zpath, uint64_t zindex)
{
  bool z2zE1559;
  {
    uint64_t z2zE1558;
    z2zE1558 = zpath_len(zpath);
    z2zE1559 = (!(zindex < z2zE1558));
  }
  if (z2zE1559) {
    zz5listz8z5bv8z9 z3zE1531;
    CREATE(zz5listz8z5bv8z9)(&z3zE1531);
    COPY(zz5listz8z5bv8z9)((*(&z8zE494)), z3zE1531);
    KILL(zz5listz8z5bv8z9)(&z3zE1531);
  } else {
    uint64_t znext;
    {    znext = (zindex + UINT64_C(1));
    }
    uint64_t z2zE1563;
    {
      uint64_t z2zE1560;
      z2zE1560 = zpath_nibble(zpath, zindex);
      uint64_t z2zE1561;
      z2zE1561 = zpath_nibble(zpath, znext);
      z2zE1563 = (z2zE1560 << 4) | z2zE1561;
    }
    zz5listz8z5bv8z9 z2zE1564;
    CREATE(zz5listz8z5bv8z9)(&z2zE1564);
    {
      uint64_t z2zE1562;
      {    z2zE1562 = (znext + UINT64_C(1));
      }
      zhex_prefix_pairs(&z2zE1564, zpath, z2zE1562);
    }
    zconsz3z5bv8((*(&z8zE494)), z2zE1563, z2zE1564);
    KILL(zz5listz8z5bv8z9)(&z2zE1564);
  }
end_function_1988: ;
  goto end_function_4060;
end_block_exception_1989: ;
  goto end_function_4060;
end_function_4060: ;
}

void zhex_prefix_compact(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE495, struct zTriePath zpath, bool zis_leaf)
{
  uint64_t zlength;
  zlength = zpath_len(zpath);
  uint64_t zpacked_pair_count;
  {    zpacked_pair_count = (zlength / UINT64_C(2));
  }
  bool zodd;
  {
    uint64_t z2zE1557;
    {    z2zE1557 = (zlength % UINT64_C(2));
    }
    zodd = (z2zE1557 != UINT64_C(0));
  }
  uint64_t zflag;
  if (zis_leaf) {  zflag = UINT64_C(0x2);  } else {  zflag = UINT64_C(0x0);  }
  uint64_t zfirst;
  if (zodd) {
    uint64_t z2zE1555;
    z2zE1555 = (zflag | UINT64_C(0x1));
    uint64_t z2zE1556;
    z2zE1556 = zpath_nibble(zpath, UINT64_C(0));
    zfirst = (z2zE1555 << 4) | z2zE1556;
  } else {  zfirst = (zflag << 4) | UINT64_C(0x0);  }
  uint64_t zfirst_path_index;
  if (zodd) {  zfirst_path_index = UINT64_C(1);  } else {  zfirst_path_index = UINT64_C(0);  }
  uint64_t zencoded_len;
  {    zencoded_len = (UINT64_C(1) + zpacked_pair_count);
  }
  zz5listz8z5bv8z9 z2zE1554;
  CREATE(zz5listz8z5bv8z9)(&z2zE1554);
  {
    zz5listz8z5bv8z9 z2zE1553;
    CREATE(zz5listz8z5bv8z9)(&z2zE1553);
    zhex_prefix_pairs(&z2zE1553, zpath, zfirst_path_index);
    zconsz3z5bv8(&z2zE1554, zfirst, z2zE1553);
    KILL(zz5listz8z5bv8z9)(&z2zE1553);
  }
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE1530;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE1530);
  COPY(zz5listz8z5bv8z9)(&((&z3zE1530)->ztup0), z2zE1554);
  z3zE1530.ztup1 = zencoded_len;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE495)), z3zE1530);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE1530);
  KILL(zz5listz8z5bv8z9)(&z2zE1554);
end_function_1986: ;
  goto end_function_4059;
end_block_exception_1987: ;
  goto end_function_4059;
end_function_4059: ;
}

