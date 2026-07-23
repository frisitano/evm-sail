/* Generated from sail/lib/mpt/indexed.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zrlp_index_byte_width_decrement(uint64_t zvalue)
{
  uint64_t z8zE545;
  bool z2zE1275;
  z2zE1275 = (UINT64_C(0) < zvalue);
  unit z3zE1328;
  z3zE1328 = sail_assert(z2zE1275, "sail/lib/mpt/indexed.sail:33.20-33.21");
  {    z8zE545 = (zvalue - UINT64_C(1));
  }
end_function_1769: ;
  return z8zE545;
end_block_exception_1770: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_index_encoded_width(uint64_t zvalue)
{
  uint64_t z8zE546;
  bool z2zE1268;
  z2zE1268 = (zvalue < UINT64_C(256));
  if (z2zE1268) {  z8zE546 = UINT64_C(1);  } else {
    bool z2zE1269;
    z2zE1269 = (zvalue < UINT64_C(65536));
    if (z2zE1269) {  z8zE546 = UINT64_C(2);  } else {
      bool z2zE1270;
      z2zE1270 = (zvalue < UINT64_C(16777216));
      if (z2zE1270) {  z8zE546 = UINT64_C(3);  } else {
        bool z2zE1271;
        z2zE1271 = (zvalue < UINT64_C(4294967296));
        if (z2zE1271) {  z8zE546 = UINT64_C(4);  } else {
          bool z2zE1272;
          z2zE1272 = (zvalue < UINT64_C(1099511627776));
          if (z2zE1272) {  z8zE546 = UINT64_C(5);  } else {
            bool z2zE1273;
            z2zE1273 = (zvalue < UINT64_C(281474976710656));
            if (z2zE1273) {  z8zE546 = UINT64_C(6);  } else {
              bool z2zE1274;
              z2zE1274 = (zvalue < UINT64_C(72057594037927936));
              if (z2zE1274) {  z8zE546 = UINT64_C(7);  } else {  z8zE546 = UINT64_C(8);  }
            }
          }
        }
      }
    }
  }
end_function_1767: ;
  return z8zE546;
end_block_exception_1768: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath ztrie_index_key(uint64_t zindex)
{
  struct zTriePath z8zE547;
  bool z2zE1261;
  z2zE1261 = (zindex == UINT64_C(0));
  if (z2zE1261) {
    struct zTriePath z3zE1326;
    zz5vecz8z5bv8z9 z3zE1327;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1327);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1327, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(31), UINT64_C(0x80));
    for (size_t z8zE1013 = 0; z8zE1013 < 32; ++z8zE1013) {
      z3zE1326.zdata.bytes[z8zE1013] = (uint8_t)(z3zE1327.data[z8zE1013] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1327);
    z3zE1326.zlen = UINT64_C(2);
    z8zE547 = z3zE1326;
  } else {
    bool z2zE1262;
    z2zE1262 = (!(UINT64_C(127) < zindex));
    if (z2zE1262) {
      uint64_t z2zE1263;
      z2zE1263 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zindex, UINT64_C(0)));
      struct zTriePath z3zE1324;
      zz5vecz8z5bv8z9 z3zE1325;
      CREATE(zz5vecz8z5bv8z9)(&z3zE1325);
      internal_vector_init_zz5vecz8z5bv8z9(&z3zE1325, INT64_C(32));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(0), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(1), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(2), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(3), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(4), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(5), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(6), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(7), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(8), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(9), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(10), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(11), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(12), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(13), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(14), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(15), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(16), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(17), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(18), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(19), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(20), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(21), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(22), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(23), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(24), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(25), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(26), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(27), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(28), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(29), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(30), UINT64_C(0x00));
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1325, z3zE1325, INT64_C(31), UINT64_C(0x00));
      for (size_t z8zE1012 = 0; z8zE1012 < 32; ++z8zE1012) {
        z3zE1324.zdata.bytes[z8zE1012] = (uint8_t)(z3zE1325.data[z8zE1012] & UINT64_C(0xff));
      }
      KILL(zz5vecz8z5bv8z9)(&z3zE1325);
      z3zE1324.zlen = UINT64_C(0);
      {
        z8zE547 = zpath_append_byte(z3zE1324, z2zE1263);
        if (have_exception) {  goto end_block_exception_1766;  }
      }
    } else {
      uint64_t zwidth;
      zwidth = zrlp_index_encoded_width(zindex);
      struct zTriePath zpath;
      {
        uint64_t z2zE1267;
        {
          uint64_t z2zE1266;
          {    z2zE1266 = (UINT64_C(128) + zwidth);
          }
          z2zE1267 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(z2zE1266, UINT64_C(0)));
        }
        struct zTriePath z3zE1316;
        zz5vecz8z5bv8z9 z3zE1317;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1317);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1317, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1317, z3zE1317, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE1011 = 0; z8zE1011 < 32; ++z8zE1011) {
          z3zE1316.zdata.bytes[z8zE1011] = (uint8_t)(z3zE1317.data[z8zE1011] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1317);
        z3zE1316.zlen = UINT64_C(0);
        {
          zpath = zpath_append_byte(z3zE1316, z2zE1267);
          if (have_exception) {  goto end_block_exception_1766;  }
        }
      }
      uint64_t zremaining;
      zremaining = zwidth;
      int64_t z3zE1318;
      {    z3zE1318 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1319;
      {    z3zE1319 = (int64_t)(UINT64_C(7));
      }
      int64_t z3zE1320;
      {    z3zE1320 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_offset;
        z_offset = z3zE1318;
        unit z3zE1321;
      for_start_1763: ;
        {
          if ((z3zE1319 < z_offset)) goto for_end_1764;
          bool z2zE1264;
          z2zE1264 = (zremaining != UINT64_C(0));
          if (z2zE1264) {
            uint64_t zbyte_offset;
            zbyte_offset = zrlp_index_byte_width_decrement(zremaining);
            uint64_t zshift;
            {    zshift = (zbyte_offset * UINT64_C(8));
            }
            uint64_t z2zE1265;
            z2zE1265 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zindex, zshift));
            {
              zpath = zpath_append_byte(zpath, z2zE1265);
              if (have_exception) {  goto end_block_exception_1766;  }
            }
            unit z3zE1322;
            z3zE1322 = UNIT;
            zremaining = zbyte_offset;
            z3zE1321 = UNIT;
          } else {  z3zE1321 = UNIT;  }
          z_offset = (z_offset + z3zE1320);
          goto for_start_1763;
        }
      for_end_1764: ;
      }
      unit z3zE1323;
      z3zE1323 = UNIT;
      z8zE547 = zpath;
    }
  }
end_function_1765: ;
  return z8zE547;
end_block_exception_1766: ;
  struct zTriePath z8zE1014 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE1014;
}

struct zRlpIndexCursor zrlp_index_cursor(uint64_t zcount)
{
  struct zRlpIndexCursor z8zE548;
  struct zRlpIndexCursor z3zE1315;
  z3zE1315.zcount = zcount;
  z3zE1315.zposition = UINT64_C(0);
  z8zE548 = z3zE1315;
end_function_1761: ;
  return z8zE548;
end_block_exception_1762: ;
  struct zRlpIndexCursor z8zE1015 = { .zcount = UINT64_C(0xdeadc0de), .zposition = UINT64_C(0xdeadc0de) };
  return z8zE1015;
}

bool zrlp_index_cursor_empty(struct zRlpIndexCursor zcursor)
{
  bool z8zE549;
  uint64_t z2zE1259;
  z2zE1259 = zcursor.zcount;
  uint64_t z2zE1260;
  z2zE1260 = zcursor.zposition;
  z8zE549 = (!(z2zE1260 < z2zE1259));
end_function_1759: ;
  return z8zE549;
end_block_exception_1760: ;

  return false;
}

uint64_t zrlp_index_at_position(struct zRlpIndexCursor zcursor)
{
  uint64_t z8zE550;
  uint64_t zcount;
  zcount = zcursor.zcount;
  uint64_t zposition;
  zposition = zcursor.zposition;
  bool z2zE1254;
  z2zE1254 = (zposition < zcount);
  if (z2zE1254) {
    uint64_t zrest;
    {    zrest = (zcount - UINT64_C(1));
    }
    uint64_t zsingle_count;
    {
      bool z2zE1257;
      z2zE1257 = (zrest < UINT64_C(127));
      if (z2zE1257) {  zsingle_count = zrest;  } else {  zsingle_count = UINT64_C(127);  }
    }
    bool z2zE1255;
    z2zE1255 = (zposition < zsingle_count);
    if (z2zE1255) {
      {    z8zE550 = (zposition + UINT64_C(1));
      }
    } else {
      bool z2zE1256;
      z2zE1256 = (zposition == zsingle_count);
      if (z2zE1256) {  z8zE550 = UINT64_C(0);  } else {  z8zE550 = zposition;  }
    }
  } else {
    struct zexception z2zE1258;
    CREATE(zexception)(&z2zE1258);
    zInvalidBlock(&z2zE1258, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1258);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/indexed.sail:118.8-118.44");
    KILL(zexception)(&z2zE1258);
    goto end_block_exception_1758;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1258);
  }
end_function_1757: ;
  return z8zE550;
end_block_exception_1758: ;

  return UINT64_C(0xdeadc0de);
}

void zrlp_index_cursor_pop(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *z8zE551, struct zRlpIndexCursor zcursor)
{
  uint64_t zcount;
  zcount = zcursor.zcount;
  uint64_t zposition;
  zposition = zcursor.zposition;
  bool z2zE1246;
  z2zE1246 = (zposition < zcount);
  if (z2zE1246) {
    uint64_t zindex;
    {
      zindex = zrlp_index_at_position(zcursor);
      if (have_exception) {  goto end_block_exception_1756;  }
    }
    struct zRlpIndexCursor znext_cursor;
    {
      uint64_t z2zE1252;
      {    z2zE1252 = (zposition + UINT64_C(1));
      }
      struct zRlpIndexCursor z3zE1312;
      z3zE1312.zcount = zcount;
      z3zE1312.zposition = z2zE1252;
      znext_cursor = z3zE1312;
    }
    struct zoptionzIRTriePathzK znext_key;
    CREATE(zoptionzIRTriePathzK)(&znext_key);
    {
      bool z2zE1249;
      z2zE1249 = zrlp_index_cursor_empty(znext_cursor);
      if (z2zE1249) {  zNonezIRTriePathzK(&znext_key, UNIT);  } else {
        struct zTriePath z2zE1251;
        {
          uint64_t z2zE1250;
          {
            z2zE1250 = zrlp_index_at_position(znext_cursor);
            if (have_exception) {
              KILL(zoptionzIRTriePathzK)(&znext_key);
              goto end_block_exception_1756;
            }
          }
          {
            z2zE1251 = ztrie_index_key(z2zE1250);
            if (have_exception) {
              KILL(zoptionzIRTriePathzK)(&znext_key);
              goto end_block_exception_1756;
            }
          }
        }
        zSomezIRTriePathzK(&znext_key, z2zE1251);
      }
    }
    struct zRlpIndexItem z2zE1248;
    CREATE(zRlpIndexItem)(&z2zE1248);
    {
      struct zTriePath z2zE1247;
      {
        z2zE1247 = ztrie_index_key(zindex);
        if (have_exception) {
          KILL(zoptionzIRTriePathzK)(&znext_key);
          KILL(zRlpIndexItem)(&z2zE1248);
          goto end_block_exception_1756;
        }
      }
      z2zE1248.zindex = zindex;
      z2zE1248.zkey = z2zE1247;
      COPY(zoptionzIRTriePathzK)(&((&z2zE1248)->znext_key), znext_key);
    }
    struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 z3zE1314;
    CREATE(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z3zE1314);
    COPY(zRlpIndexItem)(&((&z3zE1314)->ztup0), z2zE1248);
    z3zE1314.ztup1 = znext_cursor;
    COPY(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)((*(&z8zE551)), z3zE1314);
    KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z3zE1314);
    KILL(zRlpIndexItem)(&z2zE1248);
    KILL(zoptionzIRTriePathzK)(&znext_key);
  } else {
    struct zexception z2zE1253;
    CREATE(zexception)(&z2zE1253);
    zInvalidBlock(&z2zE1253, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1253);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/indexed.sail:139.8-139.44");
    KILL(zexception)(&z2zE1253);
    goto end_block_exception_1756;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1253);
  }
end_function_1755: ;
  goto end_function_4025;
end_block_exception_1756: ;
  goto end_function_4025;
end_function_4025: ;
}

