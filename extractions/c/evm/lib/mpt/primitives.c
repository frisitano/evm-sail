/* Generated from sail/lib/mpt/primitives.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_239(void) {

  uint64_t z3zE264;
  z3zE264 = UINT64_C(33);
  zHEX_PREFIX_MAX_LENGTH = z3zE264;
let_end_789: ;
}
void kill_letbind_239(void) {
}

uint64_t zpath_len(struct zTriePath zpath)
{
  uint64_t z8zE523;
  z8zE523 = zpath.zlen;
end_function_1795: ;
  return z8zE523;
end_block_exception_1796: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztrie_path_len_increment(uint64_t zvalue)
{
  uint64_t z8zE524;
  bool z2zE1189;
  z2zE1189 = (zvalue < UINT64_C(64));
  if (z2zE1189) {
    {    z8zE524 = (zvalue + UINT64_C(1));
    }
  } else {
    struct zexception z2zE1190;
    CREATE(zexception)(&z2zE1190);
    zInvalidBlock(&z2zE1190, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1190);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:35.8-35.44");
    KILL(zexception)(&z2zE1190);
    goto end_block_exception_1794;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1190);
  }
end_function_1793: ;
  return z8zE524;
end_block_exception_1794: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath zpath_new(sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  struct zTriePath z8zE525;
  struct zTriePath z3zE1271;
  z3zE1271.zdata = zdata;
  z3zE1271.zlen = zlen;
  z8zE525 = z3zE1271;
end_function_1791: ;
  return z8zE525;
end_block_exception_1792: ;
  struct zTriePath z8zE898 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE898;
}

uint64_t zto_trie_depth(uint64_t zvalue)
{
  uint64_t z8zE526;
  bool z2zE1187;
  z2zE1187 = (!(zvalue < UINT64_C(64)));
  if (z2zE1187) {
    struct zexception z2zE1188;
    CREATE(zexception)(&z2zE1188);
    zInvalidBlock(&z2zE1188, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1188);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:45.8-45.44");
    KILL(zexception)(&z2zE1188);
    goto end_block_exception_1790;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1188);
  } else {  z8zE526 = zvalue;  }
end_function_1789: ;
  return z8zE526;
end_block_exception_1790: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath zpath_from_hash(sail_fixed_bytes_32 zh)
{
  struct zTriePath z8zE527;
  z8zE527 = zpath_new(zh, UINT64_C(64));
end_function_1787: ;
  return z8zE527;
end_block_exception_1788: ;
  struct zTriePath z8zE899 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE899;
}

uint64_t zpath_byte_index(uint64_t zi)
{
  uint64_t z8zE528;
  uint64_t zquotient;
  {    zquotient = (zi / UINT64_C(2));
  }
  uint64_t znatural_index;
  {
    bool z2zE1186;
    {
      bool z2zE1185;
      z2zE1185 = (!(zquotient < UINT64_C(0)));
      bool z3zE1269;
      if (z2zE1185) {  z3zE1269 = (!(UINT64_C(31) < zquotient));  } else {  z3zE1269 = false;  }
      z2zE1186 = z3zE1269;
    }
    if (z2zE1186) {  znatural_index = zquotient;  } else {
      unit z3zE1270;
      z3zE1270 = sail_assert(false, "sail/lib/mpt/primitives.sail:64.24-64.25");
      sail_match_failure("path_byte_index");
      /* unreachable after exit */
    }
  }
  z8zE528 = znatural_index;
end_function_1785: ;
  return z8zE528;
end_block_exception_1786: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zpath_nibble(struct zTriePath zpath, uint64_t zi)
{
  uint64_t z8zE529;
  bool z2zE1180;
  {
    uint64_t z2zE1179;
    z2zE1179 = zpath_len(zpath);
    z2zE1180 = (!(zi < z2zE1179));
  }
  if (z2zE1180) {  z8zE529 = UINT64_C(0x0);  } else {
    sail_fixed_bytes_32 zbytes;
    zbytes = zpath.zdata;
    uint64_t zbyte_index;
    zbyte_index = zpath_byte_index(zi);
    bool z2zE1182;
    {
      uint64_t z2zE1181;
      {    z2zE1181 = (zi % UINT64_C(2));
      }
      z2zE1182 = (z2zE1181 == UINT64_C(0));
    }
    if (z2zE1182) {
      uint64_t z2zE1183;
      z2zE1183 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
      z8zE529 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE1183 >> UINT64_C(4)));
    } else {
      uint64_t z2zE1184;
      z2zE1184 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
      z8zE529 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE1184 >> UINT64_C(0)));
    }
  }
end_function_1783: ;
  return z8zE529;
end_block_exception_1784: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath zpath_append_nibble(struct zTriePath zpath, uint64_t zvalue)
{
  struct zTriePath z8zE530;
  uint64_t zlength;
  zlength = zpath_len(zpath);
  bool z2zE1169;
  z2zE1169 = (!(zlength < UINT64_C(64)));
  unit z3zE1267;
  if (z2zE1169) {
    struct zexception z2zE1170;
    CREATE(zexception)(&z2zE1170);
    zInvalidBlock(&z2zE1170, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1170);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:88.8-88.44");
    KILL(zexception)(&z2zE1170);
    goto end_block_exception_1782;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1170);
  } else {  z3zE1267 = UNIT;  }
  sail_fixed_bytes_32 zoriginal;
  zoriginal = zpath.zdata;
  sail_fixed_bytes_32 zbytes;
  zbytes = zoriginal;
  uint64_t zbyte_index;
  zbyte_index = zpath_byte_index(zlength);
  bool z2zE1172;
  {
    uint64_t z2zE1171;
    {    z2zE1171 = (zlength % UINT64_C(2));
    }
    z2zE1172 = (z2zE1171 == UINT64_C(0));
  }
  unit z3zE1268;
  if (z2zE1172) {
    uint64_t z2zE1173;
    z2zE1173 = (zvalue << 4) | UINT64_C(0x0);
    zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, zbyte_index, z2zE1173);
    z3zE1268 = UNIT;
  } else {
    uint64_t z2zE1176;
    {
      uint64_t z2zE1175;
      {
        uint64_t z2zE1174;
        z2zE1174 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
        z2zE1175 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE1174 >> UINT64_C(4)));
      }
      z2zE1176 = (z2zE1175 << 4) | zvalue;
    }
    zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, zbyte_index, z2zE1176);
    z3zE1268 = UNIT;
  }
  sail_fixed_bytes_32 z2zE1177;
  z2zE1177 = zB256(zbytes);
  uint64_t z2zE1178;
  {
    z2zE1178 = ztrie_path_len_increment(zlength);
    if (have_exception) {  goto end_block_exception_1782;  }
  }
  z8zE530 = zpath_new(z2zE1177, z2zE1178);
end_function_1781: ;
  return z8zE530;
end_block_exception_1782: ;
  struct zTriePath z8zE900 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE900;
}

struct zTriePath zpath_single(uint64_t zn)
{
  struct zTriePath z8zE531;
  struct zTriePath z3zE1265;
  zz5vecz8z5bv8z9 z3zE1266;
  CREATE(zz5vecz8z5bv8z9)(&z3zE1266);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE1266, INT64_C(32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(0), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(1), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(2), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(3), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(4), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(5), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(6), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(7), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(8), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(9), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(10), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(11), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(12), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(13), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(15), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(16), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(17), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(18), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(19), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(20), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(21), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(22), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(23), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(24), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(25), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(26), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(27), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(28), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(29), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(30), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE1266, z3zE1266, INT64_C(31), UINT64_C(0x00));
  for (size_t z8zE901 = 0; z8zE901 < 32; ++z8zE901) {
    z3zE1265.zdata.bytes[z8zE901] = (uint8_t)(z3zE1266.data[z8zE901] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE1266);
  z3zE1265.zlen = UINT64_C(0);
  {
    z8zE531 = zpath_append_nibble(z3zE1265, zn);
    if (have_exception) {  goto end_block_exception_1780;  }
  }
end_function_1779: ;
  return z8zE531;
end_block_exception_1780: ;
  struct zTriePath z8zE902 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE902;
}

struct zTriePath zpath_concat(struct zTriePath za, struct zTriePath zb)
{
  struct zTriePath z8zE532;
  uint64_t zalen;
  zalen = zpath_len(za);
  uint64_t zblen;
  zblen = zpath_len(zb);
  uint64_t zcombined;
  {    zcombined = (zalen + zblen);
  }
  bool z2zE1165;
  z2zE1165 = (!(UINT64_C(64) < zcombined));
  if (z2zE1165) {
    struct zTriePath zresult;
    zresult = za;
    uint64_t zindex;
    zindex = UINT64_C(0);
    int64_t z3zE1259;
    {    z3zE1259 = (int64_t)(UINT64_C(0));
    }
    int64_t z3zE1260;
    {    z3zE1260 = (int64_t)(UINT64_C(63));
    }
    int64_t z3zE1261;
    {    z3zE1261 = (int64_t)(UINT64_C(1));
    }
    {
      int64_t z_step;
      z_step = z3zE1259;
      unit z3zE1262;
    for_start_1775: ;
      {
        if ((z3zE1260 < z_step)) goto for_end_1776;
        bool z2zE1166;
        z2zE1166 = (zindex < zblen);
        if (z2zE1166) {
          uint64_t z2zE1167;
          z2zE1167 = zpath_nibble(zb, zindex);
          {
            zresult = zpath_append_nibble(zresult, z2zE1167);
            if (have_exception) {  goto end_block_exception_1778;  }
          }
          unit z3zE1263;
          z3zE1263 = UNIT;
          {
            zindex = ztrie_path_len_increment(zindex);
            if (have_exception) {  goto end_block_exception_1778;  }
          }
          z3zE1262 = UNIT;
        } else {  z3zE1262 = UNIT;  }
        z_step = (z_step + z3zE1261);
        goto for_start_1775;
      }
    for_end_1776: ;
    }
    unit z3zE1264;
    z3zE1264 = UNIT;
    z8zE532 = zresult;
  } else {
    struct zexception z2zE1168;
    CREATE(zexception)(&z2zE1168);
    zInvalidBlock(&z2zE1168, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1168);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:124.8-124.44");
    KILL(zexception)(&z2zE1168);
    goto end_block_exception_1778;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1168);
  }
end_function_1777: ;
  return z8zE532;
end_block_exception_1778: ;
  struct zTriePath z8zE903 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE903;
}

struct zTriePath zpath_take(struct zTriePath zpath, uint64_t zn)
{
  struct zTriePath z8zE533;
  bool z2zE1160;
  z2zE1160 = (zn == UINT64_C(0));
  if (z2zE1160) {
    struct zTriePath z3zE1257;
    zz5vecz8z5bv8z9 z3zE1258;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1258);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1258, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1258, z3zE1258, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE905 = 0; z8zE905 < 32; ++z8zE905) {
      z3zE1257.zdata.bytes[z8zE905] = (uint8_t)(z3zE1258.data[z8zE905] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1258);
    z3zE1257.zlen = UINT64_C(0);
    z8zE533 = z3zE1257;
  } else {
    bool z2zE1162;
    {
      uint64_t z2zE1161;
      z2zE1161 = zpath_len(zpath);
      z2zE1162 = (!(zn < z2zE1161));
    }
    if (z2zE1162) {  z8zE533 = zpath;  } else {
      struct zTriePath zresult;
      {
        struct zTriePath z3zE1249;
        zz5vecz8z5bv8z9 z3zE1250;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1250);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1250, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1250, z3zE1250, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE904 = 0; z8zE904 < 32; ++z8zE904) {
          z3zE1249.zdata.bytes[z8zE904] = (uint8_t)(z3zE1250.data[z8zE904] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1250);
        z3zE1249.zlen = UINT64_C(0);
        zresult = z3zE1249;
      }
      uint64_t zindex;
      zindex = UINT64_C(0);
      int64_t z3zE1251;
      {    z3zE1251 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1252;
      {    z3zE1252 = (int64_t)(UINT64_C(63));
      }
      int64_t z3zE1253;
      {    z3zE1253 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_step;
        z_step = z3zE1251;
        unit z3zE1254;
      for_start_1771: ;
        {
          if ((z3zE1252 < z_step)) goto for_end_1772;
          bool z2zE1163;
          z2zE1163 = (zindex < zn);
          if (z2zE1163) {
            uint64_t z2zE1164;
            z2zE1164 = zpath_nibble(zpath, zindex);
            {
              zresult = zpath_append_nibble(zresult, z2zE1164);
              if (have_exception) {  goto end_block_exception_1774;  }
            }
            unit z3zE1255;
            z3zE1255 = UNIT;
            {
              zindex = ztrie_path_len_increment(zindex);
              if (have_exception) {  goto end_block_exception_1774;  }
            }
            z3zE1254 = UNIT;
          } else {  z3zE1254 = UNIT;  }
          z_step = (z_step + z3zE1253);
          goto for_start_1771;
        }
      for_end_1772: ;
      }
      unit z3zE1256;
      z3zE1256 = UNIT;
      z8zE533 = zresult;
    }
  }
end_function_1773: ;
  return z8zE533;
end_block_exception_1774: ;
  struct zTriePath z8zE906 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE906;
}

struct zTriePath zpath_drop(struct zTriePath zpath, uint64_t zn)
{
  struct zTriePath z8zE534;
  uint64_t zlength;
  zlength = zpath_len(zpath);
  bool z2zE1154;
  z2zE1154 = (!(zn < zlength));
  if (z2zE1154) {
    struct zTriePath z3zE1247;
    zz5vecz8z5bv8z9 z3zE1248;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1248);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1248, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1248, z3zE1248, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE908 = 0; z8zE908 < 32; ++z8zE908) {
      z3zE1247.zdata.bytes[z8zE908] = (uint8_t)(z3zE1248.data[z8zE908] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1248);
    z3zE1247.zlen = UINT64_C(0);
    z8zE534 = z3zE1247;
  } else {
    bool z2zE1155;
    z2zE1155 = (zn == UINT64_C(0));
    if (z2zE1155) {  z8zE534 = zpath;  } else {
      uint64_t zremain;
      {    zremain = (zlength - zn);
      }
      struct zTriePath zresult;
      {
        struct zTriePath z3zE1237;
        zz5vecz8z5bv8z9 z3zE1238;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1238);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1238, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1238, z3zE1238, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE907 = 0; z8zE907 < 32; ++z8zE907) {
          z3zE1237.zdata.bytes[z8zE907] = (uint8_t)(z3zE1238.data[z8zE907] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1238);
        z3zE1237.zlen = UINT64_C(0);
        zresult = z3zE1237;
      }
      uint64_t zoffset;
      zoffset = UINT64_C(0);
      int64_t z3zE1239;
      {    z3zE1239 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1240;
      {    z3zE1240 = (int64_t)(UINT64_C(63));
      }
      int64_t z3zE1241;
      {    z3zE1241 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_step;
        z_step = z3zE1239;
        unit z3zE1242;
      for_start_1767: ;
        {
          if ((z3zE1240 < z_step)) goto for_end_1768;
          bool z2zE1156;
          z2zE1156 = (zoffset < zremain);
          if (z2zE1156) {
            uint64_t zcandidate;
            {    zcandidate = (zn + zoffset);
            }
            uint64_t zsource_index;
            {
              bool z2zE1159;
              {
                bool z2zE1158;
                z2zE1158 = (!(zcandidate < UINT64_C(0)));
                bool z3zE1243;
                if (z2zE1158) {  z3zE1243 = (!(UINT64_C(64) < zcandidate));  } else {  z3zE1243 = false;  }
                z2zE1159 = z3zE1243;
              }
              if (z2zE1159) {  zsource_index = zcandidate;  } else {
                unit z3zE1244;
                z3zE1244 = sail_assert(false, "sail/lib/mpt/primitives.sail:166.40-166.41");
                sail_match_failure("path_drop");
                /* unreachable after exit */
              }
            }
            uint64_t z2zE1157;
            z2zE1157 = zpath_nibble(zpath, zsource_index);
            {
              zresult = zpath_append_nibble(zresult, z2zE1157);
              if (have_exception) {  goto end_block_exception_1770;  }
            }
            unit z3zE1245;
            z3zE1245 = UNIT;
            {
              zoffset = ztrie_path_len_increment(zoffset);
              if (have_exception) {  goto end_block_exception_1770;  }
            }
            z3zE1242 = UNIT;
          } else {  z3zE1242 = UNIT;  }
          z_step = (z_step + z3zE1241);
          goto for_start_1767;
        }
      for_end_1768: ;
      }
      unit z3zE1246;
      z3zE1246 = UNIT;
      z8zE534 = zresult;
    }
  }
end_function_1769: ;
  return z8zE534;
end_block_exception_1770: ;
  struct zTriePath z8zE909 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE909;
}

bool zpath_eq(struct zTriePath za, struct zTriePath zb)
{
  bool z8zE535;
  bool z2zE1153;
  {
    uint64_t z2zE1149;
    z2zE1149 = za.zlen;
    uint64_t z2zE1150;
    z2zE1150 = zb.zlen;
    z2zE1153 = (z2zE1149 == z2zE1150);
  }
  bool z3zE1236;
  if (z2zE1153) {
    sail_fixed_bytes_32 z2zE1151;
    z2zE1151 = za.zdata;
    sail_fixed_bytes_32 z2zE1152;
    z2zE1152 = zb.zdata;
    z3zE1236 = eq_fixed_bytes_32(z2zE1151, z2zE1152);
  } else {  z3zE1236 = false;  }
  z8zE535 = z3zE1236;
end_function_1765: ;
  return z8zE535;
end_block_exception_1766: ;

  return false;
}

bool zpath_lt(struct zTriePath za, struct zTriePath zb)
{
  bool z8zE536;
  bool z2zE1144;
  {
    sail_fixed_bytes_32 z2zE1142;
    z2zE1142 = za.zdata;
    sail_fixed_bytes_32 z2zE1143;
    z2zE1143 = zb.zdata;
    z2zE1144 = eq_fixed_bytes_32(z2zE1142, z2zE1143);
  }
  if (z2zE1144) {
    uint64_t z2zE1145;
    z2zE1145 = zpath_len(za);
    uint64_t z2zE1146;
    z2zE1146 = zpath_len(zb);
    z8zE536 = (z2zE1145 < z2zE1146);
  } else {
    sail_fixed_bytes_32 z2zE1147;
    z2zE1147 = za.zdata;
    sail_fixed_bytes_32 z2zE1148;
    z2zE1148 = zb.zdata;
    z8zE536 = evmsail_hash_lt(z2zE1147, z2zE1148);
  }
end_function_1763: ;
  return z8zE536;
end_block_exception_1764: ;

  return false;
}

bool zpath_prefix_of(struct zTriePath zprefix, struct zTriePath zpath)
{
  bool z8zE537;
  bool z2zE1139;
  {
    uint64_t z2zE1137;
    z2zE1137 = zpath_len(zpath);
    uint64_t z2zE1138;
    z2zE1138 = zpath_len(zprefix);
    z2zE1139 = (z2zE1137 < z2zE1138);
  }
  if (z2zE1139) {  z8zE537 = false;  } else {
    struct zTriePath z2zE1141;
    {
      uint64_t z2zE1140;
      z2zE1140 = zpath_len(zprefix);
      {
        z2zE1141 = zpath_take(zpath, z2zE1140);
        if (have_exception) {  goto end_block_exception_1762;  }
      }
    }
    z8zE537 = zpath_eq(zprefix, z2zE1141);
  }
end_function_1761: ;
  return z8zE537;
end_block_exception_1762: ;

  return false;
}

uint64_t zcommon_prefix_from(struct zTriePath za, struct zTriePath zb, uint64_t zstart)
{
  uint64_t z8zE538;
  uint64_t zalen;
  zalen = zpath_len(za);
  uint64_t zblen;
  zblen = zpath_len(zb);
  uint64_t zstop;
  {
    bool z2zE1136;
    z2zE1136 = (zalen < zblen);
    if (z2zE1136) {  zstop = zalen;  } else {  zstop = zblen;  }
  }
  uint64_t zindex;
  zindex = zstart;
  uint64_t zcount;
  zcount = UINT64_C(0);
  bool zmatching;
  zmatching = true;
  int64_t z3zE1229;
  {    z3zE1229 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1230;
  {    z3zE1230 = (int64_t)(UINT64_C(63));
  }
  int64_t z3zE1231;
  {    z3zE1231 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z_step;
    z_step = z3zE1229;
    unit z3zE1233;
  for_start_1757: ;
    {
      if ((z3zE1230 < z_step)) goto for_end_1758;
      bool z2zE1132;
      {
        bool z3zE1232;
        if (zmatching) {  z3zE1232 = (zindex < zstop);  } else {  z3zE1232 = false;  }
        z2zE1132 = z3zE1232;
      }
      if (z2zE1132) {
        bool z2zE1135;
        {
          uint64_t z2zE1133;
          z2zE1133 = zpath_nibble(za, zindex);
          uint64_t z2zE1134;
          z2zE1134 = zpath_nibble(zb, zindex);
          z2zE1135 = (z2zE1133 == z2zE1134);
        }
        if (z2zE1135) {
          {
            zcount = ztrie_path_len_increment(zcount);
            if (have_exception) {  goto end_block_exception_1760;  }
          }
          unit z3zE1234;
          z3zE1234 = UNIT;
          {
            zindex = ztrie_path_len_increment(zindex);
            if (have_exception) {  goto end_block_exception_1760;  }
          }
          z3zE1233 = UNIT;
        } else {
          zmatching = false;
          z3zE1233 = UNIT;
        }
      } else {  z3zE1233 = UNIT;  }
      z_step = (z_step + z3zE1231);
      goto for_start_1757;
    }
  for_end_1758: ;
  }
  unit z3zE1235;
  z3zE1235 = UNIT;
  z8zE538 = zcount;
end_function_1759: ;
  return z8zE538;
end_block_exception_1760: ;

  return UINT64_C(0xdeadc0de);
}

void zhex_prefix_pairs(zz5listz8z5bv8z9 *z8zE539, struct zTriePath zpath, uint64_t zindex)
{
  bool z2zE1126;
  {
    uint64_t z2zE1125;
    z2zE1125 = zpath_len(zpath);
    z2zE1126 = (!(zindex < z2zE1125));
  }
  if (z2zE1126) {
    zz5listz8z5bv8z9 z3zE1228;
    CREATE(zz5listz8z5bv8z9)(&z3zE1228);
    COPY(zz5listz8z5bv8z9)((*(&z8zE539)), z3zE1228);
    KILL(zz5listz8z5bv8z9)(&z3zE1228);
  } else {
    uint64_t znext;
    {    znext = (zindex + UINT64_C(1));
    }
    uint64_t z2zE1130;
    {
      uint64_t z2zE1127;
      z2zE1127 = zpath_nibble(zpath, zindex);
      uint64_t z2zE1128;
      z2zE1128 = zpath_nibble(zpath, znext);
      z2zE1130 = (z2zE1127 << 4) | z2zE1128;
    }
    zz5listz8z5bv8z9 z2zE1131;
    CREATE(zz5listz8z5bv8z9)(&z2zE1131);
    {
      uint64_t z2zE1129;
      {    z2zE1129 = (znext + UINT64_C(1));
      }
      zhex_prefix_pairs(&z2zE1131, zpath, z2zE1129);
    }
    zconsz3z5bv8((*(&z8zE539)), z2zE1130, z2zE1131);
    KILL(zz5listz8z5bv8z9)(&z2zE1131);
  }
end_function_1755: ;
  goto end_function_3630;
end_block_exception_1756: ;
  goto end_function_3630;
end_function_3630: ;
}

void zhex_prefix_compact(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE540, struct zTriePath zpath, bool zis_leaf)
{
  uint64_t zlength;
  zlength = zpath_len(zpath);
  uint64_t zpacked_pair_count;
  {    zpacked_pair_count = (zlength / UINT64_C(2));
  }
  bool zodd;
  {
    uint64_t z2zE1124;
    {    z2zE1124 = (zlength % UINT64_C(2));
    }
    zodd = (z2zE1124 != UINT64_C(0));
  }
  uint64_t zflag;
  if (zis_leaf) {  zflag = UINT64_C(0x2);  } else {  zflag = UINT64_C(0x0);  }
  uint64_t zfirst;
  if (zodd) {
    uint64_t z2zE1122;
    z2zE1122 = (zflag | UINT64_C(0x1));
    uint64_t z2zE1123;
    z2zE1123 = zpath_nibble(zpath, UINT64_C(0));
    zfirst = (z2zE1122 << 4) | z2zE1123;
  } else {  zfirst = (zflag << 4) | UINT64_C(0x0);  }
  uint64_t zfirst_path_index;
  if (zodd) {  zfirst_path_index = UINT64_C(1);  } else {  zfirst_path_index = UINT64_C(0);  }
  uint64_t zencoded_len;
  {    zencoded_len = (UINT64_C(1) + zpacked_pair_count);
  }
  zz5listz8z5bv8z9 z2zE1121;
  CREATE(zz5listz8z5bv8z9)(&z2zE1121);
  {
    zz5listz8z5bv8z9 z2zE1120;
    CREATE(zz5listz8z5bv8z9)(&z2zE1120);
    zhex_prefix_pairs(&z2zE1120, zpath, zfirst_path_index);
    zconsz3z5bv8(&z2zE1121, zfirst, z2zE1120);
    KILL(zz5listz8z5bv8z9)(&z2zE1120);
  }
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE1227;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE1227);
  COPY(zz5listz8z5bv8z9)(&((&z3zE1227)->ztup0), z2zE1121);
  z3zE1227.ztup1 = zencoded_len;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE540)), z3zE1227);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE1227);
  KILL(zz5listz8z5bv8z9)(&z2zE1121);
end_function_1753: ;
  goto end_function_3629;
end_block_exception_1754: ;
  goto end_function_3629;
end_function_3629: ;
}

struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 zhex_prefix_decode_ref(struct zRlpFieldRef zf)
{
  struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z8zE541;
  bool z2zE1097;
  z2zE1097 = zf.zis_list;
  unit z3zE1220;
  if (z2zE1097) {
    struct zexception z2zE1098;
    CREATE(zexception)(&z2zE1098);
    zInvalidBlock(&z2zE1098, zRlpDecode);
    COPY(zexception)(current_exception, z2zE1098);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:261.8-261.37");
    KILL(zexception)(&z2zE1098);
    goto end_block_exception_1752;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1098);
  } else {  z3zE1220 = UNIT;  }
  uint64_t zn;
  zn = zf.zcontent_len;
  bool z2zE1099;
  z2zE1099 = (zn == UINT64_C(0));
  if (z2zE1099) {
    struct zTriePath z3zE1224;
    zz5vecz8z5bv8z9 z3zE1225;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1225);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1225, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1225, z3zE1225, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE910 = 0; z8zE910 < 32; ++z8zE910) {
      z3zE1224.zdata.bytes[z8zE910] = (uint8_t)(z3zE1225.data[z8zE910] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1225);
    z3zE1224.zlen = UINT64_C(0);
    struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE1226;
    z3zE1226.ztup0 = false;
    z3zE1226.ztup1 = z3zE1224;
    z8zE541 = z3zE1226;
  } else {
    uint64_t zmaximum_length;
    zmaximum_length = zHEX_PREFIX_MAX_LENGTH;
    bool z2zE1100;
    z2zE1100 = (zmaximum_length < zn);
    if (z2zE1100) {
      struct zexception z2zE1101;
      CREATE(zexception)(&z2zE1101);
      zInvalidBlock(&z2zE1101, zRlpDecode);
      COPY(zexception)(current_exception, z2zE1101);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:269.12-269.41");
      KILL(zexception)(&z2zE1101);
      goto end_block_exception_1752;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1101);
    } else {
      struct zByteSliceFields zcontent;
      {
        struct zByteSliceFields z2zE1118;
        z2zE1118 = zf.zsource;
        uint64_t z2zE1119;
        {
          uint64_t z2zE1117;
          {
            struct zByteSliceFields z2zE1116;
            z2zE1116 = zf.zsource;
            z2zE1117 = z2zE1116.zlen;
          }
          {    z2zE1119 = (z2zE1117 - zn);
          }
        }
        zcontent = zsub_slice(z2zE1118, z2zE1119, zn);
      }
      uint64_t zfb;
      zfb = zslice_byte(zcontent, UINT64_C(0));
      uint64_t zflag;
      zflag = (safe_rshift(UINT64_MAX, 64 - 4) & (zfb >> UINT64_C(4)));
      bool zis_leaf;
      {
        uint64_t z2zE1115;
        z2zE1115 = (UINT64_C(1) & (zflag >> UINT64_C(1)));
        zis_leaf = (z2zE1115 == UINT64_C(0b1));
      }
      bool zodd;
      {
        uint64_t z2zE1114;
        z2zE1114 = (UINT64_C(1) & (zflag >> UINT64_C(0)));
        zodd = (z2zE1114 == UINT64_C(0b1));
      }
      uint64_t ztail_length;
      {    ztail_length = (zn - UINT64_C(1));
      }
      struct zByteSliceFields ztail;
      ztail = zslice_suffix(zcontent, UINT64_C(1));
      sail_u256 zpacked;
      zpacked = zslice_load(ztail, UINT64_C(0));
      uint64_t zpaired_nibbles;
      {    zpaired_nibbles = (ztail_length * UINT64_C(2));
      }
      if (zodd) {
        bool z2zE1102;
        z2zE1102 = (zpaired_nibbles < UINT64_C(64));
        if (z2zE1102) {
          sail_fixed_bytes_32 zbytes;
          {
            sail_u256 z2zE1110;
            z2zE1110 = zword_shift_right(zpacked, UINT64_C(4));
            zbytes = evmsail_word_to_hash(z2zE1110);
          }
          uint64_t z2zE1106;
          {
            uint64_t z2zE1104;
            z2zE1104 = (safe_rshift(UINT64_MAX, 64 - 4) & (zfb >> UINT64_C(0)));
            uint64_t z2zE1105;
            {
              uint64_t z2zE1103;
              z2zE1103 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(0));
              z2zE1105 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE1103 >> UINT64_C(0)));
            }
            z2zE1106 = (z2zE1104 << 4) | z2zE1105;
          }
          zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, UINT64_C(0), z2zE1106);
          unit z3zE1222;
          z3zE1222 = UNIT;
          struct zTriePath z2zE1109;
          {
            sail_fixed_bytes_32 z2zE1107;
            z2zE1107 = zB256(zbytes);
            uint64_t z2zE1108;
            {    z2zE1108 = (zpaired_nibbles + UINT64_C(1));
            }
            z2zE1109 = zpath_new(z2zE1107, z2zE1108);
          }
          struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE1223;
          z3zE1223.ztup0 = zis_leaf;
          z3zE1223.ztup1 = z2zE1109;
          z8zE541 = z3zE1223;
        } else {
          struct zexception z2zE1111;
          CREATE(zexception)(&z2zE1111);
          zInvalidBlock(&z2zE1111, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE1111);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:286.20-286.56");
          KILL(zexception)(&z2zE1111);
          goto end_block_exception_1752;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1111);
        }
      } else {
        struct zTriePath z2zE1113;
        {
          sail_fixed_bytes_32 z2zE1112;
          z2zE1112 = evmsail_word_to_hash(zpacked);
          z2zE1113 = zpath_new(z2zE1112, zpaired_nibbles);
        }
        struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE1221;
        z3zE1221.ztup0 = zis_leaf;
        z3zE1221.ztup1 = z2zE1113;
        z8zE541 = z3zE1221;
      }
    }
  }
end_function_1751: ;
  return z8zE541;
end_block_exception_1752: ;
  struct zTriePath z8zE912 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z8zE911 = { .ztup0 = false, .ztup1 = z8zE912 };
  return z8zE911;
}

