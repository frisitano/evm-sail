/* Generated from sail/host/kernel/storage.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
struct zStorageKey zstorage_key(sail_fixed_bytes_20 za, sail_u256 zs)
{
  struct zStorageKey z8zE216;
  struct zStorageKey z3zE3188;
  z3zE3188.zaddr = za;
  z3zE3188.zslot = zs;
  z8zE216 = z3zE3188;
end_function_3232: ;
  return z8zE216;
end_block_exception_3233: ;
  struct zStorageKey z8zE857 = { .zaddr = fixed_bytes_20_zero(), .zslot = u256_zero() };
  return z8zE857;
}

bool zk_access_account(sail_fixed_bytes_20 za)
{
  bool z8zE217;
  z8zE217 = warm_addr_touch(za);
end_function_3230: ;
  return z8zE217;
end_block_exception_3231: ;

  return false;
}

bool zk_slot_is_warm(sail_fixed_bytes_20 za, sail_u256 zs)
{
  bool z8zE218;
  z8zE218 = warm_slot_touch(za, zs);
end_function_3228: ;
  return z8zE218;
end_block_exception_3229: ;

  return false;
}

struct zAccountInfo zdecode_state_account(struct zByteSliceFields zvalue)
{
  struct zAccountInfo z8zE219;
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3201;
  {
    struct zRlpCursor z2zE3200;
    {
      z2zE3200 = zrlp_node_cursor(zvalue);
      if (have_exception) {  goto end_block_exception_3227;  }
    }
    {
      z2zE3201 = zrlp_cursor_pop(z2zE3200);
      if (have_exception) {  goto end_block_exception_3227;  }
    }
  }
  struct zAccountInfo z3zE3178;
  {
    struct zRlpFieldRef znonce;
    znonce = z2zE3201.ztup0;
    struct zRlpCursor zfields;
    zfields = z2zE3201.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3202;
    {
      z2zE3202 = zrlp_cursor_pop(zfields);
      if (have_exception) {  goto end_block_exception_3227;  }
    }
    struct zAccountInfo z3zE3179;
    {
      struct zRlpFieldRef zbalance;
      zbalance = z2zE3202.ztup0;
      struct zRlpCursor z1zE6;
      z1zE6 = z2zE3202.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3203;
      {
        z2zE3203 = zrlp_cursor_pop(z1zE6);
        if (have_exception) {  goto end_block_exception_3227;  }
      }
      struct zAccountInfo z3zE3180;
      {
        struct zRlpFieldRef zstorage;
        zstorage = z2zE3203.ztup0;
        struct zRlpCursor z1zE7;
        z1zE7 = z2zE3203.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3204;
        {
          z2zE3204 = zrlp_cursor_pop(z1zE7);
          if (have_exception) {  goto end_block_exception_3227;  }
        }
        struct zAccountInfo z3zE3181;
        {
          struct zRlpFieldRef zcode;
          zcode = z2zE3204.ztup0;
          struct zRlpCursor z1zE8;
          z1zE8 = z2zE3204.ztup1;
          unit z3zE3182;
          {
            z3zE3182 = zrlp_cursor_expect_end(z1zE8);
            if (have_exception) {  goto end_block_exception_3227;  }
          }
          sail_fixed_bytes_32 zstorage_root;
          {
            bool z2zE3211;
            {
              uint64_t z2zE3210;
              z2zE3210 = zstorage.zcontent_len;
              z2zE3211 = (z2zE3210 == UINT64_C(0));
            }
            if (z2zE3211) {  zstorage_root = zEMPTY_TRIE_ROOT;  } else {
              sail_u256 z2zE3212;
              {
                z2zE3212 = zrlp_ref_word(zstorage);
                if (have_exception) {  goto end_block_exception_3227;  }
              }
              zstorage_root = zword_to_hash(z2zE3212);
            }
          }
          sail_fixed_bytes_32 zcode_hash;
          {
            bool z2zE3208;
            {
              uint64_t z2zE3207;
              z2zE3207 = zcode.zcontent_len;
              z2zE3208 = (z2zE3207 == UINT64_C(0));
            }
            if (z2zE3208) {  zcode_hash = zKECCAK_EMPTY;  } else {
              sail_u256 z2zE3209;
              {
                z2zE3209 = zrlp_ref_word(zcode);
                if (have_exception) {  goto end_block_exception_3227;  }
              }
              zcode_hash = zword_to_hash(z2zE3209);
            }
          }
          uint64_t z2zE3205;
          {
            z2zE3205 = zrlp_ref_uint64(znonce);
            if (have_exception) {  goto end_block_exception_3227;  }
          }
          sail_u256 z2zE3206;
          {
            z2zE3206 = zrlp_ref_uint_word(zbalance);
            if (have_exception) {  goto end_block_exception_3227;  }
          }
          struct zAccountInfo z3zE3183;
          z3zE3183.zbalance = z2zE3206;
          z3zE3183.zcode_hash = zcode_hash;
          z3zE3183.znonce = z2zE3205;
          z3zE3183.zstorage_root = zstorage_root;
          z3zE3181 = z3zE3183;
          goto finish_match_3224;
        }
      case_3225: ;
        sail_match_failure("decode_state_account");
      finish_match_3224: ;
        z3zE3180 = z3zE3181;
        goto finish_match_3222;
      }
    case_3223: ;
      sail_match_failure("decode_state_account");
    finish_match_3222: ;
      z3zE3179 = z3zE3180;
      goto finish_match_3220;
    }
  case_3221: ;
    sail_match_failure("decode_state_account");
  finish_match_3220: ;
    z3zE3178 = z3zE3179;
    goto finish_match_3218;
  }
case_3219: ;
  sail_match_failure("decode_state_account");
finish_match_3218: ;
  z8zE219 = z3zE3178;
end_function_3226: ;
  return z8zE219;
end_block_exception_3227: ;
  struct zAccountInfo z8zE858 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  return z8zE858;
}

struct zTriePath zpath_new(sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  struct zTriePath z8zE220;
  struct zTriePath z3zE3177;
  z3zE3177.zdata = zdata;
  z3zE3177.zlen = zlen;
  z8zE220 = z3zE3177;
end_function_3216: ;
  return z8zE220;
end_block_exception_3217: ;
  struct zTriePath z8zE859 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE859;
}

struct zTriePath zpath_from_hash(sail_fixed_bytes_32 zh)
{
  struct zTriePath z8zE221;
  z8zE221 = zpath_new(zh, UINT64_C(64));
end_function_3214: ;
  return z8zE221;
end_block_exception_3215: ;
  struct zTriePath z8zE860 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE860;
}

struct zByteSliceFields znode_db_lookup(sail_fixed_bytes_32 zh)
{
  struct zByteSliceFields z8zE222;
  z8zE222 = nodedb_lookup(zh);
end_function_3212: ;
  return z8zE222;
end_block_exception_3213: ;
  struct zByteSliceFields z8zE861 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE861;
}

void zbranch_refs_get(struct zNodeRef *z8zE223, zz5vecz8z5unionz0zzNodeRefz9 zchildren, uint64_t zindex)
{
  struct zNodeRef z3zE3146;
  CREATE(zNodeRef)(&z3zE3146);
  {
    uint64_t zp0z3;
    zp0z3 = zindex;
    bool z3zE3162;
    z3zE3162 = (zp0z3 == UINT64_C(0x0));
    if (!(z3zE3162)) {  goto case_3209;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(0));
    goto finish_match_3193;
  }
case_3209: ;
  {
    uint64_t z3zE3163;
    z3zE3163 = zindex;
    bool z3zE3161;
    z3zE3161 = (z3zE3163 == UINT64_C(0x1));
    if (!(z3zE3161)) {  goto case_3208;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(1));
    goto finish_match_3193;
  }
case_3208: ;
  {
    uint64_t z3zE3164;
    z3zE3164 = zindex;
    bool z3zE3160;
    z3zE3160 = (z3zE3164 == UINT64_C(0x2));
    if (!(z3zE3160)) {  goto case_3207;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(2));
    goto finish_match_3193;
  }
case_3207: ;
  {
    uint64_t z3zE3165;
    z3zE3165 = zindex;
    bool z3zE3159;
    z3zE3159 = (z3zE3165 == UINT64_C(0x3));
    if (!(z3zE3159)) {  goto case_3206;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(3));
    goto finish_match_3193;
  }
case_3206: ;
  {
    uint64_t z3zE3166;
    z3zE3166 = zindex;
    bool z3zE3158;
    z3zE3158 = (z3zE3166 == UINT64_C(0x4));
    if (!(z3zE3158)) {  goto case_3205;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(4));
    goto finish_match_3193;
  }
case_3205: ;
  {
    uint64_t z3zE3167;
    z3zE3167 = zindex;
    bool z3zE3157;
    z3zE3157 = (z3zE3167 == UINT64_C(0x5));
    if (!(z3zE3157)) {  goto case_3204;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(5));
    goto finish_match_3193;
  }
case_3204: ;
  {
    uint64_t z3zE3168;
    z3zE3168 = zindex;
    bool z3zE3156;
    z3zE3156 = (z3zE3168 == UINT64_C(0x6));
    if (!(z3zE3156)) {  goto case_3203;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(6));
    goto finish_match_3193;
  }
case_3203: ;
  {
    uint64_t z3zE3169;
    z3zE3169 = zindex;
    bool z3zE3155;
    z3zE3155 = (z3zE3169 == UINT64_C(0x7));
    if (!(z3zE3155)) {  goto case_3202;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(7));
    goto finish_match_3193;
  }
case_3202: ;
  {
    uint64_t z3zE3170;
    z3zE3170 = zindex;
    bool z3zE3154;
    z3zE3154 = (z3zE3170 == UINT64_C(0x8));
    if (!(z3zE3154)) {  goto case_3201;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(8));
    goto finish_match_3193;
  }
case_3201: ;
  {
    uint64_t z3zE3171;
    z3zE3171 = zindex;
    bool z3zE3153;
    z3zE3153 = (z3zE3171 == UINT64_C(0x9));
    if (!(z3zE3153)) {  goto case_3200;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(9));
    goto finish_match_3193;
  }
case_3200: ;
  {
    uint64_t z3zE3172;
    z3zE3172 = zindex;
    bool z3zE3152;
    z3zE3152 = (z3zE3172 == UINT64_C(0xA));
    if (!(z3zE3152)) {  goto case_3199;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(10));
    goto finish_match_3193;
  }
case_3199: ;
  {
    uint64_t z3zE3173;
    z3zE3173 = zindex;
    bool z3zE3151;
    z3zE3151 = (z3zE3173 == UINT64_C(0xB));
    if (!(z3zE3151)) {  goto case_3198;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(11));
    goto finish_match_3193;
  }
case_3198: ;
  {
    uint64_t z3zE3174;
    z3zE3174 = zindex;
    bool z3zE3150;
    z3zE3150 = (z3zE3174 == UINT64_C(0xC));
    if (!(z3zE3150)) {  goto case_3197;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(12));
    goto finish_match_3193;
  }
case_3197: ;
  {
    uint64_t z3zE3175;
    z3zE3175 = zindex;
    bool z3zE3149;
    z3zE3149 = (z3zE3175 == UINT64_C(0xD));
    if (!(z3zE3149)) {  goto case_3196;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(13));
    goto finish_match_3193;
  }
case_3196: ;
  {
    uint64_t z3zE3176;
    z3zE3176 = zindex;
    bool z3zE3148;
    z3zE3148 = (z3zE3176 == UINT64_C(0xE));
    if (!(z3zE3148)) {  goto case_3195;  }
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(14));
    goto finish_match_3193;
  }
case_3195: ;
  {
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE3146, zchildren, UINT64_C(15));
    goto finish_match_3193;
  }
case_3194: ;
finish_match_3193: ;
  COPY(zNodeRef)((*(&z8zE223)), z3zE3146);
  KILL(zNodeRef)(&z3zE3146);
end_function_3210: ;
  goto end_function_4060;
end_block_exception_3211: ;
  goto end_function_4060;
end_function_4060: ;
}

void create_letbind_96(void) {

  uint64_t z3zE129;
  z3zE129 = zWORD_BYTE_LENGTH;
  zMPT_HASH_LENGTH = z3zE129;
let_end_334: ;
}
void kill_letbind_96(void) {
}

struct zInlineNode zinline_node_from_slice(struct zByteSliceFields zbytes)
{
  struct zInlineNode z8zE224;
  uint64_t zlength;
  zlength = zbytes.zlen;
  bool z2zE3196;
  z2zE3196 = (!(zlength < zMPT_HASH_LENGTH));
  unit z3zE3144;
  if (z2zE3196) {
    struct zexception z2zE3197;
    CREATE(zexception)(&z2zE3197);
    zInvalidBlock(&z2zE3197, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE3197);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:23.8-23.44");
    KILL(zexception)(&z2zE3197);
    goto end_block_exception_3192;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3197);
  } else {  z3zE3144 = UNIT;  }
  sail_fixed_bytes_32 z2zE3199;
  {
    sail_u256 z2zE3198;
    z2zE3198 = zslice_load(zbytes, UINT64_C(0));
    z2zE3199 = zword_to_hash(z2zE3198);
  }
  struct zInlineNode z3zE3145;
  z3zE3145.zdata = z2zE3199;
  z3zE3145.zlen = zlength;
  z8zE224 = z3zE3145;
end_function_3191: ;
  return z8zE224;
end_block_exception_3192: ;
  struct zInlineNode z8zE862 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE862;
}

void zfield_to_ref(struct zNodeRef *z8zE225, struct zRlpFieldRef zf)
{
  bool z2zE3189;
  z2zE3189 = zf.zis_list;
  if (z2zE3189) {
    struct zInlineNode z2zE3191;
    {
      struct zByteSliceFields z2zE3190;
      z2zE3190 = zf.zsource;
      {
        z2zE3191 = zinline_node_from_slice(z2zE3190);
        if (have_exception) {  goto end_block_exception_3190;  }
      }
    }
    zInlineRef((*(&z8zE225)), z2zE3191);
  } else {
    bool z2zE3193;
    {
      uint64_t z2zE3192;
      z2zE3192 = zf.zcontent_len;
      z2zE3193 = (z2zE3192 == zMPT_HASH_LENGTH);
    }
    if (z2zE3193) {
      sail_fixed_bytes_32 z2zE3195;
      {
        sail_u256 z2zE3194;
        {
          z2zE3194 = zrlp_ref_word(zf);
          if (have_exception) {  goto end_block_exception_3190;  }
        }
        z2zE3195 = zword_to_hash(z2zE3194);
      }
      zHashRef((*(&z8zE225)), z2zE3195);
    } else {  zEmptyRef((*(&z8zE225)), UNIT);  }
  }
end_function_3189: ;
  goto end_function_4059;
end_block_exception_3190: ;
  goto end_function_4059;
end_function_4059: ;
}

void zdecode_branch_node(struct zTrieNode *z8zE226, struct zRlpCursor zcursor, uint64_t zindex, zz5vecz8z5unionz0zzNodeRefz9 zchildren)
{
  bool z2zE3182;
  z2zE3182 = (zindex < UINT64_C(16));
  if (z2zE3182) {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3183;
    {
      z2zE3183 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3188;  }
    }
    struct zTrieNode z3zE3141;
    CREATE(zTrieNode)(&z3zE3141);
    {
      struct zRlpFieldRef zchild;
      zchild = z2zE3183.ztup0;
      struct zRlpCursor znext;
      znext = z2zE3183.ztup1;
      zz5vecz8z5unionz0zzNodeRefz9 zupdated;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated, zchildren);
      struct zNodeRef z2zE3184;
      CREATE(zNodeRef)(&z2zE3184);
      {
        zfield_to_ref(&z2zE3184, zchild);
        if (have_exception) {
          KILL(zTrieNode)(&z3zE3141);
          KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
          KILL(zNodeRef)(&z2zE3184);
          goto end_block_exception_3188;
        }
      }
      fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&zupdated, zupdated, zindex, z2zE3184);
      unit z3zE3142;
      z3zE3142 = UNIT;
      KILL(zNodeRef)(&z2zE3184);
      uint64_t z2zE3185;
      {    z2zE3185 = (zindex + UINT64_C(1));
      }
      {
        zdecode_branch_node(&z3zE3141, znext, z2zE3185, zupdated);
        if (have_exception) {
          KILL(zTrieNode)(&z3zE3141);
          KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
          goto end_block_exception_3188;
        }
      }
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zupdated);
      goto finish_match_3185;
    }
  case_3186: ;
    sail_match_failure("decode_branch_node");
  finish_match_3185: ;
    COPY(zTrieNode)((*(&z8zE226)), z3zE3141);
    KILL(zTrieNode)(&z3zE3141);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3186;
    {
      z2zE3186 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3188;  }
    }
    struct zTrieNode z3zE3137;
    CREATE(zTrieNode)(&z3zE3137);
    {
      struct zRlpFieldRef zvalue;
      zvalue = z2zE3186.ztup0;
      struct zRlpCursor z1zE5;
      z1zE5 = z2zE3186.ztup1;
      unit z3zE3138;
      {
        z3zE3138 = zrlp_cursor_expect_end(z1zE5);
        if (have_exception) {
          KILL(zTrieNode)(&z3zE3137);
          goto end_block_exception_3188;
        }
      }
      struct ztuple_z8z5vecz8z5unionz0zzNodeRefz9zCz0z5structz0zzByteSliceFieldsz9 z2zE3188;
      CREATE(ztuple_z8z5vecz8z5unionz0zzNodeRefz9zCz0z5structz0zzByteSliceFieldsz9)(&z2zE3188);
      {
        struct zByteSliceFields z2zE3187;
        z2zE3187 = zrlp_ref_content(zvalue);
        COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&z2zE3188)->ztup0), zchildren);
        z2zE3188.ztup1 = z2zE3187;
      }
      zBranchNode(&z3zE3137, z2zE3188);
      KILL(ztuple_z8z5vecz8z5unionz0zzNodeRefz9zCz0z5structz0zzByteSliceFieldsz9)(&z2zE3188);
      goto finish_match_3183;
    }
  case_3184: ;
    sail_match_failure("decode_branch_node");
  finish_match_3183: ;
    COPY(zTrieNode)((*(&z8zE226)), z3zE3137);
    KILL(zTrieNode)(&z3zE3137);
  }
end_function_3187: ;
  goto end_function_4058;
end_block_exception_3188: ;
  goto end_function_4058;
end_function_4058: ;
}

void create_letbind_97(void) {

  uint64_t z3zE130;
  z3zE130 = UINT64_C(33);
  zHEX_PREFIX_MAX_LENGTH = z3zE130;
let_end_338: ;
}
void kill_letbind_97(void) {
}

struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 zhex_prefix_decode_ref(struct zRlpFieldRef zf)
{
  struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z8zE227;
  bool z2zE3159;
  z2zE3159 = zf.zis_list;
  unit z3zE3130;
  if (z2zE3159) {
    struct zexception z2zE3160;
    CREATE(zexception)(&z2zE3160);
    zInvalidBlock(&z2zE3160, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3160);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:260.8-260.37");
    KILL(zexception)(&z2zE3160);
    goto end_block_exception_3182;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3160);
  } else {  z3zE3130 = UNIT;  }
  uint64_t zn;
  zn = zf.zcontent_len;
  bool z2zE3161;
  z2zE3161 = (zn == UINT64_C(0));
  if (z2zE3161) {
    struct zTriePath z3zE3134;
    zz5vecz8z5bv8z9 z3zE3135;
    CREATE(zz5vecz8z5bv8z9)(&z3zE3135);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE3135, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3135, z3zE3135, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE863 = 0; z8zE863 < 32; ++z8zE863) {
      z3zE3134.zdata.bytes[z8zE863] = (uint8_t)(z3zE3135.data[z8zE863] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE3135);
    z3zE3134.zlen = UINT64_C(0);
    struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE3136;
    z3zE3136.ztup0 = false;
    z3zE3136.ztup1 = z3zE3134;
    z8zE227 = z3zE3136;
  } else {
    uint64_t zmaximum_length;
    zmaximum_length = zHEX_PREFIX_MAX_LENGTH;
    bool z2zE3162;
    z2zE3162 = (zmaximum_length < zn);
    if (z2zE3162) {
      struct zexception z2zE3163;
      CREATE(zexception)(&z2zE3163);
      zInvalidBlock(&z2zE3163, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3163);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:268.12-268.41");
      KILL(zexception)(&z2zE3163);
      goto end_block_exception_3182;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3163);
    } else {
      struct zByteSliceFields zcontent;
      {
        struct zByteSliceFields z2zE3180;
        z2zE3180 = zf.zsource;
        uint64_t z2zE3181;
        {
          uint64_t z2zE3179;
          {
            struct zByteSliceFields z2zE3178;
            z2zE3178 = zf.zsource;
            z2zE3179 = z2zE3178.zlen;
          }
          {    z2zE3181 = (z2zE3179 - zn);
          }
        }
        zcontent = zsub_slice(z2zE3180, z2zE3181, zn);
      }
      uint64_t zfb;
      zfb = zslice_byte(zcontent, UINT64_C(0));
      uint64_t zflag;
      zflag = (safe_rshift(UINT64_MAX, 64 - 4) & (zfb >> UINT64_C(4)));
      bool zis_leaf;
      {
        uint64_t z2zE3177;
        z2zE3177 = (UINT64_C(1) & (zflag >> UINT64_C(1)));
        zis_leaf = (z2zE3177 == UINT64_C(0b1));
      }
      bool zodd;
      {
        uint64_t z2zE3176;
        z2zE3176 = (UINT64_C(1) & (zflag >> UINT64_C(0)));
        zodd = (z2zE3176 == UINT64_C(0b1));
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
        bool z2zE3164;
        z2zE3164 = (zpaired_nibbles < UINT64_C(64));
        if (z2zE3164) {
          sail_fixed_bytes_32 zbytes;
          {
            sail_u256 z2zE3172;
            z2zE3172 = zword_shift_right(zpacked, UINT64_C(4));
            zbytes = zword_to_hash(z2zE3172);
          }
          uint64_t z2zE3168;
          {
            uint64_t z2zE3166;
            z2zE3166 = (safe_rshift(UINT64_MAX, 64 - 4) & (zfb >> UINT64_C(0)));
            uint64_t z2zE3167;
            {
              uint64_t z2zE3165;
              z2zE3165 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(31));
              z2zE3167 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE3165 >> UINT64_C(0)));
            }
            z2zE3168 = (z2zE3166 << 4) | z2zE3167;
          }
          zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, UINT64_C(31), z2zE3168);
          unit z3zE3132;
          z3zE3132 = UNIT;
          struct zTriePath z2zE3171;
          {
            sail_fixed_bytes_32 z2zE3169;
            z2zE3169 = zB256(zbytes);
            uint64_t z2zE3170;
            {    z2zE3170 = (zpaired_nibbles + UINT64_C(1));
            }
            z2zE3171 = zpath_new(z2zE3169, z2zE3170);
          }
          struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE3133;
          z3zE3133.ztup0 = zis_leaf;
          z3zE3133.ztup1 = z2zE3171;
          z8zE227 = z3zE3133;
        } else {
          struct zexception z2zE3173;
          CREATE(zexception)(&z2zE3173);
          zInvalidBlock(&z2zE3173, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE3173);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:285.20-285.56");
          KILL(zexception)(&z2zE3173);
          goto end_block_exception_3182;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3173);
        }
      } else {
        struct zTriePath z2zE3175;
        {
          sail_fixed_bytes_32 z2zE3174;
          z2zE3174 = zword_to_hash(zpacked);
          z2zE3175 = zpath_new(z2zE3174, zpaired_nibbles);
        }
        struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE3131;
        z3zE3131.ztup0 = zis_leaf;
        z3zE3131.ztup1 = z2zE3175;
        z8zE227 = z3zE3131;
      }
    }
  }
end_function_3181: ;
  return z8zE227;
end_block_exception_3182: ;
  struct zTriePath z8zE865 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z8zE864 = { .ztup0 = false, .ztup1 = z8zE865 };
  return z8zE864;
}

uint64_t zpath_len(struct zTriePath zpath)
{
  uint64_t z8zE228;
  z8zE228 = zpath.zlen;
end_function_3179: ;
  return z8zE228;
end_block_exception_3180: ;

  return UINT64_C(0xdeadc0de);
}

void zdecode_trie_node(struct zTrieNode *z8zE229, struct zByteSliceFields znode)
{
  struct zRlpCursor zfields;
  {
    zfields = zrlp_node_cursor(znode);
    if (have_exception) {  goto end_block_exception_3178;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3145;
  {
    z2zE3145 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3178;  }
  }
  struct zTrieNode z3zE3120;
  CREATE(zTrieNode)(&z3zE3120);
  {
    struct zRlpFieldRef zfirst;
    zfirst = z2zE3145.ztup0;
    struct zRlpCursor z1zE3;
    z1zE3 = z2zE3145.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3146;
    {
      z2zE3146 = zrlp_cursor_pop(z1zE3);
      if (have_exception) {
        KILL(zTrieNode)(&z3zE3120);
        goto end_block_exception_3178;
      }
    }
    struct zTrieNode z3zE3121;
    CREATE(zTrieNode)(&z3zE3121);
    {
      struct zRlpFieldRef zsecond;
      zsecond = z2zE3146.ztup0;
      struct zRlpCursor z1zE4;
      z1zE4 = z2zE3146.ztup1;
      bool z2zE3147;
      z2zE3147 = zrlp_cursor_empty(z1zE4);
      if (z2zE3147) {
        struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z2zE3148;
        {
          z2zE3148 = zhex_prefix_decode_ref(zfirst);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE3120);
            KILL(zTrieNode)(&z3zE3121);
            goto end_block_exception_3178;
          }
        }
        struct zTrieNode z3zE3125;
        CREATE(zTrieNode)(&z3zE3125);
        {
          bool zis_leaf;
          zis_leaf = z2zE3148.ztup0;
          struct zTriePath zpath;
          zpath = z2zE3148.ztup1;
          if (zis_leaf) {
            struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzByteSliceFieldsz9 z2zE3150;
            {
              struct zByteSliceFields z2zE3149;
              z2zE3149 = zrlp_ref_content(zsecond);
              struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzByteSliceFieldsz9 z3zE3128;
              z3zE3128.ztup0 = zpath;
              z3zE3128.ztup1 = z2zE3149;
              z2zE3150 = z3zE3128;
            }
            zLeafNode(&z3zE3125, z2zE3150);
          } else {
            bool z2zE3152;
            {
              uint64_t z2zE3151;
              z2zE3151 = zpath_len(zpath);
              z2zE3152 = (z2zE3151 == UINT64_C(0));
            }
            if (z2zE3152) {
              struct zexception z2zE3153;
              CREATE(zexception)(&z2zE3153);
              zInvalidBlock(&z2zE3153, zRlpDecode);
              COPY(zexception)(current_exception, z2zE3153);
              have_exception = true;
              COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:272.16-272.45");
              KILL(zTrieNode)(&z3zE3120);
              KILL(zTrieNode)(&z3zE3121);
              KILL(zTrieNode)(&z3zE3125);
              KILL(zexception)(&z2zE3153);
              goto end_block_exception_3178;
              /* unreachable after throw */
              KILL(zexception)(&z2zE3153);
            } else {
              struct ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9 z2zE3155;
              CREATE(ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9)(&z2zE3155);
              {
                struct zNodeRef z2zE3154;
                CREATE(zNodeRef)(&z2zE3154);
                {
                  zfield_to_ref(&z2zE3154, zsecond);
                  if (have_exception) {
                    KILL(zTrieNode)(&z3zE3120);
                    KILL(zTrieNode)(&z3zE3121);
                    KILL(zTrieNode)(&z3zE3125);
                    KILL(ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9)(&z2zE3155);
                    KILL(zNodeRef)(&z2zE3154);
                    goto end_block_exception_3178;
                  }
                }
                z2zE3155.ztup0 = zpath;
                COPY(zNodeRef)(&((&z2zE3155)->ztup1), z2zE3154);
                KILL(zNodeRef)(&z2zE3154);
              }
              zExtensionNode(&z3zE3125, z2zE3155);
              KILL(ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9)(&z2zE3155);
            }
          }
          goto finish_match_3175;
        }
      case_3176: ;
        sail_match_failure("decode_trie_node");
      finish_match_3175: ;
        COPY(zTrieNode)(&z3zE3121, z3zE3125);
        KILL(zTrieNode)(&z3zE3125);
      } else {
        zz5vecz8z5unionz0zzNodeRefz9 zchildren;
        CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
        {
          struct zNodeRef z2zE3158;
          CREATE(zNodeRef)(&z2zE3158);
          zEmptyRef(&z2zE3158, UNIT);
          fast_unsigned_vector_init_zz5vecz8z5unionz0zzNodeRefz9(&zchildren, UINT64_C(16), z2zE3158);
          KILL(zNodeRef)(&z2zE3158);
        }
        struct zNodeRef z2zE3156;
        CREATE(zNodeRef)(&z2zE3156);
        {
          zfield_to_ref(&z2zE3156, zfirst);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE3120);
            KILL(zTrieNode)(&z3zE3121);
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
            KILL(zNodeRef)(&z2zE3156);
            goto end_block_exception_3178;
          }
        }
        fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&zchildren, zchildren, UINT64_C(0), z2zE3156);
        unit z3zE3124;
        z3zE3124 = UNIT;
        KILL(zNodeRef)(&z2zE3156);
        struct zNodeRef z2zE3157;
        CREATE(zNodeRef)(&z2zE3157);
        {
          zfield_to_ref(&z2zE3157, zsecond);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE3120);
            KILL(zTrieNode)(&z3zE3121);
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
            KILL(zNodeRef)(&z2zE3157);
            goto end_block_exception_3178;
          }
        }
        fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&zchildren, zchildren, UINT64_C(1), z2zE3157);
        unit z3zE3123;
        z3zE3123 = UNIT;
        KILL(zNodeRef)(&z2zE3157);
        {
          zdecode_branch_node(&z3zE3121, z1zE4, UINT64_C(2), zchildren);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE3120);
            KILL(zTrieNode)(&z3zE3121);
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
            goto end_block_exception_3178;
          }
        }
        KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      }
      goto finish_match_3173;
    }
  case_3174: ;
    sail_match_failure("decode_trie_node");
  finish_match_3173: ;
    COPY(zTrieNode)(&z3zE3120, z3zE3121);
    KILL(zTrieNode)(&z3zE3121);
    goto finish_match_3171;
  }
case_3172: ;
  sail_match_failure("decode_trie_node");
finish_match_3171: ;
  COPY(zTrieNode)((*(&z8zE229)), z3zE3120);
  KILL(zTrieNode)(&z3zE3120);
end_function_3177: ;
  goto end_function_4057;
end_block_exception_3178: ;
  goto end_function_4057;
end_function_4057: ;
}

uint64_t zpath_byte_index(uint64_t zi)
{
  uint64_t z8zE230;
  uint64_t zquotient;
  {    zquotient = (zi / UINT64_C(2));
  }
  uint64_t znatural_index;
  {
    bool z2zE3144;
    {
      bool z2zE3143;
      z2zE3143 = (!(zquotient < UINT64_C(0)));
      bool z3zE3118;
      if (z2zE3143) {  z3zE3118 = (!(UINT64_C(31) < zquotient));  } else {  z3zE3118 = false;  }
      z2zE3144 = z3zE3118;
    }
    if (z2zE3144) {  znatural_index = zquotient;  } else {
      unit z3zE3119;
      z3zE3119 = sail_assert(false, "sail/lib/mpt/primitives.sail:64.24-64.25");
      sail_match_failure("path_byte_index");
      /* unreachable after exit */
    }
  }
  {    z8zE230 = (UINT64_C(31) - znatural_index);
  }
end_function_3169: ;
  return z8zE230;
end_block_exception_3170: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zpath_nibble(struct zTriePath zpath, uint64_t zi)
{
  uint64_t z8zE231;
  bool z2zE3138;
  {
    uint64_t z2zE3137;
    z2zE3137 = zpath_len(zpath);
    z2zE3138 = (!(zi < z2zE3137));
  }
  if (z2zE3138) {  z8zE231 = UINT64_C(0x0);  } else {
    sail_fixed_bytes_32 zbytes;
    zbytes = zpath.zdata;
    uint64_t zbyte_index;
    zbyte_index = zpath_byte_index(zi);
    bool z2zE3140;
    {
      uint64_t z2zE3139;
      {    z2zE3139 = (zi % UINT64_C(2));
      }
      z2zE3140 = (z2zE3139 == UINT64_C(0));
    }
    if (z2zE3140) {
      uint64_t z2zE3141;
      z2zE3141 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
      z8zE231 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE3141 >> UINT64_C(4)));
    } else {
      uint64_t z2zE3142;
      z2zE3142 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
      z8zE231 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE3142 >> UINT64_C(0)));
    }
  }
end_function_3167: ;
  return z8zE231;
end_block_exception_3168: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztrie_path_len_increment(uint64_t zvalue)
{
  uint64_t z8zE232;
  bool z2zE3135;
  z2zE3135 = (zvalue < UINT64_C(64));
  if (z2zE3135) {
    {    z8zE232 = (zvalue + UINT64_C(1));
    }
  } else {
    struct zexception z2zE3136;
    CREATE(zexception)(&z2zE3136);
    zInvalidBlock(&z2zE3136, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE3136);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:35.8-35.44");
    KILL(zexception)(&z2zE3136);
    goto end_block_exception_3166;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3136);
  }
end_function_3165: ;
  return z8zE232;
end_block_exception_3166: ;

  return UINT64_C(0xdeadc0de);
}

bool zpath_matches(struct zTriePath zkey, uint64_t zpos, struct zTriePath zseg)
{
  bool z8zE233;
  uint64_t zstop;
  {
    uint64_t z2zE3134;
    z2zE3134 = zpath_len(zseg);
    {    zstop = (zpos + z2zE3134);
    }
  }
  bool z2zE3127;
  {
    uint64_t z2zE3126;
    z2zE3126 = zpath_len(zkey);
    z2zE3127 = (z2zE3126 < zstop);
  }
  if (z2zE3127) {  z8zE233 = false;  } else {
    bool zok;
    zok = true;
    uint64_t zoffset;
    zoffset = UINT64_C(0);
    int64_t z3zE3112;
    {    z3zE3112 = (int64_t)(UINT64_C(0));
    }
    int64_t z3zE3113;
    {    z3zE3113 = (int64_t)(UINT64_C(63));
    }
    int64_t z3zE3114;
    {    z3zE3114 = (int64_t)(UINT64_C(1));
    }
    {
      int64_t z_step;
      z_step = z3zE3112;
      unit z3zE3115;
    for_start_3161: ;
      {
        if ((z3zE3113 < z_step)) goto for_end_3162;
        bool z2zE3129;
        {
          uint64_t z2zE3128;
          z2zE3128 = zpath_len(zseg);
          z2zE3129 = (zoffset < z2zE3128);
        }
        if (z2zE3129) {
          uint64_t zkey_index;
          {    zkey_index = (zpos + zoffset);
          }
          bool z2zE3130;
          z2zE3130 = (!(UINT64_C(64) < zkey_index));
          unit z3zE3116;
          if (z2zE3130) {
            bool z2zE3133;
            {
              uint64_t z2zE3131;
              z2zE3131 = zpath_nibble(zkey, zkey_index);
              uint64_t z2zE3132;
              z2zE3132 = zpath_nibble(zseg, zoffset);
              z2zE3133 = (z2zE3131 != z2zE3132);
            }
            if (z2zE3133) {
              zok = false;
              z3zE3116 = UNIT;
            } else {  z3zE3116 = UNIT;  }
          } else {
            zok = false;
            z3zE3116 = UNIT;
          }
          {
            zoffset = ztrie_path_len_increment(zoffset);
            if (have_exception) {  goto end_block_exception_3164;  }
          }
          z3zE3115 = UNIT;
        } else {  z3zE3115 = UNIT;  }
        z_step = (z_step + z3zE3114);
        goto for_start_3161;
      }
    for_end_3162: ;
    }
    unit z3zE3117;
    z3zE3117 = UNIT;
    z8zE233 = zok;
  }
end_function_3163: ;
  return z8zE233;
end_block_exception_3164: ;

  return false;
}

struct zByteSliceFields zinline_node_slice(struct zInlineNode znode)
{
  struct zByteSliceFields z8zE234;
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  sail_fixed_bytes_32 z2zE3124;
  z2zE3124 = znode.zdata;
  uint64_t z2zE3125;
  z2zE3125 = znode.zlen;
  unit z3zE3111;
  z3zE3111 = zscratch_push_b256(z2zE3124, z2zE3125);
  z8zE234 = zscratch_finish(zstart);
end_function_3159: ;
  return z8zE234;
end_block_exception_3160: ;
  struct zByteSliceFields z8zE866 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE866;
}

struct zByteSliceFields zresolve_ref(struct zNodeRef zr)
{
  struct zByteSliceFields z8zE235;
  struct zByteSliceFields z3zE3106;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_3156;
    z3zE3106 = zEMPTY_SLICE;
    goto finish_match_3153;
  }
case_3156: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_3155;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE3106 = zinline_node_slice(znode);
    goto finish_match_3153;
  }
case_3155: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    struct zByteSliceFields z3zE3110;
    z3zE3110 = znode_db_lookup(zh);
    bool z2zE3122;
    {
      uint64_t z2zE3121;
      z2zE3121 = z3zE3110.zlen;
      z2zE3122 = (z2zE3121 == UINT64_C(0));
    }
    if (z2zE3122) {
      struct zexception z2zE3123;
      CREATE(zexception)(&z2zE3123);
      zInvalidBlock(&z2zE3123, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3123);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:295.16-295.52");
      KILL(zexception)(&z2zE3123);
      goto end_block_exception_3158;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3123);
    } else {  z3zE3106 = z3zE3110;  }
    goto finish_match_3153;
  }
case_3154: ;
finish_match_3153: ;
  z8zE235 = z3zE3106;
end_function_3157: ;
  return z8zE235;
end_block_exception_3158: ;
  struct zByteSliceFields z8zE867 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE867;
}

struct zByteSliceFields ztrie_walk(struct zByteSliceFields znode, struct zTriePath zkey, uint64_t zpos)
{
  struct zByteSliceFields z8zE236;
  bool z2zE3101;
  {
    uint64_t z2zE3100;
    z2zE3100 = znode.zlen;
    z2zE3101 = (z2zE3100 == UINT64_C(0));
  }
  if (z2zE3101) {  z8zE236 = zEMPTY_SLICE;  } else {
    struct zTrieNode z2zE3102;
    CREATE(zTrieNode)(&z2zE3102);
    {
      zdecode_trie_node(&z2zE3102, znode);
      if (have_exception) {
        KILL(zTrieNode)(&z2zE3102);
        goto end_block_exception_3152;
      }
    }
    struct zByteSliceFields z3zE3100;
    {
      if (z2zE3102.kind != Kind_zLeafNode) goto case_3150;
      struct zTriePath zpath;
      zpath = z2zE3102.variants.zLeafNode.ztup0;
      struct zByteSliceFields zvalue;
      zvalue = z2zE3102.variants.zLeafNode.ztup1;
      bool z2zE3104;
      {
        bool z2zE3103;
        {
          z2zE3103 = zpath_matches(zkey, zpos, zpath);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE3102);
            goto end_block_exception_3152;
          }
        }
        z2zE3104 = not(z2zE3103);
      }
      if (z2zE3104) {  z3zE3100 = zEMPTY_SLICE;  } else {
        bool z2zE3108;
        {
          uint64_t z2zE3106;
          {
            uint64_t z2zE3105;
            z2zE3105 = zpath_len(zpath);
            {    z2zE3106 = (zpos + z2zE3105);
            }
          }
          uint64_t z2zE3107;
          z2zE3107 = zpath_len(zkey);
          z2zE3108 = (z2zE3106 == z2zE3107);
        }
        if (z2zE3108) {  z3zE3100 = zvalue;  } else {  z3zE3100 = zEMPTY_SLICE;  }
      }
      goto finish_match_3147;
    }
  case_3150: ;
    {
      if (z2zE3102.kind != Kind_zExtensionNode) goto case_3149;
      struct zTriePath z3zE3104;
      z3zE3104 = z2zE3102.variants.zExtensionNode.ztup0;
      struct zNodeRef zchildref;
      CREATE(zNodeRef)(&zchildref);
      COPY(zNodeRef)(&zchildref, z2zE3102.variants.zExtensionNode.ztup1);
      uint64_t zextension_len;
      zextension_len = zpath_len(z3zE3104);
      bool z2zE3109;
      z2zE3109 = (zextension_len == UINT64_C(0));
      if (z2zE3109) {  z3zE3100 = zEMPTY_SLICE;  } else {
        bool z2zE3111;
        {
          bool z2zE3110;
          {
            z2zE3110 = zpath_matches(zkey, zpos, z3zE3104);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE3102);
              KILL(zNodeRef)(&zchildref);
              goto end_block_exception_3152;
            }
          }
          z2zE3111 = not(z2zE3110);
        }
        if (z2zE3111) {  z3zE3100 = zEMPTY_SLICE;  } else {
          uint64_t znext_pos;
          {    znext_pos = (zpos + zextension_len);
          }
          bool z2zE3112;
          z2zE3112 = (!(UINT64_C(64) < znext_pos));
          if (z2zE3112) {
            struct zByteSliceFields z2zE3113;
            {
              z2zE3113 = zresolve_ref(zchildref);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE3102);
                KILL(zNodeRef)(&zchildref);
                goto end_block_exception_3152;
              }
            }
            {
              z3zE3100 = ztrie_walk(z2zE3113, zkey, znext_pos);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE3102);
                KILL(zNodeRef)(&zchildref);
                goto end_block_exception_3152;
              }
            }
          } else {  z3zE3100 = zEMPTY_SLICE;  }
        }
      }
      KILL(zNodeRef)(&zchildref);
      goto finish_match_3147;
    }
  case_3149: ;
    {
      /* complete */
      zz5vecz8z5unionz0zzNodeRefz9 zchildren;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren, z2zE3102.variants.zBranchNode.ztup0);
      struct zByteSliceFields z3zE3105;
      z3zE3105 = z2zE3102.variants.zBranchNode.ztup1;
      bool z2zE3115;
      {
        uint64_t z2zE3114;
        z2zE3114 = zpath_len(zkey);
        z2zE3115 = (zpos == z2zE3114);
      }
      if (z2zE3115) {  z3zE3100 = z3zE3105;  } else {
        bool z2zE3117;
        {
          uint64_t z2zE3116;
          z2zE3116 = zpath_len(zkey);
          z2zE3117 = (zpos < z2zE3116);
        }
        if (z2zE3117) {
          struct zByteSliceFields zchild;
          {
            struct zNodeRef z2zE3120;
            CREATE(zNodeRef)(&z2zE3120);
            {
              uint64_t z2zE3119;
              z2zE3119 = zpath_nibble(zkey, zpos);
              zbranch_refs_get(&z2zE3120, zchildren, z2zE3119);
            }
            {
              zchild = zresolve_ref(z2zE3120);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE3102);
                KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                KILL(zNodeRef)(&z2zE3120);
                goto end_block_exception_3152;
              }
            }
            KILL(zNodeRef)(&z2zE3120);
          }
          uint64_t z2zE3118;
          {    z2zE3118 = (zpos + UINT64_C(1));
          }
          {
            z3zE3100 = ztrie_walk(zchild, zkey, z2zE3118);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE3102);
              KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
              goto end_block_exception_3152;
            }
          }
        } else {  z3zE3100 = zEMPTY_SLICE;  }
      }
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      goto finish_match_3147;
    }
  case_3148: ;
  finish_match_3147: ;
    z8zE236 = z3zE3100;
    KILL(zTrieNode)(&z2zE3102);
  }
end_function_3151: ;
  return z8zE236;
end_block_exception_3152: ;
  struct zByteSliceFields z8zE868 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE868;
}

struct zByteSliceFields ztrie_lookup(sail_fixed_bytes_32 zroot, struct zTriePath zkey)
{
  struct zByteSliceFields z8zE237;
  bool z2zE3096;
  z2zE3096 = eq_fixed_bytes_32(zroot, zEMPTY_TRIE_ROOT);
  if (z2zE3096) {  z8zE237 = zEMPTY_SLICE;  } else {
    struct zByteSliceFields znode;
    znode = znode_db_lookup(zroot);
    bool z2zE3098;
    {
      uint64_t z2zE3097;
      z2zE3097 = znode.zlen;
      z2zE3098 = (z2zE3097 == UINT64_C(0));
    }
    if (z2zE3098) {
      struct zexception z2zE3099;
      CREATE(zexception)(&z2zE3099);
      zInvalidBlock(&z2zE3099, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3099);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:314.12-314.48");
      KILL(zexception)(&z2zE3099);
      goto end_block_exception_3146;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3099);
    } else {
      {
        z8zE237 = ztrie_walk(znode, zkey, UINT64_C(0));
        if (have_exception) {  goto end_block_exception_3146;  }
      }
    }
  }
end_function_3145: ;
  return z8zE237;
end_block_exception_3146: ;
  struct zByteSliceFields z8zE869 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE869;
}

void zstateless_account(struct zoptionzIRAccountInfozK *z8zE238, sail_fixed_bytes_32 zroot, sail_fixed_bytes_20 za)
{
  struct zByteSliceFields zvalue;
  {
    struct zTriePath z2zE3095;
    {
      sail_fixed_bytes_32 z2zE3094;
      z2zE3094 = zkeccak256_address(za);
      z2zE3095 = zpath_from_hash(z2zE3094);
    }
    {
      zvalue = ztrie_lookup(zroot, z2zE3095);
      if (have_exception) {  goto end_block_exception_3144;  }
    }
  }
  bool z2zE3092;
  {
    uint64_t z2zE3091;
    z2zE3091 = zvalue.zlen;
    z2zE3092 = (z2zE3091 == UINT64_C(0));
  }
  if (z2zE3092) {  zNonezIRAccountInfozK((*(&z8zE238)), UNIT);  } else {
    struct zAccountInfo z2zE3093;
    {
      z2zE3093 = zdecode_state_account(zvalue);
      if (have_exception) {  goto end_block_exception_3144;  }
    }
    zSomezIRAccountInfozK((*(&z8zE238)), z2zE3093);
  }
end_function_3143: ;
  goto end_function_4056;
end_block_exception_3144: ;
  goto end_function_4056;
end_function_4056: ;
}

struct zAccount zk_aload(sail_fixed_bytes_20 za)
{
  struct zAccount z8zE239;
  unit z3zE3093;
  z3zE3093 = bal_note_account_touch(za);
  struct zoptionzIRAccountzK z2zE3088;
  CREATE(zoptionzIRAccountzK)(&z2zE3088);
  acct_tx_get(&z2zE3088, za);
  unit z3zE3085;
  {
    if (z2zE3088.kind != Kind_zSomezIRAccountzK) goto case_3130;
    struct zAccount zacc;
    zacc = z2zE3088.variants.zSomezIRAccountzK;
    z8zE239 = zacc;
    goto cleanup_3140;
    /* unreachable after return */
    goto finish_match_3128;
    goto end_cleanup_3141;
  cleanup_3140: ;
    KILL(zoptionzIRAccountzK)(&z2zE3088);
    goto end_function_3137;
  end_cleanup_3141: ;
  }
case_3130: ;
  {
    /* complete */
    z3zE3085 = UNIT;
    goto finish_match_3128;
  }
case_3129: ;
finish_match_3128: ;
  unit z3zE3092;
  z3zE3092 = z3zE3085;
  KILL(zoptionzIRAccountzK)(&z2zE3088);
  struct zoptionzIRAccountzK z2zE3089;
  CREATE(zoptionzIRAccountzK)(&z2zE3089);
  acct_block_get(&z2zE3089, za);
  unit z3zE3088;
  {
    if (z2zE3089.kind != Kind_zSomezIRAccountzK) goto case_3133;
    struct zAccount z3zE3098;
    z3zE3098 = z2zE3089.variants.zSomezIRAccountzK;
    z8zE239 = z3zE3098;
    goto cleanup_3138;
    /* unreachable after return */
    goto finish_match_3131;
    goto end_cleanup_3139;
  cleanup_3138: ;
    KILL(zoptionzIRAccountzK)(&z2zE3089);
    goto end_function_3137;
  end_cleanup_3139: ;
  }
case_3133: ;
  {
    /* complete */
    z3zE3088 = UNIT;
    goto finish_match_3131;
  }
case_3132: ;
finish_match_3131: ;
  unit z3zE3091;
  z3zE3091 = z3zE3088;
  KILL(zoptionzIRAccountzK)(&z2zE3089);
  struct zAccount z3zE3099;
  {
    struct zoptionzIRAccountInfozK z2zE3090;
    CREATE(zoptionzIRAccountInfozK)(&z2zE3090);
    {
      zstateless_account(&z2zE3090, zk_parent_state_root, za);
      if (have_exception) {
        KILL(zoptionzIRAccountInfozK)(&z2zE3090);
        goto end_block_exception_3142;
      }
    }
    struct zAccount z3zE3094;
    {
      if (z2zE3090.kind != Kind_zSomezIRAccountInfozK) goto case_3136;
      struct zAccountInfo zinfo;
      zinfo = z2zE3090.variants.zSomezIRAccountInfozK;
      z3zE3094 = zaccount_from_info(zinfo);
      goto finish_match_3134;
    }
  case_3136: ;
    {
      /* complete */
      z3zE3094 = zEMPTY_ACCOUNT;
      goto finish_match_3134;
    }
  case_3135: ;
  finish_match_3134: ;
    z3zE3099 = z3zE3094;
    KILL(zoptionzIRAccountInfozK)(&z2zE3090);
  }
  unit z3zE3097;
  z3zE3097 = acct_block_cache(za, z3zE3099);
  z8zE239 = z3zE3099;
end_function_3137: ;
  return z8zE239;
end_block_exception_3142: ;
  struct zAccountInfo z8zE871 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE870 = { .zcreated = false, .zinfo = z8zE871, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE870;
}

sail_u256 zstateless_storage(sail_fixed_bytes_32 zroot, sail_u256 zslot)
{
  sail_u256 z8zE240;
  struct zByteSliceFields zvalue;
  {
    struct zTriePath z2zE3087;
    {
      sail_fixed_bytes_32 z2zE3086;
      z2zE3086 = zkeccak256_word(zslot);
      z2zE3087 = zpath_from_hash(z2zE3086);
    }
    {
      zvalue = ztrie_lookup(zroot, z2zE3087);
      if (have_exception) {  goto end_block_exception_3127;  }
    }
  }
  bool z2zE3084;
  {
    uint64_t z2zE3083;
    z2zE3083 = zvalue.zlen;
    z2zE3084 = (z2zE3083 == UINT64_C(0));
  }
  if (z2zE3084) {  z8zE240 = zZERO_WORD;  } else {
    struct zRlpFieldRef z2zE3085;
    {
      z2zE3085 = zrlp_single_ref(zvalue);
      if (have_exception) {  goto end_block_exception_3127;  }
    }
    {
      z8zE240 = zrlp_ref_uint_word(z2zE3085);
      if (have_exception) {  goto end_block_exception_3127;  }
    }
  }
end_function_3126: ;
  return z8zE240;
end_block_exception_3127: ;

  return u256_zero();
}

struct zStorageValue zk_sload(sail_fixed_bytes_20 za, sail_u256 zs)
{
  struct zStorageValue z8zE241;
  unit z3zE3072;
  z3zE3072 = bal_note_storage_read(za, zs);
  struct zStorageKey zkey;
  zkey = zstorage_key(za, zs);
  struct zoptionzIRStorageValuezK z2zE3075;
  CREATE(zoptionzIRStorageValuezK)(&z2zE3075);
  storage_tx_get(&z2zE3075, zkey);
  unit z3zE3073;
  {
    if (z2zE3075.kind != Kind_zSomezIRStorageValuezK) goto case_3116;
    struct zStorageValue ze;
    ze = z2zE3075.variants.zSomezIRStorageValuezK;
    z8zE241 = ze;
    goto cleanup_3123;
    /* unreachable after return */
    goto finish_match_3114;
    goto end_cleanup_3124;
  cleanup_3123: ;
    KILL(zoptionzIRStorageValuezK)(&z2zE3075);
    goto end_function_3120;
  end_cleanup_3124: ;
  }
case_3116: ;
  {
    /* complete */
    z3zE3073 = UNIT;
    goto finish_match_3114;
  }
case_3115: ;
finish_match_3114: ;
  unit z3zE3076;
  z3zE3076 = z3zE3073;
  KILL(zoptionzIRStorageValuezK)(&z2zE3075);
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3125;  }
  }
  struct zoptionzIRStorageValuezK z2zE3076;
  CREATE(zoptionzIRStorageValuezK)(&z2zE3076);
  storage_block_get(&z2zE3076, zkey);
  unit z3zE3077;
  {
    if (z2zE3076.kind != Kind_zSomezIRStorageValuezK) goto case_3119;
    struct zStorageValue z3zE3084;
    z3zE3084 = z2zE3076.variants.zSomezIRStorageValuezK;
    struct zStorageValue z2zE3079;
    {
      sail_u256 z2zE3077;
      z2zE3077 = z3zE3084.zcurr;
      sail_u256 z2zE3078;
      z2zE3078 = z3zE3084.zcurr;
      struct zStorageValue z3zE3079;
      z3zE3079.zcurr = z2zE3077;
      z3zE3079.zorig = z2zE3078;
      z2zE3079 = z3zE3079;
    }
    z8zE241 = z2zE3079;
    goto cleanup_3121;
    /* unreachable after return */
    goto finish_match_3117;
    goto end_cleanup_3122;
  cleanup_3121: ;
    KILL(zoptionzIRStorageValuezK)(&z2zE3076);
    goto end_function_3120;
  end_cleanup_3122: ;
  }
case_3119: ;
  {
    /* complete */
    z3zE3077 = UNIT;
    goto finish_match_3117;
  }
case_3118: ;
finish_match_3117: ;
  unit z3zE3081;
  z3zE3081 = z3zE3077;
  KILL(zoptionzIRStorageValuezK)(&z2zE3076);
  sail_u256 zv;
  {
    bool z2zE3080;
    z2zE3080 = zacc.zstorage_cleared;
    if (z2zE3080) {  zv = zZERO_WORD;  } else {
      sail_fixed_bytes_32 z2zE3082;
      {
        struct zAccountInfo z2zE3081;
        z2zE3081 = zacc.zinfo;
        z2zE3082 = z2zE3081.zstorage_root;
      }
      {
        zv = zstateless_storage(z2zE3082, zs);
        if (have_exception) {  goto end_block_exception_3125;  }
      }
    }
  }
  unit z3zE3082;
  z3zE3082 = storage_block_cache(zkey, zv);
  struct zStorageValue z3zE3083;
  z3zE3083.zcurr = zv;
  z3zE3083.zorig = zv;
  z8zE241 = z3zE3083;
end_function_3120: ;
  return z8zE241;
end_block_exception_3125: ;
  struct zStorageValue z8zE872 = { .zcurr = u256_zero(), .zorig = u256_zero() };
  return z8zE872;
}

unit zk_sstore(sail_fixed_bytes_20 za, sail_u256 zs, struct zStorageValue zv)
{
  unit z8zE242;
  struct zStorageEntry z2zE3074;
  {
    struct zStorageKey z2zE3073;
    z2zE3073 = zstorage_key(za, zs);
    struct zStorageEntry z3zE3071;
    z3zE3071.zkey = z2zE3073;
    z3zE3071.zvalue = zv;
    z2zE3074 = z3zE3071;
  }
  z8zE242 = storage_tx_update(z2zE3074);
end_function_3112: ;
  return z8zE242;
end_block_exception_3113: ;

  return UNIT;
}

sail_u256 zk_tload(sail_fixed_bytes_20 za, sail_u256 zs)
{
  sail_u256 z8zE243;
  z8zE243 = transient_storage_read(za, zs);
end_function_3110: ;
  return z8zE243;
end_block_exception_3111: ;

  return u256_zero();
}

unit zk_tstore(sail_fixed_bytes_20 za, sail_u256 zs, sail_u256 zv)
{
  unit z8zE244;
  z8zE244 = transient_storage_write(za, zs, zv);
end_function_3108: ;
  return z8zE244;
end_block_exception_3109: ;

  return UNIT;
}

