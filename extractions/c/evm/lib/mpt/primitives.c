/* Generated from sail/lib/mpt/primitives.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zto_trie_depth(uint64_t zvalue)
{
  uint64_t z8zE480;
  bool z2zE1592;
  z2zE1592 = (!(zvalue < UINT64_C(64)));
  if (z2zE1592) {
    struct zexception z2zE1593;
    CREATE(zexception)(&z2zE1593);
    zInvalidBlock(&z2zE1593, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1593);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:45.8-45.44");
    KILL(zexception)(&z2zE1593);
    goto end_block_exception_2015;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1593);
  } else {  z8zE480 = zvalue;  }
end_function_2014: ;
  return z8zE480;
end_block_exception_2015: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath zpath_append_nibble(struct zTriePath zpath, uint64_t zvalue)
{
  struct zTriePath z8zE481;
  uint64_t zlength;
  zlength = zpath_len(zpath);
  bool z2zE1582;
  z2zE1582 = (!(zlength < UINT64_C(64)));
  unit z3zE1574;
  if (z2zE1582) {
    struct zexception z2zE1583;
    CREATE(zexception)(&z2zE1583);
    zInvalidBlock(&z2zE1583, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1583);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:88.8-88.44");
    KILL(zexception)(&z2zE1583);
    goto end_block_exception_2013;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1583);
  } else {  z3zE1574 = UNIT;  }
  sail_fixed_bytes_32 zoriginal;
  zoriginal = zpath.zdata;
  sail_fixed_bytes_32 zbytes;
  zbytes = zoriginal;
  uint64_t zbyte_index;
  zbyte_index = zpath_byte_index(zlength);
  bool z2zE1585;
  {
    uint64_t z2zE1584;
    {    z2zE1584 = (zlength % UINT64_C(2));
    }
    z2zE1585 = (z2zE1584 == UINT64_C(0));
  }
  unit z3zE1575;
  if (z2zE1585) {
    uint64_t z2zE1586;
    z2zE1586 = (zvalue << 4) | UINT64_C(0x0);
    zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, zbyte_index, z2zE1586);
    z3zE1575 = UNIT;
  } else {
    uint64_t z2zE1589;
    {
      uint64_t z2zE1588;
      {
        uint64_t z2zE1587;
        z2zE1587 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
        z2zE1588 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE1587 >> UINT64_C(4)));
      }
      z2zE1589 = (z2zE1588 << 4) | zvalue;
    }
    zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, zbyte_index, z2zE1589);
    z3zE1575 = UNIT;
  }
  sail_fixed_bytes_32 z2zE1590;
  z2zE1590 = zB256(zbytes);
  uint64_t z2zE1591;
  {
    z2zE1591 = ztrie_path_len_increment(zlength);
    if (have_exception) {  goto end_block_exception_2013;  }
  }
  z8zE481 = zpath_new(z2zE1590, z2zE1591);
end_function_2012: ;
  return z8zE481;
end_block_exception_2013: ;
  struct zTriePath z8zE986 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE986;
}

struct zTriePath zpath_append_byte(struct zTriePath zpath, uint64_t zvalue)
{
  struct zTriePath z8zE482;
  struct zTriePath z2zE1580;
  {
    uint64_t z2zE1579;
    z2zE1579 = (safe_rshift(UINT64_MAX, 64 - 4) & (zvalue >> UINT64_C(4)));
    {
      z2zE1580 = zpath_append_nibble(zpath, z2zE1579);
      if (have_exception) {  goto end_block_exception_2011;  }
    }
  }
  uint64_t z2zE1581;
  z2zE1581 = (safe_rshift(UINT64_MAX, 64 - 4) & (zvalue >> UINT64_C(0)));
  {
    z8zE482 = zpath_append_nibble(z2zE1580, z2zE1581);
    if (have_exception) {  goto end_block_exception_2011;  }
  }
end_function_2010: ;
  return z8zE482;
end_block_exception_2011: ;
  struct zTriePath z8zE987 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE987;
}

struct zTriePath zpath_single(uint64_t zn)
{
  struct zTriePath z8zE483;
  struct zTriePath z3zE1572;
  zz5vecz8z5bv8z9 z3zE1573;
  CREATE(zz5vecz8z5bv8z9)(&z3zE1573);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE1573, INT64_C(32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(0), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(19), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(20), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(21), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(22), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(23), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(24), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(25), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(26), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(27), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(28), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(29), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(30), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1573, z3zE1573, INT64_C(31), UINT64_C(0x00));
  for (size_t z8zE988 = 0; z8zE988 < 32; ++z8zE988) {
    z3zE1572.zdata.bytes[z8zE988] = (uint8_t)(z3zE1573.data[z8zE988] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE1573);
  z3zE1572.zlen = UINT64_C(0);
  {
    z8zE483 = zpath_append_nibble(z3zE1572, zn);
    if (have_exception) {  goto end_block_exception_2009;  }
  }
end_function_2008: ;
  return z8zE483;
end_block_exception_2009: ;
  struct zTriePath z8zE989 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE989;
}

struct zTriePath zpath_concat(struct zTriePath za, struct zTriePath zb)
{
  struct zTriePath z8zE484;
  uint64_t zalen;
  zalen = zpath_len(za);
  uint64_t zblen;
  zblen = zpath_len(zb);
  uint64_t zcombined;
  {    zcombined = (zalen + zblen);
  }
  bool z2zE1575;
  z2zE1575 = (!(UINT64_C(64) < zcombined));
  if (z2zE1575) {
    struct zTriePath zresult;
    zresult = za;
    uint64_t zindex;
    zindex = UINT64_C(0);
    int64_t z3zE1566;
    {    z3zE1566 = (int64_t)(UINT64_C(0));
    }
    int64_t z3zE1567;
    {    z3zE1567 = (int64_t)(UINT64_C(63));
    }
    int64_t z3zE1568;
    {    z3zE1568 = (int64_t)(UINT64_C(1));
    }
    {
      int64_t z_step;
      z_step = z3zE1566;
      unit z3zE1569;
    for_start_2004: ;
      {
        if ((z3zE1567 < z_step)) goto for_end_2005;
        bool z2zE1576;
        z2zE1576 = (zindex < zblen);
        if (z2zE1576) {
          uint64_t z2zE1577;
          z2zE1577 = zpath_nibble(zb, zindex);
          {
            zresult = zpath_append_nibble(zresult, z2zE1577);
            if (have_exception) {  goto end_block_exception_2007;  }
          }
          unit z3zE1570;
          z3zE1570 = UNIT;
          {
            zindex = ztrie_path_len_increment(zindex);
            if (have_exception) {  goto end_block_exception_2007;  }
          }
          z3zE1569 = UNIT;
        } else {  z3zE1569 = UNIT;  }
        z_step = (z_step + z3zE1568);
        goto for_start_2004;
      }
    for_end_2005: ;
    }
    unit z3zE1571;
    z3zE1571 = UNIT;
    z8zE484 = zresult;
  } else {
    struct zexception z2zE1578;
    CREATE(zexception)(&z2zE1578);
    zInvalidBlock(&z2zE1578, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1578);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:124.8-124.44");
    KILL(zexception)(&z2zE1578);
    goto end_block_exception_2007;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1578);
  }
end_function_2006: ;
  return z8zE484;
end_block_exception_2007: ;
  struct zTriePath z8zE990 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE990;
}

struct zTriePath zpath_take(struct zTriePath zpath, uint64_t zn)
{
  struct zTriePath z8zE485;
  bool z2zE1570;
  z2zE1570 = (zn == UINT64_C(0));
  if (z2zE1570) {
    struct zTriePath z3zE1564;
    zz5vecz8z5bv8z9 z3zE1565;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1565);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1565, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1565, z3zE1565, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE992 = 0; z8zE992 < 32; ++z8zE992) {
      z3zE1564.zdata.bytes[z8zE992] = (uint8_t)(z3zE1565.data[z8zE992] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1565);
    z3zE1564.zlen = UINT64_C(0);
    z8zE485 = z3zE1564;
  } else {
    bool z2zE1572;
    {
      uint64_t z2zE1571;
      z2zE1571 = zpath_len(zpath);
      z2zE1572 = (!(zn < z2zE1571));
    }
    if (z2zE1572) {  z8zE485 = zpath;  } else {
      struct zTriePath zresult;
      {
        struct zTriePath z3zE1556;
        zz5vecz8z5bv8z9 z3zE1557;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1557);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1557, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1557, z3zE1557, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE991 = 0; z8zE991 < 32; ++z8zE991) {
          z3zE1556.zdata.bytes[z8zE991] = (uint8_t)(z3zE1557.data[z8zE991] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1557);
        z3zE1556.zlen = UINT64_C(0);
        zresult = z3zE1556;
      }
      uint64_t zindex;
      zindex = UINT64_C(0);
      int64_t z3zE1558;
      {    z3zE1558 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1559;
      {    z3zE1559 = (int64_t)(UINT64_C(63));
      }
      int64_t z3zE1560;
      {    z3zE1560 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_step;
        z_step = z3zE1558;
        unit z3zE1561;
      for_start_2000: ;
        {
          if ((z3zE1559 < z_step)) goto for_end_2001;
          bool z2zE1573;
          z2zE1573 = (zindex < zn);
          if (z2zE1573) {
            uint64_t z2zE1574;
            z2zE1574 = zpath_nibble(zpath, zindex);
            {
              zresult = zpath_append_nibble(zresult, z2zE1574);
              if (have_exception) {  goto end_block_exception_2003;  }
            }
            unit z3zE1562;
            z3zE1562 = UNIT;
            {
              zindex = ztrie_path_len_increment(zindex);
              if (have_exception) {  goto end_block_exception_2003;  }
            }
            z3zE1561 = UNIT;
          } else {  z3zE1561 = UNIT;  }
          z_step = (z_step + z3zE1560);
          goto for_start_2000;
        }
      for_end_2001: ;
      }
      unit z3zE1563;
      z3zE1563 = UNIT;
      z8zE485 = zresult;
    }
  }
end_function_2002: ;
  return z8zE485;
end_block_exception_2003: ;
  struct zTriePath z8zE993 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE993;
}

struct zTriePath zpath_drop(struct zTriePath zpath, uint64_t zn)
{
  struct zTriePath z8zE486;
  uint64_t zlength;
  zlength = zpath_len(zpath);
  bool z2zE1564;
  z2zE1564 = (!(zn < zlength));
  if (z2zE1564) {
    struct zTriePath z3zE1554;
    zz5vecz8z5bv8z9 z3zE1555;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1555);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1555, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1555, z3zE1555, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE995 = 0; z8zE995 < 32; ++z8zE995) {
      z3zE1554.zdata.bytes[z8zE995] = (uint8_t)(z3zE1555.data[z8zE995] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1555);
    z3zE1554.zlen = UINT64_C(0);
    z8zE486 = z3zE1554;
  } else {
    bool z2zE1565;
    z2zE1565 = (zn == UINT64_C(0));
    if (z2zE1565) {  z8zE486 = zpath;  } else {
      uint64_t zremain;
      {    zremain = (zlength - zn);
      }
      struct zTriePath zresult;
      {
        struct zTriePath z3zE1544;
        zz5vecz8z5bv8z9 z3zE1545;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1545);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1545, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1545, z3zE1545, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE994 = 0; z8zE994 < 32; ++z8zE994) {
          z3zE1544.zdata.bytes[z8zE994] = (uint8_t)(z3zE1545.data[z8zE994] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1545);
        z3zE1544.zlen = UINT64_C(0);
        zresult = z3zE1544;
      }
      uint64_t zoffset;
      zoffset = UINT64_C(0);
      int64_t z3zE1546;
      {    z3zE1546 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1547;
      {    z3zE1547 = (int64_t)(UINT64_C(63));
      }
      int64_t z3zE1548;
      {    z3zE1548 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_step;
        z_step = z3zE1546;
        unit z3zE1549;
      for_start_1996: ;
        {
          if ((z3zE1547 < z_step)) goto for_end_1997;
          bool z2zE1566;
          z2zE1566 = (zoffset < zremain);
          if (z2zE1566) {
            uint64_t zcandidate;
            {    zcandidate = (zn + zoffset);
            }
            uint64_t zsource_index;
            {
              bool z2zE1569;
              {
                bool z2zE1568;
                z2zE1568 = (!(zcandidate < UINT64_C(0)));
                bool z3zE1550;
                if (z2zE1568) {  z3zE1550 = (!(UINT64_C(64) < zcandidate));  } else {  z3zE1550 = false;  }
                z2zE1569 = z3zE1550;
              }
              if (z2zE1569) {  zsource_index = zcandidate;  } else {
                unit z3zE1551;
                z3zE1551 = sail_assert(false, "sail/lib/mpt/primitives.sail:166.40-166.41");
                sail_match_failure("path_drop");
                /* unreachable after exit */
              }
            }
            uint64_t z2zE1567;
            z2zE1567 = zpath_nibble(zpath, zsource_index);
            {
              zresult = zpath_append_nibble(zresult, z2zE1567);
              if (have_exception) {  goto end_block_exception_1999;  }
            }
            unit z3zE1552;
            z3zE1552 = UNIT;
            {
              zoffset = ztrie_path_len_increment(zoffset);
              if (have_exception) {  goto end_block_exception_1999;  }
            }
            z3zE1549 = UNIT;
          } else {  z3zE1549 = UNIT;  }
          z_step = (z_step + z3zE1548);
          goto for_start_1996;
        }
      for_end_1997: ;
      }
      unit z3zE1553;
      z3zE1553 = UNIT;
      z8zE486 = zresult;
    }
  }
end_function_1998: ;
  return z8zE486;
end_block_exception_1999: ;
  struct zTriePath z8zE996 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE996;
}

bool zpath_eq(struct zTriePath za, struct zTriePath zb)
{
  bool z8zE487;
  bool z2zE1563;
  {
    uint64_t z2zE1559;
    z2zE1559 = za.zlen;
    uint64_t z2zE1560;
    z2zE1560 = zb.zlen;
    z2zE1563 = (z2zE1559 == z2zE1560);
  }
  bool z3zE1543;
  if (z2zE1563) {
    sail_fixed_bytes_32 z2zE1561;
    z2zE1561 = za.zdata;
    sail_fixed_bytes_32 z2zE1562;
    z2zE1562 = zb.zdata;
    z3zE1543 = eq_fixed_bytes_32(z2zE1561, z2zE1562);
  } else {  z3zE1543 = false;  }
  z8zE487 = z3zE1543;
end_function_1994: ;
  return z8zE487;
end_block_exception_1995: ;

  return false;
}

bool zpath_lt(struct zTriePath za, struct zTriePath zb)
{
  bool z8zE488;
  bool z2zE1554;
  {
    sail_fixed_bytes_32 z2zE1552;
    z2zE1552 = za.zdata;
    sail_fixed_bytes_32 z2zE1553;
    z2zE1553 = zb.zdata;
    z2zE1554 = eq_fixed_bytes_32(z2zE1552, z2zE1553);
  }
  if (z2zE1554) {
    uint64_t z2zE1555;
    z2zE1555 = zpath_len(za);
    uint64_t z2zE1556;
    z2zE1556 = zpath_len(zb);
    z8zE488 = (z2zE1555 < z2zE1556);
  } else {
    sail_fixed_bytes_32 z2zE1557;
    z2zE1557 = za.zdata;
    sail_fixed_bytes_32 z2zE1558;
    z2zE1558 = zb.zdata;
    z8zE488 = zhash_lt(z2zE1557, z2zE1558);
  }
end_function_1992: ;
  return z8zE488;
end_block_exception_1993: ;

  return false;
}

bool zpath_prefix_of(struct zTriePath zprefix, struct zTriePath zpath)
{
  bool z8zE489;
  bool z2zE1549;
  {
    uint64_t z2zE1547;
    z2zE1547 = zpath_len(zpath);
    uint64_t z2zE1548;
    z2zE1548 = zpath_len(zprefix);
    z2zE1549 = (z2zE1547 < z2zE1548);
  }
  if (z2zE1549) {  z8zE489 = false;  } else {
    struct zTriePath z2zE1551;
    {
      uint64_t z2zE1550;
      z2zE1550 = zpath_len(zprefix);
      {
        z2zE1551 = zpath_take(zpath, z2zE1550);
        if (have_exception) {  goto end_block_exception_1991;  }
      }
    }
    z8zE489 = zpath_eq(zprefix, z2zE1551);
  }
end_function_1990: ;
  return z8zE489;
end_block_exception_1991: ;

  return false;
}

uint64_t zcommon_prefix_from(struct zTriePath za, struct zTriePath zb, uint64_t zstart)
{
  uint64_t z8zE490;
  uint64_t zalen;
  zalen = zpath_len(za);
  uint64_t zblen;
  zblen = zpath_len(zb);
  uint64_t zstop;
  {
    bool z2zE1546;
    z2zE1546 = (zalen < zblen);
    if (z2zE1546) {  zstop = zalen;  } else {  zstop = zblen;  }
  }
  uint64_t zindex;
  zindex = zstart;
  uint64_t zcount;
  zcount = UINT64_C(0);
  bool zmatching;
  zmatching = true;
  int64_t z3zE1536;
  {    z3zE1536 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1537;
  {    z3zE1537 = (int64_t)(UINT64_C(63));
  }
  int64_t z3zE1538;
  {    z3zE1538 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z_step;
    z_step = z3zE1536;
    unit z3zE1540;
  for_start_1986: ;
    {
      if ((z3zE1537 < z_step)) goto for_end_1987;
      bool z2zE1542;
      {
        bool z3zE1539;
        if (zmatching) {  z3zE1539 = (zindex < zstop);  } else {  z3zE1539 = false;  }
        z2zE1542 = z3zE1539;
      }
      if (z2zE1542) {
        bool z2zE1545;
        {
          uint64_t z2zE1543;
          z2zE1543 = zpath_nibble(za, zindex);
          uint64_t z2zE1544;
          z2zE1544 = zpath_nibble(zb, zindex);
          z2zE1545 = (z2zE1543 == z2zE1544);
        }
        if (z2zE1545) {
          {
            zcount = ztrie_path_len_increment(zcount);
            if (have_exception) {  goto end_block_exception_1989;  }
          }
          unit z3zE1541;
          z3zE1541 = UNIT;
          {
            zindex = ztrie_path_len_increment(zindex);
            if (have_exception) {  goto end_block_exception_1989;  }
          }
          z3zE1540 = UNIT;
        } else {
          zmatching = false;
          z3zE1540 = UNIT;
        }
      } else {  z3zE1540 = UNIT;  }
      z_step = (z_step + z3zE1538);
      goto for_start_1986;
    }
  for_end_1987: ;
  }
  unit z3zE1542;
  z3zE1542 = UNIT;
  z8zE490 = zcount;
end_function_1988: ;
  return z8zE490;
end_block_exception_1989: ;

  return UINT64_C(0xdeadc0de);
}

void zhex_prefix_pairs(zz5listz8z5bv8z9 *z8zE491, struct zTriePath zpath, uint64_t zindex)
{
  bool z2zE1536;
  {
    uint64_t z2zE1535;
    z2zE1535 = zpath_len(zpath);
    z2zE1536 = (!(zindex < z2zE1535));
  }
  if (z2zE1536) {
    zz5listz8z5bv8z9 z3zE1535;
    CREATE(zz5listz8z5bv8z9)(&z3zE1535);
    COPY(zz5listz8z5bv8z9)((*(&z8zE491)), z3zE1535);
    KILL(zz5listz8z5bv8z9)(&z3zE1535);
  } else {
    uint64_t znext;
    {    znext = (zindex + UINT64_C(1));
    }
    uint64_t z2zE1540;
    {
      uint64_t z2zE1537;
      z2zE1537 = zpath_nibble(zpath, zindex);
      uint64_t z2zE1538;
      z2zE1538 = zpath_nibble(zpath, znext);
      z2zE1540 = (z2zE1537 << 4) | z2zE1538;
    }
    zz5listz8z5bv8z9 z2zE1541;
    CREATE(zz5listz8z5bv8z9)(&z2zE1541);
    {
      uint64_t z2zE1539;
      {    z2zE1539 = (znext + UINT64_C(1));
      }
      zhex_prefix_pairs(&z2zE1541, zpath, z2zE1539);
    }
    zconsz3z5bv8((*(&z8zE491)), z2zE1540, z2zE1541);
    KILL(zz5listz8z5bv8z9)(&z2zE1541);
  }
end_function_1984: ;
  goto end_function_4041;
end_block_exception_1985: ;
  goto end_function_4041;
end_function_4041: ;
}

void zhex_prefix_compact(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE492, struct zTriePath zpath, bool zis_leaf)
{
  uint64_t zlength;
  zlength = zpath_len(zpath);
  uint64_t zpacked_pair_count;
  {    zpacked_pair_count = (zlength / UINT64_C(2));
  }
  bool zodd;
  {
    uint64_t z2zE1534;
    {    z2zE1534 = (zlength % UINT64_C(2));
    }
    zodd = (z2zE1534 != UINT64_C(0));
  }
  uint64_t zflag;
  if (zis_leaf) {  zflag = UINT64_C(0x2);  } else {  zflag = UINT64_C(0x0);  }
  uint64_t zfirst;
  if (zodd) {
    uint64_t z2zE1532;
    z2zE1532 = (zflag | UINT64_C(0x1));
    uint64_t z2zE1533;
    z2zE1533 = zpath_nibble(zpath, UINT64_C(0));
    zfirst = (z2zE1532 << 4) | z2zE1533;
  } else {  zfirst = (zflag << 4) | UINT64_C(0x0);  }
  uint64_t zfirst_path_index;
  if (zodd) {  zfirst_path_index = UINT64_C(1);  } else {  zfirst_path_index = UINT64_C(0);  }
  uint64_t zencoded_len;
  {    zencoded_len = (UINT64_C(1) + zpacked_pair_count);
  }
  zz5listz8z5bv8z9 z2zE1531;
  CREATE(zz5listz8z5bv8z9)(&z2zE1531);
  {
    zz5listz8z5bv8z9 z2zE1530;
    CREATE(zz5listz8z5bv8z9)(&z2zE1530);
    zhex_prefix_pairs(&z2zE1530, zpath, zfirst_path_index);
    zconsz3z5bv8(&z2zE1531, zfirst, z2zE1530);
    KILL(zz5listz8z5bv8z9)(&z2zE1530);
  }
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE1534;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE1534);
  COPY(zz5listz8z5bv8z9)(&((&z3zE1534)->ztup0), z2zE1531);
  z3zE1534.ztup1 = zencoded_len;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE492)), z3zE1534);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE1534);
  KILL(zz5listz8z5bv8z9)(&z2zE1531);
end_function_1982: ;
  goto end_function_4040;
end_block_exception_1983: ;
  goto end_function_4040;
end_function_4040: ;
}

