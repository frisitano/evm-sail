/* Generated from sail/host/kernel/storage.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
struct zStorageKey zstorage_key(sail_fixed_bytes_20 za, sail_u256 zs)
{
  struct zStorageKey z8zE220;
  struct zStorageKey z3zE3190;
  z3zE3190.zaddr = za;
  z3zE3190.zslot = zs;
  z8zE220 = z3zE3190;
end_function_3237: ;
  return z8zE220;
end_block_exception_3238: ;
  struct zStorageKey z8zE867 = { .zaddr = fixed_bytes_20_zero(), .zslot = u256_zero() };
  return z8zE867;
}

bool zk_access_account(sail_fixed_bytes_20 za)
{
  bool z8zE221;
  z8zE221 = warm_addr_touch(za);
end_function_3235: ;
  return z8zE221;
end_block_exception_3236: ;

  return false;
}

bool zk_slot_is_warm(sail_fixed_bytes_20 za, sail_u256 zs)
{
  bool z8zE222;
  z8zE222 = warm_slot_touch(za, zs);
end_function_3233: ;
  return z8zE222;
end_block_exception_3234: ;

  return false;
}

struct zAccountInfo zdecode_state_account(struct zByteSliceFields zvalue)
{
  struct zAccountInfo z8zE223;
  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3225;
  {
    struct zRlpCursorFields z2zE3224;
    {
      z2zE3224 = zrlp_node_cursor(zvalue);
      if (have_exception) {  goto end_block_exception_3232;  }
    }
    {
      z2zE3225 = zrlp_cursor_pop(z2zE3224);
      if (have_exception) {  goto end_block_exception_3232;  }
    }
  }
  struct zAccountInfo z3zE3180;
  {
    struct zRlpFieldRefFields znonce;
    znonce = z2zE3225.ztup0;
    struct zRlpCursorFields zfields;
    zfields = z2zE3225.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3226;
    {
      z2zE3226 = zrlp_cursor_pop(zfields);
      if (have_exception) {  goto end_block_exception_3232;  }
    }
    struct zAccountInfo z3zE3181;
    {
      struct zRlpFieldRefFields zbalance;
      zbalance = z2zE3226.ztup0;
      struct zRlpCursorFields z1zE6;
      z1zE6 = z2zE3226.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3227;
      {
        z2zE3227 = zrlp_cursor_pop(z1zE6);
        if (have_exception) {  goto end_block_exception_3232;  }
      }
      struct zAccountInfo z3zE3182;
      {
        struct zRlpFieldRefFields zstorage;
        zstorage = z2zE3227.ztup0;
        struct zRlpCursorFields z1zE7;
        z1zE7 = z2zE3227.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3228;
        {
          z2zE3228 = zrlp_cursor_pop(z1zE7);
          if (have_exception) {  goto end_block_exception_3232;  }
        }
        struct zAccountInfo z3zE3183;
        {
          struct zRlpFieldRefFields zcode;
          zcode = z2zE3228.ztup0;
          struct zRlpCursorFields z1zE8;
          z1zE8 = z2zE3228.ztup1;
          unit z3zE3184;
          {
            z3zE3184 = zrlp_cursor_expect_end(z1zE8);
            if (have_exception) {  goto end_block_exception_3232;  }
          }
          sail_fixed_bytes_32 zstorage_root;
          {
            bool z2zE3235;
            {
              uint64_t z2zE3234;
              z2zE3234 = zstorage.zcontent_len;
              z2zE3235 = (z2zE3234 == UINT64_C(0));
            }
            if (z2zE3235) {  zstorage_root = zEMPTY_TRIE_ROOT;  } else {
              sail_u256 z2zE3236;
              {
                z2zE3236 = zrlp_ref_word(zstorage);
                if (have_exception) {  goto end_block_exception_3232;  }
              }
              zstorage_root = zword_to_hash(z2zE3236);
            }
          }
          sail_fixed_bytes_32 zcode_hash;
          {
            bool z2zE3232;
            {
              uint64_t z2zE3231;
              z2zE3231 = zcode.zcontent_len;
              z2zE3232 = (z2zE3231 == UINT64_C(0));
            }
            if (z2zE3232) {  zcode_hash = zKECCAK_EMPTY;  } else {
              sail_u256 z2zE3233;
              {
                z2zE3233 = zrlp_ref_word(zcode);
                if (have_exception) {  goto end_block_exception_3232;  }
              }
              zcode_hash = zword_to_hash(z2zE3233);
            }
          }
          uint64_t z2zE3229;
          {
            z2zE3229 = zrlp_ref_account_nonce(znonce);
            if (have_exception) {  goto end_block_exception_3232;  }
          }
          sail_u256 z2zE3230;
          {
            z2zE3230 = zrlp_ref_uint_word(zbalance);
            if (have_exception) {  goto end_block_exception_3232;  }
          }
          struct zAccountInfo z3zE3185;
          z3zE3185.zbalance = z2zE3230;
          z3zE3185.zcode_hash = zcode_hash;
          z3zE3185.znonce = z2zE3229;
          z3zE3185.zstorage_root = zstorage_root;
          z3zE3183 = z3zE3185;
          goto finish_match_3229;
        }
      case_3230: ;
        sail_match_failure("decode_state_account");
      finish_match_3229: ;
        z3zE3182 = z3zE3183;
        goto finish_match_3227;
      }
    case_3228: ;
      sail_match_failure("decode_state_account");
    finish_match_3227: ;
      z3zE3181 = z3zE3182;
      goto finish_match_3225;
    }
  case_3226: ;
    sail_match_failure("decode_state_account");
  finish_match_3225: ;
    z3zE3180 = z3zE3181;
    goto finish_match_3223;
  }
case_3224: ;
  sail_match_failure("decode_state_account");
finish_match_3223: ;
  z8zE223 = z3zE3180;
end_function_3231: ;
  return z8zE223;
end_block_exception_3232: ;
  struct zAccountInfo z8zE868 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  return z8zE868;
}

struct zTriePath zpath_new(sail_fixed_bytes_32 zdata, uint64_t zlen)
{
  struct zTriePath z8zE224;
  struct zTriePath z3zE3179;
  z3zE3179.zdata = zdata;
  z3zE3179.zlen = zlen;
  z8zE224 = z3zE3179;
end_function_3221: ;
  return z8zE224;
end_block_exception_3222: ;
  struct zTriePath z8zE869 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE869;
}

struct zTriePath zpath_from_hash(sail_fixed_bytes_32 zh)
{
  struct zTriePath z8zE225;
  z8zE225 = zpath_new(zh, UINT64_C(64));
end_function_3219: ;
  return z8zE225;
end_block_exception_3220: ;
  struct zTriePath z8zE870 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE870;
}

struct zByteSliceFields znode_db_lookup(sail_fixed_bytes_32 zh)
{
  struct zByteSliceFields z8zE226;
  z8zE226 = nodedb_lookup(zh);
end_function_3217: ;
  return z8zE226;
end_block_exception_3218: ;
  struct zByteSliceFields z8zE871 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE871;
}

struct zRlpFieldRefFields zbranch_children_get(zz5vecz8z5structz0zzRlpFieldRefFieldsz9 zchildren, uint64_t zindex)
{
  struct zRlpFieldRefFields z8zE227;
  struct zRlpFieldRefFields z3zE3148;
  {
    uint64_t zp0z3;
    zp0z3 = zindex;
    bool z3zE3164;
    z3zE3164 = (zp0z3 == UINT64_C(0x0));
    if (!(z3zE3164)) {  goto case_3214;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(0));
    goto finish_match_3198;
  }
case_3214: ;
  {
    uint64_t z3zE3165;
    z3zE3165 = zindex;
    bool z3zE3163;
    z3zE3163 = (z3zE3165 == UINT64_C(0x1));
    if (!(z3zE3163)) {  goto case_3213;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(1));
    goto finish_match_3198;
  }
case_3213: ;
  {
    uint64_t z3zE3166;
    z3zE3166 = zindex;
    bool z3zE3162;
    z3zE3162 = (z3zE3166 == UINT64_C(0x2));
    if (!(z3zE3162)) {  goto case_3212;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(2));
    goto finish_match_3198;
  }
case_3212: ;
  {
    uint64_t z3zE3167;
    z3zE3167 = zindex;
    bool z3zE3161;
    z3zE3161 = (z3zE3167 == UINT64_C(0x3));
    if (!(z3zE3161)) {  goto case_3211;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(3));
    goto finish_match_3198;
  }
case_3211: ;
  {
    uint64_t z3zE3168;
    z3zE3168 = zindex;
    bool z3zE3160;
    z3zE3160 = (z3zE3168 == UINT64_C(0x4));
    if (!(z3zE3160)) {  goto case_3210;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(4));
    goto finish_match_3198;
  }
case_3210: ;
  {
    uint64_t z3zE3169;
    z3zE3169 = zindex;
    bool z3zE3159;
    z3zE3159 = (z3zE3169 == UINT64_C(0x5));
    if (!(z3zE3159)) {  goto case_3209;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(5));
    goto finish_match_3198;
  }
case_3209: ;
  {
    uint64_t z3zE3170;
    z3zE3170 = zindex;
    bool z3zE3158;
    z3zE3158 = (z3zE3170 == UINT64_C(0x6));
    if (!(z3zE3158)) {  goto case_3208;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(6));
    goto finish_match_3198;
  }
case_3208: ;
  {
    uint64_t z3zE3171;
    z3zE3171 = zindex;
    bool z3zE3157;
    z3zE3157 = (z3zE3171 == UINT64_C(0x7));
    if (!(z3zE3157)) {  goto case_3207;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(7));
    goto finish_match_3198;
  }
case_3207: ;
  {
    uint64_t z3zE3172;
    z3zE3172 = zindex;
    bool z3zE3156;
    z3zE3156 = (z3zE3172 == UINT64_C(0x8));
    if (!(z3zE3156)) {  goto case_3206;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(8));
    goto finish_match_3198;
  }
case_3206: ;
  {
    uint64_t z3zE3173;
    z3zE3173 = zindex;
    bool z3zE3155;
    z3zE3155 = (z3zE3173 == UINT64_C(0x9));
    if (!(z3zE3155)) {  goto case_3205;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(9));
    goto finish_match_3198;
  }
case_3205: ;
  {
    uint64_t z3zE3174;
    z3zE3174 = zindex;
    bool z3zE3154;
    z3zE3154 = (z3zE3174 == UINT64_C(0xA));
    if (!(z3zE3154)) {  goto case_3204;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(10));
    goto finish_match_3198;
  }
case_3204: ;
  {
    uint64_t z3zE3175;
    z3zE3175 = zindex;
    bool z3zE3153;
    z3zE3153 = (z3zE3175 == UINT64_C(0xB));
    if (!(z3zE3153)) {  goto case_3203;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(11));
    goto finish_match_3198;
  }
case_3203: ;
  {
    uint64_t z3zE3176;
    z3zE3176 = zindex;
    bool z3zE3152;
    z3zE3152 = (z3zE3176 == UINT64_C(0xC));
    if (!(z3zE3152)) {  goto case_3202;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(12));
    goto finish_match_3198;
  }
case_3202: ;
  {
    uint64_t z3zE3177;
    z3zE3177 = zindex;
    bool z3zE3151;
    z3zE3151 = (z3zE3177 == UINT64_C(0xD));
    if (!(z3zE3151)) {  goto case_3201;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(13));
    goto finish_match_3198;
  }
case_3201: ;
  {
    uint64_t z3zE3178;
    z3zE3178 = zindex;
    bool z3zE3150;
    z3zE3150 = (z3zE3178 == UINT64_C(0xE));
    if (!(z3zE3150)) {  goto case_3200;  }
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(14));
    goto finish_match_3198;
  }
case_3200: ;
  {
    z3zE3148 = fast_unsigned_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(zchildren, UINT64_C(15));
    goto finish_match_3198;
  }
case_3199: ;
finish_match_3198: ;
  z8zE227 = z3zE3148;
end_function_3215: ;
  return z8zE227;
end_block_exception_3216: ;
  struct zByteSliceFields z8zE873 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRefFields z8zE872 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcontent_off = UINT64_C(0xdeadc0de), .zfull_len = UINT64_C(0xdeadc0de), .zfull_off = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE873 };
  return z8zE872;
}

void create_letbind_96(void) {

  uint64_t z3zE129;
  z3zE129 = UINT64_C(33);
  zHEX_PREFIX_MAX_LENGTH = z3zE129;
let_end_338: ;
}
void kill_letbind_96(void) {
}

struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 zhex_prefix_decode_ref(struct zRlpFieldRefFields zf)
{
  struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z8zE228;
  bool z2zE3205;
  z2zE3205 = zf.zis_list;
  unit z3zE3141;
  if (z2zE3205) {
    struct zexception z2zE3206;
    CREATE(zexception)(&z2zE3206);
    zInvalidBlock(&z2zE3206, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3206);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:257.8-257.37");
    KILL(zexception)(&z2zE3206);
    goto end_block_exception_3197;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3206);
  } else {  z3zE3141 = UNIT;  }
  uint64_t zn;
  zn = zf.zcontent_len;
  bool z2zE3207;
  z2zE3207 = (zn == UINT64_C(0));
  if (z2zE3207) {
    struct zTriePath z3zE3145;
    zz5vecz8z5bv8z9 z3zE3146;
    CREATE(zz5vecz8z5bv8z9)(&z3zE3146);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE3146, INT64_C(32));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3146, z3zE3146, INT64_C(31), UINT64_C(0x00));
    for (size_t z8zE874 = 0; z8zE874 < 32; ++z8zE874) {
      z3zE3145.zdata.bytes[z8zE874] = (uint8_t)(z3zE3146.data[z8zE874] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE3146);
    z3zE3145.zlen = UINT64_C(0);
    struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE3147;
    z3zE3147.ztup0 = false;
    z3zE3147.ztup1 = z3zE3145;
    z8zE228 = z3zE3147;
  } else {
    uint64_t zmaximum_length;
    zmaximum_length = zHEX_PREFIX_MAX_LENGTH;
    bool z2zE3208;
    z2zE3208 = (zmaximum_length < zn);
    if (z2zE3208) {
      struct zexception z2zE3209;
      CREATE(zexception)(&z2zE3209);
      zInvalidBlock(&z2zE3209, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3209);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:265.12-265.41");
      KILL(zexception)(&z2zE3209);
      goto end_block_exception_3197;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3209);
    } else {
      struct zByteSliceFields zcontent;
      zcontent = zrlp_ref_content(zf);
      uint64_t zfb;
      zfb = zslice_byte(zcontent, UINT64_C(0));
      uint64_t zflag;
      zflag = (safe_rshift(UINT64_MAX, 64 - 4) & (zfb >> UINT64_C(4)));
      bool zis_leaf;
      {
        uint64_t z2zE3223;
        z2zE3223 = (UINT64_C(1) & (zflag >> UINT64_C(1)));
        zis_leaf = (z2zE3223 == UINT64_C(0b1));
      }
      bool zodd;
      {
        uint64_t z2zE3222;
        z2zE3222 = (UINT64_C(1) & (zflag >> UINT64_C(0)));
        zodd = (z2zE3222 == UINT64_C(0b1));
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
        bool z2zE3210;
        z2zE3210 = (zpaired_nibbles < UINT64_C(64));
        if (z2zE3210) {
          sail_fixed_bytes_32 zbytes;
          {
            sail_u256 z2zE3218;
            z2zE3218 = zword_shift_right(zpacked, UINT64_C(4));
            zbytes = zword_to_hash(z2zE3218);
          }
          uint64_t z2zE3214;
          {
            uint64_t z2zE3212;
            z2zE3212 = (safe_rshift(UINT64_MAX, 64 - 4) & (zfb >> UINT64_C(0)));
            uint64_t z2zE3213;
            {
              uint64_t z2zE3211;
              z2zE3211 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(31));
              z2zE3213 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE3211 >> UINT64_C(0)));
            }
            z2zE3214 = (z2zE3212 << 4) | z2zE3213;
          }
          zbytes = fast_unsigned_vector_update_fixed_bytes_32(zbytes, UINT64_C(31), z2zE3214);
          unit z3zE3143;
          z3zE3143 = UNIT;
          struct zTriePath z2zE3217;
          {
            sail_fixed_bytes_32 z2zE3215;
            z2zE3215 = zB256(zbytes);
            uint64_t z2zE3216;
            {    z2zE3216 = (zpaired_nibbles + UINT64_C(1));
            }
            z2zE3217 = zpath_new(z2zE3215, z2zE3216);
          }
          struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE3144;
          z3zE3144.ztup0 = zis_leaf;
          z3zE3144.ztup1 = z2zE3217;
          z8zE228 = z3zE3144;
        } else {
          struct zexception z2zE3219;
          CREATE(zexception)(&z2zE3219);
          zInvalidBlock(&z2zE3219, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE3219);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:282.20-282.56");
          KILL(zexception)(&z2zE3219);
          goto end_block_exception_3197;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3219);
        }
      } else {
        struct zTriePath z2zE3221;
        {
          sail_fixed_bytes_32 z2zE3220;
          z2zE3220 = zword_to_hash(zpacked);
          z2zE3221 = zpath_new(z2zE3220, zpaired_nibbles);
        }
        struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z3zE3142;
        z3zE3142.ztup0 = zis_leaf;
        z3zE3142.ztup1 = z2zE3221;
        z8zE228 = z3zE3142;
      }
    }
  }
end_function_3196: ;
  return z8zE228;
end_block_exception_3197: ;
  struct zTriePath z8zE876 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z8zE875 = { .ztup0 = false, .ztup1 = z8zE876 };
  return z8zE875;
}

uint64_t zpath_len(struct zTriePath zpath)
{
  uint64_t z8zE229;
  z8zE229 = zpath.zlen;
end_function_3194: ;
  return z8zE229;
end_block_exception_3195: ;

  return UINT64_C(0xdeadc0de);
}

void zdecode_trie_node(struct zTrieNode *z8zE230, struct zByteSliceFields znode)
{
  struct zRlpCursorFields zfields;
  {
    zfields = zrlp_node_cursor(znode);
    if (have_exception) {  goto end_block_exception_3193;  }
  }
  bool z2zE3191;
  {
    bool z2zE3190;
    {
      bool z2zE3189;
      z2zE3189 = zfields.zvalid;
      z2zE3190 = not(z2zE3189);
    }
    bool z3zE3117;
    if (z2zE3190) {  z3zE3117 = true;  } else {  z3zE3117 = zrlp_cursor_empty(zfields);  }
    z2zE3191 = z3zE3117;
  }
  if (z2zE3191) {  zInvalidNode((*(&z8zE230)), UNIT);  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3192;
    {
      z2zE3192 = zrlp_cursor_pop(zfields);
      if (have_exception) {  goto end_block_exception_3193;  }
    }
    struct zTrieNode z3zE3118;
    CREATE(zTrieNode)(&z3zE3118);
    {
      struct zRlpFieldRefFields zfirst;
      zfirst = z2zE3192.ztup0;
      struct zRlpCursorFields z1zE3;
      z1zE3 = z2zE3192.ztup1;
      bool z2zE3193;
      z2zE3193 = zrlp_cursor_empty(z1zE3);
      if (z2zE3193) {  zInvalidNode(&z3zE3118, UNIT);  } else {
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3194;
        {
          z2zE3194 = zrlp_cursor_pop(z1zE3);
          if (have_exception) {
            KILL(zTrieNode)(&z3zE3118);
            goto end_block_exception_3193;
          }
        }
        struct zTrieNode z3zE3120;
        CREATE(zTrieNode)(&z3zE3120);
        {
          struct zRlpFieldRefFields zsecond;
          zsecond = z2zE3194.ztup0;
          struct zRlpCursorFields z1zE4;
          z1zE4 = z2zE3194.ztup1;
          bool z2zE3195;
          z2zE3195 = zrlp_cursor_empty(z1zE4);
          if (z2zE3195) {
            struct zRlpFieldRefFields zpath_field;
            zpath_field = zfirst;
            struct zRlpFieldRefFields zvalue;
            zvalue = zsecond;
            struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 z2zE3196;
            {
              z2zE3196 = zhex_prefix_decode_ref(zpath_field);
              if (have_exception) {
                KILL(zTrieNode)(&z3zE3118);
                KILL(zTrieNode)(&z3zE3120);
                goto end_block_exception_3193;
              }
            }
            struct zTrieNode z3zE3136;
            CREATE(zTrieNode)(&z3zE3136);
            {
              bool zis_leaf;
              zis_leaf = z2zE3196.ztup0;
              struct zTriePath zpath;
              zpath = z2zE3196.ztup1;
              if (zis_leaf) {
                struct zLeafNodeData z3zE3139;
                z3zE3139.zpath = zpath;
                z3zE3139.zvalue = zvalue;
                zLeafNode(&z3zE3136, z3zE3139);
              } else {
                bool z2zE3198;
                {
                  uint64_t z2zE3197;
                  z2zE3197 = zpath_len(zpath);
                  z2zE3198 = (z2zE3197 == UINT64_C(0));
                }
                if (z2zE3198) {  zInvalidNode(&z3zE3136, UNIT);  } else {
                  struct zExtensionNodeData z3zE3138;
                  z3zE3138.zchild = zvalue;
                  z3zE3138.zpath = zpath;
                  zExtensionNode(&z3zE3136, z3zE3138);
                }
              }
              goto finish_match_3190;
            }
          case_3191: ;
            sail_match_failure("decode_trie_node");
          finish_match_3190: ;
            COPY(zTrieNode)(&z3zE3120, z3zE3136);
            KILL(zTrieNode)(&z3zE3136);
          } else {
            zz5vecz8z5structz0zzRlpFieldRefFieldsz9 zchildren;
            CREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&zchildren);
            fast_unsigned_vector_init_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(&zchildren, UINT64_C(16), zfirst);
            fast_unsigned_vector_update_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(&zchildren, zchildren, UINT64_C(0), zfirst);
            unit z3zE3123;
            z3zE3123 = UNIT;
            fast_unsigned_vector_update_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(&zchildren, zchildren, UINT64_C(1), zsecond);
            unit z3zE3122;
            z3zE3122 = UNIT;
            struct zRlpCursorFields zcursor;
            zcursor = z1zE4;
            bool zcomplete;
            zcomplete = true;
            int64_t z3zE3124;
            {    z3zE3124 = (int64_t)(UINT64_C(2));
            }
            int64_t z3zE3125;
            {    z3zE3125 = (int64_t)(UINT64_C(15));
            }
            int64_t z3zE3126;
            {    z3zE3126 = (int64_t)(UINT64_C(1));
            }
            {
              int64_t zindex;
              zindex = z3zE3124;
              unit z3zE3127;
            for_start_3184: ;
              {
                if ((z3zE3125 < zindex)) goto for_end_3185;
                bool z2zE3199;
                z2zE3199 = zrlp_cursor_empty(zcursor);
                if (z2zE3199) {
                  zcomplete = false;
                  z3zE3127 = UNIT;
                } else {
                  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3200;
                  {
                    z2zE3200 = zrlp_cursor_pop(zcursor);
                    if (have_exception) {
                      KILL(zTrieNode)(&z3zE3118);
                      KILL(zTrieNode)(&z3zE3120);
                      KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&zchildren);
                      goto end_block_exception_3193;
                    }
                  }
                  unit z3zE3128;
                  {
                    struct zRlpFieldRefFields zchild;
                    zchild = z2zE3200.ztup0;
                    struct zRlpCursorFields znext;
                    znext = z2zE3200.ztup1;
                    fast_vector_update_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(&zchildren, zchildren, zindex, zchild);
                    unit z3zE3129;
                    z3zE3129 = UNIT;
                    zcursor = znext;
                    z3zE3128 = UNIT;
                    goto finish_match_3186;
                  }
                case_3187: ;
                  sail_match_failure("decode_trie_node");
                finish_match_3186: ;
                  z3zE3127 = z3zE3128;
                }
                zindex = (zindex + z3zE3126);
                goto for_start_3184;
              }
            for_end_3185: ;
            }
            unit z3zE3131;
            z3zE3131 = UNIT;
            bool z2zE3202;
            {
              bool z2zE3201;
              z2zE3201 = not(zcomplete);
              bool z3zE3132;
              if (z2zE3201) {  z3zE3132 = true;  } else {  z3zE3132 = zrlp_cursor_empty(zcursor);  }
              z2zE3202 = z3zE3132;
            }
            if (z2zE3202) {  zInvalidNode(&z3zE3120, UNIT);  } else {
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3203;
              {
                z2zE3203 = zrlp_cursor_pop(zcursor);
                if (have_exception) {
                  KILL(zTrieNode)(&z3zE3118);
                  KILL(zTrieNode)(&z3zE3120);
                  KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&zchildren);
                  goto end_block_exception_3193;
                }
              }
              struct zTrieNode z3zE3133;
              CREATE(zTrieNode)(&z3zE3133);
              {
                struct zRlpFieldRefFields z3zE3140;
                z3zE3140 = z2zE3203.ztup0;
                struct zRlpCursorFields z1zE5;
                z1zE5 = z2zE3203.ztup1;
                bool z2zE3204;
                z2zE3204 = zrlp_cursor_empty(z1zE5);
                if (z2zE3204) {
                  struct zBranchNodeData z3zE3135;
                  CREATE(zBranchNodeData)(&z3zE3135);
                  COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&((&z3zE3135)->zchildren), zchildren);
                  z3zE3135.zvalue = z3zE3140;
                  zBranchNode(&z3zE3133, z3zE3135);
                  KILL(zBranchNodeData)(&z3zE3135);
                } else {  zInvalidNode(&z3zE3133, UNIT);  }
                goto finish_match_3188;
              }
            case_3189: ;
              sail_match_failure("decode_trie_node");
            finish_match_3188: ;
              COPY(zTrieNode)(&z3zE3120, z3zE3133);
              KILL(zTrieNode)(&z3zE3133);
            }
            KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&zchildren);
          }
          goto finish_match_3182;
        }
      case_3183: ;
        sail_match_failure("decode_trie_node");
      finish_match_3182: ;
        COPY(zTrieNode)(&z3zE3118, z3zE3120);
        KILL(zTrieNode)(&z3zE3120);
      }
      goto finish_match_3180;
    }
  case_3181: ;
    sail_match_failure("decode_trie_node");
  finish_match_3180: ;
    COPY(zTrieNode)((*(&z8zE230)), z3zE3118);
    KILL(zTrieNode)(&z3zE3118);
  }
end_function_3192: ;
  goto end_function_4077;
end_block_exception_3193: ;
  goto end_function_4077;
end_function_4077: ;
}

void create_letbind_97(void) {

  uint64_t z3zE130;
  z3zE130 = zWORD_BYTE_LENGTH;
  zMPT_HASH_LENGTH = z3zE130;
let_end_342: ;
}
void kill_letbind_97(void) {
}

struct zInlineNode zinline_node_from_slice(struct zByteSliceFields zbytes)
{
  struct zInlineNode z8zE231;
  uint64_t zlength;
  zlength = zbytes.zlen;
  bool z2zE3185;
  z2zE3185 = (!(zlength < zMPT_HASH_LENGTH));
  unit z3zE3115;
  if (z2zE3185) {
    struct zexception z2zE3186;
    CREATE(zexception)(&z2zE3186);
    zInvalidBlock(&z2zE3186, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE3186);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:23.8-23.44");
    KILL(zexception)(&z2zE3186);
    goto end_block_exception_3179;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3186);
  } else {  z3zE3115 = UNIT;  }
  sail_fixed_bytes_32 z2zE3188;
  {
    sail_u256 z2zE3187;
    z2zE3187 = zslice_load(zbytes, UINT64_C(0));
    z2zE3188 = zword_to_hash(z2zE3187);
  }
  struct zInlineNode z3zE3116;
  z3zE3116.zdata = z2zE3188;
  z3zE3116.zlen = zlength;
  z8zE231 = z3zE3116;
end_function_3178: ;
  return z8zE231;
end_block_exception_3179: ;
  struct zInlineNode z8zE877 = { .zdata = fixed_bytes_32_zero(), .zlen = UINT64_C(0xdeadc0de) };
  return z8zE877;
}

void zfield_to_ref(struct zNodeRef *z8zE232, struct zRlpFieldRefFields zf)
{
  bool z2zE3178;
  z2zE3178 = zf.zis_list;
  if (z2zE3178) {
    struct zInlineNode z2zE3180;
    {
      struct zByteSliceFields z2zE3179;
      z2zE3179 = zrlp_ref_full(zf);
      {
        z2zE3180 = zinline_node_from_slice(z2zE3179);
        if (have_exception) {  goto end_block_exception_3177;  }
      }
    }
    zInlineRef((*(&z8zE232)), z2zE3180);
  } else {
    bool z2zE3182;
    {
      uint64_t z2zE3181;
      z2zE3181 = zf.zcontent_len;
      z2zE3182 = (z2zE3181 == zMPT_HASH_LENGTH);
    }
    if (z2zE3182) {
      sail_fixed_bytes_32 z2zE3184;
      {
        sail_u256 z2zE3183;
        {
          z2zE3183 = zrlp_ref_word(zf);
          if (have_exception) {  goto end_block_exception_3177;  }
        }
        z2zE3184 = zword_to_hash(z2zE3183);
      }
      zHashRef((*(&z8zE232)), z2zE3184);
    } else {  zEmptyRef((*(&z8zE232)), UNIT);  }
  }
end_function_3176: ;
  goto end_function_4076;
end_block_exception_3177: ;
  goto end_function_4076;
end_function_4076: ;
}

uint64_t zpath_byte_index(uint64_t zi)
{
  uint64_t z8zE233;
  uint64_t zquotient;
  {    zquotient = (zi / UINT64_C(2));
  }
  uint64_t znatural_index;
  {
    bool z2zE3177;
    {
      bool z2zE3176;
      z2zE3176 = (!(zquotient < UINT64_C(0)));
      bool z3zE3113;
      if (z2zE3176) {  z3zE3113 = (!(UINT64_C(31) < zquotient));  } else {  z3zE3113 = false;  }
      z2zE3177 = z3zE3113;
    }
    if (z2zE3177) {  znatural_index = zquotient;  } else {
      unit z3zE3114;
      z3zE3114 = sail_assert(false, "sail/lib/mpt/primitives.sail:64.24-64.25");
      sail_match_failure("path_byte_index");
      /* unreachable after exit */
    }
  }
  {    z8zE233 = (UINT64_C(31) - znatural_index);
  }
end_function_3174: ;
  return z8zE233;
end_block_exception_3175: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zpath_nibble(struct zTriePath zpath, uint64_t zi)
{
  uint64_t z8zE234;
  bool z2zE3171;
  {
    uint64_t z2zE3170;
    z2zE3170 = zpath_len(zpath);
    z2zE3171 = (!(zi < z2zE3170));
  }
  if (z2zE3171) {  z8zE234 = UINT64_C(0x0);  } else {
    sail_fixed_bytes_32 zbytes;
    zbytes = zpath.zdata;
    uint64_t zbyte_index;
    zbyte_index = zpath_byte_index(zi);
    bool z2zE3173;
    {
      uint64_t z2zE3172;
      {    z2zE3172 = (zi % UINT64_C(2));
      }
      z2zE3173 = (z2zE3172 == UINT64_C(0));
    }
    if (z2zE3173) {
      uint64_t z2zE3174;
      z2zE3174 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
      z8zE234 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE3174 >> UINT64_C(4)));
    } else {
      uint64_t z2zE3175;
      z2zE3175 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, zbyte_index);
      z8zE234 = (safe_rshift(UINT64_MAX, 64 - 4) & (z2zE3175 >> UINT64_C(0)));
    }
  }
end_function_3172: ;
  return z8zE234;
end_block_exception_3173: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztrie_path_len_increment(uint64_t zvalue)
{
  uint64_t z8zE235;
  bool z2zE3168;
  z2zE3168 = (zvalue < UINT64_C(64));
  if (z2zE3168) {
    {    z8zE235 = (zvalue + UINT64_C(1));
    }
  } else {
    struct zexception z2zE3169;
    CREATE(zexception)(&z2zE3169);
    zInvalidBlock(&z2zE3169, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE3169);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/primitives.sail:35.8-35.44");
    KILL(zexception)(&z2zE3169);
    goto end_block_exception_3171;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3169);
  }
end_function_3170: ;
  return z8zE235;
end_block_exception_3171: ;

  return UINT64_C(0xdeadc0de);
}

bool zpath_matches(struct zTriePath zkey, uint64_t zpos, struct zTriePath zseg)
{
  bool z8zE236;
  uint64_t zstop;
  {
    uint64_t z2zE3167;
    z2zE3167 = zpath_len(zseg);
    {    zstop = (zpos + z2zE3167);
    }
  }
  bool z2zE3160;
  {
    uint64_t z2zE3159;
    z2zE3159 = zpath_len(zkey);
    z2zE3160 = (z2zE3159 < zstop);
  }
  if (z2zE3160) {  z8zE236 = false;  } else {
    bool zok;
    zok = true;
    uint64_t zoffset;
    zoffset = UINT64_C(0);
    int64_t z3zE3107;
    {    z3zE3107 = (int64_t)(UINT64_C(0));
    }
    int64_t z3zE3108;
    {    z3zE3108 = (int64_t)(UINT64_C(63));
    }
    int64_t z3zE3109;
    {    z3zE3109 = (int64_t)(UINT64_C(1));
    }
    {
      int64_t z_step;
      z_step = z3zE3107;
      unit z3zE3110;
    for_start_3166: ;
      {
        if ((z3zE3108 < z_step)) goto for_end_3167;
        bool z2zE3162;
        {
          uint64_t z2zE3161;
          z2zE3161 = zpath_len(zseg);
          z2zE3162 = (zoffset < z2zE3161);
        }
        if (z2zE3162) {
          uint64_t zkey_index;
          {    zkey_index = (zpos + zoffset);
          }
          bool z2zE3163;
          z2zE3163 = (!(UINT64_C(64) < zkey_index));
          unit z3zE3111;
          if (z2zE3163) {
            bool z2zE3166;
            {
              uint64_t z2zE3164;
              z2zE3164 = zpath_nibble(zkey, zkey_index);
              uint64_t z2zE3165;
              z2zE3165 = zpath_nibble(zseg, zoffset);
              z2zE3166 = (z2zE3164 != z2zE3165);
            }
            if (z2zE3166) {
              zok = false;
              z3zE3111 = UNIT;
            } else {  z3zE3111 = UNIT;  }
          } else {
            zok = false;
            z3zE3111 = UNIT;
          }
          {
            zoffset = ztrie_path_len_increment(zoffset);
            if (have_exception) {  goto end_block_exception_3169;  }
          }
          z3zE3110 = UNIT;
        } else {  z3zE3110 = UNIT;  }
        z_step = (z_step + z3zE3109);
        goto for_start_3166;
      }
    for_end_3167: ;
    }
    unit z3zE3112;
    z3zE3112 = UNIT;
    z8zE236 = zok;
  }
end_function_3168: ;
  return z8zE236;
end_block_exception_3169: ;

  return false;
}

struct zByteSliceFields zinline_node_slice(struct zInlineNode znode)
{
  struct zByteSliceFields z8zE237;
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  sail_fixed_bytes_32 z2zE3157;
  z2zE3157 = znode.zdata;
  uint64_t z2zE3158;
  z2zE3158 = znode.zlen;
  unit z3zE3106;
  z3zE3106 = zscratch_push_b256(z2zE3157, z2zE3158);
  z8zE237 = zscratch_finish(zstart);
end_function_3164: ;
  return z8zE237;
end_block_exception_3165: ;
  struct zByteSliceFields z8zE878 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE878;
}

struct zByteSliceFields zresolve_ref(struct zNodeRef zr)
{
  struct zByteSliceFields z8zE238;
  struct zByteSliceFields z3zE3101;
  {
    if (zr.kind != Kind_zEmptyRef) goto case_3161;
    z3zE3101 = zEMPTY_SLICE;
    goto finish_match_3158;
  }
case_3161: ;
  {
    if (zr.kind != Kind_zInlineRef) goto case_3160;
    struct zInlineNode znode;
    znode = zr.variants.zInlineRef;
    z3zE3101 = zinline_node_slice(znode);
    goto finish_match_3158;
  }
case_3160: ;
  {
    /* complete */
    sail_fixed_bytes_32 zh;
    zh = zr.variants.zHashRef;
    struct zByteSliceFields z3zE3105;
    z3zE3105 = znode_db_lookup(zh);
    bool z2zE3155;
    {
      uint64_t z2zE3154;
      z2zE3154 = z3zE3105.zlen;
      z2zE3155 = (z2zE3154 == UINT64_C(0));
    }
    if (z2zE3155) {
      struct zexception z2zE3156;
      CREATE(zexception)(&z2zE3156);
      zInvalidBlock(&z2zE3156, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3156);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/nodes.sail:314.16-314.52");
      KILL(zexception)(&z2zE3156);
      goto end_block_exception_3163;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3156);
    } else {  z3zE3101 = z3zE3105;  }
    goto finish_match_3158;
  }
case_3159: ;
finish_match_3158: ;
  z8zE238 = z3zE3101;
end_function_3162: ;
  return z8zE238;
end_block_exception_3163: ;
  struct zByteSliceFields z8zE879 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE879;
}

struct zByteSliceFields ztrie_walk(struct zByteSliceFields znode, struct zTriePath zkey, uint64_t zpos)
{
  struct zByteSliceFields z8zE239;
  bool z2zE3124;
  {
    uint64_t z2zE3123;
    z2zE3123 = znode.zlen;
    z2zE3124 = (z2zE3123 == UINT64_C(0));
  }
  if (z2zE3124) {  z8zE239 = zEMPTY_SLICE;  } else {
    struct zTrieNode z2zE3125;
    CREATE(zTrieNode)(&z2zE3125);
    {
      zdecode_trie_node(&z2zE3125, znode);
      if (have_exception) {
        KILL(zTrieNode)(&z2zE3125);
        goto end_block_exception_3157;
      }
    }
    struct zByteSliceFields z3zE3096;
    {
      if (z2zE3125.kind != Kind_zLeafNode) goto case_3155;
      struct zLeafNodeData zleaf;
      zleaf = z2zE3125.variants.zLeafNode;
      bool z2zE3128;
      {
        bool z2zE3127;
        {
          struct zTriePath z2zE3126;
          z2zE3126 = zleaf.zpath;
          {
            z2zE3127 = zpath_matches(zkey, zpos, z2zE3126);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE3125);
              goto end_block_exception_3157;
            }
          }
        }
        z2zE3128 = not(z2zE3127);
      }
      if (z2zE3128) {  z3zE3096 = zEMPTY_SLICE;  } else {
        bool z2zE3133;
        {
          uint64_t z2zE3131;
          {
            uint64_t z2zE3130;
            {
              struct zTriePath z2zE3129;
              z2zE3129 = zleaf.zpath;
              z2zE3130 = zpath_len(z2zE3129);
            }
            {    z2zE3131 = (zpos + z2zE3130);
            }
          }
          uint64_t z2zE3132;
          z2zE3132 = zpath_len(zkey);
          z2zE3133 = (z2zE3131 == z2zE3132);
        }
        if (z2zE3133) {
          struct zRlpFieldRefFields z2zE3134;
          z2zE3134 = zleaf.zvalue;
          z3zE3096 = zrlp_ref_content(z2zE3134);
        } else {  z3zE3096 = zEMPTY_SLICE;  }
      }
      goto finish_match_3151;
    }
  case_3155: ;
    {
      if (z2zE3125.kind != Kind_zExtensionNode) goto case_3154;
      struct zExtensionNodeData zext;
      zext = z2zE3125.variants.zExtensionNode;
      uint64_t zextension_len;
      {
        struct zTriePath z2zE3143;
        z2zE3143 = zext.zpath;
        zextension_len = zpath_len(z2zE3143);
      }
      bool z2zE3135;
      z2zE3135 = (zextension_len == UINT64_C(0));
      if (z2zE3135) {  z3zE3096 = zEMPTY_SLICE;  } else {
        bool z2zE3138;
        {
          bool z2zE3137;
          {
            struct zTriePath z2zE3136;
            z2zE3136 = zext.zpath;
            {
              z2zE3137 = zpath_matches(zkey, zpos, z2zE3136);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE3125);
                goto end_block_exception_3157;
              }
            }
          }
          z2zE3138 = not(z2zE3137);
        }
        if (z2zE3138) {  z3zE3096 = zEMPTY_SLICE;  } else {
          uint64_t znext_pos;
          {    znext_pos = (zpos + zextension_len);
          }
          bool z2zE3139;
          z2zE3139 = (!(UINT64_C(64) < znext_pos));
          if (z2zE3139) {
            struct zByteSliceFields z2zE3142;
            {
              struct zNodeRef z2zE3141;
              CREATE(zNodeRef)(&z2zE3141);
              {
                struct zRlpFieldRefFields z2zE3140;
                z2zE3140 = zext.zchild;
                {
                  zfield_to_ref(&z2zE3141, z2zE3140);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE3125);
                    KILL(zNodeRef)(&z2zE3141);
                    goto end_block_exception_3157;
                  }
                }
              }
              {
                z2zE3142 = zresolve_ref(z2zE3141);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE3125);
                  KILL(zNodeRef)(&z2zE3141);
                  goto end_block_exception_3157;
                }
              }
              KILL(zNodeRef)(&z2zE3141);
            }
            {
              z3zE3096 = ztrie_walk(z2zE3142, zkey, znext_pos);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE3125);
                goto end_block_exception_3157;
              }
            }
          } else {  z3zE3096 = zEMPTY_SLICE;  }
        }
      }
      goto finish_match_3151;
    }
  case_3154: ;
    {
      if (z2zE3125.kind != Kind_zBranchNode) goto case_3153;
      struct zBranchNodeData zbranch;
      CREATE(zBranchNodeData)(&zbranch);
      COPY(zBranchNodeData)(&zbranch, z2zE3125.variants.zBranchNode);
      bool z2zE3145;
      {
        uint64_t z2zE3144;
        z2zE3144 = zpath_len(zkey);
        z2zE3145 = (zpos == z2zE3144);
      }
      if (z2zE3145) {
        struct zRlpFieldRefFields z2zE3146;
        z2zE3146 = zbranch.zvalue;
        z3zE3096 = zrlp_ref_content(z2zE3146);
      } else {
        bool z2zE3148;
        {
          uint64_t z2zE3147;
          z2zE3147 = zpath_len(zkey);
          z2zE3148 = (zpos < z2zE3147);
        }
        if (z2zE3148) {
          struct zByteSliceFields zchild;
          {
            struct zNodeRef z2zE3153;
            CREATE(zNodeRef)(&z2zE3153);
            {
              struct zRlpFieldRefFields z2zE3152;
              {
                zz5vecz8z5structz0zzRlpFieldRefFieldsz9 z2zE3150;
                CREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&z2zE3150);
                COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&z2zE3150, zbranch.zchildren);
                uint64_t z2zE3151;
                z2zE3151 = zpath_nibble(zkey, zpos);
                z2zE3152 = zbranch_children_get(z2zE3150, z2zE3151);
                KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&z2zE3150);
              }
              {
                zfield_to_ref(&z2zE3153, z2zE3152);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE3125);
                  KILL(zBranchNodeData)(&zbranch);
                  KILL(zNodeRef)(&z2zE3153);
                  goto end_block_exception_3157;
                }
              }
            }
            {
              zchild = zresolve_ref(z2zE3153);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE3125);
                KILL(zBranchNodeData)(&zbranch);
                KILL(zNodeRef)(&z2zE3153);
                goto end_block_exception_3157;
              }
            }
            KILL(zNodeRef)(&z2zE3153);
          }
          uint64_t z2zE3149;
          {    z2zE3149 = (zpos + UINT64_C(1));
          }
          {
            z3zE3096 = ztrie_walk(zchild, zkey, z2zE3149);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE3125);
              KILL(zBranchNodeData)(&zbranch);
              goto end_block_exception_3157;
            }
          }
        } else {  z3zE3096 = zEMPTY_SLICE;  }
      }
      KILL(zBranchNodeData)(&zbranch);
      goto finish_match_3151;
    }
  case_3153: ;
    {
      /* complete */
      z3zE3096 = zEMPTY_SLICE;
      goto finish_match_3151;
    }
  case_3152: ;
  finish_match_3151: ;
    z8zE239 = z3zE3096;
    KILL(zTrieNode)(&z2zE3125);
  }
end_function_3156: ;
  return z8zE239;
end_block_exception_3157: ;
  struct zByteSliceFields z8zE880 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE880;
}

struct zByteSliceFields ztrie_lookup(sail_fixed_bytes_32 zroot, struct zTriePath zkey)
{
  struct zByteSliceFields z8zE240;
  bool z2zE3119;
  z2zE3119 = eq_fixed_bytes_32(zroot, zEMPTY_TRIE_ROOT);
  if (z2zE3119) {  z8zE240 = zEMPTY_SLICE;  } else {
    struct zByteSliceFields znode;
    znode = znode_db_lookup(zroot);
    bool z2zE3121;
    {
      uint64_t z2zE3120;
      z2zE3120 = znode.zlen;
      z2zE3121 = (z2zE3120 == UINT64_C(0));
    }
    if (z2zE3121) {
      struct zexception z2zE3122;
      CREATE(zexception)(&z2zE3122);
      zInvalidBlock(&z2zE3122, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3122);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:320.12-320.48");
      KILL(zexception)(&z2zE3122);
      goto end_block_exception_3150;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3122);
    } else {
      {
        z8zE240 = ztrie_walk(znode, zkey, UINT64_C(0));
        if (have_exception) {  goto end_block_exception_3150;  }
      }
    }
  }
end_function_3149: ;
  return z8zE240;
end_block_exception_3150: ;
  struct zByteSliceFields z8zE881 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE881;
}

void zstateless_account(struct zoptionzIRAccountInfozK *z8zE241, sail_fixed_bytes_32 zroot, sail_fixed_bytes_20 za)
{
  struct zByteSliceFields zvalue;
  {
    struct zTriePath z2zE3118;
    {
      sail_fixed_bytes_32 z2zE3117;
      z2zE3117 = zkeccak256_address(za);
      z2zE3118 = zpath_from_hash(z2zE3117);
    }
    {
      zvalue = ztrie_lookup(zroot, z2zE3118);
      if (have_exception) {  goto end_block_exception_3148;  }
    }
  }
  bool z2zE3115;
  {
    uint64_t z2zE3114;
    z2zE3114 = zvalue.zlen;
    z2zE3115 = (z2zE3114 == UINT64_C(0));
  }
  if (z2zE3115) {  zNonezIRAccountInfozK((*(&z8zE241)), UNIT);  } else {
    struct zAccountInfo z2zE3116;
    {
      z2zE3116 = zdecode_state_account(zvalue);
      if (have_exception) {  goto end_block_exception_3148;  }
    }
    zSomezIRAccountInfozK((*(&z8zE241)), z2zE3116);
  }
end_function_3147: ;
  goto end_function_4075;
end_block_exception_3148: ;
  goto end_function_4075;
end_function_4075: ;
}

struct zAccount zk_aload(sail_fixed_bytes_20 za)
{
  struct zAccount z8zE242;
  unit z3zE3089;
  z3zE3089 = bal_note_account_touch(za);
  struct zoptionzIRAccountzK z2zE3111;
  CREATE(zoptionzIRAccountzK)(&z2zE3111);
  acct_tx_get(&z2zE3111, za);
  unit z3zE3081;
  {
    if (z2zE3111.kind != Kind_zSomezIRAccountzK) goto case_3134;
    struct zAccount zacc;
    zacc = z2zE3111.variants.zSomezIRAccountzK;
    z8zE242 = zacc;
    goto cleanup_3144;
    /* unreachable after return */
    goto finish_match_3132;
    goto end_cleanup_3145;
  cleanup_3144: ;
    KILL(zoptionzIRAccountzK)(&z2zE3111);
    goto end_function_3141;
  end_cleanup_3145: ;
  }
case_3134: ;
  {
    /* complete */
    z3zE3081 = UNIT;
    goto finish_match_3132;
  }
case_3133: ;
finish_match_3132: ;
  unit z3zE3088;
  z3zE3088 = z3zE3081;
  KILL(zoptionzIRAccountzK)(&z2zE3111);
  struct zoptionzIRAccountzK z2zE3112;
  CREATE(zoptionzIRAccountzK)(&z2zE3112);
  acct_block_get(&z2zE3112, za);
  unit z3zE3084;
  {
    if (z2zE3112.kind != Kind_zSomezIRAccountzK) goto case_3137;
    struct zAccount z3zE3094;
    z3zE3094 = z2zE3112.variants.zSomezIRAccountzK;
    z8zE242 = z3zE3094;
    goto cleanup_3142;
    /* unreachable after return */
    goto finish_match_3135;
    goto end_cleanup_3143;
  cleanup_3142: ;
    KILL(zoptionzIRAccountzK)(&z2zE3112);
    goto end_function_3141;
  end_cleanup_3143: ;
  }
case_3137: ;
  {
    /* complete */
    z3zE3084 = UNIT;
    goto finish_match_3135;
  }
case_3136: ;
finish_match_3135: ;
  unit z3zE3087;
  z3zE3087 = z3zE3084;
  KILL(zoptionzIRAccountzK)(&z2zE3112);
  struct zAccount z3zE3095;
  {
    struct zoptionzIRAccountInfozK z2zE3113;
    CREATE(zoptionzIRAccountInfozK)(&z2zE3113);
    {
      zstateless_account(&z2zE3113, zk_parent_state_root, za);
      if (have_exception) {
        KILL(zoptionzIRAccountInfozK)(&z2zE3113);
        goto end_block_exception_3146;
      }
    }
    struct zAccount z3zE3090;
    {
      if (z2zE3113.kind != Kind_zSomezIRAccountInfozK) goto case_3140;
      struct zAccountInfo zinfo;
      zinfo = z2zE3113.variants.zSomezIRAccountInfozK;
      z3zE3090 = zaccount_from_info(zinfo);
      goto finish_match_3138;
    }
  case_3140: ;
    {
      /* complete */
      z3zE3090 = zEMPTY_ACCOUNT;
      goto finish_match_3138;
    }
  case_3139: ;
  finish_match_3138: ;
    z3zE3095 = z3zE3090;
    KILL(zoptionzIRAccountInfozK)(&z2zE3113);
  }
  unit z3zE3093;
  z3zE3093 = acct_block_cache(za, z3zE3095);
  z8zE242 = z3zE3095;
end_function_3141: ;
  return z8zE242;
end_block_exception_3146: ;
  struct zAccountInfo z8zE883 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE882 = { .zcreated = false, .zinfo = z8zE883, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE882;
}

sail_u256 zstateless_storage(sail_fixed_bytes_32 zroot, sail_u256 zslot)
{
  sail_u256 z8zE243;
  struct zByteSliceFields zvalue;
  {
    struct zTriePath z2zE3110;
    {
      sail_fixed_bytes_32 z2zE3109;
      z2zE3109 = zkeccak256_word(zslot);
      z2zE3110 = zpath_from_hash(z2zE3109);
    }
    {
      zvalue = ztrie_lookup(zroot, z2zE3110);
      if (have_exception) {  goto end_block_exception_3131;  }
    }
  }
  bool z2zE3107;
  {
    uint64_t z2zE3106;
    z2zE3106 = zvalue.zlen;
    z2zE3107 = (z2zE3106 == UINT64_C(0));
  }
  if (z2zE3107) {  z8zE243 = zZERO_WORD;  } else {
    struct zRlpFieldRefFields z2zE3108;
    {
      z2zE3108 = zrlp_single_ref(zvalue);
      if (have_exception) {  goto end_block_exception_3131;  }
    }
    {
      z8zE243 = zrlp_ref_uint_word(z2zE3108);
      if (have_exception) {  goto end_block_exception_3131;  }
    }
  }
end_function_3130: ;
  return z8zE243;
end_block_exception_3131: ;

  return u256_zero();
}

struct zStorageValue zk_sload(sail_fixed_bytes_20 za, sail_u256 zs)
{
  struct zStorageValue z8zE244;
  unit z3zE3068;
  z3zE3068 = bal_note_storage_read(za, zs);
  struct zStorageKey zkey;
  zkey = zstorage_key(za, zs);
  struct zoptionzIRStorageValuezK z2zE3098;
  CREATE(zoptionzIRStorageValuezK)(&z2zE3098);
  storage_tx_get(&z2zE3098, zkey);
  unit z3zE3069;
  {
    if (z2zE3098.kind != Kind_zSomezIRStorageValuezK) goto case_3120;
    struct zStorageValue ze;
    ze = z2zE3098.variants.zSomezIRStorageValuezK;
    z8zE244 = ze;
    goto cleanup_3127;
    /* unreachable after return */
    goto finish_match_3118;
    goto end_cleanup_3128;
  cleanup_3127: ;
    KILL(zoptionzIRStorageValuezK)(&z2zE3098);
    goto end_function_3124;
  end_cleanup_3128: ;
  }
case_3120: ;
  {
    /* complete */
    z3zE3069 = UNIT;
    goto finish_match_3118;
  }
case_3119: ;
finish_match_3118: ;
  unit z3zE3072;
  z3zE3072 = z3zE3069;
  KILL(zoptionzIRStorageValuezK)(&z2zE3098);
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3129;  }
  }
  struct zoptionzIRStorageValuezK z2zE3099;
  CREATE(zoptionzIRStorageValuezK)(&z2zE3099);
  storage_block_get(&z2zE3099, zkey);
  unit z3zE3073;
  {
    if (z2zE3099.kind != Kind_zSomezIRStorageValuezK) goto case_3123;
    struct zStorageValue z3zE3080;
    z3zE3080 = z2zE3099.variants.zSomezIRStorageValuezK;
    struct zStorageValue z2zE3102;
    {
      sail_u256 z2zE3100;
      z2zE3100 = z3zE3080.zcurr;
      sail_u256 z2zE3101;
      z2zE3101 = z3zE3080.zcurr;
      struct zStorageValue z3zE3075;
      z3zE3075.zcurr = z2zE3100;
      z3zE3075.zorig = z2zE3101;
      z2zE3102 = z3zE3075;
    }
    z8zE244 = z2zE3102;
    goto cleanup_3125;
    /* unreachable after return */
    goto finish_match_3121;
    goto end_cleanup_3126;
  cleanup_3125: ;
    KILL(zoptionzIRStorageValuezK)(&z2zE3099);
    goto end_function_3124;
  end_cleanup_3126: ;
  }
case_3123: ;
  {
    /* complete */
    z3zE3073 = UNIT;
    goto finish_match_3121;
  }
case_3122: ;
finish_match_3121: ;
  unit z3zE3077;
  z3zE3077 = z3zE3073;
  KILL(zoptionzIRStorageValuezK)(&z2zE3099);
  sail_u256 zv;
  {
    bool z2zE3103;
    z2zE3103 = zacc.zstorage_cleared;
    if (z2zE3103) {  zv = zZERO_WORD;  } else {
      sail_fixed_bytes_32 z2zE3105;
      {
        struct zAccountInfo z2zE3104;
        z2zE3104 = zacc.zinfo;
        z2zE3105 = z2zE3104.zstorage_root;
      }
      {
        zv = zstateless_storage(z2zE3105, zs);
        if (have_exception) {  goto end_block_exception_3129;  }
      }
    }
  }
  unit z3zE3078;
  z3zE3078 = storage_block_cache(zkey, zv);
  struct zStorageValue z3zE3079;
  z3zE3079.zcurr = zv;
  z3zE3079.zorig = zv;
  z8zE244 = z3zE3079;
end_function_3124: ;
  return z8zE244;
end_block_exception_3129: ;
  struct zStorageValue z8zE884 = { .zcurr = u256_zero(), .zorig = u256_zero() };
  return z8zE884;
}

unit zk_sstore(sail_fixed_bytes_20 za, sail_u256 zs, struct zStorageValue zv)
{
  unit z8zE245;
  struct zStorageEntry z2zE3097;
  {
    struct zStorageKey z2zE3096;
    z2zE3096 = zstorage_key(za, zs);
    struct zStorageEntry z3zE3067;
    z3zE3067.zkey = z2zE3096;
    z3zE3067.zvalue = zv;
    z2zE3097 = z3zE3067;
  }
  z8zE245 = storage_tx_update(z2zE3097);
end_function_3116: ;
  return z8zE245;
end_block_exception_3117: ;

  return UNIT;
}

sail_u256 zk_tload(sail_fixed_bytes_20 za, sail_u256 zs)
{
  sail_u256 z8zE246;
  z8zE246 = transient_storage_read(za, zs);
end_function_3114: ;
  return z8zE246;
end_block_exception_3115: ;

  return u256_zero();
}

unit zk_tstore(sail_fixed_bytes_20 za, sail_u256 zs, sail_u256 zv)
{
  unit z8zE247;
  z8zE247 = transient_storage_write(za, zs, zv);
end_function_3112: ;
  return z8zE247;
end_block_exception_3113: ;

  return UNIT;
}

