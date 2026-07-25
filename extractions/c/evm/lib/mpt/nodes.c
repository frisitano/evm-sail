/* Generated from sail/lib/mpt/nodes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_fixed_bytes_32 zinline_node_hash(struct zInlineNode znode)
{
  sail_fixed_bytes_32 z8zE493;
  zz5listz8z5unionz0zzBytesz9 z2zE1529;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE1529);
  {
    struct zBytes z2zE1528;
    CREATE(zBytes)(&z2zE1528);
    {
      sail_fixed_bytes_32 z2zE1526;
      z2zE1526 = znode.zdata;
      uint64_t z2zE1527;
      z2zE1527 = znode.zlen;
      zbytes_fixed32(&z2zE1528, z2zE1526, z2zE1527);
    }
    zconsz3z5unionz0zzBytes(&z2zE1529, z2zE1528, z2zE1529);
    KILL(zBytes)(&z2zE1528);
  }
  z8zE493 = host_keccak_segments(z2zE1529);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE1529);
end_function_1980: ;
  return z8zE493;
end_block_exception_1981: ;

  return fixed_bytes_32_zero();
}

uint64_t zbranch_content_length_add(uint64_t zcurrent, uint64_t zaddition)
{
  uint64_t z8zE494;
  bool z2zE1524;
  {
    uint64_t z2zE1523;
    {    z2zE1523 = (UINT64_C(529) - zcurrent);
    }
    z2zE1524 = (!(z2zE1523 < zaddition));
  }
  if (z2zE1524) {
    {    z8zE494 = (zcurrent + zaddition);
    }
  } else {
    struct zexception z2zE1525;
    CREATE(zexception)(&z2zE1525);
    zInvalidBlock(&z2zE1525, zRlpDecode);
    COPY(zexception)(current_exception, z2zE1525);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:62.8-62.37");
    KILL(zexception)(&z2zE1525);
    goto end_block_exception_1979;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1525);
  }
end_function_1978: ;
  return z8zE494;
end_block_exception_1979: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t znode_ref_sizze(struct zNodeRef zr)
{
  uint64_t z8zE495;
  uint64_t z3zE1529;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1975;
    z3zE1529 = UINT64_C(1);
    goto finish_match_1972;
  }
case_1975: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1974;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1529 = znode.zlen;
    goto finish_match_1972;
  }
case_1974: ;
  {
    /* complete */
    z3zE1529 = UINT64_C(33);
    goto finish_match_1972;
  }
case_1973: ;
finish_match_1972: ;
  z8zE495 = z3zE1529;
end_function_1976: ;
  return z8zE495;
end_block_exception_1977: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_node_ref(struct zNodeRef zr)
{
  unit z8zE496;
  unit z3zE1524;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1969;
    zz5listz8z5bv8z9 z3zE1527;
    CREATE(zz5listz8z5bv8z9)(&z3zE1527);
    zconsz3z5bv8(&z3zE1527, UINT64_C(0x80), z3zE1527);
    z3zE1524 = zscratch_push_bytes(z3zE1527, UINT64_C(1));
    KILL(zz5listz8z5bv8z9)(&z3zE1527);
    goto finish_match_1966;
  }
case_1969: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1968;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    sail_fixed_bytes_32 z2zE1520;
    z2zE1520 = znode.zdata;
    uint64_t z2zE1521;
    z2zE1521 = znode.zlen;
    z3zE1524 = zscratch_push_b256(z2zE1520, z2zE1521);
    goto finish_match_1966;
  }
case_1968: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    sail_u256 z2zE1522;
    z2zE1522 = zhash_to_word(zh);
    z3zE1524 = zrlp_write_word(z2zE1522);
    goto finish_match_1966;
  }
case_1967: ;
finish_match_1966: ;
  z8zE496 = z3zE1524;
end_function_1970: ;
  return z8zE496;
end_block_exception_1971: ;

  return UNIT;
}

void zchild_ref(struct zNodeRef *z8zE497, struct zByteSliceFields zencoded)
{
  bool z2zE1517;
  {
    uint64_t z2zE1516;
    z2zE1516 = zencoded.zlen;
    z2zE1517 = (z2zE1516 < zMPT_HASH_LENGTH);
  }
  if (z2zE1517) {
    struct zInlineNode z2zE1518;
    {
      z2zE1518 = zinline_node_from_slice(zencoded);
      if (have_exception) {  goto end_block_exception_1965;  }
    }
    zInlineRef((*(&z8zE497)), z2zE1518);
  } else {
    sail_fixed_bytes_32 z2zE1519;
    z2zE1519 = zkeccak256_slice(zencoded);
    zHashRef((*(&z8zE497)), z2zE1519);
  }
end_function_1964: ;
  goto end_function_4039;
end_block_exception_1965: ;
  goto end_function_4039;
end_function_4039: ;
}

uint64_t zbranch_mask_for(uint64_t zindex)
{
  uint64_t z8zE498;
  uint64_t z2zE1515;
  z2zE1515 = ((uint64_t) zindex);
  z8zE498 = ((z2zE1515 >= UINT64_C(64)) ? UINT64_C(0) : ((UINT64_C(0x0001) << z2zE1515) & UINT64_C(0xFFFF)));
end_function_1962: ;
  return z8zE498;
end_block_exception_1963: ;

  return UINT64_C(0xdeadc0de);
}

bool zbranch_mask_has(uint64_t zmask, uint64_t zindex)
{
  bool z8zE499;
  uint64_t z2zE1514;
  {
    uint64_t z2zE1513;
    z2zE1513 = zbranch_mask_for(zindex);
    z2zE1514 = (zmask & z2zE1513);
  }
  z8zE499 = (z2zE1514 != UINT64_C(0x0000));
end_function_1960: ;
  return z8zE499;
end_block_exception_1961: ;

  return false;
}

uint64_t zbranch_mask_set(uint64_t zmask, uint64_t zindex)
{
  uint64_t z8zE500;
  uint64_t z2zE1512;
  z2zE1512 = zbranch_mask_for(zindex);
  z8zE500 = (zmask | z2zE1512);
end_function_1958: ;
  return z8zE500;
end_block_exception_1959: ;

  return UINT64_C(0xdeadc0de);
}

void zleaf_child_ref(struct zNodeRef *z8zE501, struct zTriePath zkey, struct zByteSliceFields zvalue)
{
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE1508;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1508);
  zhex_prefix_compact(&z2zE1508, zkey, true);
  struct zNodeRef z3zE1518;
  CREATE(zNodeRef)(&z3zE1518);
  {
    zz5listz8z5bv8z9 zpath;
    CREATE(zz5listz8z5bv8z9)(&zpath);
    COPY(zz5listz8z5bv8z9)(&zpath, z2zE1508.ztup0);
    uint64_t zencoded_path_len;
    zencoded_path_len = z2zE1508.ztup1;
    uint64_t zcontent_len;
    {
      uint64_t z2zE1510;
      {
        z2zE1510 = zrlp_scratch_bytes_sizze(zpath, zencoded_path_len);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1508);
          KILL(zNodeRef)(&z3zE1518);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1957;
        }
      }
      uint64_t z2zE1511;
      {
        z2zE1511 = zrlp_scratch_slice_sizze(zvalue);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1508);
          KILL(zNodeRef)(&z3zE1518);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1957;
        }
      }
      {
        zcontent_len = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1510, z2zE1511);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1508);
          KILL(zNodeRef)(&z3zE1518);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1957;
        }
      }
    }
    uint64_t zmark;
    zmark = zscratch_begin(UNIT);
    unit z3zE1521;
    z3zE1521 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    unit z3zE1520;
    z3zE1520 = zrlp_write_bytes(zpath, zencoded_path_len);
    unit z3zE1519;
    z3zE1519 = zrlp_write_slice(zvalue);
    struct zNodeRef zresult;
    CREATE(zNodeRef)(&zresult);
    {
      struct zByteSliceFields z2zE1509;
      z2zE1509 = zrlp_finish(zmark);
      {
        zchild_ref(&zresult, z2zE1509);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1508);
          KILL(zNodeRef)(&z3zE1518);
          KILL(zz5listz8z5bv8z9)(&zpath);
          KILL(zNodeRef)(&zresult);
          goto end_block_exception_1957;
        }
      }
    }
    unit z3zE1522;
    z3zE1522 = zscratch_rewind(zmark);
    COPY(zNodeRef)(&z3zE1518, zresult);
    KILL(zNodeRef)(&zresult);
    KILL(zz5listz8z5bv8z9)(&zpath);
    goto finish_match_1954;
  }
case_1955: ;
  sail_match_failure("leaf_child_ref");
finish_match_1954: ;
  COPY(zNodeRef)((*(&z8zE501)), z3zE1518);
  KILL(zNodeRef)(&z3zE1518);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1508);
end_function_1956: ;
  goto end_function_4038;
end_block_exception_1957: ;
  goto end_function_4038;
end_function_4038: ;
}

void zextension_child_ref(struct zNodeRef *z8zE502, struct zTriePath zkey, struct zNodeRef zchildref)
{
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE1506;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1506);
  zhex_prefix_compact(&z2zE1506, zkey, false);
  struct zNodeRef z3zE1512;
  CREATE(zNodeRef)(&z3zE1512);
  {
    zz5listz8z5bv8z9 zpath;
    CREATE(zz5listz8z5bv8z9)(&zpath);
    COPY(zz5listz8z5bv8z9)(&zpath, z2zE1506.ztup0);
    uint64_t zencoded_path_len;
    zencoded_path_len = z2zE1506.ztup1;
    uint64_t zpath_length;
    zpath_length = zrlp_bytes_sizzezIreprzGLB8zCU64zCU64zKzIboundszGed754718030b429454cf327fc49bd29dzK(zpath, zencoded_path_len);
    uint64_t zchild_length;
    zchild_length = znode_ref_sizze(zchildref);
    uint64_t zcontent_len;
    {    zcontent_len = (zpath_length + zchild_length);
    }
    uint64_t zmark;
    zmark = zscratch_begin(UNIT);
    unit z3zE1515;
    z3zE1515 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    unit z3zE1514;
    z3zE1514 = zrlp_write_bytes(zpath, zencoded_path_len);
    unit z3zE1513;
    z3zE1513 = zrlp_write_node_ref(zchildref);
    struct zNodeRef zresult;
    CREATE(zNodeRef)(&zresult);
    {
      struct zByteSliceFields z2zE1507;
      z2zE1507 = zrlp_finish(zmark);
      {
        zchild_ref(&zresult, z2zE1507);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1506);
          KILL(zNodeRef)(&z3zE1512);
          KILL(zz5listz8z5bv8z9)(&zpath);
          KILL(zNodeRef)(&zresult);
          goto end_block_exception_1953;
        }
      }
    }
    unit z3zE1516;
    z3zE1516 = zscratch_rewind(zmark);
    COPY(zNodeRef)(&z3zE1512, zresult);
    KILL(zNodeRef)(&zresult);
    KILL(zz5listz8z5bv8z9)(&zpath);
    goto finish_match_1950;
  }
case_1951: ;
  sail_match_failure("extension_child_ref");
finish_match_1950: ;
  COPY(zNodeRef)((*(&z8zE502)), z3zE1512);
  KILL(zNodeRef)(&z3zE1512);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1506);
end_function_1952: ;
  goto end_function_4037;
end_block_exception_1953: ;
  goto end_function_4037;
end_function_4037: ;
}

void zbranch_child_ref(struct zNodeRef *z8zE503, uint64_t zmask, zz5vecz8z5unionz0zzNodeRefz9 zchildren)
{
  uint64_t zcontent_length;
  zcontent_length = UINT64_C(1);
  uint64_t zchild_bit;
  zchild_bit = UINT64_C(0x0001);
  int64_t z3zE1493;
  {    z3zE1493 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1494;
  {    z3zE1494 = (int64_t)(UINT64_C(15));
  }
  int64_t z3zE1495;
  {    z3zE1495 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zi;
    zi = z3zE1493;
    unit z3zE1497;
  for_start_1944: ;
    {
      if ((z3zE1494 < zi)) goto for_end_1945;
      bool z2zE1500;
      {
        uint64_t z2zE1499;
        z2zE1499 = (zmask & zchild_bit);
        z2zE1500 = (z2zE1499 != UINT64_C(0x0000));
      }
      unit z3zE1496;
      if (z2zE1500) {
        uint64_t zchild_length;
        {
          struct zNodeRef z2zE1501;
          CREATE(zNodeRef)(&z2zE1501);
          fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1501, zchildren, zi);
          zchild_length = znode_ref_sizze(z2zE1501);
          KILL(zNodeRef)(&z2zE1501);
        }
        {
          zcontent_length = zbranch_content_length_add(zcontent_length, zchild_length);
          if (have_exception) {  goto end_block_exception_1949;  }
        }
        z3zE1496 = UNIT;
      } else {
        {
          zcontent_length = zbranch_content_length_add(zcontent_length, UINT64_C(1));
          if (have_exception) {  goto end_block_exception_1949;  }
        }
        z3zE1496 = UNIT;
      }
      zchild_bit = ((UINT64_C(1) >= UINT64_C(64)) ? UINT64_C(0) : ((zchild_bit << UINT64_C(1)) & UINT64_C(0xFFFF)));
      z3zE1497 = UNIT;
      zi = (zi + z3zE1495);
      goto for_start_1944;
    }
  for_end_1945: ;
  }
  unit z3zE1498;
  z3zE1498 = UNIT;
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE1509;
  z3zE1509 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_length);
  zchild_bit = UINT64_C(0x0001);
  unit z3zE1508;
  z3zE1508 = UNIT;
  int64_t z3zE1499;
  {    z3zE1499 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1500;
  {    z3zE1500 = (int64_t)(UINT64_C(15));
  }
  int64_t z3zE1501;
  {    z3zE1501 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z3zE1511;
    z3zE1511 = z3zE1499;
    unit z3zE1504;
  for_start_1946: ;
    {
      if ((z3zE1500 < z3zE1511)) goto for_end_1947;
      bool z2zE1503;
      {
        uint64_t z2zE1502;
        z2zE1502 = (zmask & zchild_bit);
        z2zE1503 = (z2zE1502 != UINT64_C(0x0000));
      }
      unit z3zE1502;
      if (z2zE1503) {
        struct zNodeRef z2zE1504;
        CREATE(zNodeRef)(&z2zE1504);
        fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1504, zchildren, z3zE1511);
        z3zE1502 = zrlp_write_node_ref(z2zE1504);
        KILL(zNodeRef)(&z2zE1504);
      } else {
        zz5listz8z5bv8z9 z3zE1503;
        CREATE(zz5listz8z5bv8z9)(&z3zE1503);
        zconsz3z5bv8(&z3zE1503, UINT64_C(0x80), z3zE1503);
        z3zE1502 = zscratch_push_bytes(z3zE1503, UINT64_C(1));
        KILL(zz5listz8z5bv8z9)(&z3zE1503);
      }
      zchild_bit = ((UINT64_C(1) >= UINT64_C(64)) ? UINT64_C(0) : ((zchild_bit << UINT64_C(1)) & UINT64_C(0xFFFF)));
      z3zE1504 = UNIT;
      z3zE1511 = (z3zE1511 + z3zE1501);
      goto for_start_1946;
    }
  for_end_1947: ;
  }
  unit z3zE1507;
  z3zE1507 = UNIT;
  zz5listz8z5bv8z9 z3zE1505;
  CREATE(zz5listz8z5bv8z9)(&z3zE1505);
  zconsz3z5bv8(&z3zE1505, UINT64_C(0x80), z3zE1505);
  unit z3zE1506;
  z3zE1506 = zscratch_push_bytes(z3zE1505, UINT64_C(1));
  KILL(zz5listz8z5bv8z9)(&z3zE1505);
  struct zNodeRef zresult;
  CREATE(zNodeRef)(&zresult);
  {
    struct zByteSliceFields z2zE1505;
    z2zE1505 = zrlp_finish(zmark);
    {
      zchild_ref(&zresult, z2zE1505);
      if (have_exception) {
        KILL(zNodeRef)(&zresult);
        goto end_block_exception_1949;
      }
    }
  }
  unit z3zE1510;
  z3zE1510 = zscratch_rewind(zmark);
  COPY(zNodeRef)((*(&z8zE503)), zresult);
  KILL(zNodeRef)(&zresult);
end_function_1948: ;
  goto end_function_4036;
end_block_exception_1949: ;
  goto end_function_4036;
end_function_4036: ;
}

sail_fixed_bytes_32 ztrie_ref_to_root(struct zNodeRef zr)
{
  sail_fixed_bytes_32 z8zE504;
  sail_fixed_bytes_32 z3zE1489;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1941;
    z3zE1489 = zEMPTY_TRIE_ROOT;
    goto finish_match_1938;
  }
case_1941: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1940;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1489 = zinline_node_hash(znode);
    goto finish_match_1938;
  }
case_1940: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    z3zE1489 = zh;
    goto finish_match_1938;
  }
case_1939: ;
finish_match_1938: ;
  z8zE504 = z3zE1489;
end_function_1942: ;
  return z8zE504;
end_block_exception_1943: ;

  return fixed_bytes_32_zero();
}

void znode_to_ref(struct zNodeRef *z8zE505, struct zByteSliceFields znode)
{
  bool z2zE1494;
  {
    uint64_t z2zE1493;
    z2zE1493 = znode.zlen;
    z2zE1494 = (z2zE1493 == UINT64_C(0));
  }
  if (z2zE1494) {  zEmptyRef((*(&z8zE505)), UNIT);  } else {
    bool z2zE1496;
    {
      uint64_t z2zE1495;
      z2zE1495 = znode.zlen;
      z2zE1496 = (z2zE1495 < zMPT_HASH_LENGTH);
    }
    if (z2zE1496) {
      struct zInlineNode z2zE1497;
      {
        z2zE1497 = zinline_node_from_slice(znode);
        if (have_exception) {  goto end_block_exception_1937;  }
      }
      zInlineRef((*(&z8zE505)), z2zE1497);
    } else {
      sail_fixed_bytes_32 z2zE1498;
      z2zE1498 = zkeccak256_slice(znode);
      zHashRef((*(&z8zE505)), z2zE1498);
    }
  }
end_function_1936: ;
  goto end_function_4035;
end_block_exception_1937: ;
  goto end_function_4035;
end_function_4035: ;
}

void zmerge_ext_node(struct zNodeRef *z8zE506, struct zTriePath zprefix, struct zByteSliceFields zchildnode)
{
  bool z2zE1486;
  {
    uint64_t z2zE1485;
    z2zE1485 = zpath_len(zprefix);
    z2zE1486 = (z2zE1485 == UINT64_C(0));
  }
  if (z2zE1486) {
    {
      znode_to_ref((*(&z8zE506)), zchildnode);
      if (have_exception) {  goto end_block_exception_1935;  }
    }
  } else {
    bool z2zE1488;
    {
      uint64_t z2zE1487;
      z2zE1487 = zchildnode.zlen;
      z2zE1488 = (z2zE1487 == UINT64_C(0));
    }
    if (z2zE1488) {  zEmptyRef((*(&z8zE506)), UNIT);  } else {
      struct zTrieNode z2zE1489;
      CREATE(zTrieNode)(&z2zE1489);
      {
        zdecode_trie_node(&z2zE1489, zchildnode);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE1489);
          goto end_block_exception_1935;
        }
      }
      struct zNodeRef z3zE1484;
      CREATE(zNodeRef)(&z3zE1484);
      {
        if (z2zE1489.kind != Kind_zLeafNode) goto case_1933;
        struct zTriePath zpath;
        zpath = z2zE1489.variants.zLeafNode.ztup0;
        struct zByteSliceFields zvalue;
        zvalue = z2zE1489.variants.zLeafNode.ztup1;
        struct zTriePath z2zE1490;
        {
          z2zE1490 = zpath_concat(zprefix, zpath);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1489);
            KILL(zNodeRef)(&z3zE1484);
            goto end_block_exception_1935;
          }
        }
        {
          zleaf_child_ref(&z3zE1484, z2zE1490, zvalue);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1489);
            KILL(zNodeRef)(&z3zE1484);
            goto end_block_exception_1935;
          }
        }
        goto finish_match_1930;
      }
    case_1933: ;
      {
        if (z2zE1489.kind != Kind_zExtensionNode) goto case_1932;
        struct zTriePath z3zE1488;
        z3zE1488 = z2zE1489.variants.zExtensionNode.ztup0;
        struct zNodeRef zchild;
        CREATE(zNodeRef)(&zchild);
        COPY(zNodeRef)(&zchild, z2zE1489.variants.zExtensionNode.ztup1);
        struct zTriePath z2zE1491;
        {
          z2zE1491 = zpath_concat(zprefix, z3zE1488);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1489);
            KILL(zNodeRef)(&z3zE1484);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1935;
          }
        }
        {
          zextension_child_ref(&z3zE1484, z2zE1491, zchild);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1489);
            KILL(zNodeRef)(&z3zE1484);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1935;
          }
        }
        KILL(zNodeRef)(&zchild);
        goto finish_match_1930;
      }
    case_1932: ;
      {
        struct zNodeRef z2zE1492;
        CREATE(zNodeRef)(&z2zE1492);
        {
          znode_to_ref(&z2zE1492, zchildnode);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1489);
            KILL(zNodeRef)(&z3zE1484);
            KILL(zNodeRef)(&z2zE1492);
            goto end_block_exception_1935;
          }
        }
        {
          zextension_child_ref(&z3zE1484, zprefix, z2zE1492);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1489);
            KILL(zNodeRef)(&z3zE1484);
            KILL(zNodeRef)(&z2zE1492);
            goto end_block_exception_1935;
          }
        }
        KILL(zNodeRef)(&z2zE1492);
        goto finish_match_1930;
      }
    case_1931: ;
    finish_match_1930: ;
      COPY(zNodeRef)((*(&z8zE506)), z3zE1484);
      KILL(zNodeRef)(&z3zE1484);
      KILL(zTrieNode)(&z2zE1489);
    }
  }
end_function_1934: ;
  goto end_function_4034;
end_block_exception_1935: ;
  goto end_function_4034;
end_function_4034: ;
}

void zmerge_ext_ref(struct zNodeRef *z8zE507, struct zTriePath zprefix, struct zNodeRef zchildref)
{
  bool z2zE1480;
  {
    uint64_t z2zE1479;
    z2zE1479 = zpath_len(zprefix);
    z2zE1480 = (z2zE1479 == UINT64_C(0));
  }
  if (z2zE1480) {  COPY(zNodeRef)((*(&z8zE507)), zchildref);  } else {
    struct zNodeRef z3zE1475;
    CREATE(zNodeRef)(&z3zE1475);
    {
      if (zchildref.kind != Kind_zEmptyRef) goto case_1927;
      zEmptyRef(&z3zE1475, UNIT);
      goto finish_match_1920;
    }
  case_1927: ;
    {
      if (zchildref.kind != Kind_zHashRef) goto case_1926;
      {
        zextension_child_ref(&z3zE1475, zprefix, zchildref);
        if (have_exception) {
          KILL(zNodeRef)(&z3zE1475);
          goto end_block_exception_1929;
        }
      }
      goto finish_match_1920;
    }
  case_1926: ;
    {
      /* complete */
      struct zInlineNode znode;
      znode = zchildref.variants.zInlineRef;
      struct zTrieNode z2zE1482;
      CREATE(zTrieNode)(&z2zE1482);
      {
        struct zByteSliceFields z2zE1481;
        z2zE1481 = zinline_node_slice(znode);
        {
          zdecode_trie_node(&z2zE1482, z2zE1481);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1475);
            KILL(zTrieNode)(&z2zE1482);
            goto end_block_exception_1929;
          }
        }
      }
      struct zNodeRef z3zE1476;
      CREATE(zNodeRef)(&z3zE1476);
      {
        if (z2zE1482.kind != Kind_zLeafNode) goto case_1925;
        struct zTriePath zpath;
        zpath = z2zE1482.variants.zLeafNode.ztup0;
        struct zByteSliceFields zvalue;
        zvalue = z2zE1482.variants.zLeafNode.ztup1;
        struct zTriePath z2zE1483;
        {
          z2zE1483 = zpath_concat(zprefix, zpath);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1475);
            KILL(zTrieNode)(&z2zE1482);
            KILL(zNodeRef)(&z3zE1476);
            goto end_block_exception_1929;
          }
        }
        {
          zleaf_child_ref(&z3zE1476, z2zE1483, zvalue);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1475);
            KILL(zTrieNode)(&z2zE1482);
            KILL(zNodeRef)(&z3zE1476);
            goto end_block_exception_1929;
          }
        }
        goto finish_match_1922;
      }
    case_1925: ;
      {
        if (z2zE1482.kind != Kind_zExtensionNode) goto case_1924;
        struct zTriePath z3zE1483;
        z3zE1483 = z2zE1482.variants.zExtensionNode.ztup0;
        struct zNodeRef zchild;
        CREATE(zNodeRef)(&zchild);
        COPY(zNodeRef)(&zchild, z2zE1482.variants.zExtensionNode.ztup1);
        struct zTriePath z2zE1484;
        {
          z2zE1484 = zpath_concat(zprefix, z3zE1483);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1475);
            KILL(zTrieNode)(&z2zE1482);
            KILL(zNodeRef)(&z3zE1476);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1929;
          }
        }
        {
          zextension_child_ref(&z3zE1476, z2zE1484, zchild);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1475);
            KILL(zTrieNode)(&z2zE1482);
            KILL(zNodeRef)(&z3zE1476);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1929;
          }
        }
        KILL(zNodeRef)(&zchild);
        goto finish_match_1922;
      }
    case_1924: ;
      {
        {
          zextension_child_ref(&z3zE1476, zprefix, zchildref);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1475);
            KILL(zTrieNode)(&z2zE1482);
            KILL(zNodeRef)(&z3zE1476);
            goto end_block_exception_1929;
          }
        }
        goto finish_match_1922;
      }
    case_1923: ;
    finish_match_1922: ;
      COPY(zNodeRef)(&z3zE1475, z3zE1476);
      KILL(zNodeRef)(&z3zE1476);
      KILL(zTrieNode)(&z2zE1482);
      goto finish_match_1920;
    }
  case_1921: ;
  finish_match_1920: ;
    COPY(zNodeRef)((*(&z8zE507)), z3zE1475);
    KILL(zNodeRef)(&z3zE1475);
  }
end_function_1928: ;
  goto end_function_4033;
end_block_exception_1929: ;
  goto end_function_4033;
end_function_4033: ;
}

