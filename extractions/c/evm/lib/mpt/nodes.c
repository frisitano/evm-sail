/* Generated from sail/lib/mpt/nodes.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_240(void) {

  uint64_t z3zE265;
  z3zE265 = zWORD_BYTE_LENGTH;
  zMPT_HASH_LENGTH = z3zE265;
let_end_809: ;
}
void kill_letbind_240(void) {
}

struct zInlineNode zinline_node_from_slice(struct zByteSliceFields zbytes)
{
  struct zInlineNode z8zE542;
  uint64_t zlength;
  zlength = zbytes.zlen;
  bool z2zE1093;
  z2zE1093 = (!(zlength < zMPT_HASH_LENGTH));
  unit z3zE1218;
  if (z2zE1093) {
    struct zexception z2zE1094;
    CREATE(zexception)(&z2zE1094);
    zInvalidBlock(&z2zE1094, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1094);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:23.8-23.44");
    KILL(zexception)(&z2zE1094);
    goto end_block_exception_1750;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1094);
  } else {  z3zE1218 = UNIT;  }
  sail_fixed_bytes_32 z2zE1096;
  {
    sail_u256 z2zE1095;
    z2zE1095 = zslice_load(zbytes, UINT64_C(0));
    z2zE1096 = evmsail_word_to_hash(z2zE1095);
  }
  struct zInlineNode z3zE1219;
  z3zE1219.zdata = z2zE1096;
  z3zE1219.zlen = zlength;
  z8zE542 = z3zE1219;
end_function_1749: ;
  return z8zE542;
end_block_exception_1750: ;
  struct zInlineNode z8zE913 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE913;
}

struct zByteSliceFields zinline_node_slice(struct zInlineNode znode)
{
  struct zByteSliceFields z8zE543;
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  sail_fixed_bytes_32 z2zE1091;
  z2zE1091 = znode.zdata;
  uint64_t z2zE1092;
  z2zE1092 = znode.zlen;
  unit z3zE1217;
  z3zE1217 = zscratch_push_b256(z2zE1091, z2zE1092);
  z8zE543 = zscratch_finish(zstart);
end_function_1747: ;
  return z8zE543;
end_block_exception_1748: ;
  struct zByteSliceFields z8zE914 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE914;
}

sail_fixed_bytes_32 zinline_node_hash(struct zInlineNode znode)
{
  sail_fixed_bytes_32 z8zE544;
  zz5listz8z5unionz0zzBytesz9 z2zE1090;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE1090);
  {
    struct zBytes z2zE1089;
    CREATE(zBytes)(&z2zE1089);
    {
      sail_fixed_bytes_32 z2zE1087;
      z2zE1087 = znode.zdata;
      uint64_t z2zE1088;
      z2zE1088 = znode.zlen;
      zbytes_fixed32(&z2zE1089, z2zE1087, z2zE1088);
    }
    zconsz3z5unionz0zzBytes(&z2zE1090, z2zE1089, z2zE1090);
    KILL(zBytes)(&z2zE1089);
  }
  z8zE544 = host_keccak_segments(z2zE1090);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE1090);
end_function_1745: ;
  return z8zE544;
end_block_exception_1746: ;

  return fixed_bytes_32_zero();
}

uint64_t zbranch_content_length_add(uint64_t zcurrent, uint64_t zaddition)
{
  uint64_t z8zE545;
  bool z2zE1085;
  {
    uint64_t z2zE1084;
    {    z2zE1084 = (UINT64_C(529) - zcurrent);
    }
    z2zE1085 = (!(z2zE1084 < zaddition));
  }
  if (z2zE1085) {
    {    z8zE545 = (zcurrent + zaddition);
    }
  } else {
    struct zexception z2zE1086;
    CREATE(zexception)(&z2zE1086);
    zInvalidBlock(&z2zE1086, zRlpDecode);
    COPY(zexception)(current_exception, z2zE1086);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:62.8-62.37");
    KILL(zexception)(&z2zE1086);
    goto end_block_exception_1744;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1086);
  }
end_function_1743: ;
  return z8zE545;
end_block_exception_1744: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t znode_ref_sizze(struct zNodeRef zr)
{
  uint64_t z8zE546;
  uint64_t z3zE1212;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1740;
    z3zE1212 = UINT64_C(1);
    goto finish_match_1737;
  }
case_1740: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1739;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1212 = znode.zlen;
    goto finish_match_1737;
  }
case_1739: ;
  {
    /* complete */
    z3zE1212 = UINT64_C(33);
    goto finish_match_1737;
  }
case_1738: ;
finish_match_1737: ;
  z8zE546 = z3zE1212;
end_function_1741: ;
  return z8zE546;
end_block_exception_1742: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_node_ref(struct zNodeRef zr)
{
  unit z8zE547;
  unit z3zE1207;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1734;
    zz5listz8z5bv8z9 z3zE1210;
    CREATE(zz5listz8z5bv8z9)(&z3zE1210);
    zconsz3z5bv8(&z3zE1210, UINT64_C(0x80), z3zE1210);
    z3zE1207 = zscratch_push_bytes(z3zE1210, UINT64_C(1));
    KILL(zz5listz8z5bv8z9)(&z3zE1210);
    goto finish_match_1731;
  }
case_1734: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1733;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    sail_fixed_bytes_32 z2zE1081;
    z2zE1081 = znode.zdata;
    uint64_t z2zE1082;
    z2zE1082 = znode.zlen;
    z3zE1207 = zscratch_push_b256(z2zE1081, z2zE1082);
    goto finish_match_1731;
  }
case_1733: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    sail_u256 z2zE1083;
    z2zE1083 = evmsail_hash_to_word(zh);
    z3zE1207 = zrlp_write_word(z2zE1083);
    goto finish_match_1731;
  }
case_1732: ;
finish_match_1731: ;
  z8zE547 = z3zE1207;
end_function_1735: ;
  return z8zE547;
end_block_exception_1736: ;

  return UNIT;
}

void zchild_ref(struct zNodeRef *z8zE548, struct zByteSliceFields zencoded)
{
  bool z2zE1078;
  {
    uint64_t z2zE1077;
    z2zE1077 = zencoded.zlen;
    z2zE1078 = (z2zE1077 < zMPT_HASH_LENGTH);
  }
  if (z2zE1078) {
    struct zInlineNode z2zE1079;
    {
      z2zE1079 = zinline_node_from_slice(zencoded);
      if (have_exception) {  goto end_block_exception_1730;  }
    }
    zInlineRef((*(&z8zE548)), z2zE1079);
  } else {
    sail_fixed_bytes_32 z2zE1080;
    z2zE1080 = zkeccak256_slice(zencoded);
    zHashRef((*(&z8zE548)), z2zE1080);
  }
end_function_1729: ;
  goto end_function_3628;
end_block_exception_1730: ;
  goto end_function_3628;
end_function_3628: ;
}

uint64_t zbranch_mask_for(uint64_t zindex)
{
  uint64_t z8zE549;
  uint64_t z2zE1076;
  z2zE1076 = ((uint64_t) zindex);
  z8zE549 = ((z2zE1076 >= UINT64_C(64)) ? UINT64_C(0) : ((UINT64_C(0x0001) << z2zE1076) & UINT64_C(0xFFFF)));
end_function_1727: ;
  return z8zE549;
end_block_exception_1728: ;

  return UINT64_C(0xdeadc0de);
}

bool zbranch_mask_has(uint64_t zmask, uint64_t zindex)
{
  bool z8zE550;
  uint64_t z2zE1075;
  {
    uint64_t z2zE1074;
    z2zE1074 = zbranch_mask_for(zindex);
    z2zE1075 = (zmask & z2zE1074);
  }
  z8zE550 = (z2zE1075 != UINT64_C(0x0000));
end_function_1725: ;
  return z8zE550;
end_block_exception_1726: ;

  return false;
}

uint64_t zbranch_mask_set(uint64_t zmask, uint64_t zindex)
{
  uint64_t z8zE551;
  uint64_t z2zE1073;
  z2zE1073 = zbranch_mask_for(zindex);
  z8zE551 = (zmask | z2zE1073);
end_function_1723: ;
  return z8zE551;
end_block_exception_1724: ;

  return UINT64_C(0xdeadc0de);
}

void zleaf_child_ref(struct zNodeRef *z8zE552, struct zTriePath zkey, struct zByteSliceFields zvalue)
{
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE1069;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1069);
  zhex_prefix_compact(&z2zE1069, zkey, true);
  struct zNodeRef z3zE1201;
  CREATE(zNodeRef)(&z3zE1201);
  {
    zz5listz8z5bv8z9 zpath;
    CREATE(zz5listz8z5bv8z9)(&zpath);
    COPY(zz5listz8z5bv8z9)(&zpath, z2zE1069.ztup0);
    uint64_t zencoded_path_len;
    zencoded_path_len = z2zE1069.ztup1;
    uint64_t zcontent_len;
    {
      uint64_t z2zE1071;
      {
        z2zE1071 = zrlp_scratch_bytes_sizze(zpath, zencoded_path_len);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1069);
          KILL(zNodeRef)(&z3zE1201);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1722;
        }
      }
      uint64_t z2zE1072;
      {
        z2zE1072 = zrlp_scratch_slice_sizze(zvalue);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1069);
          KILL(zNodeRef)(&z3zE1201);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1722;
        }
      }
      {
        zcontent_len = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1071, z2zE1072);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1069);
          KILL(zNodeRef)(&z3zE1201);
          KILL(zz5listz8z5bv8z9)(&zpath);
          goto end_block_exception_1722;
        }
      }
    }
    uint64_t zmark;
    zmark = zscratch_begin(UNIT);
    unit z3zE1204;
    z3zE1204 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    unit z3zE1203;
    z3zE1203 = zrlp_write_bytes(zpath, zencoded_path_len);
    unit z3zE1202;
    z3zE1202 = zrlp_write_slice(zvalue);
    struct zNodeRef zresult;
    CREATE(zNodeRef)(&zresult);
    {
      struct zByteSliceFields z2zE1070;
      z2zE1070 = zrlp_finish(zmark);
      {
        zchild_ref(&zresult, z2zE1070);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1069);
          KILL(zNodeRef)(&z3zE1201);
          KILL(zz5listz8z5bv8z9)(&zpath);
          KILL(zNodeRef)(&zresult);
          goto end_block_exception_1722;
        }
      }
    }
    unit z3zE1205;
    z3zE1205 = zscratch_rewind(zmark);
    z3zE1201 = zresult;
    KILL(zNodeRef)(&zresult);
    KILL(zz5listz8z5bv8z9)(&zpath);
    goto finish_match_1719;
  }
case_1720: ;
  sail_match_failure("leaf_child_ref");
finish_match_1719: ;
  (*(z8zE552)) = z3zE1201;
  KILL(zNodeRef)(&z3zE1201);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1069);
end_function_1721: ;
  goto end_function_3627;
end_block_exception_1722: ;
  goto end_function_3627;
end_function_3627: ;
}

void zextension_child_ref(struct zNodeRef *z8zE553, struct zTriePath zkey, struct zNodeRef zchildref)
{
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE1067;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1067);
  zhex_prefix_compact(&z2zE1067, zkey, false);
  struct zNodeRef z3zE1195;
  CREATE(zNodeRef)(&z3zE1195);
  {
    zz5listz8z5bv8z9 zpath;
    CREATE(zz5listz8z5bv8z9)(&zpath);
    COPY(zz5listz8z5bv8z9)(&zpath, z2zE1067.ztup0);
    uint64_t zencoded_path_len;
    zencoded_path_len = z2zE1067.ztup1;
    uint64_t zpath_length;
    zpath_length = zrlp_bytes_sizzezIreprzGLB8zCU64zCU64zKzIboundszGed754718030b429454cf327fc49bd29dzK(zpath, zencoded_path_len);
    uint64_t zchild_length;
    zchild_length = znode_ref_sizze(zchildref);
    uint64_t zcontent_len;
    {    zcontent_len = (zpath_length + zchild_length);
    }
    uint64_t zmark;
    zmark = zscratch_begin(UNIT);
    unit z3zE1198;
    z3zE1198 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    unit z3zE1197;
    z3zE1197 = zrlp_write_bytes(zpath, zencoded_path_len);
    unit z3zE1196;
    z3zE1196 = zrlp_write_node_ref(zchildref);
    struct zNodeRef zresult;
    CREATE(zNodeRef)(&zresult);
    {
      struct zByteSliceFields z2zE1068;
      z2zE1068 = zrlp_finish(zmark);
      {
        zchild_ref(&zresult, z2zE1068);
        if (have_exception) {
          KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1067);
          KILL(zNodeRef)(&z3zE1195);
          KILL(zz5listz8z5bv8z9)(&zpath);
          KILL(zNodeRef)(&zresult);
          goto end_block_exception_1718;
        }
      }
    }
    unit z3zE1199;
    z3zE1199 = zscratch_rewind(zmark);
    z3zE1195 = zresult;
    KILL(zNodeRef)(&zresult);
    KILL(zz5listz8z5bv8z9)(&zpath);
    goto finish_match_1715;
  }
case_1716: ;
  sail_match_failure("extension_child_ref");
finish_match_1715: ;
  (*(z8zE553)) = z3zE1195;
  KILL(zNodeRef)(&z3zE1195);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE1067);
end_function_1717: ;
  goto end_function_3626;
end_block_exception_1718: ;
  goto end_function_3626;
end_function_3626: ;
}

void zbranch_child_ref(struct zNodeRef *z8zE554, uint64_t zmask, zz5vecz8z5unionz0zzNodeRefz9 zchildren)
{
  uint64_t zcontent_length;
  zcontent_length = UINT64_C(1);
  uint64_t zchild_bit;
  zchild_bit = UINT64_C(0x0001);
  int64_t z3zE1176;
  {    z3zE1176 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1177;
  {    z3zE1177 = (int64_t)(UINT64_C(15));
  }
  int64_t z3zE1178;
  {    z3zE1178 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zi;
    zi = z3zE1176;
    unit z3zE1180;
  for_start_1709: ;
    {
      if ((z3zE1177 < zi)) goto for_end_1710;
      bool z2zE1061;
      {
        uint64_t z2zE1060;
        z2zE1060 = (zmask & zchild_bit);
        z2zE1061 = (z2zE1060 != UINT64_C(0x0000));
      }
      unit z3zE1179;
      if (z2zE1061) {
        uint64_t zchild_length;
        {
          struct zNodeRef z2zE1062;
          CREATE(zNodeRef)(&z2zE1062);
          fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1062, zchildren, zi);
          zchild_length = znode_ref_sizze(z2zE1062);
          KILL(zNodeRef)(&z2zE1062);
        }
        {
          zcontent_length = zbranch_content_length_add(zcontent_length, zchild_length);
          if (have_exception) {  goto end_block_exception_1714;  }
        }
        z3zE1179 = UNIT;
      } else {
        {
          zcontent_length = zbranch_content_length_add(zcontent_length, UINT64_C(1));
          if (have_exception) {  goto end_block_exception_1714;  }
        }
        z3zE1179 = UNIT;
      }
      zchild_bit = ((UINT64_C(1) >= UINT64_C(64)) ? UINT64_C(0) : ((zchild_bit << UINT64_C(1)) & UINT64_C(0xFFFF)));
      z3zE1180 = UNIT;
      zi = (zi + z3zE1178);
      goto for_start_1709;
    }
  for_end_1710: ;
  }
  unit z3zE1181;
  z3zE1181 = UNIT;
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE1192;
  z3zE1192 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_length);
  zchild_bit = UINT64_C(0x0001);
  unit z3zE1191;
  z3zE1191 = UNIT;
  int64_t z3zE1182;
  {    z3zE1182 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE1183;
  {    z3zE1183 = (int64_t)(UINT64_C(15));
  }
  int64_t z3zE1184;
  {    z3zE1184 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t z3zE1194;
    z3zE1194 = z3zE1182;
    unit z3zE1187;
  for_start_1711: ;
    {
      if ((z3zE1183 < z3zE1194)) goto for_end_1712;
      bool z2zE1064;
      {
        uint64_t z2zE1063;
        z2zE1063 = (zmask & zchild_bit);
        z2zE1064 = (z2zE1063 != UINT64_C(0x0000));
      }
      unit z3zE1185;
      if (z2zE1064) {
        struct zNodeRef z2zE1065;
        CREATE(zNodeRef)(&z2zE1065);
        fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1065, zchildren, z3zE1194);
        z3zE1185 = zrlp_write_node_ref(z2zE1065);
        KILL(zNodeRef)(&z2zE1065);
      } else {
        zz5listz8z5bv8z9 z3zE1186;
        CREATE(zz5listz8z5bv8z9)(&z3zE1186);
        zconsz3z5bv8(&z3zE1186, UINT64_C(0x80), z3zE1186);
        z3zE1185 = zscratch_push_bytes(z3zE1186, UINT64_C(1));
        KILL(zz5listz8z5bv8z9)(&z3zE1186);
      }
      zchild_bit = ((UINT64_C(1) >= UINT64_C(64)) ? UINT64_C(0) : ((zchild_bit << UINT64_C(1)) & UINT64_C(0xFFFF)));
      z3zE1187 = UNIT;
      z3zE1194 = (z3zE1194 + z3zE1184);
      goto for_start_1711;
    }
  for_end_1712: ;
  }
  unit z3zE1190;
  z3zE1190 = UNIT;
  zz5listz8z5bv8z9 z3zE1188;
  CREATE(zz5listz8z5bv8z9)(&z3zE1188);
  zconsz3z5bv8(&z3zE1188, UINT64_C(0x80), z3zE1188);
  unit z3zE1189;
  z3zE1189 = zscratch_push_bytes(z3zE1188, UINT64_C(1));
  KILL(zz5listz8z5bv8z9)(&z3zE1188);
  struct zNodeRef zresult;
  CREATE(zNodeRef)(&zresult);
  {
    struct zByteSliceFields z2zE1066;
    z2zE1066 = zrlp_finish(zmark);
    {
      zchild_ref(&zresult, z2zE1066);
      if (have_exception) {
        KILL(zNodeRef)(&zresult);
        goto end_block_exception_1714;
      }
    }
  }
  unit z3zE1193;
  z3zE1193 = zscratch_rewind(zmark);
  (*(z8zE554)) = zresult;
  KILL(zNodeRef)(&zresult);
end_function_1713: ;
  goto end_function_3625;
end_block_exception_1714: ;
  goto end_function_3625;
end_function_3625: ;
}

sail_fixed_bytes_32 ztrie_ref_to_root(struct zNodeRef zr)
{
  sail_fixed_bytes_32 z8zE555;
  sail_fixed_bytes_32 z3zE1172;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1706;
    z3zE1172 = zEMPTY_TRIE_ROOT;
    goto finish_match_1703;
  }
case_1706: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1705;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1172 = zinline_node_hash(znode);
    goto finish_match_1703;
  }
case_1705: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    z3zE1172 = zh;
    goto finish_match_1703;
  }
case_1704: ;
finish_match_1703: ;
  z8zE555 = z3zE1172;
end_function_1707: ;
  return z8zE555;
end_block_exception_1708: ;

  return fixed_bytes_32_zero();
}

void znode_to_ref(struct zNodeRef *z8zE556, struct zByteSliceFields znode)
{
  bool z2zE1055;
  {
    uint64_t z2zE1054;
    z2zE1054 = znode.zlen;
    z2zE1055 = (z2zE1054 == UINT64_C(0));
  }
  if (z2zE1055) {  zEmptyRef((*(&z8zE556)), UNIT);  } else {
    bool z2zE1057;
    {
      uint64_t z2zE1056;
      z2zE1056 = znode.zlen;
      z2zE1057 = (z2zE1056 < zMPT_HASH_LENGTH);
    }
    if (z2zE1057) {
      struct zInlineNode z2zE1058;
      {
        z2zE1058 = zinline_node_from_slice(znode);
        if (have_exception) {  goto end_block_exception_1702;  }
      }
      zInlineRef((*(&z8zE556)), z2zE1058);
    } else {
      sail_fixed_bytes_32 z2zE1059;
      z2zE1059 = zkeccak256_slice(znode);
      zHashRef((*(&z8zE556)), z2zE1059);
    }
  }
end_function_1701: ;
  goto end_function_3624;
end_block_exception_1702: ;
  goto end_function_3624;
end_function_3624: ;
}

struct zByteSliceFields znode_db_lookup(sail_fixed_bytes_32 zh)
{
  struct zByteSliceFields z8zE557;
  z8zE557 = nodedb_lookup(zh);
end_function_1699: ;
  return z8zE557;
end_block_exception_1700: ;
  struct zByteSliceFields z8zE915 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE915;
}

void zfield_to_ref(struct zNodeRef *z8zE558, struct zRlpFieldRef zf)
{
  bool z2zE1047;
  z2zE1047 = zf.zis_list;
  if (z2zE1047) {
    struct zInlineNode z2zE1049;
    {
      struct zByteSliceFields z2zE1048;
      z2zE1048 = zf.zsource;
      {
        z2zE1049 = zinline_node_from_slice(z2zE1048);
        if (have_exception) {  goto end_block_exception_1698;  }
      }
    }
    zInlineRef((*(&z8zE558)), z2zE1049);
  } else {
    bool z2zE1051;
    {
      uint64_t z2zE1050;
      z2zE1050 = zf.zcontent_len;
      z2zE1051 = (z2zE1050 == zMPT_HASH_LENGTH);
    }
    if (z2zE1051) {
      sail_fixed_bytes_32 z2zE1053;
      {
        sail_u256 z2zE1052;
        {
          z2zE1052 = zrlp_ref_word(zf);
          if (have_exception) {  goto end_block_exception_1698;  }
        }
        z2zE1053 = evmsail_word_to_hash(z2zE1052);
      }
      zHashRef((*(&z8zE558)), z2zE1053);
    } else {  zEmptyRef((*(&z8zE558)), UNIT);  }
  }
end_function_1697: ;
  goto end_function_3623;
end_block_exception_1698: ;
  goto end_function_3623;
end_function_3623: ;
}

void zdecode_branch_node(struct zTrieNode *z8zE559, struct zByteSliceFields zcursor, uint64_t zindex, zz5vecz8z5unionz0zzNodeRefz9 zchildren)
{
  bool z2zE1040;
  z2zE1040 = (zindex < UINT64_C(16));
  if (z2zE1040) {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE1041;
    {
      z2zE1041 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1696;  }
    }
    struct zTrieNode z3zE1169;
    CREATE(zTrieNode)(&z3zE1169);
    {
      struct zRlpFieldRef zchild;
      zchild = z2zE1041.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE1041.ztup1;
      zz5vecz8z5unionz0zzNodeRefz9 zupdated;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated, zchildren);
      struct zNodeRef z2zE1042;
      CREATE(zNodeRef)(&z2zE1042);
      {
        zfield_to_ref(&z2zE1042, zchild);
        if (have_exception) {
          KILL(zTrieNode)(&z3zE1169);
          KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
          KILL(zNodeRef)(&z2zE1042);
          goto end_block_exception_1696;
        }
      }
      fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&zupdated, zupdated, zindex, z2zE1042);
      unit z3zE1170;
      z3zE1170 = UNIT;
      KILL(zNodeRef)(&z2zE1042);
      uint64_t z2zE1043;
      {    z2zE1043 = (zindex + UINT64_C(1));
      }
      {
        zdecode_branch_node(&z3zE1169, znext, z2zE1043, zupdated);
        if (have_exception) {
          KILL(zTrieNode)(&z3zE1169);
          KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
          goto end_block_exception_1696;
        }
      }
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
      goto finish_match_1693;
    }
  case_1694: ;
    sail_match_failure("decode_branch_node");
  finish_match_1693: ;
    COPY(zTrieNode)((*(&z8zE559)), z3zE1169);
    KILL(zTrieNode)(&z3zE1169);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE1044;
    {
      z2zE1044 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1696;  }
    }
    struct zTrieNode z3zE1165;
    CREATE(zTrieNode)(&z3zE1165);
    {
      struct zRlpFieldRef zvalue;
      zvalue = z2zE1044.ztup0;
      struct zByteSliceFields z1zE22;
      z1zE22 = z2zE1044.ztup1;
      unit z3zE1166;
      {
        z3zE1166 = zrlp_cursor_expect_end(z1zE22);
        if (have_exception) {
          KILL(zTrieNode)(&z3zE1165);
          goto end_block_exception_1696;
        }
      }
      struct ztuple_z8z5vecz8z5unionz0zzNodeRefz9zCz0z5structz0zzByteSliceFieldsz9 z2zE1046;
      CREATE(ztuple_z8z5vecz8z5unionz0zzNodeRefz9zCz0z5structz0zzByteSliceFieldsz9)(&z2zE1046);
      {
        struct zByteSliceFields z2zE1045;
        z2zE1045 = zrlp_ref_content(zvalue);
        COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&z2zE1046)->ztup0), zchildren);
        z2zE1046.ztup1 = z2zE1045;
      }
      zBranchNode(&z3zE1165, z2zE1046);
      KILL(ztuple_z8z5vecz8z5unionz0zzNodeRefz9zCz0z5structz0zzByteSliceFieldsz9)(&z2zE1046);
      goto finish_match_1691;
    }
  case_1692: ;
    sail_match_failure("decode_branch_node");
  finish_match_1691: ;
    COPY(zTrieNode)((*(&z8zE559)), z3zE1165);
    KILL(zTrieNode)(&z3zE1165);
  }
end_function_1695: ;
  goto end_function_3622;
end_block_exception_1696: ;
  goto end_function_3622;
end_function_3622: ;
}

void zdecode_trie_node(struct zTrieNode *z8zE560, struct zByteSliceFields znode)
{
  struct zByteSliceFields zfields;
  {
    zfields = zrlp_node_cursor(znode);
    if (have_exception) {  goto end_block_exception_1690;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE1025;
  {
    z2zE1025 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_1690;  }
  }
  struct zTrieNode z3zE1155;
  CREATE(zTrieNode)(&z3zE1155);
  {
    struct zRlpFieldRef zfirst;
    zfirst = z2zE1025.ztup0;
    struct zByteSliceFields z1zE20;
    z1zE20 = z2zE1025.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE1026;
    {
      z2zE1026 = zrlp_cursor_pop(z1zE20);
      if (have_exception) {
        KILL(zTrieNode)(&z3zE1155);
        goto end_block_exception_1690;
      }
    }
    struct zTrieNode z3zE1156;
    CREATE(zTrieNode)(&z3zE1156);
    {
      struct zRlpFieldRef zsecond;
      zsecond = z2zE1026.ztup0;
      struct zByteSliceFields z1zE21;
      z1zE21 = z2zE1026.ztup1;
      bool z2zE1028;
      {
        uint64_t z2zE1027;
        z2zE1027 = z1zE21.zlen;
        z2zE1028 = (z2zE1027 == UINT64_C(0));
      }
      if (z2zE1028) {
        struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z2zE1029;
        {
          z2zE1029 = zhex_prefix_decode_ref(zfirst);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE1155);
            KILL(zTrieNode)(&z3zE1156);
            goto end_block_exception_1690;
          }
        }
        struct zTrieNode z3zE1160;
        CREATE(zTrieNode)(&z3zE1160);
        {
          bool zis_leaf;
          zis_leaf = z2zE1029.ztup0;
          struct zTriePath zpath;
          zpath = z2zE1029.ztup1;
          if (zis_leaf) {
            struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzByteSliceFieldsz9 z2zE1031;
            {
              struct zByteSliceFields z2zE1030;
              z2zE1030 = zrlp_ref_content(zsecond);
              struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzByteSliceFieldsz9 z3zE1163;
              z3zE1163.ztup0 = zpath;
              z3zE1163.ztup1 = z2zE1030;
              z2zE1031 = z3zE1163;
            }
            zLeafNode(&z3zE1160, z2zE1031);
          } else {
            bool z2zE1033;
            {
              uint64_t z2zE1032;
              z2zE1032 = zpath_len(zpath);
              z2zE1033 = (z2zE1032 == UINT64_C(0));
            }
            if (z2zE1033) {
              struct zexception z2zE1034;
              CREATE(zexception)(&z2zE1034);
              zInvalidBlock(&z2zE1034, zRlpDecode);
              COPY(zexception)(current_exception, z2zE1034);
              have_exception = true;
              COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:274.16-274.45");
              KILL(zTrieNode)(&z3zE1155);
              KILL(zTrieNode)(&z3zE1156);
              KILL(zTrieNode)(&z3zE1160);
              KILL(zexception)(&z2zE1034);
              goto end_block_exception_1690;
              /* unreachable after throw */
              KILL(zexception)(&z2zE1034);
            } else {
              struct ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9 z2zE1036;
              CREATE(ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9)(&z2zE1036);
              {
                struct zNodeRef z2zE1035;
                CREATE(zNodeRef)(&z2zE1035);
                {
                  zfield_to_ref(&z2zE1035, zsecond);
                  if (have_exception) {
                    KILL(zTrieNode)(&z3zE1155);
                    KILL(zTrieNode)(&z3zE1156);
                    KILL(zTrieNode)(&z3zE1160);
                    KILL(ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9)(&z2zE1036);
                    KILL(zNodeRef)(&z2zE1035);
                    goto end_block_exception_1690;
                  }
                }
                z2zE1036.ztup0 = zpath;
                z2zE1036.ztup1 = z2zE1035;
                KILL(zNodeRef)(&z2zE1035);
              }
              zExtensionNode(&z3zE1160, z2zE1036);
              KILL(ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9)(&z2zE1036);
            }
          }
          goto finish_match_1687;
        }
      case_1688: ;
        sail_match_failure("decode_trie_node");
      finish_match_1687: ;
        COPY(zTrieNode)(&z3zE1156, z3zE1160);
        KILL(zTrieNode)(&z3zE1160);
      } else {
        zz5vecz8z5unionz0zzNodeRefz9 zchildren;
        CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
        {
          struct zNodeRef z2zE1039;
          CREATE(zNodeRef)(&z2zE1039);
          zEmptyRef(&z2zE1039, UNIT);
          fast_unsigned_vector_init_zz5vecz8z5unionz0zzNodeRefz9(&zchildren, UINT64_C(16), z2zE1039);
          KILL(zNodeRef)(&z2zE1039);
        }
        struct zNodeRef z2zE1037;
        CREATE(zNodeRef)(&z2zE1037);
        {
          zfield_to_ref(&z2zE1037, zfirst);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE1155);
            KILL(zTrieNode)(&z3zE1156);
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
            KILL(zNodeRef)(&z2zE1037);
            goto end_block_exception_1690;
          }
        }
        fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&zchildren, zchildren, UINT64_C(0), z2zE1037);
        unit z3zE1159;
        z3zE1159 = UNIT;
        KILL(zNodeRef)(&z2zE1037);
        struct zNodeRef z2zE1038;
        CREATE(zNodeRef)(&z2zE1038);
        {
          zfield_to_ref(&z2zE1038, zsecond);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE1155);
            KILL(zTrieNode)(&z3zE1156);
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
            KILL(zNodeRef)(&z2zE1038);
            goto end_block_exception_1690;
          }
        }
        fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&zchildren, zchildren, UINT64_C(1), z2zE1038);
        unit z3zE1158;
        z3zE1158 = UNIT;
        KILL(zNodeRef)(&z2zE1038);
        {
          zdecode_branch_node(&z3zE1156, z1zE21, UINT64_C(2), zchildren);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE1155);
            KILL(zTrieNode)(&z3zE1156);
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
            goto end_block_exception_1690;
          }
        }
        KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      }
      goto finish_match_1685;
    }
  case_1686: ;
    sail_match_failure("decode_trie_node");
  finish_match_1685: ;
    COPY(zTrieNode)(&z3zE1155, z3zE1156);
    KILL(zTrieNode)(&z3zE1156);
    goto finish_match_1683;
  }
case_1684: ;
  sail_match_failure("decode_trie_node");
finish_match_1683: ;
  COPY(zTrieNode)((*(&z8zE560)), z3zE1155);
  KILL(zTrieNode)(&z3zE1155);
end_function_1689: ;
  goto end_function_3621;
end_block_exception_1690: ;
  goto end_function_3621;
end_function_3621: ;
}

struct zByteSliceFields zresolve_ref(struct zNodeRef zr)
{
  struct zByteSliceFields z8zE561;
  struct zByteSliceFields z3zE1150;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_1680;
    z3zE1150 = zEMPTY_SLICE;
    goto finish_match_1677;
  }
case_1680: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_1679;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE1150 = zinline_node_slice(znode);
    goto finish_match_1677;
  }
case_1679: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    struct zByteSliceFields z3zE1154;
    z3zE1154 = znode_db_lookup(zh);
    bool z2zE1023;
    {
      uint64_t z2zE1022;
      z2zE1022 = z3zE1154.zlen;
      z2zE1023 = (z2zE1022 == UINT64_C(0));
    }
    if (z2zE1023) {
      struct zexception z2zE1024;
      CREATE(zexception)(&z2zE1024);
      zInvalidBlock(&z2zE1024, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1024);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:297.16-297.52");
      KILL(zexception)(&z2zE1024);
      goto end_block_exception_1682;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1024);
    } else {  z3zE1150 = z3zE1154;  }
    goto finish_match_1677;
  }
case_1678: ;
finish_match_1677: ;
  z8zE561 = z3zE1150;
end_function_1681: ;
  return z8zE561;
end_block_exception_1682: ;
  struct zByteSliceFields z8zE916 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE916;
}

void zmerge_ext_node(struct zNodeRef *z8zE562, struct zTriePath zprefix, struct zByteSliceFields zchildnode)
{
  bool z2zE1015;
  {
    uint64_t z2zE1014;
    z2zE1014 = zpath_len(zprefix);
    z2zE1015 = (z2zE1014 == UINT64_C(0));
  }
  if (z2zE1015) {
    {
      znode_to_ref((*(&z8zE562)), zchildnode);
      if (have_exception) {  goto end_block_exception_1676;  }
    }
  } else {
    bool z2zE1017;
    {
      uint64_t z2zE1016;
      z2zE1016 = zchildnode.zlen;
      z2zE1017 = (z2zE1016 == UINT64_C(0));
    }
    if (z2zE1017) {  zEmptyRef((*(&z8zE562)), UNIT);  } else {
      struct zTrieNode z2zE1018;
      CREATE(zTrieNode)(&z2zE1018);
      {
        zdecode_trie_node(&z2zE1018, zchildnode);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE1018);
          goto end_block_exception_1676;
        }
      }
      struct zNodeRef z3zE1145;
      CREATE(zNodeRef)(&z3zE1145);
      {
        if (z2zE1018.kind != Kind_zLeafNode) goto case_1674;
        struct zTriePath zpath;
        zpath = z2zE1018.variants.zLeafNode.ztup0;
        struct zByteSliceFields zvalue;
        zvalue = z2zE1018.variants.zLeafNode.ztup1;
        struct zTriePath z2zE1019;
        {
          z2zE1019 = zpath_concat(zprefix, zpath);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1018);
            KILL(zNodeRef)(&z3zE1145);
            goto end_block_exception_1676;
          }
        }
        {
          zleaf_child_ref(&z3zE1145, z2zE1019, zvalue);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1018);
            KILL(zNodeRef)(&z3zE1145);
            goto end_block_exception_1676;
          }
        }
        goto finish_match_1671;
      }
    case_1674: ;
      {
        if (z2zE1018.kind != Kind_zExtensionNode) goto case_1673;
        struct zTriePath z3zE1149;
        z3zE1149 = z2zE1018.variants.zExtensionNode.ztup0;
        struct zNodeRef zchild;
        CREATE(zNodeRef)(&zchild);
        zchild = z2zE1018.variants.zExtensionNode.ztup1;
        struct zTriePath z2zE1020;
        {
          z2zE1020 = zpath_concat(zprefix, z3zE1149);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1018);
            KILL(zNodeRef)(&z3zE1145);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1676;
          }
        }
        {
          zextension_child_ref(&z3zE1145, z2zE1020, zchild);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1018);
            KILL(zNodeRef)(&z3zE1145);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1676;
          }
        }
        KILL(zNodeRef)(&zchild);
        goto finish_match_1671;
      }
    case_1673: ;
      {
        struct zNodeRef z2zE1021;
        CREATE(zNodeRef)(&z2zE1021);
        {
          znode_to_ref(&z2zE1021, zchildnode);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1018);
            KILL(zNodeRef)(&z3zE1145);
            KILL(zNodeRef)(&z2zE1021);
            goto end_block_exception_1676;
          }
        }
        {
          zextension_child_ref(&z3zE1145, zprefix, z2zE1021);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1018);
            KILL(zNodeRef)(&z3zE1145);
            KILL(zNodeRef)(&z2zE1021);
            goto end_block_exception_1676;
          }
        }
        KILL(zNodeRef)(&z2zE1021);
        goto finish_match_1671;
      }
    case_1672: ;
    finish_match_1671: ;
      (*(z8zE562)) = z3zE1145;
      KILL(zNodeRef)(&z3zE1145);
      KILL(zTrieNode)(&z2zE1018);
    }
  }
end_function_1675: ;
  goto end_function_3620;
end_block_exception_1676: ;
  goto end_function_3620;
end_function_3620: ;
}

void zmerge_ext_ref(struct zNodeRef *z8zE563, struct zTriePath zprefix, struct zNodeRef zchildref)
{
  bool z2zE1009;
  {
    uint64_t z2zE1008;
    z2zE1008 = zpath_len(zprefix);
    z2zE1009 = (z2zE1008 == UINT64_C(0));
  }
  if (z2zE1009) {  (*(z8zE563)) = zchildref;  } else {
    struct zNodeRef z3zE1136;
    CREATE(zNodeRef)(&z3zE1136);
    {
      if (zchildref.kind != Kind_zEmptyRef) goto case_1668;
      zEmptyRef(&z3zE1136, UNIT);
      goto finish_match_1661;
    }
  case_1668: ;
    {
      if (zchildref.kind != Kind_zHashRef) goto case_1667;
      {
        zextension_child_ref(&z3zE1136, zprefix, zchildref);
        if (have_exception) {
          KILL(zNodeRef)(&z3zE1136);
          goto end_block_exception_1670;
        }
      }
      goto finish_match_1661;
    }
  case_1667: ;
    {
      /* complete */
      struct zInlineNode znode;
      znode = zchildref.variants.zInlineRef;
      struct zTrieNode z2zE1011;
      CREATE(zTrieNode)(&z2zE1011);
      {
        struct zByteSliceFields z2zE1010;
        z2zE1010 = zinline_node_slice(znode);
        {
          zdecode_trie_node(&z2zE1011, z2zE1010);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1136);
            KILL(zTrieNode)(&z2zE1011);
            goto end_block_exception_1670;
          }
        }
      }
      struct zNodeRef z3zE1137;
      CREATE(zNodeRef)(&z3zE1137);
      {
        if (z2zE1011.kind != Kind_zLeafNode) goto case_1666;
        struct zTriePath zpath;
        zpath = z2zE1011.variants.zLeafNode.ztup0;
        struct zByteSliceFields zvalue;
        zvalue = z2zE1011.variants.zLeafNode.ztup1;
        struct zTriePath z2zE1012;
        {
          z2zE1012 = zpath_concat(zprefix, zpath);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1136);
            KILL(zTrieNode)(&z2zE1011);
            KILL(zNodeRef)(&z3zE1137);
            goto end_block_exception_1670;
          }
        }
        {
          zleaf_child_ref(&z3zE1137, z2zE1012, zvalue);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1136);
            KILL(zTrieNode)(&z2zE1011);
            KILL(zNodeRef)(&z3zE1137);
            goto end_block_exception_1670;
          }
        }
        goto finish_match_1663;
      }
    case_1666: ;
      {
        if (z2zE1011.kind != Kind_zExtensionNode) goto case_1665;
        struct zTriePath z3zE1144;
        z3zE1144 = z2zE1011.variants.zExtensionNode.ztup0;
        struct zNodeRef zchild;
        CREATE(zNodeRef)(&zchild);
        zchild = z2zE1011.variants.zExtensionNode.ztup1;
        struct zTriePath z2zE1013;
        {
          z2zE1013 = zpath_concat(zprefix, z3zE1144);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1136);
            KILL(zTrieNode)(&z2zE1011);
            KILL(zNodeRef)(&z3zE1137);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1670;
          }
        }
        {
          zextension_child_ref(&z3zE1137, z2zE1013, zchild);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1136);
            KILL(zTrieNode)(&z2zE1011);
            KILL(zNodeRef)(&z3zE1137);
            KILL(zNodeRef)(&zchild);
            goto end_block_exception_1670;
          }
        }
        KILL(zNodeRef)(&zchild);
        goto finish_match_1663;
      }
    case_1665: ;
      {
        {
          zextension_child_ref(&z3zE1137, zprefix, zchildref);
          if (have_exception) {
            KILL(zNodeRef)(&z3zE1136);
            KILL(zTrieNode)(&z2zE1011);
            KILL(zNodeRef)(&z3zE1137);
            goto end_block_exception_1670;
          }
        }
        goto finish_match_1663;
      }
    case_1664: ;
    finish_match_1663: ;
      z3zE1136 = z3zE1137;
      KILL(zNodeRef)(&z3zE1137);
      KILL(zTrieNode)(&z2zE1011);
      goto finish_match_1661;
    }
  case_1662: ;
  finish_match_1661: ;
    (*(z8zE563)) = z3zE1136;
    KILL(zNodeRef)(&z3zE1136);
  }
end_function_1669: ;
  goto end_function_3619;
end_block_exception_1670: ;
  goto end_function_3619;
end_function_3619: ;
}

