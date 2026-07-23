/* Generated from sail/lib/mpt/nodes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_fixed_bytes_32 zinline_node_hash(struct zInlineNode znode)
{
  sail_fixed_bytes_32 z8zE496;
  zz5listz8z5unionz0zzBytesz9 z2zE1552;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE1552);
  {
    struct zBytes z2zE1551;
    CREATE(zBytes)(&z2zE1551);
    {
      sail_fixed_bytes_32 z2zE1549;
      z2zE1549 = znode.zdata;
      uint64_t z2zE1550;
      z2zE1550 = znode.zlen;
      zbytes_fixed32(&z2zE1551, z2zE1549, z2zE1550);
    }
    zconsz3z5unionz0zzBytes(&z2zE1552, z2zE1551, z2zE1552);
    KILL(zBytes)(&z2zE1551);
  }
  z8zE496 = host_keccak_segments(z2zE1552);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE1552);
end_function_1984: ;
  return z8zE496;
end_block_exception_1985: ;

  return fixed_bytes_32_zero();
}

uint64_t zbranch_content_length_add(uint64_t zcurrent, uint64_t zaddition)
{
  uint64_t z8zE497;
  bool z2zE1547;
  {
    uint64_t z2zE1546;
    {    z2zE1546 = (UINT64_C(529) - zcurrent);
    }
    z2zE1547 = (!(z2zE1546 < zaddition));
  }
  if (z2zE1547) {
    {    z8zE497 = (zcurrent + zaddition);
    }
  } else {
    struct zexception z2zE1548;
    CREATE(zexception)(&z2zE1548);
    zInvalidBlock(&z2zE1548, zRlpDecode);
    COPY(zexception)(current_exception, z2zE1548);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:62.8-62.37");
    KILL(zexception)(&z2zE1548);
    goto end_block_exception_1983;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1548);
  }
end_function_1982: ;
  return z8zE497;
end_block_exception_1983: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t znode_ref_sizze(struct zNodeRef zr)
{
  uint64_t z8zE498;
  uint64_t z3zE1525;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1979;
    z3zE1525 = UINT64_C(1);
    goto finish_match_1976;
  }
case_1979: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1978;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1525 = znode.zlen;
    goto finish_match_1976;
  }
case_1978: ;
  {
    /* complete */
    z3zE1525 = UINT64_C(33);
    goto finish_match_1976;
  }
case_1977: ;
finish_match_1976: ;
  z8zE498 = z3zE1525;
end_function_1980: ;
  return z8zE498;
end_block_exception_1981: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_node_ref(struct zNodeRef zr)
{
  unit z8zE499;
  unit z3zE1520;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1973;
    zz5listz8z5bv8z9 z3zE1523;
    CREATE(zz5listz8z5bv8z9)(&z3zE1523);
    zconsz3z5bv8(&z3zE1523, UINT64_C(0x80), z3zE1523);
    z3zE1520 = zscratch_push_bytes(z3zE1523, UINT64_C(1));
    KILL(zz5listz8z5bv8z9)(&z3zE1523);
    goto finish_match_1970;
  }
case_1973: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1972;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    sail_fixed_bytes_32 z2zE1543;
    z2zE1543 = znode.zdata;
    uint64_t z2zE1544;
    z2zE1544 = znode.zlen;
    z3zE1520 = zscratch_push_b256(z2zE1543, z2zE1544);
    goto finish_match_1970;
  }
case_1972: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    sail_u256 z2zE1545;
    z2zE1545 = zhash_to_word(zh);
    z3zE1520 = zrlp_write_word(z2zE1545);
    goto finish_match_1970;
  }
case_1971: ;
finish_match_1970: ;
  z8zE499 = z3zE1520;
end_function_1974: ;
  return z8zE499;
end_block_exception_1975: ;

  return UNIT;
}

void zchild_ref(struct zNodeRef *z8zE500, struct zByteSliceFields zencoded)
{
  bool z2zE1540;
  {
    uint64_t z2zE1539;
    z2zE1539 = zencoded.zlen;
    z2zE1540 = (z2zE1539 < zMPT_HASH_LENGTH);
  }
  if (z2zE1540) {
    struct zInlineNode z2zE1541;
    {
      z2zE1541 = zinline_node_from_slice(zencoded);
      if (have_exception) {  goto end_block_exception_1969;  }
    }
    zInlineRef((*(&z8zE500)), z2zE1541);
  } else {
    sail_fixed_bytes_32 z2zE1542;
    z2zE1542 = zkeccak256_slice(zencoded);
    zHashRef((*(&z8zE500)), z2zE1542);
  }
end_function_1968: ;
  goto end_function_4058;
end_block_exception_1969: ;
  goto end_function_4058;
end_function_4058: ;
}

uint64_t zbranch_mask_for(uint64_t zindex)
{
  uint64_t z8zE501;
  uint64_t z2zE1538;
  z2zE1538 = ((uint64_t) zindex);
  z8zE501 = ((z2zE1538 >= UINT64_C(64)) ? UINT64_C(0) : ((UINT64_C(0x0001) << z2zE1538) & UINT64_C(0xFFFF)));
end_function_1966: ;
  return z8zE501;
end_block_exception_1967: ;

  return UINT64_C(0xdeadc0de);
}

bool zbranch_mask_has(uint64_t zmask, uint64_t zindex)
{
  bool z8zE502;
  uint64_t z2zE1537;
  {
    uint64_t z2zE1536;
    z2zE1536 = zbranch_mask_for(zindex);
    z2zE1537 = (zmask & z2zE1536);
  }
  z8zE502 = (z2zE1537 != UINT64_C(0x0000));
end_function_1964: ;
  return z8zE502;
end_block_exception_1965: ;

  return false;
}

uint64_t zbranch_mask_set(uint64_t zmask, uint64_t zindex)
{
  uint64_t z8zE503;
  uint64_t z2zE1535;
  z2zE1535 = zbranch_mask_for(zindex);
  z8zE503 = (zmask | z2zE1535);
end_function_1962: ;
  return z8zE503;
end_block_exception_1963: ;

  return UINT64_C(0xdeadc0de);
}

void zleaf_child_ref(struct zNodeRef *z8zE504, struct zTriePath zkey, struct zByteSliceFields zvalue)
{
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE1531;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1531);
  zhex_prefix_compact(&z2zE1531, zkey, true);
  struct zNodeRef z3zE1514;
  CREATE(zNodeRef)(&z3zE1514);
  {
    zz5listz8z5bv8z9 zpath;
    CREATE(zz5listz8z5bv8z9)(&zpath);
    COPY(zz5listz8z5bv8z9)(&zpath, z2zE1531.ztup0);
    uint64_t zencoded_path_len;
    zencoded_path_len = z2zE1531.ztup1;
    uint64_t zcontent_len;
    {
      uint64_t z2zE1533;
      {
        z2zE1533 = zrlp_scratch_bytes_sizze(zpath, zencoded_path_len);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1531);
          KILL(zNodeRef)(&z3zE1514);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1961;
        }
      }
      uint64_t z2zE1534;
      {
        z2zE1534 = zrlp_scratch_slice_sizze(zvalue);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1531);
          KILL(zNodeRef)(&z3zE1514);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1961;
        }
      }
      {
        zcontent_len = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1533, z2zE1534);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1531);
          KILL(zNodeRef)(&z3zE1514);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1961;
        }
      }
    }
    uint64_t zmark;
    zmark = zscratch_begin(UNIT);
    unit z3zE1517;
    z3zE1517 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    unit z3zE1516;
    z3zE1516 = zrlp_write_bytes(zpath, zencoded_path_len);
    unit z3zE1515;
    z3zE1515 = zrlp_write_slice(zvalue);
    struct zNodeRef zresult;
    CREATE(zNodeRef)(&zresult);
    {
      struct zByteSliceFields z2zE1532;
      z2zE1532 = zrlp_finish(zmark);
      {
        zchild_ref(&zresult, z2zE1532);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1531);
          KILL(zNodeRef)(&z3zE1514);
          KILL(zz5listz8z5bv8z9)(&zpath);
          KILL(zNodeRef)(&zresult);
          goto end_block_exception_1961;
        }
      }
    }
    unit z3zE1518;
    z3zE1518 = zscratch_rewind(zmark);
    COPY(zNodeRef)(&z3zE1514, zresult);
    KILL(zNodeRef)(&zresult);
    KILL(zz5listz8z5bv8z9)(&zpath);
    goto finish_match_1958;
  }
case_1959: ;
  sail_match_failure("leaf_child_ref");
finish_match_1958: ;
  COPY(zNodeRef)((*(&z8zE504)), z3zE1514);
  KILL(zNodeRef)(&z3zE1514);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1531);
end_function_1960: ;
  goto end_function_4057;
end_block_exception_1961: ;
  goto end_function_4057;
end_function_4057: ;
}

void zextension_child_ref(struct zNodeRef *z8zE505, struct zTriePath zkey, struct zNodeRef zchildref)
{
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE1529;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1529);
  zhex_prefix_compact(&z2zE1529, zkey, false);
  struct zNodeRef z3zE1508;
  CREATE(zNodeRef)(&z3zE1508);
  {
    zz5listz8z5bv8z9 zpath;
    CREATE(zz5listz8z5bv8z9)(&zpath);
    COPY(zz5listz8z5bv8z9)(&zpath, z2zE1529.ztup0);
    uint64_t zencoded_path_len;
    zencoded_path_len = z2zE1529.ztup1;
    uint64_t zpath_length;
    zpath_length = zrlp_bytes_sizzezIreprzGLB8zCU64zCU64zKzIboundszGed754718030b429454cf327fc49bd29dzK(zpath, zencoded_path_len);
    uint64_t zchild_length;
    zchild_length = znode_ref_sizze(zchildref);
    uint64_t zcontent_len;
    {    zcontent_len = (zpath_length + zchild_length);
    }
    uint64_t zmark;
    zmark = zscratch_begin(UNIT);
    unit z3zE1511;
    z3zE1511 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    unit z3zE1510;
    z3zE1510 = zrlp_write_bytes(zpath, zencoded_path_len);
    unit z3zE1509;
    z3zE1509 = zrlp_write_node_ref(zchildref);
    struct zNodeRef zresult;
    CREATE(zNodeRef)(&zresult);
    {
      struct zByteSliceFields z2zE1530;
      z2zE1530 = zrlp_finish(zmark);
      {
        zchild_ref(&zresult, z2zE1530);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1529);
          KILL(zNodeRef)(&z3zE1508);
          KILL(zz5listz8z5bv8z9)(&zpath);
          KILL(zNodeRef)(&zresult);
          goto end_block_exception_1957;
        }
      }
    }
    unit z3zE1512;
    z3zE1512 = zscratch_rewind(zmark);
    COPY(zNodeRef)(&z3zE1508, zresult);
    KILL(zNodeRef)(&zresult);
    KILL(zz5listz8z5bv8z9)(&zpath);
    goto finish_match_1954;
  }
case_1955: ;
  sail_match_failure("extension_child_ref");
finish_match_1954: ;
  COPY(zNodeRef)((*(&z8zE505)), z3zE1508);
  KILL(zNodeRef)(&z3zE1508);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1529);
end_function_1956: ;
  goto end_function_4056;
end_block_exception_1957: ;
  goto end_function_4056;
end_function_4056: ;
}

void zbranch_child_ref(struct zNodeRef *z8zE506, uint64_t zmask, zz5vecz8z5unionz0zzNodeRefz9 zchildren)
{
  uint64_t zcontent_length;
  zcontent_length = UINT64_C(1);
  uint64_t zchild_bit;
  zchild_bit = UINT64_C(0x0001);
  int64_t z3zE1489;
  {    z3zE1489 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1490;
  {    z3zE1490 = (int64_t)(UINT64_C(15));
  }
  int64_t z3zE1491;
  {    z3zE1491 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zi;
    zi = z3zE1489;
    unit z3zE1493;
  for_start_1948: ;
    {
      if ((z3zE1490 < zi)) goto for_end_1949;
      bool z2zE1523;
      {
        uint64_t z2zE1522;
        z2zE1522 = (zmask & zchild_bit);
        z2zE1523 = (z2zE1522 != UINT64_C(0x0000));
      }
      unit z3zE1492;
      if (z2zE1523) {
        uint64_t zchild_length;
        {
          struct zNodeRef z2zE1524;
          CREATE(zNodeRef)(&z2zE1524);
          fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1524, zchildren, zi);
          zchild_length = znode_ref_sizze(z2zE1524);
          KILL(zNodeRef)(&z2zE1524);
        }
        {
          zcontent_length = zbranch_content_length_add(zcontent_length, zchild_length);
          if (have_exception) {  goto end_block_exception_1953;  }
        }
        z3zE1492 = UNIT;
      } else {
        {
          zcontent_length = zbranch_content_length_add(zcontent_length, UINT64_C(1));
          if (have_exception) {  goto end_block_exception_1953;  }
        }
        z3zE1492 = UNIT;
      }
      zchild_bit = ((UINT64_C(1) >= UINT64_C(64)) ? UINT64_C(0) : ((zchild_bit << UINT64_C(1)) & UINT64_C(0xFFFF)));
      z3zE1493 = UNIT;
      zi = (zi + z3zE1491);
      goto for_start_1948;
    }
  for_end_1949: ;
  }
  unit z3zE1494;
  z3zE1494 = UNIT;
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE1505;
  z3zE1505 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_length);
  zchild_bit = UINT64_C(0x0001);
  unit z3zE1504;
  z3zE1504 = UNIT;
  int64_t z3zE1495;
  {    z3zE1495 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1496;
  {    z3zE1496 = (int64_t)(UINT64_C(15));
  }
  int64_t z3zE1497;
  {    z3zE1497 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z3zE1507;
    z3zE1507 = z3zE1495;
    unit z3zE1500;
  for_start_1950: ;
    {
      if ((z3zE1496 < z3zE1507)) goto for_end_1951;
      bool z2zE1526;
      {
        uint64_t z2zE1525;
        z2zE1525 = (zmask & zchild_bit);
        z2zE1526 = (z2zE1525 != UINT64_C(0x0000));
      }
      unit z3zE1498;
      if (z2zE1526) {
        struct zNodeRef z2zE1527;
        CREATE(zNodeRef)(&z2zE1527);
        fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1527, zchildren, z3zE1507);
        z3zE1498 = zrlp_write_node_ref(z2zE1527);
        KILL(zNodeRef)(&z2zE1527);
      } else {
        zz5listz8z5bv8z9 z3zE1499;
        CREATE(zz5listz8z5bv8z9)(&z3zE1499);
        zconsz3z5bv8(&z3zE1499, UINT64_C(0x80), z3zE1499);
        z3zE1498 = zscratch_push_bytes(z3zE1499, UINT64_C(1));
        KILL(zz5listz8z5bv8z9)(&z3zE1499);
      }
      zchild_bit = ((UINT64_C(1) >= UINT64_C(64)) ? UINT64_C(0) : ((zchild_bit << UINT64_C(1)) & UINT64_C(0xFFFF)));
      z3zE1500 = UNIT;
      z3zE1507 = (z3zE1507 + z3zE1497);
      goto for_start_1950;
    }
  for_end_1951: ;
  }
  unit z3zE1503;
  z3zE1503 = UNIT;
  zz5listz8z5bv8z9 z3zE1501;
  CREATE(zz5listz8z5bv8z9)(&z3zE1501);
  zconsz3z5bv8(&z3zE1501, UINT64_C(0x80), z3zE1501);
  unit z3zE1502;
  z3zE1502 = zscratch_push_bytes(z3zE1501, UINT64_C(1));
  KILL(zz5listz8z5bv8z9)(&z3zE1501);
  struct zNodeRef zresult;
  CREATE(zNodeRef)(&zresult);
  {
    struct zByteSliceFields z2zE1528;
    z2zE1528 = zrlp_finish(zmark);
    {
      zchild_ref(&zresult, z2zE1528);
      if (have_exception) {
        KILL(zNodeRef)(&zresult);
        goto end_block_exception_1953;
      }
    }
  }
  unit z3zE1506;
  z3zE1506 = zscratch_rewind(zmark);
  COPY(zNodeRef)((*(&z8zE506)), zresult);
  KILL(zNodeRef)(&zresult);
end_function_1952: ;
  goto end_function_4055;
end_block_exception_1953: ;
  goto end_function_4055;
end_function_4055: ;
}

sail_fixed_bytes_32 ztrie_ref_to_root(struct zNodeRef zr)
{
  sail_fixed_bytes_32 z8zE507;
  sail_fixed_bytes_32 z3zE1485;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1945;
    z3zE1485 = zEMPTY_TRIE_ROOT;
    goto finish_match_1942;
  }
case_1945: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1944;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1485 = zinline_node_hash(znode);
    goto finish_match_1942;
  }
case_1944: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    z3zE1485 = zh;
    goto finish_match_1942;
  }
case_1943: ;
finish_match_1942: ;
  z8zE507 = z3zE1485;
end_function_1946: ;
  return z8zE507;
end_block_exception_1947: ;

  return fixed_bytes_32_zero();
}

void znode_to_ref(struct zNodeRef *z8zE508, struct zByteSliceFields znode)
{
  bool z2zE1517;
  {
    uint64_t z2zE1516;
    z2zE1516 = znode.zlen;
    z2zE1517 = (z2zE1516 == UINT64_C(0));
  }
  if (z2zE1517) {  zEmptyRef((*(&z8zE508)), UNIT);  } else {
    bool z2zE1519;
    {
      uint64_t z2zE1518;
      z2zE1518 = znode.zlen;
      z2zE1519 = (z2zE1518 < zMPT_HASH_LENGTH);
    }
    if (z2zE1519) {
      struct zInlineNode z2zE1520;
      {
        z2zE1520 = zinline_node_from_slice(znode);
        if (have_exception) {  goto end_block_exception_1941;  }
      }
      zInlineRef((*(&z8zE508)), z2zE1520);
    } else {
      sail_fixed_bytes_32 z2zE1521;
      z2zE1521 = zkeccak256_slice(znode);
      zHashRef((*(&z8zE508)), z2zE1521);
    }
  }
end_function_1940: ;
  goto end_function_4054;
end_block_exception_1941: ;
  goto end_function_4054;
end_function_4054: ;
}

void zmerge_ext_node(struct zNodeRef *z8zE509, struct zTriePath zprefix, struct zByteSliceFields zchildnode)
{
  bool z2zE1503;
  {
    uint64_t z2zE1502;
    z2zE1502 = zpath_len(zprefix);
    z2zE1503 = (z2zE1502 == UINT64_C(0));
  }
  if (z2zE1503) {
    {
      znode_to_ref((*(&z8zE509)), zchildnode);
      if (have_exception) {  goto end_block_exception_1939;  }
    }
  } else {
    bool z2zE1505;
    {
      uint64_t z2zE1504;
      z2zE1504 = zchildnode.zlen;
      z2zE1505 = (z2zE1504 == UINT64_C(0));
    }
    if (z2zE1505) {  zEmptyRef((*(&z8zE509)), UNIT);  } else {
      struct zTrieNode z2zE1506;
      CREATE(zTrieNode)(&z2zE1506);
      {
        zdecode_trie_node(&z2zE1506, zchildnode);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE1506);
          goto end_block_exception_1939;
        }
      }
      struct zNodeRef z3zE1481;
      CREATE(zNodeRef)(&z3zE1481);
      {
        if (z2zE1506.kind != Kind_zLeafNode) goto case_1937;
        struct zLeafNodeData zleaf;
        zleaf = z2zE1506.variants.zLeafNode;
        struct zTriePath z2zE1509;
        {
          struct zTriePath z2zE1507;
          z2zE1507 = zleaf.zpath;
          {
            z2zE1509 = zpath_concat(zprefix, z2zE1507);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE1506);
              KILL(zNodeRef)(&z3zE1481);
              goto end_block_exception_1939;
            }
          }
        }
        struct zByteSliceFields z2zE1510;
        {
          struct zRlpFieldRefFields z2zE1508;
          z2zE1508 = zleaf.zvalue;
          z2zE1510 = zrlp_ref_content(z2zE1508);
        }
        {
          zleaf_child_ref(&z3zE1481, z2zE1509, z2zE1510);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1506);
            KILL(zNodeRef)(&z3zE1481);
            goto end_block_exception_1939;
          }
        }
        goto finish_match_1934;
      }
    case_1937: ;
      {
        if (z2zE1506.kind != Kind_zExtensionNode) goto case_1936;
        struct zExtensionNodeData zext;
        zext = z2zE1506.variants.zExtensionNode;
        struct zTriePath z2zE1513;
        {
          struct zTriePath z2zE1511;
          z2zE1511 = zext.zpath;
          {
            z2zE1513 = zpath_concat(zprefix, z2zE1511);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE1506);
              KILL(zNodeRef)(&z3zE1481);
              goto end_block_exception_1939;
            }
          }
        }
        struct zNodeRef z2zE1514;
        CREATE(zNodeRef)(&z2zE1514);
        {
          struct zRlpFieldRefFields z2zE1512;
          z2zE1512 = zext.zchild;
          {
            zfield_to_ref(&z2zE1514, z2zE1512);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE1506);
              KILL(zNodeRef)(&z3zE1481);
              KILL(zNodeRef)(&z2zE1514);
              goto end_block_exception_1939;
            }
          }
        }
        {
          zextension_child_ref(&z3zE1481, z2zE1513, z2zE1514);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1506);
            KILL(zNodeRef)(&z3zE1481);
            KILL(zNodeRef)(&z2zE1514);
            goto end_block_exception_1939;
          }
        }
        KILL(zNodeRef)(&z2zE1514);
        goto finish_match_1934;
      }
    case_1936: ;
      {
        struct zNodeRef z2zE1515;
        CREATE(zNodeRef)(&z2zE1515);
        {
          znode_to_ref(&z2zE1515, zchildnode);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1506);
            KILL(zNodeRef)(&z3zE1481);
            KILL(zNodeRef)(&z2zE1515);
            goto end_block_exception_1939;
          }
        }
        {
          zextension_child_ref(&z3zE1481, zprefix, z2zE1515);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1506);
            KILL(zNodeRef)(&z3zE1481);
            KILL(zNodeRef)(&z2zE1515);
            goto end_block_exception_1939;
          }
        }
        KILL(zNodeRef)(&z2zE1515);
        goto finish_match_1934;
      }
    case_1935: ;
    finish_match_1934: ;
      COPY(zNodeRef)((*(&z8zE509)), z3zE1481);
      KILL(zNodeRef)(&z3zE1481);
      KILL(zTrieNode)(&z2zE1506);
    }
  }
end_function_1938: ;
  goto end_function_4053;
end_block_exception_1939: ;
  goto end_function_4053;
end_function_4053: ;
}

void zmerge_ext_ref(struct zNodeRef *z8zE510, struct zTriePath zprefix, struct zNodeRef zchildref)
{
  bool z2zE1491;
  {
    uint64_t z2zE1490;
    z2zE1490 = zpath_len(zprefix);
    z2zE1491 = (z2zE1490 == UINT64_C(0));
  }
  if (z2zE1491) {  COPY(zNodeRef)((*(&z8zE510)), zchildref);  } else {
    struct zNodeRef z3zE1473;
    CREATE(zNodeRef)(&z3zE1473);
    {
      if (zchildref.kind != Kind_zEmptyRef) goto case_1931;
      zEmptyRef(&z3zE1473, UNIT);
      goto finish_match_1924;
    }
  case_1931: ;
    {
      if (zchildref.kind != Kind_zHashRef) goto case_1930;
      {
        zextension_child_ref(&z3zE1473, zprefix, zchildref);
        if (have_exception) {
          KILL(zNodeRef)(&z3zE1473);
          goto end_block_exception_1933;
        }
      }
      goto finish_match_1924;
    }
  case_1930: ;
    {
      /* complete */
      struct zInlineNode znode;
      znode = zchildref.variants.zInlineRef;
      struct zTrieNode z2zE1493;
      CREATE(zTrieNode)(&z2zE1493);
      {
        struct zByteSliceFields z2zE1492;
        z2zE1492 = zinline_node_slice(znode);
        {
          zdecode_trie_node(&z2zE1493, z2zE1492);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1473);
            KILL(zTrieNode)(&z2zE1493);
            goto end_block_exception_1933;
          }
        }
      }
      struct zNodeRef z3zE1474;
      CREATE(zNodeRef)(&z3zE1474);
      {
        if (z2zE1493.kind != Kind_zLeafNode) goto case_1929;
        struct zLeafNodeData zleaf;
        zleaf = z2zE1493.variants.zLeafNode;
        struct zTriePath z2zE1496;
        {
          struct zTriePath z2zE1494;
          z2zE1494 = zleaf.zpath;
          {
            z2zE1496 = zpath_concat(zprefix, z2zE1494);
            if (have_exception) {
              KILL(zNodeRef)(&z3zE1473);
              KILL(zTrieNode)(&z2zE1493);
              KILL(zNodeRef)(&z3zE1474);
              goto end_block_exception_1933;
            }
          }
        }
        struct zByteSliceFields z2zE1497;
        {
          struct zRlpFieldRefFields z2zE1495;
          z2zE1495 = zleaf.zvalue;
          z2zE1497 = zrlp_ref_content(z2zE1495);
        }
        {
          zleaf_child_ref(&z3zE1474, z2zE1496, z2zE1497);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1473);
            KILL(zTrieNode)(&z2zE1493);
            KILL(zNodeRef)(&z3zE1474);
            goto end_block_exception_1933;
          }
        }
        goto finish_match_1926;
      }
    case_1929: ;
      {
        if (z2zE1493.kind != Kind_zExtensionNode) goto case_1928;
        struct zExtensionNodeData zext;
        zext = z2zE1493.variants.zExtensionNode;
        struct zTriePath z2zE1500;
        {
          struct zTriePath z2zE1498;
          z2zE1498 = zext.zpath;
          {
            z2zE1500 = zpath_concat(zprefix, z2zE1498);
            if (have_exception) {
              KILL(zNodeRef)(&z3zE1473);
              KILL(zTrieNode)(&z2zE1493);
              KILL(zNodeRef)(&z3zE1474);
              goto end_block_exception_1933;
            }
          }
        }
        struct zNodeRef z2zE1501;
        CREATE(zNodeRef)(&z2zE1501);
        {
          struct zRlpFieldRefFields z2zE1499;
          z2zE1499 = zext.zchild;
          {
            zfield_to_ref(&z2zE1501, z2zE1499);
            if (have_exception) {
              KILL(zNodeRef)(&z3zE1473);
              KILL(zTrieNode)(&z2zE1493);
              KILL(zNodeRef)(&z3zE1474);
              KILL(zNodeRef)(&z2zE1501);
              goto end_block_exception_1933;
            }
          }
        }
        {
          zextension_child_ref(&z3zE1474, z2zE1500, z2zE1501);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1473);
            KILL(zTrieNode)(&z2zE1493);
            KILL(zNodeRef)(&z3zE1474);
            KILL(zNodeRef)(&z2zE1501);
            goto end_block_exception_1933;
          }
        }
        KILL(zNodeRef)(&z2zE1501);
        goto finish_match_1926;
      }
    case_1928: ;
      {
        {
          zextension_child_ref(&z3zE1474, zprefix, zchildref);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1473);
            KILL(zTrieNode)(&z2zE1493);
            KILL(zNodeRef)(&z3zE1474);
            goto end_block_exception_1933;
          }
        }
        goto finish_match_1926;
      }
    case_1927: ;
    finish_match_1926: ;
      COPY(zNodeRef)(&z3zE1473, z3zE1474);
      KILL(zNodeRef)(&z3zE1474);
      KILL(zTrieNode)(&z2zE1493);
      goto finish_match_1924;
    }
  case_1925: ;
  finish_match_1924: ;
    COPY(zNodeRef)((*(&z8zE510)), z3zE1473);
    KILL(zNodeRef)(&z3zE1473);
  }
end_function_1932: ;
  goto end_function_4052;
end_block_exception_1933: ;
  goto end_function_4052;
end_function_4052: ;
}

