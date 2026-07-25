/* Generated from sail/lib/mpt/indexed.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zrlp_index_byte_width_decrement(uint64_t zvalue)
{
  uint64_t z8zE542;
  bool z2zE1264;
  z2zE1264 = (UINT64_C(0) < zvalue);
  unit z3zE1330;
  z3zE1330 = sail_assert(z2zE1264, "sail/lib/mpt/indexed.sail:33.20-33.21");
  {    z8zE542 = (zvalue - UINT64_C(1));
  }
end_function_1765: ;
  return z8zE542;
end_block_exception_1766: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_index_encoded_width(uint64_t zvalue)
{
  uint64_t z8zE543;
  bool z2zE1257;
  z2zE1257 = (zvalue < UINT64_C(256));
  if (z2zE1257) {  z8zE543 = UINT64_C(1);  } else {
    bool z2zE1258;
    z2zE1258 = (zvalue < UINT64_C(65536));
    if (z2zE1258) {  z8zE543 = UINT64_C(2);  } else {
      bool z2zE1259;
      z2zE1259 = (zvalue < UINT64_C(16777216));
      if (z2zE1259) {  z8zE543 = UINT64_C(3);  } else {
        bool z2zE1260;
        z2zE1260 = (zvalue < UINT64_C(4294967296));
        if (z2zE1260) {  z8zE543 = UINT64_C(4);  } else {
          bool z2zE1261;
          z2zE1261 = (zvalue < UINT64_C(1099511627776));
          if (z2zE1261) {  z8zE543 = UINT64_C(5);  } else {
            bool z2zE1262;
            z2zE1262 = (zvalue < UINT64_C(281474976710656));
            if (z2zE1262) {  z8zE543 = UINT64_C(6);  } else {
              bool z2zE1263;
              z2zE1263 = (zvalue < UINT64_C(72057594037927936));
              if (z2zE1263) {  z8zE543 = UINT64_C(7);  } else {  z8zE543 = UINT64_C(8);  }
            }
          }
        }
      }
    }
  }
end_function_1763: ;
  return z8zE543;
end_block_exception_1764: ;

  return UINT64_C(0xdeadc0de);
}

struct zTriePath ztrie_index_key(uint64_t zindex)
{
  struct zTriePath z8zE544;
  bool z2zE1250;
  z2zE1250 = (zindex == UINT64_C(0));
  if (z2zE1250) {
    struct zTriePath z3zE1328;
    zz5vecz8z5bv8z9 z3zE1329;
    CREATE(zz5vecz8z5bv8z9)(&z3zE1329);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE1329, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE1329, z3zE1329, INT64_C(31), UINT64_C(0x80));
    for (size_t z8zE1001 = 0; z8zE1001 < 32; ++z8zE1001) {
      z3zE1328.zdata.bytes[z8zE1001] = (uint8_t)(z3zE1329.data[z8zE1001] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE1329);
    z3zE1328.zlen = UINT64_C(2);
    z8zE544 = z3zE1328;
  } else {
    bool z2zE1251;
    z2zE1251 = (!(UINT64_C(127) < zindex));
    if (z2zE1251) {
      uint64_t z2zE1252;
      z2zE1252 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zindex, UINT64_C(0)));
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
      internal_vector_update_zz5vecz8z5bv8z9(&z3zE1327, z3zE1327, INT64_C(31), UINT64_C(0x00));
      for (size_t z8zE1000 = 0; z8zE1000 < 32; ++z8zE1000) {
        z3zE1326.zdata.bytes[z8zE1000] = (uint8_t)(z3zE1327.data[z8zE1000] & UINT64_C(0xff));
      }
      KILL(zz5vecz8z5bv8z9)(&z3zE1327);
      z3zE1326.zlen = UINT64_C(0);
      {
        z8zE544 = zpath_append_byte(z3zE1326, z2zE1252);
        if (have_exception) {  goto end_block_exception_1762;  }
      }
    } else {
      uint64_t zwidth;
      zwidth = zrlp_index_encoded_width(zindex);
      struct zTriePath zpath;
      {
        uint64_t z2zE1256;
        {
          uint64_t z2zE1255;
          {    z2zE1255 = (UINT64_C(128) + zwidth);
          }
          z2zE1256 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(z2zE1255, UINT64_C(0)));
        }
        struct zTriePath z3zE1318;
        zz5vecz8z5bv8z9 z3zE1319;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1319);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1319, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1319, z3zE1319, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE999 = 0; z8zE999 < 32; ++z8zE999) {
          z3zE1318.zdata.bytes[z8zE999] = (uint8_t)(z3zE1319.data[z8zE999] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1319);
        z3zE1318.zlen = UINT64_C(0);
        {
          zpath = zpath_append_byte(z3zE1318, z2zE1256);
          if (have_exception) {  goto end_block_exception_1762;  }
        }
      }
      uint64_t zremaining;
      zremaining = zwidth;
      int64_t z3zE1320;
      {    z3zE1320 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1321;
      {    z3zE1321 = (int64_t)(UINT64_C(7));
      }
      int64_t z3zE1322;
      {    z3zE1322 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t z_offset;
        z_offset = z3zE1320;
        unit z3zE1323;
      for_start_1759: ;
        {
          if ((z3zE1321 < z_offset)) goto for_end_1760;
          bool z2zE1253;
          z2zE1253 = (zremaining != UINT64_C(0));
          if (z2zE1253) {
            uint64_t zbyte_offset;
            zbyte_offset = zrlp_index_byte_width_decrement(zremaining);
            uint64_t zshift;
            {    zshift = (zbyte_offset * UINT64_C(8));
            }
            uint64_t z2zE1254;
            z2zE1254 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zindex, zshift));
            {
              zpath = zpath_append_byte(zpath, z2zE1254);
              if (have_exception) {  goto end_block_exception_1762;  }
            }
            unit z3zE1324;
            z3zE1324 = UNIT;
            zremaining = zbyte_offset;
            z3zE1323 = UNIT;
          } else {  z3zE1323 = UNIT;  }
          z_offset = (z_offset + z3zE1322);
          goto for_start_1759;
        }
      for_end_1760: ;
      }
      unit z3zE1325;
      z3zE1325 = UNIT;
      z8zE544 = zpath;
    }
  }
end_function_1761: ;
  return z8zE544;
end_block_exception_1762: ;
  struct zTriePath z8zE1002 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE1002;
}

struct zRlpIndexCursor zrlp_index_cursor(uint64_t zcount)
{
  struct zRlpIndexCursor z8zE545;
  struct zRlpIndexCursor z3zE1317;
  z3zE1317.zcount = zcount;
  z3zE1317.zposition = UINT64_C(0);
  z8zE545 = z3zE1317;
end_function_1757: ;
  return z8zE545;
end_block_exception_1758: ;
  struct zRlpIndexCursor z8zE1003 = { .zcount = UINT64_C(0xdeadc0de), .zposition = UINT64_C(0xdeadc0de) };
  return z8zE1003;
}

bool zrlp_index_cursor_empty(struct zRlpIndexCursor zcursor)
{
  bool z8zE546;
  uint64_t z2zE1248;
  z2zE1248 = zcursor.zcount;
  uint64_t z2zE1249;
  z2zE1249 = zcursor.zposition;
  z8zE546 = (!(z2zE1249 < z2zE1248));
end_function_1755: ;
  return z8zE546;
end_block_exception_1756: ;

  return false;
}

uint64_t zrlp_index_at_position(struct zRlpIndexCursor zcursor)
{
  uint64_t z8zE547;
  uint64_t zcount;
  zcount = zcursor.zcount;
  uint64_t zposition;
  zposition = zcursor.zposition;
  bool z2zE1243;
  z2zE1243 = (zposition < zcount);
  if (z2zE1243) {
    uint64_t zrest;
    {    zrest = (zcount - UINT64_C(1));
    }
    uint64_t zsingle_count;
    {
      bool z2zE1246;
      z2zE1246 = (zrest < UINT64_C(127));
      if (z2zE1246) {  zsingle_count = zrest;  } else {  zsingle_count = UINT64_C(127);  }
    }
    bool z2zE1244;
    z2zE1244 = (zposition < zsingle_count);
    if (z2zE1244) {
      {    z8zE547 = (zposition + UINT64_C(1));
      }
    } else {
      bool z2zE1245;
      z2zE1245 = (zposition == zsingle_count);
      if (z2zE1245) {  z8zE547 = UINT64_C(0);  } else {  z8zE547 = zposition;  }
    }
  } else {
    struct zexception z2zE1247;
    CREATE(zexception)(&z2zE1247);
    zInvalidBlock(&z2zE1247, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1247);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/indexed.sail:118.8-118.44");
    KILL(zexception)(&z2zE1247);
    goto end_block_exception_1754;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1247);
  }
end_function_1753: ;
  return z8zE547;
end_block_exception_1754: ;

  return UINT64_C(0xdeadc0de);
}

void zrlp_index_cursor_pop(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *z8zE548, struct zRlpIndexCursor zcursor)
{
  uint64_t zcount;
  zcount = zcursor.zcount;
  uint64_t zposition;
  zposition = zcursor.zposition;
  bool z2zE1235;
  z2zE1235 = (zposition < zcount);
  if (z2zE1235) {
    uint64_t zindex;
    {
      zindex = zrlp_index_at_position(zcursor);
      if (have_exception) {  goto end_block_exception_1752;  }
    }
    struct zRlpIndexCursor znext_cursor;
    {
      uint64_t z2zE1241;
      {    z2zE1241 = (zposition + UINT64_C(1));
      }
      struct zRlpIndexCursor z3zE1314;
      z3zE1314.zcount = zcount;
      z3zE1314.zposition = z2zE1241;
      znext_cursor = z3zE1314;
    }
    struct zoptionzIRTriePathzK znext_key;
    CREATE(zoptionzIRTriePathzK)(&znext_key);
    {
      bool z2zE1238;
      z2zE1238 = zrlp_index_cursor_empty(znext_cursor);
      if (z2zE1238) {  zNonezIRTriePathzK(&znext_key, UNIT);  } else {
        struct zTriePath z2zE1240;
        {
          uint64_t z2zE1239;
          {
            z2zE1239 = zrlp_index_at_position(znext_cursor);
            if (have_exception) {
              KILL(zoptionzIRTriePathzK)(&znext_key);
              goto end_block_exception_1752;
            }
          }
          {
            z2zE1240 = ztrie_index_key(z2zE1239);
            if (have_exception) {
              KILL(zoptionzIRTriePathzK)(&znext_key);
              goto end_block_exception_1752;
            }
          }
        }
        zSomezIRTriePathzK(&znext_key, z2zE1240);
      }
    }
    struct zRlpIndexItem z2zE1237;
    CREATE(zRlpIndexItem)(&z2zE1237);
    {
      struct zTriePath z2zE1236;
      {
        z2zE1236 = ztrie_index_key(zindex);
        if (have_exception) {
          KILL(zoptionzIRTriePathzK)(&znext_key);
          KILL(zRlpIndexItem)(&z2zE1237);
          goto end_block_exception_1752;
        }
      }
      z2zE1237.zindex = zindex;
      z2zE1237.zkey = z2zE1236;
      COPY(zoptionzIRTriePathzK)(&((&z2zE1237)->znext_key), znext_key);
    }
    struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 z3zE1316;
    CREATE(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z3zE1316);
    COPY(zRlpIndexItem)(&((&z3zE1316)->ztup0), z2zE1237);
    z3zE1316.ztup1 = znext_cursor;
    COPY(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)((*(&z8zE548)), z3zE1316);
    KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z3zE1316);
    KILL(zRlpIndexItem)(&z2zE1237);
    KILL(zoptionzIRTriePathzK)(&znext_key);
  } else {
    struct zexception z2zE1242;
    CREATE(zexception)(&z2zE1242);
    zInvalidBlock(&z2zE1242, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1242);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/indexed.sail:139.8-139.44");
    KILL(zexception)(&z2zE1242);
    goto end_block_exception_1752;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1242);
  }
end_function_1751: ;
  goto end_function_4006;
end_block_exception_1752: ;
  goto end_function_4006;
end_function_4006: ;
}

